########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.env

INCFILES=\
	$(INCLUDEDIR)/AlgebraManager.h \
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/SecondoSystem.h

OBJECTS=\
	Algebra.$(OBJEXT) \
	AlgebraList.o \
	AlgebraManager.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

Algebra.$(OBJEXT): Algebra.cpp $(INCFILES)
	$(CC) -c -o Algebra.$(OBJEXT) Algebra.cpp $(CCFLAGS)

AlgebraList.o: AlgebraList.cpp $(INCFILES)
	$(CC) -c -o AlgebraList.o AlgebraList.cpp $(CCFLAGS)
	$(CP) AlgebraList.o $(LIBDIR)/AlgebraList.o

AlgebraManager.$(OBJEXT): AlgebraManager.cpp AlgebraList.i $(INCFILES)
	$(CC) -c -o AlgebraManager.$(OBJEXT) AlgebraManager.cpp $(CCFLAGS)

include ../../include.dep

.PHONY: clean
clean:
	$(RM) *.o *.lo

