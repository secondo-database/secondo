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
TM_TRAIN, TM_BIKE, TM_TAXI, TM_FREE};

const string str_tm[] = {"Bus", "Walk", "Indoor", "Car", "Metro", 
                         "Train", "Bike", "Taxi", "Free"};
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
IF_METROROUTE, IF_METRO, IF_INDOOR, IF_TRAINNETWORK, IF_INDOORPATH}; 

const string symbol_type[] = 
{"BUSSTOP", "BUSROUTE", "MPPTN", "BUSNETWORK", "GROOM", 
"REGION", "LINE", "FREESPACE", "METRONETWORK", "METROSTOP", "METROROUTE", 
"METRO", "INDOOR", "TRAINNETWORK", "INDOORPATH"};

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
    case TM_BIKE:
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
  bool IsLocDef() const{
    if(loc.loc1 < 0 && loc.loc2 < 0) return false;
    else
      return true;
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


typedef Intime<GenLoc> IGenLoc; 
ListExpr IntimeGenLocProperty();
bool CheckIntimeGenLoc(ListExpr type, ListExpr& errorInfo);
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
    tm = source.tm;
  }
  UGenLoc& operator=(const UGenLoc& ugenloc)
  {
    timeInterval = ugenloc.timeInterval;
    gloc1 = ugenloc.gloc1;
    gloc2 = ugenloc.gloc2;
    del.isDefined = ugenloc.del.isDefined;
    tm = ugenloc.tm;
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
"Walk;Indoor;Taxi", "Walk;Bike", "Walk;Indoor;Bike"};

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
    static const string BasicType(){return "genmo";}
    
    GenMO(const GenMO& mo);
    void Clear();
    void CopyFrom(const Attribute* right); 
    Attribute* Clone() const; 
    void Add(const UGenLoc& unit); 
    void EndBulkLoad(const bool sort = true, const bool checkvalid = false); 
    void LowRes(GenMO& mo);
    void Trajectory(GenRange* genrange, Space* sp);
    
    void GenMOAt(string tm, GenMO* sub);
    void GenMOAt(GenLoc* genloc, GenMO* sub);
    void GenMOAt(Point* p, GenMO* sub);
    void AtInstant(Instant& t, Intime<GenLoc>& result); 
    void AtPeriods(Periods* peri, GenMO& result); 
    Intime<GenLoc> GetUnitInstant(UGenLoc& unit, Instant& t);
    bool Contain(string tm);
    bool Contain(int refid);

    bool Passes(Region* reg, Space* sp);
    void MapGenMO(MPoint* in, MPoint& res);
    
};

void GetLine(Point& p1, Point& p2, Line* l);

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
class BusNetwork; 
class MetroNetwork;
class Door3D;


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
//  static string BuildingInfo; 
  static string BuildingInfoB; 
  static string BuildingInfoM; 
  enum StreeSpeed{SPEED_RID = 0, SPEED_VAL}; 
  enum CommPath{CELL_ID1 = 0,CELL_AREA1,CELL_ID2,CELL_AREA2,CELL_PATH};
  enum BuildInfo{Build_ID = 0, Build_Type, Build_Area};

  static string BenchModeDISTR;
  enum BenchModeDISTRIInfo{BENCH_MODE = 0, BENCH_PARA};
  
  static string NNBuilding;
  enum BenchModeNNBuildingInfo{BM_NNB_ID = 0, BM_NNB_GEODATA};

  vector<GenMO> trip1_list;
  vector<MPoint> trip2_list; 
  vector<MGPoint> trip3_list; 

  vector<MPoint3D> indoor_mo_list1;//from a room to an entrance 
  vector<MPoint3D> indoor_mo_list2;//from entrance to a room


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
  void GetIdList(Door3D* d);
  void GetRef(GenMO* mo);
  ///////////////////create generic moving objects///////////////////////
  void GenerateGenMO(Space* sp, Periods* peri, int mo_no, int type);

  void GenerateGenMO2(Space* sp, Periods* peri, int mo_no, 
                      int type, Relation*, BTree*, Relation*);
  void GenerateCar(Space* sp, Periods* peri, int mo_no, Relation*);

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

  void GenerateLocPave(Pavement* pm, int mo_no, 
                       vector<GenLoc>& genloc_list, vector<Point>& p_loc_list);
  
  void GenerateWalkMovement(DualGraph* dg, Line* l, Point start_loc, 
                            GenMO* genmo, MPoint* mo, Instant& start_time);
  //////////////////////////////////////////////////////////////////////////
  //////////////////////Mode: Walk, Car Taxi Bike ////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  void GenerateGenMO_CTBWalk(Space* sp, Periods* peri, int mo_no, 
                             Relation* rel, BTree* btree, Relation*, string);
  void PaveLoc2GPoint(GenLoc loc1, GenLoc loc2, Space* sp, Relation* rel, 
                      BTree* btree, vector<GPoint>& gp_list, 
                      vector<Point>& p_list, bool& correct, Network* rn);
  void ConnectStartMove(GenLoc loc, Point start_loc, MPoint* mo, 
                        GenMO* genmo, Instant& start_time, 
                        Pavement* pm);
  void ConnectEndMove(Point start_loc, GenLoc loc, MPoint* mo, 
                        GenMO* genmo, Instant& start_time, Pavement* pm);
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
  void ConnectStartStop(DualGraph* dg, VisualGraph* vg, Relation*,
                           GenLoc loc1, vector<Point> ps_list1, int oid,
                           GenMO* genmo, MPoint* mo, 
                           Instant& start_time, Line* res_path);
  void ChangeEndBusStop(BusNetwork* bn, DualGraph* dg, 
                        Bus_Stop cur_bs1, vector<Point>& ps_list2, 
                       GenLoc& gloc2, Relation* rel2, R_Tree<2,TupleId>* rtree);
  void NearestBusStop2(Point q, Relation* rel2, 
                      R_Tree<2,TupleId>* rtree, Point& res, int& oid);

  void ConnectEndStop(DualGraph* dg, VisualGraph* vg, Relation*,
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
  void GenerateIndoorMovementFromExit2(IndoorInfra* i_infra, GenMO* genmo,
                                     MPoint* mo, Instant& start_time, Point loc,
                                    int entrance_index, int reg_id,
                                     MaxRect* maxrect, Periods* peri);

  /////////////////////////////////////////////////////////////////////////
  ////////////////////  Indoor Walk Car(Taxi) /////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  void GenerateGenMO5(Space* sp, Periods* peri, int mo_no, int type);
  void GenerateGenMO_IndoorWalkCTB(Space* sp, Periods* peri, int mo_no, 
                                   string, int);
  void GenerateGenMO_IWCTB(Space* sp, MaxRect* maxrect,
                           IndoorInfra* i_infra,
                           Pavement* pm, Network* rn,
                           RoadGraph* rg, Periods* peri, int mo_no, string,
                           vector<RefBuild> build_id1_list,
                           vector<RefBuild> build_id2_list, int);

  void CreateBuildingPair2(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_id1_list,
                          vector<RefBuild>& build_id2_list, int no, 
                          MaxRect* maxrect);
  /////////////////////////////////////////////////////////////////////////
  ////////////////////  Indoor Walk Bus   /////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

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
  void GenerateGenMO6(Space* sp, Periods* peri, int mo_no, int type, int para);
  void GenerateGenIBW(Space* sp, MaxRect* maxrect, IndoorInfra* i_infra,
                                Pavement* pm, DualGraph* dg,
                                VisualGraph* vg, BusNetwork* bn,
                                Periods* peri, int mo_no, 
                                vector<RefBuild> build_id1_list,
                                vector<RefBuild> build_id2_list, int);

  void CreateBuildingPair4(IndoorInfra* i_infra, 
                          vector<RefBuild>& build_id1_list,
                          vector<RefBuild>& build_id2_list, int no, 
                          MaxRect* maxrect, Relation* rel);
  ////////////////////////////////////////////////////////////////////
  ////////////////////// Metro Walk///////////////////////////////////
  ////////////////////////////////////////////////////////////////////
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
  void GenerateGenMO8(Space* sp, Periods* peri, int mo_no, int type, int para);
  void GenerateGenMOMIW(Space* sp, IndoorInfra* i_infra,
                                  MaxRect* maxrect, Pavement* pm, 
                                  DualGraph* dg, VisualGraph* vg, 
                                  MetroNetwork* mn, 
                                  Periods* peri, int mo_no, 
                                  vector<RefBuild> build_id1_list,
                                  vector<RefBuild> build_id2_list, int para);

   ////////////////////////////////////////////////////////////////////
   ////////////////////Benchmark Function ///////////////////////////
   ///////////////////////////////////////////////////////////////
   void GenerateGenMOBench1(Space* sp, Periods* peri, int mo_no,
                            Relation* distri, Relation* home, Relation* work);
   void GetSelectedBuilding(IndoorInfra* i_infra,
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list, MaxRect* maxrect, 
                          Relation* rel1, Relation* rel2);

   void CreateBuildingPair5(
                          vector<RefBuild> b_list1,
                          vector<RefBuild> b_list2,
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list, 
                          int build_no, bool);

   /////// buildings should also not be very far away from bus stops /////////
   void GetSelectedBuilding2(IndoorInfra* i_infra,
                          vector<RefBuild>& build_tid1_list,
                          vector<RefBuild>& build_tid2_list, MaxRect* maxrect, 
                          Relation* rel1, Relation* rel2, Relation* rel3);
  
   void GenerateGenMOBench2(Space* sp, Periods* peri, int mo_no,
                            Relation* id_rel, string type);
   void GenMOBenchRBO(Space* sp, Periods* peri, int mo_no, Relation* id_rel);
   void GenMOBenchIndoor(Space* sp, Periods* peri, int mo_no, Relation* id_rel);
   void GetSelectedBuilding3(IndoorInfra* i_infra,
                          vector<RefBuild>& build_tid1_list,
                          MaxRect* maxrect, Relation* rel);
   //////////////////based on NN Searching////////////////////////////
   void GenerateGenMOBench3(Space* sp, Periods* peri, int mo_no,
                          Relation* rel, R_Tree<2,TupleId>* rtree);
   void FindNNBuilding(vector<Point> p_loc_list, Relation* build_rel,
                     R_Tree<2,TupleId>* rtree, Relation* rel,
                       vector<RefBuild>& nn_build_list);
   void NNBTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                    Relation* build_rel, Point q_loc, int& b_tid,
                    double& min_dist, bool& dist_init);


   void GenerateGenMOBench4(Space* sp, Periods* peri, int mo_no, 
                            Relation* para_rel,
                            Relation* build_rel, R_Tree<2,TupleId>* rtree);
   void GenerateBench4_Taxi(Space* sp, IndoorInfra* i_infra, 
                            MaxRect* maxrect, Pavement* pm, 
                            int genmo_no, Periods* peri, 
                            vector<GenLoc> genloc_list, 
                            vector<Point> p_loc_list, 
                            vector<RefBuild> nn_build_list);
   void GenerateBench4_Bus(Space* sp, IndoorInfra* i_infra, 
                            MaxRect* maxrect, Pavement* pm, 
                            int genmo_no, Periods* peri, 
                            vector<GenLoc> genloc_list, 
                            vector<Point> p_loc_list, 
                            vector<RefBuild> nn_build_list);

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




struct EntryItem{
  EntryItem(){} //do not initialize the members
  EntryItem(int l, int h):low(l),high(h){}
  EntryItem(const EntryItem& le):low(le.low), high(le.high){}
  int low, high;
};

class Space:public Attribute{
  public:
  static string FreeSpaceTypeInfo; 
  
  enum RelTypeInfo{SPEED_REL = 0, TRINEW_REL, DGNODE_REL, 
                   BSPAVESORT_REL, MSPAVE_REL, BSBUILD_REL, MSBUILD_REL};
//  Space():Attribute(){}
//  Space(const Space& sp);
//  Space& operator=(const Space& sp); 
/*  Space(bool d, int id = 0):Attribute(d), def(true), 
                            space_id(id), rg_id(0), infra_list(0){}*/

  Space();
  Space(bool d, int id = 0);
  Space(ListExpr in_xValue, int in_iErrorPos, ListExpr& inout_xErrorInfo,
        bool& inout_bCorrect);
  Space(SmiRecord&, size_t&, const ListExpr);

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
  
  static bool SaveSpace(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
  static bool OpenSpace(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);

  static Space* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);

  bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo);
  /////////////very important two functions////////////////////
   inline int NumOfFLOBs() const { return 1;}
   inline Flob* GetFLOB(const int i) { return &infra_list;}

//   inline int NumOfFLOBs() const { return 3;}
//   inline Flob* GetFLOB(const int i) {
//     if(i < 1)
//       return &infra_list;
//     else if(i < 2)
//       return &pave_rid_list;
//     else 
//       return &entry_list;
//   }

  void AddRelation(Relation*, int);

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
  
  Relation* GetSpeedRel(){
    if(speed_exist) return street_speed;
    else return NULL;
  }
  Relation* GetNewTriRel(){
    if(tri_new_exist) return tri_new;
    else return NULL;
  }
  Relation* GetDualNodeRel(){
    if(dg_node_exist) return dg_node_rid;
    else return NULL;
  }
  BTree* GetDGNodeBTree(){
    if(dg_node_exist) return btree_dg_node;
    else return NULL;
  }
  Relation* GetBSPaveRel(){
    if(bs_pave_exist) return bs_pave_sort;
    else return NULL;
  }
  R_Tree<2,TupleId>* GetBSPaveRtree(){
    if(bs_pave_exist) return rtree_bs_pave;
    else return NULL;
  }
  void DFTraverse_BS(R_Tree<2,TupleId>* rtree, Relation* rel,
                             SmiRecordId adr, Point* loc, 
                             vector<int>& tid_list, double& min_dist);
  Relation* GetMSPaveRel(){
    if(ms_pave_exist) return ms_neighbor;
    else return NULL;
  }
  R_Tree<2,TupleId>* GetMSPaveRtree(){
    if(ms_pave_exist) return rtree_ms_pave;
    else return NULL;
  }
  void DFTraverse_MS(R_Tree<2,TupleId>* rtree, Relation* rel,
                             SmiRecordId adr, Point* loc, 
                             vector<int>& tid_list, double& min_dist);
  Relation* GetBSBuildRel(){
    if(build_exist_b) return bs_building;
    else return NULL;
  }

  Relation* GetMSBuildRel(){
    if(build_exist_m) return ms_building;
    else return NULL;
  }
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
  
//   inline int Pave_Rid_Size() const {return pave_rid_list.Size();}
//   inline int Entry_List_Size() const {return entry_list.Size();}
//   void GetRid(int, int&) const;
//   void GetEntry(int, EntryItem&)const;
  double Distance(GenLoc* gloc, Point* p);
  double Distance(GenLoc* gloc, Line* l);
  bool GetLocOnRoad(GenLoc* gloc, Point& loc);
  bool GetLocInBN(GenLoc* gloc, Point& loc);


  private:
    bool def; 
    int space_id;

    int rg_id;//road graph id 
    bool speed_exist; //whether speed relation is initialized 
    bool tri_new_exist;//whether the triangle new relation is initialized 
    bool dg_node_exist;//whether dual graph node relation is initialized
    bool bs_pave_exist;//whether the bus and pave sort relation is initialized 
    bool ms_pave_exist;//whether the metro and pave relation is initialized 
    bool build_exist_b;//whether bus stops and building relation is initialized
    bool build_exist_m;//whether m stops and building relation is initialized
    DbArray<InfraRef> infra_list; 

//     DbArray<int> pave_rid_list; //all rids for such a  oid 
//     DbArray<EntryItem> entry_list;//dual graph oid as indices

    Relation* street_speed;///speed relation
    Relation* tri_new;//tri new relation 
    Relation* dg_node_rid;//dual graph + route id 
    BTree* btree_dg_node; //btree on oid 
    Relation* bs_pave_sort; //bus stops and pave relation
    R_Tree<2, TupleId>* rtree_bs_pave;//rtree on bus stops and pavements 
    Relation* ms_neighbor; //metro stops and pave relation 
    R_Tree<2, TupleId>* rtree_ms_pave;//rtree on metro stops and pavements 
    Relation* bs_building;//bus stops and buildings relation 
    Relation* ms_building;//metro stops and buildings relation 

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


#define obj_scale 3
#define obj_scale_min 1.2 
#define UNDEFVAL -1.0 
#define EPSDIST 0.01 //a small distance deviation 

/////// the distance from pavements or buildings to bus and metro stops ////
#define NEARBUSSTOP 500.0  //make it larger 800.0 
#define NEARMETROSTOP 1200.0 // make it larger 1500.0 
#define BENCH_NN_DIST 500.0 //NN distance threshold 

//////////////////////////////////////////////////////////////////////
/////////////////////////////random number generator//////////////////
//////////////////////////////////////////////////////////////////////

static GslRandomgen gsl_random(true); 
unsigned long GetRandom(); 


#endif
