########################################################################
#
# SECONDO makefile for nested lists
#
########################################################################

include ../../makefile.env

ifeq ($(NL_PERSISTENT),"true")
DEFAULTCCFLAGS += -DNL_PERSISTENT
endif

DEP_FILES += NLParser.tab.dep

OBJECTS += NLLex.$(OBJEXT) NLParser.tab.cpp NLParser.tab.hpp

.PHONY: all
all: $(OBJECTS)

NLParser.tab.cpp NLParser.tab.hpp: NLParser.y
	$(YACC) -d -p __NLParser__ -o NLParser.tab.c NLParser.y
# The RECODE macro is only used for platform wine
ifeq ($(WINE),true)
	recode cp1252..lat1 NLParser.tab.c 
endif
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


include $(DEP_FILES)

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEP_FILES)
