########################################################################
#
# SECONDO makefile for Relation Algebra C++
#
########################################################################

include ../../makefile.env

MODNAME = RelationAlgebra
CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS) -I../OldRelation-C++

# define the compiler flag -DRELALG_PERSISTENT in the file 
# makefile.env to compile the Persistent Relational Algebra.
# It must be defined on a higher level since other algebras have
# to interoperate with the relational algebra.

#The define _PREFETCHING_ is used to use Prefetching Iterators instead or normal
#ones and is used only for testing purposes. It will be removed in later versions.
CCFLAGS += -D_PREFETCHING_

.PHONY: all
all: $($OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

# --- Relation Algebra C++ library ---

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra $(SMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)

