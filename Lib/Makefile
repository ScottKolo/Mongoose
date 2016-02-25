#-------------------------------------------------------------------------------
# Mongoose/Lib/Makefile
#-------------------------------------------------------------------------------

include ../../SuiteSparse_config/SuiteSparse_config.mk

ARCHIVE = $(AR) $(ARFLAGS)
RANLIB = ranlib

OBJS = \
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
  mongoose_vertexseparator.o \
  mongoose_waterdance.o

CLIB = ../../SuiteSparse_config/libsuitesparseconfig.a

INC = -I../Include -I../../SuiteSparse_config -I/Library/gurobi650/mac64/include

#-------------------------------------------------------------------------------

all: libMongoose.a

library: libMongoose.a

lib: libMongoose.a

clean:
	$(RM) -f *.o

purge: distclean

distclean: clean
	$(RM) -f libMongoose.a

libMongoose.a: $(OBJS)
	$(ARCHIVE) libMongoose.a $(OBJS)
	- $(RANLIB) libMongoose.a

#-------------------------------------------------------------------------------

mongoose_cs.o: ../Source/mongoose_cs.cpp
	$(CXX) -c $(INC) $<
	
mongoose_boundaryheap.o: ../Source/mongoose_boundaryheap.cpp
	$(CXX) -c $(INC) $<

mongoose_coarsening.o: ../Source/mongoose_coarsening.cpp
	$(CXX) -c $(INC) $<

mongoose_conditioning.o: ../Source/mongoose_conditioning.cpp
	$(CXX) -c $(INC) $<

mongoose_edgeseparator.o: ../Source/mongoose_edgeseparator.cpp
	$(CXX) -c $(INC) $<

mongoose_fidduciamattheyes.o: ../Source/mongoose_fidduciamattheyes.cpp
	$(CXX) -c $(INC) $<

mongoose_guesscut.o: ../Source/mongoose_guesscut.cpp
	$(CXX) -c $(INC) $<

mongoose_interop.o: ../Source/mongoose_interop.cpp
	$(CXX) -c $(INC) $<

mongoose_matching.o: ../Source/mongoose_matching.cpp
	$(CXX) -c $(INC) $<

mongoose_nuri_qpgradproj.o: ../Source/mongoose_nuri_qpgradproj.cpp
	$(CXX) -c $(INC) $<

mongoose_qpboundary.o: ../Source/mongoose_qpboundary.cpp
	$(CXX) -c $(INC) $<

mongoose_qpgradproj.o: ../Source/mongoose_qpgradproj.cpp
	$(CXX) -c $(INC) $<

mongoose_qplinks.o: ../Source/mongoose_qplinks.cpp
	$(CXX) -c $(INC) $<

mongoose_qpmaxheap.o: ../Source/mongoose_qpmaxheap.cpp
	$(CXX) -c $(INC) $<

mongoose_qpminheap.o: ../Source/mongoose_qpminheap.cpp
	$(CXX) -c $(INC) $<

mongoose_qpnapdown.o: ../Source/mongoose_qpnapdown.cpp
	$(CXX) -c $(INC) $<

mongoose_qpnapsack.o: ../Source/mongoose_qpnapsack.cpp
	$(CXX) -c $(INC) $<

mongoose_qpnapsack2.o: ../Source/mongoose_qpnapsack2.cpp
	$(CXX) -c $(INC) $<

mongoose_qpnapup.o: ../Source/mongoose_qpnapup.cpp
	$(CXX) -c $(INC) $<

mongoose_refinement.o: ../Source/mongoose_refinement.cpp
	$(CXX) -c $(INC) $<

mongoose_vertexseparator.o: ../Source/mongoose_vertexseparator.cpp
	$(CXX) -c $(INC) $<

mongoose_waterdance.o: ../Source/mongoose_waterdance.cpp
	$(CXX) -c $(INC) $<
