

include ../makefile.inc

CLASSPATH=..

.PHONY:all

all: Base64Encoder.class Base64Decoder.class TextFormat.class Environment.class Reporter.class




%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<



.PHONY:clean
clean:
	rm -rf *class


