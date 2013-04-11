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


#ifndef DRM_H
#define DRM_H

#include "Attribute.h"
#include "RectangleAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"

/*
1 Implementation of Direction Relation Matrix (DRM)

1.1 Description

An DRM

*/

class DRM : public Attribute{
 
  public:
    DRM() {}
    DRM(bool def): Attribute(def), value(0) {}
    DRM(const int v): Attribute(false), value(0){}
    DRM(const uint16_t v): Attribute(true), value(v){}
    DRM(const DRM& src): Attribute(src.IsDefined()), value(src.value){}

    ~DRM() {}

     DRM& operator=(const DRM& src);


    void computeFromR(const Rectangle<2>& r1, const Rectangle<2>& r2);

    void computeFrom(const StandardSpatialAttribute<2>& a, 
                     const StandardSpatialAttribute<2>& b);

    ListExpr ToListExpr(ListExpr typeInfo);
    bool ReadFrom(ListExpr value, ListExpr typeInfo);

    int Compare(const Attribute* rhs) const;
    
    bool Adjacent(const Attribute*) const {return false;}

    size_t HashValue() const { return IsDefined()?0:value+1; }

    void CopyFrom(const Attribute* rhs);

    DRM* Clone() const{ return new DRM(*this); }

    size_t Sizeof() const { return sizeof(*this); }

    virtual ostream& Print( ostream& os ) const;

    static string BasicType(){ return "drm"; }

    static const bool checkType(const ListExpr type){
        return listutils::isSymbol(type, BasicType());
    }

   static ListExpr Property(){
        return gentc::GenProperty("-> DATA",
                           BasicType(),
                          "int in 0..511",
                          "413");
    }

   static bool CheckKind(ListExpr type, ListExpr& errorInfo){
        return nl->IsEqual(type,BasicType());
   }

  private:
    uint16_t value;
};

#endif


