########################################################################
#
# SECONDO makefile for RTree Algebra
#
########################################################################

include ../../makefile.env

MODNAME = PlugJoinAlgebra
CCFLAGS = $(DEFAULTCCFLAGS) -I../Relation-C++ -I../Spatial -I../Rectangle

.PHONY: all
all: $(OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBOBJ) $(TUPLEMANAGER) -L$(LIBDIR) -lStandardAlgebra -lRelationAlgebra $(SMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif

.PHONY: clean
clean:
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)

