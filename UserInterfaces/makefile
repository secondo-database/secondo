########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env

VPATH = $(LIBDIR):$(INCLUDEDIR)

APPLOBJECTS=\
	SecondoTTY.o \
	DisplayTTY.o 


ifeq ($(smitype),ora)
APPLICATION = $(BINDIR)/SecondoTTYORA$(EXEEXT)
else
APPLICATION = $(BINDIR)/SecondoTTYBDB$(EXEEXT)
endif


.PHONY: all
all: $(APPLOBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATION)


.PHONY: client
client: $(BINDIR)/SecondoTTYCS$(EXEEXT)

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

# --- User Interface Applications
SecondoTTY.o: SecondoTTY.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

DisplayTTY.o: DisplayTTY.cpp DisplayTTY.h SecondoInterface.h AlgebraTypes.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

# --- Link applications
$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES) 
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

include ../include.dep

.PHONY: clean
clean:
	$(RM) $(APPLOBJECTS) $(APPLICATION)

.PHONY: clean_client
clean_client:
	$(RM) $(BINDIR)/SecondoTTYCS$(EXEEXT)

