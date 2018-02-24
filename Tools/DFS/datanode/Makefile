include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = datanode
DEPS = ../commlayer/libcommlayer.a ../shared/libshared.a

all: $(NAME)

$(NAME): $(OBJS)
	g++ $(OBJS) $(DEPS) -o $(NAME) -lpthread

%.o : %.c

clean:
	rm -f *.o *.a $(NAME)
