# makefile for compiling entropy.o and entropy
#
# June 2005, M. Spiekermann - Integration of G. Zimbrao's implementation
#
# This object provides access to the OPT++ library used by SecondoPL when the
# entropy approach is used for computing correlated selectivities on predicates.

# detailed information about the library installation and setup is
# explained in the file below
include ./makefile.optlib

# compiler setup
CXX      := g++
CXXFLAGS := -c -g -fwritable-strings -Wno-deprecated -static -ansi -pedantic

ENTROPY_OBJ := entropy entropy.o tmpent.o

.PHONY: allEntropy
allEntropy: $(ENTROPY_OBJ) 

entropy: tmpent.o
	$(CXX) -o $@ $< $(OPTLIB_LIBS)
# uncoment next line to run an example
#       ./entropy 3 0.1 0.1 0.1 0.01 0.001

entropy.o tmpent.o: entropy.cpp
	$(CXX) -o entropy.o $(CXXFLAGS) -I. $(OPTPP_INCL) $(NEWMAT_INCL) $<
	$(CXX) -o tmpent.o $(CXXFLAGS) -DSTAND_ALONE -I. $(OPTPP_INCL) $(NEWMAT_INCL) $<


.PHONY: clean
clean:
	rm -f $(ENTROPY_OBJ) 
