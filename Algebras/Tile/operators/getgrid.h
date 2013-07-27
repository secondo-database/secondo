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

#ifndef TILEALGEBRA_GETGRID_H
#define TILEALGEBRA_GETGRID_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"

/*
TileAlgebra includes

*/

#include "../grid/tgrid.h"
#include "../grid/mtgrid.h"
#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct getgridInfo describes name, syntax, meaning and signature
of TileAlgebra operator getgrid.

author: Dirk Zacher

*/

struct getgridInfo : OperatorInfo
{
  getgridInfo()
  {
    name      = "getgrid";
    syntax    = "getgrid(_)";
    meaning   = "Returns the grid.";

    std::vector<std::string> tTypes;
    std::vector<std::string> mtTypes;
    GettTypes(tTypes);
    GetmtTypes(mtTypes);

    if(tTypes.size() == mtTypes.size())
    {
      for(size_t i = 0; i < tTypes.size(); i++)
      {
        if(signature.empty())
        {
          signature = tTypes[i] + " -> " + tgrid::BasicType();
        }

        else
        {
          appendSignature(tTypes[i] + " -> " + tgrid::BasicType());
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " -> " + mtgrid::BasicType());
      }
    }

    else
    {
      assert(false);
    }    
  }
};

/*
declaration of getgridFunctions array.

*/

extern ValueMapping getgridFunctions[];

/*
Method getgridSelectFunction returns the index of specific getgrid function
in getgridFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of getgrid operator
return value: index of specific getgrid function in getgridFunctions
exceptions: -

*/

int getgridSelectFunction(ListExpr arguments);

/*
Method getgridTypeMappingFunction returns the return value type
of getgrid operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of getgrid operator
return value: return value type of getgrid operator
exceptions: -

*/

ListExpr getgridTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_GETGRID_H
