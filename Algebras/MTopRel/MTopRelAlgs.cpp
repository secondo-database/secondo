
/*
----
This file is part of SECONDO.

Copyright (C) 2011, 
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

*/

#include <vector>
#include <queue>
#include <algorithm>
#include <assert.h>

#include "MTopRelAlgs.h"
#include "TopRel.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Temporal/RefinementStream.h"


using namespace std;
using namespace datetime;

namespace temporalalgebra{

/*
0 Auxiliary functions


0.1 mergeAdd

Appends a pair to the end of the vector. If the interval is connected to the
valst interval and the value is the same, the last entry within the vector will
be extended. Otherwise, the entry is pushed at the end of the vector.

*/

void mergeAdd(vector<pair<Interval<Instant>, toprel::Int9M> >& vec, 
              const pair<Interval<Instant>, toprel::Int9M>& elem){
  if(vec.size()<1){ // first element
    vec.push_back(elem);
    return;
  } 
  int index = vec.size()-1;

  if(vec[index].first.end != elem.first.start){
     vec.push_back(elem);
     return;
  }
  if(vec[index].first.rc != !elem.first.lc){
     vec.push_back(elem);
     return;
  }
  // intervals are ok, check values
  if(vec[index].second != elem.second){
     vec.push_back(elem);
     return;
  }
  // extend the interval
  vec[index].first.end = elem.first.end;
  vec[index].first.rc = elem.first.rc;
}

void mergeAdd(queue<pair<Interval<Instant>, toprel::Int9M> >& q, 
              const pair<Interval<Instant>, toprel::Int9M>& elem){
  if(q.size()<1){ // first element
    q.push(elem);
    return;
  } 
  if(q.back().first.end != elem.first.start){
     q.push(elem);
     return;
  }
  if(q.back().first.rc != !elem.first.lc){
     q.push(elem);
     return;
  }
  // intervals are ok, check values
  if(q.back().second != elem.second){
     q.push(elem);
     return;
  }
  // extend the interval
  q.back().first.end = elem.first.end;
  q.back().first.rc = elem.first.rc;
}

void mergeAdd(vector<pair<Interval<Instant>, toprel::Cluster> >& vec, 
               const pair<Interval<Instant>, toprel::Cluster> & elem){
  if(vec.size()<1){ // first element
    vec.push_back(elem);
    return;
  } 
  int index = vec.size()-1;

  if(vec[index].first.end != elem.first.start){
     vec.push_back(elem);
     return;
  }
  if(vec[index].first.rc != !elem.first.lc){
     vec.push_back(elem);
     return;
  }
  // intervals are ok, check values
  if(vec[index].second != elem.second){
     vec.push_back(elem);
     return;
  }
  // extend the interval
  vec[index].first.end = elem.first.end;
  vec[index].first.rc = elem.first.rc;
}


void mergeAdd(queue<pair<Interval<Instant>, toprel::Cluster> >& q, 
               const pair<Interval<Instant>, toprel::Cluster> & elem){
  if(q.size()<1){ // first element
    q.push(elem);
    return;
  } 

  if(q.back().first.end != elem.first.start){
     q.push(elem);
     return;
  }
  if(q.back().first.rc != !elem.first.lc){
     q.push(elem);
     return;
  }
  // intervals are ok, check values
  if(q.back().second != elem.second){
     q.push(elem);
     return;
  }
  // extend the interval
  q.back().first.end = elem.first.end;
  q.back().first.rc = elem.first.rc;
}






/*
Computing topological relationships between two upoints.

*/
    MTopRelAlg_UPUP::MTopRelAlg_UPUP(const UPoint* up1, const UPoint* up2):
     toprels(), clusters(), toprelPos(0), clusterPos(0){
      init(up1, up2, 0);
    }

      MTopRelAlg_UPUP::MTopRelAlg_UPUP(const UPoint* up1, const UPoint* up2, 
                     const toprel::PredicateGroup* pg):
       toprels(), clusters(), toprelPos(0), clusterPos(0){

       init(up1, up2, pg);
     }

/*
1.2 hasNext

Checks whether more topological relationships are available.


*/
      bool MTopRelAlg_UPUP::hasNext() const{
         return toprelPos < toprels.size();
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_UPUP::next(){
          assert(hasNext());
          pair<Interval<Instant>, toprel::Int9M> res = toprels[toprelPos];
          toprelPos++;
          return res; 
      }


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool MTopRelAlg_UPUP::hasNextCluster() const{
        return clusterPos < clusters.size();
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_UPUP::nextCluster(){
          assert(hasNextCluster());
          pair<Interval<Instant>, toprel::Cluster> res = clusters[clusterPos];
          clusterPos++;
          return res; 
         
      }

      void MTopRelAlg_UPUP::init(const UPoint* up1, const UPoint* up2, 
                                 const toprel::PredicateGroup* pg){
         if(!up1->IsDefined() || !up2->IsDefined() ){
            // if one of the arguments is not defined, the result will be empty
            return;
         } 
         toprel::Int9M undefint9m(false);
         undefint9m.SetDefined(false);

         toprel::Int9M equalint9m(true);
         equalint9m.Set(1,0,0, 0,0,0, 0,0,1);

         toprel::Int9M disjointint9m(true);
         disjointint9m.Set( 0,0,1, 0,0,0, 1,0,1);

         if(!up1->timeInterval.Intersects(up2->timeInterval)){
            pair<Interval<Instant>, toprel::Int9M> 
                                     p1(up1->timeInterval, undefint9m);
            pair<Interval<Instant>, toprel::Int9M> 
                                     p2(up2->timeInterval, undefint9m);
            if(up1->timeInterval.Before(up2->timeInterval)){
               mergeAdd(toprels, p1);
               mergeAdd(toprels, p2);
            } else {
               mergeAdd(toprels, p2);
               mergeAdd(toprels, p1);
            }
            computeClustersFromTopRels(pg);
            return;       
         }
         // first, we restrict the upoints to there common interval
         Interval<Instant> commonInterval;
         up1->timeInterval.Intersection(up2->timeInterval, commonInterval);
         // process parts before the common interval          
         if(up1->timeInterval.StartsBefore(commonInterval)){
            Interval<Instant> i1(up1->timeInterval.start, commonInterval.start, 
                                 up1->timeInterval.lc, !commonInterval.lc);
             
            pair<Interval<Instant>, toprel::Int9M> p1(i1, undefint9m);
            mergeAdd(toprels, p1);
         }
         if(up2->timeInterval.StartsBefore(commonInterval)){
            Interval<Instant> i1(up2->timeInterval.start, commonInterval.start, 
                                 up2->timeInterval.lc, !commonInterval.lc);
             
            pair<Interval<Instant>, toprel::Int9M> p1(i1, undefint9m);
            mergeAdd(toprels, p1);
         }
         // process the common part
          UPoint commonWay(1);
          up1->Intersection(*up2, commonWay);
          if(!commonWay.IsDefined()){ // completely disjoint
             pair<Interval<Instant>, toprel::Int9M> 
                                p1(commonInterval, disjointint9m);
             mergeAdd(toprels, p1);
          } else { // [disjoint] equal [disjoint]
             if(commonInterval.StartsBefore(commonWay.timeInterval)){
                Interval<Instant> i1(commonInterval.start, 
                                     commonWay.timeInterval.start,
                                     commonInterval.lc,
                                     !commonWay.timeInterval.lc);
               pair<Interval<Instant>, toprel::Int9M> p1(i1, disjointint9m);
               mergeAdd(toprels, p1);
             }
             // process the common interval
             pair<Interval<Instant>, toprel::Int9M> p2(commonWay.timeInterval,
                                                        equalint9m);
             mergeAdd(toprels,p2);
             // process part after the common way
             if(commonInterval.EndsAfter(commonWay.timeInterval)){
                Interval<Instant> i1(commonWay.timeInterval.end, 
                                     commonInterval.end,
                                     !commonWay.timeInterval.rc,
                                     commonInterval.rc);
               pair<Interval<Instant>, toprel::Int9M> p1(i1, disjointint9m);
               mergeAdd(toprels,p1);
             }
          }
            

         // process part after the common interval
         if(up1->timeInterval.EndsAfter(commonInterval)){
            Interval<Instant> i1(commonInterval.end, up1->timeInterval.end,  
                                 !commonInterval.rc, up1->timeInterval.rc);
            pair<Interval<Instant>, toprel::Int9M> p1(i1, undefint9m);
            mergeAdd(toprels, p1);
         }
         if(up2->timeInterval.EndsAfter(commonInterval)){
            Interval<Instant> i1(commonInterval.end, up2->timeInterval.end,  
                                 !commonInterval.rc, up2->timeInterval.rc);
            pair<Interval<Instant>, toprel::Int9M> p1(i1, undefint9m);
            mergeAdd(toprels,p1);
         }
         computeClustersFromTopRels(pg);

      }



      void MTopRelAlg_UPUP::computeClustersFromTopRels(
                   const toprel::PredicateGroup* pg){
         if(pg==0){
            return;
         }
         if(!pg->IsDefined()){
          return;
         }
         if(toprels.size()==0){
           return;
         }

         for(unsigned int i=0; i< toprels.size(); i++){
           pair<Interval<Instant>, toprel::Int9M> npi = toprels[i];
           toprel::Cluster* cl = pg->GetClusterOf(npi.second);
           mergeAdd(clusters, pair<Interval<Instant>, 
                                    toprel::Cluster>(npi.first, *cl));
           delete cl;
        }
      }


/*
4 Combination MPoint x MPoint

*/
      MTopRelAlg_MPMP::MTopRelAlg_MPMP(const MPoint* _mp1, const MPoint* _mp2):
           mp1(_mp1), mp2(_mp2), toprelRefinement(_mp1,_mp2),
           clusterRefinement(_mp1,_mp2),
            pg(0),  toprelqueue(), clusterqueue(){
          init();
      }

      MTopRelAlg_MPMP::MTopRelAlg_MPMP(const MPoint* _mp1, const MPoint* _mp2, 
                     const toprel::PredicateGroup* _pg): 
           mp1(_mp1), mp2(_mp2), toprelRefinement(_mp1,_mp2), 
           clusterRefinement(_mp1,_mp2),
           pg(_pg), toprelqueue(), clusterqueue(){
          init();
      }

/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool MTopRelAlg_MPMP::hasNext() const{
         return toprelqueue.size()>0;
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_MPMP::next(){
        assert(hasNext());
        pair<Interval<Instant>, toprel::Int9M> res = toprelqueue.front();
        toprelqueue.pop();
        computeNextTopRel();
        return res;
      }


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool MTopRelAlg_MPMP::hasNextCluster() const{
        return clusterqueue.size() > 0;
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_MPMP::nextCluster(){
        assert(hasNextCluster());
        pair<Interval<Instant>, toprel::Cluster> res = clusterqueue.front();
        clusterqueue.pop();
        computeNextCluster();
        return res;

      }

       void MTopRelAlg_MPMP::init(){
         if( !mp1 || !mp2){
            return;
         }
         if(!mp1->IsDefined() || ! mp2->IsDefined()){
           return;
         }
        
         computeNextTopRel();
         if(pg && pg->IsDefined()){
           computeNextCluster();
         }
       }

       void MTopRelAlg_MPMP::computeNextTopRel(){
           Interval<Instant> iv; 
           int pos1;
           int pos2;
           toprel::Int9M undef(false);
           undef.SetDefined(false);
           while(toprelqueue.size() < 2 && toprelRefinement.hasNext()){
               toprelRefinement.getNext(iv, pos1,pos2);
               if(pos1<0 || pos2 <0){
                  pair<Interval<Instant> , toprel::Int9M> p(iv, undef);
                  mergeAdd(toprelqueue,p);
               } else { // wow, found a common interval
                  UPoint up1_1(1);
                  mp1->Get(pos1,up1_1);
                  UPoint up1(0);
                  up1_1.AtInterval(iv,up1);

                  UPoint up2_1(0);
                  mp2->Get(pos2,up2_1);
                  UPoint up2(0); 
                  up2_1.AtInterval(iv,up2);
                  MTopRelAlg_UPUP alg(&up1,&up2);
                  while(alg.hasNext()){
                     mergeAdd(toprelqueue,alg.next());
                  }
 
               }
           }

       }



       void MTopRelAlg_MPMP::computeNextCluster(){
           Interval<Instant> iv; 
           int pos1;
           int pos2;
           toprel::Cluster undef(false);
           undef.SetDefined(false);
           while(clusterqueue.size() < 2 && clusterRefinement.hasNext()){
               clusterRefinement.getNext(iv, pos1,pos2);
               if(pos1<0 || pos2 <0){
                  pair<Interval<Instant> , toprel::Cluster> p(iv, undef);
                  mergeAdd(clusterqueue,p);
               } else { // wow, found a common interval
                  UPoint up1_1(1);
                  mp1->Get(pos1,up1_1);
                  UPoint up1(0);
                  up1_1.AtInterval(iv,up1);
                  
                  UPoint up2_1(0);
                  mp2->Get(pos2,up2_1);
                  UPoint up2;
                  up2_1.AtInterval(iv,up2);

                  MTopRelAlg_UPUP alg(&up1,&up2, pg);
                  while(alg.hasNextCluster()){
                     mergeAdd(clusterqueue,alg.nextCluster());
                  }
               }
           }
        }


/*

Auxiliary function

returns the segment with the smallest x value from reg (unit at posreg)
qreg, and qup. The segment is removed from the input (for the region,
just the position counter is increased).

*/
avlseg::ownertype SelectNext(
     const Region& reg,
     int& posreg,
     priority_queue<avlseg::ExtendedHalfSegment,
                    vector<avlseg::ExtendedHalfSegment>,
                    greater<avlseg::ExtendedHalfSegment> >& qreg, 
     
     priority_queue<avlseg::ExtendedHalfSegment,
                    vector<avlseg::ExtendedHalfSegment>,
                    greater<avlseg::ExtendedHalfSegment> >& qup, 
     avlseg::ExtendedHalfSegment& result ){

     bool h1 = posreg < reg.Size();
     HalfSegment hs1;
     if(h1){
       reg.Get(posreg, hs1);
     }

     bool h2 = !qreg.empty();
     avlseg::ExtendedHalfSegment hs2;
     if(h2){
        hs2 = qreg.top();
     }
     
    
     bool h3 = !qup.empty();
     avlseg::ExtendedHalfSegment hs3;
     if(h3){
        hs3 = qup.top();
     }

    if(!h1 && !h2 && !h3){  // all parts exhausted
      return avlseg::none;
    }
    
    int smallest = 0;
   
    if(h1){
      smallest = 1;
    }

    if(h2){
      switch(smallest){
        case 0: { smallest = 2;  break;}
        case 1: { if(hs2 < hs1){
                    smallest = 2;
                  }
                  break;
                }
        default: assert(false);

      }
    }

    if(h3){
      switch(smallest){
         case 0: { smallest = 3; break; }
         case 1: { if(hs3 < hs1){
                    smallest = 3;
                  }
                  break;
                }

         case 2: { if(hs3 < hs2){
                    smallest = 3;
                  }
                  break;
                }
         default : assert(false);
      }
    }

    switch(smallest){
       case 1 : result = hs1;
                posreg ++;
                return avlseg::first;
       case 2 : result = hs2;
                qreg.pop();
                return avlseg::first;
       case 3 : result = hs3;
                qup.pop();
                return avlseg::second;
       default : assert(false);
    };
    return avlseg::none;



}



/*
~computeInstant~

computes the instant when up is at the point defied by (x,y).
The is noe check whether this point is actually is reached.
If not, the returned instant may be outside the time interval
of up.

*/

     Instant computeInstant(const UPoint& up, const double x, const double y){
        if(up.IsStatic()){
           return up.timeInterval.start;
        }  
        double dx = up.p1.GetX() - up.p0.GetX();
        double dy = up.p1.GetY() - up.p0.GetY();
       
        double delta;

        if(abs(dx) > abs(dy)){
           delta = (x - up.p0.GetX()) / dx;
        } else {
           delta = (y - up.p0.GetY()) / dy;
        }
        if(AlmostEqual(0.0,delta)){
          delta = 0;
        }
        if(AlmostEqual(1.0,delta)){
          delta = 1;
        }

        DateTime dur(datetime::instanttype);
        dur = up.timeInterval.end - up.timeInterval.start;
        dur.Mul(delta);
        
        return up.timeInterval.start + dur; 
     }

/*
~pcompare~

Checks whther the first component of the firt pair is smaller than
the first component of the second pair;

*/
     bool pCompare (const pair<Interval<Instant>, toprel::Int9M>& p1,
                    const pair<Interval<Instant>, toprel::Int9M>& p2) { 
          return p1.first < p2.first; 
     }


/*
~computeInterval~

Computes the interval when the upoint is at the segment seg.


*/
     Interval<Instant> computeInterval(const UPoint& up, 
                                       const avlseg::AVLSegment& seg){

        Instant start = computeInstant(up,seg.getX1(),seg.getY1());
        if(seg.isPoint()){             
            Interval<Instant> res(start, start, true, true);
            return  res;     
         }
         Instant end = computeInstant(up, seg.getX2(), seg.getY2());
         if(start>end){
             Interval<Instant> res(end, start, true, true);
             return  res;
         } else {
             Interval<Instant> res(start, end, true, true);
             return  res;
         }

     }




} // end of namespace 

