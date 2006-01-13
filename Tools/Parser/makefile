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

include ../../makefile.env

OBJECTS =\
	SecLex.$(OBJEXT) \
	SecParser.tab.$(OBJEXT) \
	NestedText.$(OBJEXT) \
	SecParser.$(OBJEXT)

CCFLAGS = $(DEFAULTCCFLAGS)

all: useddirectories $(OBJECTS) 

useddirectories: 
	$(MAKE) -C SpecParser
	$(MAKE) -C ../../Algebras specs

SecLex.$(OBJEXT): SecLex.cpp SecParser.tab.hpp NestedText.h
	$(CC) -c -o $@ $< $(CCFLAGS)

SecLex.cpp: SecLex.l
	$(LEX) -d -+ -o$@ -Pxx $<

SecParser.tab.$(OBJEXT): SecParser.tab.cpp NestedText.h
	$(CC) -c -o $@ $< $(CCFLAGS)

SecParser.tab.hpp SecParser.tab.cpp: SecParser.y
	$(YACC) -d -p xx -o SecParser.tab.cpp $<

NestedText.$(OBJEXT): NestedText.cpp NestedText.h
	$(CC) -c -o $@ $< $(CCFLAGS)

NestedText.h: CTable.h

SecParser.$(OBJEXT): SecParser.cpp NestedText.h SecParser.h
	$(CC) -c -o $@ $< $(CCFLAGS)

SecLex.l: SecLex.l.1 lexrules SecLex.l.2
	cat $^ > $@

SecParser.y: SecParser.y.1 tokens SecParser.y.2 yaccrules1 yaccrules2 SecParser.y.3
	cat $^ > $@

ifdef WINE
# wine has problem to execute the Specparser
# as work around it will be compiled with the
# native compiler from linux using the shell script
# wmake.sh
PARSEREXT := .linux
else
PARSEREXT := $(EXEEXT) 
endif

lexrules tokens yaccrules1 yaccrules2: ../../Algebras/specs SpecParser/Parser$(PARSEREXT)
	SpecParser/Parser$(PARSEREXT) < $< 

.PHONY: clean
clean:
	$(MAKE) -C SpecParser clean
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.output
	$(RM) $(OBJECTS) SecLex.cpp SecParser.tab.cpp SecParser.tab.hpp
	$(RM) lexrules tokens yaccrules1 yaccrules2 SecLex.l SecParser.y

