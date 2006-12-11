
INC := .

CC := gcc

CFLAGS := -UDEBUG -UCLOCK -I$(INC) -g

LDFLAGS := -lm


# DEBUG: turn on debugging output
#
# STANDALONE: run as a separate program. read data from file.
#	      If this flag is False, then use the interface procedure 
#	      triangulate_polygon() instead.



OBJECTS := construct.o misc.o monotone.o tri.o
APP := triangulate
LIBFILE := ../../../lib/libTriangulation.a 

.PHONY: all
all: $(LIBFILE) # $(APP)

$(APP): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJECTS): $(INC)/triangulate.h

$(LIBFILE): construct.o misc.o monotone.o tri.o
	ar -rs $@ $^
	ranlib $@

.PHONY: clean 
clean:
	rm -f $(OBJECTS) $(APP) $(LIBFILE)

