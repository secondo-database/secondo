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

#ifndef TILEALGEBRA_MAP2_H
#define TILEALGEBRA_MAP2_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"

/*
TileAlgebra includes

*/

#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct map2Info describes name, syntax, meaning and signature
of TileAlgebra operator map2.

author: Dirk Zacher

*/

struct map2Info : OperatorInfo
{
  map2Info()
  {
    name      = "map2";
    syntax    = "_ _ map2[_]";
    meaning   = "Combines two tile types to a new tile type.";

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
        for(size_t j = 0; j < tTypes.size(); j++)
        {
          for(size_t k = 0; k < valueWrapperTypes.size(); k++)
          {
            if(signature.empty())
            {
              signature = tTypes[i] + " x " + tTypes[j] + " x (" +
                          valueWrapperTypes[i] + " x " +
                          valueWrapperTypes[j] + " -> " +
                          valueWrapperTypes[k] + ") -> " + tTypes[k];
            }

            else
            {
              appendSignature(tTypes[i] + " x " + tTypes[j] + " x (" +
                              valueWrapperTypes[i] + " x " +
                              valueWrapperTypes[j] + " -> " +
                              valueWrapperTypes[k] + ") -> " + tTypes[k]);
            }
          }
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        for(size_t j = 0; j < tTypes.size(); j++)
        {
          for(size_t k = 0; k < valueWrapperTypes.size(); k++)
          {
            appendSignature(mtTypes[i] + " x " + tTypes[j] + " x (" +
                            valueWrapperTypes[i] + " x " +
                            valueWrapperTypes[j] + " -> " +
                            valueWrapperTypes[k] + ") -> " + mtTypes[k]);
          }
        }
      }

      for(size_t i = 0; i < tTypes.size(); i++)
      {
        for(size_t j = 0; j < mtTypes.size(); j++)
        {
          for(size_t k = 0; k < valueWrapperTypes.size(); k++)
          {
            appendSignature(tTypes[i] + " x " + mtTypes[j] + " x (" +
                            valueWrapperTypes[i] + " x " +
                            valueWrapperTypes[j] + " -> " +
                            valueWrapperTypes[k] + ") -> " + mtTypes[k]);
          }
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        for(size_t j = 0; j < mtTypes.size(); j++)
        {
          for(size_t k = 0; k < valueWrapperTypes.size(); k++)
          {
            appendSignature(mtTypes[i] + " x " + mtTypes[j] + " x (" +
                            valueWrapperTypes[i] + " x " +
                            valueWrapperTypes[j] + " -> " +
                            valueWrapperTypes[k] + ") -> " + mtTypes[k]);
          }
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
declaration of map2Functions array.

*/

extern ValueMapping map2Functions[];

/*
Method map2SelectFunction returns the index of specific map2 function
in map2Functions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of map2 operator
return value: index of specific map2 function in map2Functions
exceptions: -

*/

int map2SelectFunction(ListExpr arguments);

/*
Method map2TypeMappingFunction returns the return value type
of map2 operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of map2 operator
return value: return value type of map2 operator
exceptions: -

*/

ListExpr map2TypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MAP2_H
