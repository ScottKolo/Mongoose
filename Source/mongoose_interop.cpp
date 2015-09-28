
#include "mongoose_interop.hpp"
#include "mongoose_matching.hpp"
#include "mongoose_boundaryheap.hpp"
#include "mongoose_cs.hpp"
#include "stdio.h"
#include "string.h"

namespace SuiteSparse_Mongoose
{

/* Sanitizes a matrix market input file into a CSparse3 input file. */
void sanitize
(
    const char *mmFilename,
    const char *csFilename
);

//-----------------------------------------------------------------------------
// load a 1-indexed triplet matrix from a flat text file
//-----------------------------------------------------------------------------
cs *cs_load2 (FILE *f)
{
    if(!f) return NULL;

    double i, j ;   /* use double for integers to avoid csi conflicts */
    double x ;
    cs *T ;
    T = cs_spalloc (0, 0, 1, 1, 1) ;                    /* allocate result */

    double m, n, nz ;
    fscanf (f, "%lg %lg %lg\n", &m, &n, &nz) ;

    while (fscanf (f, "%lg %lg %lg\n", &i, &j, &x) == 3)
    {
        if(i == j) continue;
        if (!cs_entry (T, (csi) i-1, (csi) j-1, x)) return (cs_spfree (T)) ;
    }

    /* Override cs_entry with what the proclaimed values are,
     * since we strip the diagonal from the input. */
    T->m = (csi) m;
    T->n = (csi) n;

    return (T) ;
}

//-----------------------------------------------------------------------------
// Take a matrix market format file, sanitize it, and use the sanitized file
// to load a CSparse3 compressed column sparse matrix and returns it.
//-----------------------------------------------------------------------------
Graph *readGraphFromMM
(
    const char *mmFilename
)
{
    if(!mmFilename) return NULL;

    /* Build the name of the output file. */
    char outputFile[256];
    strcpy(outputFile, mmFilename);
    strcat(outputFile, ".s");

    /* Sanitize the MM file and save the result in the output file. */
    sanitize(mmFilename, outputFile);

    /* Open the output file. */
    FILE *csFile = fopen(outputFile, "r");
    if(!csFile) return NULL;
    cs *A = cs_load2(csFile);
    fclose(csFile);
    if(!A) return NULL;

    /* Compress to column pointer form. */
    cs *G = cs_compress(A);
    cs_spfree(A);

    /* Brain transplant CSparse3 matrix into an empty Mongoose Graph. */
    Graph *returner = CSparse3ToGraph(G);

    /* Return the Mongoose Graph. */
    return returner;
}

void sanitize
(
    const char *mmFilename,
    const char *csFilename
)
{
    FILE *graph = fopen(mmFilename, "r");
    if(!graph) return;
    FILE *output = fopen(csFilename, "w");
    if(!output) return;

    bool isFirst = true;
    char line[256];
    while(fgets(line, 256, graph))
    {
        if(line[0] == '%') continue;
        else if(isFirst)
        {
            fprintf(output, "%s", line);
            isFirst = false;
            continue;
        }
        else
        {
            fprintf(output, "%s", line);
        }
    }

    fclose(output);
    fclose(graph);
}

/* Configure a CSparse3 matrix from an existing Mongoose Graph. */
cs *GraphToCSparse3(Graph *G, bool copy)
{
    cs *A = (cs*) SuiteSparse_malloc(1, sizeof(cs));
    if(!A) return NULL;
    A->n = G->cs_n;
    A->m = G->cs_m;
    A->nz = G->cs_nz;
    A->nzmax = G->cs_nzmax;
    if(!copy)
    {
        A->p = (ptrdiff_t*) G->p;
        A->i = (ptrdiff_t*) G->i;
        A->x = G->x;
    }
    else
    {
        Int n = G->n;
        Int nz = G->nz;
        A->p = (ptrdiff_t*) SuiteSparse_malloc((n+1), sizeof(ptrdiff_t));
        A->i = (ptrdiff_t*) SuiteSparse_malloc(nz, sizeof(ptrdiff_t));
        A->x = (Double*) SuiteSparse_malloc(nz, sizeof(Double));
        if(!A->p || !A->i || !A->x)
        {
            cs_spfree(A);
            return NULL;
        }

        for(Int k=0; k<=n; k++)
        {
            A->p[k] = (ptrdiff_t) G->p[k];
        }
        for(Int p=0; p<nz; p++)
        {
            A->i[p] = (ptrdiff_t) G->i[p];
            A->x[p] = G->x[p];
        }
    }

    return A;
}

/* Create a new Mongoose Graph from an existing CSparse3 matrix. */
Graph *CSparse3ToGraph(cs *G, bool resetEW, bool resetNW)
{
    Graph *returner = (Graph*) SuiteSparse_calloc(1, sizeof(Graph));
    if(!returner) return NULL;
    new (returner) Graph();

    /* Brain-transplant the graph to the new representation. */
    returner->cs_n = G->n;
    returner->cs_m = G->m;
    returner->cs_nz = G->nz;
    returner->cs_nzmax = G->nzmax;
    returner->n = MONGOOSE_MAX2(G->n, G->m);
    returner->nz = G->p[G->n];
    returner->p = (Int*) G->p;
    returner->i = (Int*) G->i;
    returner->x = G->x;

    /* Allocate edge weights if necessary. */
    bool attachEdgeWeights = false;
    if(!returner->x || resetEW)
    {
        Int nz = returner->nz;
        returner->x = (Weight*) SuiteSparse_malloc(nz, sizeof(Weight));
        attachEdgeWeights = true;        
    }

    /* Allocate node weights if necessary. */
    bool attachNodeWeights = false;
    if(!returner->w || resetNW)
    {
        Int n = returner->n;
        returner->w = (Weight*) SuiteSparse_malloc(n, sizeof(Weight));
        attachNodeWeights = true;        
    }

    /* If we failed to attach weights, free the graph and return. */
    if((attachEdgeWeights && !returner->x) || (attachNodeWeights && !returner->w))
    {
        /* Undo the brain transplant, free the Graph skeleton, and return. */
        returner->p = NULL;
        returner->i = NULL;
        returner->x = NULL;
        returner->~Graph();
        SuiteSparse_free(returner);
        return NULL;
    }

    if(attachEdgeWeights)
    {
        Int nz = returner->nz;
        for(Int p=0; p<nz; p++) returner->x[p] = 1.0;
    }

    if(attachNodeWeights)
    {
        int n = returner->n;
        for(Int k=0; k<n; k++) returner->w[k] = 1.0;
    }

    return returner;
}

}