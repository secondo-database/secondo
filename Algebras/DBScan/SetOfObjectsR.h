/*
----
This file is part of SECONDO.

Copyright (C) 2015, 
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
----

*/

#ifndef DBSCAN_SETOFOBJECTSR_H
#define DBSCAN_SETOFOBJECTSR_H

#include "AlgebraTypes.h"
#include "RelationAlgebra.h"
#include "RectangleAlgebra.h"
#include "MMRTree.h"
#include "TupleStore1.h"
#include "Stream.h"
#include "TupleInfo.h"


/*
1 Class SetOfObjectsR

This class provides an implementation of the setOfObjects for the
DBScan algorithm where the tuples are indexes by an r-tree.

The template parameters are the dimension and the Distance function.

*/


namespace dbscan{

  template <int dim, class D>
  class SetOfObjectsR{

  public:

/*
1.1 Constructor

*/

     SetOfObjectsR(Word _stream, ListExpr _tt, double _eps, 
                   int _minPts, size_t _maxMem, int _attrPos, D _dist):
          eps(_eps), minPts(_minPts), attrPos(_attrPos), index(0), 
          buffer(0), tupleStates(), tt(0), resIt(0), dist(_dist) {
       tt = new TupleType(_tt);
       initialize(_maxMem, _stream);
     }
/*
1.2 Destructor

*/
     ~SetOfObjectsR(){
         if(index) delete index;
         if(buffer) delete  buffer;
         if(tt) tt->DeleteIfAllowed();
         if(resIt) delete resIt;

     }

/*
1.3 ~initOutput~

Starts the begin of returning tuples.

*/
     void initOutput(){
       if(resIt) delete resIt;
       resIt = buffer->MakeScan();  
     }


/*
1.4 ~next~

Returns the next output tuple.
Requires the call of initOutput before.

*/
     Tuple* next(){
         assert(resIt);
         Tuple* tuple = resIt->GetNextTuple();
         if(!tuple){
            return 0;
         }
         TupleId id = resIt->GetTupleId();
         Tuple* resTuple = new Tuple(tt);
         int as = tuple->GetNoAttributes();

         for(int i = 0; i<as; i++){
            resTuple->CopyAttribute(i,tuple,i);
         }
         tuple->DeleteIfAllowed();
         resTuple->PutAttribute(as, new CcInt(true, tupleStates[id].clusterNo));
         resTuple->PutAttribute(as+1, new CcBool(true,tupleStates[id].visited));
          return resTuple;
     }

/*
1.5 makeScan

Returns an iterator over the input tuples. The caller of this function
is responsible to delete the iterator after usage.

*/
     GenericRelationIterator* makeScan() {
        return buffer->MakeScan(); 
     }

/*
1.6 ~getProcessed~

Returns the processed state of a specified tuple.

*/
     bool getProcessed(TupleId id){
      return tupleStates[id].visited;
     }

/*
1.7. ~setProcessed~

Changes the processed flag for a tuple.

*/
     void setProcessed(TupleId id, bool value){
       tupleStates[id].visited= value;
     }

/*
1.8 ~getNeighbors~

Returns the neighbors of a tuple according to the epsilon value given
in the constructor. The caller of this function is responsible to delete
the returned list.

*/
     list<TupleId>* getNeighbors(TupleId id){
       list<TupleId>* neighbors = new list<TupleId>();
       Tuple* tuple = buffer->GetTuple(id);
       Rectangle<dim>* rect1 = (Rectangle<dim>*) tuple->GetAttribute(attrPos);
       Rectangle<dim> rect2 = *rect1;
       rect2.Extend(eps);
       typename mmrtree::RtreeT<dim,TupleId>::iterator* it = index->find(rect2);
       const TupleId* tid;
       int count = 0;
       while( (tid = it->next())){
          if(*tid!=id){
              Tuple* st = buffer->GetTuple(*tid);
               if(dist(((Rectangle<dim>*)st->GetAttribute(attrPos)),rect1)
                 <= eps){
              neighbors->push_back(*tid);
              count++;
              }
             st->DeleteIfAllowed();
          }
       }
       tuple->DeleteIfAllowed();
       delete it;
       return neighbors;
     }


/*
1.9 ~getCluster~

Returns the current cluster id of a tuple.

*/
     int getCluster(TupleId id){
       return tupleStates[id].clusterNo;
     }

/*
1.10 ~setCluster~

Changes the cluster id of a tuple.

*/
     void setCluster(TupleId id, int value){
        tupleStates[id].clusterNo = value;
     }

/*
1.11 ~isSeed~

Checks whether the isSeed flag is set for a tuple.

*/
     bool isSeed(TupleId id){
        return tupleStates[id].isSeed;
     }


/*
1.12 ~setSeed~

Changes the seed flag for a tuple.

*/
     void setSeed(TupleId id, bool value){
         tupleStates[id].isSeed = value;
     }


  private:

/*
1.13 Members

*/
     double eps;  // epsilon value
     int minPts;  // minimum amount of neighbors
     int attrPos; // position of the rectangle attribute
     mmrtree::RtreeT<dim,TupleId>* index; // the index
     TupleStore1* buffer;  // buffer for input tuples
     vector<TupleInfo> tupleStates; // structir stroing tuple states
     TupleType* tt;   // the result tuple type
     GenericRelationIterator* resIt;  // iterator 
     D dist;                          // distance function

/*
1.14 ~initialize~

Processes the complete input stream and builds an r-tree index on it.

*/
     void initialize(size_t mem, Word _stream){
         Tuple* tuple;
          buffer = new TupleStore1(mem);
          index = new mmrtree::RtreeT<dim,TupleId>(4,8);
          Stream<Tuple> stream(_stream);
          stream.open();
          while((tuple = stream.request())){
             TupleId id = buffer->AppendTuple(tuple);
             Rectangle<dim>* rect = 
                  (Rectangle<dim>*)tuple->GetAttribute(attrPos);
             index->insert(*rect,id);
             TupleInfo info(false,-1);
             tupleStates.push_back(info);
             tuple->DeleteIfAllowed();
          }
          stream.close();
     }

  };

}

#endif


