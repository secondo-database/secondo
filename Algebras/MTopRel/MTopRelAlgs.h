
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

#include "TopRel.h"
#include "TemporalAlgebra.h"
#include "SpatialAlgebra.h"
#include "RefinementStream.h"



/*
0 Declaration of auxiliary functions;

*/

/*
0.1 ~mergeAdd~

Appends a new element to a container. If possible, the last element of the
container is extened instead of appending a new element. We can merge two 
pairs, if the interval of the second pair extends the interval of the first 
one and the second components of both pairs are equal.

*/

void mergeAdd(vector<pair<Interval<Instant>, toprel::Int9M> >& vec, 
              const pair<Interval<Instant>, toprel::Int9M>& elem);


void mergeAdd(queue<pair<Interval<Instant>, toprel::Int9M> >& q, 
              const pair<Interval<Instant>, toprel::Int9M>& elem);

void mergeAdd(vector<pair<Interval<Instant>, toprel::Cluster> >& vec, 
               const pair<Interval<Instant>, toprel::Cluster> & elem);


void mergeAdd(queue<pair<Interval<Instant>, toprel::Cluster> >& q, 
               const pair<Interval<Instant>, toprel::Cluster> & elem);


/*
0.2 computeInstant

Computes the instant when the up is on the point defined by (x,y).
Note that there is no check, whether this point is actually reached.
In this case, the instant may be outside the time interval of the
upoint.

*/
Instant computeInstant(const UPoint& up, const double x, const double y);


/*

0.3 selectNext

returns the segment with the smallest x value from reg (unit at posreg)
qreg, and qup. The segment is removed from the corresponding container 
(for the region, just the position counter is increased).

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
     avlseg::ExtendedHalfSegment& result );


/*
0.4 ~pcompare~

Checks whther the first component of the firt pair is smaller than
the first component of the second pair;

*/
     bool pCompare (const pair<Interval<Instant>, toprel::Int9M>& p1,
                    const pair<Interval<Instant>, toprel::Int9M>& p2); 


/*
0.5 ~computeInterval~

Computes the interval when the upoint is at the given segment.

*/

     Interval<Instant> computeInterval(const UPoint& up, 
                                       const avlseg::AVLSegment& seg);

/*
1 Computing topological relationship between a static point and unit(point)


The complete result ist computed within the constructor. Methods 
allow to iterate over the result.

*/



template<bool sym>
class MTopRelAlg_PUP_T {

   public:

/*
1.1 Constructors




*/
      MTopRelAlg_PUP_T(const Point* p, const UPoint* up);

      MTopRelAlg_PUP_T(const Point* p, const UPoint* up, 
                     const toprel::PredicateGroup* pg);


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool hasNext() const;


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> next();


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool hasNextCluster() const;

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> nextCluster();

   private:
      vector<pair<Interval<Instant>, toprel::Int9M> > toprels;
      vector<pair<Interval<Instant>, toprel::Cluster> >clusters;
      unsigned int currentIndexInt9M;
      unsigned int currentIndexCluster;


      void init( const Point* p, const UPoint* up, 
                 const toprel::PredicateGroup* pg);

};


/*
1.2 Implementation

*/

/*
1.1 Constructors


*/
      template<bool sym>
      MTopRelAlg_PUP_T<sym>::MTopRelAlg_PUP_T(const Point* p, 
                                              const UPoint* up): 
        currentIndexInt9M(0), currentIndexCluster(0){
         init(p, up, 0);
      }

      template<bool sym>
      MTopRelAlg_PUP_T<sym>::MTopRelAlg_PUP_T(const Point* p, const UPoint* up,
                                     const toprel::PredicateGroup* pg){
         init(p,up,pg);
      }

/*
1.2 hasNext

Checks whether more topological relationships are available.


*/
      template<bool sym>
      bool MTopRelAlg_PUP_T<sym>::hasNext() const{
         return currentIndexInt9M < toprels.size();
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      template<bool sym>
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_PUP_T<sym>::next(){
          assert(hasNext());
          currentIndexInt9M++;
          return toprels[currentIndexInt9M-1];
      }


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
  template<bool sym>
     bool MTopRelAlg_PUP_T<sym>::hasNextCluster() const{
        return currentIndexCluster < clusters.size();
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
     template<bool sym>
     pair<Interval<Instant>, toprel::Cluster> 
                          MTopRelAlg_PUP_T<sym>::nextCluster(){
         assert(hasNextCluster());
         currentIndexCluster++;
         return clusters[currentIndexCluster-1];
     }

/*
1.6 init

This functions computes the result and stores it into the private members.

*/ 
  template<bool sym>
  void MTopRelAlg_PUP_T<sym>::init(const Point* p, const UPoint* up, 
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
        if(sym){
           m.Transpose();
        }
        pair<Interval<Instant>, toprel::Int9M> r(up->timeInterval, m);
        toprels.push_back(r);
     } else { // nonstatic upoint
        HalfSegment hs(true,up->p0, up->p1);
        if(!hs.Contains(*p)){ // never on line
           m.Set(0,0,1, 0,0,0, 1,0,1); // set to "disjoint"
           if(sym){
              m.Transpose();
            }
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
              if(sym){
                 m.Transpose();
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




typedef MTopRelAlg_PUP_T<false> MTopRelAlg_PUP;
typedef MTopRelAlg_PUP_T<true> MTopRelAlg_UPP;




/*
2 Computing topological relationship between a static object and moving(object)

Note. This class stores a reference to a moving point given in the constructor

SO is the type of the static object
MO is the type of the moving object
UO is the unit type of MO
UP is the class for the unitprocessor processing SO and UO where UP is the 
   unit type of MO

*/

template<class SO, class MO, class UO, class UP>
class MTopRelAlg_SMO {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_SMO(const SO* _so, const MO* _mo):
        currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
        posCluster(0), so(_so), mo(_mo), pg(0){
        init();
     }

      MTopRelAlg_SMO(const SO* _so, const MO* _mo, 
                     const toprel::PredicateGroup* _pg):
         currentUnitTopRel(), currentUnitCluster(), posTopRel(0), 
         posCluster(0), so(_so), mo(_mo), pg(_pg){
        init();
      }

      ~MTopRelAlg_SMO() {}


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool hasNext() const{
        return currentUnitTopRel.size() > 0;
      }



/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> next(){
         assert(hasNext());
         pair<Interval<Instant>, toprel::Int9M> result = 
                                            currentUnitTopRel.front();
         currentUnitTopRel.pop();
         computeNextTopRel();
         return result;
      }



/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool hasNextCluster() const{
         return currentUnitCluster.size() > 0;
      }



/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> nextCluster(){
         assert(hasNextCluster());
         pair<Interval<Instant>, toprel::Cluster> result = 
                                                   currentUnitCluster.front();
         currentUnitCluster.pop();
         computeNextCluster();
         return result;
      }


   private:
/*
1.6 Members

*/
     queue< pair<Interval<Instant>, toprel::Int9M> > currentUnitTopRel;
     queue< pair<Interval<Instant>, toprel::Cluster> > currentUnitCluster;
     int32_t posTopRel;             // position in MO
     int32_t posCluster;            // position in MO
     const SO* so;                       // the point
     const MO* mo;                     // the mpoint
     const toprel::PredicateGroup* pg;             // used predicategroup


/*
1.7 Auxiliary functions

1.7.1 init

Checks parameters and fills the vectors with first values

*/
     void init(){
        if(so->IsDefined() && mo->IsDefined()){
          computeNextTopRel();
          if(pg && pg->IsDefined()){
             computeNextCluster();
          }
        }
      }

/*
1.7.2 canBeConnected 

Checks whether i2 can be appended to i1.

*/

     bool canBeConnected(const Interval<Instant>& i1, 
                         const Interval<Instant>& i2) const{
         return i1.end == i2.start;
     }

/*

1.7.2 canBeConnected 

Checks whether s can be appended to f. This is the case when the 
corresponding intervals can be connected and the topologocal relationships
of s and f are equal.

*/
   bool canBeConnected(const pair<Interval<Instant>, toprel::Int9M>& f, 
                       const pair<Interval<Instant>, toprel::Int9M>& s) const{
        return canBeConnected(f.first, s.first) && (f.second == s.second);
   }

/*
1.7.3 connect

Appends s to f. 
Precondition: canBeConnected(f,s).

*/

     void connect(pair<Interval<Instant>, toprel::Int9M>& f, 
             const pair<Interval<Instant>, toprel::Int9M>& s){

           assert(canBeConnected(f,s));
           f.first.end = s.first.end;
           f.first.rc = s.first.rc;
     }

/*
1.7.4 computeNextTopRel

Processes the units of mo while mo has more elements and the
toprel vector contains less than 2 elements. We need at least two elements in
this vector to be sure that all connections are done before returning the
next top rel.

*/
      void computeNextTopRel(){
         if(currentUnitTopRel.size() > 1){ // we have already enough entries
            return;
         }
         // make a first entry
         if(currentUnitTopRel.empty()){
            if(posTopRel >= mo->GetNoComponents()){
              // no more units available
              return;
            }
            UO uo(1);
            mo->Get(posTopRel, uo);
            posTopRel++;
            UP up(so,&uo,pg); // create a unit processor
            while(up.hasNext()){
              currentUnitTopRel.push(up.next());
            }
         }
         // extend the last entry while it is possible 
         // (elements available and makes sense (only 1 element)
         assert(currentUnitTopRel.size()>0);
         while( (currentUnitTopRel.size()<2) && 
                (posTopRel < mo->GetNoComponents())){
            UO  uo(1);
            mo->Get(posTopRel, uo); 
            posTopRel++;
            UP up(so,&uo,pg);
            while(up.hasNext()){
              pair<Interval<Instant>, toprel::Int9M> n = up.next();
              if(canBeConnected(currentUnitTopRel.back(), n)){
                 connect(currentUnitTopRel.back(), n);
              } else {
                 currentUnitTopRel.push(n);
              }
            }
         }
      }

 
     
      bool canBeConnected(
               const pair<Interval<Instant>, toprel::Cluster>& f, 
               const pair<Interval<Instant>, toprel::Cluster>& s) const{
          return canBeConnected(f.first, s.first) && (f.second == s.second);
      }


      void connect(pair<Interval<Instant>, toprel::Cluster>& f, 
             const pair<Interval<Instant>, toprel::Cluster>& s){

           assert(canBeConnected(f,s));
           f.first.end = s.first.end;
           f.first.rc = s.first.rc;
     }


      void computeNextCluster(){
         if(currentUnitCluster.size() > 1){ // enough elements
            return;
         }
         // make a first entry
         if(currentUnitCluster.empty()){
            if(posCluster >= mo->GetNoComponents()){
              // no more units available
              return;
            }
            UO uo(1);
            mo->Get(posCluster, uo);
            posCluster++;
            UP up(so,&uo,pg);
            while(up.hasNextCluster()){
              currentUnitCluster.push(up.nextCluster());
            }
         }
         assert(currentUnitCluster.size()>0);
         while( (currentUnitCluster.size()<2) && 
                (posCluster < mo->GetNoComponents())){
            UO uo(1);
            mo->Get(posCluster, uo); 
            posCluster++;
            UP up(so,&uo,pg);
            while(up.hasNextCluster()){
              pair<Interval<Instant>, toprel::Cluster> n = up.nextCluster();
              if(canBeConnected(currentUnitCluster.back(), n)){
                 connect(currentUnitCluster.back(), n);
              } else {
                 currentUnitCluster.push(n);
              }
            }
         }
      }

};


/*
2 Computing topological relationship between a static point and moving(point)

*/

typedef  MTopRelAlg_SMO<Point, MPoint, UPoint, MTopRelAlg_PUP> MTopRelAlg_PMP;
typedef  MTopRelAlg_SMO<Point, MPoint, UPoint, MTopRelAlg_UPP> MTopRelAlg_MPP;





/*
2 Computing topological relationship between two UPoint values

If the time intervals are not equal, the topological relationship
will be undefined for such time intervals where only one of the
upoints is defined. The complete result is computed in the constructor
and stored within a vector. This is possible, because the maximum number
of resulting relationships is 5 ( 2 undefined plus three for the common 
time interval).


*/
class MTopRelAlg_UPUP {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_UPUP(const UPoint* up1, const UPoint* up2);

      MTopRelAlg_UPUP(const UPoint* up1, const UPoint* up2, 
                     const toprel::PredicateGroup* pg);

      ~MTopRelAlg_UPUP() {}


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool hasNext() const;


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> next();


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool hasNextCluster() const;

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> nextCluster();

   private:
      vector< pair<Interval<Instant>, toprel::Int9M> > toprels;
      vector< pair<Interval<Instant>, toprel::Cluster> > clusters;
      uint8_t toprelPos;
      uint8_t clusterPos;
      

      void init( const UPoint* up1, const UPoint* up2, 
                 const toprel::PredicateGroup* pg);

      void computeClustersFromTopRels( const toprel::PredicateGroup* pg);

};




/*
2 Computing topological relationship between a upoint and an mpoint


*/
template<bool sym>
class MTopRelAlg_UPMP_T{

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_UPMP_T(const UPoint* up, const MPoint* mp);

      MTopRelAlg_UPMP_T(const UPoint* up, const MPoint* mp, 
                     const toprel::PredicateGroup* pg);

      ~MTopRelAlg_UPMP_T() {}


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool hasNext() const;


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> next();


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool hasNextCluster() const;

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> nextCluster();

   private:
       const UPoint* up;
       const MPoint* mp;
       RefinementStream<MPoint, MPoint, UPoint, UPoint> toprelRefinement; 
       RefinementStream<MPoint, MPoint, UPoint, UPoint> clusterRefinement; 
       const toprel::PredicateGroup* pg;
       queue< pair<Interval<Instant> , toprel::Int9M> > toprelqueue;
       queue< pair<Interval<Instant> , toprel::Cluster> > clusterqueue;

       void init();
       void computeNextTopRel();
       void computeNextCluster();


};



/*
4.2 Implementation 

*/
      template<bool sym>
      MTopRelAlg_UPMP_T<sym>::MTopRelAlg_UPMP_T(const UPoint* _up, 
                                                const MPoint* _mp):
           up(_up), mp(_mp), toprelRefinement(_up,_mp),
           clusterRefinement(_up,_mp),
            pg(0),  toprelqueue(), clusterqueue(){
          init();
      }

      template<bool sym>
      MTopRelAlg_UPMP_T<sym>::MTopRelAlg_UPMP_T(const UPoint* _up, 
                                                const MPoint* _mp, 
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
      template<bool sym>
      bool MTopRelAlg_UPMP_T<sym>::hasNext() const{
         return toprelqueue.size()>0;
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      template<bool sym>
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_UPMP_T<sym>::next(){
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
     template<bool sym>
      bool MTopRelAlg_UPMP_T<sym>::hasNextCluster() const{
        return clusterqueue.size() > 0;
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      template<bool sym>
      pair<Interval<Instant>, toprel::Cluster> 
                           MTopRelAlg_UPMP_T<sym>::nextCluster(){
        assert(hasNextCluster());
        pair<Interval<Instant>, toprel::Cluster> res = clusterqueue.front();
        clusterqueue.pop();
        computeNextCluster();
        return res;

      }

       template<bool sym>
       void MTopRelAlg_UPMP_T<sym>::init(){
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

      template<bool sym>
       void MTopRelAlg_UPMP_T<sym>::computeNextTopRel(){
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
                     pair<Interval<Instant>, toprel::Int9M> p = alg.next();
                     if(sym){
                        p.second.Transpose();
                     }
                     mergeAdd(toprelqueue,p);
                  }
               }
           }
       }


       template<bool sym>
       void MTopRelAlg_UPMP_T<sym>::computeNextCluster(){
           if(!pg){
             return;
           }
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
                  MTopRelAlg_UPUP alg(&up1,&up2);
                  while(alg.hasNext()){
                    pair<Interval<Instant>, toprel::Int9M> p1 = alg.next();
                    if(sym){
                        p1.second.Transpose();
                    }
                    pair<Interval<Instant>, toprel::Cluster> 
                                  p2(p1.first, pg->GetClusterOf(p1.second));
                    mergeAdd(clusterqueue,p2);
                  }
               }
           }
        }


typedef MTopRelAlg_UPMP_T<false> MTopRelAlg_UPMP;
typedef MTopRelAlg_UPMP_T<true> MTopRelAlg_MPUP;

/*
4 Computation between two mpoints.


*/





class MTopRelAlg_MPMP {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_MPMP(const MPoint* mp1, const MPoint* mp2);

      MTopRelAlg_MPMP(const MPoint* mp1, const MPoint* mp2, 
                     const toprel::PredicateGroup* pg);

      ~MTopRelAlg_MPMP() {}


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool hasNext() const;


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> next();


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool hasNextCluster() const;

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> nextCluster();

   private:
       const MPoint* mp1;
       const MPoint* mp2;
       RefinementStream<MPoint, MPoint, UPoint, UPoint> toprelRefinement; 
       RefinementStream<MPoint, MPoint, UPoint, UPoint> clusterRefinement; 
       const toprel::PredicateGroup* pg;
       queue< pair<Interval<Instant> , toprel::Int9M> > toprelqueue;
       queue< pair<Interval<Instant> , toprel::Cluster> > clusterqueue;

       void init();
       void computeNextTopRel();
       void computeNextCluster();


};




/*
6 Combination Region x UPoint


*/
template<bool sym>
class MTopRelAlg_RUP_T {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_RUP_T(const Region* reg, const UPoint* up);

      MTopRelAlg_RUP_T(const Region* reg, const UPoint* up, 
                       const toprel::PredicateGroup* pg);

      ~MTopRelAlg_RUP_T() {}


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/

      bool hasNext() const;


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      pair<Interval<Instant>, toprel::Int9M> next();


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/
      bool hasNextCluster() const;

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/
      pair<Interval<Instant>, toprel::Cluster> nextCluster();

   private:
       vector<pair<Interval<Instant>, toprel::Int9M> > toprelvector;
       vector<pair<Interval<Instant>, toprel::Cluster> > clustervector;
       size_t toprelpos;
       size_t clusterpos;


       void init(const Region* reg, const UPoint* up, 
                 const toprel::PredicateGroup* pg);

};




/*

5.1 Implementation

*/
   template<bool sym>
   MTopRelAlg_RUP_T<sym>::MTopRelAlg_RUP_T(const Region* reg, 
                                           const UPoint* up):
     toprelvector(), clustervector(), toprelpos(0), clusterpos(0){
     init(reg, up, 0);
   }

   template<bool sym> 
   MTopRelAlg_RUP_T<sym>::MTopRelAlg_RUP_T(const Region* reg, 
                                           const UPoint* up, 
                    const toprel::PredicateGroup* pg):
    toprelvector(), clustervector(), toprelpos(0), clusterpos(0){
    init(reg,up,pg);
   }


/*
1.2 hasNext

Checks whether more topological relationships are available.


*/
      template<bool sym>
      bool MTopRelAlg_RUP_T<sym>::hasNext() const {
        return toprelpos < toprelvector.size();
      }


/*
1.3 next

Precondition: hasNext()

Returns the next element of the result. The result consists of a time interval
and the corresponding topological relationship between the time interval.

*/
      template<bool sym>
      pair<Interval<Instant>, toprel::Int9M> MTopRelAlg_RUP_T<sym>::next(){
         assert(hasNext());
         pair<Interval<Instant>, toprel::Int9M> res = toprelvector[toprelpos];
         toprelpos++;
         return res;
      }


/*
1.4 hasNextCluster

Checks whether at least one more cluster is available in the result.

*/    
      template<bool sym>
      bool MTopRelAlg_RUP_T<sym>::hasNextCluster() const{
         return clusterpos < clustervector.size();
      }

/*
1.5 nextCluster

Precondition: hasNextCluster

Returns the next cluster with its corresponding time interval.

*/


      template<bool sym>
      pair<Interval<Instant>, toprel::Cluster> 
                              MTopRelAlg_RUP_T<sym>::nextCluster(){
         assert(hasNextCluster());
         pair<Interval<Instant>, toprel::Cluster> 
                 res = clustervector[clusterpos];
         clusterpos++;
         return res;
      }


   template<bool sym>
   void MTopRelAlg_RUP_T<sym>::init( const Region* reg, const UPoint* up, 
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
    toprel::Int9M noreg(0,0,0, 0,0,0, 1,0,1);

    if(sym){
      disjoint.Transpose();
      onborder.Transpose();
      ininterior.Transpose();
      noreg.Transpose();
    }

     vector<pair<Interval<Instant>, toprel::Int9M> > tmpvector;
     if(reg->IsEmpty()){
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
           // priority queue. So we don't have to care about the queue 
           // and the upoint in parallel
           
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
                   pair<Interval<Instant>, toprel::Int9M> p(iv, onborder);
                   tmpvector.push_back(p); 
               }
               if( (member->getOwner()==avlseg::second)) {
                  Interval<Instant> iv = computeInterval(*up, *member);

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
             
             //cout << "crazy intervals found" << endl;
             //cout << "p1 = " << p1.first << endl;
             //cout << "p2 = " << p2.first << endl;
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
                 toprelvector.push_back(p3);
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

   }





typedef MTopRelAlg_RUP_T<false> MTopRelAlg_RUP;
typedef MTopRelAlg_RUP_T<true> MTopRelAlg_UPR;



/*
6 Computing topological relationship between a static region and moving(point)

Note. This class stores  references to the region and to the moving point. 
After deleting the sources,
this class will crash if further functions are called.

*/

typedef MTopRelAlg_SMO<Region, MPoint, UPoint, MTopRelAlg_RUP> MTopRelAlg_RMP;
typedef MTopRelAlg_SMO<Region, MPoint, UPoint, MTopRelAlg_UPR> MTopRelAlg_MPR;





