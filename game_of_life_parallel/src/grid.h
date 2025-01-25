#ifndef GRID_H
#define GRID_H

#define SWAP_PTR(xnew,xold,xtmp) (xtmp=xnew, xnew=xold, xold=xtmp)

void find_optimal_subdivision(int nprocs, int width, int height, int* nprocx, int* nprocy);
void ghost_cell_update(unsigned char** grid, int isize, int jsize, int nleft, int nrght, int nbot, int ntop);
void grid_update(unsigned char**& grid, unsigned char**& grid_new, unsigned char** grid_tmp, int isize, int jsize);

#endif