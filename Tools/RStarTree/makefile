# $Id$
# Copyright (C) 2010 by The Regents of the University of California
 
# Redistribution of this file is permitted under
# the terms of the BSD license.
 
# Date: 11/01/2009
# Author: Sattam Alsubaiee <salsubai (at) ics.uci.edu>

CC = g++
CXXFLAGS = -Wall 
LDFLAGS = 

all: librstartree.a nntest 

nntest: rstartree.o storage.o util.o


rstartree.h: storage.h \
	util.h
	touch $@

rstartree.o: rstartree.h

storage_mem.o: storage.h

util.o: util.h

librstartree.a:	librstartree.a( \
		rstartree.o \
		storage.o \
		util.o)

clean:
	-rm -f *.o *.a



