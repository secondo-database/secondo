
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
1 Computing topological relationship between a static point and unit(point)


The complete result ist computed within the constructor. Methods allow to 
iterate over the result.

*/

/*
1.1 Constructors


*/
      MTopRelAlg_PUP::MTopRelAlg_PUP(const Point* p, const UPoint* up): 
        currentIndexInt9M(0), currentIndexCluster(0){

         toprel::PredicateGroup pg(0);
         pg.SetToDefault();
         init(*p, *up,pg);

      }

      MTopRelAlg_PUP::MTopRelAlg_PUP(const Point* p, const UPoint* up, 
                                     const toprel::PredicateGroup* pg){
         init(*p,*up,*pg);
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
    void MTopRelAlg_PUP::init(const Point& p, const UPoint& up, 
                              const toprel::PredicateGroup& pg){
       toprels.clear();
       clusters.clear();
       currentIndexInt9M = 0;
       currentIndexCluster = 0;

       if(!p.IsDefined() || !up.IsDefined()){
          return;  // no result
       }
       // step 1: compute the 9 intersection matrices
       toprel::Int9M m(0);
       if(AlmostEqual(up.p0, up.p1)){
          if(AlmostEqual(p, up.p0)){
               m.Set(1,0,0, 0,0,0, 0,0,1); // set to "equal"
          } else {
               m.Set(0,0,1, 0,0,0, 1,0,1); // set to "disjoint"
          }
          pair<Interval<Instant>, toprel::Int9M> r(up.timeInterval, m);
          toprels.push_back(r);
       } else { // nonstatic upoint
          HalfSegment hs(true,up.p0, up.p1);
          if(!hs.Contains(p)){ // never on line
             m.Set(0,0,1, 0,0,0, 1,0,1); // set to "disjoint"
             pair<Interval<Instant>, toprel::Int9M> r(up.timeInterval, m);
             toprels.push_back(r);
          } else {
             // compute the instant when the points are equal
             double deltaX = up.p1.GetX() - up.p0.GetX();
             double deltaY = up.p1.GetY() - up.p0.GetY();
             double delta = deltaX>deltaY? (p.GetX() - up.p0.GetX()) / deltaX
                                         : (p.GetY() - up.p0.GetY()) / deltaY;

             DateTime dur(durationtype);
             dur = up.timeInterval.end - up.timeInterval.start;
             dur.Mul(delta);
             datetime::DateTime in(datetime::instanttype);
             in = up.timeInterval.start + dur;
             vector<Interval<Instant> > v = up.timeInterval.splitAround(in);
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
       if(!pg.IsDefined()){
         return;
       }

       if(toprels.size()==0){
         return;
       }

      pair<Interval<Instant>, toprel::Int9M> cpi = toprels[0];
      toprel::Cluster* cl = pg.GetClusterOf(cpi.second);
      pair<Interval<Instant>, toprel::Cluster> cpc(cpi.first, *cl);
      delete cl;
      for(unsigned int i=1; i< toprels.size(); i++){
         pair<Interval<Instant>, toprel::Int9M> npi = toprels[i];
         cl = pg.GetClusterOf(npi.second);
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
        posCluster(0), p(_p), mp(_mp), ownPredicateGroup(true){
        toprel::PredicateGroup pg1(0);
        pg1.SetToDefault();
        pg = new toprel::PredicateGroup(pg1);
        init();
      }
          

      MTopRelAlg_PMP::MTopRelAlg_PMP(const Point* _p, 
                                     const MPoint* _mp, 
                                     const toprel::PredicateGroup* _pg):
         currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
         posCluster(0), p(_p), mp(_mp),ownPredicateGroup(false), pg(_pg){
        init();
      }

      void MTopRelAlg_PMP::init(){
        computeNextTopRel();
        computeNextCluster();
      }


      MTopRelAlg_PMP::~MTopRelAlg_PMP(){
         if(ownPredicateGroup){
           delete pg;
         } 
      }


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



