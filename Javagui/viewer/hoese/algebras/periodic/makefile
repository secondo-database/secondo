# file: viewer/hoese/algebras/period/makefile
#
# last change: 03-2003; Thomas Behr

include ../../../../makefile.inc


JAVAFILES =$(shell find -maxdepth 1  -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY: all
all: $(CLASSFILES)

%.class: %.java	
	$(JAVAC) -classpath $(CLASSPATH) $<


.PHONY: clean
clean:
	rm -f *.class
