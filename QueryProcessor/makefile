########################################################################
#
# SECONDO makefile for query processor, catalog, system and interface
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

INCFILES=\
	../include/AlgebraManager.h \
	../include/Algebra.h \
	../include/NestedList.h \
	../include/CTable.h \
	../include/CTable.cpp

OBJECTS=\
	QueryProcessor.$(OBJEXT) \
	SecondoCatalog.$(OBJEXT) \
	SecondoSystem.$(OBJEXT) \
	SecondoInterface.$(OBJEXT) \
	SecondoInterfaceGeneral.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

QueryProcessor.$(OBJEXT): QueryProcessor.cpp $(INCFILES)
	$(CC) -c -o QueryProcessor.$(OBJEXT) QueryProcessor.cpp $(CCFLAGS)

SecondoCatalog.$(OBJEXT): SecondoCatalog.cpp $(INCFILES)
	$(CC) -c -o SecondoCatalog.$(OBJEXT) SecondoCatalog.cpp $(CCFLAGS)

SecondoSystem.$(OBJEXT): SecondoSystem.cpp $(INCFILES)
	$(CC) -c -o SecondoSystem.$(OBJEXT) SecondoSystem.cpp $(CCFLAGS)

SecondoInterface.$(OBJEXT): SecondoInterface.cpp $(INCFILES)
	$(CC) -c -o SecondoInterface.$(OBJEXT) SecondoInterface.cpp $(CCFLAGS)
	$(CP) SecondoInterface.$(OBJEXT) ../lib/SecondoInterface.$(OBJEXT)

SecondoInterfaceGeneral.$(OBJEXT): SecondoInterfaceGeneral.cpp $(INCFILES)
	$(CC) -c -o SecondoInterfaceGeneral.$(OBJEXT) SecondoInterfaceGeneral.cpp $(CCFLAGS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

