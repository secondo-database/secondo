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
//[secondo] [\textsc{Secondo}]
//[{] [\}]
//[}] [\}]

[title]
[toc]
[etoc]

1 General Description

This algebra connects the TopRelAlgebra and the SpatialAlgebra.
It implements functions computing the topological
relationships of spatial values. Furthermore it provides some
functions checking whether two objects are part of a cluster which
is given by a name together with a predicategroup. Basically, this
can also be implemented by computing the topological relationship and
check whether the result is contained in the given cluster. The advantage of a
separate implementation is that we can exit the computation early in
many cases.
Additionally, this algebra provides some topological predicates using the
standard predicate group.

Moreover, in this algebra set operations for spatial types (union2,
intersection2, difference2, commonborder2) are implemented.


2 Includes, Constants, and Definitions

*/

#include <iostream>
#include <sstream>
#include <queue>
#include <iterator>

#include "NestedList.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "LogMsg.h"
#include "Symbols.h"

#include "TopOpsAlgebra.h"
#include "SpatialAlgebra.h"
#include "AvlTree.h"
#include "TopRel.h"
#include "StandardTypes.h"
#include "AVLSegment.h"




/*
~A Macro useful for debugging ~

*/

//#define __TRACE__ cout << __FILE__ << "@" << __LINE__ << endl;
#define __TRACE__



extern NestedList* nl;
extern QueryProcessor* qp;






using namespace toprel;
namespace topops{

/*
5 Some Auxiliary Functions


*/

inline void SetII(Int9M& m, const bool useCluster,
                  Cluster& cluster,bool& done){
  if(!m.GetII()){
     m.SetII(true);
     if(useCluster){
        cluster.Restrict(II,true,false);
        done = done || cluster.isExtension(m) || cluster.IsEmpty();
     }
  }
}

inline void SetIB(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetIB()){
     m.SetIB(true);
     if(useCluster){
        cluster.Restrict(IB,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetIE(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetIE()){
     m.SetIE(true);
     if(useCluster){
        cluster.Restrict(IE,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetBI(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetBI()){
     m.SetBI(true);
     if(useCluster){
        cluster.Restrict(BI,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetBB(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetBB()){
     m.SetBB(true);
     if(useCluster){
        cluster.Restrict(BB,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetBE(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetBE()){
     m.SetBE(true);
     if(useCluster){
        cluster.Restrict(BE,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetEI(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetEI()){
     m.SetEI(true);
     if(useCluster){
        cluster.Restrict(EI,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetEB(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetEB()){
     m.SetEB(true);
     if(useCluster){
        cluster.Restrict(EB,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}

inline void SetEE(Int9M& m, const bool useCluster,
                  Cluster& cluster, bool& done){
  if(!m.GetEE()){
     m.SetEE(true);
     if(useCluster){
        cluster.Restrict(EE,true,false);
        done = done || cluster.isExtension(m)|| cluster.IsEmpty();
     }
  }
}





/*
7 Computation of the 9 intersection matrices

The following functions compute the 9 intersection matrices for
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
  if(AlmostEqual(*p1,*p2)){
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

Complexity: O(log(n) + 1)  where ~n~ is the number of points in the __ps__
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
        cluster.Restrict(IE,true,false);
        if(cluster.IsEmpty()){
           return false;
        }
     }
  }


  if(!(ps->Contains(*p))){ // Contains uses binary search
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

This function computes the 9 intersection matrix describing the
topological relationship between two __points__ values.

Complexity: O(~n~+~m~ + 1) , where ~n~ and ~m~ is the size of ~ps~1 and
~ps~2 respectively.

If ~useCluster~ is set to be __false__, the return value is always
__true__.   In this case, the parameter __res__ will contain the
9 intersection matrix describing the topological relationship between
~p1~1 and ~ps~2. In the other case, the matrix is not computed
completely. Instead, the return value will indicate whether the
topological relationship is part of ~cluster~.

*/
bool GetInt9M(Points* ps1, Points*  ps2,
              Int9M& res,
              const bool useCluster = false,
              Cluster cluster = Cluster(false)){

   if(useCluster && cluster.IsEmpty()){
       return false;
   }
   int n1 = ps1->Size();
   int n2 = ps2->Size();

   if(useCluster){ // restrict cluster to matrices which are realizable
                   // for the points/points combination

      int cb = cluster.checkBoxes(ps1->BoundingBox(),n1,
                                  ps2->BoundingBox(),n2);
      if(cb==1) {
          return true;
      }
      if(cb==2){
         return false;
      }

      cluster.Restrict(EE,true,false); // the extreiors always intersect

      // the boundary of a points value is empty, thereby no intersections
      // of this part with any other part may exist
      Int9M r(true, false, true, false, false, false, true,false,true);
      cluster.Restrict(r,false,false);

      if(cluster.IsEmpty()){
        return false;
      }
   }


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
   if(!bbox1.IntersectsUD(bbox2)){
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

   Point p1;
   Point p2;

   int i1=0;
   int i2=0;

   bool done = false;
   do{
       ps1->Get(i1,p1);
       ps2->Get(i2,p2);
       int cmp = p1.Compare(&p2);
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

*/


/*
~initBox~

Initializes the result and performs a bounding box check.
If the initialization is sufficient to determine the result,
i.e. if the line is empty, the result will be __true__. Otherwise,
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
parameter combination of the first two parameters are set in in the
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

bool isEmpty(const Point& p){
   return false;
}

bool isEmpty(const Points& ps){
   return ps.IsEmpty();
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

   if(useCluster){
      // line is not empty
      int cb = cluster.checkBoxes(line->BoundingBox(),line->Size()==0,
                                  point->BoundingBox(),
                                  isEmpty(*point));
      if(cb==1) {
         return true;
      }
      if(cb==2){
         return false;
      }
   }

   // restrict the cluster to valid matrices
   if(useCluster){
       cluster.Restrict(EE,true,false); // hold in each case
       cluster.Restrict(IE,true,false); // nonempty line

       // boundary of a point is empty
       Int9M m(true, false, true, true,false,true, true,false,true);
       cluster.Restrict(m,false,false);
       if(cluster.IsEmpty()){
         return false;
       }
   }

   // prefilter unsuccessful -> scan the halfsegments

   avltree::AVLTree<avlseg::AVLSegment> sss;
   priority_queue<avlseg::ExtendedHalfSegment,
                  vector<avlseg::ExtendedHalfSegment>,
                  greater<avlseg::ExtendedHalfSegment> > q;


   bool done = false;
   int posline = 0; // position within the line array
   avlseg::ownertype owner;

   int posPoint = pointdone1?1:0;

   avlseg::ExtendedHalfSegment resHs;
   Point resPoi;
   Point lastDomPoint;
   int lastDomPointCount = 0;
   // avoid unneeded expensive restrictions of the cluster
   avlseg::AVLSegment tmpL,tmpR;

   while(!done &&
         ((owner=selectNext(*line,q,posline,*point,
           posPoint,resHs,resPoi))!=avlseg::none)){

      if(owner==avlseg::second){ // event comes from the point(s) value
         avlseg::AVLSegment current(resPoi,avlseg::second);

         const avlseg::AVLSegment* leftN=0;
         const avlseg::AVLSegment* rightN=0;
         const avlseg::AVLSegment* member= sss.getMember(current,leftN,rightN);
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
        assert(owner==avlseg::first);

        // check for endpoints
        Point domPoint = resHs.GetDomPoint();

        // only check for dompoints if the segment is a new one (left)
        // or its actually stored in the tree
        avlseg::AVLSegment current(resHs, avlseg::first);
        const avlseg::AVLSegment* leftN=0;
        const avlseg::AVLSegment* rightN=0;
        const avlseg::AVLSegment* member= sss.getMember(current,leftN,rightN);
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
           avlseg::AVLSegment left1, left2, right1, right2;
           if(member){ //overlapping segment found
              if((!AlmostEqual(member->getX2(),current.getX2())) &&
                 (member->getX2() < current.getX2() )){
                // current is an extension of member
                current.splitAt(member->getX2(), member->getY2(),left1,right1);
                // create events for the remaining parts
                q.push(right1.convertToExtendedHs(true,avlseg::first));
                q.push(right1.convertToExtendedHs(false,avlseg::first));
              }
           } else { // there is no overlapping segment
             splitByNeighbour(sss,current,leftN,q,q);
             splitByNeighbour(sss,current,rightN,q,q);
             sss.insert(current);
           } // no overlapping segment
       } else { // right event

           avlseg::AVLSegment left1, left2, right1, right2;

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
Instantiations of the template function

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

Auxiliary function checking if ~p~ is located above ~hs~.

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
              Cluster cluster = Cluster()){

  res.SetValue(0);
  res.SetEE(true);
  if(reg->IsEmpty()){
     res.SetEI(true);
     if(useCluster){
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
       return cluster.Contains(res);
     } else {
       return true;
     }
  }

  if(useCluster){
      int cb = cluster.checkBoxes(reg->BoundingBox(),false,
                                  p->BoundingBox(),
                                  false);
      if(cb==1) {
         return true;
      }
      if(cb==2){
         return false;
      }
   }
   // bbox check failed, we have to compute the result

   int size = reg->Size();
   double x = p->GetX();

   avlseg::ExtendedHalfSegment hs;

   int number = 0;

   for(int i=0;i<size;i++){
     HalfSegment hs1;
     reg->Get(i,hs1);
     hs = hs1;
     if(hs.IsLeftDomPoint()){
        if(hs.Contains(*p)){
           res.SetBI(true); //point on boundary
           if(useCluster){
             return cluster.Contains(res);
           } else {
              return true;
           }
        }
        if(!hs.IsVertical()){
            if(pointAbove(&hs,p)){
               if((AlmostEqual(hs.GetRightPoint().GetX(),x)) ||
                  ( !AlmostEqual(hs.GetLeftPoint().GetX(),x) &&
                    hs.GetLeftPoint().GetX()<x &&
                    hs.GetRightPoint().GetX()>=x)){
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

If the event caused by the region is smaller then the one caused by the
points value, the result of the function will be ~first~. Otherwise, the
result will be ~second~. The region itself may consist of the original
halfsegments and halfsegments produced by dividing original halfsegments.
The positions indicate the current elements of the halfsegment arrays.
They are updated automatically within this function.
Depending on the return value, one of the parameter ~resultHs~ or
~resultPoint~ is set to the value of the next event.

If the region and the points value are already processed, the return
value will be ~none~.

*/

avlseg::ownertype selectNext(const Region*  reg,
                     priority_queue<avlseg::ExtendedHalfSegment,
                                    vector<avlseg::ExtendedHalfSegment>,
                                    greater<avlseg::ExtendedHalfSegment> >& q1,
                     int& pos1,
                     Points const* const p,
                     int& pos2,
                     avlseg::ExtendedHalfSegment& resultHS,
                     Point& resultPoint){

  assert(pos1>=0);
  assert(pos2>=0);

  int sizereg = reg->Size();
  int sizepoint = p->Size();

  const avlseg::ExtendedHalfSegment* rhs=0;
  const avlseg::ExtendedHalfSegment* qhs = 0;
  avlseg::ExtendedHalfSegment qhsc;
  const avlseg::ExtendedHalfSegment* minhs=0;
  const Point* cp=0;

  avlseg::ExtendedHalfSegment rhs1;
  if(pos1<sizereg){
     HalfSegment hstmp;
     reg->Get(pos1, hstmp);
     rhs1 = hstmp;
     rhs = &rhs1;
  }
  if(!q1.empty()){
     qhsc = q1.top();
     qhs = &qhsc;
  }

  Point cp1;
  if(pos2<sizepoint){
     p->Get(pos2,cp1);
     cp = &cp1;
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
       return avlseg::none;
    } case 1: { // region
       pos1++;
       resultHS = *minhs;
       return avlseg::first;
    } case 2: { // queue
       q1.pop();
       resultHS = *minhs;
       return avlseg::first;
    } case 3: {  // point
       pos2++;
       resultPoint = *cp;
       return avlseg::second;
    } default: {
        assert(false);
        return avlseg::none;
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
  if(useCluster){
      int cb = cluster.checkBoxes(regbox, false, pbox, false);
      if(cb==1) {
         return true;
      }
      if(cb==2){
         return false;
      }
   }
  // bbox failed, perform a plane sweep

  if(useCluster){
    // restrict the cluster to possible values

    // boundary of an points value is empty
    Int9M m1(true,false,true, true,false,true, true,false,true);
    cluster.Restrict(m1,false,false);

    cluster.Restrict(res,true,false);

    if(cluster.IsEmpty()){
      return false;
    }
    if(cluster.isExtension(res)){
      return true;
    }
  }

  // queue for splitted segments of the region
  priority_queue<avlseg::ExtendedHalfSegment,
                 vector<avlseg::ExtendedHalfSegment>,
                 greater<avlseg::ExtendedHalfSegment> > q1;
  avltree::AVLTree<avlseg::AVLSegment> sss;

  avlseg::ownertype owner;
  bool done = false;
  int pos1 =0;
  int pos2 =0;
  Point CP;
  avlseg::ExtendedHalfSegment CH;
  avlseg::AVLSegment tmpL,tmpR;

  while (!done &&
         ( (owner= selectNext(reg,q1,pos1, ps,pos2,CH,CP))!=avlseg::none)){
    if(owner==avlseg::second){ // the point
       avlseg::AVLSegment current(CP,avlseg::second);
       const avlseg::AVLSegment* left=0;
       const avlseg::AVLSegment* right=0;
       const avlseg::AVLSegment* member = sss.getMember(current, left, right);
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
       done = done || (res.GetII() && res.GetBI() && res.GetEI());
    } else {  // the next element comes from the region
      avlseg::AVLSegment current(CH,avlseg::first);

      const avlseg::AVLSegment* leftN = 0;
      const avlseg::AVLSegment* rightN = 0;
      const avlseg::AVLSegment* member = sss.getMember(current,leftN,rightN);
      if(leftN){
         tmpL = *leftN;
         leftN = &tmpL;
      }
      if(rightN){
         tmpR = *rightN;
         rightN = &tmpR;
      }
      if(CH.IsLeftDomPoint()){ // left Event
      // debug::start
         if(member){
            cout << "found overlapping segments"
                    " within a single region"  << endl;
            cout << "Segment1 " << current << endl;
            cout << "Segment2 " << (*member) << endl;
            cout << "Region = " << *reg << endl;
         }
      // debug::end
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
The Class ~OwnedPoint~

This class contains a point together with its owner.


*/
class OwnedPoint{

 public:
    OwnedPoint(){
      defined = false;
    }

    OwnedPoint(Point p0, avlseg::ownertype o){
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
    avlseg::ownertype owner;
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
  if(!bbox1.IntersectsUD(bbox2)){
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
     int cb = cluster.checkBoxes(bbox1, false, bbox2, false);
     if(cb==1) {
        return true;
     }
     if(cb==2){
        return false;
     }
     cluster.Restrict(res,true,false);
     if(cluster.IsEmpty() ){
        return false;
     }
     if(cluster.isExtension(res)){
        return true;
     }
  }


  avltree::AVLTree<avlseg::AVLSegment> sss;            // sweep state structure
  // dynamic parts of the sweep event structure
  priority_queue<avlseg::ExtendedHalfSegment,
                 vector<avlseg::ExtendedHalfSegment>,
                 greater<avlseg::ExtendedHalfSegment> > q1;
  priority_queue<avlseg::ExtendedHalfSegment,
                 vector<avlseg::ExtendedHalfSegment>,
                 greater<avlseg::ExtendedHalfSegment> > q2;

  int pos1=0; // current position in the halfsegment array of reg1
  int pos2=0; // current position in the halfsegment array of reg2
  int size1 = reg1->Size();
  int size2 = reg2->Size();

  bool done = false;
  avlseg::ExtendedHalfSegment nextSeg;

  const avlseg::AVLSegment* member=0; // current member stored in the tree
  const avlseg::AVLSegment* leftN=0;  // the left neightboor of member
  const avlseg::AVLSegment* rightN=0; // the right neighbour of member
  avlseg::ownertype owner;
  OwnedPoint lastDomPoint; // initialized to be undefined
  int src;
  avlseg::AVLSegment tmpL,tmpR,tmpM;
  bool empty1 = false;
  bool empty2 = false;

  while(((owner=selectNext(*reg1, pos1,
                           *reg2, pos2,
                           q1,q2,
                           nextSeg,src))!=avlseg::none) &&
          !done){

    avlseg::AVLSegment current(nextSeg,owner);

    member = sss.getMember(current,leftN,rightN);
    if(leftN){
       tmpL = *leftN;
       leftN = &tmpL;
    }
    if(rightN){
       tmpR = *rightN;
       rightN = &tmpR;
    }
    if(member){
       tmpM = *member;
       member = &tmpM;
    }

    /*
    Because right events are processed before
    left events, we have to store the last dominating point
    to detect intersections of the boundaries within a single point.
    */

    Point p = nextSeg.GetDomPoint();

    empty1 = (pos1>=size1) && q1.empty() && (owner!=avlseg::first) &&
             (   ((p.GetX()>lastDomPoint.p.GetX() &&
                  !AlmostEqual(p.GetX(),lastDomPoint.p.GetX()))
                || lastDomPoint.owner==avlseg::second));

    empty2 = (pos2>=size2) && q2.empty() && (owner!=avlseg::second) &&
             ((   (p.GetX()>lastDomPoint.p.GetX() &&
                  !AlmostEqual(p.GetX(),lastDomPoint.p.GetX()))
               || lastDomPoint.owner==avlseg::first));

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
        avlseg::AVLSegment left, common, right;
        if(member){ // there is an overlapping segment in the tree
           // check for valid region representation
           assert(member->getOwner() != current.getOwner());
           assert(member->getOwner() != avlseg::both);

           SetBB(res,useCluster,cluster,done); // common boundary found
           bool iac = current.getOwner()==avlseg::first
                                    ?current.getInsideAbove_first()
                                   :current.getInsideAbove_second();
           bool iam = member->getOwner()==avlseg::first
                                    ?member->getInsideAbove_first()
                                    :member->getInsideAbove_second();

           if(iac!=iam){
             SetIE(res,useCluster,cluster,done);
             SetEI(res,useCluster,cluster,done);
           } else {
             SetII(res,useCluster,cluster,done);
             // res.setEE(true); // already done in initialization
           }

           int parts = member->split(current,left,common,right);


           sss.remove(*member);

           if(parts & avlseg::LEFT){   // there is a left part
              sss.insert(left);  // simulates a left event.
              insertEvents(left,false,true,q1,q2);
           }


           assert(parts & avlseg::COMMON);  // there must exist a common part

           // update con_above
           if(iac){
             common.con_above++;
           } else {
             common.con_above--;
           }
           sss.insert(common);

           // insert the corresponding right event into one of the queues
           insertEvents(common,false,true,q1,q2);

           if(parts & avlseg::RIGHT) { // there is an exclusive right part
              // create the events for the remainder
              insertEvents(right,true,true,q1,q2);
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
              bool iac = current.getOwner()==avlseg::first
                              ?current.getInsideAbove_first()
                              :current.getInsideAbove_second();

              iac = current.getOwner()==avlseg::first
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



              // derive intersections from the coverage numbers
              if(current.con_below == 0){
                  ; // EE is set already in initialization
                  //res.SetEE(true);
              } else if (current.con_below == 1) {
                  if(current.getInsideAbove()){
                     if(current.getOwner()==avlseg::first){
                        SetEI(res,useCluster,cluster,done);
                     } else {
                        SetIE(res,useCluster,cluster,done);
                     }
                  } else {
                     if(current.getOwner()==avlseg::first){
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

           } else { // inner intersection with at least one neighbour
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
            bool iac = current.getOwner()==avlseg::first?
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
                   if(current.getOwner()==avlseg::first){
                      SetEI(res,useCluster,cluster,done);
                   } else {
                      SetIE(res,useCluster,cluster,done);
                   }
                } else {
                   if(current.getOwner()==avlseg::first){
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
        if(member->getOwner()!=avlseg::both){
           // member is located in the interior or in the exterior of the
           // other region
           if(member->con_below==0 || member->con_above==0){ // in exterior
              switch(member->getOwner()){
                case avlseg::first: SetBE(res,useCluster,cluster,done);
                            SetIE(res,useCluster,cluster,done);
                            //res.SetEE(true);
                            break;
                case avlseg::second: SetEB(res,useCluster,cluster,done);
                             SetEI(res,useCluster,cluster,done);
                             //res.SetEE(true);
                             break;
                default: assert(false);
              }
           } else if(member->con_below==2 || member->con_above==2){ //interior
              switch(member->getOwner()){
                case avlseg::first: SetBI(res,useCluster,cluster,done);
                            SetII(res,useCluster,cluster,done);
                            SetEI(res,useCluster,cluster,done);
                            break;
                case avlseg::second: SetIB(res,useCluster,cluster,done);
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

    if(empty1 || empty2){
       done = true;
       if(!empty1){
         res.SetBE(true);
         res.SetIE(true);
         if(!res.GetBB()){
             owner=selectNext(*reg1,pos1, *reg2,pos2, q1,q2,nextSeg,src);
             if(owner==avlseg::second && lastDomPoint.owner!=avlseg::second){
                Point dP = nextSeg.GetDomPoint();
                if(AlmostEqual(dP,lastDomPoint.p)){
                   res.SetBB(true);
                }
             }
         }
       }
       if(!empty2){
         res.SetEB(true);
         res.SetEI(true);
         if(!res.GetBB()){
             owner=selectNext(*reg1,pos1, *reg2,pos2, q1,q2,nextSeg,src);
             if(owner==avlseg::first && lastDomPoint.owner!=avlseg::first){
                Point dP = nextSeg.GetDomPoint();
                if(AlmostEqual(dP,lastDomPoint.p)){
                   res.SetBB(true);
                }
             }
         }
       }
    }
  } // sweep

  if(useCluster){
    return cluster.Contains(res);
  } else {
    return true;
  }

} // GetInt9M (region x region)





/*
~updateDomPoints~

This functions does all the things caused by the next dominating point.
This function only works correct with two line values.

*/

void updateDomPoints_Line_Line(
              Point& lastDomPoint, const Point& newDomPoint,
              int& lastDomPointCount1, int& lastDomPointCount2,
              avlseg::ownertype owner,
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
        case avlseg::first: lastDomPointCount1 = 1;
                    lastDomPointCount2 = 0;
                    break;
        case avlseg::second: lastDomPointCount1 = 0;
                     lastDomPointCount2 = 1;
                     break;
        case avlseg::both:   lastDomPointCount1 = 1;
                     lastDomPointCount2 = 1;
                     break;
        default : assert(false);
     }
  } else { // dominating point not changed
      switch(owner){
         case avlseg::first : lastDomPointCount1++;
                      break;
         case avlseg::second: lastDomPointCount2++;
                      break;
         case avlseg::both  : lastDomPointCount1++;
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
 if(useCluster){
      int cb = cluster.checkBoxes(line1->BoundingBox(),line1->IsEmpty(),
                             line2->BoundingBox(),line2->IsEmpty());
      if(cb==1) {
          return true;
      }
      if(cb==2){
         return false;
      }
 }

 if(line1->IsEmpty()){
    // line2 is non empty
    SetEI(res,useCluster,cluster,done);
    if(useCluster){
      Int9M m(false,false,false, false,false,false, true,true,true);
      cluster.Restrict(m,false,false);
    }
 }
 if(line2->IsEmpty()){
    SetIE(res,useCluster,cluster,done);
    if(useCluster){
      Int9M m(false, false, true, false,false,true, false,false,true);
      cluster.Restrict(m,false,false);
    }
 }

 if(!line1->IsEmpty() && !line2->IsEmpty()){
    if(!line1->BoundingBox().Intersects(line2->BoundingBox()) && useCluster){
        Int9M m(false,false,true,false,false,true,true,true,true);
        cluster.Restrict(m,false,false);
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
 avltree::AVLTree<avlseg::AVLSegment> sss;
 // initialize priority queues for remaining parts of splitted
 // segments
 priority_queue<avlseg::ExtendedHalfSegment,
                vector<avlseg::ExtendedHalfSegment>,
                 greater<avlseg::ExtendedHalfSegment> > q1;
 priority_queue<avlseg::ExtendedHalfSegment,
                vector<avlseg::ExtendedHalfSegment>,
                greater<avlseg::ExtendedHalfSegment> > q2;

 int pos1=0;
 int pos2=0;
 int size1 = line1->Size();
 int size2 = line2->Size();

 avlseg::ExtendedHalfSegment nextHs;

 avlseg::ownertype owner;

 const avlseg::AVLSegment* leftN=0;
 const avlseg::AVLSegment* rightN=0;
 const avlseg::AVLSegment* member=0;

 Point lastDomPoint;
 int lastDomPointCount1 = 0;
 int lastDomPointCount2 = 0;
 avlseg::AVLSegment left1,right1,left2,right2,common;
 int src;
 avlseg::AVLSegment tmpL,tmpR;

 bool empty1 = false; // flag indicating end of line 1
 bool empty1eval = false; // empty1 already evaluated
 bool empty2 = false;
 bool empty2eval = false;


 while(!done &&
       ((owner=selectNext(*line1,pos1,*line2,
                          pos2,q1,q2,nextHs,src))!=avlseg::none) ){

   avlseg::AVLSegment current(nextHs,owner);

// debug::start
   // cout << "process segment " << current << "  "
   //      << (nextHs.IsLeftDomPoint()
 //                     ?"avlseg::avlseg::LEFT"
  //                   :"avlseg::RIGHT") << endl;
// debug::end
   empty1 = pos1>=size1 && q1.empty() &&
            lastDomPointCount1==0 &&
            owner!=avlseg::first;
   empty2 = pos2>=size2 && q2.empty() &&
            lastDomPointCount2==0 &&
            owner!=avlseg::second;


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
        if(member->getOwner()==avlseg::both || member->getOwner()==owner){
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
           if(parts&avlseg::LEFT){
              bool ok = sss.insert(left1);
              assert(ok);
              insertEvents(left1,false,true,q1,q2);
           }
           assert(parts & avlseg::COMMON);

           bool ok = sss.insert(common);
           assert(ok);
           insertEvents(common,false,true,q1,q2);
           // create events for right
           if(parts & avlseg::RIGHT){
              insertEvents(right1,true,true,q1,q2);
           }

           Point newDomPoint = nextHs.GetDomPoint();
           avlseg::ownertype owner2 = owner;
           if(parts  & avlseg::LEFT){
              owner2 = avlseg::both;
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

        avlseg::AVLSegment copy(*member);
        sss.remove(current);
        member = &copy;

        switch(member->getOwner()){
          case avlseg::first:  SetIE(res,useCluster,cluster,done);
                       break;
          case avlseg::second: SetEI(res,useCluster,cluster,done);
                       break;
          case avlseg::both  : SetII(res,useCluster,cluster,done);
                       break;
          default:     assert(false);
        }
        splitNeighbours(sss,leftN,rightN,q1,q2);
     } // otherwise current comes from a splitted segment and is ignored
   }
   done = done || res.IsFull();

   if(empty1) { // end of line1 reached
     if(res.GetEI() && res.GetEB()){
       done = true;
     } else if(useCluster && !empty1eval){
        Int9M tmp = res;
        tmp.SetEI(true);
        tmp.SetEB(true);
        cluster.Restrict(tmp,false,false);
        if(cluster.IsEmpty()){
           return false;
        }
     }
   }

   if(empty2) { // end of line1 reached
     if(res.GetIE() && res.GetBE()){
       done = true;
     } else if(useCluster && !empty2eval){
        Int9M tmp = res;
        tmp.SetIE(true);
        tmp.SetBE(true);
        cluster.Restrict(tmp,false,false);
        if(cluster.IsEmpty()){
           return false;
        }
     }
   }


 } // end of sweep

 // create a point which is different to the last domPoint
 Point newDP(lastDomPoint);
 newDP.Translate(100,0);
 updateDomPoints_Line_Line(lastDomPoint, newDP,
                 lastDomPointCount1, lastDomPointCount2,
                 avlseg::first,res,useCluster,cluster,done);

 if(useCluster){
    return cluster.Contains(res);
 } else {
    return true;
 }

} // line x line





/*
~updateDomPointInfo[_]Line[_]Region~

Does the things required by the switch from ~lastDomPoint~
to ~newDomPoint~.

*/

void updateDomPointInfo_Line_Region(Point& lastDomPoint,
                                    const Point& newDomPoint,
                                    int& count_line,
                                    int& count_region,
                                    int& lastCoverage_Num,
                                    const int newCoverageNum,
                                    const avlseg::ownertype owner,
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
         case avlseg::first : count_line = 1;
                      count_region = 0;
                      break;
         case avlseg::second: count_line = 0;
                      count_region = 1;
                      break;
         case avlseg::both  : count_line = 1;
                      count_region = 1;
                      break;
         default    : assert(false);
      }
    } else { // dompoint was not changed
       switch(owner){
         case avlseg::first : count_line++;
                      break;
         case avlseg::second: count_region++;
                      break;
         case avlseg::both  : count_line++;
                      count_region++;
                      break;
         default    : assert(false);
      }
    }
    lastCoverage_Num = newCoverageNum;
}


/*
~GetInt9M~  ~line~ [x] ~region~

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

  if(useCluster){
     int cb = cluster.checkBoxes(bbox1, line->IsEmpty(),
                                 bbox2, region->IsEmpty());
     if(cb==1) {
        return true;
     }
     if(cb==2){
        return false;
     }
     cluster.Restrict(res,true,false);
     if(cluster.IsEmpty() ){
        return false;
     }
     if(cluster.isExtension(res)){
        return true;
     }
  }

  if(!bbox1.IntersectsUD(bbox2)){
      res.SetIE(true); // line is not empty
      if(!region->IsEmpty()){
        res.SetEB(true);
      }
      if(useCluster){
          Int9M m(false,false,true, false, false, true, true,true,true);
          cluster.Restrict(m,false,false);
      }
  }

  if(useCluster){
     cluster.Restrict(res,true,false);
     if(region->IsEmpty()){
        Int9M m(false,false,true,false,false,true,false,false,true);
        cluster.Restrict(m,false,false);
     }
     if(cluster.IsEmpty()){
       return false;
     }
     if(cluster.isExtension(res)){
       return true;
     }
  }

 // initialise the sweep state structure
 avltree::AVLTree<avlseg::AVLSegment> sss;
 // initialize priority queues for remaining parts of splitted
 // segments
 priority_queue<avlseg::ExtendedHalfSegment,
                vector<avlseg::ExtendedHalfSegment>,
                greater<avlseg::ExtendedHalfSegment> > q1;
 priority_queue<avlseg::ExtendedHalfSegment,
                vector<avlseg::ExtendedHalfSegment>,
                greater<avlseg::ExtendedHalfSegment> > q2;

 int pos1=0;
 int pos2=0;

 bool done = false;

 avlseg::ExtendedHalfSegment nextHs;

 avlseg::ownertype owner;

 const avlseg::AVLSegment* leftN=0;
 const avlseg::AVLSegment* rightN=0;
 const avlseg::AVLSegment* member=0;

 Point lastDomPoint;
 int lastDomPointCount1 = 0;
 int lastDomPointCount2 = 0;
 avlseg::AVLSegment left1,right1,left2,right2,common;
 avlseg::AVLSegment tmpL,tmpR;

 int src;
 int lastCoverageNum = 0;

 // plane sweep
 while(!done &&
       ((owner=selectNext(*line,pos1,*region,
                          pos2,q1,q2,nextHs,src))!=avlseg::none)){
     avlseg::AVLSegment current(nextHs,owner);

     member = sss.getMember(current,leftN,rightN);
     if(leftN){
        tmpL = *leftN;
        leftN = &tmpL;
     }
     if(rightN){
        tmpR = *rightN;
        rightN = &tmpR;
     }
     avlseg::ownertype owner2 = owner;

     if(nextHs.IsLeftDomPoint()){ // left end point
       if(member){ // overlapping segment found
         if(member->getOwner()==avlseg::both || member->getOwner()==owner){
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
            if(parts & avlseg::LEFT){
              sss.insert(left1);
              insertEvents(left1,false,true,q1,q2);
            }
            assert(parts & avlseg::COMMON);
            // update coverage numbers
            if(owner==avlseg::first){ // the line
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
            if(parts & avlseg::RIGHT){
              insertEvents(right1,true,true,q1,q2);
            }


            if(parts & avlseg::LEFT){ // this dominating point comes from both
                              // halfsegments
               owner2 = avlseg::both;
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
          if(owner==avlseg::second){ // the region
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
           avlseg::AVLSegment tmp = *member;
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
            case avlseg::first : if(member->con_above==0){ // the line
                            SetIE(res,useCluster,cluster,done);
                         } else {
                            SetII(res,useCluster,cluster,done);
                         }
                         break;
            case avlseg::second: SetEB(res,useCluster,cluster,done);
                         break;
            case avlseg::both  : SetIB(res,useCluster,cluster,done);
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
                                lastCoverageNum, 0, avlseg::both, res,
                                useCluster,
                                cluster,done);

 if(useCluster){
     return cluster.Contains(res);
 } else {
     return true;
 }
}



/*
8 Integrating Operators in [secondo]

8.1 Type Mapping Functions

*/


/*
~TopPredTypeMap~

This function is the type mapping for the ~toppred~ operator.
The signature of this operator is:
    t1 [x] t2 [x] cluster [->] bool

where t1, t2 in [{]point, points, line, region[}].


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

   return nl->SymbolAtom(CcBool::BasicType());
}


/*
~TopRelTypeMap~

This function is the type mapping for the ~toprel~ operator.
The signature is t1 [x] t2 [->] int9m
where t1, t2 in [{]point, points, line, region[}]

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
~StdPredTypeMap~

t1 [x] t2 [->] bool,

where t1,t2 in [{]point, points, line, region[}].

*/
ListExpr StdPredTypeMap(ListExpr args){
   if(nl->ListLength(args)!=2){
      ErrorReporter::ReportError("two arguments expected");
      return nl->SymbolAtom(Symbol::TYPEERROR());
   }
   if(!IsSpatialType(nl->First(args))
      || !IsSpatialType(nl->Second(args))){
       ErrorReporter::ReportError("Spatial types expected");
       return (nl->TypeError());
   }
   return nl->SymbolAtom(CcBool::BasicType());
}



/*
8.2 Value Mappings

~TopRel~

This value mapping can be instantiated with all combinations
where a function ~GetInt9M~ exists for the type combination
~type1~ [x] ~type2~.

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
This function is symmetrical to the ~TopRel~ function. This function avoids
the implementation of symmetric ~GetInt9M~ functions.

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
Value Mapping for the ~toppred~ operator


*/
template<class t1, class t2>
int TopPred(Word* args, Word& result, int message,
                    Word& local, Supplier s){

  result = qp->ResultStorage(s);
  t1* v1 = static_cast<t1*>(args[0].addr);
  t2* v2 = static_cast<t2*>(args[1].addr);
  Cluster* cluster = static_cast<Cluster*>(args[2].addr);
  if(!cluster->IsDefined()){
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
~Standard Topological Predicates~


In the following section, we define predicates
implementing our imagination of topological predicates.
They can be used without an explicit definition of a cluster.


*/

/*
~Standard Cluster~

For each standard predicate, we hold a variable of
type cluster. The variables are initialized once
at the algebra initialization.

*/

static Cluster cl_disjoint;
static Cluster cl_adjacent;
static Cluster cl_overlap;
static Cluster cl_covers;
static Cluster cl_coveredBy;
static Cluster cl_inside;
static Cluster cl_contains;
static Cluster cl_equal;

static Cluster cl_wcontains; // contains || covers


/*
Value Mappings for standard predicates

*/
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


bool overlaps(Region* r1, Region* r2){
  Int9M matrix;
  return GetInt9M(r1, r2, matrix, true, cl_overlap);
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


/*
~initClusters~

This function initializes the standard clusters.

*/

static void initClusters(){
  PredicateGroup pg(0);
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

  cl_wcontains = cl_contains;
  cl_wcontains.Union(&cl_covers);

}


/*
9.3 Operator Specifications

Here, the operator descriptions are defined.

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
       TopPred<Point,Point> , TopPred<Points,Point>,
       TopPredSym<Point,Points>, TopPred<Points,Points>,
       TopPred<Line,Point>, TopPredSym<Point,Line>,
       TopPred<Line,Points>, TopPredSym<Points,Line>,
       TopPred<Region,Point>, TopPredSym<Point,Region>,
       TopPred<Region,Points>, TopPredSym<Points,Region>,
       TopPred<Region,Region>, TopPred<Line,Line>,
       TopPred<Line,Region>, TopPredSym<Region,Line>  };

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




/*
8.5 Selection Functions

*/

static int TopOpsSelect(ListExpr args){
   string type1 = nl->SymbolValue(nl->First(args));
   string type2 = nl->SymbolValue(nl->Second(args));

  if( (type1==Point::BasicType()) && (type2==Point::BasicType())){
      return 0;
   }
   if( (type1==Points::BasicType()) && (type2==Point::BasicType())){
      return 1;
   }
   if((type1==Point::BasicType()) && (type2==Points::BasicType())){
      return 2;
   }
   if((type1==Points::BasicType()) && (type2==Points::BasicType())){
      return 3;
   }
   if((type1==Line::BasicType()) && (type2==Point::BasicType())){
       return 4;
   }
   if((type1==Point::BasicType()) && (type2==Line::BasicType())){
       return 5;
   }
   if((type1==Line::BasicType()) && (type2==Points::BasicType())){
       return 6;
   }
   if((type1==Points::BasicType()) && (type2==Line::BasicType())){
       return 7;
   }
   if((type1==Region::BasicType()) && (type2==Point::BasicType())){
       return 8;
   }
   if((type1==Point::BasicType()) && (type2==Region::BasicType())){
       return 9;
   }
   if( type1==Region::BasicType() && (type2==Points::BasicType())){
       return  10;
   }
   if(type1==Points::BasicType() && (type2==Region::BasicType())){
       return 11;
   }
   if(type1==Region::BasicType() && (type2==Region::BasicType())){
       return 12;
   }
   if( (type1==Line::BasicType()) && (type2==Line::BasicType())){
       return 13;
   }
   if( (type1==Line::BasicType()) && (type2==Region::BasicType())){
       return 14;
   }
   if( (type1==Region::BasicType()) && (type2==Line::BasicType())){
       return 15;
   }

   return -1;
}





/*
8.6 Creating Instances of Operators

In this section instances of the algebra operators are built.

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


/*
8.7 Creating the Algebra

*/
class TopOpsAlgebra : public Algebra {
  public:
     TopOpsAlgebra() : Algebra() {
        AddOperator(&optoprel);
        AddOperator(&toppred);
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
};

/*
Functions exported in the header file.

*/
bool wcontains(const Region* reg1, const Region* reg2){
   Int9M dummy;
   return topops::GetInt9M(reg1, reg2, dummy, true, topops::cl_wcontains);
}

} // end of namespace topops




/*
8.8 Initialization of the Algebra

*/
extern "C"
Algebra* InitializeTopOpsAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
    nl = nlRef;
    qp = qpRef;
    topops::initClusters();
    return (new topops::TopOpsAlgebra());
}

