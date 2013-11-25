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

#ifndef TILEALGEBRA_ATLOCATION_H
#define TILEALGEBRA_ATLOCATION_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "DateTime.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "Point.h"

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
Struct atlocationInfo describes name, syntax, meaning and signature
of TileAlgebra operator atlocation.

author: Dirk Zacher

*/

struct atlocationInfo : OperatorInfo
{
  atlocationInfo()
  {
    name      = "atlocation";
    syntax    = "_ atlocation [_, _]";
    meaning   = "Returns the value(s) at location point.";

    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string(TYPE_NAME_PREFIX_T) + "T" + " x " +
                Point::BasicType() +
                RIGHT_ARROW + "T" +
                FOR + GetTypeParametersDomain(typeParameterT,
                                              valueWrapperTypes);
    appendSignature(std::string(TYPE_NAME_PREFIX_MT) + "T" + " x " +
                    Point::BasicType() +
                    RIGHT_ARROW + TYPE_NAME_PREFIX_M + "T" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
    appendSignature(std::string(TYPE_NAME_PREFIX_MT) + "T" + " x " +
                    Point::BasicType() + " x " +
                    Instant::BasicType() +
                    RIGHT_ARROW + "T" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
  }
};

/*
declaration of atlocationFunctions array.

*/

extern ValueMapping atlocationFunctions[];

/*
Method atlocationSelectFunction returns the index of specific atlocation
function in atlocationFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of atlocation operator
return value: index of specific atlocation function in atlocationFunctions
exceptions: -

*/

int atlocationSelectFunction(ListExpr arguments);

/*
Method atlocationTypeMappingFunction returns the return value type
of atlocation function in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of atlocation operator
return value: return value type of atlocation operator
exceptions: -

*/

ListExpr atlocationTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_ATLOCATION_H
