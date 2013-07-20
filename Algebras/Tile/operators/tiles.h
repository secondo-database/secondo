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

#include "AlgebraTypes.h"
#include "Attribute.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "Stream.h"
#include "../Types/Types.h"
#include "../grid/tgrid.h"
#include "../grid/mtgrid.h"
#include "../../Raster2/grid2.h"
#include "../../Raster2/grid3.h"

namespace TileAlgebra
{

/*
definition of tiles Operator Info structure

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
    
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> itTypes;
    std::vector<std::string> sTypes;
    std::vector<std::string> msTypes;
    std::vector<std::string> isTypes;
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetitTypes(itTypes);
    GetsTypes(sTypes);
    GetmsTypes(msTypes);
    GetisTypes(isTypes);

    if(tTypes.size() == mtTypes.size() &&
       tTypes.size() == itTypes.size() &&
       tTypes.size() == sTypes.size() &&
       tTypes.size() == msTypes.size() &&
       tTypes.size() == isTypes.size())
    { 
      signature = raster2::grid2::BasicType() +
                  " -> " + tgrid::BasicType();

      appendSignature(raster2::grid3::BasicType() +
                      " -> " + mtgrid::BasicType());

      for(size_t i = 0; i < tTypes.size(); i++)
      {
        appendSignature(sTypes[i] + " -> " + Stream<Attribute>::BasicType() +
                        "(" + tTypes[i] + ")");
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(msTypes[i] + " -> " + Stream<Attribute>::BasicType() +
                        "(" + mtTypes[i] + ")");
      }

      for(size_t i = 0; i < itTypes.size(); i++)
      {
        appendSignature(isTypes[i] + " -> " + Stream<Attribute>::BasicType() +
                        "(" + itTypes[i] + ")");
      }
    }

    else
    {
      assert(false);
    }
  }
};

/*
declaration of tiles functions

*/

extern ValueMapping tilesFunctions[];

/*
declaration of tiles select function

*/

int tilesSelectFunction(ListExpr arguments);

/*
declaration of tiles type mapping function

*/

ListExpr tilesTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_TILES_H
