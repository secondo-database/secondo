
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

ToDo: 
  Remove dependency to distance computation based on available 
  distance function.

*/

template <class T>
class OrderSeeds{

  public:
    OrderSeeds( TupleBuffer* objects, int reachPos, int  _corePos, 
                int _procPos, int _attrPos, double _UNDEFINED) ;

    void update(list<TupleId>& neighbors, TupleId objId);

    TupleId next();

    bool empty();

  private:
     TupleBuffer* objs;
     int reachPos; // attribute position for reachability distance
     int corePos;  // attribute position for core distance
     int procPos;  // attribute index for processed flag
     int attrPos;  // index of attribute to cluster 
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
     double distance(T* v1, T* v2);

     // for dbugging only
     bool checkStructureDebug();
};

/*
1.1 Constructor

*/

template <class T>
OrderSeeds<T>::OrderSeeds(TupleBuffer* _objs, int _reachPos, int _corePos, 
                          int _procPos, int _attrPos, double _UNDEFINED):
    objs(_objs), reachPos(_reachPos), corePos(_corePos), procPos(_procPos), 
    attrPos(_attrPos), UNDEFINED(_UNDEFINED) {
     firstError= true;
 }

/*
1.2 check for emptyness

*/
template <class T>
  bool OrderSeeds<T>::empty(){
     return heap.empty();
  }

/*
1.3 returns the next (minimum) element

*/
template <class T>
TupleId OrderSeeds<T>::next(){
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
template <class T>
  void OrderSeeds<T>::update(list<TupleId>& neighbors, TupleId objId){
      Tuple* obj = objs->GetTuple(objId, true);
      double c_dist = ((CcReal*)obj->GetAttribute(corePos))->GetValue();
      list<TupleId>::iterator it;
      for(it = neighbors.begin(); it!=neighbors.end(); it++){
         TupleId cId = *it;
         Tuple* cur = objs->GetTuple(cId,true);
         bool processed = ((CcBool*)cur->GetAttribute(procPos))->GetValue();
         if(!processed){
            double new_r_dist = max(c_dist, distance(obj, cur));
            double old_r_dist = 
                  ((CcReal*)cur->GetAttribute(reachPos))->GetValue();
            if(old_r_dist == UNDEFINED){
               cur->PutAttribute(reachPos, new CcReal(new_r_dist));
               insert(cId, new_r_dist);
            } else {
               if(new_r_dist < old_r_dist){
                  cur->PutAttribute(reachPos, new CcReal(new_r_dist));
                  decrease(cId, new_r_dist);
               }
            }
         } 
         cur->DeleteIfAllowed();
      }
      obj->DeleteIfAllowed();
  }


/*
1.5 Distance

Returns the distance between two attributes.

*/
template <class T>
double OrderSeeds<T>::distance(T* v1, T* v2){
   return v1->Distance(*v2);
}


/*
1.6 Distance

Returns te distance between two tuples according to the cluster
attribute.

*/
template <class T>
double OrderSeeds<T>::distance(Tuple* t1, Tuple* t2){
   T* a1 = (T*) t1->GetAttribute(attrPos);
   T* a2 = (T*) t2->GetAttribute(attrPos);
   return distance(a1,a2); 
}

/*
1.7 DeletMin

Removes the minimum element.

*/
template<class T>
void OrderSeeds<T>::deleteMin(){
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
template<class T>
double OrderSeeds<T>::getReachDist(int index){
   assert(index >=0);
   assert(index < (int)heap.size());
   TupleId id = heap[index];
   Tuple* t = objs->GetTuple(id , true);
   double res = ((CcReal*)t->GetAttribute(reachPos))->GetValue();
   t->DeleteIfAllowed();
   return res;
}


/*
1.9 sink

let the element at position index sink into the heap unil
it is a leaf or all sons are greater/equal than the 
element itselfs. 

*/
template<class T>
void OrderSeeds<T>::sink(int index){
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
template<class T>
void OrderSeeds<T>::insert(TupleId id, double dist){
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

template<class T>
void OrderSeeds<T>::decrease(TupleId id, double newDist){
   Tuple* tuple = objs->GetTuple(id,true);
   double oldDist = ((CcReal*)tuple->GetAttribute(reachPos))->GetValue();
   if(newDist > oldDist){
     cout << "decrease increases distance" << endl;
     cout << "olddist = " << oldDist << endl;
     cout << "newDist = " << newDist << endl;
     assert(oldDist > newDist);
   }
   tuple->PutAttribute(reachPos, new CcReal(newDist));
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

template<class T>
bool OrderSeeds<T>::checkStructureDebug(){

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


template<class T>
void OrderSeeds<T>::swap(int index1, int index2){
   // swap
   TupleId id1 = heap[index1];
   TupleId id2 = heap[index2];
   heap[index1] = id2;
   heap[index2] = id1;
   // update index
   index[id1] = index2;
   index[id2] = index1; 
}







