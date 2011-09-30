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
# generic makefile for an algebra module 
#
########################################################################

# inlcude global definitions
include ../../makefile.env

# expand compile flags if necessary
CCFLAGS += $(ALG_INC_DIRS) 

# Activate preprocessor output
#CCFLAGS += -E 

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 
.PHONY: all
all: $(OBJECTS) $(LIBOBJ)

# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)

# local files for documentation
DOCU_FILES = ExtRelation2Algebra.cpp \
		     ExtRelation2AlgebraTest.cpp \
		     HashJoin.h HashJoin.cpp \
		     GraceHashJoin.h GraceHashJoin.cpp \
		     HybridHashJoin.h HybridHashJoin.cpp \
		     Sort.h Sort.cpp \
		     SortMergeJoin.h SortMergeJoin.cpp \
		     TupleBuffer2.h TupleBuffer2.cpp \
		     TupleQueue.h TupleQueue.cpp 
		
# non-local files for documentation located in ./docu
DOCU_FILES2 = FileSystem.h.txt \
		      FileSystem.cpp.txt \
		      RelationAlgebra.h.txt \
		      RelationPersistent.cpp.txt \
		      ExtRelation2Algebra.spec.txt \
		      ExtRelation2.examples.txt \
		      makefile.txt

DOCU_PDF1 = $(DOCU_FILES:%=%.pdf)
DOCU_PDF2 = $(DOCU_FILES2:%=%.pdf)
DOCU_PDF = $(DOCU_PDF1) $(DOCU_PDF2)

# generic rule for creating documentation	
.PHONY: docu
docu: 
	for i in $(DOCU_FILES); do pd2pdf $$i; done
	for i in $(DOCU_FILES2); do cp ./docu/$$i $$i; done
	for i in $(DOCU_FILES2); do pd2pdf $$i; done
	gs -q -sPAPERSIZE=a4 -dNOPAUSE -dBATCH  \
	   -sDEVICE=pdfwrite -sOutputFile=ExtRelation2Algebra.pdf $(DOCU_PDF)
	pdftops ExtRelation2Algebra.pdf tmp.ps    
	psnup -2 tmp.ps ExtRelation2Algebra.ps
	ps2pdf ExtRelation2Algebra.ps ExtRelation2Algebra.pdf
	for i in $(DOCU_PDF); do rm -f $$i; done
	for i in $(DOCU_FILES2); do rm -f $$i; done
	$(RM) ./*.toc 
	$(RM) ./*.aux 
	$(RM) ./*.dvi
	$(RM) ./*.log
	$(RM) ./*.ps
	$(RM) ./*.tex


	
	
