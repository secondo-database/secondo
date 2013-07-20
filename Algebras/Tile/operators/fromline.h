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

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "SpatialAlgebra.h"
#include "Stream.h"
#include "../grid/tgrid.h"
#include "../t/tbool.h"

namespace TileAlgebra
{

/*
definition of fromline Operator Info structure

*/

struct fromlineInfo : OperatorInfo
{
  fromlineInfo()
  {
    name      = "fromline";
    syntax    = "fromline(_,_)";
    meaning   = "Creates a stream of tbool objects from a line and a grid.";
    signature = Line::BasicType() + " x " + tgrid::BasicType() +
                " -> " + Stream<tbool>::BasicType() +
                "(" + tbool::BasicType() + ")";
   }
};

/*
declaration of fromline function

*/

int fromlineFunction(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier);

/*
declaration of fromline type mapping function

*/

ListExpr fromlineTypeMappingFunction(ListExpr arguments);

}

#endif // TILEALGEBRA_FROMLINE_H
