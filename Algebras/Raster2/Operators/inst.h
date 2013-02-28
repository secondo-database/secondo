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

#ifndef RASTER2_INST_H
#define RASTER2_INST_H

#include "AlgebraTypes.h"
#include "DateTime.h"
#include "NList.h"
#include "Operator.h"
#include "../istype.h"

using namespace datetime;

namespace raster2
{
  extern ValueMapping instFuns[];
  ListExpr instTypeMap(ListExpr args);
  int instSelectFun(ListExpr args);

  struct instInfo : OperatorInfo
  {
    instInfo()
    {
      name      = "inst";
      signature = "isType -> " + DateTime::BasicType();
      syntax    = "inst(_)";
      meaning   = "Returns the instant value of a isType.";
    }
  };

  template <typename T, typename Helper>
  int instFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(args[0].addr);

    result = qp->ResultStorage(s);

    DateTime* pResult = static_cast<DateTime*>(result.addr);

    if((pistype != 0) && pistype->isDefined())
    {

      if(pResult != 0)
      {
        *pResult = pistype->inst();
      }
    } else {
      pResult->SetDefined(false);
    }
    return 0;
  }
}

#endif /* #ifndef RASTER2_INST_H */
