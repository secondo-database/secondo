/*
----
This file is part of SECONDO.

Copyright (C) 2007, 
Faculty of Mathematics and Computer Science,
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

//[_] [\_]
//[toc] [\setcounter{page}{1} \renewcommand{\thepage}{\Roman{page}} 
         \tableofcontents]
//[etoc][\clearpage \setcounter{page}{1} \renewcommand{\thepage}{\arabic{page}}]

//[title] [ \thispagestyle{empty} \title{TopOps-Algebra} \author{Thomas Behr} \maketitle]
//[times] [\ensuremath{\times}]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[<=] [\ensuremath{\leq{}}]
//[>=] [\ensuremath{\ge{}}]


[title]
[toc]
[etoc]

2 General Description

This algebra connects the TopRelAlgebra and the SpatialAlgebra.
It implements functions computing the topological 
relationsships of spatial values. Furthermore it provides some
functions checking whether two objects are part of a cluster which
is given by a name together with a predicategroup. Basically, this
can also be implemented by computing the topological relationship and
check whether the result is contained in the given cluster. The advantage of a
separate implementation is that we can exit the computation early in 
many cases. 



1 Includes, Constants, and Definitions 

*/

#include <iostream>
#include <sstream>
#include <queue>
#include <iterator>

#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "LogMsg.h"
#include "AvlTree.h"


#define TOPOPS_USE_STATISTIC

#ifdef TOPOPS_USE_STATISTIC
// used for statistical information
#include "FTextAlgebra.h"
#endif


#include "SpatialAlgebra.h"
#include "TopRel.h"
#include "StandardTypes.h"


/*
~A Macro useful for debugging ~

*/

//#define __TRACE__ cout << __FILE__ << "@" << __LINE__ << endl;
#define __TRACE__



extern NestedList* nl;
extern QueryProcessor* qp;


using namespace toprel;


const int LEFT      = 1;
const int RIGHT     = 2;
const int COMMON = 4;


/*
2 Definition of ~ownertype~

This enumeration is used to store the source of an AVLSegment.

*/

enum ownertype{none, first, second, both};

ostream& operator<<(ostream& o, const ownertype& owner){
   switch(owner){
      case none   : o << "none" ; break;
      case first  : o << "first"; break;
      case second : o << "second"; break;
      case both   : o << "both"; break;
      default     : assert(false);
   }
   return o;
}


/*
3 The Class ~AVLSegment~

This class is used for inserting into an avl tree during a plane sweep.


*/

class AVLSegment; 
ostream& operator<<(ostream& o, const AVLSegment& s);


class AVLSegment{

public:

/*
3.1 Contructors

~Standard Constructor~

*/
  AVLSegment(){
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

  AVLSegment(const HalfSegment* hs, ownertype owner){
     x1 = hs->GetLeftPoint().GetX();
     y1 = hs->GetLeftPoint().GetY();
     x2 = hs->GetRightPoint().GetX();
     y2 = hs->GetRightPoint().GetY();
     if( (AlmostEqual(x1,x2) && (y2<y2) ) || (x2<x2) ){// swap the entries
        double tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
     }
     this->owner = owner;
     switch(owner){
        case first: { insideAbove_first = hs->GetAttr().insideAbove; 
                      insideAbove_second = false;
                      break;
                     }
        case second: {
                      insideAbove_second = hs->GetAttr().insideAbove; 
                      insideAbove_first = false;
                       break;
                     }
        default: assert(false);
     }
     con_below = 0;
     con_above = 0;
  }

/*
~Constructor~

Create a Segment only consisting of a single point.

*/

  AVLSegment(const Point* p, ownertype owner){
      x1 = p->GetX();
      x2 = x1;
      y1 = p->GetY();
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
   AVLSegment(const AVLSegment& src){
      Equalize(src);
   }


/*
3.2 ~Destructor~

*/
   ~AVLSegment() {}


/*
3.3  operators

*/

  AVLSegment& operator=(const AVLSegment& src){
    Equalize(src);
    return *this;
  }

  bool operator==(const AVLSegment& s) const{
    return compareTo(s)==0;
  }

  bool operator<(const AVLSegment& s) const{
     return compareTo(s)<0;
  }

  bool operator>(const AVLSegment& s) const{
     return compareTo(s)>0;
  }

/*
3.3 Further needful Function

~Print~

This function writes this segment to __out__.

*/
  void Print(ostream& out)const{
    out << "Segment("<<x1<<", " << y1 << ") -> (" << x2 << ", " << y2 <<") " 
        << owner << " [ " << insideAbove_first << ", " 
        << insideAbove_second << "] con("
        << con_below << ", " << con_above << ")";

  }
  
/*

~Equalize~

The value of this segment is taken from the argument.

*/

  void Equalize( const AVLSegment& src){
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
interior.  

*/
 bool crosses(const AVLSegment& s) const{
   double x,y;
   return crosses(s,x,y);
 }

/*
~crosses~

This function checks whether the interiors of the related 
segments are crossing. If this function returns true,
the parameters ~x~ and ~y~ are set to the intersection point.

*/
 bool crosses(const AVLSegment& s,double& x, double& y) const{
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
        return !AlmostEqual(y1,y) && !AlmostEqual(y2,y) &&
               (y>y1)  && (y<y2)
               && !AlmostEqual(s.x1,x) && !AlmostEqual(s.x2,x) ;
    }

    if(s.isVertical()){
       x = s.x1;
       y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
       return !AlmostEqual(y,s.y1) && !AlmostEqual(y,s.y2) && 
              (y>s.y1) && (y<s.y2) && 
              !AlmostEqual(x1,x) && !AlmostEqual(x2,x);
    }
    
    // both segments are non vertical 
    double m1 = (y2-y1)/(x2-x1);
    double m2 = (s.y2-s.y1)/(s.x2-s.x1);
    double c1 = y1 - m1*x1;
    double c2 = s.y1 - m2*s.x1;
    double xs = (c2-c1) / (m1-m2);  // x coordinate of the intersection point
     
    x = xs;
    y = y1 + ((x-x1)/(x2-x1))*(y2-y1);

    return !AlmostEqual(x1,xs) && !AlmostEqual(x2,xs) && // not an endpoint   
           !AlmostEqual(s.x1,xs) && !AlmostEqual(s.x2,xs) && // of any segment
           (x1<xs) && (xs<x2) && (s.x1<xs) && (xs<s.x2);
}

/*
~extends~

This function returns true, iff this segment is an extension of 
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool extends(AVLSegment s){
     return pointEqual(x1,y1,s.x2,s.y2) &&
            compareSlopes(s)==0;
  }

/*
~exactEqualsTo~

This function checks if s has the same geometry like this segment, i.e.
if both endpoints are equal.  
 
*/
bool exactEqualsTo(const AVLSegment& s)const{
  return pointEqual(x1,y1,s.x1,s.y1) &&
         pointEqual(x2,y2,s.x2,s.y2);
}

/*
~isVertical~

Checks whether this segment is vertical.

*/

 bool isVertical() const{
     return AlmostEqual(x1,x2);
 }

/*
~isPoint~

Checks if this segment consists only of a single point.

*/
  bool isPoint() const{
     return AlmostEqual(x1,x2) && AlmostEqual(y1,y2);
  }


/*
~InnerDisjoint~

This function checks whether this segment and s have at most a 
common endpoint. 

*/

  bool innerDisjoint(const AVLSegment& s)const{
      

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
    
      if(isVertical()){
        double x = x1; // compute y for s
        double y =  s.y1 + ((x-s.x1)/(s.x2-s.x1))*(s.y2 - s.y1);
        return ! ( (contains(x,y) && s.ininterior(x,y) )  ||
                   (ininterior(x,y) && s.contains(x,y)) );

      }
      if(s.isVertical()){
         double x = s.x1;
         double y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
         return ! ( (contains(x,y) && s.ininterior(x,y) )  ||
                    (ininterior(x,y) && s.contains(x,y)) );
      }



    
      // both segments are non vertical 
      double m1 = (y2-y1)/(x2-x1);
      double m2 = (s.y2-s.y1)/(s.x2-s.x1);
      double c1 = y1 - m1*x1;
      double c2 = s.y1 - m2*s.x1;

      double x = (c2-c1) / (m1-m2);  // x coordinate of the intersection point
      double y = y1 + ((x-x1)/(x2-x1))*(y2-y1);

      
      return ! ( (contains(x,y) && s.ininterior(x,y) )  ||
                 (ininterior(x,y) && s.contains(x,y)) );
  }
/*
~Intersects~

This function checks whether this segment and s have at least a 
common point. 

*/

  bool intersects(const AVLSegment& s)const{
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

Checks whether both this segment and __s__ have a common segment.

*/
   bool overlaps(const AVLSegment& s) const{
      if(isPoint() || s.isPoint()){
         return false;
      }

      if(compareSlopes(s)!=0){
          return false;
      } 
      // one segment is a extension of the other one
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
   bool ininterior(const double x,const  double y)const{
     if(isPoint()){
       return false;
     }

     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){ // an endpoint
        return false;
     }
     // check if (x,y) is located on the line 
     double res1 = (x-x1)*(y2-y1);
     double res2 = (y-y1)*(x2-x1);
     if(!AlmostEqual(res1,res2)){ // (x,y) not on the straight line 
         return false;
     }
     
     if(AlmostEqual(x1,x2)){ // vertical segment
        return (y>y1) && (y < y2);
     } else {
        return (x>x1) && (x<x2);
     }
   }


/*
~contains~

Checks whether the point defined by (x,y) is located anywhere on this
segment.

*/
   bool contains(const double x,const  double y)const{
     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){
        return true;
     }
     if(isPoint()){
       return false;
     }
     // check if (x,y) is located on the line 
     double res1 = (x-x1)*(y2-y1);
     double res2 = (y-y1)*(x2-x1);
     if(!AlmostEqual(res1,res2)){
         return false;
     }
     
     if(AlmostEqual(x1,x2)){ // vertical segment
        return (y>=y1) && (y <= y2);
     } else {
        return (x>=x1) && (x<=x2);
     }
   }

/*
3.6 Comparison

Compares this with s. The x intervals must overlap.

*/

 int compareTo(const AVLSegment& s) const{
 
    assert(xOverlaps(s));

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
3.7 Some ~Get~ Funtions

~getInsideAbove~

Returns the insideAbove value for such segments for which this value is unique,
e.g. for segments having owner __first__ or __second__.

*/
  bool getInsideAbove(){
      switch(owner){
        case first : return insideAbove_first;
        case second: return insideAbove_second;
        default : assert(false);
      }
  }

  double getX1() const { return x1; }

  double getX2() const { return x2; }

  double getY1() const { return y1; }

  double getY2() const { return y2; }

  ownertype getOwner() const { return owner; }

  bool getInsideAbove_first() const { return insideAbove_first; }
  bool getInsideAbove_second() const { return insideAbove_second; }


/*
3.8 Split functions

~split~

This function splits two overlapping segments.
Preconditions:
1) this segments and ~s~ must overlap.

2) the owner of this and s must be first and second or vice versa

*/

  int split(const AVLSegment& s, AVLSegment& left, AVLSegment& common, 
            AVLSegment& right) const{


     assert(overlaps(s));
     assert( (this->owner==first && s.owner==second) ||
             (this->owner==second && s.owner==first));

     if(pointEqual(x1,y1,s.x1,s.y1)){
       // common left point, the left part will be empty
       if(pointEqual(x2,y2,s.x2,s.y2)){
          // segments are equal , only a common segment exist
          common.x1 = x1;
          common.x2 = x2;
          common.y1 = y1;
          common.y2 = y2;
          common.owner = both;
          if(this->owner==first){
            common.insideAbove_first  = insideAbove_first;
            common.insideAbove_second = s.insideAbove_second;
          } else {
            common.insideAbove_first = s.insideAbove_first;
            common.insideAbove_second = insideAbove_second;
          }
          common.con_above = con_above;
          common.con_below = con_below;
          return COMMON;
       } else { // different end point 
          common.x1 = x1;
          common.y1 = y1;
          if(pointSmaller(x2,y2,s.x2,s.y2)){
            common.x2 = x2;
            common.y2 = y2;
          } else {
            common.x2 = s.x2;
            common.y2 = s.y2;
          }

          common.owner = both;
          if(this->owner==first){
            common.insideAbove_first  = insideAbove_first;
            common.insideAbove_second = s.insideAbove_second;
          } else {
            common.insideAbove_first = s.insideAbove_first;
            common.insideAbove_second = insideAbove_second;
          }
          common.con_above = con_above;
          common.con_below = con_below;
          right.x1 = common.x2;
          right.y1 = common.y2;
          if(pointSmaller(s.x2,s.y2,x2,y2)){
             right.x2 = x2;
             right.y2 = y2;
          } else {
             right.x2 = s.x2;
             right.y2 = s.y2;
          }
          if(pointSmaller(x2,y2,s.x2,s.y2)){
            right.owner = s.owner;
            right.insideAbove_first = s.insideAbove_first;
            right.insideAbove_second = s.insideAbove_second;
          } else {
            right.owner = this->owner;
            right.insideAbove_first = this->insideAbove_first;
            right.insideAbove_second = this->insideAbove_second; 
          }
          right.con_above = con_above;
          right.con_below = con_below;
          return COMMON | RIGHT; 
       }
     } else { // left points are different
       // create the left segment
       if(pointSmaller(x1,y1,s.x1,s.y1)){
         left.x1 = x1;
         left.y1 = y1;
         left.x2 = s.x1;
         left.y2 = s.y2;
       } else {
         left.x1 = s.x1;
         left.y1 = s.x2;
         left.x2 = x1;
         left.y2 = y1;
       }
       if(pointSmaller(x1, y1, s.x1, s.y1)){ // left is part of this
         left.owner = this->owner;
         left.insideAbove_first = this->insideAbove_first;
         left.insideAbove_second = this->insideAbove_second;
       } else { // left is owned by s
         left.owner = s.owner;
         left.insideAbove_first = s.insideAbove_first;
         left.insideAbove_second = s.insideAbove_second;
       }
       left.con_below = con_below;
       left.con_above = con_above;
       common.x1 = left.x2;
       common.y1 = left.y2;
       if(pointSmaller(x2,y2,s.x2,s.y2)){
          common.x2 = x2;
          common.y2 = y2;
       } else {
          common.x2 = s.x2;
          common.y2 = s.y2;
       }
       common.owner = both;
       if(this->owner==first){
          common.insideAbove_first = this->insideAbove_first;
          common.insideAbove_second = s.insideAbove_second;
       }else {
          common.insideAbove_first = s.insideAbove_first;
          common.insideAbove_second = this->insideAbove_second;
       }
       common.con_below = con_below;
       common.con_above = con_above;
       if(pointEqual(x2, y2, s.x2, s.y2)){ // common endpoint, no right part
         return LEFT | COMMON;
       }
       // create the right part
       right.x1 = common.x2;
       right.y1 = common.y2;
       if(pointSmaller(s.x2,s.y2,x2,y2)){
          right.x2 = x2;
          right.y2 = y2;
       } else {
          right.x2 = s.x2;
          right.y2 = s.y2;
       }
       if(pointSmaller(x2,y2,s.x2,s.y2)){ // right owned by s
         right.owner = s.owner;
         right.insideAbove_first = s.insideAbove_first;
         right.insideAbove_second = s.insideAbove_second;
       } else {
         right.owner = this->owner;
         right.insideAbove_first = this->insideAbove_first;
         right.insideAbove_second = this->insideAbove_second;
       }
       right.con_above = con_above;
       right.con_below = con_below;
       return LEFT | COMMON | RIGHT; // all parts exist
     }     

  }

/*
~splitAt~

This function divides an segment into two parts at the point 
provided by (x, y). The point must be on the interior of this segment.

*/

  void splitAt(const double x, const double y, 
               AVLSegment& left, 
               AVLSegment& right)const{

     assert(ininterior(x,y));
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
Both segments must crossing.

*/
void splitCross(const AVLSegment& s, AVLSegment& left1, AVLSegment& right1,
                AVLSegment& left2, AVLSegment& right2) const{

    double x,y;
    if(!crosses(s,x,y)){
      bool cross= false;
      assert(cross);
    } 
    splitAt(x, y, left1, right1);
    s.splitAt(x, y, left2, right2);
}

/*
3.9 Converting

~ConvertToHs~

This functions creates an HalfSegment from this segment.
The owner must be __first__ or __second__

*/
HalfSegment convertToHs(bool lpd, ownertype owner = both )const{
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
   
   HalfSegment hs(lpd, Point(true,x1,y1), Point(true,x2,y2)); 
   hs.attr.insideAbove = insideAbove;
   return hs;
}

/*
3.10 public data members 

These members are not used in this class. So the user of
this class can change them without any problems within this
class itself.

*/
 int con_below;  // should be used as coverage number
 int con_above;  // should be used as coverage number


/*
3.11 Private part of the class

Here the data members as well as some auxiliary functions are
collected.

*/


private:
  /* data members  */
  double x1,y1,x2,y2; // the geometry of this segment
  bool insideAbove_first;
  bool insideAbove_second;
  ownertype owner;    // who is the owner of this segment


/*
~pointequal~

This function checks if the points defined by (x1,y1) and 
(x2,y2) are equals using the AlmostEqual function.

*/
  static bool pointEqual(const double x1, const double y1,
                         const double x2, const double y2){
    return AlmostEqual(x1,x2) && AlmostEqual(y1,y2);
  }

/*
~pointSmaller~

This function checks if the point defined by (x1,y1) is
smaller than the point defined by (x2,y2).

*/

 static bool pointSmaller(const double x1, const double y1,
                          const double x2, const double y2){

    return comparePoints(x1,y1,x2,y2) < 0;
 }


/*
~comparePoints~

*/
  static int comparePoints(const double x1,const  double y1,
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
segemnt is greater than all other slopes. 

*/
   int compareSlopes(const AVLSegment& s) const{
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
      double res1 = (y2-y1)/(x2-x1);
      double res2 = (s.y2-s.y1)/(s.x2-s.x1);
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
~XOverlaps~

Checks whether the x interval of this segment overlaps the
x interval of s.

*/

  bool xOverlaps(const AVLSegment& s) const{
    if(!AlmostEqual(x1,s.x2) && x1 > s.x2){ // left of s
        return false;
    }
    if(!AlmostEqual(x2,s.x1) && x2 < s.x1){ // right of s
        return false;
    }
    return true;
  }

/*
~XContains~

Checks if the x coordinates provided by the parameter __x__ is contained
in the x interval of this segment;

*/
  bool xContains(const double x) const{
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
  double getY(const double x) const{

     assert(xContains(x));
     if(isVertical()){
        return y1;
     }
     double d = (x-x1)/(x2-x1);
     return y1 + d*(y2-y1);
  }

};


/*
3.12 Shift Operator 

*/
ostream& operator<<(ostream& o, const AVLSegment& s){
    s.Print(o);
    return o;
}



/*
5 Some auxiliary Function


*/

inline void SetII(Int9M& m, const bool useCluster, 
                  Cluster& cluster,bool& done){
  if(!m.GetII()){
     m.SetII(true);
     if(useCluster){
        cluster.Restrict(II,true);
        done = done || cluster.isExtension(m) || cluster.IsEmpty();
     }
  }
}

inline void SetIB(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetIB()){
     m.SetIB(true);
     if(useCluster){
        cluster.Restrict(IB,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetIE(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetIE()){
     m.SetIE(true);
     if(useCluster){
        cluster.Restrict(IE,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetBI(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetBI()){
     m.SetBI(true);
     if(useCluster){
        cluster.Restrict(BI,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetBB(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetBB()){
     m.SetBB(true);
     if(useCluster){
        cluster.Restrict(BB,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetBE(Int9M& m, const bool useCluster, 
                  Cluster& cluster, bool& done){
  if(!m.GetBE()){
     m.SetBE(true);
     if(useCluster){
        cluster.Restrict(BE,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetEI(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetEI()){
     m.SetEI(true);
     if(useCluster){
        cluster.Restrict(EI,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetEB(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetEB()){
     m.SetEB(true);
     if(useCluster){
        cluster.Restrict(EB,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetEE(Int9M& m, const bool useCluster, 
                  Cluster& cluster, bool& done){
  if(!m.GetEE()){
     m.SetEE(true);
     if(useCluster){
        cluster.Restrict(EE,true);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}


/*
~insertEvents~

Creates Events for the AVLSegment and insert them into ~q1~ and/ or ~q1~.
The target queue(s) is determined by the owner of ~seg~.
The flags ~createLeft~ and ~createRight~ determine
whether the left and / or the right events should be created.

*/

void insertEvents(const AVLSegment& seg,
                  const bool createLeft,
                  const bool createRight,
                  priority_queue<HalfSegment,  
                                 vector<HalfSegment>, 
                                 greater<HalfSegment> >& q1,
                  priority_queue<HalfSegment,  
                                 vector<HalfSegment>, 
                                 greater<HalfSegment> >& q2){
   switch(seg.getOwner()){
      case first: {
                    if(createLeft){
                      q1.push(seg.convertToHs(true, first));
                    }
                    if(createRight){
                      q1.push(seg.convertToHs(false, first));
                    }
                  }
                  break;
      case second:
                    if(createLeft){
                      q2.push(seg.convertToHs(true, second));
                    }
                    if(createRight){
                      q2.push(seg.convertToHs(false, second));
                    }
                    break;
      case both :
                    if(createLeft){
                      q1.push(seg.convertToHs(true, first));
                      q2.push(seg.convertToHs(true, second));
                    }
                    if(createRight){
                      q1.push(seg.convertToHs(false, first));
                      q2.push(seg.convertToHs(false, second));
                    }
                    break;
      default: assert(false);
   }

}

/*
~performSplit~

Replaces the original segment by left and inserts corresponding events 
into the queues.
 

*/
void performSplit(AVLTree<AVLSegment>& sss,
                  const AVLSegment& original,
                  const AVLSegment& left,
                  const AVLSegment& right,
                  priority_queue<HalfSegment,  
                                 vector<HalfSegment>, 
                                 greater<HalfSegment> >& q1,
                  priority_queue<HalfSegment,  
                                 vector<HalfSegment>, 
                                 greater<HalfSegment> >& q2){
   
   bool ok = sss.remove(original);
   assert(ok);
   ok = sss.insert(left);
   assert(ok);
   insertEvents(left, false, true, q1, q2);
   insertEvents(right, true, true, q1, q2);
}
 


/*
6 Statistical information 

The following variables collect some statistical information.

*/

#ifdef TOPOPS_USE_STATISTIC
int GetCalls_p_p;
int GetCalls_ps_p;
int GetCalls_ps_ps;
int GetCalls_l_p;
int GetCalls_l_ps;
int GetCalls_l_l;
int GetCalls_r_p;
int GetCalls_r_ps;
int GetCalls_r_r;
int bb_p_p;
int bb_ps_p;
int bb_ps_ps;
int bb_l_p;
int bb_l_ps;
int bb_l_l;
int bb_r_p;
int bb_r_ps;
int bb_r_r;
#endif


/*
~ResetStatistic~

This functions sets all variables holding statistical
information to int initial values.

*/

#ifdef TOPOPS_USE_STATISTIC
static void ResetStatistic(){
 GetCalls_p_p=0;
 GetCalls_ps_p=0;
 GetCalls_ps_ps=0;
 GetCalls_l_p=0;
 GetCalls_l_ps=0;
 GetCalls_l_l=0;
 GetCalls_r_p=0;
 GetCalls_r_ps=0;
 GetCalls_r_r=0;
 bb_p_p=0;
 bb_ps_p=0;
 bb_ps_ps=0;
 bb_l_p=0;
 bb_l_ps=0;
 bb_l_l=0;
 bb_r_p=0;
 bb_r_ps=0;
 bb_r_r=0;
}
#endif



/*
7 Computation of the 9 intersection matrices

The following functions compute the 9 intersection matrix for 
different combinations of spatial data types.


7.1 ~point~ [x] ~point~


This function computes the 9-intersection matrix between two point values.
Because a single point is very simple, no bounding box tests are
performed.

Complexity: O(1)

*/

bool GetInt9M(Point* p1 , Point*  p2,Int9M& res, 
             const bool useCluster= false,
             Cluster cluster = Cluster()){
#ifdef TOPOPS_USE_STATISTIC
  GetCalls_p_p++;
#endif
  res.SetValue(0);
  // in each case, the exteriors intersect
  res.SetEE(true);
  if(AlmostEqual(p1,p2)){
    res.SetII(true);
  }else{
    res.SetIE(true);
    res.SetEI(true);
  }
  if(useCluster){
    return cluster.Contains(res);
  } else {
    return true;
  }
}


/*
7.2 ~point~ [x] ~points~


The next function computes the 9 intersection matrix between a 
point value and a points value. 

Complexity: O(log(n))  where ~n~ is the number of points in the __ps__
value.

*/
bool GetInt9M(Points*  ps, Point* p,Int9M& res,
              const bool useCluster=false,
              Cluster cluster = Cluster()){
#ifdef TOPOPS_USE_STATISTIC
  GetCalls_ps_p++;
#endif
  // initialization
  res.SetValue(0);
  res.SetEE(true); // holds always for bounded objects
  
  // check for emptyness
  if(ps->IsEmpty()){ // the simples case
     res.SetEI(true);

#ifdef TOPOPS_USE_STATISTIC
     bb_ps_p++;
#endif
     if(useCluster){
       return cluster.Contains(res);
     } else {
       return true;
     }
  }  
  
  // bounding box check
  Rectangle<2> box_ps = ps->BoundingBox();
  Rectangle<2> box_p  = p->BoundingBox();
  if(!box_p.Intersects(box_ps)){ // disjointness of the bounding boxes
      res.SetIE(true);
      res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_ps_p++;
#endif
     if(useCluster){
        return cluster.Contains(res);
     } else {
        return true;
     }
  }

  int size = ps->Size();
  if(size>1){
     res.SetIE(true);
  }
  if(!(ps->Contains(p))){ // Contains uses binary search
     res.SetEI(true);
     res.SetIE(true);
  } else{
     res.SetII(true);  
  }
  if(useCluster){
    return cluster.Contains(res);
  } else {
    return true;
  }
}



/*
7.3 ~points~ [x] ~points~


This function returns the 9 intersection matrix describing the 
topological relationship between two __points__ values.

Complexity: O(~n~+~m~) , where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively, where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively.


If the useCluster is set to be __false__, the return value is always
__true__.   In this case, the parameter __res__ will contain the
9 intersection matrix describing the topological relationship between
~p1~1 and ~ps~2. In the other case, this matrix in not computed 
completely. Instead, the return value will indicate whether the 
topological relationship is part of the ~cluster~. 


*/
bool GetInt9M(Points* ps1, Points*  ps2,
              Int9M& res,
              const bool useCluster = false,
              Cluster cluster = Cluster(false)){
 
   if(useCluster && cluster.IsEmpty()){
       return false;
   }
   if(useCluster){ // restrict cluster to matrices which are realizable 
                   // for the points/points combination
      cluster.Restrict(EE,true); // the extreiors always intersect

      // the boundary of a points value is empty, thereby no intersections
      // of this part with any other part may exist
      Int9M r(true, false, true, false, false, false, true,false,true);
      cluster.Restrict(r,false);

      if(cluster.IsEmpty()){
        return false;
      }
   }
 
#ifdef TOPOPS_USE_STATISTIC

   GetCalls_ps_ps++;
#endif

   int n1 = ps1->Size();
   int n2 = ps2->Size();

   res.SetValue(0);
   res.SetEE(true);

   if(n1<=0 && n2<=0){
      // there are no inner parts which can intersect any part
#ifdef TOPOPS_USE_STATISTIC
     bb_ps_ps++;
#endif
     if(useCluster){
       return cluster.Contains(res);
     } else {
       return true;
     }
   }
   if(n1<=0){
      // some points of ps2 are in the exterior of ps1
      res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_ps_ps++;
#endif
      if(useCluster){
         return cluster.Contains(res);
      } else {
         return true;
      }
   }
   if(n2<=0){
      // symmetrically to the previous case
      res.SetIE(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_ps_ps++;
#endif
      if(useCluster){
         return cluster.Contains(res);
      } else {
         return true;
      }
   }


   // bounding box check
   Rectangle<2> bbox1 = ps1->BoundingBox();
   Rectangle<2> bbox2 = ps2->BoundingBox();
   if(!bbox1.Intersects(bbox2)){
      // non empty disjoint points values
      res.SetIE(true);
      res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_ps_ps++;
#endif
      if(useCluster){
        return cluster.Contains(res);
      } else {
         return true;
      }
   }

   // bounding box check failed, perform an parallel scan

   const Point* p1;
   const Point* p2;

   int i1=0; 
   int i2=0;
 
   bool done = false;
   do{ 
       ps1->Get(i1,p1);
       ps2->Get(i2,p2);
       int cmp = p1->Compare(p2);
       p1 = NULL;
       p2 = NULL;
       if(cmp==0){ //p1==p2
         SetII(res,useCluster,cluster,done); 
         i1++;
         i2++;
       } else if (cmp<0){
         // p1 in the exterior of p2 
         SetIE(res,useCluster,cluster,done);
         i1++; 
       } else{ // p1 > p2
         SetEI(res,useCluster,cluster,done);
         i2++;
       }
       done= done || 
             ((i1>=n1-1) || (i2>=n2-1)) || // end of one points value reached
             (res.GetII() && res.GetIE() && res.GetEI()); 
       if(useCluster){
          done = done || cluster.IsEmpty();
       }
   }while(!done);    // end of scan

   if(res.GetII() && res.GetIE()  && res.GetEI()){ 
      // maximum count of intersections
      if(!useCluster){
         return true;  
      } else {
         return cluster.Contains(res);
      }
   }
   if((i1<n1-1)){ // ps1 has further points
      SetIE(res,useCluster,cluster,done);
   }

   if( (i2<n2-1)){ // ps2 has further points
      SetEI(res,useCluster,cluster,done);
   }
   if(!useCluster){
      return true;
   } else {
      return cluster.Contains(res);
   }
}



/*
7.4 ~line~ [x] ~point~ and ~line~ [x] ~points~

The next functions compute the topological relationship between a
line and a point or points value. 


~SelectNext~ line x point

This function looks which event from the line or from the point
is smaller. The line is divided into the original line part
at position posLine and possible splitted segments stored
in ~q~. The return value of the function will be ~first~ if the
next event comes from the line value and ~second~ if the next
event comes from the point value. Depending of the return value,
one of the arguments ~resHs~ or ~resPoint~ is set the the value of 
this event. 
The positions are increased automatically by this function.


*/

ownertype selectNext( Line const* const line,
                      priority_queue<HalfSegment, 
                                     vector<HalfSegment>, 
                                     greater<HalfSegment> >& q,
                      int& posLine,
                      Point const* const point,
                      int& posPoint, // >0: point already used
                      HalfSegment& resHs,
                      Point& resPoint){


   int size = line->Size();
   const HalfSegment* hsl = 0;
   const HalfSegment* hsq = 0;
   const HalfSegment* hsmin = 0;
   HalfSegment hstmp;
   int src = 0;  
   if(posLine < size){
      line->Get(posLine,hsl);
   }
   if(!q.empty()){
       hstmp = q.top();
       hsq = &hstmp;
   }
   if(hsl){
      src = 1;
      hsmin = hsl;
   }
   if(hsq){
     if(!hsl || (*hsq < *hsl)){
       src = 2;
       hsmin = hsq;
     }
   }
  
   if(posPoint==0){  // point not already used
     if(!hsmin){
       src = 3;
     } else {
       Point p = hsmin->GetDomPoint();
       if(*point < p){
            src = 3;
        }
     }
   }

   switch(src){
    case 0: return none;
    case 1: posLine++;
            resHs = *hsmin;
            return first;
    case 2: q.pop();
            resHs = *hsmin;
            return first;
    case 3: resPoint = *point;
            posPoint++;
            return second;
    default: assert(false); 
             return none;
   }
}

/*
~selectNext~ line x points

This function works like the function above but instead for a point, a 
points value is used.

*/


ownertype selectNext( Line const* const line,
                      priority_queue<HalfSegment, 
                                     vector<HalfSegment>, 
                                     greater<HalfSegment> >& q,
                      int& posLine,
                      Points const* const point,
                      int& posPoint, 
                      HalfSegment& resHs,
                      Point& resPoint){

   int sizeP = point->Size();
   int sizeL = line->Size();

  
   const HalfSegment* hsl = 0;
   const HalfSegment* hsq = 0;
   const HalfSegment* hsmin = 0;
   HalfSegment hstmp;
   int src = 0;  
   if(posLine < sizeL){
      line->Get(posLine,hsl);
   }
   if(!q.empty()){
       hstmp = q.top();
       hsq = &hstmp;
   }
   if(hsl){
      src = 1;
      hsmin = hsl;
   }
   if(hsq){
     if(!hsl || (*hsq < *hsl)){
       src = 2;
       hsmin = hsq;
     }
   }
  
   const Point * cp;
   if(posPoint<sizeP){  // point not already used
     point->Get(posPoint,cp);
     if(!hsmin){
       src = 3;
     } else {
       Point p = hsmin->GetDomPoint();
       if(*cp < p){
            src = 3;
        }
     }
   }

   switch(src){
    case 0: return none;
    case 1: posLine++;
            resHs = *hsmin;
            return first;
    case 2: q.pop();
            resHs = *hsmin;
            return first;
    case 3: resPoint = *cp;
            posPoint++;
            return second;
    default: assert(false); 
             return none;
   }

}
/*
~initBox~

Initializes the result and performs a bounding box check.
If the initialisation is sufficient to determine the result,
i.e. if the line is empty, the result will be true. Otherwise,
the result will be false. 

*/
bool initBox(Line const* const line, 
             Point const* const  point, 
             Int9M& res,
             bool & pointdone){
   res.SetValue(0);
   res.SetEE(true);

   if(line->IsEmpty()){
       res.SetEI(true);
       return true;
   }
   
   // the interior of a non-empty line has always
   // an intersection with the exterior of a single point
   // because of the difference in the dimension   
   res.SetIE(true);

   pointdone = false;
   // the line contains at least one halfsegment
   Rectangle<2> bbox_line = line->BoundingBox();
   Rectangle<2> bbox_point = point->BoundingBox();
   // both objects are disjoint
   if(!bbox_line.Intersects(bbox_point)){
      res.SetIE(true);
      res.SetEI(true);
      pointdone = true;
   }
  
   return false;
}

bool initBox(Line const* const line, 
             Points const* const  point, 
             Int9M& res,
             bool & pointdone){
   res.SetValue(0);
   res.SetEE(true);

   pointdone = false;
   if(line->IsEmpty()){
       if(point->IsEmpty()){
          pointdone = true;
          return true;
       }
       else {
         res.SetEI(true);
         return true;
       }
   }

   
   // the interior of a non-empty line has always
   // an intersection with the exterior of a single point
   // because of the difference in the dimension   
   res.SetIE(true);
  
   if(point->IsEmpty()){
      pointdone = true;
      return false; // search for boundary points
   }

   
   // line and point are non empty
   Rectangle<2> bbox_line = line->BoundingBox();
   Rectangle<2> bbox_point = point->BoundingBox();
   // both objects are disjoint
   if(!bbox_line.Intersects(bbox_point)){
      res.SetIE(true);
      res.SetEI(true);
   }
   return false;
}

/*
~isDone~

This function checks whether all possible intersections for the 
parameter combination of the first to parameters are set in in the 
matrix.

*/

bool isDone(Line const* const line, Point const* const point, Int9M m,
            const bool useCluster, const Cluster& cluster){

 bool res = m.GetBE() && // endpoint of the line has been found
           (m.GetII()  || m.GetBI() || m.GetEI());
 if(useCluster){
   return res || cluster.IsEmpty() || cluster.isExtension(m);
 } else {
    return res;
 }
}


bool isDone(Line const* const line, Points const* const point, Int9M m,
            const bool useCluster, const Cluster& cluster){

  bool res = m.GetBE() && // endpoint of the line has been found
             (m.GetII()  && m.GetBI() && m.GetEI());
  if(useCluster){
     return res || cluster.IsEmpty() || cluster.isExtension(m);
  } else {
     return res;
  }
}

/*
~GetInt9M~

The template parameter can be instantiated with ~Point~ or ~Points~.

*/

template<class Pclass>
bool GetInt9M(Line const* const line, 
              Pclass const* const point,
              Int9M& res,
              const bool useCluster,
              Cluster& cluster){

   bool pointdone1 = false;
   if(initBox(line,point,res,pointdone1)){
      if(useCluster){
         return cluster.Contains(res);
      } else {
        return true;
      }
   }

   // line is not empty
 
   // restrict the cluster to valid matrices
   if(useCluster){
       cluster.Restrict(EE,true); // hold in each case
       cluster.Restrict(IE,true); // nonempty line

       // boundary of a point is empty
       Int9M m(true, false, true, true,false,true, true,false,true);
       cluster.Restrict(m,false);
       if(cluster.IsEmpty()){
         return false;
       }
   }

   // prefilter unsuccessful -> scan the halfsegments

   AVLTree<AVLSegment> sss;
   priority_queue<HalfSegment, vector<HalfSegment>, greater<HalfSegment> > q;


   bool done = false;
   int posline = 0; // position within the line array
   ownertype owner;

   int posPoint = pointdone1?1:0;

   HalfSegment resHs;
   Point resPoi;
   Point lastDomPoint;
   int lastDomPointCount = 0;
   // avoid unneeded expensive restrictions of the cluster
  

   while(!done &&  
         ((owner=selectNext(line,q,posline,point,
           posPoint,resHs,resPoi))!=none)){

      if(owner==second){ // event comes from the point(s) value
         AVLSegment current(&resPoi,second);

         const AVLSegment* leftN=0;
         const AVLSegment* rightN=0;
         const AVLSegment* member= sss.getMember(current,leftN,rightN);
         if(!member){ // point outside current, check lastdompoint
           if(lastDomPointCount>0 && AlmostEqual(lastDomPoint,resPoi)){
             // point located in the last dominating point
             if(lastDomPointCount==1){ // on boundary
                SetBI(res,useCluster,cluster,done);
             } else {
                SetII(res,useCluster,cluster,done);
             }
             lastDomPointCount++;
           } else { // point outside the line
             SetEI(res,useCluster,cluster,done);
             // update dompoint
             if(lastDomPointCount==1){ // last point was boundary
                SetBE(res,useCluster,cluster,done);
             }
             lastDomPoint = resPoi;
             lastDomPointCount = 0;
          }
         } else {
            double x = resPoi.GetX();
            double y = resPoi.GetY();
            if((leftN && leftN->contains(x,y))  ||
               (rightN && rightN->contains(x,y))||
               ( member->ininterior(x,y))){
              SetII(res,useCluster,cluster,done);
            } else { // point located on an endpoint of member
              if(lastDomPointCount>0 && AlmostEqual(resPoi,lastDomPoint)){
                 if(lastDomPointCount==1){
                   SetBI(res,useCluster,cluster,done);
                 } else {
                   SetII(res,useCluster,cluster,done);
                 }
                 lastDomPointCount++;
              } else {
                 SetEI(res,useCluster,cluster,done);
                 lastDomPointCount = 0;
              } 

            }
         }
         done = done || isDone(line,point,res,useCluster,cluster); 
         lastDomPoint = resPoi;
      } else { // an halfsegment
        assert(owner==first);

        // check for endpoints
        Point domPoint = resHs.GetDomPoint();

        // only check for dompoints if the segment is a new one (left)
        // or its actually stored in the tree 
        AVLSegment current(&resHs, first);
        const AVLSegment* leftN=0;
        const AVLSegment* rightN=0;
        const AVLSegment* member= sss.getMember(current,leftN,rightN);
 
        if(resHs.IsLeftDomPoint()  ||
           (member && member->exactEqualsTo(current))){
           if(lastDomPointCount==0 || !AlmostEqual(domPoint,lastDomPoint)){
              if(lastDomPointCount==1){
                 SetBE(res,useCluster,cluster,done);
              } 
              lastDomPoint = domPoint;
              lastDomPointCount = 1;
           } else{
              lastDomPointCount++;
              lastDomPoint = domPoint;
           }
        }

        if(resHs.IsLeftDomPoint()){  // left event
           AVLSegment left1, left2, right1, right2;
           if(member){ //overlapping segment found
              if((!AlmostEqual(member->getX2(),current.getX2())) &&
                 (member->getX2() < current.getX2() )){
                // current is an extension of member
                current.splitAt(member->getX2(), member->getY2(),left1,right1);
                // create events for the remaining parts
                q.push(right1.convertToHs(true,first));
                q.push(right1.convertToHs(false,first));
              }
           } else { // there is no overlapping segment

             // check for splits with the left segment
             if(leftN && !leftN->innerDisjoint(current)){
               if(leftN->ininterior(current.getX1(), current.getY1())){
                  leftN->splitAt(current.getX1(), current.getY1(),
                                 left1,right1);
                  sss.remove(*leftN);
                  leftN = &left1;
                  sss.insert(left1);
                  q.push(left1.convertToHs(false,first));
                  q.push(right1.convertToHs(true,first));
                  q.push(right1.convertToHs(false,first));
               } else if(leftN->ininterior(current.getX2(), current.getY2())){
                  leftN->splitAt(current.getX2(), current.getY2(),
                                 left1,right1);
                  sss.remove(*leftN);
                  leftN = &left1;
                  sss.insert(left1);
                  q.push(left1.convertToHs(false,first));
                  q.push(right1.convertToHs(true,first));
                  q.push(right1.convertToHs(false,first));
               } else if(leftN->crosses(current)){
                  leftN->splitCross(current,left1,right1, left2,right2);

                  sss.remove(*leftN);
                  leftN = &left1;
                  sss.insert(left1);
                  q.push(left1.convertToHs(false,first));
                  q.push(right1.convertToHs(true,first));
                  q.push(right1.convertToHs(false,first));
                  
                  current = left2;
                  q.push(left2.convertToHs(false,first));
                  q.push(right2.convertToHs(true,first));
                  q.push(right2.convertToHs(false,first));
               } else if(current.ininterior(leftN->getX2(), leftN->getY2())){
                 current.splitAt(leftN->getX2(), leftN->getY2(),left1,right1);
                 current=left1;
                 q.push(left1.convertToHs(false,first));
                 q.push(right1.convertToHs(true,first));
                 q.push(right1.convertToHs(false,first));
               } else { // forgotten case
                   assert(false);
               }
             }
             // do the same thing for the rigt neighbour
             if(rightN && !rightN->innerDisjoint(current)){
               if(rightN->ininterior(current.getX1(), current.getY1())){
                  rightN->splitAt(current.getX1(), current.getY1(),
                                  left1,right1);
                  sss.remove(*rightN);
                  rightN = &left1;
                  sss.insert(left1);
                  q.push(left1.convertToHs(false,first));
                  q.push(right1.convertToHs(true,first));
                  q.push(right1.convertToHs(false,first));
               }else if(rightN->ininterior(current.getX2(), current.getY2())){
                  rightN->splitAt(current.getX2(), current.getY2(),
                                  left1, right1);
                  sss.remove(*rightN);
                  rightN = &left1;
                  sss.insert(left1);
                  q.push(left1.convertToHs(false,first));
                  q.push(right1.convertToHs(true,first));
                  q.push(right1.convertToHs(false,first));
               } else if(rightN->crosses(current)){
                  rightN->splitCross(current,left1,right1, left2,right2);
                  sss.remove(*rightN);
                  rightN = &left1;
                  sss.insert(left1);
                  q.push(left1.convertToHs(false,first));
                  q.push(right1.convertToHs(true,first));
                  q.push(right1.convertToHs(false,first));
                  
                  current = left2;
                  q.push(left2.convertToHs(false,first));
                  q.push(right2.convertToHs(true,first));
                  q.push(right2.convertToHs(false,first));
               }else if(current.ininterior(rightN->getX2(), rightN->getY2())){
                 current.splitAt(rightN->getX2(), rightN->getY2(),
                                 left1, right1);
                 current=left1;
                 q.push(left1.convertToHs(false,first));
                 q.push(right1.convertToHs(true,first));
                 q.push(right1.convertToHs(false,first));
               } else { // forgotten case
                   assert(false);
               }
             }
             sss.insert(current);
           } // no overlapping segment
       } else { // right event

           AVLSegment left1, left2, right1, right2;

           if(member && member->exactEqualsTo(current)){ // segment found

              sss.remove(current);

              if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
                 // the neighbours are intersecting ... check how !
                 
                 // endpoint of leftN splits rightN
                 if(rightN->ininterior(leftN->getX2(), leftN->getY2())){
                    rightN->splitAt(leftN->getX2(), leftN->getY2(),
                                    left1, right1);
                    sss.remove(*rightN);
                    rightN = &left1;
                    sss.insert(left1);
                    q.push(left1.convertToHs(false,first));
                    q.push(right1.convertToHs(true,first));
                    q.push(right1.convertToHs(false,first));
                 }
                 // endpoint of rightN splits leftN
                 if(leftN->ininterior(rightN->getX2(), rightN->getY2())){
                    leftN->splitAt(rightN->getX2(), rightN->getY2(),
                                   left1, right1);
                    sss.remove(*leftN);
                    leftN = &left1;
                    sss.insert(left1);
                    q.push(left1.convertToHs(false,first));
                    q.push(right1.convertToHs(true,first));
                    q.push(right1.convertToHs(false,first));
                 }

                 // leftN and rightN are crossing
                 if(rightN->crosses(*leftN)){
                    rightN->splitCross(*leftN,left1,right1, left2,right2);
                    sss.remove(*rightN);
                    rightN = &left1;
                    sss.remove(*leftN);
                    leftN = &left2;

                    sss.insert(left1);
                    sss.insert(left2);
                    q.push(left1.convertToHs(false,first));
                    q.push(left2.convertToHs(false,first));
                    q.push(right1.convertToHs(true,first));
                    q.push(right1.convertToHs(false,first));
                    q.push(right2.convertToHs(true,first));
                    q.push(right2.convertToHs(false,first));
                 }
              }
           }
       }
     }
     done = done ||  isDone(line,point,res,useCluster, cluster);
   } // end sweep

   if(lastDomPointCount==1) { 
     // last Point of the line is an endpoint
     SetBE(res,useCluster,cluster,done);
   }   
   if(useCluster){
      return cluster.Contains(res);
   }else {
      return true;
   }
}

/*
~Instantiations of the template function~

*/

bool GetInt9M(Line const* const line, Point const* const point,Int9M& res,
              const bool useCluster=false, Cluster cluster = Cluster()){
   return GetInt9M<Point>(line,point,res, useCluster, cluster);
}

bool GetInt9M(Line const* const line, Points const* const point,Int9M& res,
              const bool useCluster=false, Cluster cluster = Cluster()){
  return GetInt9M<Points>(line,point,res, useCluster, cluster);
}



/*
7.5 ~region~ [x] ~point~

Computation of the 9 intersection matrix for a single point and a 
region value. 


~pointAbove~

Auxiliary functtion checking if ~p~ is located above ~hs~.

*/


bool pointAbove(const HalfSegment* hs, const Point* p) {
  double x1 = hs->GetLeftPoint().GetX();
  double y1 = hs->GetLeftPoint().GetY();
  double x2 = hs->GetRightPoint().GetX();
  double y2 = hs->GetRightPoint().GetY();

  double x = p->GetX();
  double y = p->GetY();

  if(AlmostEqual(x1,x2)){ // vertical segment
     return y > max(y1,y2);
  }
  double d = (x-x1)/(x2-x1);
  double ys = y1 + d*(y2-y1);
  return (!AlmostEqual(y,ys) && y > ys);
}


bool GetInt9M(Region const* const reg, Point const* const p, Int9M& res,
              const bool useCluster= false,
              Cluster cluster = Cluster(),
              const bool transpose = false){

#ifdef TOPOPS_USE_STATISTIC
  GetCalls_r_p++;
#endif

  res.SetValue(0);
  res.SetEE(true);
  if(reg->IsEmpty()){
     res.SetEI(true);
     if(useCluster){
       if(transpose){
          res.Transpose();
       }
       return cluster.Contains(res);
     } else {
       return true;
     }
  }
  res.SetIE(true); // the point can't cover an infinite set of points
  res.SetBE(true); 

  Rectangle<2> bboxreg = reg->BoundingBox();
  Rectangle<2> bboxp = p->BoundingBox();

  if(!bboxreg.Intersects(bboxp)){
     res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_r_p++;
#endif
     if(useCluster){
       if(transpose){
          res.Transpose();
       }
       return cluster.Contains(res);
     } else {
       return true;
     }
  }

   // bbox check failed, we have to compute the result

   int size = reg->Size();
   double x = p->GetX();
  
   const HalfSegment* hs;

   int number = 0;

   for(int i=0;i<size;i++){
     reg->Get(i,hs);
     if(hs->IsLeftDomPoint()){
        if(hs->Contains(*p)){
           res.SetBI(true); //point on boundary
           if(useCluster){
             if(transpose){
               res.Transpose();
             }
             return cluster.Contains(res);
           } else {
              return true;
           }
        }
        if(!hs->IsVertical()){
            if(pointAbove(hs,p)){
               if((AlmostEqual(hs->GetRightPoint().GetX(),x)) ||
                  ( !AlmostEqual(hs->GetLeftPoint().GetX(),x) &&
                    hs->GetLeftPoint().GetX()<x &&
                    hs->GetRightPoint().GetX()>=x)){
                   number++;
               }
            }
        }
     }
   }
   if(number % 2 == 0){
      res.SetEI(true);
   } else{
      res.SetII(true);
   }
   if(useCluster){
      if(transpose){
        res.Transpose();
      }
      return cluster.Contains(res);
   } else {
      return true;
   }
}



/*
7.6 ~region~ [x] ~points~

Computation of the 9 intersection matrix for a set of points and a 
region value. 


~selectNext~

If the event caused by the region is smaller then this one cause by the
points vaue, the result of the function will be ~first~. Otherwise, the
result will be ~second~. The region itself may be consist of the original
halfsegments and halfsegments produced by dividing original halfsegments.
The positions indicate the current elements of the halfsegment arrays.
They are updated automatically within this function. 
Depending on the return value, one of the parameter ~resultHs~ or
~resultPoint~ is set to the value of the next event.

If the region and the points value are already processed, the return value will 
be ~none~.

*/

ownertype selectNext(const Region*  reg,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q1,
                     int& pos1,
                     Points const* const p,
                     int& pos2,
                     HalfSegment& resultHS,
                     Point& resultPoint){

  assert(pos1>=0);
  assert(pos2>=0);

  int sizereg = reg->Size();
  int sizepoint = p->Size();

  const HalfSegment* rhs = 0;
  const HalfSegment* qhs = 0;
  HalfSegment qhsc;
  const HalfSegment* minhs=0;
  const Point* cp = 0;

  if(pos1<sizereg){
     reg->Get(pos1,rhs);
  } 
  if(!q1.empty()){
     qhsc = q1.top();
     qhs = &qhsc;
  }
  
  if(pos2<sizepoint){
     p->Get(pos2,cp); 
  }

  int src = 0;  // none
  if(rhs){
    src = 1; 
    minhs = rhs;
  }
  if(qhs){
    if(src==0) {
      src = 1;
      minhs = qhs;
    } else { // rhs and qhs exist
      if(*qhs < *rhs){
        src = 2;
        minhs = qhs;
      }
    }
  }
  if(cp){
   if(!minhs){
     src = 3;
   }  else {
     double px = cp->GetX();
     double hx = minhs->GetDomPoint().GetX();
     if(AlmostEqual(px,hx)){
        double py = cp->GetY();
        double hy = minhs->GetDomPoint().GetY();
        if(AlmostEqual(py,hy)){
          if(!minhs->IsLeftDomPoint()){
            // left < point < right
            src = 3;
          }
        } else if(py<hy){
          src = 3;
        } // else don't change src
     } else if(px<hx){
       src = 3;
     } // else do not change src
   }
  }

  switch(src){
    case 0: { return none;
            }
    case 1: { // region
              pos1++;
              resultHS = *minhs;
              return first;
            }
    case 2: { // queue
              q1.pop();
              resultHS = *minhs;
              return first;
            }
    case 3: {  // point
               pos2++;
               resultPoint = *cp;
               return second;
            }
    default:  assert(false);
              return none; 
  }
}


bool GetInt9M(Region const* const reg, Points const* const ps, Int9M& res,
              const bool useCluster=false,
              Cluster cluster = Cluster()){
#ifdef TOPOPS_USE_STATISTIC
  GetCalls_r_ps++;
#endif
  res.SetValue(0); 
  // test for emptyness
   res.SetEE(true);
   if(reg->IsEmpty()){
      if(ps->IsEmpty()){
#ifdef TOPOPS_USE_STATISTIC
         bb_r_ps++;
#endif
       if(useCluster){
          return cluster.Contains(res);
       } else {
         return true; 
       }
      }
      res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_r_ps++;
#endif
      if(useCluster){
        return cluster.Contains(res);;
      } else {
        return true;  
      }
   } 
   res.SetIE(true);
   res.SetBE(true);

   if(ps->IsEmpty()){ // no more intersections can be found
#ifdef TOPOPS_USE_STATISTIC
      bb_r_ps++;
#endif
      if(useCluster){
        return cluster.Contains(res);
      } else {
        return true;
      }
   }
  // bounding box test
  Rectangle<2> regbox = reg->BoundingBox();
  Rectangle<2> pbox = ps->BoundingBox();
  if(!regbox.Intersects(pbox)){ // disjoint objects
    res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
    bb_r_ps++;
#endif
    if(useCluster){
      return cluster.Contains(res);
    } else {
      return true;
    }
  }
  // bbox failed, perform a plane sweep

  if(useCluster){
    // restrict the cluster to possible values

    // boundary of an points value is empty
    Int9M m1(true,false,true, true,false,true, true,false,true);
    cluster.Restrict(m1,false);
    
    cluster.Restrict(res,true);

    if(cluster.IsEmpty()){
      return false;
    }
    if(cluster.isExtension(res)){
      return true;
    }
  }

  // queue for splitted segments of the region
  priority_queue<HalfSegment, vector<HalfSegment>, greater<HalfSegment> > q1;
  AVLTree<AVLSegment> sss;

  ownertype owner;
  bool done = false; 
  int pos1 =0; 
  int pos2 =0;
  Point CP;
  HalfSegment CH;

  while (!done && ( (owner= selectNext(reg,q1,pos1, ps,pos2,CH,CP))!=none)){
    if(owner==second){ // the point
       AVLSegment current(&CP,second);
       const AVLSegment* left=0;
       const AVLSegment* right=0;
       const AVLSegment* member = sss.getMember(current, left, right);
       if(member){ // point located on boundary
         SetBI(res,useCluster,cluster,done);
       } else if(left){
         if(left->getInsideAbove_first()){
            SetII(res,useCluster,cluster,done);
         } else {
            SetEI(res,useCluster,cluster,done);
         }
       } else { // there is nothing under cp
         SetEI(res,useCluster,cluster,done);
       }
       done = done || res.GetII() && res.GetBI() && res.GetEI();
    } else {  // the next element comes from the region
      AVLSegment current(&CH,first);

      const AVLSegment* leftN = 0;
      const AVLSegment* rightN = 0;
      const AVLSegment* member = sss.getMember(current,leftN,rightN);
      if(CH.IsLeftDomPoint()){ // left Event
         assert(!member); // a single region can't contain overlapping segments
         AVLSegment left;
         AVLSegment right;

         // check if left or right split the current element
         if(leftN && current.ininterior(leftN->getX2(), leftN->getY2())){
            current.splitAt(leftN->getX2(), leftN->getY2(), left, right);
            current = left;
            q1.push(right.convertToHs(true,first));
            q1.push(right.convertToHs(false,first)); 
         }
         if(rightN && current.ininterior(rightN->getX2(), rightN->getY2())){
            current.splitAt(rightN->getX2(), rightN->getY2(), left, right);
            current = left;
            q1.push(right.convertToHs(true,first));
            q1.push(right.convertToHs(false,first)); 
         }     
         // check if current splits left or right
         if(leftN && leftN->ininterior(current.getX1(), current.getY1())){
            leftN->splitAt(current.getX1(), current.getY1(),left,right); 
            sss.remove(*leftN);
            sss.insert(left);
            leftN = &left;
            q1.push(left.convertToHs(false,first));
            q1.push(right.convertToHs(true,first));
            q1.push(right.convertToHs(false,first));
         }
         if(leftN && leftN->ininterior(current.getX2(), current.getY2())){
            leftN->splitAt(current.getX2(), current.getY2(),left,right); 
            sss.remove(*leftN);
            sss.insert(left);
            leftN = &left;
            q1.push(left.convertToHs(false,first));
            q1.push(right.convertToHs(true,first));
            q1.push(right.convertToHs(false,first));
         }
         if(rightN && rightN->ininterior(current.getX1(), current.getY1())){
            rightN->splitAt(current.getX1(), current.getY1(),left,right); 
            sss.remove(*rightN);
            sss.insert(left);
            rightN = &left;
            q1.push(left.convertToHs(false,first));
            q1.push(right.convertToHs(true,first));
            q1.push(right.convertToHs(false,first));
         }
         if(rightN && rightN->ininterior(current.getX2(), current.getY2())){
            rightN->splitAt(current.getX2(), current.getY2(),left,right); 
            sss.remove(*rightN);
            sss.insert(left);
            rightN = &left;
            q1.push(left.convertToHs(false,first));
            q1.push(right.convertToHs(true,first));
            q1.push(right.convertToHs(false,first));
         }
         sss.insert(current);

 
      } else { // right event
        if(member){
           sss.remove(current);
           if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
              AVLSegment left;
              AVLSegment right;
              if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
                 leftN->splitAt(rightN->getX2(),rightN->getY2(),left,right);
                 sss.remove(*leftN);
                 sss.insert(left);
                 q1.push(left.convertToHs(false,first));
                 q1.push(right.convertToHs(true,first));
                 q1.push(right.convertToHs(false,first));
              }
              if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
                 rightN->splitAt(leftN->getX2(),leftN->getY2(),left,right);
                 sss.remove(*leftN);
                 sss.insert(left);
                 q1.push(left.convertToHs(false,first));
                 q1.push(right.convertToHs(true,first));
                 q1.push(right.convertToHs(false,first));
              }
           }
        }
      }
    } // element from region
  } // while
  if(useCluster){
    return cluster.Contains(res);
  } else{
    return true;
  }
}



/*
Selects the mimimum halfsegment from v1, v2, q1, and q2.
If no values are available, the return value will be __none__. 
In this case, __result__ remains unchanged. Otherwise, __result__
is set to the minimum value found.
Otherwise, it will be first or second depending on the value 
containing the minimum. If some halfsegments are equal, the one
from the v1 is selected. 
Note: pos1 and pos2 are increased automatically. In the same way, 
      the topmost element of the selected queue is deleted. 

The template paraeter may be instantiated with ~Region~ or ~Line~

*/
template<class T1, class T2>
ownertype selectNext(T1 const* const v1,
                     int& pos1,
                     T2 const* const v2,
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
  int number = 0; // number of available values
  // read the available elements
  if(pos1<v1->Size()){
     v1->Get(pos1,values[0]);
     number++;
  }  else {
     values[0] = 0;   
  }
  if(q1.empty()){
    values[1] = 0; 
  } else {
    values[1] = &q1.top();
    number++;   
  }
  if(pos2<v2->Size()){
     v2->Get(pos2,values[2]);
     number++;
  }  else {
     values[2] = 0;   
  }
  if(q2.empty()){
    values[3] = 0; 
  } else {
    values[3] = &q2.top();
    number++;   
  }
  // no halfsegments found

  if(number == 0){
     return none;
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
    case 0: pos1++; return first; 
    case 1: q1.pop();  return first;
    case 2: pos2++;  return second;
    case 3: q2.pop();  return second;
    default: assert(false);   
  }
  return none;
}

/*
~Instantiation of the selectNext Function~

*/

ownertype selectNext(Region const* const reg1,
                     int& pos1,
                     Region const* const reg2,
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
   return selectNext<Region,Region>(reg1,pos1,reg2,pos2,q1,q2,result,src);
}

class OwnedPoint{

 public:
    OwnedPoint(){
      defined = false;
    }   

    OwnedPoint(Point p0, ownertype o){
       p = p0;
       owner = o;
       defined = true;
    }

    OwnedPoint(const OwnedPoint& src){
       p = src.p;
       owner = src.owner;
       defined = src.defined;
    }

    OwnedPoint& operator=(const OwnedPoint& src){
       p = src.p;
       owner = src.owner;
       defined = src.defined;
       return *this;
    }

    Point p;
    ownertype owner; 
    bool defined; 
};

/*
7.7 ~region~ [x] ~region~

*/

bool GetInt9M(Region const* const reg1, Region const* const reg2, Int9M& res,
              const bool useCluster =false,
              Cluster cluster = Cluster()){
#ifdef TOPOPS_USE_STATISTIC
  GetCalls_r_r++;
#endif
  res.SetValue(0);;
  res.SetEE(true);
  // check for emptyness
  if(reg1->IsEmpty()){
    if(reg2->IsEmpty()){ // no more intersection possible
#ifdef TOPOPS_USE_STATISTIC
       bb_r_r++;
#endif
       if(useCluster){
           return cluster.Contains(res); 
       } else {
           return true;
       }
    }else{
      res.SetEI(true);
      res.SetEB(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_r_r++;
#endif
      if(useCluster){
         return cluster.Contains(res); 
      } else {
         return true;
      }
    }
  }

  if(reg2->IsEmpty()){
     res.SetIE(true);
     res.SetBE(true);
#ifdef TOPOPS_USE_STATISTIC
     bb_r_r++;
#endif
     if(useCluster){
        return cluster.Contains(res);
     } else {
        return true;
     }
  }
  // bounding box check
  Rectangle<2> bbox1 = reg1->BoundingBox();
  Rectangle<2> bbox2 = reg2->BoundingBox();
  if(!bbox1.Intersects(bbox2)){
     res.SetIE(true);
     res.SetEI(true);
     res.SetBE(true);
     res.SetEB(true);
#ifdef TOPOPS_USE_STATISTIC
     bb_r_r++;
#endif
     if(useCluster){
        return cluster.Contains(res);
     } else{
        return true;
     }
  }

  if(useCluster){
     cluster.Restrict(res,true);
     if(cluster.IsEmpty() ){
        return false;
     }
     if(cluster.isExtension(res)){
        return true;
     } 
  } 


  AVLTree<AVLSegment> sss;            // sweep state structure
  // dynamic parts of the sweep event structure
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;

  int pos1=0; // current position in the halfsegment array of reg1
  int pos2=0; // current position in the halfsegment array of reg2

  bool done = false;
  HalfSegment nextSeg;

  const AVLSegment* member=0; // current member stored in the tree
  const AVLSegment* leftN=0;  // the left neightboor of member
  const AVLSegment* rightN=0; // the right neighbour of member
  ownertype owner;
  OwnedPoint lastDomPoint; // initialized to be undefined
  int src; 

  while( ((owner=selectNext(reg1,pos1, reg2,pos2, q1,q2,nextSeg,src))!=none)
          && !done){

    AVLSegment current(&nextSeg,owner);

    member = sss.getMember(current,leftN,rightN);

    /*
    Because right events are processed before
    left events, we have to store the last dominating point
    to detect intersections of the boundaries within a single point.
    */

    Point p = nextSeg.GetDomPoint();
    if(!lastDomPoint.defined || !AlmostEqual(lastDomPoint.p,p)){
        lastDomPoint.defined = true;
        lastDomPoint.p = p;
        lastDomPoint.owner = owner;
    } else { // same point as before
       if(lastDomPoint.owner != owner){
          SetBB(res,useCluster,cluster,done);
       }
    }


    if(nextSeg.IsLeftDomPoint()){
        AVLSegment left, common, right;
        if(member){ // there is an overlapping segment in the tree
           // check for valid region representation
           assert(member->getOwner() != current.getOwner()); 
           assert(member->getOwner() != both);
           SetBB(res,useCluster,cluster,done); // common boundary found
           bool iac = current.getOwner()==first?current.getInsideAbove_first()
                                            :current.getInsideAbove_second();
           bool iam = member->getOwner()==first?member->getInsideAbove_first()
                                           :member->getInsideAbove_second();

           if(iac!=iam){
             SetIE(res,useCluster,cluster,done);
             SetEI(res,useCluster,cluster,done);
           } else {
             SetII(res,useCluster,cluster,done);
             // res.setEE(true); // alreay done in initialisation
           }

           int parts = member->split(current,left,common,right);

           sss.remove(*member);

           if(parts & LEFT){   // there is a left part
              sss.insert(left);  // simulates a left event.
              // create a halfsegment (rightevent) for left
              // create the right event for this segment
              switch(left.getOwner()){
                 case first: q1.push(left.convertToHs(false,first)); break;
                 case second: q2.push(left.convertToHs(false,second)); break;
                 case both  : q1.push(left.convertToHs(false,first));
                              q2.push(left.convertToHs(false,second)); break; 
                 default: assert(false);
              }
           }
           assert(parts & COMMON);  // there must exist a common part
        
           
           // update con_above
           if(iac){
             common.con_above++; 
           } else {
             common.con_above--;
           } 

           sss.insert(common);
           
           // insert the corresponding right event into one of the queues
           HalfSegment hs_common1 = common.convertToHs(false,first);
           HalfSegment hs_common2 = common.convertToHs(false,second);
           q1.push(hs_common1); 
           q2.push(hs_common2);

           if(parts & RIGHT) { // there is an exclusive right part 
              // create the events for the remainder
              HalfSegment hs_right = right.convertToHs(true);
              switch(right.getOwner()){
                 case first: { q1.push(HalfSegment(hs_right));
                               hs_right.SetLeftDomPoint(false);
                               q1.push(HalfSegment(hs_right));
                               break;
                              }
                 case second: { q2.push(hs_right);
                                hs_right.SetLeftDomPoint(false);
                                q2.push(hs_right);
                                break;
                              }
                 default: assert(false); //other values not allowed here
              }
           }

       /*

        Note:
          Here no check again the neighbours is performed because all
          parts inserted into sss here are part of this structure 
          before inserting the removed element 'member'.
       */
        } else{ // there is no overlapping segment

           // check crossing left 
           if(leftN && leftN->crosses(current)){ // a inner intersection
              assert(leftN->getOwner()!=current.getOwner());
              // computation of the intersections
              res.Fill(); 
              done = true;
              if(useCluster){
                return cluster.Contains(res);
              } else {
                return true;
              }
           }    
           // check crossing right
           if(rightN && rightN->crosses(current)){
              assert(rightN->getOwner()!=current.getOwner());
              // computation of the intersections
              res.Fill(); 
              done = true;
              if(useCluster){ 
                 return cluster.Contains(res);
              } else {
                 return true;
              }
           }
           // check for disjointness with both neighbours
           if( ( !leftN || leftN->innerDisjoint(current)) &&
               ( !rightN || rightN->innerDisjoint(current))){
             
              // update coverage numbers
              bool iac = current.getOwner()==first
                              ?current.getInsideAbove_first()
                              :current.getInsideAbove_second();

              iac = current.getOwner()==first?current.getInsideAbove_first()
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

              // derive intersections from the coverage numbers
              if(current.con_below == 0){
                  ; // EE is set already in initialization
                  //res.SetEE(true);
              } else if (current.con_below == 1) {
                  if(current.getInsideAbove()){
                     if(current.getOwner()==first){
                        SetEI(res,useCluster,cluster,done);
                     } else {
                        SetIE(res,useCluster,cluster,done);
                     }
                  } else {
                     if(current.getOwner()==first){
                        SetIE(res,useCluster,cluster,done);
                     } else {
                        SetEI(res,useCluster,cluster,done);
                     }
                  }
              } else {
                 assert(current.con_below==2);
                 SetII(res,useCluster,cluster,done);
              }
              // check for possible common endpoints with the neighbours
              if(leftN && leftN->getOwner()!=current.getOwner()){
                 if(leftN->intersects(current)){
                     SetBB(res,useCluster,cluster,done);
                 }
              }
              if(rightN && rightN->getOwner()!=current.getOwner()){
                 if(rightN->intersects(current)){
                     SetBB(res,useCluster,cluster,done);  
                 }
              }
              sss.insert(current);

           } else {
              // check for possible common points with the neighbours
              if(leftN && leftN->getOwner()!=current.getOwner()){
                 if(leftN->intersects(current)){
                     SetBB(res,useCluster,cluster,done);
                 }
              }
              if(rightN && rightN->getOwner()!=current.getOwner()){
                 if(rightN->intersects(current)){
                     SetBB(res,useCluster,cluster,done);
                 }
              }

             if(leftN && 
                current.ininterior(leftN->getX2(), leftN->getY2())){

                // case 1 endpoint of leftN devides current
                AVLSegment left, right;
                current.splitAt(leftN->getX2(), leftN->getY2(), left, right);
                current = left;
                HalfSegment hs_left = left.convertToHs(false);
                switch(current.getOwner()){
                  case first:   { 
                                  q1.push(hs_left);
                                  q1.push(right.convertToHs(true));
                                  q1.push(right.convertToHs(false));
                                  break;
                                }
                   case second: { q2.push(hs_left);
                                  q2.push(right.convertToHs(true));
                                  q2.push(right.convertToHs(false));
                                  break;
                                }
                   default: assert(false);
                }
             }
            
             if(rightN && 
                current.ininterior(rightN->getX2(), rightN->getY2())){
                // case 1 endpoint of rightN devides current
                AVLSegment left, right;
                current.splitAt(rightN->getX2(), rightN->getY2(), left, right);
                current = left;
                HalfSegment hs_left = left.convertToHs(false);
                switch(current.getOwner()){
                  case first:   { q1.push(hs_left);
                                  q1.push(right.convertToHs(true));
                                  q1.push(right.convertToHs(false));
                                  break;
                                }
                   case second: { q2.push(hs_left);
                                  q2.push(right.convertToHs(true));
                                  q2.push(right.convertToHs(false));
                                  break;
                                }
                   default: assert(false);

                }
             }
            
             if(leftN && // case leftN ist splitted by current
                ( leftN->ininterior(current.getX1(), current.getY1()) ||
                  leftN->ininterior(current.getX2(), current.getY2()))){
               // determine the split point
               double x,y;
               if( leftN->ininterior(current.getX1(), current.getY1())){
                   x = current.getX1();
                   y = current.getY1();
               } else {
                   x = current.getX2();
                   y = current.getY2();
               }

               // split leftN
               AVLSegment left, right;
               leftN->splitAt(x, y, left, right);
               // remove the nonsplitted segment from sss 
               // and insert the shorted one

               sss.remove(*leftN);
               leftN = &right; 
               sss.insert(left);

               switch(left.getOwner()){
                  case first:   { q1.push(left.convertToHs(false,first));
                                  q1.push(right.convertToHs(false));
                                  q1.push(right.convertToHs(true));
                                  break;
                                }
                   case second: { q2.push(left.convertToHs(false,second));
                                  q2.push(right.convertToHs(false));
                                  q2.push(right.convertToHs(true));
                                  break;
                                }
                   case both: {   q1.push(left.convertToHs(false,first));
                                  q2.push(left.convertToHs(false,second));
                                  q1.push(right.convertToHs(false,first));
                                  q2.push(right.convertToHs(false,second));
                                  q1.push(right.convertToHs(true,first));
                                  q2.push(right.convertToHs(true,second));
                                  break;
                              }
                   default: assert(false);

                }

             }
             if(rightN && // case rightN ist splitted by current
                ( rightN->ininterior(current.getX1(), current.getY1()) ||
                  rightN->ininterior(current.getX2(), current.getY2()))){
               // determine the split point
               double x,y;
               if( rightN->ininterior(current.getX1(), current.getY1())){
                   x = current.getX1();
                   y = current.getY1();
               } else {
                   x = current.getX2();
                   y = current.getY2();
               }
               // split leftN
               AVLSegment left, right;
               rightN->splitAt(x, y, left, right);
               // remove the nonsplitted segment from sss 
               // and insert the shorted one
               sss.remove(*rightN);
               sss.insert(left);
               // create events for the splitted segments
               switch(left.getOwner()){
                  case first:   { q1.push(left.convertToHs(false,first));
                                  q1.push(right.convertToHs(true,first));
                                  q1.push(right.convertToHs(false,first));
                                  break;
                                }
                   case second: { q2.push(left.convertToHs(false,second));
                                  q2.push(right.convertToHs(false,second));
                                  q2.push(right.convertToHs(true,second));
                                  break;
                                }
                   case both: {   q1.push(left.convertToHs(false,first));
                                  q2.push(left.convertToHs(false,second));
                                  q1.push(right.convertToHs(true,first));
                                  q2.push(right.convertToHs(true,second));
                                  q1.push(right.convertToHs(false,first));
                                  q2.push(right.convertToHs(false,second));
                                  break;
                              }
                   default: assert(false);

                }
             }
             // insert current (may be shortened) into sss
            // update coverage numbers
            bool iac = current.getOwner()==first?
                                current.getInsideAbove_first()
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

            // derive intersections from the coverage numbers
            if(current.con_below == 0){
                ; // res.SetEE(true); // already done in initialization
            } else if (current.con_below == 1) {
                if(current.getInsideAbove()){
                   if(current.getOwner()==first){
                      SetEI(res,useCluster,cluster,done);
                   } else {
                      SetIE(res,useCluster,cluster,done);
                   }
                } else {
                   if(current.getOwner()==first){
                      SetIE(res,useCluster,cluster,done);
                   } else {
                      SetEI(res,useCluster,cluster,done);
                   }
                }
            } else if(current.con_below==2){
               SetII(res,useCluster,cluster,done);
            } else {
              cerr << "invalid value for  con_below " 
                   << current.con_below << endl;
              if(!leftN){
                 cerr << "no predecessor found" << endl;
              } else {
                 cerr << "Left= " << *leftN << endl;
                 cerr << "extension : " << current.extends(*leftN) << endl;
                 cerr << "vertical  : " << leftN->isVertical() << endl;
              }
              assert(false);
              
            }

              
             assert(current.con_above>=0 && current.con_above<=2);
             assert(current.con_below + current.con_above > 0);
             sss.insert(current);
           }
       }
    } else { // right endpoint of an halfsegment

      if(member){ // element found in sss, may be an old splitted element
        // check if where member is located
        if(member->getOwner()!=both){
           // member is located in the interior or in the exterior of the 
           // other region
           if(member->con_below==0 || member->con_above==0){ // in exterior
              switch(member->getOwner()){
                case first: SetBE(res,useCluster,cluster,done);  
                            SetIE(res,useCluster,cluster,done); 
                            //res.SetEE(true); 
                            break;
                case second: SetEB(res,useCluster,cluster,done);  
                             SetEI(res,useCluster,cluster,done); 
                             //res.SetEE(true); 
                             break;
                default: assert(false);
              }
           } else if(member->con_below==2 || member->con_above==2){ //interior
              switch(member->getOwner()){
                case first: SetBI(res,useCluster,cluster,done); 
                            SetII(res,useCluster,cluster,done); 
                            SetEI(res,useCluster,cluster,done); 
                            break;
                case second: SetIB(res,useCluster,cluster,done); 
                             SetII(res,useCluster,cluster,done); 
                             SetIE(res,useCluster,cluster,done);
                             break;
                default: assert(false);
              }
           }else {
              assert(false);
           }    
        }

        sss.remove(*member);

       
        if( leftN && rightN && !leftN->innerDisjoint(*rightN)){

           // leftN and rightN are crossing or one of the segments
           // splits the other one by its right endpoint 
           if(leftN->crosses(*rightN)){
              assert(leftN->getOwner() != rightN->getOwner()); 
              res.Fill(); // we are done
              if(useCluster){ 
                 return cluster.Contains(res);
              } else {
                 return true;
              }
           } else if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
              AVLSegment left, right;
              leftN->splitAt(rightN->getX2(), rightN->getY2(),left,right);
              sss.remove(*leftN);
              leftN = &right;
              sss.insert(left);
              switch(leftN->getOwner()){
                  case first: { q1.push(left.convertToHs(false));
                               q1.push(right.convertToHs(true,first));
                               q1.push(right.convertToHs(false,first));
                               break;
                              }
                  case second: { q2.push(left.convertToHs(false));
                               q2.push(right.convertToHs(true,second));
                               q2.push(right.convertToHs(false,second));
                               break;
                               }
                  case both: { q1.push(left.convertToHs(false,first));
                               q2.push(left.convertToHs(false,second));
                               q1.push(right.convertToHs(true,first));
                               q2.push(right.convertToHs(true,second));
                               q1.push(right.convertToHs(false,first));
                               q2.push(right.convertToHs(false,second));
                               break; 
                             }
                  default:  assert(false);
              }
           } else if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
              AVLSegment left, right;
              rightN->splitAt(leftN->getX2(), leftN->getY2(),left,right);
              sss.remove(*rightN);
              sss.insert(left);
              switch(rightN->getOwner()){
                  case first: { q1.push(left.convertToHs(false));
                               q1.push(right.convertToHs(true,first));
                               q1.push(right.convertToHs(false,first));
                               break;
                              }
                  case second: { q2.push(left.convertToHs(false));
                               q2.push(right.convertToHs(true,second));
                               q2.push(right.convertToHs(false,second));
                               break;
                               }
                  case both: { q1.push(left.convertToHs(false,first));
                               q2.push(left.convertToHs(false,second));
                               q1.push(right.convertToHs(true,first));
                               q2.push(right.convertToHs(true,second));
                               q1.push(right.convertToHs(false,first));
                               q2.push(right.convertToHs(false,second));
                               break; 
                             }
                  default:  assert(false);
              }
           } else {  // should never occur
               assert(false); 
           }
        }
      }
    }
    if(res.IsFull()){
      done = true;
    }
    if(useCluster){
      done = done || cluster.IsEmpty();
    }
  } // sweep
  
  if(useCluster){
    return cluster.Contains(res); 
  } else {
    return true;
  }
 
} // GetInt9M (region x region)



/* 
7.8 ~line~ [x] ~line~


~Instantiation of the selectNext function ~

*/
ownertype selectNext(Line const* const line1,
                     int& pos1,
                     Line const* const line2,
                     int& pos2,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
                     int& src
                    ){

   return selectNext<Line,Line>(line1,pos1,line2,pos2,q1,q2,result, src);
}


/*
~updateDompoints~

This functions does all things caused by the next dominating point.
This function can only used for two line values.

*/

void updateDomPoints(Point& lastDomPoint, const Point& newDomPoint,
                     int& lastDomPointCount1, int& lastDomPointCount2,
                     ownertype owner,
                     Int9M& res,
                     bool useCluster,
                     Cluster& cluster,
                     bool& done){

  // update dominating point information
  int sum = lastDomPointCount1 + lastDomPointCount2;

  if( sum == 0 || !AlmostEqual(lastDomPoint,newDomPoint)){
     // dominating point changed
     if(lastDomPointCount1 == 1){
        if(lastDomPointCount2 == 1){
           SetBB(res,useCluster,cluster,done);
        } else if(lastDomPointCount2>1){
           SetBI(res,useCluster,cluster,done);
        } else {
           SetBE(res,useCluster,cluster,done);
        }
     } else if(lastDomPointCount1 > 1){
        if(lastDomPointCount2 == 1){
           SetIB(res,useCluster,cluster,done);
        } else if(lastDomPointCount2>1){
           SetII(res,useCluster,cluster,done);
        } else {
           SetIE(res,useCluster,cluster,done);
        }
     } else { // lastDomPointCount1 == 0
        if(lastDomPointCount2 == 1){
           SetEB(res,useCluster,cluster,done);
        } else if(lastDomPointCount2>1){
           SetEI(res,useCluster,cluster,done);
        } else {
           ;
           // SetEE(res,useCluster,cluster,done);
        }
     }
      
     // update dompoint and counter
     lastDomPoint = newDomPoint;
     switch(owner){
        case first: lastDomPointCount1 = 1;
                    lastDomPointCount2 = 0;
                    break;
        case second: lastDomPointCount1 = 0;
                     lastDomPointCount2 = 1;
                     break;
        case both:   lastDomPointCount1 = 1;
                     lastDomPointCount2 = 1;
                     break;
        default : assert(false);
     } 
  } else { // dominating point not changed
      switch(owner){
         case first : lastDomPointCount1++;
                      break;
         case second: lastDomPointCount2++;
                      break;
         case both  : lastDomPointCount1++;
                      lastDomPointCount2++;
                      break;
         default : assert(false);
       } 
  }
}


bool GetInt9M(Line const* const line1, 
              Line const* const line2, 
              Int9M& res,
              const bool useCluster=false,
              Cluster cluster = Cluster() ){


// we can only ommit the planesweep if both lines are empty
// disjointness of the lines is not sufficient to compute the 
// result completely because it's not known if one of the line has
// any endpoints. For this reason, we ommit the bounding box check

 res.SetValue(0);
 res.SetEE(true);

 if(line1->IsEmpty() && line2->IsEmpty()){
    if(useCluster){
       return cluster.Contains(res);
    }else {
       return true; // no further intersections can be found
    }
 }

 bool done = false;

 if(line1->IsEmpty()){
    // line2 is non empty
    SetEI(res,useCluster,cluster,done);
    if(useCluster){
      Int9M m(false,false,false, false,false,false, true,true,true);
      cluster.Restrict(m,false);
    }
 }
 if(line2->IsEmpty()){
    SetIE(res,useCluster,cluster,done);
    if(useCluster){
      Int9M m(false, false, true, false,false,true, false,false,true);
      cluster.Restrict(m,false);
    }
 }


 if(useCluster){
   if(cluster.IsEmpty()){
     return false;
   }
   if(cluster.isExtension(res)){
     return true;
   }
 }

 // initialise the sweep state structure  
 AVLTree<AVLSegment> sss;
 // initialize priority queues for remaining parts of splitted
 // segments
 priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
 priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;

 int pos1=0;
 int pos2=0;

 HalfSegment nextHs;

 ownertype owner;

 const AVLSegment* leftN=0;
 const AVLSegment* rightN=0;
 const AVLSegment* member=0;

 Point lastDomPoint;
 int lastDomPointCount1 = 0;
 int lastDomPointCount2 = 0;
 AVLSegment left1,right1,left2,right2,common;
 int src;

 while(!done && 
       ((owner=selectNext(line1,pos1,line2,pos2,q1,q2,nextHs,src))!=none) ){
   AVLSegment current(&nextHs,owner);

   // try to find an overlapping segment in sss
   member = sss.getMember(current,leftN,rightN);
   if(nextHs.IsLeftDomPoint()){ // left event
      if(member){ // overlapping segment found in sss
        if(member->getOwner()==both || member->getOwner()==owner){
           // member and owner comes from the same line
           // create events for the remaining part 
           if(!AlmostEqual(member->getX2(),current.getX2()) &&
              current.getX2() > member->getX2()){
             // current is an extension of member
             current.splitAt(member->getX2(),member->getY2(),left1,right1);
             insertEvents(right1,true,true,q1,q2);
           } // otherwise we can ignore current
        } else { // owner of member and current are different
           // there is a common inner part
           SetII(res,useCluster,cluster,done);

           int parts =  member->split(current, left1, common, right1);

           // remove the old entry in sss
           sss.remove(*member);
           member = &common;
           // insert left and common to sss
           if(parts&LEFT){
              bool ok = sss.insert(left1);
              assert(ok);
              insertEvents(left1,false,true,q1,q2);
           } 
           assert(parts & COMMON);
           
           bool ok = sss.insert(common);
           assert(ok);
           insertEvents(common,false,true,q1,q2);
           // create events for right
           if(parts & RIGHT){
              insertEvents(right1,true,true,q1,q2);
           }

           Point newDomPoint = nextHs.GetDomPoint();
           updateDomPoints(lastDomPoint,newDomPoint, 
                          lastDomPointCount1,lastDomPointCount2,owner, res,
                          useCluster, cluster, done);
           
           if(parts & LEFT){
              // we have already processed the left event for the 
              // segement which is replaced by common,
              switch(owner){
                 case first: lastDomPointCount2++;
                             break;
                 case second: lastDomPointCount1++;
                              break;
                 default:     assert(false);
               } 
           }

        } 
      } else { // no overlapping segment stored in sss
        if(leftN && !leftN->innerDisjoint(current)){
          if(leftN->ininterior(current.getX1(),current.getY1())){
             // leftN divided by current's start point
             leftN->splitAt(current.getX1(),current.getY1(),left1,right1);
             performSplit(sss,*leftN,left1,right1,q1,q2);
             leftN = &left1; 
          } else if(leftN->ininterior(current.getX2(),current.getY2())){
             leftN->splitAt(current.getX2(),current.getY2(),left1,right1);
             performSplit(sss,*leftN,left1,right1,q1,q2);
             leftN = &left1; 
          } else  if(leftN->crosses(current)){
             if(current.getOwner()!=leftN->getOwner()){
                 SetII(res,useCluster,cluster,done);
             }
             leftN->splitCross(current,left1,right1,left2,right2);
             performSplit(sss,*leftN,left1,right1,q1,q2);
             leftN = &left1;
             current = left2;
             insertEvents(left2,false,true,q1,q2);
             insertEvents(right2,true,true,q1,q2);
          } else if(current.ininterior(leftN->getX2(),leftN->getY2())){
             current.splitAt(leftN->getX2(),leftN->getY2(),left1,right1);
             current = left1;
             insertEvents(left1,false,true,q1,q2);
             insertEvents(right1,true,true,q1,q2); 
          } else { // forgotten case
             assert(false);
          }
        }
        if(rightN && !rightN->innerDisjoint(current)){
          if(rightN->ininterior(current.getX1(),current.getY1())){
             // leftN divided by current's start point
             rightN->splitAt(current.getX1(),current.getY1(),left1,right1);
             performSplit(sss,*rightN,left1,right1,q1,q2);
             rightN = &left1; 
          } else if(rightN->ininterior(current.getX2(),current.getY2())){
             rightN->splitAt(current.getX2(),current.getY2(),left1,right1);
             performSplit(sss,*rightN,left1,right1,q1,q2);
             rightN = &left1; 
          } else  if(rightN->crosses(current)){
             if(current.getOwner()!=rightN->getOwner()){
                 SetII(res,useCluster,cluster,done);
             }
             rightN->splitCross(current,left1,right1,left2,right2);
             performSplit(sss,*rightN,left1,right1,q1,q2);
             rightN = &left1;
             current = left2;
             insertEvents(left2,false,true,q1,q2);
             insertEvents(right2,true,true,q1,q2);
          } else if(current.ininterior(rightN->getX2(),rightN->getY2())){
             current.splitAt(rightN->getX2(),rightN->getY2(),left1,right1);
             current = left1;
             insertEvents(left1,false,true,q1,q2);
             insertEvents(right1,true,true,q1,q2); 
          } else { // forgotten case
             assert(false);
          }
        }
        updateDomPoints(lastDomPoint,nextHs.GetDomPoint(),
                        lastDomPointCount1, lastDomPointCount2, 
                        owner,res,useCluster,cluster,done);
        bool ok = sss.insert(current); 
        assert(ok);
      }

   } else { // right Event
     // check if current is stored in sss
     if(member && member->exactEqualsTo(current)){

        Point newDomPoint = nextHs.GetDomPoint();
        updateDomPoints(lastDomPoint,newDomPoint, 
                        lastDomPointCount1,lastDomPointCount2,
                        member->getOwner(), res, 
                        useCluster, cluster, done);

        AVLSegment copy(*member);
        sss.remove(current); 
        member = &copy;

        switch(member->getOwner()){
          case first:  SetIE(res,useCluster,cluster,done);
                       break;
          case second: SetEI(res,useCluster,cluster,done); 
                       break;
          case both  : SetII(res,useCluster,cluster,done);
                       break;
          default:     assert(false);
        }

        // check for intersections of the neighbours
        if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
           if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
              leftN->splitAt(rightN->getX2(),rightN->getY2(),left1,right1);
              performSplit(sss,*leftN,left1,right1,q1,q2);
              leftN = &left1;

           } else if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
              rightN->splitAt(leftN->getX2(),leftN->getY2(),left1,right1);
              performSplit(sss,*rightN,left1,right1,q1,q2);
              rightN = &left1;
           } else if(rightN->crosses(*leftN)){
               if(leftN->getOwner() != rightN->getOwner()){
                 SetII(res,useCluster,cluster,done);
               }  
               leftN->splitCross(*rightN,left1,right1,left2,right2);
               performSplit(sss,*leftN,left1,right1,q1,q2);
               performSplit(sss,*rightN,left2,right2,q1,q2);
               leftN = &left1;
               rightN = &left2;
           } else {  // forgotten case
             assert(false);
           }
        }          
     } // otherwise current comes from a splitted segment and is ignored
   }
   done = done || res.IsFull() ||
          (line1->IsEmpty() && res.GetEI() && 
           res.GetEB() && res.GetEE()) ||
          (line2->IsEmpty() && res.GetIE() && 
           res.GetBE() && res.GetEE()) ;
 } // end of sweep

 // create a point which is different to the last domPoint
 Point newDP(lastDomPoint);
 newDP.Translate(100,0);
 updateDomPoints(lastDomPoint, newDP, 
                 lastDomPointCount1, lastDomPointCount2, 
                 first,res,useCluster,cluster,done);

 if(useCluster){
    return cluster.Contains(res);
 } else {
    return true;
 }

}


/*
7.9 ~line~ [x] ~region~


~selectNext~

Instantiation of the selectNext function for line x region.

*/

ownertype selectNext(Line const* const line,
                     int& pos1,
                     Region const* const region,
                     int& pos2,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q2,
                     HalfSegment& result,
                     int& src
                    ){

   return selectNext<Line,Region>(line,pos1,region,pos2,q1,q2,result,src);
}


/*
~updateDomPointInfo[_]Line[_]Region~

Does the things required by the switch from lastDomPoint
to newDomPoint.


*/

void updateDomPointInfo_Line_Region(Point& lastDomPoint,
                                    const Point& newDomPoint,
                                    int& count_line,
                                    int& count_region,
                                    const int coverage_Num,
                                    const ownertype owner, 
                                    Int9M& res,
                                    const bool useCluster,
                                    Cluster& cluster,
                                    bool& done){

    int sum = count_line + count_region;
    if(sum == 0 || !AlmostEqual(newDomPoint,lastDomPoint)){
       // dominating point changed
       if(count_line==0) { // exterior of the line
          if(count_region>0){  // boundary of the region
             SetEB(res,useCluster,cluster,done);
          } else { 
             ; // SetEE(res,useCluster,cluster,done);
          }
       } else if(count_line == 1) { // boundary of the line
         if(count_region>0){ // boundary of the region
            SetBB(res,useCluster,cluster,done);
         } else {
           if(coverage_Num==0){
              SetBE(res,useCluster,cluster,done);
           } else {
              SetBI(res,useCluster,cluster,done);
           }
         }
       } else { // interior of the line
         if(count_region > 0){
            SetIB(res,useCluster,cluster,done);
         } else {
           if(coverage_Num==0){
              SetIE(res,useCluster,cluster,done);
           } else {
              SetII(res,useCluster,cluster,done);
           }
         }
       }
       lastDomPoint = newDomPoint;
       switch(owner){
         case first : count_line = 1;
                      count_region = 0;
                      break;
         case second: count_line = 0;
                      count_region = 1;
                      break;
         case both  : count_line = 1;
                      count_region = 1;
                      break;
         default    : assert(false);
      }
    } else { // dompoint was not changed
       switch(owner){
         case first : count_line++;
                      break;
         case second: count_region++;
                      break;
         case both  : count_line++;
                      count_region++;
                      break;
         default    : assert(false);
      }
    }
}


/*
~GetInt9M~  line x region

*/

bool GetInt9M(Line   const* const line, 
              Region const* const region, 
              Int9M& res,
              const bool useCluster=false,
              Cluster cluster = Cluster()){

  res.SetValue(0);
  res.SetEE(true);
  if(line->IsEmpty()){
     if(region->IsEmpty()){
       if(useCluster){
          return cluster.Contains(res);
       } else {
          return true;
       }
     } else {
        res.SetEI(true);
        res.SetEB(true);
        res.SetEE(true);
       if(useCluster){
          return cluster.Contains(res);
       } else {
          return true;
       }
     }
  }   

  if(!region->IsEmpty()){
     res.SetEI(true);
  }

  if(useCluster){
     cluster.Restrict(res,true);
     if(region->IsEmpty()){
        Int9M m(false,false,true,false,false,true,false,false,true);
        cluster.Restrict(m,false);
     }
     if(cluster.IsEmpty()){
       return false;
     } 
     if(cluster.isExtension(res)){
       return true;
     }
  }

 // initialise the sweep state structure  
 AVLTree<AVLSegment> sss;
 // initialize priority queues for remaining parts of splitted
 // segments
 priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
 priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;

 int pos1=0;
 int pos2=0;

 bool done = false;

 HalfSegment nextHs;

 ownertype owner;

 const AVLSegment* leftN=0;
 const AVLSegment* rightN=0;
 const AVLSegment* member=0;

 Point lastDomPoint;
 int lastDomPointCount1 = 0;
 int lastDomPointCount2 = 0;
 AVLSegment left1,right1,left2,right2,common;

 int src;

 while(!done && 
       ((owner=selectNext(line,pos1,region,pos2,q1,q2,nextHs,src))!=none)){

     AVLSegment current(&nextHs,owner);

     member = sss.getMember(current,leftN,rightN);
     ownertype owner2 = owner;

     if(nextHs.IsLeftDomPoint()){ // left end point
       if(member){ // overlapping segment found
         if(member->getOwner()==both || member->getOwner()==owner){
            // current is part of member
            if( (member->getX2() < current.getX2()) && 
                 !AlmostEqual(member->getX2(), current.getX2())){
               current.splitAt(member->getX2(),member->getY2(),left1,right1);
               insertEvents(right1,true,true,q1,q2);  
            } // otherwise there is nothing to do
         } else { // stored segment come from the other spatial object
            SetIB(res,useCluster,cluster,done);
            int parts = member->split(current,left1,common,right1);
            sss.remove(*member);
            member = &common;
            if(parts & LEFT){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
            assert(parts & COMMON);

            
            // update coverage numbers
            if(owner==first){ // the line
              common.con_below = member->con_below;
              common.con_above = member->con_above;
            } else { // a region
              common.con_below = member->con_below;
              if(current.isVertical()){
                 common.con_above = current.con_below; 
              } else { // non-vertical
                 if(nextHs.attr.insideAbove){
                   common.con_above = common.con_below +1;
                 } else {
                   common.con_above = common.con_below -1;
                 }
              }
            }

            assert(current.con_below + current.con_above >= 0);
            assert(current.con_below + current.con_above <= 1);

            sss.insert(common);
            insertEvents(common,false,true,q1,q2);
            if(parts & RIGHT){
              insertEvents(right1,true,true,q1,q2);
            }


            if(parts & LEFT){ // this dominating point comes from both 
                              // halfsegments
               owner2 = both;
            }
            // update counter for dominating points
            Point domPoint = nextHs.GetDomPoint();
            int cover = leftN?leftN->con_above:0;
            updateDomPointInfo_Line_Region(lastDomPoint, domPoint,
                                           lastDomPointCount1, 
                                           lastDomPointCount2,
                                           cover,owner2,res, 
                                           useCluster, cluster, done);
         }
       } else { // no overlapping segment found
          // check if current or an existing segment must be divided
          if(leftN && !leftN->innerDisjoint(current)){
              if(current.ininterior(leftN->getX2(), leftN->getY2())){
                current.splitAt(leftN->getX2(), leftN->getY2(),left1,right1);
                current = left1;
                insertEvents(left1,false,true,q1,q2);
                insertEvents(right1,true,true,q1,q2); 
              } else if(leftN->ininterior(current.getX1(),current.getY1())){
                leftN->splitAt(current.getX1(),current.getY1(),left1,right1);
                performSplit(sss,*leftN,left1,right1,q1,q2);
                leftN = &left1;
                if(owner != leftN->getOwner()){
                   owner2 = both;
                }
              } else if(leftN->ininterior(current.getX2(),current.getY2())){
                leftN->splitAt(current.getX2(),current.getY2(),left1,right1);
                performSplit(sss,*leftN,left1, right1, q1, q2);
                leftN = &left1;
              } else if(leftN->crosses(current)){
                 leftN->splitCross(current,left1,right1,left2,right2);
                 performSplit(sss,*leftN,left1,right1,q1,q2);
                 leftN = &left1;
                 current = left2;
                 insertEvents(left2,false,true,q1,q2); 
                 insertEvents(right2,true,true,q1,q2);
              } else {
                  assert(false);
              }
          }
          // check for intersections with the right neighbour
          if(rightN && !rightN->innerDisjoint(current)){
              if(current.ininterior(rightN->getX2(), rightN->getY2())){
                current.splitAt(rightN->getX2(), rightN->getY2(),left1,right1);
                current = left1;
                insertEvents(left1,false,true,q1,q2);
                insertEvents(right1,true,true,q1,q2); 
              } else if(rightN->ininterior(current.getX1(),current.getY1())){
                rightN->splitAt(current.getX1(),current.getY1(),left1,right1);
                performSplit(sss,*rightN,left1,right1,q1,q2);
                rightN = &left1;
                if(owner != rightN->getOwner()){
                  owner2 = both;
                }
              } else if(rightN->ininterior(current.getX2(),current.getY2())){
                rightN->splitAt(current.getX2(),current.getY2(),left1,right1);
                performSplit(sss,*rightN,left1, right1, q1, q2);
                rightN = &left1;
              } else if(rightN->crosses(current)){
                 rightN->splitCross(current,left1,right1,left2,right2);

                 performSplit(sss,*rightN,left1,right1,q1,q2);
                 rightN = &left1;
                 current = left2;
                 insertEvents(left2,false,true,q1,q2); 
                 insertEvents(right2,true,true,q1,q2);
              } else { //forgotten case
                 assert(false);
              }
          }

          // update coverage numbers
          if(leftN){
             current.con_below = leftN->con_above;               
          } else {
             current.con_below = 0;
          }
          if(owner==first || current.isVertical()){ 
               // a line does not change the coverage number 
             current.con_above = current.con_below;
          } else {
             if(nextHs.attr.insideAbove){
               current.con_above = current.con_below + 1;
             } else {
               current.con_above = current.con_below - 1;
             }
          } 
 
          // update dominating points
          Point domPoint = nextHs.GetDomPoint();
          int cover=leftN?leftN->con_above:0;
          updateDomPointInfo_Line_Region(lastDomPoint, domPoint,
                                        lastDomPointCount1,
                                        lastDomPointCount2,
                                        cover,
                                        owner2,
                                        res, useCluster, cluster, done);
          sss.insert(current);
       }
     } else { // right event
        if(member && member->exactEqualsTo(current)){
           AVLSegment tmp = *member;
           sss.remove(*member);
           member = &tmp;
 
           if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
              // leftN and rightN are intersecting
              if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
                 leftN->splitAt(rightN->getX2(),rightN->getY2(),left1,right1);
                 performSplit(sss,*leftN,left1,right1,q1,q2);
                 leftN = &left1;
              } else if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
                 rightN->splitAt(leftN->getX2(),leftN->getY2(),left1,right1);
                 performSplit(sss,*rightN,left1,right1,q1,q2);
                 rightN = &left1;
              } else if(leftN->crosses(*rightN)){
                 leftN->splitCross(*rightN,left1,right1,left2,right2);
                 performSplit(sss,*leftN,left1,right1,q1,q2);
                 leftN = &left1;
                 performSplit(sss,*rightN,left2,right2,q1,q2);
                 rightN = &left2;  
              } else { // forgotten case 
                 assert(false);
              }
           }
          // update dominating point information 
          int cover = leftN?leftN->con_above:0;
          Point domPoint = nextHs.GetDomPoint();
          updateDomPointInfo_Line_Region(lastDomPoint, domPoint,
                                         lastDomPointCount1,
                                         lastDomPointCount2,
                                         cover, member->getOwner(), res,
                                         useCluster,cluster, done);
          // detect intersections
          switch(member->getOwner()){
            case first : if(member->con_above==0){ // the line
                            SetIE(res,useCluster,cluster,done);
                         } else {
                            SetII(res,useCluster,cluster,done);
                         }
                         break;
            case second: SetEB(res,useCluster,cluster,done);
                         break;
            case both  : SetIB(res,useCluster,cluster,done);
                         break;
            default    : assert(false);
          }

        }
     }
 } 

 // sweep done, check the last dominating point
 Point domPoint(lastDomPoint);
 domPoint.Translate(100,0);
 updateDomPointInfo_Line_Region(lastDomPoint,domPoint,
                                lastDomPointCount1, lastDomPointCount2,
                                0, both, res, useCluster,cluster,done);
 
 if(useCluster){
     return cluster.Contains(res);
 } else {
     return true;
 }
}


/*
8 Integrating Operators in Secondo 


8.1 Type Mapping Functions 

8.1.1 IsSpatialType

This function checks whether the type given as a ListExpr is one of
point, points, line, or region.

*/

inline bool IsSpatialType(ListExpr type){
   if(!nl->IsAtom(type)){
      return false;
   }
   if(!nl->AtomType(type)==SymbolType){
      return false;
   }
   string t = nl->SymbolValue(type);
   if(t=="point") return true;
   if(t=="points") return true;
   if(t=="line") return true;
   if(t=="region") return true;
   return false;
}


/*
8.1.2 TopPredTypeMap

This function is the type mapping for the toppred operator.
The signature of this operator is:
    t1 x t2 x cluster -> bool

where t1, t2 in {point, points, line, region}.


*/
ListExpr TopPredTypeMap(ListExpr args){

   if(nl->ListLength(args)!=3){
      ErrorReporter::ReportError("three arguments required");
      return nl->TypeError();
   }
   ListExpr cl = nl->Third(args);
   if(!nl->IsEqual(cl,"cluster")){
       ErrorReporter::ReportError("the third argument must" 
                                  " be of type cluster\n");
       return nl->TypeError();
   }
   ListExpr o1 = nl->First(args);
   ListExpr o2 = nl->Second(args);
   if(!IsSpatialType(o1)){
      ErrorReporter::ReportError("The first argument must be a spatial type");
      return nl->TypeError();
   }   
   if(!IsSpatialType(o2)){
      ErrorReporter::ReportError("The second argument"
                                 " must be a spatial type");
      return nl->TypeError();
   }  
   
   return nl->SymbolAtom("bool");
}


/*
8.1.3 TopRelTypeMap

This function is the Type mapping for the toprel operator.
The signature is t1 x t2 -> int9m
where t1, t2 in {point, points, line, region}

*/

ListExpr TopRelTypeMap(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("two arguments expected");
      return nl->SymbolAtom("typeerror");
   }
   if(!IsSpatialType(nl->First(args)) 
      || !IsSpatialType(nl->Second(args))){
       ErrorReporter::ReportError("Spatial types expected");
       return (nl->SymbolAtom( "typeerror" ));
   }
   return nl->SymbolAtom("int9m");
}


#ifdef  TOPOPS_USE_STATISTIC
/*
8.1.4  ResetStatisticTypeMap

The signature is: -> bool

*/
ListExpr TopOpsResetStatTypeMap(ListExpr args){
   if(nl->ListLength(args)!=0){
      ErrorReporter::ReportError("no argument expected");
      return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("bool");
}

/*
8.1.5  GetStatisticTypeMap

The signature is: -> text

*/

ListExpr TopOpsGetStatTypeMap(ListExpr args){
   if(nl->ListLength(args)!=0){
      ErrorReporter::ReportError("no argument expected");
      return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("text");
}
#endif




/*
8.2 Value Mappings 

8.2.1 ~TopRel~

This value mapping can be instantiated with all combinations
where a function ~getInt9M~ exists for the type combination
~type1~ x ~type2~.

*/
template<class type1, class type2>
int TopRel(Word* args, Word& result, int message,
           Word& local, Supplier s){
  result = qp->ResultStorage(s);
  type1* p1 = static_cast<type1*>(args[0].addr);
  type2* p2 = static_cast<type2*>(args[1].addr);
  Int9M matrix; 
  GetInt9M(p1,p2,matrix);
  *(static_cast<Int9M*>(result.addr)) = matrix; 
  return 0;
}

/*
This function is symmetric to the ~TopRel~ functions. This function avoids
the implementation of symmetric GetInt9M functions.

*/
template<class type1, class type2>
int TopRelSym(Word* args, Word& result, int message,
           Word& local, Supplier s){

  result = qp->ResultStorage(s);
  type1* p1 = static_cast<type1*>(args[0].addr);
  type2* p2 = static_cast<type2*>(args[1].addr);
  Int9M matrix(0);
  GetInt9M(p2,p1,matrix);
  matrix.Transpose();
  *(static_cast<Int9M*>(result.addr))=matrix; // correct the swapping 
  return 0;
}


/*
8.2.2  Statistical Information

The next two functions implement the value mapping for the
statistical infomation operators.

*/

#ifdef TOPOPS_USE_STATISTIC
int TopOpsResetStatVM(Word* args, Word& result, int message, 
                    Word& local, Supplier s){
   result = qp->ResultStorage(s);
   ResetStatistic();
   (static_cast<CcBool*>(result.addr))->Set(true,true);
   return 0;
}

int TopOpsGetStatVM(Word* args, Word& result, int message, 
                    Word& local, Supplier s){
   result = qp->ResultStorage(s);
   stringstream ts;
   ts << "#GetInt9M(point, point): " << GetCalls_p_p << endl
      << "#BoxFilter(point, point): " << bb_p_p << endl
      << "#GetInt9M(points, point): " << GetCalls_ps_p << endl
      << "#BoxFilter(points, point): " << bb_ps_p << endl
      << "#GetInt9M(points, points): " << GetCalls_ps_ps << endl
      << "#BoxFilter(points, points): " << bb_ps_ps << endl
      << "#GetInt9M(line, point): " << GetCalls_l_p << endl
      << "#BoxFilter(line, point): " << bb_l_p << endl
      << "#GetInt9M(line, points): " << GetCalls_l_ps << endl
      << "#BoxFilter(line, points): " << bb_l_ps << endl
      << "#GetInt9M(line, line): " << GetCalls_l_l << endl
      << "#BoxFilter(line, line): " << bb_l_l << endl
      << "#GetInt9M(region, point): " << GetCalls_r_p << endl
      << "#BoxFilter(region, point): " << bb_r_p << endl
      << "#GetInt9M(region, points): " << GetCalls_r_ps << endl
      << "#BoxFilter(region, points): " << bb_r_ps << endl
      << "#GetInt9M(region,region) : " << GetCalls_r_r << endl
      << "#BoxFilter(region,region) : " << bb_r_r << endl;
   (static_cast<FText*>(result.addr)) ->Set(true,ts.str().c_str());
   return 0;
}

#endif

/*
8.2.3 Value Mapping for the ~toppred~ operator

The following value mappings are specialized versions 
where an early exit may advantageous.

*/
template<class t1, class t2>
int TopPred(Word* args, Word& result, int message,
                    Word& local, Supplier s){
    
  result = qp->ResultStorage(s);
  t1* v1 = static_cast<t1*>(args[0].addr);
  t2* v2 = static_cast<t2*>(args[1].addr);
  Cluster* cluster = static_cast<Cluster*>(args[2].addr);
  if(!cluster->IsDefined()){// name not found within group
      (static_cast<CcBool*>(result.addr))->Set(false,false);
      return 0;
  }

  Int9M matrix;
  bool res = GetInt9M(v1,v2,matrix,true,*cluster);
  (static_cast<CcBool*>(result.addr))->Set(true,res);
  return 0;
}

/*
As for the ~toprel~ opererator there is a symmetric value mapping.

*/
template<class t1, class t2>
int TopPredSym(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result = qp->ResultStorage(s);
  t1* v1 = static_cast<t1*>(args[0].addr);
  t2* v2 = static_cast<t2*>(args[1].addr);
  Cluster* cluster = static_cast<Cluster*>(args[2].addr);
  if(!cluster->IsDefined()){// name not found within group
      (static_cast<CcBool*>(result.addr))->Set(false,false);
      return 0;
  }
  Int9M matrix;
  Cluster tmp(cluster,true);
  bool res = GetInt9M(v2,v1,matrix,true,tmp);
  (static_cast<CcBool*>(result.addr))->Set(true,res);
  return 0;
}



template<class t1, class t2>
int TopPredSym2(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  result = qp->ResultStorage(s);
  t1* v1 = static_cast<t1*>(args[0].addr);
  t2* v2 = static_cast<t2*>(args[1].addr);
  Cluster* cluster = static_cast<Cluster*>(args[2].addr);
  if(!cluster->IsDefined()){// name not found within group
      (static_cast<CcBool*>(result.addr))->Set(false,false);
      return 0;
  }
  Int9M matrix;
  bool res = GetInt9M(v2,v1,matrix,true,*cluster,true);
  (static_cast<CcBool*>(result.addr))->Set(true,res);
  return 0;
}



/*
8.3  Operator specifications

*/


const string TopRelSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> {point, points, line, region} x "
   "  {points, points, line, region} -> int9m </text--->"
   " \" toprel(_ _) \" "
   " <text>computes the topological relationship of the arguments</text--->"
    "  \" query toprel(reg1, reg2) \" ))";

const string TopPredSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> so1 x so2 x cluster -> bool "
   " where o1, o2 in {point, points, line, region}</text--->"
   " \" topred(_ _ _) \" "
   " <text> checks whether the topological relationship between"
   " the spatial objects is part of the cluster </text---> "
    "  \" query toppred(c1,c2) \" ))";

#ifdef TOPOPS_USE_STATISTIC
const string TopOpsResetStatSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> -> bool </text--->"
   " \" topops_reset_stat \" "
   "  \" sets statistical information to initial values \" "
    "  \" query topops_reset_stat() \" ))";

const string TopOpsGetStatSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> -> text </text--->"
   " \" topops_get_stat() \" "
   "  \" gets statistical information of this algebra \" "
    "  \" query topops_get_stat() \" ))";

#endif

/*
8.4 Value Mapping Arrays

The following arrays collect the value mappings to enable overloaded 
operations.

*/

ValueMapping TopRelMap[] = {
       TopRel<Point,Point> , TopRel<Points,Point>,
       TopRelSym<Point,Points>, TopRel<Points,Points>, TopRel<Line,Point>,
       TopRelSym<Point,Line>,TopRel<Line,Points>,TopRelSym<Points,Line>,
       TopRel<Region,Point>,TopRelSym<Point,Region>,
       TopRel<Region,Points>, TopRelSym<Points,Region>,TopRel<Region,Region>,
       TopRel<Line,Line>, TopRel<Line,Region>};

ValueMapping TopPredMap[] = {
       TopPred<Point,Point> , 
       TopPred<Points,Point>,
       TopPredSym<Point,Points>,
       TopPred<Points,Points>, 
       TopPred<Line,Point>,
       TopPredSym<Point,Line>,
       TopPred<Line,Points>,
       TopPredSym<Points,Line>,
       TopPred<Region,Point>,
       TopPredSym2<Point,Region>,
       TopPred<Region,Points>,
       TopPredSym<Points,Region>,
       TopPred<Region,Region>,
       TopPred<Line,Line>, 
       TopPred<Line,Region> };


/*
8.5 Selection Function

The value mapping array containg the value mapping functions for both
operator in the same order. For this reason it is sufficient to have
one single selection function for both functions.

*/

static int TopOpsSelect(ListExpr args){
   string type1 = nl->SymbolValue(nl->First(args));
   string type2 = nl->SymbolValue(nl->Second(args));
   
  if( (type1=="point") && (type2=="point")){
      return 0;
   }
   if( (type1=="points") && (type2=="point")){
      return 1;
   }
   if((type1=="point") && (type2=="points")){
      return 2;
   }
   if((type1=="points") && (type2=="points")){
      return 3;
   }
   if((type1=="line") && (type2=="point")){
       return 4;
   }
   if((type1=="point") && (type2=="line")){
       return 5;
   }
   if((type1=="line") && (type2=="points")){
       return 6;
   }
   if((type1=="points") && (type2=="line")){
       return 7;
   }
   if((type1=="region") && (type2=="point")){
       return 8;
   }
   if((type1=="point") && (type2=="region")){
       return 9;
   }
   if( type1=="region" && (type2=="points")){
       return  10;
   }
   if(type1=="points" && (type2=="regions")){
       return 11;
   }
   if(type1=="region" && (type2=="region")){
       return 12;
   }
   if( (type1=="line") && (type2=="line")){
       return 13;
   }
   if( (type1=="line") && (type2=="region")){
       return 14;
   }

   return -1;
} 


/*
8.6 Definition of operators

In this section instances of the algebra operators are build.

*/
Operator optoprel(
        "toprel",     // name
         TopRelSpec,   // specification
         sizeof(TopRelMap)/sizeof(ValueMapping),  // number of functions
         TopRelMap,    // array of value mappings
         TopOpsSelect,
         TopRelTypeMap
         );


Operator toppred(
        "toppred",     // name
         TopPredSpec,   // specification
         sizeof(TopPredMap)/sizeof(ValueMapping),  // number of functions
         TopPredMap,    // array of value mappings
         TopOpsSelect,
         TopPredTypeMap
         );

#ifdef TOPOPS_USE_STATISTIC
Operator topops_reset_stat(
     "topops_reset_stat",           //name
     TopOpsResetStatSpec,   //specification
     TopOpsResetStatVM, //value mapping
     Operator::SimpleSelect,         //trivial selection function
     TopOpsResetStatTypeMap //type mapping
);
Operator topops_get_stat(
     "topops_get_stat",           //name
     TopOpsGetStatSpec,   //specification
     TopOpsGetStatVM, //value mapping
     Operator::SimpleSelect,         //trivial selection function
     TopOpsGetStatTypeMap //type mapping
);
#endif



/*
8.7 Creating the algebra

*/
class TopOpsAlgebra : public Algebra {
  public:
     TopOpsAlgebra() : Algebra() {
        AddOperator(&optoprel);
        AddOperator(&toppred);
#ifdef TOPOPS_USE_STATISTIC
        AddOperator(&topops_reset_stat);
        AddOperator(&topops_get_stat);
#endif
      }
     ~TopOpsAlgebra(){}
} topOpsAlgebra;

/*
8.8 Initialization of the Algebra
   
*/
extern "C"
Algebra* InitializeTopOpsAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
    nl = nlRef;
    qp = qpRef;
#ifdef TOPOPS_USE_STATISTIC
    ResetStatistic();
#endif
    return (&topOpsAlgebra);
}

