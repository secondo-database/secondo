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

#ifndef RASTER2_CELL1_H
#define RASTER2_CELL1_H

#include "AlgebraTypes.h"
#include "DateTime.h"
#include "NList.h"
#include "Operator.h"
#include "../stype.h"
#include "../istype.h"
#include "../grid2.h"

using namespace datetime;

namespace raster2
{
  ListExpr cell1TypeMap(ListExpr args);

  struct cell1Info : OperatorInfo
  {
    cell1Info()
    {
      name      = "CELL1";
      signature = "xT -> T";
      syntax    = "Not available";
      meaning   = "Type mapping operator.";
    }
  };

  /*ListExpr cell1TypeMap(unsigned long) { return 0; }*/
}

#endif /* #ifndef RASTER2_CELL1_H */
