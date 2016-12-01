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


#include "MainMemoryExt.h"
#include <vector>
#include "Attribute.h"


namespace mm2algebra{

class MemoryVectorObject : public MemoryObject {

    public:

        MemoryVectorObject(bool _flob, const std::string& db,
                           const std::string& _type ):
         MemoryObject(_flob, db, _type) {
           v = 0;
           sorted = false;
           flob = _flob;
        }


        ~MemoryVectorObject() {
          if(v){
            for(size_t i=0;i<v->size();i++){
              v->at(i)->DeleteIfAllowed();
            }
            delete v;
          }
        }

        void add(Attribute* a) {
            if(!v) {
              v = new std::vector<Attribute*>();
            }
            if(flob){
             a->bringToMemory();
            }
            v->push_back(a->Copy());
            memSize += a->GetMemSize() + sizeof(void*); 
            sorted = false;
        }
        
        Attribute* get(const int i) {
          if((i<0) || i>=(int)v->size()){
             return 0;
          }
          return v->at(i);
        }
        
        static const std::string BasicType() { return "mvector"; }

        void sort();

        bool isSorted() const{
          return sorted;
        }

        bool checkSorted();

        int binSearch(const Attribute* attr) const;

        size_t size() const{
          return v->size();
        }


    private:
        std::vector<Attribute*>* v;
        bool sorted;

};


}

