# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr
#
#             10-2003; M. Spiekermann, automatic creation of configuration files added.

include ./makefile.inc


.PHONY: help
help:
	@echo "make all      -> compile all gui-components and know viewers"
	@echo "make doc      -> call javadoc, run make all first"
	@echo "make clean    -> remove all .class files and the documentation"
	@echo "make mrproper -> clean + make mrproper Javainterface"


.PHONY: doc # creates the documentation
doc:
	$(shell mkdir $(DOCPATH))
	$(JAVADOC) -d $(DOCPATH) -classpath "$(CLASSPATH)" $(GUIPACKAGE) $(VIEWERPACKAGES)

.PHONY: all #creates the gui and all known viewers
all: makedirs config

.PHONY: makedirs
makedirs:
	(cd ../Javainterface;make all)
	(cd communication;make all)
	(cd extern;make all)
	(cd components;make all)
	(cd viewer;make all)
	(cd project;make all);
	(cd gui;make all)
	(cd viewer;make viewers)

.PHONY: config
config: GBS.cfg gui.cfg
	chmod ugo+x sgui
	
# Automatic creation of configuration files

INI_1 := $(shell ls gui.cfg)
gui.cfg: gui.cfg.example
ifdef INI_1
	@echo "Warning: Configuration file $< is newer than $@!"
else
	cp $< $@
endif
	
INI_2 := $(shell ls GBS.cfg)
GBS.cfg: GBS.cfg.sample
ifdef INI_2
	@echo "Warning: Configuration file $< is newer than $@!"
else
	cp $< $@
endif

.PHONY: clean # removes the documentation and all .class files
clean:
	rm -f  *.class
	(cd ../Javainterface; make clean)
	(cd extern; make clean)
	(cd viewer; make clean)
	(cd gui; make clean)
	(cd project; make clean)	
	(cd components;make clean)
	(cd communication;make clean)
	(rm -f ./doc/* -r)
	(rm -fr doc)

.PHONY: mrproper
mrproper: clean
	(cd ../Javainterface;make mrproper)


.PHONY: cleangui
cleangui:
	(cd gui;make clean)

