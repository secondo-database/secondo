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

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "DateTime.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"

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
Struct atrangeInfo describes name, syntax, meaning and signature
of TileAlgebra operator atrange.

author: Dirk Zacher

*/

struct atrangeInfo : OperatorInfo
{
  atrangeInfo()
  {
    name      = "atrange";
    syntax    = "_ atrange [_,_,_]";
    meaning   = "Returns the values at range of the rectangle.";

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
                Rectangle<2>::BasicType() +
                RIGHT_ARROW + "xT" +
                FOR + GetTypeParametersDomain(typeParameterx,
                                              tileAlgebraTypes) +
                ", " + GetTypeParametersDomain(typeParameterT,
                                               valueWrapperTypes);
    appendSignature(std::string(TYPE_NAME_PREFIX_MT) + "T" + " x " +
                    Rectangle<2>::BasicType() + " x " +
                    Instant::BasicType() + " x " +
                    Instant::BasicType() +
                    RIGHT_ARROW + TYPE_NAME_PREFIX_MT + "T" +
                    FOR + GetTypeParametersDomain(typeParameterT,
                                                  valueWrapperTypes));
  }
};

/*
declaration of atrangeFunctions array.

*/

extern ValueMapping atrangeFunctions[];

/*
Method atrangeSelectFunction returns the index of specific atrange function
in atrangeFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of atrange operator
return value: index of specific atrange function in atrangeFunctions
exceptions: -

*/

int atrangeSelectFunction(ListExpr arguments);

/*
Method atrangeTypeMappingFunction returns the return value type
of atrange operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of atrange operator
return value: return value type of atrange operator
exceptions: -

*/

ListExpr atrangeTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_ATRANGE_H
