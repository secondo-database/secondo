########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.env

INCFILES=\
	AlgebraManager.h \
	Algebra.h \
	NestedList.h \
	SecondoSystem.h

OBJECTS=\
	Algebra.$(OBJEXT) \
	AlgebraManager.$(OBJEXT) \
	$(LIBDIR)/AlgebraList.$(OBJEXT)

CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS)

.PHONY: all
all: $(OBJECTS) 

Algebra.$(OBJEXT): Algebra.cpp $(INCFILES)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(LIBDIR)/AlgebraList.$(OBJEXT): AlgebraList.cpp AlgebraList.i $(INCFILES)
	$(CC) -c -o $@ $< $(CCFLAGS)

AlgebraManager.$(OBJEXT): AlgebraManager.cpp AlgebraList.i $(INCFILES)
	$(CC) -c -o $@ $< $(CCFLAGS)

include ../../include.dep

.PHONY: clean
clean:
	$(RM) $(OBJECTS)

