#This file is part of SECONDO.
#
#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.
#
#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Filename: SecondoJava/sj/lang/JavaListExpr/makefile
#
# Last change: Oct. 2002, M. Spiekermann

include ../../../makefile.inc

SCANERJAVAFILES = NLJLex.l.java
SCANERCLASSES = NLToken.class NLTokenizer.class

PARSERJAVAFILES = NLParser.java NLSymbol.java
PARSERCLASSES = NLParser.class NLSymbol.class Stack.class

GENERATEDJAVAFILES = $(SCANERJAVAFILES) $(PARSERJAVAFILES)

JLEX_CLASS = JLex.Main

CUP_CLASS = java_cup.Main

CLASSPATH := $(CLASSPATH):$(JLEXCLASSESPATH):$(CUPCLASSESPATH):$(CUPCLASSESRUNTIME)

all: ../ListExpr.class


../ListExpr.class: ../ListExpr.java $(GENERATEDJAVAFILES) $(SCANERCLASSES) $(PARSERCLASSES)
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(GUIPATH) $<


NLJLex.l.java: NLJLex.l
# It generates the file NLJLex.l.java with the code of the scaner.
	$(JAVA) -classpath $(CLASSPATH) $(JLEX_CLASS) $<

NLParser.java NLSymbol.java: NLParser.cup
# It generates the files NLParser.java and NLSymbol.java
	$(JAVA) -classpath $(CLASSPATH) $(CUP_CLASS) -parser NLParser -symbols NLSymbol -interface < $<

NLParser.class NLSymbol.class: NLParser.java NLSymbol.java ../ListExpr.java
# It generates the files NLParser.class and NLSymbol.class
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(GUIPATH) $+

Stack.class: Stack.java
	$(JAVAC) $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(GUIPATH) Stack.java

NLToken.class NLTokenizer.class: NLJLex.l.java ../ListExpr.java
# It generates the files NLToken.class and NLTokenizer.class
	$(JAVAC) -nowarn $(JAVACFLAGS) -classpath $(CLASSPATH) -d $(GUIPATH) $+


#////////// Clean rules
.PHONY clean:
clean:
	 rm -f *.class
	(cd ..; rm -f ListExpr.class $(PARSERCLASSES) $(SCANERCLASSES))
	 rm -f $(GENERATEDJAVAFILES)

