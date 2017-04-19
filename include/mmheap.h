/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, 
Faculty of Mathematics and of Computer Science,
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

#ifndef MMHEAP_H
#define MMHEAP_H

#include <vector>
#include <stdlib.h>
#include <iostream>

namespace mmheap{


template<class T, class C >
class mmheap{

 public:
  mmheap( C _comp): content(), cmp(_comp), bulkload(false){}
  mmheap(): content(), cmp(),bulkload(false){}

  void insert(const T& elem){
     content.push_back(elem);
     if(!bulkload){
       bubbleUp(content.size());
     }
  }
  
  void push(const T& elem){
     content.push_back(elem);
     bubbleUp(content.size());
  }

  void pop(){
     if(content.size() <= 1){
        content.clear();
        return;
     }
     content[0] = content[content.size()-1]; // bring last element to front
     content.pop_back(); // remove last element
     sinkBU(); 
  }
   

  void deleteMin(){
     assert(!bulkload);
     if(content.size() <= 1){
        content.clear();
        return;
     }
     content[0] = content[content.size()-1]; // bring last element to front
     content.pop_back(); // remove last element
     sinkBU(); 
  }

  inline const T* min() const{
    assert(!bulkload);
    if(content.empty()){
      return 0;
    }
    return &content[0];
  }
  
  inline const T& top() const{
    return content[0];
  }

  inline size_t size() const{
     return content.size();
  }

  inline bool empty(){
     return content.empty();
  }

  std::ostream& print(std::ostream& o){
    o << "[";
    for(size_t i=0;i<content.size();i++){
       if(i>0) o << ", ";
       o << content[i];
    }
    o << "]";
    return o;
  }

  void startBulkload(){
    assert(!bulkload);
    bulkload = true;
  }

  void endBulkload(){
    assert(bulkload);
    for(size_t i = content.size()/2 ; i > 0; i--){
      sink(i);
    }
    bulkload = false;
  }

  inline void clear(){
    content.clear();
  }

  void swap(mmheap<T,C>& rhs){
     std::swap(content,rhs.content);
     std::swap(cmp,rhs.cmp);
     std::swap(bulkload,rhs.bulkload);
  }



private:
  std::vector<T> content;
  C cmp;
  bool bulkload; 

  void bubbleUp( size_t pos){
    size_t father;
    while(pos > 1){
       father = pos / 2;
       if(cmp(content[pos-1], content[father-1])){
          // current element is smaller
          std::swap(content[pos-1], content[father-1]);
          pos = father;
       } else { // final position reached
          return;
       }
    }
  }

  void sink(const size_t p1){ // p1 counting starts at 1
     size_t pos = p1;
     while(pos*2 <= content.size()){ // at least one son exists
        size_t son1 = pos*2;
        size_t son2 = son1 + 1;
        if(son2 <= content.size()){
           if(cmp(content[son2-1],content[son1-1])){
              // son2 is the smaller one
              son1 = son2;
           }
        }        
        if(cmp(content[son1-1],content[pos-1])){
            std::swap(content[son1-1],content[pos-1]);
            pos = son1;
        } else {
            return;
        }
     }
  }

  void sinkBU(const size_t p1 = 0){
    size_t pos = p1 + 1;
    while(pos*2 <= content.size()){
      size_t son1 = pos*2;
      size_t son2 = son1 + 1;
      if(son2 <= content.size()){
         if(cmp(content[son2-1],content[son1-1])){
            // son2 is the smaller one
            son1 = son2;
         }
      }        
      std::swap(content[son1-1],content[pos-1]);
      pos = son1;
    }
    bubbleUp(pos);
  }
};


} // end of namepsace mmheap;



#endif


