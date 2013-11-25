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

#ifndef TILEALGEBRA_FROMLINE_H
#define TILEALGEBRA_FROMLINE_H

/*
SECONDO includes

*/

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "SpatialAlgebra.h"
#include "Stream.h"

/*
TileAlgebra includes

*/

#include "../Constants.h"
#include "../grid/tgrid.h"
#include "../t/tbool.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct fromlineInfo describes name, syntax, meaning and signature
of TileAlgebra operator fromline.

author: Dirk Zacher

*/

struct fromlineInfo : OperatorInfo
{
  fromlineInfo()
  {
    name      = "fromline";
    syntax    = "fromline(_,_)";
    meaning   = "Creates a stream of tbool objects from a line and a grid.";
    signature = Line::BasicType() + " x " +
                tgrid::BasicType() +
                RIGHT_ARROW + Stream<tbool>::BasicType() +
                "(" + tbool::BasicType() + ")";
   }
};

/*
Method fromlineFunction implements the fromline operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of fromline operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of fromlineFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if fromlineFunction successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

int fromlineFunction(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier);

/*
Method fromlineTypeMappingFunction returns the return value type
of fromline operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of fromline operator
return value: return value type of fromline operator
exceptions: -

*/

ListExpr fromlineTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_FROMLINE_H
