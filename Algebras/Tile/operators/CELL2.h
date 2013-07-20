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

#ifndef TILEALGEBRA_CELL2_H
#define TILEALGEBRA_CELL2_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "../Types/Types.h"

namespace TileAlgebra
{

/*
definition of CELL2 Operator Info structure

*/

struct CELL2Info : OperatorInfo
{
  CELL2Info()
  {
    name      = "CELL2";
    syntax    = "Not available";
    meaning   = "Type mapping operator.";

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
          if(signature.empty())
          {
            signature = tTypes[i] + " x " + tTypes[j] +
                        " x ... -> " + valueWrapperTypes[j];
          }

          else
          {
            appendSignature(tTypes[i] + " x " + tTypes[j] +
                            " x ... -> " + valueWrapperTypes[j]);
          }
        }

        for(size_t j = 0; j < mtTypes.size(); j++)
        {
          appendSignature(tTypes[i] + " x " + mtTypes[j] +
                          " x ... -> " + valueWrapperTypes[j]);
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        for(size_t j = 0; j < tTypes.size(); j++)
        {
          appendSignature(mtTypes[i] + " x " + tTypes[j] +
                          " x ... -> " + valueWrapperTypes[j]);
        }

        for(size_t j = 0; j < mtTypes.size(); j++)
        {
          appendSignature(mtTypes[i] + " x " + mtTypes[j] +
                          " x ... -> " + valueWrapperTypes[j]);
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
declaration of CELL2 type mapping function

*/

ListExpr CELL2TypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_CELL2_H
