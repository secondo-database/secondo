# makefile for archiving msys configuration files
# and copying them to the Secondo-CDROM directory

include ../../makefile.env

SCRIPT_DIR=../../CM-Scripts
	
# Configuration files for MSYS
.PHONY: config
config: config /etc/profile /etc/fstab

/etc/profile: profile
	cp --backup $< $@

/etc/fstab: fstab
	cp --backup $< $@

