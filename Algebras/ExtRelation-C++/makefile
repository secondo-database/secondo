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
	$(INCLUDEDIR)/CPUTimeMeasurer.h \
	../Relation-C++/RelationAlgebra.h 

MODNAME=ExtRelationAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
  ExtRelAlgPersistent.$(OBJEXT) \
  ExtRelAlgMainMemory.$(OBJEXT) \
	$(MODNAME).$(OBJEXT) 

CODE_DEPENDENCIES = $(patsubst %.$(OBJEXT), %.dep, $(OBJECTS))

.PHONY: all
all: $(CODE_DEPENDENCIES) $(LIBDIR)/$(LIBNAME).$(LIBEXT)

# create dependencies from include directives
%.dep: %.cpp
	$(CC) -MM $(CCFLAGS) $< -o $@

.PHONY: buildlibs
buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS) -I../Relation-C++

# define the compiler flag -DRELALG_PERSISTENT in the file 
# makefile.env to compile the Persistent Relational Algebra.
# It must be defined on a higher level since other algebras have
# to interoperate with the relational algebra.

# --- Relation Algebra C++ library ---

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- Relation Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES) 
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

ExtRelAlgPersistent.$(OBJEXT): ExtRelAlgPersistent.cpp  $(INCFILES)
	$(CC) -c -o $@ $< $(CCFLAGS)

ExtRelAlgMainMemory.$(OBJEXT): ExtRelAlgMainMemory.cpp  $(INCFILES)
	$(CC) -c -o $@ $< $(CCFLAGS)


include ../../include.dep
include $(CODE_DEPENDENCIES:.cpp=.dep)

.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a *.o *.lo

