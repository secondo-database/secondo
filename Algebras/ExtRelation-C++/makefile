########################################################################
#
# SECONDO makefile for Relation Algebra C++
#
########################################################################

include ../../makefile.env

MODNAME = ExtRelationAlgebra

CCFLAGS = $(DEFAULTCCFLAGS) -I../Relation-C++

# define the compiler flag -DRELALG_PERSISTENT in the file 
# makefile.env to compile the Persistent Relational Algebra.
# It must be defined on a higher level since other algebras have
# to interoperate with the relational algebra.

.PHONY: all
all: $(OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

# --- Relation Algebra C++ library ---

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)

