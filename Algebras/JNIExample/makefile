########################################################################
#
# SECONDO makefile for fuzzy Algebra
#
# 06-2003 
#
########################################################################

include ../../makefile.env

MODNAME = JNIExampleAlgebra
CCFLAGS = $(DEFAULTCCFLAGS) $(JNIINCLUDE) $(LDFLAGS)

.PHONY: all
all: alljava allcpp

.PHONY: alljava
alljava:
	make -f makejava all

.PHONY: allcpp
allcpp: $(OBJECTS) $(LIBOBJ)

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) $(LDJNIFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif


.PHONY: clean
clean:
	$(MAKE) -f makejava clean
	$(RM) $(OBJECTS) $(DEP_FILES) $(LIBOBJ)
	$(RM) *.class *~
