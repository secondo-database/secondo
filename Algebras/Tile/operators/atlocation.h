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

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "DateTime.h"
#include "Point.h"
#include "../Types/Types.h"

namespace TileAlgebra
{

/*
definition of atlocation Operator Info structure

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
declaration of atlocation functions

*/

extern ValueMapping atlocationFunctions[];

/*
declaration of atlocation select function

*/

int atlocationSelectFunction(ListExpr arguments);

/*
declaration of atlocation type mapping function

*/

ListExpr atlocationTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_ATLOCATION_H
