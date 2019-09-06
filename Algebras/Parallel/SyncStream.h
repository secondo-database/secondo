/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
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


#pragma once

#include "Algebras/Stream/Stream.h"

template<class T>
class SyncStream{
 public:
  SyncStream(Word& w) : stream(w), endReached(true){}


  ~SyncStream(){
  } 

  void open(){
    boost::lock_guard<boost::mutex> guard(mtx);
    endReached = false;
    stream.open();
  }

  void close(){
    boost::lock_guard<boost::mutex> guard(mtx);
    endReached = true;
    stream.close();
  }

  T* request(){
    boost::lock_guard<boost::mutex> guard(mtx);
    T* res;
    if(endReached){
       res = 0;
    } else {
       res =  stream.request();
       if(res==0){
         endReached = true;
       }
    }
    return res;
  }

  void request(std::vector<T*>& buffer, int elems){
    boost::lock_guard<boost::mutex> guard(mtx);
    buffer.clear();
    if(endReached){
      buffer.push_back(nullptr);
      return;
    }
    for( int i=0;i<elems && !endReached; i++){
      T* res = stream.request(); 
      if(res==0){
         endReached = true;
      }
      buffer.push_back(res);
    }
  }


 private:
   Stream<T> stream;
   bool endReached;
   boost::mutex mtx;
}; 
