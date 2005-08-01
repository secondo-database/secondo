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

#filename makefile
#
# last change: 2003-07-03 Mirco Guenster
#
# this is the include file for makejava

# if you want to use another compiler the change this name
JAVAC=javac

#
JAVADOC = javadoc

# set the DOCPATH
DOCPATH = ./doc

PACKAGES = twodsack twodsack.io twodsack.operation twodsack.operation.basictypeoperation twodsack.operation.setoperation twodsack.set twodsack.setelement twodsack.setelement.datatype twodsack.setelement.datatype.basicdatatype twodsack.setelement.datatype.compositedatatype twodsack.util twodsack.util.collection twodsack.util.collectiontype twodsack.util.comparator twodsack.util.graph twodsack.util.iterator twodsack.util.meshgenerator twodsack.util.number


ROSE_PATH = $(SECONDO_BUILD_DIR)/Algebras/Rose/RoseJava


JAVAINTERFACE_PATH = $(SECONDO_BUILD_DIR)/Javagui


CLASSPATH=".:$(ROSE_PATH):$(JAVAINTERFACE_PATH)"




