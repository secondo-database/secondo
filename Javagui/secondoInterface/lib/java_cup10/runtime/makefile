JAVAFILES := $(wildcard *.java)

CLASSPATH = ../..

# compile all sources with a single javac invocation instead of one per file
.PHONY: all
all: .javac-stamp

.javac-stamp: $(JAVAFILES)
	javac -classpath $(CLASSPATH) $?
	@touch $@

.PHONY: clean
clean:
	rm -f *.class .javac-stamp

