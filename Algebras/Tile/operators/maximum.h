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

#ifndef TILEALGEBRA_MAXIMUM_H
#define TILEALGEBRA_MAXIMUM_H

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
Struct maximumInfo describes name, syntax, meaning and signature
of TileAlgebra operator maximum.

author: Dirk Zacher

*/

struct maximumInfo : OperatorInfo
{
  maximumInfo()
  {
    name      = "maximum";
    syntax    = "maximum(_)";
    meaning   = "Returns the maximum value.";

    std::vector<std::string> typeParameterx;
    typeParameterx.push_back("x");
    std::vector<std::string> tileAlgebraTypes;
    tileAlgebraTypes.push_back(TYPE_NAME_PREFIX_T);
    tileAlgebraTypes.push_back(TYPE_NAME_PREFIX_MT);
    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string("xT") +
                RIGHT_ARROW + "T" +
                FOR + GetTypeParametersDomain(typeParameterx,
                                              tileAlgebraTypes) +
                ", " + GetTypeParametersDomain(typeParameterT,
                                               valueWrapperTypes);
  }
};

/*
declaration of maximumFunctions array.

*/

extern ValueMapping maximumFunctions[];

/*
Method maximumSelectFunction returns the index of specific maximum function
in maximumFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of maximum operator
return value: index of specific maximum function in maximumFunctions
exceptions: -

*/

int maximumSelectFunction(ListExpr arguments);

/*
Method maximumTypeMappingFunction returns the return value type
of maximum operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of maximum operator
return value: return value type of maximum operator
exceptions: -

*/

ListExpr maximumTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MAXIMUM_H
