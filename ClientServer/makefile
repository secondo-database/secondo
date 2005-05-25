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

LIBDIROBJECTS := $(LIBDIR)/SecondoInterfaceCS.$(OBJEXT)

APPLICATIONS =\
	$(BINDIR)/SecondoRegistrar$(EXEEXT) \
	$(BINDIR)/SecondoListener$(EXEEXT) \
	$(BINDIR)/SecondoMonitor$(SMIUP)$(EXEEXT) \
	$(BINDIR)/SecondoServer$(SMIUP)$(EXEEXT)

ifneq ($(platform),win32)
DEP_FILES := $(filter-out Win32Socket%, $(DEP_FILES))
else
DEP_FILES := $(filter-out UnixSocket%, $(DEP_FILES))
endif

ifneq ($(smitype),ora)
APPLICATIONS += $(BINDIR)/SecondoCheckpoint$(EXEEXT)
endif

CCFLAGS := $(DEFAULTCCFLAGS) $(JNIINCLUDE)

.PHONY: all
all: $(OBJECTS) $(LIBDIROBJECTS)

.PHONY: buildapp
buildapp: $(APPLICATIONS)

$(LIBDIR)/%.$(OBJEXT): %.$(OBJEXT)
	$(CP) $< $@


# --- Link applications
SERVER_OBJECTS = SecondoInterface.o SecondoInterfaceGeneral.o AlgebraList.o

$(BINDIR)/SecondoMonitorBDB$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoMonitorBDB$(EXEEXT): SecondoMonitor.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoRegistrar$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoRegistrar$(EXEEXT): SecondoRegistrar.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoCheckpoint$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoCheckpoint$(EXEEXT): SecondoCheckpoint.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoListener$(EXEEXT): $(SECLIBFILES)
$(BINDIR)/SecondoListener$(EXEEXT): SecondoListener.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)

$(BINDIR)/SecondoServerBDB$(EXEEXT): $(ALGLIBFILES) $(SECLIBFILES) $(JNILIBFILES) 
$(BINDIR)/SecondoServerBDB$(EXEEXT): SecondoServer.o $(SERVER_OBJECTS)
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o,$^) $(LD_LINK_LIBS_ALG)

# Oracle SMI
$(BINDIR)/SecondoMonitorORA$(EXEEXT): $(SECLIBFILES) 
$(BINDIR)/SecondoMonitorORA$(EXEEXT): SecondoMonitor.o
	$(LD) $(EXEFLAGS) $(LDFLAGS) $(PROFILERFLAGS) -o $@ $< $(LD_LINK_LIBS)


$(BINDIR)/SecondoServerORA$(EXEEXT): $(ALGLIBFILES) $(SECLIBFILES) 
$(BINDIR)/SecondoServerORA$(EXEEXT): SecondoServer.o $(SERVER_OBJECTS) 
	$(LD) $(EXEFLAGS) $(LDLFAGS) $(LDJNIFLAGS) $(PROFILERFLAGS) -o $@ $(filter %.o,$^) $(LD_LINK_LIBS_ALG)


include $(DEP_FILES)

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBDIROBJECTS) $(APPLICATIONS)

