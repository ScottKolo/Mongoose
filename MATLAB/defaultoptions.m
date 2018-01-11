function options = defaultoptions()
%DEFAULTOPTIONS create a struct of default options.
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
%   See also EDGECUT.

error ('defaultoptions mexFunction not found') ;
