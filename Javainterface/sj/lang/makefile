# Filename: SecondoJava/sj/lang/makefile 
#
# Last change: Oct. 2002, M. Spiekermann

include ../../makejava.inc

all: Java_ListExpr langClasses

langClasses: IntByReference.class ServerErrorCodes.class SecondoInterface.class \
             ESInterface.class ListExpr.class

Java_ListExpr:
	(cd JavaListExpr; make all)


#////////////// generic rules
%.class: %.java
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH) $<


#////////// clean rules
clean_local:
	rm -f *.class

clean: clean_local
	(cd JavaListExpr; make clean)

mrproper: clean_local
	(cd JavaListExpr; make mrproper)

