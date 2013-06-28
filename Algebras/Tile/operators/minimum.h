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

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "../Types.h"

namespace TileAlgebra
{

/*
definition of minimum Operator Info structure

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
declaration of minimum functions

*/

extern ValueMapping minimumFunctions[];

/*
declaration of minimum select function

*/

int minimumSelectFunction(ListExpr arguments);

/*
declaration of minimum type mapping function

*/

ListExpr minimumTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MINIMUM_H
