########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env

APPLOBJECTS := \
	SecondoTTY.$(OBJEXT) \
	DisplayTTY.$(OBJEXT)

APPLICATIONS := $(BINDIR)/SecondoTTY$(SMIUP) \
		$(BINDIR)/SecondoTTYCS

# some optional objects and applications
ifneq ($(smitype),ora)
APPLOBJECTS += TestRunner.$(OBJEXT)
APPLICATIONS += $(BINDIR)/TestRunner
endif

ifeq ($(optimizer),"true")
APPLICATIONS += $(OPTDIR)/SecondoPL
APPLICATIONS += $(OPTDIR)/SecondoPLCS
APPLOBJECTS += SecondoPL.$(OBJEXT)
CCFLAGS += $(PLINCLUDEFLAGS)
endif

ifeq ($(readline),true)
RL_COMPILE_FLAG = -DREADLINE
RL_LD_FLAG = -lreadline -lncurses
endif


CODE_DEPENDENCIES := $(patsubst %.$(OBJEXT), %.dep, $(APPLOBJECTS))
APPLICATIONS := $(addsuffix $(EXEEXT), $(APPLICATIONS))

#
# TARGET SECTION
#

.PHONY: all
all: $(CODE_DEPENDENCIES) $(APPLOBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS)

.PHONY: TestRunner
TestRunner: $(BINDIR)/TestRunner$(EXEEXT)

.PHONY: TTYCS
TTYCS: $(BINDIR)/SecondoTTYCS$(EXEEXT)

.PHONY: TTY
TTY: $(BINDIR)/SecondoTTY$(SMIUP)$(EXEEXT)

.PHONY: optimizer
optimizer: $(OPTDIR)/SecondoPL$(EXEEXT) $(OPTDIR)/SecondoPLCS$(EXEEXT)

.PHONY: clean
clean:
	$(RM) *.dep $(APPLOBJECTS) $(APPLICATIONS)

#
# RULES SECTION
#
# create dependencies from include directives

CCFLAGS += $(DEFAULTCCFLAGS)
CCFLAGS += -fPIC

%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) $(JNIINCLUDE) $(RL_COMPILE_FLAG)


#  Rules for linkage

APP_OBJECTS :=  DisplayTTY.o SecondoInterfaceGeneral.o AlgebraList.o

$(BINDIR)/SecondoTTYBDB$(EXEEXT): $(SECLIBFILES) $(ALGLIBFILES)
$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o SecondoInterface.o $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG)


$(BINDIR)/SecondoTTYORA$(EXEEXT):  $(SECLIBFILES) $(ALGLIBFILES)
$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o SecondoInterface.o $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG)

$(BINDIR)/SecondoTTYCS$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS) $(RL_LD_FLAG)

$(BINDIR)/TestRunner$(EXEEXT): $(SECLIBFILES) $(ALGLIBFILES)
$(BINDIR)/TestRunner$(EXEEXT): TestRunner.o SecondoInterface.o  $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG)


$(OPTDIR)/SecondoPL$(EXEEXT): $(SECLIBFILES) $(ALGLIBFILES)
$(OPTDIR)/SecondoPL$(EXEEXT): SecondoPL.o SecondoInterface.o $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG) $(PLLDFLAGS)

$(OPTDIR)/SecondoPLCS$(EXEEXT): $(SECLIBFILES)
$(OPTDIR)/SecondoPLCS$(EXEEXT): SecondoPL.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS) $(PLLDFLAGS) $(RL_LD_FLAG)


include $(CODE_DEPENDENCIES:.cpp=.dep)
