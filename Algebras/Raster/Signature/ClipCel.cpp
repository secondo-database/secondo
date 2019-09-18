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
//#include <vcl.h>
#ifndef CLIPCELCPP
#define CLIPCELCPP

#include "ClipCel.h"
#include "TurningPoint.cpp"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif


//#include "Secondo.cpp"
//#include "../../Spatial/SpatialAlgebra.h"

//#define DEBUGMESSAGES
//---------------------------------------------------------------------------
bool ClipCel::clippedSegmentOnEdge(Segment &s)
{
//This function returns true if the segment lies on one of the window's edge.
// The clipped half segments that lie on the edges must be rejected according to
// the kind of clipping (returning the portion of the region that is inside the
// region or the portion that is outside).

  TurningPoint tp1,tp2;
  //Returns true if the clipped hs was treated as a segment on edge
  bool reject = false,
       result = false;
  if ( s.p1.y == s.p2.y ) //horizontal edge
  {
    if ( s.p1.y == this->max.y ) //top edge
    {
  // If the half segment lies on the upper edge and the insideAbove attribute's
  // value is true then the region's area is outside the window, and the half
  // segment mustn't be included in the clipped region (Reject). However, its
  // end points maybe will have to be connected to the vertices of the window.
  // It happens only when the vertice of the window is inside the region and the
  // end point is the first point on the window's edge (for the upper-left
  // vertice) or the last point on the window's vertice (for the upper right
  // edge).
      if ( s.insideAbove )
        reject = true;
      tp1.set(s.getLP(),false,reject,s); //--> right
      tp2.set(s.getRP(),true,reject,s);  //<-- left
      vTurningPointsTOP.push_back(tp1);
      vTurningPointsTOP.push_back(tp2);
      result = true;
    }
    else //bottom edge
      if ( s.p1.y == this->min.y )
      {
        if ( s.insideAbove )
           reject = true;
        tp1.set(s.getLP(),false,reject,s); //--> right
        tp2.set(s.getRP(),true,reject,s);  //<-- left
        vTurningPointsBOTTOM.push_back(tp1);
        vTurningPointsBOTTOM.push_back(tp2);
        result = true;
      }
  }
  else //Vertical edges
    if ( s.p1.x == s.p2.x )
    {
      if ( s.p1.x == this->min.x ) //Left edge
      {
        if ( s.insideAbove )
          reject = true;
        tp1.set(s.getLP(),false,reject,s); //^up
        tp2.set(s.getRP(),true,reject,s);  //v down
        vTurningPointsLEFT.push_back(tp1);
        vTurningPointsLEFT.push_back(tp2);
        result = true;
      }
      else
        if ( s.p1.x == this->max.x ) //Right edge
        {
          if ( !s.insideAbove )
            reject = true;
          tp1.set(s.getLP(),false,reject,s); //^up
          tp2.set(s.getRP(),true,reject,s);  //v down
          vTurningPointsRIGHT.push_back(tp1);
          vTurningPointsRIGHT.push_back(tp2);
          result = true;
        }
    }
  return result;
}
//---------------------------------------------------------------------------
void ClipCel::addTurningPoint(const RealCoordinate &p,const Segment &s)
{
  RealCoordinate v;
  //If the left and right edges are been tested then it is not needed to check
  //the angle between the half segment and the edge. If the attribute inside
  //above is true, then the direction is up (false), otherwise it is
  //down (true).
  if (p.x == this->min.x)
    vTurningPointsLEFT.push_back(TurningPoint(p,!s.insideAbove,false,s));
  else
    if (p.x == this->max.x)
      vTurningPointsRIGHT.push_back(TurningPoint(p,!s.insideAbove,false,s));
  if (p.y == this->min.y)
  {
    v.x = this->min.x;
    v.y = this->min.y;
    //In this case we don't know which point is outside the window,
    //so it is need to test both half segment's poinst. Moreover,
    //in order to use the same comparison that is used for
    //Top edge, it is need to choose the half segment point that
    //is over the bottom edge.
    if (s.p1.y > this->min.y)
      vTurningPointsBOTTOM.push_back( TurningPoint(p,
              TurningPoint::getDirectionTurningPoint(p,s.p1,s.insideAbove,v),
              false,s));
    else
      if (s.p2.y > this->min.y)
        vTurningPointsBOTTOM.push_back( TurningPoint(p,
              TurningPoint::getDirectionTurningPoint(p,s.p2,s.insideAbove,v),
              false,s));

        //If none of them is above the top edge, then the y coordinate of one
        //point is equal to p.y and the other is below.
        //In order to discover the correct direction
        //we must change the parameter to send to GetTurningPoint function
        //so that p is the above point and the other point different from p
        //is the down one.
      else
        if (p.y==s.p1.y) //==> p2 is bellow p
          vTurningPointsBOTTOM.push_back( TurningPoint(p,
            TurningPoint::getDirectionTurningPoint(s.p2,p,s.insideAbove,v),
            false,s));
        else
          vTurningPointsBOTTOM.push_back( TurningPoint(p,
            TurningPoint::getDirectionTurningPoint(s.p1,p,s.insideAbove,v),
            false,s));
  }
  else
    if (p.y == this->max.y)
    {
      v.x = this->min.x;
      v.y = this->max.y;
      //In this case we don't know which point is outside the window,
      //so it is need to test
      if (s.p1.y > this->max.y)
        vTurningPointsTOP.push_back( TurningPoint(p,
              TurningPoint::getDirectionTurningPoint(p,s.p1,s.insideAbove,v),
              false,s));
      else
        if (s.p2.y > this->max.y)
          vTurningPointsTOP.push_back( TurningPoint(p,
              TurningPoint::getDirectionTurningPoint(p,s.p2,s.insideAbove,v),
              false,s));
        //If none of them is above the top edge, then the y coordinate of one
        //point is equal to p.y and the other is below.
        //In order to discover the correct direction
        //we must change the parameter to send to GetTurningPoint function
        //so that p is the above point and the other point different from p
        //is the down one.
        else
          if (p.y==s.p1.y) //==> p2 is bellow p
            vTurningPointsTOP.push_back( TurningPoint(p,
              TurningPoint::getDirectionTurningPoint(s.p2,p,s.insideAbove,v),
              false,s));
          else
            vTurningPointsTOP.push_back( TurningPoint(p,
              TurningPoint::getDirectionTurningPoint(s.p1,p,s.insideAbove,v),
              false,s));
      }
}
//---------------------------------------------------------------------------
double ClipCel::trapezoidArea(Segment s)
{
  //If the segment has 'insideAbove' positive, then the area of the
  //trapeze must be considered negative
  int x = -1;
  if (!s.insideAbove)
    x=1;
  //bug - fabs(any value) = 0
  //  double area = x * 0.5 * fabs( (s.p1.x-s.p2.x) * ( s.p1.y - this->min.y +
  //                                       s.p2.y - this->min.y )) ;
  double area = (s.p1.x-s.p2.x) * (s.p1.y - this->min.y + s.p2.y - this->min.y);
  if ( area < 0.0 )
    area *= -1;
  area = x * 0.5 * area ;
  return area;
}
//---------------------------------------------------------------------------
//Computes the area of the rectangle between x1 and x2 (points over the
//superior edge) and ymin and ymax
double ClipCel::rectangleArea(double x1,double x2)
{
  //If the segment has 'insideAbove' positive, then the area of the
  //rectangle must be considered negative
  //  double area = fabs((x1-x2) * (this->max.y-this->min.y));
  //bug - fabs(any value) = 0
  double area = (x1-x2) * (this->max.y-this->min.y);
  if ( area < 0.0 )
    area *= -1;
  return area;
}
//---------------------------------------------------------------------------
void ClipCel::handleSegment(RealCoordinate celMin,RealCoordinate celMax,
                Segment s,  Segment &sInside,
                bool &inside, bool &isIntersectionPoint,
                RealCoordinate &intersectionPoint, SignatureType signatureType)
{
  if (!isDefined)
  {
    define(celMin,celMax);
  }
  WindowClippingIn(s,sInside,inside, isIntersectionPoint, intersectionPoint);
  if (signatureType == SIGNAT_4CRS)
  {
    if (isIntersectionPoint)
        addTurningPoint(intersectionPoint,s);
    else
      if ( inside )
      {
         bool hsOnEdge = clippedSegmentOnEdge(sInside);
         if (!hsOnEdge)
         {
          //If the point lies on one edge it must be added to the
          //corresponding vector.
          addTurningPoint(sInside.p1,s);
          addTurningPoint(sInside.p2,s);
          area +=trapezoidArea(sInside);
        }
      }
  }
}
//---------------------------------------------------------------------------
void ClipCel::WindowClippingIn(Segment &s,Segment &sInside,bool &inside,
              bool &isIntersectionPoint, RealCoordinate &intersectionPoint)
{
  double x0=s.p1.x, y0=s.p1.y,
         x1=s.p2.x, y1=s.p2.y;
  //Secondo::CohenSutherlandLineClipping(*this, x0, y0, x1, y1, inside);
  double minPoint[2], maxPoint[2];
  minPoint[0] = this->min.x;
  minPoint[1] = this->min.y;
  maxPoint[0] = this->max.x;
  maxPoint[1] = this->max.y;

  Rectangle<2> *box = new Rectangle<2>(true, minPoint, maxPoint);
  HalfSegment hs;
  hs.CohenSutherlandLineClipping(*box, x0, y0, x1, y1, inside);
  isIntersectionPoint=false;
  if (inside)
  {
    if ( (x0==x1) && (y0==y1) )
    {
      isIntersectionPoint = true;
      intersectionPoint = RealCoordinate(x0,y0);
    }
    else
    {
    sInside.set(RealCoordinate(x0,y0),RealCoordinate(x1,y1),s.insideAbove,s.id);
      sInside.insideAbove = s.insideAbove;
    }
  }
}
//---------------------------------------------------------------------------
Signature4CRS::Weight ClipCel::evaluateType(long double blockArea)
{
  evaluateNewSegmentsOnTopEdge();
  double areaTest=fabs(this->area / blockArea);
  if (areaTest==0)
    return Signature4CRS::Empty;
  else
    if (areaTest > 0 && areaTest<=0.50)
      return Signature4CRS::Weak;
    else
      if (areaTest > 0.5 && areaTest<1)
        return Signature4CRS::Strong;
      else
        return Signature4CRS::Full;
}


//---------------------------------------------------------------------------
void ClipCel::evaluateNewSegmentsOnTopEdge()
{
  bool leftVertex=false,rightVertex=false;
  evaluateNewSegmentsTopEdge(leftVertex, rightVertex);

  TurningPoint tpLEFTbegin, tpRIGHTbegin,
               tpAux;

  if (vTurningPointsTOP.size()==0)
  {
     if (vTurningPointsLEFT.size()!=0)
     {
       sort(vTurningPointsLEFT.begin(),vTurningPointsLEFT.end());
       tpAux = vTurningPointsLEFT[0];
       if (!tpAux.rejected && tpAux.direction)
         tpLEFTbegin=tpAux;
       tpAux = vTurningPointsLEFT[vTurningPointsLEFT.size()-1];
       if (!tpAux.rejected && !tpAux.direction)
         leftVertex = true;
     }
     if (vTurningPointsRIGHT.size()!=0)
     {
       sort(vTurningPointsRIGHT.begin(),vTurningPointsRIGHT.end());
       tpAux = vTurningPointsRIGHT[0];
       if (!tpAux.rejected && tpAux.direction)
         tpRIGHTbegin=tpAux;
       tpAux = vTurningPointsRIGHT[vTurningPointsRIGHT.size()-1];
       if (!tpAux.rejected && !tpAux.direction)
         rightVertex=true;
     }
     if (vTurningPointsBOTTOM.size()!=0)
     {
       sort(vTurningPointsBOTTOM.begin(),vTurningPointsBOTTOM.end());
       tpAux = vTurningPointsBOTTOM[0];
       if (!tpAux.rejected && tpAux.direction && vTurningPointsLEFT.size()==0)
           leftVertex = true;
       tpAux = vTurningPointsBOTTOM[vTurningPointsBOTTOM.size()-1];
       if (!tpAux.rejected && !tpAux.direction && vTurningPointsRIGHT.size()==0)
           rightVertex=true;
     }
     if (!leftVertex && vTurningPointsLEFT.size()==0 &&
         vTurningPointsBOTTOM.size()==0 && tpRIGHTbegin.defined &&
         tpRIGHTbegin.direction)
           leftVertex = true;
     if (!rightVertex && vTurningPointsRIGHT.size()==0 &&
         vTurningPointsBOTTOM.size()==0 && tpLEFTbegin.defined &&
         tpLEFTbegin.direction)
           rightVertex = true;
     if (leftVertex && rightVertex)
        area+=rectangleArea(this->min.x,this->max.x);
  }
  return;
}

//---------------------------------------------------------------------------
void ClipCel::evaluateVerticesBottomEdge(bool &considerLeftVertex,
                                bool &considerRightVertex)
{
  int begin, end;
  TurningPoint dp,dpAux;

  if (vTurningPointsBOTTOM.size()==0) return;

  sort(vTurningPointsBOTTOM.begin(),vTurningPointsBOTTOM.end());

  begin = 0;
  end = vTurningPointsBOTTOM.size()-1;

  dp = vTurningPointsBOTTOM[begin];
  if ( dp.direction)//dp points to left or down
  {

    if (!dp.rejected)
    {
      //If dp is a rejected point then it must not be considered
      //as point to be connected to the window edge
      considerLeftVertex = true;
    }
    begin++;
    //The variable ~begin~ must be incremented until exists points with
    //the same coordinates and directions as dp
    while (begin<=end)
    {
      dpAux = vTurningPointsBOTTOM[begin];
      if (!( (dpAux.x == dp.x) && (dpAux.y == dp.y) &&
           (dpAux.direction==dp.direction) ) )
        break;
      begin++;
    }
  }

  dp = vTurningPointsBOTTOM[end];
  if ( !dp.direction && !dp.rejected) //dp points to right or up
    considerRightVertex = true;
}
//---------------------------------------------------------------------------
void ClipCel::evaluateNewSegmentsTopEdge(bool &consideredLeftVertex,
                                         bool &consideredRightVertex)
//The inside attribute indicates if the points on edge will originate
//segments that are inside the window (its values is true), or outside
//the window (its value is false)
{
  int begin, end, i;
  TurningPoint dp,dpAux;
  TurningPoint ep1,ep2;

  if (vTurningPointsTOP.size()==0) return;

  sort(vTurningPointsTOP.begin(),vTurningPointsTOP.end());

  begin = 0;
  end = vTurningPointsTOP.size()-1;

  dp = vTurningPointsTOP[begin];
  if ( dp.direction)//dp points to left or down
  {
    ep1=dp;
    if (!dp.rejected)
    {
      //If dp is a rejected point then it must not be considered
      //as point to be connected to the window edge
      consideredLeftVertex = true;
      this->area+=rectangleArea(this->min.x,dp.x);
    }
    begin++;
    //The variable ~begin~ must be incremented until exists points
    //with the same coordinates and directions as dp
    while (begin<=end)
    {
      dpAux = vTurningPointsTOP[begin];
      if (!( (dpAux.x == dp.x) && (dpAux.y == dp.y) &&
             (dpAux.direction==dp.direction) ) )
        break;
      begin++;
    }

  }

  dp = vTurningPointsTOP[end];
  if ( !dp.direction) //dp points to right or up
  {
    bool rejectEndPoint=dp.rejected;
    end--;

    while ( end >= begin )
    {
      dpAux = vTurningPointsTOP[end];
      if ( !( (dpAux.x == dp.x ) && ( dpAux.y == dp.y ) &&
              (dpAux.direction==dp.direction) ) )
         break;

      //when a rejected point is found the rejectEndPoint
      //does not change anymore.

      end--;
    }

    if (!rejectEndPoint)
    {
      consideredRightVertex = true;
      ep2=dp;
      this->area+=rectangleArea(dp.x,this->max.x);
    }
  }
  i = begin;
  while (i < end)
  {
    if ( GetAcceptedPoint(vTurningPointsTOP,i,end,ep1) )
    {
      i++;
      if (GetAcceptedPoint(vTurningPointsTOP,i,end, ep2) )
        i++;
      else
        break;
    }
    else
      break;
    if ( ! ( (ep1.x == ep2.x) && (ep1.y == ep2.y) ) )
    //discard degenerated edges
      this->area+=rectangleArea(ep1.x,ep2.x);
  }
}
//---------------------------------------------------------------------------
bool ClipCel::GetAcceptedPoint(std::vector <TurningPoint>vTurningPoint,int &i,
	                   const int &end, TurningPoint &ep)
{
  //id is the indice of the current point in the scan
  //ep is the correct edge point that will be returned.
  ep = vTurningPoint[i];
  //discard all rejected points
  while (ep.rejected && i<=end)
  {
    i++;
    if (i>end)
      return false;
    TurningPoint epAux = vTurningPoint[i];
    //Discard all the points that was accepted but has a corresponding
    //rejection point.
    //In other words, point that has the same coordinates and direction
    //on the edge.
    if (!epAux.rejected && (epAux.direction==ep.direction) &&
         (epAux.x == ep.x) && (epAux.y == ep.y) )
    {
      while ( (i<=end) && (epAux.direction==ep.direction) &&
         (epAux.x == ep.x) && (epAux.y == ep.y) )
      {
        i++;
        if (i>end)
          return false;
        epAux = vTurningPoint[i];
      }
    }
    ep = epAux;
  }
  return true;
}

#endif
