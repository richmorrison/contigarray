#ifndef CONTIGARRAY_H
#define CONTIGARRAY_H

#include <stdlib.h>


/*
   Create an N dimensional array

dims            : Pointer to array of dimension lengths, nDims long
nDims           : Number of array dimensions
element_size    : Size of array elements

Returns void pointer to array.
Returns NULL on error to resemble calloc/malloc behaviour
 */
void* calloc_nD_array(size_t* dims, unsigned int nDims, size_t element_size);



/*
   Free N dimensional array

array           : Pointer to array
nDims           : Number of dimensions associated with array

Returns Void.

If array does not point to a previously allocated array, or
to an array that has already been freed, then undefined
behaviour occurs
 */
void free_nD_array(void* array, int nDims);



/*
   Get a pointer to the data space

array           : Pointer to array
nDims           : Number of dimensions associated with array

Returns void pointer to contiguous data space
 */
void* get_1D_ref(void* array, int nDims);

#endif
