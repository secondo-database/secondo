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

#ifndef TILEALGEBRA_BBOX_H
#define TILEALGEBRA_BBOX_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "../Types/Types.h"

namespace TileAlgebra
{

/*
definition of bbox Operator Info structure

*/

struct bboxInfo : OperatorInfo
{
  bboxInfo()
  {
    name      = "bbox";
    syntax    = "bbox(_)";
    meaning   = "Returns the bounding box rectangle.";

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
          signature = tTypes[i] + " -> " + Rectangle<2>::BasicType();
        }

        else
        {
          appendSignature(tTypes[i] + " -> " + Rectangle<2>::BasicType());
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " -> " + Rectangle<3>::BasicType());
      }
    }

    else
    {
      assert(false);
    }
  }
};

/*
declaration of bbox functions

*/

extern ValueMapping bboxFunctions[];

/*
declaration of bbox select function

*/

int bboxSelectFunction(ListExpr arguments);

/*
declaration of bbox type mapping function

*/

ListExpr bboxTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_BBOX_H
