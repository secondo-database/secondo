########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

INCFILES=\
	$(INCLUDEDIR)/AlgebraManager.h \
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/SecondoSystem.h

OBJECTS=\
	Algebra.$(OBJEXT) \
	AlgebraManager.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

Algebra.$(OBJEXT): Algebra.cpp $(INCFILES)
	$(CC) -c -o Algebra.$(OBJEXT) Algebra.cpp $(CCFLAGS)

AlgebraManager.$(OBJEXT): AlgebraManager.cpp AlgebraList.i $(INCFILES)
	$(CC) -c -o AlgebraManager.$(OBJEXT) AlgebraManager.cpp $(CCFLAGS)

include ../../include.dep

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

