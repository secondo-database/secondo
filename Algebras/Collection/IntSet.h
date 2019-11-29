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

#include <set>
#include <string>
#include <iostream>
#include <assert.h>

#include "Attribute.h" 
#include "GenericTC.h"

/*
Class ~IntSet~


This class represents a set of integer values.  


*/




namespace collection{

class IntSet: public Attribute{
  public:
     IntSet() {}  // use in cast only
     IntSet(const bool def): Attribute(def), value(0),size(0){}
     IntSet(const IntSet& src);
     IntSet(IntSet&& src);
     IntSet(const std::set<int>& src);

     IntSet& operator=(const IntSet& src);
     IntSet& operator=(IntSet&& src);
     ~IntSet(){
       if(value){ delete[] value; }
     }


    // auxiliary functions
    static const std::string BasicType(){ return "intset"; }
    static const bool checkType(ListExpr type){
      return listutils::isSymbol(type,BasicType());
    }
    // attribute related functions
    inline int NumOfFLOBs() const{
      return 0;
    }
    inline virtual Flob* GetFLOB(const int i){
       return 0;
    }
    int Compare(const IntSet&  arg)const;

    int Compare(const Attribute* arg)const{
      return Compare( * ((IntSet*)arg));
    }

    bool Adjacent(const Attribute* arg) const{
      return false;
    }
    size_t Sizeof() const{
       return sizeof(*this);
    }
    size_t HashValue() const;
  
    std::ostream& Print( std::ostream& os ) const;

    void CopyFrom(const Attribute* arg){
       *this = *((IntSet*) arg);
    }   
    Attribute* Clone() const{
       return new IntSet(*this);
    }
    // functions supporting the embedding into secondo
    static ListExpr Property(){
        return gentc::GenProperty( " -> DATA",
                                   BasicType(),
                                   "(int int int ...)",
                                   "(6 23 58)");
    }
    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
        return checkType(type);
    }

    static Word In(const ListExpr typeInfo, const ListExpr le,
                const int errorPos, ListExpr& errorInfo, bool& correct);


    static ListExpr Out(const ListExpr typeInfo, Word value);

    static Word Create(const ListExpr typeInfo){
       Word res( new IntSet(false));
       return res;
    }   

    static void Delete(const ListExpr typeInfo, Word& v){
       delete (IntSet*) v.addr;
       v.addr = 0;
    }

    static bool Open( SmiRecord& valueRecord, size_t& offset, 
                       const ListExpr typeInfo, Word& value);

   static bool Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);


   static void Close(const ListExpr typeInfo, Word& w){
     delete (IntSet*) w.addr;
     w.addr = 0;
   }
   static Word Clone(const ListExpr typeInfo, const Word& w){
      IntSet* v = (IntSet*) w.addr;
      Word res;
      res.addr = new IntSet(*v);
      return res;
   }

   static int Size() {
        return 256; // ??
   }

   static void* Cast(void* addr){
       return addr;
   }

   static bool TypeCheck(ListExpr type, ListExpr& errorInfo){
      return checkType(type);
   }

   inline virtual StorageType GetStorageType() const{
      return Extension;
   }

   inline virtual size_t SerializedSize() const{
       if(!IsDefined()) return sizeof(bool);


       size_t res =  sizeof(bool) + sizeof(size_t) + size * sizeof(int); 
       return res;
   }
    
  inline virtual void Serialize(char* buffer, size_t sz, 
                                 size_t offset) const{
      bool def = IsDefined();
      memcpy(buffer+offset, &def, sizeof(bool));
      offset += sizeof(bool);
      if(!def){
        return;
      }
      memcpy(buffer+offset, &size, sizeof(size_t));
      offset += sizeof(size_t);
      if(size > 0){
        memcpy(buffer+offset, value, size*sizeof(int));
        offset += size*sizeof(int);
      }
   }

   inline virtual void Rebuild(char* buffer, size_t sz);

   virtual size_t GetMemSize() {
     return sizeof(*this) + size * sizeof(int);
   }

   void clear(){
     size = 0;
     if(value!=nullptr){
        delete[] value;
        value = nullptr;
     }
   }

   void setTo(const std::set<int>& src);

/*
~add~

Computes the usion of both sets.

*/
   IntSet add(const IntSet& is) const;

/*
~minus~

Computes the set difference.

*/
   IntSet minus(const IntSet& is) const;

/*
~intersection~

speaks for itself

*/
   IntSet intersection(const IntSet& is) const;

/*
~sdiff~

Symmetrioc difference.

*/
   IntSet sdiff(const IntSet& is) const;

/*
~minCommon~

Computes the minimum common element of both sets.
If such an object does not exist. The return value will be
false. Otherwise the value of the minimum object
os returned in parameter result. Of course, this operator
contains also an intersects predicate.

*/
   bool minCommon(const IntSet& is, int& result) const;


   bool contains( int i) const;

   size_t getSize() const{ return size; }

   inline int get(const size_t  i) const{
      assert(i<size);
      return value[i];
   }
 

  private:
     int* value;   
     size_t size;

     IntSet(size_t size, int* value): Attribute(true), 
            value(value),size(size) {}

     void setTo(const std::vector<int>& sortedVector);

};


}

std::ostream& operator<<(std::ostream& os, const collection::IntSet& is);


