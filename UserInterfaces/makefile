########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env
include ../makefile.jni

APPLOBJECTS := \
	SecondoTTY.o \
	DisplayTTY.o


ifeq ($(smitype),ora)
APPLICATIONS = $(BINDIR)/SecondoTTYORA
else
APPLICATIONS = $(BINDIR)/SecondoTTYBDB
APPLOBJECTS += TestRunner.o
endif

ifeq ($(optimizer),"true")
APPLICATIONS += $(OPTDIR)/SecondoPL
APPLOBJECTS += SecondoPL.o
endif

APPLICATIONS := $(addsuffix $(EXEEXT), $(APPLICATIONS))


.PHONY: all
all: $(APPLOBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS) testrunner

.PHONY: testrunner
testrunner: $(BINDIR)/TestRunner$(EXEEXT)

.PHONY: client
client: $(BINDIR)/SecondoTTYCS$(EXEEXT)

CCFLAGS=  $(PICFLAG) $(DEFAULTCCFLAGS)
CCXFLAGS= $(DEFAULTCCFLAGS)

# --- User Interface Applications
SecondoTTY.o: SecondoTTY.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(CCXFLAGS) $(JNIINCLUDE)

DisplayTTY.o: DisplayTTY.cpp DisplayTTY.h SecondoInterface.h AlgebraTypes.h
	$(CC) -c -o $@ $< $(CCXFLAGS)

TestRunner.o: TestRunner.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(CCXFLAGS) $(JNIINCLUDE)

SecondoPL.o: SecondoPL.cpp Application.h Profiles.h FileSystem.h SecondoSystem.h SecondoInterface.h SecondoSMI.h NestedList.h DisplayTTY.h
	$(CC) -c -o $@ $< $(PLINCLUDEFLAGS) $(CCXFLAGS) 


# --- Link applications
$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB) $(JNIINCLUDE) $(JNILINKOPTS)

$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(ORASMILIB) $(TOOLLIB) $(DEFAULTLIB) $(JNILINKOPTS) $(JNIINCLUDE)

$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o libsdbtool.$(LIBEXT)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB) $(JNIINCLUDE) $(JNILINKOPTS)

$(BINDIR)/TestRunner$(EXEEXT): TestRunner.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB) $(JNIINCLUDE) $(JNILINKOPTS)

$(OPTDIR)/SecondoPL$(EXEEXT): SecondoPL.o DisplayTTY.o SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o $(TUPLEMANAGER) libsdbtool.$(LIBEXT) libsdbsys.$(LIBEXT) $(ALGLIBFILES)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) $(PLLDFLAGS) -L$(LIBDIR) $(SDBLIB) $(ALGLIB) $(BDBSMILIB) $(BDBLIBDIR) $(BDBLIB) $(TOOLLIB) $(DEFAULTLIB) $(JNIINCLUDE) $(JNILINKOPTS)

include ../include.dep

.PHONY: clean
clean:
	$(RM) $(APPLOBJECTS) $(APPLICATIONS)

.PHONY: clean_client
	$(BINDIR)/SecondoTTYCS$(EXEEXT)
