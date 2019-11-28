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


#include "IntSet.h"
#include "Attribute.h"
#include <set>
#include <iostream>
#include "NestedList.h"


extern NestedList* nl;


namespace collection{

IntSet::IntSet(const IntSet& src):Attribute(src),value(0),size(src.size){
  if(size>0){
     value = new int[size];
     memcpy((char*)value, src.value,size*sizeof(int));
  }
}

IntSet::IntSet(IntSet&& src): Attribute(src), 
   value(src.value), size(src.size) {
   src.size = 0;
   src.value = 0;
}

IntSet::IntSet(const std::set<int>& src):Attribute(true), value(0){
   setTo(src);
}


IntSet& IntSet::operator=(const IntSet& src) {
   Attribute::operator=(src);
   if(this->size != src.size){
      delete[] value;
      value = 0;
      this->size = src.size;
      if(size > 0){
        value = new int[size];  
      }
   }
   if(size > 0){
     memcpy((char*)value, src.value,size*sizeof(int));
   }
   return *this;
}

IntSet& IntSet::operator=(IntSet&& src) {
   Attribute::operator=(src);
   size = src.size;
   value = src.value;
   src.size = 0;
   src.value = 0;
   return *this;
}

int IntSet::Compare(const IntSet& arg) const {
   if(!IsDefined()){
      return arg.IsDefined()?-1:0;
   }
   if(!arg.IsDefined()){
      return 1;
   }
   if(size != arg.size){
     return size < arg.size ? -1 : 1;
   }
   for(size_t i=0;i<size;i++){
     if(value[i] < arg.value[i]) return -1;
     if(value[i] > arg.value[i]) return 1;
   }
   return 0;
}


size_t IntSet::HashValue() const {
  if(!IsDefined() ) return 0;
  if(value==nullptr) return 42;

  unsigned int step = size / 5;
  if(step<1) step = 1;
  size_t pos = 0;
  size_t res = 0;
  while(pos < size){
    res += value[pos];
    pos += step;
  }
  return res;
}

Word IntSet::In(const ListExpr typeInfo, const ListExpr le1,
                const int errorPos, ListExpr& errorInfo, bool& correct) {

   ListExpr le = le1; 
   Word res((void*)0);
   if(listutils::isSymbolUndefined(le)){
      res.addr = new IntSet(false);
      correct = true;
      return res;
   }
   if(nl->AtomType(le) != NoAtom){
     correct = false;
     return res;
   }
   std::set<int> tmp;
   while(!nl->IsEmpty(le)){
     ListExpr f = nl->First(le);
     le = nl->Rest(le);
     if(nl->AtomType(f)!=IntType){
       correct = false;
       return res;
     }
     tmp.insert(nl->IntValue(f));
   }
   IntSet* r = new IntSet(tmp);
   res.addr = r;
   correct = true; 
   return res;
}


ListExpr IntSet::Out(const ListExpr typeInfo, Word value) {
  IntSet* is = (IntSet*) value.addr;
  if(!is->IsDefined()){
     return listutils::getUndefined();
  }
  if(is->size == 0){
     return nl->TheEmptyList();
  }
  ListExpr res = nl->OneElemList( nl->IntAtom(is->value[0]));
  ListExpr last = res;
  for(unsigned int i=1;i<is->size;i++){
    last = nl->Append(last, nl->IntAtom(is->value[i]));
  }
  return res;
}


bool IntSet::Open( SmiRecord& valueRecord, size_t& offset, 
           const ListExpr typeInfo, Word& value){

  bool def;
  if(!valueRecord.Read(&def, sizeof(bool), offset)){
    return false;
  } 
  offset += sizeof(bool);
  if(!def){
     value.addr = new IntSet(false);
     return true;
  }

  size_t size;  
  if(!valueRecord.Read(&size, sizeof(size_t), offset)){
    return false;
  } 

  offset+=sizeof(size_t);
  if(size==0){
    value.addr = new IntSet(0,0);
    return true;
  } 
  int* v = new int[size];
  if(!valueRecord.Read(v,size*sizeof(int),offset)){
    return false;
  }
  value.addr = new IntSet(size,v);
  return true;
}


bool IntSet::Save(SmiRecord& valueRecord, size_t& offset,
          const ListExpr typeInfo, Word& value) {

   IntSet* is = (IntSet*) value.addr;
   bool def = is->IsDefined();
   if(!valueRecord.Write(&def, sizeof(bool), offset)){
     return false;
   }
   offset += sizeof(bool);
   if(!def){
     return true;
   }
   size_t size = is->size;
   if(!valueRecord.Write(&size, sizeof(size_t), offset)){
     return false;
   }
   offset += sizeof(size_t);
   if(is->size>0){
      if(!valueRecord.Write(is->value, sizeof(int) * is->size, offset)){
        return false;
      }
      offset += sizeof(int) * is->size;
   }
   return true;
}


std::ostream& IntSet::Print( std::ostream& os ) const {
    if(!IsDefined()){
       os << "undefined";
       return os;
    } 
    os << "{";
    for(size_t i=0;i<size;i++){
      if(i>0) os << ", ";
      os << value[i]; 
    }
    os << "}";
    return os;
}


void IntSet::Rebuild(char* buffer, size_t sz) {
   if(value!=nullptr){
     delete[] value;
     value = nullptr;
   }
   size = 0;
   bool def;
   size_t offset = 0;
   memcpy(&def,buffer + offset, sizeof(bool));
   offset += sizeof(bool);
   if(!def){
      SetDefined(false);
      return;
   }       
   SetDefined(true);
   memcpy(&size, buffer+offset, sizeof(size_t));
   offset += sizeof(size_t);
   if(size > 0){
     value = new int[size];
     memcpy(value, buffer+offset, size * sizeof(int));
     offset += size * sizeof(int);
   }
}


   void IntSet::setTo(const std::set<int>& src){
      clear();
      SetDefined(true);
      if(src.size()>0){
         size = src.size();
         value = new int[size];
         auto it = src.begin();
         size_t pos = 0;
         while(it!=src.end()) {
           value[pos] = *it;
           it++;
           pos++;
         }
      }
   }

   void IntSet::setTo(const std::vector<int>& src) {
      clear();
      SetDefined(true);
      if(src.size()>0){
         size = src.size();
         value = new int[size];
         auto it = src.begin();
         size_t pos = 0;
         while(it!=src.end()) {
           value[pos] = *it;
           it++;
           pos++;
         }
      }
   }

   IntSet IntSet::add(const IntSet& is) const{
     if(!IsDefined() || !is.IsDefined()){
         IntSet res(false);
         return res;
     }
     std::vector<int> v;
     size_t p1 = 0;
     size_t p2 = 0;
     while( p1 < size && p2 < is.size){
         if(value[p1] < is.value[p2]){
            v.push_back(value[p1]);
            p1++;
         } else if(value[p1] > is.value[p2]){
            v.push_back(is.value[p2]);
            p2++;
         } else {
            v.push_back(value[p1]);
            p1++;
            p2++; 
         }
     }
     while( p1 < size){
       v.push_back(value[p1]);
       p1++;
     }
     while( p2 < is.size){
       v.push_back(is.value[p2]);
       p2++;
     }
     IntSet res(true);
     res.setTo(v);
     return res; 
   }


   IntSet IntSet::minus(const IntSet& is) const{
     if(!IsDefined() || !is.IsDefined()){
         IntSet res(false);
         return res;
     }
     std::vector<int> v;
     size_t p1 = 0;
     size_t p2 = 0;
     while( p1 < size && p2 < is.size){
         if(value[p1] < is.value[p2]){
            v.push_back(value[p1]);
            p1++;
         } else if(value[p1] > is.value[p2]){
            p2++;
         } else {
            p1++;
            p2++; 
         }
     }
     while( p1 < size){
       v.push_back(value[p1]);
       p1++;
     }
     IntSet res(true);
     res.setTo(v);
     return res; 
   }
   
   IntSet IntSet::sdiff(const IntSet& is) const{
     if(!IsDefined() || !is.IsDefined()){
         IntSet res(false);
         return res;
     }
     std::vector<int> v;
     size_t p1 = 0;
     size_t p2 = 0;
     while( p1 < size && p2 < is.size){
         if(value[p1] < is.value[p2]){
            v.push_back(value[p1]);
            p1++;
         } else if(value[p1] > is.value[p2]){
            v.push_back(is.value[p2]);
            p2++;
         } else {
            p1++;
            p2++; 
         }
     }
     while( p1 < size){
       v.push_back(value[p1]);
       p1++;
     }
     while( p2 < is.size){
       v.push_back(is.value[p2]);
       p2++;
     }
     IntSet res(true);
     res.setTo(v);
     return res; 
   }



   IntSet IntSet::intersection(const IntSet& is) const {
     if(!IsDefined() || !is.IsDefined()){
         IntSet res(false);
         return res;
     }
     std::vector<int> v;
     size_t p1 = 0;
     size_t p2 = 0;
     while( p1 < size && p2 < is.size){
         if(value[p1] < is.value[p2]){
            p1++;
         } else if(value[p1] > is.value[p2]){
            p2++;
         } else {
            v.push_back(value[p1]);
            p1++;
            p2++; 
         }
     }
     IntSet res(true);
     res.setTo(v);
     return res; 
   }

   bool IntSet::contains( const int i) const {
     if(!IsDefined() || size==0){
       return false;
     }
     int min = 0;
     int max = size -1;

     while(max > min){
       int mid = (max + min) / 2;
       int mv = value[mid];
       if(mv == i){
         return true;
       }
       if(mv < i){
          min = mid + 1;
       } else {
          max = mid-1;
       }
     }
     return value[min] == i;

   }

} // end of namespace

std::ostream& operator<<(std::ostream& out, const collection::IntSet& is){
   return is.Print(out);
}




