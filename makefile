########################################################################
#
# SECONDO Makefile
#
#########################################################################

include ./makefile.env

# Configuration files which will be created as a copy of example files
# The corresponding .example files are stored in the CVS

CONFIG_FILES = bin/SecondoConfig.ini \
	Optimizer/SecondoConfig.ini \
	bin/JNI.ini \
	Javagui/gui.cfg \
	Javagui/GBS.cfg


.PHONY: all
all: makedirs buildlibs buildalg buildapps $(OPTIMIZER_SERVER) java2 checkup 

# Rules for copying configuration scripts
SCRIPT_DIR = ./CM-Scripts
SCRIPT_FILES = $(SECONDO_SDK)/bin/setvar.bash $(SECONDO_SDK)/bin/catvar.sh $(HOME)/.secondorc $(HOME)/.bashrc-sample

.PHONY: update-environment 
update-environment: $(SCRIPT_FILES)
ifeq ($(platform),win32)
	$(MAKE) -C Win32/MSYS config
endif
	
$(SECONDO_SDK)/bin/setvar.bash: $(SCRIPT_DIR)/setvar.bash
	cp --backup $< $@

$(SECONDO_SDK)/bin/catvar.sh: $(SCRIPT_DIR)/catvar.sh
	cp --backup $< $@

$(HOME)/.secondorc: $(SCRIPT_DIR)/.secondorc
	cp --backup $< $@

$(HOME)/.bashrc-sample: $(SCRIPT_DIR)/.bashrc-sample
	cp --backup $< $@



.PHONY: javagui
javagui: java2


.PHONY: clientserver
clientserver: cs

.PHONY: cs
cs: makedirs buildlibs buildalg checkup
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
java: java2 checkup

.PHONY: java2
java2:
	@echo -e "\n *** Compiling the java based GUI *** \n"
	$(MAKE) -C Javagui all


.PHONY: optimizer
optimizer: optimizer2 optserver checkup

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
	@echo -e "\n *** Linking Applications *** \n"
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


DIST_FILES := $(net)/windows/secondo-win32.tgz \
	      $(net)/windows/install-msys.bash \
	      $(net)/linux/secondo-linux.tgz \
	      $(net)/linux/install-linux.bash \
	      $(net)/Linux-Installation-Guide.pdf \
	      $(net)/Windows-Installation-Guide.pdf
 
.PHONY: dist
dist: tag-version $(DIST_FILES)

.PHONY: demo
demo: TTY2 checkup 
	mkdir $(SECONDO_DEMO)
	make -C UserInterfaces TTYDemo
	cp bin/SecondoConfig.ini bin/opt bin/testqueries $(SECONDO_DEMO)
	cp Documents/SecondoManual.pdf $(SECONDO_DEMO)
	tar -cvzf secondo-demo-$(platform).tar.gz $(SECONDO_DEMO)/*
	rm -rf $(SECONDO_DEMO)

tag := $(shell echo -n "Secondo-CD-"; date "+%y%m%d")
.PHONY: tag-version
tag-version:
	cvs tag $(tag) 

$(net)/windows/secondo-win32.tgz: secondo-win32.tgz
	cp $< $@ 

$(net)/linux/secondo-linux.tgz: secondo-linux.tgz
	cp $< $@ 

secondo-win32.tgz secondo-linux.tgz:
	cvs export -d secondo -r$(tag) secondo
ifeq ($(platform),win32)
	$(error Target dist not supported on windows!)
else	
	tar -czf secondo-linux.tgz secondo/*
	find secondo ! \( -type d -or -path "*javazoom*" -or -name "*.zip" -or -name "*.jar" -or -name  "*.bmp" -or -name "*.gif"  -or -name "*.sxd" -or -name "*.sda" -or -name "*.fig" -or -name "*.canvas" -or -name "*.fm" -or -name "*.book" -or -name "*.pdf" \) -exec recode lat1..cp1252 {} \;
	tar -czf secondo-win32.tgz secondo/*
endif
	rm -r secondo

$(net)/windows/install-msys.bash: $(SCRIPT_DIR)/install-msys.bash
	cp $< $@
	recode lat1..cp1252 $@

$(net)/linux/install-linux.bash: $(SCRIPT_DIR)/install-linux.bash
	cp $< $@

$(net)/%.pdf: Documents/Installation/%.ps
	ps2pdf $< $@

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
	$(MAKE) -f ./makefile.libs clean

.PHONY: realclean
realclean: clean
	$(MAKE) -C Javagui clean
	rm $(CONFIG_FILES) 

###
### Some special rules
### Automatic creation of configuration files

.PHONY: checkup
checkup: config showjni

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
