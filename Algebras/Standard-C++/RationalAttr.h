
/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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


This class implements a rational type in Secondo.

*/

#ifndef RATIONAL_H
#define RATIONAL_H

#include "NestedList.h"
#include "Attribute.h"
#include "IndexableAttribute.h"
#include "ListUtils.h"
#include <limits>
#include "GenericTC.h"
#include <errno.h>
#include "LongInt.h"

/*
Class Rational.

*/

class Rational : public Attribute {
 
  public:
    Rational() {}
    explicit Rational(bool defined): 
         Attribute(defined),negative(0),nominator(0), denominator(1){}

    Rational(const Rational& r):
        Attribute(r.IsDefined()),
        negative(r.negative),
        nominator(r.nominator),
        denominator(r.denominator){}

    Rational(const uint64_t v):Attribute(true),
             negative(0), nominator(v), denominator(1) {}

    Rational(const uint32_t v): Attribute(true),
          negative(0), nominator(v), denominator(1){}


    Rational(const int64_t v):Attribute(true),
             negative(v<0?1:0), nominator(abs(v)), denominator(1) {}


    Rational(const int32_t v):Attribute(true),
             negative(v<0?1:0), nominator(abs(v)), denominator(1) {}

    Rational(const CcInt& v): Attribute(v.IsDefined()){
        if(!IsDefined()){
            nominator = 0;
            denominator = 1;
            negative = 0;
        } else {
           int64_t k = v.GetValue();
           negative = k<0?1:0;
           nominator = abs(k);
           denominator = 1;
        }
    }
    Rational(const LongInt& v): Attribute(v.IsDefined()){
        if(!IsDefined()){
            nominator = 0;
            denominator = 1;
            negative = 0;
        } else {
           int64_t k = v.GetValue();
           negative = k<0?1:0;
           nominator = abs(k);
           denominator = 1;
        }
    }

    Rational(const string& v) : Attribute(false){
       ReadFromString(v);
    }

    Rational(const CcString& v) : Attribute(v.IsDefined()){
       if(IsDefined()){
          ReadFromString(v.GetValue());
       } else {
          nominator = 0;
         denominator = 1;
         negative = 0;
       }
    }

    Rational(const CcInt& n, const CcInt& d): Attribute(true){
       if(!n.IsDefined() || !d.IsDefined()){
          nominator = 0;
          denominator = 1;
          negative = 0;
          SetDefined(false);
          return;
       }
       readFrom(n.GetValue(), d.GetValue());
    }

    Rational(const CcInt& n, const LongInt& d): Attribute(true){
       if(!n.IsDefined() || !d.IsDefined()){
          nominator = 0;
          denominator = 1;
          negative = 0;
          SetDefined(false);
          return;
       }
       readFrom(n.GetValue(), d.GetValue());
    }
    
    Rational(const LongInt& n, const CcInt& d): Attribute(true){
       if(!n.IsDefined() || !d.IsDefined()){
          nominator = 0;
          denominator = 1;
          negative = 0;
          SetDefined(false);
          return;
       }
       readFrom(n.GetValue(), d.GetValue());
    }

    Rational(const LongInt& n, const LongInt& d): Attribute(true){
       if(!n.IsDefined() || !d.IsDefined()){
          nominator = 0;
          denominator = 1;
          negative = 0;
          SetDefined(false);
          return;
       }
       readFrom(n.GetValue(), d.GetValue());
    }

    ~Rational(){}


    Rational& GetValue(){
       return *this;
    }

    void readFrom(int64_t n,  int64_t d){

        if(d==0){
           nominator = 0;
           denominator = 1;
           negative = 0;
           SetDefined(false);
           return;
        } 
        if(n==0){
          nominator = 0;
          denominator = 1;
          negative = 0;
          SetDefined(true);
          return;
        }
        SetDefined(true);
        nominator = abs(n);
        denominator = abs(d);
        negative = (n<0)==(d<0)?0:1;
        shorten();
    }

    
    Rational& operator=(const Rational& r){
       SetDefined(r.IsDefined());
       nominator = r.nominator;
       denominator = r.denominator;
       negative = r.negative;
       return *this;
    }

    bool operator==(const Rational& r) const{
      if(!IsDefined()){
          return !r.IsDefined();
      }
      if(!r.IsDefined()){
        return false;
      }
      if((nominator==0) && (r.nominator==0)){
        return true;
      }
      return    (nominator==r.nominator) 
             && (denominator==r.denominator) 
             && (negative == r.negative);
    }


    bool operator<(const Rational& r) const{
       return compareTo(r) < 0;
    }
    bool operator<=(const Rational& r) const{
       return compareTo(r) <= 0;
    }
    bool operator>=(const Rational& r) const{
       return compareTo(r) >= 0;
    }
    bool operator>(const Rational& r) const{
       return compareTo(r) > 0;
    }

    int compareTo(const Rational & r) const{
       if(!IsDefined()){
          if(!r.IsDefined()){
            return 0;
          } else {
             return -1;
          }
       }
       if(!r.IsDefined()){
          return 1;
       }
       // this and r are defined
       // test for zero
       if((nominator==0) && (r.nominator==0)){
          return 0;
       }
       // test for sign
       if(negative != r.negative){
         return negative==1?-1:1;
       }
       int cmp1 = abscompare(r);
       return negative==0?cmp1:-cmp1;  
    }


   
    Rational operator*(const Rational& r){
       Rational res(*this);
       res *=(r);
       return res;
    }
      
    Rational& operator*=(const Rational& r){

       cout << "* called" << endl;
       cout << "thhis = " << this->ToString() << endl;
       cout << " r = " << r.ToString() << endl;

       if(!IsDefined() || !r.IsDefined()){
          SetDefined(false);
          return *this;
       }
       uint64_t t1 = euklid(nominator, r.denominator);
       uint64_t t2 = euklid(denominator, r.nominator);
       nominator = (nominator/t1) * (r.nominator/t2);
       denominator = (denominator/t2) * (r.denominator/t1);
       negative = (negative != r.negative)?1:0;
       return *this;   
    }

    Rational operator/(const Rational& r){
       Rational res(*this);
       res *=(r.reverse2());
       return res;
    }
      
    Rational& operator/=(const Rational& r){
       (*this) = r.reverse2();
       return *this;
    }

    Rational& operator+=(const Rational& r){
       if(negative == r.negative){
          uint64_t t1 = euklid(denominator, r.denominator);
          nominator = nominator*(r.denominator / t1) + 
                      r.nominator*(denominator/t1);
          denominator = denominator * (r.denominator/t1);  
          return this->shorten();
       }
       if(r.negative==1){
          uint64_t t1 = euklid(denominator, r.denominator);
          denominator = denominator * (r.denominator/t1);  
          uint64_t s1 = nominator*(r.denominator / t1);
          uint64_t s2 = r.nominator*(denominator/t1);
          if(s1>s2){
            nominator = s1-s2;
          }  else if(s1<s2){
            nominator = s2-s1;
            negative = 1;  
          } else {
            negative = 0;
            nominator = 0;
            denominator = 1;
          }
          return this->shorten();
       } 
       // this is negative, r is positive
       uint64_t t1 = euklid(denominator, r.denominator);
       denominator = denominator * (r.denominator/t1);  
       uint64_t s1 = nominator*(r.denominator / t1);
       uint64_t s2 = r.nominator*(denominator/t1);
       if(s2>s1){
            nominator = s2-s1;
            negative = 0;
       }  else if(s2<s1){
            nominator = s1-s2;
            negative = 1;  
       } else {
            negative = 0;
            nominator = 0;
            denominator = 1;
       }
       return this->shorten();
    }

    Rational operator+(const Rational& r) const{
       Rational res(*this);
       return res += r;
    }

    Rational& operator-=(const Rational& r) {
       Rational t(r);
       t.switchSign();
       (*this) +=t;
       return *this;
    }
    
    Rational operator-(const Rational& r) {
       Rational res(*this); 
       return res -= r;
    }

    void reverse(){
       if(IsDefined()){
          if(nominator==0){
            SetDefined(false);
          } else {
             uint64_t tmp = nominator;
             nominator = denominator;
             denominator = tmp;
          }
       }
    }

    Rational reverse2() const{
      Rational res(*this);
      res.reverse();
      return res;
    }

    void abs(){
       negative = 0;
    }

    Rational abs2(){
       Rational res(*this);
       res.negative = 0;
       return res;
    }

/*
Attribute Support

*/
  int Compare(const Attribute* arg) const{
    return compareTo(*((Rational*)arg));
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
     return (size_t) negative + nominator + denominator;
  }

  void CopyFrom(const Attribute* arg){
     *this = *((Rational*)arg);
  }

  Rational* Clone() const{
       return new Rational(*this);
  }

  std::string ToString() const{
      stringstream ss;
      if(negative && nominator!=0){
        ss << "- ";
      }
      ss << nominator;
      if((nominator!=0) && (denominator!=1)){
         ss << " / " << denominator;
      }
      return ss.str();
  }


  ostream& Print(ostream &os) const{
     return os << ToString();
  }

  ListExpr ToListExpr(ListExpr typeInfo) const{
     if(!IsDefined()){
       return listutils::getUndefined();
     }
     if(negative){
        return nl->ThreeElemList( nl->SymbolAtom("-"),
                                  toListExpr(nominator),
                                  toListExpr(denominator));
     } else {
        return nl->TwoElemList( toListExpr(nominator),
                                toListExpr(denominator));
     }
  }

  bool ReadFrom(const ListExpr LE, const ListExpr typeInfo){

    if(listutils::isSymbolUndefined(LE)){
        SetDefined(false);
        return true;
    }
    if(!nl->HasLength(LE,2) && !nl->HasLength(LE,3)){
       return false;
    }

    char sign = 0;
    bool correct = true;
    ListExpr LR;
    if(nl->HasLength(LE,3)){
       sign = getSign(nl->First(LE),correct);
       LR = nl->Rest(LE); 
    } else {
       LR = LE;
    }

    if(!correct) {
       return false;
    }
    uint64_t nom = getUint64(nl->First(LR), correct);
    if(!correct) {
       return false;
    }
    uint64_t den = getUint64(nl->Second(LR),correct);
    if(!correct){
       return false;
    }
    SetDefined(true);
    nominator = nom;
    denominator = den;
    negative = sign;
    if(denominator==0){
       nominator=0;
       denominator=1;
       negative = 0;
       SetDefined(false);
    }
    return true;
  }

  static const string BasicType(){
     return "rational";
  }

  static bool checkType(const ListExpr e){
     return listutils::isSymbol(e, BasicType());
  }

  static bool CheckKind(ListExpr type, ListExpr& errorInfo){
     return nl->IsEqual(type,BasicType());
  }    


  static ListExpr Property(){
       return gentc::GenProperty("-> DATA", 
                          BasicType(),
                          "(sign int int)",
                          "(- 1 3)");
     }   


  // Support for csv import / export
  virtual void ReadFromString(string value){
     bool error=false;
     int state = 0;
     char neg=0;
     uint64_t nom=0;
     uint64_t denom=1;
     stringutils::trim(value);
      
     for(size_t i=0;i<value.length() && !error;i++){
        char c = value[i];
        switch (c){
          case '+' : if(state==0){
                       state = 1;
                     } else {
                       error=true;
                     }
                     break;
          case '-' : if(state==0){
                       state = 1;
                       neg = 1;
                     } else {
                       error=true;
                     }
                     break;
          case ' ' : if(state==1){
                       state = 1;
                     } else if(state==2){
                       state = 4;
                     } else if(state==3){
                        state=3;
                     } else {
                        error=true;
                     }
                     break;
         case '/' : if(state==4){
                      state = 3;
                    } else if(state==2){
                      state = 3;
                    } else {
                      error = true;
                    }
                    break;
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9': if(state==0){
                      nom = getDigit(c);
                      state = 2;
                   } else if(state==1){
                      nom = getDigit(c);
                      state = 2;
                   } else if(state==2){
                      nom = nom*10 + getDigit(c);
                   } else if(state==3){
                      denom = getDigit(c);
                      state = 5;
                   } else if(state==5){
                      denom = denom*10 + getDigit(c);
                   } else {
                      error=true;
                   }
                   break;
          default: error = true;
        };
     }
     if(error || ((state!=2) && (state!=5)) || (denom==0)){
        nominator=1;
        denominator=0;
        negative  = 0;
        SetDefined(false);
     } else {
        SetDefined(true);        
        nominator = nom;
        denominator = denom;
        negative = neg;
     }
  }

  virtual string getCsvStr() const{
     if(!IsDefined()){
        return "undef";
     }
     stringstream  res1;
     if(negative!=0){
        res1 << "-";
     }
     res1 << nominator << "/" << denominator ;
     return res1.str();
  }

 

  private:
    char negative;
    uint64_t nominator;
    uint64_t denominator;


   static  uint64_t euklid(uint64_t a, uint64_t b) {
       while(b){
          uint64_t h = a % b;
          a = b;
          b = h;
       }
       return a;
    }

    int abscompare(const Rational& r) const{
      assert(IsDefined());
      assert(r.IsDefined());
      uint64_t t1 = euklid(nominator,r.nominator);
      uint64_t t2 = euklid(denominator, r.denominator);
      uint64_t tl = (nominator/t1 ) * (r.denominator/t2);
      uint64_t tr = (r.nominator/t1) * (denominator/t2);
      if(tl < tr){
         return -1;
      } else if(tl>tr){
         return 1;
      } else {
         return 0;
      }
    }

    Rational& shorten(){
      uint64_t t = euklid(nominator,denominator);
      nominator   /= t;
      denominator /= t;
      return *this;
    }


    void switchSign(){
       negative = negative==0?1:0;
    }    

    static ListExpr toListExpr(uint64_t v){
      if(v< (uint32_t)numeric_limits<int32_t>::max()){
         return nl->IntAtom((int32_t) v);
      } 
      return nl->TwoElemList(
         nl->IntAtom( (int32_t) (v>>32)),
         nl->IntAtom( (int32_t) (v&0xFFFFFFFF)));
    }

    static char getSign( const ListExpr e, bool& correct){
      correct = false;
      if(nl->AtomType(e)!=SymbolType){
         return 0;
      }
      string s = nl->SymbolValue(e);
      if(s=="-"){
         correct = true;
         return 1;
      }
      if(s=="+"){
         correct = true;
         return 0;
      }
      return 0;
    }

    static uint64_t getUint64(const ListExpr e, bool& correct){
       if(nl->AtomType(e)==IntType){
          uint32_t e2 = nl->IntValue(e);
          correct = true;
          return e2;
       }
       if(!nl->HasLength(e,2)){
          correct = false;
          return 0;
       }
       ListExpr f = nl->First(e);
       ListExpr s = nl->Second(e);
       if( (nl->AtomType(f)!=IntType)  || (nl->AtomType(s)!=IntType)){
          correct = false;
          return 0;
       }
       uint32_t fi = nl->IntValue(f);
       uint32_t si = nl->IntValue(s);
       uint64_t res = fi;
       res = (res << 32) | si;
       correct = true;
       return res;
    }

    static uint64_t abs(const int64_t v) {
       return v<0?-v:v;
    }

    static uint32_t getDigit(char c){
       return c - '0';
    }




};


#endif



