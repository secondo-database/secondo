include ../../../../makefile.env

TMPFILES =$(shell find -name "*.java")
JAVAFILES = $(subst ./,,$(TMPFILES))

.PHONY: all
all:$(JPL_CLASS_TARGET)/jpl/fli/Prolog.class

$(JPL_CLASS_TARGET)/jpl/fli/Prolog.class:
	javac -classpath $(JPL_CLASSPATH) -d $(JPL_CLASS_TARGET) $(JAVAFILES)

.PHONY: clean
clean:



