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

The operator "genRel"[2] is used to generate nested relations of several forms
for testing. It features the possibility to build relations of types
"nrel2"[2] and "arel2"[2], "nrel"[2] and "arel"[2] as well as "record"[2] and
"vector"[2]. It is possible to nest them to arbitrary depth.

Relations can contain a specifiable amount of attributes of different atomic
types and can grow to an arbitrary length given. For attributes of types
"string"[2] and "text"[2] the length is definable. Furthermore the selectivity
can be adjusted to support the testing of filter conditions.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_GENREL_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_GENREL_H_

#include "Algebras/Stream/Stream.h"

using namespace std;
namespace nr2a {

class GenRel
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "genRel";
          signature = "\"nrel\" x string x int x int x string x int x int x "
              "RELTYPE -> nrel(tuple(X))";
          appendSignature(
              "\"arel\" x string x int x int x string x int x int x "
                  "RELTYPE -> arel(tuple(X))");
          appendSignature(
              "\"nrel2\" x string x int x int x string x int x int x "
                  "RELTYPE -> nrel2(tuple(X))");
          appendSignature(
              "\"arel2\" x string x int x int x string x int x int x "
                  "RELTYPE -> arel2(tuple(X))");
          appendSignature(
              "\"vector\" x string x int x int x string x int x int x "
                  "RELTYPE -> vector(record(X))");
          syntax = "genRel(_, _, _, _, _, _, _)";
          meaning =
              "Produces a nested relation for testing. The parameters "
                  "are (left to right):\n"
                  "Resulting type, names prefix, number of attributes, "
                  "number of tuples, attributes type, string length, "
                  "selectivity, data of subrelation where "
                  "RELTYPE is one of arel(tuple(X)), arel2(tuple(X)), "
                  "vector(record(X))\n"
                  "attributes type may be: bool, int, real, string or text";
          example = "query genRel(\"nrel2\", \"Attr\", 10, 100, \"int\", "
              "0, 0, undefined) consume";
        }
    };
    virtual ~GenRel();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int GenRelValue(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    GenRel(); // Declared, but not defined => Linker error on usage

};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_GENREL_H_*/
