function [G_coarse, A_coarse, map] = mongoose_coarsen_mex (G, (O, A))
%MONGOOSE_COARSEN_MEX coarsen a graph unsafely but quickly.
%   mongoose_coarsen(G) computes a matching of vertices in the graph G
%   and then coarsens the graph by combining all matched vertices into
%   supervertices. It assumes that the matrix G provided has an all zero
%   diagonal, is symmetric, and has all positive edge weights. With no 
%   option struct specified, the coarsening is done using a combination of
%   heavy-edge matching and other more aggressive techniques to avoid 
%   stalling. An optional vertex weight vector A can also be specified. 
%   Note that mongoose_coarsen_mex does NOT check to see if the supplied 
%   matrix is of the correct form, and may provide erroneous results if 
%   used incorrectly.
%
%   Example:
%       Prob = UFget('DNVS/troll'); A = Prob.A;
%       G = mongoose_sanitizeMatrix(A);
%       G_coarse = mongoose_coarsen(G);
%       spy(G_coarse);
%
%   See also MONGOOSE_COARSEN.

error ('mongoose_coarsen_mex mexFunction not found') ;
