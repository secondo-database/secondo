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

This header file essentially contains the definition of the classes ~Point~,
~Points~, ~Line~, and ~Region~ used in the Spatial Algebra. These classes
respectively correspond to the memory representation for the type constructors
~point~, ~points~, ~line~, and ~region~.  Figure \cite{fig:spatialdatatypes.eps}
shows examples of these spatial data types.

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

Word InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
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
private:
    double x;
    double y;
    double z;
};

Point3D::Point3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
  valueRecord.Read(&x, sizeof(double), offset);
  offset += sizeof(double);
  valueRecord.Read(&y, sizeof(double), offset);
  offset += sizeof(double);
  valueRecord.Read(&z, sizeof(double), offset);
  offset += sizeof(double);
  SetDefined(true);
}

bool Point3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Point3D::Save()"<<endl;
  valueRecord.Write(&x, sizeof(double),offset);
  offset += sizeof(double);
  valueRecord.Write(&y, sizeof(double),offset);
  offset += sizeof(double);
  valueRecord.Write(&z, sizeof(double), offset);
  offset += sizeof(double);
//  cout<<x<<" "<<y<<" "<<z<<endl;
  return true;
}

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

double Line3D::Distance( const Rectangle<3>& r ) const
{
  return 0.0;
}

void Line3D::StartBulkLoad()
{

}

void Line3D::EndBulkLoad( bool sort, bool remDup, bool trim )
{
  if( !IsDefined() ) {
    Clear();
    SetDefined( false );
  }

  if( sort ){
    Sort();
  }

  if( remDup ){
    RemoveDuplicates();
  }
  if(trim){
    points.TrimToSize();
  }
}


void Line3D::RemoveDuplicates()
{

}

void Line3D::Sort(const bool exact /*= true*/)
{

}

void Line3D::Clear()
{
  points.clean();
}


Line3D& Line3D::operator+=( const Point3D& p )
{
  if(!IsDefined()){
    return *this;
  }
  points.Append(p);
  return *this;
}

bool Line3D::IsValid() const
{
  return true;
}

size_t Line3D::HashValue() const
{
  return (size_t)0.0;
}

void Line3D::CopyFrom( const Attribute* right )
{
  const Line3D *ps = (const Line3D*)right;
  assert( ps->IsOrdered() );
  *this = *ps;
}

int Line3D::Compare( const Attribute* arg ) const
{
  return 0;
}

int Line3D::CompareAlmost( const Attribute* arg ) const
{
  return 0;
}

bool Line3D::Adjacent( const Attribute* arg ) const
{
  return 0;
}

Line3D& Line3D::operator=( const Line3D& ps )
{
  assert( ps.IsOrdered() );
  points.copyFrom(ps.points);
  SetDefined(ps.IsDefined());
  return *this;
}


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
private:
    float floor_height;
    Region reg;
};


/*
Constructor function for Floor3D

*/
Floor3D::Floor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo):
StandardSpatialAttribute<2>(true),reg(0)
{
//  cout<<"Floor3D(SmiRecord& , size_t& , const ListExpr) here"<<endl;
  valueRecord.Read(&floor_height, sizeof(float), offset);
  offset += sizeof(float);
  ListExpr xType = nl->SymbolAtom("region");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Region* r = (Region*)Attribute::Open(valueRecord,offset,xNumericType);
  reg = *r;
//  cout<<*r<<endl; 
  delete r;
  SetDefined(true);
}

bool Floor3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Save()"<<endl;
  valueRecord.Write(&floor_height, sizeof(float),offset);
  offset += sizeof(float);
  ListExpr xType = nl->SymbolAtom("region");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &reg);
  return true;
}


/*
data type for door: 
<pos1:genrange pos2:genrange tpstate:mbool lift/non-lift:bool> 

*/
class Door3D:public StandardSpatialAttribute<2>{
  public:
  Door3D(){}  
  Door3D(bool b):StandardSpatialAttribute<2>(true),
  door_pos1(0),door_pos2(0),tpstate(0), lift_door(b){}
  inline Door3D(GenRange& gr1,GenRange& gr2, MBool& mb, bool& b):
  StandardSpatialAttribute<2>(true),
  door_pos1(gr1),door_pos2(gr2),tpstate(mb), lift_door(b){}

  Door3D(const Door3D& dr):StandardSpatialAttribute<2>(true),
  door_pos1(dr.door_pos1),door_pos2(dr.door_pos2),
  tpstate(dr.tpstate), lift_door(dr.lift_door){}
  Door3D& operator=(const Door3D& dr)
  {
    door_pos1 = dr.door_pos1;
    door_pos2 = dr.door_pos2; 
    tpstate = dr.tpstate;
    lift_door = dr.lift_door; 
    return *this; 
  }
  void SetValue(GenRange* gr1,GenRange* gr2, MBool* mb, bool b)
  {
      door_pos1 = *gr1;
      door_pos2 = *gr2;
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

  GenRange* GetLoc(int i){
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
            door_pos2.NumOfFLOBs() + 
            tpstate.NumOfFLOBs();
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

  GenRange door_pos1; //position in one room
  GenRange door_pos2; //position in another room
  MBool tpstate; //temporal state 
  bool lift_door; //true:elevator false:non-elevator
};



/*
Constructor function for Door3D

*/
Door3D::Door3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo):StandardSpatialAttribute<2>(true),
                 door_pos1(0),door_pos2(0), tpstate(0)
{
//  cout<<"Door3D(SmiRecord& , size_t& , const ListExpr) here"<<endl;
  
  valueRecord.Read(&door_pos1.oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);
  ListExpr xType = nl->SymbolAtom("line");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Line* l1 = (Line*)Attribute::Open(valueRecord,offset,xNumericType);
  door_pos1.loc_range = *l1;
  delete l1;
  
  valueRecord.Read(&door_pos2.oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);
  Line* l2 = (Line*)Attribute::Open(valueRecord,offset,xNumericType);
  door_pos2.loc_range = *l2;
  delete l2;

/*  ListExpr xType = nl->SymbolAtom("genrange");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  GenRange* gr1 = (GenRange*)Attribute::Open(valueRecord,offset,xNumericType);
  door_pos1 = *gr1;

  GenRange* gr2 = (GenRange*)Attribute::Open(valueRecord,offset,xNumericType);
  door_pos2 = *gr2;
  cout<<gr1->GetObjId()<<" "<<gr2->GetObjId()<<endl; */
  
  xType = nl->SymbolAtom("mbool");
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  MBool* mb = (MBool*)Attribute::Open(valueRecord,offset,xNumericType);
  tpstate = *mb; 
//  cout<<tpstate<<endl; 
  
  
  valueRecord.Read(&lift_door, sizeof(bool), offset);
  offset += sizeof(bool);

  SetDefined(true);
}

bool Door3D::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo)
{
//  cout<<"Save()"<<endl;
  
/*  ListExpr xType = nl->SymbolAtom("genrange");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &door_pos1);
  Attribute::Save(valueRecord, offset, xNumericType, &door_pos2);*/

  valueRecord.Write(&door_pos1.oid, sizeof(unsigned int),offset);
  offset += sizeof(unsigned int);
  ListExpr xType = nl->SymbolAtom("line");
  ListExpr xNumericType =
    SecondoSystem::GetCatalog()->NumericType(xType);
  Line* l1 = const_cast<Line*>( door_pos1.GetLine());  
  Attribute::Save(valueRecord, offset, xNumericType,l1);


  valueRecord.Write(&door_pos2.oid, sizeof(unsigned int),offset);
  offset += sizeof(unsigned int);
  Line* l2 = const_cast<Line*>( door_pos2.GetLine());  
  Attribute::Save(valueRecord, offset, xNumericType,l2);


  xType = nl->SymbolAtom("mbool");
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  Attribute::Save(valueRecord, offset, xNumericType, &tpstate);
  
  valueRecord.Write(&lift_door, sizeof(bool), offset);
  offset += sizeof(bool);
  return true;
}
/*
Door3D Property function 

*/
ListExpr Door3DProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("door3d"),
           nl->StringAtom("(<door3d>) (genrange genrange mbool bool)"),
           nl->StringAtom("(doorpos1 doorpos2 doorstat type)"))));
}


ListExpr OutDoor3D( ListExpr typeInfo, Word value )
{
//  cout<<"OutDoor3D()"<<endl; 
  Door3D* dr = (Door3D*)(value.addr);
  if(!dr->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( dr->IsEmpty() ){
    return nl->TheEmptyList();
  }
  ListExpr genrange1 = OutGenRange(nl->TheEmptyList(), SetWord(dr->GetLoc(1)));
  ListExpr genrange2 = OutGenRange(nl->TheEmptyList(), SetWord(dr->GetLoc(2)));
  ListExpr tempstate = OutMapping<MBool,UBool,
  OutConstTemporalUnit<CcBool,OutCcBool> >(nl->TheEmptyList(), 
                                          SetWord(dr->GetTState()));
  
  return nl->FourElemList(genrange1,genrange2,tempstate,
                          nl->BoolAtom(dr->GetDoorType()));
  
}


/*
In function

*/
Word InDoor3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      Door3D* dr = new Door3D();
      dr->SetDefined(false);
      correct=true;
      return SetWord(Address(dr));
  }
  ////////////////////////////////////////////////////////////////////////
  /////////////////////// get the first genrange /////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ListExpr first_instance = nl->First(instance); 
  ListExpr oid_list1 = nl->First(first_instance);

  if(!nl->IsAtom(oid_list1) || nl->AtomType(oid_list1) != IntType){
    string strErrorMessage = "genrange(): obj id must be int type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }
 
  unsigned int oid1 = nl->IntValue(oid_list1);
  ListExpr LineNL1 = nl->Second(first_instance);
  Line* l1 = new Line( 0 );
  HalfSegment * hs;
  l1->StartBulkLoad();
  ListExpr first, halfseg, halfpoint;
  ListExpr rest = first_instance;
  int edgeno = 0;

  while( !nl->IsEmpty( rest )){
      first = nl->First( LineNL1 );
      rest = nl->Rest( LineNL1 );

      if( nl->ListLength( first ) == 4 )
      {
        halfpoint = nl->TwoElemList(
                      nl->TwoElemList(
                        nl->First(first),
                        nl->Second(first)),
                      nl->TwoElemList(
                        nl->Third(first),
                        nl->Fourth(first)));
      } else { // wrong list representation
         l1->DeleteIfAllowed();
         correct = false;
         return SetWord( Address(0) );
      }
      halfseg = nl->TwoElemList(nl->BoolAtom(true), halfpoint);
      hs = (HalfSegment*)InHalfSegment( nl->TheEmptyList(), halfseg,
                                        0, errorInfo, correct ).addr;
      if( correct )
      {
        hs->attr.edgeno = edgeno++;
        *l1 += *hs;
        hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
        *l1 += *hs;
      }
      delete hs;
  }
  l1->EndBulkLoad();

  correct = true;
  GenRange* gr1 = new GenRange(oid1, *l1);
  delete l1; 
  ///////////////////////////////////////////////////////////////////////////
  ////////////////////////  get the second genrange /////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ListExpr second_instance = nl->Second(instance); 
  ListExpr oid_list2 = nl->First(second_instance);

  if(!nl->IsAtom(oid_list2) || nl->AtomType(oid_list2) != IntType){
    string strErrorMessage = "genrange(): obj id must be int type";
    errorInfo = nl->Append(errorInfo,nl->StringAtom(strErrorMessage));
    correct = false;
    return SetWord(Address(0));
  }

  unsigned int oid2 = nl->IntValue(oid_list2);
  ListExpr LineNL2 = nl->Second(second_instance);
  Line* l2 = new Line( 0 );
  
  l2->StartBulkLoad();
  rest = second_instance;
  edgeno = 0;
  while( !nl->IsEmpty( rest )){
      first = nl->First( LineNL2 );
      rest = nl->Rest( LineNL2 );

      if( nl->ListLength( first ) == 4 )
      {
        halfpoint = nl->TwoElemList(
                      nl->TwoElemList(
                        nl->First(first),
                        nl->Second(first)),
                      nl->TwoElemList(
                        nl->Third(first),
                        nl->Fourth(first)));
      } else { // wrong list representation
         l2->DeleteIfAllowed();
         correct = false;
         return SetWord( Address(0) );
      }
      halfseg = nl->TwoElemList(nl->BoolAtom(true), halfpoint);
      hs = (HalfSegment*)InHalfSegment( nl->TheEmptyList(), halfseg,
                                        0, errorInfo, correct ).addr;
      if( correct )
      {
        hs->attr.edgeno = edgeno++;
        *l2 += *hs;
        hs->SetLeftDomPoint( !hs->IsLeftDomPoint() );
        *l2 += *hs;
      }
      delete hs;
  }
  l2->EndBulkLoad();

  correct = true;
  GenRange* gr2 = new GenRange(oid2, *l2);
  delete l2; 
  ////////////////////////////////////////////////////////////////////////
  /////////////  time-dependent state mbool //////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ListExpr temporal_state = nl->Third(instance);
  int numUnits = 0;
  if(nl->AtomType(temporal_state)==NoAtom){
    numUnits = nl->ListLength(temporal_state);
  }
  MBool* m = new MBool( numUnits );
  correct = true;
  int unitcounter = 0;
  string errmsg;

  m->StartBulkLoad();

  rest = temporal_state;
  if (nl->AtomType( rest ) != NoAtom)
  { if(nl->IsEqual(rest,"undef")){
       m->EndBulkLoad();
       m->SetDefined(false);
       return SetWord( Address( m ) );
    } else {
      correct = false;
      m->Destroy();
      delete m;
      return SetWord( Address( 0 ) );
    }
  }
  else while( !nl->IsEmpty( rest ) )
  {
    ListExpr first = nl->First( rest );
    rest = nl->Rest( rest );

//    UBool *unit = (UBool*)InUnit( nl->TheEmptyList(), first,
//                                errorPos, errorInfo, correct ).addr;
    UBool *unit = (UBool*)InConstTemporalUnit<CcBool,InCcBool>( 
                                nl->TheEmptyList(), first,
                                errorPos, errorInfo, correct ).addr;

    if ( !correct )
    {
      errmsg = "InMapping(): Representation of Unit "
          + int2string(unitcounter) + " is wrong.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      m->Destroy();
      delete m;
      return SetWord( Address(0) );
    }
    if(  !unit->IsDefined() || !unit->IsValid() )
    {
      errmsg = "InMapping(): Unit " + int2string(unitcounter) + " is undef.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      correct = false;
      delete unit;
      m->Destroy();
      delete m;
      return SetWord( Address(0) );
    }
    m->Add( *unit );
    unitcounter++;
    delete unit;
  }

  m->EndBulkLoad( true ); 
//  cout<<*m<<endl; 
  ////////////////////////////////////////////////////////////////////////
  ////////////// type of door: non-lift or lift////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  ListExpr door_type = nl->Fourth(instance);
  bool dr_type = nl->BoolValue(door_type);
  ///////////////////////////////////////////////////////////////////////
  Door3D* dr = new Door3D(*gr1,*gr2, *m, dr_type);

  delete gr1;
  delete gr2;  
  delete m; 
  return SetWord(dr);

}


Word CreateDoor3D(const ListExpr typeInfo)
{
//  cout<<"CreateDoor3D()"<<endl;
  return SetWord (new Door3D(false));
}


void DeleteDoor3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteDoor3D()"<<endl;
  Door3D* dr = (Door3D*)w.addr;
  delete dr;
   w.addr = NULL;
}

bool OpenDoor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenDoor3D()"<<endl;
  value.addr = new Door3D(valueRecord, offset, typeInfo);
  return value.addr != NULL;
}


/*
save function for GenRange 

*/
bool SaveDoor3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//    cout<<"SaveDoor3D()"<<endl;
    Door3D* dr = (Door3D*)value.addr;
    return dr->Save(valueRecord, offset, typeInfo);
}

void CloseDoor3D( const ListExpr typeInfo, Word& w )
{
  ((Door3D*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneDoor3D( const ListExpr typeInfo, const Word& w )
{
  return SetWord( new Door3D( *((Door3D *)w.addr) ) );
}

void* CastDoor3D(void* addr)
{
  return (new (addr) Door3D());
}

int SizeOfDoor3D()
{
  return sizeof(Door3D);
}

bool CheckDoor3D( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "door3d" ));
}

//////////////////////////////////////////////////////////////////////////////
///////////////////// data type for elevator and staircase  /////////////////
/////////////////////////////////////////////////////////////////////////////
////// we can not store DbArray<Region> in Secondo  ///////////////////////
///////// 1) assume normal office has only one floor (groom) //////////////////
///////// 2) for elevator and staircase, assume each floor covers a rectangle//
///////////////////////////////////////////////////////////////////////////
/*
the height and a rectangle for one floor 

*/
struct RectFloor{
  float h;
  Rectangle<2> rect_f;
  RectFloor(){}
  RectFloor(float f,Rectangle<2> a):h(f),rect_f(a){}
  RectFloor(const RectFloor& rf):h(rf.h),rect_f(rf.rect_f){}
  bool operator<(const RectFloor& rf) const
  {
    return h < rf.h; 
  }
  RectFloor& operator=(const RectFloor& rf)
  {
    h = rf.h;
    rect_f = rf.rect_f;
    return *this; 
  }
};

class Staircase3D:public StandardSpatialAttribute<2>{
  public:
    Staircase3D(){}
    Staircase3D(const int initsize):StandardSpatialAttribute(true),
    floors(initsize){}
    Staircase3D(const Staircase3D& staircase):
    StandardSpatialAttribute<2>(staircase.IsDefined()){
      if(staircase.IsDefined())
        floors.copyFrom(staircase.floors);
    }
    ~Staircase3D(){}
    inline int Size() const {return floors.Size();}
    inline bool IsEmpty() const{return !IsDefined() || Size() == 0;}

    inline size_t Sizeof() const{return sizeof(*this);}
    int Compare(const Attribute* arg) const{ return 0;}
    inline bool Adjacent(const Attribute* arg)const{return false;}
    Staircase3D* Clone() const {return new Staircase3D(*this);}
    size_t HashValue() const{return (size_t)0;}

    Staircase3D& operator+=(RectFloor& rf);
    void Get(const int i, RectFloor& rf)
    {
      assert(IsDefined());
      assert(i >= 0 && i < floors.Size());
      floors.Get(i, &rf);
    }
    void CopyFrom(const Attribute* right)
    {
      *this = *(const Staircase3D*)right;
    }
    const Rectangle<2> BoundingBox() const
    {
      Rectangle<2> bbox;
      for(int i = 0;i < floors.Size();i++){
          RectFloor rf;
          floors.Get(i,&rf);
          if(i == 0)
            bbox = rf.rect_f;
          else
            bbox.Union(rf.rect_f);
      }
      return bbox;
    }
    double Distance(const Rectangle<2>& r)const
   {
      return BoundingBox().Distance(r);
   }
    inline int NumOfFLOBs() const { 
      return 1;
    }
    inline Flob* GetFLOB(const int i) { 
      return &floors;
    }
  private:
    DbArray<RectFloor> floors; 
};


Staircase3D& Staircase3D::operator+=(RectFloor& rf)
{
  floors.Append(rf);
  return *this; 
}

/*
Staircase3D Property Function 

*/
ListExpr Staircase3DProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("staircase3d"),
           nl->StringAtom("(<staircase3d>) ((height rect)(height rect))"),
           nl->StringAtom("((2.0 rect1)(5.0 rect2))"))));
}

/*
output the list expression for RectFloor 

*/
ListExpr OutRectFloor(ListExpr typeInfo, Word value)
{
  RectFloor* rf = (RectFloor*)value.addr;
  ListExpr height_nl = nl->RealAtom(rf->h); 
  ListExpr rect_nl = OutRectangle<2>(nl->TheEmptyList(),
                                     SetWord((void*)&(rf->rect_f)));
  return nl->TwoElemList(height_nl,rect_nl);
}

/*
output the list expression for Staircase3D 

*/
ListExpr OutStaircase3D( ListExpr typeInfo, Word value )
{
//  cout<<"OutStaircase3D()"<<endl; 
  Staircase3D* stair = (Staircase3D*)(value.addr);
  if(!stair->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  if( stair->IsEmpty() ){
    return nl->TheEmptyList();
  }
//  cout<<"stair size "<<stair->Size()<<endl; 
  bool first = true;
  ListExpr result = nl->TheEmptyList();
  ListExpr last = result; 
  for(int i = 0;i < stair->Size();i++){
    RectFloor rf;
    stair->Get(i, rf);
    ListExpr rectfloor_nl = OutRectFloor(nl->TheEmptyList(), 
                                         SetWord((void*)&rf));
    if(first){
      result = nl->OneElemList(rectfloor_nl);
      first = false;
      last = result; 
    }else
      last = nl->Append(last,rectfloor_nl);
  }
  
  return result; 
}

/*
In function

*/
Word InStaircase3D( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef")) {
      Staircase3D* staircase = new Staircase3D();
      staircase->SetDefined(false);
      correct=true;
      return SetWord(Address(staircase));
  }
  
  Staircase3D* staircase = new Staircase3D(0);
//  cout<<"length "<<nl->ListLength(instance)<<endl;

  ListExpr floor_nl = instance;
  while(!nl->IsEmpty(floor_nl)){
      ListExpr first_fl = nl->First(floor_nl);
      floor_nl = nl->Rest(floor_nl);
      ListExpr height_nl = nl->First(first_fl);
      float height = nl->RealValue(height_nl);
//      cout<<"hegiht "<<height<<endl;
      ListExpr rect_nl = nl->Second(first_fl);
      assert(nl->ListLength(rect_nl) == 4);
      if(!( nl->IsAtom(nl->First(rect_nl) && 
        nl->AtomType(nl->First(rect_nl)) == RealType))){
        cout<<"should be real type"<<endl;  
        correct = false;
        break;
      }
      if(!( nl->IsAtom(nl->Second(rect_nl) && 
         nl->AtomType(nl->Second(rect_nl)) == RealType))){
        cout<<"should be real type"<<endl;  
        correct = false;
        break;
      }
      if(!( nl->IsAtom(nl->Third(rect_nl) && 
        nl->AtomType(nl->Third(rect_nl)) == RealType))){
        cout<<"should be real type"<<endl;  
        correct = false;
        break;
      }
      if(!( nl->IsAtom(nl->Fourth(rect_nl) && 
        nl->AtomType(nl->Fourth(rect_nl)) == RealType))){
        cout<<"should be real type"<<endl;  
        correct = false;
        break;
      }
      double x1, y1, x2, y2;
      x1 = nl->RealValue(nl->First(rect_nl));
      y1 = nl->RealValue(nl->Second(rect_nl));
      x2 = nl->RealValue(nl->Third(rect_nl));
      y2 = nl->RealValue(nl->Fourth(rect_nl));
      double min[2],max[2];
      min[0] = x1;
      min[1] = y1;
      max[0] = x2;
      max[1] = y2; 
//      cout<<"x1 "<<x1<<" y1 "<<y1<<" x2 "<<x2<<" y2 "<<y2<<endl; 
      Rectangle<2>* rect = new Rectangle<2>(true, min, max);
//      cout<<*rect<<endl; 
      RectFloor* rf = new RectFloor(height,*rect);
      *staircase += *rf;
      delete rf; 
      delete rect; 
  }
  ////////////////very important ////////////////////////////////
  correct = true; 
  ////////////////////////////////////////////////////////
  return SetWord(staircase);

}

/*
Open a Staircase3D object 

*/
bool OpenStaircase3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenStaircase3D()"<<endl; 
  Staircase3D* staircase = 
      (Staircase3D*)Attribute::Open(valueRecord,offset,typeInfo);
  value = SetWord(staircase);

  return true; 
}

/*
Save a Staircase3D object 

*/
bool SaveStaircase3D(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveStaircase3D"<<endl; 
  Staircase3D* staircase = (Staircase3D*)value.addr; 
  Attribute::Save(valueRecord,offset,typeInfo, staircase);

  return true; 
}


Word CreateStaircase3D(const ListExpr typeInfo)
{
//  cout<<"CreateStaircase3D()"<<endl;
  return SetWord (new Staircase3D(0));
}


void DeleteStaircase3D(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteStaircase3D()"<<endl;
  Staircase3D* stair = (Staircase3D*)w.addr;
  delete stair;
   w.addr = NULL;
}


void CloseStaircase3D( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseStaircase3D"<<endl; 
  ((Staircase3D*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneStaircase3D( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneStaircase3D"<<endl; 
  return SetWord( new Staircase3D( *((Staircase3D *)w.addr) ) );
}

void* CastStaircase3D(void* addr)
{
//  cout<<"CastStaircase3D"<<endl; 
  return (new (addr) Staircase3D());
}

int SizeOfStaircase3D()
{
//  cout<<"SizeOfStaircase3D"<<endl; 
  return sizeof(Staircase3D);
}

bool CheckStaircase3D( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckStaircase3D"<<endl; 
  return (nl->IsEqual( type, "staircase3d" ));
}

#endif // __INDOOR_H__
