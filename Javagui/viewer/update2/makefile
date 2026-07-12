# file: viewer/update2/makefile
#
# last change: 05-2003; Thomas Behr
# very last change: 01-2005; Uwe Hartmann
#

include ../../makefile.inc


JAVAFILES =$(shell find . -name "*.java")
JAVACFLAGS := -Xlint:unchecked

.PHONY: all
all: $(JAVA_STAMP)
	$(MAKE) -C format
	$(MAKE) -C gui

$(JAVA_STAMP): $(JAVAFILES)
	$(compile-java)


.PHONY: clean
clean:
	$(MAKE) -C format clean
	$(MAKE) -C gui clean
	rm -f *.class $(JAVA_STAMP)
