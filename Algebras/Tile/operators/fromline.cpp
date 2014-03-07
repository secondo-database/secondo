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

/*
SECONDO includes

*/

#include "../HalfSegment/HalfSegment.h"
#include "RectangleAlgebra.h"

/*
TileAlgebra includes

*/

#include "fromline.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method SetLineValues sets all cells of given tbool object intersected
by given HalfSegment to true.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rtbool - reference to a tbool object containing all cells
                     intersected by rHalfSegment marked with a true value
return value: true, if the value of a cell of given tbool object set to true,
              otherwise false
exceptions: -

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
Method fromlineFunction implements the fromline operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of fromline operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of fromlineFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if fromlineFunction successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

int fromlineFunction(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier)
{
  int nRetVal = FAILURE;

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
            Rectangle<2> boundingBox = pLine->BoundingBox();
            pResultInfo->m_dMinimumX = boundingBox.MinD(0);
            pResultInfo->m_dMinimumY = boundingBox.MinD(1);
            pResultInfo->m_dMaximumX = boundingBox.MaxD(0);
            pResultInfo->m_dMaximumY = boundingBox.MaxD(1);

            double deltaX = pResultInfo->m_dMinimumX - gridX;
            double tileSizeX = xDimensionSize * gridLength;
            pResultInfo->m_dX = gridX + std::floor(deltaX / tileSizeX) *
                                tileSizeX;
            
            double deltaY = pResultInfo->m_dMinimumY - gridY;
            double tileSizeY = yDimensionSize * gridLength;    
            pResultInfo->m_dY = gridY + std::floor(deltaY / tileSizeY) *
                                tileSizeY;
            
            rLocal.addr = pResultInfo;
            nRetVal = 0;
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
                        double deltaX = pResultInfo->m_dMinimumX - gridX;
                        double tileSizeX = xDimensionSize * gridLength;
                        pResultInfo->m_dX = gridX +
                                            std::floor(deltaX / tileSizeX) *
                                            tileSizeX;
                      }
                    }
                  }

                  while(bHasTrueValue == false &&
                        pResultInfo->m_dX <= pResultInfo->m_dMaximumX &&
                        pResultInfo->m_dY <= pResultInfo->m_dMaximumY);

                  if(bHasTrueValue == true)
                  {
                    // return the next stream element
                    rResult.addr = ptbool;
                    nRetVal = YIELD;
                  }

                  else
                  {
                    delete ptbool;
                    ptbool = 0;

                    // always set the result to null before return CANCEL
                    rResult.addr = 0;
                    nRetVal = CANCEL;
                  }
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

          nRetVal = 0;
        }
        break;

        default:
        {
          assert(false);
        }
        break;
      }
    }
  }

  return nRetVal;
}

/*
Method fromlineTypeMappingFunction returns the return value type
of fromline operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of fromline operator
return value: return value type of fromline operator
exceptions: -

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
