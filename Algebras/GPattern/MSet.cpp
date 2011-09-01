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

[1] Header File of the Spatiotemporal Pattern Algebra

Jan, 2010 Mahmoud Sakr

[TOC]

1 Overview


2 Defines and includes

*/

#include "MSet.h"

namespace mset{

bool 
Helpers::string2int(char* digit, int& result) 
{
  result = 0;
  while (*digit >= '0' && *digit <='9') {
    result = (result * 10) + (*digit - '0');
    digit++;
  }
  return (*digit == 0); // true if at end of string!
}

string 
Helpers::ToString( int number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

string 
Helpers::ToString( double number )
{
  ostringstream o;
  o << number ; //<< char(0)
  return o.str();
}

ostream& Helpers::PrintSet( set<int> elems, ostream &os)
{
  set<int>::iterator elemsIt= elems.begin();
  os<<endl<<"Set has "<< elems.size() << " elements {";
  while(elemsIt != elems.end())
  {
    os<< *elemsIt<<", ";
    ++elemsIt;
  }
  os<< '}';
  return os;
}
void USetRef::GetUnit(const DbArray<int>& data, USet& res) const
{
  if (this->isdefined && this->start <= this->end && this->end <= data.Size())
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
    if((*this)[l] < rhs[r]) ++l;
    else if ((*this)[l] == rhs[r]) {++res; ++l; ++r;}
    else ++r;
  }
  return res;
}

bool IntSet::Intersects(const IntSet& rhs) const
{
  if(!(this->IsDefined() && rhs.IsDefined()))
    return false;  
  int l=0, r=0;
  int first1= (*this)[0], first2= rhs[0],
      last1= (*this)[this->Count()-1], last2= rhs[rhs.Count()-1];
  if( (last1 < first2) || (last2 < first1) ) return false; 
  while(l < this->Count() && r < rhs.Count())
  {
    if((*this)[l] < rhs[r]) ++l;
    else if ((*this)[l] == rhs[r]) return true;
    else ++r;
  }
  return false;
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
  
  os << "\nIntSet size:" + Helpers::ToString(this->Count());
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
  return (SetWord (new IntSet(false)));
}

void     IntSet::Delete( const ListExpr typeInfo, Word& w )
{
  static_cast<IntSet*>(w.addr)->points.Destroy();
  static_cast<IntSet*>(w.addr)->DeleteIfAllowed();
  w.addr= 0;
}

void     IntSet::Close( const ListExpr typeInfo, Word& w )
{
  static_cast<IntSet *>(w.addr)->DeleteIfAllowed();
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
  return (new (addr) IntSet());
}

bool     IntSet::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual(type, "intset"));
}

int      IntSet::SizeOfObj()
{ 
  return sizeof(IntSet); 
}

int IntSet::NumOfFLOBs()const 
{
  return 1;
}
Flob* IntSet::GetFLOB(const int i)
{
  assert(i==0);
  return &points;
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

const string IntSet::BasicType()
{
  return "intset";
}

InMemUSet::~InMemUSet()
{
  constValue.clear();
}

void InMemUSet::ReadFrom(USet& arg)
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

void InMemUSet::WriteToUSet(USet& res)
{
  res.constValue.Clear();
  res.timeInterval.start.ReadFrom(starttime);
  res.timeInterval.start.SetType(instanttype);
  res.timeInterval.end.ReadFrom(endtime);
  res.timeInterval.end.SetType(instanttype);
  res.timeInterval.lc= lc;
  res.timeInterval.rc= rc;
  for(it=constValue.begin(); it != constValue.end(); ++it)
    res.constValue.Insert(*it);
  res.SetDefined(true);
  res.constValue.SetDefined(true);
  assert(res.IsValid());
}

void InMemUSet::Clear()
{
  constValue.clear();
}

void InMemUSet::ReadFrom(UBool& arg, int key)
{
  constValue.clear();
  constValue.insert(key);
  SetTimeInterval(arg.timeInterval);
}

void InMemUSet::SetTimeInterval(Interval<Instant>& arg)
{
  assert(arg.IsValid());
  starttime= arg.start.millisecondsToNull();
  endtime= arg.end.millisecondsToNull();
  lc= arg.lc;
  rc= arg.rc;
}
void InMemUSet::Intersection(set<int>& arg)
{
  vector<int> res(constValue.size() + arg.size());   
  vector<int>::iterator res_end;
  res_end=set_intersection(constValue.begin(), constValue.end(), 
      arg.begin(), arg.end(), res.begin());
  constValue.clear();
  for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
    constValue.insert(*rit);    
}

bool InMemUSet::Intersects(set<int>& arg)
{
  if(this->constValue.empty() || arg.empty()) return false;
  set<int>::iterator first1= this->constValue.begin(),
    first2= arg.begin(),
    last1 = this->constValue.end(),
    last2 = arg.end();
    --last1; --last2;
    
  if( (*last1 < *first2) || (*last2 < *first1) ) return false; 
  ++last1; ++last2;
  
  while (first1 != last1 && first2 != last2)
  {
    if (*first1 < *first2)
      ++first1;
    else if (*first2 < *first1)
      ++first2;
    else return true;
  }
  return false;
}

void InMemUSet::Intersection(set<int>& arg, set<int>& result)
{
  vector<int> res(constValue.size() + arg.size());   
  vector<int>::iterator res_end;
  res_end=set_intersection(constValue.begin(), constValue.end(), 
      arg.begin(), arg.end(), res.begin());
  result.clear();
  for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
    result.insert(*rit);    
}
void InMemUSet::Union(set<int>& arg)
{
  vector<int> res(constValue.size() + arg.size());   
  vector<int>::iterator res_end;
  res_end=set_union (constValue.begin(), constValue.end(), 
      arg.begin(), arg.end(), res.begin());
  constValue.clear();
  for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
    constValue.insert(*rit);    
}

void InMemUSet::Union(set<int>& arg, set<int>& result)
{
  vector<int> res(constValue.size() + arg.size());   
  vector<int>::iterator res_end;
  res_end=set_union (constValue.begin(), constValue.end(), 
      arg.begin(), arg.end(), res.begin());
  result.clear();
  for(vector<int>::iterator rit=res.begin(); rit != res_end; ++rit)
    result.insert(*rit);    
}

ostream& InMemUSet::Print( ostream &os )
{
  if( constValue.size() == 0 )
  {
    return os << "(USet: empty)";
  }
  Instant tmp(instanttype);
  tmp.ReadFrom(starttime);
  char c= (lc)? '[' : '(';
  os<< c ; tmp.Print(os);
  tmp.ReadFrom(endtime);
  os<< "\t"; tmp.Print(os);
  c= (rc)? ']' : ')';
  os << c <<  " {";
    for(it=constValue.begin(); it != constValue.end(); ++it)
    {
      os<<(*it);
      os << ",";
    }
//  it= constValue.begin();
//  int b = *(constValue.begin()), e= *(--(constValue.end()));
//  for(int beg= b; beg <= e; ++beg)
//  {
//    if(beg == *it) {os<<(*it)<<"\t"; ++it;}
//    else os<<"e\t";
//    
//  }
  os << "}" << endl;
  return os;
}

unsigned int InMemUSet::Count()
{
  return constValue.size();
}
void InMemUSet::Insert(int elem)
{
  constValue.insert(elem);
}
void InMemUSet::CopyValueFrom(set<int>& arg)
{
  constValue.clear();
  for(it=arg.begin(); it != arg.end(); ++it)
    constValue.insert(*it);
}
void InMemUSet::CopyFrom(InMemUSet& arg)
{
  constValue.clear();
  for(it=arg.constValue.begin(); it != arg.constValue.end(); ++it)
    constValue.insert(*it);
  starttime= arg.starttime;
  endtime= arg.endtime;
  rc= arg.rc;
  lc= arg.lc;
}


InMemMSet::InMemMSet(){}
InMemMSet::InMemMSet(MSet& arg)
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
InMemMSet::InMemMSet(InMemMSet& arg, list<InMemUSet>::iterator begin,
      list<InMemUSet>::iterator end)
{
  CopyFrom(arg, begin, end);  
}

InMemMSet::~InMemMSet()
{
  Clear();
}
void InMemMSet::Clear()
{
  for(it= units.begin(); it != units.end(); ++it)
    (*it).constValue.clear();
  units.clear();
}

int InMemMSet::GetNoComponents()
{
  return units.size();
}

void InMemMSet::CopyFrom(InMemMSet& arg)
{
  Clear();
  for(arg.it= arg.units.begin(); arg.it != arg.units.end(); ++arg.it)
    units.push_back(*arg.it);
}

void InMemMSet::CopyFrom(InMemMSet& arg, list<InMemUSet>::iterator begin,
    list<InMemUSet>::iterator end)
{
  Clear();
  for(arg.it= begin; arg.it != end; ++arg.it)
    units.push_back(*arg.it);
  units.push_back(*end);
}

void InMemMSet::ReadFrom(MBool& mbool, int key)
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

void InMemMSet::WriteToMSet(MSet& res)
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

void InMemMSet::WriteToMSet(MSet& res, list<InMemUSet>::iterator begin, 
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

bool InMemMSet::MergeAdd(
    set<int>& val, int64_t &starttime, int64_t &endtime, bool lc, bool rc)
{
  bool merged= false;
  if(!units.empty())
  {
    InMemUSet* lastElem= &units.back();
    if( (starttime == lastElem->endtime) &&
        (lc || lastElem->rc))
    {
      merged= true;
      bool equals= ((val.size() == lastElem->constValue.size()) &&
          equal(val.begin(), val.end(), lastElem->constValue.begin()));
      if(equals)
      {
        lastElem->endtime= endtime;
        lastElem->rc= rc;
        return merged;
      }
    }
  }
  InMemUSet uset(val, starttime, endtime, lc, rc);
  units.push_back(uset);
  return merged;
}

ostream& InMemMSet::Print( ostream &os ) 
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


void InMemMSet::Union (InMemMSet& arg)
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
  int64_t a= (*it).starttime, A=(*it).endtime;
  int64_t b= (*arg.it).starttime, B=(*arg.it).endtime;
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

bool InMemMSet::RemoveSmallUnits(const unsigned int n)
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

bool InMemMSet::RemoveShortPariods(const int64_t dMS)
{
  bool changed=false;
  if(units.size()==0) return false;
  
  list<InMemUSet>::iterator begin=units.begin(), end= units.begin(), tmp;
  while(begin != units.end())
  {
    end = GetPeriodEndUnit(begin);
    if(((*end).endtime - (*begin).starttime) < dMS)
    {
      tmp= end; ++tmp;
      begin= units.erase(begin, tmp);
      changed= true;
    }
  }
  return changed;
}

ostream& InMemMSet::Print( map<int, inst> elems, ostream &os )
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
bool InMemMSet::RemoveShortElemParts(const int64_t dMS)
{
  bool debugme= false;
  
  //handling special cases
  if(units.size()==0) return false;
  if(units.size()==1)
  {
    if( ( (*units.begin()).endtime - (*units.begin()).starttime) < dMS)
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
    if(((*end).endtime - (*cur).starttime) < dMS)
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
      vector<int> diff(0);
      vector<int> diff2(0);
      vector<int>::iterator diffEnd, diffIt;
      vector<int>::iterator diff2End, diff2It;
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
        
        diff.resize((*prev).constValue.size());
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
          if( ((*prev).endtime - (*elemsIt).second.first) < dMS )
          {
            list<InMemUSet>::iterator unitToChange= (*elemsIt).second.second;
            for(; unitToChange != cur; ++unitToChange)
              (*unitToChange).constValue.erase(elemToRemove);
            changed = true;
          }
          elems.erase(elemsIt);
          ++diffIt;
        }
        
        diff2.resize((*cur).constValue.size());
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
        if( ((*prev).endtime - (*elemsIt).second.first) < dMS )
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

list<InMemUSet>::iterator 
InMemMSet::GetPeriodEndUnit(list<InMemUSet>::iterator begin)
{
  bool debugme= false;
  if(begin == units.end())
    return begin;

  if(debugme)
    (*begin).Print(cerr);
  
  list<InMemUSet>::iterator end=begin;   
  int64_t totalLength= (*begin).endtime - (*begin).starttime, curLength=0;
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

list<InMemUSet>::iterator 
InMemMSet::GetPeriodStartUnit(list<InMemUSet>::iterator end)
{
  if(end == units.begin())
    return end;

  list<InMemUSet>::iterator begin= end;   
  int64_t totalLength= (*end).endtime - (*end).starttime, curLength=0;
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

bool InMemMSet::GetNextTrueUnit(MBool& mbool, int& pos, UBool& unit)
{
  while(pos < mbool.GetNoComponents())
  {
    mbool.Get(pos, unit);
    if(unit.IsDefined() && unit.constValue.GetValue())
      return true;
    else
      ++pos;
  }
  return false;
}

void InMemMSet::Union (MBool& arg, int key)
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
  int64_t a= (*it).starttime, A=(*it).endtime;
  int64_t b= rhs.starttime, B=rhs.endtime;
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

void CompressedInMemUSet::Erase(int victim)
{
  it= added.find(victim);
  if(it != added.end())
    added.erase(it);
  else
    removed.insert(victim);
}
void CompressedInMemUSet::Insert(int elem)
{
  it= removed.find(elem);
  if(it != removed.end())
    removed.erase(it);
  else
    added.insert(elem);
}
ostream& CompressedInMemUSet::Print( ostream &os ) 
{
  char l= (lc)? '[' : '(';
  char r= (rc)? ']' : ')';
  Instant i1(instanttype), i2(instanttype);
  i1.ReadFrom(starttime);
  i2.ReadFrom(endtime);
  os<< endl<< l<< i1<< "\t"<< i2<< r<< "\tCount= "<< count;
  return os;
}
bool CompressedInMemUSet::EraseNodes(
    vector<int>& removedNodes, vector<pair<int,int> >& edge2nodesMap)
{
  pair<int, int> edgeNodes;
  set<int>::iterator it= added.begin();
  bool changed= false;
  while(it != added.end())
  {
    edgeNodes= edge2nodesMap[*it];
    if(find(removedNodes.begin(), removedNodes.end(), edgeNodes.first) !=
        removedNodes.end() ||
       find(removedNodes.begin(), removedNodes.end(), edgeNodes.second) !=
        removedNodes.end())
    {
      added.erase(it++);
      changed= true;
    }
    else
      ++it;
  }
  it= removed.begin();
  while(it != removed.end())
  {
    edgeNodes= edge2nodesMap[*it];
    if(find(removedNodes.begin(), removedNodes.end(), edgeNodes.first) !=
        removedNodes.end() ||
       find(removedNodes.begin(), removedNodes.end(), edgeNodes.second) !=
        removedNodes.end())
    {
      removed.erase(it++);
      changed= true;
    }
    else
      ++it;
  }
  return changed;
}

CompressedInMemMSet::CompressedInMemMSet():validLastUnitValue(false){}
CompressedInMemMSet::CompressedInMemMSet(CompressedInMemMSet& arg, 
    list<CompressedInMemUSet>::iterator begin,
    list<CompressedInMemUSet>::iterator end)
    {
      CopyFrom(arg, begin, end);
    }

int CompressedInMemMSet::GetNoComponents()
{
  return units.size();
}

void CompressedInMemMSet::CopyFrom(CompressedInMemMSet& arg, 
    list<CompressedInMemUSet>::iterator begin,
    list<CompressedInMemUSet>::iterator end)
{
  //copies the range [begin, end) into this
  bool debugme= false;
  Clear();
  if(begin == end) return;
  set<int> accumlator;
  arg.GetSet(begin, accumlator);
  
  if(debugme)
  {
    int tmp= accumlator.size();
    cerr << endl<<tmp;
  }
  
  CompressedInMemUSet first;
  copy(accumlator.begin(), accumlator.end(),  
      inserter(first.added, first.added.begin()));
  first.starttime = (*begin).starttime; first.endtime = (*begin).endtime;
  first.lc = (*begin).lc; first.rc = (*begin).rc;
  units.push_back(first);
  ++begin;  
  while(begin != end)
  {
    units.push_back(*begin);
    ++begin;
  }
  if(end == arg.units.end())
  {
    set<int>* argLast = arg.GetFinalSet();
    copy(argLast->begin(), argLast->end(), 
        inserter(lastUnitValue, lastUnitValue.begin()));
    this->validLastUnitValue= true;
  }
  else
    this->validLastUnitValue= false;
}

void CompressedInMemMSet::Clear()
{
  buffer.clear();
  units.clear();
  lastUnitValue.clear();
  validLastUnitValue= true;
}

void CompressedInMemMSet::GetSet(
    list<CompressedInMemUSet>::iterator index, set<int>& res)
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

void CompressedInMemMSet::WriteToMSet(MSet& res)
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

void CompressedInMemMSet::WriteUSet(
    set<int>& val, CompressedInMemUSet& source, USet& res)
{
  res.constValue.Clear();
  res.timeInterval.start.ReadFrom(source.starttime);
  res.timeInterval.start.SetType(instanttype);
  res.timeInterval.end.ReadFrom(source.endtime);
  res.timeInterval.end.SetType(instanttype);
  res.timeInterval.lc= source.lc;
  res.timeInterval.rc= source.rc;
  for(set<int>::iterator i= val.begin(); i != val.end(); ++i)
    res.constValue.Insert(*i);
  res.SetDefined(true);
  res.constValue.SetDefined(true);
  assert(res.IsValid());
}

void CompressedInMemMSet::WriteToMSet(
    MSet& res, list<CompressedInMemUSet>::iterator begin, 
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

void CompressedInMemMSet::WriteToInMemMSet(InMemMSet& res, 
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

void CompressedInMemMSet::WriteToInMemMSet(InMemMSet& res)
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

ostream& CompressedInMemMSet::Print( ostream &os ) 
{
  if( units.size() == 0 )
  {
    return os << "(InMemMSet: undefined)";
  }
  os << "(InMemMSet: defined, contains " << units.size() << " units: ";
  
  set<int> constValue;
  for(it=units.begin(); it != units.end(); ++it)
  {
    os<<"\n[";
    Instant i(instanttype);
    i.ReadFrom((*it).starttime ); i.Print(os);
    os<<", ";
    i.ReadFrom((*it).endtime ); i.Print(os);
    os<< "]";
    if((*it).added.size() != 0)
      constValue.insert((*it).added.begin(), (*it).added.end());
    if((*it).removed.size() != 0)
    {
      for((*it).it= (*it).removed.begin(); 
        (*it).it != (*it).removed.end(); ++(*it).it)
        constValue.erase(*(*it).it);
    }
    os<< " Set cardinality = "<< constValue.size()<<" {";
    for(set<int>::iterator k= constValue.begin(); k != constValue.end();++k)
      os<< *k<< ", ";
    os<<"}";
  }
  os << "\n)" << endl;
  return os;
}

list<CompressedInMemUSet>::iterator CompressedInMemMSet::EraseUnit(
    list<CompressedInMemUSet>::iterator pos)
{
  bool debugme= false;
  if(pos == units.end())
    return pos;
  list<CompressedInMemUSet>::iterator next= pos;
  ++next;
  if(next == units.end())
  {
    validLastUnitValue= false;
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

list<CompressedInMemUSet>::iterator CompressedInMemMSet::EraseUnits(
    list<CompressedInMemUSet>::iterator start, 
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

bool CompressedInMemMSet::RemoveSmallUnits(const unsigned int n)
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



ostream& CompressedInMemMSet::Print( map<int, inst> elems, ostream &os )
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

bool CompressedInMemMSet::RemoveShortElemParts(const int64_t dMS)
{
  bool debugme= false;
  
  //handling special cases
  if(units.size()==0) return false;
  if(units.size()==1)
  {
    if( ( (*units.begin()).endtime - (*units.begin()).starttime) < dMS)
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
    if(debugme && 0)
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
    if(((*end).endtime - (*cur).starttime) < dMS)
    {
      ++end;
      cur= EraseUnits(cur, end);
      changed= true;
      if(debugme && 0)
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
        if(debugme && 0)
          cerr<<endl<<*elem;

        inst lt((*cur).starttime, cur);
        elems.insert(pair<int, inst>(*elem, lt));
      }
      if(debugme && 0)
        Print(elems, cerr);
      prev= cur; ++cur;        
      ++end;
      int cnt=0;
      while(cur != end)
      {
        if(debugme)
        {
          cerr<<endl<< cnt++ << " Cur:  ";
          //(*cur).Print(cerr);
        }
        set<int>::iterator diffIt= (*cur).removed.begin(), tmpIt;
        if(debugme)
        {
          cerr<<endl;
          while(diffIt != (*cur).removed.end())
          {
            cerr<<*diffIt<< "\t";
            ++diffIt;
          }
          diffIt= (*cur).removed.begin();
        }
        while(diffIt != (*cur).removed.end())
        {
          int elemToRemove= *diffIt;
          if(debugme)
            cerr<< elemToRemove;
          elemsIt= elems.find(elemToRemove);
          assert(elemsIt != elems.end());
          if( ((*cur).starttime - (*elemsIt).second.first) < dMS )
          {
            list<CompressedInMemUSet>::iterator unitToChange= 
              (*elemsIt).second.second;
            (*unitToChange).Erase(elemToRemove);
            while(unitToChange != cur)
            {
              --((*unitToChange).count);
              ++unitToChange;
            }
            //tmpIt = diffIt; ++tmpIt;
            (*cur).removed.erase(diffIt++);
            //if(! (*cur).removed.empty())
            //  diffIt = --tmpIt;
            //else
            //  diffIt = (*cur).removed.end();
            changed = true;
            elems.erase(elemsIt);
          }
          else
          {
            ++diffIt;
            elems.erase(elemsIt);
          }
        }
        if(debugme)
          Print(elems, cerr);
        
        
        //Add the new elements that starts to appear in the cur unit
        diffIt= (*cur).added.begin();
        if(debugme)
        {
          cerr<<endl;
          while(diffIt != (*cur).added.end())
          {
            cerr<<*diffIt<< "\t";
            ++diffIt;
          }
          diffIt= (*cur).added.begin();
        }
        while(diffIt != (*cur).added.end())          
        {
          if(debugme)
            cerr<<endl<<*diffIt;
          inst lt((*cur).starttime, cur);
          elems.insert(pair<int, inst>(*diffIt, lt));
          ++diffIt;
        }
        prev= cur; ++cur;
        if(debugme && 0)
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
        if( ((*prev).endtime - (*elemsIt).second.first) < dMS )
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
      if(debugme )
      {
        Print(elems, cerr);
        MSet tmp(0);
        WriteToMSet(tmp);
        tmp.Print(cerr);
      }
    }
    cur=end;
  }
  if(changed)
    validLastUnitValue= false;
  return changed;
}

list<CompressedInMemUSet>::iterator CompressedInMemMSet::GetPeriodEndUnit(
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
  int64_t totalLength= (*begin).endtime - (*begin).starttime, curLength=0;
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

bool CompressedInMemMSet::GetNextTrueUnit(MBool& mbool, int& pos, UBool& unit)
{
  while(pos < mbool.GetNoComponents())
  {
    mbool.Get(pos, unit);
    if(unit.IsDefined() && unit.constValue.GetValue())
      return true;
    else
      ++pos;
  }
  return false;
}

bool CompressedInMemMSet::Buffer (MBool& arg, int key)
{
  if(arg.GetNoComponents()==0)
    return false; 

  bool trueUnitFound= false;
  UBool ubool;
  int cur=0;
  int64_t starttime, endtime;
  bool lc, rc;
  while(GetNextTrueUnit(arg, cur, ubool))
  {
    trueUnitFound= true;
    assert(ubool.IsValid());
    starttime= ubool.timeInterval.start.millisecondsToNull();
    endtime= ubool.timeInterval.end.millisecondsToNull();
    lc= ubool.timeInterval.lc;
    rc= ubool.timeInterval.rc;
    Event entrance(key,  (lc)? closedstart: openstart);
    Event theleave(key, (rc)? closedend: openend);
    buffer.insert(pair<int64_t, Event>(starttime, entrance));
    buffer.insert(pair<int64_t, Event>(endtime, theleave));
    ++cur;
  }
  return trueUnitFound;
}

bool CompressedInMemMSet::Buffer (MBool& arg, int key, int64_t dMS)
{
  if(arg.GetNoComponents()==0)
    return false; 

  bool longUnitFound= false;
  UBool ubool;
  int cur=0;
  int64_t starttime, endtime;
  bool lc, rc;
  while(GetNextTrueUnit(arg, cur, ubool))
  {
    assert(ubool.IsValid());
    starttime= ubool.timeInterval.start.millisecondsToNull();
    endtime= ubool.timeInterval.end.millisecondsToNull();
    ++cur;
    if(endtime - starttime < dMS) continue;
    lc= ubool.timeInterval.lc;
    rc= ubool.timeInterval.rc;
    Event entrance(key,  (lc)? closedstart: openstart);
    Event theleave(key, (rc)? closedend: openend);
    buffer.insert(pair<int64_t, Event>(starttime, entrance));
    buffer.insert(pair<int64_t, Event>(endtime, theleave));
    longUnitFound= true;
  }
  return longUnitFound;
}

void CompressedInMemMSet::ClassifyEvents(
    pair< multimap<int64_t, Event>::iterator,
    multimap<int64_t, Event>::iterator >& events,
    map<EventType, vector<multimap<int64_t, Event>::iterator> >& eventClasses)
{
  eventClasses[openstart].clear();  eventClasses[closedstart].clear();
  eventClasses[openend].clear();  eventClasses[closedend].clear();
  if(events.first == events.second) return;
  for(multimap<int64_t, Event>::iterator k= events.first;
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

void CompressedInMemMSet::AddUnit(
    int64_t starttime, int64_t endtime, bool lc, bool rc,
    set<int>& elemsToAdd, set<int>& elemsToRemove, int elemsCount)
{
  bool debugme= false;
  if(elemsToAdd.empty() && elemsToRemove.empty()) return;
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
    i1.ReadFrom(unit.starttime);
    i2.ReadFrom(unit.endtime);
    cerr<<"\nAdding unit "<< l ; i1.Print(cerr); cerr<<", ";
    i2.Print(cerr); cerr<<r<< "  count="<< unit.count;
  }
  if(validLastUnitValue)
  {
    lastUnitValue.insert(unit.added.begin(), unit.added.end()); 
    for(set<int>::iterator elem= unit.removed.begin(); 
    elem != unit.removed.end(); ++elem)
      lastUnitValue.erase(*elem);
  }
}

void CompressedInMemMSet::AddUnit( set<int>& constValue,
    int64_t starttime, int64_t endtime, bool lc, bool rc)
{
  bool debugme= false;
  set<int>* finalSet= GetFinalSet();
  CompressedInMemUSet unit;
  if(units.empty())
  {
    copy(constValue.begin(), constValue.end(), 
        inserter(unit.added, unit.added.begin()));
    unit.starttime= starttime; unit.endtime= endtime; unit.lc= lc; unit.rc= rc;
    unit.count= constValue.size();
    units.push_back(unit);
    validLastUnitValue= true;
    lastUnitValue.insert(constValue.begin(), constValue.end()); 
    return;
  }
  
  set_difference(constValue.begin(), constValue.end(),
      finalSet->begin(), finalSet->end(),
      inserter(unit.added, unit.added.begin()));
  set_difference(finalSet->begin(), finalSet->end(),
      constValue.begin(), constValue.end(),
      inserter(unit.removed, unit.removed.begin()));
  unit.starttime= starttime; unit.endtime= endtime;
  unit.lc= lc; unit.rc= rc; 
  unit.count= constValue.size();
  units.push_back(unit);
  if(debugme)
  {
    char l= (unit.lc)? '[' : '(', r = (unit.rc)? ']' : ')';
    Instant i1(instanttype),i2(instanttype);
    i1.ReadFrom(unit.starttime);
    i2.ReadFrom(unit.endtime);
    cerr<<"\nAdding unit "<< l ; i1.Print(cerr); cerr<<", ";
    i2.Print(cerr); cerr<<r<< "  count="<< unit.count;
  }
  if(validLastUnitValue)
  {
    lastUnitValue.insert(unit.added.begin(), unit.added.end()); 
    for(set<int>::iterator elem= unit.removed.begin(); 
    elem != unit.removed.end(); ++elem)
      lastUnitValue.erase(*elem);
  }
  if(debugme)
    assert( (lastUnitValue.size() == constValue.size())  &&
      (equal(lastUnitValue.begin(), lastUnitValue.end(), constValue.begin())));
}

bool CompressedInMemMSet::MergeAdd(set<int>& val, int64_t &starttime,
    int64_t &endtime, bool lc, bool rc)
{
  bool merged= false;
  if(!units.empty())
  {
    set<int>* finalSet= GetFinalSet();
    CompressedInMemUSet* lastUSet= &units.back();
    if( (starttime == lastUSet->endtime) &&
        (lc || lastUSet->rc))
    {
      merged= true;
      bool equals= ((val.size() == finalSet->size()) &&
          equal(val.begin(), val.end(), finalSet->begin()));
      if(equals)
      {
        lastUSet->endtime= endtime;
        lastUSet->rc= rc;
        return merged;
      }
    }
  }
  AddUnit(val, starttime, endtime, lc, rc);
  return merged;
}

void CompressedInMemMSet::ConstructFromBuffer()
{
  units.clear();
  if(this->buffer.begin() == this->buffer.end()) return;
  
  multimap<int64_t, Event>::iterator cur=this->buffer.begin();
  pair< multimap<int64_t, Event>::iterator,
    multimap<int64_t, Event>::iterator > events;
  map<EventType, vector<multimap<int64_t, Event>::iterator> > eventClasses;
  vector<multimap<int64_t, Event>::iterator >::iterator i;
  int64_t starttime; bool lc;
  int64_t curtime;
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
  this->buffer.erase(events.first, events.second);
  cur= this->buffer.begin();
  while(cur != this->buffer.end())
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
    this->buffer.erase(events.first, events.second);
    cur= this->buffer.begin();
  }
  validLastUnitValue= false;
}

void CompressedInMemMSet::MakeMinimal()
{
/*
A pre-condition for this function is that the InMemMSet has no temporal gaps

*/
  if(this->units.empty()) return;
  list<CompressedInMemUSet>::iterator prevUnitIt= this->units.begin(),
      curUnitIt= this->units.begin();
  ++curUnitIt;
  while(curUnitIt != this->units.end())
  {
    if( (*prevUnitIt).endtime !=  (*curUnitIt).starttime)
    {
      bool MSet_Has_No_Temporal_Gaps= false;
      assert(MSet_Has_No_Temporal_Gaps);
    }
    else if((*curUnitIt).added.empty() && (*curUnitIt).removed.empty())
    {
      (*prevUnitIt).endtime= (*curUnitIt).endtime;
      (*prevUnitIt).rc= (*curUnitIt).rc;
      this->units.erase(curUnitIt++);
    }
    else
    {
      ++prevUnitIt;
      assertIf(prevUnitIt == curUnitIt);
      ++curUnitIt;
    }
  }
}

set<int>* CompressedInMemMSet::GetFinalSet()
{
  if(this->units.empty())
  {
    validLastUnitValue= true;
    lastUnitValue.clear();
  }
  if(validLastUnitValue)
    return &lastUnitValue;
  else
  {
    list<CompressedInMemUSet>:: iterator it= this->units.end();
    --it;
    this->GetSet(it, lastUnitValue);
    validLastUnitValue= true;
    return &lastUnitValue;
  }
  return 0;
}

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

MSet* MSet::Clone() const
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
  USet unit(false);
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
  USet unit(false);
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

const string MSet::BasicType()
{
  return "mset";
}

USet::USet() {}

USet::USet(bool is_defined):
  StandardTemporalUnit<IntSet>(is_defined),constValue(false)
{ }

USet::USet( const Interval<Instant>& _interval, const IntSet& a ):
  StandardTemporalUnit<IntSet>( _interval ),constValue(a)
{
  this->del.isDefined = true;
}

// the following constructor is for implementation compatibility with
// UnitTypes for continious value range types (like UReal, UPoint)
USet::USet( const Interval<Instant>& _interval, const IntSet& a,
                                                       const IntSet& b ):
  StandardTemporalUnit<IntSet>( _interval ), constValue(a)
{
  assert(a == b);
  this->del.isDefined = true;
}

USet::USet( const USet& u ):
  StandardTemporalUnit<IntSet>( u.timeInterval ), constValue( u.constValue ) 
{
  this->del.isDefined = u.del.isDefined;
  
}

USet&
USet::operator=( const USet& i )
{
  this->del.isDefined = i.del.isDefined;
  if( !i.IsDefined() ){
    return *this;
  }
  *((TemporalUnit<IntSet>*)this) = *((TemporalUnit<IntSet>*)&i);
  constValue.CopyFrom( &i.constValue );
  return *this;
}

bool USet::operator==( const USet& i ) const
{
  if( !this->IsDefined() && !i.IsDefined() ){
    return true;
  }
  return (this->IsDefined()) && (i.IsDefined())
      && *((TemporalUnit<IntSet>*)this) == *((TemporalUnit<IntSet>*)&i)
      && constValue.Compare( &i.constValue ) == 0;
}
/*
Returns ~true~ if this temporal unit is equal to the temporal unit ~i~ 
and ~false~ if they are different.

*/

bool USet::operator!=( const USet& i ) const
{
  return !( *this == i );
}
/*
Returns ~true~ if this temporal unit is different to the temporal 
unit ~i~ and ~false~ if they are equal.

*/

/*
3.6.2 The Temporal Functions

~TemporalFunction~ returns an undefined result if the ConstUnit or the Instant
is undefined, or the Instant is not within the unit's timeInterval.

*/
void USet::TemporalFunction( const Instant& t,
                               IntSet& result,
                               bool ignoreLimits = false ) const
{
  if ( !this->IsDefined() ||
       !t.IsDefined() ||
       (!this->timeInterval.Contains( t ) && !ignoreLimits))
    {
      result.SetDefined( false );
    }
  else
    {
      result.CopyFrom( &constValue );
      result.SetDefined( true );
    }
}

bool USet::Passes( const IntSet& val ) const
{
  if( this->IsDefined() && (constValue.Compare( &val ) == 0) )
    return true;
  return false;
}

bool USet::At( const IntSet& val, TemporalUnit<IntSet>& result ) const
{
  if( this->IsDefined() && (constValue.Compare( &val ) == 0) )
  {
    ((USet*)&result)->CopyFrom( this );
    return true;
  }
  ((USet*)&result)->SetDefined( false );
  return false;
}

void USet::AtInterval( const Interval<Instant>& i,
                         TemporalUnit<IntSet>& result ) const
{
  if( !this->IsDefined() || !this->timeInterval.Intersects( i ) ){
    ((USet*)&result)->SetDefined( false );
  } else {
    TemporalUnit<IntSet>::AtInterval( i, result );
    ((USet*)&result)->constValue.CopyFrom( &constValue );
  }
}

bool USet::EqualValue( const USet& i ) const
{
  return this->IsDefined() && (constValue.Compare( &i.constValue ) == 0);
}
/*
Returns ~true~ if the value of this temporal unit is defined and equal to the
value of the temporal unit ~i~ and ~false~ if they are different.

*/

bool USet::Merge( const USet& i ) {
  if(!this->IsDefined() && !i.IsDefined()) { // mergeable, but nothing to do
    return true;
  } else if(!this->IsDefined() || !i.IsDefined()) { // not mergable
    return false;
  } else if(    !this->timeInterval.Adjacent(i.timeInterval)
             && !this->timeInterval.Intersects(i.timeInterval) ){
    return false; // have a gap in between --> not mergeable
  } else if(!this->EqualValue(i)) { // temporal functions are NOT equal
    return false;
  }
  // merge the units (i.e. their timeIntervals)
  USet res(false);
  if(StartsBefore(i)){
    res.timeInterval.start = this->timeInterval.start;
    res.timeInterval.lc    = this->timeInterval.lc;
  } else {
    res.timeInterval.start = i.timeInterval.start;
    res.timeInterval.lc    = i.timeInterval.lc;
  }
  if(EndsAfter(i)){
    res.timeInterval.end   = this->timeInterval.end;
    res.timeInterval.rc    = this->timeInterval.rc;
  } else {
    res.timeInterval.end   = i.timeInterval.end;
    res.timeInterval.rc    = i.timeInterval.rc;
  }
  res.constValue = this->constValue;
  if(res.IsDefined() && res.IsValid()){ // invalid result -- do nothing!
    *this = res;
    return true;
  } else {
    return false;
  }
}
/*
Merges unit ~i~ into this unit if possible and return ~true~. Otherwise do
not modify this unit and return ~false~.

*/

/*
3.6.3 Functions to be part of relations

*/

size_t USet::Sizeof() const
{
  return sizeof( *this );
}

int USet::Compare( const Attribute* arg ) const
{
  USet*  ctu = (USet*)arg;
  // SPM: this pointer added since my windows gcc (v3.4.2) reports:
  // 'timeInterval' undeclared (first use this function) which
  // seems to be a compiler bug!
  if (this->IsDefined() && !ctu->IsDefined())
    return 0;
  if (!this->IsDefined())
    return -1;
  if (!ctu->IsDefined())
    return 1;

  int cmp = this->timeInterval.CompareTo(ctu->timeInterval);
  if(cmp){
     return cmp;
  }
  return constValue.Compare(&(ctu->constValue));
}

bool USet::Adjacent( const Attribute* arg ) const
{
  return false;
}

ostream& USet::Print( ostream &os ) const
{
  if( this->IsDefined() )
    {
      os << "ConstUnit: ( ";
      TemporalUnit<IntSet>::timeInterval.Print(os);
      os << ", ";
      constValue.Print(os);
      os << " ) " << endl;
      return os;
    }
  else
    return os << "ConstUnit: (undef) ";
}

size_t USet::HashValue() const
{
  if(!this->IsDefined()){
    return 0;
  }
  return static_cast<size_t>(   this->timeInterval.start.HashValue()
                              ^ this->timeInterval.end.HashValue()   ) ;
}

USet* USet::Clone() const
{
  return new USet(*this);
}

void USet::CopyFrom( const Attribute* right )
{
  const USet* i = (const USet*)right;
  this->SetDefined(i->IsDefined());
  this->timeInterval.CopyFrom( i->timeInterval );
  constValue.CopyFrom( &(i->constValue) );
}

ListExpr USet::USetProperty()
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
bool USet::CheckUSet( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "uset" ));
}


/*
5.3 Type Constructor ~constunit~

5.3.1 ~Out~-function

*/
ListExpr USet::OutUSet( ListExpr typeInfo, Word value )
{
  //1.get the address of the object and have a class object
  USet* constunit = (USet*)(value.addr);

  //2.test for undefined value
  if ( !constunit->IsDefined() )
    return (nl->SymbolAtom("undef"));

  //3.get the time interval NL
  ListExpr intervalList = nl->FourElemList(
    OutDateTime( nl->TheEmptyList(), SetWord(&constunit->timeInterval.start) ),
    OutDateTime( nl->TheEmptyList(), SetWord(&constunit->timeInterval.end) ),
    nl->BoolAtom( constunit->timeInterval.lc ),
    nl->BoolAtom( constunit->timeInterval.rc));

  //4. return the final result
  return nl->TwoElemList( intervalList,
                          IntSet::Out(nl->TheEmptyList(),
                                  SetWord( &constunit->constValue ) ) );
}

/*
5.3.2 ~In~-function

*/
Word USet::InUSet( const ListExpr typeInfo,
                          const ListExpr instance,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct             )
{
  string errmsg;

  if( nl->ListLength( instance ) == 2 )
  {
    //1. deal with the time interval
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
              nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      Instant *start =
        (Instant *)InInstant( nl->TheEmptyList(), nl->First( first ),
                              errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InUSet(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end =
        (Instant *)InInstant( nl->TheEmptyList(), nl->Second( first ),
                              errorPos, errorInfo, correct ).addr;
      if( !correct )
      {
        errmsg = "InUSet(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }
      // get closedness parameters
      bool lc = nl->BoolValue( nl->Third( first ) );
      bool rc = nl->BoolValue( nl->Fourth( first ) );

      Interval<Instant> tinterval( *start, *end, lc, rc );

      delete start;
      delete end;

      // check, wether interval is well defined
      correct = tinterval.IsValid();
      if ( !correct )
        {
          errmsg = "InUSet: Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      //2. deal with the alpha value
      IntSet *value = 
        (IntSet *)IntSet::In( nl->TheEmptyList(), nl->Second( instance ),
                                     errorPos, errorInfo, correct ).addr;

      //3. create the class object
      if( correct  )
      {
        USet *constunit =
          new USet( tinterval, *value );

        if( constunit->IsValid() )
        {
          delete value;
          return SetWord( constunit );
        }
        delete constunit;
      }
      delete value;
    }
  }
  else if ( nl->IsAtom( instance ) &&
            nl->AtomType( instance ) == SymbolType &&
            nl->SymbolValue( instance ) == "undef" )
    {
      USet *constunit =  new USet(false);
      constunit->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = true;
      return (SetWord( constunit ));
    }
  errmsg = "USet(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
5.3.3 ~Create~-function

*/
Word USet::CreateUSet( const ListExpr typeInfo )
{
  return (SetWord( new USet(false) ));
}

/*
5.3.4 ~Delete~-function

*/
void USet::DeleteUSet( const ListExpr typeInfo, Word& w )
{
  static_cast<IntSet*>(w.addr)->DeleteIfAllowed();
  w.addr= 0;
}

/*
5.3.5 ~Close~-function

*/
void USet::CloseUSet( const ListExpr typeInfo, Word& w )
{
  static_cast<IntSet*>(w.addr)->DeleteIfAllowed();
  w.addr= 0;
}

/*
5.3.6 ~Clone~-function

*/
Word USet::CloneUSet( const ListExpr typeInfo, const Word& w )
{
  USet *constunit = (USet *)w.addr;
  return SetWord( new USet( *constunit ) );
}

/*
5.3.7 ~Sizeof~-function

*/
int USet::SizeOfUSet()
{
  return sizeof(USet);
}

/*
5.3.8 ~Cast~-function

*/
void* USet::CastUSet(void* addr)
{
  return new (addr) USet();
}

const string USet::BasicType()
{
  return "uset";
}



CompressedUSetRef::CompressedUSetRef(){}
CompressedUSetRef::CompressedUSetRef(bool def):isdefined(def){}


CompressedMSet::CompressedMSet(){}
CompressedMSet::CompressedMSet(int cnt):
    validLastUnitValue(false), removed(0), added(0), units(cnt){}
CompressedMSet::CompressedMSet(CompressedMSet& arg):
    validLastUnitValue(false), removed(0), added(0), units(0)
{
  this->CopyFrom(arg);
}

int CompressedMSet::GetNoComponents()
{
  return this->units.Size();
}

void CompressedMSet::CopyFrom(CompressedMSet& arg)
{
  Clear();
  this->added.copyFrom(arg.added);
  this->removed.copyFrom(arg.removed);
  this->units.copyFrom(arg.units);
}

void CompressedMSet::Clear()
{
  this->units.clean();
  this->added.clean();
  this->removed.clean();
  lastUnitValue.clear();
  validLastUnitValue= false;
}

void CompressedMSet::GetSet(int index, set<int>& res)
{
  CompressedUSetRef unitRef;
  int elem, cnt;
  res.clear();
  for(int i= 0; i<= index; ++i)
  {
    this->units.Get( i, unitRef );
    for(int j=unitRef.addedstart; j<= unitRef.addedend; ++j)
    {
      this->added.Get(j, elem);
      res.insert(elem);
    }

    for(int j=unitRef.removedstart; j<= unitRef.removedend; ++j)
    {
      this->removed.Get(j, elem);
      res.erase(elem);
    }
    cnt= res.size();
    assertIf(cnt == unitRef.count);
  }
}

set<int>* CompressedMSet::GetFinalSet()
{
  if(this->units.Size() == 0)
  {
    validLastUnitValue= true;
    lastUnitValue.clear();
  }
  if(validLastUnitValue)
    return &lastUnitValue;
  else
  {
    int last= this->units.Size() - 1;
    this->GetSet(last, lastUnitValue);
    validLastUnitValue= true;
    return &lastUnitValue;
  }
  return 0;
}

void CompressedMSet::WriteToCompressedInMemMSet(CompressedInMemMSet& res)
{
  res.Clear();
  CompressedInMemUSet uset;
  CompressedUSetRef unitRef;
  int elem;
  set<int> toAdd, toRemove;
  for(int i= 0; i< this->units.Size(); ++i)
  {
    this->units.Get( i, unitRef );
    toAdd.clear(); toRemove.clear();
    for(int j=unitRef.addedstart; j<= unitRef.addedend; ++j)
    {
      this->added.Get(j, elem);
      toAdd.insert(elem);
    }
    for(int j=unitRef.removedstart; j<= unitRef.removedend; ++j)
    {
      this->removed.Get(j, elem);
      toRemove.insert(elem);
    }
    res.AddUnit(unitRef.starttime, unitRef.endtime, 
        unitRef.lc, unitRef.rc, toAdd, toRemove, unitRef.count);
  }
}

void CompressedMSet::ReadFromCompressedInMemMSet(CompressedInMemMSet& arg)
{
  this->Clear();
  for(list<CompressedInMemUSet>::iterator argUnitIt= 
    arg.units.begin(); argUnitIt!= arg.units.end(); ++argUnitIt)
    this->AddUnit((*argUnitIt).added, (*argUnitIt).removed, 
        (*argUnitIt).starttime, (*argUnitIt).endtime, 
        (*argUnitIt).lc, (*argUnitIt).rc);
}

bool CompressedMSet::ReadFromCompressedInMemMSet(CompressedInMemMSet& arg,
              list<CompressedInMemUSet>::iterator b,
              list<CompressedInMemUSet>::iterator e)
{
  this->Clear();
  if(b== e) return false;
  set<int> initialSet;
  arg.GetSet(b, initialSet);
  this->AddUnit(initialSet, (*b).starttime, (*b).endtime, (*b).lc, (*b).rc);
  ++b;
  list<CompressedInMemUSet>::iterator argUnitIt;
  for(argUnitIt= b; argUnitIt!= e && argUnitIt != arg.units.end(); ++argUnitIt)
    this->AddUnit((*argUnitIt).added, (*argUnitIt).removed,
        (*argUnitIt).starttime, (*argUnitIt).endtime,
        (*argUnitIt).lc, (*argUnitIt).rc);
  if(argUnitIt == arg.units.end() && e != arg.units.end())
    return false;
  return true;
}

ostream& CompressedMSet::Print( ostream &os )
{
  CompressedInMemMSet _mset;
  this->WriteToCompressedInMemMSet(_mset);
  return _mset.Print(os);
}

void CompressedMSet::AddUnit( set<int>& constValue,
    int64_t starttime, int64_t endtime, bool lc, bool rc)
{
  bool debugme= false;
  set<int>* finalSet= GetFinalSet();
  CompressedUSetRef unit;
  if(units.Size()== 0)
  {
    this->Clear();
    for(set<int>::iterator it= constValue.begin(); it!=constValue.end(); ++it)
      this->added.Append(*it);
    unit.removedstart=0;      unit.removedend=-1;
    unit.addedstart=0;      unit.addedend=constValue.size() - 1;
    unit.starttime= starttime; unit.endtime= endtime; unit.lc= lc; unit.rc= rc;
    unit.count= constValue.size();
    this->units.Append(unit);
    
    this->validLastUnitValue= true;
    lastUnitValue= constValue;
    return;
  }
  
  set<int> _added, _removed;
  set_difference(constValue.begin(), constValue.end(),
      finalSet->begin(), finalSet->end(),   
      inserter(_added, _added.begin()));
  set_difference(finalSet->begin(), finalSet->end(),
      constValue.begin(), constValue.end(),
      inserter(_removed, _removed.begin()));
  if(debugme)
    cerr<<endl<<constValue.size()<<"\t"<<finalSet->size()<<'\t'<<
    _added.size()<<'\t'<<_removed.size() << '\t' <<
    finalSet->size() + _added.size() - _removed.size()<<endl;
  unit.removedstart= this->removed.Size();      
  unit.addedstart= this->added.Size() ;
  for(set<int>::iterator it= _added.begin(); it!=_added.end(); ++it)
    this->added.Append(*it);
  for(set<int>::iterator it= _removed.begin(); it!=_removed.end(); ++it)
    this->removed.Append(*it);
  unit.removedend= this->removed.Size() -1;      
  unit.addedend= this->added.Size() -1;
  unit.starttime= starttime; unit.endtime= endtime;
  unit.lc= lc; unit.rc= rc; 
  unit.count= constValue.size();
  this->units.Append(unit);

  if(validLastUnitValue)
  {
    //To be changed to lastUnitValue = constValue
    if(debugme)
    {
      lastUnitValue.insert(_added.begin(), _added.end());
      for(set<int>::iterator elem= _removed.begin();
          elem != _removed.end(); ++elem)
        lastUnitValue.erase(*elem);
      assert(lastUnitValue.size() == constValue.size());
    }
    else
      lastUnitValue = constValue;
  }
}

void CompressedMSet::AddUnit( set<int>& added, set<int>& removed,
    int64_t starttime, int64_t endtime, bool lc, bool rc)
{
  bool debugme= false;
  CompressedUSetRef unit;
  if(units.Size()== 0)
  {
    assertIf(removed.empty());
    this->Clear();
    for(set<int>::iterator it= added.begin(); it!=added.end(); ++it)
      this->added.Append(*it);
    unit.removedstart=0;      unit.removedend=-1;
    unit.addedstart=0;      unit.addedend=added.size() - 1;
    unit.starttime= starttime; unit.endtime= endtime; unit.lc= lc; unit.rc= rc;
    unit.count= added.size();
    this->units.Append(unit);
    
    this->validLastUnitValue= true;
    lastUnitValue.insert(added.begin(), added.end()); 
    return;
  }

  unit.removedstart= this->removed.Size();      
  unit.addedstart= this->added.Size() ;
  for(set<int>::iterator it= added.begin(); it!=added.end(); ++it)
    this->added.Append(*it);
  for(set<int>::iterator it= removed.begin(); it!=removed.end(); ++it)
    this->removed.Append(*it);
  unit.removedend= this->removed.Size() -1;      
  unit.addedend= this->added.Size() -1;
  unit.starttime= starttime; unit.endtime= endtime;
  unit.lc= lc; unit.rc= rc; 
  CompressedUSetRef lastUSet;
  this->units.Get(this->units.Size()-1, lastUSet);
  unit.count= lastUSet.count + added.size() - removed.size();
  this->units.Append(unit);

  if(debugme)
  {
    set<int> _set;
    this->GetSet(this->GetNoComponents()-2, _set);
    cerr<<"\nLastSet is: ";
    for(set<int>::iterator it= _set.begin(); it!=_set.end(); ++it)
      cerr<<*it<<", ";
    cerr<<"\nAdding: ";
    for(set<int>::iterator it= added.begin(); it!=added.end(); ++it)
      cerr<<*it<<", ";
    cerr<<"\nRemoving: ";
    for(set<int>::iterator it= removed.begin(); it!=removed.end(); ++it)
      cerr<<*it<<", ";
  }
  if(validLastUnitValue)
  {
    lastUnitValue.insert(added.begin(), added.end()); 
    for(set<int>::iterator elem= removed.begin(); 
      elem != removed.end(); ++elem)
      lastUnitValue.erase(*elem);
  }
}

bool CompressedMSet::MergeAdd(set<int>& val, int64_t &starttime,
    int64_t &endtime, bool lc, bool rc)
{
  bool merged= false;
  if(this->units.Size() != 0)
  {
    set<int>* finalSet= GetFinalSet();
    CompressedUSetRef lastUSet;
    this->units.Get(this->units.Size()-1, lastUSet);
    if( AlmostEqual(starttime, lastUSet.endtime) && 
        (lc || lastUSet.rc))
    {
      merged= true;
      bool equals= ((val.size() == finalSet->size()) &&
          equal(val.begin(), val.end(), finalSet->begin()));
      if(equals)
      {
        lastUSet.endtime= endtime;
        lastUSet.rc= rc;
        this->units.Put(this->units.Size()-1, lastUSet);
        return merged;
      }
    }
  }
  AddUnit(val, starttime, endtime, lc, rc);
  return merged;
}

int64_t CompressedMSet::DurationLength()
{
  if(this->units.Size() == 0) return 0;
  
  CompressedUSetRef firstUSet, lastUSet;
  this->units.Get(0, firstUSet);
  this->units.Get(this->units.Size()-1, lastUSet);
  return lastUSet.endtime - firstUSet.starttime;
}

bool CompressedMSet::Concat(CompressedMSet* arg)
{
  bool debugme= false;
  if(arg->GetNoComponents() == 0) return true;
  CompressedUSetRef uset;

  if(debugme)
  {
    if(this->units.Size() > 0)
    {
      cerr<<"\nConcatenating:";
      this->Print(cerr);
      arg->Print(cerr);
    }
  }

  set<int> firstUnitElems;
  arg->GetSet(0, firstUnitElems);
  arg->units.Get(0, uset);
  assertIf(uset.removedend < uset.removedstart);
  this->AddUnit(firstUnitElems, uset.starttime, uset.endtime, uset.lc, uset.rc);
  if(debugme)
  {
    set<int>* finalSet= this->GetFinalSet();
    assert(finalSet->size() == uset.count);
  }


  int removedShift= this->removed.Size(),
      addedShift= this->added.Size() - uset.count,
      argAddedReadIndex= uset.addedend + 1,
      argAddedElem;


  this->validLastUnitValue= false;
  bool ok= this->removed.Append(arg->removed);
  for(int i= argAddedReadIndex; i<arg->added.Size() && ok; ++i)
  {
    arg->added.Get(i, argAddedElem);
    ok= this->added.Append(argAddedElem);
  }
  if(ok)
  {
    for(int i=1; i<arg->units.Size(); ++i)
    {
      arg->units.Get(i, uset);
      uset.addedstart+= addedShift;
      uset.addedend+= addedShift;
      uset.removedstart+= removedShift;
      uset.removedend+= removedShift;
      this->units.Append(uset);
    }
  }
  if(debugme)
  {
    cerr<<"\nResult is:";
    this->Print(cerr);
  }
  return ok;
}

CompressedMSet* CompressedMSet::GetSubMSet(int offset, int length)
{
  bool debugme= false;
  if(offset < 0 || length < 0 || (offset + length) > this->GetNoComponents())
    return 0;

  CompressedMSet* result= new CompressedMSet(0);
  CompressedUSetRef firstUSet(true), lastUSet(true), curUSet(true);
  set<int> firstSet;
  int addedShift, removedShift;
  this->GetSet(offset,firstSet);
  this->units.Get(offset, firstUSet);
  result->AddUnit(firstSet,
      firstUSet.starttime, firstUSet.endtime, firstUSet.lc, firstUSet.rc);
  addedShift= firstUSet.addedend + 1 - firstSet.size();
  removedShift= firstUSet.removedend + 1;
  for(int i = 1; i < length; ++i)
  {
    this->units.Get(i + offset, curUSet);
    curUSet.addedstart-= addedShift;  curUSet.addedend-= addedShift;
    curUSet.removedstart-= removedShift;  curUSet.removedend-= removedShift;
    result->units.Append(curUSet);
  }
  this->units.Get(offset + length - 1, lastUSet);
  int sourceOffset= firstUSet.addedend + 1,
      numberOfElems= lastUSet.addedend - firstUSet.addedend,
      destOffset= firstSet.size();
  this->added.copyTo(result->added, sourceOffset, numberOfElems, destOffset);
  sourceOffset= firstUSet.removedend + 1;
  numberOfElems= lastUSet.removedend - firstUSet.removedend;
  destOffset= 0;
  this->removed.copyTo(
      result->removed, sourceOffset, numberOfElems, destOffset);

  if(debugme)
  {
    set<int>* s1= result->GetFinalSet();
    set<int> s2;
    this->GetSet(offset + length -1, s2);
    assertIf(equal(s1->begin(), s1->end(), s2.begin()));
  }
  return result;
}

};
