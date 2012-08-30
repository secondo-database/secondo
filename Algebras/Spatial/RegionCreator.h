
/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Department of Computer Science,
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


#include "SpatialAlgebra.h"
#include <vector>
/*
1 Class RegionCreator

This class contains functions for a creation of a region from a set 
of halfsegments.

*/

class RegionCreator{

  public:

/*
1.1 Public Function

This function creates a region from a set of halfsegments.
The Halfsegmentarray must be:
  - complete : for each halfsegment, the corresponding partner is inside
  - correct respective to partner numbers
  - realminized, i.e. overlapping or crossing halfsegments are not allowed
  - sorted with respect to the halfsegment order

*/
   static  void createRegion(const DbArray<HalfSegment>*, Region* result);

    // sets the partner, assumes equal edgenos for partners
    static void setPartnerNo(DbArray<HalfSegment>* hss);
  private:

/*
1.2 Members 

*/
     vector<vector<HalfSegment> > cycles; // stored cycles found
     vector<bool> holes;                  // flags whethe a cycle is a hole
     vector<int> correspondingOuters;     // cooresponding outer cycles for 
                                          // each cycle
                                          // outer cycles refer to themself
     vector<int> leftMostPoints;          // index of the leftmost dominating 
                                          // point of each cycle

     // Constructor
     RegionCreator(const DbArray<HalfSegment>* hss, Region* _result);



    // forcePairsFromLeftDom
    static DbArray<HalfSegment>* 
       forcePairsFromLeftDom(const DbArray<HalfSegment>* hss);



    // findCycles, fills the cycles vector from the halfsegments
    void findCycles(const DbArray<HalfSegment>* hss);

    // finds a single cycle
    void findCycle(const DbArray<HalfSegment>* hss, int pos, 
                   char* usage, const char* critical);

    
    // returns the Halfsegment with maximum slope at a given dominating point 
    // i.e. the dominating point of the halfsgement indexed by pos within hss
    static int getStartPos(const DbArray<HalfSegment>* hss, 
                           int pos, char* usage);

    // returns an extension of a cycle, or -1 if no extension is possible
    static int getNext(const DbArray<HalfSegment>* hss, int pos,  
                       const char* usage);

    
    // find the critical points within hss
    static void findCritical(const DbArray<HalfSegment>* hss, char* critical);

    // checks whether r is to the right of the ray defined by p qnd q 
    static bool isRight(const Point& p, const Point& q, const Point& r);

    // checks whether hs2 is more right than hs1
    static bool moreRight(const HalfSegment& hs1, const HalfSegment& hs2);

    // output functions
    void printCycles() const;
    void printCycle(size_t i) const;

    // detects holes within the cycles vector
    void detectHoles();
    // checks whether the cycle at position index is a hole
    bool isHole(size_t index) const; 
    // checks whether a point is inside the region defined by all cycles except 
    // the one at position ommit
    bool isInside(const double x, const double y, size_t ommit) const;
    // checks whether a horizontal ray starting at (x,y) to left 
    // intersects hs
    size_t intersects(const double x, const double y, 
                      const HalfSegment& hs) const; 

    // finds out the outer cycles where the holes belongs to
    void findCorrespondingOuters();
    // find the nearest outer cycle intersecting a horizontal ray
    // starting at(x,y) to left
    int findLeftNearestOuter(const double x, const double y) const;
    // returns the length of the segment from (x,y) to the 
    // interscetion point of a ray ... 
    double getLeftDist(const int cycle, const double x, const double y) const;
    // returns the length of the segment starting at (x,y) 
    // going horizontal to left
    // until the interscetion point with hs, if no intersection point exists,
    // -1 is returned
    double getLeftDist(const HalfSegment& hs, 
                       const double x, const double y) const;
    // sets the insideabove flags of the halfsegments within the cycles
    void setInsideAbove();
    // sets the insideAbove flags for the halfsegments of a specified cycle
    void setInsideAbove(const int i);
    // creates a region from the halfsegments and all other vectors
    void buildRegion(Region* result) const;
    // saves a single face
    void saveFace(const int cycle, const int faceno, 
                  int& edgeno, Region * result) const;
    // saves a single cycle
    void saveCycle(const int cycle, const int faceno, 
                   const int cycleno, int& edgeno, Region* result) const;

};


