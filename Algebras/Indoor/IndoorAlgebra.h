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

[1] Header File of the IndoorAlgebra

June, 2010 Jianqiu Xu

[TOC]

1 Overview

This header file essentially contains the definition of the classes ~Point~,
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  Figure \cite{fig:spatialdatatypes.eps}
shows examples of these spatial data types.

2 Defines and includes

*/
#ifndef __INDOOR_ALGEBRA_H__
#define __INDOOR_ALGEBRA_H__

#include <fstream>
#include <stack>
#include <vector>
#include <queue>
#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "RectangleAlgebra.h"
#include "WinUnix.h"
#include "AvlTree.h"
#include "Symbols.h"
#include "AlmostEqual.h"
#include "../Spatial/SpatialAlgebra.h"

Word InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );

/*
for 3D data for indoor application

*/
class Floor3D:public StandardSpatialAttribute<2>
{
public:
    Floor3D(){}
    Floor3D(const float h):
    StandardSpatialAttribute<2>(true),floor_height(h), reg(0)
    {
//      cout<<"Constructor1()"<<endl;
    }
    Floor3D(const float h, Region& r):
    StandardSpatialAttribute<2>(r.IsDefined()),floor_height(h), reg(r)
    {
//      cout<<"Constructor2()"<<endl;
    }
    Floor3D(const Floor3D& fl):
    StandardSpatialAttribute<2>(fl.IsDefined()),
    floor_height(fl.GetHeight()),reg(*(fl.GetRegion()))
    {
//      cout<<"Constructor3()"<<endl;
    }
    ~Floor3D()
    {
//      reg.Destroy();
//      reg.DeleteIfAllowed(false);
    }
    void SetValue(const float h, Region* r)
    {
      floor_height = h;
      reg = *r;
      SetDefined(true);
    }
    Floor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
    bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
    inline int Size() const {return reg.Size();}
    inline float GetHeight() const {return floor_height;}
    inline bool IsEmpty() const{return !IsDefined() || Size() == 0;}
    inline size_t Sizeof() const{return sizeof(*this);}
    inline bool Adjacent(const Attribute* arg)const{return false;}
    int Compare(const Attribute* arg)const
    {
      return 0;
    }
    void CopyFrom(const Attribute* right)
    {
      *this = *(const Floor3D*)right;
    }
    const Rectangle<2> BoundingBox() const
    {
      return reg.BoundingBox();
    }
    double Distance(const Rectangle<2>& r)const
    {
      return reg.BoundingBox().Distance(r);
    }
    Floor3D* Clone() const {return new Floor3D(*this);}
    size_t HashValue() const
    {
      return reg.HashValue();
    }
    const Region* GetRegion() const
    {
       const Region* p_to_r = &reg;
       if(reg.IsDefined()) return p_to_r;
       else return NULL;
    }
    float GetHeight() {return floor_height;}
    static void* Cast(void* addr){return new (addr)Floor3D();}
private:
    float floor_height;
    Region reg;
};

/*
3D point for indoor application

*/
class Point3D:public StandardSpatialAttribute<3>
{
public :

    inline Point3D(){}
    inline Point3D(const bool d, const double & a = 0.0,
                   const double& b = 0.0, const double& c = 0.0):
    StandardSpatialAttribute<3>(d), x(a), y(b), z(c)
    {}
    inline Point3D(const Point3D& p3d):
    StandardSpatialAttribute<3>(p3d.IsDefined()), x(p3d.GetX()),
    y(p3d.GetY()), z(p3d.GetZ())
    {}

    Point3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
    ~Point3D(){}
    inline size_t Sizeof() const{return sizeof(*this);}
    int Compare(const Attribute* arg)const
    {
      return 0;
    }
    inline bool Adjacent(const Attribute* arg)const{return false;}
    Point3D* Clone() const {return new Point3D(*this);}
    size_t HashValue() const
    {
        if(!IsDefined()) return 0;
        else
          return (size_t)(x+y+z);
    }
    void CopyFrom(const Attribute* right)
    {
      *this = *(const Point3D*)right;
    }
    inline const Rectangle<3> BoundingBox() const
    {
      if(IsDefined()){
            return Rectangle<3>(true,
                    x - ApplyFactor(x),
                    x + ApplyFactor(x),
                    y - ApplyFactor(y),
                    y + ApplyFactor(y),
                    z - ApplyFactor(z),
                    z + ApplyFactor(z));
      }else{
          return Rectangle<3>(false,0.0,0.0,0.0,0.0,0.0,0.0);
      }
    }
    double Distance(const Rectangle<3>& r)const
    {
      return BoundingBox().Distance(r);
    }
    double Distance(const Point3D& p3d) const
    {
      double a = fabs(x - p3d.GetX());
      double b = fabs(y - p3d.GetY());
      double c = fabs(z - p3d.GetZ());
      return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
    }
    bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
    inline bool IsEmpty() const{return !IsDefined();}
    inline const double& GetX() const {return x;}
    inline const double& GetY() const {return y;}
    inline const double& GetZ() const {return z;}
private:
    double x;
    double y;
    double z;
};
#endif // __INDOOR_ALGEBRA_H__
