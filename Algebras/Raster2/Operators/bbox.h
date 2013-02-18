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

#ifndef RASTER2_BBOX_H
#define RASTER2_BBOX_H

#include <NList.h>

#include "../stype.h"
#include "../mstype.h"

namespace raster2
{
  extern ValueMapping bboxFuns[];
  ListExpr bboxTypeMap(ListExpr args);
  int bboxSelectFun(ListExpr args);

  struct bboxInfo : OperatorInfo
  {
    bboxInfo()
    {
      name      = "bbox";
      signature = "sType -> " + Rect::BasicType();
      appendSignature("msType -> " + Rect::BasicType());
      syntax    = "bbox(_)";
      meaning   = "Returns the bounding box Rect of an sType or a msType.";
    }
  };

  template <typename T, typename Helper>
  int bboxFun(Word* args, Word& result, int message, Word& local, Supplier s)
  {
    typename Helper::implementation_type* pImplementationType =
      static_cast<typename Helper::implementation_type*>(args[0].addr);

    if(pImplementationType != 0)
    {
      result = qp->ResultStorage(s);

      Rect* pResult = static_cast<Rect*>(result.addr);

      if(pResult != 0)
      {
        *pResult = pImplementationType->bbox();
      }
    }
    return 0;
  }
    
  template <typename T, typename Helper>
  int bboxFun3
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {
    typename Helper::implementation_type* pImplementationType =
      static_cast<typename Helper::implementation_type*>(args[0].addr);

    if(pImplementationType != 0)
    {
      result = qp->ResultStorage(s);

      Rectangle<3>* pResult = static_cast<Rectangle<3>*>(result.addr);

      if(pResult != 0)
      {
        *pResult = pImplementationType->bbox();
      }
    }

    return 0;
  }
}

#endif /* #ifndef RASTER2_BBOX_H */
