CC=g++
CXXFLAGS =-I -Wall -g 
LD=ld 
CCO = $(CC) $(CCFLAGS)

HDFS_HOME = /home/kingster/masterarbeit/hadoop
HDFS_INCLUDE = -I$(HDFS_HOME)/include
HDFS_LIB = $(HDFS_HOME)/lib/native
