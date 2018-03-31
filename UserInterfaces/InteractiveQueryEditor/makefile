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
#
###########################################################################
#
# Makefile for the program SecondoTTYIQE
#
###########################################################################
#filename InteractiveQueryEditor/makefile

include ../../makefile.env

JNITOOL_OBJECT_PATH = ../../Tools/Jni/JNITool.o

.PHONY: all
all: Executable SecondoTTYIQE.jar SecondoKernel.so

.PHONY: Executable
Executable:
	@printf '%s\n' '#!/bin/bash' 'java -jar SecondoTTYIQE.jar -Djava.library.path=/home/secondo > secondoOut.txt 2> secondoOut.txt' > SecondoTTYIQE.sh
	@chmod +x SecondoTTYIQE.sh

.PHONY: SecondoTTYIQE.jar
SecondoTTYIQE.jar:
	@echo "Creating Jar"
	@rm -rf build
	@mkdir build
	@cd build;find ../lib -name "*.jar" -exec jar -xf {} \;
	@cd build;find . -name "*.java" -exec rm -f {} \;
	@find . -path ./build -prune -o -name "*.java" -exec cp --parent {} build \;
	@cd build;find . -name "*.java" -exec javac -cp . {} \;
	@cd build;find . -name "*.java" -exec rm -f {} \;
	@printf '%s\n' 'Main-Class: application/Application' > ./build/META-INF/MANIFEST.MF
	@cd build; jar -cfm  ../SecondoTTYIQE.jar ./META-INF/MANIFEST.MF *



.PHONY: SecondoKernel.so
SecondoKernel.so: JNITool.o SecondoKernel.o
	@echo "Creating Shared Object"

.PHONY: SecondoKernel.o
SecondoKernel.o: JNITool.o
	@echo "Creating SecondoKernel.o"

.PHONY: JNITool.o
JNITool.o:
	@echo "Checking JNITool.o"
ifeq ("$(wildcard $(JNITOOL_OBJECT_PATH))","")
	@echo "The JNITool.o file is missing. SecondoKernel.so could not be built. See readme for further information."
	@exit 1
endif	

.PHONY: clean
clean:
	rm -rf build




