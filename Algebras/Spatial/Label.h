
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/



#ifndef LABEL_H
#define LABEL_H

/*
1 class Label 

The label class consists of a piece of text, a position, and  a direction
and can be used to add object names to a map.

*/

#include "Attribute.h"
#include "GenericTC.h"


class Label: public Attribute{

public:

/*
1.1 Constructors

*/
    Label() {} // should do nothing
    explicit Label(const bool defined): Attribute(defined), 
                                        x(0),y(0),d(0), l(0) {
        memset(text,0,128*sizeof(char));       
    }

    Label(string _text, double _x, double _y, double _d =0):
     x(_x), y(_y), d(_d){
       l = min(128, (int) _text.length());
       memcpy(text,_text.c_str(),l);
    }

    Label(const Label& rhs) : x(rhs.x), y(rhs.y), d(rhs.d), l(rhs.l){
       memcpy(text,rhs.text,l);
    }

/*
1.2 Destructor

*/
   ~Label() {}

/*

1.3 Operators

*/
   Label& operator=(const Label& rhs){
      SetDefined(rhs.IsDefined());
      x = rhs.x;
      y=  rhs.y;
      d = rhs.d;
      l = rhs.l;
      memcpy(text,rhs.text,l);
      return *this;
   }

   void set(const string& t, const double x, const double y, const double d){
     this->x = x;
     this->y = y;
     this->d = d;
     l = min(128,(int) t.length());
     memcpy(text,t.c_str(),l);
   }


  string getStr() const{
     string res(text,l);
     return res;
  }

   ListExpr ToListExpr(const ListExpr& typeInfo) const{
      return nl->FourElemList(
               nl->TextAtom(getStr()),
               nl->RealAtom(x),
               nl->RealAtom(y),
               nl->RealAtom(d));
   }

   bool ReadFrom(const ListExpr LE, const ListExpr& typeInfo) {
     if(!nl->HasLength(LE,4)){
        return false;
     }
     if(nl->AtomType(nl->First(LE))!=TextType ||
        !listutils::isNumeric(nl->Second(LE)) ||
        !listutils::isNumeric(nl->Third(LE)) ||
        !listutils::isNumeric(nl->Fourth(LE))) {
        return false;
     }
     string t = nl->Text2String(nl->First(LE));
     double x = listutils::getNumValue(nl->Second(LE));
     double y = listutils::getNumValue(nl->Third(LE));
     double d = listutils::getNumValue(nl->Fourth(LE));
     set(t,x,y,d);
     return true;
   }

   string toString() const{
      stringstream ss;
      ss << getStr();
      ss << "(" << x << ", " << y << ", " << d << ")";
      return ss.str();
   }

   int Compare(const Attribute* rhs1) const{
      if(!IsDefined()){
        if(!rhs1->IsDefined()){
           return 0;
        } else {
           return -1;
        }
      }
      if(!rhs1->IsDefined()){
        return 1;
      }
      Label* rhs = (Label*) rhs1;
      if(l < rhs->l){
         return -1;
      } else if(l  > rhs->l){
         return 1;
      }
      // now, l==rhs.l holds
      int cmp = memcmp(text,rhs->text,l);
      if(cmp){
        return cmp<0?-1:1;
      }
      if(!AlmostEqual(x,rhs->x)){
        return x < rhs->x?-1:1;
      }
      if(!AlmostEqual(y,rhs->y)){
        return y < rhs->y?-1:1;
      }
      if(!AlmostEqual(d,rhs->d)){
        return d < rhs->d?-1:1;
      }
      return 0;
   }

   bool Adjacent(const Attribute* rhs) const{
     return false;
   }

   size_t HashValue() const{
     size_t res = 0;
     if(!IsDefined()){
        return 0;
     }
     int len = l<6?l:6;
     for(int i=0; i< len; i++){
        res += (unsigned char) text[i];
     }
     return res;
   } 

   void CopyFrom(const Attribute* attr) {
      operator=( *((Label*) attr));
   }

   Label* Clone() const{
     return new Label(*this);
   }

    size_t Sizeof() const { return sizeof(*this); }

    static const string BasicType(){
           return "spatiallabel";
    }

    static const bool checkType(const ListExpr type){
         return listutils::isSymbol(type, BasicType());
    }

    static ListExpr Property(){
          return gentc::GenProperty("-> DATA",
                                    BasicType(),
                                   "(text x y d)",
                           "('Name' 5.0 8.3 45.0)");
    }    

    static bool CheckKind(ListExpr type, ListExpr& errorInfo){
       return nl->IsEqual(type,BasicType());
    }

private:
   double x;
   double y;
   double d;
   uint8_t l;
   char text[128];
};

#endif


