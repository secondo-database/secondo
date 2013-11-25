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

#ifndef TILEALGEBRA_VAL_H
#define TILEALGEBRA_VAL_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"

/*
TileAlgebra includes

*/

#include "../Constants.h"
#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct valInfo describes name, syntax, meaning and signature
of TileAlgebra operator val.

author: Dirk Zacher

*/

struct valInfo : OperatorInfo
{
  valInfo()
  {
    name      = "val";
    syntax    = "val(_)";
    meaning   = "Returns the t type value of a it type.";

    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string(TYPE_NAME_PREFIX_IT) + "T" +
                RIGHT_ARROW + TYPE_NAME_PREFIX_T + "T" +
                FOR + GetTypeParametersDomain(typeParameterT,
                                              valueWrapperTypes);
  }
};

/*
declaration of valFunctions array.

*/

extern ValueMapping valFunctions[];

/*
Method valSelectFunction returns the index of specific val function
in valFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of val operator
return value: index of specific val function in valFunctions
exceptions: -

*/

int valSelectFunction(ListExpr arguments);

/*
Method valTypeMappingFunction returns the return value type
of val operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of val operator
return value: return value type of val operator
exceptions: -

*/

ListExpr valTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_VAL_H
