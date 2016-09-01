#ifndef Mongoose_QPMaxHeap_hpp
#define Mongoose_QPMaxHeap_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

void QPmaxheap_build
(
    Int *heap,                      /* on input, an unsorted set of elements */
    Int size,                       /* size of the heap */
    Double *x
);

Int QPmaxheap_delete             /* return new size of heap */
(
    Int *heap,                   /* containing indices into x, 1..n on input */
    Int size,                    /* size of the heap */
    Double *x                    /* not modified */
);

void QPmaxheapify
(
    Int p,                       /* start at node p in the heap */
    Int *heap,                   /* size n, containing indices into x */
    Int size,                    /* heap [ ... nheap] is in use */
    Double *x                    /* not modified */
);

Int QPmaxheap_add
(
    Int leaf ,   /* the new leaf */
    Int *heap ,  /* size n, containing indices into x */
    Double *x ,  /* not modified */
    Int size     /* number of elements in heap not counting new one */
);

void QPmaxheap_check
(
    Int *heap,  /* vector of size n+1 */
    Double *x,  /* vector of size n */
    Int size,       /* # items in heap */
    Int n,
    Int p       /* start checking at heap [p] */
);

} // end namespace Mongoose

#endif
