########################################################################
#
# SECONDO makefile for Relation Algebra C++
#
########################################################################

include ../../makefile.env

# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/QueryProcessor.h \
	$(INCLUDEDIR)/StandardTypes.h \
	$(INCLUDEDIR)/FLOB.h \
	$(INCLUDEDIR)/DBArray.h \
        ../OldRelation-C++/OldRelationAlgebra.h \
	RelationAlgebra.h

MODNAME=RelationAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
	FLOB.$(OBJEXT) \
	RelationCommon.$(OBJEXT) \
	RelationPersistent.$(OBJEXT) \
	RelationMainMemory.$(OBJEXT) \
	$(MODNAME).$(OBJEXT) 

CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))

.PHONY: all
all: $(CODE_DEPENDENCIES) $(LIBDIR)/$(LIBNAME).$(LIBEXT)

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< -o $@



.PHONY: buildlibs
buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS) -I../OldRelation-C++

# define the compiler flag -DRELALG_PERSISTENT in the file 
# makefile.env to compile the Persistent Relational Algebra.
# It must be defined on a higher level since other algebras have
# to interoperate with the relational algebra.

#The define _PREFETCHING_ is used to use Prefetching Iterators instead or normal
#ones and is used only for testing purposes. It will be removed in later versions.
CCFLAGS+= -D_PREFETCHING_

# --- Relation Algebra C++ library ---

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra $(SMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- Relation Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

FLOB.$(OBJEXT): FLOB.cpp  $(INCFILES)
	$(CC) -c -o FLOB.$(OBJEXT) FLOB.cpp $(CCFLAGS)

RelationCommon.$(OBJEXT): RelationCommon.cpp  $(INCFILES)
	$(CC) -c -o RelationCommon.$(OBJEXT) RelationCommon.cpp $(CCFLAGS)

RelationPersistent.$(OBJEXT): RelationPersistent.cpp  $(INCFILES)
	$(CC) -c -o RelationPersistent.$(OBJEXT) RelationPersistent.cpp $(CCFLAGS)

RelationMainMemory.$(OBJEXT): RelationMainMemory.cpp  $(INCFILES)
	$(CC) -c -o RelationMainMemory.$(OBJEXT) RelationMainMemory.cpp $(CCFLAGS)

include ../../include.dep
include $(CODE_DEPENDENCIES:.cpp=.dep)


.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a *.o *.lo

