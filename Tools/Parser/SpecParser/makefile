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
# SECONDO makefile for special Secondo parser
#
########################################################################


ifdef SECONDO_HOST_PLATFORM
SECONDO_PLATFORM=$(SECONDO_HOST_PLATFORM)
endif
include ../../../makefile.env

DEP_FILES += SpecParser.tab.dep

CCFLAGS += $(DEFAULTCCFLAGS)

OBJECTS := NestedText.o lex.yy.o lex2.yy.o 
GENFILES := lex.yy.cpp lex2.yy.cpp SpecParser.tab.cpp 

all: $(GENFILES) $(OBJECTS) Parser$(EXEEXT) Parser2$(EXEEXT)

lex.yy.cpp: SpecLex.l
	$(LEX) -o$@ $<

lex2.yy.cpp: SpecLex.l
	$(LEX) -d -olex2.yy.cpp $<

SpecParser.tab.cpp: SpecParser.y
	$(YACC) -d -o $@ $<

Parser$(EXEEXT): SpecParser.tab.o lex.yy.o NestedText.o
	$(CXX) -o Parser$(EXEEXT) $^ $(CCFLAGS) $(COMMON_LD_FLAGS)

Parser2$(EXEEXT): SpecParser.tab.o lex2.yy.o NestedText.o
	$(CXX) -o Parser2$(EXEEXT) $^ $(CCFLAGS) $(COMMON_LD_FLAGS) 

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

clean:
	$(RM) $(GENFILES) $(DEP_FILES) $(OBJECTS) SpecParser.tab.o SpecParser.tab.hpp Parser$(EXEEXT) Parser2$(EXEEXT)

