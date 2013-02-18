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
      syntax    = "_ atlocation _";
      meaning   = "Returns the value at location point.";
    }
  };

  template <typename T, typename Helper>
  int atlocationFun
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {
      result = qp->ResultStorage(s);

      typename Helper::wrapper_type* pResult =
        static_cast<typename Helper::wrapper_type*>(result.addr);

      typename Helper::implementation_type* pSobject =
        static_cast<typename Helper::implementation_type*>(args[0].addr);

      Point* pPoint = static_cast<Point*>(args[1].addr);

      if (pPoint->IsDefined()) {
          *pResult = Helper::wrap(
                  pSobject->atlocation(pPoint->GetX(), pPoint->GetY()));
      } else {
          *pResult = Helper::wrap(Helper::getUndefined());
      }

    return 0;
  }

  template <typename MSType, typename MType, typename Type>
  int atlocationFunMType
    (Word* args, Word& result, int message, Word& local, Supplier s)
  {
      MSType* msin = static_cast<MSType*>(args[0].addr);
      Point* pPoint = static_cast<Point*>(args[1].addr);
      if(pPoint != 0)
      {
          result = qp->ResultStorage(s);
          MType* res = static_cast<MType*>(result.addr);
          MType* mout = msin->atlocation(pPoint->GetX(),
                                         pPoint->GetY());

          if(mout->IsDefined())
          {
              std::swap(*res, *mout);
              delete mout;





          }
      }

      return 0;
  }
}

#endif /* #define RASTER2_ATLOCATION_H */
