CLASSPATH = ..
JAVAFILES := $(wildcard *.java)

.PHONY: all
all: Main.class
	$(MAKE) -C runtime all

Main.class: $(JAVAFILES)
	javac -classpath $(CLASSPATH) Main.java

.PHONY: clean
clean: 
	$(MAKE) -C runtime clean
	rm -f *.class
