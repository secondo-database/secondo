.PHONY: all clean

CFLAGS += -Wall
CFLAGS += -g
CFLAGS += -O2

CC = g++

OBJS := bmodplayer.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: bmodplayer

bmodplayer: $(OBJS)
	 $(CC) $(CFLAGS) $(OBJS) $(LINKDEPS) -o bmodplayer

clean:
	rm *.o
	rm bmodplayer
