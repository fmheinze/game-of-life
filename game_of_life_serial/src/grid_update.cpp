#include "grid_update.h"


void ghost_cell_update(unsigned char** grid, int imax, int jmax) {
    // Ghost cell update for periodic boundary conditions 
    for (int i = 0; i < imax; i++) {
        grid[-1][i] = grid[jmax-1][i];
        grid[jmax][i] = grid[0][i];
    }
    for (int j = 0; j < jmax; j++) {
        grid[j][-1] = grid[j][imax-1];
        grid[j][imax] = grid[j][0];
    }
    grid[-1][-1] = grid[jmax-1][imax-1];
    grid[-1][imax] = grid[jmax-1][0];
    grid[jmax][-1] = grid[0][imax-1];
    grid[jmax][imax] = grid[jmax-1][imax-1];
}


void grid_update(unsigned char**& grid, unsigned char**& grid_new, unsigned char** grid_tmp, int imax, int jmax) {
    int num_alive;
    for (int j = 0; j < jmax; j++) {
        for (int i = 0; i < imax; i++) {
            // Count the number of cells that are alive around cell (i,j) and change the state accordingly
            // (0 = dead, 255 = alive)
            num_alive = (grid[j-1][i-1] + grid[j-1][i] + grid[j-1][i+1] + grid[j][i-1] + grid[j][i+1] 
                       + grid[j+1][i-1] + grid[j+1][i] + grid[j+1][i+1]) / 255;
            if (grid[j][i] == 255) {
                if (num_alive == 2 || num_alive == 3)
                    grid_new[j][i] = 255;
                else
                    grid_new[j][i] = 0;
            }
            if (grid[j][i] == 0) {
                if (num_alive == 3)
                    grid_new[j][i] = 255;
                else
                    grid_new[j][i] = 0;
            }
        }            
    }
    SWAP_PTR(grid_new, grid, grid_tmp);
}