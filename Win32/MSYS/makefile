# makefile for archiving msys configuration files
# and copying them to the Secondo-CDROM directory

include ../../makefile.env

SETVAR_SCRIPT_DIR=../..

.PHONY: all
all: $(NETDEV)/etc.tgz $(NETDEV)/install.bash


$(NETDEV)/install.bash: install.bash
	cp $< $@

$(NETDEV)/etc.tgz: fstab profile $(SETVAR_SCRIPT_DIR)/setvar.bash 
	tar -cvzf $@ $< -C $(SETVAR_SCRIPT_DIR) setvar.bash
