include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = libshared.a

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs libshared.a $(OBJS)

%.o : %.c
	g++

clean:
	rm -f *.o *.a
