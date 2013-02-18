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

#ifndef RASTER2_COMPOSE_H
#define RASTER2_COMPOSE_H

#include <NList.h>

#include "../sbool.h"
#include "../sreal.h"
#include "../sint.h"
#include "../sstring.h"
#include "TemporalAlgebra.h"
#include "RTreeAlgebra.h"

namespace raster2 {
    extern ValueMapping composeFuns[];
    ListExpr composeTypeMap(ListExpr args);
    int composeSelectFun(ListExpr args);

double round(double r);

bool compareVals (double i, double j);

double calculateSlope(double startX,
                      double startY,
                      double endX,
                      double endY);

vector<Point> calculatePoints(double startX,
                               double startY,
                               double endX,
                               double endY,
                               double originX,
                               double originY,
                               double length);

  template <typename T, typename Helper>
  int composeFun
      (Word* args, Word& result, int message, Word& local, Supplier s)
  {
      // storage for the result
      result = qp->ResultStorage(s);
      
      // the moving point
      MPoint* movingPoint = static_cast<MPoint*>(args[0].addr);
      
      // the sT object
      typename Helper::implementation_type* pImplementationType =
          static_cast<typename Helper::implementation_type*>(args[1].addr);

      // The result of the compose
      typename Helper::moving_type* pResult =
            static_cast<typename Helper::moving_type*>(result.addr);

      if (!movingPoint->IsDefined()) {
        pResult->SetDefined(false);
        return 0;
      }

      pResult->StartBulkLoad();

      // get the number of components
      int num = movingPoint->GetNoComponents();

      for (int i = 0; i < num; i++)
      {
          UPoint u = UPoint(0);
          movingPoint->Get(i, u);
          
          // get the coordinates
          double xStart = u.p0.GetX();
          double yStart = u.p0.GetY();
          double xEnd = u.p1.GetX();
          double yEnd = u.p1.GetY();
          
          double originX = pImplementationType->getGrid().getOriginX();
          double originY = pImplementationType->getGrid().getOriginY();
          double gridLen = pImplementationType->getGrid().getLength();
          
          // get the length of the UPoint
          double len = u.p0.Distance(u.p1);
          
          // get the intersection points of movement with the grid
          vector<Point> points = calculatePoints(xStart,yStart,xEnd,yEnd,
                                                   originX, originY, gridLen);

          vector<Point>::iterator it = points.begin();
          
          if (it != points.end())
          {
              it++;  
          }
          
          // calculate for each point the date, where the intersection occurs
          while (it != points.end())
          {
              Point startPoint = *(it - 1);
              Point currentPoint = *it;
              
              if (currentPoint != startPoint)
              {
                  double lenStart = u.p0.Distance(startPoint);
                  double lenEnd = u.p0.Distance(currentPoint);
                  
                  Instant startPosition = u.getTimeInterval().start;
                  double startValue = startPosition.ToDouble();
                  Instant endPosition = u.getTimeInterval().end;
                  double endValue = startPosition.ToDouble();

                  double lenOfTime = endPosition.ToDouble()
                                     - startPosition.ToDouble();
                 
                  // the start in time is the start of the movement
                  // plus the time to reach the first intersection point
                  startValue = startPosition.ToDouble()
                               + lenOfTime * lenStart / len;
                  // the end in time is the start of the movement
                  // plus the time to reach the last intersection point
                  endValue = startPosition.ToDouble()
                             + lenOfTime * lenEnd / len;

                  Instant startOfInterval = startPosition;
                  startOfInterval.ReadFrom(startValue);

                  Instant endOfInterval = startPosition;
                  endOfInterval.ReadFrom(endValue);

                  // the last intersection point is _not_ contained in the cell
                  // so we take the values from the first intersection point
                  T value = pImplementationType->atlocation(startPoint.GetX(),
                                                            startPoint.GetY());

                  // TODO: the check with endValue and startValue is a hack!
                  if (!pImplementationType->isUndefined(value)
				      && endValue > startValue)
                  {
                      typename Helper::wrapper_type v(true,value);
                      Interval<Instant> iv(startOfInterval,
                                           endOfInterval,
                                           true,
                                           false);
                      // use merge add, to avoid the same value
                      // over and over again
                      pResult->MergeAdd(typename Helper::unit_type(iv,v,v));
                  }
              }
              it++;
          }
      }
       
      pResult->EndBulkLoad();

    return 0;
  }

    struct composeInfo : OperatorInfo 
    {
      composeInfo()
      { 
        name      = "compose";
        signature = MPoint::BasicType()
            + " compose "
            + sbool::BasicType() + "-> " 
            + MBool::BasicType();
        appendSignature(MPoint::BasicType() + " compose "
            + sreal::BasicType() + "-> " 
            + MReal::BasicType());
        appendSignature(MPoint::BasicType() + " compose "
            + sint::BasicType() + "-> " 
            + MInt::BasicType());
        appendSignature(MPoint::BasicType() + " compose "
            + sstring::BasicType() + "-> " 
            + MString::BasicType());

        syntax    = "compose(_)";
        meaning   = "merges mpoint and sT into mT";
      }          
    };
}

#endif /* #define RASTER2_COMPOSE_H */

