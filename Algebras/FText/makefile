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


CURRENT_ALGEBRA := FTextAlgebra


ALGEBRA_DEPENDENCIES := StandardAlgebra
ALGEBRA_DEPENDENCIES += RelationAlgebra
ALGEBRA_DEPENDENCIES += FunctionAlgebra


# include global definitions
include ../../makefile.env

# expand compile flags if necessary
CCFLAGS += $(ALG_INC_DIRS) 

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 

OBJECTS += RegExParser.o RegExScanner.o
.PHONY: all
all: check_dependencies $(OBJECTS) $(LIBOBJ)

check_dependencies:
	$(check-algebra-dependencies)


# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

RegExScanner.o: RegExScanner.c
	$(CC) -fPIC -c $(ALG_INC_DIRS) -I../../include -o RegExScanner.o RegExScanner.c

RegExScanner.c: RegExScanner.l RegExParser.y.c
	$(LEX) -o RegExScanner.c RegExScanner.l

RegExParser.o:  RegExParser.y.c
	$(CC) -fPIC -x c++ -c -o RegExParser.o  $(ALG_INC_DIRS) -I../../include RegExParser.y.c

RegExParser.y.c: RegExParser.y 
	$(YACC) -d -o RegExParser.y.c RegExParser.y


# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	rm -f RegExParser.y.c RegExScanner.c RegExParser.y.h 
