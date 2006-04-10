# makefile for compiling C++ code used by the entropy optimizer
#
# June 2005, M. Spiekermann - Integration of G. Zimbrao's implementation
#
# April 2006, M. Spiekermann - OPT++ Library replaced by an implementation of
#             the iterative scaling algorithm. Old C++ code moved to subdirectory
#             "old".

# compiler setup
CXX      := g++
CXXFLAGS := -c -g -D_G_HAVE_BOOL -fwritable-strings -ansi -Wno-deprecated -static -pedantic

ENTROPY_OBJS := IterScale Iterative_scaling.o

.PHONY: allEntropy
allEntropy: $(ENTROPY_OBJS) 

Iterative_scaling.o: Iterative_scaling.cpp
	gcc -c $<

IterScale: Iterative_scaling.cpp
	g++ -o $@ -DSTAND_ALONE $<

.PHONY: clean
clean:
	rm -f $(ENTROPY_OBJS) 
