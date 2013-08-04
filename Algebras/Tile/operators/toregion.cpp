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
                  Index<2> index = (int[]){column, row};
                  char value = ptbool->GetValue(index);

                  if(tProperties<char>::TypeProperties::
                     IsUndefinedValue(value) == false)
                  {
                    if(value == 1)
                    {
                      startX = gridOriginX + index[0] * gridLength;
                      startY = gridOriginY + index[1] * gridLength;
                      
                      Point help(true, startX + halfGridLength,
                                 startY + halfGridLength);
                      
                      if(robust::contains(*pBuildRegion, help) == 0)
                      {
                        vector<Point> cycle;

                        Point point1(true, startX, startY);
                        cycle.push_back(point1);

                        Point point2(true, startX, startY + gridLength);
                        cycle.push_back(point2);

                        double currentX = startX;
                        double currentY = startY + gridLength;

                        double lastX = startX;
                        double lastY = startY;

                        bool bEnd = false;

                        while(bEnd == false)
                        {
                          if(AlmostEqual(currentX, lastX) &&
                             !(AlmostEqual(currentY, lastY)) &&
                             currentY > lastY)
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY + halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY + halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY - halfGridLength,
                                               value3);

                            if(value1.IsDefined() &&
                               value1.GetBoolval() == true)
                            {
                              Point point(true, currentX - gridLength,
                                          currentY);
                              cycle.push_back(point);         

                              lastX = currentX;
                              lastY = currentY;
                              currentX -= gridLength;
                            }

                            else if(value2.IsDefined() &&
                                    value2.GetBoolval() == true)
                            {
                              Point point(true, currentX,
                                          currentY + gridLength);
                              cycle.push_back(point);        

                              lastX = currentX;
                              lastY = currentY;
                              currentY += gridLength;
                            }

                            else if(value3.IsDefined() &&
                                    value3.GetBoolval() == true)
                            {
                              Point point(true, currentX + gridLength,
                                          currentY);
                              cycle.push_back(point);         

                              lastX = currentX;
                              lastY = currentY;
                              currentX += gridLength;
                            }
                          }

                          else if(AlmostEqual(currentX, lastX)  &&
                                  !(AlmostEqual(currentY, lastY)) &&
                                  currentY < lastY )
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY - halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY - halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY + halfGridLength,
                                               value3);

                            if(value1.IsDefined() &&
                               value1.GetBoolval() == true)
                            {
                              Point point(true, currentX + gridLength,
                                          currentY);
                              cycle.push_back(point);       

                              lastX = currentX;
                              lastY = currentY;
                              currentX += gridLength;
                            }

                            else if(value2.IsDefined() &&
                                    value2.GetBoolval() == true)
                            {
                              Point point(true, currentX,
                                          currentY - gridLength);
                              cycle.push_back(point);

                              lastX = currentX;
                              lastY = currentY;
                              currentY -= gridLength;
                            }

                            else if(value3.IsDefined() &&
                                    value3.GetBoolval() == true)
                            {
                              Point point(true, currentX - gridLength,
                                          currentY);
                              cycle.push_back(point);         
             
                              lastX = currentX;
                              lastY = currentY;
                              currentX -= gridLength;
                            }
                          }

                          else if(!(AlmostEqual(currentX, lastX)) &&
                                  currentX > lastX &&
                                  AlmostEqual(currentY, lastY) )
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY + halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY - halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY - halfGridLength,
                                               value3);

                            if(value1.IsDefined() &&
                               value1.GetBoolval() == true)
                            {
                              Point point(true, currentX,
                                          currentY + gridLength);
                              cycle.push_back(point);
                 
                              lastX = currentX;
                              lastY = currentY;
                              currentY += gridLength;
                            }

                            else if(value2.IsDefined() &&
                                    value2.GetBoolval() == true)
                            {
                              Point point(true, currentX + gridLength,
                                          currentY);
                              cycle.push_back(point);
                 
                              lastX = currentX;
                              lastY = currentY;
                              currentX += gridLength;
                            }

                            else if(value3.IsDefined() &&
                                    value3.GetBoolval() == true)
                            {
                              Point point(true, currentX,
                                          currentY - gridLength);
                              cycle.push_back(point);        
             
                              lastX = currentX;
                              lastY = currentY;
                              currentY -= gridLength;
                            }
                          }

                          else if(!(AlmostEqual(currentX, lastX)) &&
                                  currentX < lastX &&
                                  AlmostEqual(currentY, lastY))
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY - halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY + halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY + halfGridLength,
                                               value3);

                            if(value1.IsDefined() &&
                               value1.GetBoolval() == true)
                            {
                              Point point(true, currentX,
                                          currentY - gridLength);
                              cycle.push_back(point);     

                              lastX = currentX;
                              lastY = currentY;
                              currentY -= gridLength;
                            }

                            else if(value2.IsDefined() &&
                                    value2.GetBoolval() == true)
                            {
                              Point point(true, currentX - gridLength,
                                          currentY);
                              cycle.push_back(point);
                 
                              lastX = currentX;
                              lastY = currentY;
                              currentX -= gridLength;
                            }

                            else if(value3.IsDefined() &&
                                    value3.GetBoolval() == true)
                            {
                              Point point(true, currentX,
                                          currentY + gridLength);
                              cycle.push_back(point);
                      
                              lastX = currentX;
                              lastY = currentY;
                              currentY += gridLength;
                            }
                          }
                          
                          if(AlmostEqual(startX, currentX) && 
                             AlmostEqual(startY, currentY))
                          {
                            bEnd = true;
                          }
                        }
                        
                        if(!getDir(cycle))
                        {
                          reverseCycle(cycle);
                        }
                        
                        cycles.push_back(cycle);

                        if(pBuildRegion)
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
                   
                      Point help(true, startX + halfGridLength,
                                 startY + halfGridLength);

                      if((robust::contains(*pBuildRegion, help) > 0) &&
                         (robust::contains(*pBuildHoles, help) == 0))
                      {
                        vector<Point> cycle;

                        Point point1(true, startX, startY);
                        cycle.push_back(point1);

                        Point point2(true, startX, startY + gridLength);
                        cycle.push_back(point2);

                        double currentX = startX;
                        double currentY = startY + gridLength;

                        double lastX = startX;
                        double lastY = startY;

                        bool bEnd = false;

                        while(bEnd == false)
                        {
                          Point p1(true,
                                   currentX - halfGridLength,
                                   currentY + halfGridLength);
                          Point p2(true,
                                   currentX + halfGridLength,
                                   currentY + halfGridLength);
                          Point p3(true,
                                   currentX + halfGridLength,
                                   currentY - halfGridLength);
                          Point p4(true,
                                   currentX - halfGridLength,
                                   currentY - halfGridLength);

                          if(AlmostEqual(currentX, lastX) &&
                             !(AlmostEqual(currentY, lastY)) &&
                             currentY > lastY)
                          {    
                            CcBool value1;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY + halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY + halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY - halfGridLength,
                                               value3);

                            if((value1.IsDefined() &&
                                value1.GetBoolval() == false) &&
                               (robust::contains(*pBuildRegion, p1) > 0))
                            {
                              Point point(true, currentX - gridLength,
                                          currentY);
                              cycle.push_back(point);

                              lastX = currentX;
                              lastY = currentY;
                              currentX -= gridLength;
                            }

                            else if((value2.IsDefined() &&
                                     value2.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p2) > 0))

                            {
                              Point point(true, currentX,
                                          currentY + gridLength);
                              cycle.push_back(point);        

                              lastX = currentX;
                              lastY = currentY;
                              currentY += gridLength;
                            }

                            else if((value3.IsDefined() &&
                                     value3.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p3) > 0))
                            {
                              Point point(true, currentX + gridLength,
                                          currentY);
                              cycle.push_back(point);         

                              lastX = currentX;
                              lastY = currentY;
                              currentX += gridLength;
                            }
                          }

                          else if(AlmostEqual(currentX, lastX) &&
                                  !(AlmostEqual(currentY, lastY)) &&
                                  currentY < lastY)
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY - halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY - halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY + halfGridLength,
                                               value3);

                            if((value1.IsDefined() &&
                                value1.GetBoolval() == false) &&
                               (robust::contains(*pBuildRegion, p3) > 0))
                            {
                              Point point(true, currentX + gridLength,
                                          currentY);
                              cycle.push_back(point);       

                              lastX = currentX;
                              lastY = currentY;
                              currentX += gridLength;
                            }

                            else if((value2.IsDefined() &&
                                     value2.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p4) > 0))
                            {
                              Point point(true, currentX,
                                          currentY - gridLength);
                              cycle.push_back(point);

                              lastX = currentX;
                              lastY = currentY;
                              currentY -= gridLength;
                            }

                            else if((value3.IsDefined() &&
                                     value3.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p1) > 0))
                            {
                              Point point(true, currentX - gridLength,
                                          currentY);
                              cycle.push_back(point);         
             
                              lastX = currentX;
                              lastY = currentY;
                              currentX -= gridLength;
                            }
                          }

                          else if(!(AlmostEqual(currentX, lastX)) &&
                                  currentX > lastX &&
                                  AlmostEqual(currentY, lastY) )
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY + halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY - halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY - halfGridLength,
                                               value3);

                            if((value1.IsDefined() &&
                                value1.GetBoolval() == false) &&
                               (robust::contains(*pBuildRegion, p2) > 0))
                            {
                              Point point(true, currentX,
                                          currentY + gridLength);
                              cycle.push_back(point);
                 
                              lastX = currentX;
                              lastY = currentY;
                              currentY += gridLength;
                            }

                            else if((value2.IsDefined() &&
                                     value2.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p3) > 0))
                            {
                              Point point(true, currentX + gridLength,
                                          currentY);
                              cycle.push_back(point);
                 
                              lastX = currentX;
                              lastY = currentY;
                              currentX += gridLength;
                            }

                            else if((value3.IsDefined() &&
                                     value3.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p4) > 0))
                            {
                              Point point(true, currentX,
                                          currentY - gridLength);
                              cycle.push_back(point);        
             
                              lastX = currentX;
                              lastY = currentY;
                              currentY -= gridLength;
                            }
                          }

                          else if(!(AlmostEqual(currentX, lastX)) &&
                                  currentX < lastX &&
                                  AlmostEqual(currentY, lastY) )
                          {
                            CcBool value1;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY - halfGridLength,
                                               value1);
                            CcBool value2;
                            ptbool->atlocation(currentX - halfGridLength,
                                               currentY + halfGridLength,
                                               value2);
                            CcBool value3;
                            ptbool->atlocation(currentX + halfGridLength,
                                               currentY + halfGridLength,
                                               value3);

                            if((value1.IsDefined() &&
                                value1.GetBoolval() == false) &&
                               (robust::contains(*pBuildRegion, p4) > 0))
                            {
                              Point point(true, currentX,
                                          currentY - gridLength);
                              cycle.push_back(point);     

                              lastX = currentX;
                              lastY = currentY;
                              currentY -= gridLength;
                            }

                            else if((value2.IsDefined() &&
                                     value2.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p1) > 0))
                            {
                              Point point(true, currentX - gridLength,
                                          currentY);
                              cycle.push_back(point);
                 
                              lastX = currentX;
                              lastY = currentY;
                              currentX -= gridLength;
                            }

                            else if((value3.IsDefined() &&
                                     value3.GetBoolval() == false) &&
                                    (robust::contains(*pBuildRegion, p2) > 0))
                            {
                              Point point(true, currentX,
                                          currentY + gridLength);
                              cycle.push_back(point);
                      
                              lastX = currentX;
                              lastY = currentY;
                              currentY += gridLength;
                            }
                          }

                          if(AlmostEqual(startX, currentX) &&
                             AlmostEqual(startY, currentY))
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

                        if(getDir(cycle))
                        {
                          reverseCycle(cycle);
                        }
                        
                        cycles.push_back(cycle);

                        if(pBuildRegion)
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
              
              std::swap(*pResult, *pBuildRegion);

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
