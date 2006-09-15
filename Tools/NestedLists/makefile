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

CC := g++

TOKENDEF := NLParser.tab.hpp
SCANNER := NLLex.cpp 
PARSER := NLParser.tab.cpp

DEP_FILES += NLParser.tab.dep

ifeq ($(NL_PERSISTENT),"true")
DEFAULTCCFLAGS += -DNL_PERSISTENT
endif

GENSRC := $(TOKENDEF) $(PARSER) $(SCANNER)
OBJECTS += NLLex.$(OBJEXT) NLParser.tab.o NLParser.o


.PHONY: all
all: $(GENSRC) $(OBJECTS)

NLParser.tab.cpp $(TOKENDEF): NLParser.y
	$(YACC) -d -o $@ $<

NLLex.cpp: NLLex.l $(TOKENDEF)
	$(LEX) -d -+ -Pnl -o$@ $<

ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

.PHONY: clean
clean:
	$(RM) NLParser.output NLParser.tab.* $(GENSRC) $(OBJECTS) $(DEP_FILES)
