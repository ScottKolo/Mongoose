#===============================================================================
# Mongoose/Tcov/Makefile
#===============================================================================

# run statement-coverage test without Valgrind
default: $(METIS) go

# to run with Valgrind as well
valgrind: $(METIS) vgo

ccode: all

include ../../SuiteSparse_config/SuiteSparse_config.mk

#-------------------------------------------------------------------------------

LIBS = \
    ../../SuiteSparse_config/libsuitesparseconfig.a
#    -lrt

INC = -I./Include -I../../SuiteSparse_config

OBJ = \
  mongoose_cs.o \
  mongoose_boundaryheap.o \
  mongoose_coarsening.o \
  mongoose_conditioning.o \
  mongoose_edgeseparator.o \
  mongoose_fidduciamattheyes.o \
  mongoose_guesscut.o \
  mongoose_interop.o \
  mongoose_matching.o \
  mongoose_nuri_qpgradproj.o \
  mongoose_qpboundary.o \
  mongoose_qpgradproj.o \
  mongoose_qplinks.o \
  mongoose_qpmaxheap.o \
  mongoose_qpminheap.o \
  mongoose_qpnapdown.o \
  mongoose_qpnapsack.o \
  mongoose_qpnapsack2.o \
  mongoose_qpnapup.o \
  mongoose_refinement.o \
  mongoose_waterdance.o

CF =  -pg \
	-O0 -g -fprofile-arcs -ftest-coverage \
	-Wall -W -Wshadow -Winline -Wno-unused-parameter \
	-Wredundant-decls -Wdisabled-optimization \
	-ansi -fexceptions

C = $(CXX) $(CF) $(INC)

all: test1

library: test1

purge: distclean

distclean: clean
	- $(RM) test1 *.mtx.txt test1 cov.out *.o *.dot *.hpp
	- $(RM) -r mongoose_*.cpp *.dSYM

clean:
	- $(RM) *.gcov *.gcda *.gcno

#------------------------------------------------------------------------------

copy_resources:
	cp ../Source/*.cpp ./
	cp ../Include/*.hpp ./

test1: test1.cpp $(OBJ)
	$(C) test1.cpp -o test1 $(OBJ) $(LIBS) -lm
	
go: copy_resources test1	
	- ./test1 ../Matrix/dwt_992.mtx > test1_dw.mtx.txt
	- ./test1 ../Matrix/G51.mtx > test1_g51.mtx.txt
	- ./cov

#------------------------------------------------------------------------------

mongoose_cs.o: mongoose_cs.cpp
	$(C) -c $(INC) $<
	
mongoose_boundaryheap.o: mongoose_boundaryheap.cpp
	$(C) -c $(INC) $<

mongoose_coarsening.o: mongoose_coarsening.cpp
	$(C) -c $(INC) $<

mongoose_conditioning.o: mongoose_conditioning.cpp
	$(C) -c $(INC) $<

mongoose_edgeseparator.o: mongoose_edgeseparator.cpp
	$(C) -c $(INC) $<

mongoose_fidduciamattheyes.o: mongoose_fidduciamattheyes.cpp
	$(C) -c $(INC) $<

mongoose_guesscut.o: mongoose_guesscut.cpp
	$(C) -c $(INC) $<

mongoose_interop.o: mongoose_interop.cpp
	$(C) -c $(INC) $<

mongoose_matching.o: mongoose_matching.cpp
	$(C) -c $(INC) $<

mongoose_nuri_qpgradproj.o: mongoose_nuri_qpgradproj.cpp
	$(C) -c $(INC) $<

mongoose_qpboundary.o: mongoose_qpboundary.cpp
	$(C) -c $(INC) $<

mongoose_qpgradproj.o: mongoose_qpgradproj.cpp
	$(C) -c $(INC) $<

mongoose_qplinks.o: mongoose_qplinks.cpp
	$(C) -c $(INC) $<

mongoose_qpmaxheap.o: mongoose_qpmaxheap.cpp
	$(C) -c $(INC) $<

mongoose_qpminheap.o: mongoose_qpminheap.cpp
	$(C) -c $(INC) $<

mongoose_qpnapdown.o: mongoose_qpnapdown.cpp
	$(C) -c $(INC) $<

mongoose_qpnapsack.o: mongoose_qpnapsack.cpp
	$(C) -c $(INC) $<

mongoose_qpnapsack2.o: mongoose_qpnapsack2.cpp
	$(C) -c $(INC) $<

mongoose_qpnapup.o: mongoose_qpnapup.cpp
	$(C) -c $(INC) $<

mongoose_refinement.o: mongoose_refinement.cpp
	$(C) -c $(INC) $<

mongoose_waterdance.o: mongoose_waterdance.cpp
	$(C) -c $(INC) $<
