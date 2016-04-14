/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen, 
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

#ifndef SEGMENT_H
#define SEGMENT_H

#include "NestedList.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "SpatialAlgebra.h"
#include "GenericTC.h"


class Segment: public Attribute{

public:
   Segment() {}
   Segment(const SimplePoint& _p1, 
           const SimplePoint& _p2): Attribute(true),p1(_p1),p2(_p2) {}

   Segment(const Segment& s): Attribute(s), p1(s.p1),p2(s.p2) {}

   Segment& operator=(const Segment& src){
      Attribute::operator=(src);
      p1 = src.p1;
      p2 = src.p2;
      return *this;
   }
   Segment (int dummy): Attribute(false), p1(0,0),p2(0,0) {}

   ~Segment() {}

   static const std::string BasicType(){ return "segment"; }
   static const bool checkType(ListExpr v){
     return listutils::isSymbol(v, BasicType());
   }
   static bool CheckKind(ListExpr type, ListExpr& errorInfo){
     return listutils::isSymbol(type, BasicType());
   }
 
   int Compare(const Attribute* arg) const{
     if(!IsDefined()){
        return arg->IsDefined()?-1:0;
     } 
     if(!arg->IsDefined()){
        return 1;
     }
     Segment* seg = (Segment*) arg;
     int cmp = p1.compare(seg->p1);
     if(cmp!=0){
       return cmp;
     }
     return p2.compare(seg->p2);
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
      return p1.hash() * 10 + p2.hash();
   }

   void CopyFrom(const Attribute* arg){
     *this = *((Segment*) arg);
   }
   Attribute* Clone() const{
      return new Segment(*this);
   }    
   
   static ListExpr Property(){
     return gentc::GenProperty("->DATA",
                    BasicType(),
                    "(real real real real)",
                    "(1.0 2.0 1.0 8.0)");

    }

    bool ReadFrom(ListExpr le, const ListExpr typeInfo){
       if( listutils::isSymbolUndefined(le)){
            SetDefined (false);
            return true ;
       }
       if(!nl->HasLength(le,4)){
           return false;
       }
       ListExpr X1 = nl->First(le);
       ListExpr Y1 = nl->Second(le);
       ListExpr X2 = nl->Third(le);
       ListExpr Y2 = nl->Fourth(le);
       if(    !listutils::isNumeric(X1) 
           || !listutils::isNumeric(Y1)
           || !listutils::isNumeric(X2)
           || !listutils::isNumeric(Y2)){
         return false;
       }
       SimplePoint pp1(listutils::getNumValue(X1), listutils::getNumValue(Y1)); 
       SimplePoint pp2(listutils::getNumValue(X2), listutils::getNumValue(Y2)); 
       if(pp1==pp2){
          return false;
       }
       SetDefined(true);
       p1 = pp1;
       p2 = pp2;
       return true;
    }

    ListExpr ToListExpr(ListExpr typeInfo) const{
       if(!IsDefined()){
          return listutils::getUndefined();
       }
      return nl->FourElemList(
                nl->RealAtom(p1.getX()),
                nl->RealAtom(p1.getY()),
                nl->RealAtom(p2.getX()),
                nl->RealAtom(p2.getY())
             );
    }

    const SimplePoint& getP1(){
        return p1;
    }
    const SimplePoint& getP2(){
        return p2;
    }


private:
  SimplePoint p1;
  SimplePoint p2;

};

#endif

