#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

########################################################################
#
# SECONDO makefile for fuzzy Algebra
#
# 06-2003
#
########################################################################

include ../../makefile.env

MODNAME = FuzzyAlgebra
CCFLAGS = $(DEFAULTCCFLAGS) $(JNIINCLUDE) $(LDFLAGS)

JAVA_FILES_TMP=$(shell find fuzzyobjects -name "*.java")
JAVA_FILES1=$(subst fuzzyobjects/,,$(JAVA_FILES_TMP))
JAVA_FILES=$(subst .java,,$(JAVA_FILES1))

.PHONY: all
all: alljava allcpp

.PHONY: alljava
alljava:
	make -f makejava all

.PHONY: allcpp
allcpp: $(OBJECTS) $(LIBOBJ)

include $(DEP_FILES)

$(LIBOBJ): $(OBJECTS)
ifeq ($(shared),yes)
# ... as shared object
	$(LD) $(LDFLAGS) $(LDJNIFLAGS) -o $(LIBOBJ) $(LDOPT) $(OBJECTS) $(TUPLEMANAGER) -L$(LIBDIR) $(BDBSMILIB) $(SDBLIB) $(TOOLLIB) $(DEFAULTLIB)
else
# ... as static library
	$(AR) -r $(LIBOBJ) $(OBJECTS)
endif


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
	$(RM) $(OBJECTS) $(DEP_FILES) $(LIBOBJ)
	$(RM) *.class *~ *.tmp
	$(RM) *.pdf *.dvi *.log *.aux *.pd *.tex *.toc
