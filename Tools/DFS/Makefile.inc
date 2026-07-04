CC=g++
CXX=g++
ifdef SECONDO_CPP
CXX := $(SECONDO_CPP)
CC := $(SECONDO_CPP)
endif
CXXFLAGS =-I -Wall -g
LD=ld
CCO = $(CC) $(CCFLAGS)

HDFS_HOME = /home/kingster/masterarbeit/hadoop
HDFS_INCLUDE = -I$(HDFS_HOME)/include
HDFS_LIB = $(HDFS_HOME)/lib/native
