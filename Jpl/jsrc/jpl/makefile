include ../../../makefile.env

# get all class files in this directory
TMPFILES =$(shell find -maxdepth 1 -name "*.java")
JAVAFILES = $(subst ./,,$(TMPFILES))



.PHONY: all
all: fli $(JPL_CLASS_TARGET)/jpl/Query.class

.PHONY: fli
fli:
	make -C fli all


$(JPL_CLASS_TARGET)/jpl/Query.class:
	javac -classpath $(JPL_CLASSPATH) -d $(JPL_CLASS_TARGET) $(JAVAFILES)

.PHONY: clean
clean:
	make -C fli clean



