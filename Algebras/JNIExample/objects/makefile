# 
#
# last change: 2003-07-03; M. Guenster

include ../makefile.inc
CLASSFILES = PointJNITest.class RationalJNITest.java

.PHONY:all
all: $(CLASSFILES)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<   

.PHONY: clean
clean:
	rm -f *.class
	rm -f *~
