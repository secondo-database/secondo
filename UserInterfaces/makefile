########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env

APPLOBJECTS=\
	SecondoTTY.o \
	DisplayTTY.o \
	SecondoTestFrame.o

APPLICATIONS=\
	$(BINDIR)/SecondoTTYCS$(EXEEXT) \
	$(BINDIR)/TupleTestBDB$(EXEEXT)

.PHONY: all
all: $(APPLOBJECTS)

.PHONY: buildapp
ifeq ($(smitype),ora)
buildapp: $(APPLICATIONS) $(BINDIR)/SecondoTTYORA$(EXEEXT)
else
buildapp: $(APPLICATIONS) $(BINDIR)/SecondoTTYBDB$(EXEEXT)
endif

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

# --- User Interface Applications
SecondoTestFrame.o: SecondoTestFrame.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/FileSystem.h $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/NestedList.h $(INCLUDEDIR)/DisplayTTY.h
		$(CC) -c -o $@ SecondoTestFrame.cpp $(CCXFLAGS)

SecondoTTY.o: SecondoTTY.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/FileSystem.h $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/NestedList.h $(INCLUDEDIR)/DisplayTTY.h
	$(CC) -c -o $@ SecondoTTY.cpp $(CCXFLAGS)

DisplayTTY.o: DisplayTTY.cpp $(INCLUDEDIR)/DisplayTTY.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/AlgebraTypes.h
	$(CC) -c -o $@ DisplayTTY.cpp $(CCXFLAGS)

# --- Link applications

$(BINDIR)/TupleTestBDB$(EXEEXT): SecondoTestFrame.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/AlgebraList.o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbsys.$(LIBEXT) SecondoTestFrame.o $(TUPLEMANAGER) 
	$(LD) $(EXEFLAGS) -o $@ SecondoTestFrame.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/AlgebraList.o $(TUPLEMANAGER) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)


$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/AlgebraList.o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbsys.$(LIBEXT) $(TUPLEMANAGER)
	$(LD) $(EXEFLAGS) -o $@ SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/AlgebraList.o $(TUPLEMANAGER) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/AlgebraList.o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/AlgebraList.o -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

include ../include.dep

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo

