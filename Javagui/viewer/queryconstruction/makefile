# file: viewer/queryconstruction/makefile
#
#

include ../../makefile.inc


JAVAFILES =$(shell find -name "*.java")

.PHONY: all
all: $(JAVA_STAMP)

$(JAVA_STAMP): $(JAVAFILES)
	$(compile-java)


.PHONY: clean
clean:
	rm -f *.class $(JAVA_STAMP)
