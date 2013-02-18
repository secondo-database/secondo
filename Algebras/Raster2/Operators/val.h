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

#ifndef RASTER2_VAL_H
#define RASTER2_VAL_H

#include "AlgebraTypes.h"
#include "NList.h"
#include "Operator.h"
#include "../istype.h"

using namespace datetime;

namespace raster2
{
  extern ValueMapping valFuns[];
  ListExpr valTypeMap(ListExpr args);
  int valSelectFun(ListExpr args);

  struct valInfo : OperatorInfo
  {
    valInfo()
    {
      name      = "val";
      signature = "isType -> sType";
      syntax    = "val(_)";
      meaning   = "Returns the sType value of a isType.";
    }
  };

  template <typename T, typename Helper>
  int valFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(args[0].addr);

    if(pistype != 0)
    {
      result = qp->ResultStorage(s);

      typename Helper::spatial_type* pResult =
        static_cast<typename Helper::spatial_type*>(result.addr);

      if(pResult != 0)
      {
        typename Helper::spatial_type* pvalResult = pistype->val();
          
        if(pvalResult != 0)
        {
          std::swap(*pResult, *pvalResult);
          pvalResult->destroy();
          delete pvalResult;
        }
      }
    }
    
    return 0;
  }
}

#endif /* #ifndef RASTER2_INST_H */
