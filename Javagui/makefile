# filename Javagui.makefile
#
# last change 05-2003; Thomas Behr

include ./makefile.inc


.PHONY: help
help:
	@echo "make all   -> compile all gui-components and know viewers"
	@echo "make doc   -> call javadoc, run make all first"
	@echo "make clean -> remove all .class files and the documentation"
	@echo ""
	@echo " create and compile the Javainterface first !! "


.PHONY: doc # creates the documentation
doc:
	$(mkdir $(DOCPATH))
	$(JAVADOC) -d $(DOCPATH) -classpath ".;$(JAVAINTERFACEPATH)" $(GUIPACKAGE) $(VIEWERPACKAGES)

.PHONY: all #creates the gui and all know viewers
all:
	(cd viewer;make all)
	(cd gui;make all)
	(cd viewer;make viewers)

.PHONY: viewers # creates all know viewers
viewers:
	(cd viewer;make viewers)


.PHONY: clean # removes the documentation and all .class files
clean:
	rm -f  *.class
	(cd viewer; make clean)
	(cd gui; make clean)
	(rm -f ./doc/* -r)
	(rmdir doc)



