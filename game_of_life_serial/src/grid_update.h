#ifndef GRID_UPDATE_H
#define GRID_UPDATE_H

#define SWAP_PTR(xnew,xold,xtmp) (xtmp=xnew, xnew=xold, xold=xtmp)

void ghost_cell_update(unsigned char** grid, int imax, int jmax);
void grid_update(unsigned char**& grid, unsigned char**& grid_new, unsigned char** grid_tmp, int imax, int jmax);

#endif