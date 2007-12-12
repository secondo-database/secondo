# March 2005, M. Spiekermann
#
# Create a relation object called paintings
# containing about 4100 JPEG objects.

.PHONY all:
all: paintings Convert.class

paintings:
	find . -iname "*.jpg" | artpics2rel.sh > $@ 


Convert.class: Convert.java
	javac Convert.java

.PHONY: clean
clean:
	rm -f *.class
