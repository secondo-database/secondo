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

#ifndef TILEALGEBRA_T2MT_H
#define TILEALGEBRA_T2MT_H

#include <cmath>
#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "DateTime.h"
#include "../grid/tgrid.h"
#include "../Index.h"

namespace TileAlgebra
{

/*
declaration of t2mt functions

*/

extern ValueMapping t2mtFunctions[];

/*
declaration of t2mt select function

*/

int t2mtSelectFunction(ListExpr arguments);

/*
declaration of t2mt type mapping function

*/

ListExpr t2mtTypeMapping(ListExpr arguments);

/*
definition of t2mt Operator Info structure

*/

struct t2mtInfo : OperatorInfo
{
  t2mtInfo()
  {
    name      = "t2mt";
    signature = "tT x duration x instant x instant2-> mtT";
    syntax    = "t2mt(_, _, _, _)";
    meaning   = "Adds a time component to a t type.";
  }
};

/*
definition of template t2mt function

*/

template <typename Type, typename Properties>
int t2mtFunction(Word* pArguments,
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
    datetime::DateTime* pDuration = static_cast<datetime::DateTime*>
                                    (pArguments[1].addr);
    Instant* pInstant1 = static_cast<Instant*>(pArguments[2].addr);
    Instant* pInstant2 = static_cast<Instant*>(pArguments[3].addr);

    if(pType != 0 &&
       pDuration != 0 &&
       pInstant1 != 0 &&
       pInstant2 != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::PropertiesType* pResult =
        static_cast<typename Properties::PropertiesType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pType->IsDefined() &&
             pDuration->IsDefined() &&
             pInstant1->IsDefined() &&
             pInstant2->IsDefined())
          {
            int tDimensionSize = Properties::GetTDimensionSize();
            int startTime = static_cast<int>
                            (round(pInstant1->ToDouble() /
                                   pDuration->ToDouble()));
            int endTime = static_cast<int>
                          (round(pInstant2->ToDouble() /
                                 pDuration->ToDouble()));

            if(startTime >= 0 &&
               startTime < tDimensionSize &&
               endTime >= 0 &&
               endTime < tDimensionSize)
            {
              pResult->SetDefined(true);

              tgrid grid;
              pType->getgrid(grid);

              bool bOK = pResult->SetGrid(grid.GetX(),
                                          grid.GetY(),
                                          grid.GetLength(),
                                          *pDuration);

              if(bOK == true)
              {
                int xDimensionSize = Properties::GetXDimensionSize();
                int yDimensionSize = Properties::GetYDimensionSize();

                for(int time = startTime; time <= endTime; time++)
                {
                  for(int row = 0; row < yDimensionSize; row++)
                  {
                    for(int column = 0; column < xDimensionSize; column++)
                    {
                      typename Properties::TypeProperties::PropertiesType value
                      = Properties::TypeProperties::GetUndefinedValue();

                      Index<2> index2 = (int[]){column, row};
                      value = pType->GetValue(index2);

                      if(Properties::TypeProperties::IsUndefinedValue(value)
                         == false)
                      {
                        Index<3> index3 = (int[]){column, row, time};
                        bOK = pResult->SetValue(index3, value, true);
                      }
                    }
                  }
                }
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

#endif // TILEALGEBRA_T2MT_H
