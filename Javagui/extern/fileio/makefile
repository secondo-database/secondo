# filename extern/fileio/makefile
#
# last change: 09-2003; Th. Behr

include ../../makefile.inc
JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))



.PHONY:all
all:  $(CLASSFILES)



%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<

.PHONY: clean
clean:
	rm -f *.class
