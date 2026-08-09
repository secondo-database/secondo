.PHONY: all clean

CFLAGS += -Wall
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -pthread 

LINKDEPS += $(shell curl-config --libs)

CC = g++

OBJS := bmodplayer.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< -MMD -MP -MF $*.dep

-include $(wildcard *.dep)

all: bmodplayer

bmodplayer: $(OBJS)
	 $(CC) $(CFLAGS) $(OBJS) -o bmodplayer $(LINKDEPS)

clean:
	rm -f *.o *.dep
	rm bmodplayer
