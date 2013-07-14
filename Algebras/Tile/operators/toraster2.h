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
#include "../Types.h"

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
    meaning   = "Converts a stream of Tile Algebra t type objects or "
                "mt type objects to a Raster2 Algebra s type object or "
                "ms type object.";

    
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    std::vector<std::string> sTypes;
    std::vector<std::string> msTypes;
    GettTypes(tTypes);
    GetmtTypes(mtTypes);
    GetsTypes(sTypes);
    GetmsTypes(msTypes);

    if(tTypes.size() == mtTypes.size() &&
       tTypes.size() == sTypes.size() &&
       tTypes.size() == msTypes.size())
    { 
      for(size_t i = 0; i < tTypes.size(); i++)
      {
        if(signature.empty())
        {
          signature = Stream<Attribute>::BasicType() +
                      "(" + tTypes[i] + ") -> " + sTypes[i];
        }

        else
        {
          appendSignature(Stream<Attribute>::BasicType() +
                          "(" + tTypes[i] + ") -> " + sTypes[i]);
        } 
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(Stream<Attribute>::BasicType() +
                        "(" + mtTypes[i] + ") -> " + msTypes[i]);
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
