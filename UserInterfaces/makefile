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
	SecondoTTY.$(OBJEXT) \
	DisplayTTY.$(OBJEXT)

APPLICATIONS=\
	../bin/SecondoTTYCS$(EXEEXT)

.PHONY: all
all: $(APPLOBJECTS)

.PHONY: buildapp
ifeq ($(smitype),ora)
buildapp: $(APPLICATIONS) ../bin/SecondoTTYORA$(EXEEXT)
else
buildapp: $(APPLICATIONS) ../bin/SecondoTTYBDB$(EXEEXT)
endif

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

# --- User Interface Applications

SecondoTTY.o: SecondoTTY.cpp $(INCLUDEDIR)/Application.h $(INCLUDEDIR)/Profiles.h $(INCLUDEDIR)/FileSystem.h $(INCLUDEDIR)/SecondoSystem.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/SecondoSMI.h $(INCLUDEDIR)/NestedList.h $(INCLUDEDIR)/DisplayTTY.h
	$(CC) -c -o SecondoTTY.o SecondoTTY.cpp $(CCXFLAGS)

DisplayTTY.o: DisplayTTY.cpp $(INCLUDEDIR)/DisplayTTY.h $(INCLUDEDIR)/SecondoInterface.h $(INCLUDEDIR)/AlgebraTypes.h
	$(CC) -c -o DisplayTTY.o DisplayTTY.cpp $(CCXFLAGS)

# --- Link applications

../bin/SecondoTTYBDB$(EXEEXT): SecondoTTY.o DisplayTTY.o ../lib/SecondoInterface.$(OBJEXT) ../lib/libsdbtool.$(LIBEXT) ../lib/libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoTTYBDB$(EXEEXT) SecondoTTY.o DisplayTTY.o ../lib/SecondoInterface.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) $(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

../bin/SecondoTTYORA$(EXEEXT): SecondoTTY.o DisplayTTY.o ../lib/SecondoInterface.$(OBJEXT) ../lib/libsdbtool.$(LIBEXT) ../lib/libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoTTYORA$(EXEEXT) SecondoTTY.o DisplayTTY.o ../lib/SecondoInterface.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) $(LIBDIR) $(SDBLIB) $(ALGLIB) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

../bin/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o ../lib/SecondoInterfaceCS.$(OBJEXT) ../lib/libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o ../bin/SecondoTTYCS$(EXEEXT) SecondoTTY.o DisplayTTY.o ../lib/SecondoInterfaceCS.$(OBJEXT) ../lib/SecondoInterfaceGeneral.$(OBJEXT) $(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

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

