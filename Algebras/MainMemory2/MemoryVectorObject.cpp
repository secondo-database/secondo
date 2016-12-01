/*

----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
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


#include "MemoryVectorObject.h"
#include <vector>

namespace mm2algebra{

bool MemoryVectorObject::checkSorted() {

   sorted = true;
   if(v->size() < 2u){
     return true;
   }
   for(size_t i=0;i<v->size()-1; i++){
      if(v->at(i)->Compare((*v)[i+1]) > 0){
         sorted = false;
         return false;
      }
   }
   return true;
}

void reheap(std::vector<Attribute*>& heap,int i, int k){ 
   int j,son;
   j=i;
   bool done = false;
   while(2*j<=k && !done){
      if(2*j+1<=k){
         if(heap[2*j-1]->Compare(heap[2*j])>0){
            son = 2*j;
         } else {
            son = 2*j+1;
         }   
      } else {
         son = 2*j;
      }   
      if(heap[j-1]->Compare(heap[son-1])<0){
         Attribute* tmp = heap[j-1];
         heap[j-1] = heap[son-1];
         heap[son-1] = tmp;
         j = son;
      } else {
         done=true;
      }   
   }   
}

void heapSort(std::vector<Attribute*>& v){
  size_t n = v.size();
  for(size_t i = n/2; i>0 ; i--){
    reheap(v,i,n);
  }
  for(size_t i=n;i>1;i--){
     Attribute* tmp = v[0];
     v[0] = v[i-1];
     v[i-1] = tmp;
     reheap(v,1,i-1);
  }
} 


void MemoryVectorObject::sort(){
   if(sorted) return;
   heapSort(*v);
   sorted=true; 
} 

int MemoryVectorObject::binSearch(const Attribute* a) const{
   if(!sorted) return -1;
   if(v->empty()){
     return 0;
   }
   size_t min = 0;
   size_t max = v->size()-1;
   while(min<max){
     size_t mid = (min + max)/2;
     int c = v->at(mid)->Compare(a);
     if(c==0) return mid; // found element
     if(c<0){
       min = mid + 1;
     } else {
       max = mid - 1;
     }
   }
   // now holds min == max
   int c = v->at(min)->Compare(a);
   return c < 0 ? min+1:min;
}







}


