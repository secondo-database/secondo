########################################################################
#
# SECONDO makefile for PointRectangle Algebra C++
#
########################################################################

include ../../makefile.env

MODNAME = DateTimeAlgebra
CCFLAGS = $(DEFAULTCCFLAGS)

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
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)

.PHONY: cleanpdf
cleanpdf:
	$(RM) *.toc *.log *.tex *.dvi *.aux *.pdf
