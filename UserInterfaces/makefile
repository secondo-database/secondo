########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env
include ../makefile.jni

APPLOBJECTS := \
	SecondoTTY.$(OBJEXT) \
	DisplayTTY.$(OBJEXT)


APPLICATIONS = $(BINDIR)/SecondoTTY$(SMIUP)
ifneq ($(smitype),ora)
APPLOBJECTS += TestRunner.$(OBJEXT)
endif


CCFLAGS= $(DEFAULTCCFLAGS) 

ifeq ($(optimizer),"true")
APPLICATIONS += $(OPTDIR)/SecondoPL
APPLOBJECTS += SecondoPL.$(OBJEXT)
CCFLAGS += $(PLINCLUDEFLAGS)
endif


CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(APPLOBJECTS))

APPLICATIONS := $(addsuffix $(EXEEXT), $(APPLICATIONS))

.PHONY: all
all: $(CODE_DEPENDENCIES) $(APPLOBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS) TestRunner

.PHONY: TestRunner 
TestRunner: $(BINDIR)/TestRunner$(EXEEXT)

.PHONY: TTYCS 
TTYCS: $(BINDIR)/SecondoTTYCS$(EXEEXT)

.PHONY: TTY 
TTY: $(BINDIR)/SecondoTTY$(SMIUP)$(EXEEXT)

.PHONY: optimizer 
optimizer: $(OPTDIR)/SecondoPL$(EXEEXT)


# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) $(JNIINCLUDE)


#  Rules for linkage 

APP_OBJECTS :=  DisplayTTY.o SecondoInterfaceGeneral.o AlgebraList.o

$(BINDIR)/SecondoTTYBDB$(EXEEXT): $(SECLIBFILES) $(ALGLIBFILES)
$(BINDIR)/SecondoTTYBDB$(EXEEXT): SecondoTTY.o SecondoInterface.o $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG)


$(BINDIR)/SecondoTTYORA$(EXEEXT):  $(SECLIBSFILES) $(ALGLIBFILES)
$(BINDIR)/SecondoTTYORA$(EXEEXT): SecondoTTY.o SecondoInterface.o $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG)

$(BINDIR)/SecondoTTYCS$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoTTYCS$(EXEEXT): SecondoTTY.o DisplayTTY.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o 
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS) 

 
$(BINDIR)/TestRunner$(EXEEXT): $(SECLIBSFILES) $(ALGLIBFILES) 
$(BINDIR)/TestRunner$(EXEEXT): TestRunner.o SecondoInterface.o  $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG)


$(OPTDIR)/SecondoPL$(EXEEXT): $(SECLIBSFILES) $(ALGLIBFILES) 
$(OPTDIR)/SecondoPL$(EXEEXT): SecondoPL.o SecondoInterface.o $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS)  -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG) $(PLLDFLAGS)

include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) *.dep $(APPLOBJECTS) $(APPLICATIONS)

.PHONY: clean_cs
	$(RM) $(BINDIR)/SecondoTTYCS$(EXEEXT)
