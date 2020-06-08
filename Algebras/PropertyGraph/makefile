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

# inlcude global definitions
include ../../makefile.env

# expand compile flags if necessary
CCFLAGS += -Werror

# Needed for our clang / bison / yacc stack
CCFLAGS += -Wno-deprecated-register

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 
OBJECTS := CypherScanner.o CypherParser.o $(OBJECTS)
.PHONY: all 
all: $(OBJECTS) $(LIBOBJ)

# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

# helper for generate docs needs to be renamed, because tex will not use filenames like "*.cpp.pdf"
define generate_docfile
	pd2pdf $(1)
	$(eval fn=$(1).pdf)
	$(eval fn=$(subst .flex.,-flex.,$(fn)) )
	$(eval fn=$(subst .cpp.,-cpp.,$(fn)) )
	echo $(fn)
	mv $(1).pdf $(fn) 
endef
	

# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	rm -f CypherParser.cpp CypherParser.h CypherScanner.h CypherScanner.cpp  position.hh location.hh stack.hh 
	rm -f *.aux
	rm -f *.dvi
	rm -f *.log
	rm -f *.tex
	rm -f *.pdf

PropertyGraphAlgebra.cpp: CypherScanner.cpp CypherParser.cpp  

CypherLang.cpp: CypherScanner.cpp CypherParser.cpp 

CypherScanner.h CypherScanner.cpp: Cypher.flex
	flex  Cypher.flex

CypherParser.h CypherParser.cpp: Cypher.bison
	bison  Cypher.bison


makedocs:
	$(call generate_docfile, Cypher.flex)
	$(call generate_docfile, PropertyGraphAlgebra.cpp)

