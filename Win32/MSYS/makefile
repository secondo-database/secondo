# makefile for archiving msys configuration files
# and copying them to the Secondo-CDROM directory

include ../../makefile.env

SCRIPT_DIR=../../CM-Scripts
	
# Configuration files for MSYS
.PHONY: config-msys
config-msys: config-all /etc/profile /etc/fstab

# Configuration files shared by Linux and Windows
.PHONY: config-all 
config-all: $(SECONDO_SDK)/bin/setvar.bash $(SECONDO_SDK)/bin/catvar $(HOME)/.secondorc
	
$(SECONDO_SDK)/bin/setvar.bash: $(SCRIPT_DIR)/setvar.bash
	cp --backup $< $@

$(SECONDO_SDK)/bin/catvar: $(SCRIPT_DIR)/catvar.sh
	cp --backup $< $@

$(HOME)/.secondorc: $(SCRIPT_DIR)/.secondorc
	cp --backup $< $@

$(HOME)/.bashrc: $(SCRIPT_DIR)/.bashrc-sample
	cp --backup $< $@

/etc/profile: profile
	cp --backup $< $@

/etc/fstab: fstab
	cp --backup $< $@

# Copying files to SECONDO_CD
.PHONY: dist
dist: $(NETDEV)/install-msys.bash

$(NETDEV)/install-msys.bash: $(SCRIPT_DIR)/install-msys.bash
	cp $< $@
