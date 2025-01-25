#include <iostream>
#include "malloc2D.h"
#include "grid.h"
#include "io.h"
#include "mpi.h"


int main(int argc, char** argv) {
    int num_iter, save_movie, imax, jmax, pos_x, pos_y, nprocx, nprocy, status;
    unsigned char** grid, **grid_new, **grid_tmp = nullptr;
    std::string filepath;

    // MPI Initialization
    int rank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    if (rank == 0) std::cout << "Starting Game of Life with " << nprocs << " processes..." << std::endl;

    // Parse input arguments from the command line
    parse_input_args(argc, argv, rank, filepath, &num_iter, &save_movie, &imax, &jmax, &pos_x, &pos_y);

    // Grid setup
    find_optimal_subdivision(nprocs, imax, jmax, &nprocx, &nprocy);
    if (rank == 0) std::cout << "Setting up domain of size " << imax << "x" << jmax 
                             << " with " << nprocx << "x" << nprocy << " processes..." << std::endl;

    // Find the x- and y-coordinates of each process
    int xcoord = rank % nprocx;
    int ycoord = rank / nprocx;

    // Find the neighbor rank of each process (using periodic boundary conditions)
    int nleft = (xcoord > 0         ) ? rank - 1      : rank + (nprocx - 1);
    int nrght = (xcoord < nprocx - 1) ? rank + 1      : rank - (nprocx - 1);
    int nbot  = (ycoord > 0         ) ? rank - nprocx : rank + (nprocy - 1) * nprocx;
    int ntop  = (ycoord < nprocy - 1) ? rank + nprocx : rank - (nprocy - 1) * nprocx;

    // Find the coordinates of each grid cell for each process
    int ibegin = imax * xcoord     / nprocx;
    int iend   = imax * (xcoord+1) / nprocx;
    int isize  = iend - ibegin;
    int jbegin = jmax * ycoord     / nprocy;
    int jend   = jmax * (ycoord+1) / nprocy;
    int jsize  = jend - jbegin;

    // Allocate arrays for the grid and grid updates
    grid     = malloc2D(jsize+2, isize+2, 1, 1);
    grid_new = malloc2D(jsize+2, isize+2, 1, 1);

    // (Re-)create folder for the image output
    if (rank == 0) {
        status = system("rm -r -f output");
        if (status != 0) {
            std::cerr << "Error in executing system call rm -r -f output" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        status = system("mkdir output");
        if (status != 0) {
            std::cerr << "Error in executing system call mkdir output" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Read initial state (either from .ppm or from .rle file)
    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ppm")
        read_state_ppm(filepath, grid, rank, imax, isize, jsize, ibegin, jbegin);
    else
        read_state_rle(filepath, grid, rank, imax, jmax, pos_x, pos_y,
                       isize, jsize, ibegin, jbegin);

    // Compute the iterations and save the states as .ppm files
    if (rank == 0) std::cout << "Computing " << num_iter << " iterations..." << std::endl;
    save_state_ppm(grid, 0, rank, imax, jmax, isize, jsize, ibegin, jbegin);
    for (int iter = 1; iter < num_iter; iter++) {
        ghost_cell_update(grid, isize, jsize, nleft, nrght, nbot, ntop);
        grid_update(grid, grid_new, grid_tmp, isize, jsize);
        save_state_ppm(grid, iter, rank, imax, jmax, isize, jsize, ibegin, jbegin);
    }

    // Save movie file
    if (rank == 0) {
        if (save_movie) save_mov(imax, jmax);
        std::cout << "Output saved successfully." << std::endl;
    }
    
    // Cleanup and finalize
    malloc2D_free(grid, 1);
    malloc2D_free(grid_new, 1);
    MPI_Finalize();
    return 0;
}