########################################################################
#
# SECONDO makefile for BTree Algebra
#
########################################################################

include ../../makefile.env

MODNAME = BTreeAlgebra
CCFLAGS = $(DEFAULTCCFLAGS) -I../Relation-C++

.PHONY: all
all: $(OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra -lRelationAlgebra $(SMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)
