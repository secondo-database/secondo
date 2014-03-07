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
TileAlgebra includes

*/

#include "HalfSegment.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method CheckPoints checks the consistency of given left point and right point.

author: Dirk Zacher
parameters: rLeftPoint - reference to left point
            rRightPoint - reference to right point
return value: true, if left point and right point were exchanged,
              otherwise false
exceptions: -

*/

bool CheckPoints(Point& rLeftPoint,
                 Point& rRightPoint)
{
  bool bPointsExchanged = false;

  if(rLeftPoint.IsDefined() &&
     rRightPoint.IsDefined())
  {
    if((rLeftPoint.GetX() > rRightPoint.GetX()) ||
       (rLeftPoint.GetX() == rRightPoint.GetX() &&
        rLeftPoint.GetY() > rRightPoint.GetY()))
    {
      Point point = rLeftPoint;
      rLeftPoint = rRightPoint;
      rRightPoint = point;
      bPointsExchanged = true;
    }
  }

  return bPointsExchanged;
}

/*
Method CheckHalfSegment checks the consistency of the given HalfSegment.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
return value: -
exceptions: -

*/

void CheckHalfSegment(HalfSegment& rHalfSegment)
{
  Point leftPoint = rHalfSegment.GetLeftPoint();
  Point rightPoint = rHalfSegment.GetRightPoint();

  if(leftPoint.IsDefined() &&
     rightPoint.IsDefined())
  {
    bool bIsLeftDomPoint = rHalfSegment.IsLeftDomPoint();
    
    if(CheckPoints(leftPoint, rightPoint) == true)
    {
      rHalfSegment.Set(!bIsLeftDomPoint, leftPoint, rightPoint);
    }
  }
}

/*
Method IsHorizontalHalfSegment checks if given HalfSegment is a horizontal line.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
return value: true, if rHalfSegment is a horizontal line, otherwise false
exceptions: -

*/

bool IsHorizontalHalfSegment(const HalfSegment& rHalfSegment)
{
  bool bIsHorizontalHalfSegment = false;

  Point leftPoint = rHalfSegment.GetLeftPoint();
  Point rightPoint = rHalfSegment.GetRightPoint();

  if(leftPoint.IsDefined() &&
     rightPoint.IsDefined())
  {
    if(AlmostEqual(leftPoint.GetY(), rightPoint.GetY()))
    {
      bIsHorizontalHalfSegment = true;
    }
  }

  return bIsHorizontalHalfSegment;
}

/*
Method IsVerticalHalfSegment checks if given HalfSegment is a vertical line.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
return value: true, if rHalfSegment is a vertical line, otherwise false
exceptions: -

*/

bool IsVerticalHalfSegment(const HalfSegment& rHalfSegment)
{
  bool bIsVerticalHalfSegment = false;

  Point leftPoint = rHalfSegment.GetLeftPoint();
  Point rightPoint = rHalfSegment.GetRightPoint();

  if(leftPoint.IsDefined() &&
     rightPoint.IsDefined())
  {
    if(AlmostEqual(leftPoint.GetX(), rightPoint.GetX()))
    {
      bIsVerticalHalfSegment = true;
    }
  }

  return bIsVerticalHalfSegment;
}

/*
Method HalfSegmentIntersectsRectangle checks if given HalfSegment
intersects given Rectangle<2>.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
return value: true, if rHalfSegment intersects rRectangle, otherwise false
exceptions: -

*/

bool HalfSegmentIntersectsRectangle(const HalfSegment& rHalfSegment,
                                    const Rectangle<2>& rRectangle)
{
  bool bHalfSegmentIntersectsRectangle = false;

  if(rRectangle.IsDefined())
  {
    Point leftPoint = rHalfSegment.GetLeftPoint();
    Point rightPoint = rHalfSegment.GetRightPoint();

    if(leftPoint.IsDefined() &&
       rightPoint.IsDefined())
    {
      CheckPoints(leftPoint, rightPoint);

      if(leftPoint.GetX() >= rRectangle.MinD(0) &&
         leftPoint.GetX() < rRectangle.MaxD(0) &&
         leftPoint.GetY() >= rRectangle.MinD(1) &&
         leftPoint.GetY() < rRectangle.MaxD(1))
      {
        /*
        case: leftPoint in Rectangle

        */

        bHalfSegmentIntersectsRectangle = true;
      }

      if(rightPoint.GetX() >= rRectangle.MinD(0) &&
         rightPoint.GetX() < rRectangle.MaxD(0) &&
         rightPoint.GetY() >= rRectangle.MinD(1) &&
         rightPoint.GetY() < rRectangle.MaxD(1))
      {
        /*
        case: rightPoint in Rectangle

        */

        bHalfSegmentIntersectsRectangle = true;
      }

      if(bHalfSegmentIntersectsRectangle == false)
      {
        if(IsHorizontalHalfSegment(rHalfSegment) &&
           leftPoint.GetX() < rRectangle.MinD(0) &&
           rightPoint.GetX() >= rRectangle.MaxD(0) &&
           leftPoint.GetY() >= rRectangle.MinD(1) &&
           leftPoint.GetY() < rRectangle.MaxD(1))
        {
          /*
          case: horizontal line

          */

          bHalfSegmentIntersectsRectangle = true;
        }

        if(IsVerticalHalfSegment(rHalfSegment) &&
           leftPoint.GetX() >= rRectangle.MinD(0) &&
           leftPoint.GetX() < rRectangle.MaxD(0) &&
           leftPoint.GetY() < rRectangle.MinD(1) &&
           rightPoint.GetY() >= rRectangle.MaxD(1))
        {
          /*
          case: vertical line

          */

          bHalfSegmentIntersectsRectangle = true;
        }

        if(bHalfSegmentIntersectsRectangle == false)
        {
          if(leftPoint.GetX() < rRectangle.MinD(0) &&
             rightPoint.GetX() > rRectangle.MaxD(0))
          {
            double deltaX = rightPoint.GetX() - leftPoint.GetX();
            double deltaY = rightPoint.GetY() - leftPoint.GetY();
            double m = deltaY / deltaX;
            double n = leftPoint.GetY() - m * leftPoint.GetX();
  
            double rectangleLeftPointY = m * rRectangle.MinD(0) + n;
            double rectangleRightPointY = m * rRectangle.MaxD(0) + n;
  
            if((rectangleLeftPointY >= rRectangle.MinD(1) &&
                rectangleLeftPointY < rRectangle.MaxD(1)) ||
               (rectangleRightPointY >= rRectangle.MinD(1) &&
                rectangleRightPointY < rRectangle.MaxD(1)))
            {
              bHalfSegmentIntersectsRectangle = true;
            }
          }
        }
      }
    }
  }

  return bHalfSegmentIntersectsRectangle;
}

/*
Method GetPointsInRectangle returns a left point and a right point
of given HalfSegment inside the given Rectangle<2>.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
            rLeftPoint - reference to the left point inside rRectangle
            rRightPoint - reference to the right point inside rRectangle
return value: true, if left point and right point were successfully calculated,
              otherwise false
exceptions: -

*/

bool GetPointsInRectangle(const HalfSegment& rHalfSegment,
                          const Rectangle<2>& rRectangle,
                          Point& rLeftPoint,
                          Point& rRightPoint)
{
  bool bRetVal = false;

  if(rRectangle.IsDefined())
  {
    if(HalfSegmentIntersectsRectangle(rHalfSegment, rRectangle))
    {
      Point halfSegmentLeftPoint = rHalfSegment.GetLeftPoint();
      Point halfSegmentRightPoint = rHalfSegment.GetRightPoint();

      if(halfSegmentLeftPoint.IsDefined() &&
         halfSegmentRightPoint.IsDefined())
      {
        CheckPoints(halfSegmentLeftPoint, halfSegmentRightPoint);
        
        rLeftPoint.Set(halfSegmentLeftPoint);
        rRightPoint.Set(halfSegmentRightPoint);

        if(IsHorizontalHalfSegment(rHalfSegment))
        {
          if(rLeftPoint.GetX() < rRectangle.MinD(0))
          {
            rLeftPoint.Set(rRectangle.MinD(0), rLeftPoint.GetY());
          }

          if(rLeftPoint.GetX() > rRectangle.MaxD(0))
          {
            rLeftPoint.Set(rRectangle.MaxD(0), rLeftPoint.GetY());
          }

          if(rRightPoint.GetX() < rRectangle.MinD(0))
          {
            rRightPoint.Set(rRectangle.MinD(0), rRightPoint.GetY());
          }

          if(rRightPoint.GetX() > rRectangle.MaxD(0))
          {
            rRightPoint.Set(rRectangle.MaxD(0), rRightPoint.GetY());
          }
        }

        else if(IsVerticalHalfSegment(rHalfSegment))
        {
          if(rLeftPoint.GetY() < rRectangle.MinD(1))
          {
            rLeftPoint.Set(rLeftPoint.GetX(), rRectangle.MinD(1));
          }

          if(rLeftPoint.GetY() > rRectangle.MaxD(1))
          {
            rLeftPoint.Set(rLeftPoint.GetX(), rRectangle.MaxD(1));
          }

          if(rRightPoint.GetY() < rRectangle.MinD(1))
          {
            rRightPoint.Set(rRightPoint.GetX(), rRectangle.MinD(1));
          }

          if(rRightPoint.GetY() > rRectangle.MaxD(1))
          {
            rRightPoint.Set(rRightPoint.GetX(), rRectangle.MaxD(1));
          }
        }

        else
        {
          double deltaX = halfSegmentRightPoint.GetX() -
                          halfSegmentLeftPoint.GetX();
          double deltaY = halfSegmentRightPoint.GetY() -
                          halfSegmentLeftPoint.GetY();
          double m = deltaY / deltaX;
          double n = halfSegmentLeftPoint.GetY() -
                     m * halfSegmentLeftPoint.GetX();

          if(rLeftPoint.GetX() < rRectangle.MinD(0))
          {
            rLeftPoint.Set(rRectangle.MinD(0), m * rRectangle.MinD(0) + n);
          }

          if(rLeftPoint.GetX() > rRectangle.MaxD(0))
          {
            rLeftPoint.Set(rRectangle.MaxD(0), m * rRectangle.MaxD(0) + n);
          }

          if(rRightPoint.GetX() < rRectangle.MinD(0))
          {
            rRightPoint.Set(rRectangle.MinD(0), m * rRectangle.MinD(0) + n);
          }

          if(rRightPoint.GetX() > rRectangle.MaxD(0))
          {
            rRightPoint.Set(rRectangle.MaxD(0), m * rRectangle.MaxD(0) + n);
          }
        }

        bRetVal = true;
      }
    }
  }

  return bRetVal;
}

}
