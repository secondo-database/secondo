########################################################################
#
# SECONDO makefile for fuzzy Algebra
#
# 06-2003 
#
########################################################################

include ../../makefile.env

# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/QueryProcessor.h \
	$(INCLUDEDIR)/StandardTypes.h

REFFILE=\

MODNAME=RoseAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
	$(MODNAME).$(OBJEXT)


.PHONY: all
all: alljava allcpp

.PHONY: alljava
alljava:
	make -f makejava all

.PHONY: allcpp
allcpp: $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)

buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# --- Standard Algebra C++ library ---

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) $(LDJNIFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- Standard Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS) $(JNIINCLUDE) $(LDFLAGS)

include ../../include.dep


.PHONY: clean
clean:
	$(MAKE) -f makejava clean
	$(RM) $(LIBDIR)/$(LIBNAME).a 
	$(RM) $(LIBDIR)/$(LIBNAME).so 
	$(RM) $(LIBDIR)/$(LIBNAME).dll 
	$(RM) $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.class
	$(RM) *~




