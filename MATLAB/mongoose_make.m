function mongoose_make (run_demo)
%MONGOOSE_MAKE compiles the GP mexFunctions

if (nargin < 1)
    run_demo = 1;
end

details = 0 ;	    % 1 if details of each command are to be printed

v = getversion ;

d = '' ;
is64 = (~isempty (strfind (computer, '64'))) ;
if (is64)
    % 64-bit MATLAB
    d = '-largeArrayDims' ;
end

include = '-I. -I../Include -I../External/Include -I../SuiteSparse_config' ;

% Linux/Unix require these flags for large file support
include = [include ' -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE'] ;

% We're compiling this from within a mex function.
include = [include ' -DGP_MEX_FUNCTION'] ;

lib = '-L../Lib' ;
if (isunix && ~ismac)
    % Mac doesn't need librt
    lib = [lib ' -lrt'];
end

% Fix the include & library path.
include = strrep (include, '/', filesep) ;
lib = strrep (lib, '/', filesep) ;

%-------------------------------------------------------------------------------

config_src = {
    '../SuiteSparse_config/SuiteSparse_config' };

gp_src = {
    '../Source/Mongoose_Coarsening', ...
    '../Source/Mongoose_Conditioning', ...
    '../Source/Mongoose_CSparse', ...
    '../Source/Mongoose_EdgeSeparator', ...
    '../Source/Mongoose_Graph', ...
    '../Source/Mongoose_GuessCut', ...
    '../Source/Mongoose_Matching', ...
    '../Source/Mongoose_Options', ...
    '../Source/Mongoose_Refinement', ...
    '../Source/Mongoose_Waterdance', ...
    '../Source/Mongoose_QPBoundaryHeap', ...
    '../Source/Mongoose_ImproveFM', ...
    '../Source/Mongoose_QPBoundary', ...
    '../Source/Mongoose_QPDelta', ...
    '../Source/Mongoose_QPGradProj', ...
    '../Source/Mongoose_QPLinks', ...
    '../Source/Mongoose_QPMinHeap', ...
    '../Source/Mongoose_QPMaxHeap', ...
    '../Source/Mongoose_QPNapDown', ...
    '../Source/Mongoose_QPNapUp', ...
    '../Source/Mongoose_QPNapsack', ...
    '../Source/Mongoose_ImproveQP', ...
    '../Source/Mongoose_Interop', ...
    '../Source/Mongoose_Sanitize', ...
    './mex_get_graph', ...
    './mex_get_options', ...
    './mex_getput_vector', ...
    './mex_put_options', ...
    './mex_struct_util' } ;

gp_mex_src = { 'mongoose_getDefaultOptions', ...
               'mongoose_computeEdgeSeparator', ...
               'mongoose_exportGraph', ...
               'mongoose_sanitizeMatrix', ...
               'mongoose_scc' } ;

obj_extension = '.o' ;

% compile each library source file
obj = '' ;

kk = 1 ;

fprintf('\n\nBuilding Mongoose') ;
for f = config_src
    ff = strrep (f{1}, '/', filesep) ;
    slash = strfind (ff, filesep) ;
    if (isempty (slash))
        slash = 1 ;
    else
        slash = slash (end) + 1 ;
    end
    o = ff (slash:end) ;
    obj = [obj ' ' o obj_extension] ;
    s = sprintf ('mex %s -DDLONG -O -silent %s -c %s.c', d, include, ff) ;
    kk = do_cmd (s, kk, details) ;
end

for f = gp_src
    ff = strrep (f{1}, '/', filesep) ;
    slash = strfind (ff, filesep) ;
    if (isempty (slash))
        slash = 1 ;
    else
        slash = slash (end) + 1 ;
    end
    o = ff (slash:end) ;
    obj = [obj ' ' o obj_extension] ;
    s = sprintf ('mex %s -DDLONG -O -silent %s -c %s.cpp', d, include, ff) ;
    kk = do_cmd (s, kk, details) ;
end

fprintf('\nBuilding Mongoose MEX functions');
for f = gp_mex_src
    s = sprintf ('mex %s -DDLONG -O -silent %s %s.cpp', d, include, f{1}) ;
    s = [s obj ' ' lib] ;
    kk = do_cmd (s, kk, details) ;
end

% clean up
s = ['delete ' obj] ;
do_cmd (s, kk, details) ;
fprintf ('\nMongoose successfully compiled\n') ;

% Run the demo if needed
if (run_demo)
    mongoose_demo
    fprintf ('\nMongoose demo completed successfully\n') ;
end


%-------------------------------------------------------------------------------
function kk = do_cmd (s, kk, details)
%DO_CMD: evaluate a command, and either print it or print a "."

if (details)
    fprintf ('%s\n', s) ;
else
    if (mod (kk, 60) == 0)
	fprintf ('\n') ;
    end
    kk = kk + 1 ;
    fprintf ('.') ;
end
eval (s) ;

%-------------------------------------------------------------------------------
function v = getversion
% determine the MATLAB version, and return it as a double.
v = sscanf (version, '%d.%d.%d') ;
v = 10.^(0:-1:-(length(v)-1)) * v ;
