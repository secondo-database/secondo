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

#ifndef TILEALGEBRA_MAP_H
#define TILEALGEBRA_MAP_H

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
Struct mapInfo describes name, syntax, meaning and signature
of TileAlgebra operator map.

author: Dirk Zacher

*/

struct mapInfo : OperatorInfo
{
  mapInfo()
  {
    name      = "map";
    syntax    = "_ map[_]";
    meaning   = "Maps a xT type to a xU type.";

    std::vector<std::string> typeParameterx;
    typeParameterx.push_back("x");
    std::vector<std::string> tileAlgebraTypes;
    tileAlgebraTypes.push_back(TYPE_NAME_PREFIX_T);
    tileAlgebraTypes.push_back(TYPE_NAME_PREFIX_MT);
    std::vector<std::string> typeParameterTU;
    typeParameterTU.push_back("T");
    typeParameterTU.push_back("U");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string("xT") + " x " +
                "(T" + RIGHT_ARROW + "U)" +
                RIGHT_ARROW + "xU" +
                FOR + GetTypeParametersDomain(typeParameterx,
                                              tileAlgebraTypes) +
                ", " + GetTypeParametersDomain(typeParameterTU,
                                               valueWrapperTypes);
  }
};

/*
declaration of mapFunctions array.

*/

extern ValueMapping mapFunctions[];

/*
Method mapSelectFunction returns the index of specific map function
in mapFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of map operator
return value: index of specific map function in mapFunctions
exceptions: -

*/

int mapSelectFunction(ListExpr arguments);

/*
Method mapTypeMappingFunction returns the return value type
of map operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of map operator
return value: return value type of map operator
exceptions: -

*/

ListExpr mapTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MAP_H
