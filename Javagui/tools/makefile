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
include ../../makefile.$(platform)
include ../../makefile.jni

GetEnvDLL := ../lib/$(DLLPREF)GetEnv.$(JNI_DLLEXT)

GCC := gcc


CLASSFILES:= $(shell ls *.java | sed -e "s/java/class/")
CLASSFILES+=GetEnv.h GetEnv.o $(GetEnvDLL)

.PHONY: all 
all: ../sj/lang/ListExpr.class $(CLASSFILES)

../sj/lang/ListExpr.class:
	make -C ../sj


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<


GetEnv.h: GetEnv.java
	$(JAVAH) -force -classpath .. -o $@ tools.GetEnv

GetEnv.o: GetEnv.c
	$(GCC) -fPIC -D_REENTRANT -Wall $(JNIINCLUDE) -I. -c $< -o $@ 

ifeq ($(platform),win32)
$(GetEnvDLL): GetEnv.o GetEnv.def
	$(GCC) $(DLLFLAGS) -Wl,-soname,GetEnv.$(JNI_DLLEXT) -o $@ $^

GetEnv.def: GetEnv.o
	dlltool -z $@.tmp $< 
	sed -e "s#\(.*\)@\(.*\)@.*#\1 = \1@\2#g" $@.tmp > $@
	rm $@.tmp

else
$(GetEnvDLL): GetEnv.o
	$(GCC) $(DLLFLAGS) -o $@ $< $(JNI_CCFLAGS)

endif

.PHONY: clean
clean:
	rm -f *.class
	rm -f GetEnv.o GetEnv.h $(GetEnvDLL) GetEnv.def


