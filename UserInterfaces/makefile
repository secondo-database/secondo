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
#LIBXML2 := /opt/local/lib/libxml2.dylib
LIBXML2 := 

SECONDOPLMODE1 := SecondoPLMode

APPLOBJECTS := \
	SecondoTTY.$(OBJEXT) \
	SecondoTTYCS.$(OBJEXT) \
	DisplayTTY.$(OBJEXT) \
	MainTTY.$(OBJEXT) \
	cmsg.$(OBJEXT) \
	MainTTYCS.$(OBJEXT) \
  getCommand.$(OBJEXT) \
  SQLLanguage.$(OBJEXT)

APPLICATIONS := $(BINDIR)/Secondo$(SMIUP) \
		$(BINDIR)/SecondoCS \

# some optional objects and applications
ifneq ($(smitype),ora)
APPLOBJECTS += TestRunner.$(OBJEXT)
endif

# Secondo is always build using the readline library
# the environment variable "readline" is no longer used
CCFLAGS += -DHAVE_LIBREADLINE

ifneq ($(optimizer),"true")
RL_LD_FLAG = -lreadline -lncurses
endif

# Some optional switches used for providing a prolog
# predicate which maximizes the entropy. For details
# refer to "SecondoPL.cpp" and the subdirectory "Optimizer/Entropy"
ifdef ENTROPY 

MK_ENTROPY := entropy
ENTROPY_OBJ := ../Optimizer/Entropy/Iterative_scaling.$(OBJEXT)
CCFLAGS += -DSECONDO_USE_ENTROPY
endif

ifeq ($(optimizer),"true")
PLOBJ := SecondoPL.$(OBJEXT) $(SECONDOPLMODE1).$(OBJEXT)
PL_OBJECTS := $(PLOBJ) $(ENTROPY_OBJ)
APPLOBJECTS += $(PLOBJ)
CCFLAGS += $(PLINCLUDEFLAGS) -DSECONDO_PL
endif

BDB_OBJECTS :=   ../ClientServer/SecondoServer.$(OBJEXT)
		 
ALG_LIST    :=  SecondoInterfaceTTY.$(OBJEXT) SecondoInterfaceCS.$(OBJEXT)  AlgebraList.$(OBJEXT) SecondoInterfaceGeneral.$(OBJEXT)

ifeq ($(optimizer),"true")
ALG_LIST += SecondoPrologInterface.$(OBJEXT)
endif

COMMON_OBJECTS := cmsg.$(OBJEXT) SecondoInterfaceGeneral.$(OBJEXT) DisplayTTY.$(OBJEXT)

TTY_OBJECTS :=   MainTTY.$(OBJEXT) SecondoInterfaceTTY.$(OBJEXT) SecondoTTY.$(OBJEXT)  TestRunner.$(OBJEXT)

FLOB := $(SECONDO_BUILD_DIR)/Tools/Flob/FlobManager.$(OBJEXT)

CS_OBJECTS :=   MainTTYCS.$(OBJEXT)  SecondoInterfaceCS.$(OBJEXT) SecondoTTYCS.$(OBJEXT) TestRunnerCS.$(OBJEXT)

APP_LIB := ../lib/libappCommon.$(LIBEXT)

APPLICATIONS := $(addsuffix $(EXEEXT), $(APPLICATIONS))

#
# TARGET SECTION
#

.PHONY: all
all: $(CODE_DEPENDENCIES) $(MK_ENTROPY) $(APPLOBJECTS) 
	@chmod u+x ../bin/SecondoTTYBDB
	@chmod u+x ../bin/TestRunner
	@chmod u+x ../bin/TestRunnerCS
	@chmod u+x ../bin/SecondoTTYCS

.PHONY: buildapp
buildapp: $(APPLICATIONS)

.PHONY: secondobdb
secondobdb: $(BINDIR)/Secondo$(SMIUP)

.PHONY: TTYDemo
TTYDemo: ../$(SECONDO_DEMO)/SecondoTTY$(SMIUP)$(EXEEXT)

.PHONY: entropy
entropy:
	$(MAKE) -C ../Optimizer/Entropy

.PHONY: clean
clean:
	$(RM) *.dep $(APPLOBJECTS) $(APPLICATIONS) $(ENTROPY_OBJ) $(DEP_FILES) $(CS_OBJECTS) $(CS_DEP_FILES)
	$(MAKE) -C InteractiveQueryEditor clean

#
# RULES SECTION
#
# create dependencies from include directives


# The client/server variants are built from the same sources as the standalone
# ones, with different flags, so they need rules of their own -- and, like every
# other object, dependency files, which they went without for a long time. An
# object that does not depend on the headers it includes is not rebuilt when one
# of them changes, and the stale copy then wins silently at link time: inline
# member functions are emitted as weak symbols in every translation unit that
# uses them, so one old copy of (say) CMsg::info is enough to give the whole
# binary a version of it that no longer matches the class. That is not a link
# error, it is a crash somewhere else entirely.
CS_DEP_FILES := MainTTYCS.dep SecondoTTYCS.dep TestRunnerCS.dep

MainTTYCS.$(OBJEXT): MainTTY.$(OBJEXT)
MainTTYCS.$(OBJEXT): MainTTY.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) -DSEC_TTYCS  $(CPPSTDOPTION) \
	    -MMD -MP -MF MainTTYCS.dep

SecondoPL.$(OBJEXT): $(ENTROPY_OBJ)

# Overrides the generic rule in makefile.env to add $(JNIINCLUDE), so it has to
# repeat the dependency generation that rule does -- without it nothing in this
# directory was rebuilt when a header changed.
%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS) $(JNIINCLUDE) $(CPPSTDOPTION) \
	    -MMD -MP -MF $*.dep

SecondoTTYCS.$(OBJEXT): SecondoTTY.cpp
	$(CC) -c -DSECONDO_CLIENT_SERVER -o $@ $< $(CCFLAGS) $(CPPSTDOPTION) \
	    -MMD -MP -MF SecondoTTYCS.dep

TestRunnerCS.$(OBJEXT): TestRunner.cpp
	$(CC) -c -DSECONDO_CLIENT_SERVER -o $@ $< $(CCFLAGS)  $(CPPSTDOPTION) \
	    -MMD -MP -MF TestRunnerCS.dep

# Read here rather than through makefile.env's DEP_FILES: that list is derived
# from the *.cpp in this directory, and these objects are named after sources
# that do not exist.
ifneq ($(MAKECMDGOALS),clean)
-include $(CS_DEP_FILES)
endif

# Rules for linkage
#
# Note: Currently, the SecondoInterface depends on a static function of the relation
# algebra which returns tuple statistics. We should change this in the future.

$(APP_LIB): $(COMMON_OBJECTS) $(BDB_OBJECTS) 	
	$(create-lib)

# SecondoBDB, the stand alone application.
$(BINDIR)/Secondo$(SMIUP)$(EXEEXT):  $(APP_LIB) $(ALG_LIST) $(FLOB) $(PL_OBJECTS) $(TTY_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) $(LIBXML2) -o $@ $(filter %.o, $^) -lappCommon -lsdbutils $(LD_LINK_LIBS_ALG) $(RL_LD_FLAG) $(PLLDFLAGS) $(COMMON_LD_FLAGS)

# The TTY-Clients, they use special implementations of SecondoInterface and don't
# need the algebra libs.
# 
# SecondoCS is a pure client/server TTY client. It does NOT embed the optimizer
# (no -pl / -pltty); the SQL dialect is sent to the server's optimizer instead.
# It always needs readline (for the interactive history); in optimizer builds
# RL_LD_FLAG is empty because readline used to come in via the (now removed)
# Prolog link flags, so link it explicitly here.
$(BINDIR)/SecondoCS$(EXEEXT): $(SECLIBFILES) $(FLOB)
$(BINDIR)/SecondoCS$(EXEEXT): $(CS_OBJECTS) $(APP_LIB) $(FLOB)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o, $^) -lappCommon -lsdbutils $(OPTLIB_LIBS)   $(LD_LINK_LIBS) $(RL_LD_FLAG) -lreadline -lncurses $(COMMON_LD_FLAGS)

