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
# SECONDO makefile for Relation Algebra C++
#
########################################################################

include ../../makefile.env

MODNAME = ExtRelationAlgebra

CCFLAGS = $(DEFAULTCCFLAGS) -I../Relation-C++ -I../TupleIdentifier

# define the compiler flag -DRELALG_PERSISTENT in the file 
# makefile.env to compile the Persistent Relational Algebra.
# It must be defined on a higher level since other algebras have
# to interoperate with the relational algebra.

.PHONY: all
all: $(OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

# --- Relation Algebra C++ library ---

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)

