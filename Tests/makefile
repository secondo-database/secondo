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
# SECONDO makefile for the Storage Management Interface
#
########################################################################

include ../makefile.env
include ../makefile.options

# Add the base name of your .cpp file here. The deafult rules should
# compile and link it, otherwise add special rules

# Nested List and Compact Tables
TESTNAMES += tctable tpctable tnestedlist

# Some Tools
TESTNAMES += tfilesystem tprofiles

# Storage Mangement Interface and Berkeley-DB
TESTNAMES += tsmi tsmi3 bdbtest bdb-perf

# Socket Communication
TESTNAMES += tsock1 tsock2 tsock3

# Dynamic Library Module 
TESTNAMES += tdlmain

# Tool for Log Messages 
TESTNAMES += itlogmsg

# Persistent Array
#TESTNAMES += tparray

# Prefetch Iterator
#TESTNAMES += tprefetch


# Base64 encoding
TESTNAMES += base64encoder

# Nested List: Parsing lists and converting from text to binary format
TESTNAMES += nlchecker 

# Raw BDB test
TESTNAMES += testRawBDB 

ifdef file
TESTFILES := $(file)
else
TESTFILES := $(TESTNAMES)
endif

CODE_FILES := $(addsuffix .cpp, $(TESTFILES))
#CODE_DEPENDENCIES := $(addsuffix .dep, $(TESTFILES))

OBJECTS := $(addsuffix $(EXEEXT), $(TESTFILES))
ifndef file
OBJECTS += libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT)
endif

CCFLAGS := -I../StorageManager -I../Algebras/TupleIdentifier $(DEFAULTCCFLAGS) $(myflags)
CCFLAGS += -std=c++17
LIBS := ../UserInterfaces/cmsg.o $(LD_LINK_LIBS)

.PHONY: all 
all:  $(OBJECTS)


# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< > $@


%.o: %.cpp
	$(CC) -c -o $@ $(CCFLAGS) $< 

# default rule for linking
%$(EXEEXT): %.o
	$(CC) $(EXEFLAGS) -o $@ $< $(LIBS) $(COMMON_LD_FLAGS)

# Test Dynamic Library module

# ... Windows needs special treatment when creating DLLs
ifeq ($(platform),win32)
LDOPT1 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib1.$(DLLEXT).a
LDOPT2 = -Wl,--export-dynamic -Wl,--out-implib,libtdynlib2.$(DLLEXT).a
endif

libtdynlib1.$(DLLEXT) libtdynlib2.$(DLLEXT): tdlmain.o tdynlib1.$(DLLOBJEXT) tdynlib2.$(DLLOBJEXT) libsdbnl.$(LIBEXT)
	$(CC) $(DLLFLAGS) -o libtdynlib1.$(DLLEXT) $(LDOPT1) tdynlib1.$(DLLOBJEXT) -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)
	$(CC) $(DLLFLAGS) -o libtdynlib2.$(DLLEXT) $(LDOPT2) tdynlib2.$(DLLOBJEXT) -L. -ltdynlib1 -L$(LIBDIR) $(TOOLLIB) $(DEFAULTLIB)


tdynlib1.$(DLLOBJEXT): tdynlib1.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib1.$(DLLOBJEXT) tdynlib1.cpp $(CCFLAGS)

tdynlib2.$(DLLOBJEXT): tdynlib2.cpp
	$(CC) -c $(DLLPICFLAG) -o tdynlib2.$(DLLOBJEXT) tdynlib2.cpp $(CCFLAGS)

	
ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_FILES:.cpp=.dep)
endif

.PHONY: clean
clean:
	$(RM) *.dep *.$(OBJEXT) $(OBJECTS) $(addprefix $(BINDIR)/,$(OBJECTS))
	$(RM) tdynlib1.$(DLLOBJEXT) tdynlib2.$(DLLOBJEXT)
