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

########################################################################
#
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.env

BDBOBJECTSBASENAMES :=\
	smiEnvironment \
	smiKey \
	bdbKeyedFile \
	bdbRecordFile \
	bdbFile \
	bdbRecord \
	bdbEnvironment

ORAOBJECTSBASENAMES :=\
	smiEnvironment \
	smiKey \
	oraKeyedFile \
	oraRecordFile \
	oraFile \
	oraRecord \
	oraEnvironment

BDBOBJECTS := $(addsuffix .$(OBJEXT), $(BDBOBJECTSBASENAMES)) 
ORAOBJECTS := $(addsuffix .$(OBJEXT), $(ORAOBJECTSBASENAMES))


ifeq ($(smitype),ora)
CCFLAGS := $(PICFLAG) $(OCICPPINCLUDE) $(DEFAULTCCFLAGS)
ALLOBJ := $(ORAOBJECTS)
else
CCFLAGS := $(PICFLAG) $(DEFAULTCCFLAGS)
ALLOBJ := $(BDBOBJECTS)
endif

CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(ALLOBJ))


.PHONY: all
all: $(CODE_DEPENDENCIES) $(ALLOBJ)

LIBNAME = libsmi$(SMILOW)
.PHONY: buildlibs
buildlibs: $(LIBNAME).$(LIBEXT)

# --- Storage Management Interface library ---

# Windows needs special treatment for dynamic libraries!
# The variable LIBNAME will be used in the LDOPTTOOL variable 
# which is only defined makefile.win32 included by makefile.env.

$(LIBDIR)/libsmibdb.$(LIBEXT): $(BDBOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $@ $(LDOPT) $(BDBOBJECTS) -L$(BDBDIR)/lib $(BDBLIB) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $@ $(BDBOBJECTS)
endif

$(LIBDIR)/libsmiora.$(LIBEXT): $(ORAOBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $@ $(LDOPT) $(ORAOBJECTS) $(OCICPPLIBDIR) $(OCICPPLIB) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $@ $(ORAOBJECTS)
endif

# --- Storage Management Interface modules ---

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@

%.$(OBJEXT): %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) *.dep $(ALLOBJ) $(LIBDIR)/$(LIBNAME).$(LIBEXT)
