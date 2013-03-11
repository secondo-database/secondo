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

#ifndef RASTER2_ATLOCATION_H
#define RASTER2_ATLOCATION_H

#include "../stype.h"
#include "../mstype.h"

namespace raster2
{
  extern ValueMapping atlocationFuns[];
  ListExpr atlocationTypeMap(ListExpr args);
  int atlocationSelectFun(ListExpr args);

  struct atlocationInfo : OperatorInfo
  {
    atlocationInfo()
    {
      name      = "atlocation";
      signature = "sType x " + Point::BasicType() + " -> Type";
      appendSignature("msType x " + Point::BasicType() + " -> mType");
      appendSignature("msType x " + Point::BasicType() + " x " +
                      DateTime::BasicType() + " -> Type");
      syntax    = "_ atlocation [_, _]";
      meaning   = "Returns the value at location point.";
    }
  };



}

#endif /* #define RASTER2_ATLOCATION_H */
