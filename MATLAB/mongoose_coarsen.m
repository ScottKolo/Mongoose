function [G_coarse, A_coarse] = mongoose_coarsen (G, O, A)
%MONGOOSE_COARSEN coarsen a graph.
%   partition = mongoose_computeEdgeSeparator(G) uses a multilevel hybrid
%   combinatoric and quadratic programming algorithm to compute a partitioning 
%   of the graph G. With no option struct specified, the target is for each 
%   part to contain 50% of the graph's vertices, and the coarsening is done 
%   using a combination of heavy-edge matching and other more aggressive
%   techniques to avoid stalling.
%
%   Example:
%       Prob = UFget('DNVS/troll'); A = Prob.A;
%       part = mongoose_computeEdgeSeparator(A);
%       part_a = find(part); part_b = find(1-part);
%       cspy (A (part_a,part_b)) ;
%
%   See also MONGOOSE_GETDEFAULTOPTIONS.

G_safe = mongoose_sanitizeMatrix(G);

if (nargin == 1)
    [G_coarse, A_coarse] = mongoose_coarsen_mex(G_safe);
elseif (nargin == 2)
    [G_coarse, A_coarse] = mongoose_coarsen_mex(G_safe, O);
else
    [G_coarse, A_coarse] = mongoose_coarsen_mex(G_safe, O, A);
end