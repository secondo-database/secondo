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

#ifndef RASTER2_CREATEGRID3_H
#define RASTER2_CREATEGRID3_H


#include <NestedList.h>
#include <Operator.h>


namespace raster2
{
  extern ValueMapping createGrid3Funs[];
  ListExpr createGrid3TM(ListExpr);
  int createGrid3SelectFun(ListExpr);

  struct createGrid3Info : OperatorInfo
  {
    createGrid3Info()
    {
      name      = "createGrid3";
      signature = "grid2 x {real,duration} -> grid3";
      syntax    = "createGrid3(_,_)";
      meaning   = "Adds the third dimension to a grid. If the duration "
                  "value is undefined or less or equal to zero, 1 day is"
                  " assumed.";
    }
  };
}

#endif
