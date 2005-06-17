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
# Filename: SecondoJava/sj/lang/makefile
#
# Last change: Oct. 2002, M. Spiekermann

include ../../makefile.inc

all: DBArray.class Java_ListExpr langClasses

langClasses: IntByReference.class ServerErrorCodes.class SecondoInterface.class \
             ESInterface.class ListExpr.class MyDataInputStream.class MyDataOutputStream.class

CP:=$(CLASSPATH):$(JARS)

Java_ListExpr:
	(cd JavaListExpr; make all)



#////////////// generic rules
%.class: %.java
	$(JAVAC) $(JAVACFLAGS) -classpath $(CP) $<


#////////// clean rules
clean_local:
	rm -f *.class

clean: clean_local
	(cd JavaListExpr; make clean)


