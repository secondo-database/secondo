/*
----
This file is part of SECONDO.

Copyright (C) 2014,
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

This file provides auxiliary function dealing with precise halfsegments.

*/


#ifndef HSTOOLS_H
#define HSTOOLS_H

#include <vector>
#include "PreciseHalfSegment.h"
#include "AvlTree.h"

namespace hstools{



enum SETOP {UNION, INTERSECTION, DIFFERENCE};


/*
~isSorted~

Checks whether ~v~ is sorted according to the halfsegment order.

*/
  bool isSorted(const vector<MPrecHalfSegment>& v);

/*
~sort~

Sorts ~v~ using the halfsegment order.

*/
  void sort(vector<MPrecHalfSegment>& v);


/*
~isLogicalSorted~

Checks whether ~v~ is sorted using the logical order (faceno, cycleno, edgeno) 

*/
   bool isLogicalSorted(const vector<MPrecHalfSegment>& v);


/*
~sortLogical~

Sorts ~v~ according to the logical order of halfsegments 
(faceno, cycleno, edgeno)

*/

  void sortLogical(vector<MPrecHalfSegment>& v);


/*
~setCoverage~

Sets the coverage number of a set of halfsegments. This number is used 
in the accerelated computation of the point in region algorithm. The argument 
v must be sorted according to the halfsegment order.

*/
   void setCoverage(vector<MPrecHalfSegment>& v);


/*
~setPartnerNumbers~

This function sets the partner number of the contained halfsegments. 
Partners must have the same edge number. Non-partners must have different
edge numbers. The edges must be numbered from 0 to n-1 where n is the 
number of contained segments ( v.size()/2).

*/

   bool setPartnerNumbers(vector<MPrecHalfSegment>& v);


/*
~checkRealm~

This function checks whether the segments contained in v are realminized. 
This means, two different segments in ~v~ have no common point except end 
points. The halfsegments in ~v~ have to be sorted in halfsegment order.

*/
   bool checkRealm(const vector<MPrecHalfSegment>& v);


/*
~realminize~

The function computes a realminized version of ~v~ and stores it in ~res~.
~v~ has to be sorted according to the halfsegment order.

*/

   void realminize(const vector<MPrecHalfSegment>& v, 
                   vector<MPrecHalfSegment>& res);





/*
~checkCycles~

This funtion checks whether ~v~ consists of cycles only, i.e. whether each 
dominating point is reached by an even number of halfsegments. ~v~ has to 
be sorted according to the halfsegment order.

*/
   bool checkCycles(const vector<MPrecHalfSegment>& v);


/*
~removeConnections~

This function will copy all halfsegments of ~v~ belonging to cycles into ~res~.
v has to be sorted in halfsegment order.

*/
   void removeConnections(const vector<MPrecHalfSegment>& v, 
                          vector<MPrecHalfSegment>& res);


/*
~setInsideAbove~

This function will compute and set the insideAbove flag for each 
halfsegment in ~v~. 
~v~ has to be sorted in halfsegment order.

*/

  void setInsideAbove(vector<MPrecHalfSegment>& v);


/*
~computeCycles~

This function computes the faceno, cycleno, edgeno for each halfsegment in ~v~.
The halfsegments in v has to be sorted in halfsegment order , has to be 
realminized, and has to build only cycles.

*/

  void computeCycles(vector<MPrecHalfSegment>& v);

/*
~checkRealm~

This function checks whether the segments contained in v are realminized. 
This means, two different segments in ~v~ have no common point except 
end points. The halfsegments in ~v~ have to be sorted in halfsegment order.

*/
   template<class HS> 
   class YComparator{
     public:
        static bool smaller(const HS& hs1, 
                            const HS& hs2){
           return compare(hs1,hs2)<0;
        }
        static bool equal(const HS& hs1, 
                          const HS& hs2){
           return compare(hs1,hs2)==0;
        }

        /** the less operator **/
        bool operator()(const HS& hs1, 
                        const HS& hs2){
           return compare(hs1,hs2)<0;
        }
     
      private: 
        static int compare(const HS& hs1, 
                           const HS& hs2){

            if(hs1.getMinY() > hs2.getMaxY()){
                return 1;
            }
            if(hs2.getMinY() > hs1.getMaxY()){
               return -1;
            }

            MPrecCoordinate x0(0);

            if(hs1.isVertical()){
               x0 = hs1.getLeftPoint().getX();
            } else if(hs2.isVertical()){
               x0 = hs2.getLeftPoint().getX();
            } else {
               x0 = max(hs1.getLeftPoint().getX(), hs2.getLeftPoint().getX());
            }

            MPrecCoordinate y1 = hs1.getY(x0);
            MPrecCoordinate y2 = hs2.getY(x0);
            int cmp = y1.compare(y2);
            if(cmp!=0){
               return cmp;
            }
            if(hs1.isVertical()){
               return hs2.isVertical()?0:1;
            }
            if(hs2.isVertical()){
              return -1;
            }


            x0 = min(hs1.getRightPoint().getX(), hs2.getRightPoint().getX());
            y1 = hs1.getY(x0);
            y2 = hs2.getY(x0);             
            return y1.compare(y2);
 
            //return hs1.compareSlope(hs2);
        }
   };

/*
~setOP~

This function computes the set operation specified by the last argument

*/
  template<class S1, class S2>
  class EventStructure{
    public: 
     EventStructure(const S1* _v1,
                    const S2* _v2) : 
                          v1(_v1), v2(_v2), pos1(0), pos2(0),
                          pq1(), pq2(),pqb() {
        tmpCand = new MPrecHalfSegment[5];
     }

     ~EventStructure(){
         delete[] tmpCand;
      }
     
     int  next(MPrecHalfSegment& result) {
         const MPrecHalfSegment* candidates[5];
         candidates[0] = pos1<v1->size()?&((*v1)[pos1]):0;
         candidates[1] = pos2<v2->size()?&((*v2)[pos2]):0;
         candidates[2] = pq1.empty()?0:&pq1.top();
         candidates[3] = pq2.empty()?0:&pq2.top();
         candidates[4] = pqb.empty()?0:&pqb.top();
         int index = -1;
         for(int i=0;i<5;i++){
            if(candidates[i]){
               if(index<0){
                   index = i;
               } else  if(cmp(*candidates[i],*candidates[index]) < 0 ){
                   index = i;
               }
            }
         }
 
         switch(index){
           case -1 : return 0; // no more halfsegments
           case  0 : result.set(FIRST, *(candidates[0]));pos1++; return 1;
           case  1 : result.set(SECOND, *(candidates[1]));pos2++; return 2;
           case  2 : result.set(FIRST, *candidates[2]);pq1.pop(); return 1;
           case  3 : result.set(SECOND, *candidates[3]);pq2.pop(); return 2;
           case  4 : result.set(BOTH, *candidates[4]);pqb.pop(); return 3;
         }; 
         return -1;
     }


     void push(const MPrecHalfSegment& evt){
         switch(evt.getOwner()){
           case FIRST  : pq1.push(evt); break;
           case SECOND : pq2.push(evt); break;
           case BOTH   : pqb.push(evt); break;
           default : assert(false);
         }
     }

     size_t size() const{
       return (v1->size()-pos1) + (v2->size()-pos2) + 
               pq1.size() + pq2.size() + pqb.size();
     }


    private:
       const S1* v1;
       const S2* v2;
       size_t pos1;
       size_t pos2;
       priority_queue<MPrecHalfSegment, 
                      vector<MPrecHalfSegment>, 
                      IsGreater<MPrecHalfSegment, HalfSegmentComparator> > pq1;
       priority_queue<MPrecHalfSegment, 
                      vector<MPrecHalfSegment>, 
                      IsGreater<MPrecHalfSegment, HalfSegmentComparator> > pq2;
       priority_queue<MPrecHalfSegment, 
                      vector<MPrecHalfSegment>, 
                      IsGreater<MPrecHalfSegment, HalfSegmentComparator> > pqb;
       HalfSegmentComparator cmp;
       MPrecHalfSegment* tmpCand;

  };

void makeRealm(const MPrecHalfSegment& hs1, const MPrecHalfSegment& hs2, 
              vector<MPrecHalfSegment>& res); 


void setOP(const vector<MPrecHalfSegment>& v1,
             const vector<MPrecHalfSegment>& v2,
             vector<MPrecHalfSegment>& res,
             SETOP op);



} // end of namespace hstools

#endif


