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
static long tocount;

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
3.3.2 IsImplementedTopRel

This function returns true if the TopRel value mapping 
is implemented for the given combination of types.

*/
bool IsImplementedTopRel(ListExpr type1, ListExpr type2){
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
    cout << t1 << " x " << t2 << " is not implemented" << endl;
    return false;
}

/*
3.3.3 IsImplementedTopPred

This function returns true if the TopPred value mapping 
is implemented for the given combination of types.

*/

inline bool IsImplementedTopPred(ListExpr type1, ListExpr type2){
   // there is a standard implementation based on the 
   // TopRel operator. Therefore we can assume that for each
   // TopRel operator an TopPred operator is implemented
   return IsImplementedTopRel(type1,type2);
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
   
   if(!IsImplementedTopPred(o1,o1)){
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
   if(!IsImplementedTopRel(nl->First(args),nl->Second(args))){
       ErrorReporter::ReportError("combination not implemented yet");
       return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("int9m");
}



/*
3.3.4 Support functions

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
  static int Compare(Point const* const p1, Point const* const p2){
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
3.4 Computation of the 9 intersection matrices

~GetInt9M~

This function computes the 9-intersection matrix between two point values.
Because a single point is very simple, no bounding box tests are
performed.

Complexity: O(1)

*/

Int9M  GetInt9M(Point* p1 , Point*  p2){
  Int9M res(0);
  // in each case, the exteriors intersect
  res.SetEE(true);
  if(Compare(p1,p1)==0){
    res.SetII(true);
  }else{
    res.SetIE(true);
    res.SetEI(true);
  }
  return res;
}

/*
~GetInt9M~

The next function computes the 9 intersection matrix between a 
point value and a points value. 

Complexity: O(log(n))  where ~n~ is the number of points in the __points__
value.

*/
Int9M GetInt9M(Points*  ps, Point* p){
  // initialization
  Int9M res(0);
  res.SetEE(true); // holds always
  
  // check for emptyness
  if(ps->IsEmpty()){ // the simples case
     res.SetEI(true);
     return res;
  }  
  
  // bounding box check
  Rectangle<2> box_ps = ps->BoundingBox();
  Rectangle<2> box_p  = p->BoundingBox();
  if(!box_p.Intersects(box_ps)){
      res.SetIE(true);
      res.SetEI(true);
      return res;
  }

  int size = ps->Size();
  if(size>1){
     res.SetIE(true);
  }
  if(ps->Contains(p)){
     res.SetEI(true);
  }
  return res;
}


/*
~GetInt9M~

This function returns the 9 intersection matrix describing the 
topological relationship between two __points__ values.

Complexity: O(~n~+~m~) , where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively, where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively.

*/
Int9M GetInt9M(Points const* const  ps1, Points const*  const ps2){
   int n = ps1->Size();
   int m = ps1->Size();
   Int9M res(0);
   res.SetEE(true);

   if(n<=0 && m<=0){
      // there are no inner parts which can intersect any part
     return res;
   }
   if(n<=0){
      // some points of ps2 are in the exterior of ps1
      res.SetEI(true);
      return res;
   }
   if(m<=0){
      // symmetrically to the previous case
      res.SetIE(true);
      return res;
   }
   // bounding box check
   Rectangle<2> bbox1 = ps1->BoundingBox();
   Rectangle<2> bbox2 = ps2->BoundingBox();
   if(!bbox1.Intersects(bbox2)){
      // non empty disjoint points values
      res.SetIE(true);
      res.SetEI(true);
      return res;
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
       assert(i1<n);
       assert(i2<m);
       p1 = NULL;
       p2 = NULL;
       ps1->Get(i1,p1);
       ps2->Get(i2,p2);
       int cmp = Compare(p1,p2);
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
       
       done= ((i1==n-1) || (i2==m-1)) || // end of one points value reached
             (ii && ie && ie);           // no more intersections possible
   }while(!done);    

   if(ii && ie && ei){ // maximum count of intersections
      return res;  
   }
   if(i1<n-1){ // ps1 has further points
      res.SetIE(true);
   }
   if(i2<m-1){ // ps2 has further points
      res.SetEI(true);
   }
   return res;
}




/*
~NumberOfEndpoints~

This function computes the number of endpoints of __line__.
The function stops the computation when the number of endpoints
is greater than or equals to __stop__. If this argument holds a
value [<=] 0, the function will not stop before all halfsegments
are processed.

Complexity: O(n)

*/
int  NumberOfEndpoints(CLine const* const line, const int stop=-1 ){
  if(line->IsEmpty()){ // an empty line has no endpoints
     return 0;
  }  
  int size = line->Size();
  // because the first sort criteria of halfsegments is the 
  // dominating point, we have just to check whether a halfsegment
  // at index+1 or index-1 has the same dominating point
  CHalfSegment const* chs1=NULL;  
  CHalfSegment const* chs2=NULL;
  Point p1;
  Point p2;
  line->Get(0,chs1);
  p1 = chs1->GetDPoint();
  int pos=1;
  int num=0; // no endpoint up to now
  while(pos<size){
    line->Get(pos,chs2);
    p2 = chs2->GetDPoint();
    if(p1!=p2){ // found an endpoint
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
~GetInt9M~

This function computes the 9-intersection matrix for a line and a single point.

Complexity: O(n)

*/
Int9M GetInt9M(CLine const* const line, Point const* const point){
   Int9M res(0);
   res.SetEE(true);
   if(line->IsEmpty()){
     res.SetEI(true);
     return res;
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
      return res;
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
         return res; 
       }
       if(InnerContains(chs,thePoint)){
           res.SetII(true);
           if(endpoints>0){
              res.SetBE(true);
           } 
           return res;
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
   return res;
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
  tocount++;
  result = qp->ResultStorage(s);
  type1* p1 = (type1*) args[0].addr;
  type2* p2 = (type2*) args[1].addr;
  Int9M matrix=GetInt9M(p1,p2);
  ((Int9M*)result.addr)->Equalize(matrix);
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
  type2* p2 = (type2*) args[0].addr;
  type1* p1 = (type1*) args[1].addr;
  Int9M matrix=GetInt9M(p2,p1);
  matrix.Transpose(); // correct the swapping 
  ((Int9M*)result.addr)->Equalize(matrix);
  return 0;
}


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
    Int9M matrix = GetInt9M(p1,p2);
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
    Int9M matrix = GetInt9M(p1,p2);
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


/*
7 Value Mapping Arrays

The following arrays collect the value mappings to enable overloaded 
operations.

*/

ValueMapping TopRelMap[] = {
       TopRel<Point,Point> , TopRel<Points,Point>,
       TopRelSym<Point,Points>, TopRel<Points,Points>, TopRel<CLine,Point>,
       TopRelSym<Point,CLine>  };

ValueMapping TopPredMap[] = {
       TopPred<Point,Point> , TopPred<Points,Point>,
       TopPredSym<Point,Points>, TopPred<Points,Points>, TopPred<CLine,Point>,
       TopPredSym<Point,CLine> };



/*
8 Selection Function

The value mapping array containg the value mapping functions for both
operator in the same order. For this reason it is sufficient to have
one single selection function for both functions.

*/

static int TopOpsSelect(ListExpr args){
   // the type mapping ensures to have at least two elements 
   // and both elements are symbols
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
   cout << "try to compute combination " << type1 << " x " << type2 << endl;
   assert(false); // SelectionFunction inconsistent to IsImplemented
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



/*
10 Creating the algebra

*/
class TopOpsAlgebra : public Algebra {
  public:
     TopOpsAlgebra() : Algebra() {
        AddOperator(&toprel);
        AddOperator(&toppred);
      }
     ~TopOpsAlgebra(){}
} topOpsAlgebra;

/*
11 Initialization of the Algebra
   
*/
extern "C"
Algebra* InitializeTopOpsAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
    tocount = 0;
    nl = nlRef;
    qp = qpRef;
    return (&topOpsAlgebra);
}

