########################################################################
#
# SECONDO makefile for JPEG Algebra C++
#
########################################################################

include ../../makefile.env

# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/QueryProcessor.h  

MODNAME=JPEGAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
	$(MODNAME).$(OBJEXT) JPEGSourceMgr.$(OBJEXT)

.PHONY: all
all: $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS) 

buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# --- JPEG Algebra C++ library ---

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) -L$(LIBDIR) -lStandardAlgebra $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- JPEG Algebra C++ library modules ---

JPEGSourceMgr.$(OBJEXT): JPEGSourceMgr.cpp $(INCFILES)
	$(CC) -c -o JPEGSourceMgr.$(OBJEXT) JPEGSourceMgr.cpp $(CCFLAGS)
$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

include ../../include.dep

.PHONY: docu
docu: JPEGAlgebraDocu.pd

DOCU_FILES = \
	introduction.pd \
        JPEGAlgebra.h \
	newpage.pd \
	JPEGAlgebra.cpp \
	newpage.pd \
	JPEGSourceMgr.h \
	newpage.pd \
	JPEGSourceMgr.cpp \
	sectionpage.pd \
	jdatasrcX.c \
	newpage.pd \
	jdatadstX.c \
	appendixpage.pd 

JPEGAlgebraDocu.pd: $(DOCU_FILES)
	cat $^ > $@

.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo

