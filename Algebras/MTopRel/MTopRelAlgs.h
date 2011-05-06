
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
2 Computing topological relationship between a static point and moving(point)

Note. This class stores a reference to a moving point given in the constructor

*/
class MTopRelAlg_PMP {

   public:

/*
1.1 Constructors


*/
      MTopRelAlg_PMP(const Point* p, const MPoint* mp);

      MTopRelAlg_PMP(const Point* p, const MPoint* mp, 
                     const toprel::PredicateGroup* pg);

      ~MTopRelAlg_PMP();


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
     queue< pair<Interval<Instant>, toprel::Int9M> > currentUnitTopRel;
     queue< pair<Interval<Instant>, toprel::Cluster> > currentUnitCluster;
     int32_t posTopRel;             // position in mpoint
     int32_t posCluster;            // position in mpoint
     const Point* p;                       // the point
     const MPoint* mp;                     // the mpoint
     const toprel::PredicateGroup* pg;             // used predicategroup


     void init(); // initializes the computation

     bool canBeConnected(const Interval<Instant>& i1, 
                         const Interval<Instant>& i2) const;

     bool canBeConnected(const pair<Interval<Instant>, toprel::Int9M>& f, 
                         const pair<Interval<Instant>, toprel::Int9M>& s) const;

     void connect(pair<Interval<Instant>, toprel::Int9M>& f, 
             const pair<Interval<Instant>, toprel::Int9M>& s);

      void computeNextTopRel(); 
     
      bool canBeConnected(
               const pair<Interval<Instant>, toprel::Cluster>& f, 
               const pair<Interval<Instant>, toprel::Cluster>& s) const;

      void connect(pair<Interval<Instant>, toprel::Cluster>& f, 
             const pair<Interval<Instant>, toprel::Cluster>& s);

      void computeNextCluster();

};




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
