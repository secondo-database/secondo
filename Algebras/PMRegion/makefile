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
CCFLAGS += -DCGAL_DISABLE_ROUNDING_MATH_CHECK=ON

# Setting C++ standard to c++14 because older CGAL library versions
# violate c++17 (i.e. CGAL libraries older than 4.12)
CPPSTDOPTION = -std=c++14

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 
.PHONY: all
all: $(OBJECTS) $(LIBOBJ)

# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES)
endif

.PHONY: deps
deps: $(DEP_FILES)

OBJECTS += pmregion/pmregcli.o pmregion/PMRegion_conversions.o pmregion/PMRegion_coverduration.o pmregion/PMRegion_helpers.o pmregion/PMRegion_operations.o pmregion/RList.o

# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra) 
