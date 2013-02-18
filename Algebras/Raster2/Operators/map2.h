/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#ifndef RASTER2_MAP2_H
#define RASTER2_MAP2_H

#include <stdexcept>

#include <AlgebraTypes.h>
#include <Application.h>
#include <NestedList.h>
#include <NList.h>
#include <Operator.h>
#include <QueryProcessor.h>
#include <RectangleAlgebra.h>

#include "../grid2.h"

namespace raster2
{
  extern ValueMapping map2Funs[];
  ListExpr map2TypeMap(ListExpr);
  int map2SelectFun(ListExpr);

  struct map2Info : OperatorInfo
  {
    map2Info()
    {
      name      = "map2";
      signature =     " sT x  sU x (T x U -> V) ->  sV";
      appendSignature("msT x  sU x (T x U -> V) -> msV");
      appendSignature(" sT x msU x (T x U -> V) -> msV");
      appendSignature("msT x msU x (T x U -> V) -> msV");
      syntax    = "_ _ map2[_]";
      meaning   = "Combines two raster types to a new raster type.";
    }
  };
}

#endif
