
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
#include "TemporalAlgebra.h"
#include "SpatialAlgebra.h"
#include "RefinementStream.h"



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
1 Computing topological relationship between a static point and unit(point)


The complete result ist computed within the constructor. Methods allow to 
iterate over the result.

*/

/*
1.1 Constructors


*/
      MTopRelAlg_PUP::MTopRelAlg_PUP(const Point* p, const UPoint* up): 
        currentIndexInt9M(0), currentIndexCluster(0){
         init(p, up, 0);
      }

      MTopRelAlg_PUP::MTopRelAlg_PUP(const Point* p, const UPoint* up, 
                                     const toprel::PredicateGroup* pg){
         init(p,up,pg);
      }

/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool MTopRelAlg_PUP::hasNext() const{
         return currentIndexInt9M < toprels.size();
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_PUP::next(){
          assert(hasNext());
          currentIndexInt9M++;
          return toprels[currentIndexInt9M-1];
      }


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
     bool MTopRelAlg_PUP::hasNextCluster() const{
        return currentIndexCluster < clusters.size();
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_PUP::nextCluster(){
         assert(hasNextCluster());
         currentIndexCluster++;
         return clusters[currentIndexCluster-1];
      }

/*
1.6 init

This functions computes the result and stores it into the private members.

*/
    void MTopRelAlg_PUP::init(const Point* p, const UPoint* up, 
                              const toprel::PredicateGroup* pg){
       toprels.clear();
       clusters.clear();
       currentIndexInt9M = 0;
       currentIndexCluster = 0;

       if(!p->IsDefined() || !up->IsDefined()){
          return;  // no result
       }
       // step 1: compute the 9 intersection matrices
       toprel::Int9M m(0);
       if(AlmostEqual(up->p0, up->p1)){
          if(AlmostEqual(*p, up->p0)){
               m.Set(1,0,0, 0,0,0, 0,0,1); // set to "equal"
          } else {
               m.Set(0,0,1, 0,0,0, 1,0,1); // set to "disjoint"
          }
          pair<Interval<Instant>, toprel::Int9M> r(up->timeInterval, m);
          toprels.push_back(r);
       } else { // nonstatic upoint
          HalfSegment hs(true,up->p0, up->p1);
          if(!hs.Contains(p)){ // never on line
             m.Set(0,0,1, 0,0,0, 1,0,1); // set to "disjoint"
             pair<Interval<Instant>, toprel::Int9M> r(up->timeInterval, m);
             toprels.push_back(r);
          } else {
             // compute the instant when the points are equal
             double deltaX = up->p1.GetX() - up->p0.GetX();
             double deltaY = up->p1.GetY() - up->p0.GetY();
             double delta = deltaX>deltaY? (p->GetX() - up->p0.GetX()) / deltaX
                                         : (p->GetY() - up->p0.GetY()) / deltaY;

             DateTime dur(durationtype);
             dur = up->timeInterval.end - up->timeInterval.start;
             dur.Mul(delta);
             datetime::DateTime in(datetime::instanttype);
             in = up->timeInterval.start + dur;
             vector<Interval<Instant> > v = up->timeInterval.splitAround(in);
             for(unsigned int i=0; i < v.size(); i++){
                Interval<Instant> ci = v[i];
                if(ci.Contains(in)){
                   m.Set(1,0,0, 0,0,0, 0,0,1); // set to "equal"
                } else {
                   m.Set(0,0,1, 0,0,0, 1,0,1);
                }
                pair<Interval<Instant>, toprel::Int9M> r(ci, m);
                toprels.push_back(r);
             }
          }
       }


       // step 2: build the cluster vector from the int9m's
       if(pg==0){
         return;
       }
       if(!pg->IsDefined()){
         return;
       }

       if(toprels.size()==0){
         return;
       }

      pair<Interval<Instant>, toprel::Int9M> cpi = toprels[0];
      toprel::Cluster* cl = pg->GetClusterOf(cpi.second);
      pair<Interval<Instant>, toprel::Cluster> cpc(cpi.first, *cl);
      delete cl;
      for(unsigned int i=1; i< toprels.size(); i++){
         pair<Interval<Instant>, toprel::Int9M> npi = toprels[i];
         cl = pg->GetClusterOf(npi.second);
         if(cpc.second == *cl){ // toprel within the same cluster, 
                                //just enlarge the interval
              cpc.first.end = npi.first.end;
              cpc.first.rc = npi.first.rc;
         } else {
            clusters.push_back(cpc);
            cpc.first = npi.first;
            cpc.second = *cl;
         }
         delete cl;
      }
      clusters.push_back(cpc);
    }





/*
2 Computing topological relationship between a static point and moving(point)

Note. This class stores a reference to a moving point given in the constructor

*/

/*
1.1 Constructors


*/
      MTopRelAlg_PMP::MTopRelAlg_PMP(const Point* _p, const MPoint* _mp): 
        currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
        posCluster(0), p(_p), mp(_mp), pg(0){
        init();
      }
          

      MTopRelAlg_PMP::MTopRelAlg_PMP(const Point* _p, 
                                     const MPoint* _mp, 
                                     const toprel::PredicateGroup* _pg):
         currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
         posCluster(0), p(_p), mp(_mp), pg(_pg){
        init();
      }

      void MTopRelAlg_PMP::init(){
        if(p->IsDefined() && mp->IsDefined()){
          computeNextTopRel();
          if(pg && pg->IsDefined()){
             computeNextCluster();
          }
        }
      }


      MTopRelAlg_PMP::~MTopRelAlg_PMP(){ }


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool MTopRelAlg_PMP::hasNext() const{
        return currentUnitTopRel.size() > 0;
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_PMP::next(){
         assert(hasNext());
         pair<Interval<Instant>, toprel::Int9M> result = 
                                            currentUnitTopRel.front();
         currentUnitTopRel.pop();
         computeNextTopRel();
         return result;
      }

      bool MTopRelAlg_PMP::canBeConnected(const Interval<Instant>& i1, 
                                          const Interval<Instant>& i2) const{
         // TODO:  improve this implementation
         return i1.end == i2.start;
      }

      
      bool MTopRelAlg_PMP::canBeConnected(
                const pair<Interval<Instant>, toprel::Int9M>& f, 
                const pair<Interval<Instant>, toprel::Int9M>& s) const{
          return canBeConnected(f.first, s.first) && (f.second == s.second);
      }

      void MTopRelAlg_PMP::connect(
              pair<Interval<Instant> , toprel::Int9M>& f, 
              const  pair<Interval<Instant> , toprel::Int9M>& s){

           assert(canBeConnected(f,s));
           f.first.end = s.first.end;
           f.first.rc = s.first.rc;
     }

      
      void MTopRelAlg_PMP::computeNextTopRel(){
         if(currentUnitTopRel.size() > 1){
            return;
         }
         // make a first entry
         if(currentUnitTopRel.empty()){
            if(posTopRel >= mp->GetNoComponents()){
              // no more units available
              return;
            }
            UPoint up(1);
            mp->Get(posTopRel, up);
            posTopRel++;
            MTopRelAlg_PUP pup(p,&up,pg);
            while(pup.hasNext()){
              currentUnitTopRel.push(pup.next());
            }
         }
         // extend the last entry while it is possible 
         // (elements available and makes sense (only 1 element)
         assert(currentUnitTopRel.size()>0);
         while( (currentUnitTopRel.size()<2) && 
                (posTopRel < mp->GetNoComponents())){
            UPoint up(1);
            mp->Get(posTopRel, up); 
            posTopRel++;
            MTopRelAlg_PUP pup(p,&up,pg);
            while(pup.hasNext()){
              pair<Interval<Instant>, toprel::Int9M> n = pup.next();
              if(canBeConnected(currentUnitTopRel.back(), n)){
                 connect(currentUnitTopRel.back(), n);
              } else {
                 currentUnitTopRel.push(n);
              }
            }
         }
      }




/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool MTopRelAlg_PMP::hasNextCluster() const{
         return currentUnitCluster.size() > 0;
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_PMP::nextCluster(){
         assert(hasNextCluster());
         pair<Interval<Instant>, toprel::Cluster> result = 
                                                   currentUnitCluster.front();
         currentUnitCluster.pop();
         computeNextCluster();
         return result;
      }


      bool MTopRelAlg_PMP::canBeConnected(
             const pair<Interval<Instant>, toprel::Cluster>& f, 
             const pair<Interval<Instant>, toprel::Cluster>& s) const{
          return canBeConnected(f.first, s.first) && (f.second == s.second);
      }

      void MTopRelAlg_PMP::connect(
             pair<Interval<Instant> , toprel::Cluster>& f, 
             const  pair<Interval<Instant> , toprel::Cluster>& s){

           assert(canBeConnected(f,s));
           f.first.end = s.first.end;
           f.first.rc = s.first.rc;
     }



      void MTopRelAlg_PMP::computeNextCluster(){
         if(currentUnitCluster.size() > 1){
            return;
         }
         // make a first entry
         if(currentUnitCluster.empty()){
            if(posCluster >= mp->GetNoComponents()){
              // no more units available
              return;
            }
            UPoint up(1);
            mp->Get(posCluster, up);
            posCluster++;
            MTopRelAlg_PUP pup(p,&up,pg);
            while(pup.hasNextCluster()){
              currentUnitCluster.push(pup.nextCluster());
            }
         }
         assert(currentUnitCluster.size()>0);
         while( (currentUnitCluster.size()<2) && 
                (posCluster < mp->GetNoComponents())){
            UPoint up(1);
            mp->Get(posCluster, up); 
            posCluster++;
            MTopRelAlg_PUP pup(p,&up,pg);
            while(pup.hasNextCluster()){
              pair<Interval<Instant>, toprel::Cluster> n = pup.nextCluster();
              if(canBeConnected(currentUnitCluster.back(), n)){
                 connect(currentUnitCluster.back(), n);
              } else {
                 currentUnitCluster.push(n);
              }
            }
         }
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
4 Combination UPoint x MPoint

*/
      MTopRelAlg_UPMP::MTopRelAlg_UPMP(const UPoint* _up, const MPoint* _mp):
           up(_up), mp(_mp), toprelRefinement(_up,_mp),
           clusterRefinement(_up,_mp),
            pg(0),  toprelqueue(), clusterqueue(){
          init();
      }

      MTopRelAlg_UPMP::MTopRelAlg_UPMP(const UPoint* _up, const MPoint* _mp, 
                     const toprel::PredicateGroup* _pg): 
           up(_up), mp(_mp), toprelRefinement(_up,_mp), 
           clusterRefinement(_up,_mp),
           pg(_pg), toprelqueue(), clusterqueue(){
          init();
      }

/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool MTopRelAlg_UPMP::hasNext() const{
         return toprelqueue.size()>0;
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_UPMP::next(){
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
      bool MTopRelAlg_UPMP::hasNextCluster() const{
        return clusterqueue.size() > 0;
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_UPMP::nextCluster(){
        assert(hasNextCluster());
        pair<Interval<Instant>, toprel::Cluster> res = clusterqueue.front();
        clusterqueue.pop();
        computeNextCluster();
        return res;

      }

       void MTopRelAlg_UPMP::init(){
         if( !up || !mp){
            return;
         }
         if(!up->IsDefined() || ! mp->IsDefined()){
           return;
         }
        
         computeNextTopRel();
         if(pg && pg->IsDefined()){
           computeNextCluster();
         }
       }

       void MTopRelAlg_UPMP::computeNextTopRel(){
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
                  UPoint up1(1);
                  up->AtInterval(iv,up1);
                  UPoint up2_1(0);
                  mp->Get(pos2,up2_1);
                  UPoint up2(0); 
                  up2_1.AtInterval(iv,up2);
                  MTopRelAlg_UPUP alg(&up1,&up2);
                  while(alg.hasNext()){
                     mergeAdd(toprelqueue,alg.next());
                  }
               }
           }
       }



       void MTopRelAlg_UPMP::computeNextCluster(){
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
                  UPoint up1(1);
                  up->AtInterval(iv,up1);
                  UPoint up2_1(0);
                  mp->Get(pos2,up2_1);
                  UPoint up2;
                  up2_1.AtInterval(iv,up2);
                  MTopRelAlg_UPUP alg(&up1,&up2, pg);
                  while(alg.hasNext()){
                     mergeAdd(clusterqueue,alg.nextCluster());
                  }
               }
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

Region x UPoint

*/

   MTopRelAlg_RUP::MTopRelAlg_RUP(const Region* reg, const UPoint* up):
     toprelvector(), clustervector(), toprelpos(0), clusterpos(0){
     init(reg, up, 0);
   }

    MTopRelAlg_RUP::MTopRelAlg_RUP(const Region* reg, const UPoint* up, 
                     const toprel::PredicateGroup* pg):
     toprelvector(), clustervector(), toprelpos(0), clusterpos(0){
     init(reg,up,pg);
    }


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool MTopRelAlg_RUP::hasNext() const {
        return toprelpos < toprelvector.size();
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_RUP::next(){
         assert(hasNext());
         pair<Interval<Instant>, toprel::Int9M> res = toprelvector[toprelpos];
         toprelpos++;
         return res;
      }


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool MTopRelAlg_RUP::hasNextCluster() const{
         return clusterpos < clustervector.size();
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

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




      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_RUP::nextCluster(){
         assert(hasNextCluster());
         pair<Interval<Instant>, toprel::Cluster> 
                 res = clustervector[clusterpos];
         clusterpos++;
         return res;
      }



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



     bool pCompare (const pair<Interval<Instant>, toprel::Int9M>& p1,
                    const pair<Interval<Instant>, toprel::Int9M>& p2) { 
          return p1.first < p2.first; 
     }


     void MTopRelAlg_RUP::init( const Region* reg, const UPoint* up, 
                                const toprel::PredicateGroup* pg ){


       if(!reg || !up){
          return;
       }
       if(!reg->IsDefined() || !up->IsDefined()){
          return;
       }
      // define sime topological predicates for furthe use 
      toprel::Int9M disjoint(0,0,1, 0,0,1, 1,0,1);
      toprel::Int9M onborder(0,0,1, 1,0,0, 0,0,1);
      toprel::Int9M ininterior(1,0,1, 0,0,1, 0,0,1);

       Periods periods(1);
       vector<pair<Interval<Instant>, toprel::Int9M> > tmpvector;
       if(reg->IsEmpty()){
         toprel::Int9M noreg(0,0,0, 0,0,0, 1,0,1);
         pair<Interval<Instant>, toprel::Int9M> p(up->timeInterval, noreg);
         tmpvector.push_back(p);
       } else { // nonempty region

          if(!reg->BoundingBox().Intersects(up->BoundingBoxSpatial())){
            pair<Interval<Instant>, toprel::Int9M> 
                     p(up->timeInterval, disjoint);
            tmpvector.push_back(p);
          } else if(AlmostEqual(up->p0, up->p1)){ // static upoint
            if(reg->InInterior(up->p0)){
               pair<Interval<Instant>, toprel::Int9M> 
                     p(up->timeInterval, ininterior);
               tmpvector.push_back(p);
            } else if(reg->OnBorder(up->p0)){
               pair<Interval<Instant>, toprel::Int9M> 
                      p(up->timeInterval, onborder);
               tmpvector.push_back(p);
            } else {
               pair<Interval<Instant>, toprel::Int9M>
                       p(up->timeInterval, disjoint);
               tmpvector.push_back(p);
            }
          } else { // non-empty region, non-static upoint

             periods.Add(up->timeInterval);
             if(up->timeInterval.lc && reg->OnBorder(up->p0)){
                pair<Interval<Instant>, toprel::Int9M> 
                        p(Interval<Instant>(up->timeInterval.start, 
                                            up->timeInterval.start,
                                            true,true),
                          onborder);
                tmpvector.push_back(p);
             }  
             
          
             // we perform a plane sweep algorithm
             // first, we create an AVLSegment from the unit and 
             // insert it into a
             // priority queue. So we don't have to care about the queue and the
             // upoint in parallel
             
             //priority queue for the splitted sgements of the region
             priority_queue<avlseg::ExtendedHalfSegment,
                  vector<avlseg::ExtendedHalfSegment>,
                  greater<avlseg::ExtendedHalfSegment> > qreg;
             // periority queue for the upoint's halgsegments
             priority_queue<avlseg::ExtendedHalfSegment,
                  vector<avlseg::ExtendedHalfSegment>,
                  greater<avlseg::ExtendedHalfSegment> > qup; 

             avlseg::ExtendedHalfSegment hs1(HalfSegment(true, up->p0, up->p1));
             avlseg::ExtendedHalfSegment hs2(HalfSegment(false, 
                                                up->p0, up->p1));

             qup.push(hs1);
             qup.push(hs2);
             int posreg=0;
             avlseg::ownertype currentOwner;
             avlseg::ExtendedHalfSegment cur;
             bool done = false;
             avltree::AVLTree<avlseg::AVLSegment> sss;
             const avlseg::AVLSegment* leftN;
             const avlseg::AVLSegment* rightN;
             avlseg::AVLSegment tmpL, tmpR, left1, common1, right1;            
 

             while( ((currentOwner = SelectNext(*reg, posreg, 
                          qreg, qup, cur)) != avlseg::none) &&
                    !done  ){

                avlseg::AVLSegment current(cur,currentOwner); 
                const avlseg::AVLSegment* member = 
                        sss.getMember(current,leftN,rightN);

                // make copies from the nieghbours to be able 
                // to manipulate the avl tree entries
                if(leftN){
                   tmpL = *leftN;
                   leftN = &tmpL;
                }
                if(rightN){
                  tmpR = *rightN;
                  rightN = &tmpR;
                }

                if(cur.IsLeftDomPoint()){
                  if(member){ // there is an overlapping segment found
                     assert(member->getOwner() != avlseg::both);
                     assert(member->getOwner() != currentOwner);
                     // split to extract the common part
                     uint32_t parts = 
                           member->split(current,left1,common1,right1);
                     // remove member from sss

                     sss.remove(*member); 
                     // by some numeric instabilities it's possibnle 
                     // to have a part left of current
                     if(parts & avlseg::LEFT){
                       if(!left1.isPoint()){
                          sss.insert(left1);
                          insertEvents(left1,false,true,qreg,qup);
                       }
                     }
                     // process common part
                    if(currentOwner==avlseg::first) {  // the region
                      if(current.getInsideAbove()){
                         common1.con_above++;
                      } else {
                         common1.con_above--;
                     }
                   } // for a upoint (line)  is nothing to do
                   if(!common1.isPoint()){
                       sss.insert(common1);
                      insertEvents(common1,false,true,qreg,qup);
                   } else {
                       Interval<Instant> iv = computeInterval(*up, common1);
                       periods.Minus(iv);   
                       pair<Interval<Instant>, toprel::Int9M> p(iv, onborder);
                       tmpvector.push_back(p);
                   }
                   if(parts & avlseg::RIGHT){
                     insertEvents(right1,true,true,qreg,qup);
                   }
                  } else { // no overlapping part found in sss
                    // check for possible intersections with neighbours


                    splitByNeighbour(sss,current,leftN,qreg,qup);

                    splitByNeighbour(sss,current,rightN,qreg,qup);




                    
                    // update coverage numbers
                   if(currentOwner==avlseg::first){ // the region
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
                  if(!current.isPoint()){


                      sss.insert(current);
                      insertEvents(current,false,true,qreg,qup);
                   } 
                  }
                } else {  // right end point
               if(member && member->exactEqualsTo(current)){
                 if( (member->getOwner()==avlseg::both)){
                     Interval<Instant> iv = computeInterval(*up, *member);
                     assert(periods.Contains(iv));
                     periods.Minus(iv);
                     pair<Interval<Instant>, toprel::Int9M> p(iv, onborder);
                     tmpvector.push_back(p); 
                 }
                 if( (member->getOwner()==avlseg::second)) {
                    Interval<Instant> iv = computeInterval(*up, *member);

                    periods.Minus(iv);
                    if(member->con_above>0){
                       pair<Interval<Instant>, toprel::Int9M> p(iv, ininterior);
                       tmpvector.push_back(p); 
                    } else {
                        // create a disjoint toprel for member
                       pair<Interval<Instant>, toprel::Int9M> p(iv, disjoint);
                       tmpvector.push_back(p); 
                    }
                 } 
                 sss.remove(*member);
                 splitNeighbours(sss,leftN,rightN,qreg,qup);
               }
           }
        } // while
          
             if(up->timeInterval.rc && reg->OnBorder(up->p1)){
                pair<Interval<Instant>, toprel::Int9M> p(
                                  Interval<Instant>(up->timeInterval.end, 
                                  up->timeInterval.end,
                                  true,true),
                           onborder);
                tmpvector.push_back(p);
             }  
              

          }
       }
       
      assert(periods.IsEmpty()); // the complete time interval 
                                 // should be exhausted

      sort(tmpvector.begin(), tmpvector.end(), pCompare);

      // go trough the vector and assign all interval 
      // endings to the "onborder predicate"
      // parallel merge intervals having the same relationship

      toprelvector.clear();

      if(tmpvector.size() > 0){
         pair<Interval<Instant>, toprel::Int9M> 
                        p1(Interval<Instant>(), toprel::Int9M(1));
         pair<Interval<Instant>, toprel::Int9M> 
                        p2(Interval<Instant>(), toprel::Int9M(1));
         p1 = tmpvector[0];
         for(size_t i = 1; i<tmpvector.size(); i++){
            p2 = tmpvector[i];
            if(p1.first.end != p2.first.start){
               
               cout << "crazy intervals found" << endl;
               cout << "p1 = " << p1.first << endl;
               cout << "p2 = " << p2.first << endl;
            }


            if(p1.second == p2.second){ // same relationship, extend 
               p1.first.Union(p2.first);
            } else {
             if(p1.first.rc && p2.first.lc){
                 if(p1.second == onborder){
                    if(p1.first.Intersects(up->timeInterval)){
                      p1.first.IntersectionWith(up->timeInterval);
                      toprelvector.push_back(p1);
                      p2.first.lc = false;
                    } 
                    p1 = p2;
                 } else if (p2.second == onborder){
                    p1.first.rc = false;
                    if(p1.first.Intersects(up->timeInterval)){
                       p1.first.IntersectionWith(up->timeInterval);
                       toprelvector.push_back(p1);
                    }
                    p1 = p2;
                 } else { // change from disjoint to ininterior or vice versa
                    p1.first.rc = false;
                    if(p1.first.Intersects(up->timeInterval)){
                       p1.first.IntersectionWith(up->timeInterval);
                       toprelvector.push_back(p1);
                    }
                    pair<Interval<Instant>, toprel::Int9M> 
                           p3(Interval<Instant>(p1.first.end,
                                                p1.first.end,true,true),
                              onborder);
                    if(p3.first.Intersects(up->timeInterval)){
                      toprelvector.push_back(p3);
                    }
                    p2.first.lc = false;
                    p1 = p2;
                 }
              } else if(!p1.first.lc && !p2.first.lc){ // small gap
                 if(p1.first.Intersects(up->timeInterval)){
                    p1.first.IntersectionWith(up->timeInterval);
                    toprelvector.push_back(p1);
                 }
                 pair<Interval<Instant>, toprel::Int9M> 
                           p3(Interval<Instant>(p1.first.end,
                                                p1.first.end,true,true),
                               onborder);
                 if(p3.first.Intersects(up->timeInterval)){
                   tmpvector.push_back(p3);
                 }
                 p1 = p2;
                 
              }
            }
         } 
         if(up->timeInterval.Intersects(p1.first)){
            p1.first.IntersectionWith(up->timeInterval);
            toprelvector.push_back(p1);
         }
      }

       // create cluster vector from toprel vector 
       if(pg==0){
         return;
       }
       if(!pg->IsDefined()){
         return;
       }

       if(toprelvector.size()==0){
         return;
       }

      pair<Interval<Instant>, toprel::Int9M> cpi = toprelvector[0];
      toprel::Cluster* cl = pg->GetClusterOf(cpi.second);
      pair<Interval<Instant>, toprel::Cluster> cpc(cpi.first, *cl);
      delete cl;
      for(unsigned int i=1; i< toprelvector.size(); i++){
         pair<Interval<Instant>, toprel::Int9M> npi = toprelvector[i];
         cl = pg->GetClusterOf(npi.second);
         if(cpc.second == *cl){ // toprel within the same cluster, 
                                //just enlarge the interval
              cpc.first.end = npi.first.end;
              cpc.first.rc = npi.first.rc;
         } else {
            clustervector.push_back(cpc);
            cpc.first = npi.first;
            cpc.second = *cl;
         }
         delete cl;
      }
      clustervector.push_back(cpc);


     
  




      // sort vector by time
      // create cluster vector from toprel vector


     }































/*
6 Computing topological relationship between a static region and moving(point)


*/

/*
1.1 Constructors


*/
      MTopRelAlg_RMP::MTopRelAlg_RMP(const Region* _r, const MPoint* _mp): 
        currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
        posCluster(0), r(_r), mp(_mp), pg(0){
        init();
      }
          

      MTopRelAlg_RMP::MTopRelAlg_RMP(const Region* _r, 
                                     const MPoint* _mp, 
                                     const toprel::PredicateGroup* _pg):
         currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
         posCluster(0), r(_r), mp(_mp), pg(_pg){
        init();
      }

      void MTopRelAlg_RMP::init(){
        if(r->IsDefined() && mp->IsDefined()){
          computeNextTopRel();
          if(pg && pg->IsDefined()){
             computeNextCluster();
          }
        }
      }


      MTopRelAlg_RMP::~MTopRelAlg_RMP(){ }


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool MTopRelAlg_RMP::hasNext() const{
        return currentUnitTopRel.size() > 0;
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_RMP::next(){
         assert(hasNext());
         pair<Interval<Instant>, toprel::Int9M> result = 
                                            currentUnitTopRel.front();
         currentUnitTopRel.pop();
         computeNextTopRel();
         return result;
      }

      bool MTopRelAlg_RMP::canBeConnected(const Interval<Instant>& i1, 
                                          const Interval<Instant>& i2) const{
         // TODO:  improve this implementation
         return i1.end == i2.start;
      }

      
      bool MTopRelAlg_RMP::canBeConnected(
                const pair<Interval<Instant>, toprel::Int9M>& f, 
                const pair<Interval<Instant>, toprel::Int9M>& s) const{
          return canBeConnected(f.first, s.first) && (f.second == s.second);
      }

      void MTopRelAlg_RMP::connect(
              pair<Interval<Instant> , toprel::Int9M>& f, 
              const  pair<Interval<Instant> , toprel::Int9M>& s) const{

           assert(canBeConnected(f,s));
           f.first.end = s.first.end;
           f.first.rc = s.first.rc;
     }

      
      void MTopRelAlg_RMP::computeNextTopRel(){
         if(currentUnitTopRel.size() > 1){
            return;
         }
         // make a first entry
         if(currentUnitTopRel.empty()){
            if(posTopRel >= mp->GetNoComponents()){
              // no more units available
              return;
            }
            UPoint up(1);
            mp->Get(posTopRel, up);
            posTopRel++;
            MTopRelAlg_RUP rup(r,&up,pg);
            while(rup.hasNext()){
              currentUnitTopRel.push(rup.next());
            }
         }
         // extend the last entry while it is possible 
         // (elements available and makes sense (only 1 element)
         assert(currentUnitTopRel.size()>0);
         while( (currentUnitTopRel.size()<2) && 
                (posTopRel < mp->GetNoComponents())){
            UPoint up(1);
            mp->Get(posTopRel, up); 
            posTopRel++;
            MTopRelAlg_RUP rup(r,&up,pg);
            while(rup.hasNext()){
              pair<Interval<Instant>, toprel::Int9M> n = rup.next();
              if(canBeConnected(currentUnitTopRel.back(), n)){
                 connect(currentUnitTopRel.back(), n);
              } else {
                 currentUnitTopRel.push(n);
              }
            }
         }
      }




/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool MTopRelAlg_RMP::hasNextCluster() const{
         return currentUnitCluster.size() > 0;
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> MTopRelAlg_RMP::nextCluster(){
         assert(hasNextCluster());
         pair<Interval<Instant>, toprel::Cluster> result = 
                                                   currentUnitCluster.front();
         currentUnitCluster.pop();
         computeNextCluster();
         return result;
      }


      bool MTopRelAlg_RMP::canBeConnected(
             const pair<Interval<Instant>, toprel::Cluster>& f, 
             const pair<Interval<Instant>, toprel::Cluster>& s) const{
          return canBeConnected(f.first, s.first) && (f.second == s.second);
      }

      void MTopRelAlg_RMP::connect(
             pair<Interval<Instant> , toprel::Cluster>& f, 
             const  pair<Interval<Instant> , toprel::Cluster>& s) const{

           assert(canBeConnected(f,s));
           f.first.end = s.first.end;
           f.first.rc = s.first.rc;
     }



      void MTopRelAlg_RMP::computeNextCluster(){
         if(currentUnitCluster.size() > 1){
            return;
         }
         // make a first entry
         if(currentUnitCluster.empty()){
            if(posCluster >= mp->GetNoComponents()){
              // no more units available
              return;
            }
            UPoint up(1);
            mp->Get(posCluster, up);
            posCluster++;
            MTopRelAlg_RUP rup(r,&up,pg);
            while(rup.hasNextCluster()){
              currentUnitCluster.push(rup.nextCluster());
            }
         }
         assert(currentUnitCluster.size()>0);
         while( (currentUnitCluster.size()<2) && 
                (posCluster < mp->GetNoComponents())){
            UPoint up(1);
            mp->Get(posCluster, up); 
            posCluster++;
            MTopRelAlg_RUP rup(r,&up,pg);
            while(rup.hasNextCluster()){
              pair<Interval<Instant>, toprel::Cluster> n = rup.nextCluster();
              if(canBeConnected(currentUnitCluster.back(), n)){
                 connect(currentUnitCluster.back(), n);
              } else {
                 currentUnitCluster.push(n);
              }
            }
         }
      }

