/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

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

1.1 Nest

Given a flat relation with some redundant information, for example a relation
~Documents~ with attributes ~Title~, ~AForename~, ~AName~ and ~Year~, where
title and year are repeated, iff the document is written by multiple authors,
one could build a nested relation with each document appearing only once. The
authors of every document are then collected in an attribute relation. Such
nested relation can be constructed by the "nest"[2] operator.

The resulting nested relation could also be nested further by the year
attribute collecting all documents wriiten in one year into one tuple. The
resulting relation of this nesting would have three levels with only one
attribute per level.

Nesting is the inverse operation to unnesting.

----
             Title            | AForename  |   AName   | Year
  ----------------------------+------------+-----------+------
   SECONDO Programmer's Guide | Ralf H.    | Gueting   | 2011
  ----------------------------+------------+-----------+------
   SECONDO Programmer's Guide | Victor T.  | de Almeda | 2011
  ----------------------------+------------+-----------+------
   SECONDO Programmer's Guide | ...        | ...       | 20111
  ----------------------------+------------+-----------+------
   SECONDO User Manual        | Ralf H.    | Gueting   | 2012
  ----------------------------+------------+-----------+------
   SECONDO User Manual        | Dirk       | Ansorge   | 2012
  ----------------------------+------------+-----------+------
   SECONDO User Manual        | ...        | ...       | 2012
  ----------------------------+------------+-----------+------
   ...                        | ...        | ...       | ...

                          ||  query DocumentsFlat feed
                         \||/ nest2[Title,Year;Authors]
                          \/  consume2

             Title            |         Authors        | Year
                              | AForename  |  AName    |
  ----------------------------+------------+-----------+------
   SECONDO Programmer's Guide | Ralf H.    | Gueting   | 2011
                              | -----------+---------- |
                              | Victor T.  | de Almeda |
                              | -----------+---------- |
                              | ...        | ...       |
  ----------------------------+------------+-----------+------
   SECONDO User Manual        | Ralf H.    | Gueting   | 2012
                              | -----------+---------- |
                              | Dirk       | Ansorge   |
                              | -----------+---------- |
                              | ...        | ...       |
  ----------------------------+------------+-----------+------
   ...                        | ...        | ...       | ...

----

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_NEST_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_NEST_H_

#include "Stream.h"

#include "Nr2aLocalInfo.h"
#include "LinearProgressEstimator.h"

namespace nr2a {

class Nest
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "nest2";
          signature = "stream(tuple(X)) -> stream(tuple(Y))";
          syntax = "_ nest2[xi1,..., xij; x0]";
          meaning = "Creates a nested tuple stream from a tuple stream. The "
              "stream should be sorted by the attributes that are "
              "to appear in the primary relation.";
          example = "query People feed sortby[Father] sortby[Mother] "
              "nest2[Father,Mother; Children]";
        }
    };
    virtual ~Nest();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int NestValue(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    Nest(); // Declared, but not defined => Linker error on usage

    struct LocalInfo :
        public Nr2aLocalInfo<LinearProgressEstimator<LocalInfo> >
    {
      public:
        ListExpr tupleTypeOut;
        ListExpr tupleTypeSubRel;
        unsigned int groupedAttributesCount;
        unsigned int nestedAttributesCount;
        Stream<Tuple>* inputStream;
        Tuple* tupleIn;
        std::vector<int> groupedAttributesIndices;
        std::vector<int> nestedAttributesIndices;

        LocalInfo(int groupedAttributesCount, int nestedAttributesCount);
        virtual ~LocalInfo();

    };

    static Tuple * BuildResultTuple(LocalInfo & localInfo);
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_NEST_H_*/
