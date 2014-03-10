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
Method IsPointInRectangle checks if given Point is in given Rectangle<2>.

author: Dirk Zacher
parameters: rPoint - reference to a point
            rRectangle - reference to a rectangle
return value: true, if rPoint is in rRectangle,
              otherwise false
exceptions: -

*/

bool IsPointInRectangle(const Point& rPoint,
                        const Rectangle<2>& rRectangle)
{
  bool bIsPointInRectangle = false;

  if(rPoint.IsDefined() &&
     rRectangle.IsDefined())
  {
    if(rPoint.GetX() >= rRectangle.MinD(0) &&
       rPoint.GetX() < rRectangle.MaxD(0) &&
       rPoint.GetY() >= rRectangle.MinD(1) &&
       rPoint.GetY() < rRectangle.MaxD(1))
    {
      bIsPointInRectangle = true;
    }
  }

  return bIsPointInRectangle;
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
Method HalfSegmentPointsInRectangle returns the number of HalfSegment points
inside the given Rectangle<2>.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
return value: number of HalfSegment points inside rRectangle
exceptions: -

*/

unsigned char HalfSegmentPointsInRectangle(const HalfSegment& rHalfSegment,
                                           const Rectangle<2>& rRectangle)
{
  unsigned char halfSegmentPointsInRectangle = 0;

  if(rRectangle.IsDefined())
  {
    Point leftPoint = rHalfSegment.GetLeftPoint();
    Point rightPoint = rHalfSegment.GetRightPoint();

    if(leftPoint.IsDefined() &&
       rightPoint.IsDefined())
    {
      CheckPoints(leftPoint, rightPoint);

      if(IsPointInRectangle(leftPoint, rRectangle))
      {
        halfSegmentPointsInRectangle++;
      }

      if(IsPointInRectangle(rightPoint, rRectangle))
      {
        halfSegmentPointsInRectangle++;
      }
    }
  }

  return halfSegmentPointsInRectangle;
}

/*
Method HalfSegmentIntersectsLeftRectangleBorder checks if given HalfSegment
intersects the left border of given Rectangle<2> and returns
the intersection point between HalfSegment and left rectangle border.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
            rPoint - reference to a Point containing intersection coordinates
return value: true, if rHalfSegment intersects left border of rRectangle,
              otherwise false
exceptions: -

*/

bool HalfSegmentIntersectsLeftRectangleBorder(const HalfSegment& rHalfSegment,
                                              const Rectangle<2>& rRectangle,
                                              Point& rPoint)
{
  bool bHalfSegmentIntersectsLeftRectangleBorder = false;

  if(rRectangle.IsDefined())
  {
    Point halfSegmentLeftPoint = rHalfSegment.GetLeftPoint();
    Point halfSegmentRightPoint = rHalfSegment.GetRightPoint();

    if(halfSegmentLeftPoint.IsDefined() &&
       halfSegmentRightPoint.IsDefined())
    {
      CheckPoints(halfSegmentLeftPoint, halfSegmentRightPoint);
      
      if(halfSegmentLeftPoint.GetX() <= rRectangle.MinD(0) &&
          rRectangle.MinD(0) <= halfSegmentRightPoint.GetX())
      {
        double deltaX = halfSegmentRightPoint.GetX() -
                        halfSegmentLeftPoint.GetX();
        double deltaY = halfSegmentRightPoint.GetY() -
                        halfSegmentLeftPoint.GetY();
        double m = deltaY / deltaX;
        double n = halfSegmentLeftPoint.GetY() -
                   m * halfSegmentLeftPoint.GetX();
  
        double y = m * rRectangle.MinD(0) + n;
  
        if(y >= rRectangle.MinD(1) &&
           y < rRectangle.MaxD(1))
        {
          bHalfSegmentIntersectsLeftRectangleBorder = true;
          rPoint.Set(rRectangle.MinD(0), y);
        }
      }
    }
  }

  return bHalfSegmentIntersectsLeftRectangleBorder;
}

/*
Method HalfSegmentIntersectsRightRectangleBorder checks if given HalfSegment
intersects the right border of given Rectangle<2> and returns
the intersection point between HalfSegment and right rectangle border.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
            rPoint - reference to a Point containing intersection coordinates
return value: true, if rHalfSegment intersects right border of rRectangle,
              otherwise false
exceptions: -

*/

bool HalfSegmentIntersectsRightRectangleBorder(const HalfSegment& rHalfSegment,
                                               const Rectangle<2>& rRectangle,
                                               Point& rPoint)
{
  bool bHalfSegmentIntersectsRightRectangleBorder = false;

  if(rRectangle.IsDefined())
  {
    Point halfSegmentLeftPoint = rHalfSegment.GetLeftPoint();
    Point halfSegmentRightPoint = rHalfSegment.GetRightPoint();

    if(halfSegmentLeftPoint.IsDefined() &&
       halfSegmentRightPoint.IsDefined())
    {
      CheckPoints(halfSegmentLeftPoint, halfSegmentRightPoint);
      
      if(halfSegmentLeftPoint.GetX() <= rRectangle.MaxD(0) &&
         rRectangle.MaxD(0) <= halfSegmentRightPoint.GetX())
      {
        double deltaX = halfSegmentRightPoint.GetX() -
                        halfSegmentLeftPoint.GetX();
        double deltaY = halfSegmentRightPoint.GetY() -
                        halfSegmentLeftPoint.GetY();
        double m = deltaY / deltaX;
        double n = halfSegmentLeftPoint.GetY() -
                   m * halfSegmentLeftPoint.GetX();
  
        double y = m * rRectangle.MaxD(0) + n;
  
        if(y >= rRectangle.MinD(1) &&
           y < rRectangle.MaxD(1))
        {
          bHalfSegmentIntersectsRightRectangleBorder = true;
          rPoint.Set(rRectangle.MaxD(0), y);
        }
      }
    }
  }

  return bHalfSegmentIntersectsRightRectangleBorder;
}

/*
Method HalfSegmentIntersectsLowerRectangleBorder checks if given HalfSegment
intersects the lower border of given Rectangle<2> and returns
the intersection point between HalfSegment and lower rectangle border.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
            rPoint - reference to a Point containing intersection coordinates
return value: true, if rHalfSegment intersects lower border of rRectangle,
              otherwise false
exceptions: -

*/

bool HalfSegmentIntersectsLowerRectangleBorder(const HalfSegment& rHalfSegment,
                                               const Rectangle<2>& rRectangle,
                                               Point& rPoint)
{
  bool bHalfSegmentIntersectsLowerRectangleBorder = false;

  if(rRectangle.IsDefined())
  {
    Point halfSegmentLeftPoint = rHalfSegment.GetLeftPoint();
    Point halfSegmentRightPoint = rHalfSegment.GetRightPoint();

    if(halfSegmentLeftPoint.IsDefined() &&
       halfSegmentRightPoint.IsDefined())
    {
      CheckPoints(halfSegmentLeftPoint, halfSegmentRightPoint);

      double deltaX = halfSegmentRightPoint.GetX() -
                      halfSegmentLeftPoint.GetX();
      double deltaY = halfSegmentRightPoint.GetY() -
                      halfSegmentLeftPoint.GetY();
      double m = deltaY / deltaX;
      double n = halfSegmentLeftPoint.GetY() -
                 m * halfSegmentLeftPoint.GetX();

      double x = (rRectangle.MinD(1) - n) / m;

      if(halfSegmentLeftPoint.GetX() <= x &&
         x <= halfSegmentRightPoint.GetX() &&
         x >= rRectangle.MinD(0) &&
         x < rRectangle.MaxD(0))
      {
        bHalfSegmentIntersectsLowerRectangleBorder = true;
        rPoint.Set(x, rRectangle.MinD(1));
      }
    }
  }

  return bHalfSegmentIntersectsLowerRectangleBorder;
}

/*
Method HalfSegmentIntersectsUpperRectangleBorder checks if given HalfSegment
intersects the upper border of given Rectangle<2> and returns
the intersection point between HalfSegment and upper rectangle border.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
            rPoint - reference to a Point containing intersection coordinates
return value: true, if rHalfSegment intersects upper border of rRectangle,
              otherwise false
exceptions: -

*/

bool HalfSegmentIntersectsUpperRectangleBorder(const HalfSegment& rHalfSegment,
                                               const Rectangle<2>& rRectangle,
                                               Point& rPoint)
{
  bool bHalfSegmentIntersectsUpperRectangleBorder = false;

  if(rRectangle.IsDefined())
  {
    Point halfSegmentLeftPoint = rHalfSegment.GetLeftPoint();
    Point halfSegmentRightPoint = rHalfSegment.GetRightPoint();

    if(halfSegmentLeftPoint.IsDefined() &&
       halfSegmentRightPoint.IsDefined())
    {
      CheckPoints(halfSegmentLeftPoint, halfSegmentRightPoint);

      double deltaX = halfSegmentRightPoint.GetX() -
                      halfSegmentLeftPoint.GetX();
      double deltaY = halfSegmentRightPoint.GetY() -
                      halfSegmentLeftPoint.GetY();
      double m = deltaY / deltaX;
      double n = halfSegmentLeftPoint.GetY() -
                 m * halfSegmentLeftPoint.GetX();

      double x = (rRectangle.MaxD(1) - n) / m;

      if(halfSegmentLeftPoint.GetX() <= x &&
         x <= halfSegmentRightPoint.GetX() &&
         x >= rRectangle.MinD(0) &&
         x < rRectangle.MaxD(0))
      {
        bHalfSegmentIntersectsUpperRectangleBorder = true;
        rPoint.Set(x, rRectangle.MaxD(1));
      }
    }
  }

  return bHalfSegmentIntersectsUpperRectangleBorder;
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

      unsigned char halfSegmentPointsInRectangle =
      HalfSegmentPointsInRectangle(rHalfSegment, rRectangle);

      if(halfSegmentPointsInRectangle > 0)
      {
        bHalfSegmentIntersectsRectangle = true;
      }

      else
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
          Point point(true, 0.0, 0.0);

          if(HalfSegmentIntersectsLeftRectangleBorder(rHalfSegment,
                                                      rRectangle,
                                                      point) ||
             HalfSegmentIntersectsRightRectangleBorder(rHalfSegment,
                                                       rRectangle,
                                                       point) ||
             HalfSegmentIntersectsLowerRectangleBorder(rHalfSegment,
                                                       rRectangle,
                                                       point) ||
             HalfSegmentIntersectsUpperRectangleBorder(rHalfSegment,
                                                       rRectangle,
                                                       point))
          {
            bHalfSegmentIntersectsRectangle = true;
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

        unsigned char halfSegmentPointsInRectangle =
        HalfSegmentPointsInRectangle(rHalfSegment, rRectangle);

        if(halfSegmentPointsInRectangle < 2)
        {
          if(IsHorizontalHalfSegment(rHalfSegment))
          {
            if(rLeftPoint.GetX() < rRectangle.MinD(0))
            {
              rLeftPoint.Set(rRectangle.MinD(0), rLeftPoint.GetY());
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

            if(rRightPoint.GetY() > rRectangle.MaxD(1))
            {
              rRightPoint.Set(rRightPoint.GetX(), rRectangle.MaxD(1));
            }
          }

          else
          {
            rLeftPoint.SetDefined(false);
            rRightPoint.SetDefined(false);

            if(halfSegmentPointsInRectangle == 1)
            {
              if(IsPointInRectangle(halfSegmentLeftPoint, rRectangle))
              {
                rLeftPoint.Set(halfSegmentLeftPoint);
              }

              if(IsPointInRectangle(halfSegmentRightPoint, rRectangle))
              {
                rLeftPoint.Set(halfSegmentRightPoint);
              }
            }

            Point point(true, 0.0, 0.0);

            if(rRightPoint.IsDefined() == false &&
               HalfSegmentIntersectsLeftRectangleBorder(rHalfSegment,
                                                        rRectangle,
                                                        point))
            {
              if(rLeftPoint.IsDefined() == false)
              {
                rLeftPoint.Set(point);
              }

              else
              {
                rRightPoint.Set(point);
              }
            }

            if(rRightPoint.IsDefined() == false &&
               HalfSegmentIntersectsRightRectangleBorder(rHalfSegment,
                                                         rRectangle,
                                                         point))
            {
              if(rLeftPoint.IsDefined() == false)
              {
                rLeftPoint.Set(point);
              }

              else
              {
                rRightPoint.Set(point);
              }
            }

            if(rRightPoint.IsDefined() == false &&
               HalfSegmentIntersectsLowerRectangleBorder(rHalfSegment,
                                                         rRectangle,
                                                         point))
            {
              if(rLeftPoint.IsDefined() == false)
              {
                rLeftPoint.Set(point);
              }

              else
              {
                rRightPoint.Set(point);
              }
            }

            if(rRightPoint.IsDefined() == false &&
               HalfSegmentIntersectsUpperRectangleBorder(rHalfSegment,
                                                         rRectangle,
                                                         point))
            {
              if(rLeftPoint.IsDefined() == false)
              {
                assert(false);
              }

              else
              {
                rRightPoint.Set(point);
              }
            }

            CheckPoints(rLeftPoint, rRightPoint);
          }
        }

        bRetVal = true;
      }
    }
  }

  return bRetVal;
}

}
