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

[1] Source File of the Spatiotemporal Pattern Algebra

June, 2009 Mahmoud Sakr

[TOC]

1 Overview

This source file essentially contains the necessary implementations for 
evaluating the spatiotemporal pattern predicates (STP). 

2 Defines and includes

*/
#include "STPatternAlgebra.h"
namespace STP{

/*
3 Classes

3.1 Class STVector 


*/
CSP csp;

string StrSimpleConnectors[]= {"aabb"  , "bbaa"  ,  "aa.bb"  ,  "bb.aa"  ,
  "abab"  ,  "baba"  ,  "baab"  ,  "abba"  ,  "a.bab"  ,  "a.bba"  ,
  "baa.b"  ,  "aba.b"  ,  "a.ba.b"  ,  "a.abb"  ,  "a.a.bb"  ,
  "ba.ab"  ,  "bb.a.a"  ,  "bba.a"  ,  "b.baa"  ,  "b.b.aa"  ,
  "ab.ba"  ,  "aa.b.b"  ,  "aab.b"  ,  "a.ab.b"  ,  "a.a.b.b"  ,
  "b.ba.a"  };

inline int STVector::Count(int vec)
{
  int c=0;
  if(vec & aabb)    c++  ;
  if(vec & bbaa)    c++  ;
  if(vec & aa_bb)    c++  ;
  if(vec & bb_aa)    c++  ;
  if(vec & abab)    c++  ;
  if(vec & baba)    c++  ;
  if(vec & baab)    c++  ;
  if(vec & abba)    c++  ;
  if(vec & a_bab)    c++  ;
  if(vec & a_bba)    c++  ;
  if(vec & baa_b)    c++  ;
  if(vec & aba_b)    c++  ;
  if(vec & a_ba_b)  c++  ;
  if(vec & a_abb)    c++  ;
  if(vec & a_a_bb)  c++  ;
  if(vec & ba_ab)    c++  ;
  if(vec & bb_a_a)  c++  ;
  if(vec & bba_a)    c++  ;
  if(vec & b_baa)    c++  ;
  if(vec & b_b_aa)  c++  ;
  if(vec & ab_ba)    c++  ;
  if(vec & aa_b_b)  c++  ;
  if(vec & aab_b)    c++  ;
  if(vec & a_ab_b)  c++  ;
  if(vec & a_a_b_b)  c++  ;
  if(vec & b_ba_a)  c++  ;
  return c;
}

inline int STVector::Str2Simple(string s)
{
  if(s=="aabb") return  1 ;
  if(s=="bbaa") return  2 ;
  if(s=="aa.bb")return  4; if(s=="ab.ab")  return  4;
  if(s=="bb.aa")return  8; if(s=="ba.ba")  return  8;
  if(s=="abab") return  16;
  if(s=="baba") return  32;
  if(s=="baab") return  64;
  if(s=="abba") return  128;
  if(s=="a.bab")return  256; if(s=="b.aab")  return  256  ;
  if(s=="a.bba")return  512; if(s=="b.aba")  return  512  ;
  if(s=="baa.b")return  1024;if(s=="bab.a")  return  1024;
  if(s=="aba.b")return  2048;if(s=="abb.a")  return  2048;
  if(s=="a.ba.b")  return  4096  ; if(s=="a.bb.a")  return  4096;
  if(s=="b.aa.b")  return  4096  ;  if(s=="b.ab.a")  return  4096;
  if(s=="a.abb")  return  8192  ;
  if(s=="a.a.bb")  return  16384  ;  if(s=="a.b.ab")  return  16384;
  if(s=="b.a.ab")  return  16384  ;
  if(s=="ba.ab")  return  32768  ;
  if(s=="bb.a.a")  return  65536  ;  if(s=="ba.b.a")  return  65536;
  if(s=="ba.a.b")  return  65536  ;
  if(s=="bba.a")  return  131072  ;
  if(s=="b.baa")  return  262144  ;
  if(s=="b.b.aa")  return  524288  ;  if(s=="b.a.ba")  return  524288;
  if(s=="a.b.ba")  return  524288  ;
  if(s=="ab.ba")  return  1048576  ;    
  if(s=="aa.b.b")  return  2097152  ;  if(s=="ab.a.b")  return  2097152;
  if(s=="ab.b.a")  return  2097152  ;
  if(s=="aab.b")  return  4194304  ;
  if(s=="a.ab.b")  return  8388608  ;
  if(s=="a.a.b.b")return  16777216; if(s=="a.b.a.b")return  16777216;
  if(s=="a.b.b.a")return  16777216;  if(s=="b.b.a.a")return  16777216;
  if(s=="b.a.b.a")return  16777216;  if(s=="b.a.a.b")return  16777216;
  if(s=="b.ba.a")  return  33554432;
  return -1;
}

inline ListExpr STVector::Vector2List()
{
  ListExpr list;
  ListExpr last;
  int simple=1;
  int i=0;
  bool first=true;
  while(i<26 && first)
  {
    if(v & simple)  
    {
      list= nl->OneElemList(nl->SymbolAtom(StrSimpleConnectors[i]));
      last=list;
      first=false;
    }
    simple*=2;
    i++;
  }
  for(i=i; i<26; i++)
  {
    if(v & simple)
      last= nl->Append(last, nl->SymbolAtom(StrSimpleConnectors[i]));
      simple*=2;
  }
  return list;
}

bool STVector::Add(string simple)
{
  int vec= Str2Simple(simple);
  if(vec==-1) return false;
  v = v|vec;
  return true;
}

bool STVector::Add(int simple)
{
  if(simple <=  b_ba_a && Count(simple)==1) 
  {
    v = v|simple;
    return true;
  }
  return false;
}

bool STVector::ApplyVector(Interval<CcReal>& p1, Interval<CcReal>& p2)
{
  int simple=1;
  bool supported=false;
  for(int i=0; i<26; i++)
  {
    if(v & simple)
    {
      supported = ApplySimple(p1, p2, simple);
      if(supported) return true;
    }
    simple*=2;
  }
  return false;
}

bool STVector::ApplySimple(Interval<CcReal>& p1, Interval<CcReal>& p2, 
    int simple)
{ 
  double  a=p1.start.GetRealval(),   A=p1.end.GetRealval(),
  b=p2.start.GetRealval(),   B=p2.end.GetRealval();
  switch(simple)
  {
  case   aabb:
    return(a<A && a<b && a<B && A<b && A<B && b<B); 
    break;
  case   bbaa:
    return(b<B && b<a && b<A && B<a && B<A && a<A);
    break;
  case   aa_bb:
    return(a<A && a<b && a<B && A==b && A<B && b<B);
    break;
  case   bb_aa:
    return(b<B && b<a && b<A && B==a && B<A && a<A);
    break;
  case   abab:
    return(a<b && a<A && a<B && b<A && b<B && A<B);
    break;
  case   baba:
    return(b<a && b<B && b<A && A<b && a<A && B<A);
    break;
  case   baab:
    return(b<a && b<A && b<B && a<A && a<B && A<B);
    break;
  case   abba:
    return(a<b && a<B && a<A && b<B && b<A && B<A);
    break;
  case   a_bab:
    return(a==b && a<A && a<B && b<A && b<B && A<B);
    break;
  case   a_bba:
    return(a==b && a<B && a<A && b<B && b<A && B<A);
    break;
  case   baa_b:
    return(b<a && b<A && b<B && a<A && a<B && A==B);
    break;
  case   aba_b:
    return(a<b && a<A && a<B && b<A && b<B && A==B);
    break;
  case   a_ba_b:
    return(a==b && a<A && a<B && b<A && b<B && A==B);
    break;
  case   a_abb:
    return(a==A && a<b && a<B && A<b && A<B && b<B); 
    break;
  case   a_a_bb:
    return(a==A && a==b && a<B && A==b && A<B && b<B); 
    break;
  case   ba_ab:
    return(b<a && b<A && b<B && a==A && a<B && A<B);
    break;
  case   bb_a_a:
    return(b<B && b<a && b<A && B==a && B==A && a==A);
    break;
  case   bba_a:
    return(b<B && b<a && b<A && B<a && B<A && a==A);
    break;
  case   b_baa:
    return(b==B && b<a && b<A && B<a && B<A && a<A);
    break;
  case   b_b_aa:
    return(b==B && b==a && b<A && B==a && B<A && a<A);
    break;
  case   ab_ba:
    return(a<b && a<B && a<A && b==B && b<A && B<A);
    break;
  case   aa_b_b:
    return(a<A && a<b && a<B && A==b && A==B && b==B); 
    break;
  case   aab_b:
    return(a<A && a<b && a<B && A<b && A<B && b==B); 
    break;
  case   a_ab_b:
    return(a==A && a<b && a<B && A<b && A<B && b==B); 
    break;
  case   a_a_b_b:
    return(a==A && a==b && a==B && A==b && A==B && b==B); 
    break;
  case   b_ba_a:
    return(b==B && b<a && b<A && B<a && B<A && a==A);
    break;
  default:
    assert(0); //illegal simple temporal connector
  }
}

Word STVector::In( const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));
  const string errMsg = "STVector::In: Expecting a simple temporal"
    " connector!";
  STVector* res=new STVector(0);
  NList list(instance), first;
  while(! list.isEmpty())
  {
    first= list.first();
    if(! res->Add(first.str()))
    {
      correct=false;
      cmsg.inFunError(errMsg);
      return result;
    }
    list.rest();
  }
  result.addr= res;
  return result;
}

ListExpr STVector::Out( ListExpr typeInfo, Word value )
{
  STVector* vec = static_cast<STVector*>( value.addr );
  return vec->Vector2List();
}

Word STVector::Create( const ListExpr typeInfo )
{
  return (SetWord( new STVector( 0)));
}

void STVector::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<STVector*>( w.addr );
  w.addr = 0;
}

bool STVector::Open( SmiRecord& valueRecord, size_t& offset, 
    const ListExpr typeInfo, Word& value ) 
{
  //cerr << "OPEN XRectangle" << endl;  
  size_t size = sizeof(int);   
  int vec;

  bool ok = true;
  ok = ok && valueRecord.Read( &vec, size, offset );
  offset += size;  
  value.addr = new STVector(vec); 
  return ok;
}

bool STVector::Save( SmiRecord& valueRecord, size_t& offset, 
    const ListExpr typeInfo, Word& value )
{  
  STVector* vec = static_cast<STVector*>( value.addr );  
  size_t size = sizeof(int);   

  bool ok = true;
  ok = ok && valueRecord.Write( &vec->v, size, offset );
  offset += size;  
  return ok;
} 

void STVector::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<STVector*>( w.addr );
  w.addr = 0;
}

Word STVector::Clone( const ListExpr typeInfo, const Word& w )
{
  STVector* vec = static_cast<STVector*>( w.addr );
  return SetWord( new STVector(*vec) );
}

int STVector::SizeOfObj()
{
  return sizeof(STVector);
}

ListExpr STVector::Property()
{
  return (nl->TwoElemList(
      nl->FiveElemList(nl->StringAtom("Signature"),
          nl->StringAtom("Example Type List"),
          nl->StringAtom("List Rep"),
          nl->StringAtom("Example List"),
          nl->StringAtom("Remarks")),
          nl->FiveElemList(nl->StringAtom("-> SIMPLE"),
              nl->StringAtom("stvector"),
              nl->StringAtom("(s1 s2 ...)"),
              nl->StringAtom("(1 128 32)"),
              nl->StringAtom(""))));
}

bool STVector::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "stvector" ));
}

void IntervalInstant2IntervalCcReal(const Interval<Instant>& in, 
    Interval<CcReal>& out)
{
  out.start.Set(in.start.IsDefined(), in.start.ToDouble());
  out.end.Set(in.end.IsDefined(), in.end.ToDouble());
}



void CSP::IntervalInstant2IntervalCcReal(const Interval<Instant>& in, 
    Interval<CcReal>& out)
{
  out.start.Set(in.start.IsDefined(), in.start.ToDouble());
  out.end.Set(in.end.IsDefined(), in.end.ToDouble());
  out.lc= in.lc;
  out.rc= in.rc;
}

void CSP::IntervalCcReal2IntervalInstant(const Interval<CcReal>& in,
    Interval<Instant>& out)
{
  out.start.ReadFrom(in.start.GetRealval());
  out.end.ReadFrom(in.end.GetRealval());
  out.lc= in.lc;
  out.rc= in.rc;
}

/*
3.2 Class CSP 


*/

/*
The MBool2Vec function is called first thing within the extend function. It
converts the time intervals for the true units from Interval<Instant> into
Interval<CcReal>. This is done for the sake of performance since the Instant 
comparisons are more expensive than the Real comparisons. 

*/
int CSP::MBool2Vec(const MBool* mb, vector<Interval<CcReal> >& vec)
{
  UBool unit(0);
  vec.clear();
  Interval<CcReal> elem(CcReal(true,0.0),CcReal(true,0.0),true,true);
  if(!mb->IsDefined() || mb->IsEmpty() || !mb->IsValid()) return 0;
  for(int i=0; i<mb->GetNoComponents(); i++)
  {
    mb->Get(i, unit);
    if( ((CcBool)unit.constValue).GetValue())
    {
      IntervalInstant2IntervalCcReal(unit.timeInterval, elem);
      vec.push_back(elem);
    }
  }
  return 0;
}

/*
The Extend function distinguishes between two cases:

 * If the variable given is the first variable to be consumed, 
    the SA is intialized.

 * Else, the function creates and checks (on the fly) the Cartesean product
    of SA and the domain of the variable.     

*/
int CSP::Extend(int index, vector<Interval<CcReal> >& domain )
{
  vector<Interval<CcReal> > sa(count);

  if(SA.size() == 0)
  {
    for(int i=0; i<count; i++)
      sa[i].CopyFrom(nullInterval);
    for(unsigned int i=0; i<domain.size(); i++)
    {

      sa[index]= domain[i];
      SA.push_back(sa);
    }
    return 0;
  }

  unsigned int SASize= SA.size();
  for(unsigned int i=0; i<SASize; i++)
  {
    sa= SA[0];
    for(unsigned int j=0; j< domain.size(); j++)
    {
      sa[index]= domain[j];
      if(IsSupported(sa,index))
        SA.push_back(sa);
    }
    SA.erase(SA.begin());
  }
  return 0;
}

/*
The function IsSupported searches the ConstraintGraph for the constraints that 
involve the given variable and check their fulfillment. It is modified to check
only the constraints related to the newly evaluated variable instead of 
re-checking all the constraints. 

*/
bool CSP::IsSupported(vector<Interval<CcReal> > sa, int index)
{

  bool supported=false; 
  for(unsigned int i=0; i<assignedVars.size()-1; i++)
  {
    for(unsigned int j=0; j<assignedVars.size(); j++)
    {
      if(i== (unsigned int)index || j == (unsigned int)index )
      {
        if(ConstraintGraph[assignedVars[i]][assignedVars[j]].size() != 0)
        {
          supported= CheckConstraint(sa[assignedVars[i]], 
              sa[assignedVars[j]], 
              ConstraintGraph[assignedVars[i]][assignedVars[j]]);
          if(!supported) return false;
        }

        if(ConstraintGraph[assignedVars[j]][assignedVars[i]].size() != 0)
        {
          supported= CheckConstraint(sa[assignedVars[j]], 
              sa[assignedVars[i]], 
              ConstraintGraph[assignedVars[j]][assignedVars[i]]);
          if(!supported) return false;
        }
      }
    }
  }
  return supported;
}

bool CSP::CheckConstraint(Interval<CcReal>& p1, Interval<CcReal>& p2, 
    vector<Supplier> constraint)
{
  bool debugme=false;
  Word Value;
  bool satisfied=false;
  for(unsigned int i=0;i< constraint.size(); i++)
  {
    if(debugme)
    {
      cout<< "\nChecking constraint "<< qp->GetType(constraint[i])<<endl;
      cout.flush();
    }
    qp->Request(constraint[i],Value);
    STVector* vec= (STVector*) Value.addr;
    satisfied= vec->ApplyVector(p1, p2);
    if(!satisfied) return false;
  }
  return true; 
}
/*
The function PickVariable picks Agenda variables according to their connectivity
rank. 
For every variable v
{
  For every constraint c(v,x)
  {
   if x in Agenda rank+=0.5
   if x is not in the Agenda rank+=1
  }
}

*/
int CSP::PickVariable()
{
  bool debugme=false;
  vector<int> vars(0);
  vector<double> numconstraints(0);
  double cnt=0;
  int index=-1;
  for(unsigned int i=0;i<Agenda.size();i++)
  {
    if(!UsedAgendaVars[i])
    {
      vars.push_back(i);
      cnt=0;
      for(unsigned int r=0; r< ConstraintGraph.size(); r++)
      {
        for(unsigned int c=0; c< ConstraintGraph[r].size(); c++)
        {
          if( r == i && ConstraintGraph[r][c].size() != 0)
          {
            cnt+= ConstraintGraph[r][c].size() * 0.5;
            for(unsigned int v=0; v< assignedVars.size(); v++)
            {
              if(c == (unsigned int)assignedVars[v]) 
                cnt+=0.5 * ConstraintGraph[r][c].size();
            }  


          }

          if( c == i && ConstraintGraph[r][c].size() != 0)
          {
            cnt+=0.5 * ConstraintGraph[r][c].size();
            for(unsigned int v=0; v< assignedVars.size(); v++)
            {
              if(r == (unsigned int)assignedVars[v]) 
                cnt+=0.5 * ConstraintGraph[r][c].size();
            }
          }
        }
      }
      numconstraints.push_back(cnt);
    }
  }
  double max=-1;

  for(unsigned int i=0; i<numconstraints.size(); i++)
  {
    if(numconstraints[i]>max){ max=numconstraints[i]; index=vars[i];}
  }
  if(debugme)
  {
    for(unsigned int i=0; i<numconstraints.size(); i++)
      cout<< "\nConnectivity of variable " <<vars[i] <<"  = "
      << numconstraints[i];
    cout<<endl<< "Picking variable "<< index<<endl;
    cout.flush();
  }
  if(index== -1) return -1; 
  assignedVars.push_back(index);
  return index;
}

int CSP::AddVariable(string alias, Supplier handle)
{
  Agenda.push_back(handle);
  UsedAgendaVars.push_back(false);
  VarAliasMap[alias]=count;
  count++;
  ConstraintGraph.resize(count);
  for(int i=0; i<count; i++)
    ConstraintGraph[i].resize(count);
  return 0;
}

int CSP::AddConstraint(string alias1, string alias2, Supplier handle)
{
  int index1=-1, index2=-1;
  try{
    index1= VarAliasMap[alias1];
    index2= VarAliasMap[alias2];
    if(index1==index2)
      throw;
  }
  catch(...)
  {
    return -1;
  }
  ConstraintGraph[index1][index2].push_back(handle);
  return 0;
}

bool CSP::Solve()
{
  bool debugme=false;
  int varIndex;
  Word Value;
  vector<Interval<CcReal> > domain(0);
  while( (varIndex= PickVariable()) != -1)
  {
    qp->Request(Agenda[varIndex], Value);
    //Agenda[varIndex]=0;
    UsedAgendaVars[varIndex]= true;
    MBool2Vec((MBool*)Value.addr, domain);
    if(domain.size()==0) {SA.clear(); return false;}
    if(Extend(varIndex, domain)!= 0) return false;
    if(SA.size()==0) return false;
    if(debugme)
      Print();
  }
  return true;
}

bool CSP::MoveNext()
{
  if(iterator < (signed int)SA.size()-1)
    iterator++;
  else
    return false;
  return true;
}

bool CSP::GetSA(unsigned int saIndex, unsigned int varIndex, Periods& res)
{
  res.Clear();
  if(saIndex >= SA.size() || varIndex >= SA[0].size())
    return false;
  Instant t0(instanttype), t1(instanttype);
  Interval<Instant> resInterval( t0, t1, false, false );
  IntervalCcReal2IntervalInstant(SA[saIndex][varIndex], resInterval);
  res.Add(resInterval);
  return true;
}

bool CSP::AppendSolutionToTuple(int saIndex, Tuple* oldTup, Tuple* resTup)
{
  if(!this->SA.empty())
  {
    Periods attrVal(0);
    for (unsigned int i=0; i < this->Agenda.size();i++)
    {
      csp.GetSA(saIndex, i, attrVal);
      resTup->PutAttribute(oldTup->GetNoAttributes()+i, attrVal.Clone());
    }
  }
  else
  {
    Periods undefRes(0);
    undefRes.SetDefined(false);
    for (unsigned int i=0; i < this->Agenda.size();i++)
    {
      resTup->PutAttribute(oldTup->GetNoAttributes()+i, undefRes.Clone());
    }
  }
  return true;
}

bool CSP::AppendUnDefsToTuple(Tuple* oldTup, Tuple* resTup)
{
  Periods undefRes(0);
  undefRes.SetDefined(false);
  for (unsigned int i=0; i < this->Agenda.size();i++)
  {
    resTup->PutAttribute(oldTup->GetNoAttributes()+i, undefRes.Clone());
  }

  return true;
}

bool CSP::GetStart(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.ReadFrom(SA[iterator][index].start.GetRealval());
  return true;
}

bool CSP::GetEnd(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.ReadFrom(SA[iterator][index].end.GetRealval());
  return true;
}

void CSP::Print()
{
  cout<< "\n==========================\nSA.size() = "<< SA.size()<< endl;
  for(unsigned int i=0; i< SA.size(); i++)
  {
    for(unsigned int j=0; j<SA[i].size(); j++)
    {
      cout<<(SA[i][j].start.GetRealval()-3444)*24*60*60<< "\t "<<
      (SA[i][j].end.GetRealval()-3444)*24*60*60 <<" | ";
    }
    cout<<endl;
  }
  cout.flush();
}

int CSP::Clear()
{
  SA.clear();
  Agenda.clear();
  UsedAgendaVars.clear();
  ConstraintGraph.clear();
  VarAliasMap.clear();
  assignedVars.clear();
  count=0;
  iterator=-1;
  return 0;
}

int CSP::ResetTuple()
{
  SA.clear();
  UsedAgendaVars.assign(Agenda.size(), false);
  assignedVars.clear();
  iterator=-1;
  return 0;
}

/*
Auxiliary functions 

*/
void RandomDelay(const MPoint* actual, const Instant* threshold, MPoint& res)
{
  bool debugme= false;

  MPoint delayed(actual->GetNoComponents());
  UPoint first(0), next(0);
  UPoint *shifted,*temp, *cur;
  int rmillisec=0, rday=0;
  actual->Get( 0, first );
  cur=new UPoint(first);
  for( int i = 1; i < actual->GetNoComponents(); i++ )
  {
    actual->Get( i, next );

    rmillisec= rand()% threshold->GetAllMilliSeconds();
    rday=0;
    if(threshold->GetDay()> 0) rday = rand()% threshold->GetDay();
    DateTime delta(rday,rmillisec,durationtype) ;

    shifted= new UPoint(*cur);
    delete cur;
    temp= new UPoint(next);
    if(rmillisec > rand()%24000 )
    {
      if((shifted->timeInterval.end + delta) <  next.timeInterval.end )
      {
        shifted->timeInterval.end += delta ;
        temp->timeInterval.start= shifted->timeInterval.end;
      }
    }
    else
    {
      if((shifted->timeInterval.end - delta) >shifted->timeInterval.start)
      {
        shifted->timeInterval.end -= delta ;
        temp->timeInterval.start= shifted->timeInterval.end;
      }
    }
    cur=temp;
    if(debugme)
    {
      cout.flush();
      cout<<"\n original "; cur->Print(cout);
      cout<<"\n shifted " ; shifted->Print(cout);
      cout.flush();
    }
    delayed.Add(*shifted);
    delete shifted;
  }
  delayed.Add(*temp);
  delete temp;
  res.CopyFrom(&delayed);
  if(debugme)
  {
    res.Print(cout);
    cout.flush();
  }
  return;
}

/*
4 Algebra Types and Operators 


*/


TypeConstructor stvectorTC(
    "stvector",                       // name of the type in SECONDO
    STVector::Property,               // property function describing signature
    STVector::Out, STVector::In,      // Out and In functions
    0, 0,                             // SaveToList, RestoreFromList functions
    STVector::Create, STVector::Delete, // object creation and deletion
    STVector::Open, STVector::Save,   // object open, save
    STVector::Close, STVector::Clone, // close, and clone
    0,                                 // cast function
    STVector::SizeOfObj,              // sizeof function
    STVector::KindCheck );            // kind checking function


ListExpr CreateSTVectorTM(ListExpr args)
{
  //  bool debugme= false;
  string argstr;
  ListExpr rest= args, first;
  while (!nl->IsEmpty(rest)) 
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);
    nl->WriteToString(argstr, first);
    CHECK_COND (nl->IsAtom(first)&&  nl->SymbolValue(first)=="string",
        "Operator v: expects a list of strings but got '" +
        argstr + "'.\n" );
  }
  return nl->SymbolAtom("stvector");
}

ListExpr STPatternTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }


  nl->WriteToString(argstr, args);

  //  //checking for the first parameter tuple(x)
  if(nl->ListLength(args) != 3)
  {
    ErrorReporter::ReportError("Operator stpattern: expects 3 arguments\n"
        "But got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  };

  ListExpr tupleExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args);    //STConstraint list

  nl->WriteToString(argstr, tupleExpr);

  //  //checking for the first parameter tuple(x)
  if(!((nl->ListLength(tupleExpr) == 2) &&
      (TypeOfRelAlgSymbol(nl->First(tupleExpr)) == tuple)))
  {
    ErrorReporter::ReportError("Operator stpattern: expects as first "
        "argument a list with structure (tuple ((a1 t1)...(an tn))).\n"
        "But got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  };

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  if(!(! nl->IsAtom(NamedPredList)))
  {
    ErrorReporter::ReportError("Operator stpattern expects as second "
        "argument a list of aliased lifted predicates.\n"
        "But got '" + argstr + "'.\n");
    return nl->SymbolAtom("typeerror");
  };
  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    if(!((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        nl->IsAtom(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Second(NamedPred))== "mbool")))
    {
      ErrorReporter::ReportError("Operator stpattern: expects a list of "
          "aliased predicates. \nBut got '" + argstr + "'.");
      return nl->SymbolAtom("typeerror");
    };
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    if(!((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool")))
    {
      ErrorReporter::ReportError("Operator stpattern: expects a list of "
          "temporal connectors. \nBut got '" + argstr + "'.");
      return nl->SymbolAtom("typeerror");
    };
  }

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr STPatternExTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  if(nl->ListLength(args) != 4)
  {
    ErrorReporter::ReportError("Operator stpatternex: expects 4 arguments\n"
        "But got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  }

  ListExpr tupleExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args),    //STConstraint list
  FilterExpr= nl->Fourth(args);

  nl->WriteToString(argstr, tupleExpr);

  //  //checking for the first parameter tuple(x)
  if(!((nl->ListLength(tupleExpr) == 2) &&
        (TypeOfRelAlgSymbol(nl->First(tupleExpr)) == tuple)))
  {
    ErrorReporter::ReportError("Operator stpatternex: expects as first "
      "argument a list with structure (tuple ((a1 t1)...(an tn))).\n"
      "But got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  }

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  if(nl->IsAtom(NamedPredList))
  {
    ErrorReporter::ReportError("Operator  stpatternex expects as second "
        "argument a list of aliased lifted predicates.\n"
        "But got '" + argstr + "'.\n");
    return nl->SymbolAtom("typeerror");
  }

  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    if(!((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        nl->IsAtom(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Second(NamedPred))== "mbool")))
    {
      ErrorReporter::ReportError("Operator stpatternex: expects a list of "
          "aliased predicates. But got '" + argstr + "'.");
      return nl->SymbolAtom("typeerror");
    }
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    if(!((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool")))
    {
      ErrorReporter::ReportError("Operator stpatternex: expects a list of "
          "temporal constraints. But got '" + argstr + "'.");
      return nl->SymbolAtom("typeerror");
    }
  }

  if(!((nl->IsAtom(FilterExpr)&&
      nl->SymbolValue(FilterExpr)== "bool")))
  {
    ErrorReporter::ReportError("Operator stpatternex: expects a bool as last "
        "argument, but got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  };

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr STPatternExtendTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  if(nl->ListLength(args) != 3)
  {
    ErrorReporter::ReportError("Operator stpatternextend: expects 4 "
        "arguments.\nBut got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  };

  ListExpr StreamExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args);    //STConstraint list

  nl->WriteToString(argstr, StreamExpr);

  //  //checking for the first parameter tuple(x)
  if(!(nl->ListLength(StreamExpr)== 2 && listutils::isTupleStream(StreamExpr)))
  {
    ErrorReporter::ReportError("Operator stpatternextend: expects as first "
        "argument a list with structure (stream(tuple ((a1 t1)...(an tn)))).\n"
        "But got '" + argstr + "'.");
    return nl->TypeError();
  };

  ListExpr TupleExpr = nl->Second(StreamExpr);   //tuple(x)
  ListExpr AttrList = nl->Second(TupleExpr);
  ListExpr NewAttrList = nl->OneElemList(nl->First(AttrList));
  ListExpr lastlistn = NewAttrList;
  AttrList = nl->Rest(AttrList);
  while (!(nl->IsEmpty(AttrList)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(AttrList));
     AttrList = nl->Rest(AttrList);
  }

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  if(!(! nl->IsAtom(NamedPredList)))
  {
    ErrorReporter::ReportError("Operator stpatternextend expects as second "
        "argument a list of aliased lifted predicates.\n"
        "But got '" + argstr + "'.\n");
    return nl->TypeError();
  };
  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    if(!((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        listutils::isMap<1>(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Third(nl->Second(NamedPred)))== "mbool")))
    {
      ErrorReporter::ReportError("Operator stpatternextend: expects a list of "
          "aliased predicates. \nBut got '" + argstr + "'.");
      return nl->TypeError();
    };

    string aliasStr = nl->SymbolValue(nl->First(NamedPred));
    ListExpr typeList;
    int pos = FindAttribute(AttrList, aliasStr, typeList);
    if(pos!=0){
       ErrorReporter::ReportError("Operator stpatternextend: the alias" +
           aliasStr + " is already an attribute name in the input stream");
       return nl->TypeError();
    }

    if(SecondoSystem::GetCatalog()->IsTypeName(aliasStr)){
       ErrorReporter::ReportError("Operator stpatternextend: the alias" +
           aliasStr + " is known as a DB type");
       return nl->TypeError();
    }
    lastlistn = nl->Append(lastlistn,
        (nl->TwoElemList(nl->First(NamedPred), nl->SymbolAtom("periods"))));
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    if(!((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool")))
    {
      ErrorReporter::ReportError("Operator stpatternextend: expects a list of "
          "temporal connectors. \nBut got '" + argstr + "'.");
      return nl->TypeError();
    };
  }



  ListExpr result= nl->TwoElemList(nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),NewAttrList));

  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpatternextend accepted the input";
    cout<<endl<< nl->ToString(result);
    cout.flush();
  }
  return result;
}

ListExpr STPatternExExtendTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  if(nl->ListLength(args) != 4)
  {
    ErrorReporter::ReportError("Operator stpatternexextend: expects 4 "
        "arguments.\nBut got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  };

  ListExpr StreamExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args),    //STConstraint list
  FilterExpr= nl->Fourth(args);

  nl->WriteToString(argstr, StreamExpr);

  //  //checking for the first parameter tuple(x)
  if(!(nl->ListLength(StreamExpr)== 2 && listutils::isTupleStream(StreamExpr)))
  {
    ErrorReporter::ReportError("Operator stpatternexextend: expects as first "
        "argument a list with structure (stream(tuple ((a1 t1)...(an tn)))).\n"
        "But got '" + argstr + "'.");
    return nl->TypeError();
  };

  ListExpr TupleExpr = nl->Second(StreamExpr);   //tuple(x)
  ListExpr AttrList = nl->Second(TupleExpr);
  ListExpr NewAttrList = nl->OneElemList(nl->First(AttrList));
  ListExpr lastlistn = NewAttrList;
  AttrList = nl->Rest(AttrList);
  while (!(nl->IsEmpty(AttrList)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(AttrList));
     AttrList = nl->Rest(AttrList);
  }

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  if(!(! nl->IsAtom(NamedPredList)))
  {
    ErrorReporter::ReportError("Operator stpatternexextend expects as second "
        "argument a list of aliased lifted predicates.\n"
        "But got '" + argstr + "'.\n");
    return nl->TypeError();
  };
  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    if(!((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        listutils::isMap<1>(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Third(nl->Second(NamedPred)))== "mbool")))
    {
      ErrorReporter::ReportError("Operator stpatternexextend: expects a "
          "list of aliased predicates. \nBut got '" + argstr + "'.");
      return nl->TypeError();
    };

    string aliasStr = nl->SymbolValue(nl->First(NamedPred));
    ListExpr typeList;
    int pos = FindAttribute(AttrList, aliasStr, typeList);
    if(pos!=0){
       ErrorReporter::ReportError("Operator stpatternexextend: the alias" +
           aliasStr + " is already an attribute name in the input stream");
       return nl->TypeError();
    }

    if(SecondoSystem::GetCatalog()->IsTypeName(aliasStr)){
       ErrorReporter::ReportError("Operator stpatternexextend: the alias" +
           aliasStr + " is known as a DB type");
       return nl->TypeError();
    }
    lastlistn = nl->Append(lastlistn,
        (nl->TwoElemList(nl->First(NamedPred), nl->SymbolAtom("periods"))));
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    if(!((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool")))
    {
      ErrorReporter::ReportError("Operator stpatternexextend: expects a "
          "list of temporal connectors. \nBut got '" + argstr + "'.");
      return nl->TypeError();
    };
  }

  nl->WriteToString(argstr, FilterExpr);
  if(! listutils::isMap<1>(FilterExpr) ||
      nl->ToString(nl->Third(FilterExpr))!= "bool")
  {
    ErrorReporter::ReportError("Operator stpatternexextend: expects a "
        "map(tuple) -> bool as last argument, but got '" + argstr + "'.");
    return nl->SymbolAtom("typeerror");
  };

  ListExpr result= nl->TwoElemList(nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),NewAttrList));

  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpatternexextend accepted the input";
    cout<<endl<< nl->ToString(result);
    cout.flush();
  }
  return result;
}

ListExpr STConstraintTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  if(!nl->HasLength(args,3)){
     return listutils::typeError("3 arguments expected");
  }

  ListExpr alias1 = nl->First(args),   //tuple(x)
  alias2  = nl->Second(args),      //named predicate list
  temporalconnector = nl->Third(args);//STConstraint list

  nl->WriteToString(argstr, alias1);
  CHECK_COND(( nl->IsAtom(alias1)&&
      nl->SymbolValue(alias1)== "string"),
      "Operator stconstraint: expects a predicate label as first "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, alias2);
  CHECK_COND(( nl->IsAtom(alias2)&&
      nl->SymbolValue(alias2)== "string"),
      "Operator stconstraint: expects a predicate label as second "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, temporalconnector);
  CHECK_COND(( nl->IsAtom(temporalconnector)&&
      nl->SymbolValue(temporalconnector)== "stvector"),
      "Operator stconstraint: expects a temporal connector as third "
      "argument.\n But got '" + argstr + "'.");

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stconstraint accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr StartEndTM(ListExpr args)
{
  bool debugme=false;
  string argstr;
  if(debugme)
  {
    cout<<endl<< nl->ToString(args) <<endl;
    cout<< nl->ListLength(args)  << ".."<< nl->IsAtom(nl->First(args))<<
    ".."<< nl->SymbolValue(nl->First(args));
    cout.flush();
  }
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 1 &&
      nl->IsAtom(nl->First(args)) &&
      nl->SymbolValue(nl->First(args))== "string",
      "Operator start/end expects a string symbol "
      "but got." + argstr);
  return nl->SymbolAtom("instant");
}

/*

The randommbool operator is used for experimental evaluation. We use it to 
generate the random mbool values that are used in the first experiment in the
technical report.

*/
ListExpr RandomMBoolTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 1 &&
      nl->IsAtom(nl->First(args)) && 
      (nl->SymbolValue(nl->First(args))== "instant") ,
  "Operator randommbool expects one parameter.");
  return nl->SymbolAtom("mbool");
}

/*

The passmbool operator is used for experimental evaluation. We use it to 
mimic lifted predicates in the first experiment in the technical report.

*/
ListExpr PassMBoolTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 1 &&
      nl->IsAtom(nl->First(args)) && 
      (nl->SymbolValue(nl->First(args))== "mbool") ,
  "Operator passmbool expects one parameter.");
  return nl->SymbolAtom("mbool");
}

/*
The randomdelay operator is used to enrich the examples. It adds random 
time delays to the moving point within a given delay threshold   

*/

ListExpr RandomDelayTM( ListExpr typeList )
{
  CHECK_COND(nl->ListLength(typeList) == 2 &&
      nl->IsAtom(nl->First(typeList)) &&
      (nl->SymbolValue(nl->First(typeList))== "mpoint") &&
      nl->IsAtom(nl->Second(typeList)) &&
      (nl->SymbolValue(nl->Second(typeList))== "duration"),
      "randomdelay operator expects (mpoint duration) but got "
      + nl->ToString(typeList))

      return (nl->SymbolAtom("mpoint"));
}


void CSPAddPredicates(Supplier& namedpredlist)
{
  Supplier namedpred,alias, pred;
  string aliasstr;
  int noofpreds= qp->GetNoSons(namedpredlist);

  for(int i=0; i< noofpreds; i++)
  {
    namedpred= qp->GetSupplierSon(namedpredlist, i);
    alias= qp->GetSupplierSon(namedpred, 0);
    pred = qp->GetSupplierSon(namedpred, 1);
    aliasstr= nl->ToString(qp->GetType(alias));
    csp.AddVariable(aliasstr,pred);
  }
}
void CSPAddContraints(Supplier& constraintlist)
{
  Supplier constraint, alias1, alias2, stvector;
  Word Value;
  string alias1str, alias2str;
  int noofconstraints= qp->GetNoSons(constraintlist);

  for(int i=0; i< noofconstraints; i++)
  {
    constraint = qp->GetSupplierSon(constraintlist, i);
    alias1= qp->GetSupplierSon(constraint, 0);
    alias2= qp->GetSupplierSon(constraint, 1);
    stvector= qp->GetSupplierSon(constraint, 2);

    qp->Request(alias1, Value);
    alias1str= ((CcString*) Value.addr)->GetValue();
    qp->Request(alias2, Value);
    alias2str= ((CcString*) Value.addr)->GetValue();
    csp.AddConstraint(alias1str,alias2str, stvector);
  }

}

bool CSPSetPredsArgs(Supplier predList, Tuple* tup)
{

  ArgVectorPointer funargs;
  Supplier namedpred,alias,pred;
  int noofpreds= qp->GetNoSons(predList);
  for(int i=0; i< noofpreds; i++)
  {
    namedpred= qp->GetSupplierSon(predList, i);
    alias= qp->GetSupplierSon(namedpred, 0);
    pred = qp->GetSupplierSon(namedpred, 1);
    funargs = qp->Argument(pred);
    ((*funargs)[0]).setAddr(tup);
  }
  return true;
}

int CreateSTVectorVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int noconn= qp->GetNoSons(s);
  string simple;
  STVector* res= (STVector*) result.addr;
  for(int i=0;i<noconn; i++)
  {
    simple= ((CcString*)args[i].addr)->GetValue();
    if(! res->Add(simple))
      return 1;
  }
  return 0;
}

int STPatternVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;
  Supplier namedpredlist, constraintlist;
  result = qp->ResultStorage( s );
  namedpredlist = args[1].addr;
  constraintlist= args[2].addr;

  csp.Clear();
  CSPAddPredicates(namedpredlist);
  CSPAddContraints(constraintlist);
  bool hasSolution=csp.Solve();

  ((CcBool*)result.addr)->Set(true,hasSolution);
  if(debugme)
  {
    //cout<< "tuple "<<tupleno++ ;
    if(hasSolution) cout<<" accepted\n"; cout<<" rejected\n";
    csp.Print();
    cout.flush();
  }
  return 0;
}

int STPatternExVM(Word* args, Word& result,int message, Word& local, Supplier s)
{
  bool debugme=false;
  Supplier namedpredlist, constraintlist, filter;
  Word Value;

  if(debugme)
  {
    cout<<" Inside STPatternEXVM\n";
    cout.flush();
  }

  result = qp->ResultStorage( s );
  namedpredlist = args[1].addr;
  constraintlist= args[2].addr;
  filter= args[3].addr;

  csp.Clear();
  CSPAddPredicates(namedpredlist);
  CSPAddContraints(constraintlist);
  bool hasSolution=csp.Solve();

  if(!hasSolution)
  {
    ((CcBool*)result.addr)->Set(true,hasSolution);
    if(debugme)
    {
      //cout<< "tuple "<<tupleno++ ;
      if(hasSolution) cout<<" part1 accepted\t";else cout<<" rejected\n";
      csp.Print();
      cout.flush();
    }
    return 0;
  }

  bool Part2=false;
  while(!Part2 && csp.MoveNext())
  {
    qp->Request(filter, Value);
    Part2= ((CcBool*)Value.addr)->GetValue();
  }
  ((CcBool*)result.addr)->Set(true,Part2);
  if(debugme)
  {
    //cout<< "tuple "<<tupleno++ ;
    if(Part2) cout<<" part2 accepted\n"; else cout<<" part2 rejected\n";
    csp.Print();
    cout.flush();
  }
  return 0;
}

int STPatternExtendVM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;

  Word t, value;
  Tuple* tup;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
    {
      Supplier stream, namedpredlist, constraintlist;

      stream = args[0].addr;
      namedpredlist = args[1].addr;
      constraintlist= args[2].addr;

      qp->Open(stream);
      csp.Clear();

      CSPAddPredicates(namedpredlist);
      CSPAddContraints(constraintlist);

      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;
    }break;

    case REQUEST :
    {
      Supplier stream, namedpredlist;
      resultTupleType = (TupleType *)local.addr;
      stream= args[0].addr;
      qp->Request(stream ,t);
      if (qp->Received(stream))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ ) {
          //cout << (void*) tup << endl;
          newTuple->CopyAttribute( i, tup, i );
        }

        csp.ResetTuple();
        namedpredlist = args[1].addr;
        CSPSetPredsArgs(namedpredlist, tup);
        csp.Solve();
        csp.AppendSolutionToTuple(0, tup, newTuple);

        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;
    }break;
    case CLOSE :
    {
      if(local.addr)
      {
         ((TupleType *)local.addr)->DeleteIfAllowed();
         local.setAddr(0);
      }
      qp->Close(args[0].addr);
      csp.Clear();
    }break;
    default:
      assert( 0);
  }
  return 0;
}

int STPatternExExtendVM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;

  Word t, Value;
  Tuple* tup;
  TupleType *resultTupleType;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
    {
      Supplier stream, namedpredlist, constraintlist;

      stream = args[0].addr;
      namedpredlist = args[1].addr;
      constraintlist= args[2].addr;

      qp->Open(stream);
      csp.Clear();

      CSPAddPredicates(namedpredlist);
      CSPAddContraints(constraintlist);

      resultType = GetTupleResultType( s );
      resultTupleType = new TupleType( nl->Second( resultType ) );
      local.setAddr( resultTupleType );
      return 0;
    }break;

    case REQUEST :
    {
      Supplier stream, namedpredlist, filter;
      resultTupleType = (TupleType *)local.addr;
      stream= args[0].addr;
      qp->Request(stream ,t);
      if (qp->Received(stream))
      {
        tup = (Tuple*)t.addr;
        Tuple *newTuple = new Tuple( resultTupleType );
        for( int i = 0; i < tup->GetNoAttributes(); i++ ) {
          //cout << (void*) tup << endl;
          newTuple->CopyAttribute( i, tup, i );
        }

        csp.ResetTuple();
        namedpredlist = args[1].addr;
        filter= args[3].addr;
        CSPSetPredsArgs(namedpredlist, tup);
        bool hasSolution= csp.Solve();
        if(!hasSolution)
          csp.AppendSolutionToTuple(0, tup, newTuple); //Append undef periods
        else
        {
          bool Part2=false;
          ArgVectorPointer funargs= qp->Argument(filter);
          ((*funargs)[0]).setAddr(tup);
          while(!Part2 && csp.MoveNext())
          {
            qp->Request(filter, Value);
            Part2= ((CcBool*)Value.addr)->GetValue();
          }

          if(Part2)
            csp.AppendSolutionToTuple(csp.iterator, tup, newTuple);
          else
            csp.AppendUnDefsToTuple(tup, newTuple);

          if(debugme)
          {
            if(Part2) cout<<" part2 accepted\n"; else cout<<" part2 rejected\n";
            csp.Print();
            cout.flush();
          }
        }
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      else
        return CANCEL;
    }break;
    case CLOSE :
    {
      if(local.addr)
      {
         ((TupleType *)local.addr)->DeleteIfAllowed();
         local.setAddr(0);
      }
      qp->Close(args[0].addr);
      csp.Clear();
    }break;
    default:
      assert( 0);
  }
  return 0;
}

struct STPExtendStreamInfo
{
  TupleType *resultTupleType;
  Tuple* tup;
};

int STPatternExtendStreamVM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;

  Word t, value;
  STPExtendStreamInfo *localInfo;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
    {
      Supplier stream, namedpredlist, constraintlist;

      stream = args[0].addr;
      namedpredlist = args[1].addr;
      constraintlist= args[2].addr;

      qp->Open(stream);
      csp.Clear();

      CSPAddPredicates(namedpredlist);
      CSPAddContraints(constraintlist);

      localInfo= new STPExtendStreamInfo();
      resultType = GetTupleResultType( s );
      localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );
      localInfo->tup=0;
      local.setAddr( localInfo );
      return 0;
    }break;

    case REQUEST :
    {
      Supplier stream, namedpredlist;
      localInfo= (STPExtendStreamInfo*) local.addr;
      stream= args[0].addr;

      bool hasMoreSol= csp.MoveNext();
      while(csp.SA.empty()|| !hasMoreSol)
      {
        if(localInfo->tup != 0)
        {
          localInfo->tup->DeleteIfAllowed();
          localInfo->tup= 0;
        }
        qp->Request(stream ,t);
        if (qp->Received(stream))
          localInfo->tup = (Tuple*)t.addr;
        else
          return CANCEL;
        csp.ResetTuple();
        namedpredlist = args[1].addr;
        CSPSetPredsArgs(namedpredlist, localInfo->tup);
        if(csp.Solve())
        {
          hasMoreSol= csp.MoveNext();
          if(debugme)
            cerr<< csp.SA.size() << " + ";
        }
      }

      if(debugme && 0)
        cerr<< "\nsa "<<csp.iterator + 1 << "/"<<csp.SA.size();

      Tuple *newTuple = new Tuple( localInfo->resultTupleType );
      for( int i = 0; i < localInfo->tup->GetNoAttributes(); i++ )
        newTuple->CopyAttribute( i, localInfo->tup, i );

      csp.AppendSolutionToTuple(csp.iterator, localInfo->tup, newTuple);
      result.setAddr(newTuple);
      return YIELD;
    }break;

    case CLOSE :
    {
      if(local.addr != 0)
      {
        ((STPExtendStreamInfo*)local.addr)->resultTupleType->DeleteIfAllowed();
        delete (STPExtendStreamInfo*)local.addr;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      csp.Clear();
    }break;
    default:
      assert( 0);
  }
  return 0;
}

int STPatternExExtendStreamVM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;

  Word t, Value;
  STPExtendStreamInfo *localInfo;
  ListExpr resultType;

  switch (message)
  {
    case OPEN :
    {
      Supplier stream, namedpredlist, constraintlist;

      stream = args[0].addr;
      namedpredlist = args[1].addr;
      constraintlist= args[2].addr;

      qp->Open(stream);
      csp.Clear();

      CSPAddPredicates(namedpredlist);
      CSPAddContraints(constraintlist);

      resultType = GetTupleResultType( s );
      localInfo= new STPExtendStreamInfo();
      localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );
      localInfo->tup= 0;
      local.setAddr(localInfo );
      return 0;
    }break;

    case REQUEST :
    {
      Supplier stream, namedpredlist, filter;
      filter= args[3].addr;
      localInfo = (STPExtendStreamInfo *)local.addr;
      stream= args[0].addr;

      while(true)
      {
        bool hasMoreSol= csp.MoveNext();
        while(csp.SA.empty() || !hasMoreSol)
        {
          if(localInfo->tup != 0)
          {
            localInfo->tup->DeleteIfAllowed();
            localInfo->tup= 0;
          }
          qp->Request(stream ,t);
          if (qp->Received(stream))
            localInfo->tup = (Tuple*)t.addr;
          else
            return CANCEL;
          csp.ResetTuple();
          namedpredlist = args[1].addr;
          CSPSetPredsArgs(namedpredlist, localInfo->tup);
          if(csp.Solve())
          {
            hasMoreSol= csp.MoveNext();
            if(debugme)
              cerr<< csp.SA.size() << " + ";
          }
        }

        if(debugme)
          cerr<< "\nsa "<<csp.iterator + 1 << "/"<<csp.SA.size();

        bool Part2=false;
        ArgVectorPointer funargs= qp->Argument(filter);
        ((*funargs)[0]).setAddr(localInfo->tup);
        qp->Request(filter, Value);
        Part2= ((CcBool*)Value.addr)->GetValue();
        while(!Part2 && csp.MoveNext())
        {
          qp->Request(filter, Value);
          Part2= ((CcBool*)Value.addr)->GetValue();
        }
        if(Part2)
        {
          if(debugme)
            cerr<< "\nsa "<<csp.iterator + 1 << "/"<<csp.SA.size();
          Tuple *newTuple = new Tuple( localInfo->resultTupleType );
          for( int i = 0; i < localInfo->tup->GetNoAttributes(); i++ )
            newTuple->CopyAttribute( i, localInfo->tup, i );
          csp.AppendSolutionToTuple(csp.iterator, localInfo->tup, newTuple);
          result.setAddr(newTuple);
          return YIELD;
        }
      }
    }break;
    case CLOSE :
    {
      if(local.addr != 0)
      {
        ((STPExtendStreamInfo*)local.addr)->resultTupleType->DeleteIfAllowed();
        delete (STPExtendStreamInfo*)local.addr;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);
      csp.Clear();
    }break;
    default:
      assert( 0);
  }
  return 0;
}

int STConstraintVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  assert(0); //this function should never be invoked.
  return 0;
}

template <bool leftbound> int StartEndVM
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  Interval<Instant> interval;
  Word input;
  string lbl;

  //qp->Request(args[0].addr, input);
  lbl= ((CcString*)args[0].addr)->GetValue();
  if(debugme)
  {
    cout<<endl<<"Accessing the value of label "<<lbl;
    cout.flush();
  }

  Instant res(0,0,instanttype);
  bool found=false;
  if(leftbound)
    found=csp.GetStart(lbl, res);
  else
    found=csp.GetEnd(lbl, res);

  if(debugme)
  {
    cout<<endl<<"Value is "; if(found) cout<<"found"; else cout<<"Not found";
    res.Print(cout);
    cout.flush();
  }

  result = qp->ResultStorage( s );
  ((Instant*)result.addr)->CopyFrom(&res);
  return 0;
}

void CreateRandomMBool(Instant starttime, MBool& result)
{
  bool debugme=false,bval=false;
  result.Clear();
  int rnd,i=0,n;
  UBool unit(true);
  Interval<Instant> intr(starttime, starttime, true, false);

  rnd=rand()%20;  //deciding the number of units in the mbool value
  n=++rnd;
  bval= ((rand()%2)==1); //deciding the bool value of the first unit
  while(i++<n)
  {
    rnd=rand()%50000; //deciding the duration of a unit
    while(rnd<2)
      rnd=rand()%50000;
    intr.end.Set(intr.start.GetYear(), intr.start.GetMonth(),
        intr.start.GetGregDay(), intr.start.GetHour(),intr.start.GetMinute(),
        intr.start.GetSecond(),intr.start.GetMillisecond()+rnd); 
    unit.constValue.Set(true, bval);
    unit.timeInterval= intr;
    result.Add(unit);
    intr.start= intr.end;
    bval=!bval;
  }
  if(debugme)
    result.Print(cout);
}

int 
RandomMBoolVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  MBool* res = (MBool*) result.addr;
  DateTime* tstart = (DateTime*) args[0].addr;
  CreateRandomMBool(*tstart,*res);
  return 0;
}

int PassMBoolVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  MBool* res = (MBool*) result.addr;
  MBool* inp = (MBool*) args[0].addr;
  res->CopyFrom(inp);
  return 0;
}

//int RandomDelayVM(ArgVector args, Word& res,
//		int msg, Word& local, Supplier s )
//{
//	bool debugme=true;
//	MPoint *pActual = static_cast<MPoint*>( args[0].addr );
//	Instant *threshold = static_cast<Instant*>(args[1].addr ); 
//
//	//MPoint* result = (MPoint*) qp->ResultStorage(s).addr;
//	MPoint* result = (MPoint*) res.addr;
//	//result->Clear();
//	if(pActual->GetNoComponents()<2 || !pActual->IsDefined())
//		result->CopyFrom(pActual);
//	else
//		RandomDelay(pActual, threshold, *result);
//	
//	if(debugme)
//	{
//		result->Print(cout);
//		cout.flush();
//	}
//	return 0;
//}

int RandomDelayVM(ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  MPoint *pActual = static_cast<MPoint*>( args[0].addr );
  Instant *threshold = static_cast<Instant*>(args[1].addr );

  MPoint* shifted = (MPoint*) qp->ResultStorage(s).addr;

  if(pActual->GetNoComponents()<2 || !pActual->IsDefined())
    shifted->CopyFrom(pActual);
    else
    {
      RandomDelay(pActual, threshold, *shifted);
    }
  result= SetWord(shifted); 
  //This looks redundant but it is really necessary. After 2 hours of 
  //debugging, it seems that the "result" word is not correctly set 
  //by the query processor to point to the results.

  return 0;
}

const string CreateSTVectorSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(stringlist) -> stvector</text--->"
  "<text>vec( _ )</text--->"
  "<text>Creates a vector temporal connector.</text--->"
  "<text>let meanwhile = vec(\"abab\",\"abba\",\"aba.b\")</text--->"
  ") )";

const string STPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList -> bool</text--->"
  "<text>_ stpattern[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator implements the Spatiotemporal Pattern Predicate."
  "</text--->"
  "<text>query Trains feed filter[. stpattern[a: .Trip inside msnow,"
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;"
  "stconstraint(\"a\",\"b\",vec(\"aabb\")), "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"))  ]] count </text--->"
  ") )";

const string STPatternExSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ stpatternex[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern Predicate."
  "</text--->"
  "<text>query Trains feed filter[. stpatternex[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string STPatternExtendSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) ( <text>stream(tuple(x)) X namedFunlist X constraintList -> "
  "stream(tuple(x,alias1:Periods,...,alias1:Periods))</text--->"
  "<text>_ stpatternextend[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator extends the input stream with the first supported "
  "assignment. Tuples that doesn't fulfill the pattern are extended with "
  "undef values.</text--->"
  "<text>query Trains feed stpatternextend[insnow: .Trip inside msnow,"
  "isclose: distance(.Trip, mehringdamm)<10.0, fast: speed(.Trip)>8.0 ;"
  "stconstraint(\"insnow\",\"isclose\",vec(\"aabb\")), "
  "stconstraint(\"isclose\",\"fast\",vec(\"bbaa\"))  ]] count </text--->"
  ") )";

const string STPatternExExtendSpec = "( (\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) ( <text>stream(tuple(x)) X namedFunlist X constraintList "
  "X bool-> stream(tuple(x,alias1:Periods,...,alias1:Periods))</text--->"
  "<text>_ stpatternexextend[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator extends the input stream with the first supported "
  "assignment. Tuples that doesn't fulfill "
  "the pattern are extended with undef values.</text--->"
  "<text>query Trains feed stpatternexextend[insnow: .Trip inside msnow,"
  "isclose: distance(.Trip, mehringdamm)<10.0, fast: speed(.Trip)>8.0 ;"
  "stconstraint(\"insnow\",\"isclose\",vec(\"aabb\")), "
  "stconstraint(\"isclose\",\"fast\",vec(\"bbaa\"));  (end(\"b\") - "
  "start(\"a\")) < [const duration value (1 0)]  ]] count </text--->"
  ") )";

const string STPatternExtendStreamSpec  = "( ( \"Signature\" \"Syntax\" "
  "\"Meaning\" "
  "\"Example\" ) ( <text>stream(tuple(x)) X namedFunlist X constraintList -> "
  "stream(tuple(x,alias1:Periods,...,alias1:Periods))</text--->"
  "<text>_ stpatternextend[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator extends each tuple in the input stream with all the "
  "supported assignemts (i.e. periods that fulfill the pattern). Tuples that "
  "doesn't fulfill the pattern don't appear in the results.</text--->"
  "<text>query Trains feed stpatternextend[insnow: .Trip inside msnow,"
  "isclose: distance(.Trip, mehringdamm)<10.0, fast: speed(.Trip)>8.0 ;"
  "stconstraint(\"insnow\",\"isclose\",vec(\"aabb\")), "
  "stconstraint(\"isclose\",\"fast\",vec(\"bbaa\"))  ]] count </text--->"
  ") )";

const string STPatternExExtendStreamSpec = "( (\"Signature\" \"Syntax\" "
  "\"Meaning\" "
  "\"Example\" ) ( <text>stream(tuple(x)) X namedFunlist X constraintList "
  "X bool-> stream(tuple(x,alias1:Periods,...,alias1:Periods))</text--->"
  "<text>_ stpatternexextend[ namedFunlist;  constraintList ]</text--->"
    "<text>The operator extends each tuple in the input stream with all the "
    "supported assignemts (i.e. periods that fulfill the pattern). Tuples that "
    "doesn't fulfill the pattern don't appear in the results.</text--->"
  "<text>query Trains feed stpatternexextend[insnow: .Trip inside msnow,"
  "isclose: distance(.Trip, mehringdamm)<10.0, fast: speed(.Trip)>8.0 ;"
  "stconstraint(\"insnow\",\"isclose\",vec(\"aabb\")), "
  "stconstraint(\"isclose\",\"fast\",vec(\"bbaa\"));  (end(\"b\") - "
  "start(\"a\")) < [const duration value (1 0)]  ]] count </text--->"
  ") )";

const string STConstraintSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>string X string X stvector -> bool</text--->"
  "<text>_ stconstraint( string, string, vec(_))</text--->"
  "<text>The operator is used only within the stpattern and stpatternex "
  "operators. It is used to express a spatiotemporal constraint. The operator "
  "doesn't have a value mapping function because it is evaluated within the "
  "stpattern. It should never be called elsewhere."
  "</text--->"
  "<text>query Trains feed filter[. stpattern[a: .Trip inside msnow,"
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;"
  "stconstraint(\"a\",\"b\",vec(\"aabb\")), "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));"
  "(end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count </text--->"
  ") )";

const string StartEndSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>string -> instant</text--->"
  "<text>start( _ )/ end(_)</text--->"
  "<text>Are used only within an extended spatiotemporal pattern predicate. "
  "They return the start and end time instants for a predicate in the SA list."
  "These operators should never be called unless within the stpatternex "
  "operator."
  "</text--->"
  "<text> query Trains feed filter[. stpatternex[a: .Trip inside msnow, "
  "b:distance(.Trip, mehringdamm)<10.0 ; "
  "stconstraint(\"a\",\"b\", vec(\"aabb\", \"aab.b\")) ; "
  "(start(\"b\")-end(\"a\"))< [const duration vecalue(0 1200000)] ] ] "
  "count </text--->"
  ") )";

const string RandomMBoolSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> instant -> mbool</text--->"
  "<text>randommbool( _ )</text--->"
  "<text>Creates a random mbool value. The operator is used for testing"
  "purposes.</text--->"
  "<text>let mb1 = randommbool(now())</text--->"
  ") )";

const string PassMBoolSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mbool -> mbool</text--->"
  "<text>passmbool( _ )</text--->"
  "<text>Mimics a lifted predicate. The operator takes the name"
  "of an mbool dbobject and return the object itself. The operator is "
  "used for testing purposes.</text--->"
  "<text>let mb2= passmbool(mb1)</text--->"
  ") )";

const string RandomDelaySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint x duration -> mpoint</text--->"
  "<text>randomdelay(schedule, delay_threshold)</text--->"
  "<text>Given an mpoint and a duration value, the operator randomly shift the" 
  "start and end intstants of every unit in the mpoint. This gives the "
  "effect of having positive and negative delays in the movement. The " 
  "random shift value is bound by the given threshold.</text--->"
  "<text>query randomdelay(train7)</text--->"
  ") )";


Operator createstvector (
    "vec",    //name
    CreateSTVectorSpec,     //specification
    CreateSTVectorVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CreateSTVectorTM        //type mapping
);

Operator stpattern (
    "stpattern",    //name
    STPatternSpec,     //specification
    STPatternVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternTM        //type mapping
);

Operator stpatternex (
    "stpatternex",    //name
    STPatternExSpec,     //specification
    STPatternExVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExTM        //type mapping
);

Operator stpatternextend (
    "stpatternextend",    //name
    STPatternExtendSpec,     //specification
    STPatternExtendVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM        //type mapping
);

Operator stpatternexextend (
    "stpatternexextend",    //name
    STPatternExExtendSpec,     //specification
    STPatternExExtendVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExExtendTM        //type mapping
);

Operator stpatternextendstream (
    "stpatternextendstream",    //name
    STPatternExtendStreamSpec,     //specification
    STPatternExtendStreamVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM        //type mapping
);

Operator stpatternexextendstream (
    "stpatternexextendstream",    //name
    STPatternExExtendStreamSpec,     //specification
    STPatternExExtendStreamVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExExtendTM        //type mapping
);

Operator stconstraint (
    "stconstraint",    //name
    STConstraintSpec,     //specification
    STConstraintVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STConstraintTM        //type mapping
);

Operator start (
    "start",    //name
    StartEndSpec,     //specification
    StartEndVM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    StartEndTM        //type mapping
);

Operator end (
    "end",    //name
    StartEndSpec,     //specification
    StartEndVM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    StartEndTM        //type mapping
);

Operator randommbool (
    "randommbool",               // name
    RandomMBoolSpec,             // specification
    RandomMBoolVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    RandomMBoolTM          // type mapping
);

Operator passmbool (
    "passmbool",               // name
    PassMBoolSpec,             // specification
    PassMBoolVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    PassMBoolTM          // type mapping
);

Operator randomdelay (
    "randomdelay",               // name
    RandomDelaySpec,             // specification
    RandomDelayVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    RandomDelayTM          // type mapping
);



class STPatternAlgebra : public Algebra
{
public:
  STPatternAlgebra() : Algebra()
  {

    AddTypeConstructor( &stvectorTC );

/*
The spattern and stpatternex operators are registered as lazy variables.

*/
    stpattern.SetRequestsArguments();
    stpatternex.SetRequestsArguments();
    stpatternextend.SetRequestsArguments();
    stpatternexextend.SetRequestsArguments();
    stpatternextendstream.SetRequestsArguments();
    stpatternexextendstream.SetRequestsArguments();
    AddOperator(&STP::createstvector);
    AddOperator(&STP::stpattern);
    AddOperator(&STP::stconstraint);
    AddOperator(&STP::stpatternex);
    AddOperator(&STP::stpatternextend);
    AddOperator(&STP::stpatternexextend);
    AddOperator(&STP::stpatternextendstream);
    AddOperator(&STP::stpatternexextendstream);
    AddOperator(&STP::start);
    AddOperator(&STP::end);
    AddOperator(&randommbool);
    AddOperator(&passmbool);
    AddOperator(&randomdelay);
  }
  ~STPatternAlgebra() {};
};

};

/*
5 Initialization

*/



extern "C"
Algebra*
InitializeSTPatternAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new STP::STPatternAlgebra;
    }
