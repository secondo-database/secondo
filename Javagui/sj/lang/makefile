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

ifeq ($(NL_PERSISTENT),true)
PERSISTENT_FLAG:=-DPERSISTENT
else
PERSISTENT_FLAS:=
endif


all: SelectNL DBArray.class Java_ListExpr langClasses

.PHONY: SelectNL
SelectNL:
	@echo "/* " > ListExpr.tmp
	@echo "This file is created automatically. Please don't edit it. " >> ListExpr.tmp
	@echo "If changes are requiered at the ListExpr class, make them in the" >> ListExpr.tmp
	@echo "Original File 'ListExpr.both'">>ListExpr.tmp
	@echo "*/">>ListExpr.tmp
	$(CC) -E -xc -P -C $(PERSISTENT_FLAG) ListExpr.both >>ListExpr.tmp
	diff ListExpr.tmp ListExpr.java >/dev/null || cp ListExpr.tmp ListExpr.java
	rm ListExpr.tmp

langClasses: IntByReference.class ServerErrorCodes.class SecondoInterface.class \
             ESInterface.class ListExpr.class MyDataInputStream.class MyDataOutputStream.class

CP:=$(CLASSPATH):$(JARS)

Java_ListExpr:
	(cd JavaListExpr; make all)


ListExpr.class: ListExpr.java
	$(JAVAC) $(JAVACFLAGS) -classpath $(CP) ListExpr.java

#////////////// generic rules
%.class: %.java
	$(JAVAC) $(JAVACFLAGS) -classpath $(CP) $<


#////////// clean rules
clean_local:
	rm -f *.class
	rm -f ListExpr.java

clean: clean_local
	(cd JavaListExpr; make clean)


