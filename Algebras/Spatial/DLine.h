
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]



1 Class DLine

*/


#ifndef DLINE_H
#define DLINE_H

#include "Attribute.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"


class SimpleSegment{
   public: 

      SimpleSegment(){ }

      SimpleSegment(const double& _x1, const double& _y1,
                    const double& _x2, const double& _y2):
                    x1(_x1), y1(_y1),x2(_x2),y2(_y2) {}

      double x1;
      double y1;
      double x2;
      double y2;

      ListExpr ToListExpr() const{
          return nl->FourElemList(
                      nl->RealAtom(x1),
                      nl->RealAtom(y1),
                      nl->RealAtom(x2),
                      nl->RealAtom(y2));

      }

      static bool readFirstAsNum(ListExpr& list, double& value){
          ListExpr f = nl->First(list);
          if(!listutils::isNumeric(f)){
              return false;
          }
          value = listutils::getNumValue(f);
          list = nl->Rest(list);
          return true;   
      }


      bool readFrom(ListExpr le){
         if(!nl->HasLength(le,4)){
             return false;
         }
         if(!readFirstAsNum(le,x1)){
           return false;
         }
         if(!readFirstAsNum(le,y1)){
           return false;
         }
         if(!readFirstAsNum(le,x2)){
           return false;
         }
         if(!readFirstAsNum(le,y2)){
           return false;
         }
         return true;
      }


      static int comp(const double d1, const double d2){
         if(d1<d2) return -1;
         if(d1>d2) return 1;
         return 0;
      }

      int compare(const SimpleSegment& s2){
         int cmp = comp(x1,s2.x1);
         if(cmp!=0) { return cmp; }
         cmp = comp(y1,s2.y1);
         if(cmp!=0) { return cmp; }
         cmp = comp(x2,s2.x2);
         if(cmp!=0) { return cmp; }
         cmp = comp(y2,s2.x2);
         return cmp; 
 
      }

      ostream& print(ostream& os) const{
          os << "(" << x1 << ", " << y1 << ") ( " << x2 << ", " << y2 << ")";
          return os;
      }

};


class DLine : public Attribute{

  public:
     DLine(){}
     DLine(bool def): Attribute(def), segments(0) {}

     DLine(const DLine& s): Attribute(s.IsDefined()),
                            segments(s.segments.Size()){
        segments.copyFrom(s.segments);
     }


     ~DLine(){}

     void clear(){
        SetDefined(true);
        segments.clean();
     }

     void append(const SimpleSegment& s) {
        segments.Append(s);
     }

     void get(size_t index, SimpleSegment& s) const{
        segments.Get(index,s);
     }
 
     void set(size_t index, const SimpleSegment& s){
        segments.Put(index,s);
     }    

     void resize(size_t newSize){
       segments.resize(newSize);
     }

     
     ListExpr ToListExpr(ListExpr typeInfo) const {
        if(segments.Size()<1){
           return nl->TheEmptyList();
        }
        SimpleSegment s;
        segments.Get(0,s);
        ListExpr res = nl->OneElemList(s.ToListExpr());
        ListExpr last = res;
        for(int i=1;i<segments.Size();i++){
           segments.Get(i,s);
           last = nl->Append(last, s.ToListExpr());
        }
        return res;
     }

     bool ReadFrom(ListExpr LE, const ListExpr typeInfo) {
        segments.clean();
        if(listutils::isSymbolUndefined(LE)){
          SetDefined(false);
          return true;
        }
        if(nl->AtomType(LE) != NoAtom){
          return false;
        }
        SimpleSegment s;
        while(!nl->IsEmpty(LE)){
            if(!s.readFrom(nl->First(LE))){
               segments.clean();
               SetDefined(false);
               return false; 
            }
            segments.Append(s);
            LE = nl->Rest(LE);
        } 
        return true; 
     }


     int Compare(const Attribute* rhs) const{
        if(!IsDefined()){
           return rhs->IsDefined()?-1:0; 
        }
        if(!rhs->IsDefined()){
           return 1;
        }
       
        DLine* dl = (DLine*) rhs;
        if(segments.Size() < dl->segments.Size()){
           return -1;
        }
        if(segments.Size() > dl->segments.Size()){
           return 1;
        }
        SimpleSegment ts;
        SimpleSegment ds;
        for(int i=0;i<segments.Size();i++){
             segments.Get(i,ts);
              dl->segments.Get(i,ds);
              int cmp = ts.compare(ds);
              if(cmp!=0){
                 return cmp;
              }
        }
        return 0;
     }

     int NumOfFLOBs() const{
        return 1;
     }
     Flob* GetFLOB(int i){
        return &segments;
     }
    
     bool Adjacent(const Attribute*) const {return false;}

     size_t HashValue() const { return segments.Size(); }

     void CopyFrom(const Attribute* arg) {
         segments.clean();
         if(!arg->IsDefined()){
             SetDefined(false);
         }  
         SetDefined(true);
         DLine* d = (DLine*) arg;
         segments.copyFrom(d->segments); 
     }

     DLine* Clone() const {
         return new DLine(*this);
     }

     size_t Sizeof() const { return sizeof(*this); }

     virtual ostream& Print( ostream& os ) const{
        if(!IsDefined()){
           os << "Undefined";
           return os;
        }
        SimpleSegment s;
        for(int i=0;i<segments.Size();i++){
           segments.Get(i,s);
           s.print(os);
           os << endl;
        }
        return os;
     }

     static string BasicType(){
         return "dline";
     }
     
     static const bool checkType(const ListExpr type){
        return listutils::isSymbol(type, BasicType());
     }

   
     static ListExpr Property(){
         return gentc::GenProperty("-> DATA",
                                    BasicType(),
                                   "(s2 s2 ...) with s_i=(x1 y1 x2 y2)",
                           "((14.0 18.0 16.0 15) )");
     }    

       static bool CheckKind(ListExpr type, ListExpr& errorInfo){
         return nl->IsEqual(type,BasicType());
       }    

  private:
     DbArray<SimpleSegment> segments;



};



#endif


