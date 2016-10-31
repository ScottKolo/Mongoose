#!/usr/bin/env python
from __future__ import absolute_import, print_function, unicode_literals
from subprocess import call, check_output
import os       # For filesystem access
import sys      # For sys.exit()
import argparse # For parsing command-line arguments
import urllib   # For downloading the UFget index
import tarfile  # For un-tar/unzipping matrix files
import csv      # For reading the UFget index
import shutil   # For using 'which'
from distutils.spawn import find_executable # For using find_executable

parser = argparse.ArgumentParser(
                    description='Run tests on the Mongoose library.')
parser.add_argument('-k', '--keep', 
                    action='store_true', 
                    help='do not remove downloaded files when test is complete')
parser.add_argument('-p', '--purge', 
                    action='store_true', 
                    help='force remove downloaded matrix files when complete')
parser.add_argument('-min', 
                    action='store', 
                    metavar='max_id', 
                    type=int, 
                    default=1,
                    help='minimum matrix ID to run tests on [default: 1]')
parser.add_argument('-max', 
                    action='store', 
                    metavar='min_id', 
                    type=int, 
                    default=2757,
                    help='maximum matrix ID to run tests on [default: 2757]')
parser.add_argument('-i', '--ids', 
                    action='store', 
                    nargs='+', 
                    metavar='matrix_ID', 
                    type=int,
                    help='list of matrix IDs to run tests on')
parser.add_argument('-t', '--tests',
                    choices=['all', 'memory', 'io', 'edgesep', 'performance'],
                    default='all',
                    help='choice of which tests to run')
parser.add_argument('-d', '--matrix-directory', 
                    action='store', 
                    metavar='matrix_dir',
                    default='../Matrix',
                    help='download directory for Matrix Market data files')
parser.add_argument('-c', '--coverage', 
                    action='store_true',
                    help='generate coverage information')
parser.add_argument('--html-coverage',
                    action='store_true',
                    help='generate html coverage pages if gcovr is available')
parser.add_argument('--gcov', 
                    action='store',
                    metavar='gcov_path',
                    help='path to gcov tool')

args = parser.parse_args()

# Check if we are in the right location - if not, exit
if not (os.path.isdir('./CMakeFiles/mongoose_dbg.dir/Source')
        and os.path.isdir('../Source')):
    print(
        "\n\033[91mERROR!\033[0m Looks like you might not be running this from "
        "your build directory.\n\n"
        "Make sure that... \n\n"
        "  * You are in your build directory (e.g. Mongoose/_build) and\n"
        "  * You have built Mongoose ('cmake ..' followed by 'make')\n")
    sys.exit()

# Check if Mongoose has been built - if not, exit
if not (os.path.exists('./CMakeFiles/mongoose_dbg.dir/Source/Mongoose_Graph.o')):
    print(
        "\n\033[91mERROR!\033[0m Looks like you might not have built Mongoose "
        "yet.\n\n"
        "Make sure that... \n\n"
        "  * You are in your build directory (e.g. Mongoose/_build) and\n"
        "  * You have built Mongoose ('cmake ..' followed by 'make')\n")
    sys.exit()

# Check if the supplied matrix download directory exists - if not, create it
matrix_dir = args.matrix_directory
if not os.path.exists(matrix_dir):
    os.makedirs(matrix_dir)

# Make sure the directory ends with '/'
if not matrix_dir.endswith('/'):
    matrix_dir = matrix_dir + '/'

# Get the minimum/maximum matrix ID from the arguments
id_min = args.min
id_max = args.max

# Download the matrix stats csv file
testfile = urllib.URLopener()
testfile.retrieve(
    "https://www.cise.ufl.edu/research/sparse/matrices/UFstats.csv", 
    matrix_dir + "/UFstats.csv")

stats_file = matrix_dir + "/UFstats.csv"

# Matrix IDs are not listed in the stats file - we just have to keep count
matrix_id = 0

with open(stats_file, 'rb') as f:
    reader = csv.reader(f)
    for row in reader:
        if len(row) == 12:
            matrix_id += 1

            # Check if the matrix ID is in the proper range and 
            # that the matrix is real and symmetric
            if (matrix_id >= id_min and 
                matrix_id <= id_max and 
                row[2] == row[3] and # Rows = columns (square)
                row[5] == '1' and    # Real
                row[10] == '1'):      # Numerically symmetric
                if args.ids is None or matrix_id in args.ids:
                    matrix_name = row[0] + '/' + row[1] + '.tar.gz'
                    gzip_path = matrix_dir + row[0] + '_' + row[1] + '.tar.gz'
                    matrix_path = matrix_dir + row[1] + '/' + row[1] + ".mtx"
                    
                    matrix_exists = os.path.isfile(gzip_path)
                    if matrix_exists:
                        tar = tarfile.open(gzip_path, mode='r:gz')
                        matrix_files = tar.getnames()
                    else:
                        # Download matrix if it doesn't exist
                        print("Downloading " + matrix_name)
                        testfile.retrieve("https://www.cise.ufl.edu/research/sparse/MM/" + matrix_name, gzip_path)
                        tar = tarfile.open(gzip_path, mode='r:gz')
                        tar.extractall(path=matrix_dir) # Extract the matrix from the tar.gz file
                        tar.close()

                    # Determine which test executables to run
                    if args.tests == 'all':
                        print("Calling ALL Tests...")
                        print("Calling I/O Test...")
                        call(["./tests/mongoose_test_io", matrix_path, "1"])
                        print("Calling Edge Separator Test...")
                        call(["./tests/mongoose_test_edgesep", matrix_path])
                        print("Calling Memory Test...")
                        call(["./tests/mongoose_test_memory", matrix_path])
                        print("Calling Performance Test...")
                        call(["./tests/mongoose_test_performance", matrix_path])
                    elif args.tests == 'memory':
                        print("Calling Memory Test...")
                        call(["./tests/mongoose_test_memory", matrix_path])
                    elif args.tests == 'io':
                        print("Calling I/O Test...")
                        call(["./tests/mongoose_test_io", matrix_path, "1"])
                    elif args.tests == 'edgesep':
                        print("Calling Edge Separator Test...")
                        call(["./tests/mongoose_test_edgesep", matrix_path])
                    elif args.tests == 'performance':
                        print("Calling Performance Test...")
                        call(["./tests/mongoose_test_performance", matrix_path])

                    # Delete the matrix only if we downloaded it and the keep
                    # flag is off
                    if args.purge or not (args.keep or matrix_exists):
                        files = os.listdir(matrix_dir + row[1])
                        for file in files:
                            os.remove(os.path.join(matrix_dir + row[1] + '/', file))
                        os.rmdir(matrix_dir + row[1])
                        os.remove(gzip_path)

os.remove(stats_file)

if args.coverage or args.html_coverage:
    if args.gcov:
        gcov = args.gcov
    else:
        gcov = find_executable('gcov')

    if gcov:
        # Determine if we are using GCC gcov or LLVM gcov
        gcov_version = check_output([gcov, "--version"])
        if gcov_version.find('LLVM') == -1:
            call([gcov + " -o ./CMakeFiles/mongoose_dbg.dir/Source ../Source/*.cpp"], shell=True)
        else:
            call(gcov + " -o=./CMakeFiles/mongoose_dbg.dir/Source ../Source/*.cpp", shell=True)
    
    gcovr = find_executable('gcovr')
    if gcovr:
        if args.html_coverage:
            print("Running gcovr with HTML generation")
            call([gcovr, 
                  "--html",
                  "--html-details", 
                  "--output=coverage.html",
                  "--gcov-executable=" + gcov, 
                  "--object-directory=CMakeFiles/mongoose_dbg.dir/Source", 
                  "--root=../Source/"])
        else:
            print("Running gcovr without HTML generation")
            call([gcovr, "--gcov-executable=" + gcov, "--object-directory=CMakeFiles/mongoose_dbg.dir/Source", "--root=../Source/"])
