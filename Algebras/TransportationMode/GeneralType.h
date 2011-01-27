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
#include "FTextAlgebra.h"
#include <fstream>
#include "GSLAlgebra.h"


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

const string str_tm[] = {"Bus", "Walk", "Indoor", "Car", "Tube", 
                         "Train", "Bicycle"}; 
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
const string symbol_type[] = {"MPPTN", "GROOM", "REGION", "LINE", "FREESPACE"};

inline unsigned int GetSymbol(string s)
{
//  int symbol_size = sizeof(symbol_type)/sizeof(symbol_type[0]);
  unsigned int symbol_size = ARR_SIZE(symbol_type);
//  cout<<"tm_size "<<tm_size<<endl; 
  for(unsigned int i = 0;i < symbol_size;i++){
      if(symbol_type[i].compare(s) == 0)return i;
  }
  return -1;
}
inline string GetSymbolStr(int symbol)
{
//  int symbol_size = sizeof(symbol_type)/sizeof(symbol_type[0]);
  int symbol_size = ARR_SIZE(symbol_type);
  assert(0 <= symbol && symbol < symbol_size);
  return symbol_type[symbol];
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
    void SetValue(unsigned int o, string s)
    {
      oid = o;
      label = GetSymbol(s); 
      if(label >= 0)
        SetDefined(true);
      else
        SetDefined(false);
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
  unsigned GetOid(){return oid;}
  unsigned int GetLabel(){return label;}
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
  const Rectangle<2> BoundingBox() const
  {
      Point* p = new Point(true, loc.loc1, loc.loc2);
      Rectangle<2> bbox = p->BoundingBox();
      delete p;
      return bbox;
  }
  double Distance(const Rectangle<2>& r)const
  {
      return BoundingBox().Distance(r);
  }
  static void* Cast(void* addr){return new (addr)GenLoc();}
  unsigned int GetOid() const {return oid;}
  Loc GetLoc() const {return loc;}
  void SetLoc(Loc& l){loc = l;}
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
    
   GenRange(const GenRange& gr):
   StandardSpatialAttribute<2>(gr.IsDefined()){
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
  const Rectangle<2> BoundingBox() const
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
  double Distance(const Rectangle<2>& r)const
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
  const Rectangle<3> BoundingBox() const; 
  double Distance(const Rectangle<3>& rect) const
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
    void Trajectory(GenRange& genrange);
};


bool CheckGenMO( ListExpr type, ListExpr& errorInfo );
ListExpr GenMOProperty();



struct GenMObject{
  unsigned int count;
  TupleType* resulttype; 
  vector<int> tm_list; 
  GenMObject(){ count = 0; resulttype = NULL;} 
  ~GenMObject(){if(resulttype != NULL) delete resulttype;}
  void GetTM(GenMO* mo); 
  
}; 


//////////////////////////////////////////////////////////////////////
////////////////// Space  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
class Space{
  public:
  Space();
  Space(int id);
  Space(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo);
  ~Space();
  void SetId(int id);
  static void* Cast(void* addr){return new (addr)Space();}
  bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
  static Space* Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo); 
  bool IsDefined(){return def;}
  void SetDefined(bool b){def = b;}
  int GetNetworkId() { return network_id;}
  int GetSpaceId(){return space_id;}
  private:
    bool def; 
    int space_id; 
    int network_id;
    
//    bool pave; 
//    Relation* pave_rel;//pavements relation 
//    BTree* btree_pave;//btree on pavements
//    R_Tree<2, TupleId>* rtree_pave; //rtree on pavements 

};
ListExpr SpaceProperty(); 
bool CheckSpace( ListExpr type, ListExpr& errorInfo );
int SizeOfSpace();
Word CloneSpace( const ListExpr typeInfo, const Word& w );
void CloseSpace( const ListExpr typeInfo, Word& w ); 
void DeleteSpace(const ListExpr typeInfo, Word& w); 
Word CreateSpace(const ListExpr typeInfo); 
bool SaveSpace(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
bool OpenSpace(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
Word InSpace( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 
ListExpr OutSpace( ListExpr typeInfo, Word value );
//////////////////////////////////////////////////////////////////////
/////////////////////////////random number generator//////////////////
//////////////////////////////////////////////////////////////////////

static GslRandomgen gsl_random(true); 
unsigned long GetRandom(); 

#endif
