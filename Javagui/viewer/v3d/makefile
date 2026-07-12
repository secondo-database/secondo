# file: viewer/update/makefile
#
# last change: 05-2003; Thomas Behr
# very last change: 01-2005; Uwe Hartmann
#

include ../../makefile.inc


JAVAFILES =$(shell find . -name "*.java")
EXTRACP = $(JAVA3DCP)
JAVA3DCP = /usr/share/java/j3dcore.jar:/usr/share/java/j3dutils.jar:/usr/share/java/vecmath.jar

.PHONY: all
all: $(JAVA_STAMP)

$(JAVA_STAMP): $(JAVAFILES)
	$(compile-java)


.PHONY: clean
clean:
	rm -f *.class $(JAVA_STAMP)
