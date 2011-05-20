
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
1 Computing topological relationship between a static point and unit(point)


The complete result ist computed within the constructor. Methods 
allow to iterate over the result.

*/
class MTopRelAlg_PUP {

   public:

/*
1.1 Constructors




*/
      MTopRelAlg_PUP(const Point* p, const UPoint* up);

      MTopRelAlg_PUP(const Point* p, const UPoint* up, 
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
2 Computing topological relationship between a static iobject  and moving(object)

Note. This class stores a reference to a moving point given in the constructor

SO is the type of the static object
MO is the type of the moving object
UO is the unit type of MO
UP is the class for the unitprocessor processing SO and UO where UP is the unit type of MO

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
class MTopRelAlg_UPMP {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_UPMP(const UPoint* up, const MPoint* mp);

      MTopRelAlg_UPMP(const UPoint* up, const MPoint* mp, 
                     const toprel::PredicateGroup* pg);

      ~MTopRelAlg_UPMP() {}


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

class MTopRelAlg_RUP {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_RUP(const Region* reg, const UPoint* up);

      MTopRelAlg_RUP(const Region* reg, const UPoint* up, 
                     const toprel::PredicateGroup* pg);

      ~MTopRelAlg_RUP() {}


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
6 Computing topological relationship between a static region and moving(point)

Note. This class stores  references to the region and to the moving point. After deleting the sources,
this class will crash if further functions are called.

*/

typedef MTopRelAlg_SMO<Region, MPoint, UPoint, MTopRelAlg_RUP> MTopRelAlg_RMP;





