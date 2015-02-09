.PHONY: all clean

CFLAGS += -Wall
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -pthread 

CC = g++

OBJS := bmodplayer.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: bmodplayer

bmodplayer: $(OBJS)
	 $(CC) $(CFLAGS) $(OBJS) $(LINKDEPS) -o bmodplayer

clean:
	rm *.o
	rm bmodplayer
