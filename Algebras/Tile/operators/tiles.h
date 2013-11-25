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

#ifndef TILEALGEBRA_TILES_H
#define TILEALGEBRA_TILES_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Attribute.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "Stream.h"

/*
Raster2Algebra includes

*/

#include "../../Raster2/grid2.h"
#include "../../Raster2/grid3.h"

/*
TileAlgebra includes

*/

#include "../Constants.h"
#include "../grid/tgrid.h"
#include "../grid/mtgrid.h"
#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct tilesInfo describes name, syntax, meaning and signature
of TileAlgebra operator tiles.

author: Dirk Zacher

*/

struct tilesInfo : OperatorInfo
{
  tilesInfo()
  {
    name      = "tiles";
    syntax    = "tiles(_)";
    meaning   = "Converts a Raster2 Algebra grid2 object to "
                "a Tile Algebra tgrid object, "
                "a Raster2 Algebra grid3 object to "
                "a Tile Algebra mtgrid object, "
                "a Raster2 Algebra s type object to "
                "a stream of Tile Algebra t type objects, "
                "a Raster2 Algebra ms type object to "
                "a stream of Tile Algebra mt type objects or "
                "a Raster 2 Algebra is type object to "
                "a stream of Tile Algebra it type objects.";
    
    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = raster2::grid2::BasicType() +
                RIGHT_ARROW + tgrid::BasicType();
    appendSignature(raster2::grid3::BasicType() +
                    RIGHT_ARROW + mtgrid::BasicType());
    appendSignature(std::string(TYPE_NAME_PREFIX_S) + "T" +
                    RIGHT_ARROW + Stream<Attribute>::BasicType() +
                    "(" + TYPE_NAME_PREFIX_T + "T)" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
    appendSignature(std::string(TYPE_NAME_PREFIX_MS) + "T" +
                    RIGHT_ARROW + Stream<Attribute>::BasicType() +
                    "(" + TYPE_NAME_PREFIX_MT + "T)" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
    appendSignature(std::string(TYPE_NAME_PREFIX_IS) + "T" +
                    RIGHT_ARROW + Stream<Attribute>::BasicType() +
                    "(" + TYPE_NAME_PREFIX_IT + "T)" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
  }
};

/*
declaration of tilesFunctions array.

*/

extern ValueMapping tilesFunctions[];

/*
Method tilesSelectFunction returns the index of specific tiles function
in tilesFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of tiles operator
return value: index of specific tiles function in tilesFunctions
exceptions: -

*/

int tilesSelectFunction(ListExpr arguments);

/*
Method tilesTypeMappingFunction returns the return value type
of tiles operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of tiles operator
return value: return value type of tiles operator
exceptions: -

*/

ListExpr tilesTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_TILES_H
