#file javagui/viewer/jpeg
#
# changes
# stefan.wich@fernuni-hagen.de created 2003-12

include ../../makefile.inc

.PHONY:all
all: JPEGViewer.class

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<

.PHONY: clean
clean:
	rm -f *.class
	rm -f *.java~
