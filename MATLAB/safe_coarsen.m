function [G_coarse, A_coarse, map] = safe_coarsen (G, O, A)
%SAFE_COARSEN coarsen a graph after attempting to sanitize it.
%   safe_coarsen(G) computes a matching of vertices in the graph G
%   and then coarsens the graph by combining all matched vertices into
%   supervertices. With no option struct specified, the coarsening is done 
%   using a combination of heavy-edge matching and other more aggressive
%   techniques to avoid stalling. An optional vertex weight vector A can 
%   also be specified, and a fine-to-coarse mapping of vertices can also
%   be obtained (e.g. map(vertex_fine) = vertex_coarse.
%
%   Example:
%       Prob = UFget('DNVS/troll'); G = Prob.A;
%       G_coarse = safe_coarsen(G);
%       spy(G_coarse);
%
%   See also COARSEN.

G_safe = sanitize(G);

if (nargin == 1)
    [G_coarse, A_coarse, map] = coarsen(G_safe);
elseif (nargin == 2)
    [G_coarse, A_coarse, map] = coarsen(G_safe, O);
else
    [G_coarse, A_coarse, map] = coarsen(G_safe, O, A);
end