# This tree is built on its own and does not include makefile.env. CC used to
# hold g++ as well, i.e. a C++ compiler in the C variable; the implicit rules
# here compile C++, so only CXX is needed.
CXX ?= g++
ifdef SECONDO_CPP
CXX := $(SECONDO_CPP)
endif
CXXFLAGS = -Wall -g
AR ?= ar
LD=ld
CCO = $(CXX) $(CXXFLAGS)

# Override on the command line or in the environment:
#   make -C Tools/DFS hdfs HDFS_HOME=/opt/hadoop
# The default is the usual package location, not somebody's home directory.
HDFS_HOME ?= /usr/lib/hadoop
HDFS_INCLUDE = -I$(HDFS_HOME)/include
HDFS_LIB = $(HDFS_HOME)/lib/native
