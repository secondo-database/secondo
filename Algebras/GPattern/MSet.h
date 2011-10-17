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

[1] Header File of the MSet data type

Jan, 2010 Mahmoud Sakr

[TOC]

1 Overview

The MSet SECONDO type is a moving constant, where every unit is a time interval 
and an IntSet. The type is not generalized to represent moving sets of any 
types. This is to make the implementation as efficient as possible for 
processing the group patterns.

The MSet type has the problem of the nested DBArrays. This is because each unit
(uset) contains a set (represented as a DBArray), and the MSet contains a 
DBArray of usets. To solve the problem, we store two parallel DBArrays arrays 
in the MSet class: (1) the \emph{data} array concatenates the sets of all the 
usets in order, and (2) the \emph{units} array, which is inherited from the 
Mapping class. The \emph{units} DBArray stores USetRef rather than USet. A 
USetRef object has to indexes that point to a range (start and end positions) 
in the \emph{data} DBArray. This range is the set elements that corresponds to 
the unit. We also declare the \emph{USet} class, that can be casted to 
\emph{USetRef} and visa versa, so that one could still use the temporal algebra
operators for the MSet and USet types.

Besides the SECONDO type, this file declares the InMemMSet. This is an in 
memory representation for the MSet. In this representation, no DBArrays are 
used, rather the data structures are nested in the memory. Basically we use the 
data structures in the standard template library. One can cast the InMemMSet 
and the InMemUSet into MSet, and USet and visa versa. We use the in memory 
classes during the processing of the \emph{gpattern} operator to achive 
efficiency. The stategy is to load the data from disk using the MSet and USet 
classes, cast them to the in memory classes, do the required processing, cast 
the results back so that they can be handled by SECONDO in further processing.        

2 Defines and includes

*/

#ifndef MSET_H_
#define MSET_H_

using namespace std;

#include <string>
#include "../../Tools/Flob/DbArray.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "RegionInterpolator.h"
#include <math.h>
#include <vector>
#include <set>

#define ENABLE_ASSERTS true
#define assertIf(pred) if(ENABLE_ASSERTS) assert(pred) 

namespace mset{

class Helpers
{
public:  
  static inline bool string2int(char* digit, int& result);
  static inline string ToString( int number );
  static inline string ToString( double number );
  static ostream& PrintSet( set<int> elems, ostream &os);
};
/*
3 Classes

*/

class IntSet;
class ISet;
class USerRef;
class USet;
class MSet;
class CompressedInMemUSet;
class CompressedInMemMSet;
class CompressedUSet;
class CompressedMSet;
class InMemUSet;
class InMemMSet;

/*
3.1 The IntSet Class

*/
class IntSet: public Attribute {
public:  
  IntSet() {} 
   
/*
Constructors and the destructor

*/
 
  IntSet(int numElem);
  IntSet(bool def);
  IntSet(const IntSet& arg);
  ~IntSet();

/*
Set operations and predicates

*/
  void Union(IntSet& op2, IntSet& res);
  void Union(IntSet& op2);
  int IntersectionCount(const IntSet& arg) const;
  bool IsSubset(const IntSet& rhs) const;
  bool operator==(const IntSet& rhs) const;
  bool operator<(const IntSet& rhs) const;
  bool operator>(const IntSet& rhs) const;
  IntSet* Intersection(const IntSet& arg) const;
  void Intersection2(const IntSet& arg);
  bool Intersects(const IntSet& rhs) const;
  void Minus(IntSet& op2, IntSet& res);
  void Insert(const int elem);
  void Delete(const int elem);
  int Count()const;
  void Clear();
  int BinSearch(int elem);
  int operator[](int index) const;
  static const string BasicType();
/*
members required for the Attribute interface

*/
  size_t HashValue() const; 
  void CopyFrom(const Attribute* right);
  virtual int Compare( const Attribute* rhs ) const;
  virtual int CompareAlmost( const Attribute* arg ) const;
  ostream& Print( ostream &os ) const; 
  size_t Sizeof() const;
  bool Adjacent(const Attribute*) const ;
  Attribute* Clone() const ;
  int NumOfFLOBs()const;
  Flob *GetFLOB(const int i);
  

/*
members required for SECONDO types

*/ 
  static Word     In( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct );
  static ListExpr Out( ListExpr typeInfo, Word value );
  static Word     Create( const ListExpr typeInfo );
  static void     Delete( const ListExpr typeInfo, Word& w );
  static void     Close( const ListExpr typeInfo, Word& w );
  static Word     Clone( const ListExpr typeInfo, const Word& w );
  static void*    Cast(void* addr);
  static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
  static int      SizeOfObj();
  static ListExpr Property(); 

/*
Data members

*/  
  DbArray<int> points;
};



/*
3.1 The ISet Class

*/
class ISet: public Intime<IntSet> {
public:
  ISet();

/*
Constructors and the destructor

*/

  ISet(int i);
  ISet( const Instant& _instant, const IntSet& alpha );
  ISet( const ISet& intime );
  ~ISet();


/*
members required for the Attribute interface

*/
  int NumOfFLOBs()const;
  Flob *GetFLOB(const int i);
  ISet* Clone() const ;
  int Compare( const Attribute* rhs ) const;
  int CompareAlmost( const Attribute* rhs ) const;
/*
members required for SECONDO types

*/
  static const string BasicType();
  static Word     Create( const ListExpr typeInfo );
  static void     Delete( const ListExpr typeInfo, Word& w );
  static void     Close( const ListExpr typeInfo, Word& w );
  static Word Clone( const ListExpr typeInfo, const Word& w );
  static void*    Cast(void* addr);
  static bool     KindCheck(const ListExpr type, ListExpr& errorInfo );
  static int      SizeOf();
  static ListExpr Property();
};





/*
3.2 The USet Class

*/
class USet : public StandardTemporalUnit<IntSet>
{
  public:
/*
3.6.1 Constructors, Destructor

*/
  USet();
  USet(bool is_defined);
  USet( const Interval<Instant>& _interval, const IntSet& a );
  
  // the following constructor is for implementation compatibility with
  // UnitTypes for continious value range types (like UReal, UPoint)
  USet( const Interval<Instant>& _interval, const IntSet& a,
                                                         const IntSet& b );
  USet( const USet& u );
/*
3.6.2 Operator redefinitions

*/

  virtual USet&  operator=( const USet& i );
/*
Redefinition of the copy operator ~=~.

Two ConstTemporalUnits are equal, if both are either undefined, or both are
defined and represent the same temporal function

*/

  virtual bool operator==( const USet& i ) const;
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool operator!=( const USet& i ) const;
/*
Returns ~true~ if this temporal unit is different to the temporal unit ~i~ and ~false~ if they are equal.

*/

/*
3.6.2 The Temporal Functions

~TemporalFunction~ returns an undefined result if the ConstUnit or the Instant
is undefined, or the Instant is not within the unit's timeInterval.

*/
  virtual void TemporalFunction( const Instant& t,
                                 IntSet& result,
                                 bool ignoreLimits) const;
  
  virtual bool Passes( const IntSet& val ) const;
  virtual bool At( const IntSet& val, TemporalUnit<IntSet>& result ) const;
  virtual void AtInterval( const Interval<Instant>& i,
                           TemporalUnit<IntSet>& result ) const;
  virtual bool EqualValue( const USet& i ) const;
/*
Returns ~true~ if the value of this temporal unit is defined and equal to the
value of the temporal unit ~i~ and ~false~ if they are different.

*/

  virtual bool Merge( const USet& i ) ;
/*
Merges unit ~i~ into this unit if possible and return ~true~. Otherwise do
not modify this unit and return ~false~.

*/

/*
3.6.3 Functions to be part of relations

*/

  virtual size_t Sizeof() const;
  virtual int Compare( const Attribute* arg ) const;
  virtual int CompareAlmost( const Attribute *rhs ) const;
  virtual bool Adjacent( const Attribute* arg ) const;
  virtual int NumOfFLOBs()const;
  virtual Flob *GetFLOB(const int i);
  virtual ostream& Print( ostream &os ) const;
  virtual size_t HashValue() const;
  virtual USet* Clone() const;
  virtual void CopyFrom( const Attribute* right );
  static ListExpr USetProperty();
  static bool CheckUSet( ListExpr type, ListExpr& errorInfo );
  static ListExpr OutUSet( ListExpr typeInfo, Word value );
  static Word InUSet( const ListExpr typeInfo,
                            const ListExpr instance,
                            const int errorPos,
                            ListExpr& errorInfo,
                            bool& correct             );
  static Word CreateUSet( const ListExpr typeInfo );
  static void DeleteUSet( const ListExpr typeInfo, Word& w );
  static void CloseUSet( const ListExpr typeInfo, Word& w );
  static Word CloneUSet( const ListExpr typeInfo, const Word& w );
  static int SizeOfUSet();
  static void* CastUSet(void* addr);
  inline static const string BasicType();
/*
3.6.4 Attributes

*/

  IntSet constValue;
/*
The constant value of the temporal unit.

*/
};

const string USet::BasicType()
{
  return "uset";
}
/*
3.3 The USetRef Class

*/

class USetRef
{
public:
/*
Constructors and the destructor

*/
  USetRef(){}
  USetRef(bool def):isdefined(def){}
  ~USetRef(){}
  USetRef(const int s, const int e, const Interval<Instant> &i)
    :start(s), end(e), isdefined(true),timeInterval(i) {}
  USetRef(const int s, const int e, const Interval<Instant> &i, const bool def)
    :start(s), end(e), isdefined(def), timeInterval(i) {}
  
/*
Calss member functions

*/  
  void GetUnit(const DbArray<int>& data, USet& res) const;
  
  void GetSet(const DbArray<int>& data, set<int>& res) const;
  
  bool operator==( const USetRef& i ) const
  {
    assert( timeInterval.IsValid() && i.timeInterval.IsValid() );
    return( timeInterval == i.timeInterval && start== i.start && end== i.end);
  }
  
  bool EqualValue( const USetRef& i )
  {
    return ((*this) == i);
  }
  
  int Compare( const USetRef* ctu ) const
  {
    if (this->IsDefined() && !ctu->IsDefined())
      return 0;
    if (!this->IsDefined())
      return -1;
    if (!ctu->IsDefined())
      return 1;

    int cmp = this->timeInterval.CompareTo(ctu->timeInterval);
    return cmp;
  }
  
  size_t HashValue() const
  {
    if(!this->IsDefined()){
      return 0;
    }
    return static_cast<size_t>(   this->timeInterval.start.HashValue()
        ^ this->timeInterval.end.HashValue()   ) ;
  }
  
  bool Before( const USetRef& i ) const
  {
    assert( IsValid() && i.IsValid() );
    return ( timeInterval.Before(i.timeInterval) );
  }
  
  bool IsValid() const 
  { 
    return (this->start <= this->end && timeInterval.IsValid());
  }
  
  bool IsDefined() const {return isdefined;}
  
  void SetDefined(bool def) {isdefined= def;}
  
  void AtInterval( const Interval<Instant>& i,  USetRef& result ) const
  {
    if( !this->IsDefined() || !this->timeInterval.Intersects( i ) ){
      ((USetRef*)&result)->isdefined=  false ;
    } else {
      timeInterval.Intersection( i, result.timeInterval );
      result.start = start;
      result.end = end;
      result.isdefined = isdefined;
    }
  }
  
  ostream& Print( ostream &os ) const
  {
    return os << "[" << this->start << "," << this->end<< "[" ; 
  }

/*
For meaningfull printout, one would like to see the elements of the set. This 
function therefore accepts the \emph{data} array in its arguements.

*/
  ostream& Print(DbArray<int> data, ostream &os ) const
  {
    USet tmp(0);
    GetUnit(data, tmp);
    tmp.Print(os);
    return os; 
  }

/*
Data Members

*/

  int start;
  int end;
  bool isdefined;
  Interval<Instant> timeInterval;
};


class MSet : public  Mapping< USetRef, IntSet > 
{
public:
  MSet():Mapping<USetRef, IntSet>(){}
  MSet(const int n):
    Mapping<USetRef, IntSet>(n), data(n){}
  ~MSet() { }
  //MRegion* MSet2MRegion(vector<int>* ids, vector<MPoint*>* sourceMPoints,
  //    Instant& samplingDuration);
  void Add( const USet& unit );
  void MergeAdd( const USet& unit );
  void LiftedUnion(MSet& arg, MSet& res);
  void LiftedUnion2(MSet& arg, MSet& res);
  void LiftedUnion(MSet& arg);
  void LiftedUnion2(MSet& arg);
  void LiftedMinus(MSet& arg, MSet& res);
  void LiftedMinus2(MSet& arg, MSet& res);
  void LiftedIsSubset(MSet& arg2, MBool& res);
  void LiftedCount(MInt& res);
  void MBool2MSet(MBool& mb, int elem);
  bool operator ==(MSet& rhs);
  bool operator !=(MSet& rhs);
  void Clear();
  void AtPeriods( const Periods& periods, MSet& result ) const;
  void Initial( ISet& result ) const;
  void Final( ISet& result ) const;
  MSet* Clone() const;
  void CopyFrom( const Attribute* right );
  int NumOfFLOBs()const;
  Flob *GetFLOB(const int i);
  int Compare( const Attribute* rhs ) const;
  int CompareAlmost( const Attribute* rhs ) const;
  virtual ostream& Print( ostream &os ) const;
  static bool KindCheck( ListExpr type, ListExpr& errorInfo );
  static ListExpr Property();  
  static Word InMSet(const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct) ;
  static ListExpr OutMSet(ListExpr typeInfo, Word value);
  static Word CreateMSet( const ListExpr typeInfo );
  static void DeleteMSet( const ListExpr typeInfo, Word& w );
  static void CloseMSet( const ListExpr typeInfo, Word& w );
  static Word CloneMSet( const ListExpr typeInfo, const Word& w );
  static int SizeOfMSet();
  static void* CastMSet(void* addr);
  inline static const string BasicType();
  DbArray<int> data;
};

const string MSet::BasicType()
{
  return "mset";
}

class InMemUSet
{
public:
  InMemUSet(){}
  InMemUSet(USet& arg)
  {
    ReadFrom(arg);
  }
  InMemUSet(
      const set<int>& s, int64_t start, int64_t end, bool left, bool right):
    starttime(start), endtime(end), lc(left), rc(right),
    constValue(s.begin(), s.end()) {}

  ~InMemUSet();

  void ReadFrom(USet& arg);
  
  void WriteToUSet(USet& res);
  
  void Clear();
  
  void ReadFrom(UBool& arg, int key);
  
  void SetTimeInterval(Interval<Instant>& arg);

  void Intersection(set<int>& arg);
  
  bool Intersects(set<int>& arg);

  void Intersection(set<int>& arg, set<int>& result);

  void Union(set<int>& arg);

  void Union(set<int>& arg, set<int>& result);

  ostream& Print( ostream &os );

  unsigned int Count();

  void Insert(int elem);

  void CopyValueFrom(set<int>& arg);

  void CopyFrom(InMemUSet& arg);

  int64_t starttime, endtime;
  bool lc, rc;
  set<int> constValue;
  set<int>::iterator it;
};

class InMemMSet
{
public:
  InMemMSet();
  
  InMemMSet(MSet& arg);
  
  InMemMSet(InMemMSet& arg, list<InMemUSet>::iterator begin,
        list<InMemUSet>::iterator end);
  
  ~InMemMSet();
  
  void Clear();
  
  int GetNoComponents();
  
  void CopyFrom(InMemMSet& arg);
  
  void CopyFrom(InMemMSet& arg, list<InMemUSet>::iterator begin,
      list<InMemUSet>::iterator end);
  
  void ReadFrom(MBool& mbool, int key);
  
  void WriteToMSet(MSet& res);
  
  void WriteToMSet(MSet& res, list<InMemUSet>::iterator begin, 
      list<InMemUSet>::iterator end);

  bool MergeAdd(set<int>& val, int64_t &starttime,
      int64_t &endtime, bool lc, bool rc);
  
  ostream& Print( ostream &os );
  
  void Union (InMemMSet& arg);

  bool RemoveSmallUnits(const unsigned int n);
  
  bool RemoveShortPariods(const int64_t dMS);
 
  typedef pair<int64_t, list<InMemUSet>::iterator > inst;
  ostream& Print( map<int, inst> elems, ostream &os );
  
  bool RemoveShortElemParts(const int64_t dMS);
  
  list<InMemUSet>::iterator GetPeriodEndUnit(list<InMemUSet>::iterator begin);

  list<InMemUSet>::iterator GetPeriodStartUnit(list<InMemUSet>::iterator end);
  
  bool GetNextTrueUnit(MBool& mbool, int& pos, UBool& unit);

  void Union (MBool& arg, int key);

  
  list<InMemUSet> units;
  list<InMemUSet>::iterator it;
};


class CompressedInMemUSet
{
public:
 
  CompressedInMemUSet():count(0){}
  void Erase(int victim);
  bool EraseNodes(
      vector<int>& removedNodes, vector<pair<int,int> >& edge2nodesMap);
  void Insert(int elem);
  
  ostream& Print( ostream &os );
  
  int64_t starttime, endtime;
  bool lc, rc;
  set<int> added;
  set<int> removed;
  set<int>::iterator it;
  unsigned int count;
};

class CompressedInMemMSet
{
private:
  enum EventType{openstart=0, closedstart=1, openend=2, closedend=3};
  struct Event
  {
    Event(int _obj, EventType _type): obj(_obj), type(_type){}
    int obj;
    EventType type;
  };
  set<int> lastUnitValue;
  bool validLastUnitValue;
//  struct EventInstant
//  {
//    EventInstant(double _t, byte _side): t(_t), side(_side) {}
//    double t;  // time instant
//  byte side; //-1 for approach from right, 0 for at, 1 for approach from left
//  };
//  struct classcomp {
//    bool operator() (const EventInstant& lhs, const EventInstant& rhs) const
//    {return  (lhs.t < rhs.t) || ((lhs.t < rhs.t) && (lhs.side < rhs.side)) ;}
//  };

public:
  
  CompressedInMemMSet();
  
  CompressedInMemMSet(CompressedInMemMSet& arg, 
      list<CompressedInMemUSet>::iterator begin,
      list<CompressedInMemUSet>::iterator end);
  
  int GetNoComponents();
  
  void CopyFrom(CompressedInMemMSet& arg, 
      list<CompressedInMemUSet>::iterator begin,
      list<CompressedInMemUSet>::iterator end);
  
  void Clear();
  
  void GetSet(list<CompressedInMemUSet>::iterator index, set<int>& res);
  
  set<int>* GetFinalSet();
  
  void WriteToMSet(MSet& res);
  
  void WriteUSet(set<int>& val, CompressedInMemUSet& source, USet& res);
  
  void WriteToMSet(MSet& res, list<CompressedInMemUSet>::iterator begin, 
      list<CompressedInMemUSet>::iterator end);
  
  void WriteToInMemMSet(InMemMSet& res, 
      list<CompressedInMemUSet>::iterator begin, 
      list<CompressedInMemUSet>::iterator end);
  
  void WriteToInMemMSet(InMemMSet& res);
  
  ostream& Print( ostream &os );
  
  list<CompressedInMemUSet>::iterator 
    EraseUnit(list<CompressedInMemUSet>::iterator pos);
  
  list<CompressedInMemUSet>::iterator 
    EraseUnits(list<CompressedInMemUSet>::iterator start, 
        list<CompressedInMemUSet>::iterator end);
  
  bool RemoveSmallUnits(const unsigned int n);
  
  typedef pair<int64_t, list<CompressedInMemUSet>::iterator > inst;
  ostream& Print( map<int, inst> elems, ostream &os );
  
  bool RemoveShortElemParts(const int64_t dMS);
  
  list<CompressedInMemUSet>::iterator GetPeriodEndUnit(
      list<CompressedInMemUSet>::iterator begin);
  
  bool GetNextTrueUnit(MBool& mbool, int& pos, UBool& unit);
  
  bool Buffer (MBool& arg, int key);
  bool Buffer (MBool& arg, int key, int64_t duration);
  
  void ClassifyEvents(pair< multimap<int64_t, Event>::iterator,
     multimap<int64_t, Event>::iterator >& events,
     map<EventType, vector<multimap<int64_t, Event>::iterator> >& eventClasses);
  
  void AddUnit(int64_t starttime, int64_t endtime, bool lc, bool rc,
      set<int>& elemsToAdd, set<int>& elemsToRemove, int elemsCount);
  
  void AddUnit( set<int>& constValue,
      int64_t starttime, int64_t endtime, bool lc, bool rc);
  
  bool MergeAdd(set<int>& val, int64_t &starttime,
      int64_t &endtime, bool lc, bool rc);
   
  void ConstructFromBuffer();
  
/*
MakeMinimal merges the consecutive units having the same value together, so
that the representation is minimal.

*/
  void MakeMinimal();

  multimap<int64_t, Event> buffer;
  list<CompressedInMemUSet> units;
  list<CompressedInMemUSet>::iterator it;
};


class CompressedUSetRef
{
public:
 
  CompressedUSetRef();
  CompressedUSetRef(bool);
  
  int addedstart;
  int addedend;
  int removedstart;
  int removedend;
  int count;
  bool isdefined;
  int64_t starttime, endtime;
  bool lc, rc;
};

class CompressedMSet
{
private:
  set<int> lastUnitValue;

public:
  bool validLastUnitValue;

  CompressedMSet();
  
  CompressedMSet(int);
  
  CompressedMSet(CompressedMSet& arg);
  
  int GetNoComponents();
  
  void CopyFrom(CompressedMSet& arg);
  
  void Clear();
  
  void GetSet(int index, set<int>& res);
  
  set<int>* GetFinalSet();
  
  void WriteToCompressedInMemMSet(CompressedInMemMSet& res);
  void ReadFromCompressedInMemMSet(CompressedInMemMSet& arg);
  bool ReadFromCompressedInMemMSet(CompressedInMemMSet& arg,
                list<CompressedInMemUSet>::iterator b,
                list<CompressedInMemUSet>::iterator e);
  
  ostream& Print( ostream &os );
  
  void AddUnit( set<int>& constValue,
      int64_t starttime, int64_t endtime, bool lc, bool rc);
  
  void AddUnit( set<int>& added, set<int>& removed,
      int64_t starttime, int64_t endtime, bool lc, bool rc);
  
  bool MergeAdd(set<int>& val, int64_t &starttime,
      int64_t &endtime, bool lc, bool rc);

  int64_t DurationLength();
  
  bool Concat(CompressedMSet* arg);

  CompressedMSet* GetSubMSet(int offset, int length);

  DbArray<int> removed;
  DbArray<int> added;
  DbArray<CompressedUSetRef> units;
};



};
#endif 
