#include <iostream>
#include "malloc2D.h"
#include "grid_update.h"
#include "io.h"


int main(int argc, char** argv) {
    int num_iter, save_movie, imax, jmax, pos_x, pos_y, status;
    unsigned char** grid, **grid_new, **grid_tmp = nullptr;
    std::string filepath;

    // Parse input arguments from the command line
    parse_input_args(argc, argv, filepath, &num_iter, &save_movie, &imax, &jmax, &pos_x, &pos_y);

    // Allocate arrays for the grid and grid updates
    grid     = malloc2D(jmax+2, imax+2, 1, 1);
    grid_new = malloc2D(jmax+2, imax+2, 1, 1);

    // (Re-)create folder for the image output
    status = system("rm -r -f output");
    if (status != 0) {
        std::cerr << "Error in executing system call rm -r -f output" << std::endl;
        abort();
    }
    status = system("mkdir output");
    if (status != 0) {
        std::cerr << "Error in executing system call mkdir output" << std::endl;
        abort();
    }

    // Read initial state (either from .ppm or from .rle file)
    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ppm")
        read_state_ppm(filepath, grid);
    else
        read_state_rle(filepath, grid, imax, jmax, pos_x, pos_y);

    // Compute iterations
    std::cout << "Computing " << num_iter << " iterations..." << std::endl;
    save_state_ppm(grid, imax, jmax, 0);
    for (int iter = 1; iter < num_iter; iter++) {
        ghost_cell_update(grid, imax, jmax);
        grid_update(grid, grid_new, grid_tmp, imax, jmax);
        save_state_ppm(grid, imax, jmax, iter);
    }

    // Save movie file
    if (save_movie) save_mov(imax, jmax);
    std::cout << "Output saved successfully." << std::endl;

    // Cleanup and finalize
    malloc2D_free(grid, 1);
    malloc2D_free(grid_new, 1);
    return 0;
}