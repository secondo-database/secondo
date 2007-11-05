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


3 Implementation

3.1 Includes
3.1.1 The Standard Includes for Algebra Modules

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

/*
3.1.2 The Includes for the used Algebras

*/

#include "SpatialAlgebra.h"
#include "TopRel.h"
#include "StandardTypes.h"

//#define __TRACE__ cout << __FILE__ << "@" << __LINE__ << endl;
#define __TRACE__


/*
3.2 Instances of NestedList and the QueryProcessor

*/

extern NestedList* nl;
extern QueryProcessor* qp;
Coord  EPSILON = 0.000001;

using namespace toprel;




/*
3.3 Helper class for plane sweep algorithms

This class can be used to store halfsegments whithin an
AVL tree. It is possible to switch the functionality of
the comparison operators. The make it possible to insert
/ delete a halfsegment with an exact match but to search only 
dor the y value.

*/

class AvlEntry;
ostream& operator<<(ostream& o, const AvlEntry& e);


class AvlEntry{
public:

/*
3.3.0 Standard Constructor
This constructor does nothing but is required for an AVLTree entry.

*/
AvlEntry():source(0){}


/*
3.3.1 Constructor

This constructor creates an AvlEntry from a 
halfsegment.

*/
  AvlEntry(HalfSegment const * const Hs){
     Point p1 = Hs->GetLeftPoint();
     Point p2 = Hs->GetRightPoint();
     x1 = p1.GetX();
     y1 = p1.GetY();
     x2 = p2.GetX();
     y2 = p2.GetY();
     vertical = x1==x2; 
     if(!vertical){
        slope = (y2-y1)/(x2-x1);
     } else{
        slope = 1.0;
     }
     insideAbove = Hs->GetAttr().insideAbove;
     source = 0;
  }

/*
3.3.2 Constructor

This constructor creates an AvlEntry from a 
point.

*/

 AvlEntry(Point const * const P){
    x1 = P->GetX();
    x2 = x1;
    y1 = P->GetY();
    y2 = y1;
   // set some dummy values 
    vertical = true;
    slope = 0;
    insideAbove = false; 
    source = 0;
 }



/*
3.3.2 Copy Constructor

*/

  AvlEntry(const AvlEntry& source){
    this->x1 = source.x1;
    this->x2 = source.x2;
    this->y1 = source.y1;
    this->y2 = source.y2;
    this->vertical = source.vertical;
    this->slope = source.slope;
    this->source = source.source;
  }

  
/*
3.3.3 Assignment Operator

*/  
   AvlEntry& operator=(const AvlEntry& source){
     this->x1 = source.x1;
     this->x2 = source.x2;
     this->y1 = source.y1;
     this->y2 = source.y2;
     this->vertical = source.vertical;
     this->insideAbove = source.insideAbove;
     this->slope = source.slope;
     this->source = source.source;
     return *this;
   }

/*
3.3.4 Destructor

*/
   ~AvlEntry(){}


/*
3.3.5 isPoint

*/
  bool isPoint() const{
    return AlmostEqual(x1,x2) && AlmostEqual(y1,y2);
  }


/*
3.3.5 Comparison Operators

We have to distinct between different modes of comparisons.
If the compexact flag is set, we compare two segment as usual.
Otherwise, we compute the y value at the current x value (defined 
in the static member ~x~) and compare them. For vertical segments, we use
the ~y1~ values for the comparison.



*/
   int compareTo(const AvlEntry& c) const{

   double ty = vertical? y1 : y1 + ((x-x1)/(x2-x1))*(y2-y1);
   double cy = c.vertical? c.y1 : c.y1 + ((x-c.x1)/(c.x2-c.x1))*(c.y2-c.y1) ;

   if(compexact){  

      // first check : current y value
      if(!AlmostEqual(ty,cy)){
        if(ty<cy )
           return -1;
        else 
           return 1;
      }

      // a point is smaller than a segment
      if(isPoint() || c.isPoint()){
        if(isPoint()&&c.isPoint()){
           return 0;
        }
        if(isPoint()){
          return -1;
        } else{
          return 1;
        }
      }


      if(!AlmostEqual(slope,c.slope)){
         // if the current position is the last point of one 
         // segment, we must use the y coordinates before
         // the current x , otherwise the y coordinates after
         // that
         int f = 1;
         if(AlmostEqual(x2,x)  || AlmostEqual(c.x2,x) )   {
             f = -1;
         }
         if(slope<c.slope){
            return -f;
         } else {
            return f;
         }
      } 

      if(!AlmostEqual(x1,c.x1)){
        int f= slope>0?1:-1;
        if(x1<c.x1) 
           return -f; 
        else 
           return f;
       }     
      if(!AlmostEqual(y1,c.y1)){
        if(y1<c.y1) 
           return -1; 
        else 
           return 1;
      }     
      if(!AlmostEqual(x2,c.x2)){
        if(x2<c.x2) 
           return -1; 
        else 
           return 1;
      }     
      if(!AlmostEqual(y2,c.y2)){
        if(y2<c.y2) 
           return -1; 
        else 
           return 1;
      }  else {
         if(source < c.source){
            return -1;
         } else if (source > c.source){
            return 1;
         } else {
            return 0;
         }
      }    
  } 
  else { // comparison with y coordinate
        if(AlmostEqual(ty,cy)){
         if(isPoint() || c.isPoint()){
             if(source < c.source){
                return -1;
             } else if (source > c.source){
                return 1;
             } else {
                return 0;
             }
         }
         int f = 1;
         if( (AlmostEqual(x2,x) && AlmostEqual(y2,ty))  ||
             (AlmostEqual(c.x2,x) && AlmostEqual(c.y2,cy))) {
             f = -1;
         }
           if(AlmostEqual(slope,c.slope)){
             if(source < c.source){
                return -1;
             } else if (source > c.source){
                return 1;
             } else {
                return 0;
             }
           } else if(slope<c.slope){
              return -f;
           } else {
              return f;
           }
        } else if (ty<cy){
           return -1;
        } else {
           return 1;
        }
     }
   }



   bool operator<(const AvlEntry& c)const{
      return compareTo(c)<0;
   }

  bool operator>(const AvlEntry& c)const{
      return compareTo(c)>0;
  }
  
  bool operator==(const AvlEntry& c)const{
      return compareTo(c)==0;
  }

  bool isVertical()const{
     return vertical;
  }

  bool isInsideAbove() const{
     return insideAbove;
  }

  void Print(ostream& out) const{
     out << "(" << x1 << ", " << y1 << ")->(" 
                << x2 << ", " << y2 << ")["
                << source << ", " << GetY(x) << "]" ;
  }

  bool Contains(const Point p) const{

     Point p1(true,x1,y1);
     Point p2(true,x2,y2);
     if(AlmostEqual(p1,p)){
        return true;
     }
     if(AlmostEqual(p2,p)){
        return true;
     }

     double x = p.GetX();
     double y = p.GetY();
     if(x<x1){
       return false;
     }
     if(x>x2){
       return false;
     }
     if(vertical){
        if(y<min(y1,y2)){
          return false;
        } else if(y>max(y1,y2)){
          return false;
        } else {
          return true;
        }
     } else {
        double y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
        bool res =  AlmostEqual(y,p.GetY());
        return res;
     } 
  }

  void setSource(const int source){
     this->source = source;
  } 

  int getSource() const{
    return source;
  }

/*
Returns the y value for the given x coordinate. If the 
segment is vertical or the given x coordinate is 
outside the range of the contained segment this function
will force an assertion

*/ 
  double GetY(const double x)const{
     assert(!isVertical());
     bool endpoint = AlmostEqual(x,x1) || AlmostEqual(x,x2);
     bool innerpoint = (x>=x1) && (x <=x2);
     assert(endpoint || innerpoint);
     return  y1 + ((x-x1)/(x2-x1))*(y2-y1);
  }

/*
~Accessing members~

*/
   double GetY1() const { return y1; }
   double GetY2() const { return y2; }
   double GetX1() const { return x1; }
   double GetX2() const { return x2; }
   double GetMinY() const { return y1<y2?y1:y2; }
   double GetMaxY() const { return y1>y2?y1:y2; }



/*
~CheckIntersection~

This function assumes that this instance and e are neighbours
for position x. It will set all intersections in res which may occur.
Note: this function works only correct, if __e__ is the single event on
position __x__.
if onlydown is set to true, only such entries are regarded were the
y coordinate is smaller than __maxY__ otherwise, such entries are ignored.


*/
  void checkIntersections(const AvlEntry* e, Int9M& res, const double x,
                          bool onlydown = false, const double maxY = 0)const{
     if(source==e->source){
       return;
     }

     // compute the relations at the current x coordinate
     // between both segments no further segment can exist
     double cy = y1 + ((x-x1)/(x2-x1))*(y2-y1);
     double cey = e->y1 + ((x-e->x1)/(e->x2-e->x1))*(e->y2-e->y1);
     int cmp = 0;

     if(onlydown){
        cout << "check only elements smaller than " << maxY << endl;
     } else {
        cout << "Check all elements." << endl;
     }


     if(!AlmostEqual(cy,cey) && onlydown && (cey>maxY || 
         AlmostEqual(cey,maxY))){
        cout << "skip check because y is to high" << endl;
        return;
     }

     cout << " Perform the test " << endl << " cey = " << cey << endl;
     cout << "cy = " << cy << endl;

     if(AlmostEqual(cy,cey)){
       // boundaries intersect
       cmp = 0;
       res.SetBB(true);
     } else if(cy<cey){ 
       cmp = -1;
       if(insideAbove){
          res.SetIE(true);
          res.SetIB(true);
          res.SetII(true);
       } else {
          res.SetEE(true);
          res.SetEB(true);
          res.SetEI(true);
       }
       if(e->insideAbove){
         res.SetEE(true);
         res.SetBE(true);
         res.SetIE(true);
       } else {
         res.SetEI(true);
         res.SetBI(true);
         res.SetII(true);
       }
     } else { // cy>cey, symmetric to <
       cmp = 1;
      if(!onlydown || cy < maxY){
         if(insideAbove){
           res.SetEE(true),
           res.SetEB(true);
           res.SetEI(true);
         } else {
           res.SetIE(true);
           res.SetIB(true);
           res.SetII(true);
         }
         if(e->insideAbove){
           res.SetEI(true);
           res.SetBI(true);
           res.SetII(true);
         } else {
           res.SetEE(true);
           res.SetBE(true),
           res.SetIE(true);
         }
      }
     }


     cout << "After processing the first pint, the resuot is " 
          << endl << res << endl;



     // compute the y value at the last x position
     double x_last = x2<e->x2?x2:e->x2;


     if(AlmostEqual(x_last,x)){
       return;
     }

     double cy_last  = y1 + ((x_last-x1)/(x2-x1))*(y2-y1);
     double cey_last = e->y1 + ((x_last-e->x1)/(e->x2-e->x1))*(e->y2-e->y1);
    
     if(AlmostEqual(cy_last,cey_last)){
       res.SetBB(true);
     } else if(cy_last < cey_last){
       if(cmp>0){ // relation changed, segments must be intersect
         res.SetBB(true);
         res.SetBI(true);
         res.SetBE(true);
         res.SetII(true);
         res.SetIE(true);
         res.SetIB(true),
         res.SetEE(true);
         res.SetEB(true);
         res.SetEI(true);   
       }  
     } else {  // cy_last > cey_last
       if(cmp<0){ // relation changed
         res.SetBB(true);
         res.SetBB(true);
         res.SetBI(true);
         res.SetBE(true);
         res.SetII(true);
         res.SetIE(true);
         res.SetIB(true),
         res.SetEE(true);
         res.SetEB(true);
         res.SetEI(true);   
       }
     }
  }

  static bool compexact;
  static double x;

private:
  double x1;
  double y1;
  double x2;
  double y2;
  double slope; // redundant to avoid computations 
  bool   vertical;
  bool   insideAbove;
  int    source;
};

/*
3.3.5 Initialization of static members of AvlEntry


*/

bool AvlEntry::compexact=true;
double AvlEntry::x = 0.0;


/*
3.3.6 Shift operator for the AVLEntry

*/

ostream& operator<<(ostream& o, const AvlEntry& entry){
    entry.Print(o);
    return o;
}




/*
3.3 Statistical information 

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
3.3 Type Mapping of the __TopPred__ operator

This operator checks whether the topological relationship between
two spatial objects is part of a predicate cluster which is defined
by a predicate group together with the name of this cluster. So, the 
signature of this operator is:

  $o_1$ [times] $o_2$ [times] string [times] predicategroup [->] bool 


3.3.1 IsSpatialType

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
3.3.2 IsImplemented

This function checks whether the given combination of 
spatial objects is implemented. 

~Recondition~ Both lists must hold SymbolAtoms

*/
bool IsImplemented(ListExpr type1, ListExpr type2){
    string t1 = nl->SymbolValue(type1);
    string t2 = nl->SymbolValue(type2);
    if( (t1=="point" || t1=="points")  &&
        (t2=="point" || t2=="points")){
        return true;
    }
    if( ((t1=="point") && (t2=="line"))){
       return true;
    }
      
    if(((t1=="line") && (t2=="point"))){
      return true;
    }
    
    if( ((t1=="points") && (t2=="line"))){
       return true;
    }
      
    if(((t1=="line") && (t2=="points"))){
      return true;
    }
    if(((t1=="region") && (t2=="point"))) return true;
    if(((t1=="point") && (t2=="region"))) return true;
    if(((t1=="region") && (t2=="points"))) return true;
    if(((t1=="points") && (t2=="region"))) return true;
    

    if(((t1=="region") && (t2=="region"))) return true;

    cout << t1 << " x " << t2 << " is not implemented" << endl;
    return false;
}

/*
3.3.4 TopPredTypeMap

This function is the type mapping for the toppred operator.

*/
ListExpr TopPredTypeMap(ListExpr args){
   if(nl->ListLength(args)!=4){
      ErrorReporter::ReportError("four arguments required");
      return nl->SymbolAtom("typeerror");
   }
   ListExpr str = nl->Third(args);
   ListExpr group = nl->Fourth(args);
   if(!nl->IsEqual(str,"string")){
       ErrorReporter::ReportError("the third argument must" 
                                  " be of type string\n");
       return nl->SymbolAtom("typeerror");
   }
   if(!nl->IsEqual(group,"predicategroup")){
       ErrorReporter::ReportError("the third argument must" 
                                  " be of type predicategroup\n");
       return nl->SymbolAtom("typeerror");
   }
   ListExpr o1 = nl->First(args);
   ListExpr o2 = nl->Second(args);
   if(!IsSpatialType(o1)){
      ErrorReporter::ReportError("The first argument must be a spatial type");
      return nl->SymbolAtom("typeerror");
   }   
   if(!IsSpatialType(o2)){
      ErrorReporter::ReportError("The second argument"
                                 " must be a spatial type");
      return nl->SymbolAtom("typeerror");
   }  
   
   if(!IsImplemented(o1,o1)){
      ErrorReporter::ReportError("not implemented combination");
      return nl->SymbolAtom("typeerror");
   }
 
   return nl->SymbolAtom("bool");
}


/*
3.3.5 TopRelTypeMap

This function is the Type mapping for the toppred operator.

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
   if(!IsImplemented(nl->First(args),nl->Second(args))){
       ErrorReporter::ReportError("combination not implemented yet");
       return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("int9m");
}


#ifdef  TOPOPS_USE_STATISTIC
/*
3.3.6 ResetStatisticTypeMap

*/
ListExpr TopOpsResetStatTypeMap(ListExpr args){
   if(nl->ListLength(args)!=0){
      ErrorReporter::ReportError("no argument expected");
      return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("bool");
}

ListExpr TopOpsGetStatTypeMap(ListExpr args){
   if(nl->ListLength(args)!=0){
      ErrorReporter::ReportError("no argument expected");
      return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("text");
}
#endif

/*
3.3.4 Support functions

The following function are supporting the GetInt9M functions.

~About~

This function compares two Coord values. If the deviation is smaller than
a procentual value of the first number, the result will be true. Set EPSILON
to zero to reach an exact result.

*/
bool About( Coord const& a, Coord const& b){
  Coord e = EPSILON*a;
  e = e<0?-e:e;
  Coord dist = a>b?a-b:b-a;
  return dist<e;
}

/*
~Compare~

This function compare two point values

*/
  static int Compare(Point const*  p1, Point const* p2){
     Coord x1 = p1->GetX();
     Coord y1 = p1->GetY();
     Coord x2 = p2->GetX();
     Coord y2 = p2->GetY();
     if(About(x1,x2) && About(y1,y2))
       return 0;
     if( (x1<x2) || ( About(x1,x2) && (y1<y2 ))){
       return -1;
     }
     return 1;
  } 

/*
~HasEndpointAt~

Returns true if the dom,inating point of the halfsegment at 
position __pos__ of line is an endpoint of this line.

*/
bool HasEndpointAt(Line const* line, const int pos){
  int size = line->Size();
  if((pos<0) || (pos>=size)){
     return false;
  }
  HalfSegment const* chs;
  line->Get(pos,chs);
  Point p0 = chs->GetDomPoint();
  
  Point p1(.0,.0);
  if(pos>0){
     line->Get(pos-1,chs); 
     p1 = chs->GetDomPoint();
     if(p0==p1){
        return false;
     } 
  }
  if(pos<size-1){
    line->Get(pos+1,chs);
    p1 = chs->GetDomPoint();
    if(p0==p1){
       return false;
    }
  }
  return true;
}


/*
~NumberOfEndpoints~

This function computes the number of endpoints of __line__.
The function stops the computation when the number of endpoints
is greater than or equals to __stop__. If this argument holds a
value [<=] 0, the function will not stop before all halfsegments
are processed. If the first argument is given, the search will start
from this index within the halfsegments.

Complexity: O(n)

*/
int  NumberOfEndpoints(Line const* const line, 
                       const int stop=-1, 
                       int first=0){
  if(line->IsEmpty()){ // an empty line has no endpoints
     return 0;
  } 
   
  if(first<0){
     first=0;
  }
  int size = line->Size();
  if(first>=size){
     return 0;
  }

  // because the first sort criteria of halfsegments is the 
  // dominating point, we have just to check whether a halfsegment
  // at index+1 or index-1 has the same dominating point
  HalfSegment const* chs1=NULL;  
  HalfSegment const* chs2=NULL;
  Point p1;
  Point p2;
  line->Get(first,chs1);
  p1 = chs1->GetDomPoint();
  int pos=first+1;
  int num=0; // no endpoint up to now
  while(pos<size){
    line->Get(pos,chs2);
    p2 = chs2->GetDomPoint();
    if(p1!=p2){ // found an endpoint
      if( (pos==first+1) && first>0){
         // check if the former dp is equals
         HalfSegment const* chs3;
         line->Get(first-1,chs3);
         Point p3 = chs3->GetDomPoint();
         if(p3==p1){
             num--; 
         }
      }
      num++;
      if( (stop>0) && (num>=stop)){
        return num;
      }
    }
    pos++;
    // search a point different to p2
    bool found = false;
    while( (pos<size) && !found){
      line->Get(pos,chs1);
      p1 = chs1->GetDomPoint();
      if(p1!=p2){
        found = true;
      }
      pos++;
    }
  }
  return num;
}


/* 
~Contains~

This function checks if the halfsegment contains the point.

 

*/

inline bool Contains(HalfSegment const* const chs, const Point& point){
   Coord x = point.GetX();
   Coord y = point.GetY();
   Point p1 = chs->GetLeftPoint();
   Point p2 = chs->GetRightPoint();
   Coord x1 = p1.GetX();
   Coord x2 = p2.GetX();
   Coord y1 = p1.GetY();
   Coord y2 = p2.GetY();
   // distance between p1 and p2
   double d1 = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
   // distance between p2 and p
   double d2 = sqrt( (x2-x)*(x2-x) + (y2-y)*(y2-y));
   // distance between p1 and p
   double d3 = sqrt( (x1-x)*(x1-x) + (y1-y)*(y1-y));
   double d = d1 - (d2+d3);
   return About(0,d); 

}



/*
~InnerContains~

This function checks whether __point__ is located on the 
interior of __chs__.
This check is done by checking whether the distance between the
endpoints of the segments is equals to the sum of the distances between
the endpoints to the point to check.

*/
bool InnerContains(HalfSegment const* const chs, const Point& point){
  Coord x = point.GetX();
  Coord y = point.GetY();
  Point p1 = chs->GetLeftPoint();
  Point p2 = chs->GetRightPoint();
  Coord x1 = p1.GetX();
  Coord x2 = p2.GetX();
  Coord y1 = p1.GetY();
  Coord y2 = p2.GetY();
  if( ( (x==x1) && (y==y1)) || // endpoint, not inner point
      ( (x==x2) && (y==y2))){
      return false;
  } 
  return Contains(chs,point);  
}



/*
3.4 Computation of the 9 intersection matrices

~GetInt9M~

This function computes the 9-intersection matrix between two point values.
Because a single point is very simple, no bounding box tests are
performed.

Complexity: O(1)

*/

void GetInt9M(Point* p1 , Point*  p2,Int9M& res){
#ifdef TOPOPS_USE_STATISTIC
  GetCalls_p_p++;
#endif
  res.SetValue(0);
  // in each case, the exteriors intersect
  res.SetEE(true);
  if(Compare(p1,p2)==0){
    res.SetII(true);
  }else{
    res.SetIE(true);
    res.SetEI(true);
  }
}

/*
~GetInt9M~

The next function computes the 9 intersection matrix between a 
point value and a points value. 

Complexity: O(log(n))  where ~n~ is the number of points in the __points__
value.

*/
void GetInt9M(Points*  ps, Point* p,Int9M& res){
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
     return;
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
      return;
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
  return;
}


/*
~GetInt9M~

This function returns the 9 intersection matrix describing the 
topological relationship between two __points__ values.

Complexity: O(~n~+~m~) , where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively, where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively.

*/
void GetInt9M(Points* ps1, Points*  ps2,
              Int9M& res){


  
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
     return;
   }
   if(n1<=0){
      // some points of ps2 are in the exterior of ps1
      res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_ps_ps++;
#endif
      return;
   }
   if(n2<=0){
      // symmetrically to the previous case
      res.SetIE(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_ps_ps++;
#endif
      return;
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
      return;
   }

   const Point* p1;
   const Point* p2;
   // some boolean constants for early exit
   bool ii=false;
   bool ie=false;
   bool ei=false;
   // perform a parallel scan
   int i1=0; 
   int i2=0;
  
 
   bool done = false;
   do{ 
       assert(i1<n1);
       assert(i2<n2);
       ps1->Get(i1,p1);
       ps2->Get(i2,p2);
       int cmp = Compare(p1,p2);
       p1 = NULL;
       p2 = NULL;
       if(cmp==0){ //p1==p2
         if(!ii){
            res.SetII(true);
         }
         ii=true;
         i1++;
         i2++;
       } else if (cmp<0){
         // p1 in the exterior of p2 
         if(!ie){
             res.SetIE(true);
         }
         ie=true;
         i1++; 
       } else{ // p1 > p2
          if(!ei){
             res.SetEI(true);
          }
          ei=true;
          i2++;
       }
       
       done= ((i1>=n1-1) || (i2>=n2-1)) || // end of one points value reached
             (ii && ie && ie);           // no more intersections possible
   }while(!done);    
   


   if(ii && ie && ei){ // maximum count of intersections
      return;  
   }
   if(i1<n1-1){ // ps1 has further points
      res.SetIE(true);
   }
   if(i2<n2-1){ // ps2 has further points
      res.SetEI(true);
   }
   return;
}


/*
~GetInt9M~

This function computes the 9-intersection matrix for a line and a single point.

Complexity: O(n)

*/
void GetInt9M(Line const* const line, Point const* const point,Int9M& res){
#ifdef TOPOPS_USE_STATISTIC
   GetCalls_l_p++;
#endif
   res.SetValue(0);
   res.SetEE(true);
   if(line->IsEmpty()){
     res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
     bb_l_p++;
#endif
     return;
   }
   // the interior of a non-empty line has always
   // an intersection with the exterior of a single point
   // because of the difference in the dimension   
   res.SetIE(true);

   // the line contains at least one halfsegment
   Rectangle<2> bbox_line = line->BoundingBox();
   Rectangle<2> bbox_point = point->BoundingBox();
   if(!bbox_line.Intersects(bbox_point)){
      res.SetIE(true);
      res.SetEI(true);
      if(NumberOfEndpoints(line,1)>0){
        res.SetBE(true); 
      }
#ifdef TOPOPS_USE_STATISTIC
      bb_l_p++;
#endif
      return;
   }

   // prefilter unsuccessful -> scan the halfsegments
   int size = line->Size(); 
   bool done = false;
   Point thePoint = (*point);
   HalfSegment const* chs;
   Point p;
   int endpoints = NumberOfEndpoints(line,2);
   for(int i=0;(i<size) && !done; i++){
       line->Get(i,chs);
       p = chs->GetDomPoint();
       if(p==thePoint){ // point on endpoint of chs
         done = true;
         if(i+1<size){
            line->Get(i+1,chs);
            p=chs->GetDomPoint();
            if(p==thePoint){ // an inner point of the line
               res.SetII(true);
               if(endpoints>0){
                 res.SetBE(true);
               }
            } else{ // an endpoint of the line
               res.SetBI(true);
               if(endpoints>1){
                 res.SetBE(true);
               }
            }
         } else{ // an endpoint of the line
             res.SetBI(true);
             if(endpoints>1){
               res.SetBE(true);
             }
         }
         return; 
       }
       if(InnerContains(chs,thePoint)){
           res.SetII(true);
           if(endpoints>0){
              res.SetBE(true);
           } 
           return;
       }
       // we can stop the computation when the next point is
       // greater than thePoint
       done = p>thePoint;
   }
   // the point is outside the closure of the line
   res.SetEI(true); // point in exterior
   if(endpoints>0){
      res.SetBE(true);
   } 
   return;
}


/*
~GetInt9M~

This function computes the topological relationship between a 
__line__ and a pointset as 9-intersection matrix. 

~complexity~ O(n * (m+1))

should be changed to a plane sweep algorithm!

*/

void GetInt9M(Line const* const line, Points const* const ps, Int9M& res){

#ifdef TOPOPS_USE_STATISTIC
   GetCalls_l_ps++;
#endif
   
   res.SetValue(0);
   res.SetEE(true);
   // special case empty line 
   if(line->IsEmpty()){
#ifdef TOPOPS_USE_STATISTIC
      bb_l_ps++;
#endif
      if(!ps->IsEmpty()){
         res.SetEI(true);
      }
      return;
   }
    
   // line net empty => interior of the line intersects the 
   // exterior of the point (dimension difference)
   res.SetIE(true);
  
   // special case empty point set 
   if(ps->IsEmpty()){
     // non-empty line
     int num = NumberOfEndpoints(line,1);
     if(num>0){
       res.SetBE(true);
     }
#ifdef TOPOPS_USE_STATISTIC
     bb_l_ps++; 
#endif
     return;
   }

   // both objects are not empty 
   // bounding box check
   Rectangle<2> bbox1 = line->BoundingBox();
   Rectangle<2> bbox2 = ps->BoundingBox();
   if(!bbox1.Intersects(bbox2)){
      int num = NumberOfEndpoints(line,1,0);
      if(num>0){
        res.SetBE(true);
      }
      res.SetIE(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_l_ps++;
#endif
      return;
   }

   // we have to scan the objects
   HalfSegment const* chs=NULL; // current halfsegment
   Point dp;                     // dominating point of chs
   Point ndp;                    // non-dominating point of chs  
   Coord dpx;                   // x coordinate of dp
   Coord dpy;
   Coord ndpx;

   line->Get(0,chs);
   dp = chs->GetDomPoint();
   dpx = dp.GetX();

   int ps_size = ps->Size();
   // array stored the state of each point in ps
   bool processed[ps_size];
   for(int i=0;i<ps_size;i++){
      processed[i] = false;
   }

   // jump over all points left from dp 
   int min = 0;
   int max = ps_size-1;
   int mid;
   Point const* psp=NULL; // current point in pointset
   Coord pspx; // x coordinate of psp
   while(min<max){
      mid = (min+max)/2;
      ps->Get(mid,psp);
      pspx = psp->GetX();
      if(pspx>dpx){
        max = mid-1;
      } else if (pspx<dpx){
        min = mid+1;
      } else{ // equals
        max = mid;
      }
   }

   if(min>0){ //there are points left of the line
      res.SetEI(true);
   }


  
   // min contains the first relevant point
   int line_size = line->Size();
   int pos = 0; // position of the current halfsegment
   bool done = false; // no more points to handle

   // check all interesting halfsegments
   while(pos<line_size && !done){
     line->Get(pos,chs);
     dp = chs->GetDomPoint();
     dpx = dp.GetX();
     // jump overal all points left to dp
     ps->Get(min,psp);
     pspx = psp->GetX();
     while( (min<ps_size) && (pspx < dpx) ){
        if(!processed[min]){ // found point outside of line
           res.SetEI(true);
           //processed[min]=true; // not required
        }
        min++;
        ps->Get(min,psp);
        pspx = psp->GetX();
     } 
     if(min==ps_size){ // all points processed
       done=true;
       // check wether boundary points exists beginning from this point
       int num = NumberOfEndpoints(line,1,pos);
       if(num>0){
          res.SetBE(true);
       }
     }else{ // further points exist
       bool isEP = HasEndpointAt(line,pos);
       ndp = chs->GetSecPoint();
       ndpx = ndp.GetX();
       dpy = dp.GetY();
       int pointpos=min;
       Point const* currentPoint;
       Coord point_x;
       Coord point_y;
       while(((pspx<ndpx)|| (pspx==dpx)) && pointpos<ps_size){
         ps->Get(pointpos,currentPoint);
         point_x = currentPoint->GetX();
         point_y = currentPoint->GetY();
         if(!processed[pointpos]){  
            if(About(dpx,point_x) && About(dpy,point_y)){
              if(isEP){
                res.SetBI(true);
              } else{
                res.SetII(true);
              } 
              processed[pointpos] = true; 
            } else if(InnerContains(chs,*currentPoint)){
              res.SetII(true);
              processed[pointpos] = true;  
            }
         }
         pointpos++;
       } // end list of points
     } // end points exists
     pos++; // go to the next halfsegment
   }  // go trought the halfsegments

   return;

}



/*
~GetInt9M~

Computation of the 9 intersection matrix for a single point and a 
region value. 

*hacked* (should be changed to a single run

[3] O(~m~) where m is the number of segments of the region parameter.

*/
void GetInt9M(Region const* const reg, Point const* const p, Int9M& res){

#ifdef TOPOPS_USE_STATISTIC
  GetCalls_r_p++;
#endif

  res.SetValue(0);
  res.SetEE(true);
  if(reg->IsEmpty()){
     res.SetEI(true);
     return;
  }
  res.SetIE(true); // the point can't cover an infinite set of points
  res.SetBE(true); //  see above

  Rectangle<2> bboxreg = reg->BoundingBox();
  Rectangle<2> bboxp = p->BoundingBox();

  if(!bboxreg.Intersects(bboxp)){
     res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_r_p++;
#endif
     return;
  }

  // the bounding boxes overlap, we perform a plane sweep algorithm

  AVLTree<AvlEntry> sss; // sweep state structure
  int size = reg->Size();
  int pos = 0; // current pos within the halfsegments
  double xpos = p->GetX();
  
  vector<HalfSegment> v; // vector containing all events having the
                         // same x position
  
  bool done = false;
  const HalfSegment* hs;
  bool innercand = false;
  while(!done && pos<size){
     reg->Get(pos,hs);
     double cx = hs->GetDomPoint().GetX();
     AvlEntry entry(hs);

     if(AlmostEqual(cx,xpos)){
        if(entry.isVertical()){
           if(hs->Contains(*p)){
              res.SetBI(true);
              done = true;
           } else {
              pos++;
           }
        } else {
          if(hs->IsLeftDomPoint()){
             AvlEntry::x = xpos;
             AvlEntry::compexact = true;
             bool ins = sss.insert(entry);    
             if(!ins){
                 cerr << "failed to insert " << entry << " at position " 
                      << xpos << " into " << endl;
                 sss.Print(cerr);
                 cerr << " The element which is recognized to be equal is " 
                      <<  *(sss.getMember(entry)) << endl;
             }
             assert(ins);
             
             if(!sss.Check(cerr)){
                cerr << "AVL Tree property violated" << endl;
                cerr << " Tree after inserting " << entry << " is " 
                     << sss << endl <<endl;
                assert(false);
             } 
             pos++;
          } else {
             // perform the point
             AvlEntry pe(p);
             AvlEntry::x = xpos;
             AvlEntry::compexact=false;
             const AvlEntry* sm = sss.GetNearestSmallerOrEqual(pe);
             if(sm==NULL){ // nothing found, point is possible 
                           //located in exterior
                pos++;
             }  else if(sm->Contains(*p)){
                res.SetBI(true);
                done = true;
             } else if(sm->isInsideAbove()){
                AvlEntry::x = xpos;
                AvlEntry::compexact = true;
                sss.remove(*sm);
                assert(sss.Check(cerr));
                innercand = true;
                pos++;
             }  else {
                 pos++;
             }          
          }
        } 
     } else if(cx>xpos){
       // perform the point
       AvlEntry pe(p);
       AvlEntry::x = xpos;
       AvlEntry::compexact=false;
       const AvlEntry* sm = sss.GetNearestSmallerOrEqual(pe);
       if(sm==NULL){ // nothing found, point is located in exterior
          if(innercand){
            res.SetII(true);
          } else {
            res.SetEI(true);
          }
       }  else if(sm->Contains(*p)){
          res.SetBI(true);
       } else if(sm->isInsideAbove()){
          res.SetII(true);
       }  else if(innercand){
          res.SetII(true);
       } else {
          if(innercand){
            res.SetII(true);
          } else {
            res.SetEI(true);
          }
       }         
       done = true;
     } else {
       AvlEntry::x = cx;
       if(hs->IsLeftDomPoint()){
           AvlEntry::compexact = true;
           bool ins = sss.insert(entry);    
           assert(ins);
           assert(sss.Check(cerr)); 
           pos++;
       } else {
           AvlEntry::compexact= true;
           sss.remove(entry);
           assert(sss.Check(cerr)); 
           pos++;
       }
     }
  }
  if(!done){
    if(innercand){
      res.SetII(true);
    } else {
      res.SetEI(true);  
    }
  }

}



/*
~GetInt9M~

Computation of the 9 intersection matrix for a set of points and a 
region value. 

*hacked* should be changed to a plane sweep algorithm

[3] O(~m~~n~) where m and n  are the sizes of the parameters .

*/
void GetInt9M(Region const* const reg, Points const* const ps, Int9M& res){
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
         return; 
      }
      res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_r_ps++;
#endif
      return;
   } 
   res.SetIE(true);
   res.SetBE(true);
   if(ps->IsEmpty()){ // no more intersections can be found
#ifdef TOPOPS_USE_STATISTIC
      bb_r_ps++;
#endif
      return;
   }
  // bounding box test
  Rectangle<2> regbox = reg->BoundingBox();
  Rectangle<2> pbox = ps->BoundingBox();
  if(!regbox.Intersects(pbox)){ // disjoint objects
    res.SetEI(true);
#ifdef TOPOPS_USE_STATISTIC
    bb_r_ps++;
#endif
    return;
  }

  AVLTree<AvlEntry> sss;
  int size_reg = reg->Size(); 
  int pos_reg = 0;
  int size_poi = ps->Size();
  int pos_poi = 0;
  bool done = false;

  vector<Point> cps; // all points located on the same x coordinate
  // fill the vector
  const Point* cp;
  double x_poi;
  ps->Get(pos_poi,cp);
  cps.push_back(*cp);
  x_poi = cp->GetX();
  bool d = false;
  pos_poi++;
  while(pos_poi<size_poi && ! d){
     ps->Get(pos_poi,cp);
     if(AlmostEqual(cp->GetX(),x_poi)){ // p is part of this group
       cps.push_back(*cp);
       pos_poi++;
     } else { // start of the next group
       d = true;
     }
  }
  const HalfSegment* hs;
  while(!done){
    reg->Get(pos_reg,hs);
    Point p = hs->GetDomPoint();
    double x_reg = p.GetX();
    bool isLeft = hs->IsLeftDomPoint();
    if(AlmostEqual(x_reg,x_poi)){
      AvlEntry::x = x_poi;
      AvlEntry e(hs);
      if(e.isVertical()){
        // check if there are points located on this vertical segment
        vector<Point>::iterator it, it_first, it_last;
        bool d = false;
        bool first = true;
        for(it = cps.begin();it!=cps.end() && !d; it++){
           Point p = *it;
           if(e.Contains(p)){
              if(first){
                 first = false;
                 it_first = it;
              }
              it_last = it;
           } 
        }  
        if(!first){ // there are points on this segment
           res.SetBI(true);
            it_last++;
           cps.erase(it_first,it_last);
        }
        pos_reg++; // next hs
      } else { // non-vertical segments
        __TRACE__
        if(isLeft){ // left endpoint
           AvlEntry e(hs);
           AvlEntry::compexact = true;
           bool ins = sss.insert(e);
           assert(ins);
           assert(sss.Check(cerr));
        } else { // right endpoint 
           // check for points located at this point
           bool d = false;
           for(vector<Point>::iterator it = cps.begin();
               it!=cps.end() && !d; it++){
              if(AlmostEqual(p,*it)){
                res.SetBI(true);
                d = true; 
                cps.erase(it);
              }
           }
           AvlEntry::compexact=true;
           sss.remove(e);
           assert(sss.Check(cerr));
        }
        pos_reg++;
      } 
    } else if (x_reg<x_poi){
      
      AvlEntry::x = x_reg;
      AvlEntry e(hs);
      if(!e.isVertical()){ // ignore vertical segments
         if(isLeft){
           AvlEntry::compexact = true;
           bool ins = sss.insert(e);
           assert(ins);
           assert(sss.Check(cerr));
         } else {
           AvlEntry::compexact = true;
           sss.remove(e);
           assert(sss.Check(cerr));
         }
      }
      pos_reg++;
    } else { // x_hs > x_poi
       // check the current pointset again the content of sss
       AvlEntry::x = x_poi;
       bool full = false;
       AvlEntry::compexact = false;
       

       for(vector<Point>::iterator i = cps.begin();i!=cps.end() && !full;i++){
          Point cp = *i;
          AvlEntry e(&cp);
          const AvlEntry* seg = sss.GetNearestSmallerOrEqual(e);
          if(seg==NULL){ // point under all segments
            res.SetEI(true);
          } else if(seg->Contains(*i)){ // on boundary
            res.SetBI(true);
          } else if(seg->isInsideAbove()){ 
            res.SetII(true);
          } else { 
            res.SetEI(true);
          }
          full = res.GetEI() && res.GetII() && res.GetBI();
       }
       if(full || pos_poi >= size_poi){ 
         cps.clear();
         done = true;
       } else { // build the next group of point
         cps.clear();
       }
    }
    if(cps.empty()){ // all points processed, get the next group
       if(pos_poi>=size_poi){ // no points available
         done = true;
       } else {
         ps->Get(pos_poi,cp);
         cps.push_back(*cp);
         x_poi = cp->GetX();
         bool d = false;
         pos_poi++;
         while(pos_poi<size_poi && ! d){
           ps->Get(pos_poi,cp);
           if(AlmostEqual(cp->GetX(),x_poi)){ // p is part of this group
              cps.push_back(*cp);
              pos_poi++;
            } else { // start of the next group
              d = true;
            }
         }
       }
    }
    if(res.GetII() && res.GetEI() && res.GetBI()){
        done = true; // all possible intersections found
    }
    if(pos_reg>=size_reg){
       done = true;
    }
  }
  if(pos_poi<size_poi-1 || !cps.empty()){ // there are points right of reg
    res.SetEI(true);
  }  

}

/*
~GetInt9M~

This function computes the 9-intesection matrix for two line objects.

~Complexity~ $O(n^2)$

*/
void GetInt9M(Line const* const line1, Line const* const line2, Int9M& res){
#ifdef TOPOPS_USE_STATISTIC
  GetCalls_l_l++;
#endif
  res.SetValue(0);;
  res.SetEE(true);
  // check for emptyness
  if(line1->IsEmpty()){
    if(line2->IsEmpty()){ // no more intersection possible
#ifdef TOPOPS_USE_STATISTIC
       bb_l_l++;
#endif
       return; 
    }else{
      res.SetEI(true);
      if( NumberOfEndpoints(line2,1)>0){
         res.SetEB(true);
      }
#ifdef TOPOPS_USE_STATISTIC
      bb_l_l++;
#endif
      return;
    }
  }

  if(line2->IsEmpty()){
     res.SetIE(true);
     if(NumberOfEndpoints(line1,1)>0){
        res.SetBE(true);
     }
#ifdef TOPOPS_USE_STATISTIC
     bb_l_l++;
#endif
     return;
  }
  // bounding box check
  Rectangle<2> bbox1 = line1->BoundingBox();
  Rectangle<2> bbox2 = line2->BoundingBox();
  if(!bbox1.Intersects(bbox2)){
     res.SetIE(true);
     res.SetEI(true);
     if(NumberOfEndpoints(line1,1)>0){
       res.SetBE(true);
     }
     if(NumberOfEndpoints(line2,1)>0){
       res.SetEB(true);
     }
#ifdef TOPOPS_USE_STATISTIC
     bb_l_l++;
#endif
     return;
  }

   HalfSegment const* chs1;
   //HalfSegment const* chs2;
   
   int size1 = line1->Size();
   //int size2 = line2->Size();
   //bool done=false;
   Point dp1;
   Point dp2;

   for(int i=0;i<size1;i++){
      line1->Get(i,chs1);
      Point dp1 = chs1->GetDomPoint();
      // process endpoint if present 
      if(HasEndpointAt(line1,i)){
         

      }
               


   }  
   


}





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


const int LEFT      = 1;
const int RIGHT     = 2;
const int COMMON = 4;

/*
3 Class Segment

This class is used for inserting into an avl tree during a plane sweep.


*/

class AVLSegment; 
ostream& operator<<(ostream& o, const AVLSegment& s);


class AVLSegment{

public:

/*
3.0 ~Standard Constructor~

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
3.1 ~Constructor~

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
3.2 ~Copy Constructor~

*/
   AVLSegment(const AVLSegment& src){
      Equalize(src);
   }

/*
3.3. Assignmet operator

*/

  AVLSegment& operator=(const AVLSegment& src){
    Equalize(src);
    return *this;
  }

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
3.3 ~Destructor~

*/
   ~AVLSegment() {}


/*
3.4 ~crosses~

Checks whether this segment and s have an intersection point of their
interior. 

*/
 bool crosses(const AVLSegment& s) const{
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
        double x = x1; // compute y for s
        double y =  s.y1 + ((x-s.x1)/(s.x2-s.x1))*(s.y2 - s.y1);
        return !AlmostEqual(y1,y) && !AlmostEqual(y2,y) &&
               (y>y1)  && (y<y2)
               && !AlmostEqual(s.x1,x) && !AlmostEqual(s.x2,x) ;
    }

    if(s.isVertical()){
       double x = s.x1;
       double y = y1 + ((x-x1)/(x2-x1))*(y2-y1);
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
    return !AlmostEqual(x1,xs) && !AlmostEqual(x2,x2) && // not an endpoint   
           !AlmostEqual(s.x1,xs) && !AlmostEqual(s.x2,xs) && // of any segment
           (x1<xs) && (xs<x2) && (s.x1<xs) && (xs<s.x2);
}

/*
3.5 ~Extends~

This function returns true, iff this segment is an extension of 
the argument, i.e. if the right point of ~s~ is the left point of ~this~
and the slopes are equal.

*/
  bool extends(AVLSegment s){
     return pointEqual(x1,y1,s.x2,s.y2) &&
            compareSlopes(s)==0;
  }


/*
3.5 ~CompareTo~

Compares this with s. The x intervals must overlap.

*/

 int compareTo(const AVLSegment& s) const{
    
    assert(xOverlaps(s));
    

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
3.6 ~isVertical~

Checks whether this segment is vertical.

*/

 bool isVertical() const{
     return AlmostEqual(x1,x2);
 }


/*
3.7 getInsideAbove

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

/*
3.7 Comparison Operators 

*/


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
3.8 ~split~

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
          common.x2 = min(x2,s.x2);
          common.y2 = min(y2,s.y2);
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
          right.x2 = max(x2,s.x2);
          right.y2 = max(y2,s.y2);
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
       left.x1 = min(x1, s.x1);
       left.y1 = min(y1, s.y1);
       left.x2 = max(x1, s.x1);
       left.y2 = max(y1, s.y1);
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
       common.x2 = min(x2, s.x2);
       common.y2 = min(y1, s.y2);
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
       right.x2 = max(x2, s.x2);
       right.y2 = max(y2, s.y2);
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
3.10 ~ Get functions ~

*/
  double getX1() const { return x1; }

  double getX2() const { return x2; }

  double getY1() const { return y1; }

  double getY2() const { return y2; }

  ownertype getOwner() const { return owner; }

  bool getInsideAbove_first() const { return insideAbove_first; }
  bool getInsideAbove_second() const { return insideAbove_second; }


/*
~overlaps~

Checks whether both this segment and __s__ have a common segment.

*/
   bool overlaps(const AVLSegment& s) const{
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
3.6 public data members 

These members are not used in this class. So the user of
this class can change them without any problems within this
class itself.

*/
 int con_below;  // should be used as coverage number
 int con_above;  // should be used as coverage number


/*
3.7 checkCon

If the public data members are used as coverage numbers, they must
fullfill some conditions. Because maximum two regions are concerned,
ecah coverage number must be in range {0,1,2}. Because each segment
is assigned to a region, the sum of the coverage numbers must be
proper greater than zero.

*/
  bool checkCon(AVLTree<AVLSegment>* tree=0){

       bool res = ((con_below + con_above) >0) &&
                  ( (con_below>=0) && (con_below<=2)) &&
                  ( (con_above>=0) && (con_above<=2)); 

       if(!res){
          cout << "checkCondFailed for " << (*this);
          if(tree){
              cout << "the tree is " << *tree << endl << endl;
          }
       }


       assert((con_below + con_above) >0);
       assert( (con_below>=0) && (con_below<=2)); 
       assert( (con_above>=0) && (con_above<=2)); 
       return true;
  }


/*
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
    if(pointEqual(x1,y1,x2,y2)){
       return false;
    }
    return (AlmostEqual(x1,x2) && y1 < y2 ) ||  
          (!AlmostEqual(x1,x2) && x1 < x2);
 }

/*
~pointBelow~

This function checks if the point defined by (x,y) is below this
segment. The x coordinate of the point must be located in the
x range of this segment.

*/

  bool pointBelow(const double x,const double y)const{
    if(AlmostEqual(x1,x2)){ // a vertical segment
       return !AlmostEqual(y,y1) && y < y1;
    } else {
      double res1 = (y*(x2-x1));
      double res2 = (y1*(x2-x) + y2*(x-x1));
      return  !AlmostEqual(res1,res2) && (res1 < res2);
    }
  }

/*
~pointAbove~

This function checks if the point defined by (x,y) is above this
segment.

*/
  bool pointAbove(const double x , const double y) const{
    if(AlmostEqual(x1,x2)){ // vertical segment
      return !AlmostEqual(y,y2) && y > y2;
    } else {
      double res1 = (y*(x2-x1));
      double res2 = (y1*(x2-x) + y2*(x-x1));
      return !AlmostEqual(res1,res2) && (res1 > res2);
    }
  }

/*
~contains~

Checks whether the the defined by (x,y) is located anywhere on this
segment.

*/
   bool contains(const double x,const  double y)const{
     if(pointEqual(x,y,x1,y1) || pointEqual(x,y,x2,y2)){
        return true;
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
~compareSlopes~

compares the slopes of __this__ and __s__. The slope of a vertical
segemnt is greater than all other slopes. 

*/
   int compareSlopes(const AVLSegment& s) const{
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

Computes the y value for the spcified  __x__. 
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
4 Shift Operator 

*/
ostream& operator<<(ostream& o, const AVLSegment& s){
    s.Print(o);
    return o;
}




/*
Selects the mimimum halfsegment from reg1, reg2, q1, and q2.
If no values are availablee, the return value will be __none__. 
In this case, __result__ remains unchanged. Otherwise, __result__
is set to the minimum value found.
Otherwise, it will be first or second depending on the region 
containing the minimum. If some halfsegments are equal, the one
from the first region is selected. 
Note: pos1 and pos2 are increased automatically. In the same way, 
      the topmost element of the selected queue is deleted. 


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
                     HalfSegment& result
                    ){


  const HalfSegment* values[4];
  int number = 0; // number of available values
  // read the available elements
  if(pos1<reg1->Size()){
     reg1->Get(pos1,values[0]);
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
  if(pos2<reg2->Size()){
     reg2->Get(pos2,values[2]);
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
  switch(index){
    case 0: pos1++; return first; 
    case 1: q1.pop();  return first;
    case 2: pos2++;  return second;
    case 3: q2.pop();  return second;
    default: assert(false);   
  }
  return none;

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




void GetInt9M(Region const* const reg1, Region const* const reg2, Int9M& res){
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
       return; 
    }else{
      res.SetEI(true);
      res.SetEB(true);
#ifdef TOPOPS_USE_STATISTIC
      bb_r_r++;
#endif
      return;
    }
  }

  if(reg2->IsEmpty()){
     res.SetIE(true);
     res.SetBE(true);
#ifdef TOPOPS_USE_STATISTIC
     bb_r_r++;
#endif
     return;
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
     return;
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

  while( ((owner=selectNext(reg1,pos1, reg2,pos2, q1,q2,nextSeg))!=none)
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
          res.SetBB(true); // common point found
       }
    }


 // debug::start
    //cout << "process semnet" << current << "   ";
    //cout << (nextSeg.IsLeftDomPoint()?"LEFT":"RIGHT");
    //cout << endl << endl; 

 // debug::end   



    if(nextSeg.IsLeftDomPoint()){
        AVLSegment left, common, right;
        if(member){ // there is an overlapping segment in the tree
           // check for valid region representation
           assert(member->getOwner() != current.getOwner()); 
           assert(member->getOwner() != both);

           res.SetBB(true); // common boundary found
           bool iac = current.getOwner()==first?current.getInsideAbove_first()
                                            :current.getInsideAbove_second();
           bool iam = member->getOwner()==first?member->getInsideAbove_first()
                                           :member->getInsideAbove_second();

           if(iac!=iam){
             res.SetIE(true);
             res.SetEI(true); 
           } else {
             res.SetII(true);
             // res.setEE(true); // alreay done in initialisation
           }

           int parts = member->split(current,left,common,right);

           sss.remove(*member);
           assert(sss.Check(cerr));

           if(parts & LEFT){   // there is a left part
              left.checkCon(); 
              sss.insert(left);  // simulates a left event.
              assert(sss.Check(cerr));
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

           common.checkCon(); 
           sss.insert(common);
           
           assert(sss.Check(cerr));
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
              return;
           }    
           // check crossing right
           if(rightN && rightN->crosses(current)){
              assert(rightN->getOwner()!=current.getOwner());
              // computation of the intersections
              res.Fill(); 
              done = true;
              return;
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
                  res.SetEE(true);
              } else if (current.con_below == 1) {
                  if(current.getInsideAbove()){
                     if(current.getOwner()==first){
                        res.SetEI(true);

                     } else {
                        res.SetIE(true);
                     }
                  } else {
                     if(current.getOwner()==first){
                        res.SetIE(true);
                     } else {
                        res.SetEI(true);

                     }
                  }
              } else {
                 assert(current.con_below==2);
                 res.SetII(true); 
              }
              // check for possible common endpoints with the neighbours
              if(leftN && leftN->getOwner()!=current.getOwner()){
                 if(leftN->intersects(current)){
                     res.SetBB(true);
                 }
              }
              if(rightN && rightN->getOwner()!=current.getOwner()){
                 if(rightN->intersects(current)){
                     res.SetBB(true);
                 }
              }
              current.checkCon(&sss); 
              sss.insert(current);
              assert(sss.Check(cerr));

           } else {
              // check for possible common points with the neighbours
              if(leftN && leftN->getOwner()!=current.getOwner()){
                 if(leftN->intersects(current)){
                     res.SetBB(true);
                 }
              }
              if(rightN && rightN->getOwner()!=current.getOwner()){
                 if(rightN->intersects(current)){
                     res.SetBB(true);
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
               left.checkCon();
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
               left.checkCon();
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
                res.SetEE(true);
            } else if (current.con_below == 1) {
                if(current.getInsideAbove()){
                   if(current.getOwner()==first){
                      res.SetEI(true);

                   } else {
                      res.SetIE(true);
                   }
                } else {
                   if(current.getOwner()==first){
                      res.SetIE(true);
                   } else {
                      res.SetEI(true);

                   }
                }
            } else if(current.con_below==2){
               res.SetII(true); 
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
             current.checkCon();           
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
                case first: res.SetBE(true); 
                            res.SetIE(true); 
                            res.SetEE(true); 
                            break;
                case second: res.SetEB(true); 
                             res.SetEI(true); 
                             res.SetEE(true); 
                             break;
                default: assert(false);
              }
           } else if(member->con_below==2 || member->con_above==2){ //interior
              switch(member->getOwner()){
                case first: res.SetBI(true); 
                            res.SetII(true); 
                            res.SetEI(true); 
                            break;
                case second: res.SetIB(true); 
                             res.SetII(true); 
                             res.SetIE(true);
                             break;
                default: assert(false);
              }
           }else {
              assert(false);
           }    
        }

        sss.remove(*member);
        assert(sss.Check(cerr));

       
        if( leftN && rightN && !leftN->innerDisjoint(*rightN)){

           // leftN and rightN are crossing or one of the segments
           // splits the other one by its right endpoint 
           if(leftN->crosses(*rightN)){
              assert(leftN->getOwner() != rightN->getOwner()); 
              res.Fill(); // we are done
              return;
           } else if(leftN->ininterior(rightN->getX2(),rightN->getY2())){
              AVLSegment left, right;
              leftN->splitAt(rightN->getX2(), rightN->getY2(),left,right);
              sss.remove(*leftN);
              leftN = &right;
              assert(sss.Check(cerr));
              left.checkCon();
              sss.insert(left);
              assert(sss.Check(cerr));
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
              assert(sss.Check(cerr));
              left.checkCon();
              sss.insert(left);
              assert(sss.Check(cerr));
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

  } 

  


}



/*
4 Implementation of the TopRel Value Mappings 

4.1 TopRel

This function is a template for all computations of topological
predicates. It requires the Implementation of the function GetInt9M
for all instantiations.

*/
template<class type1, class type2>
int TopRel(Word* args, Word& result, int message,
           Word& local, Supplier s){
  result = qp->ResultStorage(s);
  type1* p1 = (type1*) args[0].addr;
  type2* p2 = (type2*) args[1].addr;
  Int9M matrix; 
  GetInt9M(p1,p2,matrix);
  *((Int9M*)result.addr) = matrix; 
  return 0;
}

/*
4.2 TopRelSym

This function is symmetric to the ~TopRel~ functions. This function avoids
the implementation of symmetric GetInt9M functions.

For example, we have implemented the function GetInt9M(Points*, Point*).
To provide [secondo] operators for both  TopRel(points,point) and 
TopRel(point,points), you can use TopRel<Points,Point> and 
TopRelSym<Points,Point> as value mappings.

*/
template<class type1, class type2>
int TopRelSym(Word* args, Word& result, int message,
           Word& local, Supplier s){

  result = qp->ResultStorage(s);
  type1* p2 = (type1*) args[0].addr;
  type2* p1 = (type2*) args[1].addr;
  Int9M matrix(0);
  GetInt9M(p1,p2,matrix);
  matrix.Transpose();
  *((Int9M*)result.addr)=matrix; // correct the swapping 
  return 0;
}


/*
5 Statistical Information

The next two functions implement the value mapping for the
statistical infomation operators.

*/

#ifdef TOPOPS_USE_STATISTIC
int TopOpsResetStatVM(Word* args, Word& result, int message, 
                    Word& local, Supplier s){
   result = qp->ResultStorage(s);
   ResetStatistic();
   ((CcBool*)result.addr)->Set(true,true);
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
   ((FText*)result.addr) ->Set(true,ts.str().c_str());
   return 0;
}

#endif

/*
5 Implementation of the TopPred Value Mappings

5.1 Simple Value Mappings

Some computations of the 9 intersection matrix are simple enough to
compute it within the TopPred operator without exploiting properties
of the arguments for an early exit. We handle all such functions 
by two template functions in the same way as for the TopRel operator.

~TopPred~

*/
template<class type1, class type2>
int TopPred(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  
    result = qp->ResultStorage(s);
    type1* p1 = (type1*) args[0].addr;
    type2* p2 = (type2*) args[1].addr;
    CcString* name = (CcString*) args[2].addr;
    PredicateGroup* pg = (PredicateGroup*) args[3].addr;
    const STRING_T* n = name->GetStringval(); 
    Cluster* theCluster = pg->GetClusterOf(n);
    if(!theCluster){// name not found within group
        ((CcBool*)result.addr)->Set(true,false);
        return 0;
    }
    Int9M matrix;
    GetInt9M(p1,p2,matrix);
    ((CcBool*)result.addr)->Set(true,theCluster->Contains(matrix));
    delete theCluster;
    return 0;
}

/*
~TopPredSym~

The symmetric function to ~TopPred~.

*/
template<class type1, class type2>
int TopPredSym(Word* args, Word& result, int message,
                    Word& local, Supplier s){
  
    result = qp->ResultStorage(s);
    type2* p1 = (type2*) args[0].addr;
    type1* p2 = (type1*) args[1].addr;
    CcString* name = (CcString*) args[2].addr;
    PredicateGroup* pg = (PredicateGroup*) args[3].addr;
    const STRING_T* n = name->GetStringval(); 
    Cluster* theCluster = pg->GetClusterOf(n);
    if(!theCluster){// name not found within group
        ((CcBool*)result.addr)->Set(true,false);
        return 0;
    }
    Int9M matrix;
    GetInt9M(p1,p2,matrix);
    matrix.Transpose();
    ((CcBool*)result.addr)->Set(true,theCluster->Contains(matrix));
    delete theCluster;
    return 0;
}


/*
6  Operator specifications

*/


const string TopRelSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> {point, points, line, region} x "
   "  {points, points, line, region} -> int9m </text--->"
   " \" toprel(_ _) \" "
   " <text>computes the topological relationship of the arguments</text--->"
    "  \" query toppred(c1,c2) \" ))";

const string TopPredSpec =
   "((\"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   " ( <text> so1 x so2 x string x predicategroup -> bool "
   " where o1, o2 in {point, points, line, region}</text--->"
   " \" topred(_ _ _ _) \" "
   " <text> checks whether the topological relationship between"
   " the spatial objects is part of the cluster defined by name"
   " and group</text---> "
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
7 Value Mapping Arrays

The following arrays collect the value mappings to enable overloaded 
operations.

*/

ValueMapping TopRelMap[] = {
       TopRel<Point,Point> , TopRel<Points,Point>,
       TopRelSym<Point,Points>, TopRel<Points,Points>, TopRel<Line,Point>,
       TopRelSym<Point,Line>,TopRel<Line,Points>,TopRelSym<Points,Line>,
       TopRel<Region,Point>,TopRelSym<Point,Region>,
       TopRel<Region,Points>, TopRelSym<Points,Region>,TopRel<Region,Region>};

ValueMapping TopPredMap[] = {
       TopPred<Point,Point> , TopPred<Points,Point>,
       TopPredSym<Point,Points>, TopPred<Points,Points>, TopPred<Line,Point>,
       TopPredSym<Point,Line>,TopPred<Line,Points>,TopPredSym<Points,Line>,
       TopPred<Region,Point>,TopPredSym<Point,Region>,
       TopPred<Region,Points>, TopRelSym<Points,Region>,TopRel<Region,Region>};



/*
8 Selection Function

The value mapping array containg the value mapping functions for both
operator in the same order. For this reason it is sufficient to have
one single selection function for both functions.

*/

static int TopOpsSelect(ListExpr args){
   int len = nl->ListLength(args);
   if(len!=2 && len !=4){
       return -1; 
   }
   if( (nl->AtomType(nl->First(args))!=SymbolType) ||
       (nl->AtomType(nl->Second(args))!=SymbolType)){
       return -1;
   } 
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

   cerr << "selection function does not allow (" << type1 << " x " 
        << type2 << ")" << endl;
   
   ListExpr tmreslist;
   string tmres;
   if(len==2){
     tmreslist = TopRelTypeMap(args);
     nl->WriteToString(tmres,tmreslist);
   }else{
     tmreslist = TopPredTypeMap(args);
     nl->WriteToString(tmres,tmreslist);
   }
   cerr << "SelectionFunction called when tm returns " +tmres+"\n";
   return -1;
   //   assert(false);
} 


/*
9 Definition of operators

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
10 Creating the algebra

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
11 Initialization of the Algebra
   
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

