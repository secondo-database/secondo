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
ALL_OBJS = AlgebraList.i $(DEP_FILES) $(LIBDIR)/AlgebraList.$(OBJEXT) $(OBJECTS)

.PHONY: all
all:  $(ALL_OBJS) 


AlgebraList.i: ../../makefile.algebras
AlgebraList.i: AlgebraList.i.cfg 
	@rm -f $@
ifndef SECONDO_ACTIVATE_ALL_ALGEBRAS
	chmod 755 ./alg.bash && ./alg.bash $< $(ALGEBRAS) $(JNI_ALGEBRAS) > $@
else
	sed -e "s#ALGEBRA(#ALGEBRA_INCLUDE(#" $< > $@
endif

AlgebraList.cpp : AlgebraList.i
	
# After building AlgebraList.o the file AlgebraList.i will be deleted
# this ensures that it will be rebuild for every call of make. This guarantees
# that only the algebras of makefile.algebras are used even if the timestamp is
# older
$(LIBDIR)/AlgebraList.$(OBJEXT): AlgebraList.cpp AlgebraList.i
	$(CC) -c -o  $@ $(CCFLAGS) $<
	rm -f AlgebraList.i

include $(DEP_FILES)

.PHONY: clean
clean:
	$(RM) $(ALL_OBJS)
