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

#ifndef TILEALGEBRA_CELLS_H
#define TILEALGEBRA_CELLS_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
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
Struct CELLSInfo describes name, syntax, meaning and signature
of TileAlgebra operator CELLS.

author: Dirk Zacher

*/

struct CELLSInfo : OperatorInfo
{
  CELLSInfo()
  {
    name      = "CELLS";
    syntax    = "Not available";
    meaning   = "Type mapping operator.";

    std::vector<std::string> typeParameterx;
    typeParameterx.push_back("x");
    std::vector<std::string> tileAlgebraTypes;
    tileAlgebraTypes.push_back(TYPE_NAME_PREFIX_T);
    tileAlgebraTypes.push_back(TYPE_NAME_PREFIX_MT);
    std::vector<std::string> typeParameterT;
    typeParameterT.push_back("T");
    std::vector<std::string> valueWrapperTypes;
    GetValueWrapperTypes(valueWrapperTypes);

    signature = std::string("xT") + " x " +
                "..." +
                RIGHT_ARROW + "rel(tuple([Elem : T]))" +
                FOR + GetTypeParametersDomain(typeParameterx,
                                              tileAlgebraTypes) +
                ", " + GetTypeParametersDomain(typeParameterT,
                                               valueWrapperTypes);
  }
};

/*
Method CELLSTypeMappingFunction returns the return value type
of CELLS operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of CELLS operator
return value: return value type of CELLS operator
exceptions: -

*/

ListExpr CELLSTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_CELLS_H
