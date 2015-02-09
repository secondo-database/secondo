
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


1 class OrderSeeds

This class represents the orderSeeds structure for the optics algorithm.

*/

#ifndef OPTICS_ORDERSEEDS_H
#define OPTICS_ORDERSEEDS_H

 // attribute type for clustering
 // Distance function
 // set of objects
template <class SoO>
class OrderSeeds{

  public:
    OrderSeeds( SoO* objects, double _UNDEFINED);

    void update(list<TupleId>& neighbors, TupleId objId);

    TupleId next();

    bool empty();

  private:
     SoO* objs;
     double UNDEFINED; 
     vector<TupleId> heap;
     map<TupleId, size_t> index;

     bool firstError;

     void insert( TupleId id, double new_r_dist );
     void decrease( TupleId id, double new_r_dist );
     double distance(Tuple* t1, Tuple* t2);
     void deleteMin();
     void sink(int index);
     double getReachDist(int index);
     void swap(int index1, int index2);
     // for dbugging only
     bool checkStructureDebug();
};

/*
1.1 Constructor

*/

template <class S>
OrderSeeds<S>::OrderSeeds(S* _objs, double _UNDEFINED):
    objs(_objs), UNDEFINED(_UNDEFINED) {
     firstError= true;
 }

/*
1.2 check for emptyness

*/
template <class S>
  bool OrderSeeds<S>::empty(){
     return heap.empty();
  }

/*
1.3 returns the next (minimum) element

*/
template <class S>
TupleId OrderSeeds<S>::next(){
     assert(!empty());
     TupleId res = heap[0];
     deleteMin();
     return res;
}  


/*
1.4 update operation

This operation updates thes structure. neighbbors are the neighbors of the 
tuple with id ~objId~.

*/
template <class S>
  void OrderSeeds<S>::update(list<TupleId>& neighbors, TupleId objId){
      Tuple* obj = objs->GetTuple(objId);
      double c_dist = objs->getCoreDist(objId);
      list<TupleId>::iterator it;
      for(it = neighbors.begin(); it!=neighbors.end(); it++){
         TupleId cId = *it;
         Tuple* cur = objs->GetTuple(cId);
         bool processed = objs->getProcessed(cId);
         if(!processed){
            double new_r_dist = max(c_dist, objs->distance(obj, cur));
            double old_r_dist = objs->getReachDist(cId); 
            if(old_r_dist == UNDEFINED){
               objs->updateReachability( cId, new_r_dist);
               insert(cId, new_r_dist);
            } else {
               if(new_r_dist < old_r_dist){
                  objs->updateReachability(cId, new_r_dist);
                  decrease(cId, new_r_dist);
               }
            }
         } 
         cur->DeleteIfAllowed();
      }
      obj->DeleteIfAllowed();
  }


/*
1.7 DeletMin

Removes the minimum element.

*/
template<class S>
void OrderSeeds<S>::deleteMin(){
   assert(!empty());
   TupleId id = heap[0];
   index.erase(id);
   if(heap.size()==1){ // last element
     heap.clear();
     return;
   }
   // more than one element before removing minimum
   heap[0] = heap[heap.size()-1]; // bring last element to front
   index[heap[0]] = 0; // update index
   heap.pop_back();
   sink(0);
}

/*
1.8 getReachDist

Extracts the reachability distance of the tuple at index index in the 
heap.

*/
template<class S>
double OrderSeeds<S>::getReachDist(int index){
   assert(index >=0);
   assert(index < (int)heap.size());
   TupleId id = heap[index];
   return objs->getReachDist(id);
}


/*
1.9 sink

let the element at position index sink into the heap unil
it is a leaf or all sons are greater/equal than the 
element itselfs. 

*/
template<class S>
void OrderSeeds< S>::sink(int index){
   double dist = getReachDist(index);
   int pos = index + 1;
   int s1 = pos*2;
   int s2 = pos*2 + 1;
   while( s1 <= heap.size()){ // at least 1 son present
     if(s2 > heap.size()){ // only one son
        double s1dist = getReachDist(s1-1);
        if(s1dist < dist){
           swap(pos-1, s1-1);
           pos = s1;
        } else {
           return;
        }
     } else { // both sons are there
        double s1dist = getReachDist(s1-1);
        double s2dist = getReachDist(s2-1);
        int s;
        double sdist;
        if(s1dist < s2dist){
           s = s1;
           sdist = s1dist;
        }  else {
           s = s2;
           sdist = s2dist;
        }
        if(sdist < dist){
          swap(pos-1, s-1);
          pos = s;
        } else {
          return;
        }
     }
     s1 = pos*2;
     s2 = pos*2 + 1;
   }
}

/*
1.9 insert

Inserts a new element into the heap.

*/
template<class S>
void OrderSeeds<S>::insert(TupleId id, double dist){
   heap.push_back(id);
   index[id] = heap.size()-1; 
   int pos = heap.size();
   while(pos > 1){
     int father = pos / 2;
     double fdist = getReachDist(father-1);
     if(fdist > dist){
        swap(pos-1, father-1);
        pos = father;
     } else {
        return; 
     }
   }
}

/*
1.10 decrease

Updates the reachability distance to an already existing object.

*/

template<class S>
void OrderSeeds<S>::decrease(TupleId id, double newDist){
   Tuple* tuple = objs->GetTuple(id);
   double oldDist = objs->getReachDist(id); 
   if(newDist > oldDist){
     cout << "decrease increases distance" << endl;
     cout << "olddist = " << oldDist << endl;
     cout << "newDist = " << newDist << endl;
     assert(oldDist > newDist);
   }
   objs->updateReachability(id, newDist);
   tuple->DeleteIfAllowed();
   // blow up
   int pos = index[id]+1;  
   while(pos > 1){
     int father = pos / 2;
     double fdist = getReachDist(father-1);
     if(fdist > newDist){
        swap(pos-1, father-1);
        pos = father;
     } else {
        return; 
     }
   }
}

/*
1.11 checkStructureDebug

This method checks the internal structure for corectness.

*/

template<class S>
bool OrderSeeds<S>::checkStructureDebug(){

  // check the index
  for(size_t i=0;i<heap.size();i++){
     size_t mapped =  index[heap[i]];
      if(mapped!=i){
         cout << "error in index found" << endl;
         return false;
      }
  }
  // check whether map only contains vector entries
  map<TupleId,size_t>::iterator it;
  for(it=index.begin();it!=index.end();it++){
     TupleId id = it->first;
     int index = it->second;
     if(heap[index] != id){
        cout << "error in index found" << endl;
        return false;
     }
  }

  // check for heap property
  for(size_t i=0;i<heap.size(); i++){
     int f = i+1;
     double fdist = getReachDist(f-1);

     int s1 = f*2 -1;
     int s2 = s1 +1;
     if(s1<heap.size() && fdist > getReachDist(s1)){
        cout << "Error in heap structure" << endl;
        if(firstError){
            cout << "Heap : " << endl;
            for(size_t j=0;j<heap.size();j++){
                cout << heap[j] << getReachDist(j) << endl;
            }
           firstError=false;
        }

        return false;
     } 
     if(s2<heap.size() && fdist > getReachDist(s2)){
        cout << "Error in heap structure" << endl;
        if(firstError){
            cout << "Heap : " << endl;
            for(size_t j=0;j<heap.size();j++){
                cout << heap[j] << getReachDist(j) << endl;
            }
           firstError=false;
        }
        return false;
     } 
  }
  return true;
}


template<class S>
void OrderSeeds<S>::swap(int index1, int index2){
   // swap
   TupleId id1 = heap[index1];
   TupleId id2 = heap[index2];
   heap[index1] = id2;
   heap[index2] = id1;
   // update index
   index[id1] = index2;
   index[id2] = index1; 
}

#endif

