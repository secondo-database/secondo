########################################################################
#
# SECONDO makefile for nested lists
#
########################################################################

include ../../makefile.env

ifeq ($(NL_PERSISTENT),"true")
DEFAULTCCFLAGS += -DNL_PERSISTENT
endif

OBJECTS=\
	NLLex.$(OBJEXT) \
	NLScanner.$(OBJEXT) \
	NLParser.tab.$(OBJEXT) \
	NLParser.$(OBJEXT) \
	NestedList.$(OBJEXT)


CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))


.PHONY: all
all: $(CODE_DEPENDENCIES) $(OBJECTS) NLParser.tab.cpp NLParser.tab.hpp

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

NLParser.tab.cpp NLParser.tab.hpp: NLParser.y
	$(YACC) -d -p __NLParser__ -o NLParser.tab.c NLParser.y
	sed \
	   -e "/^int yyparse (void[ *]*);$$/d" \
	   -e "/^YYSTYPE yylval;/d" \
	   -e "/^YYPARSE_RETURN_TYPE yyparse (void[ *]*);$$/d" \
	   -e s/__NLParser__parse/NLParser::yyparse/g \
	   -e s/__NLParser__lex/yylex/g \
	   -e s/__NLParser__error/yyerror/g \
	   -e s/__NLParser__lval/NLParser::yylval/g \
	   -e s/__NLParser__char/NLParser::yychar/g \
	   -e s/__NLParser__debug/NLParser::yydebug/g \
	   -e s/__NLParser__nerrs/NLParser::yynerrs/g \
		   < NLParser.tab.c > NLParser.tab.cpp
	$(RM) NLParser.tab.c
	sed \
	   -e s/__NLParser__lval/NLParser::yylval/g \
	   < NLParser.tab.h > NLParser.tab.hpp
	$(RM) NLParser.tab.h


NLLex.cpp: NLLex.l
	$(LEX) -+ -Pnl -oNLLex.cpp NLLex.l


include $(CODE_DEPENDENCIES:.cpp=.dep)

#include ../../include.dep

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(CODE_DEPENDENCIES) NLParser.tab.hpp NLParser.tab.cpp

