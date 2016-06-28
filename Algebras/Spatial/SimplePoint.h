
/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

*/

#ifndef SIMPLEPOINT_H
#define SIMPLEPOINT_H

#include "Point.h"


class SimplePoint;

std::ostream& operator<<(std::ostream& o,const SimplePoint& p);

class SimplePoint{
  public:
     explicit SimplePoint(const Point* p){
       this->x = p->GetX();
       this->y = p->GetY();
     }

     explicit SimplePoint(Point& p){
       this->x = p.GetX();
       this->y = p.GetY();
     }

     SimplePoint(){ }

     SimplePoint(double x, double y){
       this->x = x;
       this->y = y;
     }

     SimplePoint(const SimplePoint& p){
        this->x = p.x;
        this->y = p.y;
     }

     SimplePoint& operator=(const SimplePoint& p){
       this->x = p.x;
       this->y = p.y;
       return *this;
     }

     int compare(const SimplePoint& p)const{
         if(AlmostEqual(x,p.x)){
            if(AlmostEqual(y,p.y)){
                return 0;
            }
            return y<p.y?-1:1;
         }
         return x<p.x?-1:1;
     }


     ~SimplePoint(){}

     SimplePoint relTo(const SimplePoint& p) const{
        return SimplePoint(this->x - p.x, this->y-p.y);
     }

     void makeRelTo(const SimplePoint& p){
        this->x -= p.x;
        this->y -= p.y;
     }

     SimplePoint moved(const double x0, const double y0)const{
        return SimplePoint(x+x0, y+y0);
     }

     SimplePoint reversed()const{
        return SimplePoint(-x,-y);
     }

     bool isLower(const SimplePoint& p)const{
        if(!AlmostEqual(y,p.y)){
           return y < p.y;
        }
        if(AlmostEqual(x,p.x)){ // equal points
           return false;
        }
        return x < p.x;
     }

     double mdist()const{ // manhatten distance to (0,0)
       return std::fabs(x) + std::fabs(y);
     }

     double mdist(const SimplePoint p)const{
        return std::fabs(x-p.x) + std::abs(y-p.y);
     }

     bool isFurther(const SimplePoint& p)const{
        return mdist() > p.mdist();
     }

     bool isBetween(const SimplePoint& p0, const SimplePoint p1) const{
        return p0.mdist(p1) >= mdist(p0)+mdist(p1);
     }

     double cross(const SimplePoint& p)const{
        return x*p.y - p.x*y;
     }

     bool isLess(const SimplePoint& p) const{
        double f = cross(p);
        bool res;
        if(AlmostEqual(f,0.0)){
          res = isFurther(p);
        } else {
          res = f>0;
        }
        return  res;
     }

     bool operator<(const SimplePoint& p) const{
         return isLess(p);
     }

     bool operator==(const SimplePoint& p) const{
         return AlmostEqual(x,p.x)&& AlmostEqual(y,p.y);
     }
     bool operator!=(const SimplePoint& p) const{
         return !AlmostEqual(x,p.x) ||  !AlmostEqual(y,p.y);
     }

    bool operator>(const SimplePoint& p) const{
         return !(AlmostEqual(x,p.x) && AlmostEqual(y,p.y)) && !isLess(p);
     }

     double area2(const SimplePoint& p0, const SimplePoint& p1) const{
        return p0.relTo(*this).cross(p1.relTo(*this));
     }

     bool isConvex(const SimplePoint& p0, const SimplePoint& p1) const {
        double f = area2(p0,p1);
        if(AlmostEqual(f,0.0)){
           bool between = isBetween(p0,p1);
           return !between;
        }
        return f<0;
     }

     Point getPoint()const {
        return Point(true,x,y);
     }

     double getX()const{ return x;}
     double getY()const{ return y;}

     void  setX( const double _x){ x=_x;}
     void  setY( const double _y){ y=_y;}

     size_t hash() const{
        return (size_t)( x*y + y);
      }

  private:
     double x;
     double y;

}; // end of class SimplePoint

#endif



