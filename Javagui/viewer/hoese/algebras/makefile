# file: viewer/hoese/algebras/makefile
#
# last change: 05-2003; Thomas Behr

include ../../../makefile.inc


JAVAFILES =$(shell find -maxdepth 1  -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY: all
all: $(CLASSFILES)
	make -C periodic all

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<


.PHONY: clean
clean:
	rm -f *.class
	make -C periodic clean
