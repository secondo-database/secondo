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

#ifndef TILEALGEBRA_VAL_H
#define TILEALGEBRA_VAL_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "../Types/Types.h"

namespace TileAlgebra
{

/*
definition of val Operator Info structure

*/

struct valInfo : OperatorInfo
{
  valInfo()
  {
    name      = "val";
    syntax    = "val(_)";
    meaning   = "Returns the t type value of a it type.";

    std::vector<std::string> tTypes;
    std::vector<std::string> itTypes;
    GettTypes(tTypes);
    GetitTypes(itTypes);

    if(tTypes.size() == itTypes.size())
    {
      for(size_t i = 0; i < itTypes.size(); i++)
      {
        if(signature.empty())
        {
          signature = itTypes[i] + " -> " + tTypes[i];
        }

        else
        {
          appendSignature(itTypes[i] + " -> " + tTypes[i]);
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
declaration of val functions

*/

extern ValueMapping valFunctions[];

/*
declaration of val select function

*/

int valSelectFunction(ListExpr arguments);

/*
declaration of val type mapping function

*/

ListExpr valTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_VAL_H
