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
#include "../HalfSegment/HalfSegment.h"
#include "RectangleAlgebra.h"

namespace TileAlgebra
{

/*
definition of SetLineValues function

*/

bool SetLineValues(const HalfSegment& rHalfSegment,
                   tbool& rtbool)
{
  bool bHasTrueValue = false;

  if(rtbool.IsDefined())
  {
    HalfSegment halfSegment = rHalfSegment;
    CheckHalfSegment(halfSegment);

    Point lineStartPoint = halfSegment.GetLeftPoint();
    Point lineEndPoint = halfSegment.GetRightPoint();

    if(lineStartPoint.IsDefined() &&
       lineEndPoint.IsDefined())
    {
      int xDimensionSize = tProperties<char>::GetXDimensionSize();
      int yDimensionSize = tProperties<char>::GetYDimensionSize();

      tgrid grid;
      rtbool.getgrid(grid);
      double gridX = grid.GetX();
      double gridY = grid.GetY();
      double gridLength = grid.GetLength();

      double Minima[2] = { gridX, gridY };
      double Maxima[2] = { gridX + xDimensionSize * gridLength,
                           gridY + yDimensionSize * gridLength };
      Rectangle<2> rectangle(true, Minima, Maxima);

      if(HalfSegmentIntersectsRectangle(halfSegment, rectangle))
      {
        if(IsHorizontalHalfSegment(halfSegment))
        {
          double xStart = lineStartPoint.GetX();
          double xEnd = lineEndPoint.GetX();
          double y = lineStartPoint.GetY();

          if(xStart < Minima[0])
          {
            xStart = Minima[0];
          }

          if(xEnd > Maxima[0])
          {
            xEnd = Maxima[0];
          }

          for(double x = xStart; x <= xEnd; x += gridLength)
          {
            bHasTrueValue |= rtbool.SetValue(x, y, true, true);
          }
        }

        else if(IsVerticalHalfSegment(halfSegment))
        {
          double x = lineStartPoint.GetX();
          double yStart = lineStartPoint.GetY();
          double yEnd = lineEndPoint.GetY();

          if(yStart < Minima[1])
          {
            yStart = Minima[1];
          }

          if(yEnd > Maxima[1])
          {
            yEnd = Maxima[1];
          }

          for(double y = yStart; y <= yEnd; y += gridLength)
          {
            bHasTrueValue |= rtbool.SetValue(x, y, true, true);
          }
        }

        else
        {
          double xStart = lineStartPoint.GetX();
          double xEnd = lineEndPoint.GetX();

          if(xStart < Minima[0])
          {
            xStart = Minima[0];
          }

          if(xEnd > Maxima[0])
          {
            xEnd = Maxima[0];
          }

          double deltaX = lineEndPoint.GetX() - lineStartPoint.GetX();
          double deltaY = lineEndPoint.GetY() - lineStartPoint.GetY();
          double m = deltaY / deltaX;
          double n = lineStartPoint.GetY() - m * lineStartPoint.GetX();

          for(double x = xStart; x <= xEnd; x += gridLength)
          {
            double y = m * x + n;

            if(y >= Minima[1] &&
               y < Maxima[1])
            {
              bHasTrueValue |= rtbool.SetValue(x, y, true, true);
            }
          }
        }
      }
    }
  }

  return bHasTrueValue;
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
       pGrid != 0 &&
       pLine->IsDefined() &&
       pGrid->IsDefined())
    {
      struct ResultInfo
      {
        double m_dMinimumX;
        double m_dMinimumY;
        double m_dMaximumX;
        double m_dMaximumY;
        double m_dX;
        double m_dY;
      };

      int xDimensionSize = tProperties<char>::GetXDimensionSize();
      int yDimensionSize = tProperties<char>::GetYDimensionSize();
      double gridX = pGrid->GetX();
      double gridY = pGrid->GetY();
      double gridLength = pGrid->GetLength();

      switch(message)
      {
        case OPEN:
        {
          // initialize the local storage
          ResultInfo* pResultInfo = new ResultInfo;

          if(pResultInfo != 0)
          {
            double doubleMaximum = std::numeric_limits<double>::max();
            pResultInfo->m_dMinimumX = doubleMaximum;
            pResultInfo->m_dMinimumY = doubleMaximum;
            pResultInfo->m_dMaximumX = -doubleMaximum;
            pResultInfo->m_dMaximumY = -doubleMaximum;

            for(int i = 0; i < pLine->Size(); i++)
            {
              HalfSegment halfSegment;
              pLine->Get(i, halfSegment);
              Point startPoint = halfSegment.GetLeftPoint();
              Point endPoint = halfSegment.GetRightPoint();

              if(startPoint.GetX() < pResultInfo->m_dMinimumX)
              {
                pResultInfo->m_dMinimumX = startPoint.GetX();
              }

              if(startPoint.GetX() > pResultInfo->m_dMaximumX)
              {
                pResultInfo->m_dMaximumX = startPoint.GetX();
              }

              if(startPoint.GetY() < pResultInfo->m_dMinimumY)
              {
                pResultInfo->m_dMinimumY = startPoint.GetY();
              }

              if(startPoint.GetY() > pResultInfo->m_dMaximumY)
              {
                pResultInfo->m_dMaximumY = startPoint.GetY();
              }

              if(endPoint.GetX() < pResultInfo->m_dMinimumX)
              {
                pResultInfo->m_dMinimumX = endPoint.GetX();
              }

              if(endPoint.GetX() > pResultInfo->m_dMaximumX)
              {
                pResultInfo->m_dMaximumX = endPoint.GetX();
              }

              if(endPoint.GetY() < pResultInfo->m_dMinimumY)
              {
                pResultInfo->m_dMinimumY = endPoint.GetY();
              }

              if(endPoint.GetY() > pResultInfo->m_dMaximumY)
              {
                pResultInfo->m_dMaximumY = endPoint.GetY();
              }
            }

            pResultInfo->m_dX = gridX +
                                std::floor((pResultInfo->m_dMinimumX - gridX) /
                                          (xDimensionSize * gridLength)) *
                                          (xDimensionSize * gridLength);
            pResultInfo->m_dY = gridY +
                                std::floor((pResultInfo->m_dMinimumY -gridY) /
                                          (yDimensionSize * gridLength)) *
                                          (yDimensionSize * gridLength);
            rLocal.addr = pResultInfo;
          }
        }
        break;

        case REQUEST:
        {
          if(rLocal.addr != 0)
          {
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              if(pResultInfo->m_dX <= pResultInfo->m_dMaximumX ||
                 pResultInfo->m_dY <= pResultInfo->m_dMaximumY)
              {
                tbool* ptbool = new tbool(true);

                if(ptbool != 0)
                {
                  ptbool->SetValues(false, true);
                  bool bHasTrueValue = false;

                  do
                  {
                    ptbool->SetGrid(pResultInfo->m_dX,
                                    pResultInfo->m_dY,
                                    gridLength);

                    for(int i = 0; i < pLine->Size(); i++)
                    {
                      HalfSegment halfSegment;
                      pLine->Get(i, halfSegment);
                      bHasTrueValue |= SetLineValues(halfSegment, *ptbool);
                    }

                    pResultInfo->m_dX += xDimensionSize * gridLength;

                    if(pResultInfo->m_dX > pResultInfo->m_dMaximumX)
                    {
                      pResultInfo->m_dY += yDimensionSize * gridLength;

                      if(pResultInfo->m_dY <=
                         pResultInfo->m_dMaximumY)
                      {
                        pResultInfo->m_dX = gridX +
                                            std::floor
                                            ((pResultInfo->m_dMinimumX -
                                              gridX) /
                                            (xDimensionSize * gridLength)) *
                                            (xDimensionSize * gridLength);
                      }
                    }
                  }

                  while(bHasTrueValue == false &&
                        pResultInfo->m_dX <= pResultInfo->m_dMaximumX &&
                        pResultInfo->m_dY <= pResultInfo->m_dMaximumY);

                  // return the next stream element
                  rResult.addr = ptbool;
                  nRetVal = YIELD;
                }
              }

              else
              {
                // always set the result to null before return CANCEL
                rResult.addr = 0;
                nRetVal = CANCEL;
              }
            }
          }
        }
        break;

        case CLOSE:
        {
          if(rLocal.addr != 0)
          {
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              delete pResultInfo;
              rLocal.addr = 0;
            }
          }
        }
        break;

        default:
        {
          assert(false);
          nRetVal = -1;
        }
        break;
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

  if(nl != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      std::string argument1 = argumentsList.first().str();
      std::string argument2 = argumentsList.second().str();

      if(argument1 == Line::BasicType() &&
         argument2 == tgrid::BasicType())
      {
        type = nl->TwoElemList(nl->SymbolAtom(Stream<tbool>::BasicType()),
                               nl->SymbolAtom(tbool::BasicType()));
      }
    }
  }

  return type;
}

}
