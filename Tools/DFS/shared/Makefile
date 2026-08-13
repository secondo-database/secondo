include ../Makefile.inc

SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
NAME = libshared.a

all: $(NAME)

$(NAME): $(OBJS)
	$(AR) rcs libshared.a $(OBJS)


clean:
	rm -f *.o *.a
