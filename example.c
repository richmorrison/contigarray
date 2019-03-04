#include <stdio.h>
#include "contigarray.h"


int main(void)
{
    /* Create a 3d int array of extent 3 in all dimensions */

    int ndims = 3;
    int dimx = 3, dimy = 3, dimz = 3;
    size_t dims[] = {dimx,dimy,dimz};

    int*** array = (int***) calloc_nD_array(dims, ndims, sizeof(int));

    if(array==NULL)
    {
        printf("nD array allocation failure\n\n");
        return 1;
    }

    /* Loop over dimensions, assign data and print */

    for(int x=0, n=0; x<dimx; x++) for(int y=0; y<dimy; y++) for(int z=0; z<dimz; z++, n++)
        array[x][y][z] = n;

    printf("\nPrinting via multidimensional indexing:\n");
    for(int x=0; x<dimx; x++) for(int y=0; y<dimy; y++) for(int z=0; z<dimz; z++)
        printf("%i ", array[x][y][z]);
    printf("\n");    

    /* Data is contiguous so we can access all elements as a 1d array if we so choose */

    int* _1darray = get_1D_ref(array, ndims); /* a convenience function for ((int*)&(array[0][0][0])) */

    printf("\nPrinting via 1D indexing:\n");
    for(int n=0; n<dimx*dimy*dimz; n++)
        printf("%i ", _1darray[n]);
    printf("\n");


    /* Don't forget to release memory from the heap! */

    free_nD_array((void*)array, ndims);

    return 0;
}
