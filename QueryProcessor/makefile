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
	$(INCLUDEDIR)/AlgebraManager.h \
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h

OBJECTS=\
	QueryProcessor.$(OBJEXT) \
	SecondoCatalog.$(OBJEXT) \
	SecondoSystem.$(OBJEXT) \
	SecondoInterface.o \
	SecondoInterfaceGeneral.o

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

QueryProcessor.$(OBJEXT): QueryProcessor.cpp $(INCLUDEDIR)/CTable.h $(INCLUDEDIR)/NameIndex.h $(INCLUDEDIR)/NestedList.h $(INCLUDEDIR)/QueryProcessor.h $(INCLUDEDIR)/AlgebraManager.h $(INCLUDEDIR)/SecondoCatalog.h $(INCLUDEDIR)/SecondoSystem.h
	$(CC) -c -o QueryProcessor.$(OBJEXT) QueryProcessor.cpp $(CCFLAGS)

SecondoCatalog.$(OBJEXT): SecondoCatalog.cpp $(INCLUDEDIR)/SecondoCatalog.h $(INCLUDEDIR)/SecondoSystem.h
	$(CC) -c -o SecondoCatalog.$(OBJEXT) SecondoCatalog.cpp $(CCFLAGS)

SecondoSystem.$(OBJEXT): SecondoSystem.cpp $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/QueryProcessor.h
	$(CC) -c -o SecondoSystem.$(OBJEXT) SecondoSystem.cpp $(CCFLAGS)

SecondoInterface.o: SecondoInterface.cpp $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/SecondoCatalog.h $(INCLUDEDIR)/QueryProcessor.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/FileSystem.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/SecParser.h
	$(CC) -c -o SecondoInterface.o SecondoInterface.cpp $(CCFLAGS)
	$(CP) SecondoInterface.o $(LIBDIR)/SecondoInterface.o

SecondoInterfaceGeneral.o: SecondoInterfaceGeneral.cpp $(INCLUDEDIR)/SecondoInterface.h
	$(CC) -c -o SecondoInterfaceGeneral.o SecondoInterfaceGeneral.cpp $(CCFLAGS)
	$(CP) SecondoInterfaceGeneral.o $(LIBDIR)/SecondoInterfaceGeneral.o

include ../include.dep

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

