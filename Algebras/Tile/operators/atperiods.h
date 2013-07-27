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

#ifndef TILEALGEBRA_ATPERIODS_H
#define TILEALGEBRA_ATPERIODS_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "TemporalAlgebra.h"

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
Struct atperiodsInfo describes name, syntax, meaning and signature
of TileAlgebra operator atperiods.

author: Dirk Zacher

*/

struct atperiodsInfo : OperatorInfo 
{
  atperiodsInfo()
  { 
    name      = "atperiods";
    syntax    = "atperiods(_)";
    meaning   = "Restricts values to periods.";

    std::vector<std::string> mtTypes;
    GetmtTypes(mtTypes);

    for(size_t i = 0; i < mtTypes.size(); i++)
    {
      if(signature.empty())
      {
        signature = mtTypes[i] + " x " + Periods::BasicType() +
                    " -> " + mtTypes[i];
      }

      else
      {
        appendSignature(mtTypes[i] + " x " + Periods::BasicType() +
                        " -> " + mtTypes[i]);
      }
    }
  }          
};

/*
declaration of atperiodsFunctions array.

*/

extern ValueMapping atperiodsFunctions[];

/*
Method atperiodsSelectFunction returns the index of specific atperiods function
in atperiodsFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of atperiods operator
return value: index of specific atperiods function in atperiodsFunctions
exceptions: -

*/

int atperiodsSelectFunction(ListExpr arguments);

/*
Method atperiodsTypeMappingFunction returns the return value type
of atperiods operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of atperiods operator
return value: return value type of atperiods operator
exceptions: -

*/

ListExpr atperiodsTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_ATPERIODS_H
