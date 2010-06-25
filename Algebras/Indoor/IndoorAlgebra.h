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
    bool IsEqual(const Point3D& p3d)
    {
      if(AlmostEqual(x, p3d.GetX()) &&
         AlmostEqual(y, p3d.GetY()) &&
         AlmostEqual(z, p3d.GetZ())) return true;
      else return false;
    }
    inline bool operator<(const Point3D& p3d) const
    {
        assert(IsDefined() && p3d.IsDefined());
        if(AlmostEqual(x, p3d.GetX())){
            if(AlmostEqual(y, p3d.GetY())){
               return z < p3d.GetZ();
            }else
              return y < p3d.GetY();
        }else
            return x < p3d.GetX();
    }
    inline Point3D& operator=(const Point3D& p)
    {
        SetDefined(p.IsDefined());
        if(IsDefined()){
          x = p.GetX();
          y = p.GetY();
          z = p.GetZ();
        }
        return *this;
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
      return sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
    }
    bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
    inline bool IsEmpty() const{return !IsDefined();}
    inline const double& GetX() const {return x;}
    inline const double& GetY() const {return y;}
    inline const double& GetZ() const {return z;}
    inline bool operator==(const Point3D& p3d) const
    {
      if(!IsDefined() && !p3d.IsDefined())return true;
      if(!IsDefined() || !p3d.IsDefined())return false;
      bool result = AlmostEqual(x, p3d.GetX()) &&
         AlmostEqual(y, p3d.GetY()) &&
         AlmostEqual(z, p3d.GetZ());
      return result;
    }
    inline bool operator != (const Point3D& p3d) const
    {
      return !(*this == p3d);
    }
    void Print() const
    {
      cout<<"x "<<x<<" y "<<y<<" z "<<z<<endl;
    }
private:
    double x;
    double y;
    double z;
};

class Line3D: public StandardSpatialAttribute<3>
{
  public:

    inline Line3D() {}

    inline Line3D( const int initsize );

    inline Line3D( const Line3D& ps);

    inline void Destroy()
    {
      points.destroy();
    }

    inline ~Line3D()
    {}

    inline bool IsOrdered() const;

    void StartBulkLoad();

    void EndBulkLoad( bool sort = true, bool remDup = true, bool trim = true );

    inline const Rectangle<3> BoundingBox() const;

    inline bool IsEmpty() const;

    bool IsValid() const;

    inline int Size() const;

    void Clear();

    inline void Resize(const int newSize);

    inline void TrimToSize();

//    inline bool Get( const int i, Point& p ) const;
    inline bool Get( const int i, Point3D& p ) const;

    Line3D& operator=( const Line3D& ps );


    bool operator==( const Line3D& ) const;

    bool operator!=( const Line3D& ) const;

//    Line3D& operator+=( const Point& p );
    Line3D& operator+=( const Point3D& p );

    bool Adjacent( const Region& r ) const;

    inline int NumOfFLOBs() const;
    inline Flob* GetFLOB( const int i );
    inline size_t Sizeof() const;
    size_t HashValue() const;
    void CopyFrom( const Attribute* right );
    int Compare( const Attribute *arg ) const;
    int CompareAlmost( const Attribute *arg ) const;
    bool Adjacent( const Attribute *arg ) const;
    Line3D* Clone() const
    {
      return new Line3D( *this );
    }
    double Distance( const Rectangle<3>& r ) const;

  private:

    void Sort(const bool exact = true);

    void RemoveDuplicates();

//    DbArray<Point> points;
    DbArray<Point3D> points;
};

inline Line3D::Line3D( const int initsize ) :
StandardSpatialAttribute<3>(true),
points( initsize )
{ }

inline Line3D::Line3D( const Line3D& ps ) :
StandardSpatialAttribute<3>(ps.IsDefined()),
points( ps.Size() )
{
  if( IsDefined() ) {
    assert( ps.IsOrdered() );
    points.copyFrom(ps.points);
  }
}

inline const Rectangle<3> Line3D::BoundingBox() const
{
  return new Rectangle<3>(true,0.0,0.0,0.0,0.0,0.0,0.0);
}

/*inline bool Line3D::Get( const int i, Point& p ) const
{
  assert( IsDefined() );
  return points.Get( i, &p );
}*/

inline bool Line3D::Get( const int i, Point3D& p ) const
{
  assert( IsDefined() );
  return points.Get( i, &p );
}

inline int Line3D::Size() const
{
  return points.Size();
}

inline bool Line3D::IsEmpty() const
{
  return !IsDefined() || (points.Size() == 0);
}

inline bool Line3D::IsOrdered() const
{
  return true;
}

inline int Line3D::NumOfFLOBs() const
{
  return 1;
}


inline Flob *Line3D::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &points;
}


inline size_t Line3D::Sizeof() const
{
  return sizeof( *this );
}


inline void Line3D::Resize(const int newSize){
  if(newSize>Size()){
    points.resize(newSize);
  }
}

inline void Line3D::TrimToSize(){
  points.TrimToSize();
}
#endif // __INDOOR_ALGEBRA_H__
