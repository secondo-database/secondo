# makefile for archiving msys configuration files
# and copying them to the Secondo-CDROM directory

SETVAR_SCRIPT_DIR=../..

.PHONY: all
all: $(NETDEV)/etc.tgz $(NETDEV)/install.bash


.PHONY: install
install: /etc/setvar.bash /etc/profile

/etc/setvar.bash: ../../setvar.bash
	cp --backup $< $@

/etc/profile: profile
	cp --backup $< $@

$(NETDEV)/install.bash: install.bash
	cp $< $@

$(NETDEV)/etc.tgz: fstab profile $(SETVAR_SCRIPT_DIR)/setvar.bash 
	tar -cvzf $@ fstab profile -C $(SETVAR_SCRIPT_DIR) setvar.bash
