# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr

include ./makefile.inc

.PHONY: all #creates the gui and all know viewers
all:
	(cd ../Javainterface;make all)
	(cd extern;make all)
	(cd components;make all)
	(cd viewer;make all)
	(cd project;make all);
	(cd gui;make all)
	(cd viewer;make viewers)


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
GBS.cfg: GBS.cfg.sample
	@test -e $@  && echo "Warning: Configuration file $< is newer than $@!" \
	|| test ! -e $@ && cp $< $@
	
gui.cfg: gui.cfg.example
	@test -e $@  && echo "Warning: Configuration file $< is newer than $@!" \
	|| test ! -e $@ && cp $< $@


.PHONY: clean # removes the documentation and all .class files
clean:
	rm -f  *.class
	(cd ../Javainterface; make clean)
	(cd extern; make clean)
	(cd viewer; make clean)
	(cd gui; make clean)
	(cd project; make clean)	
	(cd components;make clean)
	(rm -f ./doc/* -r)
	(rm -fr doc)

.PHONY: mrproper
mrproper: clean
	(cd ../Javainterface;make mrproper)


.PHONY: cleangui
cleangui:
	(cd gui;make clean)

