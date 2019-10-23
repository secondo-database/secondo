

include ../../../makefile.env

CFLAGS := -UDEBUG -UCLOCK -I$(INC) -g -fPIC

LDFLAGS := -lm


# DEBUG: turn on debugging output
#
# STANDALONE: run as a separate program. read data from file.
#	      If this flag is False, then use the interface procedure 
#	      triangulate_polygon() instead.



OBJECTS := construct.o misc.o monotone.o tri.o

.PHONY: all
all: $(OBJECTS) 

$(OBJECTS): triangulate.h


.PHONY: clean 
clean:
	rm -f $(OBJECTS)
