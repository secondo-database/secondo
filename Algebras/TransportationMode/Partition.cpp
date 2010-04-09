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


