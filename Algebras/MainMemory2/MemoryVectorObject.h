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


#ifndef MEMORYVECTOROBJECT_H
#define MEMORYVECTOROBJECT_H

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

        void add(Attribute* a, bool checkSort) {
            if(!v) {
              v = new std::vector<Attribute*>();
            }
            if(flob){
             a->bringToMemory();
            }
            v->push_back(a->Copy());
            memSize += a->GetMemSize() + sizeof(void*); 
            if(!checkSort){
              sorted= false;
              return;
            }

            if(v->size()==1){
              sorted= true;
              return;
            }
            if(!sorted || v->size()==1){
              return;
            } 
            int cmp = v->at(v->size()-2)->Compare(v->at(v->size()-1));
            if(cmp>0){
              sorted = false;
            }
        }


        
        Attribute* get(const int i) const {
          if(!v){
            return 0;
          }
          if((i<0) || i>=(int)v->size()){
             return 0;
          }
          return v->at(i);
        }

        Attribute* put(const int i, Attribute* nv, bool checkSorted){
          if(!v) return 0;
          if((i<0) || i>=(int)v->size()){
             return 0;
          }
          Attribute* res = v->at(i);
          if(flob){
             nv->bringToMemory();
          }
          v->at(i) = nv->Copy();
          if(!checkSorted){
             sorted = false;
             return res;
          } 
          if(!sorted || v->size()<2){
             return res;
          }
          if(i>0){
            int cmp = v->at(i-1)->Compare(v->at(i));
            if(cmp>0){
              sorted = false;
              return res;
            } 
          }
          if((size_t)i<(v->size()-1)){
            int cmp = v->at(i)->Compare(v->at(i+1));
            if(cmp>0){
              sorted = false;
              return res;
            } 
          }

          return res;
        }

        
        static const std::string BasicType() { return "mvector"; }

        static bool checkType(ListExpr t){
          return nl->HasLength(t,2) 
                 && listutils::isSymbol(nl->First(t),BasicType());
        }

        void sort();

        bool isSorted() const{
          return sorted;
        }

        bool checkSorted();

        int binSearch(const Attribute* attr) const;

        size_t size() const{
          return v->size();
        }

        Attribute* matchBelow(Attribute* attr) const{
          if(!sorted) return 0;
          if(v->size()<1){
            return 0;
          }
          int pos = binSearch(attr);
          if((size_t)pos==v->size()){ // after last pos
            return v->at(pos-1);
          }
          int cmp = attr->Compare(v->at(pos));
          if(cmp==0){
            return v->at(pos);
          }
          return pos==0?0:v->at(pos-1);
        }


    private:
        std::vector<Attribute*>* v;
        bool sorted;

};


}
#endif


