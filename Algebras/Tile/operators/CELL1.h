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

#ifndef TILEALGEBRA_CELL1_H
#define TILEALGEBRA_CELL1_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"

namespace TileAlgebra
{

/*
declaration of CELL1 type mapping function

*/

ListExpr CELL1TypeMappingFunction(ListExpr arguments);

/*
definition of CELL1 Operator Info structure

*/

struct CELL1Info : OperatorInfo
{
  CELL1Info()
  {
    name      = "CELL1";
    signature = "xT x ... -> T";
    syntax    = "Not available";
    meaning   = "Type mapping operator.";
  }
};

}

#endif // TILEALGEBRA_CELL1_H
