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
definition of toraster2 Operator Info structure

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
      signature = tgrid::BasicType() +
                  " -> " + raster2::grid2::BasicType();

      appendSignature(mtgrid::BasicType() +
                      " -> " + raster2::grid3::BasicType());

      for(size_t i = 0; i < tTypes.size(); i++)
      {
        appendSignature(Stream<Attribute>::BasicType() +
                        "(" + tTypes[i] + ") -> " + sTypes[i]);
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(Stream<Attribute>::BasicType() +
                        "(" + mtTypes[i] + ") -> " + msTypes[i]);
      }

      for(size_t i = 0; i < itTypes.size(); i++)
      {
        appendSignature(Stream<Attribute>::BasicType() +
                        "(" + itTypes[i] + ") -> " + isTypes[i]);
      }
    }

    else
    {
      assert(false);
    }
  }
};

/*
declaration of toraster2 functions

*/

extern ValueMapping toraster2Functions[];

/*
declaration of toraster2 select function

*/

int toraster2SelectFunction(ListExpr arguments);

/*
declaration of toraster2 type mapping function

*/

ListExpr toraster2TypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_TORASTER2_H
