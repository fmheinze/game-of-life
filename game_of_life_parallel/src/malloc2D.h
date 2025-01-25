#ifndef _MALLOC2D_H
#define _MALLOC2D_H

   unsigned char** malloc2D(int jmax, int imax, int joffset, int ioffset);
   void malloc2D_free(unsigned char** x, int joffset);

#endif