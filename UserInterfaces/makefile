########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env

APPLOBJECTS=\
	SecondoTTY.o \
	DisplayTTY.o \
	TestRunner.o
	
# this list is used for cleaning applications.
# Add new applications here which have not a
# cleaning rule of their own.
ALLAPPLICATIONS := \
	$(BINDIR)/SecondoTTYORA\
	$(BINDIR)/SecondoTTYBDB\
	$(BINDIR)/TestRunner\
	
	
ALLAPPLICATIONS := $(addsuffix $(EXEEXT), $(ALLAPPLICATIONS))

ifeq ($(smitype),ora)
APPLICATION = $(BINDIR)/SecondoTTYORA$(EXEEXT)
else
APPLICATION = $(BINDIR)/SecondoTTYBDB$(EXEEXT)
endif


.PHONY: all
all: $(APPLOBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATION) testrunner

.PHONY: testrunner
testrunner: $(BINDIR)/TestRunner$(EXEEXT)

.PHONY: secondopl
secondopl: $(OPTDIR)/SecondoPL$(EXEEXT)

.PHONY: client
client: $(BINDIR)/SecondoTTYCS$(EXEEXT)

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

# --- User Interface Applications
SecondoTTY.o: SecondoTTY.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

DisplayTTY.o: DisplayTTY.cpp DisplayTTY.h SecondoInterface.h AlgebraTypes.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

TestRunner.o: TestRunner.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

SecondoPL.o: SecondoPL.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(PLINCLUDEFLAGS) $(CCXFLAGS) 


# --- Link applications
$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)

$(BINDIR)/TestRunner$(EXEEXT): TestRunner.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

$(OPTDIR)/SecondoPL$(EXEEXT): SecondoPL.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES)
	$(LD) $(EXEFLAGS) -o $@ $(filter %.o, $^) $(PLLDFLAGS) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB)

include ../include.dep

.PHONY: clean
clean:
	$(RM) $(APPLOBJECTS) $(ALLAPPLICATIONS)

.PHONY: clean_client
clean_client:
	$(RM) $(BINDIR)/SecondoTTYCS$(EXEEXT)

.PHONY: clean_secondopl
clean_secondopl:
	$(RM) $(OPTDIR)/SecondoPL$(EXEEXT) SecondoPL.o
