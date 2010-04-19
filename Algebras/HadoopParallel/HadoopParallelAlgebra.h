/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]

[1] Implementation of HadoopParallelAlgebra

April 2010 Jiamin Lu

[TOC]

[newpage]

1 Abstract

This algebra implements all relevant operators about combining Hadoop and Secondo
together to execute some parallel work. The new operators include:

  * ~doubleexport~.

  * ~paraJoin~.

1 Includes,  Globals

*/
#ifndef HADOOPPARALLELALGEBRA_H_
#define HADOOPPARALLELALGEBRA_H_

class deLocalInfo
{
private:

  Word streamA;
  Word streamB;
  bool isAEnd;          //Check out whether streamA is exhausted

  // the indexes of the attributes which will
  // be merged and the result type
  int attrIndexA;
  int attrIndexB;

  ListExpr tupTypeA;
  ListExpr tupTypeB;

  TupleType *resultTupleType;

  Word makeTuple(Word stream, int index, ListExpr typeInfo, int sig);

public:
  deLocalInfo(Word _streamA, Word wAttrIndexA,
              Word _streamB, Word wAttrIndexB,
              Supplier s);

  Tuple* nextResultTuple();
};

ListExpr renameList(ListExpr oldTupleList, string appendName);

class phjLocalInfo
{
private:

  Word mixStream;
  ListExpr resultTupleInfo;

  TupleBuffer *joinedTuples;
  GenericRelationIterator *tupleIterator;

  bool getNewProducts();

  //For debug
  int count;

public:
  //phjLocalInfo(Word _streamA, Supplier s);
  phjLocalInfo(Word _streamA, Supplier s);

  Word nextJoinTuple();
};

#endif /* HADOOPPARALLELALGEBRA_H_ */
