########################################################################
#
# SECONDO Makefile
#
########################################################################

include ./makefile.env

.PHONY: all
all: makedirs buildlibs buildalg buildapps java $(OPTIMIZER_SERVER) checkup 


.PHONY: javagui
javagui:
	$(MAKE) -C Javagui all

.PHONY: clientserver
clientserver: cs

.PHONY: cs
cs: makedirs buildlibs buildalg checkup
	$(MAKE) -C ClientServer
	$(MAKE) -C UserInterfaces TTYCS
	$(MAKE) -C ClientServer buildapp


.PHONY: makedirs
makedirs:
	$(MAKE) -C ClientServer
	$(MAKE) -C Tools
	$(MAKE) -C StorageManager
	$(MAKE) -C Algebras/Management
	$(MAKE) -C QueryProcessor
	$(MAKE) -C UserInterfaces


.PHONY: buildalg
buildalg:
	$(MAKE) -C Algebras buildlibs


.PHONY: buildlibs
buildlibs: 
	$(MAKE) -f ./makefile.libs


.PHONY: java
java:
	$(MAKE) -C Javagui all


.PHONY: optimizer
optimizer: optimizer2 optserver checkup

.PHONY: optimizer2
optimizer2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces optimizer


.PHONY: optserver
optserver:
	$(MAKE) -C Jpl all
	$(MAKE) -C OptServer all


.PHONY: TTY
TTY: TTY2 checkup

.PHONY: TTY2
TTY2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces TTY


.PHONY: TestRunner
TestRunner: TestRunner2 checkup
	
.PHONY: TestRunner2
TestRunner2: makedirs buildlibs buildalg
	$(MAKE) -C UserInterfaces TestRunner


.PHONY: buildapps
buildapps: 
	$(MAKE) -C UserInterfaces buildapp
	$(MAKE) -C ClientServer buildapp

.PHONY: tests
tests: makedirs buildlibs
	$(MAKE) -C Tests


# rules for deployment of source files
# default value for cvs tag 
ifndef tag
tag=HEAD
endif

.PHONY: dist
dist:	secondo.tgz

secondo.tgz:
	cvs export -r$(tag) secondo
	tar -czf  secondo.tgz secondo/*
	cp secondo.tgz $(NETDEV)
	rm -r secondo

.PHONY: clean
clean:
	$(MAKE) -C ClientServer clean
	$(MAKE) -C StorageManager clean
	$(MAKE) -C Tools clean
	$(MAKE) -C Algebras clean
	$(MAKE) -C QueryProcessor clean
	$(MAKE) -C UserInterfaces clean
	$(MAKE) -C Jpl clean
	$(MAKE) -C OptServer clean
	$(MAKE) -f ./makefile.libs clean

###
### Some special rules
###

.PHONY: checkup
checkup: config showjni

.PHONY: showjni
showjni:
	@echo $(JNITEXT)
	
.PHONY: config
config: bin/SecondoConfig.ini Optimizer/SecondoConfig.ini

BIN_INI := $(shell ls bin/SecondoConfig.ini)
bin/SecondoConfig.ini: bin/SecondoConfig.example
#	echo -$(BIN_INI)-
ifdef BIN_INI
	@echo "Warning: Configuration file $< is newer than $@!"
else
	cp $< $@
endif
	
OPT_INI := $(shell ls Optimizer/SecondoConfig.ini)
Optimizer/SecondoConfig.ini: bin/SecondoConfig.example
#	echo -$(OPT_INI)-
ifdef OPT_INI
	@echo "Warning: Configuration file $< is newer than $@!"
else
	cp $< $@
endif



.PHONY: help
help:
	@echo "*** Usage of the SECONDO makefile:"
	@echo "***"
	@echo "*** make [shared=yes] [target]"
	@echo "***"
	@echo "*** The optional definition shared=yes specifies"
	@echo "*** that shared libraries are built. Make sure that"
	@echo "*** the ./lib directory is in your LD_LIBRARY_PATH"
	@echo "*** variable (on windows the PATH variable). Consult"
	@echo "*** the readme file for deatiled information."
