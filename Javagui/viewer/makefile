# file viewer/makefile
#
# last change 05-2003; Thomas Behr

include ../makefile.inc


.PHONY: all  # compile Menuvector and SecondoViewer
all: MenuVector.class SecondoViewer.class


# compile all viewers
# if a new viewer inserted then
# extend this call
.PHONY: viewers 
viewers: hoese RelViewer.class StandardViewer.class

#
# the viewer from Thomas Hoese (a gui in the past)
#
.PHONY: hoese
hoese: subhoese HoeseViewer.class
.PHONY: subhoese
subhoese:
	(cd hoese;make all)

HoeseViewer.class: HoeseViewer.java
	$(JAVAC) -classpath $(CLASSPATH) HoeseViewer.java
	
	
RelViewer.class:RelViewer.java
	$(JAVAC) -classpath $(CLASSPATH) RelViewer.java

StandardViewer.class: StandardViewer.java
	$(JAVAC) -classpath $(CLASSPATH) StandardViewer.java



MenuVector.class: MenuVector.java
	$(JAVAC) -classpath $(CLASSPATH) MenuVector.java

SecondoViewer.class: SecondoViewer.java 
	$(JAVAC) -classpath $(CLASSPATH) SecondoViewer.java

.PHONY: clean
clean: 
	(cd hoese;make clean)
	rm -f *.class
