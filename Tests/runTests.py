#!/usr/bin/env python
from __future__ import absolute_import, division, print_function, unicode_literals
from subprocess import call
import os       # For filesystem access
import argparse # For parsing command-line arguments
import urllib   # For downloading the UFget index
import tarfile  # For un-tar/unzipping matrix files
import csv      # For reading the UFget index

parser = argparse.ArgumentParser(description='Run tests on the Mongoose library.')
parser.add_argument('-k', '--keep', action='store_true', 
                    help='do not remove downloaded files when test is complete')
parser.add_argument('-min', action='store', metavar='max_id', type=int, default=1,
                    help='minimum matrix ID to run tests on [default: 1]')
parser.add_argument('-max', action='store', metavar='min_id', type=int, default=2757,
                    help='maximum matrix ID to run tests on [default: 2757]')
parser.add_argument('-i', '--ids', action='store', nargs='+', metavar='matrix_ID', type=int,
                    help='list of matrix IDs to run tests on')
parser.add_argument('-t', '--tests', choices=['all', 'memory', 'io', 'edgesep'],
                    default='all',
                    help='choice of which tests to run')
parser.add_argument('-d', '--matrix-directory', action='store', metavar='matrix_dir',
                    default='../Matrix',
                    help='download directory for Matrix Market data files.')

args = parser.parse_args()

matrix_dir = args.matrix_directory

testfile = urllib.URLopener()
testfile.retrieve("https://www.cise.ufl.edu/research/sparse/matrices/UFstats.csv", matrix_dir + "/UFstats.csv")

SSstats = matrix_dir + "/UFstats.csv"
matrix_id = 0
id_min = args.min
id_max = args.max

# Make sure the directory ends with '/'
if not matrix_dir.endswith('/'):
    matrix_dir = matrix_dir + '/'

with open(SSstats, 'rb') as f:
    reader = csv.reader(f)
    for row in reader:
        if len(row) == 12:
            matrix_id += 1

            # Check if the matrix ID is in the proper range and that it is real and symmetric
            if matrix_id >= id_min and matrix_id <= id_max and row[2] == row[3] and row[5] == '1' and row[10] == '1':
                if args.ids is None or matrix_id in args.ids:
                    matrix_name = row[0] + '/' + row[1] + '.tar.gz'
                    gzip_path = matrix_dir + row[0] + '_' + row[1] + '.tar.gz'
                    matrix_path = matrix_dir + row[1] + '/' + row[1] + ".mtx"
                    print("Downloading " + matrix_name)

                    # Download matrix
                    testfile.retrieve("https://www.cise.ufl.edu/research/sparse/MM/" + matrix_name, gzip_path)
                    tar = tarfile.open(gzip_path, mode='r:gz')
                    matrix_files = tar.getnames()
                    tar.extractall(path=matrix_dir) # Extract the matrix from the tar.gz file
                    tar.close()

                    # Determine which test executables to run
                    if args.tests == 'all':
                        print("Calling ALL Tests...")
                        print("Calling I/O Test...")
                        call(["./tests/mongoose_test_io", matrix_path])
                        print("Calling Edge Separator Test...")
                        call(["./tests/mongoose_test_edgesep", matrix_path])
                        print("Calling Memory Test...")
                        call(["./tests/mongoose_test_memory", matrix_path])
                    elif args.tests == 'memory':
                        print("Calling Memory Test...")
                        call(["./tests/mongoose_test_memory", matrix_path])
                    elif args.tests == 'io':
                        print("Calling I/O Test...")
                        call(["./tests/mongoose_test_io", matrix_path])
                    elif args.tests == 'edgesep':
                        print("Calling Edge Separator Test...")
                        call(["./tests/mongoose_test_memory", matrix_path])

                    # Delete the matrix
                    if not args.keep:
                        for file in matrix_files:
                            os.remove(matrix_dir + file)
                            os.rmdir(matrix_dir + row[1])
                        os.remove(gzip_path)

os.remove(SSstats)