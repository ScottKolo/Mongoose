function mongoose_make (run_test)
%MONGOOSE_MAKE compiles the Mongoose mex functions

if (nargin < 1)
    run_demo = 1;
end

details = 0 ;	    % 1 if details of each command are to be printed

v = getversion ;

flags = '' ;
is64 = (~isempty (strfind (computer, '64'))) ;
if (is64)
    % 64-bit MATLAB
    flags = '-largeArrayDims' ;
end

include = '-I. -I../Include -I../External/Include -I../SuiteSparse_config' ;

% Linux/Unix require these flags for large file support
flags = [flags ' -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE'] ;

% We're compiling this from within a mex function.
flags = [flags ' -DGP_MEX_FUNCTION'] ;

% Append optimization and 64-bit flags
flags = [flags ' -DDLONG -O -silent'];

lib = '-L../Lib' ;
cpp_flags = '' ;
if (isunix)
    % Check for ICC - use that if possible
    [no_icc_found, icc_path] = system('which icc');
    if(~no_icc_found)
        cpp_flags = [cpp_flags ' GCC=''' strtrim(icc_path) ''''];
    end
    if(~ismac)
        % Mac doesn't need librt
        lib = [lib ' -lrt'];
    end
end

% Fix the include & library path.
include = strrep (include, '/', filesep) ;
lib = strrep (lib, '/', filesep) ;

%-------------------------------------------------------------------------------

config_src = {
    '../SuiteSparse_config/SuiteSparse_config' };

mongoose_src = {
    '../Source/Mongoose_BoundaryHeap', ...
    '../Source/Mongoose_Coarsening', ...
    '../Source/Mongoose_CSparse', ...
    '../Source/Mongoose_EdgeSeparator', ...
    '../Source/Mongoose_Graph', ...
    '../Source/Mongoose_GuessCut', ...
    '../Source/Mongoose_Logger', ...
    '../Source/Mongoose_Matching', ...
    '../Source/Mongoose_Options', ...
    '../Source/Mongoose_Random', ...
    '../Source/Mongoose_Refinement', ...
    '../Source/Mongoose_Waterdance', ...
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
    '../Source/Mongoose_Sanitize'};

mex_util_src = {
    './mex_util/mex_get_graph', ...
    './mex_util/mex_get_options', ...
    './mex_util/mex_getput_vector', ...
    './mex_util/mex_put_options', ...
    './mex_util/mex_struct_util' } ;

mongoose_mex_src = { 
    'defaultoptions', ...
    'edgecut', ...
    'coarsen', ...
    'sanitize' } ;

% Keep track of object files
obj_list = '' ;

fprintf('\n\nBuilding Mongoose') ;

% Build SuiteSparse config
obj_files = mex_compile(config_src, 'c', flags, include, details);
obj_list = [obj_list obj_files];

% Build Mongoose
obj_files = mex_compile(mongoose_src, 'cpp', [cpp_flags flags], include, details);
obj_list = [obj_list obj_files];

fprintf('\nBuilding MEX Utilities') ;

obj_files = mex_compile(mex_util_src, 'cpp', [cpp_flags flags], include, details);
obj_list = [obj_list obj_files];

fprintf('\nBuilding Mongoose MEX functions');
for f = mongoose_mex_src
    s = sprintf ('mex %s %s %s.cpp', [cpp_flags flags], include, f{1}) ;
    s = [s obj_list ' ' lib] ;
    kk = do_cmd (s, 1, details) ;
end

% Clean up
fprintf('\nCleaning up');
s = ['delete ' obj_list] ;
do_cmd (s, 1, details) ;
fprintf ('\nMongoose successfully compiled\n') ;

% Run the demo if needed
if (run_test)
    fprintf ('\nRunning Mongoose test...\n') ;
    mongoose_test
    fprintf ('\nMongoose test completed successfully\n') ;
end

%-------------------------------------------------------------------------------
function obj_files = mex_compile (files, ext, flags, include, details)
%MEX_COMPILE: compile C/C++ files using mex and return list of obj files
kk = 1;
obj_files = '';
for f = files
    ff = strrep (f{1}, '/', filesep) ;
    slash = strfind (ff, filesep) ;
    if (isempty (slash))
        slash = 1 ;
    else
        slash = slash (end) + 1 ;
    end
    o = ff (slash:end) ;
    obj_files = [obj_files ' ' o '.o'] ;
    s = sprintf ('mex %s %s -c %s.%s', flags, include, ff, ext) ;
    kk = do_cmd (s, kk, details) ;
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
