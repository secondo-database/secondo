########################################################################
#
# SECONDO makefile for Secondo parser
#
########################################################################

include ../../makefile.env

OBJECTS =\
	SecLex.$(OBJEXT) \
	SecParser.tab.$(OBJEXT) \
	NestedText.$(OBJEXT) \
	SecParser.$(OBJEXT)

CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

all: $(CODE_DEPENDENCIES) $(OBJECTS) 


# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $<  $(CCFLAGS)

#
# Rules for the generation of scanner and parser code
#
SCANNER_SPEC = SecLex.l.1 lexrules SecLex.l.2
PARSER_GRAMMAR = SecParser.y.1 tokens SecParser.y.2 yaccrules1 yaccrules2 SecParser.y.3
GENERATED_CPP_FILES = SecParser.tab.hpp SecParser.tab.cpp SecLex.cpp
GENERATED_LEXYACC_FILES = SecLex.l SecParser.y

SecParser.tab.hpp SecParser.tab.cpp: SecParser.y
	$(YACC) -d -p xx -o SecParser.tab.cpp SecParser.y

SecLex.cpp: SecLex.l
	$(LEX) -+ -oSecLex.cpp -Pxx SecLex.l

SecLex.l: $(SCANNER_SPEC) 
	cat $^ > SecLex.l

SecParser.y: $(PARSER_GRAMMAR) 
	cat $^ > SecParser.y


../../Algebras/specs:
	$(MAKE) -C ../../Algebras specs

SpecParser/Parser$(EXEEXT):
	$(MAKE) -C SpecParser

lexrules tokens yaccrules1 yaccrules2: ../../Algebras/specs SpecParser/Parser$(EXEEXT)

	SpecParser/Parser < ../../Algebras/specs


include $(CODE_DEPENDENCIES:.cpp=.dep)


.PHONY: clean
clean:
	$(MAKE) -C SpecParser clean
	$(RM) *.dep $(OBJECTS) $(GENERATED_CPP_FILES) $(GENERATED_LEXYAC_FILES)
