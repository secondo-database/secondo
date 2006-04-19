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
# file: gui/makefile
#
# last changed : 11-2003

include ../makefile.inc

platform := $(SECONDO_PLATFORM)
ifeq ($(platform),win32)
   GetEnvSO := ../lib/GetEnv.dll
   JNI_MD := win32
else
   GetEnvSO := ../lib/libGetEnv.so
   JNI_MD = linux
endif

ifndef GCC
  GCC := gcc
endif


CLASSFILES=xxx2secondo.class xxx2secdb.class MovingPointBuilder.class Base64Encoder.class Base64Decoder.class Base64.class DupRemover.class PeriodicPointBuilder.class TFWCreator.class TextFormat.class Reporter.class GetEnv.class GetEnv.h $(GetEnvSO)


.PHONY: all 
all: ../sj/lang/ListExpr.class $(CLASSFILES)

../sj/lang/ListExpr.class:
	make -C ../sj


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<


GetEnv.h: GetEnv.java
	$(JAVAH) -classpath .. -o GetEnv.h tools.GetEnv

GetEnv.o: GetEnv.c
	$(GCC) -fPIC -D_REENTRANT -Wall -I$(J2SDK_ROOT)/include -I$(J2SDK_ROOT)/include/$(JNI_MD) -I. -c GetEnv.c -o GetEnv.o

../lib/libGetEnv.so: GetEnv.o
	$(GCC) -shared -o $(GetEnvSO) GetEnv.o

../lib/GetEnv.dll: GetEnv.o GetEnv.def
	gcc -shared -Wl,-soname,GetEnv.dll GetEnv.o GetEnv.def

GetEnv.def: GetEnv.o
	dlltool -z GetEnv.def.tmp  MeshGenerator.o
	sed -e "s#\(.*\)@\(.*\)@.*;#\1 = \1@\2;#g" GetEnv.def.tmp >GetEnv.def
	rm GetEnv.def.tmp


.PHONY: clean
clean:
	rm -f *.class
	rm -f GetEnv.o GetEnv.h $(GETENVSO) GetEnv.def


