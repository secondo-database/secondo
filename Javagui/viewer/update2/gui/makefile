# file: viewer/update2/gui/makefile
#
# last change: 01-2014; Thomas Behr
#

include ../../../makefile.inc


JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY: all
all: $(CLASSFILES)

%.class: %.java
	$(JAVAC) -Xlint:unchecked -classpath $(CLASSPATH) $<


.PHONY: clean
clean:
	rm -f *.class
