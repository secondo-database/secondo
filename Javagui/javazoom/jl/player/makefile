
include ../../../makefile.inc


JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))
CLASSPATH = $(GUIPATH)

.PHONY: all
all: $(CLASSFILES)
	make -C advanced

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<


.PHONY: clean
clean:
	rm -f *.class
	make -C advanced clean
