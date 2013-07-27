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

#ifndef TILEALGEBRA_FROMREGION_H
#define TILEALGEBRA_FROMREGION_H

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

#include "../grid/tgrid.h"
#include "../t/tbool.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Struct fromregionInfo describes name, syntax, meaning and signature
of TileAlgebra operator fromregion.

author: Dirk Zacher

*/

struct fromregionInfo : OperatorInfo
{
  fromregionInfo()
  {
    name      = "fromregion";
    syntax    = "fromregion(_,_)";
    meaning   = "Creates a stream of tbool objects from a region and a grid.";
    signature = Region::BasicType() + " x " + tgrid::BasicType() +
                " -> " + Stream<tbool>::BasicType() +
                "(" + tbool::BasicType() + ")";
  }
};

/*
Method fromregionFunction implements the fromregion operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of fromregion operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of fromregionFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if fromregionFunction successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

int fromregionFunction(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier);

/*
Method fromregionTypeMappingFunction returns the return value type
of fromregion operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of fromregion operator
return value: return value type of fromregion operator
exceptions: -

*/

ListExpr fromregionTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_FROMREGION_H
