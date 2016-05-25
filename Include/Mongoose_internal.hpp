#ifndef MONGOOSE_INTERNAL_HPP
#define MONGOOSE_INTERNAL_HPP

#ifndef MONGOOSE_ONE
#define MONGOOSE_ONE 1.0
#endif

#ifndef MONGOOSE_ZERO
#define MONGOOSE_ZERO 0.0
#endif

#ifndef MONGOOSE_MIN2
#define MONGOOSE_MIN2(x,y) ((x)<(y) ? (x) : (y))
#endif

#ifndef MONGOOSE_MAX2
#define MONGOOSE_MAX2(x,y) ((x)>(y) ? (x) : (y))
#endif

#include <climits>
#include <cstdlib>

struct SuiteSparse_config_struct
{
    void *(*malloc_func) (size_t) ;             /* pointer to malloc */
    void *(*calloc_func) (size_t, size_t) ;     /* pointer to calloc */
    void *(*realloc_func) (void *, size_t) ;    /* pointer to realloc */
    void (*free_func) (void *) ;                /* pointer to free */
    int (*printf_func) (const char *, ...) ;    /* pointer to printf */
    double (*hypot_func) (double, double) ;     /* pointer to hypot */
    int (*divcomplex_func) (double, double, double, double, double *, double *);
};

extern struct SuiteSparse_config_struct SuiteSparse_config ;

void SuiteSparse_start ( void ) ;   /* called to start SuiteSparse */

void SuiteSparse_finish ( void ) ;  /* called to finish SuiteSparse */

void *SuiteSparse_malloc    /* pointer to allocated block of memory */
(
    size_t nitems,          /* number of items to malloc (>=1 is enforced) */
    size_t size_of_item     /* sizeof each item */
) ;

void *SuiteSparse_calloc    /* pointer to allocated block of memory */
(
    size_t nitems,          /* number of items to calloc (>=1 is enforced) */
    size_t size_of_item     /* sizeof each item */
) ;

void *SuiteSparse_realloc   /* pointer to reallocated block of memory, or
                               to original block if the realloc failed. */
(
    size_t nitems_new,      /* new number of items in the object */
    size_t nitems_old,      /* old number of items in the object */
    size_t size_of_item,    /* sizeof each item */
    void *p,                /* old object to reallocate */
    int *ok                 /* 1 if successful, 0 otherwise */
) ;

void *SuiteSparse_free      /* always returns NULL */
(
    void *p                 /* block to free */
) ;

/* returns sqrt (x^2 + y^2), computed reliably */
double SuiteSparse_hypot (double x, double y) ;

/* complex division of c = a/b */
int SuiteSparse_divcomplex
(
    double ar, double ai,   /* real and imaginary parts of a */
    double br, double bi,   /* real and imaginary parts of b */
    double *cr, double *ci  /* real and imaginary parts of c */
) ;

#include "mongoose.hpp"

#endif