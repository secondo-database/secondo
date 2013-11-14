
/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of mathematics and computer science,
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
#include "ListUtils.h"
#include "NestedList.h"
#include "Attribute.h"
#include "Disc.h"


   Disc::Disc(const Point& p1, const Point& p2, const Point& p3):
     Attribute(false){
      if(!p1.IsDefined() || !p2.IsDefined() || !p3.IsDefined()){
         return; 
      }
      SetDefined(true);
      double x1 = p1.GetX(); 
      double y1 = p1.GetY();
      double x2 = p2.GetX(); 
      double y2 = p2.GetY();
      double x3 = p3.GetX(); 
      double y3 = p3.GetY();

      double a = 2*x2 - 2*x1;
      double b = 2*y2 - 2*y1;
      double c = x1*x1-x2*x2+y1*y1-y2*y2;
      double d = 2*x3 - 2*x1;
      double e = 2*y3 - 2*y1;
      double f = x1*x1 - x3*x3 + y1*y1 - y3*y3;
      double q = b*d - a*e;
      if (q == 0){ 
         //points are on a line
         double d1 = p1.Distance(p2);
         double d2 = p1.Distance(p3);
         double d3 = p2.Distance(p3);
         Point P1(false);
         Point P2(false);
         double D;
         if(d1 > d2){
           if(d1 > d3){ // d1 is the biggest
              P1 = p1;
              P2 = p2;
              D = d1;
           } else { // d3 is the biggest
              P1 = p2;
              P2 = p3;
              D = d3;
           }
         } else {
           if(d2 > d3){ // d2 is the biggest
              P1 = p1;
              P2 = p3;
              D = d2;
           } else {  // d3 is the biggest
              P1 = p2;
              P2 = p3;
              D = d3;
           }
         }
         double x = (P1.GetX()+P2.GetX())/2.0;
         double y = (P1.GetY()+P2.GetY())/2.0;
         set(x,y,D/2);
         return;
      }
 
      y = (a*f - c*d) / q;
     if (a != 0){
        x = -1*((c + b*y)/a);
     } else {
        x = -1* ((e*y+f) / d); 
     }
     double r = p1.Distance(Point(true,x,y));
     set(x,y,r);
   }


    ListExpr Disc::ToListExpr(ListExpr typeInfo){
       if(!IsDefined()){
        return listutils::getUndefined();
       }
       return nl->ThreeElemList( nl->RealAtom(x), 
                                 nl->RealAtom(y), 
                                 nl->RealAtom(radius)); 
    }
    
    bool Disc::ReadFrom(ListExpr value, ListExpr typeInfo){
       if(listutils::isSymbolUndefined(value)){
          SetDefined(false);
          return true;
       }
       if(!nl->HasLength(value,3)){
          return false;
       }
       ListExpr X = nl->First(value);
       ListExpr Y = nl->Second(value);
       ListExpr R = nl->Third(value);
       if(    !listutils::isNumeric(X) || !listutils::isNumeric(Y) 
           || !listutils::isNumeric(R)){
         return false;
       }
       double x = listutils::getNumValue(X);
       double y = listutils::getNumValue(Y);
       double r = listutils::getNumValue(R);
       if(r<0){
         return false;
       }
       set(x,y,r);
       return true;
    }

    int Disc::Compare(const Attribute* rhs) const{
       Disc* d = (Disc*) rhs;
       if(!IsDefined()){
         return d->IsDefined()?-1:0;
       }
       if(!d->IsDefined()){
          return 1;
       }
       if(!AlmostEqual(x,d->x)){
          return x<d->x?-1:1;
       }
       if(!AlmostEqual(y,d->y)){
          return y<d->y?-1:1;
       }
       if(!AlmostEqual(radius,d->radius)){
          return radius<d->radius?-1:1;
       }
       return 0;
    }

    size_t Disc::HashValue() const { 
        if(!IsDefined()){
           return 0;
        }
        return (size_t)(x+y+radius); 
    }

    void Disc::CopyFrom(const Attribute* rhs) {
        *this = *((Disc*)rhs);
    }

    ostream& Disc::Print( ostream& os ) const{
      os << "(" << x << ", " << y <<", " << radius << ")";
      return os;
    }



