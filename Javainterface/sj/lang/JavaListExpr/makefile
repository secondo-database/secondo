# Filename: SecondoJava/sj/lang/JavaListExpr/makefile
#
# Last change: Oct. 2002, M. Spiekermann

include ../../../makejava.inc

SCANERJAVAFILES = NLJLex.l.java
SCANERCLASSES = NLToken.class NLTokenizer.class

PARSERJAVAFILES = NLParser.java NLSymbol.java
PARSERCLASSES = NLParser.class NLSymbol.class

GENERATEDJAVAFILES = $(SCANERJAVAFILES) $(PARSERJAVAFILES)

JLEX_CLASS = JLex.Main

CUP_CLASS = java_cup.Main


all: ../ListExpr.class


../ListExpr.class: ../ListExpr.java $(GENERATEDJAVAFILES) $(SCANERCLASSES) $(PARSERCLASSES) 
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(JAVAINTERFACEDIR) $<


NLJLex.l.java: NLJLex.l
# It generates the file NLJLex.l.java with the code of the scaner.
	$(JAVA) -classpath $(CLASSPATH) $(JLEX_CLASS) $<

NLParser.java NLSymbol.java: NLParser.cup
# It generates the files NLParser.java and NLSymbol.java
	$(JAVA) -classpath $(CLASSPATH) $(CUP_CLASS) -parser NLParser -symbols NLSymbol -interface < $<

NLParser.class NLSymbol.class: NLParser.java NLSymbol.java ../ListExpr.java
# It generates the files NLParser.class and NLSymbol.class
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(JAVAINTERFACEDIR) $+ 

NLToken.class NLTokenizer.class: NLJLex.l.java ../ListExpr.java
# It generates the files NLToken.class and NLTokenizer.class
	$(JAVAC) -nowarn $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(JAVAINTERFACEDIR) $+


#////////// Clean rules
.PHONY clean:
clean:
	 rm -f *.class
	(cd ..; rm -f ListExpr.class $(PARSERCLASSES) $(SCANERCLASSES))

.PHONY mrproper:
mrproper: clean
	 rm -f $(GENERATEDJAVAFILES)
