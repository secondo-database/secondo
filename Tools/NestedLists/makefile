########################################################################
#
# SECONDO makefile for nested lists
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

OBJECTS=\
	NLLex.$(OBJEXT) \
	NLScanner.$(OBJEXT) \
	NLParser.tab.$(OBJEXT) \
	NLParser.$(OBJEXT) \
	NestedList.$(OBJEXT)

.PHONY: all
all: $(OBJECTS) ../../include/CTable.h

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

../../include/CTable.h: ../ListsAndTables/CTable.h ../ListsAndTables/CTable.cpp
	$(CP) ../ListsAndTables/CTable.h ../../include
	$(CP) ../ListsAndTables/CTable.cpp ../../include

NestedList.$(OBJEXT): NestedList.cpp ../../include/NestedList.h NLParser.h
	$(CC) -c -o NestedList.$(OBJEXT) NestedList.cpp $(CCFLAGS)

NLParser.$(OBJEXT):	NLParser.cpp NLParser.h NLScanner.h ../../include/NestedList.h
	$(CC) -c -o NLParser.$(OBJEXT) NLParser.cpp $(CCFLAGS)

NLParser.tab.$(OBJEXT):	NLParser.tab.cpp NLParser.h ../../include/NestedList.h
	$(CC) -DYYDEBUG -c -o NLParser.tab.$(OBJEXT) NLParser.tab.cpp $(CCFLAGS)

NLParser.tab.cpp NLParser.tab.hpp:	NLParser.y
	$(YACC) -d -p __NLParser__ -o NLParser.tab.c NLParser.y
	sed \
	   -e "/^int yyparse (void[ *]*);$$/d" \
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

NLScanner.$(OBJEXT):	NLScanner.cpp NLScanner.h
	$(CC) -c -o NLScanner.$(OBJEXT) NLScanner.cpp $(CCFLAGS)

NLLex.$(OBJEXT):	NLLex.cpp NLScanner.h NLParser.h NLParser.tab.hpp
	$(CC) $(CCFLAGS) -c -o NLLex.$(OBJEXT) NLLex.cpp

NLLex.cpp:	NLLex.l ../../include/NestedList.h NLParser.tab.hpp
	$(LEX) -+ -Pnl -oNLLex.cpp NLLex.l

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo
	$(RM) NLLex.cpp NLParser.tab.hpp NLParser.tab.cpp

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

