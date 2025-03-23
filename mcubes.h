#ifndef _MARCHING_CUBES_H_
#define _MARCHING_CUBES_H_


#include "vector.h"


void mcubes_init();
#define mcubes_done()
void mcubes_polygonize(int, int, int);

#ifdef MCUBES_USE_STATIC_LISTS

void mcubes_begin(vector_t *, int *,
                  unsigned short (*)[3], int *,
                  float (*)(int, int, int));
                  
#else

void mcubes_begin(vector_t **, int *, int *,
                  unsigned short (**)[3], int *, int *,
                  float (*)(int, int, int));

#endif /* MCUBES_USE_STATIC_LISTS */

void mcubes_end();


#endif /* _MARCHING_CUBES_H_ */
