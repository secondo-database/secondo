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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Transportation Mode Algebra

March, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
partitioning space.


*/

#include "Partition.h"

/*
~Shift~ Operator for ~ownertype~

*/

ostream& myavlseg::operator<<(ostream& o, const myavlseg::ownertype& owner){
   switch(owner){
      case myavlseg::none   : o << "none" ; break;
      case myavlseg::first  : o << "first"; break;
      case myavlseg::second : o << "second"; break;
      case myavlseg::both   : o << "both"; break;
      default     : assert(false);
   }
   return o;
}

/*
3.1 Constructors

~Standard Constructor~

*/
myavlseg::MyAVLSegment::MyAVLSegment()
{
  x1 = 0;
  x2 = 0;
  y1 = 0;
  y2 = 0;
  owner = none;
  insideAbove_first = false;
  insideAbove_second = false;
  con_below = 0;
  con_above = 0;
}

/*
~Constructor~

This constructor creates a new segment from the given HalfSegment.
As owner only __first__ and __second__ are the allowed values.

*/

  myavlseg::MyAVLSegment::MyAVLSegment(const HalfSegment& hs, ownertype owner){
     x1 = hs.GetLeftPoint().GetX();
     y1 = hs.GetLeftPoint().GetY();
     x2 = hs.GetRightPoint().GetX();
     y2 = hs.GetRightPoint().GetY();
     if( (MyAlmostEqual(x1,x2) && (y2<y2) ) || (x2<x2) ){// swap the entries
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
  }

/*
~Constructor~

Create a Segment only consisting of a single point.

*/

myavlseg::MyAVLSegment::MyAVLSegment(const Point& p, ownertype owner)
{
    x1 = p.GetX();
    x2 = x1;
    y1 = p.GetY();
    y2 = y1;
    this->owner = owner;
    insideAbove_first = false;
    insideAbove_second = false;
    con_below = 0;
    con_above = 0;
}


/*
~Copy Constructor~

*/
   myavlseg::MyAVLSegment::MyAVLSegment(const MyAVLSegment& src){
      Equalize(src);
   }



/*
3.3 Operators

*/

  myavlseg::MyAVLSegment& myavlseg::MyAVLSegment::operator=(
                                         const myavlseg::MyAVLSegment& src){
    Equalize(src);
    return *this;
  }

  bool myavlseg::MyAVLSegment::operator==(const myavlseg::MyAVLSegment& s)const{
    return compareTo(s)==0;
  }

  bool myavlseg::MyAVLSegment::operator<(const myavlseg::MyAVLSegment& s) const{
     return compareTo(s)<0;
  }

  bool myavlseg::MyAVLSegment::operator>(const myavlseg::MyAVLSegment& s) const{
     return compareTo(s)>0;
  }

/*
3.3 Further Needful Functions

~Print~

This function writes this segment to __out__.

*/
  void myavlseg::MyAVLSegment::Print(ostream& out)const{
    out << "Segment("<<x1<<", " << y1 << ") -> (" << x2 << ", " << y2 <<") "
        << owner << " [ " << insideAbove_first << ", "
        << insideAbove_second << "] con("
        << con_below << ", " << con_above << ")";

  }

/*

~Equalize~

The value of this segment is taken from the argument.

*/

  void myavlseg::MyAVLSegment::Equalize( const myavlseg::MyAVLSegment& src){
     x1 = src.x1;
     x2 = src.x2;
     y1 = src.y1;
     y2 = src.y2;
     owner = src.owner;
     insideAbove_first = src.insideAbove_first;
     insideAbove_second = src.insideAbove_second;
     con_below = src.con_below;
     con_above = src.con_above;
  }




/*
3.5 Geometric Functions

~crosses~

Checks whether this segment and __s__ have an intersection point of their
interiors.

*/
 bool myavlseg::MyAVLSegment::crosses(const myavlseg::MyAVLSegment& s) const{
   double x,y;
   return crosses(s,x,y);
 }

/*
~crosses~

This function checks whether the interiors of the related
segments are crossing. If this function returns true,
the parameters ~x~ and ~y~ are set to the intersection point.

*/
 bool myavlseg::MyAVLSegment::crosses(const myavlseg::MyAVLSegment& s,
                                  double& x, double& y) const{
    if(isPoint() || s.isPoint()){
      return false;
    }

    if(!xOverlaps(s)){
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
        y =  s.y1 + ((x-s.x1)/(s.x2-s.x1))*(s.y2 - s.y1);
        return !MyAlmostEqual(y1,y) && !MyAlmostEqual(y2,y) &&
               (y>y1)  && (y<y2)
               && !MyAlmostEqual(s.x1,x) && !MyAlmostEqual(s.x2,x) ;
    }

    if(s.isVertical()){
       x = s.x1;
       y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
       return !MyAlmostEqual(y,s.y1) && !MyAlmostEqual(y,s.y2) &&
              (y>s.y1) && (y<s.y2) &&
              !MyAlmostEqual(x1,x) && !MyAlmostEqual(x2,x);
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
    double m1 = (y2-y1)/(x2-x1);
    double m2 = (s.y2-s.y1)/(s.x2-s.x1);
    double c1 = y1 - m1*x1;
    double c2 = s.y1 - m2*s.x1;
    double xs = (c2-c1) / (m1-m2);  // x coordinate of the intersection point

    x = xs;
    y = y1 + ((x-x1)/(x2-x1))*(y2-y1);

    return !MyAlmostEqual(x1,xs) && !MyAlmostEqual(x2,xs) && // not an endpoint
          !MyAlmostEqual(s.x1,xs) && !MyAlmostEqual(s.x2,xs) && //of any segment
           (x1<xs) && (xs<x2) && (s.x1<xs) && (xs<s.x2);
}

/*
~extends~

This function returns true, iff this segment is an extension of
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool myavlseg::MyAVLSegment::extends(const myavlseg::MyAVLSegment& s)const{
     return pointEqual(x1,y1,s.x2,s.y2) &&
            compareSlopes(s)==0;
  }

/*
~exactEqualsTo~

This function checks if s has the same geometry like this segment, i.e.
if both endpoints are equal.

*/
bool myavlseg::MyAVLSegment::exactEqualsTo(const myavlseg::MyAVLSegment& s)
   const{
  return pointEqual(x1,y1,s.x1,s.y1) &&
         pointEqual(x2,y2,s.x2,s.y2);
}

/*
~isVertical~

Checks whether this segment is vertical.

*/

 bool myavlseg::MyAVLSegment::isVertical() const{
     return MyAlmostEqual(x1,x2);
 }

/*
~isPoint~

Checks if this segment consists only of a single point.

*/
  bool myavlseg::MyAVLSegment::isPoint() const{
     return MyAlmostEqual(x1,x2) && MyAlmostEqual(y1,y2);
  }

/*
~length~

Returns the length of this segment.

*/
  double myavlseg::MyAVLSegment::length(){
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
  }


/*
~InnerDisjoint~

This function checks whether this segment and s have at most a
common endpoint.

*/

  bool myavlseg::MyAVLSegment::innerDisjoint(const myavlseg::MyAVLSegment& s)
    const{
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
      if(crosses(s)){
         return false;
      }
      return true;

  }
/*
~Intersects~

This function checks whether this segment and ~s~ have at least a
common point.

*/

  bool myavlseg::MyAVLSegment::intersects(const myavlseg::MyAVLSegment& s)const{
      if(pointEqual(x1,y1,s.x2,s.y2)){ // common endpoint
        return true;
      }
      if(pointEqual(s.x1,s.y1,x2,y2)){ // common endpoint
        return true;
      }
      if(overlaps(s)){ // a common line
         return true;
      }
      if(compareSlopes(s)==0){ // parallel or disjoint lines
         return false;
      }

      if(isVertical()){
        double x = x1; // compute y for s
        double y =  s.y1 + ((x-s.x1)/(s.x2-s.x1))*(s.y2 - s.y1);
        return  ( (contains(x,y) && s.contains(x,y) ) );

      }
      if(s.isVertical()){
         double x = s.x1;
         double y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
         return ((contains(x,y) && s.contains(x,y)));
      }

      // both segments are non vertical
      double m1 = (y2-y1)/(x2-x1);
      double m2 = (s.y2-s.y1)/(s.x2-s.x1);
      double c1 = y1 - m1*x1;
      double c2 = s.y1 - m2*s.x1;
      double x = (c2-c1) / (m1-m2);  // x coordinate of the intersection point
      double y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
      return ( (contains(x,y) && s.contains(x,y) ) );
  }

/*
~overlaps~

Checks whether this segment and ~s~ have a common segment.

*/
   bool myavlseg::MyAVLSegment::overlaps(const myavlseg::MyAVLSegment& s) const{
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
      return contains(s.x1,s.y1) || contains(s.x2,s.y2);
   }

/*
~ininterior~

This function checks whether the point defined by (x,y) is
part of the interior of this segment.

*/
   bool myavlseg::MyAVLSegment::ininterior(const double x,const  double y)const{
     if(isPoint()){ // a point has no interior
       return false;
     }

     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){ // an endpoint
        return false;
     }

     if(!MyAlmostEqual(x,x1) && x < x1){ // (x,y) left of this
         return false;
     }
     if(!MyAlmostEqual(x,x2) && x > x2){ // (X,Y) right of this
        return false;
     }
     if(isVertical()){
       return (!MyAlmostEqual(y,y1) && (y>y1) &&
               !MyAlmostEqual(y,y2) && (y<y2));
     }
     double ys = getY(x);
     return MyAlmostEqual(y,ys);
   }


/*
~contains~

Checks whether the point defined by (x,y) is located anywhere on this
segment.

*/
   bool myavlseg::MyAVLSegment::contains(const double x,const  double y)const{
     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){
        return true;
     }
     if(isPoint()){
       return false;
     }
     if(MyAlmostEqual(x1,x2)){ // vertical segment
        return (y>=y1) && (y <= y2);
     }
     // check if (x,y) is located on the line
     double res1 = (x-x1)*(y2-y1);
     double res2 = (y-y1)*(x2-x1);
     if(!MyAlmostEqual(res1,res2)){
         return false;
     }

     return ((x>x1) && (x<x2)) ||
            MyAlmostEqual(x,x1) ||
            MyAlmostEqual(x,x2);
   }

/*
3.6 Comparison

Compares this with s. The x intervals must overlap.

*/

 int myavlseg::MyAVLSegment::compareTo(const myavlseg::MyAVLSegment& s) const{

    if(!xOverlaps(s)){
     cerr << "Warning: compare MyAVLSegments with disjoint x intervals" << endl;
      cerr << "This may be a problem of roundig errors!" << endl;
      cerr << "*this = " << *this << endl;
      cerr << " s    = " << s << endl;
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
       if(!MyAlmostEqual(y_this,y_s)){
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
         if(MyAlmostEqual(x2,s.x1)){
             return -1;
         }
         if(MyAlmostEqual(s.x2,x1)){
             return 1;
         }
         // the segments have an proper overlap
         return 0;
       }
   } else if(v1 && v2){ // both are vertical
      if(MyAlmostEqual(y1,s.y2) || (y1>s.y2)){ // this is above s
        return 1;
      }
      if(MyAlmostEqual(s.y1,y2) || (s.y1>y2)){ // s above this
        return 1;
      }
      // proper overlapping part
      return 0;
  } else { // one segment is vertical

    double x = v1? x1 : s.x1; // x coordinate of the vertical segment
    double y1 = getY(x);
    double y2 = s.getY(x);
    if(MyAlmostEqual(y1,y2)){
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
  void myavlseg::MyAVLSegment::setOwner(myavlseg::ownertype o){
    this->owner = o;
  }

/*
3.7 Some ~Get~ Functions

~getInsideAbove~

Returns the insideAbove value for such segments for which this value is unique,
e.g. for segments having owner __first__ or __second__.

*/
  bool myavlseg::MyAVLSegment::getInsideAbove() const{
      switch(owner){
        case first : return insideAbove_first;
        case second: return insideAbove_second;
        default : assert(false);
      }
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
an explecitely right part. The left and/or right part
my be empty. The existence can be checked using the return
value of this function. Let ret the return value. It holds:

  __ret | LEFT__: the left part exists

  __ret | COMMON__: the common part exist (always true)

  __ret | RIGHT__: the right part exists


The constants LEFT, COMMON, and RIGHT have been defined
earlier.

*/

  int myavlseg::MyAVLSegment::split(const myavlseg::MyAVLSegment& s,
                               myavlseg::MyAVLSegment& left,
                               myavlseg::MyAVLSegment& common,
                               myavlseg::MyAVLSegment& right,
                               const bool checkOwner/* = true*/) const{

     assert(overlaps(s));
     if(checkOwner){
       assert( (this->owner==first && s.owner==second) ||
               (this->owner==second && s.owner==first));
     }


     int result = 0;



     int cmp = comparePoints(x1,y1,s.x1,s.y1);
     if(cmp==0){
        left.x1 = x1;
        left.y1 = y1;
        left.x2 = x1;
        left.y2 = y1;
     } else { // there is a left part
       result = result | myavlseg::LEFT;
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

    if(cmp<0){
       common.x2 = x2;
       common.y2 = y2;
    } else {
       common.x2 = s.x2;
       common.y2 = s.y2;
    }
    if(cmp==0){ // common right endpoint
        return result;
    }

    result = result | myavlseg::RIGHT;
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
    }  else { // right part comes from this
       right.owner = this->owner;
       right.x2 = this->x2;
       right.y2 = this->y2;
       right.insideAbove_first = this->insideAbove_first;
       right.insideAbove_second = this->insideAbove_second;
       right.con_below = this->con_below;
       right.con_above = this->con_above;
    }
   return result;


  }

/*
~splitAt~

This function divides a segment into two parts at the point
provided by (x, y). The point must be on the interior of this segment.

*/

  void myavlseg::MyAVLSegment::splitAt(const double x, const double y,
               myavlseg::MyAVLSegment& left,
               myavlseg::MyAVLSegment& right)const{

  /*
    // debug::start
    if(!ininterior(x,y)){
         cout << "ininterior check failed (may be an effect"
              << " of rounding errors !!!" << endl;
         cout << "The segment is " << *this << endl;
         cout << "The point is (" <<  x << " , " << y << ")" << endl;
     }
     // debug::end
   */

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

  }

/*
~splitCross~

Splits two crossing segments into the 4 corresponding parts.
Both segments have to cross each other.

*/
void myavlseg::MyAVLSegment::splitCross(const myavlseg::MyAVLSegment& s,
                                          myavlseg::MyAVLSegment& left1,
                                          myavlseg::MyAVLSegment& right1,
                                          myavlseg::MyAVLSegment& left2,
                                          myavlseg::MyAVLSegment& right2) const{

    double x,y;
    bool cross = crosses(s,x,y);
    assert(cross);
    splitAt(x, y, left1, right1);
    s.splitAt(x, y, left2, right2);
}

/*
3.9 Converting Functions

~ConvertToHs~

This functions creates a ~HalfSegment~ from this segment.
The owner must be __first__ or __second__.

*/
HalfSegment myavlseg::MyAVLSegment::convertToHs(bool lpd,
                            myavlseg::ownertype owner/* = both*/)const{
   assert( owner!=both || this->owner==first || this->owner==second);
   assert( owner==both || owner==first || owner==second);

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
   return hs;
}

/*
~pointequal~

This function checks if the points defined by (x1, y1) and
(x2,y2) are equals using the ~MyAlmostEqual~ function.

*/
  bool myavlseg::MyAVLSegment::pointEqual(const double x1, const double y1,
                         const double x2, const double y2){
    return MyAlmostEqual(x1,x2) && MyAlmostEqual(y1,y2);
  }

/*
~pointSmaller~

This function checks if the point defined by (x1, y1) is
smaller than the point defined by (x2, y2).

*/

 bool myavlseg::MyAVLSegment::pointSmaller(const double x1, const double y1,
                          const double x2, const double y2){

    return comparePoints(x1,y1,x2,y2) < 0;
 }


/*
~comparePoints~

*/
  int myavlseg::MyAVLSegment::comparePoints(const double x1,const  double y1,
                            const double x2,const double y2){
     if(MyAlmostEqual(x1,x2)){
       if(MyAlmostEqual(y1,y2)){
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
   int myavlseg::MyAVLSegment::compareSlopes(const myavlseg::MyAVLSegment& s)
    const{
      assert(!isPoint() && !s.isPoint());
      bool v1 = MyAlmostEqual(x1,x2);
      bool v2 = MyAlmostEqual(s.x1,s.x2);
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
      double res1 = (y2-y1)/(x2-x1);
      double res2 = (s.y2-s.y1)/(s.x2-s.x1);
      int result = -3;
      if( MyAlmostEqual(res1,res2)){
         result = 0;
      } else if(res1<res2){
         result =  -1;
      } else { // res1>res2
         result = 1;
      }
      return result;
   }

/*
~XOverlaps~

Checks whether the x interval of this segment overlaps the
x interval of ~s~.

*/

  bool myavlseg::MyAVLSegment::xOverlaps(const myavlseg::MyAVLSegment& s) const{
    if(!MyAlmostEqual(x1,s.x2) && x1 > s.x2){ // left of s
        return false;
    }
    if(!MyAlmostEqual(x2,s.x1) && x2 < s.x1){ // right of s
        return false;
    }
    return true;
  }

/*
~XContains~

Checks if the x coordinate provided by the parameter __x__ is contained
in the x interval of this segment;

*/
  bool myavlseg::MyAVLSegment::xContains(const double x) const{
    if(!MyAlmostEqual(x1,x) && x1>x){
      return false;
    }
    if(!MyAlmostEqual(x2,x) && x2<x){
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
  double myavlseg::MyAVLSegment::getY(const double x) const{

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
     double d = (x-x1)/(x2-x1);
     return y1 + d*(y2-y1);
  }


/*
3.12 Shift Operator

*/
ostream& myavlseg::operator<<(ostream& o, const myavlseg::MyAVLSegment& s){
    s.Print(o);
    return o;
}

/*

~selectNext~

Selects the minimum halfsegment from ~v~1, ~v~2, ~q~1, and ~q~2.
If no values are available, the return value will be __none__.
In this case, __result__ remains unchanged. Otherwise, __result__
is set to the minimum value found. In this case, the return value
will be ~first~ or ~second~.
If some halfsegments are equal, the one
from  ~v~1 is selected.
Note: ~pos~1 and ~pos~2 are increased automatically. In the same way,
      the topmost element of the selected queue is deleted.

The template parameter can be instantiated with ~Region~ or ~Line~

*/
template<class T1, class T2>
myavlseg::ownertype myselectNext(const T1& v1,
                     int& pos1,
                     const T2& v2,
                     int& pos2,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
                     int& src = 0
                    ){


  const HalfSegment* values[4];
  HalfSegment hs0, hs1, hs2, hs3;
  int number = 0; // number of available values
  // read the available elements
  if(pos1<v1.Size()){
     v1.Get(pos1,hs0);
     values[0] = &hs0;
     number++;
  }  else {
     values[0]=0;
  }
  if(q1.empty()){
    values[1] = 0;
  } else {
    values[1] = &q1.top();
    number++;
  }
  if(pos2<v2.Size()){
     v2.Get(pos2,hs2);
     values[2] = &hs2;
     number++;
  }  else {
     values[2] = 0;
  }
  if(q2.empty()){
    values[3]=0;
  } else {
    values[3] = &q2.top();
    number++;
  }
  // no halfsegments found

  if(number == 0){
     return myavlseg::none;
  }
  // search for the minimum.
  int index = -1;
  for(int i=0;i<4;i++){
    if(values[i]){
       if(index<0 || (result > *values[i])){
          result = *values[i];
          index = i;
       }
    }
  }
  src = index +  1;
  switch(index){
    case 0: pos1++; return myavlseg::first;
    case 1: q1.pop();  return myavlseg::first;
    case 2: pos2++;  return myavlseg::second;
    case 3: q2.pop();  return myavlseg::second;
    default: assert(false);
  }
  return myavlseg::none;
}

/*
Instantiation of the ~selectNext~ Function.

*/

myavlseg::ownertype myselectNext(const Region& reg1,
                     int& pos1,
                     const Region& reg2,
                     int& pos2,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
                     int& src // for debugging only
                    ){
   return myselectNext<Region,Region>(reg1,pos1,reg2,pos2,q1,q2,result,src);
}

/*
~insertEvents~

Creates events for the ~AVLSegment~ and insert them into ~q1~ and/ or ~q1~.
The target queue(s) is (are) determined by the owner of ~seg~.
The flags ~createLeft~ and ~createRight~ determine
whether the left and / or the right events should be created.

*/

void myinsertEvents(const myavlseg::MyAVLSegment& seg,
                  const bool createLeft,
                  const bool createRight,
                  priority_queue<HalfSegment,
                                 vector<HalfSegment>,
                                 greater<HalfSegment> >& q1,
                  priority_queue<HalfSegment,
                                 vector<HalfSegment>,
                                 greater<HalfSegment> >& q2){
   if(seg.isPoint()){
     return;
   }
   switch(seg.getOwner()){
      case myavlseg::first: {
           if(createLeft){
              q1.push(seg.convertToHs(true, myavlseg::first));
           }
           if(createRight){
              q1.push(seg.convertToHs(false, myavlseg::first));
           }
           break;
      } case myavlseg::second:{
           if(createLeft){
              q2.push(seg.convertToHs(true, myavlseg::second));
           }
           if(createRight){
              q2.push(seg.convertToHs(false, myavlseg::second));
           }
           break;
      } case myavlseg::both : {
           if(createLeft){
              q1.push(seg.convertToHs(true, myavlseg::first));
              q2.push(seg.convertToHs(true, myavlseg::second));
           }
           if(createRight){
              q1.push(seg.convertToHs(false, myavlseg::first));
              q2.push(seg.convertToHs(false, myavlseg::second));
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

bool MysplitByNeighbour(avltree::AVLTree<myavlseg::MyAVLSegment>& sss,
                      myavlseg::MyAVLSegment& current,
                      myavlseg::MyAVLSegment const*& neighbour,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q1,
                      priority_queue<HalfSegment,
                                     vector<HalfSegment>,
                                     greater<HalfSegment> >& q2){
    myavlseg::MyAVLSegment left1, right1, left2, right2;

    if(neighbour && !neighbour->innerDisjoint(current)){
       if(neighbour->ininterior(current.getX1(),current.getY1())){
          neighbour->splitAt(current.getX1(),current.getY1(),left1,right1);
          sss.remove(*neighbour);
          if(!left1.isPoint()){
            neighbour = sss.insert2(left1);
            myinsertEvents(left1,false,true,q1,q2);
          }
          myinsertEvents(right1,true,true,q1,q2);
          return false;
       } else if(neighbour->ininterior(current.getX2(),current.getY2())){
          neighbour->splitAt(current.getX2(),current.getY2(),left1,right1);
          sss.remove(*neighbour);
          if(!left1.isPoint()){
            neighbour = sss.insert2(left1);
            myinsertEvents(left1,false,true,q1,q2);
          }
          myinsertEvents(right1,true,true,q1,q2);
          return false;
       } else if(current.ininterior(neighbour->getX2(),neighbour->getY2())){
          current.splitAt(neighbour->getX2(),neighbour->getY2(),left1,right1);
          current = left1;
          myinsertEvents(left1,false,true,q1,q2);
          myinsertEvents(right1,true,true,q1,q2);
          return true;
       } else if(current.crosses(*neighbour)){
          neighbour->splitCross(current,left1,right1,left2,right2);
          sss.remove(*neighbour);
          if(!left1.isPoint()){
            neighbour = sss.insert2(left1);
          }
          current = left2;
          myinsertEvents(left1,false,true,q1,q2);
          myinsertEvents(right1,true,true,q1,q2);
          myinsertEvents(left2,false,true,q1,q2);
          myinsertEvents(right2,true,true,q1,q2);
          return true;
       } else {  // forgotten case or wrong order of halfsegments
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
             myinsertEvents(current,false,true,q1,q2);
             return true;

          }
          if(current.ininterior(neighbour->getX2(),neighbour->getY2())){
            cerr << " 5 : current.ininterior(neighbour->getX2(),"
                 << "neighbour->getY2())" << endl;
          }
          if(current.crosses(*neighbour)){
             cerr << "6 : crosses" << endl;
          }
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

void MysplitNeighbours(avltree::AVLTree<myavlseg::MyAVLSegment>& sss,
                     myavlseg::MyAVLSegment const*& leftN,
                     myavlseg::MyAVLSegment const*& rightN,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,
                                    vector<HalfSegment>,
                                    greater<HalfSegment> >& q2){
  if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
    myavlseg::MyAVLSegment left1, right1, left2, right2;
    if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
       leftN->splitAt(rightN->getX2(),rightN->getY2(),left1,right1);
       sss.remove(*leftN);
       if(!left1.isPoint()){
         leftN = sss.insert2(left1);
         myinsertEvents(left1,false,true,q1,q2);
       }
       myinsertEvents(right1,true,true,q1,q2);
    } else if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
       rightN->splitAt(leftN->getX2(),leftN->getY2(),left1,right1);
       sss.remove(*rightN);
       if(!left1.isPoint()){
         rightN = sss.insert2(left1);
         myinsertEvents(left1,false,true,q1,q2);
       }
       myinsertEvents(right1,true,true,q1,q2);
    } else if (rightN->crosses(*leftN)){
         leftN->splitCross(*rightN,left1,right1,left2,right2);
         sss.remove(*leftN);
         sss.remove(*rightN);
         if(!left1.isPoint()) {
           leftN = sss.insert2(left1);
         }
         if(!left2.isPoint()){
            rightN = sss.insert2(left2);
         }
         myinsertEvents(left1,false,true,q1,q2);
         myinsertEvents(left2,false,true,q1,q2);
         myinsertEvents(right1,true,true,q1,q2);
         myinsertEvents(right2,true,true,q1,q2);
    } else { // forgotten case or overlapping segments (rounding errors)
       if(leftN->overlaps(*rightN)){
         cerr << "Overlapping neighbours found" << endl;
         cerr << "leftN = " << *leftN << endl;
         cerr << "rightN = " << *rightN << endl;
         myavlseg::MyAVLSegment left;
         myavlseg::MyAVLSegment common;
         myavlseg::MyAVLSegment right;
         int parts = leftN->split(*rightN, left,common,right,false);
         sss.remove(*leftN);
         sss.remove(*rightN);
         if(parts & avlseg::LEFT){
           if(!left.isPoint()){
             cerr << "insert left part" << left << endl;
             leftN = sss.insert2(left);
             myinsertEvents(left,false,true,q1,q2);
           }
         }
         if(parts & avlseg::COMMON){
           if(!common.isPoint()){
             cerr << "insert common part" << common << endl;
             rightN = sss.insert2(common);
             myinsertEvents(common,false,true,q1,q2);
           }
         }
         if(parts & avlseg::RIGHT){
           if(!right.isPoint()){
             cerr << "insert events for the right part" << right << endl;;
             myinsertEvents(right,true,true,q1,q2);
           }
         }

       } else {
          assert(false);
       }
    }
  } // intersecting neighbours
}
void MyMinus(const Region& reg1, const Region& reg2, Region& result)
{
  MySetOp(reg1,reg2,result,myavlseg::difference_op);
}

void MyIntersection(const Region& reg1, const Region& reg2, Region& result)
{
  MySetOp(reg1,reg2,result,myavlseg::intersection_op);
}

void MyUnion(const Region& reg1, const Region& reg2, Region& result)
{
  MySetOp(reg1,reg2,result,myavlseg::union_op);
}

void MySetOp(const Region& reg1,
           const Region& reg2,
           Region& result,
           myavlseg::SetOperation op){

   result.Clear();
   if(!reg1.IsDefined() || !reg2.IsDefined()){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(reg1.Size()==0){
       switch(op){
         case myavlseg::union_op : result = reg2;
                         return;
         case myavlseg::intersection_op : return; // empty region
         case myavlseg::difference_op : return; // empty region
         default : assert(false);
       }
   }
   if(reg2.Size()==0){
      switch(op){
         case myavlseg::union_op: result = reg1;
                        return;
         case myavlseg::intersection_op: return;
         case myavlseg::difference_op: result = reg1;
                             return;
         default : assert(false);
      }
   }

   if(!reg1.BoundingBox().Intersects(reg2.BoundingBox())){
      switch(op){
        case myavlseg::union_op: {
          result.StartBulkLoad();
          int edgeno=0;
          int s = reg1.Size();
          HalfSegment hs;
          for(int i=0;i<s;i++){
              reg1.Get(i,hs);
              if(hs.IsLeftDomPoint()){
                 HalfSegment HS(hs);
                 HS.attr.edgeno = edgeno;
                 result += HS;
                 HS.SetLeftDomPoint(false);
                 result += HS;
                 edgeno++;
              }
          }
          s = reg2.Size();
          for(int i=0;i<s;i++){
              reg2.Get(i,hs);
              if(hs.IsLeftDomPoint()){
                 HalfSegment HS(hs);
                 HS.attr.edgeno = edgeno;
                 result += HS;
                 HS.SetLeftDomPoint(false);
                 result += HS;
                 edgeno++;
              }
          }
          result.EndBulkLoad();
          return;
        } case myavlseg::difference_op: {
           result = reg1;
           return;
        } case myavlseg::intersection_op:{
           return;
        } default: assert(false);
      }
   }

  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;
  avltree::AVLTree<myavlseg::MyAVLSegment> sss;
  myavlseg::ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  HalfSegment nextHs;
  int src = 0;

  const myavlseg::MyAVLSegment* member = 0;
  const myavlseg::MyAVLSegment* leftN  = 0;
  const myavlseg::MyAVLSegment* rightN = 0;

  myavlseg::MyAVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  myavlseg::MyAVLSegment tmpL,tmpR;

  result.StartBulkLoad();

  while( (owner=myselectNext(reg1,pos1,
                           reg2,pos2,
                           q1,q2,nextHs,src))!=myavlseg::none){

       myavlseg::MyAVLSegment current(nextHs,owner);
       member = sss.getMember(current,leftN,rightN);
       if(leftN){
          tmpL = *leftN;
          leftN = &tmpL;
       }
       if(rightN){
          tmpR = *rightN;
          rightN = &tmpR;
       }
       if(nextHs.IsLeftDomPoint()){
          if(member){ // overlapping segment found
            if((member->getOwner()==myavlseg::both) ||
               (member->getOwner()==owner)){
               cerr << "overlapping segments detected within a single region"
                    << endl;
               cerr << "the argument is "
                    << (owner==myavlseg::first?"first":"second")
                    << endl;
               cerr.precision(16);
               cerr << "stored is " << *member << endl;
               cerr << "current = " << current << endl;
               myavlseg::MyAVLSegment tmp_left, tmp_common, tmp_right;
               member->split(current,tmp_left, tmp_common, tmp_right, false);
               cerr << "The common part is " << tmp_common << endl;
               cerr << "The lenth = " << tmp_common.length() << endl;
               assert(false);
            }
            int parts = member->split(current,left1,common1,right1);
            sss.remove(*member);
            if(parts & myavlseg::LEFT){
              if(!left1.isPoint()){
                sss.insert(left1);
                myinsertEvents(left1,false,true,q1,q2);
              }
            }
            assert(parts & myavlseg::COMMON);
            // update coverage numbers
            if(current.getInsideAbove()){
               common1.con_above++;
            }  else {
               common1.con_above--;
            }
            if(!common1.isPoint()){
              sss.insert(common1);
              myinsertEvents(common1,false,true,q1,q2);
            }
            if(parts & myavlseg::RIGHT){
               myinsertEvents(right1,true,true,q1,q2);
            }
          } else { // there is no overlapping segment
            // try to split segments if required
            MysplitByNeighbour(sss,current,leftN,q1,q2);
            MysplitByNeighbour(sss,current,rightN,q1,q2);

            // update coverage numbers
            bool iac = current.getOwner()== myavlseg::first
                            ?current.getInsideAbove_first()
                            :current.getInsideAbove_second();

            iac = current.getOwner()== myavlseg::first
                                           ?current.getInsideAbove_first()
                                           :current.getInsideAbove_second();

            if(leftN && current.extends(*leftN)){
              current.con_below = leftN->con_below;
              current.con_above = leftN->con_above;
            }else{
              if(leftN && leftN->isVertical()){
                 current.con_below = leftN->con_below;
              } else if(leftN){
                 current.con_below = leftN->con_above;
              } else {
                 current.con_below = 0;
              }
              if(iac){
                 current.con_above = current.con_below+1;
              } else {
                 current.con_above = current.con_below-1;
              }
            }
            // insert element
            if(!current.isPoint()){
              sss.insert(current);
              myinsertEvents(current,false,true,q1,q2);
            }
          }
       } else {  // nextHs.IsRightDomPoint
          if(member && member->exactEqualsTo(current)){
              switch(op){
                case myavlseg::union_op :{

                   if( (member->con_above==0) || (member->con_below==0)) {
                      HalfSegment hs1 = member->getOwner()==myavlseg::both
                                      ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                      hs1.attr.edgeno = edgeno;
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                   }
                   break;
                }
                case myavlseg::intersection_op: {

                  if(member->con_above==2 || member->con_below==2){
                      HalfSegment hs1 = member->getOwner()==myavlseg::both
                                      ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                      hs1.attr.edgeno = edgeno;
                      hs1.attr.insideAbove = (member->con_above==2);
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                  }
                  break;
                }
                case myavlseg::difference_op : {
                  switch(member->getOwner()){
                    case myavlseg::first:{
                      if(member->con_above + member->con_below == 1){
                         HalfSegment hs1 = member->getOwner()==myavlseg::both
                                      ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case myavlseg::second:{
                      if(member->con_above + member->con_below == 3){
                         HalfSegment hs1 = member->getOwner()==myavlseg::both
                                     ?member->convertToHs(true,myavlseg::second)
                                      :member->convertToHs(true);
                         hs1.attr.insideAbove = ! hs1.attr.insideAbove;
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case myavlseg::both: {
                      if((member->con_above==1) && (member->con_below== 1)){
                         HalfSegment hs1 = member->getOwner()==myavlseg::both
                                     ?member->convertToHs(true,myavlseg::first)
                                      :member->convertToHs(true);
                         hs1.attr.insideAbove = member->getInsideAbove_first();
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    default : assert(false);
                  } // switch member->getOwner
                  break;
                } // case difference
                default : assert(false);
              } // end of switch
              sss.remove(*member);
              MysplitNeighbours(sss,leftN,rightN,q1,q2);
          } // current found in sss
       } // right endpoint
  }
  result.EndBulkLoad();


} // setOP region x region -> region

/*
The following is the implementation of structure for space partition

*/

/*
Default constructor function

*/
SpacePartition::SpacePartition()
{
      l = NULL;
      resulttype = NULL;
      count = 0;
}

SpacePartition::SpacePartition(Relation* in_line):l(in_line),count(0){}

/*
take the input point as the center and delta as the radius
a line function represented by a and b
it returns the intersection point (only x value) for the circle and line
function

*/
void SpacePartition::GetDeviation(Point center, double a, double b,double& x1,
                   double& x2, int delta)
{
    double x0 = center.GetX();
    double y0 = center.GetY();
    double A = 1 + a*a;
    double B = 2*a*(b-y0)-2*x0;
//    double C = x0*x0 + (b-y0)*(b-y0) - 1;
    double C = x0*x0 + (b-y0)*(b-y0) - delta*delta;
    x1 = (-B - sqrt(B*B-4*A*C))/(2*A);
    x2 = (-B + sqrt(B*B-4*A*C))/(2*A);
}

/*
It checks whether the rotation from segment (p1-p0) to segment (p2-p0) is
counterclockwise or clockwise
TRUE--clockwise  false--couterclockwise

*/

bool SpacePartition::GetClockwise(Point& p0,Point& p1, Point& p2)
{
    double x0 = p0.GetX();
    double y0 = p0.GetY();
    double x1 = p1.GetX();
    double y1 = p1.GetY();
    double x2 = p2.GetX();
    double y2 = p2.GetY();
    bool result;
    if(AlmostEqual(x0,x1)){
        if(y1 >= y0){
//          if(x2 < x0) result = false;
          if(x2 < x0 || AlmostEqual(x2,x0)) result = false;
          else result = true;
        }else{
          if(x2 < x0) result = true;
          else result = false;
        }
    }else{
          double slope = (y1-y0)/(x1-x0);

/*          if(AlmostEqual(y1,y0))
            slope = 0;
          else
            slope = (y1-y0)/(x1-x0);*/

          double intercept = y1-slope*x1;
          if(x1 < x0){
            if(y2 < (slope*x2 + intercept)) result = false;
            else result = true;
          }else{
            if(y2 < (slope*x2 + intercept)) result = true;
            else result = false;
          }
    }
//      if(result) cout<<"clockwise "<<endl;
//     else cout<<"counterclokwise "<<endl;
      return result;
}

/*
It gets the angle of the rotation from segment (p1-p0) to segment (p2-p0)

*/

double SpacePartition::GetAngle(Point& p0,Point& p1, Point& p2)
{
      /////cosne theorem ///
      double angle; //radian [0-pi]
      double b = p0.Distance(p1);
      double c = p0.Distance(p2);
      double a = p1.Distance(p2);
      assert(AlmostEqual(b*c,0.0) == false);
      double value = (b*b+c*c-a*a)/(2*b*c);

      if(AlmostEqual(value,-1.0)) value = -1;
      if(AlmostEqual(value,1.0)) value = 1;
      angle = acos(value);
//      cout<<"angle "<<angle<<" degree "<<angle*180.0/pi<<endl;
      assert(0.0 <= angle && angle <= 3.1416);
      return angle;
}

/*
Given a halfsegment, transfer it by a deviation (delta) to the left or right
side (up or down), determined by clockflag.
Put the transfered halfsegment into structure boundary
for example, (2, 2)--(3, 2), delta = 1
it return (2, 1)---(3,1) or (2,3)--(3,3)

*/

void SpacePartition::TransferSegment(MyHalfSegment& mhs,
                    vector<MyHalfSegment>& boundary, int delta, bool clock_flag)
{

    Point from = mhs.GetLeftPoint();
    Point to = mhs.GetRightPoint();
    Point next_from1;
    Point next_to1;

    Point p1,p2,p3,p4;

    if(AlmostEqual(from.GetX(),to.GetX())){
 /*     next_from1.Set(from.GetX() + delta, from.GetY());
      next_to1.Set(to.GetX() + delta, to.GetY());
      MyHalfSegment* seg = new MyHalfSegment(true,next_from1,next_to1);
      boundary.push_back(*seg);
      delete seg;
      return;*/
      p1.Set(from.GetX() - delta,from.GetY());
      p2.Set(from.GetX() + delta,from.GetY());
      p3.Set(to.GetX() - delta, to.GetY());
      p4.Set(to.GetX() + delta, to.GetY());
    }
    else if(AlmostEqual(from.GetY(), to.GetY())){
/*      next_from1.Set(from.GetX(),from.GetY() - delta);
      next_to1.Set(to.GetX(), to.GetY() - delta);
      MyHalfSegment* seg = new MyHalfSegment(true,next_from1,next_to1);
      boundary.push_back(*seg);
      delete seg;
      return;*/
      p1.Set(from.GetX(), from.GetY() - delta);
      p2.Set(from.GetX(), from.GetY() + delta);
      p3.Set(to.GetX(), to.GetY() - delta);
      p4.Set(to.GetX(), to.GetY() + delta);
    }else{

      double k1 = (from.GetY() - to.GetY())/(from.GetX() - to.GetX());

      double k2 = -1/k1;
      double b1 = from.GetY() - k2*from.GetX();

      double x1,x2;
      GetDeviation(from,k2,b1,x1,x2,delta);

      double y1 = x1*k2 + b1;
      double y2 = x2*k2 + b1;

      double x3,x4;
      double b2 = to.GetY() - k2*to.GetX();
      GetDeviation(to,k2,b2,x3,x4,delta);

      double y3 = x3*k2 + b2;
      double y4 = x4*k2 + b2;

      p1.Set(x1,y1);
      p2.Set(x2,y2);
      p3.Set(x3,y3);
      p4.Set(x4,y4);
    }

    vector<Point> clock_wise;
    vector<Point> counterclock_wise;
    if(GetClockwise(from,to,p1)) clock_wise.push_back(p1);
    else counterclock_wise.push_back(p1);

    if(GetClockwise(from,to,p2)) clock_wise.push_back(p2);
    else counterclock_wise.push_back(p2);

    if(GetClockwise(from,to,p3)) clock_wise.push_back(p3);
      else counterclock_wise.push_back(p3);

    if(GetClockwise(from,to,p4)) clock_wise.push_back(p4);
      else counterclock_wise.push_back(p4);

    assert(clock_wise.size() == 2 && counterclock_wise.size() == 2);
    if(clock_flag){
      next_from1 = clock_wise[0];
      next_to1 = clock_wise[1];
    }else{
      next_from1 = counterclock_wise[0];
      next_to1 = counterclock_wise[1];
    }

    MyHalfSegment* seg = new MyHalfSegment(true,next_from1,next_to1);
    boundary.push_back(*seg);
    delete seg;
}

/*
Moidfy the coordinate value of the point, change it into int value or
reduce the precision to be more practical

*/

inline void SpacePartition::ModifyPoint(Point& p)
{
    double a, b;
    int x, y;
    a = p.GetX();
    b = p.GetY();

    x = static_cast<int>(GetCloser(a));
    y = static_cast<int>(GetCloser(b));
    p.Set(x,y);

//    printf("%.8f %.8f\n",a,b);
//    printf("%d %d\n",x,y);
}
inline void SpacePartition::Modify_Point(Point& p)
{
    double x,y;
    x = p.GetX();
    y = p.GetY();
//    printf("%.10f %.10f\n",x, y);
    x = ((int)(x*100.0 + 0.5))/100.0;
    y = ((int)(y*100.0 + 0.5))/100.0;
//    printf("%.10f %.10f\n",x, y);
    p.Set(x,y);
}

/*
Add one segment to line, but change the point value to int

*/
void SpacePartition::AddHalfSegmentResult(MyHalfSegment hs, Line* res,
                                         int& edgeno)
{
      const double delta_dist = 0.1;
      double a1,b1,a2,b2;
      int x1,y1,x2,y2;
      a1 = hs.GetLeftPoint().GetX();
      b1 = hs.GetLeftPoint().GetY();
      a2 = hs.GetRightPoint().GetX();
      b2 = hs.GetRightPoint().GetY();

      x1 = static_cast<int>(GetCloser(a1));
      y1 = static_cast<int>(GetCloser(b1));
      x2 = static_cast<int>(GetCloser(a2));
      y2 = static_cast<int>(GetCloser(b2));

      HalfSegment hs2;
      Point p1,p2;
      p1.Set(x1,y1);
      p2.Set(x2,y2);

      if(p1.Distance(p2) > delta_dist){//////////////final check
        hs2.Set(true,p1,p2);
        hs2.attr.edgeno = edgeno++;
        *res += hs2;
        hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
        *res += hs2;
      }
}



/*
for the given line stored in segs, it gets its left or right side line after
transfer by a deviation given by delta

*/
void SpacePartition::Gettheboundary(vector<MyHalfSegment>& segs,
                      vector<MyHalfSegment>& boundary, int delta,
                      bool clock_wise)
{
    const double delta_dist = 0.1;
    for(unsigned int i = 0;i < segs.size();i++){

      TransferSegment(segs[i], boundary, delta, clock_wise);

    }

    ////////  connect the new boundary ////////////////////////
    for(unsigned int i = 0;i < boundary.size() - 1; i++){
      Point p1_1 = boundary[i].GetLeftPoint();
      Point p1_2 = boundary[i].GetRightPoint();
      Point p2_1 = boundary[i+1].GetLeftPoint();
      Point p2_2 = boundary[i+1].GetRightPoint();

//      cout<<p1_1<<" "<<p1_2<<" "<<p2_1<<" "<<p2_2<<endl;

      if(p1_2.Distance(p2_1) < delta_dist) continue;


      if(AlmostEqual(p1_1.GetX(),p1_2.GetX())){
        assert(!AlmostEqual(p2_1.GetX(),p2_2.GetX()));
        double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
        double b2 = p2_2.GetY() - a2*p2_2.GetX();

        double x = p1_1.GetX();
        double y = a2*x + b2;
        boundary[i].to.Set(x,y);
        boundary[i+1].from.Set(x,y);

      }else{
        if(AlmostEqual(p2_1.GetX(),p2_2.GetX())){

          assert(!AlmostEqual(p1_1.GetX(),p1_2.GetX()));
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double x = p2_1.GetX();
          double y = a1*x + b1;
          boundary[i].to.Set(x,y);
          boundary[i+1].from.Set(x,y);

        }else{
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
          double b2 = p2_2.GetY() - a2*p2_2.GetX();

//          assert(!AlmostEqual(a1,a2));
//          cout<<"a1 "<<a1<<" a2 "<<a2<<endl;

          if(AlmostEqual(a1,a2)) assert(AlmostEqual(b1,b2));

          double x = (b2-b1)/(a1-a2);
          double y = a1*x + b1;
          ////////////process speical case angle too small////////////
          Point q1;
          q1.Set(x,y);
          Point q2 = segs[i].GetRightPoint();
//          cout<<"q1 "<<q1<<" q2 "<<q2<<endl;
          if(q1.Distance(q2) > 5*delta){

//            assert(false);
//            cout<<"special case"<<endl;
          }
          /////////////////////////////////////////////////////
          boundary[i].to.Set(x,y);
          boundary[i+1].from.Set(x,y);
        }
//        cout<<boundary[i].to<<" "<<boundary[i+1].from<<endl;

      }// end if

    }//end for

}

/*
For the given line, it gets all the points forming its boundary where delta
defines the deviation of the left or right side line for transfer.
outer does not include points from road, but outerhalf includes
It is to create the region for road

*/
void SpacePartition::ExtendSeg1(vector<MyHalfSegment>& segs,int delta,
                bool clock_wise,
                vector<Point>& outer, vector<Point>& outer_half)
{
    const double delta_dist = 0.1;

    for(unsigned int i = 0;i < segs.size();i++){
 //     cout<<"start "<<segs[i].GetLeftPoint()<<" to "
 //         <<segs[i].GetRightPoint()<<endl;
      if(i < segs.size() - 1){
        Point to1 = segs[i].GetRightPoint();
        Point from2 = segs[i+1].GetLeftPoint();
        assert(to1.Distance(from2) < delta_dist);
      }
    }

    vector<MyHalfSegment> boundary;
    Gettheboundary(segs, boundary, delta, clock_wise);

    ///////////////////////add two more segments ///////////////////////
      Point old_start = segs[0].GetLeftPoint();
      Point old_end = segs[segs.size() - 1].GetRightPoint();

      Point new_start = boundary[0].GetLeftPoint();
      Point new_end = boundary[boundary.size() - 1].GetRightPoint();


      MyHalfSegment* mhs = new MyHalfSegment(true, old_start, new_start);
      boundary.push_back(*mhs);
      for(int i = boundary.size() - 2; i >= 0;i --)
          boundary[i+1] = boundary[i];

      boundary[0] = *mhs;
      delete mhs;


      mhs = new MyHalfSegment(true,new_end,old_end);
      boundary.push_back(*mhs);
      delete mhs;

      if(clock_wise){
        for(unsigned int i = 0;i < boundary.size();i++){
          ///////////////outer segments////////////////////////////////
          Point p = boundary[i].GetLeftPoint();
          ModifyPoint(p);

          if(i == 0){
              outer.push_back(boundary[i].GetLeftPoint());
              outer_half.push_back(boundary[i].GetLeftPoint());
          }
          else{
            outer.push_back(p);
            outer_half.push_back(p);
          }
        }

        for(int i = segs.size() - 1; i >= 0; i--)
          outer_half.push_back(segs[i].GetRightPoint());

      }else{
        for(unsigned int i = 0; i < segs.size(); i++)
          outer_half.push_back(segs[i].GetLeftPoint());

        for(int i = boundary.size() - 1;i >= 0;i--){
          /////////////////////////////////////////////////////////////
          Point p = boundary[i].GetRightPoint();
          ModifyPoint(p);
         /////////////////////////////////////////////////////////////
          if((unsigned)i == boundary.size() - 1){
              outer.push_back(boundary[i].GetRightPoint());
              outer_half.push_back(boundary[i].GetRightPoint());
          }else{
              outer.push_back(p);
              outer_half.push_back(p);
          }
        }

      }

}

/*
For the given line, it gets all the points forming its boundary where delta
defines the deviation of the left or right side line for transfer.
It is to create the region covering both road and pavement so that the original
road line is not used. This region is larger than the one created by function
ExtendSeg1 because it contains pavements.

*/
void SpacePartition::ExtendSeg2(vector<MyHalfSegment>& segs,int delta,
                     bool clock_wise, vector<Point>& outer)
{
    const double delta_dist = 0.1;

    for(unsigned int i = 0;i < segs.size();i++){
 //     cout<<"start "<<segs[i].GetLeftPoint()<<" to "
 //         <<segs[i].GetRightPoint()<<endl;
      if(i < segs.size() - 1){
        Point to1 = segs[i].GetRightPoint();
        Point from2 = segs[i+1].GetLeftPoint();
        assert(to1.Distance(from2) < delta_dist);
      }
    }


    vector<MyHalfSegment> boundary;
    Gettheboundary(segs, boundary, delta, clock_wise);

    ///////////////////////add two more segments ///////////////////////
      Point old_start = segs[0].GetLeftPoint();
      Point old_end = segs[segs.size() - 1].GetRightPoint();

      Point new_start = boundary[0].GetLeftPoint();
      Point new_end = boundary[boundary.size() - 1].GetRightPoint();


      MyHalfSegment* mhs = new MyHalfSegment(true,old_start,new_start);
      boundary.push_back(*mhs);
      for(int i = boundary.size() - 2; i >= 0;i --)
          boundary[i+1] = boundary[i];

      boundary[0] = *mhs;
      delete mhs;


      mhs = new MyHalfSegment(true,new_end,old_end);
      boundary.push_back(*mhs);
      delete mhs;


      if(clock_wise){
        for(unsigned int i = 0;i < boundary.size();i++){
          ///////////////outer segments////////////////////////////////
          Point p = boundary[i].GetLeftPoint();

          ModifyPoint(p);

          /////////////////////////////////////////////////////////
          if(i == 0){
              outer.push_back(boundary[i].GetLeftPoint());
          }
          else{
            outer.push_back(p);

          }
        }

      }else{
        for(int i = boundary.size() - 1;i >= 0;i--){
          /////////////////////////////////////////////////////////////
          Point p = boundary[i].GetRightPoint();
          ModifyPoint(p);
         /////////////////////////////////////////////////////////////
          if((unsigned)i == boundary.size() - 1){
              outer.push_back(boundary[i].GetRightPoint());

          }else{
              outer.push_back(p);
          }
        }

      }

}

/*
for the given line, order it in such a way that segi.to = seg{i+1}.from
store the result in vector<MyHalfSegment>

*/
void SpacePartition::ReorderLine(SimpleLine* sline,
                                 vector<MyHalfSegment>& seq_halfseg)
{
    Point sp;
    assert(sline->AtPosition(0.0,true,sp));
    vector<MyHalfSegment> copyline;
    for(int i = 0;i < sline->Size();i++){
      HalfSegment hs;
      sline->Get(i,hs);
      if(hs.IsLeftDomPoint()){
        Point lp = hs.GetLeftPoint();
        Point rp = hs.GetRightPoint();
        MyHalfSegment* mhs = new MyHalfSegment(true,lp,rp);
        copyline.push_back(*mhs);
        delete mhs;
      }
    }
    ////////////////reorder /////////////////////////////////////
/*    cout<<"before"<<endl;
    for(unsigned i = 0;i < copyline.size();i++)
      copyline[i].Print();*/

    const double delta_dist = 0.1;
    unsigned int count = 0;
    int index = 0;
    while(count < copyline.size()){
      if(copyline[index].def == false){
        index = (index + 1) % copyline.size();
        continue;
      }
      Point from = copyline[index].GetLeftPoint();
      Point to = copyline[index].GetRightPoint();

      if(from.Distance(sp) > delta_dist && to.Distance(sp) > delta_dist){
        index = (index + 1) % copyline.size();
        continue;
      }
      if(from.Distance(sp) < delta_dist){
        sp = to;
        count++;
        copyline[index].def = false;
        index = (index + 1) % copyline.size();
        MyHalfSegment* mhs = new MyHalfSegment(true,from,to);
        seq_halfseg.push_back(*mhs);
        delete mhs;
        continue;
      }
      if(to.Distance(sp) < delta_dist){
        sp = from;
        count++;
        copyline[index].def = false;
        index = (index + 1) % copyline.size();
        MyHalfSegment* mhs = new MyHalfSegment(true,to,from);
        seq_halfseg.push_back(*mhs);
        delete mhs;
        continue;
      }
    }

}

/*
for the given set of ordered points, it creates a region

*/
void SpacePartition:: ComputeRegion(vector<Point>& outer_region,
                                    vector<Region>& regs)
{
    /////////note that points are counter_clock_wise ordered///////////////
//    for(unsigned i = 0;i < outer_region.size();i++)
//      cout<<outer_region[i];


      Region* cr = new Region( 0 );
      cr->StartBulkLoad();

      int fcno=-1;
      int ccno=-1;
      int edno= 0;
      int partnerno = 0;

      fcno++;
      ccno++;
      bool isCycle = true;

      Point firstPoint = outer_region[0];
      Point prevPoint = firstPoint;

      //Starting to compute a new cycle

      Points *cyclepoints= new Points( 8 ); // in memory
      Point currvertex, p1, p2, firstP;
      Region *rDir = new Region(32);
      rDir->StartBulkLoad();
      currvertex = firstPoint;


      cyclepoints->StartBulkLoad();
      *cyclepoints += currvertex;
      p1 = currvertex;
      firstP = p1;
      cyclepoints->EndBulkLoad();

      for(unsigned int i = 1;i < outer_region.size();i++){
        currvertex = outer_region[i];

//        if(cyclepoints->Contains(currvertex))assert(false);
        if(cyclepoints->Contains(currvertex))continue;

        ////////////////step -- 1/////////////////////////////
        p2 = currvertex;
        cyclepoints->StartBulkLoad();
        *cyclepoints += currvertex;
        cyclepoints->EndBulkLoad(true,false,false);
        /////////////step --- 2 create halfsegment/////////////////////////

        HalfSegment* hs = new HalfSegment(true,prevPoint, currvertex);
        hs->attr.faceno=fcno;
        hs->attr.cycleno=ccno;
        hs->attr.edgeno=edno;
        hs->attr.partnerno=partnerno;
        partnerno++;
        hs->attr.insideAbove = (hs->GetLeftPoint() == p1);
        ////////////////////////////////////////////////////////
        p1 = p2;
        edno++;
        prevPoint= currvertex;

        if(cr->InsertOk(*hs)){
           *cr += *hs;
//           cout<<"cr+1 "<<*hs<<endl;
           if( hs->IsLeftDomPoint()){
              *rDir += *hs;
//              cout<<"rDr+1 "<<*hs<<endl;
              hs->SetLeftDomPoint( false );
           }else{
                hs->SetLeftDomPoint( true );
//                cout<<"rDr+2 "<<*hs<<endl;
                (*rDir) += (*hs);
                }
            (*cr) += (*hs);
//            cout<<"cr+2 "<<*hs<<endl;
            delete hs;
        }else assert(false);
      }//end for

      delete cyclepoints;
      ////////////////////last segment//////////////////////////
      edno++;
      HalfSegment* hs = new HalfSegment(true, firstPoint,currvertex);
      hs->attr.faceno=fcno;
      hs->attr.cycleno=ccno;
      hs->attr.edgeno=edno;
      hs->attr.partnerno=partnerno;
      hs->attr.insideAbove = (hs->GetRightPoint() == firstP);
      partnerno++;

      //////////////////////////////////////////////////////////
      if (cr->InsertOk(*hs)){
//          cout<<"insert last segment"<<endl;
          *cr += *hs;
//          cout<<"cr+3 "<<*hs<<endl;
          if(hs->IsLeftDomPoint()){
             *rDir += *hs;
//            cout<<"rDr+3 "<<*hs<<endl;
            hs->SetLeftDomPoint( false );
          }else{
              hs->SetLeftDomPoint( true );
//              cout<<"rDr+4 "<<*hs<<endl;
              *rDir += *hs;
            }
          *cr += *hs;
//          cout<<"cr+4 "<<*hs<<endl;
          delete hs;
          rDir->EndBulkLoad(true, false, false, false);


          //To calculate the inside above attribute
//          bool direction = rDir->GetCycleDirection();
          ////explicitly define it for all regions, false -- area > 0////
          bool direction = false;//counter_wise
//          cout<<"direction "<<direction<<endl;
          int h = cr->Size() - ( rDir->Size() * 2 );
          while(h < cr->Size()){
            //after each left half segment of the region is its
            //correspondig right half segment
            HalfSegment hsIA;
            bool insideAbove;
            cr->Get(h,hsIA);

            if (direction == hsIA.attr.insideAbove)
               insideAbove = false;
            else
               insideAbove = true;
            if (!isCycle)
                insideAbove = !insideAbove;
            HalfSegment auxhsIA( hsIA );
            auxhsIA.attr.insideAbove = insideAbove;
            cr->UpdateAttr(h,auxhsIA.attr);
            //Get right half segment
            cr->Get(h+1,hsIA);
            auxhsIA = hsIA;
            auxhsIA.attr.insideAbove = insideAbove;
            cr->UpdateAttr(h+1,auxhsIA.attr);
            h+=2;
          }
          delete rDir;
        }else assert(false);

      cr->SetNoComponents( fcno+1 );
      cr->EndBulkLoad( true, true, true, false );

      regs.push_back(*cr);
      delete cr;

}

/*
this function is called by operator segment2region.
It is to create the region for the road and pavement for each input route
w is defined as the deviation

*/
void SpacePartition::ExtendRoad(int attr_pos, int w)
{
    if(w < 3){
      cout<<"road width should be larger than 2"<<endl;
      return;
    }
/*    double min_length = numeric_limits<double>::max();
    double max_length = numeric_limits<double>::min();

    for(int i = 1;i <= l->GetNoTuples();i++){
      Tuple* t = l->GetTuple(i);
      SimpleLine* sline = (SimpleLine*)t->GetAttribute(attr_pos);
      if(sline->Length() < min_length) min_length = sline->Length();
      if(sline->Length() > max_length) max_length = sline->Length();
      t->DeleteIfAllowed();
    }*/


    for(int i = 1;i <= l->GetNoTuples();i++){
      Tuple* t = l->GetTuple(i);
      SimpleLine* sline = (SimpleLine*)t->GetAttribute(attr_pos);
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
      ReorderLine(sline, seq_halfseg);

      int delta1;//width of road on each side, depend on the road length
      int delta2;
      //main road-2w, side road--w, pavement -- w/2

/*     if(sline->Length() < (min_length+max_length)/2){
           delta1 = w;

           int delta;
           delta = w/2;
           if(delta < 2) delta = 2;
           delta2 = w + delta;
      }
      else{
          delta1 = 2*w;

          int delta;
          delta = w/2;
          if(delta < 2) delta = 2;
          delta2 = 2*w + delta;
      }*/
      delta1 = w;
      int delta = w/2;
      if(delta < 2) delta = 2;
      delta2 = delta1 + delta;

      vector<Point> outer_s;
      vector<Point> outer1;
      vector<Point> outer2;
      vector<Point> outer4;
      vector<Point> outer_l;
      vector<Point> outer5;

      assert(delta1 != delta2);


//      cout<<"small1"<<endl;
      ExtendSeg1(seq_halfseg, delta1, true, outer_s,outer1);
//      cout<<"large1"<<endl;
      ExtendSeg2(seq_halfseg, delta2, true, outer_l);
      /////////////////////////////////////////////////////////////

      outer_l.push_back(outer_s[1]);
      for(int j = outer_l.size() - 1;j > 0;j--)
        outer_l[j] = outer_l[j-1];
      outer_l[1] = outer_s[1];
      outer_l.push_back(outer_s[outer_s.size() - 1]);
      int point_count1 = outer_s.size();
      int point_count2 = outer_l.size();
      //////////////////paveroad--larger/////////////////////////////////

      for(unsigned int j = 0;j < outer_l.size();j++)
          outer4.push_back(outer_l[j]);
      for(int j = seq_halfseg.size() - 1; j >= 0; j--)
          outer4.push_back(seq_halfseg[j].GetRightPoint());

      //////////////////////////////////////////////////////////
      ExtendSeg1(seq_halfseg, delta1, false, outer_s, outer2);
      ExtendSeg2(seq_halfseg, delta2, false,outer_l);
      ////////////////////////////////////////////////////////////
      outer_l.push_back(outer_s[point_count1+1]);
      for(int j = outer_l.size() - 1;j > point_count2;j--)
        outer_l[j] = outer_l[j-1];
      outer_l[point_count2 + 1] = outer_s[point_count1 + 1];
      outer_l.push_back(outer_s[outer_s.size() - 1]);
      //////////////////paveroad---larger////////////////////////////

      for(unsigned int j = 0; j < seq_halfseg.size(); j++)
          outer5.push_back(seq_halfseg[j].GetLeftPoint());
      for(unsigned int j = point_count2; j < outer_l.size();j++)
          outer5.push_back(outer_l[j]);
      /////////////////////////////////////////////////////////////
      t->DeleteIfAllowed();
      /////////////get the boundary//////////////////

      ComputeRegion(outer_s, outer_regions_s);
      ComputeRegion(outer1, outer_regions1);
      ComputeRegion(outer2, outer_regions2);
      ComputeRegion(outer_l, outer_regions_l);
      ComputeRegion(outer4, outer_regions4);
      ComputeRegion(outer5, outer_regions5);

    }
}

/*
cut the intersection region between pavement and road, especailly at junction

*/
void SpacePartition::ClipPaveRegion(Region& reg, Region& pave1, Region& pave2,
                       vector<Region>& paves,int rid, Region* inborder)
  {

      Region* comm_reg = new Region(0);
//      reg.Intersection(*inborder,*comm_reg);
      MyIntersection(reg,*inborder,*comm_reg);

      Region* result = new Region(0);
//      reg.Minus(*comm_reg,*result);
      MyMinus(reg,*comm_reg,*result);

      paves[rid - 1] = *result;
      delete result;

      delete comm_reg;
//    }

}

/*
fill the gap between two pavements at some junction positions

*/
void SpacePartition::FillPave(Network* n, vector<Region>& pavements1,
                vector<Region>& pavements2,
                vector<double>& routes_length,
                vector<Region>& paves1, vector<Region>& paves2)
{

    Relation* routes = n->GetRoutes();
    Relation* juns = n->GetJunctions();


    vector<MyJun> myjuns;
    for(int i = 1;i <= n->GetNoJunctions();i++){
      Tuple* jun_tuple = juns->GetTuple(i);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();

      Point* junp = (Point*)jun_tuple->GetAttribute(JUNCTION_POS);

      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

      double len1 = meas1->GetRealval();
      double len2 = meas2->GetRealval();

        MyJun mj(*junp, id1, id2, len1, len2);
        myjuns.push_back(mj);

      jun_tuple->DeleteIfAllowed();
    }
    juns->Delete();

    sort(myjuns.begin(), myjuns.end());


    const double delta_dist = 0.1;
    for(unsigned int i = 0 ;i < myjuns.size();i++){
        Point loc = myjuns[i].loc;
        int rid1 = myjuns[i].rid1;
        int rid2 = myjuns[i].rid2;

//        cout<<"rid1 "<<rid1<<" rid2 "<<rid2<<endl;
//      if(!(rid1 == 2397 && rid2 == 2398)) continue;

      if((MyAlmostEqual(myjuns[i].len1, 0.0)||
          MyAlmostEqual(myjuns[i].len1, routes_length[rid1 - 1])) &&
          (MyAlmostEqual(myjuns[i].len2, 0.0)||
          MyAlmostEqual(myjuns[i].len2, routes_length[rid2 - 1]))){

          vector<int> rids;

          int j = i - 1;
          while(j >= 0 && loc.Distance(myjuns[j].loc) < delta_dist){
            rids.push_back(myjuns[j].rid1);
            rids.push_back(myjuns[j].rid2);
            j--;
          }

          j = i;
          while(j < myjuns.size() && loc.Distance(myjuns[j].loc) < delta_dist){
            rids.push_back(myjuns[j].rid1);
            rids.push_back(myjuns[j].rid2);
            j++;
          }

//          cout<<" rids size "<<rids.size()<<endl;
//        cout<<"rid1 "<<rid1<<" rid2 "<<rid2<<endl;

        if(rids.size() == 2){
            NewFillPavement3(routes, rid1, rid2, &loc,
                        pavements1, pavements2, rids, paves1, paves2);
        }
        if(rids.size() == 6){
           NewFillPavement4(routes, rid1, rid2, &loc,
                        pavements1, pavements2, rids, paves1, paves2);
        }

        rids.clear();
      }
    }

}

/*
Create the pavement beside each road

*/
void SpacePartition::Getpavement(Network* n, Relation* rel1, int attr_pos,
                  Relation* rel2, int attr_pos1, int attr_pos2, int w)
{
   //cut the pavement for each junction
    vector<Region> pavements1;
    vector<Region> pavements2;
    Relation* routes = n->GetRoutes();
    vector<double> routes_length;
    for(int i = 1;i <= rel2->GetNoTuples();i++){
      Tuple* tuple = rel2->GetTuple(i);
      Region* reg1 = (Region*)tuple->GetAttribute(attr_pos1);
      Region* reg2 = (Region*)tuple->GetAttribute(attr_pos2);
      pavements1.push_back(*reg1);
      pavements2.push_back(*reg2);
      tuple->DeleteIfAllowed();

      Tuple* route_tuple = routes->GetTuple(i);
      CcReal* len = (CcReal*)route_tuple->GetAttribute(ROUTE_LENGTH);
      double length = len->GetRealval();
      route_tuple->DeleteIfAllowed();
      routes_length.push_back(length);

    }

    //////////////////////////////////////////////////////////////
    Relation* juns = n->GetJunctions();

    for(int i = 1;i <= n->GetNoJunctions();i++){
      Tuple* jun_tuple = juns->GetTuple(i);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();

//      Point* junp = (Point*)jun_tuple->GetAttribute(JUNCTION_POS);

/*      if(!(id1 == 1754 || id2 == 1754)) {
          jun_tuple->DeleteIfAllowed();
          continue;
      }*/
//      cout<<"rid1 "<<id1<<" rid2 "<<id2<<endl;

//      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
//      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

//      double len1 = meas1->GetRealval();
//      double len2 = meas2->GetRealval();


      Region rid1_pave1 = pavements1[id1 - 1];
      Region rid1_pave2 = pavements2[id1 - 1];

      Region rid2_pave1 = pavements1[id2 - 1];
      Region rid2_pave2 = pavements2[id2 - 1];

      Tuple* inborder_tuple1 = rel1->GetTuple(id1);
      Tuple* inborder_tuple2 = rel1->GetTuple(id2);
      Region* reg1 = (Region*)inborder_tuple1->GetAttribute(attr_pos);
      Region* reg2 = (Region*)inborder_tuple2->GetAttribute(attr_pos);


      ClipPaveRegion(rid1_pave1,rid2_pave1,rid2_pave2,pavements1,id1,reg2);
      ClipPaveRegion(rid1_pave2,rid2_pave1,rid2_pave2,pavements2,id1,reg2);



      ClipPaveRegion(rid2_pave1,rid1_pave1,rid1_pave2,pavements1,id2,reg1);
      ClipPaveRegion(rid2_pave2,rid1_pave1,rid1_pave2,pavements2,id2,reg1);


      inborder_tuple1->DeleteIfAllowed();
      inborder_tuple2->DeleteIfAllowed();
      jun_tuple->DeleteIfAllowed();

    }

    juns->Delete();


    //////////////fill the hole of pavement/////////////////////
      vector<Region> newpave1;
      vector<Region> newpave2;
      for(unsigned int i = 0;i < pavements1.size();i++){
        Region* reg = new Region(0);
        newpave1.push_back(*reg);
        newpave2.push_back(*reg);
        delete reg;
      }
      FillPave(n, pavements1, pavements2, routes_length, newpave1, newpave2);
      for(unsigned int i = 0;i < pavements1.size();i++){
        Region* reg1 = new Region(0);
        MyUnion(pavements1[i], newpave1[i], *reg1);
        pavements1[i] = *reg1;
        delete reg1;

        Region* reg2 = new Region(0);
        MyUnion(pavements2[i], newpave2[i], *reg2);
        pavements2[i] = *reg2;
        delete reg2;
      }
    ////////////////////////////////////////////////////////////

    for(unsigned int i = 0;i < pavements1.size();i++){
        outer_regions1.push_back(pavements1[i]);
        outer_regions2.push_back(pavements2[i]);
    }
}

/*
For the Given halfsegment, transfer it by a deviation.
Similar as the function TransferSegment()

*/
void SpacePartition::TransferHalfSegment(HalfSegment& hs, int delta, bool flag)
{
//      cout<<"before hs "<<hs<<endl;
      Point lp = hs.GetLeftPoint();
      Point rp = hs.GetRightPoint();
//      cout<<"delta "<<delta<<endl;
      if(MyAlmostEqual(lp.GetY(), rp.GetY())){
          Point p1, p2;

          if(flag){
            p1.Set(lp.GetX(), lp.GetY() + delta);
            p2.Set(rp.GetX(), rp.GetY() + delta);
          }else{
            p1.Set(lp.GetX(), lp.GetY() - delta);
            p2.Set(rp.GetX(), rp.GetY() - delta);
          }

          hs.Set(true,p1,p2);
      }else if(MyAlmostEqual(lp.GetX(), rp.GetX())){
          Point p1, p2;
          if(flag){
            p1.Set(lp.GetX() + delta, lp.GetY());
            p2.Set(rp.GetX() + delta, rp.GetY());
          }else{
            p1.Set(lp.GetX() - delta, lp.GetY());
            p2.Set(rp.GetX() - delta, rp.GetY());
          }
          hs.Set(true,p1,p2);
      }else{

          double k1 = (lp.GetY() - rp.GetY())/(lp.GetX() - rp.GetX());
          double k2 = -1/k1;
          double c1 = lp.GetY() - lp.GetX()*k2;
          double c2 = rp.GetY() - rp.GetX()*k2;

          double x1 , x2;
          x1 = 0.0; x2 = 0.0;
          GetDeviation(lp, k2, c1, x1, x2, delta);
          double y1 = x1*k2 + c1;
          double y2 = x2*k2 + c1;

          double x3, x4;
          x3 = 0.0; x4 = 0.0;
          GetDeviation(rp, k2, c2, x3, x4, delta);
          double y3 = x3*k2 + c2;
          double y4 = x4*k2 + c2;
          Point p1,p2;

//          cout<<"x1 "<<x1<<" x2 "<<x2<<" y1 "<<y1<<" y2 "<<y2<<endl;

          if(flag){
            p1.Set(x1, y1);
            p2.Set(x3, y3);
          }else{
            p1.Set(x2, y2);
            p2.Set(x4, y4);
          }
          hs.Set(true, p1, p2);
      }
//      cout<<"after hs "<<hs<<endl;
}

/*
for the given sline, get its left or right line after transfer
Similar as Gettheboundary()

*/
void SpacePartition::GetSubCurve(SimpleLine* curve, Line* newcurve,
                    int roadwidth, bool clock)
{
      newcurve->StartBulkLoad();
      for(int i = 0; i < curve->Size();i++){
          HalfSegment hs;
          curve->Get(i,hs);
          TransferHalfSegment(hs, roadwidth, clock);
          *newcurve += hs;
      }
      newcurve->EndBulkLoad();
}

/*
build the zebracrossing at the junction position.
The main determined input parameters are endpoints1 and endpoints2.
If the four points can construct a zebra, returns true, otherwise return false

*/
bool SpacePartition::BuildZebraCrossing(vector<MyPoint>& endpoints1,
                          vector<MyPoint>& endpoints2,
                          Region* reg_pave1, Region* reg_pave2,
                          Line* pave1, Region* crossregion, Point& junp)
{
      if(endpoints1.size() > 0 && endpoints2.size() > 0){

         MyPoint lp = endpoints1[0];
         MyPoint rp = endpoints2[0];
         Line* pave = new Line(0);
         pave->StartBulkLoad();
         HalfSegment hs;
         hs.Set(true,lp.loc,rp.loc);
         int edgeno = 0;
         hs.attr.edgeno = edgeno++;
         *pave += hs;
         hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
         *pave += hs;
         pave->EndBulkLoad();

       //////////////extend it to a region/////////////////////////////
         HalfSegment hs1 = hs;
         HalfSegment hs2 = hs;
         TransferHalfSegment(hs1, 2, true);
         TransferHalfSegment(hs2, 2, false);
         vector<Point> outer_ps;
         vector<Region> result;
         Point lp1 = hs1.GetLeftPoint();
         Point rp1 = hs1.GetRightPoint();
         Point lp2 = hs2.GetLeftPoint();
         Point rp2 = hs2.GetRightPoint();
         Point mp1, mp2;
         mp1.Set((lp1.GetX() + rp1.GetX())/2, (lp1.GetY() + rp1.GetY())/2);
         mp2.Set((lp2.GetX() + rp2.GetX())/2, (lp2.GetY() + rp2.GetY())/2);


         if(mp1.Distance(junp) > mp2.Distance(junp)){
             lp1 = hs.GetLeftPoint();
             rp1 = hs.GetRightPoint();
             lp2 = hs1.GetLeftPoint();
             rp2 = hs1.GetRightPoint();

         }else{
             lp1 = hs.GetLeftPoint();
             rp1 = hs.GetRightPoint();
             lp2 = hs2.GetLeftPoint();
             rp2 = hs2.GetRightPoint();

         }

/*         if((lp2.Inside(*reg_pave1) && rp2.Inside(*reg_pave2)) ||
                 (lp2.Inside(*reg_pave2) && rp2.Inside(*reg_pave1)) ){
             if(GetClockwise(lp2, lp1, rp2)){
               outer_ps.push_back(lp2);
               outer_ps.push_back(rp2);
               outer_ps.push_back(rp1);
               outer_ps.push_back(lp1);
             }else{
               outer_ps.push_back(lp1);
               outer_ps.push_back(rp1);
               outer_ps.push_back(rp2);
               outer_ps.push_back(lp2);
             }
            ComputeRegion(outer_ps, result);
            *crossregion = result[0];
            *pave1 = *pave;
            delete pave;

            return true;
          }*/
//        ModifyPoint(lp1);
//        ModifyPoint(rp1);
//        ModifyPoint(lp2);
//        ModifyPoint(rp2);

        if((lp2.Inside(*reg_pave1) && rp2.Inside(*reg_pave2)) ||
                 (lp2.Inside(*reg_pave2) && rp2.Inside(*reg_pave1)) ){
             if(GetClockwise(lp2, lp1, rp2)){
               outer_ps.push_back(lp2);
               outer_ps.push_back(rp2);
               outer_ps.push_back(rp1);
               outer_ps.push_back(lp1);
             }else{
               outer_ps.push_back(lp1);
               outer_ps.push_back(rp1);
               outer_ps.push_back(rp2);
               outer_ps.push_back(lp2);
             }


            for(unsigned int j = 0;j < outer_ps.size();j++){
              vector<Point> outer_ps1;
              int index = (j + 1) % outer_ps.size();
              for(int i = 0 ; i < outer_ps.size() ;i++){
                  Point p = outer_ps[index];
                  outer_ps1.push_back(p);
                  index = (index + 1) % outer_ps.size();
              }
              vector<Region> result1;
              ComputeRegion(outer_ps1, result1);
              if(result1[0].GetCycleDirection()){
                  *crossregion = result1[0];
                  *pave1 = *pave;
                  delete pave;
                  return true;
              }
            }
          }

          delete pave;
       ///////////////////////////////////////////////////////////////
      }
      endpoints1.clear();
      endpoints2.clear();
      return false;
}


/*
for the road line around the junction position, it creates the zebra crossing

*/
void SpacePartition::GetZebraCrossing(SimpleLine* subcurve,
                        Region* reg_pave1, Region* reg_pave2,
                        int roadwidth, Line* pave1, double delta_l,
                        Point p1, Region* crossregion)
{
    vector<MyPoint> endpoints1;
    vector<MyPoint> endpoints2;
    double delta = 1;

    Line* subline1 = new Line(0);
    Line* subline2 = new Line(0);

    Point junp = p1;

    GetSubCurve(subcurve, subline1, roadwidth + 1, true);
    GetSubCurve(subcurve, subline2, roadwidth + 1, false);

//    *pave1 = *subline1;

    Point p2;
    double l;

/*    if(flag)
      l = delta;
    else
      l = subcurve->Length() - delta;*/

    //////////////////////////////////////////////////////
    Point startp, endp;
    assert(subcurve->AtPosition(0.0, true, startp));
    assert(subcurve->AtPosition(subcurve->Length(), true, endp));
    bool flag1;
    if(startp.Distance(p1) < endp.Distance(p1)){
      flag1 = true;
      l = delta;
    }
    else{
      l = subcurve->Length() - delta;
      flag1 = false;
    }

//    cout<<"l "<<l<<endl;
//    cout<<"p1 "<<p1<<" startp "<<startp<<"endp "<<endp<<endl;
    ///////////////////////////////////////////////////////

    bool find = false;
    while(find == false && (0 < l && l < subcurve->Length())){
/*      if(flag)
        l = l + delta;
      else
        l = l - delta;*/

      if(flag1)
        l = l + delta;
      else
        l = l - delta;

      assert(subcurve->AtPosition(l,true,p2));


      if(MyAlmostEqual(p1.GetX(), p2.GetX())){
        double y = p2.GetY();
        double x1 = p2.GetX() - (roadwidth + 2);
        double x2 = p2.GetX() + (roadwidth + 2);
        Line* line1 = new Line(0);
        line1->StartBulkLoad();
        HalfSegment hs;
        Point lp,rp;
        lp.Set(x1, y);
        rp.Set(x2, y);
        hs.Set(true,lp,rp);
        int edgeno = 0;
        hs.attr.edgeno = edgeno++;
        *line1 += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        *line1 += hs;
        line1->EndBulkLoad();


        Points* ps1 = new Points(0);
        Points* ps2 = new Points(0);
        subline1->Crossings(*line1,*ps1);
        subline2->Crossings(*line1,*ps2);

        if(ps1->Size() > 0 && ps2->Size() > 0 &&
             ((ps1->Inside(*reg_pave1) && ps2->Inside(*reg_pave2)) ||
             (ps1->Inside(*reg_pave2) && ps2->Inside(*reg_pave1)))){

//        if(ps1->Size() > 0 && ps2->Size() > 0){
//          cout<<"1 "<<p2<<endl;

          for(int i = 0;i < ps1->Size();i++){
            Point p;
            ps1->Get(i,p);
            MyPoint mp(p,p.Distance(p2));
            endpoints1.push_back(mp);

          }
          for(int i = 0;i < ps2->Size();i++){
            Point p;
            ps2->Get(i,p);
            MyPoint mp(p, p.Distance(p2));
            endpoints2.push_back(mp);
          }

          sort(endpoints1.begin(),endpoints1.end());
          sort(endpoints2.begin(),endpoints2.end());
//          find = true;
          find=BuildZebraCrossing(endpoints1, endpoints2,
                        reg_pave1, reg_pave2, pave1, crossregion, junp);
        }
        p1 = p2;

        delete ps1;
        delete ps2;

        delete line1;

      }else if(MyAlmostEqual(p1.GetY(), p2.GetY())){
            double y1 = p2.GetY() - (roadwidth + 2);
            double y2 = p2.GetY() + (roadwidth + 2);
            double x = p2.GetX();

            Line* line1 = new Line(0);
            line1->StartBulkLoad();
            HalfSegment hs;
            Point lp,rp;
            lp.Set(x, y1);
            rp.Set(x, y2);
            hs.Set(true,lp,rp);
            int edgeno = 0;
            hs.attr.edgeno = edgeno++;
            *line1 += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *line1 += hs;
            line1->EndBulkLoad();


            Points* ps1 = new Points(0);
            Points* ps2 = new Points(0);
            subline1->Crossings(*line1,*ps1);
            subline2->Crossings(*line1,*ps2);

            if(ps1->Size() > 0 && ps2->Size() > 0 &&
             ((ps1->Inside(*reg_pave1) && ps2->Inside(*reg_pave2)) ||
              (ps1->Inside(*reg_pave2) && ps2->Inside(*reg_pave1)))){
//            if(ps1->Size() > 0 && ps2->Size() > 0){

//              cout<<"2 "<<p2<<endl;

            for(int i = 0;i < ps1->Size();i++){
              Point p;
              ps1->Get(i,p);
              MyPoint mp(p,p.Distance(p2));
              endpoints1.push_back(mp);
            }
            for(int i = 0;i < ps2->Size();i++){
              Point p;
              ps2->Get(i,p);
              MyPoint mp(p, p.Distance(p2));
              endpoints2.push_back(mp);
            }

            sort(endpoints1.begin(),endpoints1.end());
            sort(endpoints2.begin(),endpoints2.end());
//            find = true;
            find=BuildZebraCrossing(endpoints1, endpoints2,
                        reg_pave1, reg_pave2, pave1, crossregion, junp);
          }
          p1 = p2;

          delete ps1;
          delete ps2;
          delete line1;
      }else{//not vertical
        double k1 = (p2.GetY() - p1.GetY())/(p2.GetX() - p1.GetX());
        double k2 = -1/k1;
        double c2 = p2.GetY() - p2.GetX()*k2;

        double x1 = p2.GetX() - (roadwidth + 2);
        double x2 = p2.GetX() + (roadwidth + 2);

        Line* line1 = new Line(0);
        line1->StartBulkLoad();
        HalfSegment hs;
        Point lp,rp;
        lp.Set(x1, x1*k2 + c2);
        rp.Set(x2, x2*k2 + c2);
        hs.Set(true,lp,rp);
        int edgeno = 0;
        hs.attr.edgeno = edgeno++;
        *line1 += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        *line1 += hs;
        line1->EndBulkLoad();

        Points* ps1 = new Points(0);
        Points* ps2 = new Points(0);
        subline1->Crossings(*line1,*ps1);
        subline2->Crossings(*line1,*ps2);


        if(ps1->Size() > 0 && ps2->Size() > 0 &&
            ((ps1->Inside(*reg_pave1) && ps2->Inside(*reg_pave2)) ||
             (ps1->Inside(*reg_pave2) && ps2->Inside(*reg_pave1)))){

//        if(ps1->Size() > 0 && ps2->Size() > 0){
//          cout<<"3 "<<p2<<endl;
          for(int i = 0;i < ps1->Size();i++){
            Point p;
            ps1->Get(i,p);
            MyPoint mp(p, p.Distance(p2));
            endpoints1.push_back(mp);

          }
          for(int i = 0;i < ps2->Size();i++){
            Point p;
            ps2->Get(i,p);
            MyPoint mp(p, p.Distance(p2));
            endpoints2.push_back(mp);
          }

          sort(endpoints1.begin(),endpoints1.end());
          sort(endpoints2.begin(),endpoints2.end());
//          find = true;
          find=BuildZebraCrossing(endpoints1, endpoints2,
                        reg_pave1, reg_pave2, pave1, crossregion, junp);
        }

        p1 = p2;

        delete ps1;
        delete ps2;
        delete line1;
      }

    }


    delete subline1;
    delete subline2;

}


/*
Extend the line in decreasing direction

*/
void SpacePartition::Decrease(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave,
                      int roadwidth, Region* crossregion)
{
    double l = len;
    double delta_l = 20;
//    double delta_l = 30;
    Point p1;
    assert(curve->AtPosition(l, true, p1));

    while(1){
      SimpleLine* subcurve = new SimpleLine(0);
      if(l - delta_l > 0.0)
        curve->SubLine(l - delta_l, l, true, *subcurve);
      else
        curve->SubLine(0.0, l, true, *subcurve);

//    subcurve2->toLine(*pave2); ///crossing at junction

//    cout<<"decrease subcurve1 len"<<subcurve->Length()<<endl;

      GetZebraCrossing(subcurve, reg_pave1,
                   reg_pave2, roadwidth, pave, delta_l, p1, crossregion);
      delete subcurve;
      if(pave->Size() > 0 || delta_l >= curve->Length()) break;

      delta_l += delta_l;

    }
}

/*
Extend the line in increasing direction

*/
void SpacePartition::Increase(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave,
                      int roadwidth, Region* crossregion)
{

    double route_length = curve->Length();
    double l = len;
    double delta_l = 20;
//    double delta_l = 30;
    Point p1;
    assert(curve->AtPosition(l, true, p1));

    while(1){
      SimpleLine* subcurve = new SimpleLine(0);
      if(l + delta_l < route_length)
        curve->SubLine(l, l+delta_l, true, *subcurve);
      else
        curve->SubLine(l, route_length, true, *subcurve);

  //    subcurve1->toLine(*pave1);/////////crossing at junction

        GetZebraCrossing(subcurve, reg_pave1,
                reg_pave2, roadwidth, pave, delta_l, p1, crossregion);

      delete subcurve;
      if(pave->Size() > 0 || delta_l >= route_length) break;

      delta_l += delta_l;

    }

}

/*
Create the pavement for each junction position, called by function Junpavement()

*/
void SpacePartition::CreatePavement(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave1,
                      Line* pave2, int roadwidth, Region* crossregion,
                      Region* reg_road)
{
    Region* crossreg1 = new Region(0);
    Region* crossreg2 = new Region(0);
    if(MyAlmostEqual(curve->Length(), len))
      Decrease(curve, reg_pave1, reg_pave2, len, pave2,
               roadwidth, crossreg2);//--
    else if(MyAlmostEqual(len, 0.0))
      Increase(curve, reg_pave1, reg_pave2, len, pave1,
              roadwidth, crossreg1);//++
    else{
      Increase(curve, reg_pave1, reg_pave2, len, pave1,
              roadwidth, crossreg1);
      Decrease(curve, reg_pave1, reg_pave2, len, pave2,
              roadwidth, crossreg2);
    }

//    MyUnion(*crossreg1, *crossreg2, *crossregion);

    ///////////////cut the common area by zc and pave////////////////////
    Region* reg = new Region(0);
    MyUnion(*crossreg1, *crossreg2, *reg);
    MyIntersection(*reg, *reg_road, *crossregion);
    delete reg;
    /////////////////////////////////////////////////////////////////////

    delete crossreg1;
    delete crossreg2;

}

/*
called by operator junregion

*/
void SpacePartition::Junpavement(Network* n, Relation* rel, int attr_pos1,
                  int attr_pos2, int width, Relation* rel_road, int attr_pos3)
{
    //get the pavement for each junction
    Relation* routes = n->GetRoutes();
    vector<double> routes_length;
//    double min_length = numeric_limits<double>::max();
//    double max_length = numeric_limits<double>::min();

    for(int i = 1;i <= routes->GetNoTuples();i++){

      Tuple* route_tuple = routes->GetTuple(i);
      CcReal* len = (CcReal*)route_tuple->GetAttribute(ROUTE_LENGTH);
      double length = len->GetRealval();

//      if(length < min_length) min_length = length;
//      if(length > max_length) max_length = length;

      route_tuple->DeleteIfAllowed();
      routes_length.push_back(length);

    }

    Relation* juns = n->GetJunctions();

    for(int i = 1;i <= n->GetNoJunctions();i++){
      Tuple* jun_tuple = juns->GetTuple(i);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();

/*      if(!(id1 == 822 && id2 == 2406)){
          jun_tuple->DeleteIfAllowed();
          continue;
      }*/

//      cout<<"rid1 "<<id1<<" rid2 "<<id2<<endl;

      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

      double len1 = meas1->GetRealval();
      double len2 = meas2->GetRealval();
//      double delta = 0.05;

//      cout<<"len1 "<<len1<<" len2 "<<len2<<endl;

      Tuple* inborder_tuple1 = rel->GetTuple(id1);
      Tuple* inborder_tuple2 = rel->GetTuple(id2);
      Region* reg1_in = (Region*)inborder_tuple1->GetAttribute(attr_pos1);
      Region* reg1_out = (Region*)inborder_tuple1->GetAttribute(attr_pos2);
      Region* reg2_in = (Region*)inborder_tuple2->GetAttribute(attr_pos1);
      Region* reg2_out = (Region*)inborder_tuple2->GetAttribute(attr_pos2);

      ///////////get the road region/////////////////////////
      Tuple* tuple_road1 = rel_road->GetTuple(id1);
      Region* reg_road1 = (Region*)tuple_road1->GetAttribute(attr_pos3);
//      cout<<*reg_road1<<endl;
      ////////////////////////////////////////////////////////
      Tuple* route_tuple1 = routes->GetTuple(id1);
      SimpleLine* curve1 = (SimpleLine*)route_tuple1->GetAttribute(ROUTE_CURVE);
      int input_w1 = width;


      Line* pave1 = new Line(0);
      Line* pave2 = new Line(0);
      Line* result1 = new Line(0);

      Region* crossregion1 = new Region(0);
      CreatePavement(curve1, reg1_in, reg1_out, len1, pave1,
                     pave2, input_w1, crossregion1,reg_road1);
      pave1->Union(*pave2,*result1);

      junid1.push_back(id1);
      pave_line1.push_back(*result1);
      outer_regions1.push_back(*crossregion1);

      delete crossregion1;
      delete result1;
      delete pave1;
      delete pave2;
      tuple_road1->DeleteIfAllowed();

      int input_w2 = width;

      /////////////get the road region/////////////////////
      Tuple* tuple_road2 = rel_road->GetTuple(id2);
      Region* reg_road2 = (Region*)tuple_road2->GetAttribute(attr_pos3);
//      cout<<*reg_road2<<endl;
      ////////////////////////////////////////////////////
      Tuple* route_tuple2 = routes->GetTuple(id2);
      SimpleLine* curve2 = (SimpleLine*)route_tuple2->GetAttribute(ROUTE_CURVE);


      Line* pave3 = new Line(0);
      Line* pave4 = new Line(0);
      Line* result2 = new Line(0);

      Region* crossregion2 = new Region(0);
      CreatePavement(curve2, reg2_in, reg2_out, len2, pave3,
                     pave4, input_w2, crossregion2,reg_road2);
      pave3->Union(*pave4,*result2);

      junid2.push_back(id2);
      pave_line2.push_back(*result2);
      outer_regions2.push_back(*crossregion2);

      delete crossregion2;
      delete result2;
      delete pave3;
      delete pave4;
      tuple_road2->DeleteIfAllowed();


      route_tuple1->DeleteIfAllowed();
      route_tuple2->DeleteIfAllowed();

      inborder_tuple1->DeleteIfAllowed();
      inborder_tuple2->DeleteIfAllowed();
      jun_tuple->DeleteIfAllowed();

    }

      juns->Delete();
}

/*
Detect whether three points collineation

*/

inline bool SpacePartition::Collineation(Point& p1, Point& p2, Point& p3)
{
      if(MyAlmostEqual(p1.GetX(), p2.GetX())){
          if(MyAlmostEqual(p2.GetX(), p3.GetX())) return true;
      }
      if(MyAlmostEqual(p1.GetY(), p2.GetY())){
          if(MyAlmostEqual(p2.GetY(), p3.GetY())) return true;
      }
      double k1 = (p1.GetY() - p2.GetY())/(p1.GetX() - p2.GetX());
      double k2 = (p2.GetY() - p3.GetY())/(p2.GetX() - p3.GetX());
      if(MyAlmostEqual(k1, k2)) return true;
      return false;
}


/*
Check that the pavement gap should not intersect the two roads

*/
bool SpacePartition::SameSide1(Region* reg1, Region* reg2, Line* r1r2,
                               Point* junp)
{
      vector<MyPoint> mps1;
      vector<MyPoint> mps2;
      for(int i = 0;i < reg1->Size();i++){
          HalfSegment hs;
          reg1->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps1.push_back(mp1);
            mps1.push_back(mp2);
          }
      }
      for(int i = 0;i < reg2->Size();i++){
          HalfSegment hs;
          reg2->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps2.push_back(mp1);
            mps2.push_back(mp2);
          }
      }
      sort(mps1.begin(), mps1.end());
      sort(mps2.begin(), mps2.end());


//      cout<<mps1[0].loc<<mps2[0].loc<<mps1[2].loc<<mps2[2].loc<<endl;

      vector<Point> border1;
      border1.push_back(mps1[0].loc);
      border1.push_back(mps1[2].loc);
      border1.push_back(mps2[2].loc);
      border1.push_back(mps2[0].loc);
      vector<Point> border;
      const double delta_dist = 0.1;
      for(unsigned int i = 0;i < border1.size();i++){
        unsigned int j = 0;
        for(;j < border.size();j++){
          if(border[j].Distance(border1[i]) < delta_dist) break;
        }
        if(j == border.size())
          border.push_back(border1[i]);
      }

//      for(unsigned int i = 0;i < border.size();i++)
//        cout<<border[i]<<endl;


      if(border.size() < 3) return false; //not a region
      if(border.size() == 3){
        unsigned int count = 0;
        unsigned int index = 0;

        while(count < 3){
          HalfSegment hs;
          hs.Set(true, border[index], border[(index + 1)%border.size()]);
          Line* temp =  new Line(0);
          int edgeno = 0;
          hs.attr.edgeno = edgeno++;
          *temp += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *temp += hs;
          if(temp->Intersects(*r1r2)){
            delete temp;
            return false;
          }
          count++;
          index++;
          delete temp;
        }
        //three points collineation
        if(Collineation(border[0], border[1], border[2])) return false;

            vector<Point> ps;
            if(GetClockwise(border[0], border[1], border[2])){
                ps.push_back(border[1]);
                ps.push_back(border[0]);
                ps.push_back(border[2]);
            }else{
                ps.push_back(border[2]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
            }

        //should be counter clock wise
            vector<Region> gap;
            ComputeRegion(ps, gap);
            outer_fillgap.push_back(gap[0]);
            return true;

      }else{ // four points construct a region
          HalfSegment hs;
          hs.Set(true, border[0], border[3]);
          Line* temp =  new Line(0);
          int edgeno = 0;
          hs.attr.edgeno = edgeno++;
          *temp += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *temp += hs;
          if(temp->Intersects(*r1r2)){
            delete temp;
            return false;
          }else{
            delete temp;
            vector<Point> ps;

            if(GetClockwise(border[3], border[0], border[2])){
                ps.push_back(border[0]);
                ps.push_back(border[3]);
                ps.push_back(border[2]);
                ps.push_back(border[1]);
            }else{
                ps.push_back(border[2]);
                ps.push_back(border[3]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
            }

        //should be counter clock wise

            vector<Region> gap;
            ComputeRegion(ps, gap);
            outer_fillgap.push_back(gap[0]);
            return true;
          }
      }
}

/*
build a small region around the three halfsegments
the pavement gap should not intersect the small region

*/
bool SpacePartition::SameSide2(Region* reg1, Region* reg2, Line* r1r2,
                  Point* junp, MyHalfSegment thirdseg, Region& smallreg)
{
      vector<MyPoint> mps1;
      vector<MyPoint> mps2;
      for(int i = 0;i < reg1->Size();i++){
          HalfSegment hs;
          reg1->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps1.push_back(mp1);
            mps1.push_back(mp2);
          }
      }
      for(int i = 0;i < reg2->Size();i++){
          HalfSegment hs;
          reg2->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            MyPoint mp1(lp, lp.Distance(*junp));
            MyPoint mp2(rp, rp.Distance(*junp));
            mps2.push_back(mp1);
            mps2.push_back(mp2);
          }
      }
      sort(mps1.begin(), mps1.end());
      sort(mps2.begin(), mps2.end());

//      cout<<mps1[0].loc<<mps2[0].loc<<mps1[2].loc<<mps2[2].loc<<endl;

      vector<Point> border1;
      border1.push_back(mps1[0].loc);
      border1.push_back(mps1[2].loc);
      border1.push_back(mps2[2].loc);
      border1.push_back(mps2[0].loc);
      vector<Point> border;
      const double delta_dist = 0.1;
      for(unsigned int i = 0;i < border1.size();i++){
        unsigned int j = 0;
        for(;j < border.size();j++){
          if(border[j].Distance(border1[i]) < delta_dist) break;
        }
        if(j == border.size())
          border.push_back(border1[i]);
      }

//      for(unsigned int i = 0;i < border.size();i++)
//        cout<<border[i]<<endl;


      if(border.size() < 3) return false; //not a region
      if(border.size() == 3){
//          cout<<"3"<<endl;
          unsigned int count = 0;
          unsigned int index = 0;

          while(count < 3){
            HalfSegment hs;
            hs.Set(true, border[index], border[(index + 1)%border.size()]);
            Line* temp =  new Line(0);
            int edgeno = 0;
            hs.attr.edgeno = edgeno++;
            *temp += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *temp += hs;

            bool flag = true;
            if(border[index].Inside(smallreg) == false)
              flag = true;
            else
              flag = false;

            if(temp->Intersects(*r1r2) || flag == false){
              delete temp;
              return false;
            }
            count++;
            index++;
            delete temp;
          }
          if(Collineation(border[0], border[1], border[2])) return false;

          vector<Point> ps;
          if(GetClockwise(border[0], border[1], border[2])){
                ps.push_back(border[1]);
                ps.push_back(border[0]);
                ps.push_back(border[2]);
          }else{
                ps.push_back(border[2]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
          }

        //should be counter clock wise
            vector<Region> gap;
            ComputeRegion(ps, gap);
            outer_fillgap.push_back(gap[0]);
            return true;

      }else{ // four points construct a region
//          cout<<"4"<<endl;
          HalfSegment hs;
          hs.Set(true, border[0], border[3]);
          Line* temp =  new Line(0);
          int edgeno = 0;
          hs.attr.edgeno = edgeno++;
          *temp += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *temp += hs;

          bool flag = true;
          if(border[0].Inside(smallreg) == false &&
             border[3].Inside(smallreg) == false)
            flag = true;
          else
            flag = false;

          if(temp->Intersects(*r1r2) || flag == false){
            delete temp;
            return false;
          }else{
            delete temp;
            vector<Point> ps;

            if(GetClockwise(border[3], border[0], border[2])){
                ps.push_back(border[0]);
                ps.push_back(border[3]);
                ps.push_back(border[2]);
                ps.push_back(border[1]);
            }else{
                ps.push_back(border[2]);
                ps.push_back(border[3]);
                ps.push_back(border[0]);
                ps.push_back(border[1]);
            }

        //should be counter clock wise

            vector<Region> gap;
            ComputeRegion(ps, gap);
            outer_fillgap.push_back(gap[0]);
            return true;
          }

      }
}

/*
the common area of two regions

*/
inline bool SpacePartition::PavementIntersection(Region* reg1, Region* reg2)
{
    if(reg1->Intersects(*reg2) == false)
      return false;

    Region* reg = new Region(0);
    MyIntersection(*reg1, *reg2, *reg);
    double regarea = reg->Area();

//    cout<<"intersection area "<<regarea<<endl;
    delete reg;
    if(MyAlmostEqual(regarea,0.0)) return false;
    return true;
}

/*
check the junction position rids.size() != 2 rids.size() != 6

*/

void SpacePartition::NewFillPavementDebug(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      vector<int> rids)
{

    Line* r1r2 = new Line(0);
    int edgeno = 0;
    double delta_dist = 0.1;
    r1r2->StartBulkLoad();

    for(unsigned int i = 0;i < rids.size();i++){
        Tuple* route_tuple = routes->GetTuple(rids[i]);
        SimpleLine* route = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
        for(int j = 0;j < route->Size();j++){
           HalfSegment hs;
           route->Get(j,hs);
            if(hs.IsLeftDomPoint()){
              Point lp = hs.GetLeftPoint();
              Point rp = hs.GetRightPoint();
              if(lp.Distance(*junp) < delta_dist ||
                rp.Distance(*junp) < delta_dist){
                HalfSegment newhs;
                newhs.Set(true,lp,rp);
                newhs.attr.edgeno = edgeno++;
                *r1r2 += newhs;
                newhs.SetLeftDomPoint(!newhs.IsLeftDomPoint());
                *r1r2 += newhs;
              }
            }
        }
        route_tuple->DeleteIfAllowed();
    }
    r1r2->EndBulkLoad();


    Tuple* pave_tuple1 = rel->GetTuple(id1);
    Tuple* pave_tuple2 = rel->GetTuple(id2);
    Region* reg1_pave1 = (Region*)pave_tuple1->GetAttribute(attr_pos1);
    Region* reg1_pave2 = (Region*)pave_tuple1->GetAttribute(attr_pos2);
    Region* reg2_pave1 = (Region*)pave_tuple2->GetAttribute(attr_pos1);
    Region* reg2_pave2 = (Region*)pave_tuple2->GetAttribute(attr_pos2);
    if(reg1_pave1->Intersects(*reg2_pave1) == false){

       junid1.push_back(id1);
       junid2.push_back(id2);
       pave_line1.push_back(*r1r2);

       outer_fillgap1.push_back(*reg1_pave1);
       outer_fillgap2.push_back(*reg2_pave1);

       Region* result1 = new Region(0);

       assert(reg1_pave1->GetCycleDirection());
       assert(reg2_pave1->GetCycleDirection());
       outer_fillgap.push_back(*result1);
       delete result1;

    }
    if(reg1_pave1->Intersects(*reg2_pave2) == false){

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave1);
      outer_fillgap2.push_back(*reg2_pave2);

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result1 = new Region(0);
      outer_fillgap.push_back(*result1);
      delete result1;

    }
    if(reg1_pave2->Intersects(*reg2_pave1) == false ){

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave2);
      outer_fillgap2.push_back(*reg2_pave1);

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());

      Region* result1 = new Region(0);
      outer_fillgap.push_back(*result1);
      delete result1;

    }
    if(reg1_pave2->Intersects(*reg2_pave2) == false){

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave2);
      outer_fillgap2.push_back(*reg2_pave2);

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result1 = new Region(0);
      outer_fillgap.push_back(*result1);
      delete result1;

    }

    pave_tuple1->DeleteIfAllowed();
    pave_tuple2->DeleteIfAllowed();
    delete r1r2;

}


/*
check for the junction where two road intersect
rids.size() == 2, used by operator fillgap

*/

void SpacePartition::NewFillPavement1(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      vector<int> rids)
{

    Line* r1r2 = new Line(0);
    int edgeno = 0;
    double delta_dist = 0.1;
    r1r2->StartBulkLoad();

    for(unsigned int i = 0;i < rids.size();i++){
        Tuple* route_tuple = routes->GetTuple(rids[i]);
        SimpleLine* route = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
        for(int j = 0;j < route->Size();j++){
           HalfSegment hs;
           route->Get(j,hs);
            if(hs.IsLeftDomPoint()){
              Point lp = hs.GetLeftPoint();
              Point rp = hs.GetRightPoint();
              if(lp.Distance(*junp) < delta_dist ||
                rp.Distance(*junp) < delta_dist){
                HalfSegment newhs;
                newhs.Set(true,lp,rp);
                newhs.attr.edgeno = edgeno++;
                *r1r2 += newhs;
                newhs.SetLeftDomPoint(!newhs.IsLeftDomPoint());
                *r1r2 += newhs;
              }
            }
        }
        route_tuple->DeleteIfAllowed();
    }
    r1r2->EndBulkLoad();


    Tuple* pave_tuple1 = rel->GetTuple(id1);
    Tuple* pave_tuple2 = rel->GetTuple(id2);
    Region* reg1_pave1 = (Region*)pave_tuple1->GetAttribute(attr_pos1);
    Region* reg1_pave2 = (Region*)pave_tuple1->GetAttribute(attr_pos2);
    Region* reg2_pave1 = (Region*)pave_tuple2->GetAttribute(attr_pos1);
    Region* reg2_pave2 = (Region*)pave_tuple2->GetAttribute(attr_pos2);


//    if(reg1_pave1->Intersects(*reg2_pave1) == false &&

     if(PavementIntersection(reg1_pave1, reg2_pave1) == false &&
       SameSide1(reg1_pave1, reg2_pave1, r1r2, junp)){

       junid1.push_back(id1);
       junid2.push_back(id2);
       pave_line1.push_back(*r1r2);

       outer_fillgap1.push_back(*reg1_pave1);
       outer_fillgap2.push_back(*reg2_pave1);

      Region* result1 = new Region(0);

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());
//      cout<<outer_fillgap[outer_fillgap.size() - 1].GetCycleDirection()<<endl;

//      cout<<*reg1_pave1<<endl;
//      cout<<outer_fillgap[outer_fillgap.size() - 1]<<endl;


      MyUnion(*reg1_pave1, outer_fillgap[outer_fillgap.size() - 1], *result1);

//      outer_fillgap1.push_back(*result1);
//      outer_fillgap2.push_back(*reg2_pave1);
      delete result1;

    }
//   if(reg1_pave1->Intersects(*reg2_pave2) == false &&
     if(PavementIntersection(reg1_pave1, reg2_pave2) == false &&
       SameSide1(reg1_pave1, reg2_pave2, r1r2, junp)){

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave1);
      outer_fillgap2.push_back(*reg2_pave2);

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result1 = new Region(0);
      MyUnion(*reg1_pave1, outer_fillgap[outer_fillgap.size() - 1], *result1);

//      outer_fillgap1.push_back(*result1);
//      outer_fillgap2.push_back(*reg2_pave1);
      delete result1;

    }
//    if(reg1_pave2->Intersects(*reg2_pave1) == false &&
      if(PavementIntersection(reg1_pave2, reg2_pave1) == false &&
       SameSide1(reg1_pave2, reg2_pave1, r1r2, junp)){

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave2);
      outer_fillgap2.push_back(*reg2_pave1);

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());

      Region* result1 = new Region(0);
      MyUnion(*reg1_pave2, outer_fillgap[outer_fillgap.size() - 1], *result1);

//      outer_fillgap1.push_back(*result1);
//      outer_fillgap2.push_back(*reg2_pave1);
      delete result1;

    }
//    if(reg1_pave2->Intersects(*reg2_pave2) == false &&
      if(PavementIntersection(reg1_pave2, reg2_pave2) == false &&
       SameSide1(reg1_pave2, reg2_pave2, r1r2, junp)){

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave2);
      outer_fillgap2.push_back(*reg2_pave2);

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result1 = new Region(0);
      MyUnion(*reg1_pave2, outer_fillgap[outer_fillgap.size() - 1], *result1);

//    outer_fillgap1.push_back(*result1);
//    outer_fillgap2.push_back(*reg2_pave1);
      delete result1;

    }

    pave_tuple1->DeleteIfAllowed();
    pave_tuple2->DeleteIfAllowed();
    delete r1r2;

}



/*
check for the junction where three roads intersect
called by operator fillgap

*/

void SpacePartition:: NewFillPavement2(Relation* rel, Relation* routes,
                      int id1, int id2,
                      Point* junp, int attr_pos1, int attr_pos2,
                      vector<int> rids)
{

    Line* r1r2 = new Line(0);
    int edgeno = 0;
    double delta_dist = 0.1;
    r1r2->StartBulkLoad();
    MyHalfSegment firstseg;
    MyHalfSegment secondseg;
    MyHalfSegment thirdseg;

    int third_seg = 0;
    for(unsigned int i = 0;i < rids.size();i++){
        Tuple* route_tuple = routes->GetTuple(rids[i]);
        SimpleLine* route = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
        for(int j = 0;j < route->Size();j++){
           HalfSegment hs;
           route->Get(j,hs);
            if(hs.IsLeftDomPoint()){
              Point lp = hs.GetLeftPoint();
              Point rp = hs.GetRightPoint();
              if(lp.Distance(*junp) < delta_dist ||
                rp.Distance(*junp) < delta_dist){
                HalfSegment newhs;
                newhs.Set(true,lp,rp);
                newhs.attr.edgeno = edgeno++;
                *r1r2 += newhs;
                newhs.SetLeftDomPoint(!newhs.IsLeftDomPoint());
                *r1r2 += newhs;
                if(rids[i] != id1 && rids[i] != id2){
                  third_seg++;
                  if(lp.Distance(*junp) < delta_dist){
                    thirdseg.def = true;
                    thirdseg.from = lp;
                    thirdseg.to = rp;

                  }else{
                    thirdseg.def = true;
                    thirdseg.from = rp;
                    thirdseg.to = lp;

                  }
                }
                if(rids[i] == id1){
                  if(lp.Distance(*junp) < delta_dist){
                    firstseg.def = true;
                    firstseg.from = lp;
                    firstseg.to = rp;
                  }else{
                    firstseg.def = true;
                    firstseg.from = rp;
                    firstseg.to = lp;
                  }
                }
                if(rids[i] == id2){
                  if(lp.Distance(*junp) < delta_dist){
                    secondseg.def = true;
                    secondseg.from = lp;
                    secondseg.to = rp;
                  }else{
                    secondseg.def = true;
                    secondseg.from = rp;
                    secondseg.to = lp;
                  }
                }
              }
            }
        }
        route_tuple->DeleteIfAllowed();
    }
    r1r2->EndBulkLoad();
    if(third_seg > 2){
        delete r1r2;
        return;
    }

    ////////////////////////////////
    double angle1 = GetAngle(firstseg.from, firstseg.to, thirdseg.to);
    double angle2 = GetAngle(firstseg.from, firstseg.to, secondseg.to);
    bool clock1 = GetClockwise(firstseg.from, firstseg.to, thirdseg.to);
    bool clock2 = GetClockwise(firstseg.from, firstseg.to, secondseg.to);
    if(clock2){
      if(clock1){
        if(angle1 > angle2){
          delete r1r2;
          return;
        }
      }else{
        delete r1r2;
        return;
      }
    }else{
      if(clock1 == false){
        if(angle1 > angle2){
          delete r1r2;
          return;
        }
      }else{
        delete r1r2;
        return;
      }
    }
    //////////////////////////////
    vector<Point> ps;
    vector<Region> smallreg;
    ps.push_back(firstseg.from);
    ps.push_back(firstseg.to);
    ps.push_back(thirdseg.to);
    ps.push_back(secondseg.to);
    ComputeRegion(ps, smallreg);
    //////////////////////////////

    Tuple* pave_tuple1 = rel->GetTuple(id1);
    Tuple* pave_tuple2 = rel->GetTuple(id2);
    Region* reg1_pave1 = (Region*)pave_tuple1->GetAttribute(attr_pos1);
    Region* reg1_pave2 = (Region*)pave_tuple1->GetAttribute(attr_pos2);
    Region* reg2_pave1 = (Region*)pave_tuple2->GetAttribute(attr_pos1);
    Region* reg2_pave2 = (Region*)pave_tuple2->GetAttribute(attr_pos2);

//    if(reg1_pave1->Intersects(*reg2_pave1) == false &&
     if(PavementIntersection(reg1_pave1, reg2_pave2) == false &&
       SameSide2(reg1_pave1, reg2_pave1, r1r2, junp, thirdseg, smallreg[0])){
/*       if(third_seg > 2){
        cout<<"1"<<endl;
        cout<<"id1 "<<id1<<"id2 "<<id2<<endl;
       } */
       junid1.push_back(id1);
       junid2.push_back(id2);
       pave_line1.push_back(*r1r2);

       outer_fillgap1.push_back(*reg1_pave1);
       outer_fillgap2.push_back(*reg2_pave1);

      Region* result1 = new Region(0);

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());


      MyUnion(*reg1_pave1, outer_fillgap[outer_fillgap.size() - 1], *result1);

      delete result1;

    }

//    if(reg1_pave1->Intersects(*reg2_pave2) == false &&
      if(PavementIntersection(reg1_pave1, reg2_pave2) == false &&
       SameSide2(reg1_pave1, reg2_pave2, r1r2, junp, thirdseg, smallreg[0])){
//       if(third_seg > 2){
//         cout<<"2"<<endl;
//         cout<<"id1 "<<id1<<"id2 "<<id2<<endl;
//       }

      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave1);
      outer_fillgap2.push_back(*reg2_pave2);

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result1 = new Region(0);
      MyUnion(*reg1_pave1, outer_fillgap[outer_fillgap.size() - 1], *result1);

      delete result1;

    }

//    if(reg1_pave2->Intersects(*reg2_pave1) == false &&
    if(PavementIntersection(reg1_pave2, reg2_pave1) == false &&
       SameSide2(reg1_pave2, reg2_pave1, r1r2, junp, thirdseg, smallreg[0])){

//       if(third_seg > 2){
//         cout<<"3"<<endl;
//         cout<<"id1 "<<id1<<"id2 "<<id2<<endl;
//       }
      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave2);
      outer_fillgap2.push_back(*reg2_pave1);

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());

      Region* result1 = new Region(0);
      MyUnion(*reg1_pave2, outer_fillgap[outer_fillgap.size() - 1], *result1);
      delete result1;

    }

//    if(reg1_pave2->Intersects(*reg2_pave2) == false &&
     if(PavementIntersection(reg1_pave2, reg2_pave2) == false &&
       SameSide2(reg1_pave2, reg2_pave2, r1r2, junp, thirdseg, smallreg[0])){

//       if(third_seg > 2){
//         cout<<"4"<<endl;
//         cout<<"id1 "<<id1<<"id2 "<<id2<<endl;
//       }
      junid1.push_back(id1);
      junid2.push_back(id2);
      pave_line1.push_back(*r1r2);
      outer_fillgap1.push_back(*reg1_pave2);
      outer_fillgap2.push_back(*reg2_pave2);

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result1 = new Region(0);
      MyUnion(*reg1_pave2, outer_fillgap[outer_fillgap.size() - 1], *result1);
      delete result1;

    }

    pave_tuple1->DeleteIfAllowed();
    pave_tuple2->DeleteIfAllowed();
    delete r1r2;

}

/*
the same function as in NewFillPavement2, but with different input parameters
called by function FillPave()

*/

void SpacePartition::NewFillPavement3(Relation* routes, int id1, int id2,
                      Point* junp, vector<Region>& paves1,
                      vector<Region>& paves2, vector<int> rids,
                      vector<Region>& newpaves1, vector<Region>& newpaves2)
{

    Line* r1r2 = new Line(0);
    int edgeno = 0;
    double delta_dist = 0.1;
    r1r2->StartBulkLoad();

    for(unsigned int i = 0;i < rids.size();i++){
        Tuple* route_tuple = routes->GetTuple(rids[i]);
        SimpleLine* route = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
        for(int j = 0;j < route->Size();j++){
           HalfSegment hs;
           route->Get(j,hs);
            if(hs.IsLeftDomPoint()){
              Point lp = hs.GetLeftPoint();
              Point rp = hs.GetRightPoint();
              if(lp.Distance(*junp) < delta_dist ||
                rp.Distance(*junp) < delta_dist){
                HalfSegment newhs;
                newhs.Set(true,lp,rp);
                newhs.attr.edgeno = edgeno++;
                *r1r2 += newhs;
                newhs.SetLeftDomPoint(!newhs.IsLeftDomPoint());
                *r1r2 += newhs;
              }
            }
        }
        route_tuple->DeleteIfAllowed();
    }
    r1r2->EndBulkLoad();


    Region* reg1_pave1 = &paves1[id1 - 1];
    Region* reg1_pave2 = &paves2[id1 - 1];
    Region* reg2_pave1 = &paves1[id2 - 1];
    Region* reg2_pave2 = &paves2[id2 - 1];
//    if(reg1_pave1->Intersects(*reg2_pave1) == false &&
      if(PavementIntersection(reg1_pave1, reg2_pave1) == false &&
       SameSide1(reg1_pave1, reg2_pave1, r1r2, junp)){

      Region* result = new Region(0);

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());

      MyUnion(newpaves1[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves1[id1 - 1] = *result;
      delete result;

    }
//    if(reg1_pave1->Intersects(*reg2_pave2) == false &&
      if(PavementIntersection(reg1_pave1, reg2_pave2) == false &&
       SameSide1(reg1_pave1, reg2_pave2, r1r2, junp)){

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result = new Region(0);
      MyUnion(newpaves1[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves1[id1 - 1] = *result;
      delete result;

    }
//    if(reg1_pave2->Intersects(*reg2_pave1) == false &&
      if(PavementIntersection(reg1_pave2, reg2_pave1) == false &&
       SameSide1(reg1_pave2, reg2_pave1, r1r2, junp)){

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());

      Region* result = new Region(0);
      MyUnion(newpaves2[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves2[id1 - 1] = *result;
      delete result;

    }
//    if(reg1_pave2->Intersects(*reg2_pave2) == false &&
      if(PavementIntersection(reg1_pave2, reg2_pave2) == false &&
       SameSide1(reg1_pave2, reg2_pave2, r1r2, junp)){

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result = new Region(0);
      MyUnion(newpaves2[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves2[id1 - 1] = *result;
      delete result;

    }

    delete r1r2;

}



/*
the same function as NewFillPavement2, but with different input parameters
called by function FillPave()

*/

void SpacePartition::NewFillPavement4(Relation* routes, int id1, int id2,
                      Point* junp, vector<Region>& paves1,
                      vector<Region>& paves2, vector<int> rids,
                      vector<Region>& newpaves1, vector<Region>& newpaves2)
{

    Line* r1r2 = new Line(0);
    int edgeno = 0;
    double delta_dist = 0.1;
    r1r2->StartBulkLoad();
    MyHalfSegment firstseg;
    MyHalfSegment secondseg;
    MyHalfSegment thirdseg;

    int third_seg = 0;
    for(unsigned int i = 0;i < rids.size();i++){
        Tuple* route_tuple = routes->GetTuple(rids[i]);
        SimpleLine* route = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
        for(int j = 0;j < route->Size();j++){
           HalfSegment hs;
           route->Get(j,hs);
            if(hs.IsLeftDomPoint()){
              Point lp = hs.GetLeftPoint();
              Point rp = hs.GetRightPoint();
              if(lp.Distance(*junp) < delta_dist ||
                rp.Distance(*junp) < delta_dist){
                HalfSegment newhs;
                newhs.Set(true,lp,rp);
                newhs.attr.edgeno = edgeno++;
                *r1r2 += newhs;
                newhs.SetLeftDomPoint(!newhs.IsLeftDomPoint());
                *r1r2 += newhs;
                if(rids[i] != id1 && rids[i] != id2){
                  third_seg++;
                  if(lp.Distance(*junp) < delta_dist){
                    thirdseg.def = true;
                    thirdseg.from = lp;
                    thirdseg.to = rp;
                  }else{
                    thirdseg.def = true;
                    thirdseg.from = rp;
                    thirdseg.to = lp;
                  }
                }
                if(rids[i] == id1){
                  if(lp.Distance(*junp) < delta_dist){
                    firstseg.def = true;
                    firstseg.from = lp;
                    firstseg.to = rp;
                  }else{
                    firstseg.def = true;
                    firstseg.from = rp;
                    firstseg.to = lp;
                  }
                }
                if(rids[i] == id2){
                  if(lp.Distance(*junp) < delta_dist){
                    secondseg.def = true;
                    secondseg.from = lp;
                    secondseg.to = rp;
                  }else{
                    secondseg.def = true;
                    secondseg.from = rp;
                    secondseg.to = lp;
                  }
                }
              }
            }
        }
        route_tuple->DeleteIfAllowed();
    }
    r1r2->EndBulkLoad();
    if(third_seg > 2){
        delete r1r2;
        return;
    }

    ////////////////////////////////
    double angle1 = GetAngle(firstseg.from, firstseg.to, thirdseg.to);
    double angle2 = GetAngle(firstseg.from, firstseg.to, secondseg.to);
    bool clock1 = GetClockwise(firstseg.from, firstseg.to, thirdseg.to);
    bool clock2 = GetClockwise(firstseg.from, firstseg.to, secondseg.to);
    if(clock2){
      if(clock1){
        if(angle1 > angle2){
          delete r1r2;
          return;
        }
      }else{
        delete r1r2;
        return;
      }
    }else{
      if(clock1 == false){
        if(angle1 > angle2){
          delete r1r2;
          return;
        }
      }else{
        delete r1r2;
        return;
      }
    }
    //////////////////////////////
    vector<Point> ps;
    vector<Region> smallreg;
    ps.push_back(firstseg.from);
    ps.push_back(firstseg.to);
    ps.push_back(thirdseg.to);
    ps.push_back(secondseg.to);
    ComputeRegion(ps, smallreg);
    //////////////////////////////


    Region* reg1_pave1 = &paves1[id1 - 1];
    Region* reg1_pave2 = &paves2[id1 - 1];
    Region* reg2_pave1 = &paves1[id2 - 1];
    Region* reg2_pave2 = &paves2[id2 - 1];

//    if(reg1_pave1->Intersects(*reg2_pave1) == false &&
      if(PavementIntersection(reg1_pave1, reg2_pave2) == false &&
       SameSide2(reg1_pave1, reg2_pave1, r1r2, junp, thirdseg, smallreg[0])){

        Region* result = new Region(0);
        assert(reg1_pave1->GetCycleDirection());
        assert(reg2_pave1->GetCycleDirection());

        MyUnion(newpaves1[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
                *result);
        newpaves1[id1 - 1] = *result;
        delete result;

    }
//    if(reg1_pave1->Intersects(*reg2_pave2) == false &&
      if(PavementIntersection(reg1_pave1, reg2_pave2) == false &&
       SameSide2(reg1_pave1, reg2_pave2, r1r2, junp, thirdseg, smallreg[0])){

      assert(reg1_pave1->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result = new Region(0);
      MyUnion(newpaves1[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves1[id1 - 1] = *result;
      delete result;

    }
//    if(reg1_pave2->Intersects(*reg2_pave1) == false &&
      if(PavementIntersection(reg1_pave2, reg2_pave1) == false &&
       SameSide2(reg1_pave2, reg2_pave1, r1r2, junp, thirdseg, smallreg[0])){

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave1->GetCycleDirection());

      Region* result = new Region(0);
      MyUnion(newpaves2[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves2[id1 - 1] = *result;
      delete result;

    }
//    if(reg1_pave2->Intersects(*reg2_pave2) == false &&
     if(PavementIntersection(reg1_pave2, reg2_pave2) == false &&
       SameSide2(reg1_pave2, reg2_pave2, r1r2, junp, thirdseg, smallreg[0])){

      assert(reg1_pave2->GetCycleDirection());
      assert(reg2_pave2->GetCycleDirection());

      Region* result = new Region(0);
      MyUnion(newpaves2[id1 - 1], outer_fillgap[outer_fillgap.size() - 1],
              *result);
      newpaves2[id1 - 1] = *result;
      delete result;

    }

    delete r1r2;

}

/*
for operator fillgap

*/

void SpacePartition::FillHoleOfPave(Network* n, Relation* rel,  int attr_pos1,
                      int attr_pos2, int width)
{

    Relation* routes = n->GetRoutes();
    Relation* juns = n->GetJunctions();
    vector<double> routes_length;
    double min_length = numeric_limits<double>::max();
    double max_length = numeric_limits<double>::min();


    for(int i = 1;i <= routes->GetNoTuples();i++){
      Tuple* route_tuple = routes->GetTuple(i);
      CcReal* len = (CcReal*)route_tuple->GetAttribute(ROUTE_LENGTH);
      double length = len->GetRealval();
      if(length < min_length) min_length = length;
      if(length > max_length) max_length = length;

      route_tuple->DeleteIfAllowed();
      routes_length.push_back(length);
    }

    vector<MyJun> myjuns;
    for(int i = 1;i <= n->GetNoJunctions();i++){
      Tuple* jun_tuple = juns->GetTuple(i);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();

/*      if(!(id2 == 530)){
        jun_tuple->DeleteIfAllowed();
        continue;
      }*/


      Point* junp = (Point*)jun_tuple->GetAttribute(JUNCTION_POS);
//      cout<<"rid1 "<<id1<<" rid2 "<<id2<<endl;

      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

      double len1 = meas1->GetRealval();
      double len2 = meas2->GetRealval();

        MyJun mj(*junp, id1, id2, len1, len2);
        myjuns.push_back(mj);

      jun_tuple->DeleteIfAllowed();
    }
    juns->Delete();

    sort(myjuns.begin(), myjuns.end());

//    for(unsigned int i = 0;i < myjuns.size();i++)
//      myjuns[i].Print();


    const double delta_dist = 0.1;
    for(unsigned int i = 0 ;i < myjuns.size();i++){
        Point loc = myjuns[i].loc;
        int rid1 = myjuns[i].rid1;
        int rid2 = myjuns[i].rid2;

//        cout<<"rid1 "<<rid1<<" rid2 "<<rid2<<endl;

//      if(!(rid1 == 340 && rid2 == 2728))continue;

      if((MyAlmostEqual(myjuns[i].len1, 0.0)||
          MyAlmostEqual(myjuns[i].len1, routes_length[rid1 - 1])) &&
          (MyAlmostEqual(myjuns[i].len2, 0.0)||
          MyAlmostEqual(myjuns[i].len2, routes_length[rid2 - 1]))){

          vector<int> rids;

        int j = i - 1;
        while(j >= 0 && loc.Distance(myjuns[j].loc) < delta_dist){
          rids.push_back(myjuns[j].rid1);
          rids.push_back(myjuns[j].rid2);
          j--;
        }

        j = i;
        while(j < myjuns.size() && loc.Distance(myjuns[j].loc) < delta_dist){
          rids.push_back(myjuns[j].rid1);
          rids.push_back(myjuns[j].rid2);
          j++;
        }

//        cout<<" rids size "<<rids.size()<<endl;
//        cout<<"rid1 "<<rid1<<" rid2 "<<rid2<<endl;

        if(rids.size() == 2){
          NewFillPavement1(rel, routes, rid1, rid2, &loc,
                        attr_pos1, attr_pos2, rids);
        }

        if(rids.size() == 6){

          NewFillPavement2(rel, routes, rid1, rid2, &loc,
                        attr_pos1, attr_pos2, rids);

        }

        if(rids.size() != 2 && rids.size() != 6 && rids.size() != 12){

            NewFillPavementDebug(rel, routes, rid1, rid2, &loc,
                        attr_pos1, attr_pos2, rids);
        }

        rids.clear();
      }
    }
}
