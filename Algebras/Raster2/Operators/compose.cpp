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

#include "compose.h"

namespace raster2 {

  ValueMapping composeFuns[] =
  {
    composeFun<int, stype_helper<int> >,
    composeFun<double, stype_helper<double> >,
    composeFun<char, sbool_helper >,
    composeFun<string, stype_helper<string> >,
    0
  };

    ListExpr composeTypeMap(ListExpr args)
    {
      NList type(args);

      if (type.length() != 2) {
        return type.typeError("Expect one argument."); 
      }

      if (!type.first().isSymbol(MPoint::BasicType()))
      {
        return NList::typeError("Expected MPoint and sT.");
      }

      if (type.second().isSymbol(sint::BasicType())) {
         return NList(MInt::BasicType()).listExpr();
      }

      if (type.second().isSymbol(sreal::BasicType())) {
         return NList(MReal::BasicType()).listExpr();
      }

      if (type.second().isSymbol(sbool::BasicType())) {
         return NList(MBool::BasicType()).listExpr();
      }

      if (type.second().isSymbol(sstring::BasicType())) {
         return NList(MString::BasicType()).listExpr();
      }

      return NList::typeError("Expected MPoint and sT.");
    }

    int composeSelectFun(ListExpr args) {
        NList type(args);

        assert(type.first().isSymbol(MPoint::BasicType()));
        
        if (type.second().isSymbol(sint::BasicType())) {
            return 0;
        }
        
        if (type.second().isSymbol(sreal::BasicType())) {
            return 1;
        }
        
        if (type.second().isSymbol(sbool::BasicType())) {
            return 2;
        }
        
        if(type.second().isSymbol(sstring::BasicType())) {
            return 3;
        }
        
        return -1;
    }


double round(double r)
{
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

struct pointComparer {
  bool operator() (Point i, Point j)
  {
      if (i.GetX() == j.GetX())
      {
          return i.GetY() > j.GetY();
      }
      
      return i.GetX() > j.GetX();
  }
} myComparer;

bool compareVals (double i, double j) { return (i > j); }

double calculateSlope(double startX,
                      double startY,
                      double endX,
                      double endY)
{
    if (startX > endX)
    {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }

    double slope = (endY - startY) / (endX - startX);
    
    return slope;
}

// creates a vector of the intersecting points
// determined by the movement from (startx / starty)
// to (endx / endy) through the grid give by
// (originx / originy) and grid witdh length
vector<Point> calculatePoints(double startX,
                               double startY,
                               double endX,
                               double endY,
                               double originX,
                               double originY,
                               double length)
{
    vector<Point> points;

    bool sortDesc = startX > endX;
    
    if (startX > endX)
    {
        std::swap(startX, endX);
        std::swap(startY, endY);
    }
    
    // special case of a vertical line
    if (startX == endX)
    {
        if (startY == endY)
        {
            return points;
        }
        
        sortDesc = startX > endX;
        
        if (startY > endY)
        {
            std::swap(startY, endY);
        }

        double firstY = round(startY) / length;
        double currentY = firstY;
        
        while (currentY <= endY)
        {
            if (currentY > startY)
            {
                points.push_back(Point(true, startX, currentY));
            }
            
            firstY++;
            currentY = firstY * length;
        }

        if (points.empty())
        {
            return points;
        }
        
        if (sortDesc)
        {
            sort(points.begin(), points.end(), myComparer);    
        }
        else
        {
            sort(points.begin(), points.end());    
        }

        return points;
    }


    double slope = (endY - startY) / (endX - startX);
    
    vector<double> xValues;
    
    double firstX = round(startX) / length;
    double currentX = startX;
    
    xValues.push_back(startX);
    xValues.push_back(endX);
    
    currentX = firstX * length;
    
    while (currentX <= endX)
    {
        xValues.push_back(currentX);
        firstX++;
        currentX = firstX * length;
    }

    double firstY = round(startY) / length;
    double currentY = firstY;

    while (currentY <= endY)
    {
        double calculated = (currentY - startY) / slope + startX;
        
        if (calculated > startX)
        {
            xValues.push_back(calculated);
        }
        
        firstY++;
        currentY = firstY * length;
    }

    if (sortDesc)
    {
        sort(xValues.begin(), xValues.end(), compareVals);    
    }
    else
    {
        sort(xValues.begin(), xValues.end());    
    }
    

    for (vector<double>::iterator it = xValues.begin();
               it != xValues.end(); ++it)
    {
        double x = *it;
        double y = (*it - startX) * slope + startY;

        points.push_back(Point(true, x, y));
    }

    return points;
  }
}
