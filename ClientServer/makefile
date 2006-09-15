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
# SECONDO makefile for client server modules 
#
########################################################################

include ../makefile.env

LD := g++

LIBDIROBJECTS := $(LIBDIR)/SecondoInterfaceCS.$(OBJEXT)

APPLICATIONS =\
	$(BINDIR)/SecondoRegistrar$(EXEEXT) \
	$(BINDIR)/SecondoListener$(EXEEXT) \
	$(BINDIR)/SecondoMonitor$(EXEEXT) \
	$(BINDIR)/SecondoCheckpoint$(EXEEXT)

ifneq ($(platform),win32)
DEP_FILES := $(filter-out Win32Socket%, $(DEP_FILES))
else
DEP_FILES := $(filter-out UnixSocket%, $(DEP_FILES))
endif


.PHONY: all
all: $(OBJECTS) $(LIBDIROBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS)

$(LIBDIR)/%.$(OBJEXT): %.$(OBJEXT)
	$(CP) $< $@


# --- Link applications

CMSG := ../UserInterfaces/cmsg.o

define link-app
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $^ $(LD_LINK_LIBS)
endef

$(BINDIR)/SecondoMonitor$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoMonitor$(EXEEXT): $(CMSG) SecondoMonitor.o
	$(link-app)

$(BINDIR)/SecondoRegistrar$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoRegistrar$(EXEEXT): $(CMSG) SecondoRegistrar.o
	$(link-app)

$(BINDIR)/SecondoCheckpoint$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoCheckpoint$(EXEEXT): $(CMSG) SecondoCheckpoint.o
	$(link-app)

$(BINDIR)/SecondoListener$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoListener$(EXEEXT): $(CMSG) SecondoListener.o
	$(link-app)

ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBDIROBJECTS) $(APPLICATIONS)

