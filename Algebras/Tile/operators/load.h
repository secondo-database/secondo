/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

#ifndef TILEALGEBRA_LOAD_H
#define TILEALGEBRA_LOAD_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

/*
TileAlgebra includes

*/

#include "../t/tintArray.h"
#include "../t/tintFlob.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct loadInfo describes name, syntax, meaning and signature
of TileAlgebra operator load.

author: Dirk Zacher

*/

struct loadInfo : OperatorInfo
{
  loadInfo()
  {
    name      = "load";
    syntax    = "load(_)";
    meaning   = "Loads the values of a tintArray or a tintFlob.";
    signature = tintArray::BasicType() + " -> " + CcBool::BasicType();
    appendSignature(tintFlob::BasicType() + " -> " + CcBool::BasicType());
  }
};

/*
declaration of loadFunctions array.

*/

extern ValueMapping loadFunctions[];

/*
Method loadSelectFunction returns the index of specific load function
in loadFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of load operator
return value: index of specific load function in loadFunctions
exceptions: -

*/

int loadSelectFunction(ListExpr arguments);

/*
Method loadTypeMappingFunction returns the return value type
of load operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of load operator
return value: return value type of load operator
exceptions: -

*/

ListExpr loadTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_LOAD_H
