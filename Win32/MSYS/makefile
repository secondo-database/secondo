# makefile for archiving msys configuration files
# and copying them to the Secondo-CDROM directory

include ../../makefile.env

ROOT_DIR=../..
	
# Configuration files for MSYS
.PHONY: config-msys
config-msys: config-all /etc/profile /etc/fstab

# Configuration files shared by Linux and Windows
.PHONY: config-all 
config-all: $(SECONDO_SDK)/bin/setvar.bash $(SECONDO_SDK)/bin/catvar $(HOME)/secondo-bashrc
	
$(SECONDO_SDK)/bin/setvar.bash: $(ROOT_DIR)/setvar.bash
	cp --backup $< $@

$(SECONDO_SDK)/bin/catvar: $(ROOT_DIR)/catvar.sh
	cp --backup $< $@

$(HOME)/secondo-bashrc: $(ROOT_DIR)/secondo-bashrc
	cp --backup $< $@

/etc/profile: profile
	cp --backup $< $@

/etc/fstab: fstab
	cp --backup $< $@

# Copying files to SECONDO_CD
.PHONY: dist
dist: $(NETDEV)/fstab $(NETDEV)/profile $(NETDEV)/install-msys.bash

$(NETDEV)/install-msys.bash: $(ROOT_DIR)/install-msys.bash
	cp $< $@

$(NETDEV)/fstab: fstab
	cp $< $@

$(NETDEV)/profile: profile
	cp $< $@
