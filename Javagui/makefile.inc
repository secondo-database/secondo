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

# JAVAC and JAVA define the full path name of the Java compiler and java
# interpreter respectively. We recommend to use jdk 1.4 or higher. If you
# have different jdk versions change the names

JAVAC = javac
JAVA = java
JAVADOC = javadoc
MAKE = make

# SECONDOPATH should be a shell-variable
ifndef SECONDO_BUILD_DIR
$(error Variable SECONDO_BUILD_DIR is not defined. This variable must hold the directory of Secondo)
endif

SECONDOPATH := $(SECONDO_BUILD_DIR)

GUIPATH := $(SECONDOPATH)/Javagui

LIBPATH=$(GUIPATH)/lib
JLEX:=JLex_v1.2.6.jar
CUP:=java_cup_v10k.jar
CUPRUNTIME:=java_cup_v10k_runtime.jar
MP3:=jl0.4.jar
PDF:=14_os_jpedal.jar

JLEXCLASSESPATH := $(LIBPATH)/$(JLEX)
CUPCLASSESPATH := $(LIBPATH)/$(CUP)
CUPCLASSESRUNTIME := $(LIBPATH)/$(CUPRUNTIME)
MP3CLASSESPATH := $(LIBPATH)/$(MP3)
PDFCLASSESPATH := $(LIBPATH)/$(PDF)

CLASSPATH := $(GUIPATH)
JARS := $(JLEXCLASSESPATH):$(CUPCLASSESPATH):$(MP3CLASSESPATH):$(PDFCLASSESPATH)

DOCPATH := ./doc

