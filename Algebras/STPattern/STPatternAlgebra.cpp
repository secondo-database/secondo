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
  const UBool* unit;
  vec.clear();
  Interval<CcReal> elem(CcReal(true,0.0),CcReal(true,0.0),true,true);
  for(int i=0; i<mb->GetNoComponents(); i++)
  {
    mb->Get(i, unit);
    if( ((CcBool)unit->constValue).GetValue())
    {
      IntervalInstant2IntervalCcReal(unit->timeInterval, elem);
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
    if(Agenda[i] != 0)
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
    Agenda[varIndex]=0;
    MBool2Vec((MBool*)Value.addr, domain);
    if(domain.size()==0) return false;
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
  ConstraintGraph.clear();
  VarAliasMap.clear();
  assignedVars.clear();
  count=0;
  iterator=-1;
  return 0;
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

  ListExpr tupleExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args);    //STConstraint list

  nl->WriteToString(argstr, tupleExpr);

  //  //checking for the first parameter tuple(x)
  CHECK_COND( (nl->ListLength(tupleExpr) == 2) &&
      (TypeOfRelAlgSymbol(nl->First(tupleExpr)) == tuple),
      "Operator stpattern: expects as first argument "
      "a list with structure (tuple ((a1 t1)...(an tn))).\n"
      "But got '" + argstr + "'.");

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  CHECK_COND( ! nl->IsAtom(NamedPredList) ,
      "Operator  stpattern expects as second argument a "
      "list of aliased lifted predicates.\n"
      "But got '" + argstr + "'.\n" );

  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    CHECK_COND
    ((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        nl->IsAtom(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Second(NamedPred))== "mbool"),
        "Operator stpattern: expects a list of aliased predicates. "
        "But got '" + argstr + "'.");
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    CHECK_COND
    ((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool"),
        "Operator stpattern: expects a list of temporal connectors. "
        "But got '" + argstr + "'.");
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

  ListExpr tupleExpr = nl->First(args),   //tuple(x)
  NamedPredList  = nl->Second(args),  //named predicate list
  ConstraintList = nl->Third(args);    //STConstraint list

  nl->WriteToString(argstr, tupleExpr);

  //  //checking for the first parameter tuple(x)
  CHECK_COND( (nl->ListLength(tupleExpr) == 2) &&
      (TypeOfRelAlgSymbol(nl->First(tupleExpr)) == tuple),
      "Operator stpattern: expects as first argument "
      "a list with structure (tuple ((a1 t1)...(an tn))).\n"
      "But got '" + argstr + "'.");

  //  //checking ofr the second parameter predicatelist
  nl->WriteToString(argstr, NamedPredList);
  CHECK_COND( ! nl->IsAtom(NamedPredList) ,
      "Operator  stpattern expects as second argument a "
      "list of aliased lifted predicates.\n"
      "But got '" + argstr + "'.\n" );

  ListExpr NamedPredListRest = NamedPredList;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    nl->WriteToString(argstr, NamedPred);

    CHECK_COND
    ((nl->ListLength(NamedPred) == 2 &&
        nl->IsAtom(nl->First(NamedPred))&&
        nl->IsAtom(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Second(NamedPred))== "mbool"),
        "Operator stpattern: expects a list of aliased predicates. "
        "But got '" + argstr + "'.");
  }

  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);
    nl->WriteToString(argstr, STConstraint);

    CHECK_COND
    ((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool"),
        "Operator stpattern: expects a list of temporal connectors. "
        "But got '" + argstr + "'.");
  }

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stpattern accepted the input";
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
  Supplier root, namedpredlist, namedpred,alias, pred, constraintlist,
  constraint, alias1, alias2, stvector;
  Word Value;
  string aliasstr, alias1str, alias2str;
  int noofpreds, noofconstraints;


  result = qp->ResultStorage( s );
  root = args[0].addr;

  namedpredlist = args[1].addr;
  constraintlist= args[2].addr;

  noofpreds= qp->GetNoSons(namedpredlist);
  noofconstraints= qp->GetNoSons(constraintlist);

  csp.Clear();
  for(int i=0; i< noofpreds; i++)
  {
    namedpred= qp->GetSupplierSon(namedpredlist, i);
    alias= qp->GetSupplierSon(namedpred, 0);
    pred = qp->GetSupplierSon(namedpred, 1);
    aliasstr= nl->ToString(qp->GetType(alias));
    csp.AddVariable(aliasstr,pred);
  }

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
  Supplier root, namedpredlist, namedpred,alias, pred, constraintlist, filter,
  constraint, alias1, alias2, stvector;
  Word Value;
  string aliasstr, alias1str, alias2str;
  int noofpreds, noofconstraints;

  if(debugme)
  {
    cout<<" Inside STPatternEXVM\n";
    cout.flush();
  }

  result = qp->ResultStorage( s );
  root = args[0].addr;

  namedpredlist = args[1].addr;
  constraintlist= args[2].addr;
  filter= args[3].addr;

  noofpreds= qp->GetNoSons(namedpredlist);
  noofconstraints= qp->GetNoSons(constraintlist);

  csp.Clear();
  for(int i=0; i< noofpreds; i++)
  {
    namedpred= qp->GetSupplierSon(namedpredlist, i);
    alias= qp->GetSupplierSon(namedpred, 0);
    pred = qp->GetSupplierSon(namedpred, 1);
    aliasstr= nl->ToString(qp->GetType(alias));
    csp.AddVariable(aliasstr,pred);
  }

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
  while(csp.MoveNext() && !Part2)
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
    AddOperator(&STP::createstvector);
    AddOperator(&STP::stpattern);
    AddOperator(&STP::stconstraint);
    AddOperator(&STP::stpatternex);
    AddOperator(&STP::start);
    AddOperator(&STP::end);
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
