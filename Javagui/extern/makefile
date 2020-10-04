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
# filename extern/makefile
#
# last change: 08-2003; Th. Behr

include ../makefile.inc
JAVAFILES =$(shell find -name "*.java")
TMPFILES = $(subst .java,.class,$(JAVAFILES))
CLASSFILES = $(subst ./,,$(TMPFILES))



.PHONY:all
all: subdirs $(CLASSFILES)


.PHONY:subdirs
subdirs:
	($(MAKE) -C numericreader all)
	($(MAKE) -C shapereader all)
	($(MAKE) -C dbfreader all)
	($(MAKE) -C stlreader all)
	($(MAKE) -C shapedbf  all)
	($(MAKE) -C binarylist all)
	($(MAKE) -C psexport all)
	($(MAKE) -C gpxreader all)
	($(MAKE) -C nl all)

%.class:%.java
	$(JAVAC) -classpath $(CLASSPATH) $<

.PHONY: clean
clean:
	($(MAKE) -C numericreader clean)
	($(MAKE) -C shapereader clean)
	($(MAKE) -C dbfreader clean)
	($(MAKE) -C stlreader clean)
	($(MAKE) -C shapedbf clean)
	($(MAKE) -C binarylist clean)
	($(MAKE) -C psexport clean)
	($(MAKE) -C gpxreader clean)
	($(MAKE) -C nl clean)
	rm -f *.class
