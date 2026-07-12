# file: viewer/pictureviewer/makefile
#
# last change: 05-2003; Thomas Behr
# very last change: 01-2005; Uwe Hartmann
#

include ../../makefile.inc


JAVAFILES =$(shell find . -name "*.java")

.PHONY: all
all: $(JAVA_STAMP)

$(JAVA_STAMP): $(JAVAFILES)
	$(compile-java)


.PHONY: clean
clean:
	rm -f *.class $(JAVA_STAMP)
