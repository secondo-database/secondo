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

Started June, 2009 Mahmoud Sakr

[TOC]

\section{Overview}

This source file contains the necessary implementations for evaluating the
spatiotemporal pattern (STP) predicates [Mahmoud Attia Sakr,
Ralf Hartmut G\"{u}ting: Spatiotemporal pattern queries, Geoinformatica (2011)
15:497-540]. It contains the implementations of the following:
\begin{itemize}
\item The \emph{stvector} type for representing a temporal relationship between
two time intervals. There are 26 such simple relatioships accounting for
Allen's 13 operator, and variants of them that allow each of the two intervals
to degenerate into a time instant. The \emph{stvector} type is a bit vector
(represented by a single integer) containing one bit for each of these 26
simple relationships. An object of type \emph{stvector} represents a temporal
relationship, which is the disjunction of all the simple relationships whose
corresponding bits are set to 1.
\item The \emph{CSP} class (stands for Constraint Satisfaction Problem). This
class implements our algorithm for solving the CSP associated with the STP
predicate.
\item The algebra operators. See the most recent list of operators at the end of
this file.
\item In 2011, the implementation of the temporal reasoning was added by
Mahmoud Sakr. For more details about this technique, please refer to the PhD
thesis of Mahmoud Sakr, Chapter 6 Temporal Reasoning.
\end{itemize}

\section{Defines and includes}

*/
#include "STPatternAlgebra.h"
#include <limits>
#include "Symbols.h"
#include "Stream.h"

namespace STP{

/*
\section{Classes}

\subsection{Class STVector}

~StrSimpleConnectors~\\
A string array that enumerates the 26 simple interval relationships. Each of
these entries describes a relationship between two intervals. The traditional
way that used to be adopted in the literature is to assign names for such
relationships (e.g., before, after, etc like the Allen's operators. Here we
alternatively propose a language instead of names. This is because in our case
26 such relationships are possible, which makes it difficult for a user to
memorize the names. In the entries below, the letters $aa$ denote the begin and
end time instants of the first interval. Similarly $bb$ are the begin and end
of the second interval. The order of the letters describes the temporal
relationship, that is, a sequence $ab$ means $a < b$. The dot symbol denotes
the equality constraint, hence, the sequence $a.b$ means $a = b$, and $a.a$
means that the start and the ends of the first interval are the same
(i.e., the interval degenerates into a time instant). For example, the
relationship \emph{aa.bb} between the intervals $i_1, i_2$ denotes the
order: ($(i_1.t_1 < i_1.t_2) \wedge (i_1.t_2 = i_2.t_1) \wedge
(i_2.t_1 < i_2.t_2)$).

*/
string StrSimpleConnectors[]= {"aabb"  , "bbaa"  ,  "aa.bb"  ,  "bb.aa"  ,
    "abab"  ,  "baba"  ,  "baab"  ,  "abba"  ,  "a.bab"  ,  "a.bba"  ,
    "baa.b"  ,  "aba.b"  ,  "a.ba.b"  ,  "a.abb"  ,  "a.a.bb"  ,
    "ba.ab"  ,  "bb.a.a"  ,  "bba.a"  ,  "b.baa"  ,  "b.b.aa"  ,
    "ab.ba"  ,  "aa.b.b"  ,  "aab.b"  ,  "a.ab.b"  ,  "a.a.b.b"  ,
    "b.ba.a"  };

/*
~STVector::Count~\\
A utility for counting the number of simple relationships inside an ~stvector~
instance.

*/
int STVector::Count(int vec)
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

/*
~STVector::Str2Simple~\\
A utility that gets a string representing one of the 26 simple relationships,
and returns the bit vector corresponding to this simple relationship.

*/
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

/*
~STVector::Vector2List~\\
A utility that converts an ~stvector~ into a nested list of symbols. This is
used in the out function of the ~stvector~ type to provide a nice display of
the vector content.

*/
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

/*
~STVector::Add~\\
Inserts a simple relationship into an ~stvector~ instance. The relationship is
given as a string. If the relationship is already there in the vector,
nothing changes.

*/
bool STVector::Add(string simple)
{
  int vec= Str2Simple(simple);
  if(vec==-1) return false;
  v = v|vec;
  return true;
}

/*
~STVector::Add~\\
Inserts a simple relationship into an ~stvector~ instance. The relationship is
given as an integer (i.e., the bit vector of the given relationship). If the
relationship is already there in the vector, nothing changes.

*/
bool STVector::Add(int simple)
{
  if(simple <=  b_ba_a && Count(simple)==1) 
  {
    v = v|simple;
    return true;
  }
  return false;
}

/*
~STVector::ApplyVector~\\
Checks whether the relationship specified in this ~stvector~ instance holds for
the two intervals ~p1, p2~. The function yields true if any of the simple
relationships inside the vector holds for the given two intervals.

*/
bool STVector::ApplyVector(Interval<Instant>& p1, Interval<Instant>& p2)
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

/*
~STVector::ApplySimple~\\
Checks whether the given simple relationship holds for the two intervals
~p1, p2~.

*/
bool STVector::ApplySimple(Interval<Instant>& p1, Interval<Instant>& p2,
    int simple)
{ 
  Instant  a=p1.start,   A=p1.end,    b=p2.start,   B=p2.end;
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
    assert(0); //invalid simple relationships
  }
  return false;
}

/*
~STVector::Clear~\\
Clears all entries in an ~stvector~ instnace.

*/
void STVector::Clear()
{
  this->count= 0;
  this->v= 0;
}

/*
SECONDO framework support functions. These functions are required by SECONDO
to be able to deal with the ~stvector~ type.

In, Out, Create, Delete, Open, Save, Close, Clone, SizeOfObj, Property, and
KindCheck.

*/

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

/*
\subsection{Class CSP}

~csp~\\
A global variable holding the csp during the evaluation of the STP operator.

*/
CSP csp;

/*
~Constructor and destructor~\\

*/
CSP::CSP(): closureRes(notPA), count(0),iterator(-1),
    nullInterval(Instant((int64_t)0),Instant((int64_t)0), true,true)
{}

CSP::~CSP()
{
}

/*
~CSP::IntervalInstant2IntervalCcReal~\\
A utility to convert an interval<instant> into an interval<real>. This is used
to speed up the computations, because comparing reals is faster than comparing
instants.

In 2011 Thomas Behr added another mechanism to deal with instants as int64.
Comparing instants is not anymore slow. Since then, this algebra was modified,
and the ~CSP::IntervalInstant2IntervalCcReal~ utility, and its inverse
~CSP::IntervalCcReal2IntervalInstant~ are not anymore used.

*/
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
~CSP::MBool2Vec~\\
This function is used within the ~extend~ function. It gets an mbool instance
and yields the list of time intervals during which this mbool instance is true.
It is equivalent to deftime(mb at TRUE).

*/
int CSP::MBool2Vec(const MBool* mb, vector<Interval<Instant> >& vec)
{
  UBool unit(0);
  vec.clear();
  if(!mb->IsDefined() || mb->IsEmpty() || !mb->IsValid()) return 0;
  for(int i=0; i<mb->GetNoComponents(); i++)
  {
    mb->Get(i, unit);
    if( ((CcBool)unit.constValue).GetValue())
      vec.push_back(unit.timeInterval);
  }
  return 0;
}

/*
~CSP::Solve~\\
This is the main interface to the CSP class. When called it iteratively tries
to find a solution to the CSP. If such a solution is found it yields true.
Our proposed algorithm for solving the CSP tries to solve the sub-CSP of
$k-1$ variables ($\mathit{CSP_{k-1}}$) first and then to extend it to
$\mathit{CSP_k}$. Therefore, an early stop is possible if a solution to the
$\mathit{CSP_{k-1}}$ cannot be found. Which means that, in case no solution is
found, the evaluation will be stopped as soon as this is realized, without the
unnecessary evaluation of the remaining time-dependent predicates.

This ~CSP::Solve~ function uses three data structures, which are made
globally accessible for all its helper functions: the $\mathit{SA}$ list (for
Supported Assignments), the \emph{Agenda} and the \emph{ConstraintGraph}. The
Agenda keeps a list of variables that are not yet consumed by the algorithm. One
variable from the Agenda is consumed in every iteration. Consuming a variable is
equivalent to computing its domain (i.e., evaluating the corresponding
time-dependent predicate). Every supported assignment in the $\mathit{SA}$ list
is a solution for the sub-CSP consisting of the variables that have been
consumed so far. In iteration $k$ there are $k-1$ previously consumed variables
and one newly consumed variable ($X_k$ with domain $D_k$). Every entry in
$\mathit{SA}$ at this iteration is a solution for the $\mathit{CSP_{k-1}}$. To
extend the $\mathit{SA}$, the Cartesian product of $\mathit{SA}$ and $D_k$ is
computed. Then only the entries that constitute a solution for
$\mathit{CSP_{k}}$ are kept in $\mathit{SA}$. $\mathit{CSP_{k}}$ is
constructed using the consumed variables and their corresponding constraints in
the constraint graph.

*/
bool CSP::Solve()
{
  bool debugme=false;
  int varIndex;
  Word Value;
  vector<Interval<Instant> > domain(0);
  int loopCnt=0;
  Counter::getRef("TemporalReasoningOn", false)= 0;
  string counterPrefix= "CSP::Solve::Iteration";
  if(debugme)
    cerr<<"Iterations ";
  while( (varIndex= PickVariable()) != -1)
  {
    ++loopCnt;
    if(debugme)
      cerr<<loopCnt<<", ";
    Counter::getRef(counterPrefix + int2Str(loopCnt), true)++;
    qp->Request(Agenda[varIndex], Value);
    //Agenda[varIndex]=0;
    UsedAgendaVars[varIndex]= true;
    MBool2Vec((MBool*)Value.addr, domain);
    if(domain.size()==0) {SA.clear(); return false;}
    if(Extend(varIndex, domain)!= 0) return false;
    if(SA.size()==0) return false;
    if(debugme&&0)
      Print();
  }
  return true;
}

/*
~CSP::Extend~\\
A helper function called by the previous ~CSP::Solve~. Mainly it distinguishes
between two cases:
\begin{itemize}
\item If the variable given is the first agenda variable to be consumed,
    the SA is initialized.
\item Else, the function creates and checks (on the fly) the Cartesian product
    of SA and the domain of the variable.
\end{itemize}

*/
int CSP::Extend(int index, vector<Interval<Instant> >& domain )
{
  vector<Interval<Instant> > sa(count);

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
~CSP::IsSupported~\\
This function searches the ConstraintGraph for the constraints that
involve the given variable and check their fulfillment. It is modified to check
only the constraints related to the newly evaluated variable instead of 
re-checking all the constraints. 

*/
bool CSP::IsSupported(vector<Interval<Instant> > sa, int index)
{
  bool debugme= false;
  bool supported=false;
  if(debugme)
  {
    cerr<<endl<<assignedVars.size()<<endl;
    for(unsigned int u=0; u<assignedVars.size(); u++)
      cerr<< assignedVars[u];
  }
  for(unsigned int i=0; i<assignedVars.size()-1; i++)
  {
    for(unsigned int j=i; j<assignedVars.size(); j++)
    {
      if(debugme)
        cerr<<endl<<"Checking constraints ("<<assignedVars[i] <<
        ", "<< assignedVars[j] <<") and ("<< assignedVars[j] <<
        ", "<< assignedVars[i] << ")";
      if(assignedVars[i]== index || assignedVars[j] == index )
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

/*
~CSP::CheckConstraint~\\
A predicate that checks a temporal constraint, represented as an ~stvector~
instance, on two time intervals.

*/
bool CSP::CheckConstraint(Interval<Instant>& p1, Interval<Instant>& p2,
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
~CSP::PickVariable~\\
The function PickVariable picks Agenda variables according to their
connectivity rank.
\begin{alltt}
For every variable v
\{
  For every constraint c(v,x)
  \{
   if x in Agenda rank+=0.5
   if x is not in the Agenda rank+=1
  \}
\}
\end{alltt}

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

/*
~CSP::AddVariable~\\
Inserts one variable into the CSP, and reserves place for it in the
ConstraintGraph.

*/
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

/*
~CSP::AddConstraint~\\
Inserts one binary constraint into the CSP.

*/
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


/*
~CSP::MoveNext~\\
Iterates over the supported assignments. This function is used, for example, in
the ~stpatternex~ operator in order to iterate over the SA, and find solutions
to the second part of the predicate.

*/
bool CSP::MoveNext()
{
  if(iterator < (signed int)SA.size()-1)
    iterator++;
  else
    return false;
  return true;
}

/*
~CSP::GetSA~\\
Yields an SA entry.

*/
bool CSP::GetSA(unsigned int saIndex, unsigned int varIndex, Periods& res)
{
  res.Clear();
  if(saIndex >= SA.size() || varIndex >= SA[0].size())
    return false;
  res.Add(SA[saIndex][varIndex]);
  return true;
}

/*
~CSP::AppendSolutionToTuple~\\
A utility that is used within the stpattern??extend? operators to append the
fulfillment periods of the time-dependent predicates to an input tuple that
matches the pattern.

*/
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

/*
~CSP::AppendUnDefsToTuple~\\
A utility that is used within the stpattern??extend? operators to append the
undefined periods to an input tuple that does not match the pattern.

*/
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

/*
~CSP::GetStart~\\
Used in the second part of the extended STP predicate. It accesses the current
SA entry (specified by the SA iterator), and yields the starting time instant
of the fulfillment interval of the given time-dependent predicate.

*/
bool CSP::GetStart(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.CopyFrom(& SA[iterator][index].start);
  return true;
}

/*
~CSP::GetEnd~\\
Used in the second part of the extended STP predicate. It accesses the current
SA entry (specified by the SA iterator), and yields the final time instant
of the fulfillment interval of the given time-dependent predicate.

*/
bool CSP::GetEnd(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.CopyFrom(& SA[iterator][index].end);
  return true;
}

/*
~CSP::Print~\\
A debug utility. It prints the content of the CSP to the stderr.

*/
void CSP::Print()
{
  cout<< "\n==========================\nSA.size() = "<< SA.size()<< endl;
  for(unsigned int i=0; i< SA.size(); i++)
  {
    for(unsigned int j=0; j<SA[i].size(); j++)
    {
      cout<<SA[i][j].start.millisecondsToNull()<< "\t "<<
          SA[i][j].end.millisecondsToNull()<<" | ";
    }
    cout<<endl;
  }
  cout.flush();
}

/*
~CSP::Clear~\\
Clears this CSP instance, and prepare it for the next operator call.

*/
int CSP::Clear()
{
  closureRes= notPA;
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

/*
~CSP::ResetTuple~\\
Partially clears this CSP instance, and prepare it for matching a new tuple
against the pattern. The Agenda, and the ConstraintGraph are kept. Other members
related to the evaluation are reset.

*/
int CSP::ResetTuple()
{
  SA.clear();
  UsedAgendaVars.assign(Agenda.size(), false);
  assignedVars.clear();
  iterator=-1;
  return 0;
}


/*
\section{Auxiliary functions}
This section defines some helper functions that are used within operators or
other class members.

~RandomDelay~\\
Apply different random delays to the units of the given mpoint. The resulting
mpoint will have the same trajectory but different unit speeds form the input
mpoint.

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
~IsSimplePredicateList~\\
A typemap helper function. It accepts the signature ((Alias: mbool), ...).

*/
bool IsSimplePredicateList(ListExpr args, bool WithUserArgs=false)
{
  if(nl->IsAtom(args)) return false;

  ListExpr NamedPredListRest = WithUserArgs? nl->First(args): args;
  ListExpr NamedPred;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);

    if(!(nl->ListLength(NamedPred) == 2 && nl->IsAtom(nl->First(NamedPred))&&
        nl->IsAtom(nl->Second(NamedPred))&&
        nl->SymbolValue(nl->Second(NamedPred))== MBool::BasicType()))
      return false;
  }
  return true;
}

/*
~IsMapTuplePredicateList~\\
A typemap helper function. It accepts the signature
(Alias: map(tuple) -> mbool), ...).

*/
bool IsMapTuplePredicateList(ListExpr args, bool WithUserArgs=false)
{
  if(nl->IsAtom(args)) return false;

  ListExpr NamedPredListRest = WithUserArgs? nl->First(args): args;
  ListExpr NamedPred, Map;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    Map= nl->Second(NamedPred);
    if(nl->ListLength(NamedPred) != 2 || !nl->IsAtom(nl->First(NamedPred)) ||
        !listutils::isMap<1>(Map)||
        !nl->IsEqual(nl->Nth(nl->ListLength(Map), Map), MBool::BasicType()))
      return false;
    if(!nl->IsEqual(nl->First(nl->Second(Map)), Tuple::BasicType()))
      return false;
  }
  return true;
}

/*
~IsMapTuplePredicateList~\\
A typemap helper function. It accepts the signature
(Alias: map(tuple, periods) -> mbool), ...).

*/
bool IsMapTuplePeriodsPredicateList(ListExpr args, bool WithUserArgs=false)
{
  if(nl->IsAtom(args)) return false;

  ListExpr NamedPredListRest = WithUserArgs? nl->First(args): args;
  ListExpr NamedPred, Map;
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);
    Map= nl->Second(NamedPred);
    if(nl->ListLength(NamedPred) != 2 || !nl->IsAtom(nl->First(NamedPred)) ||
        !listutils::isMap<2>(Map)||
        !nl->IsEqual(nl->Nth(nl->ListLength(Map), Map), MBool::BasicType()))
      return false;
    if(!nl->IsEqual(nl->First(nl->Second(Map)), Tuple::BasicType()) ||
        !nl->IsEqual(nl->Third(Map), Periods::BasicType()) )
      return false;
  }
  return true;
}

/*
~IsConstraintList~\\
A typemap helper function. It accepts the signature
(bool, ...).

*/
bool IsConstraintList(ListExpr args, bool WithUserArgs=false)
{
  if(nl->IsAtom(args)) return false;

  ListExpr ConstraintListRest = WithUserArgs? nl->First(args): args;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);

    if(!((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== CcBool::BasicType())))
      return false;
  }
  return true;
}

/*
~IsTupleExpr~\\
A typemap helper function. It accepts the signature (tuple (attrs)).

*/
bool IsTupleExpr(ListExpr args, bool WithUserArgs=false)
{
  ListExpr tuple = WithUserArgs? nl->First(args): args;
  return ((nl->ListLength(tuple) == 2) &&
      nl->IsEqual(nl->First(tuple), Tuple::BasicType()));
}

/*
~IsTupleStream~\\
A typemap helper function. It accepts the signature (stream streamElems).

*/
bool IsTupleStream(ListExpr args, bool WithUserArgs=false)
{
  ListExpr stream = WithUserArgs? nl->First(args): args;
  return (nl->ListLength(stream)== 2 &&
      listutils::isTupleStream(stream));
}

/*
~IsBoolExpr~\\
A typemap helper function. It accepts the signature bool.

*/
bool IsBoolExpr(ListExpr args, bool WithUserArgs=false)
{
  ListExpr boolExpr = WithUserArgs? nl->First(args): args;
  return (nl->IsAtom(boolExpr) && nl->IsEqual(boolExpr, CcBool::BasicType()));
}


/*
~IsBoolMap~\\
A typemap helper function. It accepts the signature (map (-) bool).

*/
bool IsBoolMap(ListExpr args, bool WithUserArgs=false)
{
  ListExpr boolExpr = WithUserArgs? nl->First(args): args;
  if(nl->IsAtom(args)) return false;

  ListExpr Map = nl->First(boolExpr),
      MapReturn= nl->Nth(nl->ListLength(boolExpr), boolExpr);
  if(nl->ListLength(boolExpr) < 3 || !nl->IsEqual(Map, Symbols::MAP()) ||
      !nl->IsEqual(MapReturn, CcBool::BasicType()))
    return false;
  return true;
}

/*
~IsPeriodsExpr~\\
A typemap helper function. It accepts the signature periods.

*/
bool IsPeriodsExpr(ListExpr args, bool WithUserArgs=false)
{
  ListExpr periodsExpr = WithUserArgs? nl->First(args): args;
  return (nl->IsAtom(periodsExpr) &&
      nl->IsEqual(periodsExpr, Periods::BasicType()));
}

/*
~IsAliasInCatalog~\\
A typemap helper function. It checks whether an alias that is given to a
time-dependent predicate collides with a known DB object name in the SECONDO
catalog.

*/
bool IsAliasInCatalog(set<string>& aliases)
{
  for(set<string>::iterator it= aliases.begin(); it!= aliases.end(); ++it)
    if(SecondoSystem::GetCatalog()->IsTypeName(*it)) return true;
  return false;
}

/*
~IsAliasInAttributeNames~\\
A typemap helper function. It checks whether an alias that is given to a
time-dependent predicate collides with an attribute in the input tuple.

*/
bool IsAliasInAttributeNames(ListExpr AttrList,set<string>& aliases)
{
  ListExpr typeList;
  for(set<string>::iterator it= aliases.begin(); it!= aliases.end(); ++it)
    if(FindAttribute(AttrList, *it, typeList) != 0) return true;
  return false;
}

/*
~ExtractPredAliasesFromPredList~\\
A typemap helper function. It return a set of strings containing all the
aliases of the time-dependent predicates in the STP operator.

*/
void ExtractPredAliasesFromPredList(ListExpr args, set<string>& aliases)
{
  ListExpr NamedPredListRest = nl->First(args);
  ListExpr NamedPred;
  string alias;

  aliases.clear();
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);

    alias= nl->ToString(nl->First(NamedPred));
    aliases.insert(alias);
  }
}

/*
~ExtractPredAliasesFromPredList~\\
A typemap helper function. It return a vector of strings containing all the
aliases of the time-dependent predicates in the STP operator, in the same order
they appear.

*/
void ExtractPredAliasesFromPredList(ListExpr args, vector<string>& aliases)
{
  ListExpr NamedPredListRest = nl->First(args);
  ListExpr NamedPred;
  string alias;

  aliases.clear();
  while( !nl->IsEmpty(NamedPredListRest) )
  {
    NamedPred = nl->First(NamedPredListRest);
    NamedPredListRest = nl->Rest(NamedPredListRest);

    alias= nl->ToString(nl->First(NamedPred));
    aliases.push_back(alias);
  }
}

/*
~ExtractPredAliasesFromConstraintList~\\
A typemap helper function. It return a set of strings containing all the
aliases that are mentioned in the list of tempotral constraints.

*/
void ExtractPredAliasesFromConstraintList(ListExpr args, set<string>& aliases)
{
  ListExpr ConstraintListRest = nl->Second(args);
  ListExpr STConstraint;
  string alias;

  aliases.clear();
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);

    alias= nl->StringValue(nl->Second(STConstraint));
    aliases.insert(alias);

    alias= nl->StringValue(nl->Third(STConstraint));
    aliases.insert(alias);
  }
}

/*
~CSPAddPredicates~\\
A helper function that fills the csp with the time-dependent predicates in the
user query.

*/
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

/*
~CSPAddConstraints~\\
A helper function that fills the csp with the temporal constraints in the
user query.

*/
void CSPAddConstraints(Supplier& constraintlist)
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

/*
~CSPSetPredsArgs~\\
A helper function that fills the argument vector of all the time-dependent
predicates with the current tuple from the input tuple stream.

*/
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

/*
~ComputeClosure~\\
Computes the closure within the Type Map

*/
ListExpr ComputeClosure(ListExpr ConstraintList, vector<string> IntervalVars)
{
  bool debugme= false;

  if(debugme)
  {
    string cList= nl->ToString(ConstraintList);
    cerr<<endl<<cList<<endl;
    for(vector<string>::iterator it= IntervalVars.begin(); it!=
        IntervalVars.end(); ++it)
      cerr<<*it<<'\t';
  }

  enum relIndex{
  aA=0, ab=1, aB=2, ba=3, bA=4, bB=5, Ab=6, AB=7, Ba=8, BA=9, Aa=10, Bb=11};
  //defined in TemporalReasoner.h
  //enum PARelation{lss=0, leq=1, grt=2, geq=3, eql=4, neq=5, uni=6, inc=7};
  PARelation PARels[12];
  int aIndex, AIndex, bIndex, BIndex;
  unsigned int numIntervals= 2 * IntervalVars.size();
  PointAlgebraReasoner PAReasoner(numIntervals);
  map<string, int> alias2IAIndex;
  ListExpr ConstraintListRest = nl->Second(ConstraintList);
  ListExpr STConstraint;
  ListExpr IARelListRest;
  STVector IAVector(0);
  string alias, IARel;
  set<pair<int, int> > relatedPairs;
  int i=-1, j;

  for(vector<string>::iterator
      it=IntervalVars.begin(); it< IntervalVars.end(); ++it)
    assert(alias2IAIndex.insert(pair<string, int>(*it, ++i)).second);
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);

    alias= nl->StringValue(nl->Second(STConstraint));
    i= alias2IAIndex[alias];

    alias= nl->StringValue(nl->Third(STConstraint));
    j= alias2IAIndex[alias];

    if((! relatedPairs.insert(make_pair<int,int>(i,j)).second) ||
       (! relatedPairs.insert(make_pair<int,int>(j,i)).second))
      return nl->TwoElemList(nl->IntAtom(0), nl->IntAtom(notPA));

    IAVector.Clear();
    IARelListRest= nl->Fourth(STConstraint); //(vec "aabb"  "abab" ...)
    IARelListRest= nl->Rest(IARelListRest);  //("aabb"  "abab" ...)
    while( !nl->IsEmpty(IARelListRest) )
    {
      IARel= nl->StringValue(nl->First(IARelListRest));
      IARelListRest= nl->Rest(IARelListRest);
      IAVector.Add(IARel);
    }
    if(! IAVector.Vector2PARelations((int*)PARels))
      return nl->TwoElemList(nl->IntAtom(0), nl->IntAtom(notPA));
    aIndex= i*2; AIndex= (i*2)+1; bIndex= j*2; BIndex= (j*2)+1;
    for(int k=0; k<12; ++k)
    {
      if(PARels[k] == unknown) continue;
      switch(k)
      {
      case aA: PAReasoner.Add(aIndex, AIndex, PARels[k]);
      break;
      case ab: PAReasoner.Add(aIndex, bIndex, PARels[k]);
      break;
      case aB: PAReasoner.Add(aIndex, BIndex, PARels[k]);
      break;
      case ba: PAReasoner.Add(bIndex, aIndex, PARels[k]);
      break;
      case bA: PAReasoner.Add(bIndex, AIndex, PARels[k]);
      break;
      case bB: PAReasoner.Add(bIndex, BIndex, PARels[k]);
      break;
      case Ab: PAReasoner.Add(AIndex, bIndex, PARels[k]);
      break;
      case AB: PAReasoner.Add(AIndex, BIndex, PARels[k]);
      break;
      case Ba: PAReasoner.Add(BIndex, aIndex, PARels[k]);
      break;
      case BA: PAReasoner.Add(BIndex, AIndex, PARels[k]);
      break;
      case Aa: PAReasoner.Add(AIndex, aIndex, PARels[k]);
      break;
      case Bb: PAReasoner.Add(BIndex, bIndex, PARels[k]);
      break;
      default: assert(0);
      }
    }
  }

  if(debugme)
    PAReasoner.Print(cerr);
  bool isConsistent= PAReasoner.Close();
  if(!isConsistent)
    return nl->TwoElemList(nl->IntAtom(0), nl->IntAtom(inconsistent));
  if(debugme)
    PAReasoner.Print(cerr);
  return PAReasoner.ExportToNestedList();
}

/*
~ImportPAReasonerFromArgs~\\
This function is used to import the PointAlgebraReasoner in the value mapping.

*/
ClosureResult ImportPAReasonerFromArgs(
    Supplier TRTable, PointAlgebraReasoner*& paReasoner)
{
  Word value;
  bool debugme=false;
  if(debugme)
    qp->ListOfTree(TRTable, cerr);
  Supplier son= qp->GetSon(TRTable, 0), raw;
  qp->Request(son, value);
  int PAReasonerN= static_cast<CcInt*>(value.addr)->GetValue();
  int tblIndex=0;
  ClosureResult res= consistent;
  if(PAReasonerN != 0)
  {
    paReasoner= new PointAlgebraReasoner(PAReasonerN);
    int *Table= new int[PAReasonerN * PAReasonerN + 1];
    Table[tblIndex++]= PAReasonerN;
    for(int i=1; i<= PAReasonerN; ++i)
    {
      if(debugme)
        cerr<<endl;
      raw= qp->GetSon(TRTable, i);
      for(int j=0; j< PAReasonerN ; ++j)
      {
        son= qp->GetSon(raw, j);
        qp->Request(son, value);
        Table[tblIndex++]= static_cast<CcInt*>(value.addr)->GetIntval();
        if(debugme)
          cerr<<static_cast<CcInt*>(value.addr)->GetIntval()<<",";
      }
    }
    paReasoner->ImportFromArray(Table);
    delete[] Table;
  }
  else
  {
    son= qp->GetSon(TRTable, 1);
    qp->Request(son, value);
    res= static_cast<ClosureResult>(
        static_cast<CcInt*>(value.addr)->GetIntval());
  }
  return res;
}

/*
~CreateMaximalPeriods~\\

*/
Periods* CreateMaximalPeriods()
{
  Instant i1(instanttype);    i1.ToMinimum();
  Instant i2(instanttype);    i2.ToMaximum();
  Interval<Instant> I(i1, i2, true, true);
  Periods* periods= new Periods(0);
  periods->Add(I);
  return periods;
}

/*
~CSPSetPredsArgs~\\

*/
bool CSPSetPredsArgs(Supplier predList, Tuple* tup, Periods* periods)
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
    ((*funargs)[1]).setAddr(periods);
  }
  return true;
}


/*
\section{Algebra Types and Operators}

*/


/*
\subsection{The stvector Type}

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

/*
\subsection{The createstvector Operator}

*/
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
    if(!nl->IsAtom(first) ||  nl->SymbolValue(first)!= CcString::BasicType())
    {
      ErrorReporter::ReportError(
          "Operator v: expects a list of strings but got '" +
          argstr + "'.\n");
      return nl->SymbolAtom(Symbols::TYPEERROR());
    }
  }
  return nl->SymbolAtom(STVector::BasicType());
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

const string CreateSTVectorSpec  =
  "( ( "
  " \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  " ( "
  " <text>(list of string) -> stvector</text--->"
  " <text>vec( _ )</text--->"
  " <text>Creates a vector temporal relations, to be used within the "
  "  spatiotemporal pattern query operators (e.g., stpattern, reportpattern)."
  " </text--->"
  " <text>let meanwhile = vec(\"abab\",\"abba\",\"aba.b\");</text--->"
  ") )";

Operator createstvector (
    "vec",    //name
    CreateSTVectorSpec,     //specification
    CreateSTVectorVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CreateSTVectorTM        //type mapping
);

/*
\subsection{The stpattern and stpatternex Operators}

*/
template<bool extended>
ListExpr STPatternTM(ListExpr args)
{
  bool debugme= false;

  string opName= extended? "stpatternex": "stpattern";
  string argstr;
  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if((!extended && nl->ListLength(args) != 3) ||
      (extended && nl->ListLength(args) != 4))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects " +
        int2string(3 + extended) + "arguments\nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  ListExpr tupleExpr = nl->First(args),
      NamedPredList  = nl->Second(args),
      ConstraintList = nl->Third(args);

  nl->WriteToString(argstr, tupleExpr);
  if(!IsTupleExpr(tupleExpr, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects "
        "argument a list with structure (tuple ((a1 t1)...(an tn))).\n"
        "But got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, NamedPredList);
  if(! IsSimplePredicateList(NamedPredList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "aliased predicates. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, ConstraintList);
  if(!IsConstraintList(ConstraintList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "temporal connectors. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  set<string> predAliases1, predAliases2;
  ExtractPredAliasesFromPredList(NamedPredList, predAliases1);
  ExtractPredAliasesFromConstraintList(ConstraintList, predAliases2);

  if(debugme)
  {
    set<string>::iterator it1= predAliases1.begin(), it2= predAliases2.begin();
    cerr<<endl;
    while(it1 != predAliases1.end())
      cerr<<*it1++<< '\t';
    cerr<<endl;
    while(it2 != predAliases2.end())
      cerr<<*it2++<< '\t';
  }
  if((predAliases1.size() != predAliases2.size()) ||
   !std::equal(predAliases1.begin(), predAliases1.end(), predAliases2.begin()))
  {
    ErrorReporter::ReportError("Operator "+ opName +": unknown alises in "
        "temporal constraints.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  if(extended)
  {
    ListExpr boolExpr= nl->Fourth(args);
    nl->WriteToString(argstr, boolExpr);
    if(!IsBoolExpr(boolExpr, true))
    {
      ErrorReporter::ReportError("Operator "+ opName +": expects a boolean "
          "expression. \nBut got '" + argstr + "'.");
      return nl->SymbolAtom(Symbols::TYPEERROR());
    }
  }

  ListExpr result = nl->SymbolAtom(CcBool::BasicType());
  if(debugme)
  {
    cout<<endl<<endl<<"Operator "+ opName +" accepted the input";
    cout.flush();
  }
  return result;
}

template<bool extended>
int STPatternVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  Word Value;
  Supplier namedpredlist, constraintlist, filter;
  result = qp->ResultStorage( s );
  namedpredlist = args[1].addr;
  constraintlist= args[2].addr;

  if(debugme)
  {
    Word w;
    qp->Request(args[0].addr, w);
    Tuple* tuple= static_cast<Tuple*>(w.addr);
    cerr<<endl<<"Id: "<<
        static_cast<CcInt*>(tuple->GetAttribute(0))->GetValue();
    cerr<<", Line: "<<
        static_cast<CcInt*>(tuple->GetAttribute(1))->GetValue();
  }

  csp.Clear();
  CSPAddPredicates(namedpredlist);
  CSPAddConstraints(constraintlist);

  bool hasSolution= false;
  hasSolution=csp.Solve();

  ((CcBool*)result.addr)->Set(true, hasSolution);
  if(!hasSolution || !extended)
    return 0;

  //if extended
  filter= args[3].addr;
  bool Part2=false;
  while(!Part2 && csp.MoveNext())
  {
    qp->Request(filter, Value);
    Part2= ((CcBool*)Value.addr)->GetValue();
  }
  ((CcBool*)result.addr)->Set(true,Part2);
  return 0;
}

const string STPatternSpec  =
  "( ( "
  " \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  " <text>tuple(x) X namedFunlist X constraintList -> bool</text--->"
  " <text>_ stpattern[ namedFunlist;  constraintList ]</text--->"
  " <text>The operator implements the Spatiotemporal Pattern Predicate. The "
  "  namedFunList parameter is a comma separated list in the form alias: "
  "  time-dependent predicate, ... . The constraintList is a comma separated "
  "  list in the form stconstraint(_,_,_), ... . The stpattern predicate "
  "  yields true iff the input tuple fulfills all the time-dependent "
  "  predicates (namedFunList) in the exact temporal order specified by "
  "  (constraintList). Please refer to Mahmoud Sakr, Ralf Hartmut G\"{u}ting: '"
  "  Spatiotemporal pattern queries', Geoinformatica (2011) 15:497-540."
  " </text--->"
  " <text> "
  "  query Trains feed "
  "   filter[. stpattern["
  "     InSnow: .Trip inside msnow, "
  "     IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "     IsFast: speed(.Trip) > 8.0 "
  "   ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "     stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))  ]] "
  "  count;"
  "</text--->"
  ") )";

const string STPatternExSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  " <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  " <text>_ stpatternex[ namedFunlist;  constraintList; bool ]</text--->"
  " <text>This operator implements the Extended Spatiotemporal Pattern "
  "   Predicate. The original predicate (the stpattern operator) asserts that "
  "   all the time-dependent predicates (namedFunList) are fulfilled in the "
  "   exact temporal order specified in (constraintList). This stpatternex "
  "   predicate additionally allows for a bool condition expressed on the parts"
  "   of the trajecotry that match the pattern. One can use this parameter "
  "   for example to assert that the second time-dependent predicate starts to "
  "   be true no more than 5 minutes after the first time-dependent predicate "
  "   ends to be true. For more information, please refer to Mahmoud Sakr, "
  "   Ralf Hartmut G\"{u}ting: ' Spatiotemporal pattern queries', "
  "   Geoinformatica (2011) 15:497-540.</text--->"
  " <text>query Trains feed "
  "   filter[. stpatternex["
  "     InSnow: .Trip inside msnow, "
  "     IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "     IsFast: speed(.Trip) > 8.0 "
  "   ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "     stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))"
  "   ; (end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)]]]"
  "   count</text--->"
  ") )";

Operator stpattern (
    "stpattern",    //name
    STPatternSpec,     //specification
    STPatternVM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternTM<false>        //type mapping
);

Operator stpatternex (
    "stpatternex",    //name
    STPatternExSpec,     //specification
    STPatternVM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternTM<true>        //type mapping
);

/*
\subsection{The stpatternextend and stpatternexextend Operators}

*/
template<bool extended, bool enableTemporalReasoner, bool extendstream>
ListExpr STPatternExtendTM(ListExpr args)
{
  bool debugme= false;
  string opName;
  if(extended && enableTemporalReasoner && extendstream)
    opName= "stpatternexextendstream2";
  else if(!extended && enableTemporalReasoner && extendstream)
    opName= "stpatternextendstream2";
  else if(extended && !enableTemporalReasoner && extendstream)
    opName= "stpatternexextendstream";
  else if(extended && enableTemporalReasoner && !extendstream)
    opName= "stpatternexextend2";
  else if(!extended && !enableTemporalReasoner && extendstream)
    opName= "stpatternextendstream";
  else if(!extended && enableTemporalReasoner && !extendstream)
    opName= "stpatternextend2";
  else if(extended && !enableTemporalReasoner && !extendstream)
    opName= "stpatternexextend";
  else if(!extended && !enableTemporalReasoner && !extendstream)
    opName= "stpatternextend";

  string argstr;
  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if((!extended && nl->ListLength(args) != 3) ||
      (extended && nl->ListLength(args) != 4))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects " +
        int2string(3 + extended) + "arguments\nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  ListExpr StreamExpr = nl->First(args),   //stream(tuple(x))
      NamedPredList  = nl->Second(args),  //named predicate list
      ConstraintList = nl->Third(args);    //STConstraint list


  nl->WriteToString(argstr, StreamExpr);
  if(!IsTupleStream(StreamExpr, true))
  {
    ErrorReporter::ReportError("Operator "+ opName+ ": expects "
        "a list with structure (stream(tuple ((a1 t1)...(an tn)))).\n"
        "But got '" + argstr + "'.");
    return nl->TypeError();
  };


  nl->WriteToString(argstr, NamedPredList);
  if((enableTemporalReasoner &&
      !IsMapTuplePeriodsPredicateList(NamedPredList, true))||
      (!enableTemporalReasoner &&
          !IsMapTuplePredicateList(NamedPredList, true)))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "aliased predicates. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, ConstraintList);
  if(!IsConstraintList(ConstraintList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "temporal connectors. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  set<string> predAliases1, predAliases2;
  ListExpr TupleExpr = nl->Second(nl->First(StreamExpr));   //tuple(x)
  ListExpr AttrList = nl->Second(TupleExpr);
  ExtractPredAliasesFromPredList(NamedPredList, predAliases1);
  ExtractPredAliasesFromConstraintList(ConstraintList, predAliases2);

  if(IsAliasInAttributeNames(AttrList, predAliases1))
  {
    ErrorReporter::ReportError("Operator "+ opName +": alias"
        " is already an attribute name in the input stream");
    return nl->TypeError();
  }
  if(IsAliasInCatalog(predAliases1))
  {
    ErrorReporter::ReportError("Operator "+ opName +": alias"
        " is a known DB type");
    return nl->TypeError();
  }
  if((predAliases1.size() != predAliases2.size()) ||
   !std::equal(predAliases1.begin(), predAliases1.end(), predAliases2.begin()))
  {
    ErrorReporter::ReportError("Operator "+ opName +": unknown aliases in "
        "temporal constraints.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }


  vector<string> aliases;
  ExtractPredAliasesFromPredList(NamedPredList, aliases);
  ListExpr resAttrlist= nl->OneElemList(nl->TwoElemList(
      nl->First(nl->First(AttrList)), nl->Second(nl->First(AttrList)))),
      last= resAttrlist;
  int n= 1;
  while(n < nl->ListLength(AttrList))
    last= nl->Append(last, nl->Nth(++n, AttrList));
  for(vector<string>::iterator it= aliases.begin(); it!=aliases.end(); ++it)
    last= nl->Append(last,
        nl->TwoElemList(nl->SymbolAtom(*it),
            nl->SymbolAtom(Periods::BasicType())));

  if(extended)
  {
    ListExpr boolExpr= nl->Fourth(args);
    nl->WriteToString(argstr, boolExpr);
    if(!IsBoolMap(boolExpr, true))
    {
      ErrorReporter::ReportError("Operator "+ opName +": expects a boolean "
          "expression. \nBut got '" + argstr + "'.");
      return nl->SymbolAtom(Symbols::TYPEERROR());
    }
  }

  ListExpr result,
  resStream= nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),resAttrlist));
  if(enableTemporalReasoner)
  {
    ListExpr PAReasoner= ComputeClosure(ConstraintList, aliases);
    result = nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
        nl->OneElemList(PAReasoner), resStream);

  }
  else
    result= resStream;

  if(debugme)
  {
    cout<<endl<<endl<<"Operator "+ opName + " accepted the input";
    cout<<endl<< nl->ToString(result);
    cout.flush();
    //return nl->SymbolAtom(Symbols::TYPEERROR());
  }
  return result;
}

template<bool extended>
int STPatternExtendVM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  //bool debugme=false;
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
    CSPAddConstraints(constraintlist);
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
      for( int i = 0; i < tup->GetNoAttributes(); ++i )
        newTuple->CopyAttribute( i, tup, i );

      csp.ResetTuple();
      namedpredlist = args[1].addr;
      CSPSetPredsArgs(namedpredlist, tup);
      bool hasSolution= csp.Solve();
      if(!hasSolution || !extended)
      {
        csp.AppendSolutionToTuple(0, tup, newTuple);
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        return YIELD;
      }
      //IF !extended
      filter= args[3].addr;
      bool Part2=false;
      ArgVectorPointer funargs= qp->Argument(filter);
      ((*funargs)[0]).setAddr(tup);
      while(!Part2 && csp.MoveNext())
      {
        qp->Request(filter, value);
        Part2= ((CcBool*)value.addr)->GetValue();
      }
      if(Part2)
        csp.AppendSolutionToTuple(csp.iterator, tup, newTuple);
      else
        csp.AppendUnDefsToTuple(tup, newTuple);

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
    assert(0);
  }
  return 0;
}

const string STPatternExtendSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList -> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternextend[ namedFunlist;  constraintList ]</text--->"
  "<text>The stpatternextend is the (non-predicate) version of the stpattern "
  " operator. The namedFunList parameter is a comma "
  " separated list in the form alias: time-dependent predicate, ... . The "
  " constraintList is a comma separated list in the form stconstraint(_,_,_),"
  " ... . The stpattern predicate yields true iff the input tuple fulfills "
  " all the time-dependent predicates (namedFunList) in the exact temporal "
  " order specified by (constraintList). Unlike the stpattern predicate, the "
  " stpatternextend extends the input stream with the first supported "
  " assignment. Tuples that doesn't fulfill the pattern are extended with "
  " undef values.</text--->"
  "<text>query Trains feed "
  " stpatternextend["
  "   InSnow: .Trip inside msnow, "
  "   IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "   IsFast: speed(.Trip) > 8.0 "
  " ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))  ] "
  " filter[ isdefined(.IsClose) ] count "
  "</text--->"
  ") )";

const string STPatternExExtendSpec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList X bool-> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))</text--->"
  "<text>_ stpatternexextend[ namedFunlist;  constraintList; bool]</text--->"
  "<text>The operator extends the input stream with the first supported "
  " assignment. Tuples that doesn't fulfill "
  " the pattern are extended with undef values.</text--->"
  "<text>query Trains feed "
  " stpatternexextend["
  "   InSnow: .Trip inside msnow, "
  "   IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "   IsFast: speed(.Trip) > 8.0 "
  " ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))"
  " ; (end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)]] "
  " filter[ isdefined(.IsClose) ] count"
  "</text--->"
  ") )";

Operator stpatternextend (
    "stpatternextend",    //name
    STPatternExtendSpec,     //specification
    STPatternExtendVM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<false, false, false>        //type mapping
);

Operator stpatternexextend (
    "stpatternexextend",    //name
    STPatternExExtendSpec,     //specification
    STPatternExtendVM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<true, false, false>        //type mapping
);

/*
\subsection{The stpatternextendstream and stpatternexextendstream Operators}

The typemap is defined before as ~STPatternExtendTM~

*/
struct STPExtendStreamInfo
{
TupleType *resultTupleType;
Tuple* tup;
PointAlgebraReasoner* paReasoner;
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
    CSPAddConstraints(constraintlist);

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
    CSPAddConstraints(constraintlist);

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

const string STPatternExtendStreamSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList -> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternextendstream[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator extends each tuple in the input stream with all the "
  " supported assignments (i.e. periods that fulfill the pattern). "
  " Tuples that don't fulfill the pattern don't appear in the results."
  "</text--->"
  "<text>query Trains feed "
  " stpatternextendstream["
  "   InSnow: .Trip inside msnow, "
  "   IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "   IsFast: speed(.Trip) > 8.0 "
  " ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))  ] "
  "   filter[isdefined(.IsClose)] count "
  "</text--->"
  ") )";

const string STPatternExExtendStreamSpec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList X bool-> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternexextendstream[ namedFunlist;  constraintList; bool]"
  "</text--->"
  "<text>The operator extends each tuple in the input stream with all the "
  " supported assignemts (i.e., periods that fulfill the pattern). Tuples that"
  " doesn't fulfill the pattern don't appear in the results."
  "</text--->"
  "<text>query Trains feed "
  " stpatternexextendstream["
  "   InSnow: .Trip inside msnow, "
  "   IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "   IsFast: speed(.Trip) > 8.0 "
  " ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))"
  " ; (end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)] ] "
  " filter[ isdefined(.IsClose) ] count "
  "</text--->"
  ") )";

Operator stpatternextendstream (
    "stpatternextendstream",    //name
    STPatternExtendStreamSpec,     //specification
    STPatternExtendStreamVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<false, false, true>        //type mapping
);

Operator stpatternexextendstream (
    "stpatternexextendstream",    //name
    STPatternExExtendStreamSpec,     //specification
    STPatternExExtendStreamVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<true, false, true>        //type mapping
);

/*
\subsection{The stconstraint faked Operator}

This valuemap of this operator is not called by query processor. It is
evaluated inside the value maps of the STP query operators (i.e., inside the
~Solve~ function).

*/
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
  if(!nl->IsAtom(alias1) || nl->SymbolValue(alias1)!= CcString::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator stconstraint: expects a predicate label as first "
        "argument.\n But got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, alias2);
  if(!nl->IsAtom(alias2) || nl->SymbolValue(alias2)!= CcString::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator stconstraint: expects a predicate label as second "
        "argument.\n But got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, temporalconnector);
  if(!nl->IsAtom(temporalconnector) ||
      nl->SymbolValue(temporalconnector) != STVector::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator stconstraint: expects a temporal connector as third "
        "argument.\n But got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  ListExpr result = nl->SymbolAtom(CcBool::BasicType());
  if(debugme)
  {
    cout<<endl<<endl<<"Operator stconstraint accepted the input";
    cout.flush();
  }
  return result;
}

int STConstraintVM
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  assert(0); //this function should never be invoked.
  return 0;
}

const string STConstraintSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>string X string X stvector -> bool</text--->"
  "<text>_ stconstraint( string, string, vec(_))</text--->"
  "<text>This operator is faked. It can be used only within the stpattern and "
  " stpatternex operators. It is used to express a temporal constraint. "
  " The operator doesn't have a value mapping function because it "
  " is evaluated within the stpattern. It should never be called elsewhere."
  "</text--->"
  " <text> "
  "  query Trains feed "
  "   filter[. stpattern["
  "     InSnow: .Trip inside msnow, "
  "     IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "     IsFast: speed(.Trip) > 8.0 "
  "   ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "     stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))  ]] "
  "  count;"
  "</text--->"
  ") )";

Operator stconstraint (
    "stconstraint",    //name
    STConstraintSpec,     //specification
    STConstraintVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STConstraintTM        //type mapping
);

/*
\subsection{The start and end Operators}

*/
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
  if(nl->ListLength(args) != 1 ||
      !nl->IsAtom(nl->First(args)) ||
      nl->SymbolValue(nl->First(args))!= CcString::BasicType())
  {
    ErrorReporter::ReportError("Operator start/end expects a string symbol "
        "but got." + argstr);
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }
  return nl->SymbolAtom(Instant::BasicType());
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
    cout<<endl<<"Executing ";
    if(leftbound) cout<<"start("; else cout<<"end("; cout<< lbl<<")";
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
    cout<<endl<<"Value is "; if(!found) cout<<"Not found";
    res.Print(cout);
    cout.flush();
  }

  result = qp->ResultStorage( s );
  ((Instant*)result.addr)->CopyFrom(&res);
  return 0;
}

const string StartEndSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>string -> instant</text--->"
  "<text>start( _ )/ end(_)</text--->"
  "<text>These are faked operators. They can only be used within an extended "
  " spatiotemporal pattern predicate or operator. They return the start and "
  " end time instants for a predicate in the SA list. These operators should "
  " never be called alone.</text--->"
  " <text>query Trains feed "
  "   filter[. stpatternex["
  "     InSnow: .Trip inside msnow, "
  "     IsClose: distance(.Trip, mehringdamm) < 10.0, "
  "     IsFast: speed(.Trip) > 8.0 "
  "   ; stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),  "
  "     stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))"
  "   ; (end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)]]]"
  "   count</text--->"
  ") )";

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
/*
\subsection{The randommbool, passmbool, and randomdelay Operators}

The randommbool operator is used for experimental evaluation. We use it to 
generate the random mbool values that are used in the first experiment in the
technical report.

*/
ListExpr RandomMBoolTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  if(nl->ListLength(args) != 1 ||
      !nl->IsAtom(nl->First(args)) ||
      nl->SymbolValue(nl->First(args))!= Instant::BasicType())
  {
    ErrorReporter::ReportError("Operator randommbool expects one parameter.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  };
  return nl->SymbolAtom(MBool::BasicType());
}

/*

The passmbool operator is used for experimental evaluation. We use it to 
mimic time-dependent predicates in the first experiment in the technical report.

*/
ListExpr PassMBoolTM(ListExpr args)
{
  //cout<<nl->ToString(args);
  if(nl->ListLength(args) != 1 ||
      !nl->IsAtom(nl->First(args)) ||
      nl->SymbolValue(nl->First(args))!= MBool::BasicType())
  {
    ErrorReporter::ReportError("Operator passmbool expects one parameter.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }
  return nl->SymbolAtom(MBool::BasicType());
}

/*
The randomdelay operator is used to enrich the examples. It adds random 
time delays to the moving point within a given delay threshold   

*/
ListExpr RandomDelayTM( ListExpr typeList )
{
  if(!(nl->ListLength(typeList) == 2 &&
      nl->IsAtom(nl->First(typeList)) &&
      (nl->SymbolValue(nl->First(typeList))== MPoint::BasicType()) &&
      nl->IsAtom(nl->Second(typeList)) &&
      (nl->SymbolValue(nl->Second(typeList))== Duration::BasicType())))
  {
    ErrorReporter::ReportError(
        "randomdelay operator expects (" + MPoint::BasicType() +
        " " + Duration::BasicType() + ") but got "
        + nl->ToString(typeList));
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  return (nl->SymbolAtom(MPoint::BasicType()));
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
  "<text>Mimics a time-dependent predicate. The operator takes the name"
  "of an mbool dbobject and return the object itself. The operator is "
  "used for testing purposes.</text--->"
  "<text>let mb2= passmbool(mb1)</text--->"
  ") )";

const string RandomDelaySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint x duration -> mpoint</text--->"
  "<text>randomdelay(schedule, delay_threshold)</text--->"
  "<text>Given an mpoint and a duration value, the operator randomly shift "
  "the start and end instants of every unit in the mpoint. This gives the "
  "effect of having positive and negative delays in the movement. The "
  "random shift value is bound by the given threshold.</text--->"
  "<text>query randomdelay(train7)</text--->"
  ") )";

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


/*
\section{Temporal Reasoning}
This part was added to the algebra in 2011. The technique is descriped in
Chapter 6 of the PhD thesis of Mahmoud Sakr. Mainly we introduce a copy of each
of the STP operators defined above, where this copy has the same name with an
appended "2" at the end. For example the temporal reasoning version of
stpattern is called stpattern2. The goal of temporal reasoning is to use the
temporal information that the ~Solve~ function collects from the temporal
constraints and the during the evaluation of the CSP, in order to restrict the
trajectories after every iteration. This should improve the run-time of the
operators.

*/

/*
\subsection{The computeclosure Operator}

*/
ListExpr ComputeClosureTM(ListExpr args)
{
  bool debugme= false;
  string opName= "computeclosure";

  string argstr;
  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if( nl->ListLength(args) != 3)
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects " +
        int2string(3 ) + "arguments\nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  ListExpr tupleExpr = nl->First(args),
      NamedPredList  = nl->Second(args),
      ConstraintList = nl->Third(args);

  nl->WriteToString(argstr, tupleExpr);
  if(!IsTupleExpr(tupleExpr, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects "
        "argument a list with structure (tuple ((a1 t1)...(an tn))).\n"
        "But got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, NamedPredList);
  if(!IsMapTuplePeriodsPredicateList(NamedPredList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "aliased predicates, each of which has the syntax "
        "(tuple, periods)->bool. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, ConstraintList);
  if(!IsConstraintList(ConstraintList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "temporal connectors. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  vector<string> predAliases1Unsorted;
  set<string> predAliases1, predAliases2;
  ExtractPredAliasesFromPredList(NamedPredList, predAliases1Unsorted);
  ExtractPredAliasesFromConstraintList(ConstraintList, predAliases2);
  predAliases1.insert(predAliases1Unsorted.begin(), predAliases1Unsorted.end());
  if((predAliases1.size() != predAliases2.size()) ||
   !std::equal(predAliases1.begin(), predAliases1.end(), predAliases2.begin()))
  {
    ErrorReporter::ReportError("Operator "+ opName +": unknown aliases in "
        "temporal constraints.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  ListExpr PAReasoner= ComputeClosure(ConstraintList, predAliases1Unsorted);
  ListExpr result = nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
      nl->OneElemList(PAReasoner), nl->SymbolAtom(CcInt::BasicType()));

  if(debugme)
  {
    cout<<endl<<endl<<"Operator "+ opName +" accepted the input";
    nl->WriteToString(argstr, result);
    cout<<endl<<"Output is:"<<argstr;
    cout.flush();
  }
  return result;
}

int ComputeClosureVM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  Supplier TRTable= args[3].addr;
  Word value;
  PointAlgebraReasoner* paReasoner= 0;
  ClosureResult closureRes= ImportPAReasonerFromArgs(TRTable, paReasoner);
  result = qp->ResultStorage( s );
  ((CcInt*)result.addr)->Set(true, closureRes);
  return 0;
}

const string ComputeClosureSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) ( <text>tuple(x) X periods X namedFunlist X "
  "constraintList -> int</text--->"
  "<text>_  computeclosure[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator is gets the same set of arguments of stpattern2 "
  "Predicate. It constructs the point algebra constraint network for the given "
  "arguments, and tries to compute its closure. The result is: 0 if the network"
  " is inconsistent, 1 if it is consistent, 2 if it is not representable by the"
  "point algebra (i.e., does not belong to the continuous endpoint algebra)."
  "</text--->"
  "<text>query Trains feed  "
  "filter[. computeclosure["
  "insnow: fun(t1: STREAMELEM, p1: periods) "
  "   (attr(t1, Trip) atperiods p1) inside msnow, "
  "isclose: fun(t2: STREAMELEM, p2: periods) "
  "   distance(attr(t2, Trip) atperiods p2, mehringdamm)<10.0,    "
  "isfast: fun(t3: STREAMELEM, p3: periods) "
  "   speed( attr(t3, Trip) atperiods p3) >8.0 ;  "
  "stconstraint(\"insnow\",\"isclose\",vec(\"aabb\")),    "
  "stconstraint(\"isclose\",\"isfast\",vec(\"bbaa\"))] = 1 ] count"
  "</text--->) )";



void RandomShiftDelay( const MPoint* actual, const Instant* threshold,
    double dx, double dy, MPoint& res)
{
  bool debugme= false;

  MPoint delayed(actual->GetNoComponents());
  UPoint first(0), next(0);
  UPoint *shifted,*temp, *cur;
  int xshift=0, yshift=0;
  int64_t rmillisec=0;
  actual->Get( 0, first );
  cur=new UPoint(first);
  for( int i = 1; i < actual->GetNoComponents(); i++ )
  {
    actual->Get( i, next );

    if(threshold->millisecondsToNull() > 0)
      rmillisec= rand()% threshold->GetAllMilliSeconds();
    DateTime delta(durationtype, rmillisec) ;
    xshift= rand() % ((int)dx +1);
    yshift= rand() % ((int)dy +1);
    shifted= new UPoint(*cur);
    delete cur;
    temp= new UPoint(next);
    if(rmillisec > rand()%24000 )
    {
      if((shifted->timeInterval.end + delta) <  next.timeInterval.end )
      {
        shifted->timeInterval.end += delta ;
        temp->timeInterval.start= shifted->timeInterval.end;
        shifted->p1.Set(shifted->p1.GetX()+xshift, shifted->p1.GetY()+yshift);
        temp->p0.Set(shifted->p1.GetX(), shifted->p1.GetY());
      }
    }
    else
    {
      if((shifted->timeInterval.end - delta) >shifted->timeInterval.start)
      {
        shifted->timeInterval.end -= delta ;
        temp->timeInterval.start= shifted->timeInterval.end;
        shifted->p1.Set(shifted->p1.GetX()-xshift, shifted->p1.GetY()-yshift);
        temp->p0.Set(shifted->p1.GetX(), shifted->p1.GetY());
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
Value mapping function for the operator ~ndefunit~

*/
int NDefUnitVM( ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  bool debugme=false;
  MBool *arg = static_cast<MBool*>( args[0].addr );
  CcBool *ndefval = static_cast<CcBool*>( args[1].addr );

  MBool* tmp=new MBool(0);
  result= qp->ResultStorage(s);
  MBool* res= (MBool*) result.addr;
  NDefUnit(arg, ndefval, res);
  //res->CopyFrom(tmp);
  tmp->Destroy();
  delete tmp;
  if(debugme)
  {
    cout.flush();
    res->Print(cout);
    cout.flush();
  }
  return 0;
}



ListExpr RandomShiftDelayTM( ListExpr typeList )
{
  CHECK_COND(nl->ListLength(typeList) == 4 &&
      nl->IsAtom(nl->First(typeList)) &&
      (nl->SymbolValue(nl->First(typeList))== "mpoint") &&
      nl->IsAtom(nl->Second(typeList)) &&
      (nl->SymbolValue(nl->Second(typeList))== "duration")&&
      nl->IsAtom(nl->Third(typeList)) &&
      (nl->SymbolValue(nl->Third(typeList))== "real")&&
      nl->IsAtom(nl->Fourth(typeList)) &&
      (nl->SymbolValue(nl->Fourth(typeList))== "real"),
      "randomshiftdelay operator expects (mpoint duration real real) but got "
      + nl->ToString(typeList))

      return (nl->SymbolAtom("mpoint"));
}

int RandomShiftDelayVM(ArgVector args, Word& result,
    int msg, Word& local, Supplier s )
{
  MPoint *pActual = static_cast<MPoint*>( args[0].addr );
  Instant *threshold = static_cast<Instant*>(args[1].addr );
  double dx = static_cast<CcReal*>(args[2].addr )->GetRealval();
  double dy = static_cast<CcReal*>(args[3].addr )->GetRealval();


  MPoint* shifted = (MPoint*) qp->ResultStorage(s).addr;

  if(pActual->GetNoComponents()<2 || !pActual->IsDefined())
    shifted->CopyFrom(pActual);
    else
    {
      RandomShiftDelay(pActual, threshold, dx, dy, *shifted);
    }
  result= SetWord(shifted);
  //This looks redundant but it is really necessary. After 2 hours of
  //debugging, it seems that the "result" word is not correctly set
  //by the query processor to point to the results.

  return 0;
}


const string RandomShiftDelaySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>mpoint x duration x real x real-> mpoint</text--->"
  "<text>randomdelay(schedule, delay_threshold, dx, dy)</text--->"
  "<text>Given an mpoint and a duration value, the operator randomly shift the"
  "start and end intstants of every unit in the mpoint. This gives the "
  "effect of having positive and negative delays and spatial shifts in the "
  "movement. The random shift values are bound by the given threshold."
  "</text---> <text>query randomdelay(train7)</text--->"
  ") )";

Operator computeClosure (
    "computeclosure",    //name
    ComputeClosureSpec,     //specification
    ComputeClosureVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    ComputeClosureTM        //type mapping
);

Operator randomshiftdelay (
    "randomshiftdelay",               // name
    RandomShiftDelaySpec,             // specification
    RandomShiftDelayVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    RandomShiftDelayTM          // type mapping

/*
\subsection{The stpattern2 and stpatternex2 Operators}

*/
template<bool extended>
ListExpr STPattern2TM(ListExpr args)
{
  bool debugme= false;
  string opName= (extended)? "stpatternex2": "stpattern2";

  string argstr;
  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if((!extended && nl->ListLength(args) != 3) ||
      (extended && nl->ListLength(args) != 4))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects " +
        int2string(3 + extended) + "arguments\nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  ListExpr tupleExpr = nl->First(args),
      NamedPredList  = nl->Second(args),
      ConstraintList = nl->Third(args);

  nl->WriteToString(argstr, tupleExpr);
  if(!IsTupleExpr(tupleExpr, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects "
        "argument a list with structure (tuple ((a1 t1)...(an tn))).\n"
        "But got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, NamedPredList);
  if(!IsMapTuplePeriodsPredicateList(NamedPredList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "aliased predicates, each of which has the syntax "
        "(tuple, periods)->bool. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  nl->WriteToString(argstr, ConstraintList);
  if(!IsConstraintList(ConstraintList, true))
  {
    ErrorReporter::ReportError("Operator "+ opName +": expects a list of "
        "temporal connectors. \nBut got '" + argstr + "'.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  vector<string> predAliases1Unsorted;
  set<string> predAliases1, predAliases2;
  ExtractPredAliasesFromPredList(NamedPredList, predAliases1Unsorted);
  ExtractPredAliasesFromConstraintList(ConstraintList, predAliases2);
  predAliases1.insert(predAliases1Unsorted.begin(), predAliases1Unsorted.end());
  if((predAliases1.size() != predAliases2.size()) ||
   !std::equal(predAliases1.begin(), predAliases1.end(), predAliases2.begin()))
  {
    ErrorReporter::ReportError("Operator "+ opName +": unknown aliases in "
        "temporal constraints.");
    return nl->SymbolAtom(Symbols::TYPEERROR());
  }

  if(extended)
  {
    ListExpr boolExpr= nl->Fourth(args);
    nl->WriteToString(argstr, boolExpr);
    if(!IsBoolExpr(boolExpr, true))
    {
      ErrorReporter::ReportError("Operator "+ opName +": expects a boolean "
          "expression. \nBut got '" + argstr + "'.");
      return nl->SymbolAtom(Symbols::TYPEERROR());
    }
  }

  ListExpr PAReasoner= ComputeClosure(ConstraintList, predAliases1Unsorted);
  ListExpr result = nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
      nl->OneElemList(PAReasoner), nl->SymbolAtom(CcBool::BasicType()));

  if(debugme)
  {
    cout<<endl<<endl<<"Operator "+ opName +" accepted the input";
    nl->WriteToString(argstr, result);
    cout<<endl<<"Output is:"<<argstr;
    cout.flush();
  }
  return result;
}

template<bool extended>
int STPattern2VM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  if(debugme)
  {
    Word w;
    qp->Request(args[0].addr, w);
    Tuple* tuple= static_cast<Tuple*>(w.addr);
    cerr<<endl<<"Id: "<<
        static_cast<CcInt*>(tuple->GetAttribute(0))->GetValue();
    cerr<<", Line: "<<
        static_cast<CcInt*>(tuple->GetAttribute(1))->GetValue();
  }
  Supplier namedpredlist, constraintlist, filter, TRTable;
  result = qp->ResultStorage( s );
  Word w;
  qp->Request(args[0].addr, w);
  Tuple* tup= static_cast<Tuple*>(w.addr);
  namedpredlist = args[1].addr;
  constraintlist= args[2].addr;
  if(extended)
  {
    filter= args[3].addr;
    TRTable= args[4].addr;
  }
  else
    TRTable= args[3].addr;
  Word value;

  PointAlgebraReasoner* paReasoner= 0;
  ClosureResult closureRes= ImportPAReasonerFromArgs(TRTable, paReasoner);
  csp.Clear();
  csp.SetClosureResult(closureRes);
  if(closureRes == inconsistent)
  {
    ((CcBool*)result.addr)->Set(true, false);
    return 0;
  }

  CSPAddPredicates(namedpredlist);
  CSPAddConstraints(constraintlist);

  bool hasSolution= false;
  Periods* periods= CreateMaximalPeriods();
  CSPSetPredsArgs(namedpredlist, tup, periods);
  hasSolution=csp.Solve(periods, paReasoner);
  delete periods;
  delete paReasoner;

  ((CcBool*)result.addr)->Set(true, hasSolution);
  if(!hasSolution || !extended)
    return 0;


  bool Part2=false;
  while(!Part2 && csp.MoveNext())
  {
    qp->Request(filter, value);
    Part2= ((CcBool*)value.addr)->GetValue();
  }
  ((CcBool*)result.addr)->Set(true,Part2);
  return 0;
}

const string STPattern2Spec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) ( <text>tuple(x) X namedFunlist X "
  "constraintList -> bool</text--->"
  "<text>_  stpattern2[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator is an optimized version of the stpattern Predicate "
  "(Please refer to the stpattern operator). It reasons over the temporal "
  "constraints and allows the user to restrict the trajectories based on the "
  "reasoning results. The goal is to reduce the number of trajectory units "
  "processed by the time-dependent predicates, and thus reduce the run-time. "
  "For more information, please refer to the PhD thesis of Mahmoud Sakr "
  "'Spatiotemporal Pattern Queries'|'Chapter 6 Temporal Reasoning'."
  "</text--->"
  "<text>query Trains feed  "
  " filter[. stpattern2["
  "   InSnow: fun(t1: STREAMELEM, p1: periods) "
  "     (attr(t1, Trip) atperiods p1) inside msnow, "
  "   IsCclose: fun(t2: STREAMELEM, p2: periods) "
  "     distance(attr(t2, Trip) atperiods p2, mehringdamm) < 10.0,    "
  "   IsFast: fun(t3: STREAMELEM, p3: periods) "
  "     speed( attr(t3, Trip) atperiods p3) > 8.0 "
  "  ;stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),    "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\")) "
  "  ]] "
  " count </text--->"
  ") )";

const string STPatternEx2Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ stpatternex2[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator is an optimized version of the stpatternex Predicate. "
  "   It reasons over the temporal constraints and allows the user to restrict "
  "   the trajectories based on the reasoning results. The goal is to reduce "
  "   the number of trajectory units processed by the time-dependent "
  "   predicates, and thus reduce the run-time. For more information, please "
  "   refer to the PhD thesis of Mahmoud Sakr 'Spatiotemporal Pattern "
  "   Queries'|'Chapter 6 Temporal Reasoning'. "
  "</text--->"
  "<text>query Trains feed  "
  " filter[. stpatternex2["
  "   InSnow: fun(t1: STREAMELEM, p1: periods) "
  "     (attr(t1, Trip) atperiods p1) inside msnow, "
  "   IsClose: fun(t2: STREAMELEM, p2: periods) "
  "     distance(attr(t2, Trip) atperiods p2, mehringdamm) < 10.0, "
  "   IsFast: fun(t3: STREAMELEM, p3: periods) "
  "     speed( attr(t3, Trip) atperiods p3) > 8.0 "
  "  ;stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),    "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\")) "
  "  ;(end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)] ]]"
  " count </text--->"
  ") )";

Operator stpattern2 (
    "stpattern2",    //name
    STPattern2Spec,     //specification
    STPattern2VM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPattern2TM<false>        //type mapping
);

Operator stpatternex2 (
    "stpatternex2",    //name
    STPatternEx2Spec,     //specification
    STPattern2VM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPattern2TM<true>        //type mapping
);

/*
\subsection{The stpatternextend2 and stpatternexextend2 Operators}

The typemap is defined before as ~STPatternExtendTM~

*/
struct STPatternExtendLocalInfo
{
public:
STPatternExtendLocalInfo(): resultTupleType(0), paReasoner(0){}
TupleType* resultTupleType;
PointAlgebraReasoner* paReasoner;
};

template<bool extended>
int STPatternExtend2VM(
    Word* args, Word& result, int message, Word& local, Supplier s)
{
  //bool debugme=false;

  Word t, value;
  Tuple* tup;
  STPatternExtendLocalInfo *localInfo;
  ListExpr resultType;

  switch (message)
  {
  case OPEN :
  {
    Supplier stream, namedpredlist, constraintlist, filter, TRTable;
    stream = args[0].addr;
    namedpredlist = args[1].addr;
    constraintlist= args[2].addr;
    if(extended)
    {
      filter= args[3].addr;
      TRTable= args[4].addr;
    }
    else
      TRTable= args[3].addr;
    Word value;

    localInfo= new STPatternExtendLocalInfo();
    resultType = GetTupleResultType( s );
    localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );

    ClosureResult closureRes=
        ImportPAReasonerFromArgs(TRTable, localInfo->paReasoner);
    csp.Clear();
    csp.SetClosureResult(closureRes);
    qp->Open(stream);
    CSPAddPredicates(namedpredlist);
    CSPAddConstraints(constraintlist);
    local.setAddr(localInfo);
    return 0;
  }break;

  case REQUEST :
  {
    if(csp.GetClosureResult() == inconsistent) return CANCEL;
    Periods* periods= CreateMaximalPeriods();
    Supplier stream, namedpredlist, filter;
    localInfo= static_cast<STPatternExtendLocalInfo*>(local.addr);
    stream= args[0].addr;
    qp->Request(stream ,t);
    if (qp->Received(stream))
    {
      tup = (Tuple*)t.addr;
      Tuple *newTuple = new Tuple( localInfo->resultTupleType );
      for( int i = 0; i < tup->GetNoAttributes(); i++ )
        newTuple->CopyAttribute( i, tup, i );
      csp.ResetTuple();
      namedpredlist = args[1].addr;
      CSPSetPredsArgs(namedpredlist, tup, periods);
      bool hasSolution= csp.Solve(periods, localInfo->paReasoner);
      if(!hasSolution || !extended)
      {
        csp.AppendSolutionToTuple(0, tup, newTuple);
        tup->DeleteIfAllowed();
        result.setAddr(newTuple);
        delete periods;
        return YIELD;
      }
      //IF extended
      bool Part2=false;
      filter= args[3].addr;
      ArgVectorPointer funargs= qp->Argument(filter);
      ((*funargs)[0]).setAddr(tup);
      while(!Part2 && csp.MoveNext())
      {
        qp->Request(filter, value);
        Part2= ((CcBool*)value.addr)->GetValue();
      }
      if(Part2)
        csp.AppendSolutionToTuple(csp.iterator, tup, newTuple);
      else
        csp.AppendUnDefsToTuple(tup, newTuple);
      tup->DeleteIfAllowed();
      result.setAddr(newTuple);
      delete periods;
      return YIELD;
    }
    else
    {
      delete periods;
      return CANCEL;
    }
  }break;
  case CLOSE :
  {
    if(local.addr != 0)
    {
      localInfo= static_cast<STPatternExtendLocalInfo*>(local.addr);
      if(localInfo->resultTupleType != 0)
        localInfo->resultTupleType->DeleteIfAllowed();
      if(localInfo->paReasoner != 0)
        delete localInfo->paReasoner;
      delete localInfo;
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

const string STPatternExtend2Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList -> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternextend2[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator extends the input stream with the first supported "
  " assignment. Tuples that doesn't fulfill the pattern are extended with "
  " undef values. It is an optimized version of the stpatternextend operator."
  " It reasons over the temporal constraints and allows the user to "
  " restrict the trajectories based on the reasoning results.</text--->"
  "<text>query Trains feed  "
  " stpatternextend2["
  "   InSnow: fun(t1: TUPLE, p1: periods) "
  "     (attr(t1, Trip) atperiods p1) inside msnow, "
  "   IsClose: fun(t2: TUPLE, p2: periods) "
  "     distance(attr(t2, Trip) atperiods p2, mehringdamm) < 10.0,    "
  "   IsFast: fun(t3: TUPLE, p3: periods) "
  "     speed( attr(t3, Trip) atperiods p3) > 8.0 "
  "  ;stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),    "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))  ] "
  " filter[isdefined(.IsClose)] count"
  "</text--->"
  ") )";

const string STPatternExExtend2Spec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList X bool-> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternexextend2[ namedFunlist;  constraintList; bool]</text--->"
  "<text>The operator extends the input stream with the first supported "
  "  assignment. Tuples that doesn't fulfill the pattern are extended with "
  "  undef values. This operator is an optimized version of the "
  "  stpatternexextend operator. It reasons over the temporal constraints and "
  "  allows the user to restrict the trajectories based on the reasoning "
  "  results."
  "</text--->"
  "<text>query Trains feed  "
  " stpatternexextend2["
  "   InSnow: fun(t1: TUPLE, p1: periods) "
  "     (attr(t1, Trip) atperiods p1) inside msnow, "
  "   IsClose: fun(t2: TUPLE, p2: periods) "
  "     distance(attr(t2, Trip) atperiods p2, mehringdamm) < 10.0,"
  "   IsFast: fun(t3: TUPLE, p3: periods) "
  "     speed( attr(t3, Trip) atperiods p3) > 8.0 "
  "  ;stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")), "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\")) "
  "  ;(end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)] ] "
  " filter[ isdefined(.IsClose) ] count "
  " </text--->"
  ") )";

Operator stpatternextend2 (
    "stpatternextend2",    //name
    STPatternExtend2Spec,     //specification
    STPatternExtend2VM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<false, true, false>        //type mapping
);

Operator stpatternexextend2 (
    "stpatternexextend2",    //name
    STPatternExExtend2Spec,     //specification
    STPatternExtend2VM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<true, true, false>        //type mapping
);

/*
\subsection{The stpatternextendstream2 and stpatternexextendstream2 Operators}

The typemap is defined before as ~STPatternExtendTM~

*/
int STPatternExtendStream2VM(
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
    Supplier stream, namedpredlist, constraintlist, TRTable;

    stream = args[0].addr;
    namedpredlist = args[1].addr;
    constraintlist= args[2].addr;
    TRTable= args[3].addr;

    localInfo= new STPExtendStreamInfo();
    resultType = GetTupleResultType( s );
    localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );
    localInfo->tup=0;
    ClosureResult closureRes=
        ImportPAReasonerFromArgs(TRTable, localInfo->paReasoner);
    csp.Clear();
    csp.SetClosureResult(closureRes);
    qp->Open(stream);
    CSPAddPredicates(namedpredlist);
    CSPAddConstraints(constraintlist);
    local.setAddr( localInfo );
    return 0;
  }break;

  case REQUEST :
  {
    if(csp.GetClosureResult() == inconsistent) return CANCEL;
    Periods* periods= CreateMaximalPeriods();
    Supplier stream, namedpredlist;
    localInfo= static_cast<STPExtendStreamInfo*>(local.addr);
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
      {
        delete periods;
        return CANCEL;
      }
      csp.ResetTuple();
      namedpredlist = args[1].addr;
      CSPSetPredsArgs(namedpredlist, localInfo->tup, periods);
      if(csp.Solve(periods, localInfo->paReasoner))
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
    delete periods;
    return YIELD;
  }break;

  case CLOSE :
  {
    if(local.addr != 0)
    {
      localInfo= static_cast<STPExtendStreamInfo*>(local.addr);
      if(localInfo->resultTupleType != 0)
        localInfo->resultTupleType->DeleteIfAllowed();
      if(localInfo->paReasoner != 0)
        delete localInfo->paReasoner;
      delete localInfo;
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

int STPatternExExtendStream2VM(
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
    Supplier stream, namedpredlist, constraintlist, TRTable;

    stream = args[0].addr;
    namedpredlist = args[1].addr;
    constraintlist= args[2].addr;
    TRTable= args[4].addr;

    localInfo= new STPExtendStreamInfo();
    resultType = GetTupleResultType( s );
    localInfo->resultTupleType = new TupleType( nl->Second( resultType ) );
    localInfo->tup= 0;
    ClosureResult closureRes=
        ImportPAReasonerFromArgs(TRTable, localInfo->paReasoner);
    csp.Clear();
    csp.SetClosureResult(closureRes);
    qp->Open(stream);
    CSPAddPredicates(namedpredlist);
    CSPAddConstraints(constraintlist);
    local.setAddr(localInfo );
    return 0;
  }break;

  case REQUEST :
  {
    if(csp.GetClosureResult() == inconsistent) return CANCEL;
    Periods* periods= CreateMaximalPeriods();
    Supplier stream, namedpredlist, filter;
    localInfo= static_cast<STPExtendStreamInfo*>(local.addr);
    filter= args[3].addr;
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
        {
          delete periods;
          return CANCEL;
        }
        csp.ResetTuple();
        namedpredlist = args[1].addr;
        CSPSetPredsArgs(namedpredlist, localInfo->tup, periods);
        if(csp.Solve(periods, localInfo->paReasoner))
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
        delete periods;
        return YIELD;
      }
    }
  }break;
  case CLOSE :
  {
    if(local.addr != 0)
    {
      localInfo= static_cast<STPExtendStreamInfo*>(local.addr);
      if(localInfo->resultTupleType != 0)
        localInfo->resultTupleType->DeleteIfAllowed();
      if(localInfo->paReasoner != 0)
        delete localInfo->paReasoner;
      delete localInfo;
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

const string STPatternExtendStream2Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList ->"
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternextendstream2[ namedFunlist;  constraintList ]</text--->"
  "<text>The operator extends each tuple in the input stream with all the "
  " supported assignments (i.e. periods that fulfill the pattern). "
  " Tuples that doesn't fulfill the pattern don't appear in the results. "
  " This operator is an optimized version of the stpatternextendstream "
  " operator. It reasons over "
  " the temporal constraints and allows the user to restrict the trajectories"
  " based on the reasoning results.</text--->"
  "<text>query Trains feed  "
  " stpatternextendstream2["
  "   InSnow: fun(t1: TUPLE, p1: periods) "
  "     (attr(t1, Trip) atperiods p1) inside msnow, "
  "   IsClose: fun(t2: TUPLE, p2: periods) "
  "     distance(attr(t2, Trip) atperiods p2, mehringdamm) < 10.0, "
  "   IsFast: fun(t3: TUPLE, p3: periods) "
  "     speed( attr(t3, Trip) atperiods p3) > 8.0 "
  "  ;stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),    "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\"))  ] "
  "   filter[ isdefined(.IsClose) ] count "
  "</text--->"
  ") )";

const string STPatternExExtendStream2Spec =
  "( (\"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>stream(tuple(x)) X namedFunlist X constraintList X bool-> "
  "   stream(tuple(x,alias1:Periods,...,alias1:Periods))"
  "</text--->"
  "<text>_ stpatternexextendstream2[ namedFunlist;  constraintList;  bool]"
  "</text--->"
  "<text>The operator extends each tuple in the input stream with all the "
  " supported assignments (i.e. periods that fulfill the pattern). Tuples that "
  " doesn't fulfill the pattern don't appear in the results. This operator is "
  " an optimized version of the stpatternexextendstream operator. It reasons "
  " over the temporal constraints and allows the user to restrict the "
  " trajectories based on the reasoning results.</text--->"
  "<text>query Trains feed "
  " stpatternexextendstream2["
  "   InSnow: fun(t1: TUPLE, p1: periods) "
  "     (attr(t1, Trip) atperiods p1) inside msnow, "
  "   IsClose: fun(t2: TUPLE, p2: periods) "
  "     distance(attr(t2, Trip) atperiods p2, mehringdamm) < 10.0, "
  "   IsFast: fun(t3: TUPLE, p3: periods) "
  "     speed( attr(t3, Trip) atperiods p3) > 8.0 "
  "  ;stconstraint(\"InSnow\",\"IsClose\",vec(\"aabb\")),    "
  "   stconstraint(\"IsClose\",\"IsFast\",vec(\"bbaa\")) "
  "  ;(end(\"IsClose\") - start(\"InSnow\")) < [const duration value (1 0)]  ]"
  " filter[ isdefined(.IsClose) ] count "
  "</text--->"
  ") )";

Operator stpatternextendstream2 (
    "stpatternextendstream2",    //name
    STPatternExtendStream2Spec,     //specification
    STPatternExtendStream2VM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<false, true, true>        //type mapping
);

Operator stpatternexextendstream2 (
    "stpatternexextendstream2",    //name
    STPatternExExtendStream2Spec,     //specification
    STPatternExExtendStream2VM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    STPatternExtendTM<true, true, true>        //type mapping
);

/*
\subsection{Auxiliary temporal reasoning functions}

*/

/*
~STVector::Vector2PARelations~\\
The Vector2PARelations function converts the IA relation represented in the
vector into a set of PA relations among the end points of the two intervals.
The function returns false if the conversion is not possible (i.e., the IA
relation does not belong to the continuous point algebra. That is, it cannot be
represented as point relations unless the != relation is used).

The output PA relations are reported in the rels argument. It has 10 places for
the relations aA ab aB ba bA bB Ab AB Ba BA Aa Bb in order. Each array elem has
the value 0 (relation not specified), or 1-7 (<, =, >, <=, >=, !=, ?). As
indicated above, if an elem in rels has the value 6 (!=), the function yields
false.

*/

bool STVector::Vector2PARelations(int rels[12])
{
  bool debugme= false;
  enum relIndex{
  aA=0, ab=1, aB=2, ba=3, bA=4, bB=5, Ab=6, AB=7, Ba=8, BA=9, Aa=10, Bb=11};
/*
defined in TemporalReasoner.h
enum PARelation{
lss=0, leq=1, grt=2, geq=3, eql=4, neq=5, uni=6, inc=7, unknown=8};

*/
  PARelation orTable[][9]=
  {{lss,leq,neq,uni,leq,neq,uni,inc,lss},
      {leq,leq,uni,uni,leq,uni,uni,inc,leq},
      {neq,uni,grt,geq,geq,neq,uni,inc,grt},
      {uni,uni,geq,geq,geq,uni,uni,inc,geq},
      {leq,leq,geq,geq,eql,uni,uni,inc,eql},
      {neq,uni,neq,uni,uni,neq,uni,inc,neq},
      {uni,uni,uni,uni,uni,uni,uni,inc,uni},
      {inc,inc,inc,inc,inc,inc,inc,inc,inc},
      {lss,leq,grt,geq,eql,neq,uni,inc,unknown}
  };

  int simple=1;
  PARelation relsSimple[12], relsVector[12], relSimple, relVector, relNew;

  for(int i= 0; i<12; ++i)
    relsVector[i]= unknown;

  for(int i=0; i<26; i++)
  {
    if(v & simple)
    {
      if(! Simple2PARelations(simple, (int*)relsSimple)) return false;
      for(int i= 0; i<12; ++i)
      {
        relSimple= relsSimple[i];
        relVector= relsVector[i];
        relNew= orTable[relSimple][relVector];
        relsVector[i]= relNew;
      }
    }
    simple*=2;
  }

  for(int i= 0; i<12; ++i)
  {
    if(relsVector[i]== neq) return false;
    rels[i]= relsVector[i];
    if(debugme)
      cerr<<endl<<i<< ": "<< relsVector[i];
  }
  return true;
}

/*
~STVector::Simple2PARelations~\\
The Simple2PARelations function converts an IA simple relation (i.e., one term)
into a set of PA relations. It works similar to Vectore2PARelations, yet it is
able to convert one term only.

*/
bool STVector::Simple2PARelations(int simple, int rels[12])
{
  enum relIndex{
  aA=0, ab=1, aB=2, ba=3, bA=4, bB=5, Ab=6, AB=7, Ba=8, BA=9,  Aa=10, Bb=11};
  //defined in TemporalReasoner.h
  //enum PARelation{lss=0, leq=1, grt=2, geq=3, eql=4, neq=5, uni=6, inc=7};

  for(int i= 0; i<12; ++i)
    rels[i]= unknown;

  switch(simple)
  {
  case   aabb:
  {
    rels[aA]= lss; rels[ab]= lss; rels[aB]= lss;
    rels[ba]= grt; rels[bA]= grt; rels[bB]= lss;
    rels[Ab]= lss; rels[AB]= lss; rels[Ba]= grt;
    rels[BA]= grt; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   bbaa:
  {
    rels[aA]= lss; rels[ab]= grt; rels[aB]= grt;
    rels[ba]= lss; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= grt; rels[Ba]= lss;
    rels[BA]= lss; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   aa_bb:
  {
    rels[aA]= lss; rels[ab]= lss; rels[aB]= lss;
    rels[ba]= grt; rels[bA]= eql; rels[bB]= lss;
    rels[Ab]= eql; rels[AB]= lss; rels[Ba]= grt;
    rels[BA]= grt; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   bb_aa:
  {
    rels[aA]= lss; rels[ab]= grt; rels[aB]= eql;
    rels[ba]= lss; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= grt; rels[Ba]= eql;
    rels[BA]= lss; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   abab:
  {
    rels[aA]= lss; rels[ab]= lss; rels[aB]= lss;
    rels[ba]= grt; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= lss; rels[Ba]= grt;
    rels[BA]= grt; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   baba:
  {
    rels[aA]= lss; rels[ab]= grt; rels[aB]= lss;
    rels[ba]= lss; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= grt; rels[Ba]= grt;
    rels[BA]= lss; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   baab:
  {
    rels[aA]= lss; rels[ab]= grt; rels[aB]= lss;
    rels[ba]= lss; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= lss; rels[Ba]= grt;
    rels[BA]= grt; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   abba:
  {
    rels[aA]= lss; rels[ab]= lss; rels[aB]= lss;
    rels[ba]= grt; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= grt; rels[Ba]= grt;
    rels[BA]= lss; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   a_bab:
  {
    rels[aA]= lss; rels[ab]= eql; rels[aB]= lss;
    rels[ba]= eql; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= lss; rels[Ba]= grt;
    rels[BA]= grt; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   a_bba:
  {
    rels[aA]= lss; rels[ab]= eql; rels[aB]= lss;
    rels[ba]= eql; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= grt; rels[Ba]= grt;
    rels[BA]= lss; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   baa_b:
  {
    rels[aA]= lss; rels[ab]= grt; rels[aB]= lss;
    rels[ba]= lss; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= eql; rels[Ba]= grt;
    rels[BA]= eql; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   aba_b:
  {
    rels[aA]= lss; rels[ab]= lss; rels[aB]= lss;
    rels[ba]= grt; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= eql; rels[Ba]= grt;
    rels[BA]= eql; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   a_ba_b:
  {
    rels[aA]= lss; rels[ab]= eql; rels[aB]= lss;
    rels[ba]= eql; rels[bA]= lss; rels[bB]= lss;
    rels[Ab]= grt; rels[AB]= eql; rels[Ba]= grt;
    rels[BA]= eql; rels[Bb]= grt; rels[Aa]= grt;
  }break;
  case   a_abb:
  case   a_a_bb:
  case   ba_ab:
  case   bb_a_a:
  case   bba_a:
  case   b_baa:
  case   b_b_aa:
  case   ab_ba:
  case   aa_b_b:
  case   aab_b:
  case   a_ab_b:
  case   a_a_b_b:
  case   b_ba_a:
    return false;
    break;
  default:
    assert(0); //illegal simple temporal connector
  }
  return true;
}

/*
~CSP::SetConsistentPeriods~\\
The SetConsistentPeriods is a helper function, used by the CSP::Solve as a part
of the temporal reasoning.

*/
void CSP::SetConsistentPeriods(
    int varIndex, Periods* periodsArg, PointAlgebraReasoner* paReasoner)
{
  Periods* consistentPeriods= periodsArg;
  consistentPeriods->Clear();

  Instant tMIN(instanttype), tMAX(instanttype);
  tMIN.ToMinimum(); tMAX.ToMaximum();

  unsigned int SASize= SA.size();
  if(SASize == 0)
  {
    Interval<Instant> I(tMIN, tMAX, false, false);
    consistentPeriods->Add(I);
    return;
  }

  vector<Interval<Instant> > sa(count);
  vector<PARelation> PARels;
  vector<int> PAVarIndexes(0), lowerBoundIndexes(0), upperBoundIndexes(0);
  vector<Instant> lowerBounds(0), upperBounds(0);
  DateTime epsilon(0,1010, durationtype);
  int curPAVarIndex, IAVar;
  Instant lowerBound, upperBound;
  Interval<Instant> bounds(tMIN, tMAX, false, false);
  Range<Instant> unionp(0);

  for(vector<int>::iterator it= assignedVars.begin();
      it!=assignedVars.end(); ++it)
  {
    IAVar= *it;
    if(IAVar == varIndex) continue;
    PAVarIndexes.push_back(IAVar * 2);
    PAVarIndexes.push_back((IAVar * 2) + 1 );
  }

  int varStartIndex= varIndex*2, varEndIndex= (varIndex*2) +1;
  PARels= paReasoner->GetRelations(varStartIndex);
  for(unsigned int j= 0; j<PAVarIndexes.size(); ++j)
  {
    if(PARels[PAVarIndexes[j]] == grt || PARels[PAVarIndexes[j]] == geq ||
        PARels[PAVarIndexes[j]] == eql )
      lowerBoundIndexes.push_back(PAVarIndexes[j]);
  }

  PARels= paReasoner->GetRelations(varEndIndex);
  for(unsigned int j= 0; j<PAVarIndexes.size(); ++j)
  {
    if(PARels[PAVarIndexes[j]] == lss || PARels[PAVarIndexes[j]] == leq ||
        PARels[PAVarIndexes[j]] == eql )
      upperBoundIndexes.push_back(PAVarIndexes[j]);
  }

  if(lowerBoundIndexes.empty() && upperBoundIndexes.empty())
  {
    Interval<Instant> I(tMIN, tMAX, false, false);
    consistentPeriods->Add(I);
    return;
  }

  for(unsigned int i=0; i<SASize; i++)
  {
    sa= SA[i];
    bounds.start.ToMinimum(); bounds.end.ToMaximum();
    if(!lowerBoundIndexes.empty())
    {
      //find the maximum lower bound
      for(unsigned int j=0; j<lowerBoundIndexes.size(); ++j)
      {
        curPAVarIndex = lowerBoundIndexes[j];
        lowerBound= (curPAVarIndex % 2)?
            sa[(curPAVarIndex / 2)].end:
            sa[(curPAVarIndex / 2)].start;
        if(bounds.start < lowerBound)  bounds.start.CopyFrom(&lowerBound);
      }
    }


    if(!upperBoundIndexes.empty())
    {
      //find the minimum upper bound
      for(unsigned int j=0; j<upperBoundIndexes.size(); ++j)
      {
        curPAVarIndex = upperBoundIndexes[j];
        upperBound= (curPAVarIndex % 2)?
            sa[(curPAVarIndex / 2)].end:
            sa[(curPAVarIndex / 2)].start;
        if(bounds.end > upperBound)  bounds.end.CopyFrom(&upperBound);
      }
    }

    //expand the bounds with epslon
    if(!bounds.start.IsMinimum())   bounds.start-= epsilon;
    if(!bounds.end.IsMaximum())     bounds.end+= epsilon;

    //union the bounds with the consistentPeriods
    if(bounds.IsValid())
    {
      consistentPeriods->Union(bounds , unionp);
      consistentPeriods->CopyFrom(&unionp);
    }
  }
}

/*
~CSP::Solve~\\
The second variant of the CSP::Solve function. This variant performs temporal
reasoning to compute the supported assignments efficiently.

*/
bool CSP::Solve(Periods* periodsArg, PointAlgebraReasoner* paReasoner)
{
  bool debugme=false;
  int varIndex;
  Word Value;
  vector<Interval<Instant> > domain(0);
  int loopCnt=0;
  string counterPrefix= "CSP::SolveTR::Iteration";
  Counter::getRef("TemporalReasoningOn", false)= 1;
  if(debugme)
    cerr<<"TR Iterations ";
  while( (varIndex= PickVariable()) != -1)
  {
    ++loopCnt;
    if(debugme)
      cerr<<loopCnt<<", ";
    Counter::getRef(counterPrefix + int2Str(loopCnt), true)++;
    if(GetClosureResult() == consistent )
      SetConsistentPeriods(varIndex, periodsArg, paReasoner);
    if(debugme && 0)
    {
      if(!IsMaximumPeriods(*periodsArg))
      {
        periodsArg->Print(cerr);
        for(int i= 0; i<periodsArg->GetNoComponents(); ++i)
        {
          Interval<Instant> intr;
          periodsArg->Get(i, intr);
          cerr<<endl<<intr.start.millisecondsToNull()<< "\t"<<
              intr.end.millisecondsToNull();
        }
      }
    }
    qp->Request(Agenda[varIndex], Value);
    UsedAgendaVars[varIndex]= true;
    if(debugme && 0)
      ((MBool*)Value.addr)->Print(cerr);
    MBool2Vec((MBool*)Value.addr, domain);
    if(debugme && 0)
    {
      for(unsigned int i=0; i<domain.size(); ++i)
      {
        cerr<<endl;
        domain[i].Print(cerr);
      }
    }
    if(domain.size()==0) {SA.clear(); return false;}
    if(Extend(varIndex, domain)!= 0) return false;
    if(SA.size()==0) return false;
    if(debugme && 0)
      Print();
  }
  return true;
}

/*
~CSP::SetClosureResult~\\

*/
void CSP::SetClosureResult(ClosureResult _res)
{
  closureRes= _res;
}

/*
~SP::GetClosureResult~\\

*/
ClosureResult CSP::GetClosureResult()
{
  return closureRes;
}

/*
\section{The Algebra Definition}

*/
class STPatternAlgebra : public Algebra
{
public:
STPatternAlgebra() : Algebra()
{

  AddTypeConstructor( &stvectorTC );

/*
~SetRequestsArguments~ annotates the operator as being lazy, in the sense that
the query processor will not evaluate its arguments before calling it. The
operator is responsible in its value map to evaluate its arguments.

*/
  stpattern.SetRequestsArguments();
  stpattern2.SetRequestsArguments();
  stpatternex.SetRequestsArguments();
  stpatternex2.SetRequestsArguments();
  stpatternextend.SetRequestsArguments();
  stpatternextend2.SetRequestsArguments();
  stpatternexextend.SetRequestsArguments();
  stpatternexextend2.SetRequestsArguments();
  stpatternextendstream.SetRequestsArguments();
  stpatternextendstream2.SetRequestsArguments();
  stpatternexextendstream.SetRequestsArguments();
  stpatternexextendstream2.SetRequestsArguments();

/*
~SetUsesArgsInTypeMapping~ forces the query processor to pass the values of the
literal operator arguments to the typemap. That is, if one of the operator
arguments is a string literal, the typemap receives a list (string value).

*/
  stpattern.SetUsesArgsInTypeMapping();
  stpattern2.SetUsesArgsInTypeMapping();
  stpatternex.SetUsesArgsInTypeMapping();
  stpatternex2.SetUsesArgsInTypeMapping();
  stpatternextend.SetUsesArgsInTypeMapping();
  stpatternextend2.SetUsesArgsInTypeMapping();
  stpatternexextend.SetUsesArgsInTypeMapping();
  stpatternexextend2.SetUsesArgsInTypeMapping();
  stpatternextendstream.SetUsesArgsInTypeMapping();
  stpatternextendstream2.SetUsesArgsInTypeMapping();
  stpatternexextendstream.SetUsesArgsInTypeMapping();
  stpatternexextendstream2.SetUsesArgsInTypeMapping();
  computeClosure.SetUsesArgsInTypeMapping();

  AddOperator(&STP::createstvector);
  AddOperator(&STP::stpattern);
  AddOperator(&STP::stpattern2);
  AddOperator(&STP::stconstraint);
  AddOperator(&STP::stpatternex);
  AddOperator(&STP::stpatternex2);
  AddOperator(&STP::stpatternextend);
  AddOperator(&STP::stpatternextend2);
  AddOperator(&STP::stpatternexextend);
  AddOperator(&STP::stpatternexextend2);
  AddOperator(&STP::stpatternextendstream);
  AddOperator(&STP::stpatternextendstream2);
  AddOperator(&STP::stpatternexextendstream);
  AddOperator(&STP::stpatternexextendstream2);
  AddOperator(&STP::start);
  AddOperator(&STP::end);
  AddOperator(&randommbool);
  AddOperator(&passmbool);
  AddOperator(&randomdelay);
  AddOperator(&computeClosure);
  AddOperator(&randomshiftdelay);
}
~STPatternAlgebra() {};
};

};

/*
\section{Initialization}

*/



extern "C"
Algebra*
InitializeSTPatternAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new STP::STPatternAlgebra;
    }
