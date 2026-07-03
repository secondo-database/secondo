

include ../makefile.inc

CLASSPATH=..

JAVAFILES := $(wildcard *.java)

.PHONY:all

all: .javac-stamp

# compile all sources with a single javac invocation instead of one per file
.javac-stamp: $(JAVAFILES)
	$(JAVAC) -classpath $(CLASSPATH) $?
	@touch $@



.PHONY:clean
clean:
	rm -rf *class .javac-stamp


