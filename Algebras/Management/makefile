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
########################################################################
#
# SECONDO makefile for Algebra and Algebra Manager
#
########################################################################

include ../../makefile.env

CCFLAGS = $(DEFAULTCCFLAGS)

DEP_FILES += AlgebraList.dep

.PHONY: all
all: $(OBJECTS)

AlgebraList.dep: AlgebraList.i

include $(DEP_FILES) 

# special rule for AlgebraList.o
AlgebraList.$(OBJEXT): AlgebraList.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)
	cp $@ $(LIBDIR)/$@

ifeq ($(alg),auto)
AlgebraList.i: AlgebraList.i.cfg $(BUILDDIR)/makefile.algebras
	rm -f $@
	@chmod 755 ./alg.bash && ./alg.bash $(ALGEBRAS) $(JNI_ALGEBRAS) > $@
else
AlgebraList.i: AlgebraList.i.cfg
	$(cp-config-file)
endif

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEP_FILES) $(LIBDIR)/AlgebraList.$(OBJEXT)
