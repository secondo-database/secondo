#file javagui/viewer/viewer3d/makefile
#
# last change: 05-2003; Th.Behr
include ../../makefile.inc

JAVAFILES =$(shell find -maxdepth 1 -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))

.PHONY: all
all: $(CLASSFILES)
	(cd mathe;make all)
	(cd graphic2d;make all)
	(cd graphic3d; make all)
	(cd objects; make all)

%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<

.PHONY:clean
clean:
	(cd graphic2d;make clean)
	(cd graphic3d;make clean)
	(cd mathe; make clean)
	(cd objects; make clean)
	rm -f *.class