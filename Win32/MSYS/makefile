# makefile for archiving msys configuration files
# and copying them to the Secondo-CDROM directory

include ../../makefile.env

SCRIPT_DIR=../../CM-Scripts
	
# Configuration files for MSYS
.PHONY: config
config: /etc/fstab $(HOME)/.profile 

$(HOME)/.profile: $(SCRIPT_DIR)/.profile
	cp --backup $< $@

/etc/fstab: fstab
	cp --backup $< $@

