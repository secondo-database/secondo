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
# file viewer/makefile
#
# changes:
# 05-2003; Thomas Behr
# 01-2004: stefan.wich@fernuni-hagen.de: jpeg added, sorted viewers alnum, one line per subtask
# 02-2004: Markus Spiekermann: The makfile was revised to make it more generic.
include ../makefile.inc


.PHONY: all  # compile Menuvector and SecondoViewer
all: makefile.viewers viewers $(JAVA_STAMP)

makefile.viewers: makefile.viewers.example
	$(cp-config-file)

include makefile.viewers

INC_FILES := $(shell ls ./makefile.n*c 2>/dev/null)
ifdef INC_FILES
include $(INC_FILES)
endif

# the top-level viewer sources are exactly the viewer classes plus the two
# framework classes; compile them all with a single javac invocation.
# NOTE: this must come after the makefile.n*c includes above, which may append
# to VIEWER_CLASSES (e.g. makefile.nonpublic adds ChessViewer.class).
EXTRACP := $(JARS)
JAVAFILES := $(VIEWER_CLASSES:.class=.java) MenuVector.java SecondoViewer.java

# compile all viewers
.PHONY: viewers
viewers: makedirs


.PHONY: makedirs
makedirs:
	for dir in $(VIEWER_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done


.PHONY: clean
clean:
	rm -f *.class $(JAVA_STAMP)
	rm -f makefile.viewers
	for dir in $(VIEWER_DIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi \
        done

# compile all top-level viewer sources in one javac invocation
$(JAVA_STAMP): $(JAVAFILES)
	$(compile-java)

