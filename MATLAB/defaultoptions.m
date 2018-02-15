function options = defaultoptions()
%DEFAULTOPTIONS create a struct of default options.
%   options = defaultoptions() returns an options struct with defaults set.
%   If modifications to the default options are needed to modify how EDGECUT
%   functions, call DEFAULTOPTIONS and modify the struct as needed.
%
%   Example:
%       options = defaultoptions();
%       options.targetSplit = 0.3;
%       options.matchingStrategy = 0;   % Random matching
%       Prob = ssget('DNVS/troll'); A = Prob.A;
%       part = edgecut(A, O);
%       sum(part)/length(part)    % 0.3000
%
%   See also EDGECUT.

error ('defaultoptions mexFunction not found') ;
