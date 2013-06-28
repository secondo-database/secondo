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

#ifndef TILEALGEBRA_INST_H
#define TILEALGEBRA_INST_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "DateTime.h"
#include "../Types.h"

namespace TileAlgebra
{

/*
definition of inst Operator Info structure

*/

struct instInfo : OperatorInfo
{
  instInfo()
  {
    name      = "inst";
    syntax    = "inst(_)";
    meaning   = "Returns the instant value.";

    std::vector<std::string> itTypes;
    GetitTypes(itTypes);

    for(size_t i = 0; i < itTypes.size(); i++)
    {
      if(signature.empty())
      {
        signature = itTypes[i] + " -> " + Instant::BasicType();
      }

      else
      {
        appendSignature(itTypes[i] + " -> " + Instant::BasicType());
      }
    }
  }
};

/*
declaration of inst functions

*/

extern ValueMapping instFunctions[];

/*
declaration of inst select function

*/

int instSelectFunction(ListExpr arguments);

/*
declaration of inst type mapping function

*/

ListExpr instTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_INST_H
