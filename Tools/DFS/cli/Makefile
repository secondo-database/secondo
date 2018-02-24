include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = cli
DEPS = ../shared/libshared.a ../dfs/libdfs.a ../commlayer/libcommlayer.a

all: $(NAME)
	
$(NAME): $(OBJS)
	g++ $(OBJS) $(DEPS) -o $(NAME)

main.o: main.cpp

clean:
	rm -f *.o *.a cli
