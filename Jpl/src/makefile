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
include ../../makefile.jni

INCLUDEFLAGS := -I. $(JNIINCLUDE) $(PLINCLUDEFLAGS) -I$(INCLUDEDIR)

CCOBJECTS := $(JPL_LIB)/jpl.o $(JPL_LIB)/regSecondo.o
OBJECTS := $(CCOBJECTS)
ifeq ($(platform),win32)
  DEF += jpl.def
  OBJECTS += $(DEF)
endif

JNIDLL := $(OPTDIR)/$(DLLPREF)jpl.$(JNI_DLLEXT)

LD := g++

LINKFILES := $(SECONDO_BUILD_DIR)/UserInterfaces/cmsg.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o

# Some optional switches used for providing a prolog
# predicate which maximizes the entropy. For details
# refer to "SecondoPL.cpp" and the subdirectory "Optimizer/Entropy"

ifdef ENTROPY
  LINKFILES += $(OPTDIR)/Entropy/Iterative_scaling.o
endif

.PHONY: all
all: compile build

.PHONY:compile
compile: $(OBJECTS)

.PHONY:build
build: $(JNIDLL)

$(JPL_LIB)/jpl.o: jpl_fli_Prolog.c
	$(CC) -c -fPIC -g -o $@ $(INCLUDEFLAGS) $<

$(JPL_LIB)/regSecondo.o: regSecondo.c
	$(CC) -c -fPIC -g -o $@ $(INCLUDEFLAGS) $<

LINK_FLAGS := $(ENT_LINK_LIBS) $(LD_LINK_LIBS) \
              $(JNI_CCFLAGS) $(PLLDFLAGS) $(JPLDEF)
ifeq ($(platform),win32)
$(JNIDLL): $(CCOBJECTS) $(LINKFILES)
	$(CC) $(DLLFLAGS) -Wl,-soname,jpl.$(JNI_DLLEXT) -o $@  \
	$^ $(LINK_FLAGS) jpl.def

jpl.def: $(JPL_LIB)/jpl.o $(JPL_LIB)/regSecondo.o
	dlltool -z $@.tmp $^
	sed -e "s#\(.*\)@\(.*\)@.*#\1 = \1@\2#g" $@.tmp > $@
	rm $@.tmp

else
$(JNIDLL): $(CCOBJECTS) $(LINKFILES)
	$(LD) $(DLLFLAGS) $(EXEFLAGS) $(LDFLAGS) -o $@ $^ $(LINK_FLAGS)
		
endif

.PHONY:clean
clean:
	rm -f $(OBJECTS) $(JNIDLL)
