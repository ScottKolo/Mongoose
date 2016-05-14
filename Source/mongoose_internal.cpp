#include "mongoose_internal.hpp"

struct SuiteSparse_config_struct SuiteSparse_config =
{

    /* memory management functions */
    #ifndef NMALLOC
        #ifdef MATLAB_MEX_FILE
            /* MATLAB mexFunction: */
            mxMalloc, mxCalloc, mxRealloc, mxFree,
        #else
            /* standard ANSI C: */
            malloc, calloc, realloc, free,
        #endif
    #else
        /* no memory manager defined; you must define one at run-time: */
        NULL, NULL, NULL, NULL,
    #endif

    /* printf function */
    #ifndef NPRINT
        #ifdef MATLAB_MEX_FILE
            /* MATLAB mexFunction: */
            mexPrintf,
        #else
            /* standard ANSI C: */
            printf,
        #endif
    #else
        /* printf is disabled */
        NULL,
    #endif

} ;

/* -------------------------------------------------------------------------- */
/* SuiteSparse_malloc: malloc wrapper */
/* -------------------------------------------------------------------------- */

void *SuiteSparse_malloc    /* pointer to allocated block of memory */
(
    size_t nitems,          /* number of items to malloc */
    size_t size_of_item     /* sizeof each item */
)
{
    void *p ;
    size_t size ;
    if (nitems < 1) nitems = 1 ;
    if (size_of_item < 1) size_of_item = 1 ;
    size = nitems * size_of_item  ;

    if (size != ((double) nitems) * size_of_item)
    {
        /* size_t overflow */
        p = NULL ;
    }
    else
    {
        p = (void *) (SuiteSparse_config.malloc_func) (size) ;
    }
    return (p) ;
}


/* -------------------------------------------------------------------------- */
/* SuiteSparse_calloc: calloc wrapper */
/* -------------------------------------------------------------------------- */

void *SuiteSparse_calloc    /* pointer to allocated block of memory */
(
    size_t nitems,          /* number of items to calloc */
    size_t size_of_item     /* sizeof each item */
)
{
    void *p ;
    size_t size ;
    if (nitems < 1) nitems = 1 ;
    if (size_of_item < 1) size_of_item = 1 ;
    size = nitems * size_of_item  ;

    if (size != ((double) nitems) * size_of_item)
    {
        /* size_t overflow */
        p = NULL ;
    }
    else
    {
        p = (void *) (SuiteSparse_config.calloc_func) (nitems, size_of_item) ;
    }
    return (p) ;
}

/* -------------------------------------------------------------------------- */
/* SuiteSparse_realloc: realloc wrapper */
/* -------------------------------------------------------------------------- */

/* If p is non-NULL on input, it points to a previously allocated object of
   size nitems_old * size_of_item.  The object is reallocated to be of size
   nitems_new * size_of_item.  If p is NULL on input, then a new object of that
   size is allocated.  On success, a pointer to the new object is returned,
   and ok is returned as 1.  If the allocation fails, ok is set to 0 and a
   pointer to the old (unmodified) object is returned.
 */

void *SuiteSparse_realloc   /* pointer to reallocated block of memory, or
                               to original block if the realloc failed. */
(
    size_t nitems_new,      /* new number of items in the object */
    size_t nitems_old,      /* old number of items in the object */
    size_t size_of_item,    /* sizeof each item */
    void *p,                /* old object to reallocate */
    int *ok                 /* 1 if successful, 0 otherwise */
)
{
    size_t size ;
    if (nitems_old < 1) nitems_old = 1 ;
    if (nitems_new < 1) nitems_new = 1 ;
    if (size_of_item < 1) size_of_item = 1 ;
    size = nitems_new * size_of_item  ;

    if (size != ((double) nitems_new) * size_of_item)
    {
        /* size_t overflow */
        (*ok) = 0 ;
    }
    else if (p == NULL)
    {
        /* a fresh object is being allocated */
        p = SuiteSparse_malloc (nitems_new, size_of_item) ;
        (*ok) = (p != NULL) ;
    }
    else if (nitems_old == nitems_new)
    {
        /* the object does not change; do nothing */
        (*ok) = 1 ;
    }
    else
    {
        /* change the size of the object from nitems_old to nitems_new */
        void *pnew ;
        pnew = (void *) (SuiteSparse_config.realloc_func) (p, size) ;
        if (pnew == NULL)
        {
            if (nitems_new < nitems_old)
            {
                /* the attempt to reduce the size of the block failed, but
                   the old block is unchanged.  So pretend to succeed. */
                (*ok) = 1 ;
            }
            else
            {
                /* out of memory */
                (*ok) = 0 ;
            }
        }
        else
        {
            /* success */
            p = pnew ;
            (*ok) = 1 ;
        }
    }
    return (p) ;
}

/* -------------------------------------------------------------------------- */
/* SuiteSparse_free: free wrapper */
/* -------------------------------------------------------------------------- */

void *SuiteSparse_free      /* always returns NULL */
(
    void *p                 /* block to free */
)
{
    if (p)
    {
        (SuiteSparse_config.free_func) (p) ;
    }
    return (NULL) ;
}