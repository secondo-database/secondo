########################################################################
#
# SECONDO makefile for special Secondo parser
#
########################################################################

include ../../../makefile.config
ifdef platform
include ../../../makefile.$(platform)

all: Lexer$(EXEEXT) Parser$(EXEEXT)

lex.yy.cpp: SpecLex.l
	$(LEX) -olex.yy.cpp SpecLex.l

SpecParser.tab.hpp SpecParser.tab.cpp: SpecParser.y
	$(YACC) -d -o SpecParser.tab.cpp SpecParser.y

Lexer$(EXEEXT): LexTest.cpp lex.yy.cpp ../NestedText.h ../NestedText.$(OBJEXT) SpecParser.tab.hpp
	$(CC) -o Lexer$(EXEEXT) LexTest.cpp lex.yy.cpp ../NestedText.$(OBJEXT) -I.. $(DEFAULTCCFLAGS) -lstdc++

Parser$(EXEEXT): Translate.cpp SpecParser.tab.cpp lex.yy.cpp ../NestedText.h ../NestedText.$(OBJEXT)
	$(CC) -o Parser$(EXEEXT) Translate.cpp SpecParser.tab.cpp lex.yy.cpp ../NestedText.$(OBJEXT) -I.. $(DEFAULTCCFLAGS) -lstdc++

../NestedText.$(OBJEXT): ../NestedText.cpp ../NestedText.h
	$(CC) -c -o ../NestedText.$(OBJEXT) ../NestedText.cpp -I.. $(DEFAULTCCFLAGS)

clean:
	$(RM) *.o
	$(RM) *.lo
	$(RM) SpecParser.tab.hpp SpecParser.tab.cpp lex.yy.cpp
	$(RM) Lexer$(EXEEXT) Parser$(EXEEXT)

else
# Platform not specified
.DEFAULT:
	@echo *** Platform not specified in makefile.config
endif

