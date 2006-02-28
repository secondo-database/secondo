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
all: $(DEP_FILES) $(OBJECTS)

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
	$(LEX) -d -+ -Pnl -oNLLex.cpp NLLex.l


include $(DEP_FILES)

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEP_FILES)
