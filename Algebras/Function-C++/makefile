########################################################################
#
# SECONDO makefile for Function Algebra C++
#
########################################################################

include ../../makefile.config
ifdef platform
include ../../makefile.$(platform)
.SUFFIXES: .exe .res .a .o .c .cpp .cc .cxx .m .rc .p .f .F .r .y .l .s .S .def .h

# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	../../include/Algebra.h \
	../../include/AlgebraManager.h \
	../../include/NestedList.h \
	../../include/CTable.h \
	../../include/CTable.cpp \
	../../include/QueryProcessor.h \
	../../include/SecondoCatalog.h

LIBNAME=libfunctionalgebra
MODNAME=FunctionAlgebra

OBJECTS=\
	$(MODNAME).$(OBJEXT)

.PHONY: all
all: $(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# --- Function Algebra C++ library ---

# ... Windows needs special treatment when creating DLLs
ifeq ($(shared),yes)
ifeq ($(platform),win32)
LDOPT = -Wl,--export-dynamic -Wl,--out-implib,$(LIBNAME).$(LIBEXT).a
endif
endif

$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) $(DEFAULTLIB)
	$(CP) $(LIBNAME).$(LIBEXT) ../../lib/$(LIBNAME).$(LIBEXT)
ifeq ($(platform),win32)
	$(CP) $(LIBNAME).$(LIBEXT) ../../bin/$(LIBNAME).$(LIBEXT)
	$(CP) $(LIBNAME).$(LIBEXT).a ../../lib/$(LIBNAME).$(LIBEXT).a
endif
else
# ... as static library
	$(AR) -r $(LIBNAME).$(LIBEXT) $(OBJECTS)
	$(CP) $(LIBNAME).$(LIBEXT) ../../lib/$(LIBNAME).$(LIBEXT)
endif

# --- Function Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

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

