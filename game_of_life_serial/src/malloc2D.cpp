#include <stdlib.h>
#include "malloc2D.h"


unsigned char** malloc2D(int jmax, int imax, int joffset, int ioffset) {
    // Allocate a single block of memory for row pointers and array data
    unsigned char** x = (unsigned char**)malloc((jmax + joffset) * sizeof(unsigned char*) + 
                                                (jmax + joffset) * (imax + ioffset) * sizeof(unsigned char));
    if (!x) return NULL; // Check for allocation failure

    // Pointer to the data block (starting after row pointers)
    unsigned char* data = (unsigned char*)(x + jmax + joffset);

    // Assign row pointers to the appropriate positions in the data block
    for (int j = -joffset; j < jmax; j++) {
        x[j + joffset] = data + (j + joffset) * (imax + ioffset);
    }

    // Adjust the pointer to allow negative indexing for rows
    return x + joffset;
}


void malloc2D_free(unsigned char** x, int joffset){
   x -= joffset;
   free(x);
}