########################################################################
#
# SECONDO makefile for Secondo parser
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)

OBJECTS =\
	SecLex.$(OBJEXT) \
	SecParser.tab.$(OBJEXT) \
	NestedText.$(OBJEXT) \
	SecParser.$(OBJEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

all: useddirectories $(OBJECTS) 

useddirectories: 
	$(MAKE) -C ../../Algebras specs
	$(MAKE) -C SpecParser

SecLex.$(OBJEXT): SecLex.cpp
	$(CC) -c -o SecLex.$(OBJEXT) SecLex.cpp $(DEFAULTCCFLAGS)

SecLex.cpp: SecLex.l SecParser.tab.hpp
	$(LEX) -+ -oSecLex.cpp -Pxx SecLex.l

SecParser.tab.$(OBJEXT): SecParser.tab.cpp
	$(CC) -c -o SecParser.tab.$(OBJEXT) SecParser.tab.cpp $(CCFLAGS)

SecParser.tab.hpp SecParser.tab.cpp: SecParser.y NestedText.h
	$(YACC) -d -p xx -o SecParser.tab.cpp SecParser.y

NestedText.$(OBJEXT): NestedText.cpp NestedText.h
	$(CC) -c -o NestedText.$(OBJEXT) NestedText.cpp $(CCFLAGS)

SecParser.$(OBJEXT): SecParser.cpp
	$(CC) -c -o SecParser.$(OBJEXT) SecParser.cpp $(CCFLAGS)

SecLex.l: SecLex.l.1 lexrules SecLex.l.2
	cat SecLex.l.1 lexrules SecLex.l.2 > SecLex.l

SecParser.y: SecParser.y.1 tokens SecParser.y.2 yaccrules1 yaccrules2 SecParser.y.3
	cat SecParser.y.1 tokens SecParser.y.2 yaccrules1 yaccrules2 SecParser.y.3 > SecParser.y

lexrules tokens yaccrules1 yaccrules2: ../../Algebras/specs SpecParser/Parser$(EXEEXT)
ifeq ($(platform),win32)
	SpecParser\Parser < ..\..\Algebras\specs
else
	SpecParser/Parser < ../../Algebras/specs
endif

clean:
	$(MAKE) -C SpecParser clean
	$(RM) $(OBJECTS) SecLex.cpp SecParser.tab.cpp SecParser.tab.hpp
	$(RM) lexrules tokens yaccrules1 yaccrules2 SecLex.l SecParser.y

else
# Platform not specified
.DEFAULT:
	@echo *** Platform not specified in makefile.config
endif

