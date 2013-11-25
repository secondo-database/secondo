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

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "DateTime.h"
#include "Operator.h"
#include "QueryProcessor.h"

/*
TileAlgebra includes

*/

#include "../Constants.h"
#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct instInfo describes name, syntax, meaning and signature
of TileAlgebra operator inst.

author: Dirk Zacher

*/

struct instInfo : OperatorInfo
{
  instInfo()
  {
    name      = "inst";
    syntax    = "inst(_)";
    meaning   = "Returns the instant value.";

    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string(TYPE_NAME_PREFIX_IT) + "T" +
                RIGHT_ARROW + Instant::BasicType() +
                FOR + GetTypeParametersDomain(typeParameterT,
                                              valueWrapperTypes);
  }
};

/*
declaration of instFunctions array.

*/

extern ValueMapping instFunctions[];

/*
Method instSelectFunction returns the index of specific inst function
in instFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of inst operator
return value: index of specific inst function in instFunctions
exceptions: -

*/

int instSelectFunction(ListExpr arguments);

/*
Method instTypeMappingFunction returns the return value type
of inst operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of inst operator
return value: return value type of inst operator
exceptions: -

*/

ListExpr instTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_INST_H
