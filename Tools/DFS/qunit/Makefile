include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = libqunit.a

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

%.o : %.c
	g++

clean:
	rm -f *.o *.a
