########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.config
ifdef platform
include ../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

APPLOBJECTS=\
	SecondoTTY.o \
	DisplayTTY.o

APPLICATIONS=\
	$(BINDIR)/SecondoTTYCS$(EXEEXT)

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

SecondoTTY.o: SecondoTTY.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/FileSystem.h $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/NestedList.h $(INCLUDEDIR)/DisplayTTY.h
	$(CC) -c -o SecondoTTY.o SecondoTTY.cpp $(CCXFLAGS)

DisplayTTY.o: DisplayTTY.cpp $(INCLUDEDIR)/DisplayTTY.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/AlgebraTypes.h
	$(CC) -c -o DisplayTTY.o DisplayTTY.cpp $(CCXFLAGS)

# --- Link applications

$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $(BINDIR)/SecondoTTYBDB$(EXEEXT) SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/libsdbtool.$(LIBEXT) $(LIBDIR)/libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $(BINDIR)/SecondoTTYORA$(EXEEXT) SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterface.o $(LIBDIR)/SecondoInterfaceGeneral.o -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o $(LIBDIR)/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $(BINDIR)/SecondoTTYCS$(EXEEXT) SecondoTTY.o DisplayTTY.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

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

