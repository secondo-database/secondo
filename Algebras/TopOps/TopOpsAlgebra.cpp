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
Additionally, this algebra provides some topological predicates using the
standard predicate group. 

Moreover, in this algebra are set operations for spatial types (union2, 
intersection2, difference2, commonborder2) implemented.


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
        case first: { 
             insideAbove_first = hs->GetAttr().insideAbove; 
             insideAbove_second = false;
             break;
        } case second: {
             insideAbove_second = hs->GetAttr().insideAbove; 
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
       return (!AlmostEqual(y,y1) && (y>y1) &&
               !AlmostEqual(y,y2) && (y<y2));
     }
     double ys = getY(x);
     return AlmostEqual(y,ys);
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
     if(AlmostEqual(x1,x2)){ // vertical segment
        return (y>=y1) && (y <= y2);
     } 
     // check if (x,y) is located on the line 
     double res1 = (x-x1)*(y2-y1);
     double res2 = (y-y1)*(x2-x1);
     if(!AlmostEqual(res1,res2)){
         return false;
     }

     return ((x>x1) && (x<x2)) ||
            AlmostEqual(x,x1) ||
            AlmostEqual(x,x2);
   }

/*
3.6 Comparison

Compares this with s. The x intervals must overlap.

*/

 int compareTo(const AVLSegment& s) const{
 
    if(!xOverlaps(s)){
      cerr << "Warning: compare AVLSegments with disjoint x intervals" << endl;
      cerr << "This may be a problem of roundig errors!" << endl;
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

2) the owner of this and s must be different 

*/

  int split(const AVLSegment& s, AVLSegment& left, AVLSegment& common, 
            AVLSegment& right) const{

     assert(overlaps(s));
     assert( (this->owner==first && s.owner==second) ||
             (this->owner==second && s.owner==first));


     int result = 0;



     int cmp = comparePoints(x1,y1,s.x1,s.y1);
     if(cmp==0){
        left.x1 = x1;
        left.y1 = y1;
        left.x2 = x1;
        left.y2 = y1; 
     } else { // there is a left part
       result = result | LEFT;
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
    common.con_above = con_above;
    common.con_below = con_below;
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

    result = result | RIGHT;
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

     if(!xContains(x)){
       cerr << "Warning: compute y value for a x outside the x interval!" 
            << endl;
       double diff1 = x1 - x;
       double diff2 = x - x2;
       cerr << "difference to x is " << (diff1>diff2?diff1:diff2) << endl;
     }
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
           break;
      } case second:{
           if(createLeft){
              q2.push(seg.convertToHs(true, second));
           }
           if(createRight){
              q2.push(seg.convertToHs(false, second));
           }
           break;
      } case both : { 
           if(createLeft){
              q1.push(seg.convertToHs(true, first));
              q2.push(seg.convertToHs(true, second));
           }
           if(createRight){
              q1.push(seg.convertToHs(false, first));
              q2.push(seg.convertToHs(false, second));
           }
           break;
      } default: {
           assert(false);
      }
   }
}


/*
~splitByNeighbour~


neighbour has to be an neighbour from current within sss.

*/

void splitByNeighbour(AVLTree<AVLSegment>& sss,
                      AVLSegment& current,
                      AVLSegment const*& neighbour,
                      priority_queue<HalfSegment,  
                                     vector<HalfSegment>, 
                                     greater<HalfSegment> >& q1,
                      priority_queue<HalfSegment,  
                                     vector<HalfSegment>, 
                                     greater<HalfSegment> >& q2){
    AVLSegment left1, right1, left2, right2;

    if(neighbour && !neighbour->innerDisjoint(current)){
       if(neighbour->ininterior(current.getX1(),current.getY1())){
          neighbour->splitAt(current.getX1(),current.getY1(),left1,right1);
          sss.remove(*neighbour);
          neighbour = sss.insert2(left1);
          insertEvents(left1,false,true,q1,q2);
          insertEvents(right1,true,true,q1,q2);
       } else if(neighbour->ininterior(current.getX2(),current.getY2())){
          neighbour->splitAt(current.getX2(),current.getY2(),left1,right1);
          sss.remove(*neighbour);
          neighbour = sss.insert2(left1);
          insertEvents(left1,false,true,q1,q2);
          insertEvents(right1,true,true,q1,q2);
       } else if(current.ininterior(neighbour->getX2(),neighbour->getY2())){
          current.splitAt(neighbour->getX2(),neighbour->getY2(),left1,right1);
          current = left1; 
          insertEvents(left1,false,true,q1,q2);
          insertEvents(right1,true,true,q1,q2);
       } else if(current.crosses(*neighbour)){
          neighbour->splitCross(current,left1,right1,left2,right2);
          sss.remove(*neighbour);
          neighbour = sss.insert2(left1);
          current = left2;
          insertEvents(left1,false,true,q1,q2);
          insertEvents(right1,true,true,q1,q2);
          insertEvents(left2,false,true,q1,q2);
          insertEvents(right2,true,true,q1,q2);
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
             insertEvents(current,false,true,q1,q2);
             return;  

          }
          if(current.ininterior(neighbour->getX2(),neighbour->getY2())){
            cerr << " 5 : current.ininterior(neighbour->getX2(),"
                 << "neighbour->getY2())" << endl;
          }
          if(current.crosses(*neighbour)){
             cerr << "6 : crosses" << endl;
          }
          assert(false);
       }
    } 
}


/*
~splitNeighbours~

checks if the left and the right neighbour are intersection in their
interiors and permorms the required actions.


*/

void splitNeighbours(AVLTree<AVLSegment>& sss,
                     AVLSegment const*& leftN,
                     AVLSegment const*& rightN,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q1,
                     priority_queue<HalfSegment,  
                                    vector<HalfSegment>, 
                                    greater<HalfSegment> >& q2){
  if(leftN && rightN && !leftN->innerDisjoint(*rightN)){
    AVLSegment left1, right1, left2, right2;
    if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
       leftN->splitAt(rightN->getX2(),rightN->getY2(),left1,right1);
       sss.remove(*leftN);
       leftN = sss.insert2(left1);
       insertEvents(left1,false,true,q1,q2);
       insertEvents(right1,true,true,q1,q2);
    } else if(rightN->ininterior(leftN->getX2(),leftN->getY2())){
       rightN->splitAt(leftN->getX2(),leftN->getY2(),left1,right1);
       sss.remove(*rightN);
       rightN = sss.insert2(left1);
       insertEvents(left1,false,true,q1,q2);
       insertEvents(right1,true,true,q1,q2);
    } else if (rightN->crosses(*leftN)){
         leftN->splitCross(*rightN,left1,right1,left2,right2);
         sss.remove(*leftN);
         sss.remove(*rightN);
         leftN = sss.insert2(left1);
         rightN = sss.insert2(left2);
         insertEvents(left1,false,true,q1,q2);
         insertEvents(left2,false,true,q1,q2);
         insertEvents(right1,true,true,q1,q2);
         insertEvents(right2,true,true,q1,q2);
    } else { // forgotten case
       assert(false);
    }
  } // intersecting neighbours
}



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
  // initialization
  res.SetValue(0);
  res.SetEE(true); // holds always for bounded objects
  
  // check for emptyness
  if(ps->IsEmpty()){ // the simples case
     res.SetEI(true);

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
     if(useCluster){
        return cluster.Contains(res);
     } else {
        return true;
     }
  }

  int size = ps->Size();
  if(size>1){
     res.SetIE(true);
     if(useCluster){
        cluster.Restrict(IE,true);
        if(cluster.IsEmpty()){
           return false;
        }
     }
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
 
   int n1 = ps1->Size();
   int n2 = ps2->Size();

   res.SetValue(0);
   res.SetEE(true);

   if(n1<=0 && n2<=0){
      // there are no inner parts which can intersect any part
     if(useCluster){
       return cluster.Contains(res);
     } else {
       return true;
     }
   }
   if(n1<=0){
      // some points of ps2 are in the exterior of ps1
      res.SetEI(true);
      if(useCluster){
         return cluster.Contains(res);
      } else {
         return true;
      }
   }
   if(n2<=0){
      // symmetrically to the previous case
      res.SetIE(true);
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
   AVLSegment tmpL,tmpR;  

   while(!done &&  
         ((owner=selectNext(line,q,posline,point,
           posPoint,resHs,resPoi))!=none)){

      if(owner==second){ // event comes from the point(s) value
         AVLSegment current(&resPoi,second);

         const AVLSegment* leftN=0;
         const AVLSegment* rightN=0;
         const AVLSegment* member= sss.getMember(current,leftN,rightN);
         if(leftN){
            tmpL = *leftN;
            leftN = &tmpL;
         }
         if(rightN){
            tmpR = *rightN;
            rightN = &tmpR;
         }


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
        if(leftN){
           tmpL = *leftN;
           leftN = &tmpL;
        }
        if(rightN){
           tmpR = *rightN;
           rightN = &tmpR;
        }
 
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
             splitByNeighbour(sss,current,leftN,q,q);
             splitByNeighbour(sss,current,rightN,q,q);
             sss.insert(current);
           } // no overlapping segment
       } else { // right event

           AVLSegment left1, left2, right1, right2;

           if(member && member->exactEqualsTo(current)){ // segment found

              sss.remove(current);
              splitNeighbours(sss,leftN,rightN,q,q);
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

If the region and the points value are already processed, the return 
value will be ~none~.

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
    case 0: { 
       return none;
    } case 1: { // region
       pos1++;
       resultHS = *minhs;
       return first;
    } case 2: { // queue
       q1.pop();
       resultHS = *minhs;
       return first;
    } case 3: {  // point
       pos2++;
       resultPoint = *cp;
       return second;
    } default: {
        assert(false);
        return none; 
    }
  }
}


bool GetInt9M(Region const* const reg, Points const* const ps, Int9M& res,
              const bool useCluster=false,
              Cluster cluster = Cluster()){
  res.SetValue(0); 
  // test for emptyness
   res.SetEE(true);
   if(reg->IsEmpty()){
      if(ps->IsEmpty()){
       if(useCluster){
          return cluster.Contains(res);
       } else {
         return true; 
       }
      }
      res.SetEI(true);
      if(useCluster){
        return cluster.Contains(res);;
      } else {
        return true;  
      }
   } 
   res.SetIE(true);
   res.SetBE(true);

   if(ps->IsEmpty()){ // no more intersections can be found
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
  AVLSegment tmpL,tmpR;

  while (!done && ( (owner= selectNext(reg,q1,pos1, ps,pos2,CH,CP))!=none)){
    if(owner==second){ // the point
       AVLSegment current(&CP,second);
       const AVLSegment* left=0;
       const AVLSegment* right=0;
       const AVLSegment* member = sss.getMember(current, left, right);
       if(left){
          tmpL = *left;
          left = &tmpL;
       }
       if(right){
          tmpR = *right;
          right = &tmpR;
       }
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
      if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
      }
      if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
      }
      if(CH.IsLeftDomPoint()){ // left Event
         assert(!member); // a single region can't contain overlapping segments
         splitByNeighbour(sss,current,leftN,q1,q1);
         splitByNeighbour(sss,current,rightN,q1,q1);
         sss.insert(current);
      } else { // right event
        if(member && member->exactEqualsTo(current)){
           sss.remove(current);
           splitNeighbours(sss,leftN,rightN,q1,q1);
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
  res.SetValue(0);;
  res.SetEE(true);
  // check for emptyness
  if(reg1->IsEmpty()){
    if(reg2->IsEmpty()){ // no more intersection possible
       if(useCluster){
           return cluster.Contains(res); 
       } else {
           return true;
       }
    }else{
      res.SetEI(true);
      res.SetEB(true);
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
  AVLSegment tmpL,tmpR;

  while( ((owner=selectNext(reg1,pos1, reg2,pos2, q1,q2,nextSeg,src))!=none)
          && !done){

    AVLSegment current(&nextSeg,owner);

    member = sss.getMember(current,leftN,rightN);
    if(leftN){
       tmpL = *leftN;
       leftN = &tmpL;
    }
    if(rightN){
       tmpR = *rightN;
       rightN = &tmpR;
    }

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
              splitByNeighbour(sss,current,leftN,q1,q2);
              splitByNeighbour(sss,current,rightN,q1,q2);
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
           if(leftN->crosses(*rightN)){
              assert(leftN->getOwner() != rightN->getOwner()); 
              res.Fill(); // we are done
              if(useCluster){ 
                 return cluster.Contains(res);
              } else {
                 return true;
              }
           }
           splitNeighbours(sss,leftN,rightN,q1,q2);
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

void updateDomPoints_Line_Line(
              Point& lastDomPoint, const Point& newDomPoint,
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

 if(!line1->IsEmpty() && !line2->IsEmpty()){ 
    if(!line1->BoundingBox().Intersects(line2->BoundingBox()) && useCluster){
        Int9M m(false,false,true,false,false,true,true,true,true);
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
 AVLSegment tmpL,tmpR;



 while(!done && 
       ((owner=selectNext(line1,pos1,line2,pos2,q1,q2,nextHs,src))!=none) ){
   AVLSegment current(&nextHs,owner);

// debug::start
   // cout << "process segment " << current << "  "
   //      << (nextHs.IsLeftDomPoint()?"LEFT":"RIGHT") << endl;
// debug::end


   // try to find an overlapping segment in sss
   member = sss.getMember(current,leftN,rightN);
   if(leftN){
      tmpL = *leftN;
      leftN = &tmpL;
   }
   if(rightN){
      tmpR = *rightN;
      rightN = &tmpR;
   }
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
           ownertype owner2 = owner;
           if(parts  & LEFT){
              owner2 = both;
           }
           updateDomPoints_Line_Line(lastDomPoint,newDomPoint, 
                          lastDomPointCount1,lastDomPointCount2,owner2, res,
                          useCluster, cluster, done);
           
        } 
      } else { // no overlapping segment stored in sss
        splitByNeighbour(sss,current,leftN,q1,q2);
        splitByNeighbour(sss,current,rightN,q1,q2);
        updateDomPoints_Line_Line(lastDomPoint,nextHs.GetDomPoint(),
                        lastDomPointCount1, lastDomPointCount2, 
                         owner,res,useCluster,cluster,done);
        bool ok = sss.insert(current); 
        assert(ok);
      }

   } else { // right Event
     // check if current is stored in sss
     if(member && member->exactEqualsTo(current)){

        Point newDomPoint = nextHs.GetDomPoint();
        
        updateDomPoints_Line_Line(lastDomPoint,newDomPoint, 
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
        splitNeighbours(sss,leftN,rightN,q1,q2);
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
 updateDomPoints_Line_Line(lastDomPoint, newDP, 
                 lastDomPointCount1, lastDomPointCount2, 
                 first,res,useCluster,cluster,done);

 if(useCluster){
    return cluster.Contains(res);
 } else {
    return true;
 }

} // line x line


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
                                    int& lastCoverage_Num,
                                    const int newCoverageNum, 
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
           if(lastCoverage_Num==0){
              SetBE(res,useCluster,cluster,done);
           } else {
              SetBI(res,useCluster,cluster,done);
           }
         }
       } else { // interior of the line
         if(count_region > 0){
            SetIB(res,useCluster,cluster,done);
         } else {
           if(lastCoverage_Num==0){
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
    lastCoverage_Num = newCoverageNum;
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

  Rectangle<2> bbox1 = line->BoundingBox();
  Rectangle<2> bbox2 = region->BoundingBox();

  if(!bbox1.Intersects(bbox2)){
      res.SetIE(true); // line is not empty
      if(!region->IsEmpty()){
        res.SetEB(true);
      }
      if(useCluster){
          Int9M m(false,false,true, false, false, true, true,true,true);
          cluster.Restrict(m,false);
      }
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
 AVLSegment tmpL,tmpR;

 int src;
 int lastCoverageNum = 0;

 // plane sweep
 while(!done && 
       ((owner=selectNext(line,pos1,region,pos2,q1,q2,nextHs,src))!=none)){
     AVLSegment current(&nextHs,owner);

     member = sss.getMember(current,leftN,rightN);
     if(leftN){
        tmpL = *leftN;
        leftN = &tmpL;
     }
     if(rightN){
        tmpR = *rightN;
        rightN = &tmpR;
     }
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
            updateDomPointInfo_Line_Region(lastDomPoint, domPoint,
                                           lastDomPointCount1, 
                                           lastDomPointCount2,
                                           lastCoverageNum,current.con_below,
                                           owner2,res, 
                                           useCluster, cluster, done);
         }
       } else { // no overlapping segment found
          // check if current or an existing segment must be divided
          splitByNeighbour(sss,current,leftN,q1,q2);
          splitByNeighbour(sss,current,rightN,q1,q2);
          // update coverage numbers
          if(owner==second){ // the region
            bool iac = current.getInsideAbove();
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
          } else { // the line
            if(leftN){
               if(current.extends(*leftN)){
                  current.con_below = leftN->con_below;
                  current.con_above = leftN->con_above;
               } else if(leftN->isVertical()){
                  current.con_below = leftN->con_below;
               } else {
                  current.con_below = leftN->con_above;
               }
            } else { // no left neighbour found
               current.con_below = 0;
            }
            current.con_above = current.con_below; 
          }


          // update dominating points
          Point domPoint = nextHs.GetDomPoint();
          updateDomPointInfo_Line_Region(lastDomPoint, domPoint,
                                        lastDomPointCount1,
                                        lastDomPointCount2,
                                        lastCoverageNum,
                                        current.con_below,
                                        owner2,
                                        res, useCluster, cluster, done);
          sss.insert(current);
       }
     } else { // right event
        if(member && member->exactEqualsTo(current)){
           AVLSegment tmp = *member;
           sss.remove(*member);
           member = &tmp;
           splitNeighbours(sss,leftN,rightN,q1,q2); 
           // update dominating point information 
           Point domPoint = nextHs.GetDomPoint();
           updateDomPointInfo_Line_Region(lastDomPoint, domPoint,
                                         lastDomPointCount1,
                                         lastDomPointCount2,
                                         lastCoverageNum,
                                         member->con_below,
                                         member->getOwner(), res,
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
                                lastCoverageNum, 0, both, res, useCluster,
                                cluster,done);
 
 if(useCluster){
     return cluster.Contains(res);
 } else {
     return true;
 }
}




/*
8 ~Realminize~

This function converts Line givens as source into a realminized version
stored in ~result~.



*/
ownertype selectNext(const Line& src, int& pos,
                            priority_queue<HalfSegment,  
                                           vector<HalfSegment>, 
                                           greater<HalfSegment> >& q,
                            HalfSegment& result){

 int size = src.Size();
 if(size<=pos){
    if(q.empty()){
      return none;
    } else {
      result = q.top();
      q.pop();
      return first;
    }
 } else {
   const HalfSegment* hs;
   src.Get(pos,hs);
   if(q.empty()){
      result = *hs;      
      pos++;
      return first;
   } else{
      HalfSegment hsq = q.top();
      if(hsq<*hs){
         result = hsq;
         q.pop();
         return first;
      } else {
         pos++;
         result = *hs;
         return first;
      }
   }
 }
}


void Realminize2(const Line& src, Line& result){

  result.Clear();
  if(!src.IsDefined()){
     result.SetDefined(false);
     return;
  }
  result.SetDefined(true);
  if(src.Size()==0){ // empty line, nothing to realminize
    return;
  }

  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q;
  AVLTree<AVLSegment> sss;

  int pos = 0;

  HalfSegment nextHS;
  const AVLSegment* member=0;
  const AVLSegment* leftN  = 0;
  const AVLSegment* rightN = 0;

  AVLSegment left1, right1,left2,right2;
  
  result.StartBulkLoad();
  int edgeno = 0;
  AVLSegment tmpL,tmpR;


  while(selectNext(src,pos,q,nextHS)!=none) {
      AVLSegment current(&nextHS,first);
      member = sss.getMember(current,leftN,rightN);
      if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
      }
      if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
      }
      if(nextHS.IsLeftDomPoint()){
         if(member){ // overlapping segment found in sss
            double xm = member->getX2();
            double xc = current.getX2();
            if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
               current.splitAt(xm,member->getY2(),left1,right1);
               insertEvents(right1,true,true,q,q);
            }
         } else { // no overlapping segment found
            splitByNeighbour(sss,current,leftN,q,q);
            splitByNeighbour(sss,current,rightN,q,q);
            sss.insert(current);
         }
      } else {  // nextHS rightDomPoint
          if(member && member->exactEqualsTo(current)){
             // insert the halfsegments
             HalfSegment hs1 = current.convertToHs(true);
             HalfSegment hs2 = current.convertToHs(false);
             hs1.attr.edgeno = edgeno;
             hs2.attr.edgeno = edgeno;
             result += hs1;
             result += hs2;
             splitNeighbours(sss,leftN,rightN,q,q);
             edgeno++;
             sss.remove(*member);
          }
      }      
  }
  result.EndBulkLoad();
} // Realminize2




/*
9 Set Operations


The following functtion implement the operations union, intersection and
difference for some combinations of spatial types.


9.1 Definition of possible opperations

*/

enum SetOperation{union_op, intersection_op, difference_op};


/*
9.2 ~line~ [x] ~line~ [->] ~line~

This combination can be used for all possible set operations.


*/

void SetOp(const Line& line1,
           const Line& line2,
           Line& result,
           SetOperation op){

   result.Clear();
   if(!line1.IsDefined() || !line2.IsDefined()){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(line1.Size()==0){
       switch(op){
         case union_op : result = line2;
                         return;
         case intersection_op : return; // empty line
         case difference_op : return; // empty line
         default : assert(false);
       }
   }
   if(line2.Size()==0){
      switch(op){
         case union_op: result = line1;
                        return;
         case intersection_op: return;
         case difference_op: result = line1;
                             return;
         default : assert(false);
      }
   }


  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;
  AVLTree<AVLSegment> sss;
  ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  HalfSegment nextHs;
  int src = 0;

  const AVLSegment* member=0;
  const AVLSegment* leftN = 0;
  const AVLSegment* rightN = 0;

  AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(&line1,pos1,&line2,pos2,q1,q2,nextHs,src))!=none){
       AVLSegment current(&nextHs,owner);
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
          if(member){ // found an overlapping segment
             if(member->getOwner()==current.getOwner() ||
                member->getOwner()==both){ // same source
                 double xm = member->getX2();
                 double xc = current.getX2();
                 if(!AlmostEqual(xm,xc) && (xm<xc)){ // current extends member
                    current.splitAt(xm,member->getY2(),left1,right1);
                    insertEvents(right1,true,true,q1,q2);
                 }
             }  else { // member and current come from different sources
                 int parts = member->split(current,left1,common1,right1);
                 sss.remove(*member);
                 member = &common1;
                 if(parts & LEFT){
                     sss.insert(left1);
                     insertEvents(left1,false,true,q1,q2);
                 }
                 assert(parts & COMMON);
                 sss.insert(common1);
                 insertEvents(common1,false,true,q1,q2);
                 if(parts & RIGHT){
                    insertEvents(right1,true,true,q1,q2);
                 }
             }
          } else { // no overlapping segment found
            splitByNeighbour(sss,current,leftN,q1,q2);
            splitByNeighbour(sss,current,rightN,q1,q2);
            sss.insert(current);
          }
       } else { // nextHS rightDomPoint
         if(member && member->exactEqualsTo(current)){
             // insert the segments into the result
             switch(op){
                case union_op : { 
                     HalfSegment hs1 = member->convertToHs(true,first);
                     hs1.attr.edgeno = edgeno; 
                     result += hs1;
                     hs1.SetLeftDomPoint(false);
                     result += hs1;
                     edgeno++;
                     break;
                } case intersection_op : {
                     if(member->getOwner()==both){
                        HalfSegment hs1 = member->convertToHs(true,first);
                        hs1.attr.edgeno = edgeno; 
                        result += hs1;
                        hs1.SetLeftDomPoint(false);
                        result += hs1;
                        edgeno++;
                      } 
                      break;
                } case difference_op :{
                      if(member->getOwner()==first){
                        HalfSegment hs1 = member->convertToHs(true,first);
                        hs1.attr.edgeno = edgeno; 
                        result += hs1;
                        hs1.SetLeftDomPoint(false);
                        result += hs1;
                        edgeno++;
                      } 
                      break;
                } default : {
                      assert(false);
                }
             }
             sss.remove(*member);
             splitNeighbours(sss,leftN,rightN,q1,q2);
         }
       }
  } 
  result.EndBulkLoad(true,false,true,true);
} // setop line x line -> line

/*

9.3 ~region~ [x] ~region~ [->] ~region~

*/

void SetOp(const Region& reg1,
           const Region& reg2,
           Region& result,
           SetOperation op){

   result.Clear();
   if(!reg1.IsDefined() || !reg2.IsDefined()){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(reg1.Size()==0){
       switch(op){
         case union_op : result = reg2;
                         return;
         case intersection_op : return; // empty region
         case difference_op : return; // empty region
         default : assert(false);
       }
   }
   if(reg2.Size()==0){
      switch(op){
         case union_op: result = reg1;
                        return;
         case intersection_op: return;
         case difference_op: result = reg1;
                             return;
         default : assert(false);
      }
   }

   if(!reg1.BoundingBox().Intersects(reg2.BoundingBox())){
      switch(op){
        case union_op: {
          result.StartBulkLoad();
          int edgeno=0;
          int s = reg1.Size();
          const HalfSegment* hs;
          for(int i=0;i<s;i++){
              reg1.Get(i,hs);
              if(hs->IsLeftDomPoint()){
                 HalfSegment HS(*hs);
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
              if(hs->IsLeftDomPoint()){
                 HalfSegment HS(*hs);
                 HS.attr.edgeno = edgeno;
                 result += HS;
                 HS.SetLeftDomPoint(false);
                 result += HS;
                 edgeno++;
              }
          }
          result.EndBulkLoad();
          return;
        } case difference_op: {
           result = reg1;
           return; 
        } case intersection_op:{
           return;
        } default: assert(false);
      }
   }

  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;
  AVLTree<AVLSegment> sss;
  ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  HalfSegment nextHs;
  int src = 0;

  const AVLSegment* member=0;
  const AVLSegment* leftN = 0;
  const AVLSegment* rightN = 0;

  AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  while( (owner=selectNext(&reg1,pos1,&reg2,pos2,q1,q2,nextHs,src))!=none){
       AVLSegment current(&nextHs,owner);
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
            assert(member->getOwner()!=both);   
            assert(member->getOwner()!=owner); 
            int parts = member->split(current,left1,common1,right1);
            sss.remove(*member);
            if(parts & LEFT){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
            assert(parts & COMMON);
            // update coverage numbers
            if(current.getInsideAbove()){
               common1.con_above++;
            }  else {
               common1.con_above--;
            }
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
            if(parts & RIGHT){
               insertEvents(right1,true,true,q1,q2);
            }
          } else { // there is no overlapping segment
            // try to split segments if required
            splitByNeighbour(sss,current,leftN,q1,q2);
            splitByNeighbour(sss,current,rightN,q1,q2);


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
            // insert element
            sss.insert(current); 
          }
       } else {  // nextHs.IsRightDomPoint
          if(member && member->exactEqualsTo(current)){
              switch(op){
                case union_op :{
                   if( (member->con_above==0) || (member->con_below==0)) {
                      HalfSegment hs1 = member->getOwner()==both
                                      ?member->convertToHs(true,first)
                                      :member->convertToHs(true);
                      hs1.attr.edgeno = edgeno;
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                   }
                   break;
                }
                case intersection_op: {
                  if(member->con_above==2 || member->con_below==2){
                      HalfSegment hs1 = member->getOwner()==both
                                      ?member->convertToHs(true,first)
                                      :member->convertToHs(true);
                      hs1.attr.edgeno = edgeno;
                      result += hs1;
                      hs1.SetLeftDomPoint(false);
                      result += hs1;
                      edgeno++;
                  } 
                  break;
                }
                case difference_op : {
                  switch(member->getOwner()){
                    case first:{
                      if(member->con_above + member->con_below == 1){
                         HalfSegment hs1 = member->getOwner()==both
                                          ?member->convertToHs(true,first)
                                          :member->convertToHs(true);
                         hs1.attr.edgeno = edgeno;
                         result += hs1;
                         hs1.SetLeftDomPoint(false);
                         result += hs1;
                         edgeno++;
                      }
                      break;
                    }
                    case second:{
                      if(member->con_above + member->con_below == 3){
                         HalfSegment hs1 = member->getOwner()==both
                                          ?member->convertToHs(true,second)
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
                    case both: {
                      if((member->con_above==1) && (member->con_below== 1)){
                         HalfSegment hs1 = member->getOwner()==both
                                          ?member->convertToHs(true,first)
                                          :member->convertToHs(true);
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
              splitNeighbours(sss,leftN,rightN,q1,q2);
          } // current found in sss
       } // right endpoint
  }
  result.EndBulkLoad();
} // setOP region x region -> region



/*
9.4 ~region~ [x] ~line~ [->] ~region~  

This combination can only be used for the operations
union and difference. In both cases, the result will be
the original region value.

*/

void SetOp(const Region& region,
           const Line& line,
           Region& result,
           SetOperation op){

   assert(op == union_op || op == difference_op);
   result.Clear();
   if(!line.IsDefined() || !region.IsDefined()){
      result.SetDefined(false);
      return;
   }
   result.SetDefined(true);
   result.CopyFrom(&region);
}

/*
9.5  ~line~ [x] ~region~ [->] ~line~

Here, only the difference and intersection operation are applicable.


*/
void SetOp(const Line& line,
           const Region& region,
           Line& result,
           SetOperation op){

  assert(op==intersection_op || op == difference_op);

  result.Clear();
  if(!line.IsDefined() || !region.IsDefined()){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(line.Size()==0){ // empty line -> empty result
       switch(op){
         case intersection_op : return; // empty region
         case difference_op : return; // empty region
         default : assert(false);
       }
   }
   if(region.Size()==0){
      switch(op){
         case intersection_op: return; 
         case difference_op: result = line;
                             return;
         default : assert(false);
      }
   }


  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;
  AVLTree<AVLSegment> sss;
  ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  int size1= line.Size();
  HalfSegment nextHs;
  int src = 0;

  const AVLSegment* member=0;
  const AVLSegment* leftN = 0;
  const AVLSegment* rightN = 0;

  AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  AVLSegment tmpL,tmpR;
  bool done = false;

  result.StartBulkLoad();
  // perform a planesweeo
  while( ((owner=selectNext(&line,pos1,&region,pos2,q1,q2,nextHs,src))!=none)
         && ! done){
     AVLSegment current(&nextHs,owner);
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
        if(member){ // there is an overlapping segment in sss
           if(member->getOwner()==owner ||
              member->getOwner()==both     ){
              if(current.ininterior(member->getX2(),member->getY2())){
                 current.splitAt(member->getX2(),member->getY2(),left1,right1);
                 insertEvents(right1,true,true,q1,q2);
              }
           } else { // member and source come from difference sources
             int parts = member->split(current,left1,common1,right1);
             sss.remove(*member);
             member = &common1;
             if(parts & LEFT){
                sss.insert(left1);
                insertEvents(left1,false,true,q1,q2);
             }
             assert(parts & COMMON);
             if(owner==second) {  // the region
               if(current.getInsideAbove()){
                  common1.con_above++;
               } else {
                  common1.con_above--;
               }
             } // for a line is nothing to do
             sss.insert(common1);
             insertEvents(common1,false,true,q1,q2);
             if(parts & RIGHT){
                 insertEvents(right1,true,true,q1,q2);
             }
           }
        } else { // no overlapping segment in sss found
          splitByNeighbour(sss,current,leftN,q1,q2);
          splitByNeighbour(sss,current,rightN,q1,q2); 
          // update coverage numbers
          if(owner==second){ // the region
            bool iac = current.getInsideAbove();
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
          } else { // the line
            if(leftN){
               if(leftN->isVertical()){
                  current.con_below = leftN->con_below;
               } else {
                  current.con_below = leftN->con_above;
               }
            }
            current.con_above = current.con_below; 
          }
          // insert element
          sss.insert(current); 
        }
     } else { // nextHs.IsRightDomPoint()
       if(member && member->exactEqualsTo(current)){

          switch(op){
              case intersection_op: {
                if( (member->getOwner()==both) ||
                    (member->getOwner()==first && member->con_above>0)){
                    HalfSegment hs1 = member->convertToHs(true,first);
                    hs1.attr.edgeno = edgeno;
                    result += hs1;
                    hs1.SetLeftDomPoint(false);
                    result += hs1;
                    edgeno++;
                }
                break;
              }
              case difference_op: {
                if( (member->getOwner()==first) &&  
                    (member->con_above==0)){
                    HalfSegment hs1 = member->convertToHs(true,first);
                    hs1.attr.edgeno = edgeno;
                    result += hs1;
                    hs1.SetLeftDomPoint(false);
                    result += hs1;
                    edgeno++;
                }
                break;
              }
              default : assert(false);
          }
          sss.remove(*member);
          splitNeighbours(sss,leftN,rightN,q1,q2);
       }
       if(pos1>=size1 && q1.empty()){ // line is processed
          done = true;
       }
     }
  }
  result.EndBulkLoad();
} // setOP(line x region -> line)



/*

9.5  ~CommonBorder~


Signature: ~region~ [x] ~region~ [->] line

*/

void CommonBorder(
           const Region& reg1,
           const Region& reg2,
           Line& result){

   result.Clear();
   if(!reg1.IsDefined() || !reg2.IsDefined()){
       result.SetDefined(false);
       return;
   }
   result.SetDefined(true);
   if(reg1.Size()==0 || reg2.Size()==0){
       // a region is empty -> the common border is also empty
       return;
   }
   if(!reg1.BoundingBox().Intersects(reg2.BoundingBox())){
      // no common border possible
      return;
   }

  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q1;
  priority_queue<HalfSegment,  vector<HalfSegment>, greater<HalfSegment> > q2;
  AVLTree<AVLSegment> sss;
  ownertype owner;
  int pos1 = 0;
  int pos2 = 0;
  HalfSegment nextHs;
  int src = 0;

  const AVLSegment* member=0;
  const AVLSegment* leftN = 0;
  const AVLSegment* rightN = 0;

  AVLSegment left1,right1,common1,
             left2,right2;

  int edgeno =0;
  AVLSegment tmpL,tmpR;

  result.StartBulkLoad();
  bool done = false;
  int size1 = reg1.Size();
  int size2 = reg2.Size();

  while( ((owner=selectNext(&reg1,pos1,&reg2,pos2,q1,q2,nextHs,src))!=none)
         && !done  ){
       AVLSegment current(&nextHs,owner);
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
            assert(member->getOwner()!=both);   
            assert(member->getOwner()!=owner); 
            int parts = member->split(current,left1,common1,right1);
            sss.remove(*member);
            if(parts & LEFT){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
            assert(parts & COMMON);
            sss.insert(common1);
            insertEvents(common1,false,true,q1,q2);
            if(parts & RIGHT){
               insertEvents(right1,true,true,q1,q2);
            }
          } else { // there is no overlapping segment
            // try to split segments if required
            splitByNeighbour(sss,current,leftN,q1,q2);
            splitByNeighbour(sss,current,rightN,q1,q2);

            sss.insert(current); 
          }
       } else {  // nextHs.IsRightDomPoint
          if(member && member->exactEqualsTo(current)){
              if(member->getOwner()==both){
                 HalfSegment hs = member->convertToHs(true,first);
                 hs.attr.edgeno = edgeno;
                 result += hs;
                 hs.SetLeftDomPoint(false);
                 result += hs;
                 edgeno++; 
              }
              sss.remove(*member);
              splitNeighbours(sss,leftN,rightN,q1,q2);
          } // current found in sss
          if(((pos1 >= size1) && q1.empty())  || 
             ((pos2 >= size2) && q2.empty())){
             done = true;
          } 
       } // right endpoint
  }
  result.EndBulkLoad();
} // commonborder 




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
    t1 [x] t2 [x] cluster [->] bool

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
The signature is t1 [x] t2 [->] int9m
where t1, t2 in {point, points, line, region}

*/

ListExpr TopRelTypeMap(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("two arguments expected");
      return nl->TypeError();
   }
   if(!IsSpatialType(nl->First(args)) 
      || !IsSpatialType(nl->Second(args))){
       ErrorReporter::ReportError("Spatial types expected");
       return (nl->TypeError());
   }
   return nl->SymbolAtom("int9m");
}

/*
8.1.4 StdPredTypeMap

*/

ListExpr StdPredTypeMap(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("two arguments expected");
      return nl->SymbolAtom("typeerror");
   }
   if(!IsSpatialType(nl->First(args)) 
      || !IsSpatialType(nl->Second(args))){
       ErrorReporter::ReportError("Spatial types expected");
       return (nl->TypeError());
   }
   return nl->SymbolAtom("bool");
}

/*
8.1.5 Realminize2TypeMap

Signature: line [->] line.

*/
ListExpr Realminize2TypeMap(ListExpr args){
  if(nl->ListLength(args)==1 &&
     nl->IsEqual(nl->First(args),"line")){
     return nl->SymbolAtom("line");
  }else {
     ErrorReporter::ReportError("line expected");
     return nl->TypeError();
  }
}




/*
8.1.6 Union2TypeMap

Signatures:   
  line [x] line [->] line
  line [x] region [->] region 
  region [x] line [->] region
  region [x] region [->] region

*/


ListExpr Union2TypeMap(ListExpr args){

   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("2 arguments expected.");
     return nl->TypeError();
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);

   if(!IsSpatialType(arg1) || !IsSpatialType(arg2)){
     ErrorReporter::ReportError("two spatial type expected");
     return nl->TypeError();
   }

   if(nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line")){
     return nl->SymbolAtom("line");
   }
   if(nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region")){
     return nl->SymbolAtom("region");
   }

   if(nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region")){
     return nl->SymbolAtom("region");
   }

   if(nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line")){
     return nl->SymbolAtom("region");
   }
   ErrorReporter::ReportError("combination not implemented yet");
   return nl->TypeError();
}



/*
8.1.7 Intersection2TypeMap

Signatures:   
  line [x] line [->] line
  line [x] region [->] line 
  region [x] line [->] line
  region [x] region [->] region

*/
ListExpr Intersection2TypeMap(ListExpr args){

   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("2 arguments expected.");
     return nl->TypeError();
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);

   if(!IsSpatialType(arg1) || !IsSpatialType(arg2)){
     ErrorReporter::ReportError("two spatial type expected");
     return nl->TypeError();
   }

   if(nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line")){
     return nl->SymbolAtom("line");
   }
   if(nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region")){
     return nl->SymbolAtom("region");
   }

   if(nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region")){
     return nl->SymbolAtom("line");
   }

   if(nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line")){
     return nl->SymbolAtom("line");
   }
   ErrorReporter::ReportError("combination not implemented yet");
   return nl->TypeError();
}


/*
8.1.8 Difference2TypeMap

Signatures:   
  line [x] line [->] line
  line [x] region [->] line 
  region [x] line [->] region
  region [x] region [->] region

*/

ListExpr Difference2TypeMap(ListExpr args){

   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("2 arguments expected.");
     return nl->TypeError();
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);

   if(!IsSpatialType(arg1) || !IsSpatialType(arg2)){
     ErrorReporter::ReportError("two spatial type expected");
     return nl->TypeError();
   }

   if(nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"line")){
     return nl->SymbolAtom("line");
   }
   if(nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"region")){
     return nl->SymbolAtom("region");
   }

   if(nl->IsEqual(arg1,"line") && nl->IsEqual(arg2,"region")){
     return nl->SymbolAtom("line");
   }

   if(nl->IsEqual(arg1,"region") && nl->IsEqual(arg2,"line")){
     return nl->SymbolAtom("region");
   }
   ErrorReporter::ReportError("combination not implemented yet");
   return nl->TypeError();
}


/*

8.1.9 CommonBorder2TypeMap

Signature: ~region~ [x] ~region~ [->] ~line~

*/

ListExpr CommonBorder2TypeMap(ListExpr args){

  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("Wrong number of arguments,"
                                " region x region expected");
     return nl->TypeError();
  }
  if(nl->IsEqual(nl->First(args),"region") &&
     nl->IsEqual(nl->Second(args),"region")){
     return nl->SymbolAtom("line");
  }
  ErrorReporter::ReportError(" region x region expected");
  return nl->TypeError();
}


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
  Cluster tmp(cluster);
  tmp.Transpose();
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
8.2.4 Realminize2

*/
int Realminize2VM(Word* args, Word& result, int message,
                Word& local, Supplier s){

   Line* arg = static_cast<Line*>(args[0].addr);
   result = qp->ResultStorage(s);
   Line* res = static_cast<Line*>(result.addr);
   Realminize2(*arg,*res);
   return 0;
}


/*
8.2.5 Value mapping for set operations

*/

template<class t1, class t2, class tres, SetOperation op>
int SetOpVM(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   t1* arg1 = static_cast<t1*>(args[0].addr);
   t2* arg2 = static_cast<t2*>(args[1].addr); 
   tres* res = static_cast<tres*>(result.addr);
   SetOp(*arg1,*arg2,*res,op);
   return 0;
}

template<class t1, class t2, class tres, SetOperation op>
int SetOpVMSym(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   t1* arg1 = static_cast<t1*>(args[0].addr);
   t2* arg2 = static_cast<t2*>(args[1].addr); 
   tres* res = static_cast<tres*>(result.addr);
   SetOp(*arg2,*arg1,*res,op);
   return 0;
}


/*
8.2.6 Value Mapping for CommonBorder2

*/
int CommonBorder2VM(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Region* arg1 = static_cast<Region*>(args[0].addr);
   Region* arg2 = static_cast<Region*>(args[1].addr); 
   Line* res = static_cast<Line*>(result.addr);
   CommonBorder(*arg2,*arg1,*res);
   return 0;
}

/*
8.2.7 Standard Topological Predicates

*/

static Cluster cl_disjoint;
static Cluster cl_adjacent;
static Cluster cl_overlap;
static Cluster cl_covers;
static Cluster cl_coveredBy;
static Cluster cl_inside;
static Cluster cl_contains;
static Cluster cl_equal;


template<class t1, class t2>
int StdPred(Word* args, Word& result, int message,
                    Word& local, Supplier s, Cluster& cl){
    
  result = qp->ResultStorage(s);
  t1* v1 = static_cast<t1*>(args[0].addr);
  t2* v2 = static_cast<t2*>(args[1].addr);
  Int9M matrix;
  bool res = GetInt9M(v1,v2,matrix,true,cl);
  (static_cast<CcBool*>(result.addr))->Set(true,res);
  return 0;
}

template<class t1, class t2>
int StdPredSym(Word* args, Word& result, int message,
                    Word& local, Supplier s, Cluster& cl){
    
  result = qp->ResultStorage(s);
  t1* v1 = static_cast<t1*>(args[0].addr);
  t2* v2 = static_cast<t2*>(args[1].addr);
  Int9M matrix;
  Cluster tmp(cl);
  tmp.Transpose();
  bool res = GetInt9M(v2,v1,matrix,true,tmp);
  (static_cast<CcBool*>(result.addr))->Set(true,res);
  return 0;
}

template<class t1, class t2>
int TrAdjacentVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_adjacent);
}

template<class t1, class t2>
int TrAdjacentVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_adjacent);
}


template<class t1, class t2>
int TrInsideVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_inside);
}

template<class t1, class t2>
int TrInsideVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_inside);
}


template<class t1, class t2>
int TrCoversVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_covers);
}

template<class t1, class t2>
int TrCoversVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_covers);
}


template<class t1, class t2>
int TrCoveredByVM(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_coveredBy);
}

template<class t1, class t2>
int TrCoveredByVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_coveredBy);
}


template<class t1, class t2>
int TrEqualVM(Word* args, Word& result, int message,
              Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_equal);
}

template<class t1, class t2>
int TrEqualVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_equal);
}

template<class t1, class t2>
int TrDisjointVM(Word* args, Word& result, int message,
              Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_disjoint);
}

template<class t1, class t2>
int TrDisjointVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_disjoint);
}


template<class t1, class t2>
int TrOverlapVM(Word* args, Word& result, int message,
              Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_overlap);
}

template<class t1, class t2>
int TrOverlapVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_overlap);
}

template<class t1, class t2>
int TrContainsVM(Word* args, Word& result, int message,
              Word& local, Supplier s){
  return StdPred<t1,t2>(args,result,message,local,s,cl_contains);
}

template<class t1, class t2>
int TrContainsVMSymm(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  return StdPredSym<t1,t2>(args,result,message,local,s,cl_contains);
}


static void initClusters(){
  PredicateGroup pg;
  pg.SetToDefault();

  Cluster* cl = pg.GetClusterOf("disjoint");
  assert(cl);
  cl_disjoint = *cl;
  delete cl;

  cl = pg.GetClusterOf("meet");
  assert(cl);
  cl_adjacent = *cl;
  delete cl;


  cl = pg.GetClusterOf("overlap");
  assert(cl);
  cl_overlap = *cl;
  delete cl;

  cl = pg.GetClusterOf("covers");
  assert(cl);
  cl_covers = *cl;
  delete cl;

  cl = pg.GetClusterOf("coveredBy");
  assert(cl);
  cl_coveredBy = *cl;
  delete cl;

  cl = pg.GetClusterOf("inside");
  assert(cl);
  cl_inside = *cl;
  delete cl;

  cl = pg.GetClusterOf("contains");
  assert(cl);
  cl_contains = *cl;
  delete cl;
  
  cl = pg.GetClusterOf("equal");
  assert(cl);
  cl_equal = *cl;
  delete cl;
}


/*
8.3  Operator specifications

*/


const string TopRelSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> {point, points, line, region} x "
   "  {points, points, line, region} -> int9m </text--->"
   " \" toprel(_ _) \" "
   " <text>computes the 9 intersection matrix describing the"
   " topological relationship between the arguments</text--->"
    "  \" query toprel(reg1, reg2) \" ))";

const string TopPredSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> so1 x so2 x cluster -> bool "
   " where o1, o2 in {point, points, line, region}</text--->"
   " \" topred(_, _, _) \" "
   " <text> checks whether the topological relationship between"
   " the spatial objects is part of the cluster </text---> "
    "  \" query toppred(l1,r1,cl_equals) \" ))";


const string Realminize2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> line -> line </text--->"
 " \" realminize2() \" "
 "  <text>removes crossings and overlapping"
        " segments from a line value </text---> "
"  \" query realminize2(trajectory(Train2)) \" ))";

const string Union2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> tx, ti in {line, region}  </text--->"
 " \"  _ union2 _ \" "
 "  \" computes the union of two spatial values \" "
  "  \" query l1 union2 l2 \" ))";

const string Intersection2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> {line,region} x {line,region} -> {line,region} </text--->"
 " \"  _ intersection2 _ \" "
 "  <text> computes the intersection of two spatial values </text---> "
  "  \" query l1 intersection2 l2 \" ))";

const string Difference2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> {line,region} x {line,region} -> {line,region} </text--->"
 " \"  _ difference2 _ \" "
 "  \" computes the difference of two spatial values \" "
  "  \" query l1 difference2 l2 \" ))";

const string CommonBorder2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> region x region -> line </text--->"
 " \"  _ commonborder2  _ \" "
 "  <text> computes the common part of the"
 " boundaries of the arguments </text---> "
  "  \" query r1 commonborder2 r2 \" ))";


const string TrAdjacentSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ tradjacent  _ \" "
 " <text> checks whether the arguments have exacly a common border </text--->"
  "  \" query r1 tradjacent r2 \" ))";


const string TrInsideSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ trinside  _ \" "
 " <text> checks whether the first argument is"
 " part of the second one </text---> "
  "  \" query r1 trinside r2 \" ))";

const string TrContainsSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ trcontains  _ \" "
 " <text> checks whether the second argument is part of"
 " the first one </text---> "
  "  \" query r1 trcontains r2 \" ))";

const string TrCoversSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ trcovers  _ \" "
 " <text> checks whether the first argument is part of the second one and"
 " the boundaries touches each other</text---> "
  "  \" query r1 trcovers r2 \" ))";

const string TrCoveredBySpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ trcoveredby  _ \" "
 "  <text> checks whether the second argument is part of the first one and"
 " the boundaries touches each other</text---> "
  "  \" query r1 trcoveredby r2 \" ))";

const string TrEqualSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ trequal  _ \" "
 "  <text> checks whether the arguments have the same geometry</text---> "
  "  \" query r1 trequal r2 \" ))";


const string TrOverlapSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ toverlap  _ \" "
 "  <text> checks whether the arguments have a common part ans "
 "also exclusive ones</text---> "
 "  \" query r1 troverlaps r2 \" ))";

const string TrDisjointSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
 " ( <text> t1 x t2 -> bool, t1,t2 in {point, points, line, region}</text--->"
 " \"  _ trdisjoint  _ \" "
 "  <text> checks if the arguments have no common part </text--->"
 "  \" query r1 trdisjoint r2 \" ))";

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
       TopRel<Line,Line>, TopRel<Line,Region>, TopRelSym<Region,Line>};

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
       TopPredSym<Point,Region>, 
       TopPred<Region,Points>,
       TopPredSym<Points,Region>,
       TopPred<Region,Region>,
       TopPred<Line,Line>, 
       TopPred<Line,Region>,
       TopPredSym<Region,Line>  };

ValueMapping AdjacentMap[] = {
       TrAdjacentVM<Point,Point> ,     TrAdjacentVM<Points,Point>,
       TrAdjacentVMSymm<Point,Points>, TrAdjacentVM<Points,Points>, 
       TrAdjacentVM<Line,Point>,       TrAdjacentVMSymm<Point,Line>,
       TrAdjacentVM<Line,Points>,      TrAdjacentVMSymm<Points,Line>,
       TrAdjacentVM<Region,Point>,     TrAdjacentVMSymm<Point,Region>,
       TrAdjacentVM<Region,Points>,    TrAdjacentVMSymm<Points,Region>,
       TrAdjacentVM<Region,Region>,    TrAdjacentVM<Line,Line>, 
       TrAdjacentVM<Line,Region>,      TrAdjacentVMSymm<Region,Line>};

ValueMapping InsideMap[] = {
       TrInsideVM<Point,Point> ,     TrInsideVM<Points,Point>,
       TrInsideVMSymm<Point,Points>, TrInsideVM<Points,Points>, 
       TrInsideVM<Line,Point>,       TrInsideVMSymm<Point,Line>,
       TrInsideVM<Line,Points>,      TrInsideVMSymm<Points,Line>,
       TrInsideVM<Region,Point>,     TrInsideVMSymm<Point,Region>,
       TrInsideVM<Region,Points>,    TrInsideVMSymm<Points,Region>,
       TrInsideVM<Region,Region>,    TrInsideVM<Line,Line>, 
       TrInsideVM<Line,Region>,      TrInsideVMSymm<Region,Line>};

ValueMapping CoversMap[] = {
       TrCoversVM<Point,Point> ,     TrCoversVM<Points,Point>,
       TrCoversVMSymm<Point,Points>, TrCoversVM<Points,Points>, 
       TrCoversVM<Line,Point>,       TrCoversVMSymm<Point,Line>,
       TrCoversVM<Line,Points>,      TrCoversVMSymm<Points,Line>,
       TrCoversVM<Region,Point>,     TrCoversVMSymm<Point,Region>,
       TrCoversVM<Region,Points>,    TrCoversVMSymm<Points,Region>,
       TrCoversVM<Region,Region>,    TrCoversVM<Line,Line>, 
       TrCoversVM<Line,Region>,      TrCoversVMSymm<Region,Line>};

ValueMapping CoveredByMap[] = {
       TrCoveredByVM<Point,Point> ,     TrCoveredByVM<Points,Point>,
       TrCoveredByVMSymm<Point,Points>, TrCoveredByVM<Points,Points>, 
       TrCoveredByVM<Line,Point>,       TrCoveredByVMSymm<Point,Line>,
       TrCoveredByVM<Line,Points>,      TrCoveredByVMSymm<Points,Line>,
       TrCoveredByVM<Region,Point>,     TrCoveredByVMSymm<Point,Region>,
       TrCoveredByVM<Region,Points>,    TrCoveredByVMSymm<Points,Region>,
       TrCoveredByVM<Region,Region>,    TrCoveredByVM<Line,Line>, 
       TrCoveredByVM<Line,Region>,      TrCoveredByVMSymm<Region,Line>};

ValueMapping EqualMap[] = {
       TrEqualVM<Point,Point> ,     TrEqualVM<Points,Point>,
       TrEqualVMSymm<Point,Points>, TrEqualVM<Points,Points>, 
       TrEqualVM<Line,Point>,       TrEqualVMSymm<Point,Line>,
       TrEqualVM<Line,Points>,      TrEqualVMSymm<Points,Line>,
       TrEqualVM<Region,Point>,     TrEqualVMSymm<Point,Region>,
       TrEqualVM<Region,Points>,    TrEqualVMSymm<Points,Region>,
       TrEqualVM<Region,Region>,    TrEqualVM<Line,Line>, 
       TrEqualVM<Line,Region>,      TrEqualVMSymm<Region,Line>};

ValueMapping DisjointMap[] = {
       TrDisjointVM<Point,Point> ,     TrDisjointVM<Points,Point>,
       TrDisjointVMSymm<Point,Points>, TrDisjointVM<Points,Points>, 
       TrDisjointVM<Line,Point>,       TrDisjointVMSymm<Point,Line>,
       TrDisjointVM<Line,Points>,      TrDisjointVMSymm<Points,Line>,
       TrDisjointVM<Region,Point>,     TrDisjointVMSymm<Point,Region>,
       TrDisjointVM<Region,Points>,    TrDisjointVMSymm<Points,Region>,
       TrDisjointVM<Region,Region>,    TrDisjointVM<Line,Line>, 
       TrDisjointVM<Line,Region>,      TrDisjointVMSymm<Region,Line>};

ValueMapping OverlapMap[] = {
       TrOverlapVM<Point,Point> ,     TrOverlapVM<Points,Point>,
       TrOverlapVMSymm<Point,Points>, TrOverlapVM<Points,Points>, 
       TrOverlapVM<Line,Point>,       TrOverlapVMSymm<Point,Line>,
       TrOverlapVM<Line,Points>,      TrOverlapVMSymm<Points,Line>,
       TrOverlapVM<Region,Point>,     TrOverlapVMSymm<Point,Region>,
       TrOverlapVM<Region,Points>,    TrOverlapVMSymm<Points,Region>,
       TrOverlapVM<Region,Region>,    TrOverlapVM<Line,Line>, 
       TrOverlapVM<Line,Region>,      TrOverlapVMSymm<Region,Line>};

ValueMapping ContainsMap[] = {
       TrContainsVM<Point,Point> ,     TrContainsVM<Points,Point>,
       TrContainsVMSymm<Point,Points>, TrContainsVM<Points,Points>, 
       TrContainsVM<Line,Point>,       TrContainsVMSymm<Point,Line>,
       TrContainsVM<Line,Points>,      TrContainsVMSymm<Points,Line>,
       TrContainsVM<Region,Point>,     TrContainsVMSymm<Point,Region>,
       TrContainsVM<Region,Points>,    TrContainsVMSymm<Points,Region>,
       TrContainsVM<Region,Region>,    TrContainsVM<Line,Line>, 
       TrContainsVM<Line,Region>,      TrContainsVMSymm<Region,Line>};


ValueMapping Union2Map[] = {
       SetOpVM<Line,Line,Line,union_op>,
       SetOpVM<Region,Region,Region,union_op>,
       SetOpVM<Region,Line,Region,union_op>,
       SetOpVMSym<Line,Region,Region,union_op>
       };

ValueMapping Intersection2Map[] = {
       SetOpVM<Line,Line,Line,intersection_op>,
       SetOpVM<Region,Region,Region,intersection_op>,
       SetOpVMSym<Region,Line,Line,intersection_op>,
       SetOpVM<Line,Region,Line,intersection_op>
      };

ValueMapping Difference2Map[] = {
       SetOpVM<Line,Line,Line,difference_op>,
       SetOpVM<Region,Region,Region,difference_op>,
       SetOpVM<Region,Line,Region,difference_op>,
       SetOpVM<Line,Region,Line,difference_op>
      };


/*
8.5 Selection Functions

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
   if( (type1=="region") && (type2=="line")){
       return 15;
   }

   return -1;
} 


static int SetOpSelect(ListExpr args){
   string type1 = nl->SymbolValue(nl->First(args));
   string type2 = nl->SymbolValue(nl->Second(args));
   
  if( (type1=="line") && (type2=="line")){
      return 0;
   }
  if( (type1=="region") && (type2=="region")){
      return 1;
   }
   if( (type1=="region") && (type2=="line")){
      return 2;
   }
   if( (type1=="line") && (type2=="region")){
      return 3;
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

Operator trAdjacent(
        "tradjacent",     // name
         TrAdjacentSpec,   // specification
         sizeof(AdjacentMap)/sizeof(ValueMapping),  // number of functions
         AdjacentMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trContains(
        "trcontains",     // name
         TrContainsSpec,   // specification
         sizeof(ContainsMap)/sizeof(ValueMapping),  // number of functions
         ContainsMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trOverlap(
        "troverlaps",     // name
         TrOverlapSpec,   // specification
         sizeof(OverlapMap)/sizeof(ValueMapping),  // number of functions
         OverlapMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trDisjoint(
        "trdisjoint",     // name
         TrDisjointSpec,   // specification
         sizeof(DisjointMap)/sizeof(ValueMapping),  // number of functions
         DisjointMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trEqual(
        "trequal",     // name
         TrEqualSpec,   // specification
         sizeof(EqualMap)/sizeof(ValueMapping),  // number of functions
         EqualMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trCoveredBy(
        "trcoveredby",     // name
         TrCoveredBySpec,   // specification
         sizeof(CoveredByMap)/sizeof(ValueMapping),  // number of functions
         CoveredByMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trCovers(
        "trcovers",     // name
         TrCoversSpec,   // specification
         sizeof(CoversMap)/sizeof(ValueMapping),  // number of functions
         CoversMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator trInside(
        "trinside",     // name
         TrInsideSpec,   // specification
         sizeof(InsideMap)/sizeof(ValueMapping),  // number of functions
         InsideMap,    // array of value mappings
         TopOpsSelect,
         StdPredTypeMap
         );

Operator realminize2(
     "realminize2",           //name
     Realminize2Spec,   //specification
     Realminize2VM, //value mapping
     Operator::SimpleSelect,         //trivial selection function
     Realminize2TypeMap //type mapping
);


Operator union2(
        "union2",     // name
         Union2Spec,   // specification
         sizeof(Union2Map)/sizeof(ValueMapping),  // number of functions
         Union2Map,    // array of value mappings
         SetOpSelect,
         Union2TypeMap
         );


Operator intersection2(
        "intersection2",     // name
         Intersection2Spec,   // specification
         sizeof(Intersection2Map)/sizeof(ValueMapping),  // number of functions
         Intersection2Map,    // array of value mappings
         SetOpSelect,
         Intersection2TypeMap
         );

Operator difference2(
        "difference2",     // name
         Difference2Spec,   // specification
         sizeof(Difference2Map)/sizeof(ValueMapping),  // number of functions
         Difference2Map,    // array of value mappings
         SetOpSelect,
         Difference2TypeMap
         );


Operator commonborder2(
         "commonborder2",           //name
          CommonBorder2Spec,   //specification
          CommonBorder2VM, //value mapping
          Operator::SimpleSelect,         //trivial selection function
          CommonBorder2TypeMap //type mapping
);

/*
8.7 Creating the algebra

*/
class TopOpsAlgebra : public Algebra {
  public:
     TopOpsAlgebra() : Algebra() {
        AddOperator(&optoprel);
        AddOperator(&toppred);
        AddOperator(&realminize2);
        AddOperator(&union2);
        AddOperator(&intersection2);
        AddOperator(&difference2);
        AddOperator(&commonborder2);
        AddOperator(&trAdjacent);
        AddOperator(&trInside);
        AddOperator(&trCovers);
        AddOperator(&trCoveredBy);
        AddOperator(&trEqual);
        AddOperator(&trDisjoint);
        AddOperator(&trOverlap);
        AddOperator(&trContains);
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
    initClusters();
    return (&topOpsAlgebra);
}

