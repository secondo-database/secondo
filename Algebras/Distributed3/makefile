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

CURRENT_ALGEBRA := Distributed3Algebra

ALGEBRA_DEPENDENCIES := StandardAlgebra
ALGEBRA_DEPENDENCIES += RelationAlgebra
ALGEBRA_DEPENDENCIES += FunctionAlgebra
ALGEBRA_DEPENDENCIES += ArrayAlgebra
ALGEBRA_DEPENDENCIES += FTextAlgebra
ALGEBRA_DEPENDENCIES += DFSAlgebra
ALGEBRA_DEPENDENCIES += Distributed2Algebra

# include global definitions
include ../../makefile.env

# expand compile flags if necessary
CCFLAGS += -Werror 

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 

.PHONY: all
all: check_dependencies $(OBJECTS) $(LIBOBJ)
	$(MAKE) -C tes
	$(MAKE) -C tes/Operators
	$(MAKE) -C tes/Operators/Interface2TES
	$(MAKE) -C tes/Operators/Messaging
	$(MAKE) -C tes/Operators/OnMaster
	$(MAKE) -C tes/Operators/OnWorker
	$(MAKE) -C tes/MessageBroker
	$(MAKE) -C tes/Helpers

check_dependencies:
	$(check-algebra-dependencies)

# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	$(MAKE) -C tes clean
	$(MAKE) -C tes/Operators clean
	$(MAKE) -C tes/Operators/Interface2TES clean
	$(MAKE) -C tes/Operators/Messaging clean
	$(MAKE) -C tes/Operators/OnMaster clean
	$(MAKE) -C tes/Operators/OnWorker clean
	$(MAKE) -C tes/MessageBroker clean
	$(MAKE) -C tes/Helpers clean

