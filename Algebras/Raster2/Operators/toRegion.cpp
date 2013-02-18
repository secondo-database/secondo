/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#include "toRegion.h"
#include "RobustSetOps.h"
#include "RegionTools.h"

namespace raster2 {

    int toRegionFun
        (Word* args, Word& result, int message, Word& local, Supplier s)
    {
        result = qp->ResultStorage(s);
    
        Region* re = static_cast<Region*>(result.addr);
        sbool* sbool1 = static_cast<sbool*>(args[0].addr);

        grid2 grid = sbool1->getGrid();

        double length = grid.getLength();

        HalfSegment hs;

        Region* buildRegionResult = new Region(0);
        Region* buildHolesResult = new Region(0);
        vector<vector<Point> > cycles;
        vector<vector<Point> > holes;
        
        double startX = 0.0;
        double startY = 0.0;

        Rectangle<2> bbox = sbool1->bbox();
      
        double gridOriginX = grid.getOriginX();
        double gridOriginY = grid.getOriginY();

        RasterIndex<2> from = grid.getIndex(bbox.MinD(0), bbox.MinD(1));
        RasterIndex<2> to = grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));
    
        for (RasterIndex<2> index=from; index < to; index.increment(from, to))
        {
           if ( sbool1->get(index) == true )
           {
              startX = index[0] * length + gridOriginX;
              startY = index[1] * length + gridOriginY;
              
              Point help(true, startX + length/2, startY + length/2);
              
              if ( robust::contains(*buildRegionResult, help) == false )
              {
                vector<Point> cycle;

                Point point1(true, startX, startY);
                cycle.push_back(point1);

                Point point2(true, startX, startY + length);
                cycle.push_back(point2);
         

                double currentX = startX;
                double currentY = startY + length;

                double lastX = startX;
                double lastY = startY;
  
                bool ende = false;

                while ( ende == false )
                {
                  if ( AlmostEqual(currentX, lastX) 
                  && !(AlmostEqual(currentY, lastY)) && currentY > lastY )
                  {
                    if (sbool1->atlocation(currentX - length/2, 
                                           currentY + length/2) == true )
                    {
                      Point point(true, currentX - length, currentY);
                      cycle.push_back(point);         
   
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX - length;
                    }
                    else if ( sbool1->atlocation(currentX + length/2, 
                                                 currentY + length/2) == true )
                    {
                      Point point(true, currentX, currentY + length);
                      cycle.push_back(point);        

                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY + length;
                    }
                    else if ( sbool1->atlocation(currentX + length/2, 
                                                 currentY - length/2) == true )
                    {
                      Point point(true, currentX + length, currentY);
                      cycle.push_back(point);         
  
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX + length;
                    }
                  }
                  else if ( AlmostEqual(currentX, lastX) 
                       && !(AlmostEqual(currentY, lastY)) && currentY < lastY )
                  {
                    if ( sbool1->atlocation(currentX + length/2, 
                                            currentY - length/2) == true )
                    {
                      Point point(true, currentX + length, currentY);
                      cycle.push_back(point);       
   
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX + length;
                    }
                    else if (sbool1->atlocation(currentX - length/2,
                                                currentY - length/2) == true )
                    {
                      Point point(true, currentX, currentY - length);
                      cycle.push_back(point);

                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY - length;
                    }
                    else if ( sbool1->atlocation(currentX - length/2, 
                                                 currentY + length/2) == true )
                    {
                      Point point(true, currentX - length, currentY);
                      cycle.push_back(point);         
     
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX - length;
                    }
                  }
                  else if ( !(AlmostEqual(currentX, lastX)) && currentX > lastX
                           && AlmostEqual(currentY, lastY) )
                  {
                    if ( sbool1->atlocation(currentX + length/2, 
                                            currentY + length/2) == true )
                    {
                      Point point(true, currentX, currentY + length);
                      cycle.push_back(point);
         
                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY + length;
                    }
                    else if ( sbool1->atlocation(currentX + length/2, 
                                                 currentY - length/2) == true )
                    {
                      Point point(true, currentX + length, currentY);
                      cycle.push_back(point);
         
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX + length;
                    }
                    else if (sbool1->atlocation(currentX - length/2,
                                                currentY - length/2) == true )
                    {
                      Point point(true, currentX, currentY - length);
                      cycle.push_back(point);        
     
                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY - length;
                    }
                  }
                  else if ( !(AlmostEqual(currentX, lastX)) && currentX < lastX
                           && AlmostEqual(currentY, lastY) )
                  {
                    if ( sbool1->atlocation(currentX - length/2, 
                                            currentY - length/2) == true )
                    {
                      Point point(true, currentX, currentY - length);
                      cycle.push_back(point);     

                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY - length;
                    }
                    else if ( sbool1->atlocation(currentX - length/2, 
                                                 currentY + length/2) == true )
                    {
                      Point point(true, currentX - length, currentY);
                      cycle.push_back(point);
         
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX - length;
                    }
                    else if ( sbool1->atlocation(currentX + length/2, 
                                                 currentY + length/2) == true )
                    {
                      Point point(true, currentX, currentY + length);
                      cycle.push_back(point);
              
                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY + length;
                    }
                  }
                  
                  if ( AlmostEqual(startX, currentX) && 
                       AlmostEqual(startY, currentY) )
                  {
                    ende = true; 
                  }
                }
                
                if(!getDir(cycle))
                {
                    reverseCycle(cycle);
                }
                
                cycles.push_back(cycle);
                buildRegionResult = buildRegion(cycles);
              }
            }
            else if ( sbool1->get(index) == false )
            {
              startX = index[0] * length + gridOriginX;
              startY = index[1] * length + gridOriginY;
           
              Point help(true, startX + length/2, startY + length/2);

              if ( (robust::contains(*buildRegionResult, help) == true) &&
                    (robust::contains(*buildHolesResult, help) == false) )
              {
                vector<Point> cycle;

                Point point1(true, startX, startY);
                cycle.push_back(point1);

                Point point2(true, startX, startY + length);
                cycle.push_back(point2);

                double currentX = startX;
                double currentY = startY + length;

                double lastX = startX;
                double lastY = startY;
  
                bool ende = false;

                while ( ende == false )
                {
                  Point p1(true, currentX - length/2, currentY + length/2);
                  Point p2(true, currentX + length/2, currentY + length/2);
                  Point p3(true, currentX + length/2, currentY - length/2);
                  Point p4(true, currentX - length/2, currentY - length/2);

                  if ( AlmostEqual(currentX, lastX) 
                  && !(AlmostEqual(currentY, lastY)) && currentY > lastY )
                  {    
                    if ((sbool1->atlocation(currentX - length/2,
                                            currentY + length/2) == false)
                       && (robust::contains(*buildRegionResult, p1) == true))
                    {
                      Point point(true, currentX - length, currentY);
                      cycle.push_back(point);         
   
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX - length;
                    }
                    else if ((sbool1->atlocation(currentX + length/2, 
                                                 currentY + length/2) == false)
                         && (robust::contains(*buildRegionResult, p2) == true))
                    {
                      Point point(true, currentX, currentY + length);
                      cycle.push_back(point);        

                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY + length;
                    }
                    else if ((sbool1->atlocation(currentX + length/2, 
                                                 currentY - length/2) == false)
                         && (robust::contains(*buildRegionResult, p3) == true))
                    {
                      Point point(true, currentX + length, currentY);
                      cycle.push_back(point);         
  
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX + length;
                    }
                  }
                  else if ( AlmostEqual(currentX, lastX) 
                       && !(AlmostEqual(currentY, lastY)) && currentY < lastY )
                  {
                    if ((sbool1->atlocation(currentX + length/2,
                                            currentY - length/2) == false)
                       && (robust::contains(*buildRegionResult, p3) == true))
                    {
                      Point point(true, currentX + length, currentY);
                      cycle.push_back(point);       
   
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX + length;
                    }
                    else if ((sbool1->atlocation(currentX - length/2,
                                                 currentY - length/2) == false) 
                         && (robust::contains(*buildRegionResult, p4) == true))
                    {
                      Point point(true, currentX, currentY - length);
                      cycle.push_back(point);

                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY - length;
                    }
                    else if ((sbool1->atlocation(currentX - length/2, 
                                                 currentY + length/2) == false) 
                         && (robust::contains(*buildRegionResult, p1) == true))
                    {
                      Point point(true, currentX - length, currentY);
                      cycle.push_back(point);         
     
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX - length;
                    }
                  }
                  else if ( !(AlmostEqual(currentX, lastX)) && currentX > lastX
                           && AlmostEqual(currentY, lastY) )
                  {
                    if ((sbool1->atlocation(currentX + length/2, 
                                            currentY + length/2) == false) 
                       && (robust::contains(*buildRegionResult, p2) == true))
                    {
                      Point point(true, currentX, currentY + length);
                      cycle.push_back(point);
         
                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY + length;
                    }
                    else if ((sbool1->atlocation(currentX + length/2, 
                                                 currentY - length/2) == false)
                         && (robust::contains(*buildRegionResult, p3) == true))
                    {
                      Point point(true, currentX + length, currentY);
                      cycle.push_back(point);
         
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX + length;
                    }
                    else if ((sbool1->atlocation(currentX - length/2,
                                                 currentY - length/2) == false) 
                         && (robust::contains(*buildRegionResult, p4) == true))
                    {
                      Point point(true, currentX, currentY - length);
                      cycle.push_back(point);        
     
                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY - length;
                    }
                  }
                  else if ( !(AlmostEqual(currentX, lastX)) && currentX < lastX
                           && AlmostEqual(currentY, lastY) )
                  {
                    if ((sbool1->atlocation(currentX - length/2, 
                                            currentY - length/2) == false) 
                       && (robust::contains(*buildRegionResult, p4) == true))
                    {
                      Point point(true, currentX, currentY - length);
                      cycle.push_back(point);     

                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY - length;
                    }
                    else if ((sbool1->atlocation(currentX - length/2, 
                                                 currentY + length/2) == false) 
                         && (robust::contains(*buildRegionResult, p1) == true))
                    {
                      Point point(true, currentX - length, currentY);
                      cycle.push_back(point);
         
                      lastX = currentX;
                      lastY = currentY;
                      currentX = currentX - length;
                    }
                    else if ((sbool1->atlocation(currentX + length/2, 
                                                 currentY + length/2) == false)
                         && (robust::contains(*buildRegionResult, p2) == true))
                    {
                      Point point(true, currentX, currentY + length);
                      cycle.push_back(point);
              
                      lastX = currentX;
                      lastY = currentY;
                      currentY = currentY + length;
                    }
                  }

                  if ( AlmostEqual(startX, currentX) && 
                       AlmostEqual(startY, currentY) )
                  {
                    ende = true; 
                  }
                }
                
                holes.push_back(cycle);
                buildHolesResult = buildRegion(holes);

                if(getDir(cycle))
                {
                    reverseCycle(cycle);
                }
                
                cycles.push_back(cycle);
                buildRegionResult = buildRegion(cycles);
              }
           }
        }
        
        std::swap(*re, *buildRegionResult);
        delete buildRegionResult;
        delete buildHolesResult;

        return 0;
    }

    ListExpr toRegionTypeMap(ListExpr args)
    {
        NList type(args);

        if (type.length() != 1)
            return type.typeError("Expect one argument.");

        if ( type.first() == NList(sbool::BasicType()))
        {
            return NList(Region::BasicType()).listExpr();
        }

        return NList::typeError
                 ("Expecting a sbool.");
    }
}
