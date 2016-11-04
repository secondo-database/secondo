
/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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

#ifndef POINTERWRAPPER_H
#define POINTERWRAPPER_H

template<class T>
class PointerWrap{
 public:

  PointerWrap(): pointer(0){}

  PointerWrap(T* t):pointer(t){
     if(t){
        t->IncReference();
     }
  }

  PointerWrap(const PointerWrap& tw): pointer(tw.pointer){
    if(pointer){
      pointer->IncReference();
    }
  } 

  PointerWrap<T>& operator=(const PointerWrap<T>& rhs){
    if(pointer){
       pointer->DeleteIfAllowed();
    }
    this->pointer=rhs.pointer;
    if(pointer){
      pointer->IncReference();
    }
    return *this;
  }
 

  ~PointerWrap(){
     if(pointer){
       pointer->DeleteIfAllowed();
     }
  }

  T* getPointer() const{
     return pointer;
  }
  
  T* operator()() const{
     return pointer;
  }


  private:
    T* pointer;
};

#endif

