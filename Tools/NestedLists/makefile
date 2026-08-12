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

platform=$(SECONDO_PLATFORM)


TOKENDEF := NLParser.tab.hpp
SCANNER := NLLex.cpp 
PARSER := NLParser.tab.cpp

DEP_FILES += NLParser.tab.dep

# expand compile flags if necessary
#CCFLAGS += $(ALL_ALGEBRA_DIR_INCLUDE_FLAGS)

GENSRC := $(TOKENDEF) $(PARSER) $(SCANNER)
OBJECTS += NLLex.$(OBJEXT) NLParser.tab.o NLParser.o

.PHONY: all
all: $(GENSRC) $(OBJECTS)


NLParser.tab.cpp: NLParser.y
	$(YACC) -d -o NLParser.tab.cpp $<
$(TOKENDEF): NLParser.tab.cpp ;

# yynerrs is bison's own: yyparse counts syntax errors in it, and a grammar
# reads that count back through its error rules. NLParser.y has none, so the
# variable is written and never read, which clang reports.
NLParser.tab.o: CCFLAGS += -Wno-unused-but-set-variable

# -Cf: full (fastest) scanner tables, instead of flex's default -Cem, which is
# the smallest and slowest. It also implies batch input, so LexerInput does a
# bulk read instead of fetching one character at a time through the istream.
#
# -8 is REQUIRED and must not be dropped. flex generates 7-bit tables by
# default under -Cf/-CF (it halves their size), and nested lists carry 8-bit
# data: bin/berlintest is ISO-8859 with ~9000 bytes.
NLLex.cpp: NLLex.l $(TOKENDEF)
	$(LEX) -Cf -8 -+ -Pnl -o$@ $<

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

.PHONY: clean
clean:
	$(RM) NLParser.output NLParser.tab.* $(GENSRC) $(OBJECTS) $(DEP_FILES)
