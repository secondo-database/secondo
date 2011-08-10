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

[1] Header File of the Transportation Mode Algebra

Jan, 2011 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef GeneralType_H
#define GeneralType_H


#include "Algebra.h"

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
#include "TemporalNetAlgebra.h"
#include "FTextAlgebra.h"
#include <fstream>
#include "GSLAlgebra.h"
#include "Indoor2.h"
#include "ArrayAlgebra.h"


/*
technique macro definition 

*/
#define ARR_SIZE(a) sizeof(a)/sizeof(a[0])


Word InHalfSegment( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutHalfSegment(ListExpr typeInfo, Word value);


/*
for data type transportation mode. to make it more readable, I use string 
instead of int, real, or enum 

*/


enum tm_value{TM_BUS = 0, TM_WALK, TM_INDOOR, TM_CAR, TM_METRO, 
TM_TRAIN, TM_BICYCLE, TM_TAXI, TM_FREE};

const string str_tm[] = {"Bus", "Walk", "Indoor", "Car", "Metro", 
                         "Train", "Bicycle", "Taxi", "Free"};
inline int GetTM(string s)
{
//  int tm_size = sizeof(str_tm)/sizeof(str_tm[0]);
  int tm_size = ARR_SIZE(str_tm);
//  cout<<"tm_size "<<tm_size<<endl; 
  for(int i = 0;i < tm_size;i++){
      if(str_tm[i].compare(s) == 0)return i;
  }
  return -1;
}
inline string GetTMStr(int tm)
{
//  int tm_size = sizeof(str_tm)/sizeof(str_tm[0]);
  int tm_size = ARR_SIZE(str_tm); 
  assert(0 <= tm && tm < tm_size);
  return str_tm[tm];
}
///////////////////////////////////////////////////////////////////////////
///////////////////// symbol reference data type  //////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
symbols for data type. to make it more readable, I use string insteand of enum

*/ 
enum InfraSymbol{IF_BUSSTOP = 0, IF_BUSROUTE, IF_MPPTN, IF_BUSNETWORK,
IF_GROOM, IF_REGION, IF_LINE, IF_FREESPACE, IF_METRONETWORK, IF_METROSTOP,
IF_METROROUTE, IF_METRO, IF_TRAINNETWORK}; 

const string symbol_type[] = 
{"BUSSTOP", "BUSROUTE", "MPPTN", "BUSNETWORK", "GROOM", 
"REGION", "LINE", "FREESPACE", "METRONETWORK", "METROSTOP", "METROROUTE", 
"METRO", "TRAINNETWORK"};

inline int GetSymbol(string s)
{
//  int symbol_size = sizeof(symbol_type)/sizeof(symbol_type[0]);
  int symbol_size = ARR_SIZE(symbol_type);
//  cout<<"tm_size "<<tm_size<<endl; 
  for(int i = 0;i < symbol_size;i++){
      if(symbol_type[i].compare(s) == 0)return i;
  }
  return -1;
}
inline string GetSymbolStr(int symbol)
{
//  int symbol_size = sizeof(symbol_type)/sizeof(symbol_type[0]);
  int symbol_size = ARR_SIZE(symbol_type);
  if(0 <= symbol && symbol < symbol_size) return symbol_type[symbol];
  else return "none"; 
}

inline int TM2InfraLabel(int tm)
{
  int label = -1;
  switch(tm){
    case TM_BUS: 
          label = IF_BUSNETWORK;
          break;
    case TM_WALK:
          label = IF_REGION;
          break;
    case TM_INDOOR:
          label = IF_GROOM;
          break;
    case TM_CAR:
          label = IF_LINE;
          break;
    case TM_METRO:
          label = IF_METRONETWORK;
          break; 
    case TM_TRAIN:
          label = IF_TRAINNETWORK;
          break;
    case TM_BICYCLE:
          label = IF_LINE;
          break;
    case TM_TAXI:
          label = IF_LINE;;
          break;
  }
  assert(label >= 0);
  return label;

}

/*
reference data type (oid, symbol), oid = 0 is for free space 

*/
class IORef:public Attribute{
  public:
    IORef():Attribute(){}
    IORef(bool d, unsigned int id, unsigned int l):Attribute(d), 
    oid(id), label(l)
    {
//      cout<<"Constructor1()"<<endl;
    }
    
   IORef(const IORef& ref):Attribute(ref.IsDefined()){
      if(ref.IsDefined()){
        oid = ref.oid;
        label = ref.label; 
        SetDefined(true);
      }
   }
   ~IORef()
    {

    }
    IORef& operator=(const IORef& ref)
    {
        SetDefined(ref.IsDefined());
        if(IsDefined()){
          oid = ref.GetOid();
          label = ref.GetLabel();
        }
        return *this;
    }
    void SetValue(unsigned int o, string s)
    {
      oid = o;
      if(GetSymbol(s) >= 0){
        label = GetSymbol(s);
        SetDefined(true);
      }  
      else{
        cout<<"invalid symbol"<<endl;
        SetDefined(false);
      }
//      cout<<"oid "<<oid <<" label "<<label<<endl; 
    }
  inline size_t Sizeof() const{return sizeof(*this);}
  inline bool IsEmpty() const{return !IsDefined();}
  int Compare(const Attribute* arg) const
  {
      return 0;
  }
  
  inline bool Adjacent(const Attribute* arg)const{return false;}
  IORef* Clone() const {return new IORef(*this);}
  size_t HashValue() const
  {
    return (size_t)0; 
  }
  void CopyFrom(const Attribute* right)
  {
      *this = *(const IORef*)right;
  }

    
  static void* Cast(void* addr){return new (addr)IORef();}
  unsigned int GetOid() const {return oid;}
  unsigned int GetLabel() const{return label;}
  void Print(){
    cout<<"oid "<<oid<<" label "<<GetSymbolStr(label)<<endl; 
  }
  private:
    unsigned int oid;
    unsigned int label;
};

ListExpr IORefProperty();
ListExpr OutIORef( ListExpr typeInfo, Word value );
Word InIORef( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
bool OpenIORef(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool SaveIORef(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
Word CreateIORef(const ListExpr typeInfo);
void DeleteIORef(const ListExpr typeInfo, Word& w); 
void CloseIORef( const ListExpr typeInfo, Word& w );
Word CloneIORef( const ListExpr typeInfo, const Word& w );
int SizeOfIORef();
bool CheckIORef( ListExpr type, ListExpr& errorInfo );


///////////////////////////////////////////////////////////////////////////
//////////////////////  GenLoc /////////////////////////////////////////
////////////////////  oid = 0 is for free space  ///////////////////////
//////////////////////////////////////////////////////////////////////////
struct Loc{
  double loc1;
  double loc2; 
  Loc(){}
  Loc(double p1, double p2):loc1(p1), loc2(p2){}
  Loc(const Loc& loc):loc1(loc.loc1), loc2(loc.loc2){}
  Loc& operator=(const Loc& loc)
  {
    loc1 = loc.loc1;
    loc2 = loc.loc2; 
    return *this; 
  }
  void Print(){cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl;}
};

/*
genericl location representation (oid, loc)

*/
class GenLoc:public StandardSpatialAttribute<2>{
public:
    GenLoc(){}
    GenLoc(const unsigned int o):
    StandardSpatialAttribute<2>(true), oid(o), loc()
    {
//      cout<<"Constructor1()"<<endl;
        assert(oid >= 0); 
    }

    GenLoc(const unsigned int o, Loc l):
    StandardSpatialAttribute<2>(true), oid(o), loc(l)
    {
      assert(oid >= 0); 
    }
    

   GenLoc(const GenLoc& genl):StandardSpatialAttribute<2>(genl.IsDefined()){
      if(genl.IsDefined()){
        oid = genl.oid;
        loc = genl.loc; 
        SetDefined(true);
        assert(oid >= 0); 
      }
   }
   GenLoc& operator=(const GenLoc& genloc)
   {
    del.isDefined = genloc.del.isDefined;
    oid = genloc.oid;
    loc = genloc.loc; 
    assert(oid >= 0); 
    return *this; 
   }
   ~GenLoc()
    {

    }
    void SetValue(unsigned int o, Loc& l)
    {
      oid = o;
      loc = l;
      SetDefined(true);
      assert(oid >= 0); 
    }
  inline size_t Sizeof() const{return sizeof(*this);}
  inline bool IsEmpty() const{return !IsDefined();}
  int Compare(const Attribute* arg) const
  {
      return 0;
  }
  
  inline bool Adjacent(const Attribute* arg)const{return false;}
  GenLoc* Clone() const {return new GenLoc(*this);}
  size_t HashValue() const
  {
    return (size_t)0; 
  }
  void CopyFrom(const Attribute* right)
  {
      *this = *(const GenLoc*)right;
  }
  const Rectangle<2> BoundingBox(const Geoid* geoid=0) const
  {
      Point* p = new Point(true, loc.loc1, loc.loc2);
      Rectangle<2> bbox = p->BoundingBox();
      delete p;
      return bbox;
  }
  double Distance(const Rectangle<2>& r,const Geoid* geoid=0)const
  {
      return BoundingBox().Distance(r);
  }
  static void* Cast(void* addr){return new (addr)GenLoc();}
  unsigned int GetOid() const {return oid;}
  Loc GetLoc() const {return loc;}
  void SetLoc(Loc& l){loc = l;}
  static const string BasicType(){
       return "genloc";
  } 
  private:
    unsigned int oid;
    Loc loc; 
};

ListExpr GenLocProperty();
ListExpr OutGenLoc( ListExpr typeInfo, Word value );
Word InGenLoc( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
bool OpenGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool SaveGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
Word CreateGenLoc(const ListExpr typeInfo);
void DeleteGenLoc(const ListExpr typeInfo, Word& w);
void CloseGenLoc( const ListExpr typeInfo, Word& w );
Word CloneGenLoc( const ListExpr typeInfo, const Word& w );
int SizeOfGenLoc();
bool CheckGenLoc( ListExpr type, ListExpr& errorInfo );
ostream& operator<<(ostream& o, const GenLoc& gloc); 

///////////////////////////////////////////////////////////////////////////
//////////////////////  GenRange /////////////////////////////////////////
////////////////////// oid = 0 is for free space  ///////////////////////
//////////////////////////////////////////////////////////////////////////

/*
it records the start position in DbArray and number of points 
also the height for these points constructing a region 

*/
struct GenRangeElem{
  int oid;
  int start_pos; 
  int num; 
  int tm;
  GenRangeElem(){}
  GenRangeElem(int i,int pos, int n, int s):
  oid(i), start_pos(pos), num(n), tm(s){}
  GenRangeElem(const GenRangeElem& fe):
  oid(fe.oid), start_pos(fe.start_pos), num(fe.num), tm(fe.tm){}
  GenRangeElem& operator=(const GenRangeElem& gre)
  {
    oid = gre.oid;
    start_pos = gre.start_pos;
    num = gre.num; 
    tm = gre.tm; 
    return *this; 
  }
  void Print()
  {
    cout<<"obj id "<<oid<<" start_pos "<<start_pos<<
         " no "<<num<<" transportation mode "<<str_tm[tm]<<endl; 
  }
};

/*
data structure for genrange: a set of possible locations 

*/
class GenRange:public StandardSpatialAttribute<2>{
public:  
    /////////// it will call the default constructor of line   ////////////
    /////////////     In SpatialAlgebra   //////////////////////////////
    //////// inline Line::Line(){}  needs to be public instead of protected//
    ////////// Cast function will this constructor  //////////////////////
    ////////////////////////////////////////////////////////////////////////
    GenRange(){}
    GenRange(const unsigned int o):
    StandardSpatialAttribute<2>(true), elemlist(0), seglist(0)
    {
//      cout<<"Constructor1()"<<endl;
    }

   GenRange(const GenRange& gr):StandardSpatialAttribute<2>(gr.IsDefined()),
                                elemlist(0), seglist(0){
      if(gr.IsDefined()){
//        cout<<"not implemented"<<endl; 
          GenRange* temp_gr = const_cast<GenRange*>(&gr); 
          for(int i = 0;i < temp_gr->ElemSize();i++){
            GenRangeElem gelem;
            temp_gr->GetElem(i, gelem);
            elemlist.Append(gelem);
          }

          for(int i = 0;i < temp_gr->SegSize();i++){
            HalfSegment hs;
            temp_gr->GetSeg(i, hs);
            seglist.Append(hs);
          }
      }
   }
   ~GenRange()
    {

    }

  void Clear()
  {
    elemlist.clean();
    seglist.clean();
    SetDefined(true);
  }
  
  GenRange& operator= (const GenRange& gr)
  {
      elemlist.clean();
      seglist.clean();
//      cout<<"GRoom ="<<endl; 
      GenRange* genrange = const_cast<GenRange*>(&gr);
      for(int i = 0;i < genrange->ElemSize();i++){
        GenRangeElem gelem;
        genrange->GetElem(i, gelem);
        elemlist.Append(gelem);
      }
      for(int i = 0;i < genrange->SegSize();i++){
        HalfSegment hs;
        genrange->GetSeg(i, hs);
        seglist.Append(hs);
      }
      SetDefined(true);
      return *this;
  }
  
  inline size_t Sizeof() const{return sizeof(*this);}
  inline int Size() const {return elemlist.Size();}
  inline bool IsEmpty() const{return !IsDefined() || Size() == 0;}
  int Compare(const Attribute* arg) const
  {
      return 0;
  }
  void Get(const int i, GenRangeElem& grelem, Line& l)
  {
//      cout<<"GenRange::Get()"<<endl; 
      if( 0 <= i && i < elemlist.Size()){
          elemlist.Get(i, grelem);
          int start_pos = grelem.start_pos; 
          int end_pos = grelem.start_pos + grelem.num;
//          gre.Print(); 
          l.StartBulkLoad();
//          cout<<"seglist size "<<seglist.Size()<<endl; 
          for(;start_pos < end_pos; start_pos++){
            HalfSegment hs;
//            cout<<"start_pos "<<start_pos<<endl; 
            seglist.Get(start_pos, hs);
            l += hs;
          }
          l.EndBulkLoad();
      }else{
        cout<<"not valid index in Get()"<<endl;
        assert(false);
      }
  }
  void GetElem(const int i, GenRangeElem& grelem)
  {
    if(0 <= i && i < elemlist.Size())
        elemlist.Get(i, grelem);
    else{
      cout<<"not valid index in GetElem()"<<endl;
      assert(false);
    }
  }
  void GetSeg(const int i, HalfSegment& hs)
  {
    if(0 <= i && i < seglist.Size())
      seglist.Get(i, hs);
    else{
      cout<<"not valid index in GetSeg()"<<endl; 
      assert(false);
    }  
  }
  void Add(unsigned int id, Line * l, int tm);
  int ElemSize(){return elemlist.Size();}
  int SegSize(){return seglist.Size();}
  
  inline bool Adjacent(const Attribute* arg)const{return false;}
  GenRange* Clone() const {return new GenRange(*this);}
  size_t HashValue() const
  {
    return (size_t)0; 
  }
  void CopyFrom(const Attribute* right)
  {
      *this = *(const GenRange*)right;
  }
  const Rectangle<2> BoundingBox(const Geoid* geoid=0) const
  {
      Rectangle<2> bbox;
      for(int i = 0;i < seglist.Size();i++){
          HalfSegment hs;
          seglist.Get(i,&hs);
          if(i == 0)
            bbox = hs.BoundingBox();
          else
            bbox.Union(hs.BoundingBox());
      }
      return bbox;
  }
  double Distance(const Rectangle<2>& r,const Geoid* geoid=0)const
  {
      return BoundingBox().Distance(r);
  }
  double Length(); 
  static void* Cast(void* addr){return new (addr)GenRange();}
  /////////////very important two functions////////////////////
  ////////especially genrange is an attribute in a relation/////
  inline int NumOfFLOBs() const { 
    return 2;
  }
  inline Flob* GetFLOB(const int i) { 
//    cout<<"GetFLOB"<<endl; 
     if(i < 1)
      return &elemlist;
    else 
      return &seglist;
  }
  /////////////////////////////////////////////////////////////////
  private:
    DbArray<GenRangeElem> elemlist; 
    DbArray<HalfSegment> seglist; 

};

ListExpr OutGenRange( ListExpr typeInfo, Word value );
Word InGenRange( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr GenRangeProperty();
bool OpenGenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool SaveGenRange(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
Word CreateGenRange(const ListExpr typeInfo);
void DeleteGenRange(const ListExpr typeInfo, Word& w);
void CloseGenRange( const ListExpr typeInfo, Word& w );
Word CloneGenRange( const ListExpr typeInfo, const Word& w );
int SizeOfGenRange();
bool CheckGenRange( ListExpr type, ListExpr& errorInfo );

/////////////////////////////////////////////////////////////////////
///////////// temporal unit: UGenLoc ////////////////////////////////
////////////////////  oid = 0 is for free space  ///////////////////////
/////////////////////////////////////////////////////////////////////
class UGenLoc: public SpatialTemporalUnit<GenLoc,3>
{
  public:
  UGenLoc(){}; 
  UGenLoc(bool def):SpatialTemporalUnit<GenLoc,3>(def){}
  UGenLoc(const Interval<Instant>& interval, const GenLoc& loc1, 
          const GenLoc& loc2, int m):
  SpatialTemporalUnit<GenLoc,3>(interval),gloc1(loc1),gloc2(loc2),tm(m)
  {
    assert(gloc1.GetOid() == gloc2.GetOid()); 
    SetDefined(loc1.IsDefined() && loc2.IsDefined()); 
  }
  UGenLoc(const UGenLoc& source):
  SpatialTemporalUnit<GenLoc,3>(source.IsDefined())
  {
    timeInterval = source.timeInterval; 
    gloc1 = source.gloc1;
    gloc2 = source.gloc2;
    del.refs = 1;
    del.SetDelete(); 
    del.isDefined = source.del.isDefined; 
  }
  UGenLoc& operator=(const UGenLoc& ugenloc)
  {
    timeInterval = ugenloc.timeInterval;
    gloc1 = ugenloc.gloc1;
    gloc2 = ugenloc.gloc2;
    del.isDefined = ugenloc.del.isDefined;
    return *this; 
  }

  void TemporalFunction( const Instant& t, GenLoc& result,
                               bool ignoreLimits ) const;
  bool Passes( const GenLoc& gloc ) const; 
  bool At( const GenLoc& p, TemporalUnit<GenLoc>& res ) const; 

  static void* Cast(void* addr){return new (addr)UGenLoc();}
  inline size_t Sizeof() const { return sizeof(*this);}
  UGenLoc* Clone() const;
  void CopyFrom(const Attribute* right); 
  const Rectangle<3> BoundingBox(const Geoid* geoid=0) const; 
  double Distance(const Rectangle<3>& rect, const Geoid* geoid=0) const
  {
    return BoundingBox().Distance(rect); 
  }
  inline bool IsEmpty() const
  {
    return !IsDefined(); 
  }
  int GetOid(){ 
    if(IsDefined()){
        assert(gloc1.GetOid() == gloc2.GetOid()); 
        return gloc1.GetOid(); 
    }else
      return -1; 
  }
  int GetTM(){
    if(IsDefined())return tm;
    else
      return -1; 
  }
  GenLoc gloc1;
  GenLoc gloc2;
  int tm; 
};
ListExpr OutUGenLoc( ListExpr typeInfo, Word value );
Word InUGenLoc( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 

bool OpenUGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value); 
bool SaveUGenLoc(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value); 
Word CreateUGenLoc(const ListExpr typeInfo);
void DeleteUGenLoc(const ListExpr typeInfo, Word& w); 
void CloseUGenLoc( const ListExpr typeInfo, Word& w ); 
Word CloneUGenLoc( const ListExpr typeInfo, const Word& w ); 
int SizeOfUGenLoc();
bool CheckUGenLoc( ListExpr type, ListExpr& errorInfo );
ListExpr UGenLocProperty();
ostream& operator<<(ostream& o, const UGenLoc& gloc);

/////////////////////////////////////////////////////////////////////
///////////// general moving objects ////////////////////////////////
/////////////////////////////////////////////////////////////////////
const string genmo_tmlist[] = 
{"Walk", "Indoor", "Bus", "Car", "Metro", "Taxi",
"Walk;Car", "Walk;Bus", "Walk;Indoor", "Walk;Metro", "Walk;Taxi",
"Walk;Bus;Metro", "Walk;Indoor;Car", "Walk;Indoor;Bus","Walk;Indoor;Metro",
"Walk;Indoor;Taxi", "Walk;Indoor;Bus;Metro"};

class Space;


/*
the bounding box for genmpoint should be calculated somewhere else because it
needs the object identifier to calculate the absolute coordinates in space 

*/
class GenMO:public Mapping<UGenLoc,GenLoc>
{
  public:
    GenMO(){}
    GenMO(const int n):Mapping<UGenLoc,GenLoc>(n)
    {
      del.refs = 1;
      del.SetDelete();
      del.isDefined = true;
    }
    GenMO(const GenMO& mo);
    void Clear();
    void CopyFrom(const Attribute* right); 
    Attribute* Clone() const; 
    void Add(const UGenLoc& unit); 
    void EndBulkLoad(const bool sort = true, const bool checkvalid = false); 
    void LowRes(GenMO& mo);
    void Trajectory(GenRange* genrange, Space* sp);
    
    void AtMode(string tm, GenMO* sub);
};


bool CheckGenMO( ListExpr type, ListExpr& errorInfo );
ListExpr GenMOProperty();


struct MyHalfSegment;
class Pavement;
class DualGraph;
class VisualGraph;
class Bus_Stop;
struct BNNav;
class BusNetwork;
class IndoorInfra;
class MaxRect;
class RefBuild;



struct GenMO_MP{
  GenMO_MP():genmo(0), mp(0){tid = 0, oid = 0, index = -1;}
  GenMO_MP(GenMO& mo1, MPoint& mo2, int id1, int id2):
  genmo(mo1), mp(mo2), tid(id1), oid(id2){}
  GenMO_MP(const GenMO_MP& mo):
  genmo(mo.genmo), mp(mo.mp), tid(mo.tid), oid(mo.oid), index(mo.index){}

  GenMO genmo;
  MPoint mp;
  int tid;
  int oid;
  int index;
};

struct MNNav;
class RoadGraph;

/*
used to generate generic moving objects 

*/
struct GenMObject{
  unsigned int count;
  TupleType* resulttype; 
  vector<int> tm_list; 
  vector<string> tm_str_list; 
  vector<int> id_list; 
  static string StreetSpeedInfo;
  static string CommPathInfo;
  static string RTreeCellInfo;
  enum StreeSpeed{SPEED_RID = 0, SPEED_VAL}; 
  enum CommPath{CELL_ID1,CELL_AREA1,CELL_ID2,CELL_AREA2,CELL_PATH};

  vector<GenMO> trip1_list;
  vector<MPoint> trip2_list; 
  vector<MGPoint> trip3_list; 

  vector<MPoint3D> indoor_mo_list1;
  vector<MPoint3D> indoor_mo_list2;


  vector<Point> loc_list1;
  vector<Point> loc_list2;
  vector<Point> loc_list3;

  vector<GPoint> gp_list;
  vector<Line> line_list1;
  vector<Line> line_list2;

  vector<int> oid_list;
  vector<int> label_list; 
  
  vector<Rectangle<2> > rect_list1;
  vector<Rectangle<2> > rect_list2;
  vector<Line> path_list;
  
  vector<int> build_type_list1;
  vector<int> build_type_list2;
  
  vector<int> cell_id_list1;
  vector<int> cell_id_list2;
  vector<GLine> gline_list;

  vector< map<int, Line3D> > indoor_paths_list;//read indoor paths from disk
  vector< map<int, Line3D> > rooms_id_list; //groom oid for each point3D

  GenMObject(){ count = 0; resulttype = NULL;} 
  ~GenMObject(){if(resulttype != NULL) delete resulttype;}
  void GetMode(GenMO* mo); 
  void GetTMStr(bool v);
  void GetIdList(GenMO*);
  void GetIdList(GenRange* gr); 
  void GetRef(GenMO* mo);
  ///////////////////create generic moving objects///////////////////////

  void GenerateGenMO2(Space* sp, Periods* peri, int mo_no, 
                      int type, Relation*, BTree*, Relation*);
  void GenerateCar(Space* sp, Periods* peri, int mo_no, Relation*);
  void GenerateCarExt(Network* rn, Periods* peri, int mo_no, 
                   Relation*,Relation*);
  void SetCellId(vector<Point> p_loc_list, vector<int>& loc_cellid_list, 
                 R_Tree<2,TupleId>* rtree_cell, Relation* rel);
  
  void DFTraverse3(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& cellid__list);

  void GenerateCarMO(Network*, int i, Periods* peri, GLine* newgl,
                     Relation* rel, Point);
  void CreateCarMPMGP1(MPoint* mo, MGPoint* mgp,
                       vector<MyHalfSegment> seq_halfseg,
                      Instant& start_time, double speed,
                      int networkId, int routeId, Side s, 
                       double pos_len, bool increase);
  void CreateCarMPMGP2(MPoint* mo, MGPoint* mgp,
                       vector<MyHalfSegment> seq_halfseg,
                      Instant& start_time, double speed,
                      int networkId, int routeId, Side s, 
                       double pos_len, bool increase);

  //////////////////////////////////////////////////////////////////////////
  //////////////////////Mode: Car or Taxi///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  
  void GenerateGPoint(Network* rn, int mo_no, vector<GPoint>& gp_list);
  void GenerateGPoint2(Network* rn, int mo_no, 
                       vector<GPoint>& gp_list, vector<Point>& gp_loc_list);
  
  void CreateCarTrip1(MPoint* mo, vector<MyHalfSegment> seq_halfseg, 
                      Instant& start_time, double speed);
  void CreateCarTrip2(MPoint* mo, vector<MyHalfSegment> seq_halfseg, 
                      Instant& start_time, double speed);

  ///////////////////////////////////////////////////////////////////////
  /////////////////////Mode:Walk/////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  void GenerateLocPave(Pavement* pm, int mo_no, vector<GenLoc>& genloc_list);
  
  void GenerateWalkMovement(DualGraph* dg, Line* l, Point start_loc, 
                            GenMO* genmo, MPoint* mo, Instant& start_time);
  //////////////////////////////////////////////////////////////////////////
  //////////////////////Mode: Walk, Car//////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  void GenerateGenMO_CarTaxiWalk(Space* sp, Periods* peri, int mo_no, 
                             Relation* rel, BTree* btree, Relation*, string);
  void PaveLoc2GPoint(GenLoc loc1, GenLoc loc2, Space* sp, Relation* rel, 
                      BTree* btree, vector<GPoint>& gp_list, 
                      vector<Point>& p_list, bool& correct);
  void ConnectStartMove(GenLoc loc, Point start_loc, MPoint* mo, 
                        GenMO* genmo, Instant& start_time, 
                        Pavement* pm, string);
  void ConnectEndMove(Point start_loc, GenLoc loc, MPoint* mo, 
                        GenMO* genmo, Instant& start_time, 
                      Pavement* pm, string);
  void ConnectGP1GP2(Network*, Point start_loc, GLine* newgl, MPoint* mo,
                     GenMO* genmo, Instant& start_time,
                     Relation* speed_rel, string mode);
  //////////////////////////////////////////////////////////////////////////
  //////////////////////Mode: Walk, Bus/////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  void GenerateGenMO3(Space* sp, Periods* peri, int mo_no, 
                      int type, Relation*, Relation*, R_Tree<2,TupleId>*);
  void GenerateGenMO_BusWalk(Space* sp, Periods* peri, int mo_no, 
                             Relation* rel1, Relation* rel2, 
                             R_Tree<2,TupleId>* rtree, string mode);
  bool NearestBusStop(GenLoc loc, Relation* rel2, 
                      R_Tree<2,TupleId>* rtree, Bus_Stop& bs, 
                      vector<Point>& ps_list, GenLoc& gl, bool start);
  void DFTraverse1(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& tid_list);
  void ConnectStartBusStop(DualGraph* dg, VisualGraph* vg, Relation*,
                           GenLoc loc1, vector<Point> ps_list1, int oid,
                           GenMO* genmo, MPoint* mo, 
                           Instant& start_time, Line* res_path);
  void ChangeEndBusStop(BusNetwork* bn, DualGraph* dg, 
                        Bus_Stop cur_bs1, vector<Point>& ps_list2, 
                       GenLoc& gloc2, Relation* rel2, R_Tree<2,TupleId>* rtree);
  void NearestBusStop2(Point q, Relation* rel2, 
                      R_Tree<2,TupleId>* rtree, Point& res, int& oid);

  void ConnectEndBusStop(DualGraph* dg, VisualGraph* vg, Relation*,
                           GenLoc loc1, vector<Point> ps_list1, int oid,
                         GenMO* genmo, MPoint* mo, 
                         Instant& start_time, Line* res_path);
  int ConnectTwoBusStops(BNNav* bn_nav, Point sp, Point ep, GenMO* genmo,
                          MPoint* mo, Instant& start_time, 
                          DualGraph* dg, Line* res_path);
  void StringToBusStop(string str,  Bus_Stop& bs);
  void ShortMovement(GenMO* genmo, MPoint* mo, Instant& start_time, 
                     Point* p1, Point* p2);
  void FindPosInMP(MPoint* mo_bus, Point* start_loc, Point* end_loc, 
                   int& pos1, int& pos2, int index);
  void SetMO_GenMO(MPoint* mo_bus, int pos1, int pos2, Instant& start_time, 
                   MPoint* mo, GenMO* genmo, int mobus_oid, string str_tm);
  /////////////////////////////////////////////////////////////////////////
  //////////////////////Indoor Walk////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  void GenerateGenMO4(Space* sp, Periods* peri, int mo_no,int type, Relation*);

  
  void CreateBuildingPair(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_id1_list,
                          vector<RefBuild>& build_id2_list, int no, 
                          MaxRect* maxrect);
  /////////free movement in space where the transportation mode is given///////
  void GenerateFreeMovement(Line* l, Point start_loc,
                            GenMO* genmo, MPoint* mo, Instant& start_time);
  void GenerateFreeMovement2(Point start_loc, Point end_loc,
                            GenMO* genmo, MPoint* mo, Instant& start_time);
  void GenerateIndoorMovementToExit(IndoorInfra* i_infra, 
                                    GenMO* genmo, MPoint* mo, 
                                    Instant& start_time, Point loc,
                                    int entrance_index, int reg_id,
                                     MaxRect* maxrect, Periods* peri);
  void GenerateIndoorMovementFromExit(IndoorInfra* i_infra, GenMO* genmo,
                                     MPoint* mo, Instant& start_time, Point loc,
                                    int entrance_index, int reg_id,
                                     MaxRect* maxrect, Periods* peri);
  /////////////////////////////////////////////////////////////////////////
  ////////////////////  Indoor Walk Car(Taxi) /////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  void GenerateGenMO5(Space* sp, Periods* peri, 
                      int mo_no, int type, Relation* rel1, BTree* btree, 
                      Relation* rel2);
  void GenerateGenMO_IndoorWalkCarTaxi(Space* sp, IndoorInfra* i_infra, 
                                       Periods* peri, int mo_no, 
                                       Relation* rel1, BTree* btree, 
                                       Relation* rel2, string);
  void CreateBuildingPair2(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_id1_list,
                          vector<RefBuild>& build_id2_list, int no, 
                          MaxRect* maxrect);
  /////////////////////////////////////////////////////////////////////////
  ////////////////////  Indoor Walk Bus   /////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  void GenerateGenMO6(Space* sp, Periods* peri, int mo_no, int type, 
                      Relation* rel1, Relation* rel2,
                      R_Tree<2,TupleId>* rtree);
  void CreateBuildingPair3(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_id1_list,
                          vector<RefBuild>& build_id2_list, int no, 
                          MaxRect* maxrect);
  void GenerateIndoorMovementToExit2(IndoorInfra* i_infra, 
                                    GenMO* genmo, MPoint* mo, 
                                    Instant& start_time, Point loc,
                                    int entrance_index, int reg_id,
                                    MaxRect* maxrect, Periods* peri,
                                     MPoint3D*);
  //////////////////////////////////////////////////////////////////////
  ////////////////////// Metro Walk///////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  void GenerateGenMO7(Space* sp, Periods* peri, int mo_no, 
                      int type, Relation*, Relation*, R_Tree<2,TupleId>*);
  bool NearestMetroStop(GenLoc loc, Relation* rel2, 
                      R_Tree<2,TupleId>* rtree, Bus_Stop& ms,
                      vector<Point>& ps_list, GenLoc& gl);
  void DFTraverse2(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& tid_list);

  void ConnectTwoMetroStops(MNNav* mn_nav, Point sp, Point ep, GenMO* genmo,
                          MPoint* mo, Instant& start_time,
                          DualGraph* dg, Line* res_path);
  //////////////////////////////////////////////////////////////////////
  ////////////////Indoor Metro Walk/////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
    void GenerateGenMO8(Space* sp, Periods* peri, int mo_no, int type, 
                      Relation* rel1, Relation* rel2,
                      R_Tree<2,TupleId>* rtree);

  ///////////////////////////////////////////////////////////////////////
  //////////////////improve creating road shortest path/////////////////
  ///////////////////////////////////////////////////////////////////////
  void CreateCommPath(RoadGraph*, Network*, Relation*, int attr1, int attr2, 
                      int attr3, int attr4);
  inline int GetNewCellId(vector<int>&, int);
  void MergeCommPath(Network*, Relation* );
  void MergeRoadPath(vector<GLine>& gl_list, GLine* res_gl);
  void PrintRouteInterval(vector< vector<RouteInterval> >& ri_list, 
                          unsigned int);
  
};


/*
for navgiation 

*/

struct Navigation{
  unsigned int count;
  TupleType* resulttype; 

  vector<Point> loc_list1;
  vector<Point> loc_list2;
  vector<Point> neighbor1;
  vector<Point> neighbor2;

  vector<GenMO> trip_list1; 
  vector<MPoint> trip_list2;
  
  Navigation(){ count = 0; resulttype = NULL;} 
  ~Navigation(){if(resulttype != NULL) delete resulttype;}

  void Navigation1(Space* sp, Relation* rel1, Relation* rel2, 
                   Instant* start_time, Relation* rel3, Relation* rel4, 
                   R_Tree<2,TupleId>* rtree);
  bool NearestBusStop1(Point loc, Relation* rel, R_Tree<2,TupleId>* rtree, 
                       vector<Bus_Stop>& bs_list1, vector<Point>& ps_list1, 
                       vector<Point>& ps_list2, vector<GenLoc>& gloc_list1);
  void DFTraverse1(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                             Relation* rel,
                             Point query_loc, vector<int>& tid_list);

};
//////////////////////////////////////////////////////////////////////
////////////////// Space  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/*
ref representation for each infrastructure 

*/
struct InfraRef{
  int infra_id; 
  int infra_type; 
  int ref_id_low; ////////////////max int 21474 83647
  int ref_id_high;
//  int64_t ref_id_low; /////////////max int64_t 9223372036854775807
//  int64_t ref_id_high;

  InfraRef(){}
  InfraRef(int id, int t):infra_id(id), infra_type(t), 
                          ref_id_low(0), ref_id_high(0){}
  InfraRef(const InfraRef& iref):infra_id(iref.infra_id), 
  infra_type(iref.infra_type), ref_id_low(iref.ref_id_low),
  ref_id_high(iref.ref_id_high){}
  void SetIdRange(int l, int h)
  {
    assert(l >= 0 && l < h);
    ref_id_low = l;
    ref_id_high = h;
  }
  InfraRef& operator=(const InfraRef& iref)
  {
    infra_id = iref.infra_id;
    infra_type = iref.infra_type;
    ref_id_low = iref.ref_id_low;
    ref_id_high = iref.ref_id_high;
    return *this; 
  }
  void Print()
  {
    cout<<"ref id "<<infra_id<<" infra type "<<GetSymbolStr(infra_type)
        <<" low refid "<<ref_id_low<<" high refid "<<ref_id_high<<endl; 
  }
}; 


class BusNetwork; 
class MetroNetwork;


class Space:public Attribute{
  public:
  static string FreeSpaceTypeInfo; 
  
  Space():Attribute(){}
  Space(const Space& sp);
  Space(bool d, int id = 0):Attribute(d), def(true), 
                            space_id(id), rg_id(0), infra_list(0){}

  Space& operator=(const Space& sp); 
  ~Space();
  void SetId(int id);
  static void* Cast(void* addr){return new (addr)Space();}
  bool IsDefined() const{return def;}
  void SetDefined(bool b){def = b;}
  int GetSpaceId(){return space_id;}
  int GetRGId(){return rg_id;}
  
  inline size_t Sizeof() const{return sizeof(*this);}
  int Compare(const Attribute* arg) const{return 0;}
  inline bool Adjacent(const Attribute* arg)const{return false;}
  Space* Clone() const {return new Space(*this);}
  size_t HashValue() const{return (size_t)0;}
  void CopyFrom(const Attribute* right){*this = *(const Space*)right;}
  
  /////////////very important two functions////////////////////
  inline int NumOfFLOBs() const { return 1;}
  inline Flob* GetFLOB(const int i) { return &infra_list;}

  inline int Size() const {return infra_list.Size();}
  void Get(int i, InfraRef& inf_ref) const;
  void Add(InfraRef& inf_ref);

  bool CheckExist(InfraRef&  inf_ref); 
  Relation* GetInfra(string type);

  void AddRoadNetwork(Network* n);
  Network* LoadRoadNetwork(int type); 
  void CloseRoadNetwork(Network* rn);

  void AddPavement(Pavement* pn);
  Pavement* LoadPavement(int type); 
  void ClosePavement(Pavement* pn);

  void AddBusNetwork(BusNetwork* bn);
  BusNetwork* LoadBusNetwork(int type); 
  void CloseBusNetwork(BusNetwork* bn);

  void AddMetroNetwork(MetroNetwork* mn);
  MetroNetwork* LoadMetroNetwork(int type);
  void CloseMetroNetwork(MetroNetwork* mn);


  void AddIndoorInfra(IndoorInfra* indoor);
  IndoorInfra* LoadIndoorInfra(int type);
  void CloseIndoorInfra(IndoorInfra*);

  
  void AddRoadGraph(RoadGraph*);
  RoadGraph* LoadRoadGraph();
  void CloseRoadGraph(RoadGraph*);

  ////////////get a specific infrastructure////////////////////
  int GetInfraType(int oid);
  /////////////open and close all infrastructures////////////////
  void OpenInfra(vector<void*>&);
  void CloseInfra(vector<void*>&);


  int MaxRefId();
  //////////////////////////////////////////////////////////////////////////
  ////////////get the submovement in an infrastructure object///////////////
  /////////////////////////////////////////////////////////////////////////
  void GetLineInIFObject(int& oid, GenLoc gl1, GenLoc gl2, 
                         Line* l, vector<void*> infra_pointer,
                         Interval<Instant> time_range, int infra_type);
  void GetLineInRoad(int oid, GenLoc gl1, GenLoc gl2, Line* l, Network*);
  void GetLineInRegion(int oid, GenLoc gl1, GenLoc gl2, Line* l);
  void GetLineInFreeSpace(GenLoc gl1, GenLoc gl2, Line* l);
  void GetLineInBusNetwork(int& oid, Line* l,
                           BusNetwork* bn, Interval<Instant> time_range);
  void GetLineInGRoom(int oid, GenLoc gl1, GenLoc gl2, Line* l);

  private:
    bool def; 
    int space_id;
    
    int rg_id;//road graph id 
    DbArray<InfraRef> infra_list; 
};
ListExpr SpaceProperty(); 
bool CheckSpace( ListExpr type, ListExpr& errorInfo );
int SizeOfSpace();
Word CloneSpace( const ListExpr typeInfo, const Word& w );
void CloseSpace( const ListExpr typeInfo, Word& w ); 
void DeleteSpace(const ListExpr typeInfo, Word& w); 
Word CreateSpace(const ListExpr typeInfo); 

Word InSpace( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 
ListExpr OutSpace( ListExpr typeInfo, Word value );


#define obj_scale 5

//////////////////////////////////////////////////////////////////////
/////////////////////////////random number generator//////////////////
//////////////////////////////////////////////////////////////////////

static GslRandomgen gsl_random(true); 
unsigned long GetRandom(); 

#endif
