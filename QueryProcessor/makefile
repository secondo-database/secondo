########################################################################
#
# SECONDO makefile for query processor, catalog, system and interface
#
########################################################################

include ../makefile.env

INCFILES=\
	$(INCLUDEDIR)/AlgebraManager.h \
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h

OBJECTS=\
	QueryProcessor.$(OBJEXT) \
	SecondoCatalog.$(OBJEXT) \
	SecondoSystem.$(OBJEXT) \
	$(LIBDIR)/SecondoInterface.$(OBJEXT) \
	$(LIBDIR)/SecondoInterfaceGeneral.$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

QueryProcessor.$(OBJEXT): QueryProcessor.cpp CTable.h NameIndex.h NestedList.h QueryProcessor.h AlgebraManager.h SecondoCatalog.h SecondoSystem.h
	$(CC) -c -o $@ $< $(CCFLAGS)

SecondoCatalog.$(OBJEXT): SecondoCatalog.cpp SecondoCatalog.h SecondoSystem.h
	$(CC) -c -o $@ $< $(CCFLAGS)

SecondoSystem.$(OBJEXT): SecondoSystem.cpp SecondoSystem.h QueryProcessor.h
	$(CC) -c -o $@ $< $(CCFLAGS)

$(LIBDIR)/SecondoInterface.$(OBJEXT): SecondoInterface.cpp SecondoInterface.h SecondoSystem.h SecondoCatalog.h QueryProcessor.h Profiles.h FileSystem.h SecondoSMI.h SecParser.h
	$(CC) -c -o $@ $< $(CCFLAGS)

$(LIBDIR)/SecondoInterfaceGeneral.$(OBJEXT): SecondoInterfaceGeneral.cpp SecondoInterface.h
	$(CC) -c -o $@ $< $(CCFLAGS)

include ../include.dep

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
