
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
                  while(alg.hasNext()){
                     mergeAdd(clusterqueue,alg.nextCluster());
                  }
               }
           }
        }


