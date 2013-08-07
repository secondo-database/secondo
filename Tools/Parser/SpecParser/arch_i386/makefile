########################################################################
#
# SECONDO makefile for special Secondo parser
#
########################################################################
# 31.03.2013 Jürgen Ehnes



include ../../../../makefile.env

DEP_FILES += ../SpecParser.tab.dep
CC := g++


OBJECTS := NestedText.o lex.yy.o lex2.yy.o
GENFILES := lex.yy.cpp lex2.yy.cpp SpecParser.tab.cpp SpecParser.tab.hpp

all: $(GENFILES) $(OBJECTS) Parser$(EXEEXT) Parser2$(EXEEXT)

NestedText.o: NestedText.cpp

lex.yy.o: lex.yy.cpp

lex2.yy.o: lex2.yy.cpp


lex.yy.cpp lex2.yy.cpp: ../SpecLex.l
	$(LEX) -o $@ $<
	$(LEX) -d -o../lex2.yy.cpp $<

NestedText.cpp: ../NestedText.cpp
	$(CP) $< $@     

SpecParser.tab.cpp SpecParser.tab.hpp: ../SpecParser.y
	$(YACC) -d -o $@ $<

Parser$(EXEEXT): SpecParser.tab.o lex.yy.o NestedText.o
	$(CC) -o Parser$(EXEEXT) $^ $(DEFAULTCCFLAGS)
	$(CP) $@ ../$@ 

Parser2$(EXEEXT): SpecParser.tab.o lex2.yy.o NestedText.o
	$(CC) -o Parser2$(EXEEXT) $^ $(DEFAULTCCFLAGS)
	$(CP) $@ ../$@

ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

clean:
	$(RM) $(GENFILES) $(DEP_FILES) $(OBJECTS) SpecParser.tab.o ../Parser$(EXEEXT) ../Parser2$(EXEEXT) Parser$(EXEEXT) Parser2$(EXEEXT) NestedText.cpp

