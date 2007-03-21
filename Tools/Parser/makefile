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
# SECONDO makefile for Secondo parser
#
########################################################################

# Remark: Sept. 2006, M. Spiekermann
#
# Here we build first a parser called spec parser which translates the
# algebra's spec-files int flex and bison specifications which are needed
# to define and build the secondo parser. This is the parser which translates
# secondo commands of the executable level into nested list syntax which will
# then be processed by the SecondoInterface::Secondo(). 
#

include ../../makefile.env

ALL_ALGEBRA_DIRS := $(shell find ../../Algebras -maxdepth 1 -type d ! -name "Algebras" ! -name "Management" ! -name "CVS" -printf "%p ")

SPECFILES := $(wildcard $(addsuffix /*.spec, $(ALL_ALGEBRA_DIRS) ))

ifdef WINE
# wine has problem to execute the Specparser
# as work around it will be compiled with the
# native compiler from linux using the shell script
# wmake.sh
PARSEREXT := .linux
else
PARSEREXT := $(EXEEXT) 
endif

OBJECTS =\
	SecLex.$(OBJEXT) \
	SecParser.tab.$(OBJEXT) \
	SecParser.$(OBJEXT)

SCANNER_DEF := SecLex.l
PARSER_DEF := SecParser.y
TOKEN_DEF := SecParser.tab.hpp
SCANNER := SecLex.cpp
PARSER := SecParser.tab.cpp

DEP_FILES += SecParser.tab.dep

GENFILES := $(SCANNER_DEF) $(PARSER_DEF) $(TOKEN_DEF) $(SCANNER) $(PARSER) \
	    lexrules tokens yaccrules1 yaccrules2 

CC := g++
CCFLAGS = $(DEFAULTCCFLAGS)

SPECPARSER := SpecParser/Parser$(PARSEREXT)
SPECFILE := ../../Algebras/specs 

all: SpecParser/Parser$(PARSEREXT) $(SPECFILE) $(GENFILES) $(OBJECTS)
#	echo $(ALL_ALGEBRA_DIRS)
#	echo "---------------"
#	echo $(SPECFILES)

$(SPECPARSER):
	$(MAKE) -C SpecParser

$(SPECFILE): $(SPECPARSER) $(SPECFILES) 
	$(MAKE) -C ../../Algebras specs

SecLex.cpp: SecLex.l
	$(LEX) -d -+ -o$@ -Pxx $<

SecParser.tab.cpp SecParser.tab.hpp: SecParser.y
	$(YACC) -d -p xx -o $@ $<

SecLex.l: SecLex.l.1 lexrules SecLex.l.2
	cat $^ > $@

SecParser.y: SecParser.y.1 tokens SecParser.y.2 yaccrules1 yaccrules2 SecParser.y.3
	cat $^ > $@


ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

lexrules tokens yaccrules1 yaccrules2: $(SPECFILE) $(SPECPARSER) 
	$(SPECPARSER) < $< 

.PHONY: clean
clean:
	$(MAKE) -C SpecParser clean
	$(RM) *.output
	$(RM) $(GENFILES) $(OBJECTS) $(DEP_FILES)
	$(RM) lexrules tokens yaccrules1 yaccrules2 

