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

.PHONY: all
all: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

.PHONY: buildlibs
buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS) -I../Relation-C++

#This define RELALG_PERSISTENT is used to compile the Persistent Relational Algebra.
#Uncomment this line below to change it to the Main Memory Relational Algebra.
CCFLAGS+= -DRELALG_PERSISTENT

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

.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a *.o *.lo

