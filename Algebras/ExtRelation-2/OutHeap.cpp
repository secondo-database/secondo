/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

*/


#include "OutHeap.h"

  OutHeap::~OutHeap(){
       // merge heaps into a single one
       if(h2){
         while(!h2->empty()){
           std::pair<Tuple*, TupleFileIterator*>m  = *h2->min();
           h2->deleteMin();
           m.first->DeleteIfAllowed();
           delete m.second;
         }   
         delete h2; 
       }   
       if(lastFromFile){
           lastFromFile->DeleteIfAllowed();
       }   
       if(lastFromHeap){
          lastFromHeap->DeleteIfAllowed();
       }   
       if(nextRes){
          nextRes->DeleteIfAllowed();
       }
   }

  void OutHeap::init() {
    if(files.size()>0){
       h2 = new mmheap::mmheap<
                std::pair<Tuple*,TupleFileIterator*>, pc>(comp);
       for(size_t i=0;i<files.size(); i++){
          TupleFileIterator* it = files[i]->MakeScan();
          Tuple* first = it->GetNextTuple();
          if(first){
             h2->insert(std::make_pair(first,it));
          } else {
            delete it;
          }
       }
    } else {
      h2 = 0;
    }
    retrieveNextFromHeap();
    if(!h2){
          lastFromFile = 0;
    } else {
      retrieveNextFromFile();
    }
    retrieveNext();
 }

 void OutHeap::retrieveNext() {
     if(!lastFromFile){
        nextRes = lastFromHeap;
        retrieveNextFromHeap();
     } else if(!lastFromHeap){
         nextRes = lastFromFile;
         retrieveNextFromFile();
     }  else {
        if( comp(lastFromHeap, lastFromFile)){
          nextRes = lastFromHeap;
          retrieveNextFromHeap();
        } else {
         nextRes = lastFromFile;
         retrieveNextFromFile();
        }
     }
 }

 void OutHeap::retrieveNextFromFile(){
    if(h2->empty()){
      lastFromFile=0;
    } else {
       std::pair<Tuple*,TupleFileIterator*> m = *h2->min();
       lastFromFile = m.first;
       h2->deleteMin();
       Tuple* n = m.second->GetNextTuple();
        if(n){
           h2->insert(std::make_pair(n,m.second));
        } else {
           delete m.second;
        }
    }
 }

 void OutHeap::retrieveNextFromHeap(){
    if(heap && !heap->empty()){
      lastFromHeap = *heap->min();
      heap->deleteMin();
    } else {
      lastFromHeap=0;
    }
 }
   
