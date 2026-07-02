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
# SECONDO makefile for query processor, catalog, system and interface
#
########################################################################

include ../makefile.env

DEFAULTCCFLAGS+=-Werror 

LIBOBJECTS=\
	$(LIBDIR)/SecondoInterfaceTTY.$(OBJEXT) \
	$(LIBDIR)/SecondoInterfaceGeneral.$(OBJEXT)

.PHONY: all
all: $(OBJECTS) $(LIBOBJECTS)

CCFLAGS += $(PICFLAG) $(DEFAULTCCFLAGS) -I../Algebras/Relation-C++ -I../Algebras/TupleIdentifier $(PLINCLUDEFLAGS)

# Anchor the objects that are copied into $(LIBDIR) as current-directory
# targets.  VPATH contains $(LIBDIR), so without an explicit prerequisite
# make conflates the local X.o with the $(LIBDIR)/X.o copy target below,
# reports a false "circular dependency" and leaves the object unbuilt.
# The generated %.dep files used to provide this anchor; with -MMD they do
# not exist yet on a clean build, so anchor these objects explicitly.
$(notdir $(LIBOBJECTS)): %.$(OBJEXT): %.cpp

$(LIBDIR)/%.$(OBJEXT): %.$(OBJEXT)
	$(CP) $< $@ 

.PHONY: clean
clean: 
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJECTS)

