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

MODNAME=FuzzyAlgebra
LIBNAME=lib$(MODNAME)

OBJECTS=\
	$(MODNAME).$(OBJEXT)


JAVA_FILES_TMP=$(shell find fuzzyobjects -name "*.java")
JAVA_FILES1=$(subst fuzzyobjects/,,$(JAVA_FILES_TMP))
JAVA_FILES=$(subst .java,,$(JAVA_FILES1))

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


.PHONY: pdf
pdf: FuzzyAlgebra.pdf


FuzzyAlgebra.pdf: FuzzyAlgebra.pd.dvi
	dvipdfm -p a4 -o FuzzyAlgebra.pdf FuzzyAlgebra.pd.dvi


.PHONY: dvi
dvi: FuzzyAlgebra.pd.dvi

FuzzyAlgebra.pd.dvi: FuzzyAlgebra.pd.tex
	latex FuzzyAlgebra.pd.tex
	latex FuzzyAlgebra.pd.tex
	latex FuzzyAlgebra.pd.tex

FuzzyAlgebra.pd.tex: FuzzyAlgebra.cpp Appendix.pd
	pd2tex FuzzyAlgebra.cpp
	grep -v "\end{document}" FuzzyAlgebra.cpp.tex > FuzzyAlgebra.cpp.tex.tmp
	cat FuzzyAlgebra.cpp.tex.tmp Appendix.pd end >FuzzyAlgebra.pd.tex
	rm FuzzyAlgebra.cpp.tex.tmp
	rm FuzzyAlgebra.cpp.tex

Appendix.pd: $(JAVA_FILES_TMP)
	sh createAppendix $(JAVA_FILES) >Appendix.pd


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
	$(RM) *.tmp
	$(RM) *.pdf *.dvi *.log *.aux *.pd *.tex *.toc




