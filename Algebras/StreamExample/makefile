########################################################################
#
# SECONDO makefile for StreamExample Algebra
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/QueryProcessor.h

LIBNAME=libstreamexamplealgebra
MODNAME=StreamExampleAlgebra

OBJECTS=\
	$(MODNAME).$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# --- ... Algebra C++ library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPT = -Wl,--export-dynamic -Wl,--out-implib,$(LIBDIR)/$(LIBNAME).$(LIBEXT).a
endif
endif

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) -L$(LIBDIR) -lstandardalgebra $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(BINDIR)/$(LIBNAME).$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- ... Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

include ../../include.dep

.PHONY: clean
clean:
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

