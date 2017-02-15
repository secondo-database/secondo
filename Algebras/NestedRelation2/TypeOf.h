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

The operator "typeof"[2] accepts one argument of any type and results in a
type error if executed. The error message states the type of the given
argument. This is useful during building a complex query.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_TYPEOF_H_
#define ALGEBRAS_NESTEDRELATION2_TYPEOF_H_

namespace nr2a {

class TypeOf
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "typeof";
          signature = "X -> X";
          syntax = "_ typeof";
          meaning = "Cancels the query's execution with an error, whose "
              "message states the operator's input type. Useful for debugging "
              "complex, possibly long running queries.";
          example = "query MyRelation complexFunction typeof complexFunction"
              "count";
        }
    };
    virtual ~TypeOf();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int MapValue(Word* args, Word& result, int message, Word& local,
        Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    TypeOf(); // Declared, but not defined => Linker error on usage
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_TYPEOF_H_*/
