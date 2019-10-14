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

#ifndef TILEALGEBRA_TORASTER2_H
#define TILEALGEBRA_TORASTER2_H

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
Struct toraster2Info describes name, syntax, meaning and signature
of TileAlgebra operator toraster2.

author: Dirk Zacher

*/

struct toraster2Info : OperatorInfo
{
  toraster2Info()
  {
    name      = "toraster2";
    syntax    = "toraster2(_)";
    meaning   = "Converts a Tile Algebra tgrid object to "
                "a Raster2 Algebra grid2 object, "
                "a Tile Algebra mtgrid object to "
                "a Raster2 Algebra grid3 object, "
                "a stream of Tile Algebra t type objects to "
                "a Raster2 Algebra s type object, "
                "a stream of Tile Algebra mt type objects to "
                "a Raster2 Algebra ms type object or "
                "a stream of Tile Algebra it type objects to "
                "a Raster2 Algebra is type object.";
    
    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = tgrid::BasicType() +
                RIGHT_ARROW + raster2::grid2::BasicType();
    appendSignature(mtgrid::BasicType() +
                    RIGHT_ARROW + raster2::grid3::BasicType());
    appendSignature(Stream<Attribute>::BasicType() +
                    "(" + TYPE_NAME_PREFIX_T + "T)" +
                    RIGHT_ARROW + TYPE_NAME_PREFIX_S + "T" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
    appendSignature(Stream<Attribute>::BasicType() +
                    "(" + TYPE_NAME_PREFIX_MT + "T)" +
                    RIGHT_ARROW + TYPE_NAME_PREFIX_MS + "T" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
    appendSignature(Stream<Attribute>::BasicType() +
                    "(" + TYPE_NAME_PREFIX_IT + "T)" +
                    RIGHT_ARROW + TYPE_NAME_PREFIX_IS + "T" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
  }
};

/*
declaration of toraster2Functions array.

*/

extern ValueMapping toraster2Functions[];

/*
Method toraster2SelectFunction returns the index of specific toraster2 function
in toraster2Functions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of toraster2 operator
return value: index of specific toraster2 function in toraster2Functions
exceptions: -

*/

int toraster2SelectFunction(ListExpr arguments);

/*
Method toraster2TypeMappingFunction returns the return value type
of toraster2 operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of toraster2 operator
return value: return value type of toraster2 operator
exceptions: -

*/

ListExpr toraster2TypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_TORASTER2_H
