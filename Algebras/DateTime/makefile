########################################################################
#
# SECONDO makefile for PointRectangle Algebra C++
#
########################################################################

include ../../makefile.env

# --- List of all needed header files (including indirect dependencies)

INCFILES=\
	$(INCLUDEDIR)/Algebra.h \
	$(INCLUDEDIR)/NestedList.h \
	$(INCLUDEDIR)/QueryProcessor.h \
        $(INCLUDEDIR)/DateTime.h

MODNAME=DateTimeAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
	$(MODNAME).$(OBJEXT)

.PHONY: all
all: $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)

buildlibs: $(LIBDIR)/$(LIBNAME).$(LIBEXT)

CCFLAGS= $(PICFLAG) $(DEFAULTCCFLAGS)

# --- Date Algebra C++ library ---

$(LIBDIR)/$(LIBNAME).$(LIBEXT): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) -o $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(LDOPT) $(OBJECTS) -L$(LIBDIR) -lStandardAlgebra $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBDIR)/$(LIBNAME).$(LIBEXT) $(OBJECTS)
endif

# --- PointRectangle Algebra C++ library modules ---

$(MODNAME).$(OBJEXT): $(MODNAME).cpp $(INCFILES)
	$(CC) -c -o $(MODNAME).$(OBJEXT) $(MODNAME).cpp $(CCFLAGS)

include ../../include.dep


.PHONY:pdf
pdf: DateTimeAlgebra.pdf

DateTimeAlgebra.pdf: DateTimeAlgebra.pd.dvi
	dvipdfm -p a4 -o DateTimeAlgebra.pdf DateTimeAlgebra.pd.dvi

.PHONY:dvi
dvi: DateTimeAlgebra.pd.dvi


DateTimeAlgebra.pd.dvi: DateTimeAlgebra.pd.tex
	latex DateTimeAlgebra.pd.tex
	latex DateTimeAlgebra.pd.tex
	latex DateTimeAlgebra.pd.tex

DateTimeAlgebra.pd.tex: DateTimeAlgebra.cpp ../../include/DateTime.h
	cat DateTimeAlgebra.cpp appendix ../../include/DateTime.h >DateTimeAlgebra.pd
	pd2tex DateTimeAlgebra.pd
	$(RM) DateTimeAlgebra.pd



.PHONY: clean
clean: cleanpdf
	$(RM) $(LIBDIR)/$(LIBNAME).a $(LIBDIR)/$(LIBNAME).so $(LIBDIR)/$(LIBNAME).dll $(LIBDIR)/$(LIBNAME).dll.a
	$(RM) *.o
	$(RM) *.lo

.PHONY: cleanpdf
cleanpdf:
	$(RM) *.toc *.log *.tex *.dvi *.aux *.pdf
