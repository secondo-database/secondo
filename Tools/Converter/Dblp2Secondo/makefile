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
# set Variables

platform := $(SECONDO_PLATFORM)
BDBDIR := $(BERKELEY_DB_DIR)
LEX := flex
LEX_LIB_FLAG := -lfl
BDB_LIB_FLAG := -ldb


ifndef platform
 $(error Variable SECONDO_PLATFORM not defined. Set it to linux, win32 or solaris!)
endif

ifndef BDBDIR
 $(error Variable BERKELEY_DB_DIR not defined. It must point to the root directory of the Berkeley DB installation!)
endif


ifeq ($(platform),win32)
  EXE_EXT := ".exe"
  BDB_LIB_FLAG := -ldb32
else
  EXE_EXT :=
  BDB_LIB_FLAG := $(BDB_LIB_FLAG) -lpthread
endif

BDB_INCLUDE_FLAG = -I$(BDBDIR)/include
BDB_LIB_DIR_FLAG = -L$(BDBDIR)/lib


.PHONY: all
all: DoubleTitle$(EXE_EXT) Parser KeyWordCreator.class progress FilterHTML


progress: progress.cpp
	gcc -o progress progress.cpp -lstdc++

DoubleTitle$(EXE_EXT): DoubleTitle.c
	gcc -o DoubleTitle$(EXE_EXT) DoubleTitle.c $(LEX_LIB_FLAG)

DoubleTitle.c: DoubleTitle.l
	$(LEX) -o DoubleTitle.c DoubleTitle.l


FilterHTML: FilterHTML.c
	gcc -o FilterHTML FilterHTML.c $(LEX_LIB_FLAG)

FilterHTML.c: FilterHTML.l
	$(LEX) -o FilterHTML.c FilterHTML.l

Parser: DBLPParser.o dblp.o
	gcc -o Parser DBLPParser.o dblp.o $(BDB_INCLUDE_FLAG) $(BDB_LIB_DIR_FLAG) $(LEX_LIB_FLAG) $(BDB_LIB_FLAG)

DBLPParser.o: DBLPParser.y.c
	gcc -c -o DBLPParser.o  $(BDB_INCLUDE_FLAG) DBLPParser.y.c

DBLPParser.y.c: DBLPParser.y
	bison --yacc -d  -o DBLPParser.y.c DBLPParser.y

dblp.o:	lex.yy.c
	gcc -c -o dblp.o lex.yy.c

lex.yy.c:	DBLPLex.l
	flex  DBLPLex.l


KeyWordCreator.class: KeyWordCreator.java
	javac KeyWordCreator.java

clean:
	rm -f *.exe DBLPParser.y.h DBLPParser.y.c DoubleTitle.c FilterHTML.c lex.yy.c
	rm -f *.o *.class DoubleTitle$(EXE_EXT) Parser$(EXE_EXT) FilterHTML$(EXE_EXT) progress$(EXE_EXT)
