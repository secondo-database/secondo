# filename fuzzyobjects/makefile
#
# last change: 05-2003; Th. Behr

include ../makefile.inc
CLASSFILES = GeoObject.class Params.class

.PHONY:all
all: $(CLASSFILES)
	(cd basic; make all)
	(cd simple; make all)
	(cd composite; make all)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<   

.PHONY: clean
clean:
	(cd basic; make clean)
	(cd simple; make clean)
	(cd composite; make clean)
	rm -f *.class