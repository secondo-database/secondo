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
#CCFLAGS += -Werror

# librip/ ships its own standalone makefiles that do not include makefile.env.
# Forward the SECONDO C++ compiler (so ccache and the configured compiler are
# used, not a bare g++) and, on macOS, the Homebrew include path, since Boost
# is not on a default search path there.
RIP_MAKE_ARGS := CC="$(CPPC)" CXX="$(CPPC)"
ifeq ($(platform),mac_osx)
RIP_MAKE_ARGS += EXTRA_CPPFLAGS="-I$(HOMEBREW_PREFIX)/include"
endif

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 
.PHONY: all
all: $(OBJECTS) $(LIBOBJ)
	cp -n librip/example/matchFaces.lua ../../bin || true
	cp -n librip/example/matchStrategies.lua ../../bin || true

librip/lib/librip.a:
	$(MAKE) -C librip $(RIP_MAKE_ARGS)

# Create the algebra library from the algebra objects plus librip's objects.
# Do NOT use $(create-lib) here: it archives $^, which includes the librip.a
# prerequisite, nesting an archive inside the .a
$(LIBOBJ): $(OBJECTS) librip/lib/librip.a
	$(MAKE) -C librip $(RIP_MAKE_ARGS)
	$(AR) -rs $@ $(OBJECTS) librip/lib/*.o

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)
	$(MAKE) -C librip clean
