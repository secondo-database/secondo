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

Oct. 2010 Jianqiu Xu Move from IndoorAlgebra to Transportation Mode Algebra 


[TOC]

1 Overview


2 Defines and includes

*/
#ifndef __INDOOR_H__
#define __INDOOR_H__

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
#include "Partition.h"
#include "PaveGraph.h"
#include "GeneralType.h"



#define ARR_SIZE(a) sizeof(a)/sizeof(a[0])

Word InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );

Word
InLine( const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct );



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

/*
3D Line 

*/
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
    {
//      points.print(cout);
    }

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
    double Distance( const Rectangle<3>& r ) const;
    void Print();
    static void* Cast(void* addr){return (new(addr)Line3D());}
    double Length();
    
  private:

    void Sort(const bool exact = true);

    void RemoveDuplicates();

    DbArray<Point3D> points;
};

inline Line3D::Line3D( const int initsize ) :
StandardSpatialAttribute<3>(true),
points( initsize )
{ 
//  cout<<"constructor1 "<<endl; 
//  cout<<"initsize "<<initsize<<endl; 
}

inline Line3D::Line3D( const Line3D& ps ) :
StandardSpatialAttribute<3>(ps.IsDefined()),
points( ps.Size() )
{
//  cout<<"constructor2"<<endl; 
  if( IsDefined() ) {
    assert( ps.IsOrdered() );
    points.copyFrom(ps.points);
  }
}

inline const Rectangle<3> Line3D::BoundingBox() const
{
//  return new Rectangle<3>(true,0.0,0.0,0.0,0.0,0.0,0.0);
//  cout<<"Rectangle<3> BoundingBox "<<endl; 
  double min[3];
  double max[3];
  for(int i = 0;i < 3;i++){
    min[i] = numeric_limits<double>::max();
    max[i] = numeric_limits<double>::min();
  }

  for(int i = 0;i < points.Size();i++){
      Point3D p;
      points.Get(i, p);

      min[0] = MIN(p.GetX(), min[0]);
      min[1] = MIN(p.GetY(), min[1]);
      min[2] = MIN(p.GetZ(), min[2]);
      
      max[0] = MAX(p.GetX(), max[0]);
      max[1] = MAX(p.GetY(), max[1]);
      max[2] = MAX(p.GetZ(), max[2]);
  }

  Rectangle<3> bbox3d(true, min, max);
  return bbox3d; 
}

/*inline bool Line3D::Get( const int i, Point& p ) const
{
  assert( IsDefined() );
  return points.Get( i, &p );
}*/

inline bool Line3D::Get( const int i, Point3D& p ) const
{
  assert( IsDefined() && 0 <= i && i < Size());
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
//        cout<<"Constructor3()"<<endl;
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
    /////////////very important two functions////////////////////
    ////////especially floor3d is an attribute in a relation/////
    inline int NumOfFLOBs() const { return reg.NumOfFLOBs();}
    inline Flob* GetFLOB(const int i) { return reg.GetFLOB(i);}
    /////////////////////////////////////////////////////////////////
    void Print()
    {
      cout<<"height "<<floor_height<<"reg "<<reg<<endl; 
    }
    
private:
    float floor_height;
    Region reg;
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////  Floor3D  ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

ListExpr Floor3DProperty();
ListExpr OutFloor3D(ListExpr typeInfo, Word value);
Word InFloor3D(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct);
void CloseFloor3D(const ListExpr typeInfo, Word& w);
Word CloneFloor3D(const ListExpr typeInfo, const Word& w);
Word CreateFloor3D(const ListExpr typeInfo);
void DeleteFloor3D(const ListExpr typeInfo, Word& w);
int SizeOfFloor3D();
bool CheckFloor3D(ListExpr type, ListExpr& errorInfo);
bool OpenFloor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool SaveFloor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);

/*
data type for door: 
<pos1:genrange pos2:genrange tpstate:mbool lift/non-lift:bool> 

*/
class Door3D:public StandardSpatialAttribute<2>{
  public:
  Door3D(){}  
  Door3D(bool b):StandardSpatialAttribute<2>(true),
  door_pos1(0),door_pos2(0),tpstate(0), lift_door(b){}
  inline Door3D(int id1, int id2, Line& gr1, Line& gr2, MBool& mb, bool b):
  StandardSpatialAttribute<2>(true),
  oid1(id1), oid2(id2), door_pos1(gr1), door_pos2(gr2),
  tpstate(mb), lift_door(b){}

  Door3D(const Door3D& dr):StandardSpatialAttribute<2>(true),
  oid1(dr.oid1), oid2(dr.oid2), door_pos1(dr.door_pos1),
  door_pos2(dr.door_pos2),
  tpstate(dr.tpstate), lift_door(dr.lift_door){}
  Door3D& operator=(const Door3D& dr)
  {
    oid1 = dr.oid1;
    oid2 = dr.oid2; 
    door_pos1 = dr.door_pos1;
    door_pos2 = dr.door_pos2; 
    tpstate = dr.tpstate;
    lift_door = dr.lift_door; 
    return *this; 
  }
  void SetValue(int id1, Line* l1, int id2, Line* l2, MBool* mb, bool b)
  {
      oid1 = id1;
      door_pos1 = *l1;
      oid2 = id2; 
      door_pos2 = *l2;
      tpstate = *mb; 
      lift_door = b; 
      SetDefined(true);
  }
  
  Door3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo);
  inline size_t Sizeof() const{return sizeof(*this);}
  int Compare(const Attribute* arg) const
  {
      return 0;
  }
  inline bool Adjacent(const Attribute* arg)const{return false;}
  Door3D* Clone() const {return new Door3D(*this);}
  inline int Size() const {
    return door_pos1.Size();
  }
  inline bool IsEmpty() const{return !IsDefined() || Size() == 0;}
  size_t HashValue() const
  {
    return (size_t)0;
  }
  
  void CopyFrom(const Attribute* right)
  {
      *this = *(const Door3D*)right;
  }
  const Rectangle<2> BoundingBox() const
  {
      return door_pos1.BoundingBox();
  }
  double Distance(const Rectangle<2>& r)const
  {
      return door_pos1.BoundingBox().Distance(r);
  }
  
  bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo); 

  int GetOid(int i){
    if(i == 1) return oid1;
    if(i == 2) return oid2;
    return 0; 
  }
  Line* GetLoc(int i){
    if(i == 1)
      return &door_pos1;
    if(i == 2)
      return &door_pos2;
    return NULL; 
  }
  
  MBool* GetTState(){return &tpstate;}
  bool GetDoorType(){return lift_door;}

  ~Door3D()
  {
    
  }
  /////////////very important two functions////////////////////
  ////////especially door3d is an attribute in a relation/////
  inline int NumOfFLOBs() const { 
     return door_pos1.NumOfFLOBs() + 
            door_pos2.NumOfFLOBs() + tpstate.NumOfFLOBs();
  }
  inline Flob* GetFLOB(const int i) { 
    if(i < door_pos1.NumOfFLOBs())
      return door_pos1.GetFLOB(i);
    else if(i < (door_pos1.NumOfFLOBs() + door_pos2.NumOfFLOBs())){
      return door_pos2.GetFLOB(i - door_pos1.NumOfFLOBs());
    }  
    else{
      int j = door_pos1.NumOfFLOBs() + door_pos2.NumOfFLOBs();
      return tpstate.GetFLOB(i - j);
    }  
  }
  
  int oid1;
  int oid2;
  Line door_pos1;
  Line door_pos2;
  MBool tpstate; //temporal state 
  bool lift_door; //true:elevator false:non-elevator
};

ListExpr Door3DProperty(); 
ListExpr OutDoor3D( ListExpr typeInfo, Word value ); 
Word InDoor3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
Word CreateDoor3D(const ListExpr typeInfo);
void DeleteDoor3D(const ListExpr typeInfo, Word& w);
bool OpenDoor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool SaveDoor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
void CloseDoor3D( const ListExpr typeInfo, Word& w );
Word CloneDoor3D( const ListExpr typeInfo, const Word& w );
void* CastDoor3D(void* addr);
int SizeOfDoor3D();
bool CheckDoor3D( ListExpr type, ListExpr& errorInfo );

//////////////////////////////////////////////////////////////////////////////
//////////////////// data type for general room /////////////////////////////
////////////////////////////////////////////////////////////////////////////
Word InRegion(const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct ); 
Word InRegion_old( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct );

/*
old version of region input 
pointlist is to store the point in such a way that it is the same as for the
input. 
1) Outercycle in a clock-wise and holes in a counter-clock wise 
2) index list stores the start position of the first point for the outercycle 
and the hole 

*/
Word MyInRegion(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct, 
                int& cycno);


/*
it records the start position in DbArray and number of points 
also the height for these points constructing a region 
One cycle one floorelem. If a region has holes inside, there are several 
elements where the first stores the outer and the rest stores the holes 
But they have the same id (one region). the outer cycle and holes are 
distinguished by value-hole 

*/
struct FloorElem{
  int id;
  int start_pos; 
  int num;//the first is outercycle, and the second, third are holes 
  float h; 
  FloorElem(){}
  FloorElem(int i,int pos, int n, float f):
  id(i), start_pos(pos), num(n), h(f){}
  FloorElem(const FloorElem& fe):
  id(fe.id), start_pos(fe.start_pos), num(fe.num), h(fe.h){}
  FloorElem& operator=(const FloorElem& fe)
  {
    id = fe.id;
    start_pos = fe.start_pos;
    num = fe.num;
    h = fe.h; 
    return *this; 
  }
  void Print()
  {
    cout<<"obj id "<<id<<" start_pos "<<start_pos
        <<"num "<<num<<" height "<<h<<endl; 
  }
};

/*
for all indoor rooms:OR,BR,CO,ST,EL
  type of grooms   
  OR:Office Room (office room,conference room chamber...)
  BR:Bathroom   
  CO:Corridor    
  ST:Staicase  
  EL:Elevator  

*/

const string room_type[] = {"OR", "BR", "CO", "ST", "EL"}; 
enum ROOM_TYPE{OR = 0, BR, CO, ST, EL}; 

inline int GetRoomEnum(string s)
{
//  int tm size = sizeof(str_tm)/sizeof(str_tm[0]);
  int type_size = ARR_SIZE(room_type);
//  cout<<"tm_size "<<tm_size<<endl; 
  for(int i = 0;i < type_size;i++){
      if(room_type[i].compare(s) == 0){
        if(i == 0) return OR;
        if(i == 1) return BR;
        if(i == 2) return CO;
        if(i == 3) return ST;
        if(i == 4) return EL;
      }
  }
  return -1;
}
inline string GetRoomStr(int t)
{
//  int tm size = sizeof(str_tm)/sizeof(str_tm[0]);
  int type_size = ARR_SIZE(room_type); 
  assert(0 <= t && t < type_size);
  return room_type[t];
}


/*
GRoom: a set of 3D regions. the implementation is similar as for GenRange 

*/
class GRoom:public StandardSpatialAttribute<2>{ 
  public:
    GRoom(){}
    GRoom(const int initsize):StandardSpatialAttribute(true),
    elem_list(initsize), seg_list(initsize){}


    ~GRoom()
    {

    }
    GRoom& operator=(const GRoom& gr)
    {
      elem_list.clean();
      seg_list.clean();
//      cout<<"GRoom ="<<endl; 
      GRoom* groom = const_cast<GRoom*>(&gr);
      for(int i = 0;i < groom->RealElemSize();i++){
        FloorElem felem;
        groom->GetElem(i, felem);
        elem_list.Append(felem);
      }
      for(int i = 0;i < groom->SegSize();i++){
        HalfSegment hs;
        groom->GetSeg(i, hs);
        seg_list.Append(hs);
      }
      SetDefined(true);
      return *this; 
    }

    inline size_t Sizeof() const{return sizeof(*this);}
    int Compare(const Attribute* arg) const{ return 0;}
    inline bool Adjacent(const Attribute* arg)const{return false;}
    GRoom* Clone() const {return new GRoom(*this);}
    size_t HashValue() const{return (size_t)0;}
    void CopyFrom(const Attribute* right)
    {
      *this = *(const GRoom*)right;
    }

    inline int Size() const {
//        cout<<"Size "<<endl;
        ////////// holes have the same id as the outer cycle ///////
        ///////   they do not have to be considered  ////////////////
        vector<int> rid_list;
        for(int i = 0;i < elem_list.Size();i++){
          FloorElem felem;
          elem_list.Get(i, felem);
          if(rid_list.size() == 0)
            rid_list.push_back(felem.id);
          else{
            int last_id = rid_list[rid_list.size() - 1];
            if(felem.id != last_id)
              rid_list.push_back(felem.id);
          }
        }
        return rid_list.size(); 
    }
    inline bool IsEmpty() const{return !IsDefined() || Size() == 0;}
    
    int ElemSize(){return Size();}
    int RealElemSize(){return elem_list.Size();}
    int SegSize(){return seg_list.Size();}
    void Add(int id, float h, vector<HalfSegment>&);
    void Get(const int i, float& h, Region& r) const;
    void Clear()
    {
      elem_list.clean();
      seg_list.clean();
    }
    void GetElem(const int i, FloorElem& felem)
    {
      if(0 <= i && i < elem_list.Size())
        elem_list.Get(i, felem);
      else{
        cout<<"not valid index in GetElem()"<<endl;
        assert(false);
      }
    
    }
    void GetSeg(const int i, HalfSegment& hs)
    {
      if(0 <= i && i < seg_list.Size())
        seg_list.Get(i, hs);
      else{
        cout<<"not valid index in GetSeg()"<<endl;
        assert(false);
      }
    }
    void PutSeg(const int i, HalfSegment hs)
    {
      if(0 <= i && i < seg_list.Size())
        seg_list.Put(i, hs);
      else{
        cout<<"not valid index in PutSeg()"<<endl;
        assert(false);
      }
    }
    void AddHeight(float h)
    {
      for(int i = 0;i < elem_list.Size();i++){
        FloorElem felem;
        elem_list.Get(i, felem);
        felem.h += h;
        elem_list.Put(i, felem);
      }
    }
    void Translate(const Coord& x, const Coord& y, GRoom& result);
    void GetRegion(Region& r); //2D area covered by the room 
    const Rectangle<2> BoundingBox() const
    {
      Rectangle<2> bbox;
      for( int i = 0; i < Size(); i++ ){
        Region r(0);
        float h;
        Get( i, h, r);
        if( i == 0 ){
          bbox = r.BoundingBox();
        }else
          bbox = bbox.Union(r.BoundingBox());
      }

      return bbox;
    }
    double Distance(const Rectangle<2>& r)const
   {
      return BoundingBox().Distance(r);
   }
   float GetLowHeight();
   float GetHighHeight();
   
   
  /////////////very important two functions////////////////////
  ////////especially genrange is an attribute in a relation/////
  inline int NumOfFLOBs() const { 
    return 2;
  }
  inline Flob* GetFLOB(const int i) { 
//    cout<<"GetFLOB"<<endl; 
     if(i < 1)
      return &elem_list;
    else 
      return &seg_list;
  }
  
  void Print(); 
  private:
    DbArray<FloorElem> elem_list;//one cycle one element, reg id starts from 0
    DbArray<HalfSegment> seg_list; 
};

ListExpr GRoomProperty();
ListExpr OutGRoom( ListExpr typeInfo, Word value );
Word InGRoom( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );

bool OpenGRoom(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);

bool SaveGRoom(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
Word CreateGRoom(const ListExpr typeInfo);
void DeleteGRoom(const ListExpr typeInfo, Word& w);
void CloseGRoom( const ListExpr typeInfo, Word& w );
Word CloneGRoom( const ListExpr typeInfo, const Word& w );
void* CastGRoomD(void* addr);
int SizeOfGRoom();
bool CheckGRoom( ListExpr type, ListExpr& errorInfo );



/*
for indoor navigation 

*/

struct MySegHeight:public MyHalfSegment{
  float h;
  MySegHeight(){ h = 0.0;}
  MySegHeight(bool def, const Point& p1, const Point& p2, double d):
            MyHalfSegment(def,p1,p2), h(d){}
  MySegHeight(const MySegHeight& msd):MyHalfSegment(msd), h(msd.h){}
  MySegHeight& operator=(const MySegHeight& msd)
  {
      MyHalfSegment::operator=(msd);
      h = msd.h;
      return *this;
  }
  bool operator<(const MySegHeight& msd) const
  {
//    cout<<"from1 "<<from<<" to1 "<<to<<endl;
//    cout<<"from2 "<<msd.from<<" to2 "<<msd.to<<endl;
    bool result = h < msd.h;
//    cout<<"< "<<result<<endl;
    return result;
  }

};

class IndoorGraph; 
struct IPath_elem; 

struct I_Parameter{
  int num_floors;
  float floor_height;
  float speed_person;
  float speed_elevator; 
  I_Parameter(){}
  I_Parameter(int n, float h, float v1, float v2):
  num_floors(n), floor_height(h), speed_person(v1), speed_elevator(v2){}
  I_Parameter(const I_Parameter& ip):num_floors(ip.num_floors),
  floor_height(ip.floor_height), speed_person(ip.speed_person),
  speed_elevator(ip.speed_elevator){}
  I_Parameter& operator=(const I_Parameter& ip)
  {
    num_floors = ip.num_floors;
    floor_height = ip.floor_height;
    speed_person = ip.speed_person;
    speed_elevator = ip.speed_elevator;
    return *this; 
  }
  void Print()
  {
    cout<<"num of floors "<<num_floors<<" height "<<floor_height
        <<"speed_person "<<speed_person
        <<"speed_elevator "<<speed_elevator<<endl; 
  }
}; 
struct IndoorNav{
  Relation* rel1; //university room relation 
  Relation* rel2; //door 3d box relation 
  IndoorGraph* ig; 
  
  vector<int> oid_list; 
  vector<int> tid_list; 
  vector<Rectangle<3> > box_list; 
  
  vector<Door3D> door_list; 
  vector<Line> line_list;
  vector<int> groom_id_list1;
  vector<int> groom_id_list2; 
  vector<float> door_heights; 
  
  
  vector<int> groom_oid_list; 
  vector<unsigned int> door_tid_list1;
  vector<unsigned int> door_tid_list2; 
  vector<Line3D> path_list; 
  
  
  vector<GenLoc> genloc_list;
  vector<Point3D> p3d_list; 
  

  vector<GRoom> room_list; 
  vector<double> cost_list; 
  
  int type; 
  /////////////the attribute position for indoor (groom+door) relation 
  static string Indoor_GRoom_Door; 
  enum GROOM_REL{I_OID = 0, I_Name, I_Type, I_Room, I_Door};
  
  unsigned int count;
  TupleType* resulttype;
  
  IndoorNav(){count = 0; resulttype = NULL;}
  IndoorNav(Relation* r1, Relation* r2):
  rel1(r1), rel2(r2)
  { count = 0; 
    resulttype = NULL;
  }

  IndoorNav(IndoorGraph* g):rel1(NULL), rel2(NULL), ig(g), 
  count(0), resulttype(NULL)
  {
  }

  ~IndoorNav(){if(resulttype != NULL) delete resulttype;}

  void CreateLine3D(int oid, Line* l, float h);
  void CreateDoor3D();
  ///////////////////build 3d box on each door //////////////////////////
  
  void CreateDoorBox();
  void CreateBox3D(int, int, Line*, float);
  float NextFloorHeight(float h, vector<float>& floor_height);
  ////////////////create a relation storing door////////////////////////
  void CreateDoor1(R_Tree<3, TupleId>*, int, int ,int);
  void CreateDoorState(MBool* mb);
  void CreateDoor2();
  float GetVirtualDoor1(GRoom* groom, Line* l1, Line* l2, Line3D* l3d);
  float GetVirtualDoor2(GRoom* groom, Line* l1, Line* l2, Line3D* l3d);
  void DFTraverse(R_Tree<3,TupleId>* rtree, SmiRecordId adr, unsigned int id, 
                  Rectangle<3>* bbox3d, int attr1, int attr2, 
                  int attr3, vector<TupleId>& id_list); 
  bool BBox3DEqual(Rectangle<3>* bbox3d, Rectangle<3>* bbox_3d);
  void CreateResDoor(int id, int oid, int tid, vector<TupleId> id_list, 
                     int attr1, int attr2, int attr3);
  void GRoomDoorLine(Rectangle<3>* bbox3d_1, Rectangle<3>* bbox3d_2, 
                     Line* l1, Line* l2, Line* l3, 
                     const Rectangle<2>*, const Rectangle<2>*, 
                     Line3D* l, float h);
   ////////////////create a relation storing edges connecting doors////////////
   void CreateAdjDoor1(BTree*, int, int ,int, int);
   void CreateAdjDoor2(R_Tree<3,TupleId>*);
   void DFTraverse(R_Tree<3,TupleId>* rtree, SmiRecordId adr, unsigned int id,
                   Line3D* l, vector<TupleId>& id_list, int groom_oid);
   
   void BuildPathEL(int groom_oid, GRoom* groom, vector<int> tid_list, 
                            int attr1, int attr2, 
                            int attr3, int attr4);
   void BuildPathST(int groom_oid, GRoom* groom, vector<int> tid_list, 
                            int attr1, int attr2, 
                            int attr3, int attr4);
   void ST_ConnectOneFloor(int groom_oid, GRoom* groom, Line* l1, 
                                   Line* l2, int tid1, int tid2, float h);
   void FindPathInRegion(GRoom* groom, float h, 
                         vector<MyHalfSegment>& mhs, Point* p1, Point* p2); 
   void ST_ConnectFloors(int groom_oid, GRoom* groom, Line* l1, 
                                   Line* l2, int tid1, int tid2, 
                         float h1, float h2, vector<MySegHeight>&); 

   void ConstructMiddlePath(GRoom* groom, vector<MySegHeight>& middle_path); 
   void BuildPathORAndCO(int groom_oid, GRoom* groom, 
                  vector<int> tid_list, int attr1, int attr2, 
                  int attr3, int attr4);
   /////////////////////////////////////////////////////////////////////
   void GetAdjNodeIG(int oid);
   ////////////////////////// data generation/////////////////////////
   void GenerateIP1(int num);
   //////////////////////////shortest path searching////////////////////
   bool IsLocEqual(GenLoc* loc1, GenLoc* loc2, Relation* rel);
   void PathInOneRoom(GenLoc* gloc1, GenLoc* gloc2, Relation* rel, 
                      BTree* btree); 
   void ShortestPath_Length(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree);

   void ConnectStartLoc(GenLoc* gloc, int door_tid, Relation* rel, 
                        BTree* btree, Line3D* l3d_start); 
   void Path_StartDoor_EndLoc(int id, GenLoc* d, 
                           vector<Line3D>& candidate_path, Line3D* s,
                           Relation* rel, BTree* btree);
   void ConnectEndLoc(GenLoc* gloc, int door_tid, Relation* rel, BTree* btree,
                        Line3D* l3d_end); 
   void IndoorShortestPath(int id1, int id2, 
                           vector<Line3D>& candidate_path, 
                           Line3D* s, Line3D* d);
   void InitializeQueue(int id, Point3D* start_p, Point3D* end_p, 
                        priority_queue<IPath_elem>& path_queue, 
                        vector<IPath_elem>& expand_path);
   bool MiddlePoint(Line3D* l, Point3D& p); 
   ////////////////////////////////////////////////////////////////////
   /////////////////////////minimum number of rooms////////////////////
   ////////////////////////////////////////////////////////////////////
   void ShortestPath_Room(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree);
   void Path_StartDoor_EndLoc_Room(int id, GenLoc* d, 
                           vector< vector<TupleId> >& candidate_path, 
                                   int stid, int etid);
   void IndoorShortestPath_Room(int id1, int id2,
                                vector< vector<TupleId> >& candidate_path,
                                int s_tid, int e_tid); 
   /////////////////////////////////////////////////////////////////////
   ///////////////////////minimum travelling time///////////////////////
   /////////////////////////////////////////////////////////////////////
   void ShortestPath_Time(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree);
   void Path_StartDoor_EndLoc_Time(int id, GenLoc* d, 
                           vector<Line3D>& candidate_path, Line3D* s,
                           Relation* rel, BTree* btree, vector<double>&,
                                   I_Parameter& param);
   void IndoorShortestPath_Time1(int id1, int id2, 
                           vector<Line3D>& candidate_path, 
                           Line3D* s, Line3D* d, vector<double>& timecost,
                           I_Parameter& param, Relation* rel, BTree* btree);
   void IndoorShortestPath_Time2(int id1, int id2, 
                           vector<Line3D>& candidate_path, 
                           Line3D* s, Line3D* d, vector<double>& timecost,
                           I_Parameter& param, Relation* rel, BTree* btree);
   bool IsElevator(int groom_oid, Relation* rel, BTree* btree); 
   float CostInElevator(double l, I_Parameter& param); 
   float SetTimeWeight(double l, int groom_oid, Relation* rel, 
                       BTree* btree, I_Parameter& param, vector<float>& h_list);
};

/*
for indoor shortest path searching 

*/
struct IPath_elem:public Path_elem{
  double weight;
  double real_w;
  Line3D path; 
  int groom_oid; 
  IPath_elem():path(0){}
  IPath_elem(int p, int c, int t, double w, double w2, Line3D& l, int gid = 0):
  Path_elem(p, c, t), weight(w), real_w(w2), path(l), groom_oid(gid){}
  IPath_elem(const IPath_elem& wp):Path_elem(wp),
            weight(wp.weight),real_w(wp.real_w), 
            path(wp.path), groom_oid(wp.groom_oid){}
  IPath_elem& operator=(const IPath_elem& wp)
  {
//    cout<<"SPath_elem ="<<endl;
    Path_elem::operator=(wp);
    weight = wp.weight;
    real_w = wp.real_w;
    path = wp.path;  
    groom_oid = wp.groom_oid; 
    return *this;
  }
  bool operator<(const IPath_elem& ip) const
  {
    return weight > ip.weight;
  }

  void Print()
  {
    cout<<" tri_index " <<tri_index<<" realweight "<<real_w
        <<" weight "<<weight<<" Path "<<endl;
//    path.Print();
//    cout<<endl; 
  }
};

ostream& operator<<(ostream& o, const IPath_elem& elem); 


struct PointAndID{
  int pid; 
  Point loc; 
  PointAndID(){}
  PointAndID(int id, Point& q):pid(id), loc(q){}
  PointAndID(const PointAndID& paid):
  pid(paid.pid), loc(paid.loc){}
  PointAndID& operator=(const PointAndID& paid)
  {
    pid = paid.pid; 
    loc = paid.loc; 
    return *this; 
  }
  void Print()
  {
    cout<<" pid "<<pid<<" loc "<<loc<<endl; 
  }
};

/*
structure used for the priority queue for shortest path searching 

*/
struct RPath_elem:public Path_elem{
  double weight;
  double real_w; 
  RPath_elem(){}
  RPath_elem(int p, int c, int t, double w1, double w2):
  Path_elem(p, c, t), weight(w1), real_w(w2){}
  RPath_elem(const RPath_elem& se):Path_elem(se),
                       weight(se.weight), real_w(se.real_w){}
  RPath_elem& operator=(const RPath_elem& se)
  {
//    cout<<"SPath_elem ="<<endl;
    Path_elem::operator=(se);
    weight = se.weight;
    real_w = se.real_w; 
    return *this;
  }
  bool operator<(const RPath_elem& se) const
  {
    return weight > se.weight;
  }

  void Print()
  {
    cout<<"prev "<<prev_index<<" cur "<<cur_index
        <<" tri_index " <<tri_index<<
        " weight1 "<<weight<<" weight2 "<<real_w<<endl;
  }
};


/*
compute the shorest path inside a region where the region can be 
convex with holes or  concave with holes i

*/

void ShortestPath_InRegion(Region* reg, Point* s, Point* d, Line* pResult);
void InitializeQueue(Region* reg, priority_queue<RPath_elem>& path_queue, 
                     vector<RPath_elem>& expand_queue,
                     PointAndID start_loc, PointAndID end_loc,
                     vector<PointAndID>& ps_list,
                     vector<HalfSegment>& seg_list); 
void FindAdj(Region* reg, PointAndID top, vector<bool>& visit_flag, 
             vector<int>& adj_list, vector<PointAndID>& ps_list,
             vector<HalfSegment>& seg_list); 
bool SegAvailable(HalfSegment hs, vector<HalfSegment>& set_list); 
void GetBoundaryPoints(Region* r, vector<Point>& ps, unsigned int); 


/*
Indoor graph for navigation 

*/

class IndoorGraph: public BaseGraph{
public:
  static string NodeTypeInfo;
  static string EdgeTypeInfo;
  static string NodeBTreeTypeInfo;
  
  enum IGNodeTypeInfo{I_DOOR = 0, I_DOOR_LOC, I_GROOM_OID1, 
                      I_GROOM_OID2, I_DOOR_LOC_3D, I_DOOR_HEIGHT};
  enum IGEdgeTypeInfo{I_GROOM_OID = 0,I_DOOR_TID1, I_DOOR_TID2, I_PATH};

  //////////////////////////////////////////////////////////////
  ~IndoorGraph();
  IndoorGraph();
  IndoorGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect);
  IndoorGraph(SmiRecord&, size_t&, const ListExpr);
  //////////////////////////////////////////////////////////////
  void Load(int, Relation*,Relation*);
  static ListExpr OutIndoorGraph(ListExpr typeInfo, Word value);
  ListExpr Out(ListExpr typeInfo);
  static bool CheckIndoorGraph(ListExpr type, ListExpr& errorInfo);
  static void CloseIndoorGraph(const ListExpr typeInfo, Word& w);
  static void DeleteIndoorGraph(const ListExpr typeInfo, Word& w);
  static Word CreateIndoorGraph(const ListExpr typeInfo);
  static Word InIndoorGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect);
  static bool OpenIndoorGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
  static IndoorGraph* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);
  static bool SaveIndoorGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
  bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo);

  BTree* GetBTree(){return btree_node;}
  private:
    BTree* btree_node; //btree on node relation 


};



#endif // __INDOOR_H__
