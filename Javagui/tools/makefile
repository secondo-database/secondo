# file: gui/makefile
#
# last changed : 11-2003

include ../makefile.inc

CLASSFILES=xxx2secondo.class MovingPointBuilder.class Base64Encoder.class Base64Decoder.class Base64.class DupRemover.class

.PHONY: all
all: $(CLASSFILES)


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<




.PHONY: clean
clean:
	rm -f *.class


