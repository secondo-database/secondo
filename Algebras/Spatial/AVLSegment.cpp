

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

1 Implementation of AVLSegment

*/


#include "SpatialAlgebra.h"
#include "AVLSegment.h"
#include "RobustSetOps.h"

#include <functional>
#include <typeinfo>

using namespace std;


/*
~Shift~ Operator for ~ownertype~

*/

ostream& avlseg::operator<<(ostream& o, const avlseg::ownertype& owner){
   switch(owner){
      case avlseg::none   : o << "none" ; break;
      case avlseg::first  : o << "first"; break;
      case avlseg::second : o << "second"; break;
      case avlseg::both   : o << "both"; break;
      default     : assert(false);
   }
   return o;
}

ostream& operator<<(ostream& o, const avlseg::ExtendedHalfSegment& hs) {
  return hs.Print(o);
}



bool PointOnSegment(double x, double y,
                    double x1, double y1,
                    double x2, double y2){

   double len  = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
   double len1 = sqrt((x1-x)*(x1-x) + (y1-y)*(y1-y));
   double len2 = sqrt((x2-x)*(x2-x) + (y2-y)*(y2-y));
   return AlmostEqual(len, len1+len2);
}



/*
3 Implementation of ~AVLSegment~

3.0 static variables

*/

  bool avlseg::AVLSegment::x_error = false;
  double avlseg::AVLSegment::error_value = 0.0;




/*
3.1 Constructors

~Standard Constructor~

*/

  avlseg::AVLSegment::AVLSegment():
   con_below(0), con_above(0),x1(0),x2(0),y1(0),y2(0),
   insideAbove_first(false),
   insideAbove_second(false),owner(none),
   originX1(0),originX2(0),originY1(0),originY2(0)
   { }


/*
~Constructor~

This constructor creates a new segment from the given HalfSegment.
As owner only __first__ and __second__ are the allowed values.

*/

  avlseg::AVLSegment::AVLSegment(const avlseg::ExtendedHalfSegment& hs,
                                 ownertype owner){


     assert(hs.isInitialized());

     x1 = hs.GetLeftPoint().GetX();
     y1 = hs.GetLeftPoint().GetY();
     x2 = hs.GetRightPoint().GetX();
     y2 = hs.GetRightPoint().GetY();
     originX1 = hs.getOriginX1();
     originX2 = hs.getOriginX2();
     originY1 = hs.getOriginY1();
     originY2 = hs.getOriginY2();
     if( (AlmostEqual(x1,x2) && (y2<y1) ) || (x2<x1) ){// swap the entries
        double tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
     }
     this->owner = owner;
     switch(owner){
        case first: {
             insideAbove_first = hs.GetAttr().insideAbove;
             insideAbove_second = false;
             break;
        } case second: {
             insideAbove_second = hs.GetAttr().insideAbove;
             insideAbove_first = false;
             break;
        } default: {
             assert(false);
        }
     }
     con_below = 0;
     con_above = 0;
     originX1 = hs.getOriginX1();
     originX2 = hs.getOriginX2();
     originY1 = hs.getOriginY1();
     originY2 = hs.getOriginY2();


  //   assert(CheckPoints());
  }

/*
~Constructor~

Create a Segment only consisting of a single point.

*/

  avlseg::AVLSegment::AVLSegment(const Point& p, ownertype owner){
      x1 = p.GetX();
      x2 = x1;
      y1 = p.GetY();
      y2 = y1;
      this->owner = owner;
      insideAbove_first = false;
      insideAbove_second = false;
      con_below = 0;
      con_above = 0;
      originX1 = x1;
      originX2 = x2;
      originY1 = y1;
      originY2 = y2;
    //  assert(CheckPoints());
  }


/*
~Copy Constructor~

*/
   avlseg::AVLSegment::AVLSegment(const AVLSegment& src):
    con_below(src.con_below),
    con_above(src.con_above),
    x1(src.x1), x2(src.x2), y1(src.y1), y2(src.y2),
    insideAbove_first (src.insideAbove_first),
    insideAbove_second(src.insideAbove_second),
    owner(src.owner),
    originX1(src.originX1), originX2(src.originX2),
    originY1(src.originY1), originY2(src.originY2)
   {

     // assert(CheckPoints());
   }



/*
3.3 Operators

*/

  avlseg::AVLSegment& avlseg::AVLSegment::operator=(
                                         const avlseg::AVLSegment& src){
     x1 = src.x1;
     x2 = src.x2;
     y1 = src.y1;
     y2 = src.y2;
     owner = src.owner;
     insideAbove_first = src.insideAbove_first;
     insideAbove_second = src.insideAbove_second;
     con_below = src.con_below;
     con_above = src.con_above;
     originX1 = src.originX1;
     originX2 = src.originX2;
     originY1 = src.originY1;
     originY2 = src.originY2;
     // assert(CheckPoints());
     return *this;
  }

  bool avlseg::AVLSegment::operator==(const avlseg::AVLSegment& s) const{
    return compareTo(s)==0;
  }

  bool avlseg::AVLSegment::operator<(const avlseg::AVLSegment& s) const{
     int res = compareTo(s);
     return res<0;
  }

  bool avlseg::AVLSegment::operator>(const avlseg::AVLSegment& s) const{
     return compareTo(s)>0;
  }

/*
3.3 Further Needful Functions

~Print~

This function writes this segment to __out__.

*/
  void avlseg::AVLSegment::Print(ostream& out)const{
    out << "Segment("<<x1<<", " << y1 << ") -> (" << x2 << ", " << y2 <<") "
        << owner << " [ " << insideAbove_first << ", "
        << insideAbove_second << "] con("
        << con_below << ", " << con_above << ")"
        << "orig = ( (" << originX1 << ", " << originY1 << ") -> ("
        << originX2 << ", " << originY2 << "))";

  }

/*
~CheckPoints~

This function checks whether the points (x1,y1) and (x2,y2) are
located on the segment defined by (orinigX1, originY1) and
(originY2, originY2). Furthermore, the distance from (x1,y1) to
(originX1,originY1) must be smaller than the distance to
(originX2, originY2).

*/
  bool avlseg::AVLSegment::CheckPoints() const{

    if(!PointOnSegment(x1,y1,originX1,originY1, originX2, originY2)){
      HalfSegment hs(true, Point(originX1,originY1),Point(originX2,originY2));
      double dist = hs.Distance(Point(x1,y1));
      if(!AlmostEqual(dist,0.0)){ // rounding errors?
        cerr.precision(16);
        cerr << "Left Point not on OriginSegment" << endl;
        cerr << "this = " << (*this) << endl;
        cerr << "distance = " << dist << endl;
        return false;
      }
    }
    if(!PointOnSegment(x2,y2,originX1,originY1, originX2, originY2)){
      HalfSegment hs(true, Point(originX1,originY1),Point(originX2,originY2));
      double dist = hs.Distance(Point(x2,y2));
      if(!AlmostEqual(dist,0.0)){ // rounding errors?
        cerr.precision(16);
        cerr << "right Point not on OriginSegment" << endl;
        cerr << "this = " << (*this) << endl;
        cerr << "distance = " << dist << endl;
        return false;
      }
    }
    double d1 = (x1-originX1)*(x1-originX1) + (y1-originY1)*(y1-originY1);
    double d2 = (x2-originX1)*(x2-originX1) + (y2-originY1)*(y2-originY1);
    if(d2<d1){
      cerr << "distance from left origin point to left "
              "point greater than to right point" << endl;
      return  false;
    }
    d1 = (x1-originX2)*(x1-originX2) + (y1-originY2)*(y1-originY2);
    d2 = (x2-originX2)*(x2-originX2) + (y2-originY2)*(y2-originY2);
    if(d1<d2){
      cerr << "distance from right origin point to right"
              " point greater than to left point" << endl;
      return  false;
    }


    return true;
  }



/*
3.5 Geometric Functions

~crosses~

Checks whether this segment and __s__ have an intersection point of their
interiors.

*/
 bool avlseg::AVLSegment::crosses(const avlseg::AVLSegment& s) const{
   double x,y;
   bool res = crosses(s,x,y);
   return res;
 }


/*
~innerBoxContains~

checks whether (x,y) is not an endpoint of this segment and also
that the point is located within the bounding box of this segment

*/
  bool avlseg::AVLSegment::innerBoxContains(const double x,
                                            const double y) const{
    if(AlmostEqual(x1,x) && AlmostEqual(y1,y)){
       return false; // endpoint 1
    }
    if(AlmostEqual(x2,x) && AlmostEqual(y2,y)){
       return false; // endpoint 2
    }

    bool xcontains = ( AlmostEqual(x1,x) || AlmostEqual(x2,x) ||
                       ((x1<=x) && (x<=x2)));
    bool ycontains = ( AlmostEqual(y1,y) || AlmostEqual(y2,y) ||
                       ((y1<=y) && (y<=y2)) || ((y2<=y)&&(y<=y1)));
    return xcontains && ycontains;
  }


/*
~crosses~

This function checks whether the interiors of the related
segments are crossing. If this function returns true,
the parameters ~x~ and ~y~ are set to the intersection point.

*/
 bool avlseg::AVLSegment::crosses(const avlseg::AVLSegment& s,
                                  double& x, double& y) const{
    if(isPoint() || s.isPoint()){
      return false;
    }

    if(!xOverlaps(s)){
       return false;
    }
    if(!yOverlaps(s)){
       return false;
    }
    if(overlaps(s)){ // a common line
       return false;
    }
    if(compareSlopes(s)==0){ // parallel or disjoint lines
       return false;
    }

    if(isVertical()){
        x = x1; // compute y for s
        y =  s.originY1 + ((x-s.originX1)/(s.originX2-s.originX1))*
                              (s.originY2 - s.originY1);
        roundPoint(x,y);
        bool res = innerBoxContains(x,y) && s.innerBoxContains(x,y);
        return res;
    }

    if(s.isVertical()){
       x = s.x1;
       y = originY1 + ((x-originX1)/(originX2-originX1))*(originY2-originY1);
       roundPoint(x,y);
       bool res = innerBoxContains(x,y) && s.innerBoxContains(x,y);
       return res;
    }

    // avoid problems with rounding errors during computation of
    // the intersection point
    if(pointEqual(x1,y1,s.x1,s.y1)){
      return false;
    }
    if(pointEqual(x2,y2,s.x1,s.y1)){
      return false;
    }
    if(pointEqual(x1,y1,s.x2,s.y2)){
      return false;
    }
    if(pointEqual(x2,y2,s.x2,s.y2)){
      return false;
    }


    // both segments are non vertical
    double m1 = (originY2-originY1)/(originX2-originX1);
    double m2 = (s.originY2-s.originY1)/(s.originX2-s.originX1);
    double c1 = originY1 - m1*originX1;
    double c2 = s.originY1 - m2*s.originX1;
    double xs = (c2-c1) / (m1-m2);  // x coordinate of the intersection point

    x = xs;
    y = originY1 + ((x-originX1)/(originX2-originX1))*(originY2-originY1);
    roundPoint(x,y);
    bool res = innerBoxContains(x,y) && s.innerBoxContains(x,y);
    return res;
}

bool avlseg::AVLSegment::commonPoint(const AVLSegment& s,
                                     double&x, double& y)const{
    bool res = crosses(s,x,y);
    if(res){
      return res;
    }
    if(overlaps(s)){ // more than one common point
       return false;
    }
    if(contains(s.getX1(), s.getY1())){
       x = s.getX1();
       y = s.getY1();
       return true;
    }
    if(contains(s.getX2(), s.getY2())){
       x = s.getX2();
       y = s.getY2();
       return true;
    }
    if(s.contains(getX1(), getY1())){
       x = getX1();
       y = getY1();
       return true;
    }
    if(s.contains(getX2(), getY2())){
       x = getX2();
       y = getY2();
       return true;
    }
    return false;

}





/*
~extends~

This function returns true, iff this segment is an extension of
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool avlseg::AVLSegment::extends(const avlseg::AVLSegment& s)const{
     return pointEqual(x1,y1,s.x2,s.y2) &&
            compareSlopes(s)==0;
  }

/*
~exactEqualsTo~

This function checks if s has the same geometry like this segment, i.e.
if both endpoints are equal.

*/
bool avlseg::AVLSegment::exactEqualsTo(const avlseg::AVLSegment& s)const{
  return pointEqual(x1,y1,s.x1,s.y1) &&
         pointEqual(x2,y2,s.x2,s.y2);
}

/*
~isVertical~

Checks whether this segment is vertical.

*/

 bool avlseg::AVLSegment::isVertical() const{
     return AlmostEqual(originX1,originX2);
 }

/*
~isPoint~

Checks if this segment consists only of a single point.

*/
  bool avlseg::AVLSegment::isPoint() const{
     return AlmostEqual(x1,x2) && AlmostEqual(y1,y2);
  }

/*
~length~

Returns the length of this segment.

*/
  double avlseg::AVLSegment::length() const{
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
  }


/*
~InnerDisjoint~

This function checks whether this segment and s have at most a
common endpoint.

*/

  bool avlseg::AVLSegment::innerDisjoint(const avlseg::AVLSegment& s)const{
      if(!xOverlaps(s) || !yOverlaps(s)){ // bounding box check
         return true;
      }

      if(pointEqual(x1,y1,s.x2,s.y2)){ // common endpoint
        return true;
      }
      if(pointEqual(s.x1,s.y1,x2,y2)){ // common endpoint
        return true;
      }
      if(overlaps(s)){ // a common line
         return false;
      }
      if(compareSlopes(s)==0){ // parallel or disjoint lines
         return true;
      }

      if(pointEqual(x1,y1,s.x1,s.y1)){ // common left end point
         return true;
      }
      if(pointEqual(x2,y2,s.x2,s.y2)){ // common right end point
         return true;
      }

      // endpoints within interior of the other segment
      if(ininterior(s.x1,s.y1)){
         return false;
      }
      if(ininterior(s.x2,s.y2)){
         return false;
      }
      if(s.ininterior(x1,y1)){
        return false;
      }
      if(s.ininterior(x2,y2)){
        return false;
      }
      // check for crosses
      if(crosses(s)){
         return false;
      }
      return true;

  }


  bool avlseg::AVLSegment::printInnerDisjoint(const avlseg::AVLSegment& s)const{
     cout << "Check for inner disjoint" << endl;
      if(!xOverlaps(s) || !yOverlaps(s)){ // bounding box check
         cout << "intersection free by bounding box check" << endl;
         return true;
      }

      if(pointEqual(x1,y1,s.x2,s.y2)){ // common endpoina
        cout << "disjoint, because this extends s" << endl;
        return true;
      }
      if(pointEqual(s.x1,s.y1,x2,y2)){ // common endpoint
        cout << "disjoint because s extends this " << endl;
        return true;
      }
      if(overlaps(s)){ // a common line
         cout << "common part" << endl;
         return false;
      }
      if(compareSlopes(s)==0){ // parallel or disjoint lines
         cout << "parallel or disjoint lines" << endl;
         return true;
      }

      if(pointEqual(x1,y1,s.x1,s.y1)){ // common left end point
         cout << "disjoint because common left endpoint"
              << " and different slopes" << endl;
         return true;
      }
      if(pointEqual(x2,y2,s.x2,s.y2)){ // common right end point
         cout << "disjoint because common right endpoint and different slopes"
              << endl;
         return true;
      }

      // endpoints within interior of the other segment
      if(ininterior(s.x1,s.y1)){
         cout << "this contains the left endpoint of s " << endl;
         return false;
      }
      if(ininterior(s.x2,s.y2)){
         cout << "this contains the right endpoint of s " << endl;
         return false;
      }
      if(s.ininterior(x1,y1)){
         cout << " s contains the left endpoint ofd this " << endl;
        return false;
      }
      if(s.ininterior(x2,y2)){
        cout << "s contains the right end point of this " << endl;
        return false;
      }
      // check for crosses
      if(crosses(s)){
         cout << " this crosses s " << endl;
         return false;
      }
      cout << "disjoint because no other case is found" << endl;
      return true;

  }
/*
~Intersects~

This function checks whether this segment and ~s~ have at least a
common point.

*/

  bool avlseg::AVLSegment::intersects(const avlseg::AVLSegment& s)const{
      if(!xOverlaps(s) || !yOverlaps(s)){ // bounding box check
         return true;
      }

      if(pointEqual(x1,y1,s.x2,s.y2)){ // common endpoint
        return true;
      }
      if(pointEqual(s.x1,s.y1,x2,y2)){ // common endpoint
        return true;
      }
      if(pointEqual(x1,y1,s.x1,s.y1)){ // common endpoint
        return true;
      }
      if(pointEqual(s.x2,s.y2,x2,y2)){ // common endpoint
        return true;
      }

      if(overlaps(s)){ // a common line segment
         return true;
      }

      if(compareSlopes(s)==0){ // parallel or disjoint lines
         return false;
      }
      if(ininterior(s.x1,s.y1)){
         return true;
      }
      if(ininterior(s.x2,s.y2)){
         return true;
      }
      if(s.ininterior(x1,y1)){
        return true;
      }
      if(s.ininterior(x2,y2)){
        return true;
      }
      // check for crosses
      if(crosses(s)){
         return true;
      }
      return false;

  }

/*
~overlaps~

Checks whether this segment and ~s~ have a common segment.

*/
   bool avlseg::AVLSegment::overlaps(const avlseg::AVLSegment& s) const{
      if(isPoint() || s.isPoint()){
         return false;
      }

      if(compareSlopes(s)!=0){
          return false;
      }
      // one segment is an extension of the other one
      if(pointEqual(x1,y1,s.x2,s.y2)){
          return false;
      }
      if(pointEqual(x2,y2,s.x1,s.y1)){
         return false;
      }
      if(!xOverlaps(s)) {
         return false;
      }
      if(!yOverlaps(s)){
        return false;
      }

      return contains(s.x1,s.y1) || contains(s.x2,s.y2);
   }

/*
~ininterior~

This function checks whether the point defined by (x,y) is
part of the interior of this segment.

*/
   bool avlseg::AVLSegment::ininterior(const double x,const  double y)const{
     if(isPoint()){ // a point has no interior
       return false;
     }

     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){ // an endpoint
        return false;
     }

     if(!AlmostEqual(x,x1) && x < x1){ // (x,y) left of this
         return false;
     }
     if(!AlmostEqual(x,x2) && x > x2){ // (X,Y) right of this
        return false;
     }
     if(isVertical()){
       return ((y>y1) && (y<y2)) ||
              ((y<y1) && (y>y2)) ;
     }
     double ys = getY(x);
     return AlmostEqual(y,ys);
   }


/*
~contains~

Checks whether the point defined by (x,y) is located anywhere on this
segment.

*/
   bool avlseg::AVLSegment::contains(const double x,const  double y)const{
     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){
        return true;
     }
     if(isPoint()){
       return false;
     }
     if(!PointOnSegment(x,y,originX1, originY1, originX2, originY2)){
        return false;
     }
     return innerBoxContains(x,y);
   }

/*
3.6 Comparison

Compares this with s. The x intervals must overlap.

*/

 int avlseg::AVLSegment::compareTo(const avlseg::AVLSegment& s) const{

    if(!xOverlaps(s) && !avlseg::AVLSegment::x_error){
      cerr << "Warning: compare AVLSegments with disjoint x intervals" << endl;
      cerr << "This may be a problem of roundig errors!" << endl;
      cerr << "*this = " << *this << endl;
      cerr << " s    = " << s << endl;
      x_error  = true;
      error_value = max(x1,s.x1);
    }

    if(isPoint()){
      if(s.isPoint()){
        return comparePoints(x1,y1,s.x1,s.y1);
      } else {
        if(s.contains(x1,y1)){
           return 0;
        } else {
           double y = s.getY(x1);
           if(y1<y){
             return -1;
           } else {
             return 1;
           }
        }
      }
    }
    if(s.isPoint()){
      if(contains(s.x1,s.y1)){
        return 0;
      } else {
        double y = getY(s.x1);
        if(y<s.y1){
          return -1;
        } else {
          return 1;
        }
      }
    }


   if(overlaps(s)){
     return 0;
   }

    bool v1 = isVertical();
    bool v2 = s.isVertical();

    if(!v1 && !v2){
       double x = max(x1,s.x1); // the right one of the left coordinates
       double y_this = getY(x);
       double y_s = s.getY(x);
       if(!AlmostEqual(y_this,y_s)){
          if(y_this<y_s){
            return -1;
          } else  {
            return 1;
          }
       } else {
         int cmp = compareSlopes(s);
         if(cmp!=0){
           return cmp;
         }
         // if the segments are connected, the left segment
         // is the smaller one
         if(AlmostEqual(x2,s.x1)){
             return -1;
         }
         if(AlmostEqual(s.x2,x1)){
             return 1;
         }
         // the segments have an proper overlap
         return 0;
       }
   } else if(v1 && v2){ // both are vertical
      if(AlmostEqual(y1,s.y2) || (y1>s.y2)){ // this is above s
        return 1;
      }
      if(AlmostEqual(s.y1,y2) || (s.y1>y2)){ // s above this
        return 1;
      }
      // proper overlapping part
      return 0;
  } else { // one segment is vertical

    double x = v1? x1 : s.x1; // x coordinate of the vertical segment
    double y1 = getY(x);
    double y2 = s.getY(x);
    if(AlmostEqual(y1,y2)){
        return v1?1:-1; // vertical segments have the greatest slope
    } else if(y1<y2){
       return -1;
    } else {
       return 1;
    }
  }
 }


/*
~SetOwner~

This function changes the owner of this segment.

*/
  void avlseg::AVLSegment::setOwner(avlseg::ownertype o){
    this->owner = o;
  }

/*
3.7 Some ~Get~ Functions

~getInsideAbove~

Returns the insideAbove value for such segments for which this value is unique,
e.g. for segments having owner __first__ or __second__.

*/
  bool avlseg::AVLSegment::getInsideAbove() const{
      switch(owner){
        case first : return insideAbove_first;
        case second: return insideAbove_second;
        default : assert(false);
      }
      return false;
  }

/*
3.8 Split Functions

~split~

This function splits two overlapping segments.
Preconditions:

1) this segment and ~s~ have to overlap.

2) the owner of this and ~s~ must be different

~left~, ~common~ and ~right~ will contain the
explicitely left part, a common part, and
an explicitely right part. The left and/or right part
may be empty. The existence can be checked using the return
value of this function. Let ret the return value. It holds:

  __ret | LEFT__: the left part exists

  __ret | COMMON__: the common part exist (always true)

  __ret | RIGHT__: the right part exists


The constants LEFT, COMMON, and RIGHT have been defined
earlier.

*/

  uint32_t avlseg::AVLSegment::split(const avlseg::AVLSegment& s,
                               avlseg::AVLSegment& left,
                               avlseg::AVLSegment& common,
                               avlseg::AVLSegment& right,
                               const bool checkOwner/* = true*/) const{

     assert(overlaps(s));
     if(checkOwner){
       assert( (this->owner==first && s.owner==second) ||
               (this->owner==second && s.owner==first));
     }


     uint32_t result = 0;



     int cmp = comparePoints(x1,y1,s.x1,s.y1);
     if(cmp==0){        // there is no left part
        left.x1 = x1;
        left.y1 = y1;
        left.x2 = x1;
        left.y2 = y1;
        left.originX1 = x1;
        left.originY1 = y1;
        left.originX2 = x2;
        left.originY2 = y2;
     } else { // there is a left part
       result = result | avlseg::LEFT;
       if(cmp<0){ // this is smaller
         left.x1 = x1;
         left.y1 = y1;
         left.x2 = s.x1;
         left.y2 = s.y1;
         left.owner = this->owner;
         left.con_above = this->con_above;
         left.con_below = this->con_below;
         left.insideAbove_first = this->insideAbove_first;
         left.insideAbove_second = this->insideAbove_second;
         left.originX1 = originX1;
         left.originX2 = originX2;
         left.originY1 = originY1;
         left.originY2 = originY2;
       } else { // s is smaller than this
         left.x1 = s.x1;
         left.y1 = s.y1;
         left.x2 = this->x1;
         left.y2 = this->y1;
         left.owner = s.owner;
         left.con_above = s.con_above;
         left.con_below = s.con_below;
         left.insideAbove_first = s.insideAbove_first;
         left.insideAbove_second = s.insideAbove_second;
         left.originX1 = s.originX1;
         left.originX2 = s.originX2;
         left.originY1 = s.originY1;
         left.originY2 = s.originY2;
       }
     }

    // there is an overlapping part
    result = result | COMMON;
    cmp = comparePoints(x2,y2,s.x2,s.y2);
    common.owner = both;
    common.x1 = left.x2;
    common.y1 = left.y2;
    if(this->owner==first){
      common.insideAbove_first  = insideAbove_first;
      common.insideAbove_second = s.insideAbove_second;
    } else {
      common.insideAbove_first = s.insideAbove_first;
      common.insideAbove_second = insideAbove_second;
    }
    common.con_above = this->con_above;
    common.con_below = this->con_below;

    // for the common part, the origin is not clear. we take just the
    // line with maximum length;
    common.originX1 = left.originX1;
    common.originY1 = left.originY1;


    if(cmp<0){
       common.x2 = x2;
       common.y2 = y2;
    } else {
       common.x2 = s.x2;
       common.y2 = s.y2;
    }
    if(cmp==0){ // common right endpoint
        common.originX2 = common.x2;
        common.originY2 = common.y2;
      //  assert(left.CheckPoints());
      //  assert(common.CheckPoints());
        return result;
    }


    result = result | avlseg::RIGHT;
    right.x1 = common.x2;
    right.y1 = common.y2;
    if(cmp<0){ // right part comes from s
       right.owner = s.owner;
       right.x2 = s.x2;
       right.y2 = s.y2;
       right.insideAbove_first = s.insideAbove_first;
       right.insideAbove_second = s.insideAbove_second;
       right.con_below = s.con_below;
       right.con_above = s.con_above;
       right.originX1 = s.originX1;
       right.originY1 = s.originY1;
       right.originX2 = s.originX2;
       right.originY2 = s.originY2;
    }  else { // right part comes from this
       right.owner = this->owner;
       right.x2 = this->x2;
       right.y2 = this->y2;
       right.insideAbove_first = this->insideAbove_first;
       right.insideAbove_second = this->insideAbove_second;
       right.con_below = this->con_below;
       right.con_above = this->con_above;
       right.originX1 = originX1;
       right.originY1 = originY1;
       right.originX2 = originX2;
       right.originY2 = originY2;
    }
    common.originX2 = right.originX2;
    common.originY2 = right.originY2;
   // assert(left.CheckPoints());
   // assert(common.CheckPoints());
   // assert(right.CheckPoints());
    return result;
  }

/*
~splitAt~

This function divides a segment into two parts at the point
provided by (x, y). The point must be on the interior of this segment.

*/

  void avlseg::AVLSegment::splitAt(const double x, const double y,
               avlseg::AVLSegment& left,
               avlseg::AVLSegment& right)const{

     left.x1=x1;
     left.y1=y1;
     left.x2 = x;
     left.y2 = y;
     left.owner = owner;
     left.insideAbove_first = insideAbove_first;
     left.insideAbove_second = insideAbove_second;
     left.con_below = con_below;
     left.con_above = con_above;

     right.x1=x;
     right.y1=y;
     right.x2 = x2;
     right.y2 = y2;
     right.owner = owner;
     right.insideAbove_first = insideAbove_first;
     right.insideAbove_second = insideAbove_second;
     right.con_below = con_below;
     right.con_above = con_above;

     left.originX1 = originX1;
     left.originX2 = originX2;
     left.originY1 = originY1;
     left.originY2 = originY2;

     right.originX1 = originX1;
     right.originX2 = originX2;
     right.originY1 = originY1;
     right.originY2 = originY2;

     //assert(left.CheckPoints());
     //assert(right.CheckPoints());

  }


  void avlseg::AVLSegment::splitAtRight(const double x, const double y,
               avlseg::AVLSegment& right)const{
     if(AlmostEqual(x,x1) && AlmostEqual(y,y1)){
        right = *this;
        return;
     }
     if(AlmostEqual(x,x2) && AlmostEqual(y,y2)){
        // return only the right endpoint
        right = *this;
        right.x1 = x2;
        right.y1 = y2;
        return;
     }
     if(!AlmostEqual(x,x1) && (x < x1)){
        right = *this;
        return;
     }

     right.x1=x;
     right.y1=y;
     right.x2 = x2;
     right.y2 = y2;
     right.owner = owner;
     right.insideAbove_first = insideAbove_first;
     right.insideAbove_second = insideAbove_second;
     right.con_below = con_below;
     right.con_above = con_above;
     right.originX1 = originX1;
     right.originX2 = originX2;
     right.originY1 = originY1;
     right.originY2 = originY2;
     //assert(right.CheckPoints());
  }

/*
~splitCross~

Splits two crossing segments into the 4 corresponding parts.
Both segments have to cross each other.

*/
void avlseg::AVLSegment::splitCross(const avlseg::AVLSegment& s,
                                          avlseg::AVLSegment& left1,
                                          avlseg::AVLSegment& right1,
                                          avlseg::AVLSegment& left2,
                                          avlseg::AVLSegment& right2) const{

    double x,y;
    bool cross = crosses(s,x,y);
    assert(cross);
    splitAt(x, y, left1, right1);
    s.splitAt(x, y, left2, right2);
    //assert(left1.CheckPoints());
    //assert(right1.CheckPoints());
    //assert(left2.CheckPoints());
    //assert(right2.CheckPoints());
}

/*
3.9 Converting Functions

~ConvertToHs~

This functions creates a ~HalfSegment~ from this segment.
The owner must be __first__ or __second__.

*/
avlseg::ExtendedHalfSegment avlseg::AVLSegment::convertToExtendedHs(bool lpd,
                            avlseg::ownertype owner/* = both*/)const{
   //assert( owner!=both || this->owner==first || this->owner==second);
   //assert( owner==both || owner==first || owner==second);

   bool insideAbove;
   if(owner==both){
      insideAbove = this->owner==first?insideAbove_first
                                  :insideAbove_second;
   } else {
      insideAbove = owner==first?insideAbove_first
                                  :insideAbove_second;
   }
   Point p1(true,x1,y1);
   Point p2(true,x2,y2);
   HalfSegment hs(lpd, p1, p2);
   hs.attr.insideAbove = insideAbove;
   avlseg::ExtendedHalfSegment extHs(hs);
   extHs.setOrigin(originX1, originY1, originX2, originY2);
   return extHs;
}

/*
~pointequal~

This function checks if the points defined by (x1, y1) and
(x2,y2) are equals using the ~AlmostEqual~ function.

*/
  bool avlseg::AVLSegment::pointEqual(const double x1, const double y1,
                         const double x2, const double y2){
    return AlmostEqual(x1,x2) && AlmostEqual(y1,y2);
  }

/*
~pointSmaller~

This function checks if the point defined by (x1, y1) is
smaller than the point defined by (x2, y2).

*/

 bool avlseg::AVLSegment::pointSmaller(const double x1, const double y1,
                          const double x2, const double y2) {

    return comparePoints(x1,y1,x2,y2) < 0;
 }


/*
~comparePoints~

*/
  int avlseg::AVLSegment::comparePoints(const double x1,const  double y1,
                            const double x2,const double y2){
     if(AlmostEqual(x1,x2)){
       if(AlmostEqual(y1,y2)){
          return 0;
       } else if(y1<y2){
          return -1;
       } else {
          return 1;
       }
     } else if(x1<x2){
       return -1;
     } else {
       return 1;
     }
  }

/*
~compareSlopes~

compares the slopes of __this__ and __s__. The slope of a vertical
segment is greater than all other slopes.

*/
   int avlseg::AVLSegment::compareSlopes(const avlseg::AVLSegment& s) const{
      assert(!isPoint() && !s.isPoint());
      bool v1 = AlmostEqual(x1,x2);
      bool v2 = AlmostEqual(s.x1,s.x2);
      if(v1 && v2){ // both segments are vertical
        return 0;
      }
      if(v1){
        return 1;  // this is vertical, s not
      }
      if(v2){
        return -1; // s is vertical
      }

      // both segments are non-vertical
      double res1 = (originY2-originY1)/(originX2-originX1);
      double res2 = (s.originY2-s.originY1)/(s.originX2-s.originX1);
      int result = -3;
      if( AlmostEqual(res1,res2)){
         result = 0;
      } else if(res1<res2){
         result =  -1;
      } else { // res1>res2
         result = 1;
      }
      return result;
   }

/*
~roundPoint~

whenever the point (x,y) is almostequal to an endpoint, its rounded
to that endpoint.

*/

   void avlseg::AVLSegment::roundPoint(double& x , double& y) const{
     if(pointEqual(x,y,originX1,originY1)){
       x = originX1;
       y = originY1;
       return;
     }
     if(pointEqual(x,y,originX2,originY2)){
       x = originX2;
       y = originY2;
       return;
     }
     if(pointEqual(x,y,x1,y1)){
       x = x1;
       y = y1;
       return;
     }
     if(pointEqual(x,y,x2,y2)){
       x = x2;
       y = y2;
       return;
     }

   }

/*
~XOverlaps~

Checks whether the x interval of this segment overlaps the
x interval of ~s~.

*/

  bool avlseg::AVLSegment::xOverlaps(const avlseg::AVLSegment& s) const{
    if(!AlmostEqual(x1,s.x2) && x1 > s.x2){ // left of s
        return false;
    }
    if(!AlmostEqual(x2,s.x1) && x2 < s.x1){ // right of s
        return false;
    }
    return true;
  }

  bool avlseg::AVLSegment::yOverlaps(const avlseg::AVLSegment& s) const{
     if(AlmostEqual(y1,s.y1) || AlmostEqual(y1,s.y2) ||
        AlmostEqual(y2,s.y1) || AlmostEqual(y2,s.y2)){
       return true;
     }
     double ymin = y1<y2?y1:y2;
     double ymax = y1>y2?y1:y2;

     if((ymin>s.y1) && (ymin>s.y2)) return  false;
     if((ymax<s.y1) && (ymax<s.y2)) return false;
     return true;

  }

/*
~XContains~

Checks if the x coordinate provided by the parameter __x__ is contained
in the x interval of this segment;

*/
  bool avlseg::AVLSegment::xContains(const double x) const{
    if(!AlmostEqual(x1,x) && x1>x){
      return false;
    }
    if(!AlmostEqual(x2,x) && x2<x){
      return false;
    }
    return true;
  }

/*
~GetY~

Computes the y value for the specified  __x__.
__x__ must be contained in the x-interval of this segment.
If the segment is vertical, the minimum y value of this
segment is returned.

*/
  double avlseg::AVLSegment::getY(const double x) const{
     if(!xContains(x)){
       cerr << "Warning: compute y value for a x outside the x interval!"
            << endl;
       double diff1 = x1 - x;
       double diff2 = x - x2;
       double diff = (diff1>diff2?diff1:diff2);
       cerr << "difference to x is " << diff << endl;
       cerr << "The segment is " << *this << endl;
       //assert(diff < 1.0);
     }
     if(isVertical()){
        return y1;
     }
     if(AlmostEqual(x,x1)){
        return y1;
     }
     if(AlmostEqual(x,x2)){
        return y2;
     }
     double d = (x-originX1)/(originX2-originX1);
     return originY1 + d*(originY2-originY1);
  }


/*
3.12 Shift Operator

*/
ostream& avlseg::operator<<(ostream& o, const avlseg::AVLSegment& s){
    s.Print(o);
    return o;
}



/*
~insertEvents~

Creates events for the ~AVLSegment~ and insert them into ~q1~ and/ or ~q1~.
The target queue(s) is (are) determined by the owner of ~seg~.
The flags ~createLeft~ and ~createRight~ determine
whether the left and / or the right events should be created.

*/

void insertEvents(const avlseg::AVLSegment& seg,
                  const bool createLeft,
                  const bool createRight,
                  priority_queue<avlseg::ExtendedHalfSegment,
                                 vector<avlseg::ExtendedHalfSegment>,
                                 greater<avlseg::ExtendedHalfSegment> >& q1,
                  priority_queue<avlseg::ExtendedHalfSegment,
                                 vector<avlseg::ExtendedHalfSegment>,
                                 greater<avlseg::ExtendedHalfSegment> >& q2){

   if(seg.isPoint()){
     return;
   }
   switch(seg.getOwner()){
      case avlseg::first: {
           if(createLeft){
              q1.push(seg.convertToExtendedHs(true, avlseg::first));
           }
           if(createRight){
              q1.push(seg.convertToExtendedHs(false, avlseg::first));
           }
           break;
      } case avlseg::second:{
           if(createLeft){
              q2.push(seg.convertToExtendedHs(true, avlseg::second));
           }
           if(createRight){
              q2.push(seg.convertToExtendedHs(false, avlseg::second));
           }
           break;
      } case avlseg::both : {
           if(createLeft){
              q1.push(seg.convertToExtendedHs(true, avlseg::first));
              q2.push(seg.convertToExtendedHs(true, avlseg::second));
           }
           if(createRight){
              q1.push(seg.convertToExtendedHs(false, avlseg::first));
              q2.push(seg.convertToExtendedHs(false, avlseg::second));
           }
           break;
      } default: {
           assert(false);
      }
   }
}


/*
~splitByNeighbour~


~neighbour~ has to be an neighbour from ~current~ within ~sss~.

The return value is true, if current was changed.


*/

bool splitByNeighbour(avltree::AVLTree<avlseg::AVLSegment>& sss,
                      avlseg::AVLSegment& current,
                      avlseg::AVLSegment *& neighbour,
                      priority_queue<avlseg::ExtendedHalfSegment,
                                 vector<avlseg::ExtendedHalfSegment>,
                                 greater<avlseg::ExtendedHalfSegment> >& q1,
                      priority_queue<avlseg::ExtendedHalfSegment,
                                 vector<avlseg::ExtendedHalfSegment>,
                                 greater<avlseg::ExtendedHalfSegment> >& q2,
                     const bool forceThrow ){
    avlseg::AVLSegment left1, right1, left2, right2;

    if(neighbour && !neighbour->innerDisjoint(current)){
       if(neighbour->ininterior(current.getX1(),current.getY1())){
          neighbour->splitAt(current.getX1(),current.getY1(),left1,right1);
          sss.remove(*neighbour);
          if(!left1.isPoint()){
            // debug::start
            avlseg::AVLSegment* x = sss.getMember(left1);
            if(x){
               cerr << "we have a problem " << __LINE__ << endl;
               cerr << " left1 = " << left1 << endl;
               cerr << " *x = " << *x << endl;
            }
            // debug::end
            neighbour = sss.insert2(left1);
            insertEvents(left1,false,true,q1,q2);
          }
          insertEvents(right1,true,true,q1,q2);
          return false;
       } else if(neighbour->ininterior(current.getX2(),current.getY2())){
          neighbour->splitAt(current.getX2(),current.getY2(),left1,right1);
          sss.remove(*neighbour);
          if(!left1.isPoint()){
            // debug::start
            const avlseg::AVLSegment* x = sss.getMember(left1);
            if(x){
               cerr << "we have a problem " << __LINE__ << endl;
               cerr << " left1 = " << left1 << endl;
               cerr << " *x = " << *x << endl;
            }
            // debug::end
            neighbour = sss.insert2(left1);
            insertEvents(left1,false,true,q1,q2);
          }
          insertEvents(right1,true,true,q1,q2);
          return false;
       } else if(current.ininterior(neighbour->getX2(),neighbour->getY2())){
          current.splitAt(neighbour->getX2(),neighbour->getY2(),left1,right1);
          current = left1;
          insertEvents(left1,false,true,q1,q2);
          insertEvents(right1,true,true,q1,q2);
          return true;
       } else if(current.crosses(*neighbour)){
          neighbour->splitCross(current,left1,right1,left2,right2);
          sss.remove(*neighbour);
          if(!left1.isPoint()){
            neighbour = sss.insert2(left1);
          }
          current = left2;
          insertEvents(left1,false,true,q1,q2);
          insertEvents(right1,true,true,q1,q2);
          insertEvents(left2,false,true,q1,q2);
          insertEvents(right2,true,true,q1,q2);
          return true;
       } else {  // forgotten case or wrong order of halfsegments
          if(forceThrow){
             throw myexception("Invalid halfsegment order");
          }

          cerr.precision(16);
          cerr << "Warning wrong order in halfsegment array detected" << endl;

          cerr << "current" << current << endl
               << "neighbour " << (*neighbour) << endl;
          if(current.overlaps(*neighbour)){ // a common line
              cerr << "1 : The segments overlaps" << endl;
           }
           if(neighbour->ininterior(current.getX1(),current.getY1())){
              cerr << "2 : neighbour->ininterior(current.x1,current.y1)"
                   << endl;
           }
           if(neighbour->ininterior(current.getX2(),current.getY2())){
              cerr << "3 : neighbour->ininterior(current.getX2()"
                   << ",current.getY2()" << endl;
           }
          if(current.ininterior(neighbour->getX1(),neighbour->getY1())){
             cerr << " case 4 : current.ininterior(neighbour->getX1(),"
                  << "neighbour.getY1()" << endl;
             cerr << "may be an effect of rounding errors" << endl;

             cerr << "remove left part from current" << endl;
             current.splitAt(neighbour->getX1(),neighbour->getY1(),
                             left1,right1);
             cerr << "removed part is " << left1 << endl;
             current = right1;
             insertEvents(current,false,true,q1,q2);
             return true;

          }
          if(current.ininterior(neighbour->getX2(),neighbour->getY2())){
            cerr << " 5 : current.ininterior(neighbour->getX2(),"
                 << "neighbour->getY2())" << endl;
          }
          if(current.crosses(*neighbour)){
             cerr << "6 : crosses" << endl;
          }
          throw myexception("Invalid order of halfsegments");
          assert(false);
          return true;
       }
    } else {
      return false;
    }
}


/*
~splitNeighbours~

Checks if the left and the right neighbour are intersecting in their
interiors and performs the required actions.


*/

void splitNeighbours(avltree::AVLTree<avlseg::AVLSegment>& sss,
                     avlseg::AVLSegment *& leftN,
                     avlseg::AVLSegment *& rightN,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q1,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q2,
                     const bool forceThrow){

  if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
    avlseg::AVLSegment left1, right1, left2, right2;
    if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
       // right endpoint of rightN in interior of leftN
       leftN->splitAt(rightN->getX2(),rightN->getY2(),left1,right1);
       sss.remove(*leftN);
       if(!left1.isPoint()){
         leftN = sss.insert2(left1);
         insertEvents(left1,false,true,q1,q2);
       }
       insertEvents(right1,true,true,q1,q2);
    } else if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
       // right endpoint of leftN in interior of rightN
       rightN->splitAt(leftN->getX2(),leftN->getY2(),left1,right1);
       sss.remove(*rightN);
       if(!left1.isPoint()){
         rightN = sss.insert2(left1);
         insertEvents(left1,false,true,q1,q2);
       }
       insertEvents(right1,true,true,q1,q2);
    } else if (rightN->crosses(*leftN)){
       // leftN and rightN are crossing
         leftN->splitCross(*rightN,left1,right1,left2,right2);
         sss.remove(*leftN);
         sss.remove(*rightN);
         if(!left1.isPoint()) {
           leftN = sss.insert2(left1);
         }
         if(!left2.isPoint()){
            rightN = sss.insert2(left2);
         }
         insertEvents(left1,false,true,q1,q2);
         insertEvents(left2,false,true,q1,q2);
         insertEvents(right1,true,true,q1,q2);
         insertEvents(right2,true,true,q1,q2);
    } else if(leftN->ininterior(rightN->getX1(), rightN->getY1())){
       if(forceThrow){
          throw myexception("found element to split too late");
       }
       cerr << __LINE__ << "Warning found an element to split too late" << endl;
       sss.remove(*leftN);
       leftN->splitAt(rightN->getX1(), rightN->getY1(), left1, right1);
       if(!left1.isPoint()){
         leftN = sss.insert2(left1);
         insertEvents(left1, false,true,q1,q2);
       }
       insertEvents(right1, true,true,q1,q2);
    } else if(rightN->ininterior(leftN->getX1(), leftN->getY1())){
       if(forceThrow){
          throw myexception("found element to split too late");
       }
       cerr << __LINE__ << "Warning found an element to split too late" << endl;
       sss.remove(*rightN);
       rightN->splitAt(leftN->getX1(), leftN->getY1(), left1, right1);
       if(!left1.isPoint()){
          rightN = sss.insert2(left1);
          insertEvents(left1, false, true, q1,q2);
       }
       insertEvents(right1, true,true,q1,q2);
    } else { // forgotten case or overlapping segments (rounding errors)
       if(leftN->overlaps(*rightN)){
         if(forceThrow){
            throw myexception("found element to split too late");
         }
         cerr << "Overlapping neighbours found" << endl;
         cerr << "leftN = " << *leftN << endl;
         cerr << "rightN = " << *rightN << endl;
         avlseg::AVLSegment left;
         avlseg::AVLSegment common;
         avlseg::AVLSegment right;
         uint32_t parts = leftN->split(*rightN, left,common,right,false);
         sss.remove(*leftN);
         sss.remove(*rightN);
         if(parts & avlseg::LEFT){
           if(!left.isPoint()){
             cerr << "insert left part" << left << endl;
             leftN = sss.insert2(left);
             insertEvents(left,false,true,q1,q2);
           }
         }
         if(parts & avlseg::COMMON){
           if(!common.isPoint()){
             cerr << "insert common part" << common << endl;
             rightN = sss.insert2(common);
             insertEvents(common,false,true,q1,q2);
           }
         }
         if(parts & avlseg::RIGHT){
           if(!right.isPoint()){
             cerr << "insert events for the right part" << right << endl;;
             insertEvents(right,true,true,q1,q2);
           }
         }

       } else {
          if(forceThrow){
             throw myexception("found element to split too late");
          }
          cout.precision(16);
          cout << "Found to segments which are in no  valid relation " << endl;
          cout << " leftN  = " << (*leftN) << endl;
          cout << " rightN = " << (*rightN) << endl;

          cout << "leftN.isVertical  : " << leftN->isVertical() << endl;
          cout << "rightN.isVertical : " << rightN->isVertical() << endl;

          cout << "leftN.innerDisjoint(rightN) : "
               << leftN->innerDisjoint(*rightN) << endl;
          cout << "rightN.innerDisjoint(leftN) : "
               << rightN->innerDisjoint(*leftN) << endl;

          cout << "leftN.intersects(rightN) : "
               << leftN->intersects(*rightN) << endl;
          cout << "rightN.intersects(leftN) : "
               << rightN->intersects(*leftN) << endl;


          cout << "leftN.overlaps(rightN) : "
               << leftN->overlaps(*rightN) << endl;
          cout << "rightN.overlaps(leftN) : "
               << rightN->overlaps(*leftN) << endl;


          cout << "leftN.compareTo(rightN) : "
               << leftN->compareTo(*rightN) << endl;
          cout << "rightN.compareTo(leftN) : "
               << rightN->compareTo(*leftN) << endl;


          cout << "leftN.compareSlopes(rightN) : "
               << leftN->compareSlopes(*rightN) << endl;
          cout << "rightN.compareSlopes(leftN) : "
               << rightN->compareSlopes(*leftN) << endl;


          cout << "leftN.xOverlaps(rightN) : "
               << leftN->xOverlaps(*rightN) << endl;
          cout << "rightN.xOverlaps(leftN) : "
               << rightN->xOverlaps(*leftN) << endl;

          cout << "leftN.yOverlaps(rightN) : "
               << leftN->yOverlaps(*rightN) << endl;
          cout << "rightN.yOverlaps(leftN) : "
               << rightN->yOverlaps(*leftN) << endl;

          leftN->printInnerDisjoint(*rightN);

          assert(false);
       }
    }
  } // intersecting neighbours
}


/*
9 Set Operations (union, intersection, difference)


The following functions implement the operations ~union~,
~intersection~ and ~difference~ for some combinations of spatial types.


*/











