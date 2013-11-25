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

#ifndef TILEALGEBRA_DEFTIME_H
#define TILEALGEBRA_DEFTIME_H

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

#include "../Constants.h"
#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct deftimeInfo describes name, syntax, meaning and signature
of TileAlgebra operator deftime.

author: Dirk Zacher

*/

struct deftimeInfo : OperatorInfo
{
  deftimeInfo()
  {
    name      = "deftime";
    syntax    = "deftime(_)";
    meaning   = "Returns the defined periods.";

    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string(TYPE_NAME_PREFIX_MT) + "T" +
                RIGHT_ARROW + Periods::BasicType() +
                FOR + GetTypeParametersDomain(typeParameterT,
                                              valueWrapperTypes);
  }
};

/*
declaration of deftimeFunctions array.

*/

extern ValueMapping deftimeFunctions[];

/*
Method deftimeSelectFunction returns the index of specific deftime function
in deftimeFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of deftime operator
return value: index of specific deftime function in deftimeFunctions
exceptions: -

*/

int deftimeSelectFunction(ListExpr arguments);

/*
Method deftimeTypeMappingFunction returns the return value type
of deftime operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of deftime operator
return value: return value type of deftime operator
exceptions: -

*/

ListExpr deftimeTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_DEFTIME_H
