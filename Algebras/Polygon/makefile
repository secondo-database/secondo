########################################################################
#
# SECONDO makefile for Polygon Algebra
#
# $Log$
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
        $(INCLUDEDIR)/DBArray.h

REFFILE=\

MODNAME=PolygonAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
	$(MODNAME).$(OBJEXT)

.PHONY: all
all: $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)

buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

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

include ../../include.dep

.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo

