# file: viewer/hoese/makefile
#
# last change: 05-2003 ; Thomas Behr

include ../../makefile.inc


JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))


.PHONY: all
all: algebras $(CLASSFILES)

.PHONY: algebras
algebras:
	(cd algebras;make all)

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<	


.PHONY: clean
clean:
	(cd algebras; make clean)
	rm -f *.class