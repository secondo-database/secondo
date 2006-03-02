########################################################################
#
# SECONDO makefile for PointRectangle Algebra C++
#
########################################################################

include ../../makefile.env



MODNAME = PeriodicAlgebra
CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS) -I../Spatial -I../Rectangle -I../Temporal

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
clean: cleanpdf
	$(RM) $(DEP_FILES) $(OBJECTS) $(LIBOBJ)
	make -f makefile.tree clean

.PHONY: cleanpdf
cleanpdf:
	$(RM) -f PeriodicAlgebra.doc* PeriodicAlgebra.dvi PeriodicAlgebra.pdf *.toc *.log
	make -C Figures clean

#.PHONY: tree
#	make -f makefile.tree


.PHONY: pdf
pdf: PeriodicAlgebra.pdf

PeriodicAlgebra.pdf: PeriodicAlgebra.doc.dvi
	dvipdfm -p a4 -o PeriodicAlgebra.pdf PeriodicAlgebra.doc.dvi

PeriodicAlgebra.doc.dvi: PeriodicAlgebra.doc.tex
	latex $<
	latex $<
	latex $<

PeriodicAlgebra.doc.tex: PeriodicAlgebra.doc.tex1
	sed -e 's/\\begin{verbatim}/\\begin{lstlisting}/g' -e 's/\\end{verbatim}/\\end{lstlisting}/g' $< >PeriodicAlgebra.doc.tex

PeriodicAlgebra.doc.tex1: PeriodicAlgebra.doc Figures/ObviousSolution.eps Figures/UniLogo.eps Figures/MRealFormula.eps Figures/SecondoLogo.eps pdheader
	PD_HEADER=pdheader pd2tex PeriodicAlgebra.doc	
	mv PeriodicAlgebra.doc.tex PeriodicAlgebra.doc.tex1


%eps:
	make -C Figures 

PeriodicAlgebra.doc: PeriodicAlgebra.cpp List2.h RepTree.h RepTree.cpp NodeTypes.h  makefile
	cat PeriodicAlgebra.cpp > PeriodicAlgebra.doc
	cat List2.h >> PeriodicAlgebra.doc
	cat RepTree.h >> PeriodicAlgebra.doc
	cat RepTree.cpp >> PeriodicAlgebra.doc
	cat NodeTypes.h >> PeriodicAlgebra.doc
	

