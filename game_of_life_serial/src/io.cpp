#include <iostream>
#include <fstream>
#include <sstream>
#include "io.h"


void parse_input_args(int argc, char** argv, std::string& filepath, int* num_iter, int* save_movie,
                      int* imax, int* jmax, int* pos_x, int* pos_y) {
    if (argc < 4) {
        std::cerr << "Error: Not all necessary parameters specified."
                  << "Usage: " << argv[0] << " <path/to/initial/state> <num_iterations> <save_movie>\n"
                  << "Additional parameters when reading a .rle file: <width> <height> <pos_x> <pos_y>" << std::endl;
        abort();
    }
    filepath = argv[1];

    std::stringstream ss(argv[2]);
    if (!(ss >> *num_iter)) {
        std::cerr << "Error: Number of iterations must be an integer." << std::endl;
        abort();
    }
    std::stringstream ss2(argv[3]);
    if (!(ss2 >> *save_movie)) {
        std::cerr << "Error: save_movie must be an integer, either 0 (no) or 1 (yes)." << std::endl;
        abort();
    }
    if (*save_movie != 0 && *save_movie != 1) {
        std::cerr << "Error: save_movie must be either 0 (no) or 1 (yes)" << std::endl;
        abort();
    }

    // Check file type of initial state
    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".rle") {
        if (argc < 8) {
            std::cerr << "Error: <width> <height> <pos_x> <pos_y> are required when initial state is a .rle file!" << std::endl;
            abort();
        }
        // Read imax, jmax, pos_x and pos_y from the command line
        int extraArgs[4];
        for (int i = 0; i < 4; ++i) {
            std::stringstream extraArgStream(argv[4 + i]);
            if (!(extraArgStream >> extraArgs[i])) {
                std::cerr << "Error: Argument " << (4 + i) << " must be an integer." << std::endl;
                abort();
            }
        }
        *imax = extraArgs[0];
        *jmax = extraArgs[1];
        *pos_x = extraArgs[2];
        *pos_y = extraArgs[3];
    }
    else if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ppm") {
        // Read imax and jmax from the ppm header data
        read_ppm_headerdata(filepath, imax, jmax);
    }
    else {
        std::cerr << "Error: The file type of the initial state must be either .ppm or .rle!" << std::endl;
        abort();
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


void save_state_ppm(unsigned char** grid, int width, int height, int iter) {
    // Open the file in binary mode
    std::string filename = generateFilename(iter);
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file for writing.\n";
        abort();
    }

    // Write the header
    file << "P5\n";
    file << width << " " << height << "\n";
    file << 255 << "\n";

    // Write pixel data in binary format, row by row
    for (int j = 0; j < height; j++)
        file.write(reinterpret_cast<const char*>(grid[j]), width);
    file.close();
}


void read_state_ppm(std::string filepath, unsigned char**& grid) {
    if (filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".ppm") {
        std::cerr << "Error: Filepath must end with '.ppm'.\n";
        abort();
    }
    // Open the file in binary mode
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filepath << "\n";
        abort();
    }
    std::string format;
    file >> format;
    if (format != "P5") {
        std::cerr << "Error: Only P5 format is supported.\n";
        abort();
    }
    int width, height, max_value;
    file >> width >> height >> max_value;
    if (max_value != 255) {
        std::cerr << "Error: Only max grayscale value of 255 is supported.\n";
        abort();
    }

    // Skip the single newline character after the header
    file.ignore();

    // Read pixel data, row by row
    for (int j = 0; j < height; j++)
        file.read(reinterpret_cast<char*>(grid[j]), width);
    std::cout << "File read successfully." << std::endl;
}


void read_ppm_headerdata(std::string filepath, int* width, int* height) {
    // Only reads the header data of the .ppm file and store width and heigth in imax and jmax
    if (filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".ppm") {
        std::cerr << "Error: Filepath must end with '.ppm'.\n";
        abort();
    }
    // Open the file in binary mode
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filepath << "\n";
        abort();
    }
    std::string format;
    file >> format;
    if (format != "P5") {
        std::cerr << "Error: Only P5 format is supported.\n";
        abort();
    }
    // Read imax and jmax
    int max_value;
    file >> *width >> *height >> max_value;
    if (max_value != 255) {
        std::cerr << "Error: Only max grayscale value of 255 is supported.\n";
        abort();
    }
}


void read_state_rle(std::string filepath, unsigned char** grid, int width, int height, int pos_x, int pos_y) {
    int status;
    if (filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".rle") {
        std::cerr << "Error: Filepath must end with '.rle'.\n";
        abort();
    }

    std::string parent_path = filepath.substr(0, filepath.find_last_of("/\\"));
    parent_path = parent_path.substr(0, parent_path.find_last_of("/\\"));
    std::string src_path = parent_path + "/src";

    // Create temporary .ppm file
    std::string base_filepath = filepath.substr(0, filepath.size() - 4);
    std::stringstream cmd;
    cmd << "python3 " << src_path << "/rle_converter.py "
        << width << " " << height << " " << pos_x << " " << pos_y << " " << filepath;
    status = system(cmd.str().c_str());
    if (status != 0) {
        std::cerr << "Error in executing system call " << cmd.str() << std::endl;
        abort();
    }


    read_state_ppm(base_filepath + ".ppm", grid);

    // Remove temporary .ppm file
    cmd.str("");
    cmd << "rm " << base_filepath << ".ppm";
    status = system(cmd.str().c_str());
    if (status != 0) {
        std::cerr << "Error in executing system call " << cmd.str() << std::endl;
        abort();
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
        abort();
    }
}