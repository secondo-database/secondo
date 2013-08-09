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

ifeq ($(platform),android)
CC := $(HOME)/toolchain-standalone/bin/arm-linux-androideabi-g++
CCFLAGS += -I$(SECONDO_BUILD_DIR)/../flex/include
else
CC := g++
endif

TOKENDEF := NLParser.tab.hpp
SCANNER := NLLex.cpp 
PARSER := NLParser.tab.cpp

DEP_FILES += NLParser.tab.dep

# expand compile flags if necessary
CCFLAGS += $(ALL_ALGEBRA_DIR_INCLUDE_FLAGS)
CCFLAGS += -DYY_FLEX_MAJOR_VERSION=$(shell ../../bin/getversion.sh major flex -V)
CCFLAGS += -DYY_FLEX_MINOR_VERSION=$(shell ../../bin/getversion.sh minor flex -V)
CCFLAGS += -DYY_FLEX_SUBMINOR_VERSION=$(shell ../../bin/getversion.sh subminor flex -V)

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
