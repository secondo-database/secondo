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

CLASSFILES=xxx2secondo.class xxx2secdb.class MovingPointBuilder.class Base64Encoder.class Base64Decoder.class Base64.class DupRemover.class PeriodicPointBuilder.class TFWCreator.class


.PHONY: all 
all: ../sj/lang/ListExpr.class $(CLASSFILES)

../sj/lang/ListExpr.class:
	make -C ../sj


%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH) $<




.PHONY: clean
clean:
	rm -f *.class


