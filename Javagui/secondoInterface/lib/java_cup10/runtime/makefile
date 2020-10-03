JAVAFILES := $(wildcard *.java)
CLASSFILES := $(subst .java,.class,$(JAVAFILES))

CLASSPATH = ../..

.PHONY: all
all: $(CLASSFILES) 

%.class: %.java
	javac -classpath $(CLASSPATH) $<

.PHONY: clean
clean: 
	rm -f *.class

