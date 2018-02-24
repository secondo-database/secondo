include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = libdfshdfs.a
CXXFLAGS += $(HDFS_INCLUDE)

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

%.o : %.c %.h
	g++

clean:
	rm -f *.o *.a
