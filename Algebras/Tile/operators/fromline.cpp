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

#include "fromline.h"
#include "../Index.h"

namespace TileAlgebra
{

/*
definition of drawLine function

*/

void drawLine(tbool& rtbool,
              Index<2> startIndex,
              Index<2> endIndex)
{
  if(startIndex[0] > endIndex[0])
  {
    std::swap(startIndex, endIndex);
  }

  if(startIndex[0] == endIndex[0])
  {
    /*
    case: vertical line

    */

    if(startIndex[1] > endIndex[1])
    {
      std::swap(startIndex, endIndex);
    }

    for(int row = startIndex[1]; row <= endIndex[1]; row++)
    {
      Index<2> index = (int[]){startIndex[0], row};
      rtbool.SetValue(index, true, true);
    }
  }
  
  else if(startIndex[1] == endIndex[1])
  {
    /*
    case: horizontal line

    */

    for(int column = startIndex[0]; column <= endIndex[0]; column++)
    {
      Index<2> index = (int[]){column, startIndex[1]};
      rtbool.SetValue(index, true, true);
    }
  }
  
  else
  {
    int dx = endIndex[0] - startIndex[0];
    int dy = endIndex[1] - startIndex[1];
    int dx2 = 2 * dx;
    int dy2 = 2 * dy;
    int dy2_minus_dx2 = dy2 - dx2;
    int dy2_plus_dx2 = dy2 + dx2;

    if(dy >= 0)
    {
      // m >= 0
      if(dy <= dx)
      {
        // 0 <= m <= 1
        int F = dy2 - dx;

        for(Index<2> current = startIndex; current[0] <= endIndex[0];
            current.Increment(0))
        {
          rtbool.SetValue(current, true, true);

          if(F <= 0)
          {
            F += dy2;
          }
          
          else
          {
            current.Increment(1);
            F += dy2_minus_dx2;
          }
        }
      }

      else
      {
        // 1 < m
        int F = dx2 - dy;

        for(Index<2> current = startIndex; current[1] <= endIndex[1];
            current.Increment(1))
        {
          rtbool.SetValue(current, true, true);

          if(F <= 0)
          {
            F += dx2;
          }
          
          else
          {
            current.Increment(0);
            F -= dy2_minus_dx2;
          }
        }
      }
    }
    
    else
    {
      // m < 0
      if (dx >= -dy)
      {
        // -1 <= m < 0
        int F = -dy2 - dx;

        for(Index<2> current = startIndex; current[0] <= endIndex[0];
            current.Increment(0))
        {
          rtbool.SetValue(current, true, true);

          if (F <= 0)
          {
            F -= dy2;
          }
          
          else
          {
            current.Decrement(1);
            F -= dy2_plus_dx2;
          }
        }
      }
      
      else
      {
        // m < -1
        int F = dx2 + dy;

        for(Index<2> current = startIndex; current[1] >= endIndex[1];
            current.Decrement(1))
        {
          rtbool.SetValue(current, true, true);

          if(F <= 0)
          {
            F += dx2;
          }
          
          else
          {
            current.Increment(0);
            F += dy2_plus_dx2;
          }
        }
      }
    }
  }
}

/*
definition of fromline function

*/

int fromlineFunction(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    Line* pLine = static_cast<Line*>(pArguments[0].addr);
    tgrid* pGrid = static_cast<tgrid*>(pArguments[1].addr);

    if(pLine != 0 &&
       pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        tbool* pResult = static_cast<tbool*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pLine->IsDefined() &&
             pGrid->IsDefined())
          {
            pResult->SetDefined(true);
            pResult->SetGrid(*pGrid);

            for(int i = 0; i < pLine->Size(); i++)
            {
              HalfSegment halfSegment;
              pLine->Get(i, halfSegment);
              Point startPoint = halfSegment.GetLeftPoint();
              Point endPoint = halfSegment.GetRightPoint();

              if(pResult->IsValidLocation(startPoint.GetX(),
                                          startPoint.GetY()) &&
                 pResult->IsValidLocation(endPoint.GetX(),
                                          endPoint.GetY()))
              {
                 Index<2> startIndex = pResult->GetLocationIndex
                                       (startPoint.GetX(),
                                        startPoint.GetY());
                 Index<2> endIndex = pResult->GetLocationIndex
                                     (endPoint.GetX(),
                                      endPoint.GetY());

                 drawLine(*pResult, startIndex, endIndex);
              }
            }

            int xDimensionSize = tProperties<char>::GetXDimensionSize();
            int yDimensionSize = tProperties<char>::GetYDimensionSize();

            for(int row = 0; row < yDimensionSize; row++)
            {
              for(int column = 0; column < xDimensionSize; column++)
              {
                Index<2> index = (int[]){column, row};
                char value = pResult->GetValue(index);

                if(tProperties<char>::TypeProperties::IsUndefinedValue(value)
                   == true)
                {
                  pResult->SetValue(index, false, true);
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

/*
definition of fromline type mapping function

*/

ListExpr fromlineTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator fromline expects "
                                   "a line and a tgrid.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(argument1 == Line::BasicType() &&
       argument2 == tgrid::BasicType())
    {
      type = NList(tbool::BasicType()).listExpr();
    }
  }

  return type;
}

}
