.PHONY: all clean

CFLAGS += -Wall
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -pthread 

LINKDEPS += $(shell curl-config --libs)

CC = g++

OBJS := bmodplayer.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: bmodplayer

bmodplayer: $(OBJS)
	 $(CC) $(CFLAGS) $(OBJS) -o bmodplayer $(LINKDEPS)

clean:
	rm *.o
	rm bmodplayer
