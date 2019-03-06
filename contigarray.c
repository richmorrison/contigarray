#include "contigarray.h"
#include <stdlib.h>
#include <stdarg.h>


/* Headers for private functions */

void* assemble_array(void** index, size_t* index_avail, void** data, size_t* data_avail, size_t element_size, size_t *dims, unsigned int nDims);
int get_index_size(size_t* dims, unsigned int nDims);
int get_array_size(size_t* dims, unsigned int nDims);
void* fragment(size_t nmemb, size_t size, void** heap, size_t* avail);



/*
   Create an N dimensional array

dims            : Pointer to array of dimension lengths, nDims long
nDims           : Number of array dimensions
element_size    : Size of array elements

Returns void pointer to array.
Returns NULL on error to resemble calloc/malloc behaviour
 */
void* calloc_nD_array(size_t* dims, unsigned int nDims, size_t element_size)
{

    /* Quick sanity check of input */
    if(nDims==0) return NULL;
    if(element_size==0) return NULL;
    for(int i=0; i<nDims; i++) if(dims[i]<1) return NULL;

    /* Allocate data spaces for chunking */
    void* index_calloc = calloc(get_index_size(dims, nDims), sizeof(void*) );
    void*  data_calloc = calloc(get_array_size(dims, nDims), element_size);

    /* If either of these fail free space to prevent leak and return NULL */
    if(index_calloc==NULL || data_calloc == NULL )
    {
        if(index_calloc!=NULL) free(index_calloc);
        if(data_calloc!=NULL)  free(data_calloc);

        return NULL;
    }

    /* Calculate the size of the allocated spaces */
    size_t index_remaining = get_index_size(dims, nDims)*sizeof(void*);
    size_t data_remaining = get_array_size(dims, nDims)*element_size;

    /* Take copies of references in case a free is required (these references are changes by following function call */
    void* index = index_calloc;
    void* data = data_calloc;
    void* array = assemble_array(&index, &index_remaining, &data, &data_remaining, element_size, dims, nDims);

    /* On error free space and return NULL */
    if(array==NULL)
    {
        free(index_calloc);
        free(data_calloc);
    }

    return array;
}



/*
   Create an N dimensional array

element_size    : Size of array elements
nDims           : Number of array dimensions
...             : Dimension sizes

Returns void pointer to array.
Return NULL on error to resemble calloc/malloc behaviour

Use calloc_nD_array() if the number of dimensions is not known until runtime.
*/
void* calloc_nD_array_va(size_t element_size, unsigned int nDims, ...)
{
    if(nDims==0) return NULL;

    size_t* dims = calloc(nDims, sizeof(size_t));

    if(dims==NULL) return NULL;

    va_list ap;
    va_start(ap, nDims);

    void* array=NULL;

    for(int dim=0; dim<nDims; dim++)
        dims[dim] = (size_t) va_arg(ap, size_t);

    va_end(ap);

    array = calloc_nD_array(dims, nDims, element_size);

    free(dims);

    return array;
}

/*
   Free N dimensional array

array           : Pointer to array
nDims           : Number of dimensions associated with array

Returns Void.

If array does not point to a previously allocated array, or
to an array that has already been freed, then undefined
behaviour occurs
 */
void free_nD_array(void* array, int nDims)
{
    if(array==NULL) return;

    /* Get a pointer to the data space */
    void* data = get_1D_ref(array, nDims);

    /* Free the data space */
    free(data);
    /* Free the index space */
    free(array);

    return;
}



/*
   Get a pointer to the data space

array           : Pointer to array
nDims           : Number of dimensions associated with array

Returns void pointer to contiguous data space
 */
void* get_1D_ref(void* array, int nDims)
{
    if(array==NULL) return NULL;

    void* data_ref = array;

    /* For each dimension (except the last)
       dereference the pointer. Works because
       array[0][0]...[0] gives the first data item.
     */
    for(int i=0; i<nDims-1; i++)
        data_ref = *(void**)data_ref;

    return data_ref;
}



/*
   Calculate the size of the index structure

dims            : Pointer to array of intended dimension lengths, nDims long
nDims           : Number of array dimensions

Returns the calculated size of the index descriptor
required for a given array specification
 */
int get_index_size(size_t* dims, unsigned int nDims)
{
    int size = 0;

    for(int i=0, mult=1; i<nDims-1; i++)
    {
        mult *= dims[i];
        size += mult;
    }

    return size;
}



/*
   Calculate the size of the array

dims            : Pointer to array of intended dimension lengths, nDims long
nDims           : Number of array dimensions

Returns the calculated size of the data space
required for a given array specification
 */
int get_array_size(size_t* dims, unsigned int nDims)
{
    int size=1;

    for(int i=0; i<nDims; i++)
        size *= dims[i];

    return size;
}



/*
   Take a chunk from pre-allocated heap space

nmemb           : Number of data members to subtract from heap
size            : size of a data member
heap            : Address of heap pointer. The heap pointer is incremented by the chunk size. (Target gets modified.)
avail           : Pointer to a record of the size of the heap space remaining. (Target gets modified.)

Returns the value of heap pointer before pointer arithmetic is performed.

This function leaves side-effects on the input and could have easily
been coded in a more fucntional way. This decision was made
since it results in simpler code within other functions.
 */
void* fragment(size_t nmemb, size_t size, void** heap, size_t* avail)
{
    void* frag = *heap;

    if(size*nmemb>*avail) return NULL;

    *heap = *(char**)heap + size*nmemb;
    *avail -= size*nmemb;

    return frag;
}



/*
   Build N dimensional array using pre-allocated memory space.

index           : Address of a pointer to pre-allocated space for index structure. (Target gets modified.)
index_avail     : Pointer to a record of the size of the space remaining in index. (Target gets modified.)
data            : Address of a pointer to pre-allocated space for array data. (Target gets modified.)
data_avail      : Pointer to a record of the size of the space remaining in data. (Target gets modified.)
element_size    : Size of data elements
dims            : Pointer to array of intended dimension lengths, nDims long
nDims           : Number of array dimensions

Returns void pointer to assembled array or assembled array dimension.
Return NULL on error.

This function could be designed non-recursively using more complex pointer
arithmetic and not need to slice pre-allocated data. I like this solution
since some coding standards argue against complex pointer arithmetic and
this abstracts the arithmetic into the fragment function and those pointer
operations are inherently simpler and easier to check by inspection.
 */
void* assemble_array(void** index, size_t* index_avail, void** data, size_t* data_avail, size_t element_size, size_t* dims, unsigned int nDims)
{

    char** array = (char**) fragment(dims[0], sizeof(char*), index, index_avail);
    if(array==NULL) return NULL;

    for(int i=0; i<(int)dims[0]; i++)
    {
        if (nDims==2)
            array[i] = (char*) fragment(dims[1], element_size, data, data_avail);
        else
            array[i] = (char*) assemble_array(index, index_avail, data, data_avail, element_size, &dims[1], nDims-1);

        if(array[i]==NULL) return NULL;
    }

    return (void*)array;

}

