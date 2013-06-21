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

#ifndef TILEALGEBRA_ATRANGE_H
#define TILEALGEBRA_ATRANGE_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "DateTime.h"

namespace TileAlgebra
{

/*
declaration of atrange functions

*/

extern ValueMapping atrangeFunctions[];

/*
declaration of atrange select function

*/

int atrangeSelectFunction(ListExpr arguments);

/*
declaration of atrange type mapping function

*/

ListExpr atrangeTypeMappingFunction(ListExpr arguments);

/*
definition of atrange Operator Info structure

*/

struct atrangeInfo : OperatorInfo
{
  atrangeInfo()
  {
    name      = "atrange";
    signature = "xT x " + Rectangle<2>::BasicType() + " -> xT";
    appendSignature("mtT x " + Rectangle<2>::BasicType() +
                    " x Instant x Instant -> mtT ");
    syntax    = "_ atrange [_,_,_]";
    meaning   = "Returns the values at range of the rectangle.";
  }
};

/*
definition of template atrangeFunction

*/

template <typename Type>
int atrangeFunction(Word* pArguments,
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
    Rectangle<2>* pRectangle = static_cast<Rectangle<2>*>(pArguments[1].addr);

    if(pType != 0 &&
       pRectangle != 0)
    {
      if(qp->GetNoSons(supplier) == 2)
      {
        rResult = qp->ResultStorage(supplier);

        if(rResult.addr != 0)
        {
          Type* pResult = static_cast<Type*>(rResult.addr);

          if(pResult != 0)
          {
            if(pType->IsDefined() &&
               pRectangle->IsDefined())
            {
              pType->atrange(*pRectangle, *pResult);
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
        Instant* pInstant1 = static_cast<Instant*>(pArguments[2].addr);
        Instant* pInstant2 = static_cast<Instant*>(pArguments[3].addr);

        if(pInstant1 != 0 &&
           pInstant2 != 0)
        {
          rResult = qp->ResultStorage(supplier);

          if(rResult.addr != 0)
          {
            Type* pResult = static_cast<Type*>(rResult.addr);

            if(pResult != 0)
            {
              if(pType->IsDefined() &&
                 pRectangle->IsDefined() &&
                 pInstant1->IsDefined() &&
                 pInstant2->IsDefined())
              {
                pType->atrange(*pRectangle,
                               pInstant1->ToDouble(),
                               pInstant2->ToDouble(),
                               *pResult);
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

#endif // TILEALGEBRA_ATRANGE_H
