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

OBJECTS = FLOB.o Tuple.o # DBArray.o

.PHONY: all
all: $(OBJECTS)

DBArray.o: DBArray.cc DBArray.h
	$(CC) $(CFLAGS) -c DBArray.cc

FLOB.o: FLOB.cpp $(INCLUDEDIR)/FLOB.h $(INCLUDEDIR)/SecondoSMI.h
	$(CC) -c -o FLOB.o FLOB.cpp $(CCFLAGS)

Tuple.o: Tuple.cpp $(INCLUDEDIR)/Tuple.h $(INCLUDEDIR)/Algebra.h $(INCLUDEDIR)/SecondoSMI.h
	$(CC) $(CFLAGS) -c Tuple.cpp $(CCFLAGS)

clean: 
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.a
	$(RM) *.so
	$(RM) *.dll
