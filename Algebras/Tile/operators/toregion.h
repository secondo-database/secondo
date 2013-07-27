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

#ifndef TILEALGEBRA_TOREGION_H
#define TILEALGEBRA_TOREGION_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "SpatialAlgebra.h"

/*
TileAlgebra includes

*/

#include "../t/tbool.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct toregionInfo describes name, syntax, meaning and signature
of TileAlgebra operator toregion.

author: Dirk Zacher

*/
  
struct toregionInfo : OperatorInfo
{
  toregionInfo()
  { 
    name      = "toregion";
    syntax    = "_ toregion";
    meaning   = "Maps a tbool object to a region.";
    signature = tbool::BasicType() + " -> " + Region::BasicType();
  }
};

/*
Method toregionFunction implements the toregion operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toregion operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toregionFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toregionFunction successfully executed, otherwise FAILURE
exceptions: -

*/

int toregionFunction(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier);

/*
Method toregionTypeMappingFunction returns the return value type
of toregion operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of toregion operator
return value: return value type of toregion operator
exceptions: -

*/

ListExpr toregionTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_TOREGION_H
