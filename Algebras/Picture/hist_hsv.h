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

//[_] [\_]

*/


#include "Attribute.h"
#include "StringUtils.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "GenericTC.h"

class FLOB;


/*
1 Class hist[_]hsv

This class represents a simple representation of 
a histogram. The dim template determines the size
(number of slots) of this histogram, the lab template
just changes the BasicType function.

*/

template<unsigned int dim, bool lab>
class hist_hsv : public Attribute{
public:

  typedef float domain;

  hist_hsv() {}
  hist_hsv(const int dummy): Attribute(false){
     memset(hist,0,dim*sizeof(domain));
  }

  hist_hsv(const hist_hsv<dim, lab>& src): Attribute(src) {
     memcpy(hist, src.hist, dim*sizeof(domain));
  }

  hist_hsv<dim,lab>& operator=(hist_hsv<dim,lab>& src){
    SetDefined(src.IsDefined());
     memcpy(hist, src.hist, dim*sizeof(domain));
     return *this;
  }

  ~hist_hsv() {}

   static const std::string BasicType(){
     return lab? "hist_lab_" + stringutils::int2str(dim)
               : "hist_hsv_" + stringutils::int2str(dim);
   }

   static const bool checkType(const ListExpr list){
     return listutils::isSymbol(list,BasicType());
   }

   static bool CheckKind(ListExpr type, ListExpr & errorIndo){
     return checkType(type);
   }

   inline virtual int NumOfFLOBs() const{
      return 0;
   }

   inline virtual Flob* GetFLOB(const int i){
      return 0;
   }

   int Compare(const Attribute* arg) const{
     if(!IsDefined()){
        return arg->IsDefined()?-1:0;
     }
     if(!arg->IsDefined()){
        return 1;
     }
     hist_hsv<dim,lab>* h = (hist_hsv<dim,lab>*) arg;

     for(unsigned int i=0;i<dim;i++){
        if(hist[i] != h->hist[i]){
           return hist[i]>h->hist[i]?1:-1;
        }        
     }
     return 0;
   }

   bool Adjacent(const Attribute* arg) const{
     return false;
   }

   size_t Sizeof() const{
     return sizeof(*this);
   }

   size_t HashValue() const{
     if(!IsDefined()) return 0;
     size_t s=0;
     for(unsigned int i=0;i<dim;i++){
       s += (i+1) * hist[i];
     }
     return (size_t) s;
   }

   void CopyFrom(const Attribute* arg){
      *this = *((hist_hsv<dim,lab>*) arg);
   }

   Attribute* Clone() const{
      return new hist_hsv<dim,lab>(*this);
   }

   static ListExpr Property(){
       return gentc::GenProperty(
                        "->DATA",
                        BasicType(),
                        "(double, double, ...) ",
                        "(4.5 8.7 ...)" );
   } 

   bool ReadFrom(ListExpr LE, ListExpr typeInfo){
       if(listutils::isSymbolUndefined(LE)){
          SetDefined(false);
          return true;
       }
       if(!nl->HasLength(LE, dim)){
          return false;
       }
       for(unsigned int i=0;i<dim;i++){
          ListExpr first = nl->First(LE);
          LE = nl->Rest(LE);
          if(!listutils::isNumeric(first)){
             return false;
          }
          hist[i] = (domain) listutils::getNumValue(first);
       }
       SetDefined(true);
       return true;
   }

   ListExpr ToListExpr(ListExpr typeInfo) const{
     if(!IsDefined()){
       return listutils::getUndefined();
     }
     ListExpr res = nl->OneElemList( nl->RealAtom(hist[0]));
     ListExpr last = res;
     for(unsigned int i=1;i<dim;i++){
        last = nl->Append(last, nl->RealAtom(hist[i]));
     }
     return res;
   }

   void distance(const hist_hsv<dim, lab>& h, bool& def, double& res){
      if(!IsDefined() || !h.IsDefined()){
          def = false;
          res = 0;
      } else {
         double s = 0;
         for(size_t i=0;i<dim;i++){
            domain  diff =   hist[i] - h.hist[i];
            s += diff*diff;
         }
         def = true;
         res = std::sqrt(s);
      }
   }

   void set(const unsigned long* hist_abs, const unsigned int numOfPixels, 
            const domain threshold=5e-5) {
      SetDefined(true);
      for(unsigned int i=0;i<dim;i++){
         hist[i] = (domain) hist_abs[i] / (domain) numOfPixels;
         if(hist[i] < threshold){
           hist[i] = 0;
         }
      }
   }



private:
   domain hist[dim];

};




