# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr
#
#             10-2003; M. Spiekermann, automatic creation of configuration files added.
# 2004-01 stefan.wich@fernuni-hagen.de: doc sj.lang added

include ./makefile.inc

.PHONY: all #creates the gui and all known viewers
all: makedirs
	@chmod ugo+x sgui


.PHONY: help
help:
	@echo "make all      -> compile all gui-components and know viewers"
	@echo "make doc      -> call javadoc, run make all first"
	@echo "make clean    -> remove all .class files and the documentation"
	@echo "make mrproper -> cleans all created files"


.PHONY: doc # creates the documentation
doc: removedoc
	$(JAVADOC) -d $(DOCPATH) -classpath $(CLASSPATH) $(shell find -name "*.class"  | sed "s#/[^/]*\.class##g"  | sed "s#/[^/]*\.class##g" | sed "s#^\./##g"  | sed "s#/#.#g" | java tools.DupRemover)


.PHONY: demo
demo: Javagui.jar
	@rm -rf Demo
	@mkdir Demo
	@mv Javagui.jar Demo
	@cp $(CUPCLASSESRUNTIME) $(MP3CLASSESPATH) Demo
	@cp -r res Demo
	@cp gui.cfg Demo
	@cp GBS.cfg Demo
	@echo -e "java -Xmx80M -cp "$(CUPRUNTIME)":"$(MP3)":Javagui.jar gui.MainWindow\n" >Demo/start
	@chmod +x Demo/start
	@echo -e "java -Xmx80M -cp "$(CUPRUNTIME)";"$(MP3)";Javagui.jar gui.MainWindow\r\n" >Demo/start.bat

Javagui.jar:
	@make -s all
	@(jar -cfM Javagui.jar $(shell find -type d | grep -v "CVS" | sed -e "s#\./##g" -e "s#.*#&/*.class#g")) 2>/dev/null ; echo ""

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
	make -C viewer clean
	(rm -f ./doc/* -r)
	(rm -fr doc)
	rm -rf Demo


.PHONY: cleangui
cleangui:
	(cd gui;make clean)

