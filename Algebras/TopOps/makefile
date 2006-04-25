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
# SECONDO makefile for PointRectangle Algebra C++
#
########################################################################

include ../../makefile.env

MODNAME = TopOpsAlgebra
CCFLAGS = $(PICFLAG) $(DEFAULTCCFLAGS) -I../Spatial -I../TopRel -I../Rectangle\
          -I../FText
LATEX = latex -interaction=nonstopmode


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

.PHONY: pdf
pdf: $(MODNAME).cpp.ps
	ps2pdf $<

$(MODNAME).cpp.ps: $(MODNAME).cpp.dvi
	dvips $<

$(MODNAME).cpp.dvi: $(MODNAME).cpp.tex
	$(LATEX) $<
	$(LATEX) $<
	$(LATEX) $<

$(MODNAME).cpp.tex: $(MODNAME).cpp
	pd2tex $<
.PHONY: cleanpdf
cleanpdf:
	$(RM)  $(MODNAME).cpp.*
