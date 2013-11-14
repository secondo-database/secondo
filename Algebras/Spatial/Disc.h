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


#ifndef DISC_H
#define DISC_H

#include "Attribute.h"
#include "RectangleAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Point.h"

/*
1 Implementation of a disc

*/

class Disc : public Attribute{
 
  public:
    Disc() {}
    explicit Disc(bool def): Attribute(def), x(0),y(0),radius(0) {}
    Disc(const double _x, const double _y, const double _radius): 
       Attribute(true), x(_x),y(_y),radius(_radius){
       SetDefined(radius>=0);
    }
    Disc(const Disc& src): 
       Attribute(src), x(src.x),y(src.y),radius(src.radius){}

   
    Disc(const Point& p): Attribute(false){
      if(!p.IsDefined()){
        return;
      }
      set(p.GetX(),p.GetY(),0);
    }

    Disc(const Point& p1, const Point& p2): Attribute(false){
      if(!p1.IsDefined() || !p2.IsDefined()){
        return;
      }
      double x = (p1.GetX() + p2.GetX())/2.0;
      double y = (p1.GetY()+p2.GetY())/2.0;
      double r = p1.Distance(p2) / 2.0;
      set(x,y,r);
    }


    Disc(const Point& p1, const Point& p2, const Point& p3);
    


    ~Disc() {}

    Disc& operator=(const Disc& src){
      SetDefined(src.IsDefined());
      x = src.x;
      y = src.y;
      radius = src.radius;
      return *this;
    }

    bool set(const double x, const double y, const double radius){
      if(radius<0){
         return false;
      }
      this->x = x;
      this->y = y;
      this->radius = radius;
      SetDefined(true);
      return true;
    }

    bool contains(const Point& p) const{
       if(!IsDefined() || !p.IsDefined()){
         return false;
       }
       Point center(true,x,y);
       double d = center.Distance(p);
       return (AlmostEqual(d,radius) || d<radius);
    }


    ListExpr ToListExpr(ListExpr typeInfo);
    

    bool ReadFrom(ListExpr value, ListExpr typeInfo);

    int Compare(const Attribute* rhs) const;
    
    bool Adjacent(const Attribute*) const {return false;}

    size_t HashValue() const;

    void CopyFrom(const Attribute* rhs);

    Disc* Clone() const{ return new Disc(*this); }

    size_t Sizeof() const { return sizeof(*this); }

    virtual ostream& Print( ostream& os ) const;

    static string BasicType(){ return "disc"; }

    static const bool checkType(const ListExpr type){
        return listutils::isSymbol(type, BasicType());
    }

   static ListExpr Property(){
        return gentc::GenProperty("-> DATA",
                           BasicType(),
                          "(x y radius)",
                          "(8.4 16.7 27");
    }

   static bool CheckKind(ListExpr type, ListExpr& errorInfo){
        return nl->IsEqual(type,BasicType());
   }

  private:
     double x;
     double y;
     double radius;
};




#endif


