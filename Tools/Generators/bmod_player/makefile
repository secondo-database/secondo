.PHONY: all clean

# This tool is built on its own and does not include makefile.env, so it names
# the compiler itself -- but it honours SECONDO_CPP like the rest of the tree,
# and uses CXX/CXXFLAGS rather than putting a C++ compiler in CC.
CXX ?= g++
ifdef SECONDO_CPP
CXX := $(SECONDO_CPP)
endif

CXXFLAGS += -Wall
CXXFLAGS += -g
CXXFLAGS += -O2
CXXFLAGS += -pthread

# ":=" so curl-config is run once, not on every expansion of LINKDEPS.
LINKDEPS := $(shell curl-config --libs)

OBJS := bmodplayer.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< -MMD -MP -MF $*.dep

-include $(wildcard *.dep)

all: bmodplayer

bmodplayer: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o bmodplayer $(LINKDEPS)

clean:
	rm -f *.o *.dep bmodplayer
