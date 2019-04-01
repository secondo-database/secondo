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


/*
5.20 Type ~mem~

This type encapsulates just a string. 

*/

#ifndef MEM_H
#define MEM_H

#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "GenericTC.h"

namespace mm2algebra{

class Mem: public Attribute{
  public:
     Mem(){}
     Mem(int i) : Attribute(false) {
        strcpy(value,"");
     }

     Mem(const Mem& src): Attribute(src){
        strcpy(value, src.value);
     }

     Mem& operator=(const Mem& src){
        Attribute::operator=(src);
        strcpy(value, src.value);
        return *this;
     }
     
     ~Mem(){}

      void set(const bool def, const std::string& v){
         SetDefined(def);
         if(def){
            strcpy(value,v.c_str());
         }
      }

      std::string GetValue() const{
        assert(IsDefined());
        return std::string(value);
      }

     
     static const std::string BasicType(){ return "mem"; }

     static const bool checkType(ListExpr type){
        if(!nl->HasLength(type,2)){
           return false;
        }
        if(!listutils::isSymbol(nl->First(type), BasicType())){
          return false;
        }
        // TODO: check whether the second element is a valid type
        return true;
     }  

     static ListExpr wrapType(ListExpr type){
        return nl->TwoElemList(nl->SymbolAtom(BasicType()),
                               type);
     }


     inline virtual int NumOfFLOBs() const{
        return 0;
     }
     inline virtual Flob* GetFLOB(const int i){
        assert(false);
        return 0;
     }

     int Compare(const Attribute* arg) const{
        if(!IsDefined() && !arg->IsDefined()){
          return 0;
        }
        if(!IsDefined()){
          return -1;
        }
        if(!arg->IsDefined()){
          return 1;
        }
        return strcmp(value,((Mem*)arg)->value);
     }

     bool Adjacent(const Attribute* arg) const{
        return false;
     }
     size_t Sizeof() const{
        return sizeof(*this);
     }

     size_t HashValue() const{
        if(!IsDefined()){
           return 0;
        }
        size_t m = 5;
        size_t l = std::min(m, strlen(value));
        int sum = 0;
        for(size_t i=0;i<l;i++){
          sum = sum*10 + value[i];
        }
        return sum;
     }

     void CopyFrom(const Attribute* arg){
        *this = *((Mem*) arg);
     }

     Attribute* Clone() const{
       return new Mem(*this);
     }

     


     static ListExpr Property() {
          return gentc::GenProperty( "-> DATA",
                                    "("+ BasicType() + " <subtype> )",
                                    "<string>",
                                    "\"testobj\"");
                                    
     }
     static bool CheckKind(ListExpr type, ListExpr& errorInfo){
        return checkType(type);
     }

     bool ReadFrom(ListExpr le, const ListExpr typeInfo){
        if(listutils::isSymbolUndefined(le)){
          SetDefined(false);
          return true;
        }         
        if(nl->AtomType(le)!=StringType){
           return false;
        }
        set(true, nl->StringValue(le));
        return true; 
     }

     ListExpr ToListExpr( const ListExpr typeInfo){
       if(!IsDefined()){
         return listutils::getUndefined();
       }
       return nl->StringAtom(value);
     }

      
   static bool requiresTypeCheckInVM(){
     return true;
   }
     
  private:
     STRING_T value;

};

} // end of namespace mm2algebra


#endif


