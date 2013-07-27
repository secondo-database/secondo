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

#ifndef TILEALGEBRA_MINIMUM_H
#define TILEALGEBRA_MINIMUM_H

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
Struct minimumInfo describes name, syntax, meaning and signature
of TileAlgebra operator minimum.

author: Dirk Zacher

*/

struct minimumInfo : OperatorInfo
{
  minimumInfo()
  {
    name      = "minimum";
    syntax    = "minimum(_)";
    meaning   = "Returns the minimum value.";

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
        if(signature.empty())
        {
          signature = tTypes[i] + " -> " + valueWrapperTypes[i];
        }

        else
        {
          appendSignature(tTypes[i] + " -> " + valueWrapperTypes[i]);
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " -> " + valueWrapperTypes[i]);
      }
    }

    else
    {
      assert(false);
    }
  }
};

/*
declaration of minimumFunctions array.

*/

extern ValueMapping minimumFunctions[];

/*
Method minimumSelectFunction returns the index of specific minimum function
in minimumFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of minimum operator
return value: index of specific minimum function in minimumFunctions
exceptions: -

*/

int minimumSelectFunction(ListExpr arguments);

/*
Method minimumTypeMappingFunction returns the return value type
of minimum operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of minimum operator
return value: return value type of minimum operator
exceptions: -

*/

ListExpr minimumTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MINIMUM_H
