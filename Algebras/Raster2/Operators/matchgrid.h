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

#ifndef RASTER2_MATCHGRID_H
#define RASTER2_MATCHGRID_H

#include <AlgebraTypes.h>
#include <NestedList.h>
#include <Operator.h>

namespace raster2
{
  extern ValueMapping matchgridFuns[];
  ListExpr matchgridTypeMap(ListExpr args);
  int matchgridSelectFun(ListExpr args);

  struct matchgridInfo : OperatorInfo
  {
    matchgridInfo()
    {
      name      = "matchgrid";
      signature = "sT x grid2 x (rel(tuple([Elem: T])) -> U ) x bool -> sU";
      appendSignature(
                  "msT x grid3 x (rel(tuple([Elem: T])) -> U ) x bool -> msU");
      syntax    = "_ matchgrid [_, _, _]";
      meaning   = "Resamples an sT or msT by applying a user function.";
    }
  };
}

#endif
