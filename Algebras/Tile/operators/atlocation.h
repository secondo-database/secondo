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

    std::vector<std::string> valueWrapperTypes;
    std::vector<std::string> MTypes;
    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    GetValueWrapperTypes(valueWrapperTypes);
    GetMTypes(MTypes);
    GettTypes(tTypes);
    GetmtTypes(mtTypes);

    if(valueWrapperTypes.size() == MTypes.size() &&
       valueWrapperTypes.size() == tTypes.size() &&
       valueWrapperTypes.size() == mtTypes.size())
    {
      for(size_t i = 0; i < tTypes.size(); i++)
      {
        if(signature.empty())
        {
          signature = tTypes[i] + " x " + Point::BasicType() +
                      " -> " + valueWrapperTypes[i];
        }

        else
        {
          appendSignature(tTypes[i] + " x " + Point::BasicType() +
                          " -> " + valueWrapperTypes[i]);
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " x " + Point::BasicType() +
                        " -> " + MTypes[i]);
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " x " + Point::BasicType() + " x " +
                        Instant::BasicType() + " -> " + valueWrapperTypes[i]);
      }
    }

    else
    {
      assert(false);
    }
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
