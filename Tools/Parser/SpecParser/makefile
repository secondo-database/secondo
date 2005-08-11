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
########################################################################
#
# SECONDO makefile for special Secondo parser
#
########################################################################

include ../../../makefile.env

all: Parser$(EXEEXT) Parser2$(EXEEXT)

lex.yy.cpp lex2.yy.cpp: SpecLex.l
	$(LEX) -o lex.yy.cpp SpecLex.l
	$(LEX) -d -o lex2.yy.cpp SpecLex.l

SpecParser.tab.hpp SpecParser.tab.cpp: SpecParser.y
	$(YACC) -d -o SpecParser.tab.cpp SpecParser.y


Parser$(EXEEXT): SpecParser.tab.cpp lex.yy.cpp NestedText.h ../NestedText.o
	$(CC) -o Parser$(EXEEXT) SpecParser.tab.cpp lex.yy.cpp ../NestedText.o -I.. $(DEFAULTCCFLAGS) -lstdc++

Parser2$(EXEEXT): SpecParser.tab.cpp lex2.yy.cpp NestedText.h ../NestedText.o
	$(CC) -o Parser2$(EXEEXT) SpecParser.tab.cpp lex2.yy.cpp ../NestedText.o -I.. $(DEFAULTCCFLAGS) -lstdc++
../NestedText.o: ../NestedText.cpp NestedText.h
	$(CC) -c -o ../NestedText.o ../NestedText.cpp -I.. $(DEFAULTCCFLAGS)

clean:
	$(RM) *.o *.lo SpecParser.tab.hpp SpecParser.tab.cpp lex2.yy.cpp lex.yy.cpp Lexer$(EXEEXT) Parser2$(EXEEXT) Parser$(EXEEXT) ../NestedText.o
