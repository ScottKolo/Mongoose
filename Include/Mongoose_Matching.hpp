#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

enum MatchType
{
    MatchType_Orphan = 0,
    MatchType_Standard = 1,
    MatchType_Brotherly = 2,
    MatchType_Community = 3
};

void match(Graph *graph, Options *options);

void matching_Random(Graph *graph, Options *options);
void matching_HEM(Graph *graph, Options *options);
void matching_PA(Graph *graph, Options *options);
void matching_DavisPA(Graph *graph, Options *options);
void matching_LabelProp(Graph *G, Options *O);
void matching_Cleanup(Graph *graph, Options *options);

} // end namespace Mongoose

/* Mongoose Matching Macros */
#ifndef MONGOOSE_MATCHING_MACROS
#define MONGOOSE_MATCHING_MACROS

#define MONGOOSE_MATCH(a,b,t)                           \
{                                                       \
    matching[(a)] = (b)+1;                              \
    matching[(b)] = (a)+1;                              \
    invmatchmap[cn] = (a);                              \
    matchtype[(a)] = (t);                               \
    matchtype[(b)] = (t);                               \
    matchmap[(a)] = cn;                                 \
    matchmap[(b)] = cn;                                 \
    cn++;                                               \
}                                                       \

#define MONGOOSE_COMMUNITY_MATCH(a,b,t)                 \
{                                                       \
    Int vm[4] = {-1,-1,-1,-1};                          \
    vm[0] = a;                                          \
    vm[1] = graph->getMatch(vm[0]);                   \
    vm[2] = graph->getMatch(vm[1]);                   \
    vm[3] = graph->getMatch(vm[2]);                   \
                                                        \
    bool is3Way = (vm[0] == vm[3]);                     \
    if(is3Way)                                          \
    {                                                   \
        matching[vm[1]] = a+1;                          \
        MONGOOSE_MATCH(vm[2], b, t);                    \
    }                                                   \
    else                                                \
    {                                                   \
        matching[b] = matching[a];                      \
        matching[a] = b+1;                              \
        matchmap[b] = matchmap[a];                      \
        matchtype[b] = t;                               \
    }                                                   \
}                                                       \

#endif
