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
/* SuiteSparse_start */
/* -------------------------------------------------------------------------- */

/* All applications that use SuiteSparse should call SuiteSparse_start prior
   to using any SuiteSparse function.  Only a single thread should call this
   function, in a multithreaded application.  Currently, this function is
   optional, since all this function currently does is to set the four memory
   function pointers to NULL (which tells SuiteSparse to use the default
   functions).  In a multi- threaded application, only a single thread should
   call this function.

   Future releases of SuiteSparse might enforce a requirement that
   SuiteSparse_start be called prior to calling any SuiteSparse function.
 */

void SuiteSparse_start ( void )
{

    /* memory management functions */
    #ifndef NMALLOC
        #ifdef MATLAB_MEX_FILE
            /* MATLAB mexFunction: */
            SuiteSparse_config.malloc_func  = mxMalloc ;
            SuiteSparse_config.calloc_func  = mxCalloc ;
            SuiteSparse_config.realloc_func = mxRealloc ;
            SuiteSparse_config.free_func    = mxFree ;
        #else
            /* standard ANSI C: */
            SuiteSparse_config.malloc_func  = malloc ;
            SuiteSparse_config.calloc_func  = calloc ;
            SuiteSparse_config.realloc_func = realloc ;
            SuiteSparse_config.free_func    = free ;
        #endif
    #else
        /* no memory manager defined; you must define one after calling
           SuiteSparse_start */
        SuiteSparse_config.malloc_func  = NULL ;
        SuiteSparse_config.calloc_func  = NULL ;
        SuiteSparse_config.realloc_func = NULL ;
        SuiteSparse_config.free_func    = NULL ;
    #endif

    /* printf function */
    #ifndef NPRINT
        #ifdef MATLAB_MEX_FILE
            /* MATLAB mexFunction: */
            SuiteSparse_config.printf_func = mexPrintf ;
        #else
            /* standard ANSI C: */
            SuiteSparse_config.printf_func = printf ;
        #endif
    #else
        /* printf is disabled */
        SuiteSparse_config.printf_func = NULL ;
    #endif

    /* math functions */
    SuiteSparse_config.hypot_func = SuiteSparse_hypot ;
    SuiteSparse_config.divcomplex_func = SuiteSparse_divcomplex ;
}

/* -------------------------------------------------------------------------- */
/* SuiteSparse_finish */
/* -------------------------------------------------------------------------- */

/* This currently does nothing, but in the future, applications should call
   SuiteSparse_start before calling any SuiteSparse function, and then
   SuiteSparse_finish after calling the last SuiteSparse function, just before
   exiting.  In a multithreaded application, only a single thread should call
   this function.

   Future releases of SuiteSparse might use this function for any
   SuiteSparse-wide cleanup operations or finalization of statistics.
 */

void SuiteSparse_finish ( void )
{
    /* do nothing */ ;
}

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


/* -------------------------------------------------------------------------- */
/* SuiteSparse_hypot */
/* -------------------------------------------------------------------------- */

/* There is an equivalent routine called hypot in <math.h>, which conforms
 * to ANSI C99.  However, SuiteSparse does not assume that ANSI C99 is
 * available.  You can use the ANSI C99 hypot routine with:
 *
 *  #include <math.h>
 *  SuiteSparse_config.hypot_func = hypot ;
 *
 * Default value of the SuiteSparse_config.hypot_func pointer is
 * SuiteSparse_hypot, defined below.
 *
 * s = hypot (x,y) computes s = sqrt (x*x + y*y) but does so more accurately.
 * The NaN cases for the double relops x >= y and x+y == x are safely ignored.
 * 
 * Source: Algorithm 312, "Absolute value and square root of a complex number,"
 * P. Friedland, Comm. ACM, vol 10, no 10, October 1967, page 665.
 */

double SuiteSparse_hypot (double x, double y)
{
    double s, r ;
    x = fabs (x) ;
    y = fabs (y) ;
    if (x >= y)
    {
    if (x + y == x)
    {
        s = x ;
    }
    else
    {
        r = y / x ;
        s = x * sqrt (1.0 + r*r) ;
    }
    }
    else
    {
    if (y + x == y)
    {
        s = y ;
    }
    else
    {
        r = x / y ;
        s = y * sqrt (1.0 + r*r) ;
    }
    } 
    return (s) ;
}

/* -------------------------------------------------------------------------- */
/* SuiteSparse_divcomplex */
/* -------------------------------------------------------------------------- */

/* c = a/b where c, a, and b are complex.  The real and imaginary parts are
 * passed as separate arguments to this routine.  The NaN case is ignored
 * for the double relop br >= bi.  Returns 1 if the denominator is zero,
 * 0 otherwise.
 *
 * This uses ACM Algo 116, by R. L. Smith, 1962, which tries to avoid
 * underflow and overflow.
 *
 * c can be the same variable as a or b.
 *
 * Default value of the SuiteSparse_config.divcomplex_func pointer is
 * SuiteSparse_divcomplex.
 */

int SuiteSparse_divcomplex
(
    double ar, double ai,   /* real and imaginary parts of a */
    double br, double bi,   /* real and imaginary parts of b */
    double *cr, double *ci  /* real and imaginary parts of c */
)
{
    double tr, ti, r, den ;
    if (fabs (br) >= fabs (bi))
    {
    r = bi / br ;
    den = br + r * bi ;
    tr = (ar + ai * r) / den ;
    ti = (ai - ar * r) / den ;
    }
    else
    {
    r = br / bi ;
    den = r * br + bi ;
    tr = (ar * r + ai) / den ;
    ti = (ai * r - ar) / den ;
    }
    *cr = tr ;
    *ci = ti ;
    return (den == 0.) ;
}
