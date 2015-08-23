#===============================================================================
# Mongoose/Demo/Makefile
#===============================================================================

include ../../SuiteSparse_config/SuiteSparse_config.mk

#-------------------------------------------------------------------------------

LIBS = \
    ../Lib/libMongoose.a \
    ../../SuiteSparse_config/libsuitesparseconfig.a \
    -lrt

INC = -I../Include -I../../SuiteSparse_config

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
