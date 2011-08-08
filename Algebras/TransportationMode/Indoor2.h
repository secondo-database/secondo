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

May, 2011 Jianqiu Xu

[TOC]

1 Overview


2 Defines and includes

*/
#ifndef __INDOOR2_H__
#define __INDOOR2_H__

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
#include "../Temporal/TemporalAlgebra.h"


#include "NestedList.h"

#include "QueryProcessor.h"
#include "RTreeAlgebra.h"
#include "BTreeAlgebra.h"
#include "TemporalAlgebra.h"
#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "NetworkAlgebra.h"
#include "SpatialAlgebra.h"

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
    inline const Rectangle<3> BoundingBox(const Geoid* geoid = 0) const
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

    double Distance(const Rectangle<3>& r, const Geoid* geoid=0)const
    {
      return BoundingBox().Distance(r);
    }
    double Distance(const Point3D& p3d, const Geoid* geoid=0) const
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
    static const string BasicType(){
       return "point3d";
    } 
    void Print() const
    {
      cout<<"x "<<x<<" y "<<y<<" z "<<z<<endl;
    }
    double GetX(){return x;}
    double GetY(){return y;}
    double GetZ(){return z;}
private:
    double x;
    double y;
    double z;
};

ListExpr OutPoint3D(ListExpr typeInfo, Word value); 
Word InPoint3D(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct); 
ListExpr Point3DProperty(); 
Word CreatePoint3D(const ListExpr typeInfo); 
void DeletePoint3D(const ListExpr typeInfo, Word& w);
void ClosePoint3D(const ListExpr typeInfo, Word& w);
Word ClonePoint3D(const ListExpr typeInfo, const Word& w); 
void* CastPoint3D(void* addr);
int SizeOfPoint3D();
bool CheckPoint3D(ListExpr type, ListExpr& errorInfo);
bool SavePoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool OpenPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
ostream& operator<<(ostream& o, const Point3D& loc); 

/*
3D Line 

*/

class Line3D: public StandardSpatialAttribute<3>
{
  public:

    Line3D() {}

    Line3D( const int initsize );

    Line3D( const Line3D& ps);

    inline void Destroy()
    {
      points.destroy();
    }

    ~Line3D()
    {
//      points.print(cout);
    }

    inline bool IsOrdered() const;

    void StartBulkLoad();

    void EndBulkLoad( bool sort = true, bool remDup = true, bool trim = true );

    const Rectangle<3> BoundingBox(const Geoid* geoid=0) const;

    inline bool IsEmpty() const;

    bool IsValid() const;

    inline int Size() const;

    void Clear();

    inline void Resize(const int newSize);

    inline void TrimToSize();

    inline bool Get( const int i, Point3D& p ) const;

    Line3D& operator=( const Line3D& ps );


    bool operator==( const Line3D& l) const;

    bool operator!=( const Line3D& l) const;


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
    double Distance( const Rectangle<3>& r,const Geoid* geoid=0 ) const;
    void Print();
    static void* Cast(void* addr){return (new(addr)Line3D());}
    double Length();
    
    static const string BasicType(){
       return "line3d";
    }
    
  private:

    void Sort(const bool exact = true);

    void RemoveDuplicates();

    DbArray<Point3D> points;
};

ListExpr OutLine3D( ListExpr typeInfo, Word value ); 
Word InLine3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 
Word CreateLine3D( const ListExpr typeInfo );
void DeleteLine3D( const ListExpr typeInfo, Word& w );
void CloseLine3D( const ListExpr typeInfo, Word& w );
Word CloneLine3D( const ListExpr typeInfo, const Word& w );
bool OpenLine3D( SmiRecord& valueRecord, size_t& offset,
            const ListExpr typeInfo, Word& value );
bool SaveLine3D( SmiRecord& valueRecord, size_t& offset,
            const ListExpr typeInfo, Word& value );
int SizeOfLine3D();
ListExpr Line3DProperty();
bool CheckLine3D( ListExpr type, ListExpr& errorInfo );


/////////////////////////////////////////////////////////////////////
///////////// temporal unit: UPoint3D ///////////////////////////////
///////////// only for Java3D visualization ////////////////////////
/////////////////////////////////////////////////////////////////////
class UPoint3D: public SpatialTemporalUnit<Point3D, 4>
{
  public:
  UPoint3D(){}; 
  UPoint3D(bool def):SpatialTemporalUnit<Point3D, 4>(def){}
  UPoint3D(const Interval<Instant>& interval, const Point3D& loc1, 
          const Point3D& loc2):
  SpatialTemporalUnit<Point3D, 4>(interval),p0(loc1), p1(loc2)
  {
    SetDefined(p0.IsDefined() && p1.IsDefined()); 
  }
  UPoint3D(const UPoint3D& source):
  SpatialTemporalUnit<Point3D, 4>(source.IsDefined())
  {
    timeInterval = source.timeInterval; 
    p0 = source.p0;
    p1 = source.p1;
    del.refs = 1;
    del.SetDelete(); 
    del.isDefined = source.del.isDefined; 
  }
  UPoint3D& operator=(const UPoint3D& loc)
  {
    timeInterval = loc.timeInterval;
    p0 = loc.p0;
    p1 = loc.p1;
    del.isDefined = loc.del.isDefined;
    return *this; 
  }

  void TemporalFunction( const Instant& t,
                               Point3D& result,
                               bool ignoreLimits ) const; 
  bool Passes( const Point3D& gloc ) const; 
  bool At( const Point3D& p, TemporalUnit<Point3D>& res ) const; 

  static void* Cast(void* addr){return new (addr)UPoint3D();}
  inline size_t Sizeof() const { return sizeof(*this);}
  UPoint3D* Clone() const;
  void CopyFrom(const Attribute* right); 
  const Rectangle<4> BoundingBox(const Geoid* geoid=0) const; 
  double Distance(const Rectangle<4>& rect, const Geoid* geoid=0) const
  {
    return BoundingBox().Distance(rect); 
  }
  inline bool IsEmpty() const
  {
    return !IsDefined(); 
  }
  Point3D p0;
  Point3D p1; 
};

ListExpr UPoint3DProperty(); 
bool OpenUPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);

bool SaveUPoint3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
Word CreateUPoint3D(const ListExpr typeInfo);
void DeleteUPoint3D(const ListExpr typeInfo, Word& w); 
void CloseUPoint3D( const ListExpr typeInfo, Word& w ); 
Word CloneUPoint3D( const ListExpr typeInfo, const Word& w ); 
int SizeOfUPoint3D(); 
bool CheckUPoint3D( ListExpr type, ListExpr& errorInfo ); 
Word InUPoint3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 
ListExpr OutUPoint3D( ListExpr typeInfo, Word value ); 
/////////////////////////////////////////////////////////////////////
///////////////////   MPoint3D   ///////////////////////////////////
/////////////////////////////////////////////////////////////////////

class MPoint3D:public Mapping<UPoint3D,Point3D>
{
  public:
    MPoint3D(){}
    MPoint3D(const int n):Mapping<UPoint3D, Point3D>(n)
    {
      del.refs = 1;
      del.SetDelete();
      del.isDefined = true;
    }
    void Clear();
    void CopyFrom(const Attribute* right); 
    Attribute* Clone() const; 
    void Add(const UPoint3D& unit); 
    void EndBulkLoad(const bool sort = true, const bool checkvalid = false);
    void Trajectory(Line3D& l);
    static const string BasicType(){
       return "mpoint3d";
    }
};


bool CheckMPoint3D( ListExpr type, ListExpr& errorInfo );
ListExpr MPoint3DProperty();



#endif // __INDOOR2_H__
