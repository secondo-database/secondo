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

.PHONY: makedirs
makedirs: makedirsnormal makedirsjni

.PHONY: makedirsnormal
makedirsnormal:
	$(MAKE) -C NestedLists
	$(MAKE) -C Parser
	$(MAKE) -C Utilities
	$(MAKE) -C pd 

.PHONY: makedirsjni
makedirsjni:
 ifeq ($(USE_JNI),"true")
	$(MAKE) -C Jni
 endif

.PHONY: clean
clean:
	$(MAKE) -C NestedLists clean
	$(MAKE) -C Parser clean
	$(MAKE) -C Utilities clean
	$(MAKE) -C Jni clean
	$(MAKE) -C pd clean

