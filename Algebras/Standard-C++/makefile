########################################################################
#
# SECONDO makefile for Standard Algebra C++
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
	$(INCLUDEDIR)/QueryProcessor.h \
	$(INCLUDEDIR)/StandardTypes.h

LIBNAME=libstandardalgebra
MODNAME=StandardAlgebra

OBJECTS=\
	$(MODNAME).$(OBJEXT)

.PHONY: all
all: $(OBJECTS)

buildlibs: ../../lib/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# --- Standard Algebra C++ library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPT = -Wl,--export-dynamic -Wl,--out-implib,../../lib/$(LIBNAME).$(LIBEXT).a
endif
endif

../../lib/libstandardalgebra.$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o ../../lib/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) $(LIBDIR) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
ifeq ($(platform),win32)
	$(CP) ../../lib/$(LIBNAME).$(LIBEXT) ../../bin/$(LIBNAME).$(LIBEXT)
endif
else
# ... as static library
	$(AR) -r $(LIBNAME).$(LIBEXT) $(OBJECTS)
	$(CP) $(LIBNAME).$(LIBEXT) ../../lib/$(LIBNAME).$(LIBEXT)
endif

# --- Standard Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

include ../../include.dep

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) *.lo
	$(RM) *.a
	$(RM) *.so
	$(RM) *.dll

else
# Platform not specified
all:
	@echo *** Platform not specified in makefile.config
endif

