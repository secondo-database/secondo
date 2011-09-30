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
CCFLAGS += $(ALG_INC_DIRS) -Werror

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
clean: cleanpdf cleanplugin
	$(clean-algebra)


.PHONY: pdf
pdf: PeriodicAlgebra.pd.dvi
	dvipdfm -p a4 $<

PeriodicAlgebra.pd.dvi: PeriodicAlgebra.pd.tex
	latex $<
	latex $<
	latex $<

PeriodicAlgebra.pd.tex: PeriodicAlgebra.pd
	pd2tex $<



SRCFILES = PeriodicTypes.h PeriodicSupport.h PeriodicSupport.cpp classImpl.txt SimplePoint.cpp PBBox.cpp RelInterval.cpp PInterval.cpp CompositeMove.cpp SpatialCompositeMove.cpp SubMove.cpp CSubMove.cpp PeriodicMove.cpp SpatialPeriodicMove.cpp LinearConstantMove.h LinearBoolMove.cpp LinearIntMove.cpp LinearInt9mMove.cpp LinearPointMove.cpp LinearPointsMove.cpp MRealMap.cpp MovingRealUnit.cpp TwoPoints.cpp PMSimple.h PMInt9M.cpp PMPoint.cpp PMPoints.cpp NodeTypes.h List2.h RepTree.h RepTree.cpp PeriodicAlgebra.cpp 

PeriodicAlgebra.pd: $(SRCFILES) makefile
	make -C Figures all
	echo -e ""  > PeriodicAlgebra.pd
	for i in $(SRCFILES); do \
     cat $$i >>PeriodicAlgebra.pd; \
  done

 
.PHONY: cleanpdf
cleanpdf:
	rm -f *.pd.*
	rm -f PeriodicAlgebra.pd
	make -C Figures clean


.PHONY: plugin
plugin:
	rm -rf algebra
	mkdir algebra
	cp *.txt *.h *.cpp makefile *.spec *.examples algebra
	rm -rf displayClasses
	mkdir displayClasses
	cp ../../Javagui/viewer/hoese/algebras/periodic/*.java displayClasses
	cp ../../Javagui/viewer/hoese/algebras/periodic/makefile displayClasses
	cp ../../Javagui/viewer/hoese/algebras/Dsplpmpoint.java displayClasses
	cp ../../Javagui/viewer/hoese/algebras/Dsplpmpoints.java displayClasses
	cp ../../Javagui/viewer/hoese/algebras/Dsplpmreal.java displayClasses
	cp ../../Javagui/viewer/hoese/algebras/Dsplpmrect.java displayClasses
	cp ../../Javagui/viewer/hoese/algebras/Dsplpmregion.java displayClasses
	cp copyright.txt displayClasses
	rm -f Periodic.zip
	zip -r Periodic.zip  algebra displayClasses SecondoExtension.xml 
	rm -rf algebra
	rm -rf displayClasses	

.PHONY: cleanplugin
cleanplugin:
	rm -rf algebra
	rm -rf displayClasses
	rm -f Periodic.zip




