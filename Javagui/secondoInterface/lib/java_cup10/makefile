
JAVAFILES := $(wildcard *.java)
CLASSFILES := $(subst .java,.class,$(JAVAFILES))

CLASSPATH = ..

.PHONY: all
all: $(CLASSFILES)
	$(MAKE) -C runtime all

%.class: %.java
	javac -classpath $(CLASSPATH) $<

.PHONY: clean
clean: 
	$(MAKE) -C runtime clean
	rm -f *.class
