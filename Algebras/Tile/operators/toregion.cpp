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

#include "RobustSetOps.h"
#include "RegionTools.h"

/*
TileAlgebra includes

*/

#include "toregion.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method UpdateCycleAndCoordinates updates current cycle and
rLastX, rLastY, rCurrentX and rCurrentY coordinates.

author: Dirk Zacher
parameters: rCycle - reference to a vector of points containing cycle points
            rLastX - reference to the last x coordinate
            rLastY - reference to the last y coordinate
            rCurrentX - reference to the current x coordinate
            rCurrentY - reference to the current y coordinate
            rDeltaX - reference to delta x value
            rDeltaY - reference to delta y value
return value: -
exceptions: -

*/

void UpdateCycleAndCoordinates(vector<Point>& rCycle,
                               double& rLastX,
                               double& rLastY,
                               double& rCurrentX,
                               double& rCurrentY,
                               const double& rDeltaX,
                               const double& rDeltaY)
{
  Point point(true,
              rCurrentX + rDeltaX,
              rCurrentY + rDeltaY);
  rCycle.push_back(point);

  rLastX = rCurrentX;
  rLastY = rCurrentY;
  rCurrentX += rDeltaX;
  rCurrentY += rDeltaY;
}

/*
Method toregionFunction implements the toregion operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toregion operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toregionFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toregionFunction successfully executed, otherwise FAILURE
exceptions: -

*/

int toregionFunction(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    tbool* ptbool = static_cast<tbool*>(pArguments[0].addr);

    if(ptbool != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        Region* pResult = static_cast<Region*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(ptbool->IsDefined())
          {
            Index<2> minimumIndex;
            Index<2> maximumIndex;
            bool bOK = ptbool->GetBoundingBoxIndexes(minimumIndex,
                                                     maximumIndex);

            if(bOK == true)
            {
              tgrid grid;
              ptbool->getgrid(grid);
              double gridOriginX = grid.GetX();
              double gridOriginY = grid.GetY();
              double gridLength = grid.GetLength();
              double halfGridLength = gridLength / 2.0;

              double startX = 0.0;
              double startY = 0.0;
              Region* pBuildRegion = new Region(0);
              Region* pBuildHoles = new Region(0);
              vector<vector<Point> > cycles;
              vector<vector<Point> > holes;

              for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
              {
                for(int column = minimumIndex[0]; column < maximumIndex[0];
                    column++)
                {
                  Index<2> index((int[]){column, row});
                  char value = ptbool->GetValue(index);

                  if(tProperties<char>::TypeProperties::
                     IsUndefinedValue(value) == false)
                  {
                    if(value == 1)
                    {
                      startX = gridOriginX + index[0] * gridLength;
                      startY = gridOriginY + index[1] * gridLength;
                      
                      Point centerPoint(true,
                                        startX + halfGridLength,
                                        startY + halfGridLength);
                      
                      if(robust::contains(*pBuildRegion, centerPoint) == 0)
                      {
                        vector<Point> cycle;

                        Point currentPoint(true, startX, startY);
                        cycle.push_back(currentPoint);

                        Point nextPoint(true, startX, startY + gridLength);
                        cycle.push_back(nextPoint);

                        double lastX = currentPoint.GetX();
                        double lastY = currentPoint.GetY();
                        double currentX = nextPoint.GetX();
                        double currentY = nextPoint.GetY();
                        bool bEnd = false;

                        while(bEnd == false)
                        {
                          CcBool currentCenterValue;
                          ptbool->atlocation(currentX + halfGridLength,
                                             currentY + halfGridLength,
                                             currentCenterValue);
                          CcBool leftCenterValue;
                          ptbool->atlocation(currentX - halfGridLength,
                                             currentY + halfGridLength,
                                             leftCenterValue);
                          CcBool leftbottomCenterValue;
                          ptbool->atlocation(currentX - halfGridLength,
                                             currentY - halfGridLength,
                                             leftbottomCenterValue);
                          CcBool bottomCenterValue;
                          ptbool->atlocation(currentX + halfGridLength,
                                             currentY - halfGridLength,
                                             bottomCenterValue);

                          if(AlmostEqual(currentX, lastX) == true &&
                             AlmostEqual(currentY, lastY) == false &&
                             currentY > lastY)
                          {
                            if(leftCenterValue.IsDefined() &&
                               leftCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        -gridLength, 0.0);
                            }

                            else if(currentCenterValue.IsDefined() &&
                                    currentCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, gridLength);
                            }

                            else if(bottomCenterValue.IsDefined() &&
                                    bottomCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        gridLength, 0.0);
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) == true  &&
                                  AlmostEqual(currentY, lastY) == false &&
                                  currentY < lastY)
                          {
                            if(bottomCenterValue.IsDefined() &&
                               bottomCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        gridLength, 0.0);
                            }

                            else if(leftbottomCenterValue.IsDefined() &&
                                    leftbottomCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, -gridLength);
                            }

                            else if(leftCenterValue.IsDefined() &&
                                    leftCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        -gridLength, 0.0);
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) == false &&
                                  AlmostEqual(currentY, lastY) == true &&
                                  currentX > lastX)
                          {
                            if(currentCenterValue.IsDefined() &&
                               currentCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, gridLength);
                            }

                            else if(bottomCenterValue.IsDefined() &&
                                    bottomCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        gridLength, 0.0);
                            }

                            else if(leftbottomCenterValue.IsDefined() &&
                                    leftbottomCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, -gridLength);
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) == false &&
                                  AlmostEqual(currentY, lastY) == true &&
                                  currentX < lastX)
                          {
                            if(leftbottomCenterValue.IsDefined() &&
                               leftbottomCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, -gridLength);
                            }

                            else if(leftCenterValue.IsDefined() &&
                                    leftCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        -gridLength, 0.0);
                            }

                            else if(currentCenterValue.IsDefined() &&
                                    currentCenterValue.GetBoolval() == true)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, gridLength);
                            }
                          }
                          
                          if(AlmostEqual(startX, currentX) == true &&
                             AlmostEqual(startY, currentY) == true)
                          {
                            bEnd = true;
                          }
                        }
                        
                        if(getDir(cycle) == false)
                        {
                          reverseCycle(cycle);
                        }
                        
                        cycles.push_back(cycle);

                        if(pBuildRegion != 0)
                        {
                          delete pBuildRegion;
                          pBuildRegion = 0;
                        }

                        pBuildRegion = buildRegion(cycles);
                      }
                    }

                    else if(value == 0)
                    {
                      startX = gridOriginX + index[0] * gridLength;
                      startY = gridOriginY + index[1] * gridLength;
                   
                      Point centerPoint(true,
                                        startX + halfGridLength,
                                        startY + halfGridLength);

                      if(robust::contains(*pBuildRegion, centerPoint) > 0 &&
                         robust::contains(*pBuildHoles, centerPoint) == 0)
                      {
                        vector<Point> cycle;

                        Point currentPoint(true, startX, startY);
                        cycle.push_back(currentPoint);

                        Point nextPoint(true, startX, startY + gridLength);
                        cycle.push_back(nextPoint);

                        double lastX = currentPoint.GetX();
                        double lastY = currentPoint.GetY();
                        double currentX = nextPoint.GetX();
                        double currentY = nextPoint.GetY();
                        bool bEnd = false;

                        while(bEnd == false)
                        {
                          CcBool currentCenterValue;
                          ptbool->atlocation(currentX + halfGridLength,
                                             currentY + halfGridLength,
                                             currentCenterValue);
                          CcBool leftCenterValue;
                          ptbool->atlocation(currentX - halfGridLength,
                                             currentY + halfGridLength,
                                             leftCenterValue);
                          CcBool leftbottomCenterValue;
                          ptbool->atlocation(currentX - halfGridLength,
                                             currentY - halfGridLength,
                                             leftbottomCenterValue);
                          CcBool bottomCenterValue;
                          ptbool->atlocation(currentX + halfGridLength,
                                             currentY - halfGridLength,
                                             bottomCenterValue);
                          Point leftCenterPoint(true,
                                                currentX - halfGridLength,
                                                currentY + halfGridLength);
                          Point currentCenterPoint(true,
                                                   currentX + halfGridLength,
                                                   currentY + halfGridLength);
                          Point bottomCenterPoint(true,
                                                  currentX + halfGridLength,
                                                  currentY - halfGridLength);
                          Point leftbottomCenterPoint(true,
                                                      currentX - halfGridLength,
                                                      currentY - halfGridLength
                                                     );

                          if(AlmostEqual(currentX, lastX) == true &&
                             AlmostEqual(currentY, lastY) == false &&
                             currentY > lastY)
                          {
                            if(leftCenterValue.IsDefined() &&
                               leftCenterValue.GetBoolval() == false &&
                               robust::contains(*pBuildRegion,
                                                leftCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        -gridLength, 0.0);
                            }

                            else if(currentCenterValue.IsDefined() &&
                                    currentCenterValue.GetBoolval() == false &&
                                    robust::contains(*pBuildRegion,
                                                     currentCenterPoint) > 0)

                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, gridLength);
                            }

                            else if(bottomCenterValue.IsDefined() &&
                                    bottomCenterValue.GetBoolval() == false &&
                                    robust::contains(*pBuildRegion,
                                                     bottomCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        gridLength, 0.0);
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) == true &&
                                  AlmostEqual(currentY, lastY) == false &&
                                  currentY < lastY)
                          {
                            if(bottomCenterValue.IsDefined() &&
                               bottomCenterValue.GetBoolval() == false &&
                               robust::contains(*pBuildRegion,
                                                bottomCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        gridLength, 0.0);
                            }

                            else if(leftbottomCenterValue.IsDefined() &&
                                    leftbottomCenterValue.GetBoolval()
                                    == false &&
                                    robust::contains(*pBuildRegion,
                                                     leftbottomCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, -gridLength);
                            }

                            else if(leftCenterValue.IsDefined() &&
                                    leftCenterValue.GetBoolval() == false &&
                                    robust::contains(*pBuildRegion,
                                                     leftCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        -gridLength, 0.0);
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) == false &&
                                  AlmostEqual(currentY, lastY) == true &&
                                  currentX > lastX)
                          {
                            if(currentCenterValue.IsDefined() &&
                               currentCenterValue.GetBoolval() == false &&
                               robust::contains(*pBuildRegion,
                                                currentCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, gridLength);
                            }

                            else if(bottomCenterValue.IsDefined() &&
                                    bottomCenterValue.GetBoolval() == false &&
                                    robust::contains(*pBuildRegion,
                                                     bottomCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        gridLength, 0.0);
                            }

                            else if(leftbottomCenterValue.IsDefined() &&
                                    leftbottomCenterValue.GetBoolval()
                                    == false &&
                                    robust::contains(*pBuildRegion,
                                                     leftbottomCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, -gridLength);
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) == false &&
                                  AlmostEqual(currentY, lastY) == true &&
                                  currentX < lastX)
                          {
                            if(leftbottomCenterValue.IsDefined() &&
                               leftbottomCenterValue.GetBoolval() == false &&
                               robust::contains(*pBuildRegion,
                                                leftbottomCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, -gridLength);
                            }

                            else if(leftCenterValue.IsDefined() &&
                                    leftCenterValue.GetBoolval() == false &&
                                    robust::contains(*pBuildRegion,
                                                     leftCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        -gridLength, 0.0);
                            }

                            else if(currentCenterValue.IsDefined() &&
                                    currentCenterValue.GetBoolval() == false &&
                                    robust::contains(*pBuildRegion,
                                                     currentCenterPoint) > 0)
                            {
                              UpdateCycleAndCoordinates(cycle, lastX, lastY,
                                                        currentX, currentY,
                                                        0.0, gridLength);
                            }
                          }

                          if(AlmostEqual(startX, currentX) == true &&
                             AlmostEqual(startY, currentY) == true)
                          {
                            bEnd = true; 
                          }
                        }
                        
                        holes.push_back(cycle);

                        if(pBuildHoles != 0)
                        {
                           delete pBuildHoles;
                           pBuildHoles = 0;
                        }

                        pBuildHoles = buildRegion(holes);

                        if(getDir(cycle) == true)
                        {
                          reverseCycle(cycle);
                        }
                        
                        cycles.push_back(cycle);

                        if(pBuildRegion != 0)
                        {
                          delete pBuildRegion;
                          pBuildRegion = 0;
                        }

                        pBuildRegion = buildRegion(cycles);
                      }
                    }
                  }
                }
              }
              
              *pResult = *pBuildRegion;

              delete pBuildRegion;
              pBuildRegion = 0;

              delete pBuildHoles;
              pBuildHoles = 0;
            }
          }

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
Method toregionTypeMappingFunction returns the return value type
of toregion operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of toregion operator
return value: return value type of toregion operator
exceptions: -

*/

ListExpr toregionTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator toregion expects a tbool.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(argument1 == tbool::BasicType())
    {
      type = NList(Region::BasicType()).listExpr();
    }
  }

  return type;
}

}
