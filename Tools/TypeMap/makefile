#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

########################################################################
#
# generic makefile for an algebra module 
#
########################################################################

# include global definitions
include ../../makefile.env

# expand compile flags if necessary
CCFLAGS += -Werror


# create objects and library. 

OBJECTS += scan.o parse.o
LIBOBJ=$(TYPEMAPLIB)

BASE:=OpSigParser

.PHONY: all
all: $(OBJECTS) $(LIBOBJ) makedirs 

# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

# OpSigParser

scan.o: lex.opsig.c 
	g++ -c -fPIC -ggdb -o scan.o lex.opsig.c

lex.opsig.c: $(BASE).l parse.o 
	flex $(BASE).l

parse.o: $(BASE).tab.c
	g++ -c -fPIC -ggdb -o parse.o $(BASE).tab.c -lstd


$(BASE).tab.c: $(BASE).y
	bison -v -d $(BASE).y


# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)


# make also for Parsers directory
.PHONY: makedirs
makedirs: 
	$(MAKE) -C OpSigParser
	$(MAKE) -C OpSpecParser


# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	rm -f $(BASE).tab.c parse.o lex.opsig.c scan.o $(BASE).tab.h $(BASE).output
	$(MAKE) -C OpSigParser clean
	$(MAKE) -C OpSpecParser clean
