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
#include "RobustSetOps.h"

/*
TileAlgebra includes

*/

#include "fromregion.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method SetRegionValues sets all cells of given tbool object whose center point
is inside or on the border of the given Region to true.

author: Dirk Zacher
parameters: rRegion - reference to a Region
            rLeftPoint - reference to the left point of a HalfSegment
            rRightPoint - reference to the right point of a HalfSegment
            rtbool - reference to a tbool object containing all cells inside
                     or on the border of rRegion marked with a true value
return value: true, if the value of a cell of given tbool object set to true,
              otherwise false
exceptions: -

*/

bool SetRegionValues(const Region& rRegion,
                     const Point& rLeftPoint,
                     const Point& rRightPoint,
                     tbool& rtbool)
{
  bool bHasTrueValue = false;

  if(rRegion.IsDefined() &&
     rLeftPoint.IsDefined() &&
     rRightPoint.IsDefined() &&
     rtbool.IsDefined())
  {
    tgrid grid;
    rtbool.getgrid(grid);
    double gridX = grid.GetX();
    double gridY = grid.GetY();
    double gridLength = grid.GetLength();
    double halfGridLength = gridLength / 2.0;

    if(rLeftPoint.GetY() <= rRightPoint.GetY())
    {
      for(double y = rLeftPoint.GetY(); y <= rRightPoint.GetY();
          y += gridLength)
      {
        for(double x = rLeftPoint.GetX(); x <= rRightPoint.GetX();
            x += gridLength)
        {
          if(rtbool.IsValidLocation(x, y))
          {
            Index<2> index = rtbool.GetLocationIndex(x, y);
            Point centerPoint(true,
                              gridX + index[0] * gridLength + halfGridLength,
                              gridY + index[1] * gridLength + halfGridLength);

            if(robust::contains(rRegion, centerPoint) > 0)
            {
              bHasTrueValue |= rtbool.SetValue(x, y, true, true);
            }
          }
        }
      }
    }

    else
    {
      for(double y = rLeftPoint.GetY(); y >= rRightPoint.GetY();
          y -= gridLength)
      {
        for(double x = rLeftPoint.GetX(); x <= rRightPoint.GetX();
            x += gridLength)
        {
          if(rtbool.IsValidLocation(x, y))
          {
            Index<2> index = rtbool.GetLocationIndex(x, y);
            Point centerPoint(true,
                              gridX + index[0] * gridLength + halfGridLength,
                              gridY + index[1] * gridLength + halfGridLength);

            if(robust::contains(rRegion, centerPoint) > 0)
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
Method SetRegionValues sets all cells of given tbool object whose center point
is inside or on the border of the given Region to true.

author: Dirk Zacher
parameters: rRegion - reference to a Region
            rHalfSegment - reference to a HalfSegment
            rtbool - reference to a tbool object containing all cells inside
                     or on the border of rRegion marked with a true value
return value: true, if the value of a cell of given tbool object set to true,
              otherwise false
exceptions: -

*/

bool SetRegionValues(const Region& rRegion,
                     const HalfSegment& rHalfSegment,
                     tbool& rtbool)
{
  bool bHasTrueValue = false;

  if(rRegion.IsDefined() &&
     rtbool.IsDefined())
  {
    HalfSegment halfSegment = rHalfSegment;
    CheckHalfSegment(halfSegment);

    Point halfSegmentLeftPoint = halfSegment.GetLeftPoint();
    Point halfSegmentRightPoint = halfSegment.GetRightPoint();

    if(halfSegmentLeftPoint.IsDefined() &&
       halfSegmentRightPoint.IsDefined())
    {
      int xDimensionSize = tProperties<char>::GetXDimensionSize();
      int yDimensionSize = tProperties<char>::GetYDimensionSize();

      tgrid grid;
      rtbool.getgrid(grid);
      double gridX = grid.GetX();
      double gridY = grid.GetY();
      double gridLength = grid.GetLength();

      bool bOK = false;
      double Minima[2] = { gridX, gridY };
      double Maxima[2] = { gridX + xDimensionSize * gridLength,
                           gridY + yDimensionSize * gridLength };
      Rectangle<2> rectangle(true, Minima, Maxima);

      Point leftPoint(true);
      Point rightPoint(true);
      Point offsetPoint(true, 0, gridLength);
      bOK = GetPointsInRectangle(halfSegment, rectangle,
                                 leftPoint, rightPoint);

      if(bOK == true)
      {
        if(IsHorizontalHalfSegment(halfSegment))
        {
          /*
          performance optimization: region is calculated only
                                    by vertical offsets

          */

          Point nextLeftPoint = leftPoint;
          Point nextRightPoint = rightPoint;
          bool bNextRow = true;

          while(bNextRow == true &&
                nextLeftPoint.GetY() >= Minima[1])
          {
            bNextRow = SetRegionValues(rRegion, nextLeftPoint,
                                       nextRightPoint, rtbool);
            nextLeftPoint = nextLeftPoint - offsetPoint;
            nextRightPoint = nextRightPoint - offsetPoint;
            bHasTrueValue |= bNextRow;
          }

          nextLeftPoint = leftPoint + offsetPoint;
          nextRightPoint = rightPoint + offsetPoint;
          bNextRow = true;

          while(bNextRow == true &&
                nextRightPoint.GetY() < Maxima[1])
          {
            bNextRow = SetRegionValues(rRegion, nextLeftPoint,
                                       nextRightPoint, rtbool);
            nextLeftPoint = nextLeftPoint + offsetPoint;
            nextRightPoint = nextRightPoint + offsetPoint;
            bHasTrueValue |= bNextRow;
          }
        }

        else if(IsVerticalHalfSegment(halfSegment))
        {
          /*
          performance optimization: region is calculated only
                                    by vertical offsets

          */
          
          bHasTrueValue |= SetRegionValues(rRegion,
                                           leftPoint,
                                           rightPoint,
                                           rtbool);
        }

        else
        {
          /*
          performance optimization: region is calculated only
                                    by vertical offsets

          */

          bool bNextRow = SetRegionValues(rRegion, leftPoint, rightPoint,
                                          rtbool);

          if(bNextRow == true)
          {
            bHasTrueValue = true;
            
            Point nextLeftPoint(true);
            Point nextRightPoint(true);
            nextLeftPoint.Set(leftPoint.GetX(),
                              std::min(leftPoint.GetY(), rightPoint.GetY()));
            nextRightPoint.Set(rightPoint.GetX(),
                               std::min(leftPoint.GetY(), rightPoint.GetY()));
            bNextRow = true;

            while(bNextRow == true &&
                  nextLeftPoint.GetY() >= Minima[1] &&
                  nextRightPoint.GetY() >= Minima[1])
            {
              bNextRow = SetRegionValues(rRegion, nextLeftPoint,
                                         nextRightPoint, rtbool);
              nextLeftPoint = nextLeftPoint - offsetPoint;
              nextRightPoint = nextRightPoint - offsetPoint;
            }

            nextLeftPoint.Set(leftPoint.GetX(),
                              std::max(leftPoint.GetY(), rightPoint.GetY()));
            nextRightPoint.Set(rightPoint.GetX(),
                               std::max(leftPoint.GetY(), rightPoint.GetY()));
            bNextRow = true;

            while(bNextRow == true &&
                  nextLeftPoint.GetY() < Maxima[1] &&
                  nextRightPoint.GetY() < Maxima[1])
            {
              bNextRow = SetRegionValues(rRegion, nextLeftPoint,
                                         nextRightPoint, rtbool);
              nextLeftPoint = nextLeftPoint + offsetPoint;
              nextRightPoint = nextRightPoint + offsetPoint;
            }
          }
        }
      }
    }
  }

  return bHasTrueValue;
}

/*
Method fromregionFunction implements the fromregion operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of fromregion operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of fromregionFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if fromregionFunction successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

int fromregionFunction(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    Region* pRegion = static_cast<Region*>(pArguments[0].addr);
    tgrid* pGrid = static_cast<tgrid*>(pArguments[1].addr);

    if(pRegion != 0 &&
       pGrid != 0 &&
       pRegion->IsDefined() &&
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
            Rectangle<2> boundingBox = pRegion->BoundingBox();
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
                  int regionSize = pRegion->Size();
                  bool bHasTrueValue = false;

                  do
                  {
                    ptbool->SetGrid(pResultInfo->m_dX,
                                    pResultInfo->m_dY,
                                    gridLength);
                    
                    double tileMinima[2] = { pResultInfo->m_dX,
                                             pResultInfo->m_dY };
                    double tileMaxima[2] = { pResultInfo->m_dX +
                                             xDimensionSize * gridLength,
                                             pResultInfo->m_dY +
                                             yDimensionSize * gridLength };
                    Rectangle<2> tileRectangle(true, tileMinima, tileMaxima);
                    bool halfSegmentIntersectsRectangle = false;

                    for(int i = 0; i < regionSize; i++)
                    {
                      HalfSegment halfSegment;
                      pRegion->Get(i, halfSegment);
                      
                      if(HalfSegmentIntersectsRectangle(halfSegment,
                                                        tileRectangle))
                      {
                        halfSegmentIntersectsRectangle = true;
                        break;
                      }
                    }
                    
                    if(halfSegmentIntersectsRectangle == true)
                    {
                      Point tileMinimaPoint(true,
                                            pResultInfo->m_dX,
                                            pResultInfo->m_dY);
                      Point tileMaximaPoint(true,
                                            pResultInfo->m_dX +
                                            xDimensionSize * gridLength,
                                            pResultInfo->m_dY +
                                            yDimensionSize * gridLength);

                      bHasTrueValue = SetRegionValues(*pRegion,
                                                      tileMinimaPoint,
                                                      tileMaximaPoint,
                                                      *ptbool);
                    }
                    
                    else
                    {
                      Point tileMinimaPoint(true,
                                            pResultInfo->m_dX,
                                            pResultInfo->m_dY);

                      if(robust::contains(*pRegion, tileMinimaPoint) > 0)
                      {
                        bHasTrueValue = ptbool->SetValues(true, true);
                      }
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
Method fromregionTypeMappingFunction returns the return value type
of fromregion operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of fromregion operator
return value: return value type of fromregion operator
exceptions: -

*/

ListExpr fromregionTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator fromregion expects "
                                   "a region and a tgrid.");

  if(nl != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      std::string argument1 = argumentsList.first().str();
      std::string argument2 = argumentsList.second().str();

      if(argument1 == Region::BasicType() &&
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
