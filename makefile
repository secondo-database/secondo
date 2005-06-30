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
# SECONDO Makefile
#
#########################################################################

include ./makefile.env

ifneq ($(PWD),$(BUILDDIR))
  $(error SECONDO_BUID_DIR has another value than the current directory. \
    Please check your environment setup! Go to the root of your SECONDO \
    tree and enter the command setvar )
endif

# Configuration files which will be created as a copy of example files
# The corresponding .example files are stored in the CVS

CONFIG_FILES = bin/SecondoConfig.ini \
	Optimizer/SecondoConfig.ini \
	bin/JNI.ini \
	Javagui/gui.cfg \
	Javagui/GBS.cfg


ALL_TARGETS = makedirs \
	buildlibs \
	buildalg \
	buildapps \
	$(OPTIMIZER_SERVER) \
	java2 \
	update-config

.PHONY: all
all: $(ALL_TARGETS) 



.PHONY: javagui
javagui: java2


.PHONY: clientserver
clientserver: cs

.PHONY: cs
cs: makedirs buildlibs buildalg update-config
	$(MAKE) -C ClientServer
	$(MAKE) -C UserInterfaces TTYCS
	$(MAKE) -C ClientServer buildapp


.PHONY: makedirs
makedirs:
	@echo -e "\n *** Building objects for Secondo libraries *** \n"
	$(MAKE) -C ClientServer
	$(MAKE) -C Tools
	$(MAKE) -C StorageManager
	$(MAKE) -C Algebras/Management
	$(MAKE) -C QueryProcessor
	$(MAKE) -C UserInterfaces


.PHONY: libs
libs: makedirs buildlibs

.PHONY: buildalg
buildalg:
	$(MAKE) -C Algebras buildlibs


.PHONY: buildlibs
buildlibs:
	@echo -e "\n *** Creating library files *** \n"
	$(MAKE) -f ./makefile.libs


.PHONY: java
java: java2 update-config

.PHONY: java2
java2:
	@echo -e "\n *** Compiling the java based GUI *** \n"
	$(MAKE) -C Javagui all


.PHONY: optimizer
optimizer: optimizer2 optserver update-config

.PHONY: optimizer2
optimizer2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces optimizer


.PHONY: optserver
optserver:
ifeq ($(optimizer),"true")
	@echo -e "\n *** Building JPL and the optimizer server *** \n"
	$(MAKE) -C Jpl all
	$(MAKE) -C OptServer all
	@chmod ugo+x Optimizer/StartOptServer
endif


.PHONY: TTY
TTY: TTY2 update-config

.PHONY: TTY2
TTY2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces TTY


.PHONY: TestRunner
TestRunner: TestRunner2 update-config
	
.PHONY: TestRunner2
TestRunner2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces TestRunner


.PHONY: buildapps
buildapps: 
	@echo -e "\n *** Linking Applications *** \n"
	$(MAKE) -C UserInterfaces buildapp
	$(MAKE) -C ClientServer buildapp

.PHONY: tests
tests: makedirs buildlibs
	$(MAKE) -C Tests



.PHONY: clean
clean:
	$(MAKE) -C ClientServer clean
	$(MAKE) -C Tools clean
	$(MAKE) -C StorageManager clean
	$(MAKE) -C Algebras/Management clean
	$(MAKE) -C Algebras clean
	$(MAKE) -C QueryProcessor clean
	$(MAKE) -C UserInterfaces clean
	$(MAKE) -C Jpl clean
	$(MAKE) -C OptServer clean
	$(MAKE) -C Optimizer clean
	$(MAKE) -f ./makefile.libs clean

.PHONY: realclean
realclean: clean
	$(MAKE) -C Javagui clean
	rm $(CONFIG_FILES) 


.PHONY: runtests 
runtests:
	CM-Scripts/run-tests.sh


.PHONY: cvstest
cvstest:
	CM-Scripts/cvs-make.sh -r$(HOME)


include ./makefile.cm

######################################################
#
# Automatic creation of configuration files.
# This mechanism avoids that someone checks in his
# local configuration files into CVS
#
######################################################

.PHONY: update-config 
update-config: config showjni

.PHONY: showjni
showjni:
	@echo -e $(JNITEXT)

.PHONY: config
config: $(CONFIG_FILES) 

bin/SecondoConfig.ini: bin/SecondoConfig.example
	$(cp-config-file)
	
Optimizer/SecondoConfig.ini: bin/SecondoConfig.example
	$(cp-config-file)

bin/JNI.ini: bin/JNI.ini.sample
	$(cp-config-file)

$(BUILDDIR)/makefile.algebras: $(BUILDDIR)/makefile.algebras.sample
	$(cp-config-file)

Javagui/gui.cfg: Javagui/gui.cfg.example
	$(cp-config-file)
	
Javagui/GBS.cfg: Javagui/GBS.cfg.sample
	$(cp-config-file)

.PHONY: help
help:
	@echo "*** Usage of the SECONDO makefile:"
	@echo "*** "
	@echo "*** make [alg=auto] [TTY||optimizer|java|clean|TestRunner]"
	@echo "*** "
	@echo "*** The optional parameters or targets are explained below:"
	@echo "*** -------------------------------------------------------"
	@echo "*** alg=auto: The File Management/LagebraList.i will be generated automatically."
	@echo "*** "
	@echo "*** TTY       : Compile only a single user Version of Secondo."
	@echo "*** optimizer : Create only SecondoPL, SecondoPLCS and OptServer." 
	@echo "*** java      : The Java-Gui of Secondo will be created."
	@echo "*** clean     : Delete all created objects."
	@echo "*** TestRunner: Compile only the TestRunner, a tool to automate tests."
	@echo "*** "
	@echo "*** without any options every of the above applications will be compiled."
