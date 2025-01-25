#include <iostream>
#include <sstream>
#include "mpi.h"
#include "io.h"


void parse_input_args(int argc, char** argv, int rank, std::string& filepath, int* num_iter, int* save_movie,
                      int* imax, int* jmax, int* pos_x, int* pos_y) {
    if (argc < 4) {
        if (rank == 0) std::cerr << "Error: Not all necessary parameters specified."
                << "Usage: " << argv[0] << " <path/to/initial/state> <num_iterations> <save_movie>\n"
                << "Additional parameters when reading a .rle file: <width> <height> <pos_x> <pos_y>" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    filepath = argv[1];

    std::stringstream ss(argv[2]);
    if (!(ss >> *num_iter)) {
        if (rank == 0) std::cerr << "Error: Number of iterations must be an integer." << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    std::stringstream ss2(argv[3]);
    if (!(ss2 >> *save_movie)) {
        if (rank == 0) std::cerr << "Error: save_movie must be an integer, either 0 (no) or 1 (yes)." << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (*save_movie != 0 && *save_movie != 1) {
        if (rank == 0) std::cerr << "Error: save_movie must be either 0 (no) or 1 (yes)" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Check file type of initial state
    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".rle") {
        if (argc < 8) {
            if (rank == 0) std::cerr << "Error: <width> <height> <pos_x> <pos_y> are required when initial state is a .rle file!" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // Read imax, jmax, pos_x and pos_y from the command line
        int extraArgs[4];
        for (int i = 0; i < 4; ++i) {
            std::stringstream extraArgStream(argv[4 + i]);
            if (!(extraArgStream >> extraArgs[i])) {
                if (rank == 0) std::cerr << "Error: Argument " << (4 + i) << " must be an integer." << std::endl;
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        *imax = extraArgs[0];
        *jmax = extraArgs[1];
        *pos_x = extraArgs[2];
        *pos_y = extraArgs[3];
    }
    else if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ppm") {
        // Read imax and jmax from the ppm header data
        read_ppm_headerdata(filepath, rank, imax, jmax);
    }
    else {
        std::cerr << "Error: The file type of the initial state must be either .ppm or .rle!" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}


std::string generateFilename(int iter) {
    // Generate filename, e.g. 0000001.ppm
    int width = 7;
    std::string iterStr = std::to_string(iter);
    if (iterStr.length() < static_cast<size_t>(width))
        iterStr = std::string(width - iterStr.length(), '0') + iterStr;
    return "./output/" + iterStr + ".ppm";
}


void save_state_ppm(unsigned char** grid, int iter, int rank, int width, int height, int isize, int jsize, int ibegin, int jbegin) {
    MPI_File file;
    MPI_Offset header_size, offset;
    std::string filename = generateFilename(iter);
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

    // Rank 0 writes the PPM header and every process determines the header size to compute their offsets
    std::ostringstream header;
    header << "P5\n" << width << "\n" << height << "\n" << "255\n";
    std::string header_str = header.str();
    header_size = header_str.size();
    if (rank == 0) MPI_File_write_at(file, 0, header_str.c_str(), header_size, MPI_CHAR, MPI_STATUS_IGNORE);

    // Compute the offset for each rank and write the data
    offset = header_size + jbegin * width + ibegin;
    for (int j = 0; j < jsize; j++)
        MPI_File_write_at(file, offset + j * width, grid[j], isize, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&file);
}


void read_state_ppm(std::string filepath, unsigned char **grid, int rank, int imax, int isize, int jsize, int ibegin, int jbegin) {
    if (filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".ppm") {
        std::cerr << "Error: Filepath must end with '.ppm'.\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_File file;
    MPI_Offset header_size, offset;
    char header[256];
    int file_width, file_height, maxval;

    // Open the file and read the header
    MPI_File_open(MPI_COMM_WORLD, filepath.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read(file, header, sizeof(header), MPI_CHAR, MPI_STATUS_IGNORE);

    // Parse the header and determine the header size
    std::istringstream header_stream(header);
    std::string magic_number;
    header_stream >> magic_number >> file_width >> file_height >> maxval;
    header_stream.ignore(); // Skip the single newline character after the header
    header_size = header_stream.tellg();

    if (magic_number != "P5" || maxval != 255) {
        if (rank == 0) std::cerr << "Error: Invalid PPM file, only greyscale P5 with maxval = 255 is supported." << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Compute the offset for each process and read the pixel data in parallel
    offset = header_size + jbegin * imax + ibegin;
    for (int j = 0; j < jsize; j++)
        MPI_File_read_at(file, offset + j * imax, grid[j], isize, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&file);
    if (rank == 0) std::cout << "Reading input file successful." << std::endl;
}


void read_ppm_headerdata(std::string filepath, int rank, int* width, int* height) {
    // Only reads the header data of the .ppm file and store width and heigth in imax and jmax
    if (filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".ppm") {
        if (rank == 0) std::cerr << "Error: Filepath must end with '.ppm'.\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_File file;
    char header[256];
    int maxval;

    // Open the file and read the header
    MPI_File_open(MPI_COMM_WORLD, filepath.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read(file, header, sizeof(header), MPI_CHAR, MPI_STATUS_IGNORE);

    // Read width and height
    std::istringstream header_stream(header);
    std::string magic_number;
    header_stream >> magic_number >> *width >> *height >> maxval;

    if (magic_number != "P5" || maxval != 255) {
        if (rank == 0) std::cerr << "Error: Invalid PPM file, only greyscale P5 with maxval = 255 is supported." << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}


void read_state_rle(std::string filepath, unsigned char** grid, int rank, int imax, int jmax, int pos_x, int pos_y,
                    int isize, int jsize, int ibegin, int jbegin) {
    int status;
    if (filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".rle") {
        if (rank == 0) std::cerr << "Error: Filepath must end with '.rle'.\n";
        return;
    }
    
    std::string parent_path = filepath.substr(0, filepath.find_last_of("/\\"));
    parent_path = parent_path.substr(0, parent_path.find_last_of("/\\"));
    std::string src_path = parent_path + "/src";

    // Create temporary .ppm file
    std::string base_filepath = filepath.substr(0, filepath.size() - 4);
    std::stringstream cmd;
    cmd << "python3 " << src_path << "/rle_converter.py "
        << imax << " " << jmax << " " << pos_x << " " << pos_y << " " << filepath;
    if (rank == 0) {
        status = system(cmd.str().c_str());
        if (status != 0) {
            std::cerr << "Error in executing system call " << cmd.str() << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    read_state_ppm(base_filepath + ".ppm", grid, rank, imax, isize, jsize, ibegin, jbegin);

    // Remove temporary .ppm file
    cmd.str("");
    cmd << "rm " << base_filepath << ".ppm";
    if (rank == 0) {
        status = system(cmd.str().c_str());
        if (status != 0) {
            std::cerr << "Error in executing system call " << cmd.str() << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}


void save_mov(int width, int height) {
    int status;
    std::cout << "Saving movie..." << std::endl;
    std::stringstream cmd;
    cmd << "ffmpeg -framerate 25 -i ./output/\\%07d.ppm -c:v libx264 -crf 25 -vf scale="
        << width << ":" << height
        << ":sws_flags=neighbor,format=yuv420p -movflags +faststart ./output/movie.mp4 > /dev/null 2>&1";
    status = system(cmd.str().c_str());
    if (status != 0) {
        std::cerr << "Error in executing system call " << cmd.str() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}