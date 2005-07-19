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

include ../../makefile.env

INCLUDEFLAGS = -I. -I$(JNI_INCLUDE) -I$(JNI_MD_INCLUDE) $(PLINCLUDEFLAGS) -I$(INCLUDEDIR)

LINKFILES = $(JPL_LIB)/jpl.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o

# Some optional switches used for providing a prolog
# predicate which maximizes the entropy. For details
# refer to "SecondoPL.cpp" and the subdirectory "Optimizer/Entropy"
ifdef ENTROPY

include ../../Optimizer/Entropy/makefile.optlib

LINKFILES += ../../Optimizer/Entropy/entropy.$(OBJEXT)
ENT_LINK_LIBS = -L$(OPTLIB_BUILD) -lopt-$(TARGET_NAME) -lnewmat-$(TARGET_NAME)
CCFLAGS += -DSECONDO_USE_ENTROPY

endif

.PHONY: all
all: compile build

.PHONY:compile
compile: $(JPL_LIB)/jpl.o

.PHONY:build
build: $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT)


$(JPL_LIB)/jpl.o:
	$(CC) -c -fPIC -g -o $(JPL_LIB)/jpl.o $(INCLUDEFLAGS) jpl_fli_Prolog.c

$(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT):
	$(CC) -shared $(EXEFLAGS) $(LDFLAGS) -Wl,-soname,$(SHARED_PREF)jpl.$(SHARED_EXT)\
              -o $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT) $(LINKFILES) $(ENT_LINK_LIBS) $(LD_LINK_LIBS) $(PLLDFLAGS) $(JPLDEF)

.PHONY:clean
clean:
	rm -f jpl_fli_Prolog.h
	rm -f $(OPTDIR)/$(SHARED_PREF)jpl.$(SHARED_EXT)
