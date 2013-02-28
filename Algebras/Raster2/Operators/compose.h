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

#ifndef RASTER2_COMPOSE_H
#define RASTER2_COMPOSE_H

#include <NList.h>

#include "../sbool.h"
#include "../sreal.h"
#include "../sint.h"
#include "../sstring.h"
#include "TemporalAlgebra.h"
#include "RTreeAlgebra.h"
#include "CellIterator.h"

namespace raster2 {
  extern ValueMapping composeFuns[];
  ListExpr composeTypeMap(ListExpr args);
  int composeSelectFun(ListExpr args);


    struct composeInfo : OperatorInfo 
    {
      composeInfo()
      { 
        name      = "compose";
        signature = MPoint::BasicType()
            + " compose "
            + sbool::BasicType() + "-> " 
            + MBool::BasicType();
        appendSignature(MPoint::BasicType() + " compose "
            + sreal::BasicType() + "-> " 
            + MReal::BasicType());
        appendSignature(MPoint::BasicType() + " compose "
            + sint::BasicType() + "-> " 
            + MInt::BasicType());
        appendSignature(MPoint::BasicType() + " compose "
            + sstring::BasicType() + "-> " 
            + MString::BasicType());

        syntax    = "_ compose _";
        meaning   = "merges mpoint and sT into mT";
      }          
    };
}

#endif /* #define RASTER2_COMPOSE_H */

