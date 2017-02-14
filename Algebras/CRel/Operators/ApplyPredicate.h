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
#include "StandardTypes.h"

namespace CRelAlgebra
{
  namespace Operators
  {
    /*
    Operator implementation of the 'ap' operator.
    This operator determines the indices of entries in a array of attributes
    which satisfy a given predicate.

    The Operator expects two parameters.

    The first parameter represents the array and must be of type 'attrarray'.

    The second parameter represents the predicate and must be a function
    evaluating from the array's attribute-type to 'bool'.

    The returned value represents the determined indices and is of type
    'indices'.
    */
    class ApplyPredicate : public Operator
    {
    public:
      ApplyPredicate();

    private:
      static const OperatorInfo info;

      static ListExpr TypeMapping(ListExpr args);

      static int ValueMapping(ArgVector args, Word &result, int message,
                              Word &local, Supplier s);
    };
  }
}