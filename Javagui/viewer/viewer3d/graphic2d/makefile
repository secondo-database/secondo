#file javagui/viewer/viewer3d/graphic2d/makefile
#
# last change: 05-2003 ; Th.Behr

include ../../../makefile.inc

JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY:all
all: $(CLASSFILES);

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<

.PHONY: clean
clean:
	rm -f *.class