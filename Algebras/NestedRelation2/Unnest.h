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

1.1 Unnest

Unnesting is the inverse operation to nesting.

Given a nested relation with at least one subrelation, one can ~unnest~ it with
this operator. The resulting relation will not have the given subrelation,
instead of this the attributes will be part of the parent relation of it. The
attribute values of the formner parent relation will be duplicated for each
tuple of the subrelation.

----
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

                          ||  query DocumentsNested feed
                         \||/ unnest[Authors]
                          \/  consume

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

----

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_UNNEST_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_UNNEST_H_

#include "Algebras/Stream/Stream.h"

using namespace std;
namespace nr2a {

class Unnest
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "unnest";
          signature = "stream(tuple(X)) -> stream(tuple(Y))";
          syntax = "_ unnest[xi] (where xi is of type nrel2)";
          meaning = "Unnests an attribute relation in a tuple stream.";
          example = "query Families feed unnest[Children] consume";
        }
    };
    virtual ~Unnest();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int UnnestValue(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    Unnest(); // Declared, but not defined => Linker error on usage
    static Tuple * BuildTuple(TupleType* tupleTypeOut,
        const Tuple * const tupleRel, const Tuple * const tupleSubRel,
        const int attributeIndex);
    static void TupleSetValuesOfSubRel(Tuple * tupleOut,
        const Tuple * const tupleSubRel, const int attributeIndex);
    static ListExpr BuildResultingType(const ListExpr attributesTypes,
        const string attributeUnnestName);
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_UNNEST_H_*/
