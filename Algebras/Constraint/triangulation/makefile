inclpath = .

CC=gcc

CFLAGS= -UDEBUG -UCLOCK\
	-I$(inclpath) -L/lib/pa1.1 -g


# DEBUG: turn on debugging output
#
# STANDALONE: run as a separate program. read data from file.
#	      If this flag is False, then use the interface procedure 
#	      triangulate_polygon() instead.


LDFLAGS= -lm

objects= construct.o misc.o monotone.o tri.o
executable = triangulate
libfile = ../../../lib/libTriangulation.a 

.PHONY: all
all: $(libfile) # $(executable)

$(executable): $(objects)
	rm -f $(executable)
	$(CC) $(CFLAGS) $(objects) $(LDFLAGS) -o $(executable)

$(objects): $(inclpath)/triangulate.h

../../../lib/libTriangulation.a: construct.o misc.o monotone.o tri.o
	ar r $@ $^
	ranlib $@

.PHONY: clean 
clean:
	rm -f $(objects) $(libfile)

