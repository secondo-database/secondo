# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr
#
#             10-2003; M. Spiekermann, automatic creation of configuration files added.

include ./makefile.inc

.PHONY: all #creates the gui and all known viewers
all: makedirs
	chmod ugo+x sgui


.PHONY: help
help:
	@echo "make all      -> compile all gui-components and know viewers"
	@echo "make doc      -> call javadoc, run make all first"
	@echo "make clean    -> remove all .class files and the documentation"
	@echo "make mrproper -> cleans all created files"


.PHONY: doc # creates the documentation
doc: removedoc
	$(JAVADOC) -d $(DOCPATH) -classpath $(CLASSPATH) $(shell find -name "*.class"  | sed "s#/[^/]*\.class##g"  | sed "s#/[^/]*\.class##g" | sed "s#^\./##g"  | sed "s#/#.#g" | java tools.DupRemover)




.PHONY: removedoc # removes the documentation
removedoc:
	(rm -fr $(DOCPATH))



.PHONY: makedirs
makedirs:
	make -C sj all
	make -C communication all
	make -C extern all
	make -C components all
	make -C viewer all
	make -C project all
	make -C gui all
	make -C javazoom all
	make -C viewer viewers
	make -C tools all

.PHONY: clean # removes the documentation and all .class files
clean:
	make -C sj clean
	make -C communication clean
	make -C components clean
	make -C extern clean
	make -C gui clean
	make -C project clean
	make -C tools clean
	make -C javazoom clean
	make -C viewer clean
	make -C java_cup clean
	(rm -f ./doc/* -r)
	(rm -fr doc)

.PHONY: mrproper
mrproper: clean
	(make -C sj mrproper)


.PHONY: cleangui
cleangui:
	(cd gui;make clean)

