/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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
----

May, 2007 Leonardo Azevedo, Rafael Brand

*/
//---------------------------------------------------------------------------
#ifndef ClipCelH
#define ClipCelH
#include <vector>
#include <cmath>
#include "TurningPoint.h"
#include "Segment.h"
//#include "Secondo.h"
#include "Signature4CRS.h"
//---------------------------------------------------------------------------
class ClipCel : Box
{
  public:
    std::vector<TurningPoint> vTurningPointsTOP, vTurningPointsBOTTOM,
                         vTurningPointsLEFT, vTurningPointsRIGHT;

    ClipCel(RealCoordinate MIN, RealCoordinate MAX)
    {
      define(MIN,MAX);
    };

    void define(RealCoordinate MIN, RealCoordinate MAX)
    {
      this->min.x = (long int)MIN.x;
      this->min.y = (long int)MIN.y;
      this->max.x = (long int)MAX.x;
      this->max.y = (long int)MAX.y;
      area = 0;
      isDefined=true;
    };

    ClipCel(){isDefined=false;};

    ~ClipCel()
    {
    };

    Signature4CRS::Weight evaluateType(long double blockArea);

    bool clippedSegmentOnEdge(Segment &s);
    void addTurningPoint(const RealCoordinate &p,const Segment &s);
    void handleSegment(RealCoordinate celMin,RealCoordinate celMax,
                Segment s,Segment &sInside,
                bool &inside, bool &isIntersectionPoint,
                RealCoordinate &intersectionPoint, SignatureType signatureType);
  private:
    double area;
    bool isDefined;

    double trapezoidArea(Segment s);
    double rectangleArea(double x1,double x2);
    void WindowClippingIn(Segment &s,Segment &sInside,bool &inside,
              bool &isIntersectionPoint, RealCoordinate &intersectionPoint);
    void evaluateNewSegmentsTopEdge(bool &consideredLeftVertex,
                                         bool &consideredRightVertex);
    bool GetAcceptedPoint(std::vector <TurningPoint>vTurningPoint,int &i,
                      const int &end, TurningPoint &ep);
    void evaluateNewSegmentsOnTopEdge();
    void evaluateVerticesBottomEdge(bool &considerLeftVertex,
                                bool &considerRightVertex);
};
//---------------------------------------------------------------------------
#endif
