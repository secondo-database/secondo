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
CXXFLAGS := -c -g -D_G_HAVE_BOOL -fwritable-strings -ansi -Wno-deprecated -static -pedantic

ENTROPY_OBJ := IterScale Iterative_scaling.o # entropy entropy.o tmpent.o

CURDIR := $(PWD)

.PHONY: allEntropy
allEntropy: $(ENTROPY_OBJ) 


entropy: tmpent.o
	$(CXX) -o $@ $< $(OPTLIB_LIBS)
# uncoment next line to run an example
#       ./entropy 3 0.1 0.1 0.1 0.01 0.001

entropy.o tmpent.o: entropy.cpp
	$(CXX) -o entropy.o $(CXXFLAGS) -I. $(OPTPP_INCL) $(NEWMAT_INCL) $<
	$(CXX) -o tmpent.o $(CXXFLAGS) -DSTAND_ALONE -I. $(OPTPP_INCL) $(NEWMAT_INCL) $<


Iterative_scaling.o: Iterative_scaling.cpp
	gcc -c $<

IterScale: Iterative_scaling.cpp
	g++ -o $@ -DSTAND_ALONE $<

.PHONY: patch
patch:
	@echo "OPTLIB is in $(OPTLIB_ROOT)"
#ifeq ($(platform),win32) 
	cp timers.c $(OPTLIB_ROOT)/src/Utils; 
	cd $(OPTLIB_ROOT)/src/Utils; \
	mv Makefile x.tmp; \
	sed -e 's#\(CFLAGS.*\)-ansi #\1#' x.tmp > Makefile; \
	rm x.tmp
#endif
	cd $(OPTLIB_ROOT); \
	tar -xzf $(CURDIR)/opt++patch.tgz;
	cd $(OPTLIB_ROOT); \
	make; \
	make tests


.PHONY: clean
clean:
	rm -f $(ENTROPY_OBJ) 
