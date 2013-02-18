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

#ifndef RASTER2_ATPERIODS_H
#define RASTER2_ATPERIODS_H

#include <NList.h>

#include "../msbool.h"
#include "../msreal.h"
#include "../msint.h"
#include "../msstring.h"
#include "TemporalAlgebra.h"

namespace raster2 {
    extern ValueMapping atperiodsFuns[];
    ListExpr atperiodsTypeMap(ListExpr args);
    int atperiodsSelectFun(ListExpr args);

  template <typename T, typename Helper>
  int atperiodsFun
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {
    typename Helper::implementation_type* pImplementationType =
      static_cast<typename Helper::implementation_type*>(args[0].addr);
    Periods* periods = static_cast<Periods*>(args[1].addr);

    if(pImplementationType != 0)
    {
      result = qp->ResultStorage(s);

      typename Helper::implementation_type* pResult =
            static_cast<typename Helper::implementation_type*>(result.addr);

      if(pResult != 0)
      {

        typename Helper::implementation_type* patperiodsResult = 
          pImplementationType->atperiods(*periods);
          
        if(patperiodsResult != 0)
        {
            std::swap(*pResult, *patperiodsResult);
            delete patperiodsResult;
        }
      }
    }

    return 0;
  }

    struct atperiodsInfo : OperatorInfo 
    {
      atperiodsInfo()
      { 
        name      = "atperiods";
        signature = msbool::BasicType() + " atperiods "
            + Periods::BasicType() + "-> " 
            + msbool::BasicType();
        appendSignature(msreal::BasicType() + " atperiods "
            + Periods::BasicType() + "-> " 
            + msreal::BasicType());
        appendSignature(msint::BasicType() + " atperiods "
            + Periods::BasicType() + "-> " 
            + msint::BasicType());
        appendSignature(msstring::BasicType() + " atperiods "
            + Periods::BasicType() + "-> " 
            + msstring::BasicType());
        syntax    = "atperiods(_)";
        meaning   = "restricts values to periods";
      }          
    };
}

#endif /* #define RASTER2_ATPERIODS_H */

