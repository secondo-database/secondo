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

#ifndef TILEALGEBRA_T2MT_H
#define TILEALGEBRA_T2MT_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "DateTime.h"
#include "../Types.h"

namespace TileAlgebra
{

/*
definition of t2mt Operator Info structure

*/

struct t2mtInfo : OperatorInfo
{
  t2mtInfo()
  {
    name      = "t2mt";
    syntax    = "t2mt(_, _, _, _)";
    meaning   = "Adds a time component to a t type.";

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
          signature = tTypes[i] + " x " +
                      Duration::BasicType() + " x " +
                      Instant::BasicType() + " x " +
                      Instant::BasicType() +
                      " -> " + mtTypes[i];
        }

        else
        {
          appendSignature(tTypes[i] + " x " +
                          Duration::BasicType() + " x " +
                          Instant::BasicType() + " x " +
                          Instant::BasicType() +
                          " -> " + mtTypes[i]);
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
declaration of t2mt functions

*/

extern ValueMapping t2mtFunctions[];

/*
declaration of t2mt select function

*/

int t2mtSelectFunction(ListExpr arguments);

/*
declaration of t2mt type mapping function

*/

ListExpr t2mtTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_T2MT_H
