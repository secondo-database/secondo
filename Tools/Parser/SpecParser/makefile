########################################################################
#
# SECONDO makefile for special Secondo parser
#
########################################################################

include ../../../makefile.env

all: Lexer$(EXEEXT) Parser$(EXEEXT)

lex.yy.cpp: SpecLex.l
	$(LEX) -olex.yy.cpp SpecLex.l

SpecParser.tab.hpp SpecParser.tab.cpp: SpecParser.y
	$(YACC) -d -o SpecParser.tab.cpp SpecParser.y

Lexer$(EXEEXT): LexTest.cpp lex.yy.cpp ../NestedText.h ../NestedText.o SpecParser.tab.hpp
	$(CC) -o Lexer$(EXEEXT) LexTest.cpp lex.yy.cpp ../NestedText.o -I.. $(DEFAULTCCFLAGS) -lstdc++

Parser$(EXEEXT): Translate.cpp SpecParser.tab.cpp lex.yy.cpp ../NestedText.h ../NestedText.o
	$(CC) -o Parser$(EXEEXT) Translate.cpp SpecParser.tab.cpp lex.yy.cpp ../NestedText.o -I.. $(DEFAULTCCFLAGS) -lstdc++

../NestedText.o: ../NestedText.cpp ../NestedText.h
	$(CC) -c -o ../NestedText.o ../NestedText.cpp -I.. $(DEFAULTCCFLAGS)

clean:
	$(RM) *.o
	$(RM) *.lo
	$(RM) SpecParser.tab.hpp SpecParser.tab.cpp lex.yy.cpp
	$(RM) Lexer$(EXEEXT) Parser$(EXEEXT)

