########################################################################
#
# SECONDO makefile for the tuple manager
#
########################################################################

include ../../makefile.env

CCFLAGS= -I$(INCLUDEDIR) $(BDBINCLUDE) $(DEFAULTCCFLAGS)

# old stuff
#CFLAGS = $(OPTIONS) -x c++ -O -I../../$(INCLUDEDIR) $(SHINCLUDE) -fno-implicit-templates
#CFLAGS = $(OPTIONS) -x c++ -O -I../../$(INCLUDEDIR) -I$(SHROOT)/include -I/opt/pdsoft/libg++-2.7.1/lib/g++-include -fno-implicit-templates

OBJECTS = FLOB.o TMTuple.o

.PHONY: all
all: $(OBJECTS)

FLOB.o: FLOB.cpp $(INCLUDEDIR)/FLOB.h $(INCLUDEDIR)/SecondoSMI.h
	$(CC) -c -o FLOB.o FLOB.cpp $(CCFLAGS)

TMTuple.o: TMTuple.cpp $(INCLUDEDIR)/TMTuple.h $(INCLUDEDIR)/Algebra.h $(INCLUDEDIR)/SecondoSMI.h
	$(CC) $(CFLAGS) -c TMTuple.cpp $(CCFLAGS)

clean: 
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.a
	$(RM) *.so
	$(RM) *.dll
	$(RM) *~
