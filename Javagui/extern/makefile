# filename extern/makefile
#
# last change: 08-2003; Th. Behr

include ../makefile.inc
JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))



.PHONY:all
all: subdirs $(CLASSFILES)


.PHONY:subdirs
subdirs:
	(make -C numericreader all)	
	(make -C shapereader all)
	(make -C dbfreader all)
	(make -C shapedbf  all)
	(make -C fileio all)
	(make -C binarylist all)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<

.PHONY: clean
clean:
	(make -C numericreader clean)
	(make -C shapereader clean)
	(make -C dbfreader clean)
	(make -C shapedbf clean)
	(make -C fileio clean)
	(make -C binarylist clean)
	rm -f *.class
