#ifndef Mongoose_QPMinHeap_hpp
#define Mongoose_QPMinHeap_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

void QPminheap_build
(
    Int *heap,                      /* on input, an unsorted set of elements */
    Int size,                       /* size of the heap */
    double *x
);

Int QPminheap_delete             /* return new size of heap */
(
    Int *heap,                   /* containing indices into x, 1..n on input */
    Int size,                    /* size of the heap */
    double *x                    /* not modified */
);

void QPminheapify
(
    Int p,                       /* start at node p in the heap */
    Int *heap,                   /* size n, containing indices into x */
    Int size,                    /* heap [ ... nheap] is in use */
    double *x                    /* not modified */
);

Int QPminheap_add
(
    Int leaf ,   /* the new leaf */
    Int *heap ,  /* size n, containing indices into x */
    double *x ,  /* not modified */
    Int size     /* number of elements in heap not counting new one */
);

void QPminheap_check
(
    Int *heap,  /* vector of size n+1 */
    double *x,  /* vector of size n */
    Int size,       /* # items in heap */
    Int n,
    Int p       /* start checking at heap [p] */
);

} // end namespace Mongoose

#endif
