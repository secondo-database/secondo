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
#include "Indoor2.h"


#define ARR_SIZE(a) sizeof(a)/sizeof(a[0])

Word InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );

Word
InLine( const ListExpr typeInfo, const ListExpr instance,
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
    const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const
    {
      return reg.BoundingBox();
    }
    double Distance(const Rectangle<2>& r, const Geoid* geoid=0)const
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
  const Rectangle<2> BoundingBox(const Geoid* geoid=0) const
  {
      return door_pos1.BoundingBox();
  }
  double Distance(const Rectangle<2>& r, const Geoid* geoid=0)const
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

#define MIN_FLOOR_HEIGHT -1000.0

/*
GRoom: a set of 3D regions. the implementation is similar as for GenRange 

*/
class GRoom:public StandardSpatialAttribute<2>{ 
  public:
    GRoom(){}
    GRoom(const int initsize):StandardSpatialAttribute<2>(true),
    elem_list(initsize), seg_list(initsize){}

    /////////////////////////////////////////////////////////////
    ///////!!!copy constructor function has to be implemented////
    /////////////////////////////////////////////////////////////

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
    const Rectangle<2> BoundingBox(const Geoid* geoid=0) const
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
    double Distance(const Rectangle<2>& r, const Geoid* geoid=0)const
    {
      return BoundingBox().Distance(r);
    }

   const Rectangle<3> BoundingBox3D() const;

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
class MPoint3D; 
bool BBoxContainPoint3D(Rectangle<3> bbox, Point3D& p); 

struct Elevator{
  float h;
  double t1, t2; //two arrive time instants
  double m_t, w_t; //moving time and waiting time 
  Rectangle<2> el_rect;
  Elevator(){}
  Elevator(float height, double a, double b, double mt, double wt):h(height),
          t1(a), t2(b), m_t(mt), w_t(wt){
            double min[2] = {0, 1};
            double max[2] = {0, 1};
            Rectangle<2>* r = new Rectangle<2>(true, min, max);
            el_rect = *r;
            delete r;
          }
  Elevator(const Elevator& el):h(el.h), t1(el.t1), t2(el.t2), 
          m_t(el.m_t), w_t(el.w_t), el_rect(el.el_rect){}
  bool operator<(const Elevator& el) const{
    return h < el.h; 
  }
  void Print()
  {
    printf("h:%f t1:%.12f t2:%.12f\n", h, t1, t2);
  }
};



/*
lowest height. a build can not have such a height 

*/
#define  INVALID_HEIGHT -10000.0 

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
  vector<Line3D> rooms_id_list;///
  
  
  vector<GenLoc> genloc_list;
  vector<Point3D> p3d_list; 
  

  vector<GRoom> room_list; 
  vector<double> cost_list; 
  
  vector<MPoint3D> mo_list; 
  vector<GenMO> genmo_list; 
  vector<int> entrance_index;
  
  
  map<int, Line3D> indoor_paths_list;//indoor paths from disk files 
  map<int, Line3D> rooms_list;
  
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
  float NextFloorHeight(float h, vector<float>& floor_height, bool& flag_h);
  ////////////////create a relation storing door////////////////////////
  bool IsGRoom(int tid, Relation* rel);
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
  void CreateEntranceDoor(int id, int oid, int tid, 
                               int attr1, int attr2, int attr3); 
   ////////////////create a relation storing edges connecting doors//////////
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
   void ConstructMiddlePath2(GRoom* groom, vector<Point3D>& middle_path,
                             float h1, float h2);
   void BuildPathOR(int groom_oid, GRoom* groom, 
                  vector<int> tid_list, int attr1, int attr2, 
                  int attr3, int attr4);
   bool BuildPathCO(int groom_oid, GRoom* groom, 
                  vector<int> tid_list, int attr1, int attr2, 
                  int attr3, int attr4);
   void ConnectComplexRegion(int groom_oid, Line* l1, Line* l2,
                             int tid1, int tid2, float h,
                             DualGraph* dg, VisualGraph* vg, 
                             Relation* tri_rel, Region* reg);
   /////////////////////////////////////////////////////////////////////
   void GetAdjNodeIG(int oid);
   ////////////////////////// data generation/////////////////////////
   void GenerateIP1(int num);
   void GenerateIP2(int num);
   void GenerateIP3(int num);
   float GetHeightInST(GRoom* groom, Point p);
   float GetHeightInST2(GRoom* groom, Point p, bool& correct);
   void InitializeElevator(Interval<Instant>& periods, 
                                   vector<Elevator>& elev_list, double speed);
   void GenerateMO1(IndoorGraph* ig, BTree* btree, R_Tree<3,TupleId>* rtree,
                    int num, Periods* peri, bool convert); 
   //////////////////path to building entrance/////////////////////////////
   void GenerateMO2_Start(IndoorGraph* ig, BTree* btree, 
                          R_Tree<3,TupleId>* rtree,
                    int num, Periods* peri, bool convert);
   void GenerateMO2_End(IndoorGraph* ig, BTree* btree, R_Tree<3,TupleId>* rtree,
                    int num, Periods* peri, bool convert);
   void GetDoorLoc(IndoorGraph* ig, BTree* btree, 
                   vector<GenLoc>& doorloc_list, vector<int>& door_tid_list);

   /////////create one indoor moving object/////////////////////////////////
   void GenerateMO3_End(IndoorGraph* ig, BTree* btree, 
                        R_Tree<3,TupleId>* rtree,
                        Instant& start_time, int build_id, int entrance_index,
                        MPoint3D* mp3d, GenMO* genmo, Periods* peri);
   void GenerateMO3_EndExt(IndoorGraph* ig, BTree* btree, 
                           R_Tree<3,TupleId>* rtree,
                           Instant& start_time, int build_id, 
                           int entrance_index, MPoint3D* mp3d, 
                           GenMO* genmo, Periods* peri, GenLoc);
   void GenerateMO3_Start(IndoorGraph* ig, BTree* btree, 
                        R_Tree<3,TupleId>* rtree,
                        Instant& start_time, int build_id, int entrance_index,
                        MPoint3D* mp3d, GenMO* genmo, Periods* peri);
   /////////////////////////////////////////////////////////////////////
   unsigned int NumerOfElevators();
   
   void GenerateMO1_New(IndoorGraph* ig, BTree* btree, R_Tree<3,TupleId>* rtree,
                  int num, Periods* peri, bool convert, unsigned int num_elev);

   void InitializeElevator_New(Interval<Instant>& periods, 
                       vector< vector<Elevator> >& elev_list, 
                               double speed);
   void AddUnitToMO_Elevator_New(MPoint3D* mp3d, vector<Point3D>& , 
                    Instant& start_time, Instant& st, 
                    vector< vector<Elevator> >&);

   void AddUnitToMO_Elevator_New2(MPoint3D* mp3d, vector<Point3D>& , 
                    Instant& start_time, Instant& st, 
                    vector< vector<Elevator> >&, int index,
                    Line3D* l_room, int build_id, GenMO* genmo);

   void GenerateMO2_New_Start(IndoorGraph* ig, BTree* btree,
                          R_Tree<3,TupleId>* rtree,
                  int num, Periods* peri, bool convert, unsigned int num_elev);
   void GenerateMO2_New_End(IndoorGraph* ig, BTree* btree, 
                        R_Tree<3,TupleId>* rtree,
                  int num, Periods* peri, bool convert, unsigned int num_elev);

   void GenerateMO3_New_End(IndoorGraph* ig, BTree* btree, 
                            R_Tree<3,TupleId>* rtree,
                    Instant& start_time, int build_id, int entrance_index,
                    MPoint3D* mp3d, GenMO* genmo, Periods* peri,
                            unsigned int num_elev);
   void GenerateMO3_New_EndExt(IndoorGraph* ig, BTree* btree, 
                            R_Tree<3,TupleId>* rtree,
                    Instant& start_time, int build_id, int entrance_index,
                    MPoint3D* mp3d, GenMO* genmo, Periods* peri,
                    unsigned int num_elev, GenLoc gloc);
  void GenerateMO3_New_Start(IndoorGraph* ig, BTree* btree, 
                            R_Tree<3,TupleId>* rtree,
                    Instant& start_time, int build_id, int entrance_index,
                    MPoint3D* mp3d, GenMO* genmo, Periods* peri,
                            unsigned int num_elev);


   /////////////////////////////////////////////////////////////////////////
   float GetMinimumDoorWidth();
   void AddUnitToMO(MPoint3D* mp3d, Point3D& p1, Point3D& p2, 
                    Instant& start_time, double speed);
   void AddUnitToMO_Elevator(MPoint3D* mp3d, vector<Point3D>& , 
                    Instant& start_time, Instant& st, vector<Elevator>&);

    void AddUnitToMO2(MPoint3D* mp3d, Point3D& p1, Point3D& p2,
                    Instant& start_time, double speed, int index,
                    Line3D* l_room, int build_id, GenMO* genmo);
    void CreateIUnits1(Point3D& p1, Point3D& p2, string type,Rectangle<2> bbox,
                       double speed, Instant& start_time, 
                       MPoint3D* mp3d, GenMO* genmo, int new_groom_oid);
    void CreateIUnits2(Point3D& p1, Point3D& p2, string type,Rectangle<2> bbox,
                       double speed, Instant& start_time, 
                       MPoint3D* mp3d, GenMO* genmo, int new_groom_oid);

    void AddUnitToMO_Elevator2(MPoint3D* mp3d, vector<Point3D>& , 
                     Instant& start_time, Instant& st, vector<Elevator>&,
                     int index, Line3D* l_room, int build_id, GenMO* genmo);

   int GetRef_RoomTid(int, Line3D*, bool E);

   void ToGenLoc(MPoint3D* mp3d, R_Tree<3,TupleId>* rtree);
   void ToGenLoc2(MPoint3D* mp3d, R_Tree<3,TupleId>* rtree, 
                  int build_id, GenMO* genmo);


   void Get_GenLoc(Point3D p1, Point3D p2, GenLoc& loc1, GenLoc& loc2,
                   R_Tree<3,TupleId>* rtree);
   void DFTraverse(R_Tree<3,TupleId>* rtree, SmiRecordId adr, 
                           Point3D p, vector<int>& tid_list);

   void Get_GenLoc2(Point3D p1, Point3D p2, GenLoc& loc1, GenLoc& loc2,
                   R_Tree<3,TupleId>* rtree, int building_id);
   //////////////////////////shortest path searching////////////////////
   bool IsLocEqual(GenLoc* loc1, GenLoc* loc2, Relation* rel);
   void PathInOneRoom(GenLoc* gloc1, GenLoc* gloc2, Relation* rel, 
                      BTree* btree); 
   void PathInOneST(Tuple* groom_tupe, GenLoc* gloc1, GenLoc* gloc2, 
                    Line3D* l3d);
   void ComputePath3DST(GRoom* groom, Point loc1, Point loc2, float h1,
                      float h2, vector<Line3D>& candidate_path);
   void ShortestPath_Length(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree);
   bool DeadDoor(int door_tid, int groom_oid, int groom_oid_end, 
                 vector<Point3D>& door_list);

   bool ConnectStartLoc(GenLoc* gloc,  vector<int> tid_list, Relation* rel,
                         BTree* btree, vector<Line3D>&, float&, float&);


   void ShortestPath_Length_Start(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree, int start_tid);

   void ShortestPath_Length_Start2(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree, 
                            int start_tid, int entrance);

   void ShortestPath_Length_End(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree, int end_tid);

   void ShortestPath_Length_End2(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree, 
                            int end_tid, int entrance_id);

   ////////connection start locaton to all doors in staircase///////////////
   void ConnectStartLocST(Tuple* groom_tuple, GenLoc* gloc,  
                         vector<int> tid_list, vector<Line3D>& candidate_path);
  ////////connection end locaton to all doors in staircase///////////////
   void ConnectEndLocST(Tuple* groom_tuple, GenLoc* gloc,  
                         vector<int> tid_list, vector<Line3D>& candidate_path);

   bool ConnectEndLoc(GenLoc* gloc,  vector<int> tid_list, Relation* rel,
                         BTree* btree, vector<Line3D>&, float&, float&);

   bool ConnectEndLoc2(GenLoc* gloc, Relation* rel, BTree* btree, 
                       vector<Line3D>&, float&, float&);

   void IndoorShortestPath(int id1, int id2, vector<Line3D>& candidate_path, 
                           Line3D* s, Line3D* d, 
                           double& prune_dist, float, float, int);
   void InitializeQueue(int id, Point3D* start_p, Point3D* end_p, 
                        priority_queue<IPath_elem>& path_queue, 
                        vector<IPath_elem>& expand_path);
   inline bool MiddlePoint(Line3D* l, Point3D& p); 
   ////////////////////////////////////////////////////////////////////
   /////////////////////////minimum number of rooms////////////////////
   ////////////////////////////////////////////////////////////////////
   void ShortestPath_Room(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree);
   void GetHeightOfGRoom(int groom_oid1, BTree* groom_btree, Relation* rel, 
                         float& start_h1, float& start_h2);
   void IndoorShortestPath_Room(int id1, int id2,
                                vector< vector<TupleId> >& candidate_path,
                                int s_tid, int e_tid, float min_h, float max_h);
   /////////////////////////////////////////////////////////////////////
   ///////////////////////minimum travelling time///////////////////////
   /////////////////////////////////////////////////////////////////////
   void ShortestPath_Time(GenLoc* gloc1, GenLoc* gloc2, 
                            Relation* rel, BTree* btree);
   
   void IndoorShortestPath_Time1(int id1, int id2, 
                           vector<Line3D>& candidate_path, 
                           Line3D* s, Line3D* d, vector<double>& timecost,
                           I_Parameter& param, Relation* rel, 
                           BTree* btree, double& prune_time, float, 
                           float, int);
   void IndoorShortestPath_Time2(int id1, int id2, 
                           vector<Line3D>& candidate_path, 
                           Line3D* s, Line3D* d, vector<double>& timecost,
                           I_Parameter& param, Relation* rel, 
                           BTree* btree, double& prune_time, float, 
                           float, int);
   bool IsElevator(int groom_oid, Relation* rel, BTree* btree); 
   float CostInElevator(double l, I_Parameter& param); 
   float SetTimeWeight(double l, int groom_oid, Relation* rel, 
                       BTree* btree, I_Parameter& param);
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
    cout<<"groom oid "<<groom_oid<<endl;
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
create dual graph and visual graph in secondo

*/
void ShortestPath_InRegionNew(Region* reg, Point* s, Point* d, Line* pResult);
bool EuclideanConnection(Region* reg, Point*s, Point* d, Line* pResult);
bool CheckCommand(string& str1, string& str2, ListExpr& parsedCommand);
bool RunCommand(SecondoCatalog* ctlg, ListExpr parsedCommand, string str);
void GetSecondoObj(Region* reg, vector<string>& obj_name);
void DeleteSecondoObj(vector<string> obj_name); 
void FindPointInDG(DualGraph* dg, Point* loc1, Point* loc2, int& id1, int& id2);
void FindPointInDG1(DualGraph* dg, Point* loc1, int& id1);


/*
Indoor graph for navigation 

*/

class IndoorGraph: public BaseGraph{
public:
  static string NodeTypeInfo;
  static string EdgeTypeInfo;
  static string NodeBTreeTypeInfo;
  static string EntranceTidTypeInfo;

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
  void Load(int, Relation*, Relation*, int graph_type);
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
  int GetGraphType(){return graph_type;}

  BTree* GetBTree(){return btree_node;}
  void GetEntranceDoor(vector<Point>& door_loc);
  void GetEntranceDoor2(vector<Point>& door_loc, 
                        vector<int>& groom_list, vector<int>& door_tid_list);
  void GetDoorsInGRoom(int groom_oid, vector<int>& tid_list);
  
  private:
    BTree* btree_node; //btree on node relation on grood oid 1
    Relation* entrance_list;//store tid of door relation for building entrance
    int graph_type;
};

///////////////////////////////////////////////////////////////
//////////different types of buildings/////////////////////////
///////////////////////////////////////////////////////////////

enum building_type{BUILD_NONE = 0, BUILD_HOUSE,
BUILD_UNIVERSITY, BUILD_OFFICE24,
BUILD_CINEMA,  BUILD_TRAINSTATION, BUILD_HOTEL,
BUILD_AIRPORT, BUILD_HOSPITAL, BUILD_SHOPPINGMALL,
BUILD_SCHOOL, BUILD_LIBRARY, BUILD_OFFICE38};

const string str_build_type[] = {"BUILDING_NONE", "HOUSE",
"UNIVERSITY", "OFFICE24",
"CINEMA", "TRAINSTATION", "HOTEL", "AIRPORT", 
"HOSPITAL", "SHOPPINGMALL", "SCHOOL", "LIBRARY", "OFFICE38"};

inline bool WorkBuilding(int type)
{
  if(type == BUILD_UNIVERSITY || type == BUILD_OFFICE24 ||
     type == BUILD_OFFICE38 || type == BUILD_HOSPITAL || 
     type == BUILD_SHOPPINGMALL) return true;
  else
  return false;
}

inline bool CIBuilding(int type)
{
  if(type == BUILD_CINEMA) return true;
  else
  return false;
}

inline bool HotelBuilding(int type)
{
  if(type == BUILD_HOTEL) return true;
  else
  return false;
}


inline int GetBuildingType(string s)
{
  int build_size = ARR_SIZE(str_build_type);
  for(int i = 0;i < build_size;i++){
      if(str_build_type[i].compare(s) == 0)return i;
  }
  return -1;
}

inline string GetBuildingStr(int build)
{
  int build_size = ARR_SIZE(str_build_type); 
  assert(0 <= build && build < build_size);
  return str_build_type[build];
}


/*
the path where the indoor paths are stored 

*/
const string IndoorPathPrefix = "./TM-Data/";
const string IndoorPathSuffix = "_Paths";
#define MAX_ENTRANCE 9
#define MAX_ROOM_NO 10000
#define MAX_DOOR_INROOM 100

/*
for an indoor building which can be of different types: office, university...

*/
class Building{
  public:

   Building();
   Building(bool d, int id, unsigned int type);
   Building(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
   ~Building();

   static void* Cast(void* addr);
   int GetId() const {return building_id;}
   bool IsDefined() const {return def;}
   bool IsIGInit(){return indoorgraph_init;}
   unsigned int GetType(){return building_type;}
   void SetIndoorGraphId(int gid);
   unsigned int GetIGId();

   bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
   static Building* Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo);
   void Load(int id, int type, Relation* rel1, Relation* rel2);

   static string RoomBTreeTypeInfo;
   static string Indoor_GRoom_Door_Extend;
   static string RoomRTreeTypeInfo;
  
   Relation* GetRoom_Rel(){return rel_rooms;}
   BTree* GetBTree(){return btree_room;}
   R_Tree<3, TupleId>* GetRTree(){return rtree_rel_box;}
   IndoorGraph* OpenIndoorGraph();
   void CloseIndoorGraph(IndoorGraph* ig);
   
   void StorePaths();
   void WritePathToFile(FILE* fp, Line3D* path, int entrance, int groom_oid, 
                        int door_id, bool from);
   void DFTraverse(SmiRecordId adr, Point3D p, vector<int>& tid_list);
   void LoadPaths(map<int, Line3D>& path_list, map<int, Line3D>& room_id_list);
  
  private:
    bool def; 
    int building_id;
    unsigned int building_type;
    bool indoorgraph_init;
    unsigned int indoorgraph_id;
    Relation* rel_rooms;// a relation for all rooms 
    BTree* btree_room;// a btree index on room relation

    R_Tree<3, TupleId>* rtree_rel_box;// an rtree on the relation with box 

};

ListExpr BuildingProperty();
int SizeOfBuilding();
bool CheckBuilding( ListExpr type, ListExpr& errorInfo );
Word CloneBuilding( const ListExpr typeInfo, const Word& w );
void CloseBuilding( const ListExpr typeInfo, Word& w );
Word CreateBuilding(const ListExpr typeInfo);
Word InBuilding( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutBuilding( ListExpr typeInfo, Word value );
void DeleteBuilding(const ListExpr typeInfo, Word& w);
bool SaveBuilding(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
bool OpenBuilding(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);

/*
compressed storage of a building

*/
struct RefBuild{
    bool def; 
    int reg_id;
    int build_id;///real building id
    unsigned int type;
    Rectangle<2> rect;
    int tid;

    RefBuild(){}
    RefBuild(bool d, int i1, int i2, unsigned int t, Rectangle<2>& r,
             int i3):
    def(d), reg_id(i1), build_id(i2), type(t), rect(r), tid(i3){}
    RefBuild(const RefBuild& refb):
    def(refb.def), reg_id(refb.reg_id), build_id(refb.build_id), 
    type(refb.type), rect(refb.rect), tid(refb.tid){}
    bool operator<(const RefBuild& refb) const
    {
      return build_id < refb.build_id;
    }
    void Print()
    {
      cout<<"build id "<<build_id<<" type "<<GetBuildingStr(type)<<endl;
    }
};


/*
for indoor infrastructure
  building id: the first six numbers 
  room id: the first six numbers are for building id + room id in that building
  1. given a room id, we can find the building id, load the building graph 
  2. we can know the type of the building so that we can load this kind of 
  building 

*/
class IndoorInfra{
  public:

  static string BuildingPath_Info;
  static string RegId1BTreeTypeInfo;
  static string BuildingType_Info;
  static string RegId2BTreeTypeInfo;
  static string BuildingTypeRtreeInfo;

  enum IndoorInfra_Path{INDOORIF_REG_ID, INDOORIF_SP, INDOORIF_SP_INDEX,
                        INDOORIF_EP, INDOORIF_EP2, INDOORIF_EP2_GLOC,
                        INDOORIF_PATH};
  enum IndoorInfra_TYPE{INDOORIF_REG_ID_2,INDOORIF_GEODATA,INDOORIF_POLY_ID,
                        INDOORIF_REG_TYPE,INDOORIF_BUILD_TYPE,
                        INDOORIF_BUILD_TYPE2, INDOORIF_BUILD_ID};

  IndoorInfra();
  IndoorInfra(bool d, int id);
  IndoorInfra(SmiRecord& valueRecord, size_t& offset, 
                   const ListExpr typeInfo);
  ~IndoorInfra();
  static void* Cast(void* addr);
  int GetId() const {return indoor_id;}
  bool IsDefined() const {return def;}

  static IndoorInfra* Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo);
  bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);

  void Load(int id, Relation* rel1, Relation* rel2);
  Relation* BuildingPath_Rel(){return building_path;}
  Relation* BuildingType_Rel(){return building_type;}
  R_Tree<2,TupleId>* BuildingRTree() {return rtree_building;}
  void GetPathIDFromTypeID(int reg_id, vector<int>& path_id_list);
  void GetTypeFromRegId(int reg_id, int& type, int& build_id, Rectangle<2>&);
  int Get_Digit_Build_ID(){return digit_build_id;}

  private:
    bool def;
    int indoor_id;
    int digit_build_id; ///the first six or seven number is for building id
    Relation* building_path;//path for building to the pavement 
    BTree* btree_reg_id1;  //btree on reg id. relation for paths 
    Relation* building_type; // the type of a building 
    BTree* btree_reg_id2;  //btree on reg id  relation for types 

    R_Tree<2,TupleId>* rtree_building;    //rtree build on building type 

};

ListExpr IndoorInfraProperty();
Word InIndoorInfra( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutIndoorInfra( ListExpr typeInfo, Word value );
bool OpenIndoorInfra(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
bool SaveIndoorInfra(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
Word CreateIndoorInfra(const ListExpr typeInfo);
void DeleteIndoorInfra(const ListExpr typeInfo, Word& w);
void CloseIndoorInfra( const ListExpr typeInfo, Word& w );
Word CloneIndoorInfra( const ListExpr typeInfo, const Word& w );
int SizeOfIndoorInfra();
bool CheckIndoorInfra( ListExpr type, ListExpr& errorInfo );



void ReadIndoorPath(string name, int path_id, Line3D* res);
int GetIndooPathID(int, int , bool);
int GetIndooPathID2(int, int , int, bool);
struct IndoorPath{
  int oid;
  Line3D l3d;
  IndoorPath():oid(0), l3d(0){}
  IndoorPath(int id, Line3D& l):oid(id), l3d(l){}
  IndoorPath(const IndoorPath& ip):oid(ip.oid), l3d(ip.l3d){}
  IndoorPath& operator=(const IndoorPath& ip)
  {
    oid = ip.oid;
    l3d = ip.l3d;
    return *this;
  }
  bool operator<(const IndoorPath& ip) const
  {
    return oid < ip.oid;  
  }

};

///////////////whether the indoor paths are stored already ////////////////
#define INDOOR_PATH TRUE

////////a very small value for comparing wait time --1 means 1day///100ms/////
#define WT_EPS 1/(86400.0*10)


#endif // __INDOOR_H__
