
/*
----
This file is part of SECONDO.

Copyright (C) 2015, University if Hagen, 
Faculty of Mathematics and  Computer Science,
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


1 Alternative implementation of r tree based optics algorithm

Todo:
   Removing requirement that the complete input relation has to fit
   to the main memory.

*/

#include "OrderSeeds.h"
#include "RectangleAlgebra.h"
#include "RelationAlgebra.h"
#include "Stream.h"

#include <iostream>     
#include <algorithm>    
#include <vector>       

template <int dim>
class OpticsR2{

public:


/*

1.1 Constructor

*/   
   OpticsR2(Word _instream, int _attrPos, double _eps, int _minPts,  
            double _UNDEFINED, ListExpr _resultType, size_t maxMem):
    instream(_instream), attrPos(_attrPos), eps(_eps), minPts(_minPts),  
    UNDEFINED(_UNDEFINED){

     tt = new TupleType(_resultType);
     index = new mmrtree::RtreeT<dim, TupleId>(4,8);
     initialize(maxMem);  
     optics();
     it = result.begin();
     delete index;
     index = 0;
   }

/*
1.2 Destructor

*/

   ~OpticsR2(){
      tt->DeleteIfAllowed();
      delete buffer;
      delete orderSeeds;
   }

/*
1.3 ~next~ function

Returns the next output tuple.

*/
   Tuple* next(){
      if(it!=result.end()){
         TupleId nextId = *it;
         it++;
         return buffer->GetTuple(nextId, true);
      }
      return 0;
   }

private:
   Stream<Tuple> instream;
   int attrPos;
   double eps;
   int minPts;
   int reachPos;
   int corePos; 
   int procPos;
   double UNDEFINED;
   TupleType* tt;
   TupleBuffer* buffer;
   mmrtree::RtreeT<dim, TupleId>* index;
   OrderSeeds<Rectangle<dim> >* orderSeeds;
   list<TupleId> result;
   list<TupleId>::iterator it;

   
/*
1.4 initialize
  
Fills the tuple buffer and builds an rtree over it.

*/
   void initialize( size_t maxMem ){
     buffer = new TupleBuffer(maxMem);
     instream.open();
     Tuple* inTuple;
     bool first = true;
     while((inTuple = instream.request())){
        Tuple *newTuple = new Tuple(tt);
        //Copy data from given tuple to the new tuple
        int attrCnt = inTuple->GetNoAttributes();
        for( int i = 0; i < attrCnt; i++ )  {
           newTuple->CopyAttribute( i, inTuple, i);
        }
        //Initialize the result tuple with default values
        newTuple->PutAttribute( attrCnt, new CcReal(-1.0));   // coreDist
        newTuple->PutAttribute( attrCnt+1, new CcReal(-1.0)); // reachDist
        newTuple->PutAttribute( attrCnt+2, new CcBool(true,false)); // processed
        newTuple->PutAttribute( attrCnt+3, new CcReal(eps)); // epsilon
        buffer->AppendTuple(newTuple);
        inTuple->DeleteIfAllowed();
        newTuple->DeleteIfAllowed();
        if(first){
          corePos = attrCnt;
          reachPos = attrCnt + 1;  
          procPos = attrCnt + 2;
          first = false;
        }
     }
     instream.close();

     GenericRelationIterator* relIt = buffer->MakeScan();
     Tuple* tuple;
     while( (tuple = relIt->GetNextTuple()) ){
        TupleId id = relIt->GetTupleId();
        Rectangle<dim>* rect = (Rectangle<dim>*) tuple->GetAttribute(attrPos);
        if(rect->IsDefined()){
           index->insert(*rect, id);
        } 
        tuple->DeleteIfAllowed();
     }
     delete relIt;
     orderSeeds = new OrderSeeds<Rectangle<dim> >(buffer, reachPos, 
                                    corePos, procPos, attrPos, UNDEFINED);

   }

/*
1.5 main algorithm

*/

   void optics(){ // optics main loop
      GenericRelationIterator* it = buffer->MakeScan();
      Tuple* tuple;
      while( ( tuple = it->GetNextTuple() )){
          TupleId id = it->GetTupleId();
          bool processed = ((CcBool*)tuple->GetAttribute(procPos))->GetValue();
          tuple->DeleteIfAllowed();
          if(!processed){
             expandCluster(id);
          }
      }
      delete it; 
   }

/*
1.6 expandCluster

Puts new elements to a cluster

*/
   void expandCluster(TupleId object){
      Tuple* tuple = buffer->GetTuple(object, true);
      list<TupleId>* neighbors = getNeighbors(tuple, object);
      tuple->PutAttribute(procPos, new CcBool(true,true));
      tuple->PutAttribute(reachPos, new CcReal(true, UNDEFINED));
      double coreDist = setCoreDistance(tuple, neighbors);
      result.push_back(object);
      if(coreDist != UNDEFINED){
         orderSeeds->update(*neighbors, object);
         delete neighbors;    
         while(!orderSeeds->empty()){
           TupleId currentId = orderSeeds->next();
           Tuple* curTup = buffer->GetTuple(currentId, true);
           neighbors = getNeighbors(curTup, currentId);
           curTup->PutAttribute(procPos, new CcBool(true,true));
           coreDist = setCoreDistance(curTup, neighbors);
           result.push_back(currentId);
           if(coreDist != UNDEFINED){
              orderSeeds->update(*neighbors, currentId);
           }
           delete neighbors;
           curTup->DeleteIfAllowed();
         }
      } else {
         delete neighbors;
      }
      tuple->DeleteIfAllowed();
   }

/*
1.7 getNeighbors

Retrieves the neighbors of an element.

*/
   list<TupleId>* getNeighbors(Tuple* tuple, TupleId id){
       Rectangle<dim>* rect = ((Rectangle<dim>*) 
          tuple->GetAttribute(attrPos)->Clone());
       rect->Extend(eps);
       set<TupleId> candidates;
       index->findAll(*rect , candidates);
       list<TupleId>* res = new list<TupleId>();
       set<TupleId>::iterator it;

       for(it = candidates.begin(); it!=candidates.end(); it++){
            if(id != *it){
               double dist = getDistance(tuple, *it);
               if(dist <= eps){
                  res->push_back(*it);
               } 
            }
       }
       delete rect;
       return res;
   }


/*
1.8 setCoreDistance

Computes the core distance of an element. 
The core distance is undefined of less than minPTs neighbors 
available, the distance to the min-pts neighbor (ordered by distance)
otherwise.

*/
  double setCoreDistance(Tuple* tuple, list<TupleId>* neighbors){
      if(neighbors->size() < minPts){
          tuple->PutAttribute(corePos, new CcReal(true, UNDEFINED));
          return UNDEFINED;
      }
      // find out the distance to the minPts't neighbor

      // maximum heap having exactly minPts entries
      // if the heap is full an a new entry shoul be inserted:
      // if the new entry is bigger than the stored maximum, ignore
      // otherwise replace the maximum value with the new value 
      // and let sink in this value
      double heap[minPts]; 
      list<TupleId>::iterator it;
      int entries = 0; 
      for(it= neighbors->begin(); it!=neighbors->end();it++){
         double dist = getDistance(tuple, *it);
         if(entries < minPts){ // heap has capacity
            heap[entries] = dist;
            int son = entries+1;
            // let go up the new dist
            while(son > 1){
              int father = (son)/2;
              if(heap[father-1] < dist){
                 heap[son-1] = heap[father-1];
                 heap[father-1] = dist;
                 son = father; 
              } else {
                 break;
              }
            } 
            entries++;
         } else {
            if(dist < heap[0]){
               heap[0] = dist; // replace old maximum by new value
               int pos = 1;
               int s1 = pos * 2;
               int s2 = pos * 2+1;
               while(s1 <= minPts){ // there is at least one son
                   int s;
                   if(s2 <= minPts){
                      // get son with maximum value
                      s = heap[s1-1] < heap[s2-1] ? s2 : s1;
                   } else {
                      // there is only one son
                      s = s1;
                   }
                   if(heap[pos-1] < heap[s-1]){ // new value smaller than son
                      heap[pos-1] = heap[s-1];
                      heap[s-1] = dist;
                   } else {
                      // stop sinking
                      break;
                   }
                   pos = s;
                   s1 = pos * 2;
                   s2 = s1 + 1;
               }
            }
         } 
      } 
      tuple->PutAttribute(corePos, new CcReal(true, heap[0]));
      return heap[0];     
   }


/*
1.9 getCoreDistanceSlow


Alternative implementation not using a heap.
For debugging only.

*/
   double getCoreDistanceSlow(Tuple* tuple, list<TupleId>* neighbors){
     if(neighbors->size() < minPts){
          return -1;
     }
     // put all Distances into an vector
     vector<double> distances;
     list<TupleId>::iterator it;
     for(it = neighbors->begin(); it != neighbors->end(); it++){
        distances.push_back(getDistance(tuple, *it));
     }

     sort(distances.begin(), distances.end());

     return distances[minPts-1];     
   }


/*
1.10 distance

Returns the distance between two tuples where the second one is given by id.

*/
   double getDistance(Tuple* t1, TupleId t2id){
      Tuple* t2 = buffer->GetTuple(t2id, true);
      double res = getDistance(t1,t2);
      t2->DeleteIfAllowed();
      return res;
   }


/*
1.11 distance

Computes the distance between two tuples according to the selected attribute.

*/
   double getDistance(Tuple* t1, Tuple* t2){
     return ((Rectangle<dim>*)t1->GetAttribute(attrPos))->Distance(
             *( (Rectangle<dim>*)t2->GetAttribute(attrPos)));
   }

};


