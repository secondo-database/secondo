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

#ifndef RASTER2_MAXIMUM_H
#define RASTER2_MAXIMUM_H

#include <NList.h>

#include "../stype.h"
#include "../mstype.h"

namespace raster2
{
    extern ValueMapping maximumFuns[];
    ListExpr maximumTypeMap(ListExpr args);
    int maximumSelectFun(ListExpr args);

    struct maximumInfo : OperatorInfo
    {
      maximumInfo()
      {
        name      = "maximum";
        signature = "sType -> Type";
        appendSignature("msType -> Type");
        syntax    = "maximum(_)";
        meaning   = "Returns the maximum value.";
      }
    };

    template <typename T, typename Helper>
    int maximumFun
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
      typename Helper::implementation_type* pImplementationType =
        static_cast<typename Helper::implementation_type*>(args[0].addr);

      if(pImplementationType != 0)
      {
        result = qp->ResultStorage(s);

        typename Helper::wrapper_type* pResult
          = static_cast<typename Helper::wrapper_type*>(result.addr);

        if(pResult != 0)
        {
          *pResult = Helper::wrap(pImplementationType->getMaximum());
        }
      }

      return 0;
    }
}

#endif /* #ifndef RASTER2_MAXIMUM_H */
