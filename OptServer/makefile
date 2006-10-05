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

include ../makefile.env
include ../makefile.optimizer
include ../makefile.jni

INCLUDEFLAGS := -I. $(JNIINCLUDE) $(PLINCLUDEFLAGS) -I$(INCLUDEDIR)

REG_SEC_DLL=$(OPTDIR)/$(DLLPREF)regSecondo.$(JNI_DLLEXT)

ifndef JPL_DLL
JPL_DLL=$(OPTDIR)/$(DLLPREF)jpl.$(JNI_DLLEXT)
endif


LINKFILES := $(SECONDO_BUILD_DIR)/UserInterfaces/cmsg.o $(SECONDOPL_DIR)/SecondoPL.o $(LIBDIR)/SecondoInterfaceCS.o $(LIBDIR)/SecondoInterfaceGeneral.o $(JPL_DLL)

# Some optional switches used for providing a prolog
# predicate which maximizes the entropy. For details
# refer to "SecondoPL.cpp" and the subdirectory "Optimizer/Entropy"

ifdef ENTROPY
  LINKFILES += $(OPTDIR)/Entropy/Iterative_scaling.o
endif

LINK_FLAGS := $(ENT_LINK_LIBS) $(LD_LINK_LIBS) \
              $(JNI_CCFLAGS) 



.PHONY:all
all: jsrc $(REG_SEC_DLL)

jsrc:
	make -C $(JPLVER) all

regSecondo.o: regSecondo.c
	$(CC) -c -fPIC -g -o $@ $(INCLUDEFLAGS) $<

ifeq ($(platform),win32)
LINKFILES += $(PL_DLL)

$(REG_SEC_DLL): regSecondo.o $(LINKFILES) regSeg.def
	$(CC) $(DLLFLAGS) -Wl,-soname,jpl.$(JNI_DLLEXT) -o $@  \
	$^ $(LINK_FLAGS)  regSeg.def

regSeg.def: regSecondo.o
	dlltool -z $@.tmp $^
	sed -e "s#\(.*\)@\(.*\)@.*#\1 = \1@\2#g" $@.tmp > $@
	rm $@.tmp

else
$(REG_SEC_DLL): regSecondo.o $(LINKFILES)
	$(LD) $(DLLFLAGS) $(EXEFLAGS) $(LDFLAGS) -o $@ $^ $(LINK_FLAGS)
endif

.PHONY:clean
clean:
	make -C 10 clean
	make -C 30 clean 
	rm -f regSecondo.o
	rm -f $(REG_SEC_DLL)

