#This file is part of SECONDO.
#
#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.
#
#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

include ../../../makefile.env
include ../../../Javagui/makefile.inc

CLASSES = AlgebraType.class AlgebraWriter.class ClassMethod.class 
CLASSES += Environment.class FinalPanel.class JavaWrapper.class 
CLASSES += OperatorSpecification.class Serializer.class SpecificationPanel.class 
CLASSES += TypeDescription.class TypeDescriptionPanel.class

S=:

CP = $(SECONDO_BUILD_DIR)/Tools/Jni$S$(SECONDO_BUILD_DIR)/Javagui$S$(JARS)

.PHONY: all
all: sj $(CLASSES)

.PHONY: sj
sj:
	make -C $(SECONDO_BUILD_DIR)/Javagui/sj all

%.class: %.java
	javac -classpath $(CP) $<

.PHONY: clean
clean:
	rm -f *.class

