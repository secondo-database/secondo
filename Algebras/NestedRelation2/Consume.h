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

1.1 Consume2

The operator "consume2"[2] ~consumes~ a stream of tuples -- presumably
containing attribute relations -- and builds a nested relation of type
"nrel2"[2] from it. To distinguish it from "consume"[2] of NestedRelation
algebra it is postfixed.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_CONSUME_H_
#define ALGEBRAS_NESTEDRELATION2_CONSUME_H_

#include "Include.h"

namespace nr2a {

class Consume
{
  public:
    struct Info : OperatorInfo
    {

        Info()
        {
          name = "consume2";
          signature =  "stream(tuple(X)) -> nrel2(tuple(X))";
          syntax = "_ consume2";
          meaning = "Collects tuples from a stream.";
          example = "query Documents feed unnest2[Authors] consume2";
          supportsProgress = true;
        }
    };
    virtual ~Consume();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int ConsumeValue(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    Consume(); // Declared, but not defined => Linker error on usage

};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_CONSUME_H_*/
