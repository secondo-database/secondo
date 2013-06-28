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

#ifndef TILEALGEBRA_ATRANGE_H
#define TILEALGEBRA_ATRANGE_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "DateTime.h"
#include "RectangleAlgebra.h"
#include "../Types.h"

namespace TileAlgebra
{

/*
definition of atrange Operator Info structure

*/

struct atrangeInfo : OperatorInfo
{
  atrangeInfo()
  {
    name      = "atrange";
    syntax    = "_ atrange [_,_,_]";
    meaning   = "Returns the values at range of the rectangle.";

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
          signature = tTypes[i] + " x " + Rectangle<2>::BasicType() +
                      " -> " + tTypes[i];
        }

        else
        {
          appendSignature(tTypes[i] + " x " + Rectangle<2>::BasicType() +
                          " -> " + tTypes[i]);
        }
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " x " + Rectangle<2>::BasicType() +
                        " -> " + mtTypes[i]);
      }

      for(size_t i = 0; i < mtTypes.size(); i++)
      {
        appendSignature(mtTypes[i] + " x " +
                        Rectangle<2>::BasicType() + " x " +
                        Instant::BasicType() + " x " +
                        Instant::BasicType() +
                        " -> " + mtTypes[i]);
      }
    }

    else
    {
      assert(false);
    }
  }
};

/*
declaration of atrange functions

*/

extern ValueMapping atrangeFunctions[];

/*
declaration of atrange select function

*/

int atrangeSelectFunction(ListExpr arguments);

/*
declaration of atrange type mapping function

*/

ListExpr atrangeTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_ATRANGE_H
