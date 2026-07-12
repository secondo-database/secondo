# file: viewer/update2/gui/makefile
#
# last change: 01-2014; Thomas Behr
#

include ../../../makefile.inc


JAVAFILES =$(shell find . -name "*.java")
JAVACFLAGS := -Xlint:unchecked

.PHONY: all
all: $(JAVA_STAMP)

$(JAVA_STAMP): $(JAVAFILES)
	$(compile-java)


.PHONY: clean
clean:
	rm -f *.class $(JAVA_STAMP)
