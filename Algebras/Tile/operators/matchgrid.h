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

#ifndef TILEALGEBRA_MATCHGRID_H
#define TILEALGEBRA_MATCHGRID_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

/*
TileAlgebra includes

*/

#include "../Constants.h"
#include "../grid/tgrid.h"
#include "../grid/mtgrid.h"
#include "../Types/Types.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct matchgridInfo describes name, syntax, meaning and signature
of TileAlgebra operator matchgrid.

author: Dirk Zacher

*/

struct matchgridInfo : OperatorInfo
{
  matchgridInfo()
  {
    name      = "matchgrid";
    syntax    = "_ matchgrid [_, _, _]";
    meaning   = "Resamples a t type object or a mt type object "
                "by applying a user function.";

    std::vector<std::string> typeParameterTU;
    typeParameterTU.push_back("T");
    typeParameterTU.push_back("U");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string(TYPE_NAME_PREFIX_T) + "T" + " x " +
                tgrid::BasicType() + " x " +
                "(rel(tuple([Elem : T]))" + RIGHT_ARROW + "U)" + " x " +
                CcBool::BasicType() +
                RIGHT_ARROW + TYPE_NAME_PREFIX_T + "U" +
                FOR + GetTypeParametersDomain(typeParameterTU,
                                              valueWrapperTypes);
    appendSignature(std::string(TYPE_NAME_PREFIX_MT) + "T" + " x " +
                    mtgrid::BasicType() + " x " +
                    "(rel(tuple([Elem : T]))" + RIGHT_ARROW + "U)" + " x " +
                    CcBool::BasicType() +
                    RIGHT_ARROW + TYPE_NAME_PREFIX_MT + "U" +
                    FOR + GetTypeParametersDomain(typeParameterTU,
                                                  valueWrapperTypes));
  }
};

/*
declaration of matchgridFunctions array.

*/

extern ValueMapping matchgridFunctions[];

/*
Method matchgridSelectFunction returns the index of specific matchgrid function
in matchgridFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of matchgrid operator
return value: index of specific matchgrid function in matchgridFunctions
exceptions: -

*/

int matchgridSelectFunction(ListExpr arguments);

/*
Method matchgridTypeMappingFunction returns the return value type
of matchgrid operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of matchgrid operator
return value: return value type of matchgrid operator
exceptions: -

*/

ListExpr matchgridTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_MATCHGRID_H
