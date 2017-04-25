/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

*/

#pragma once

#include "AlgebraTypes.h"
#include "NestedList.h"
#include "Operator.h"

namespace CRelAlgebra
{
  namespace Operators
  {
    /*
    Operator implementation of the 'cconsume' operator.
    This operator appends a stream of tuples or tuple-blocks to a new
    column oriented relation.

    The Operator expects two parameters.

    The first parameter represents the incoming values and must be either a
    stream of 'tuple' or a stream of 'tblock'.

    The second parameter represents the:
     *desired block-size: must be of type 'int'.
     *type of the relation (template): must be of type 'crel'

    The returned value represents the relation and is of type 'crel'.
    */
    class CConsume : public Operator
    {
    public:
      CConsume();

    private:
      static const OperatorInfo info;

      static ValueMapping valueMappings[];

      static ListExpr TypeMapping(ListExpr args);

      static int SelectValueMapping(ListExpr args);

      //Value mapping if first parameter is a stream of 'tblock'
      static int TBlockValueMapping(ArgVector args, Word &result, int message,
                                    Word &local, Supplier s);

      //Value mapping if first parameter is a stream of 'tuple'
      static int TupleValueMapping(ArgVector args, Word &result, int message,
                                   Word &local, Supplier s);
    };
  }
}