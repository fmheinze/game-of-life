#include "mpi.h"
#include "grid.h"
#include "math.h"


void find_optimal_subdivision(int nprocs, int width, int height, int* nprocx, int* nprocy) {
    // Subdivide grid of size width x heigth into nprocs = nprocx x nprocy subdomains of equal size
    // such that each subdomain is as close to a square as possible
    int nx, ny;
    int best_nx = 1, best_ny = nprocs;
    double aspect_ratio, best_aspect_ratio = 1e9;

    for (int i = 1; i <= nprocs; i++) {
        if (nprocs % i == 0) {
            nx = i;
            ny = nprocs / i;
            aspect_ratio = fmax((double)width / nx / (height / ny), (double)height / ny / (width / nx));
            if (aspect_ratio < best_aspect_ratio) {
                best_aspect_ratio = aspect_ratio;
                best_nx = nx;
                best_ny = ny;
            }
        }
    }
    *nprocx = best_nx;
    *nprocy = best_ny;
}


void ghost_cell_update(unsigned char** grid, int isize, int jsize, int nleft, int nrght, int nbot, int ntop) {
    MPI_Request request[4];
    MPI_Status status[4];

    // Send and receive buffers for left and right communication
    unsigned char xbuf_left_send[jsize];
    unsigned char xbuf_rght_send[jsize];
    unsigned char xbuf_rght_recv[jsize];
    unsigned char xbuf_left_recv[jsize];

    // Fill the send buffers for left and right communication
    for (int j = 0; j < jsize; j++) {
        xbuf_left_send[j] = grid[j][0];
        xbuf_rght_send[j] = grid[j][isize - 1];
    }

    // Left and right communication
    MPI_Irecv(&xbuf_rght_recv, jsize, MPI_UNSIGNED_CHAR, nrght, 1, MPI_COMM_WORLD, &request[0]);
    MPI_Isend(&xbuf_left_send, jsize, MPI_UNSIGNED_CHAR, nleft, 1, MPI_COMM_WORLD, &request[1]);
    MPI_Irecv(&xbuf_left_recv, jsize, MPI_UNSIGNED_CHAR, nleft, 2, MPI_COMM_WORLD, &request[2]);
    MPI_Isend(&xbuf_rght_send, jsize, MPI_UNSIGNED_CHAR, nrght, 2, MPI_COMM_WORLD, &request[3]);
    MPI_Waitall(4, request, status);

    // Copy the receive buffers for the left and right communication into the ghost cells
    for (int j = 0; j < jsize; j++){
        grid[j][isize] = xbuf_rght_recv[j];
        grid[j][-1] = xbuf_left_recv[j];
    }

    // Send the row data for top and bottom communication directly (including the corner cells)
    int bufcount = isize + 2;
    MPI_Irecv(&grid[jsize][-1],   bufcount, MPI_UNSIGNED_CHAR, nbot, 3, MPI_COMM_WORLD, &request[0]);
    MPI_Isend(&grid[0][-1],       bufcount, MPI_UNSIGNED_CHAR, ntop, 3, MPI_COMM_WORLD, &request[1]);
    MPI_Irecv(&grid[-1][-1],      bufcount, MPI_UNSIGNED_CHAR, ntop, 4, MPI_COMM_WORLD, &request[2]);
    MPI_Isend(&grid[jsize-1][-1], bufcount, MPI_UNSIGNED_CHAR, nbot, 4, MPI_COMM_WORLD, &request[3]);
    MPI_Waitall(4, request, status);
}


void grid_update(unsigned char**& grid, unsigned char**& grid_new, unsigned char** grid_tmp, int isize, int jsize) {
    int num_alive;
    for (int j = 0; j < jsize; j++) {
        for (int i = 0; i < isize; i++) {
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