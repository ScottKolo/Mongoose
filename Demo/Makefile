#===============================================================================
# Mongoose/Demo/Makefile
#===============================================================================

include ../../SuiteSparse_config/SuiteSparse_config.mk

#-------------------------------------------------------------------------------

LIBS = \
    ../Lib/libMongoose.a \
    ../../SuiteSparse_config/libsuitesparseconfig.a \
    /Library/gurobi650/mac64/lib/libgurobi_c++.a \
    /Library/gurobi650/mac64/lib/libgurobi65.so \
    /usr/local/lib/libmetis.a
#    -lrt

INC = -I../Include -I../../SuiteSparse_config -I/usr/local/include

C = $(CXX) $(CF) $(INC)

#------------------------------------------------------------------------------

default: all

all: demo

demo: $(LIBS)
	$(C) Demo.cpp -o Demo $(LIBS)

purge: distclean

distclean: clean
	- $(RM) Demo

clean:
	- $(RM) $(CLEAN)
