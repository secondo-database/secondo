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

1 Operators

In this documentation the operators are sorted by ``complexity'' in ascending
order. First there are some more simple operators, afterwards the ones dealing
with streams and at last the ones implemented for testing and/or debugging.

1.1 Count

This implementation of "count"[2] can be used in conjunction with "ARel"[2] and
"NRel"[2] values an returns their amount of tuples on the top level. An
implementation for streams thereof is provided by the Stream algebra.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_COUNT_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_COUNT_H_

namespace nr2a {

class Count
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "count";
          signature = ARel::BasicType() + " -> " + CcInt::BasicType();
          appendSignature(string("nrel2") + " -> " + CcInt::BasicType());
          syntax = "_ count";
          meaning = "Returns the amount of tuples in the given relation.";
          example = "query MyNestedRelation count";
          supportsProgress = true;
        }
    };
    virtual ~Count();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int CountArel(Word* args, Word& result, int message,
        Word& local, Supplier s);
    static int CountNrel(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    Count(); // Declared, but not defined => Linker error on usage

};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_COUNT_H_*/
