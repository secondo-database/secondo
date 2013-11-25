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
Struct t2mtInfo describes name, syntax, meaning and signature
of TileAlgebra operator t2mt.

author: Dirk Zacher

*/

struct t2mtInfo : OperatorInfo
{
  t2mtInfo()
  {
    name      = "t2mt";
    syntax    = "t2mt(_, _, _, _)";
    meaning   = "Adds a time component to a t type.";

    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string(TYPE_NAME_PREFIX_T) + "T" + " x " +
                Duration::BasicType() + " x " +
                Instant::BasicType() + " x " +
                Instant::BasicType() +
                RIGHT_ARROW + TYPE_NAME_PREFIX_MT + "T" +
                FOR + GetTypeParametersDomain(typeParameterT,
                                              valueWrapperTypes);
  }
};

/*
declaration of t2mtFunctions array.

*/

extern ValueMapping t2mtFunctions[];

/*
Method t2mtSelectFunction returns the index of specific t2mt function
in t2mtFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of t2mt operator
return value: index of specific t2mt function in t2mtFunctions
exceptions: -

*/

int t2mtSelectFunction(ListExpr arguments);

/*
Method t2mtTypeMappingFunction returns the return value type
of t2mt operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of t2mt operator
return value: return value type of t2mt operator
exceptions: -

*/

ListExpr t2mtTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_T2MT_H
