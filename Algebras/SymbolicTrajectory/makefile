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

CURRENT_ALGEBRA := SymbolicTrajectoryAlgebra

ALGEBRA_DEPENDENCIES := TrieAlgebra

# inlcude global definitions
include ../../makefile.env



# expand compile flags if necessary
CCFLAGS += $(ALG_INC_DIRS) 

OBJECTS += scan.o parse.o PatternRegExParser.o PatternRegExScanner.o

BASE:=PatternParser

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 
.PHONY: all 
all: check_dependencies $(OBJECTS) $(LIBOBJ)

check_dependencies:
	$(check-algebra-dependencies)


# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)


scan.o: lex.yy.c
	        $(CPPC) -c -ggdb -o scan.o lex.pattern.c $(DEFAULTCCFLAGS)

lex.yy.c: $(BASE).l parse.o
	        flex $(BASE).l

parse.o: $(BASE).tab.c
	        $(CPPC) -c -ggdb -o parse.o $(BASE).tab.c -lstd -I ../../include -I ../../Tools/ListsAndTables -I $(BDB_INCLUDE_DIR) $(ALL_ALGEBRA_DIR_INCLUDE_FLAGS)  $(DEFAULTCCFLAGS)


$(BASE).tab.c: $(BASE).y
	        bison -v -d $(BASE).y

PatternRegExScanner.o: PatternRegExScanner.c
	$(CC) -fPIC -c $(ALG_INC_DIRS) -I../../include -o PatternRegExScanner.o PatternRegExScanner.c

PatternRegExScanner.c: PatternRegExScanner.l PatternRegExParser.y.c
	$(LEX) -o PatternRegExScanner.c PatternRegExScanner.l

PatternRegExParser.o:  PatternRegExParser.y.c
	$(CC) -fPIC -x c++ -c -o PatternRegExParser.o  $(ALG_INC_DIRS) -I../../include PatternRegExParser.y.c

PatternRegExParser.y.c: PatternRegExParser.y
	$(YACC) -d -o PatternRegExParser.y.c PatternRegExParser.y


# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	rm -f $(BASE).tab.c parse.o lex.pattern.c scan.o PatternRegExParser.o PatternRegExScanner.o mini $(BASE).tab.h $(BASE).output PatternRegExParser.y.c PatternRegExScanner.c PatternRegExParser.y.h



