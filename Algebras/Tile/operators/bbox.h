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

#ifndef TILEALGEBRA_BBOX_H
#define TILEALGEBRA_BBOX_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"

namespace TileAlgebra
{

/*
declaration of bbox functions

*/

extern ValueMapping bboxFunctions[];

/*
declaration of bbox select function

*/

int bboxSelectFunction(ListExpr arguments);

/*
declaration of bbox type mapping function

*/

ListExpr bboxTypeMapping(ListExpr arguments);

/*
definition of bbox Operator Info structure

*/

struct bboxInfo : OperatorInfo
{
  bboxInfo()
  {
    name      = "bbox";
    signature = "tT -> " + Rect::BasicType();
    appendSignature("mtT -> " + Rect::BasicType());
    syntax    = "bbox(_)";
    meaning   = "Returns the bounding box Rectangle of a t type or a mt type.";
  }
};

/*
definition of template bbox function

*/

template <typename Type, typename Properties>
int bboxFunction(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    Type* pType = static_cast<Type*>(pArguments[0].addr);

    if(pType != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::bboxType* pResult =
        static_cast<typename Properties::bboxType*>(rResult.addr);
        
        if(pResult != 0)
        {
          *pResult = pType->bbox();
        }
      }
    }
  }

  return nRetVal;
}

}

#endif // TILEALGEBRA_BBOX_H
