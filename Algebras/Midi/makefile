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
# SECONDO makefile for Midi Algebra
#
# $Log$
# Revision 1.3  2005/07/21 17:39:46  spieker
# Some old code corrected, since the algebra was excluded for a long time
#
# Revision 1.2  2005/01/04 15:27:05  hoffman
# Added GPL License
#
# Revision 1.1  2004/03/01 16:07:27  dansorge
# Added new Midi algebra.
#
# Revision 1.8  2004/02/11 21:12:04  laesche
# *** empty log message ***
#
# Revision 1.7  2004/02/08 20:11:56  laesche
# first version
#
# Revision 1.6  2004/01/31 15:35:10  buettner
# FTextAlgebra is available in MidAlgebra.cpp. makefile changed.
#
# Revision 1.5  2004/01/07 23:08:06  buettner
# MidiAlgebra.cpp
#
# Revision 1.4  2004/01/04 17:16:51  buettner
# MidiAlgebra.cpp
#
# Revision 1.3  2003/12/28 22:19:01  buettner
# MidiAlgebra.h
#
# Revision 1.2  2003/12/28 19:08:53  laesche
# *** empty log message ***
#
# Revision 1.1  2003/12/23 19:10:45  laesche
# first version
#
# Revision 1.1.1.1  2003/12/12 20:17:01  soanso
# FaPra Phase2
#
# Revision 1.6  2003/11/07 15:48:24  valmeida
#
# This modification contains the new version of the Relational Algebra.
#
# There are two new algebras called Relation-C++ and ExtRelation-C++ which
# corresponds to the new Relational Algebra and the new Extended Relational
# Algebra. The Relational Algebra contains the type constructors "rel" and
# "tuple" and the main operations like feed, consume, project, product, rename,
# filter, and count. The Extended Relational Algebra has the extended
# operations such as sum, avg, loopjoin, hashjoin, sort, sortby, etc. For more
# information on these algebras one can try the pd-system documentation.
#
#   The Relational Algebra supports both the Main Memory and Persistent models.
# There is a define called "PERSISTENT_RELALG" that can be switched on and off
# for this purposes. This define can be seen in the makefiles of the Relational
# Algebra and the Extended Relational Algebra and must be set together. One
# should remember that the databases created in these two models are not
# compatible. I expect that the databases created with the Old Main Memory
# Relational Algebra are compatible with the new (Main Memory) one, but I did
# not test it. In case of problems, all databases must be re-created.
#
#   The Old Main Memory Relational Algebra is kept alive. There is an algebra
# called OldRelational-C++ that corresponds to it. The type constructors were
# renamed to "mrel" and "mtuple" and the operators still have the same name.
#
#   The Array Algebra points to the Old Relational Algebra, because it would be
# easier to the algebra implementor to make the conversion.
#
#   All type constructors that will be part of a relation must have a memory
# representation without pointers. This need was discussed in the Secondo
# Programmers Guide. In this way, the FText Algebra will not work inside
# relations, as it is implemented now. As a suggestion, one could use FLOBs to
# implement the "text" inside the FText Algebra.
#
# Revision 1.5  2003/04/17 21:04:17  telle
# Fixed makefile problems when building shared library version. (LIBNAME needs to be a target specific variable if there are more than one library to be built within one makefile.)
# Definition of variable SMILIB moved from main makefile to makefile.env.
# Changed references to variable BDBSMILIB to SMILIB in several algebra modul makefiles to make them independent of the used SMI implementation.
# FTextAlgebra.cpp: include <iostream> instead of <iostream.h> (which is deprecated)
# SpatialAlgebra.cpp: changed long string constants to avoid warnings.
#
# Revision 1.4  2003/04/03 09:35:27  valmeida
#
# Correcting a bug in the PArray organization. Now the SecondoSystem is
# responsible to opening and closing a LOB file which the PArrays will use.
#
# Revision 1.3.4.1  2003/04/08 15:39:48  valmeida
#
# This modification adds the first working version of the Persistent
# Relational Algebra. Lots of works must be done to be the final version.
#
# Revision 1.3  2002/11/27 20:31:04  spieker
# Rules for generation of libraries were revised. Many rules are written
# in a short manner by use of VPATH and automatic variables $@ and $<.
# Hopefully the makefiles are now easier to read and to maintain.
#
# Revision 1.2  2002/09/26 17:27:14  spieker
# Problems with make shared=yes, because the polygon algebra references the
# Tuplemanager and the SMI. Tuplemanager files and libsmi have been added to the
# rule which creates the shared object.
# .
#
#
########################################################################

include ../../makefile.env


# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/QueryProcessor.h \
	$(INCLUDEDIR)/StandardTypes.h \
  $(INCLUDEDIR)/DBArray.h \
  ../FText/FTextAlgebra.h

REFFILE=\

MODNAME=MidiAlgebra
LIBNAME=lib$(MODNAME)

DOCU_FILES = Midi1 MidiAbstract \
	     Midi2 MidiIntroduction \
	     Midi3 MidiAlgebra.cpp \
	     Midi4 MidiAlgebra.h \
	     Midi5 MidiAlgebra.spec

OBJECTS=\
	$(MODNAME).$(OBJEXT)

.PHONY: all
all: $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)

docu: $(DOCU_FILES)
	cat $^ > $@

MidiAlgebra2:	MidiAbstract MidiIntroduction MidiAlgebra.cpp MidiAlgebra.h MidiAlgebra.spec
	cat MidiAbstract MidiIntroduction MidiAlgebra.cpp MidiAlgebra.h MidiAlgebra.spec > MidiAlgebra2

buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS) -I../FText

#This define RELALG_PERSISTENT is used to compile the Persistent Relational Algebra.
#Uncomment this line below to change it to the Main Memory Relational Algebra.
CCFLAGS+= -DRELALG_PERSISTENT

# --- Standard Algebra C++ library ---

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) $(SMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- Standard Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)


.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo

