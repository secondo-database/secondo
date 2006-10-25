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


# check if java compiler is working
include ./Javagui/makefile.inc
compileJava := "$(shell $(JAVAC) -help > /dev/null 2>&1; if [ $$? != 0 ]; then echo "false"; else echo "true"; fi)"

define javac-msg
  @echo ""
  @echo "Warning: The command \"$(JAVAC) -help\" returned an error!"
  @echo "         Please check your Java 2 SDK configuration an run again."
endef

# check if J2SDK_ROOT is present
j2sdkIsPresent := "$(shell if [ "$$J2SDK_ROOT" != "" ] && [ -e "$$J2SDK_ROOT" ]; then echo "true"; else echo "false"; fi)"

define j2sdk-msg
  @echo ""
  @echo "Warning: The variable J2SDK_ROOT is not set or points to an non existing directory."
  @echo "         Please check the settings in file ~/.secondo.$(platform).rc and run again."
endef




# Configuration files which will be created as a copy of example files
# The corresponding .example files are stored in the CVS

CONFIG_FILES = bin/SecondoConfig.ini \
	bin/JNI.ini \
	Javagui/gui.cfg \
	Javagui/GBS.cfg


ALL_TARGETS = makedirs \
	buildlibs \
	buildalg \
	buildapps \
	$(OPTIMIZER_SERVER) \
	java2 \
	tests \
	update-config

.PHONY: all
all: jnicheck $(ALL_TARGETS) 

.PHONY: TTY 
TTY: kernel


.PHONY: kernel
kernel: makedirs buildlibs buildalg buildapps

.PHONY: linkonly
linkonly: buildapps

.PHONY: jnicheck
jnicheck:
ifeq ($(USE_JNI),"true")
ifeq ($(j2sdkIsPresent),"false")
	@echo -e "\nJNI based algebras can not be compiled!" 
	$(j2sdk-msg)
	@exit 1
endif 
endif


.PHONY: show-vars
show-vars:
	@echo "USE_JNI = <$(USE_JNI)>"
	@echo "compileJava = <$(compileJava)>"
	@echo "j2sdkIsPresent = <$(j2sdkIsPresent)>"
	@echo "JAVAC = <$(JAVAC)>"

.PHONY: javagui
javagui: java2


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
ifeq ($(compileJava),"true")
	@echo -e "\n *** Compiling the java based GUI *** \n"
	$(MAKE) -C Javagui all
else
	@echo -e "\nThe java based GUI was not compiled!"
	$(javac-msg)
endif


.PHONY: optimizer
optimizer: optimizer2 optserver update-config

.PHONY: optimizer2
optimizer2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces optimizer


.PHONY: optserver
optserver:
ifeq ($(compileJava),"true")
ifeq ($(optimizer),"true")
ifeq ($(j2sdkIsPresent),"true")
	@echo -e "\n *** Building JPL and the optimizer server *** \n"
	$(MAKE) -C Jpl all
	$(MAKE) -C OptServer all
	@chmod ugo+x Optimizer/StartOptServer
endif
endif
endif

.PHONY: optsrv-msg
optsrv-msg:
ifeq ($(compileJava),"true")
ifeq ($(j2sdkIsPresent),"false")
	@echo -e "\nJPL and the optimizer server were not compiled!"
	$(j2sdk-msg)
endif
else
	@echo -e "\nJPL and the optimizer server were not compiled!"
	$(javac-msg)
endif




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
	rm -f lib/*.a
	rm -f lib/*.o

.PHONY: realclean
realclean: clean
	$(MAKE) -C Javagui clean
	$(MAKE) -C Tests clean
	$(MAKE) -C Algebras realclean
	rm -f $(CONFIG_FILES) makefile.algebras 
	rm -f Documents/.Secondo-News.txt 


.PHONY: runtests
runtests: ttytest

.PHONY: ttytest 
ttytest:
	cd CM-Scripts; ./run-tests.sh -tty /tmp/runtests-$(USER) 900

.PHONY: cstest 
cstest:
	cd CM-Scripts; ./run-tests.sh -cs

.PHONY: cvstest
cvstest:
	cd CM-Scripts; cvs-make.sh -r$(HOME)


include ./makefile.cm

######################################################
#
# Automatic creation of configuration files.
# This mechanism avoids that someone checks in his
# local configuration files into CVS
#
######################################################

.PHONY: update-config 
update-config: config optsrv-msg showjni Documents/.Secondo-News.txt

.PHONY: showjni
showjni:
	@echo -e $(JNITEXT)

.PHONY: config
config: $(CONFIG_FILES) 
	@chmod u+x bin/rmlogs

# Alert for new information in Secondo-News
Documents/.Secondo-News.txt : Documents/Secondo-News.txt
	@touch $@ 
	@echo -e "\n *** New information in the file $< *** \n"
	@firstSectionEnd=$$(grep -ine "=====" $< | sed -ne '2s/:.*//gp'); \
	head -n $$[$$firstSectionEnd] $< | sed -ne '10~1p'
	@echo -e "\n *** file truncated *** \n"

bin/SecondoConfig.ini: bin/SecondoConfig.example
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
	@echo "*** make [TTY||optimizer|java|clean|realclean|TestRunner]"
	@echo "***      [clean|realclean|runtests]"
	@echo "*** "
	@echo "*** The optional parameters or targets are explained below:"
	@echo "*** ---------------------------------------------------------------------"
	@echo "*** TTY       : Compile only a single user version of Secondo."
	@echo "*** optimizer : Create only SecondoPL, SecondoPLCS and OptServer." 
	@echo "*** java      : The Java-GUI will be created."
	@echo "*** TestRunner: Compile only the TestRunner, a tool to automate tests."
	@echo "*** "
	@echo "*** without any Options every of the above applications will be compiled."
	@echo "*** "
	@echo "*** runtests  : Run some automatic Tests. Useful to check if changes have side effects."
	@echo "*** "
	@echo "*** clean     : Delete all created objects, libraries and applications."
	@echo "*** realclean : Delete all like clean and the Java-GUI."
