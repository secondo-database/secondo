#This file is part of SECONDO.
#
#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.
#
#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
########################################################################
#
# SECONDO makefile for utility modules
#
########################################################################

include ../makefile.env

APPLOBJECTS := \
	SecondoTTY.$(OBJEXT) \
	DisplayTTY.$(OBJEXT) \
	MainTTY.$(OBJEXT) \
	MainTTYCS.$(OBJEXT)

APPLICATIONS := $(BINDIR)/Secondo$(SMIUP) \
		$(BINDIR)/SecondoCS

# some optional objects and applications
ifneq ($(smitype),ora)
APPLOBJECTS += TestRunner.$(OBJEXT)
endif


ifeq ($(readline),true)
RL_COMPILE_FLAG = -DREADLINE
RL_LD_FLAG = -lreadline -lncurses
endif

# Some optional switches used for providing a prolog
# predicate which maximizes the entropy. For details
# refer to "SecondoPL.cpp" and the subdirectory "Optimizer/Entropy"
ifdef ENTROPY 

include ../Optimizer/Entropy/makefile.optlib

ENTROPY_OBJ := ../Optimizer/Entropy/entropy.$(OBJEXT) 
APPLOBJECTS += callEntropy
CCFLAGS += -DSECONDO_USE_ENTROPY
endif

ifeq ($(optimizer),"true")
APPLOBJECTS += SecondoPL.$(OBJEXT)
CCFLAGS += $(PLINCLUDEFLAGS)
PL_OBJECTS := SecondoPL.$(OBJEXT) $(ENTROPY_OBJ)
endif



CODE_DEPENDENCIES := $(patsubst %.$(OBJEXT), %.dep, $(APPLOBJECTS))
APPLICATIONS := $(addsuffix $(EXEEXT), $(APPLICATIONS))
#
# TARGET SECTION
#

.PHONY: all
all: $(CODE_DEPENDENCIES) $(APPLOBJECTS) 
	@chmod u+x ../bin/SecondoTTYBDB
	@chmod u+x ../bin/TestRunner
	@chmod u+x ../bin/TestRunnerCS
	@chmod u+x ../bin/SecondoTTYCS

.PHONY: buildapp
buildapp: $(APPLICATIONS)

.PHONY: TTYDemo
TTYDemo: ../$(SECONDO_DEMO)/SecondoTTY$(SMIUP)$(EXEEXT)

.PHONY: callEntropy
callEntropy:
	$(MAKE) -C ../Optimizer/Entropy allEntropy


.PHONY: clean
clean:
	$(RM) *.dep $(APPLOBJECTS) $(APPLICATIONS)

#
# RULES SECTION
#
# create dependencies from include directives

CCFLAGS += $(DEFAULTCCFLAGS)

MainTTYCS.cpp: MainTTY.cpp
	$(CP) $< $@ 

%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

MainTTYCS.$(OBJEXT): MainTTY.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) -DSEC_TTYCS 

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) $(JNIINCLUDE) $(RL_COMPILE_FLAG) 


#  Rules for linkage

APP_OBJECTS :=  SecondoTTY.o MainTTY.o TestRunner.o \
		$(PL_OBJECTS) \
		../ClientServer/SecondoServer.o \
		SecondoInterface.o \
		DisplayTTY.o \
		SecondoInterfaceGeneral.o \
		AlgebraList.o

$(BINDIR)/Secondo$(SMIUP)$(EXEEXT): $(SECLIBFILES) $(ALGLIBFILES) $(JNILIBFILES)
$(BINDIR)/Secondo$(SMIUP)$(EXEEXT): $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(OPTLIB_LIBS) $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG) $(PLLDFLAGS) 

../$(SECONDO_DEMO)/Secondo$(SMIUP)$(EXEEXT): $(SECLIBFILES) $(ALGLIBFILES) $(JNILIBFILES) 
../$(SECONDO_DEMO)/Secondo$(SMIUP)$(EXEEXT): $(APP_OBJECTS)
	$(LD) $(EXEFLAGS) -static $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG)
	strip $@

$(BINDIR)/SecondoCS$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoCS$(EXEEXT): SecondoTTY.o MainTTYCS.o TestRunner.o $(PL_OBJECTS) DisplayTTY.o SecondoInterfaceCS.o SecondoInterfaceGeneral.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) $(OPTLIB_LIBS) $(LD_LINK_LIBS) $(RL_LD_FLAG) $(PLLDFLAGS)

include $(CODE_DEPENDENCIES:.cpp=.dep)
