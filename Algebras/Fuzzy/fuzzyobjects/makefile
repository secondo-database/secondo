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

# filename fuzzyobjects/makefile
#
# last change: 05-2003; Th. Behr

include ../makefile.inc
CLASSFILES = GeoObject.class Params.class

.PHONY:all
all: $(CLASSFILES)
	(cd basic; make all)
	(cd simple; make all)
	(cd composite; make all)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<   

.PHONY: clean
clean:
	(cd basic; make clean)
	(cd simple; make clean)
	(cd composite; make clean)
	rm -f *.class
