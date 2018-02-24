include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = qunitrunner-hdfs
DEPS = ../dfs/libdfs.a ../qunit/libqunit.a ../dfshdfs/libdfshdfs.a ../shared/libshared.a -L/home/kingster/masterarbeit/hadoop/lib/native/ -lhdfs
CXXFLAGS += $(HDFS_INCLUDE)

all: $(NAME)
	
$(NAME): $(OBJS)
	g++ $(OBJS) $(DEPS) -o $(NAME)

clean:
	rm -f *.o *.a qunitrunner-hdfs
