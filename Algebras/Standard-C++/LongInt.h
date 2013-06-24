/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

----


This class implements a longint type in Secondo.

*/

#ifndef LONGINT_H
#define LONGINT_H

#include "NestedList.h"
#include "Attribute.h"
#include "IndexableAttribute.h"
#include "ListUtils.h"
#include <limits>
#include "GenericTC.h"
#include <errno.h>
#include "SecondoSMI.h"

/*
1 Class Definition for ~LongInt~

*/
class LongInt: public IndexableAttribute{

public:

  typedef int64_t inttype;

  static SmiKey::KeyDataType getSMIKeyType(){
       return SmiKey::Longint;
  }

/*
1.1 Constructors

*/
   LongInt() {} 
   LongInt(int64_t v):IndexableAttribute(true),value(v){}
   LongInt(int32_t v):IndexableAttribute(true),value(v){}
   LongInt(bool defined, int64_t v):IndexableAttribute(defined),value(v){}
   LongInt(const LongInt& s): IndexableAttribute(s),value(s.value) {}
   LongInt(const CcInt& s):IndexableAttribute(s.IsDefined()),value(0) {
      if(IsDefined()){
        value = s.GetValue();
      }
   }
   LongInt(const string& s):IndexableAttribute(true), value(0){
       ReadFrom(s);
   }
   LongInt(const char* s):IndexableAttribute(true), value(0){
      ReadFrom(s);
   }
   explicit LongInt(bool defined): IndexableAttribute(defined), value(0) {}

   LongInt& operator=(const LongInt& s){
      SetDefined(s.IsDefined());
      value = s.value;
      return *this;
   }
/*
1.2 Destructor

*/
   ~LongInt() {}


/*
1.3 Getter and Setter

*/
   void SetValue(const int64_t v){
     SetDefined(true);
     value = v;
   }

   void SetValue(const CcInt& v){
     if(v.IsDefined()){
       SetDefined(true);
       value = v.GetValue();
     } else {
      SetDefined(false);
     }
      
   }

   int64_t GetValue() const{
     return value;
   }

  
   std::string ToString()const{
     if(!IsDefined()){
       return listutils::getUndefinedString();
     } else {
       return stringutils::any2str<int64_t>(value);
     }
   }   

   bool ReadFrom(std::string v){
      stringutils::trim(v);
      if(v.find_last_not_of("+-0123456789")!=std::string::npos){
         return false;
      } else if(listutils::isSymbolUndefined(v)){
         SetDefined(false);
         return true;
      } else {
         std::stringstream ss(v);
         ss >> value;
         SetDefined(true);
         return true;
      }

   }

   virtual void ReadFromString(const std::string& v){
     if(!ReadFrom(v)){
         SetDefined(false);
     }
   }


/*
1.4 Functions to act as an attribute

*/
int Compare(const Attribute* arg) const{
   if(IsDefined()!=arg->IsDefined()){
     return IsDefined()?-1:1;
   }
   LongInt* s = (LongInt*) arg;
   if(value != s->value){
     return value<s->value?-1:1;
   }
   return 0;
}

bool Adjacent(const Attribute *arg) const{
   if(!IsDefined() || !arg->IsDefined()){
     return false;
   } 
   LongInt* s = (LongInt*) arg;
   return (value == s->value+1)  || (value == s->value-1);
}

size_t Sizeof() const{
   return sizeof(*this);
}


size_t HashValue() const{
   if(!IsDefined()){
      return 0;
   }
   return (size_t) value;
}

void CopyFrom(const Attribute* arg){
  SetDefined(arg->IsDefined());
  value = ((LongInt*)arg)->value;    
}


LongInt* Clone() const{
  return new LongInt(*this);
}


void WriteTo( char *dest ) const{
   strcpy( dest, ToString().c_str() );
}

void ReadFrom( const char *src ){
   ReadFromString(string(src));
}

SmiSize SizeOfChars() const{
   return ToString().length()+1;

}

ostream& Print(ostream &os) const{
  return os << ToString();
}
/*
1.4 Some operators

1.4.1 Arithmetic operators

*/


 inline void Inc(){
    value++;
 }
 inline void Dec(){
    value--;
 } 

/*
Addition

*/

 LongInt operator+(const LongInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value += i.value;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

 LongInt operator+(const CcInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value += i.GetValue();
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

 LongInt operator+(const int64_t i)const{
   LongInt res(*this);
   if(!IsDefined() ){
     res.SetDefined(false);
     return res;
   }
   res.value += i;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

/*
Internal Addition

*/



 LongInt& operator+=(const LongInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value += i.value;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator+=(const CcInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value += i.GetValue();
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator+=(const int64_t i){
   if(IsDefined()){
      value += i;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

/*
Subtraction

*/


 LongInt operator-(const LongInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value -= i.value;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }
 LongInt operator-(const CcInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value -= i.GetValue();
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

 LongInt operator-(const int64_t i)const{
   LongInt res(*this);
   if(!IsDefined() ){
     res.SetDefined(false);
     return res;
   }
   res.value -= i;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

/*
Internal Subtraction

*/

 LongInt& operator-=(const LongInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value -= i.value;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator-=(const CcInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value -= i.GetValue();
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator-=(const int64_t i){
   if(IsDefined() ){
      value -= i;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

/*
Multiplication

*/

 LongInt operator*(const LongInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value *= i.value;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }
 LongInt operator*(const CcInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value *= i.GetValue();
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }
 
 LongInt operator*(const int64_t i)const{
   LongInt res(*this);
   if(!IsDefined() ){
     res.SetDefined(false);
     return res;
   }
   res.value *= i;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

/*
Internal Muliplication

*/

 LongInt& operator*=(const LongInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value *= i.value;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator*=(const CcInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value *= i.GetValue();
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator*=(const int64_t i){
   if(IsDefined() ){
      value *= i;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }
/*
Division

*/

 LongInt operator/(const LongInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value /= i.value;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }
 LongInt operator/(const CcInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value /= i.GetValue();
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

 LongInt operator/(const int64_t i)const{
   LongInt res(*this);
   if(!IsDefined() ){
     res.SetDefined(false);
     return res;
   }
   res.value /= i;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

/*
Internal Division

*/


 LongInt& operator/=(const LongInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value /= i.value;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator/=(const CcInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value /= i.GetValue();
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator/=(const int64_t i){
   if(IsDefined() ){
      value /= i;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }
/*
Modulo function

*/
 LongInt operator%(const LongInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value %= i.value;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

 LongInt operator%(const CcInt& i)const{
   LongInt res(*this);
   if(!IsDefined() || !i.IsDefined()){
     res.SetDefined(false);
     return res;
   }
   res.value %= i.GetValue();
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

 LongInt operator%(const int64_t i)const{
   LongInt res(*this);
   if(!IsDefined() ){
     res.SetDefined(false);
     return res;
   }
   res.value %= i;
   if(errno){
     res.SetDefined(false);
   }
   return res;
 }

/*
Internal Modulo function

*/

 LongInt& operator%=(const LongInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value %= i.value;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

 LongInt& operator%=(const CcInt& i){
   if(!IsDefined() || !i.IsDefined()){
      SetDefined(false);
   } else {
      value %= i.GetValue();
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }
 
 LongInt& operator%=(const int64_t i){
   if(IsDefined()){
      value %= i;
      if(errno){
         SetDefined(false);
      }
   }
   return *this;
 }

/*
Increment, Decrement

*/
 LongInt& operator++(){
   value++;
   if(errno){
     SetDefined(false);
   }
   return *this;
 }

LongInt operator++(int){
  LongInt res(*this);
  ++(*this);
  return res;
}

 LongInt& operator--(){
   value--;
   if(errno){
     SetDefined(false);
   }
   return *this;
 }

LongInt operator--(int){
  LongInt res(*this);
  --(*this);
  return res;
}

/*
1.4.2 Comparisons

*/
bool operator==(const LongInt& i)const{
   if(!IsDefined() || !i.IsDefined()){
      return IsDefined() == i.IsDefined();
   }
   return value == i.value;
}

bool operator==(const CcInt& i)const{
   if(!IsDefined() || !i.IsDefined()){
      return IsDefined() == i.IsDefined();
   }
   return value == i.GetValue();
}

bool operator!=(const LongInt& i)const{
   if(!IsDefined() || !i.IsDefined()){
      return IsDefined() != i.IsDefined();
   }
   return value != i.value;
}

bool operator!=(const CcInt& i)const{
   if(!IsDefined() || !i.IsDefined()){
      return IsDefined() != i.IsDefined();
   }
   return value != i.GetValue();
}


bool operator<(const LongInt& i)const{
   if(!IsDefined()){
      return i.IsDefined();
   }
   if(!i.IsDefined()){
      return false;
   }
   return value < i.value;
}

bool operator<(const CcInt& i)const{
   if(!IsDefined()){
      return i.IsDefined();
   }
   if(!i.IsDefined()){
      return false;
   }
   return value < i.GetValue();
}

bool operator<=(const LongInt& i)const{
   if(!IsDefined()){
      return true;
   }
   if(!i.IsDefined()){
      return false;
   }
   return value <= i.value;
}

bool operator<=(const CcInt& i)const{
   if(!IsDefined()){
      return true;
   }
   if(!i.IsDefined()){
      return false;
   }
   return value <= i.GetValue();
}


bool operator>(const LongInt& i)const{
   if(!IsDefined()){
      return false;
   }
   if(!i.IsDefined()){
      return true;
   }
   return value > i.value;
}

bool operator>(const CcInt& i)const{
   if(!IsDefined()){
      return false;
   }
   if(!i.IsDefined()){
      return true;
   }
   return value > i.GetValue();
}

bool operator>=(const LongInt& i)const{
   if(!IsDefined()){
      return !i.IsDefined();
   }
   if(!i.IsDefined()){
      return true;
   }
   return value >= i.value;
}

bool operator>=(const CcInt& i)const{
   if(!IsDefined()){
      return !i.IsDefined();
   }
   if(!i.IsDefined()){
      return true;
   }
   return value >= i.GetValue();
}

/*
1.5 Conversion between lists and Class

*/
ListExpr ToListExpr(ListExpr typeinfo){
   if((value<numeric_limits<int32_t>::max()) && 
      (value>numeric_limits<int32_t>::min())){
     return nl->IntAtom( (int)value);
   }  else {
     int32_t v1 = (int)(value>>32);
     int32_t v2 = (int)(value&0xFFFFFFFF);
     ListExpr res =  nl->TwoElemList(
                         nl->IntAtom(v1),
                         nl->IntAtom(v2));
      return res;
   }
}

bool ReadFrom(const ListExpr LE, const ListExpr typeInfo){

  if(listutils::isSymbolUndefined(LE)){
      SetDefined(false);
      return true;
  }
  if(nl->AtomType(LE)==IntType){
     SetDefined(true);
     value = nl->IntValue(LE);
     return true;
  }
  if(nl->AtomType(LE)==StringType){
    return ReadFrom(nl->StringValue(LE));
  }
  if(!nl->HasLength(LE,2)){
    return false;
  }
  ListExpr f = nl->First(LE);
  if(nl->AtomType(f)!=IntType){
     return false;
  }
  ListExpr s = nl->Second(LE);
  if(nl->AtomType(s)!=IntType){
     return false;
  }
  SetDefined(true);
  int64_t v1 = nl->IntValue(f);
  uint32_t v2 = nl->IntValue(s);
  value = v1<<32 | v2; 
  return true;
}

int CompareTo(const LongInt* s){
   if(IsDefined()!=s->IsDefined()){
     return IsDefined()?-1:1;
   }
   if(value != s->value){
     return value<s->value?-1:1;
   }
   return 0;
}


static bool CheckKind(ListExpr type, ListExpr& errorInfo){
   return nl->IsEqual(type,BasicType());
}    

static const string BasicType(){
      return "longint";
}    

static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
}    

static ListExpr Property(){
       return gentc::GenProperty("-> DATA", 
                          BasicType(),
                          "(int int)",
                          "(98 64)");
     }    

private:
   int64_t value;
};




#endif
