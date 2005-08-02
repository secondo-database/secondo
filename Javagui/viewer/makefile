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
all: viewers MenuVector.class SecondoViewer.class MessageBox.class


# add below new Viewer classes or subdirectories to compile
VIEWER_CLASSES := \
		FormattedViewer.class \
		InquiryViewer.class \
		QueryViewer.class \
		RelViewer.class \
		StandardViewer.class \
		Viewer3D.class \
		Fuzzy2D.class \
		RelSplit.class \
		HoeseViewer.class \
		MP3V.class \
		MidiViewer.class \
                TreeViewer.class \
                PictureViewer.class\
                UpdateViewer.class

VIEWER_DIRS := fuzzy2d midi relsplit viewer3d hoese pictureviewer update


# compile all viewers
.PHONY: viewers
viewers: $(VIEWER_CLASSES) makedirs


.PHONY: makedirs
makedirs:
	for dir in $(VIEWER_DIRS); do \
          if !($(MAKE) -C $$dir) then \
            exit 1; \
          fi \
        done


.PHONY: clean
clean:
	rm -f *.class
	for dir in $(VIEWER_DIRS); do \
          if !($(MAKE) -C $$dir clean) then \
            exit 1; \
          fi \
        done

# default rule for creating java classes
%.class: %.java
	$(JAVAC) -classpath $(CLASSPATH):$(MP3CLASSESPATH) $<

