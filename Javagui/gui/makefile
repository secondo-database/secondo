# file: gui/makefile
#
# last changed : 05-2003  

include ../makefile.inc

JAVAFILES =$(shell find -maxdepth 1 -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY: all
all: IDManager $(CLASSFILES)


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<


.PHONY: IDManager
IDManager:
	(cd idmanager; make all);


.PHONY: clean
clean:
	(cd idmanager; make clean);
	rm -f *.class


