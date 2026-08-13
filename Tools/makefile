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

########################################################################
#
# SECONDO makefile for tool and utility modules
#
########################################################################

include ../makefile.env

.PHONY: all
all: makedirs

# One list drives both building and cleaning. They used to be two hand-kept
# recipes that had drifted apart: "clean" recursed into RStarTree, which is
# never built.
SUBDIRS := DFS Flob BigArray NestedLists Parser Utilities pd SyntaxParser TypeMap

# Built only when JNI algebras are enabled, but always cleaned -- a leftover
# object from an earlier JNI build should not survive "make clean".
JNI_SUBDIRS := Jni

.PHONY: makedirs
makedirs: makedirsnormal makedirsjni

# Left sequential on purpose. Turning these into per-directory targets would
# let -j run them concurrently, but the order here is load bearing in at least
# one place (the generated parsers), and nothing establishes the dependencies
# between them.
.PHONY: makedirsnormal
makedirsnormal:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir || exit 1; done

.PHONY: makedirsjni
makedirsjni:
 ifeq ($(USE_JNI),"true")
	@for dir in $(JNI_SUBDIRS); do $(MAKE) -C $$dir || exit 1; done
 endif

.PHONY: clean
clean:
	@for dir in $(SUBDIRS) $(JNI_SUBDIRS); do \
	  $(MAKE) -C $$dir clean || exit 1; \
	done

