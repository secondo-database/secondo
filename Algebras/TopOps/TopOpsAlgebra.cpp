/*
----
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, Department of Computer Science,
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
//[toc] [\tableofcontents]
//[title] [ \title{TopOps-Algebra} \author{Thomas Behr} \maketitle]
//[times] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[<=] [\ensuremath{\leq{}}]
//[>=] [\ensuremath{\ge{}}]

[title]
[toc]

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
#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "LogMsg.h"

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

/*
3.2 Instances of NestedList and the QueryProcessor

*/

extern NestedList* nl;
extern QueryProcessor* qp;
Coord  EPSILON = 0.000001;


/*
3.3 Statistical information 

The following variables collect some statistical information.

*/
int GetCalls_p_p;
int GetCalls_ps_p;
int GetCalls_ps_ps;
int GetCalls_l_p;
int GetCalls_l_ps;
int GetCalls_l_l;
int bb_p_p;
int bb_ps_p;
int bb_ps_ps;
int bb_l_p;
int bb_l_ps;
int bb_l_l;

/*
~ResetStatistic~

This functions sets all variables holding statistical
information to int initial values.

*/
static void ResetStatistic(){
 GetCalls_p_p=0;
 GetCalls_ps_p=0;
 GetCalls_ps_ps=0;
 GetCalls_l_p=0;
 GetCalls_l_ps=0;
 GetCalls_l_l=0;
 bb_p_p=0;
 bb_ps_p=0;
 bb_ps_ps=0;
 bb_l_p=0;
 bb_l_ps=0;
 bb_l_l=0;
}


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
bool HasEndpointAt(CLine const* line, const int pos){
  int size = line->Size();
  if((pos<0) || (pos>=size)){
     return false;
  }
  CHalfSegment const* chs;
  line->Get(pos,chs);
  Point p0 = chs->GetDPoint();
  
  Point p1(.0,.0);
  if(pos>0){
     line->Get(pos-1,chs); 
     p1 = chs->GetDPoint();
     if(p0==p1){
        return false;
     } 
  }
  if(pos<size-1){
    line->Get(pos+1,chs);
    p1 = chs->GetDPoint();
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
int  NumberOfEndpoints(CLine const* const line, 
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
  CHalfSegment const* chs1=NULL;  
  CHalfSegment const* chs2=NULL;
  Point p1;
  Point p2;
  line->Get(first,chs1);
  p1 = chs1->GetDPoint();
  int pos=first+1;
  int num=0; // no endpoint up to now
  while(pos<size){
    line->Get(pos,chs2);
    p2 = chs2->GetDPoint();
    if(p1!=p2){ // found an endpoint
      if( (pos==first+1) && first>0){
         // check if the former dp is equals
         CHalfSegment const* chs3;
         line->Get(first-1,chs3);
         Point p3 = chs3->GetDPoint();
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
      p1 = chs1->GetDPoint();
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

This fucntion checks if the halfsegment contains the point. 

*/

inline bool Contains(CHalfSegment const* const chs, const Point& point){
   double x = point.GetX();
   double y = point.GetY();
   Point p1 = chs->GetLP();
   Point p2 = chs->GetRP();
   double x1 = p1.GetX();
   double x2 = p2.GetX();
   double y1 = p1.GetY();
   double y2 = p2.GetY();
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
bool InnerContains(CHalfSegment const* const chs, const Point& point){
  double x = point.GetX();
  double y = point.GetY();
  Point p1 = chs->GetLP();
  Point p2 = chs->GetRP();
  double x1 = p1.GetX();
  double x2 = p2.GetX();
  double y1 = p1.GetY();
  double y2 = p2.GetY();
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
  GetCalls_p_p++;
  res.SetValue(0);
  // in each case, the exteriors intersect
  res.SetEE(true);
  if(Compare(p1,p1)==0){
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
  GetCalls_ps_p++;
  // initialization
  res.SetValue(true);
  res.SetEE(true); // holds always
  
  // check for emptyness
  if(ps->IsEmpty()){ // the simples case
     res.SetEI(true);
     bb_ps_p++;
     return;
  }  
  
  // bounding box check
  Rectangle<2> box_ps = ps->BoundingBox();
  Rectangle<2> box_p  = p->BoundingBox();
  if(!box_p.Intersects(box_ps)){
      res.SetIE(true);
      res.SetEI(true);
      bb_ps_p++;
      return;
  }

  int size = ps->Size();
  if(size>1){
     res.SetIE(true);
  }
  if(ps->Contains(p)){
     res.SetEI(true);
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


  

   GetCalls_ps_ps++;
   int n1 = ps1->Size();
   int n2 = ps2->Size();

   res.SetValue(0);
   res.SetEE(true);

   if(n1<=0 && n2<=0){
      // there are no inner parts which can intersect any part
     bb_ps_ps++;
     return;
   }
   if(n1<=0){
      // some points of ps2 are in the exterior of ps1
      res.SetEI(true);
      bb_ps_ps++;
      return;
   }
   if(n2<=0){
      // symmetrically to the previous case
      res.SetIE(true);
      bb_ps_ps++;
      return;
   }
   // bounding box check
   Rectangle<2> bbox1 = ps1->BoundingBox();
   Rectangle<2> bbox2 = ps2->BoundingBox();
   if(!bbox1.Intersects(bbox2)){
      // non empty disjoint points values
      res.SetIE(true);
      res.SetEI(true);
      bb_ps_ps++;
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
void GetInt9M(CLine const* const line, Point const* const point,Int9M& res){
   GetCalls_l_p++;
   res.SetValue(0);
   res.SetEE(true);
   if(line->IsEmpty()){
     res.SetEI(true);
     bb_l_p++;
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
      bb_l_p++;
      return;
   }

   // prefilter unsuccessful -> scan the halfsegments
   int size = line->Size(); 
   bool done = false;
   Point thePoint = (*point);
   CHalfSegment const* chs;
   Point p;
   int endpoints = NumberOfEndpoints(line,2);
   for(int i=0;(i<size) && !done; i++){
       line->Get(i,chs);
       p = chs->GetDPoint();
       if(p==thePoint){ // point on endpoint of chs
         done = true;
         if(i+1<size){
            line->Get(i+1,chs);
            p=chs->GetDPoint();
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

*/
void GetInt9M(CLine const* const line, Points const* const ps, Int9M& res){

   GetCalls_l_ps++;
   
   res.SetValue(0);
   res.SetEE(true);
   /** special case empty line **/ 
   if(line->IsEmpty()){
      bb_l_ps++;
      if(!ps->IsEmpty()){
         res.SetEI(true);
      }
      return;
   }
    
   // line net empty => interior of the line intersects the 
   // exterior of the point (dimension difference)
   res.SetIE(true);
  
   /** special case empty point set **/
   if(ps->IsEmpty()){
     // non-empty line
     int num = NumberOfEndpoints(line,1);
     if(num>0){
       res.SetBE(true);
     }
     bb_l_ps++; 
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
      bb_l_ps++;
      return;
   }

   // we have to scan the objects
   CHalfSegment const* chs=NULL; // current halfsegment
   Point dp;                     // dominating point of chs
   Point ndp;                    // non-dominating point of chs  
   double dpx;                   // x coordinate of dp
   double dpy;
   double ndpx;

   line->Get(0,chs);
   dp = chs->GetDPoint();
   dpx = dp.GetX();

   int ps_size = ps->Size();
   // array stored the state of each point in ps
   bool processed[ps_size];
   for(int i=0;i<ps_size;i++){
      processed[i] = false;
   }

   /* jump over all points left from dp **/
   int min = 0;
   int max = ps_size-1;
   int mid;
   Point const* psp=NULL; // current point in pointset
   double pspx; // x coordinate of psp
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
     dp = chs->GetDPoint();
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
       ndp = chs->GetSPoint();
       ndpx = ndp.GetX();
       dpy = dp.GetY();
       int pointpos=min;
       Point const* currentPoint;
       double point_x;
       double point_y;
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

This function computes the 9-intesection matrix for two line objects.

~Complexity~ $O(n^2)$

*/
void GetInt9M(CLine const* const line1, CLine const* const line2, Int9M& res){
  GetCalls_l_l++;
  res.SetValue(0);;
  res.SetEE(true);
  // check for emptyness
  if(line1->IsEmpty()){
    if(line2->IsEmpty()){ // no more intersection possible
       bb_l_l++;
       return; 
    }else{
      res.SetEI(true);
      if( NumberOfEndpoints(line2,1)>0){
         res.SetEB(true);
      }
      bb_l_l++;
      return;
    }
  }

  if(line2->IsEmpty()){
     res.SetIE(true);
     if(NumberOfEndpoints(line1,1)>0){
        res.SetBE(true);
     }
     bb_l_l++;
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
     bb_l_l++;
     return;
  }

   CHalfSegment const* chs1;
   //CHalfSegment const* chs2;
   
   int size1 = line1->Size();
   //int size2 = line2->Size();
   //bool done=false;
   Point dp1;
   Point dp2;

   for(int i=0;i<size1;i++){
      line1->Get(i,chs1);
      Point dp1 = chs1->GetDPoint();
      // process endpoint if present 
      if(HasEndpointAt(line1,i)){
         

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
To provide [secondo] operators for both  TopRel(points,point) and TopRel(point,points),
you can use TopRel<Points,Point> and TopRelSym<Points,Point> as value mappings.

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
      << "#BoxFilter(line, line): " << bb_l_l << endl;
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
    const STRING* n = name->GetStringval(); 
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
    const STRING* n = name->GetStringval(); 
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
   "  \" computes the topological relationship of the arguments \" "
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
       TopRelSym<Point,Points>, TopRel<Points,Points>, TopRel<CLine,Point>,
       TopRelSym<Point,CLine>,TopRel<CLine,Points>,TopRelSym<Points,CLine>  };

ValueMapping TopPredMap[] = {
       TopPred<Point,Point> , TopPred<Points,Point>,
       TopPredSym<Point,Points>, TopPred<Points,Points>, TopPred<CLine,Point>,
       TopPredSym<Point,CLine>,TopPred<CLine,Points>,TopPredSym<Points,CLine> };



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
Operator toprel(
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
        AddOperator(&toprel);
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
    ResetStatistic();
    return (&topOpsAlgebra);
}

