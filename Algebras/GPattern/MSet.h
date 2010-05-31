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


class Helpers
{
public:  
  static inline bool string2int(char* digit, int& result);
  static inline string ToString( int number );
  static inline string ToString( double number );
};

namespace mset{

/*
For efficiency, we cast the datetime objects into double and process the 
doubles instead. Since this casting yields the value of the instant in days, we 
define the following constant to convert the days to minutes so that to avoid 
numerical problems of very small fractions

*/
const double day2min= 1440;

/*
3 Classes

*/

class IntSet;
class USerRef;
class MSet;
class CompressedInMemUSet;
class CompressedInMemMSet;
class InMemUSet;
class InMemMSet;
/*
3.1 The IntSet Class

*/
class IntSet: public Attribute {
public:
/*
This constructor is reserved for the SECONDO cast function.

*/  
  IntSet():points(0) {} 
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
  IntSet* Intersection(const IntSet& arg) const;
  void Intersection2(const IntSet& arg);
  void Insert(const int elem);
  void Delete(const int elem);
  int Count()const;
  void Clear();
  int BinSearch(int elem);
  int operator[](int index) const;

/*
members required for the Attribute interface

*/
  size_t HashValue() const; 
  void CopyFrom(const Attribute* right);
  int Compare( const Attribute* rhs ) const;
  ostream& Print( ostream &os ) const; 
  size_t Sizeof() const;
  bool Adjacent(const Attribute*) const ;
  Attribute* Clone() const ;

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
3.2 The USet Class

*/
typedef ConstTemporalUnit< IntSet > USet;

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
    return (this->start< this->end && timeInterval.IsValid());
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


void USetRef::GetUnit(const DbArray<int>& data, USet& res) const
{
  if (this->isdefined && this->start < this->end && this->end <= data.Size())
  {
    res.constValue.Clear();
    int elem=0;
    for(int i= this->start; i< this->end; ++i)
    {
      data.Get(i, elem);
      res.constValue.Insert(elem);
    }
    res.SetDefined(true);
    res.constValue.SetDefined(true);
    res.timeInterval= this->timeInterval;
  }
}

void USetRef::GetSet(const DbArray<int>& data, set<int>& res) const
{
  if (this->isdefined && this->start < this->end && this->end <= data.Size())
  {
    res.clear();
    int elem=0;
    for(int i= this->start; i< this->end; ++i)
    {
      data.Get(i, elem);
      res.insert(elem);
    }
  }
}

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
  void LiftedCount(MInt& res);
  void MBool2MSet(MBool& mb, int elem);
  bool operator ==(MSet& rhs);
  bool operator !=(MSet& rhs);
  void Clear();
  void AtPeriods( const Periods& periods, MSet& result ) const;
  inline MSet* Clone() const;
  inline void CopyFrom( const Attribute* right );
  int NumOfFLOBs()const;
  Flob *GetFLOB(const int i);
  inline virtual ostream& Print( ostream &os ) const;
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
  
  DbArray<int> data;
};

class InMemUSet
{
public:
  InMemUSet(){}
  InMemUSet(USet& arg)
  {
    ReadFrom(arg);
  }
  InMemUSet(const set<int>& s, double start, double end, bool left, bool right):
    starttime(start), endtime(end), lc(left), rc(right),
    constValue(s.begin(), s.end()) {}
  ~InMemUSet()
  {
    constValue.clear();
  }
  void ReadFrom(USet& arg)
  {
    constValue.clear();
    SetTimeInterval(arg.timeInterval);
    int elem;
    for(int i=0; i< arg.constValue.points.Size(); ++i)
    {
      arg.constValue.points.Get(i, elem);
      constValue.insert(elem);
    }
  }
  
  void WriteToUSet(USet& res)
  {
    res.constValue.Clear();
    res.timeInterval.start.ReadFrom(starttime/day2min);
    res.timeInterval.start.SetType(instanttype);
    res.timeInterval.end.ReadFrom(endtime/day2min);
    res.timeInterval.end.SetType(instanttype);
    res.timeInterval.lc= lc;
    res.timeInterval.rc= rc;
    for(it=constValue.begin(); it != constValue.end(); ++it)
      res.constValue.Insert(*it);
    res.SetDefined(true);
    res.constValue.SetDefined(true);
    assert(res.IsValid());
  }
  
  void Clear()
  {
    constValue.clear();
  }
  
  void ReadFrom(UBool& arg, int key)
  {
    constValue.clear();
    constValue.insert(key);
    SetTimeInterval(arg.timeInterval);
  }
  
  void SetTimeInterval(Interval<Instant>& arg)
  {
    assert(arg.IsValid());
    starttime= arg.start.ToDouble() * day2min;
    endtime= arg.end.ToDouble() * day2min;
    lc= arg.lc;
    rc= arg.rc;
  }
  void Intersection(set<int>& arg)
  {
    vector<int> res(constValue.size() + arg.size());   
    vector<int>::iterator res_end;
    res_end=set_intersection(constValue.begin(), constValue.end(), 
        arg.begin(), arg.end(), res.begin());
    constValue.clear();
    for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
      constValue.insert(*rit);    
  }
  void Intersection(set<int>& arg, set<int>& result)
  {
    vector<int> res(constValue.size() + arg.size());   
    vector<int>::iterator res_end;
    res_end=set_intersection(constValue.begin(), constValue.end(), 
        arg.begin(), arg.end(), res.begin());
    result.clear();
    for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
      result.insert(*rit);    
  }
  void Union(set<int>& arg)
  {
    vector<int> res(constValue.size() + arg.size());   
    vector<int>::iterator res_end;
    res_end=set_union (constValue.begin(), constValue.end(), 
        arg.begin(), arg.end(), res.begin());
    constValue.clear();
    for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
      constValue.insert(*rit);    
  }

  void Union(set<int>& arg, set<int>& result)
  {
    vector<int> res(constValue.size() + arg.size());   
    vector<int>::iterator res_end;
    res_end=set_union (constValue.begin(), constValue.end(), 
        arg.begin(), arg.end(), res.begin());
    result.clear();
    for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
      result.insert(*rit);    
  }
  
  ostream& Print( ostream &os )
  {
    if( constValue.size() == 0 )
    {
      return os << "(USet: empty)";
    }
    Instant tmp(instanttype);
    tmp.ReadFrom(starttime/day2min);
    char c= (lc)? '[' : '(';
    os<< c ; tmp.Print(os);
    tmp.ReadFrom(endtime/day2min);
    os<< "\t"; tmp.Print(os);
    c= (rc)? ']' : ')';
    os << c <<  " {";
//    for(it=constValue.begin(); it != constValue.end(); ++it)
//    {
//      os<<(*it);
//      os << ",";
//    }
    it= constValue.begin();
    int b = *(constValue.begin()), e= *(--(constValue.end()));
    for(int beg= b; beg <= e; ++beg)
    {
      if(beg == *it) {os<<(*it)<<"\t"; ++it;}
      else os<<"e\t";
      
    }
    os << "}" << endl;
    return os;
  }
  
  unsigned int Count()
  {
    return constValue.size();
  }
  void Insert(int elem)
  {
    constValue.insert(elem);
  }
  void CopyValueFrom(set<int>& arg)
  {
    constValue.clear();
    for(it=arg.begin(); it != arg.end(); ++it)
      constValue.insert(*it);
  }
  void CopyFrom(InMemUSet& arg)
  {
    constValue.clear();
    for(it=arg.constValue.begin(); it != arg.constValue.end(); ++it)
      constValue.insert(*it);
    starttime= arg.starttime;
    endtime= arg.endtime;
    rc= arg.rc;
    lc= arg.lc;
  }
  double starttime, endtime;
  bool lc, rc;
  set<int> constValue;
  set<int>::iterator it;
};

class InMemMSet
{
public:
  InMemMSet(){}
  InMemMSet(MSet& arg)
  {
    USet uset(true);
    USetRef usetref(true);
    for(int i=0; i< arg.GetNoComponents(); ++i)
    {
      arg.Get(i, usetref);
      usetref.GetUnit(arg.data, uset);
      InMemUSet tmp(uset);
      units.push_back(tmp);
    }
  }
  InMemMSet(InMemMSet& arg, list<InMemUSet>::iterator begin,
        list<InMemUSet>::iterator end)
  {
    CopyFrom(arg, begin, end);  
  }
  
  ~InMemMSet()
  {
    Clear();
  }
  void Clear()
  {
    for(it= units.begin(); it != units.end(); ++it)
      (*it).constValue.clear();
    units.clear();
  }
  
  int GetNoComponents()
  {
    return units.size();
  }
  
  void CopyFrom(InMemMSet& arg)
  {
    Clear();
    for(arg.it= arg.units.begin(); arg.it != arg.units.end(); ++arg.it)
      units.push_back(*arg.it);
  }
  
  void CopyFrom(InMemMSet& arg, list<InMemUSet>::iterator begin,
      list<InMemUSet>::iterator end)
  {
    Clear();
    for(arg.it= begin; arg.it != end; ++arg.it)
      units.push_back(*arg.it);
    units.push_back(*end);
  }

  void ReadFrom(MBool& mbool, int key)
  {
    UBool ubool;
    InMemUSet uset;
    uset.constValue.insert(key);
    for(int i=0; i<mbool.GetNoComponents(); ++i)
    {
      mbool.Get(i, ubool);
      if(ubool.constValue.GetValue())
      {
        uset.SetTimeInterval(ubool.timeInterval);
        this->units.push_back(uset);
      }
    }
  }
  
  void WriteToMSet(MSet& res)
  {
    if(units.begin() == units.end())
    {
      res.SetDefined(false);
      return;
    }
    
    res.Clear();
    res.SetDefined(true);
    for(it=units.begin(); it != units.end(); ++it)
    {
      USet uset(true);
      (*it).WriteToUSet(uset);
      res.MergeAdd(uset);
    }
  }
  
  void WriteToMSet(MSet& res, list<InMemUSet>::iterator begin, 
      list<InMemUSet>::iterator end)
  {
    if(begin == units.end())
    {
      res.SetDefined(false);
      return;
    }
    res.Clear();
    res.SetDefined(true);
    for(it=begin; it != end; ++it)
    {
      USet uset(true);
      (*it).WriteToUSet(uset);
      res.MergeAdd(uset);
    }
  }
  
  ostream& Print( ostream &os ) 
  {
    if( units.size() == 0 )
    {
      return os << "(InMemMSet: undefined)";
    }
    os << "(InMemMSet: defined, contains " << units.size() << " units: ";
    for(it=units.begin(); it != units.end(); ++it)
    {
      //os << "\n\t";
      (*it).Print(os);
    }
    os << "\n)" << endl;
    return os;
  }
  
  
  void Union (InMemMSet& arg)
  {
    bool debugme=false;
    int no1 = units.size();
    int no2 = arg.units.size();

    if(no2==0)
      return; 

    if(no1==0)
    {
      this->CopyFrom(arg);
      return;
    }

    // both arguments are non-empty
    arg.it = arg.units.begin();
    it= units.begin();
    double a= (*it).starttime, A=(*it).endtime;
    double b= (*arg.it).starttime, B=(*arg.it).endtime;
    bool lcA= (*it).lc, rcA= (*it).rc;
    bool lcB= (*arg.it).lc, rcB= (*arg.it).rc;
    InMemMSet result;
    while(it != units.end()  &&  arg.it != arg.units.end() )
    {  
      // both arguments have units
      if(a < b){
        if (debugme) cerr<<endl<<"case 1: t1 starts before t2 ";
        // t1 starts before t2
        if(A < b){ // t1 before t2
          if (debugme) cerr<<endl<<"case 1.1: t1 ends before t2 starts";
          InMemUSet tmp((*it).constValue, a, A, lcA, rcA);
          result.units.push_back(tmp);
          ++it;
          if(it != units.end())
          {
            a= (*it).starttime; A=(*it).endtime;
            lcA= (*it).lc; rcA= (*it).rc;
          }
        } else if(A > b){
          if (debugme) cerr<<endl<<"case 1.2: t1 ends after t2 starts";
          // overlapping intervals
          InMemUSet tmp((*it).constValue, a, b, lcA, !lcB);
          result.units.push_back(tmp);
          a= b;
          lcA= lcB;
        } else { // u1.timeInterval.end == u2.timeInterval.start
          if (debugme) cerr<<endl<<"case 1.3: t1 ends when t2 starts ";
          if( !rcA  || !lcB){
            if (debugme) 
              cerr<<endl<<"case 1.3.1: t1 ends before t2 starts (closeness) ";
            // u1 before u2

            InMemUSet tmp((*it).constValue, a, A, lcA, rcA);
            result.units.push_back(tmp);
            ++it;
            if(it != units.end()){
              a= (*it).starttime; A=(*it).endtime;
              lcA= (*it).lc; rcA= (*it).rc;
            }
          } else { // intervals have a common instant
            if (debugme) 
              cerr<<endl<<"case 1.3.2: t2 ends when t2 starts (common instant)";
            InMemUSet tmp((*it).constValue, a, A, lcA, false);
            lcA = true;
            a = b;
          }
        }
      } else if(b < a){
        if (debugme) cerr<<endl<<"case 2: t2 starts before t1 starts";
        // symmetric case , u2 starts before u1
        if(B < a){ // u2 before u1
          if (debugme) cerr<<endl<<"case 2.1: t2 ends before t1 ends ";
          InMemUSet tmp((*arg.it).constValue, b, B, lcB, rcB);
          result.units.push_back(tmp);
          ++arg.it;
          if(arg.it != arg.units.end()){
            b= (*arg.it).starttime; B=(*arg.it).endtime;
            lcB= (*arg.it).lc; rcB= (*arg.it).rc;
          }
        } else if(B > a){
          if (debugme) cerr<<endl<<"case 2.2: t2 ends after t1 starts";
          // overlapping intervals
          InMemUSet tmp((*arg.it).constValue, b, a, lcB, !lcA);
          result.units.push_back(tmp);
          b = a;
          lcB = lcA;
        } else { // u1.timeInterval.end == u2.timeInterval.start
          if (debugme) cerr<<endl<<"case 2.3: t2 ends when t1 starts";
          if( !rcB  || !lcA){
            if (debugme) 
              cerr<<endl<<"case 2.3.1: t2 ends before t1 starts (closeness)";
            // u1 before u2
            InMemUSet tmp((*arg.it).constValue, b, B, lcB, rcB);
            result.units.push_back(tmp);
            ++arg.it;
            if(arg.it != arg.units.end()){
              b= (*arg.it).starttime; B=(*arg.it).endtime;
              lcB= (*arg.it).lc; rcB= (*arg.it).rc;
            }
          } else { // intervals have a common instant
            if (debugme) 
              cerr<<endl<<"case 2.3.2: t2 ends when t1 starts (common instant)";
            InMemUSet tmp((*arg.it).constValue, b, B, lcB, false);
            result.units.push_back(tmp);
            lcB = true;
            b = a;
          }
        }
      } else { // u1.timeInterval.start == u2.timeInterval.start
        if (debugme) 
          cerr<<endl<<"case 3: t1 and t2 start at the same instant";
        // both intervals start at the same instant
        if(lcA != lcB){
          if (debugme) 
            cerr<<endl<<"case 3.1: membership of the instant differs";
          if(lcA){ // add a single instant interval
            InMemUSet tmp((*it).constValue, a, a, true, true);
            result.units.push_back(tmp);
            if(a == A){ // u1 exhausted
              ++it;
              if(it != units.end()){
                a= (*it).starttime; A=(*it).endtime;
                lcA= (*it).lc; rcA= (*it).rc;
              }
            } else {
              lcA = false;
            }
          } else {
            // symmetric case
            InMemUSet tmp((*arg.it).constValue, b, b, true, true);
            result.units.push_back(tmp);
            if(b == B){
              ++arg.it;
              if(arg.it != arg.units.end()){
                b= (*arg.it).starttime; B=(*arg.it).endtime;
                lcB= (*arg.it).lc; rcB= (*arg.it).rc;
              }
            } else {
              lcB = false;
            }
          }
        } else { // intervals start exact at the same instant
          if (debugme) cerr<<endl<<"case 3.2: intervalls start exact together";
          set<int> opres;
          (*it).Union((*arg.it).constValue, opres);

          if(A < B){
            if (debugme) cerr<<endl<<"case 3.2.1: t1 ends before t2 ends";
            InMemUSet tmp(opres, a, A, lcA, rcA);
            result.units.push_back(tmp);
            b = A;
            lcB = !rcA;
            ++it;
            if(it != units.end()){
              a= (*it).starttime; A=(*it).endtime;
              lcA= (*it).lc; rcA= (*it).rc;
            }
          } else if (B < A){
            if (debugme) cerr<<endl<<"case 3.2.2: t2 ends before t1 ends";
            InMemUSet tmp(opres, b, B, lcB, rcB);
            result.units.push_back(tmp);
            a = B;
            lcA = !rcB;
            ++arg.it;
            if(arg.it != arg.units.end()){
              b= (*arg.it).starttime; B=(*arg.it).endtime;
              lcB= (*arg.it).lc; rcB= (*arg.it).rc;
            }
          } else { // both units end at the same instant
            if (debugme) 
              cerr<<endl<<"case 3.2.3: both intervals ends at the same instant";
            if(rcA == rcB){  // equal intervals
              if (debugme) cerr<<endl<<"case 3.2.3.1: intervals are equal" ;
              InMemUSet tmp(opres, a, A, lcA, rcA);
              result.units.push_back(tmp);
              ++it;
              if(it != units.end()){
                a= (*it).starttime; A=(*it).endtime;
                lcA= (*it).lc; rcA= (*it).rc;
              }
              ++arg.it;
              if(arg.it != arg.units.end()){
                b= (*arg.it).starttime; B=(*arg.it).endtime;
                lcB= (*arg.it).lc; rcB= (*arg.it).rc;
              }
            } else {
              if (debugme) 
                cerr<<endl<<"case 3.2.3.2: intervals differ at right closeness";
              // process common part
              InMemUSet tmp(opres, a, A, lcA, false);
              result.units.push_back(tmp);
              if(rcA){
                ++arg.it;
                if(arg.it != arg.units.end()){
                  b= (*arg.it).starttime; B=(*arg.it).endtime;
                  lcB= (*arg.it).lc; rcB= (*arg.it).rc;
                }
                lcA = true;
                a = A;
              } else {
                ++it;
                if(it != units.end()){
                  a= (*it).starttime; A=(*it).endtime;
                  lcA= (*it).lc; rcA= (*it).rc;
                }
                lcB = true;
                b = B;
              }
            }
          }
        }
      }
    }

    if (debugme) cerr<<endl<<"one of the arguments is finished";

    // process remainder of m1
    while(it != units.end()){
      InMemUSet tmp((*it).constValue, a, A, lcA, rcA);
      result.units.push_back(tmp);
      ++it;
      if(it != units.end()){
        a= (*it).starttime; A=(*it).endtime;
        lcA= (*it).lc; rcA= (*it).rc;
      }

    }
    // process remainder of m2
    while(arg.it != arg.units.end()){
      InMemUSet tmp((*arg.it).constValue, b, B, lcB, rcB);
      result.units.push_back(tmp);
      ++arg.it;
      if(arg.it != arg.units.end()){
        b= (*arg.it).starttime; B=(*arg.it).endtime;
        lcB= (*arg.it).lc; rcB= (*arg.it).rc;
      }
    }
    this->CopyFrom(result);
  }

  bool RemoveSmallUnits(const unsigned int n)
  {
    bool debugme= false;
    bool changed=false;
    if(units.size()==0) return false;
    it= units.end();
    --it;
    while(it!= units.begin())
    {
      if(debugme) {cerr<<endl<< (*it).Count() ; (*it).Print(cerr);}
      if((*it).Count() < n)
      {
        units.erase(it);
        changed= true;
      }
      --it;
    }
    if((*units.begin()).Count() < n)
    {
        units.erase(units.begin());
        changed = true;
    }
    return changed;
  }
  
  bool RemoveShortPariods(const double d)
  {
    bool changed=false;
    if(units.size()==0) return false;
    
    list<InMemUSet>::iterator begin=units.begin(), end= units.begin(), tmp;
    while(begin != units.end())
    {
      end = GetPeriodEndUnit(begin);
      if(((*end).endtime - (*begin).starttime) < d)
      {
        tmp= end; ++tmp;
        begin= units.erase(begin, tmp);
        changed= true;
      }
    }
    return changed;
  }
  typedef pair<double, list<InMemUSet>::iterator > inst;
  ostream& Print( map<int, inst> elems, ostream &os )
  {
    map<int, inst>::iterator elemsIt= elems.begin();
    while(elemsIt != elems.end())
    {
      os<<(*elemsIt).first<<" ";
      (*(*elemsIt).second.second).Print(os);
      os<<endl;
      ++elemsIt;
    }
    return os;
    
  }
  bool RemoveShortElemParts(const double d)
  {
    bool debugme= false;
    
    //handling special cases
    if(units.size()==0) return false;
    if(units.size()==1)
    {
      if( ( (*units.begin()).endtime - (*units.begin()).starttime) < d)
      { 
        units.clear(); return true;
      }
      else
        return false;
    }
       
    list<InMemUSet>::iterator cur= units.begin() , prev, end, tmp;
//    typedef pair<double, list<InMemUSet>::iterator > inst;
    map<int, inst> elems;
    map<int, inst>::iterator elemsIt;
    bool changed=false;
//    if(debugme)
//      Print(cerr);
    
    
    while(cur != units.end())
    {
      end = GetPeriodEndUnit(cur);
      if(debugme)
      {
        (*cur).Print(cerr);
        (*end).Print(cerr);
      }
      if(debugme)
      {
        MSet tmp1(0);
        WriteToMSet(tmp1);
        tmp1.Print(cerr);
      }
/*
IF the period length is less than d, remove all units within the period

*/      
      if(((*end).endtime - (*cur).starttime) < d)
      {
        ++end;
        cur= units.erase(cur, end);
        changed= true;
      }
/*
ELSE remove short parts of elements as follows:
 1. Initialize a hashtable [mset element, its deftime]
 2. Update the deftime of every element while iterating ovet the units
 3. After each iteration, elements are not updated must have deftime > d, 
    otherwise their observed part is removed from the MSet 

*/
      else
      {
        elems.clear();
        bool firstIteration= true;
        ++end;
        while(cur != end)
        {
          if(firstIteration)
          {
            for(set<int>::iterator elem= (*cur).constValue.begin(); elem !=
              (*cur).constValue.end(); ++elem)
            {
              if(debugme)
              {
                cerr<<endl;
                cerr<<*elem<< " "; (*cur).Print(cerr);
              }

              inst lt((*cur).starttime, cur);
              elems.insert(pair<int, inst>(*elem, lt));
            }
            firstIteration= false;
            prev= cur; ++cur;
            continue;
          }

          if(debugme)
          {
            Print(elems,cerr);
            (*prev).Print(cerr);
            (*cur).Print(cerr);
          }
          vector<int> diff(0);
          vector<int>::iterator diffEnd, diffIt;

          //Finalize the elements that do not appear anymore in the cur unit
          diffEnd= 
            set_difference((*prev).constValue.begin(), 
                (*prev).constValue.end(), (*cur).constValue.begin(), 
                (*cur).constValue.end(), diff.begin());

          diffIt = diff.begin();
          while(diffIt != diffEnd)
          {
            int elemToRemove= *diffIt;
            elemsIt= elems.find(elemToRemove);
            assert(elemsIt != elems.end());
            if( ((*prev).endtime - (*elemsIt).second.first) < d )
            {
              list<InMemUSet>::iterator unitToChange= (*elemsIt).second.second;
              for(; unitToChange != cur; ++unitToChange)
                (*unitToChange).constValue.erase(elemToRemove);
              changed = true;
            }
            elems.erase(elemsIt);
            ++diffIt;
          }
          
          vector<int> diff2(0);
          vector<int>::iterator diff2End, diff2It;
          //Add the new elements that starts to appear in the cur unit
          diff2End= 
            set_difference((*cur).constValue.begin(), (*cur).constValue.end(), 
                (*prev).constValue.begin(), (*prev).constValue.end(), 
                diff2.begin());

          diff2It = diff2.begin();
          while(diff2It != diff2End)          {
            inst lt((*cur).starttime, cur);
            elems.insert(pair<int, inst>(*diff2It, lt));
            ++diff2It;
          }
          prev= cur; ++cur;
        }
        for(elemsIt= elems.begin(); elemsIt != elems.end(); ++elemsIt)
        {

          if(debugme)
          {
            cerr<<(*elemsIt).first;
            cerr<<endl<<((*prev).endtime - (*elemsIt).second.first);
          }
          if( ((*prev).endtime - (*elemsIt).second.first) < d )
          {
            list<InMemUSet>::iterator unitToChange= (*elemsIt).second.second;
            for(; unitToChange != cur; ++unitToChange)
              (*unitToChange).constValue.erase( (*elemsIt).first);
            changed = true;
          }
        }
        cur= end;
      }
    }
    return changed;
  }
  
  
  
//  bool RemoveShortElemParts(const double d)
//  {
//    bool debugme= false;
//    
//    //handling special cases
//    if(units.size()==0) return false;
//    if(units.size()==1)
//    {
//      if((units[0].endtime - units[0].starttime) < d)
//      { 
//        units.clear(); return true;
//      }
//      else
//        return false;
//    }
//    
//    
//    
//    list<InMemUSet>::iterator begin=units.begin(),cur=units.begin(), end, tmp;
//    typedef pair<double, list<InMemUSet>::iterator > inst;
//    typedef pair< inst , inst> lifetime;
//    map<int, lifetime> elems;
//    map<int, lifetime>::iterator elemsIt;
//    bool changed=false;
//    if(debugme)
//      Print(cerr);
//    while(begin != units.end())
//    {
//      end = GetPeriodEndUnit(begin);
//      if(debugme)
//      {
//        (*begin).Print(cerr);
//        (*end).Print(cerr);
//      }
///*
//IF the period length is less than d, remove all units within the period
//
//*/      
//      if(((*end).endtime - (*begin).starttime) < d)
//      {
//        tmp= end; ++tmp;
//        begin= units.erase(begin, tmp);
//        changed= true;
//      }
///*
//ELSE remove short parts of elements as follows:
// 1. Initialize a hashtable [mset element, its deftime]
// 2. Update the deftime of every element while iterating ovet the units
// 3. After each iteration, elements are not updated must have deftime > d, 
//    otherwise their observed part is removed from the MSet 
//*/
//      else
//      {
//        cur= begin;
//        list<InMemUSet>::iterator periodend= end;
//        for(set<int>::iterator elem= (*begin).constValue.begin(); elem !=
//          (*begin).constValue.end(); ++elem)
//        {
//          if(debugme)
//          {
//            cerr<<endl;
//            cerr<<*elem<< " ";
//          }
//            
//          lifetime lt(inst((*begin).starttime, begin), 
//              inst((*begin).endtime, begin));
//          elems.insert(pair<int, lifetime>(*elem, lt));
//        }
//        ++periodend;
//        ++cur;
//
//        while(cur != periodend)
//        {
//          for(set<int>::iterator elem= (*cur).constValue.begin(); elem !=
//                    (*cur).constValue.end(); ++elem)
//          {
//            if(debugme)
//              cerr<<endl<<*elem<<endl;
//            elemsIt = elems.find(*elem);
//            if(elemsIt != elems.end())
//            {
//              (*elemsIt).second.second.first = (*cur).endtime;
//              (*elemsIt).second.second.second = cur;
//            }
//            else
//            {
//              lifetime lt(inst((*cur).starttime, cur), 
//                inst((*cur).endtime, cur));
//              elems.insert(pair<int, lifetime>(*elem, lt));
//              if(debugme)
//              {
//                cerr<<endl;
//                cerr<<*elem<< " ";
//              }
//                
//            }
//          }
//          
//          double curtime= (*cur).endtime;
//          elemsIt= elems.end();
//          --elemsIt;
//          for(; elemsIt != elems.begin(); --elemsIt)
//          {
//            if(debugme)
//              cerr<<endl<<(*elemsIt).first;
//              
//            lifetime elemLifeTime= (*elemsIt).second;
//            if(( elemLifeTime.second.first != curtime) &&
//                elemLifeTime.second.first - elemLifeTime.first.first < d)
//            {
//              for(it= elemLifeTime.second.second; it!= 
//                elemLifeTime.first.second; --it)
//                  (*it).constValue.erase( (*elemsIt).first );
//           (*elemLifeTime.first.second).constValue.erase( (*elemsIt).first);
//              elems.erase(elemsIt);
//              changed=true;
//            }
//          }
//
//          lifetime elemLifeTime= (*elems.begin()).second;
//          if(( elemLifeTime.second.first != curtime) &&
//              elemLifeTime.second.first - elemLifeTime.first.first < d)
//          {
//            for(it= elemLifeTime.second.second; it!= 
//              elemLifeTime.first.second; --it)
//              (*it).constValue.erase( (*elems.begin()).first );
//            (*elemLifeTime.first.second).constValue.erase(
//                (*elems.begin()).first);
//            elems.erase(elems.begin());
//            changed=true;
//          }
//          ++cur;
//        }
//        begin = end;
//        ++begin;
//      }
//      if(elems.size()==0) return changed;
//      
//      elemsIt= elems.end();
//      --elemsIt;
//      for(; elemsIt != elems.begin(); --elemsIt)
//      {
//        if(debugme)
//          cerr<<endl<<(*elemsIt).first;
//
//        lifetime elemLifeTime= (*elemsIt).second;
//        if(elemLifeTime.second.first - elemLifeTime.first.first < d)
//        {
//          for(it= elemLifeTime.second.second; it!= 
//            elemLifeTime.first.second; --it)
//            (*it).constValue.erase( (*elemsIt).first );
//          (*elemLifeTime.first.second).constValue.erase( (*elemsIt).first);
//          elems.erase(elemsIt);
//          changed=true;
//        }
//      }
//      lifetime elemLifeTime= (*elems.begin()).second;
//      if(elemLifeTime.second.first - elemLifeTime.first.first < d)
//      {
//        for(it= elemLifeTime.second.second; it!= 
//          elemLifeTime.first.second; --it)
//          (*it).constValue.erase( (*elems.begin()).first );
//        (*elemLifeTime.first.second).constValue.erase(
//            (*elems.begin()).first);
//        elems.erase(elems.begin());
//        changed=true;
//      }
//    }
//    
//    return changed;
//  }
  
  list<InMemUSet>::iterator GetPeriodEndUnit(list<InMemUSet>::iterator begin)
  {
    bool debugme= false;
    if(begin == units.end())
      return begin;
 
    if(debugme)
      (*begin).Print(cerr);
    
    list<InMemUSet>::iterator end=begin;   
    double totalLength= (*begin).endtime - (*begin).starttime, curLength=0;
    ++end;
    while(end != units.end())
    {
      curLength = (*end).endtime - (*end).starttime;
      totalLength += curLength;
      if(totalLength  !=  ((*end).endtime - (*begin).starttime))
        break;
      ++end;
    }
    --end;
    return end;
  } 

  list<InMemUSet>::iterator GetPeriodStartUnit(list<InMemUSet>::iterator end)
  {
    bool debugme=false;
    if(end == units.begin())
      return end;
 
    list<InMemUSet>::iterator begin= end;   
    double totalLength= (*end).endtime - (*end).starttime, curLength=0;
    --begin;
    while(begin != units.begin())
    {
      curLength = (*begin).endtime - (*begin).starttime;
      totalLength += curLength;
      if(totalLength  !=  ((*end).endtime - (*begin).starttime))
      {
        ++begin;
        return begin;
      }
      --begin;
    }
    curLength = (*begin).endtime - (*begin).starttime;
    totalLength += curLength;
    if(totalLength  !=  ((*end).endtime - (*begin).starttime))
      ++begin;
    return begin;
  } 
  
  bool GetNextTrueUnit(MBool& mbool, int& pos, UBool& unit)
  {
    while(pos < mbool.GetNoComponents())
    {
      mbool.Get(pos, unit);
      if(unit.constValue.GetValue())
        return true;
      else
        ++pos;
    }
    return false;
  }
  
  void Union (MBool& arg, int key)
  {
    bool debugme=false;
    int no1 = units.size();
    int no2 = arg.GetNoComponents();

    if(no2==0)
      return; 

    if(no1==0)
    {
      this->ReadFrom(arg, key);
      return;
    }

    // both arguments are non-empty
    UBool ubool;
    int rhsit=0;
    InMemUSet rhs;
    if (!GetNextTrueUnit(arg, rhsit, ubool)) return;
    rhs.ReadFrom(ubool, key);
    it= units.begin();
    double a= (*it).starttime, A=(*it).endtime;
    double b= rhs.starttime, B=rhs.endtime;
    bool lcA= (*it).lc, rcA= (*it).rc;
    bool lcB= rhs.lc, rcB= rhs.rc;
    InMemMSet result;
    while(it != units.end()  &&  rhsit < arg.GetNoComponents() )
    {  
      // both arguments have units
      if(a < b){
        if (debugme) cerr<<endl<<"case 1: t1 starts before t2 ";
        // t1 starts before t2
        if(A < b){ // t1 before t2
          if (debugme) cerr<<endl<<"case 1.1: t1 ends before t2 starts";
          InMemUSet tmp((*it).constValue, a, A, lcA, rcA);
          result.units.push_back(tmp);
          ++it;
          if(it != units.end())
          {
            a= (*it).starttime; A=(*it).endtime;
            lcA= (*it).lc; rcA= (*it).rc;
          }
        } else if(A > b){
          if (debugme) cerr<<endl<<"case 1.2: t1 ends after t2 starts";
          // overlapping intervals
          InMemUSet tmp((*it).constValue, a, b, lcA, !lcB);
          result.units.push_back(tmp);
          a= b;
          lcA= lcB;
        } else { // u1.timeInterval.end == u2.timeInterval.start
          if (debugme) cerr<<endl<<"case 1.3: t1 ends when t2 starts ";
          if( !rcA  || !lcB){
            if (debugme) 
              cerr<<endl<<"case 1.3.1: t1 ends before t2 starts (closeness) ";
            // u1 before u2

            InMemUSet tmp((*it).constValue, a, A, lcA, rcA);
            result.units.push_back(tmp);
            ++it;
            if(it != units.end()){
              a= (*it).starttime; A=(*it).endtime;
              lcA= (*it).lc; rcA= (*it).rc;
            }
          } else { // intervals have a common instant
            if (debugme) 
              cerr<<endl<<"case 1.3.2: t2 ends when t2 starts (common instant)";
            InMemUSet tmp((*it).constValue, a, A, lcA, false);
            lcA = true;
            a = b;
          }
        }
      } else if(b < a){
        if (debugme) cerr<<endl<<"case 2: t2 starts before t1 starts";
        // symmetric case , u2 starts before u1
        if(B < a){ // u2 before u1
          if (debugme) cerr<<endl<<"case 2.1: t2 ends before t1 ends ";
          InMemUSet tmp(rhs.constValue, b, B, lcB, rcB);
          result.units.push_back(tmp);
          ++rhsit;
          if(GetNextTrueUnit(arg, rhsit, ubool)){
            rhs.ReadFrom(ubool, key);
            b= rhs.starttime; B=rhs.endtime;
            lcB= rhs.lc; rcB= rhs.rc;
          }
        } else if(B > a){
          if (debugme) cerr<<endl<<"case 2.2: t2 ends after t1 starts";
          // overlapping intervals
          InMemUSet tmp(rhs.constValue, b, a, lcB, !lcA);
          result.units.push_back(tmp);
          b = a;
          lcB = lcA;
        } else { // u1.timeInterval.end == u2.timeInterval.start
          if (debugme) cerr<<endl<<"case 2.3: t2 ends when t1 starts";
          if( !rcB  || !lcA){
            if (debugme) 
              cerr<<endl<<"case 2.3.1: t2 ends before t1 starts (closeness)";
            // u1 before u2
            InMemUSet tmp(rhs.constValue, b, B, lcB, rcB);
            result.units.push_back(tmp);
            ++rhsit;
            if(GetNextTrueUnit(arg, rhsit, ubool)){
              rhs.ReadFrom(ubool, key);
              b= rhs.starttime; B=rhs.endtime;
              lcB= rhs.lc; rcB= rhs.rc;
            }
          } else { // intervals have a common instant
            if (debugme) 
              cerr<<endl<<"case 2.3.2: t2 ends when t1 starts (common instant)";
            InMemUSet tmp(rhs.constValue, b, B, lcB, false);
            result.units.push_back(tmp);
            lcB = true;
            b = a;
          }
        }
      } else { // u1.timeInterval.start == u2.timeInterval.start
        if (debugme) cerr<<endl<<"case 3: t1 and t2 start at the same instant";
        // both intervals start at the same instant
        if(lcA != lcB){
          if (debugme) 
            cerr<<endl<<"case 3.1: membership of the instant differs";
          if(lcA){ // add a single instant interval
            InMemUSet tmp((*it).constValue, a, a, true, true);
            result.units.push_back(tmp);
            if(a == A){ // u1 exhausted
              ++it;
              if(it != units.end()){
                a= (*it).starttime; A=(*it).endtime;
                lcA= (*it).lc; rcA= (*it).rc;
              }
            } else {
              lcA = false;
            }
          } else {
            // symmetric case
            InMemUSet tmp(rhs.constValue, b, b, true, true);
            result.units.push_back(tmp);
            if(b == B){
              ++rhsit;
              if(GetNextTrueUnit(arg, rhsit, ubool)){
                rhs.ReadFrom(ubool, key);
                b= rhs.starttime; B=rhs.endtime;
                lcB= rhs.lc; rcB= rhs.rc;
              }
            } else {
              lcB = false;
            }
          }
        } else { // intervals start exact at the same instant
          if (debugme) cerr<<endl<<"case 3.2: intervalls start exact together";
          set<int> opres;
          (*it).Union(rhs.constValue, opres);

          if(A < B){
            if (debugme) cerr<<endl<<"case 3.2.1: t1 ends before t2 ends";
            InMemUSet tmp(opres, a, A, lcA, rcA);
            result.units.push_back(tmp);
            b = A;
            lcB = !rcA;
            ++it;
            if(it != units.end()){
              a= (*it).starttime; A=(*it).endtime;
              lcA= (*it).lc; rcA= (*it).rc;
            }
          } else if (B < A){
            if (debugme) cerr<<endl<<"case 3.2.2: t2 ends before t1 ends";
            InMemUSet tmp(opres, b, B, lcB, rcB);
            result.units.push_back(tmp);
            a = B;
            lcA = !rcB;
            ++rhsit;
            if(GetNextTrueUnit(arg, rhsit, ubool)){
              rhs.ReadFrom(ubool, key);
              b= rhs.starttime; B=rhs.endtime;
              lcB= rhs.lc; rcB= rhs.rc;
            }
          } else { // both units end at the same instant
            if (debugme) 
              cerr<<endl<<"case 3.2.3: both intervals ends at the same instant";
            if(rcA == rcB){  // equal intervals
              if (debugme) cerr<<endl<<"case 3.2.3.1: intervals are equal" ;
              InMemUSet tmp(opres, a, A, lcA, rcA);
              result.units.push_back(tmp);
              ++it;
              if(it != units.end()){
                a= (*it).starttime; A=(*it).endtime;
                lcA= (*it).lc; rcA= (*it).rc;
              }
              ++rhsit;
              if(GetNextTrueUnit(arg, rhsit, ubool)){
                rhs.ReadFrom(ubool, key);
                b= rhs.starttime; B=rhs.endtime;
                lcB= rhs.lc; rcB= rhs.rc;
              }
            } else {
              if (debugme) 
                cerr<<endl<<"case 3.2.3.2: intervals differ at right closeness";
              // process common part
              InMemUSet tmp(opres, a, A, lcA, false);
              result.units.push_back(tmp);
              if(rcA){
                ++rhsit;
                if(GetNextTrueUnit(arg, rhsit, ubool)){
                  rhs.ReadFrom(ubool, key);
                  b= rhs.starttime; B=rhs.endtime;
                  lcB= rhs.lc; rcB= rhs.rc;
                }
                lcA = true;
                a = A;
              } else {
                ++it;
                if(it != units.end()){
                  a= (*it).starttime; A=(*it).endtime;
                  lcA= (*it).lc; rcA= (*it).rc;
                }
                lcB = true;
                b = B;
              }
            }
          }
        }
      }
    }

    if (debugme) cerr<<endl<<"one of the arguments is finished";

    // process remainder of m1
    while(it != units.end()){
      InMemUSet tmp((*it).constValue, a, A, lcA, rcA);
      result.units.push_back(tmp);
      ++it;
      if(it != units.end()){
        a= (*it).starttime; A=(*it).endtime;
        lcA= (*it).lc; rcA= (*it).rc;
      }

    }
    // process remainder of m2
    while(rhsit < arg.GetNoComponents()){
      InMemUSet tmp(rhs.constValue, b, B, lcB, rcB);
      result.units.push_back(tmp);
      ++rhsit;
      if(GetNextTrueUnit(arg, rhsit, ubool)){
        rhs.ReadFrom(ubool, key);
        b= rhs.starttime; B=rhs.endtime;
        lcB= rhs.lc; rcB= rhs.rc;
      }
    }
    this->CopyFrom(result);
  }


  
  list<InMemUSet> units;
  list<InMemUSet>::iterator it;
};


class CompressedInMemUSet
{
public:
 
  CompressedInMemUSet():count(0){}
  void Erase(int victim)
  {
    it= added.find(victim);
    if(it != added.end())
      added.erase(it);
    else
      removed.insert(victim);
  }
  void Insert(int elem)
  {
    it= removed.find(elem);
    if(it != removed.end())
      removed.erase(it);
    else
      added.insert(elem);
  }
  ostream& Print( ostream &os ) 
  {
    char l= (lc)? '[' : '(';
    char r= (rc)? ']' : ')';
    Instant i1(instanttype), i2(instanttype);
    i1.ReadFrom(starttime/day2min);
    i2.ReadFrom(endtime/day2min); 
    os<< endl<< l<< i1<< "\t"<< i2<< r<< "\tCount= "<< count;
    return os;
  }
  
  double starttime, endtime;
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
  
  CompressedInMemMSet(){}
  CompressedInMemMSet(CompressedInMemMSet& arg, 
      list<CompressedInMemUSet>::iterator begin,
      list<CompressedInMemUSet>::iterator end)
      {
        CopyFrom(arg, begin, end);
      }
  
  int GetNoComponents()
  {
    return units.size();
  }
  void CopyFrom(CompressedInMemMSet& arg, 
      list<CompressedInMemUSet>::iterator begin,
      list<CompressedInMemUSet>::iterator end)
  {
    bool debugme= false;
    Clear();
    set<int> accumlator;
    for(arg.it= arg.units.begin(); arg.it != begin; ++arg.it)
    {
      accumlator.insert((*arg.it).added.begin(), (*arg.it).added.end());
      for(set<int>::iterator i= (*arg.it).removed.begin();
        i!=(*arg.it).removed.end(); ++i)
      {
        int cnt= accumlator.erase(*i);
        assert(cnt > 0);
      }
    }
    
    if(debugme)
    {
      int tmp= accumlator.size();
      cerr << endl<<tmp;
    }
    
    CompressedInMemUSet first= *begin;
    first.added.insert(accumlator.begin(), accumlator.end());
    units.push_back(first);
    arg.it= begin;
    ++arg.it;
    for(; arg.it != end; ++arg.it)
      units.push_back(*arg.it);
    units.push_back(*end);
  }
  void Clear()
  {
    buffer.clear();
    units.clear();
  }
  void GetSet(list<CompressedInMemUSet>::iterator index, set<int>& res)
  {
    res.clear();
    if(index == units.end()) return;
    list<CompressedInMemUSet>::iterator i= units.begin();
    for(; i!= index; ++i)
    {
      res.insert( (*i).added.begin(), (*i).added.end() );
      for(set<int>::iterator elem= (*i).removed.begin(); 
        elem != (*i).removed.end(); ++elem)
        res.erase(*elem);
    }
    res.insert( (*index).added.begin(), (*index).added.end() );
    for(set<int>::iterator elem= (*index).removed.begin(); 
      elem != (*index).removed.end(); ++elem)
      res.erase(*elem);
  }
  void WriteToMSet(MSet& res)
  {
    set<int> constValue;
    if(units.begin() == units.end())
    {
      res.SetDefined(false);
      return;
    }
    
    res.Clear();
    res.SetDefined(true);
    for(it=units.begin(); it != units.end(); ++it)
    {
      USet uset(true);
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
      WriteUSet(constValue, *it, uset);
      res.MergeAdd(uset);
    }
  }
  
  void WriteUSet(set<int>& val, CompressedInMemUSet& source, USet& res)
  {
    res.constValue.Clear();
    res.timeInterval.start.ReadFrom(source.starttime/day2min);
    res.timeInterval.start.SetType(instanttype);
    res.timeInterval.end.ReadFrom(source.endtime/day2min);
    res.timeInterval.end.SetType(instanttype);
    res.timeInterval.lc= source.lc;
    res.timeInterval.rc= source.rc;
    for(set<int>::iterator i= val.begin(); i != val.end(); ++i)
      res.constValue.Insert(*i);
    res.SetDefined(true);
    res.constValue.SetDefined(true);
    assert(res.IsValid());
  }
  
  void WriteToMSet(MSet& res, list<CompressedInMemUSet>::iterator begin, 
      list<CompressedInMemUSet>::iterator end)
  {
    if(begin == units.end())
    {
      res.SetDefined(false);
      return;
    }
    res.Clear();
    res.SetDefined(true);
    set<int> constValue;
    for(it=units.begin(); it != begin; ++it)
    {
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
    }
    for(it=begin; it != end; ++it)
    {
      USet uset(true);
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
      WriteUSet(constValue, *it, uset);
      res.MergeAdd(uset);
    }
  }
  
  void WriteToInMemMSet(InMemMSet& res, 
      list<CompressedInMemUSet>::iterator begin, 
      list<CompressedInMemUSet>::iterator end)
  {
    if(begin == units.end())
    {
      res.Clear();
      return;
    }
    res.Clear();
    set<int> constValue;
    for(it=units.begin(); it != begin; ++it)
    {
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
    }
    for(it=begin; it != end; ++it)
    {
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
      InMemUSet uset(constValue, (*it).starttime, 
                (*it).endtime, (*it).lc, (*it).rc);
      res.units.push_back(uset);
    }
  }
  
  void WriteToInMemMSet(InMemMSet& res)
  {
     
    list<CompressedInMemUSet>::iterator begin = units.begin(); 
    list<CompressedInMemUSet>::iterator end = units.end();
    if(begin == units.end())
    {
      res.Clear();
      return;
    }
    res.Clear();
    set<int> constValue;
    
    for(it=begin; it != end; ++it)
    {
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
      InMemUSet uset(constValue, (*it).starttime, 
                (*it).endtime, (*it).lc, (*it).rc);
      res.units.push_back(uset);
    }
  }
  
  ostream& Print( ostream &os ) 
  {
    if( units.size() == 0 )
    {
      return os << "(InMemMSet: undefined)";
    }
    os << "(InMemMSet: defined, contains " << units.size() << " units: ";
    
    set<int> constValue;
    for(it=units.begin(); it != units.end(); ++it)
    {
      if((*it).added.size() != 0)
        constValue.insert((*it).added.begin(), (*it).added.end());
      if((*it).removed.size() != 0)
      {
        for((*it).it= (*it).removed.begin(); 
          (*it).it != (*it).removed.end(); ++(*it).it)
          constValue.erase(*(*it).it);
      }
      os<< "\n Set cardinality = "<< constValue.size()<<" {";
      for(set<int>::iterator k= constValue.begin(); k != constValue.end();++k)
        os<< *k<< ", ";
      os<<"}";
    }
    os << "\n)" << endl;
    return os;
  }
  list<CompressedInMemUSet>::iterator 
    EraseUnit(list<CompressedInMemUSet>::iterator pos)
  {
    bool debugme= false;
    if(pos == units.end())
      return pos;
    list<CompressedInMemUSet>::iterator next= pos;
    ++next;
    if(next == units.end())
    {
      next= units.erase(pos);
      return next;
    }
    
    if(debugme)
    {
      (*pos).Print(cerr);
      cerr<<endl;
      (*next).Print(cerr);
    }
    vector<int> common((*pos).added.size() + (*pos).removed.size()); 
    vector<int>::iterator last;
    last= set_intersection((*pos).added.begin(), (*pos).added.end(),
        (*next).removed.begin(), (*next).removed.end(),
        common.begin());
    for(vector<int>::iterator i= common.begin(); i< last; ++i)
    {
      (*next).removed.erase(*i);
      (*pos).added.erase(*i);
    }
    (*next).added.insert((*pos).added.begin(), (*pos).added.end());
    
    last= set_intersection((*pos).removed.begin(), (*pos).removed.end(),
        (*next).added.begin(), (*next).added.end(),
        common.begin());
    for(vector<int>::iterator i= common.begin(); i< last; ++i)
    {
      (*next).added.erase(*i);
      (*pos).removed.erase(*i);
    } 
    
    (*next).removed.insert((*pos).removed.begin(), (*pos).removed.end());

    if(debugme)
    {
      (*pos).Print(cerr);
      cerr<<endl;
      (*next).Print(cerr);
    }
    next= units.erase(pos);
    return next;
  }
  
  list<CompressedInMemUSet>::iterator 
    EraseUnits(list<CompressedInMemUSet>::iterator start, 
        list<CompressedInMemUSet>::iterator end)
  {
    if(start== units.end()) return start;
    list<CompressedInMemUSet>::iterator pos= end;
    --pos;
    while(pos != start)
    {
      pos= EraseUnit(pos);
      --pos;
    }
    pos= EraseUnit(pos);
    return pos;
  }
  
  bool RemoveSmallUnits(const unsigned int n)
  {
    bool debugme= false;
    bool changed=false;
    if(units.size()==0) return false;
    list<CompressedInMemUSet>::iterator i= units.end();
    --i;
    while(i!= units.begin())
    {
      if(debugme) {cerr<<endl<< (*i).count;}
      if((*i).count < n)
      {
        i= EraseUnit(i);
        changed= true;
      }
      --i;
      if(debugme)
      {
        MSet tmp1(0);
        WriteToMSet(tmp1);
        tmp1.Print(cerr);
      }
    }
    if((*units.begin()).count < n)
    {
        EraseUnit(units.begin());
        changed = true;
    }
    return changed;
  }
  

  typedef pair<double, list<CompressedInMemUSet>::iterator > inst;
  ostream& Print( map<int, inst> elems, ostream &os )
  {
    map<int, inst>::iterator elemsIt= elems.begin();
    while(elemsIt != elems.end())
    {
      os<<(*elemsIt).first<<" ";
      os<< "\n Set cardinality = "<< (*(*elemsIt).second.second).count<<" {";
//   for(set<int>::iterator k= (*(*elemsIt).second.second).constValue.begin(); 
//        k != (*(*elemsIt).second.second).constValue.end();++k)
//        os<< *k<< ", ";
//      os<<"}";
//      os<<endl;
      ++elemsIt;
    }
    return os;
    
  }
  bool RemoveShortElemParts(const double d)
  {
    bool debugme= false;
    
    //handling special cases
    if(units.size()==0) return false;
    if(units.size()==1)
    {
      if( ( (*units.begin()).endtime - (*units.begin()).starttime) < d)
      { 
        units.clear(); return true;
      }
      else
        return false;
    }
    
    list<CompressedInMemUSet>::iterator cur= units.begin() , prev, end, tmp;
//    typedef pair<double, list<InMemUSet>::iterator > inst;
    map<int, inst> elems;
    map<int, inst>::iterator elemsIt;
    bool changed=false;
   
    
    while(cur != units.end())
    {
      end = GetPeriodEndUnit(cur);
      if(debugme)
      {
        MSet tmp(0);
        WriteToMSet(tmp);
        tmp.Print(cerr);
        cerr<<"\nCur:  "; (*cur).Print(cerr);
        cerr<<"\nEnd:  "; (*end).Print(cerr);
      }


/*
IF the period length is less than d, remove all units within the period

*/      
      if(((*end).endtime - (*cur).starttime) < d)
      {
        ++end;
        cur= EraseUnits(cur, end);
        changed= true;
        if(debugme)
        {
          MSet tmp(0);
          WriteToMSet(tmp);
          tmp.Print(cerr);
        }   continue;
      }
/*
ELSE remove short parts of elements as follows:
 1. Initialize a hashtable [mset element, its deftime]
 2. Update the deftime of every element while iterating ovet the units
 3. After each iteration, elements are not updated must have deftime > d, 
    otherwise their observed part is removed from the MSet 

*/
      else
      {
        elems.clear();
        //assert( (*cur).removed.size() == 0);
        set<int> initialIdSet;
        GetSet(cur, initialIdSet);
        for(set<int>::iterator elem= initialIdSet.begin(); elem !=
          initialIdSet.end(); ++elem)
        {
          if(debugme)
            cerr<<endl<<*elem;

          inst lt((*cur).starttime, cur);
          elems.insert(pair<int, inst>(*elem, lt));
        }
        if(debugme)
          Print(elems, cerr);
        prev= cur; ++cur;        
        ++end;
        while(cur != end)
        {
          if(debugme)
          {cerr<<"\nCur:  "; (*cur).Print(cerr);}
          set<int>::iterator diffIt= (*cur).removed.begin();
          while(diffIt != (*cur).removed.end())
          {
            int elemToRemove= *diffIt;
            if(debugme)
              cerr<< elemToRemove;
            elemsIt= elems.find(elemToRemove);
            assert(elemsIt != elems.end());
            if( ((*cur).starttime - (*elemsIt).second.first) < d )
            {
              list<CompressedInMemUSet>::iterator unitToChange= 
                (*elemsIt).second.second;
              (*unitToChange).Erase(elemToRemove);
              while(unitToChange != cur)
              {
                --((*unitToChange).count);
                ++unitToChange;
              }
              (*cur).removed.erase(diffIt);
              changed = true;
            }
            elems.erase(elemsIt);
            ++diffIt;
          }
          if(debugme)
            Print(elems, cerr);
          
          
          //Add the new elements that starts to appear in the cur unit
          diffIt= (*cur).added.begin();
          while(diffIt != (*cur).added.end())          
          {
            if(debugme)
              cerr<<endl<<*diffIt;
            inst lt((*cur).starttime, cur);
            elems.insert(pair<int, inst>(*diffIt, lt));
            ++diffIt;
          }
          prev= cur; ++cur;
          if(debugme)
          {
            Print(elems, cerr);
            MSet tmp(0);
            WriteToMSet(tmp);
            tmp.Print(cerr);
          }
        }
        for(elemsIt= elems.begin(); elemsIt != elems.end(); ++elemsIt)
        {
          if(debugme)
          {
            cerr<<(*elemsIt).first;
            cerr<<endl<<((*prev).endtime - (*elemsIt).second.first);
          }
          if( ((*prev).endtime - (*elemsIt).second.first) < d )
          {
            list<CompressedInMemUSet>::iterator unitToChange= 
              (*elemsIt).second.second;
            //Erase from the current period
            (*unitToChange).Erase((*elemsIt).first); 
            while(unitToChange != cur)
            {
              --((*unitToChange).count);
              ++unitToChange;
            }
            if(end != units.end())
              //insert again in the following period
              (*end).Insert((*elemsIt).first);      
            changed = true;
          }
        }
      }
      cur=end;
    }
    return changed;
  }
  
  list<CompressedInMemUSet>::iterator GetPeriodEndUnit(
      list<CompressedInMemUSet>::iterator begin)
  {
    bool debugme= false;
    if(begin == units.end())
      return begin;
 
    if(debugme)
    {
      cerr<< "\n Set cardinality = "<< (*begin).count<<" {";
//      for(set<int>::iterator k= (*begin).constValue.begin(); 
//        k != (*begin).constValue.end();++k)
//        cerr<< *k<< ", ";
      cerr<<"}";
    }
    
    list<CompressedInMemUSet>::iterator end=begin;   
    double totalLength= (*begin).endtime - (*begin).starttime, curLength=0;
    ++end;
    while(end != units.end())
    {
      curLength = (*end).endtime - (*end).starttime;
      totalLength += curLength;
      if(totalLength  !=  ((*end).endtime - (*begin).starttime))
        break;
      ++end;
    }
    --end;
    return end;
  } 
  
  bool GetNextTrueUnit(MBool& mbool, int& pos, UBool& unit)
  {
    while(pos < mbool.GetNoComponents())
    {
      mbool.Get(pos, unit);
      if(unit.constValue.GetValue())
        return true;
      else
        ++pos;
    }
    return false;
  }
  
  void Buffer (MBool& arg, int key)
  {
    bool debugme=false;

    if(arg.GetNoComponents()==0)
      return; 

    UBool ubool;
    int cur=0;
    double starttime, endtime;
    bool lc, rc;
    while(GetNextTrueUnit(arg, cur, ubool))
    {
      assert(ubool.IsValid());
      starttime= ubool.timeInterval.start.ToDouble() * day2min;
      endtime= ubool.timeInterval.end.ToDouble() * day2min;
      lc= ubool.timeInterval.lc;
      rc= ubool.timeInterval.rc;
      Event entrance(key,  (lc)? closedstart: openstart);
      Event theleave(key, (rc)? closedend: openend);
      buffer.insert(pair<double, Event>(starttime, entrance));
      buffer.insert(pair<double, Event>(endtime, theleave));
      ++cur;
    }
  }
  void ClassifyEvents(pair< multimap<double, Event>::iterator, 
      multimap<double, Event>::iterator >& events, 
      map<EventType, vector<multimap<double, Event>::iterator> >& eventClasses)
  {
    eventClasses.clear();
    if(events.first == events.second) return;
    for(multimap<double, Event>::iterator k= events.first; 
      k != events.second; ++k)
    {
      if((*k).second.type == openstart)
        eventClasses[openstart].push_back(k);
      else if((*k).second.type == closedstart)
        eventClasses[closedstart].push_back(k);
      else if((*k).second.type == openend)
        eventClasses[openend].push_back(k);
      else if((*k).second.type == closedend)
        eventClasses[closedend].push_back(k);
      else
        assert(0);
    }
  }
  
  void AddUnit(double starttime, double endtime, bool lc, bool rc, 
      set<int>& elemsToAdd, set<int>& elemsToRemove, int elemsCount)
  {
    bool debugme= false;
    if(elemsCount == 0) return;
    CompressedInMemUSet unit;
    unit.starttime= starttime; unit.endtime= endtime;
    unit.lc= lc; unit.rc= rc; 
    unit.added= elemsToAdd; unit.removed= elemsToRemove;
    unit.count= elemsCount;
    units.push_back(unit);
    if(debugme)
    {
      char l= (unit.lc)? '[' : '(', r = (unit.rc)? ']' : ')';
      Instant i1(instanttype),i2(instanttype);
      i1.ReadFrom(unit.starttime/day2min);
      i2.ReadFrom(unit.endtime/day2min);
      cerr<<"\nAdding unit "<< l ; i1.Print(cerr); cerr<<", ";
      i2.Print(cerr); cerr<<r<< "  count="<< unit.count;
    }
  }
  
//  void ConstructPeriodFromBuffer(
//      multimap<EventInstant, Event, classcomp>::iterator periodStart,
//      multimap<EventInstant, Event, classcomp>::iterator periodEnd)
//  {
//    initialSet.clear();
//    units.clear();
//    if(periodStart == periodEnd) return;
//
//    multimap<EventInstant, Event, classcomp>::iterator cur=periodStart;
//    pair< EventInstant, Event, classcomp>::iterator, 
//    multimap<EventInstant, Event, classcomp>::iterator > events;
//    //map<EventType, vector<multimap<double, Event>::iterator> > eventClasses;
//    //vector<multimap<double, Event>::iterator >::iterator i;
//    EventInstant starttime, curtime, i;
//    set<int> elemsToAdd, elemsToRemove;
//
//    starttime= (*cur).first;
//    events = buffer.equal_range(starttime);
//    assert(events.first != events.second);
//    if(!starttime.side == 0)  //closed start 
//    {
//      elemsToAdd.clear();
//      for(i= events.first; i!= events.second; ++i)
//        elemsToAdd.insert( (*i).second.obj);
//      elemsToRemove.clear();
//      ++cur;
//      starttime= (*cur).first;
//      if(!starttime.side == 1)  //closed start 
//      {
//        //create a unit [starttime, starttime]
//        AddUnit(starttime, starttime, true, true, elemsToAdd, elemsToRemove);
//
//        //open a unit (starttime,...
//        elemsToAdd.clear();
//        for(i= eventClasses[openstart].begin(); i!= 
//          eventClasses[openstart].end(); ++i)
//          elemsToAdd.insert((*(*i)).second.obj);
//        lc= false;
//      }
//      else
//      {
//        //open a unit [starttime, ...
//        lc= true;
//      }
//    }
//    else
//    {
//      //open a unit (starttime, ...
//      elemsToAdd.clear();
//      for(i= eventClasses[openstart].begin(); i!= 
//        eventClasses[openstart].end(); ++i)
//        elemsToAdd.insert((*(*i)).second.obj);
//      elemsToRemove.clear();
//      lc=false;
//    }
//
//    ++cur;
//    while(cur != periodEnd)
//    {
//      curtime= (*cur).first;
//      events = buffer.equal_range(curtime);
//      assert(events.first != events.second);
//      ClassifyEvents(events, eventClasses);
//      if(eventClasses[closedend].empty() && eventClasses[openstart].empty())
//      {
//        //close unit ..., curtime)
//        AddUnit(starttime, curtime, lc, false, elemsToAdd, elemsToRemove);
//        //open unit [curtime, ...
//        starttime= curtime;
//        elemsToAdd.clear();
//        elemsToRemove.clear();
//        for(i= eventClasses[closedstart].begin(); i!= 
//          eventClasses[closedstart].end(); ++i)
//          elemsToAdd.insert((*(*i)).second.obj);
//        for(i= eventClasses[openend].begin(); i!= 
//          eventClasses[openend].end(); ++i)
//          elemsToRemove.insert((*(*i)).second.obj);
//        lc=true;
//      }
//      else 
//   if (eventClasses[openend].empty() && eventClasses[closedstart].empty())
//        {
//          //close unit ..., curtime]
//          AddUnit(starttime, curtime, lc, true, elemsToAdd, elemsToRemove);
//          //open unit (curtime, ...
//          starttime= curtime;
//          elemsToAdd.clear();
//          elemsToRemove.clear();
//          for(i= eventClasses[openstart].begin(); i!= 
//            eventClasses[openstart].end(); ++i)
//            elemsToAdd.insert((*(*i)).second.obj);
//          for(i= eventClasses[closedend].begin(); i!= 
//            eventClasses[closedend].end(); ++i)
//            elemsToRemove.insert((*(*i)).second.obj);
//          lc=false;
//        }
//        else  //a mix
//        {
//          //close a unit ..., curtime)
//          AddUnit(starttime, curtime, lc, false, elemsToAdd, elemsToRemove);
//          //create a unit [curtime, curtime]
//          elemsToAdd.clear();
//          elemsToRemove.clear();
//          for(i= eventClasses[closedstart].begin(); i!= 
//            eventClasses[closedstart].end(); ++i)
//            elemsToAdd.insert((*(*i)).second.obj);
//          for(i= eventClasses[openend].begin(); i!= 
//            eventClasses[openend].end(); ++i)
//            elemsToRemove.insert((*(*i)).second.obj);
//          AddUnit(starttime, curtime, lc, false, elemsToAdd, elemsToRemove);
//          //open a unit (curtime, ...
//          starttime= curtime;
//          elemsToAdd.clear();
//          elemsToRemove.clear();
//          for(i= eventClasses[openstart].begin(); i!= 
//            eventClasses[openstart].end(); ++i)
//            elemsToAdd.insert((*(*i)).second.obj);
//          for(i= eventClasses[closedend].begin(); i!= 
//            eventClasses[closedend].end(); ++i)
//            elemsToRemove.insert((*(*i)).second.obj);
//          lc=false;
//        }
//      ++cur;
//    }
//  }
//  
  void ConstructPeriodFromBuffer(multimap<double, Event>::iterator periodStart,
      multimap<double, Event>::iterator periodEnd)
  {
    units.clear();
    if(periodStart == periodEnd) return;
    
    multimap<double, Event>::iterator cur=periodStart;
    pair< multimap<double, Event>::iterator, 
      multimap<double, Event>::iterator > events;
    map<EventType, vector<multimap<double, Event>::iterator> > eventClasses;
    vector<multimap<double, Event>::iterator >::iterator i;
    double starttime; bool lc;
    double curtime;
    set<int> elemsToAdd, elemsToRemove;
    int curElemsCount=0;
    
    starttime= (*cur).first;
    events = buffer.equal_range(starttime);
    assert(events.first != events.second);
    ClassifyEvents(events, eventClasses);
    assert(eventClasses[openend].empty() && eventClasses[closedend].empty());
    if(!eventClasses[closedstart].empty())
    {
      elemsToAdd.clear();
      for(i= eventClasses[closedstart].begin(); i!= 
        eventClasses[closedstart].end(); ++i)
        elemsToAdd.insert( (*(*i)).second.obj);
      elemsToRemove.clear();
      if(!eventClasses[openstart].empty())
      {
        //create a unit [starttime, starttime]
        AddUnit(starttime, starttime, true, true, elemsToAdd, 
            elemsToRemove, curElemsCount);
        
        //open a unit (starttime,...
        elemsToAdd.clear();
        for(i= eventClasses[openstart].begin(); i!= 
          eventClasses[openstart].end(); ++i)
          elemsToAdd.insert((*(*i)).second.obj);
        lc= false;
      }
      else
      {
        //open a unit [starttime, ...
        lc= true;
      }
      curElemsCount+= elemsToAdd.size();
    }
    else
    {
      //open a unit (starttime, ...
      elemsToAdd.clear();
      for(i= eventClasses[openstart].begin(); i!= 
        eventClasses[openstart].end(); ++i)
        elemsToAdd.insert((*(*i)).second.obj);
      elemsToRemove.clear();
      lc=false;
      curElemsCount+= elemsToAdd.size();
    }
    for(multimap<double,Event>::iterator k=events.first; k!= events.second;++k) 
      ++cur;
    while(cur != periodEnd)
    {
      curtime= (*cur).first;
      events = buffer.equal_range(curtime);
      assert(events.first != events.second);
      ClassifyEvents(events, eventClasses);
      if(eventClasses[closedend].empty() && eventClasses[openstart].empty())
      {
        //close unit ..., curtime)
        AddUnit(starttime, curtime, lc, false, elemsToAdd, 
            elemsToRemove, curElemsCount);
        //open unit [curtime, ...
        starttime= curtime;
        elemsToAdd.clear();
        elemsToRemove.clear();
        for(i= eventClasses[closedstart].begin(); i!= 
          eventClasses[closedstart].end(); ++i)
          elemsToAdd.insert((*(*i)).second.obj);
        for(i= eventClasses[openend].begin(); i!= 
          eventClasses[openend].end(); ++i)
          elemsToRemove.insert((*(*i)).second.obj);
        lc=true;
        curElemsCount+= elemsToAdd.size() - elemsToRemove.size();
      }
      else 
      if (eventClasses[openend].empty() && eventClasses[closedstart].empty())
      {
        //close unit ..., curtime]
        AddUnit(starttime, curtime, lc, true, elemsToAdd, 
            elemsToRemove, curElemsCount);
        //open unit (curtime, ...
        starttime= curtime;
        elemsToAdd.clear();
        elemsToRemove.clear();
        for(i= eventClasses[openstart].begin(); i!= 
          eventClasses[openstart].end(); ++i)
          elemsToAdd.insert((*(*i)).second.obj);
        for(i= eventClasses[closedend].begin(); i!= 
          eventClasses[closedend].end(); ++i)
          elemsToRemove.insert((*(*i)).second.obj);
        lc=false;
        curElemsCount+= elemsToAdd.size() - elemsToRemove.size();
      }
      else  //a mix
      {
        //close a unit ..., curtime)
        AddUnit(starttime, curtime, lc, false, elemsToAdd, 
            elemsToRemove, curElemsCount);
        //create a unit [curtime, curtime]
        elemsToAdd.clear();
        elemsToRemove.clear();
        for(i= eventClasses[closedstart].begin(); i!= 
          eventClasses[closedstart].end(); ++i)
          elemsToAdd.insert((*(*i)).second.obj);
        for(i= eventClasses[openend].begin(); i!= 
          eventClasses[openend].end(); ++i)
          elemsToRemove.insert((*(*i)).second.obj);
        curElemsCount+= elemsToAdd.size() - elemsToRemove.size();
        AddUnit(curtime, curtime, true, true, elemsToAdd, 
            elemsToRemove, curElemsCount);
        //open a unit (curtime, ...
        starttime= curtime;
        elemsToAdd.clear();
        elemsToRemove.clear();
        for(i= eventClasses[openstart].begin(); i!= 
          eventClasses[openstart].end(); ++i)
          elemsToAdd.insert((*(*i)).second.obj);
        for(i= eventClasses[closedend].begin(); i!= 
          eventClasses[closedend].end(); ++i)
          elemsToRemove.insert((*(*i)).second.obj);
        lc=false;
        curElemsCount+= elemsToAdd.size() - elemsToRemove.size();
      }
      for(map<double,Event>::iterator k= events.first; k!= events.second; ++k) 
         ++cur;
    }
  }
  
  
  void ConstructFromBuffer()
  {   
    ConstructPeriodFromBuffer(buffer.begin(), buffer.end());
  }
  
  multimap<double, Event> buffer;
  list<CompressedInMemUSet> units;
  list<CompressedInMemUSet>::iterator it;
};


void IntSet::Insert(int elem)
{
  int curElem, tmp;
  if(this->Count()==0)
  {
    points.Put(0, elem);
    return;
  }
  if(this->Count()==1)
  {
    points.Get(0, tmp);
    if(tmp < elem)
      points.Put(1, elem);
    else if (tmp > elem)
    {
      points.Put(1, tmp);
      points.Put(0, elem);
    }
    return;
  }
  points.Get(this->Count()-1, tmp);
  if(elem > tmp)
  {
    points.Put(this->Count(), elem);
    return;
  }
  points.Get(0, tmp);
  if(elem < tmp)
  {
    for(int i= this->Count() ; i>0 ; --i)
    {
      points.Get(i-1, tmp);
      points.Put(i, tmp);
    }
    points.Put(0, elem);
    return;
  }
  
  int lb=0,ub=this->Count()-1,mid;    //lb=>lower bound,ub=>upper bound  
  while( (ub-lb) > 1)
  {
    mid= (lb+ub)/2;
    points.Get(mid, curElem);
    if(curElem==elem)
      return; //element already in set

    else
      if(curElem < elem)
        lb=mid;
        
    else
      if(curElem>elem)
        ub=mid;
  }
  
  for(int i= this->Count() ; i>ub ; --i)
  {
    points.Get(i-1, tmp);
    points.Put(i, tmp);
  }
  
  points.Put(ub, elem);
}

void IntSet::Union(IntSet& rhs, IntSet& res)
{
  if(!(this->IsDefined() && rhs.IsDefined()))
  {
    res.SetDefined(false);
    return;
  }
  res.SetDefined(true);
  res.Clear();
  res.CopyFrom(this);
  for(int i=0; i<rhs.Count(); ++i)
    res.Insert(rhs[i]);
}

void IntSet::Union(IntSet& rhs)
{
  if(!(this->IsDefined() && rhs.IsDefined()))
  {
    SetDefined(false);
    return;
  }
  for(int i=0; i<rhs.Count(); ++i)
    Insert(rhs[i]);
}

void IntSet::Clear()
{
  points.clean();
}

IntSet::IntSet(int numElem):points(numElem)
{
  this->SetDefined(true); 
}

IntSet::IntSet(bool def):points(0)
{
  this->SetDefined(def);
}

IntSet::IntSet(const IntSet& arg):points(0)
{
  if(!arg.IsDefined())
  {
    this->SetDefined(false);  
  }
  else
  {
    this->points.resize(arg.Count());
    this->SetDefined(true);
    for(int i=0; i< arg.Count(); ++i)
      this->Insert(arg[i]);
  }
}

IntSet::~IntSet(){}

//IntSet functions
bool IntSet::IsSubset(const IntSet& rhs) const
{
  if(!(this->IsDefined() && rhs.IsDefined()))
    return false;
  if(this->Count() > rhs.Count())
    return false;
  
  int l=0, r=0;
  while(l < this->Count() && r < rhs.Count())
  {
    if( (*this)[l] < rhs[r]) return false;
    else if ( (*this)[l] == rhs[r]) {++l; ++r;}
    else ++r;
  }
  return (l == this->Count());
}

bool IntSet::operator==(const IntSet& rhs) const
{
  if(this->Count() != rhs.Count()) 
    return false;
  for(int i=0; i < this->Count() ; ++i)
    if( (*this)[i] != rhs[i] )
      return false;
  return true;
}

bool IntSet::operator<(const IntSet& rhs) const
{
  return (this->Count() < rhs.Count());
}

IntSet* IntSet::Intersection(const IntSet& rhs) const
{
  if(!(this->IsDefined() && rhs.IsDefined()))
    return 0;
  IntSet* res(0);  
  int l=0, r=0;
  while(l < this->Count() && r < rhs.Count())
  {
    if( (*this)[l] < rhs[r]) l++;
    else if ( (*this)[l] == rhs[r]) {res->Insert((*this)[l]);}
    else ++r;
  }
  return res;
}

void IntSet::Intersection2(const IntSet& rhs) 
{
  if(!(this->IsDefined() && rhs.IsDefined()))
    return;
  IntSet* res(0);  
  int l=0, r=0;
  while(l < this->Count() && r < rhs.Count())
  {
    if( (*this)[l] < rhs[r]) l++;
    else if ( (*this)[l] == rhs[r]) {res->Insert((*this)[l]);}
    else ++r;
  }
  this->CopyFrom(res);
  res->DeleteIfAllowed(true);
}

int IntSet::IntersectionCount(const IntSet& rhs) const
{
  if(!(this->IsDefined() && rhs.IsDefined()))
    return 0;  
  int l=0, r=0, res=0;
  while(l < this->Count() && r < rhs.Count())
  {
    if((*this)[l] < rhs[r]) l++;
    else if ((*this)[l] == rhs[r]) {res++;}
    else ++r;
  }
  return res;
}

int IntSet::BinSearch(int elem)
{       
  int curElem=0;
  int lb=0,ub=this->Count()-1,mid;    //lb=>lower bound,ub=>upper bound

  for(;lb<ub;)
  {
    mid=(lb+ub)/2;
    points.Get(mid, curElem);
    if(curElem==elem)
      return mid;

    else
      if(curElem < elem)
        ub=mid-1;
    else
      if(curElem>elem)
        lb=mid+1;
  }
  if(ub<lb)
    return -1;
  return -1;
}

void IntSet::Delete(const int elem)
{
  int tmp=0;
  int pos= BinSearch(elem);
  if(pos != -1)
  {  
    for(int i=pos; i < this->Count()-1 ; ++i)
    {
      this->points.Get(i+1, tmp);
      this->points.Put(i, tmp);
    }
  }
  this->points.resize(this->points.Size()-1);
}

int IntSet::Count()const
{
  return points.Size();
}

/*
mosh 3aref

*/
int IntSet::operator[](int index) const
{
  if(index >= points.Size())
    assert(0);
  int elem;
  this->points.Get(index, elem);
  return (elem);
}

/*
members required for the Attribute interface

*/
size_t IntSet::HashValue() const
{ 
  if(this->IsDefined())
    return this->points.Size();
  return 0; 
} 

void IntSet::CopyFrom(const Attribute* rhs)
{
  const IntSet* arg= dynamic_cast<const IntSet* >(rhs);
  this->Clear();
  if(!arg->IsDefined())
  {
    this->SetDefined(false);
    return;
  }
  this->SetDefined(true);
  for (int a = 0; a < arg->Count(); ++a)
  {
    this->Insert( (*arg)[a]);
  }
}

int IntSet::Compare( const Attribute* rhs ) const
{
  return Attribute::GenericCompare< IntSet >( this, 
      dynamic_cast<IntSet* >(const_cast<Attribute*>(rhs)), 
      this->IsDefined(), rhs->IsDefined() );
}

ostream& IntSet::Print( ostream &os ) const
{
  if (! this->IsDefined()) return (os << "UnDefined IntSet");
  
  os << "\nIntSet size:" + this->Count();
  os << "\nIntSet members: " ;
  for (int pointIt=0 ;pointIt < this->Count(); ++pointIt)
  {
    os<<(*this)[pointIt] <<", ";
  }
  os << "\n";
  return os;
}

size_t IntSet::Sizeof() const 
{ 
  return sizeof(IntSet); 
}

bool IntSet::Adjacent(const Attribute*) const 
{
  return false;
}

Attribute* IntSet::Clone() const
{
  return new IntSet(*this);
}

/*
members required for SECONDO types

*/
Word     IntSet::In( const ListExpr typeInfo, const ListExpr instance,
        int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(nl->IsEqual(instance,"undef"))
      return SetWord(Address( 0 ));
  ListExpr elem, inst(instance);
  int count=0;
  Word elemWord;
  
  IntSet* set= new IntSet(nl->ListLength(inst));
  
  while(! nl->IsEmpty(inst))
  {
    elem= nl->First(inst);
    inst= nl->Rest(inst);
    if(!nl->IsAtom(elem))
    {
      correct= false;
      errorInfo= nl->StringAtom("Non-integer element in list");
      errorPos= count;
      delete set;
      return SetWord(Address(0));
    }
    count++;
    set->Insert(nl->IntValue(elem));
  }
  correct= true;
  return SetWord(set);
}

ListExpr IntSet::Out( ListExpr typeInfo, Word value )
{
  IntSet* set = static_cast<IntSet*>(value.addr);
  if(!set->IsDefined())
    return nl->SymbolAtom("undef");
  
  ListExpr lastElem, elems;
  elems= lastElem= nl->TheEmptyList();
  if(set->Count()>0)
  { 
    elems = lastElem = nl->OneElemList(nl->IntAtom((*set)[0]));
    for(int i=1; i< set->Count(); ++i)
      lastElem = nl->Append(lastElem, nl->IntAtom((*set)[i]));
  }
  return elems;
}

Word     IntSet::Create( const ListExpr typeInfo )
{
  return (SetWord (new IntSet(0)));
}

void     IntSet::Delete( const ListExpr typeInfo, Word& w )
{
  static_cast<IntSet*>(w.addr)->points.Destroy();
  delete static_cast<IntSet*>(w.addr);
  w.addr= 0;
}

void     IntSet::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<IntSet*>(w.addr);
  w.addr= 0;
}

Word     IntSet::Clone( const ListExpr typeInfo, const Word& w )
{
  IntSet* arg= static_cast<IntSet*>(w.addr);
  IntSet* res= static_cast<IntSet*>(arg->Clone());
  return SetWord(res);
}

void*    IntSet::Cast(void* addr)
{
  return (new (addr) IntSet); 
}

bool     IntSet::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "intset"));
}

int      IntSet::SizeOfObj()
{ 
  return sizeof(IntSet); 
}

ListExpr IntSet::Property()
{
  return (nl->TwoElemList(
             nl->FourElemList(nl->StringAtom("Signature"),
                              nl->StringAtom("Example Type List"),
                              nl->StringAtom("List Rep"),
                              nl->StringAtom("Example List")),
             nl->FourElemList(nl->StringAtom("-> intset"),
                              nl->StringAtom("(int int int int)"),
                              nl->StringAtom("(elem1 elem2 elem3)"),
                              nl->TextAtom("(4 1 7 3)"))));
}

TypeConstructor intSetTC(
        "intset",       //name
        IntSet::Property, //property function describing signature
        IntSet::Out,
        IntSet::In,     //Out and In functions
        0, 0,          //SaveToList and RestoreFromList functions
        IntSet::Create,
        IntSet::Delete, //object creation and deletion
        0, 0,
        IntSet::Close,
        IntSet::Clone,  //object close and clone
        IntSet::Cast,   //cast function
        IntSet::SizeOfObj, //sizeof function
        IntSet::KindCheck );  


ListExpr
USetProperty()
{
  return (nl->TwoElemList(
      nl->FourElemList(nl->StringAtom("Signature"),
          nl->StringAtom("Example Type List"),
          nl->StringAtom("List Rep"),
          nl->StringAtom("Example List")),
          nl->FourElemList(nl->StringAtom("-> uset"),
              nl->StringAtom("(uset) "),
              nl->StringAtom("(timeInterval intset) "),
              nl->StringAtom("((i1 i2 FALSE FALSE) (4 7 1 2))"))));
}

/*
4.7.3 Kind Checking Function

*/
bool
CheckUSet( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "uset" ));
}

/*
4.7.4 Creation of the type constructor ~ubool~

*/
TypeConstructor usetTC(
        "uset",        //name
        USetProperty,    //property function describing signature
        OutConstTemporalUnit<IntSet, IntSet::Out>,
        InConstTemporalUnit<IntSet, IntSet::In>,    //Out and In functions
        0,     0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<IntSet>,
        DeleteConstTemporalUnit<IntSet>,          //object creation and deletion
        OpenAttribute<USet>,
        SaveAttribute<USet>,  // object open and save
        CloseConstTemporalUnit<IntSet>,
        CloneConstTemporalUnit<IntSet>,           //object close and clone
        CastConstTemporalUnit<IntSet>,            //cast function
        SizeOfConstTemporalUnit<IntSet>,          //sizeof function
        CheckUSet );  

void MSet::Add( const USet& unit )
{
  if ( !unit.IsDefined() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
      << " Add(Unit): Unit is undefined or invalid:";
    unit.Print(cout); cout << endl;
    assert( false );
  }
  int start= data.Size();
  int end= start + unit.constValue.Count();
  USetRef unitref(start, end, unit.timeInterval, unit.IsDefined());
  for(int i=0; i< unit.constValue.Count(); ++i)
    this->data.Append( unit.constValue[i] );
  this->units.Append(unitref);
}


int MSet::NumOfFLOBs()const {return 2;}
Flob *MSet::GetFLOB(const int i)
{
  if(i==0)
    return &units;
  if(i==1)
    return &data;
  assert(0);
}

ostream& MSet::Print( ostream &os ) const
{
  if( !IsDefined() )
  {
    return os << "(MSet: undefined)";
  }
  os << "(MSet: defined, contains " << GetNoComponents() << " units: ";
  for(int i=0; i<GetNoComponents(); i++)
  {
    USetRef unit;
    Get( i , unit );
    os << "\n\t"; 
    unit.Print(data, os);
  }
  os << "\n)" << endl;
  return os;
}

inline MSet* MSet::Clone() const
{
  MSet *result;

  if( !IsDefined() ){
    result = new MSet( 0 );
    result->SetDefined( false );
    return result;
  }
  result = new MSet( GetNoComponents() );
  result->SetDefined( true );

  assert( IsOrdered() );

  if(GetNoComponents()>0){
     result->units.resize(GetNoComponents());
     result->data.resize(GetNoComponents());
  }

  result->StartBulkLoad();
  USetRef unitRef;
  USet unit;
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unitRef );
    unitRef.GetUnit(this->data, unit);
    result->Add( unit );
  }
  result->EndBulkLoad( false );
  return result;
}

/*
mosh 3aref

*/
inline void MSet::CopyFrom( const Attribute* right )
{
  const MSet *r = (MSet*)right;
  Clear();
  SetDefined( r->IsDefined() );
  if( !r->IsDefined() ){
    return;
  }

  assert( r->IsOrdered() );
  StartBulkLoad();
  USetRef unitRef;
  USet unit;
  for( int i = 0; i < r->GetNoComponents(); i++ )
  {
    r->Get( i, unitRef );
    unitRef.GetUnit(r->data, unit);
    Add( unit );
  }
  EndBulkLoad( false );
  this->SetDefined(r->IsDefined());
}

//int MSet::NumOfFLOBs() const {  return 3;}
//Flob* MSet::GetFLOB(const int i)
//{
//  if(dirty)
//  {
//    Finalize();
//    dirty=false;
//  }
//  if(i==0)
//    return &this->offset;
//  else if(i==1)
//    return &this->interval;
//  else if(i==2)
//    return &this->elems;
//  else
//    assert(0);
//  return 0;
//}

bool     MSet::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "mset"));
}

ListExpr MSet::Property()
{
  return (nl->TwoElemList(
             nl->FourElemList(nl->StringAtom("Signature"),
                              nl->StringAtom("Example Type List"),
                              nl->StringAtom("List Rep"),
                              nl->StringAtom("Example List")),
             nl->FourElemList(nl->StringAtom("-> mset"),
                              nl->StringAtom("((uset) (uset) (uset))"),
                              nl->StringAtom("((uset1) (uset2) (uset3))"),
                              nl->TextAtom("(_ _ _ _)"))));
}

ListExpr MSet::OutMSet(ListExpr typeInfo, Word value) 
{
  bool debugme=false;
    MSet* ms = static_cast<MSet*>( value.addr );

    // check for undefined value
    if( !ms->IsDefined() ) return (nl->SymbolAtom("undef"));

    if (ms->IsEmpty()) return (nl->TheEmptyList());

    ListExpr l = nl->TheEmptyList();
    ListExpr lastElem = l; // CD: lastElem was uninitialized 

    USetRef unitRef;
    USet unit(true);
    for (int i = 0; i < ms->GetNoComponents(); i++) 
    {
        
        ms->Get(i, unitRef);
        unitRef.GetUnit(ms->data, unit);
        ListExpr unitList = 
          OutConstTemporalUnit<IntSet, IntSet::Out>(nl->TheEmptyList(), 
              SetWord(&unit));
          
        if (l == nl->TheEmptyList()) {
            l = nl->Cons(unitList, nl->TheEmptyList());
            lastElem = l;
        } else
            lastElem = nl->Append(lastElem, unitList);
    }
    if(debugme)
      cerr<< endl<<nl->ToString(l)<<endl;
    return l;
}

Word MSet::InMSet(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct) {
  bool debugme=false;
  if(debugme)
    cerr<<endl<<nl->ToString(instance)<<endl;
  MSet* ms = new MSet(0);

    if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
         && nl->SymbolValue( instance ) == "undef" )
    {
      ms->SetDefined(false);
      correct = true;
      return SetWord ( ms );
    }

    ms->StartBulkLoad();
    Word unit;
    ListExpr rest = instance;
    while(!nl->IsEmpty(rest)) 
    {
        ListExpr first = nl->First(rest);
        rest = nl->Rest(rest);
        unit = InConstTemporalUnit<IntSet, IntSet::In>(nl->TheEmptyList(),
            first,
            errorPos,
            errorInfo,
            correct);
        if (!correct) {
            ms->Destroy();
            delete ms;
            return SetWord(Address(0));
        }
        
        ms->Add( (*static_cast<USet*>(unit.addr)));
        delete static_cast<USet*>(unit.addr);
    }

    ms->EndBulkLoad(true);

    if (ms->IsValid()) {
        correct = true;
        return SetWord(ms);
    } else {
        correct = false;
        ms->Destroy();
        delete ms;
        return SetWord(Address(0));
    }
}

Word MSet::CreateMSet( const ListExpr typeInfo )
{
  return (SetWord( new MSet( 0 ) ));
}

void MSet::DeleteMSet( const ListExpr typeInfo, Word& w )
{
  static_cast<MSet *>(w.addr)->data.Destroy();
  static_cast<MSet *>(w.addr)->Destroy();
  delete static_cast<MSet *>(w.addr);
  w.addr = 0;
}

bool MSet::operator ==(MSet& rhs)
{
  return ! (*this != rhs);
}
bool MSet::operator !=(MSet& rhs)
{
  if( !this->IsDefined() || !rhs.IsDefined()) return true;
  if(this->GetNoComponents() != rhs.GetNoComponents()) return true;
  USetRef ref1(true), ref2(true);
  USet unit1(true), unit2(true);
  for(int i=0; i< this->GetNoComponents(); ++i)
  {
    this->Get(i, ref1);
    rhs.Get(i, ref2);
    ref1.GetUnit(this->data, unit1);
    ref2.GetUnit(rhs.data, unit2);
    if(unit1.Compare(&unit2) != 0)
      return true;
  }
  return false;
}


void MSet::CloseMSet( const ListExpr typeInfo, Word& w )
{
  delete static_cast<MSet *>(w.addr);
  w.addr = 0;
}

Word MSet::CloneMSet( const ListExpr typeInfo, const Word& w )
{
  return SetWord( static_cast<MSet *>(w.addr)->Clone());
}

void* MSet::CastMSet(void* addr)
{
  return new (addr) MSet;
}

int MSet::SizeOfMSet()
{
  return sizeof(MSet);
}

void MSet::LiftedUnion(MSet& arg, MSet& res)
{
  bool debugme=false;
  res.Clear();
  if( !this->IsDefined() || !arg.IsDefined()){
    res.SetDefined( false );
    return;
  }
  res.SetDefined( true );
  USet un(true);  //part of the Result
  RefinementPartition<MSet, MSet, USetRef, USetRef>  rp( *this, arg);
  if(debugme)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  res.Resize(rp.Size());
  res.StartBulkLoad();

  Interval<Instant> iv;
  int u1Pos, u2Pos;
  USetRef u1, u2;
  USet op1(true), op2(true), resunit(true);
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1 )
      continue;
    else 
    {
      if(debugme)
        cout<<"Both operands existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u1Pos, u1);
      arg.Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
      
      u1.GetUnit(this->data, op1);
      u2.GetUnit(arg.data, op2);
      
      op1.constValue.Union(op2.constValue, resunit.constValue);
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);     
    }
  }
  res.EndBulkLoad(false);
}

void MSet::LiftedUnion2(MSet& arg, MSet& res)
{
  bool debugme=false;
  res.Clear();
  if( this->IsDefined() && !arg.IsDefined())
  {
    res.CopyFrom(this);
    return;
  }
  else if ( !this->IsDefined() && arg.IsDefined())
  {
    res.CopyFrom(&arg);
    return;
  }
  else if ( !this->IsDefined() && !arg.IsDefined())
  {
    res.SetDefined(false);
    return;
  }
      
  res.SetDefined( true );
  USet un(true);  //part of the Result
  RefinementPartition<MSet, MSet, USetRef, USetRef>  rp( *this, arg);
  if(debugme)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  res.Resize(rp.Size());
  res.StartBulkLoad();

  Interval<Instant> iv;
  int u1Pos, u2Pos;
  USetRef u1, u2;
  USet op1(true), op2(true), resunit(true);
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 && u2Pos == -1 )
      continue;
    else if (u1Pos != -1 && u2Pos == -1 )
    {
      if(debugme)
        cout<<"Only operand 1 existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u1Pos, u1);
      if(!u1.IsDefined())
        continue;

      u1.GetUnit(this->data, op1);      
      resunit.constValue.CopyFrom(&op1.constValue) ;
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);     
    }
    else if (u1Pos == -1 && u2Pos != -1 )
    {
      if(debugme)
        cout<<"Only operand 2 existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u2Pos, u2);
      if(!u2.IsDefined())
        continue;

      u2.GetUnit(arg.data, op2);      
      resunit.constValue.CopyFrom(&op2.constValue) ;
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);           
    }
    else 
    {
      if(debugme)
        cout<<"Both operands existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u1Pos, u1);
      arg.Get(u2Pos, u2);
        continue;
      
      u1.GetUnit(this->data, op1);
      u2.GetUnit(arg.data, op2);
      
      op1.constValue.Union(op2.constValue, resunit.constValue);
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);     
    }
  }
  res.EndBulkLoad(false);
}

void MSet::LiftedUnion(MSet& arg)
{
  bool debugme=false;
  MSet res(0);
  if( !this->IsDefined() || !arg.IsDefined()){
    res.SetDefined( false );
    return;
  }
  res.SetDefined( true );
  USet un(true);  //part of the Result
  RefinementPartition<MSet, MSet, USetRef, USetRef>  rp( *this, arg);
  if(debugme)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  res.Resize(rp.Size());
  res.StartBulkLoad();

  Interval<Instant> iv;
  int u1Pos, u2Pos;
  USetRef u1, u2;
  USet op1(true), op2(true), resunit(true);
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 || u2Pos == -1 )
      continue;
    else 
    {
      if(debugme)
        cout<<"Both operands existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u1Pos, u1);
      arg.Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
      
      u1.GetUnit(this->data, op1);
      u2.GetUnit(arg.data, op2);
      
      op1.constValue.Union(op2.constValue, resunit.constValue);
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);     
    }
  }
  res.EndBulkLoad(false);
  res.Destroy();
  this->Clear();
  this->CopyFrom(&res);
}

void MSet::LiftedUnion2(MSet& arg)
{
  bool debugme=false;
  MSet res(0);
  if( this->IsDefined() && !arg.IsDefined())
  {
    res.CopyFrom(this);
    return;
  }
  else if ( !this->IsDefined() && arg.IsDefined())
  {
    res.CopyFrom(&arg);
    return;
  }
  else if ( !this->IsDefined() && !arg.IsDefined())
  {
    res.SetDefined(false);
    return;
  }
      
  res.SetDefined( true );
  USet un(true);  //part of the Result
  RefinementPartition<MSet, MSet, USetRef, USetRef>  rp( *this, arg);
  if(debugme)
    cout<<"Refinement finished, rp.size: "<<rp.Size()<<endl;

  res.Resize(rp.Size());
  res.StartBulkLoad();

  Interval<Instant> iv;
  int u1Pos, u2Pos;
  USetRef u1, u2;
  USet op1(true), op2(true), resunit(true);
  for(unsigned int i = 0; i < rp.Size(); i++)
  {
    rp.Get(i, iv, u1Pos, u2Pos);
    if (u1Pos == -1 && u2Pos == -1 )
      continue;
    else if (u1Pos != -1 && u2Pos == -1 )
    {
      if(debugme)
        cout<<"Only operand 1 existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u1Pos, u1);
      if(!u1.IsDefined())
        continue;

      u1.GetUnit(this->data, op1);      
      resunit.constValue.CopyFrom(&op1.constValue) ;
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);     
    }
    else if (u1Pos == -1 && u2Pos != -1 )
    {
      if(debugme)
        cout<<"Only operand 2 existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u2Pos, u2);
      if(!u2.IsDefined())
        continue;

      u2.GetUnit(arg.data, op2);      
      resunit.constValue.CopyFrom(&op2.constValue) ;
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);           
    }
    else 
    {
      if(debugme)
        cout<<"Both operands existant in interval iv #"<<i<<" ["
        << iv.start.ToString()<< " "<< iv.end.ToString()<< " "<< iv.lc
        << " "<< iv.rc<< "] "<< u1Pos<< " "<< u2Pos<< endl;

      this->Get(u1Pos, u1);
      arg.Get(u2Pos, u2);
      if(!(u1.IsDefined() && u2.IsDefined()))
        continue;
      
      u1.GetUnit(this->data, op1);
      u2.GetUnit(arg.data, op2);
      
      op1.constValue.Union(op2.constValue, resunit.constValue);
      resunit.timeInterval= iv;
      res.MergeAdd(resunit);     
    }
  }
  res.EndBulkLoad(false);
  this->Clear();
  this->CopyFrom(&res);
  res.Destroy();
}

void MSet::LiftedCount(MInt& res)
{
  res.SetDefined(this->IsDefined());
  if(!this->IsDefined()) return;
  res.Clear();
  USetRef unitS;
  UInt unitI(true);
  CcInt count(0);
  for(int i=0; i< this->GetNoComponents(); ++i)
  {
    this->Get(i, unitS);
    unitI.timeInterval = unitS.timeInterval;
    count.Set(unitS.end - unitS.start);
    unitI.constValue= count;
    res.MergeAdd(unitI);
  }
  
}
void MSet::MBool2MSet(MBool& mb, int elem)
{
  if(!mb.IsDefined()) { this->SetDefined(false); return; }
  this->SetDefined(true);
  this->Clear();
  IntSet set(true);
  set.Insert(elem);
  UBool ubool(true);
  CcBool tmp;
  USet uset(true);
  uset.constValue.CopyFrom(&set);
  for(int i= 0; i<mb.GetNoComponents(); ++i)
  {
    mb.Get(i, ubool);
    if(ubool.constValue.GetValue())
    {
      uset.timeInterval= ubool.timeInterval;
      this->Add(uset);
    }
  }
}

void MSet::MergeAdd( const USet& unit )
{
  bool debugme= false;
  assert( IsDefined() );
  USetRef lastunitref;
  USet lastunit(true);
  int size = units.Size();
  if ( !unit.IsDefined() || !unit.IsValid() )
  {
    cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
    << " MergeAdd(Unit): Unit is undefined or invalid:";
    unit.Print(cout); cout << endl;
    assert( false );
  }

  if (size > 0) 
  {
    units.Get( size - 1, lastunitref );
    lastunitref.GetUnit(this->data, lastunit);
    if(debugme)
    {
      unit.Print(cerr);
      lastunit.Print(cerr);
    }
    if (lastunit.EqualValue(unit) &&
        (lastunitref.timeInterval.end == unit.timeInterval.start) &&
        (lastunitref.timeInterval.rc || unit.timeInterval.lc)) 
    {
      lastunitref.timeInterval.end = unit.timeInterval.end;
      lastunitref.timeInterval.rc = unit.timeInterval.rc;
      if ( !lastunitref.IsValid() )
      {
        cout << __FILE__ << "," << __LINE__ << ":" << __PRETTY_FUNCTION__
        << "\nMapping::MergeAdd(): lastunit is invalid:";
        lastunit.Print(cout); cout << endl;
        assert( false );
      }
      units.Put(size - 1, lastunitref);
    }
    else 
      this->Add( unit );

  }
  else 
    this->Add( unit );
}

void MSet::Clear()
{
  this->units.clean();
  this->data.clean();
}

void MSet::AtPeriods( const Periods& periods, MSet& result ) const
{
  result.Clear();
  if( !IsDefined() || !periods.IsDefined() ) {
    result.SetDefined( false );
    return;
  }
  assert( IsOrdered() );
  assert( periods.IsOrdered() );

  if( IsEmpty() || periods.IsEmpty() )
    return;

  result.StartBulkLoad();

  USetRef unitRef;
  USet unit(true);
  Interval<Instant> interval;
  int i = 0, j = 0;
  Get( i, unitRef );
  periods.Get( j, interval );

  while( 1 )
  {
    if( unitRef.timeInterval.Before( interval ) )
    {
      if( ++i == GetNoComponents() )
        break;
      Get( i, unitRef );
    }
    else if( interval.Before( unitRef.timeInterval ) )
    {
      if( ++j == periods.GetNoComponents() )
        break;
      periods.Get( j, interval );
    }
    else
    {
      USet r(true);
      unitRef.GetUnit(this->data, unit);
      unit.AtInterval( interval, r );
      result.Add( r );

      if( interval.end == unit.timeInterval.end )
      {
        if( interval.rc == unit.timeInterval.rc )
        {
          if( ++i == GetNoComponents() )
            break;
          Get( i, unitRef );
          if( ++j == periods.GetNoComponents() )
            break;
          periods.Get( j, interval );
        }
        else if( interval.rc == true )
        {
          if( ++i == GetNoComponents() )
            break;
          Get( i, unitRef );
        }
        else
        {
          assert( unit.timeInterval.rc == true );
          if( ++j == periods.GetNoComponents() )
            break;
          periods.Get( j, interval );
        }
      }
      else if( interval.end > unit.timeInterval.end )
      {
        if( ++i == GetNoComponents() )
          break;
        Get( i, unitRef );
      }
      else
      {
        assert( interval.end < unit.timeInterval.end );
        if( ++j == periods.GetNoComponents() )
          break;
        periods.Get( j, interval );
      }
    }
  }
  result.EndBulkLoad( false );
// VTA - The merge of the result is not implemented yet.
}

TypeConstructor msetTC(
        "mset", //name
        MSet::Property,  //property function describing signature
        MSet::OutMSet,
        MSet::InMSet,
       //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        MSet::CreateMSet,
        MSet::DeleteMSet,        //object creation and deletion
        0,
        0,          // object open and save
        MSet::CloseMSet,
        MSet::CloneMSet,     //object close and clone
        MSet::CastMSet,     //cast function
        MSet::SizeOfMSet,    //sizeof function
        MSet::KindCheck );   



};
#endif 
