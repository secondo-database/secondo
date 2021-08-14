CLASSPATH = ..

.PHONY: all
all: Main.class
	$(MAKE) -C runtime all

%.class: %.java
	javac -classpath $(CLASSPATH) $<

.PHONY: clean
clean: 
	$(MAKE) -C runtime clean
	rm -f *.class
