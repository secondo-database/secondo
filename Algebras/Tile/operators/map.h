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

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "../Types.h"

namespace TileAlgebra
{

/*
definition of map Operator Info structure

*/

struct mapInfo : OperatorInfo
{
  mapInfo()
  {
    name      = "map";
    syntax    = "_ map[_]";
    meaning   = "Maps a xT type to a xU type.";

    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;

    GetValueWrapperTypes(valueWrapperTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);

    if(valueWrapperTypes.size() == tTypes.size() &&
       valueWrapperTypes.size() == mtTypes.size())
    {
      for(size_t i = 0; i < tTypes.size(); i++)
      {
        for(size_t j = 0; j < valueWrapperTypes.size(); j++)
        {
          if(signature.empty())
          {
            signature = tTypes[i] + " x (" +
                        valueWrapperTypes[i] + " -> " +
                        valueWrapperTypes[j] + ") -> " + tTypes[j];
          }

          else
          {
            appendSignature(tTypes[i] + " x (" +
                            valueWrapperTypes[i] + " -> " +
                            valueWrapperTypes[j] + ") -> " + tTypes[j]);
          }
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        for(size_t j = 0; j < valueWrapperTypes.size(); j++)
        {
          appendSignature(mtTypes[i] + " x (" +
                          valueWrapperTypes[i] + " -> " +
                          valueWrapperTypes[j] + ") -> " + mtTypes[j]);
        }
      }
    }

    else
    {
      assert(false);
    }
  }
};

/*
declaration of map functions

*/

extern ValueMapping mapFunctions[];

/*
declaration of map select function

*/

int mapSelectFunction(ListExpr arguments);

/*
declaration of map type mapping function

*/

ListExpr mapTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MAP_H
