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
include ../../makefile.jni

JNITOOL_OBJECT_PATH = ../../Tools/Jni/JNITool.o
JAVAFILES = $(shell find . -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))
LIBPATH := -L../..//api_cpp/direct/lib

.PHONY: all
all: Executable SecondoTTYIQE.jar SecondoKernel.so

.PHONY: Executable
Executable:
	@printf '%s\n' '#!/bin/bash' 'java -Djava.library.path="../lib/" -jar ../lib/SecondoTTYIQE.jar  > .iqe_secondo_out 2> .iqe_secondo_out' > ../../bin/SecondoTTYIQE.sh
	@chmod +x ../../bin/SecondoTTYIQE.sh


.PHONY: SecondoTTYIQE.jar
SecondoTTYIQE.jar: $(CLASSFILES)
	@echo "Creating Jar"
	@rm -rf build
	@mkdir build
	@cd build;find ../lib -name "*.jar" -exec jar -xf {} \;
	@cd build;find . -name "*.java" -exec rm -f {} \;
	@find . -path ./build -prune -o -name "*.class" -exec cp --parent {} build \;
	@printf '%s\n' 'Main-Class: application/Application' > ./build/META-INF/MANIFEST.MF
	@cd build; jar -cfm  ../../../lib/SecondoTTYIQE.jar ./META-INF/MANIFEST.MF *

%.class: %.java
	javac -cp .:lib/SecondoInterface.jar $<

.PHONY: SecondoKernel.so
SecondoKernel.so: JNITool.o SecondoKernel.o
	@echo "Creating Shared Object"
	$(CC) -shared -o ../../lib/libSecondoKernel.so util/secondo/SecondoKernel.o $(JNITOOL_OBJECT_PATH) -Wl,--whole-archive -L$(SECONDO_BUILD_DIR)/api_cpp/direct/lib -lsecondo -ljpeg -lgsl -L$(BDB_LIB_DIR) -l$(BERKELEY_DB_LIB) -Wl,--no-whole-archive,-soname,SecondoKernel.so $(JNIINCLUDE) $(LIBPATH) $(DEFAULTCCFLAGS)
	
.PHONY: SecondoKernel.o
SecondoKernel.o: JNIHeader util/secondo/SecondoKernel.cpp util/secondo/SecondoKernel.h
	@echo "Compiling new SecondoKernel.o File"
	@$(CC) -c $(JNIINCLUDE) $(DEFAULTCCFLAGS) util/secondo/SecondoKernel.cpp -o util/secondo/SecondoKernel.o
	
.PHONY: JNIHeader
JNIHeader: util/secondo/SecondoKernel.java
	@echo "Creating new SecondoKernel.h File"
	@$(JAVAH) -o util/secondo/SecondoKernel.h -classpath $(CLASSPATH) util.secondo.SecondoKernel

.PHONY: JNITool.o
JNITool.o:
	make -C ../../Tools/Jni

.PHONY: clean
clean:
	rm -rf build
	find ./ -name "*.class" -exec rm {} \;  
	rm -f SecondoTTYIQE.jar .iqe_secondo_out specs util/secondo/SecondoKernel.h SecondoTTYIQE.sh

