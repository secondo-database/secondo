# makefile for build the open geo db converter
# last change - Jul 2004

.PHONY: all
all: Converter.class
	@echo -e "to run the converter type in \"java Converter InputFile > OutputFile\""

Converter.class: Converter.java
	javac Converter.java

.PHONY:clean
clean:
	rm -f *.class
