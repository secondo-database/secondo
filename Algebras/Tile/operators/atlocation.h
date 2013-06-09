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

#ifndef TILEALGEBRA_ATLOCATION_H
#define TILEALGEBRA_ATLOCATION_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "Point.h"
#include "DateTime.h"

namespace TileAlgebra
{

/*
declaration of atlocation functions

*/

extern ValueMapping atlocationFunctions[];

/*
declaration of atlocation select function

*/

int atlocationSelectFunction(ListExpr arguments);

/*
declaration of atlocation type mapping function

*/

ListExpr atlocationTypeMapping(ListExpr arguments);

/*
definition of atlocation Operator Info structure

*/

struct atlocationInfo : OperatorInfo
{
  atlocationInfo()
  {
    name      = "atlocation";
    signature = "tT x " + Point::BasicType() + " -> T";
    appendSignature("mtT x " + Point::BasicType() + " -> mT");
    appendSignature("mtT x " + Point::BasicType() +
                    datetime::DateTime::BasicType() + " -> T");
    syntax    = "_ atlocation [_, _]";
    meaning   = "Returns the value(s) of a t type or a mt type "
                "at location point.";
  }
};

/*
definition of template atlocation function

*/

template <typename Type, typename Properties>
int atlocationFunction(Word* pArguments,
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
    Point* pPoint = static_cast<Point*>(pArguments[1].addr);

    if(pType != 0 &&
       pPoint != 0)
    {
      if(qp->GetNoSons(supplier) == 2)
      {
        rResult = qp->ResultStorage(supplier);

        if(rResult.addr != 0)
        {
          typename Properties::atlocationType* pResult =
          static_cast<typename Properties::atlocationType*>(rResult.addr);

          if(pResult != 0)
          {
            if(pType->IsDefined() &&
               pPoint->IsDefined())
            {
              *pResult = pType->atlocation(pPoint->GetX(), pPoint->GetY());
            }

            else
            {
              pResult->SetDefined(false);
            }
          }
        }
      }

      else
      {
        datetime::DateTime* pInstant = static_cast<datetime::DateTime*>
                                       (pArguments[2].addr);

        if(pInstant != 0)
        {
          rResult = qp->ResultStorage(supplier);

          if(rResult.addr != 0)
          {
            typename Properties::TypeProperties::WrapperType* pResult =
            static_cast<typename Properties::TypeProperties::WrapperType*>
            (rResult.addr);

            if(pResult != 0)
            {
              if(pType->IsDefined() &&
                 pPoint->IsDefined() &&
                 pInstant->IsDefined())
              {
                *pResult = pType->atlocation(pPoint->GetX(), pPoint->GetY(),
                                             pInstant->ToDouble());
              }

              else
              {
                pResult->SetDefined(false);
              }
            }
          }
        }
      }
    }
  }

  return nRetVal;
}

}

#endif // TILEALGEBRA_ATLOCATION_H
