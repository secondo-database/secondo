
/*
----
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

*/

#ifndef STREAM_H
#define STREAM_H

#include "QueryProcessor.h"
#include "NestedList.h"
#include "ListUtils.h"
#include <string>

extern NestedList* nl;
extern QueryProcessor* qp;


template<class T>
class Stream{
  public:

/*
Constructor

*/
     Stream(Word& s):source(s.addr),
                        opened(false),returned(0){ }

     Stream(void* s):source(s), opened(false){ }

     Stream(const Stream<T>& src): source(src.source),
                                   opened(src.opened){}

     ~Stream() {
      source = 0;
      opened = false;
     }

     Stream<T>& operator=(const Stream<T> src) {
        source = src.source;
        opened = src.opened;
        return *this;
     }


/*
Returns "stream"

*/
     static const std::string  BasicType(){
       return "stream";
     }

/*
Checks whether list is a description of a stream with given type.

*/
     static const bool checkType(const ListExpr type){
        if(!nl->HasLength(type,2)){
          return false;
        }
        if(!listutils::isSymbol(nl->First(type),BasicType())){
          return false;
        }
        return T::checkType(nl->Second(type));
     }

/*
Opens the stream. It's not allowed to open an already opened stream.

*/
    void open(){
       assert(!opened);
       qp->Open(source);
       opened = true;
    }

/*
Requests the next element from the stream. If no element is available,
the result is 0.

*/
    T* request() {
      Word res;
      qp->Request(source, res);
      if(!qp->Received(source)){
        return 0;
      } else {
         T* tres = static_cast<T*>(res.addr);
         returned++;
         return tres;
      }
    }

/*
Closes the stream.
The stream must be opened before.


*/
    void close(){
      assert(opened);
      qp->Close(source);
      opened=false;
    }


/*
Request for Progress.

*/
    bool requestProgress(ProgressInfo* p){
      return qp->RequestProgress(source, p);
    }


/*
Close Progress

*/
   void closeProgress(){
      qp->CloseProgress(source);
   }



   std::ostream& print(std::ostream& o){
      o << "stream[ source = " << (void*) source << ", opened = "
        << opened << ", qp = " << (void*) qp << "]";
      return o;
   }


   size_t getCount(){
      return returned;
   }


  private:
    Stream() {}
    void* source;
    bool opened;
    size_t returned;

};

#endif

