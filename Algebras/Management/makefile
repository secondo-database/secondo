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
	../../include/AlgebraManager.h \
	../../include/Algebra.h \
	../../include/NestedList.h \
	../../include/CTable.h \
	../../include/CTable.cpp

OBJECTS=\
	Algebra.$(OBJEXT) \
	AlgebraManager.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

Algebra.$(OBJEXT): Algebra.cpp $(INCFILES)
	$(CC) -c -o Algebra.$(OBJEXT) Algebra.cpp $(CCFLAGS)

AlgebraManager.$(OBJEXT): AlgebraManager.cpp AlgebraList.i $(INCFILES) ../../include/SecondoSystem.h
	$(CC) -c -o AlgebraManager.$(OBJEXT) AlgebraManager.cpp $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

