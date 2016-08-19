function A_safe = mongoose_sanitizeMatrix (A)
%MONGOOSE_SANITIZEMATRIX sanitize a sparse matrix.
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

error ('mongoose_sanitizeMatrix mexFunction not found') ;
