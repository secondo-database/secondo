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

[1] Header File of the Spatiotemporal Group Pattern Algebra

JAN, 2010 Mahmoud Sakr

[TOC]

1 Overview

2 Defines and includes


*/

#ifndef GPATTERNALGEBRA_H_
#define GPATTERNALGEBRA_H_
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "STPatternAlgebra.h"
#include "SpatialAlgebra.h"
#include "MSet.h"
#include <map>
using namespace datetime;
using namespace mset;
typedef DateTime Instant;
extern NestedList *nl;
extern QueryProcessor* qp;  


namespace GPattern {
enum quantifier {exactly, atleast};


struct DoubleInterval
{
  double start, end;
  bool lc, rc;
  DoubleInterval(double s, double e, bool l, bool r): start(s), 
     end(e), lc(l), rc(r){}
  void Set(double s, double e, bool l, bool r){
    start=s;    end=e;    lc=l;    rc=r;    }
  bool Inside(double s, double e, bool l, bool r)
  {
    return ((s< start && e > end) ||
    (s== start && (l || !lc) && e > end) ||
    (s< start && e == end && (r || !rc)) ||
    (s== start && (l || !lc) && e == end && (r || !rc)));
  }
  bool Inside(DoubleInterval& arg)
  {
    return ((arg.start< start && arg.end > end) ||
    (arg.start== start && (arg.lc || !lc) && arg.end > end) ||
    (arg.start< start && arg.end == end && (arg.rc || !rc)) ||
    (arg.start== start &&(arg.lc || !lc)&& arg.end == end &&(arg.rc || !rc)));
  }
};

class GPatternHelper
{
public:

  GPatternHelper():stream(0)
  {

  }
  ~GPatternHelper()
  {
    Clear(); 
  }

  void Clear()
  {
//    for(unsigned int i=0; i<stream.size(); ++i)
//      delete stream[i];
    stream.clear();
  }

  ostream& Print( ostream &os ) 
  {
    if ( stream.size() == 0) return (os << "Empty result");
    
    os << "\nResult size:" + stream.size();
    os << "\nMembers: " ;
    for(vector<InMemMSet>::iterator i=stream.begin() ;i != stream.end(); ++i)
    {
      (*i).Print(os); 
      os<<"\n";
    }
    os << "\n";
    return os;
  }
  
  void ComputeGPatternResultStream(InMemMSet& accumlator,int n, double d,
      quantifier q)
  {
    bool debugme=false;
    Clear();
    accumlator.HighPassCardinalityFilter(n);
    if(debugme)
    { cerr<<"\n***********GPattern Input *******\n"; accumlator.Print(cerr);}

    list<InMemUSet>::iterator t1, t2, tnext;
    t1= accumlator.units.begin();
    tnext= GetNextPariod(accumlator, t1);
    while( t1 != accumlator.units.end())
    {
      t2= tnext;
      --t2;
      if( ((*t2).endtime - (*t1).starttime) >= d)
      {
        vector<InMemMSet> streamPart;
        ComputeResultStreamPart(accumlator, n, d, q, t1, t2, streamPart);
        for(vector<InMemMSet>::iterator j= streamPart.begin(); 
          j!= streamPart.end(); ++j)
          stream.push_back(*j);
      }
      t1= tnext;
      tnext= GetNextPariod(accumlator, t1);
    }
    if(debugme)
    { 
      cerr<<"\n***********GPattern result *******\nNumber of results ="
          << stream.size()<<endl; 
      Print(cerr);
    }
  }
  vector<InMemMSet>::iterator it;
  vector<InMemMSet> stream;
private:
  void GenerateAllCombinations(InMemUSet& cand, int select, 
      vector< set<int> > & res)
  {
    int *a = new int[select];
    for (int k = 0; k < select; ++k)                   // initialize 
      a[k] = k + 1;                              // 1 - 2 - 3 - 4 - ...
    
    vector<int> candidates(cand.constValue.begin(), cand.constValue.end());
    int number= candidates.size();
    while (true)
    {     
      set<int> s;
      for (int i = 0; i < select; ++i)
      {
        int index= a[i] -1;
        s.insert(candidates[index]);
      }
      res.push_back(s);
      // generate next combination in lexicographical order
      int i = select - 1;                           // start at last item
      // find next item to increment
      while ( (i > -1) && (a[i] == (number - select + i + 1)))  
        --i;

      if (i < 0) break;                          // all done
      ++a[i];                                    // increment

      // do next 
      for (int j = i + 1; j < select; ++j)
        a[j] = a[i] + j - i;
    }
    delete[] a;
  }
  void ComputeResultStreamPart(InMemMSet& acc,unsigned int n, 
      double d, quantifier q, 
      list<InMemUSet>::iterator t1, list<InMemUSet>::iterator t2, 
      vector<InMemMSet>& result)
      {
    bool debugme= false;
    assert(acc.GetNoComponents() > 0 );
    assert(d > 0);
    assert(((*t2).endtime - (*t1).starttime) > d);
    
    if(debugme)
    {
      cerr<<"\nComputeResultStreamPart Called: n= "<< n <<"---------------\n";
      for(list<InMemUSet>::iterator t= t1; t!=t2; ++t)
        (*t).Print(cerr);
      (*t2).Print(cerr);
      cerr<<"End of input -----------------------";
    }
    multimap< set<int>, DoubleInterval> res;
   
    list<InMemUSet>::iterator unitIterator1=t1, unitIterator2=t2;
    double startInstant= (*t1).starttime, curInstant=0,
    endInstant= (*t2).endtime;
    bool lc= (*t1).lc, rc=false; 
    list<InMemUSet>::iterator curUnit;
    InMemUSet candidates;  
    curUnit= unitIterator1;
    while( endInstant - startInstant >= d)
    {
      unitIterator2= unitIterator1;
      curInstant= (*curUnit).endtime;
      rc= (*curUnit).rc;
      candidates.CopyFrom(*curUnit);
      while( candidates.Count() >= n && 
          curInstant - startInstant < d && unitIterator2 != t2)
      {
        curUnit = ++unitIterator2;
        curInstant= (*curUnit).endtime;
        rc= (*curUnit).rc;
        candidates.Intersection((*curUnit).constValue);
      }
      if(candidates.Count() >= n && curInstant - startInstant >= d)
        AddAllSubSetsToVector(candidates, startInstant, curInstant, 
            lc, rc, n, q, res);

      while( curInstant < endInstant && candidates.Count() >=n &&
          unitIterator2 != t2)
      {
        curUnit= ++unitIterator2;
        curInstant= (*curUnit).endtime;
        rc= (*curUnit).rc;
        candidates.Intersection( (*curUnit).constValue);
        if(candidates.Count() >= n )
          AddAllSubSetsToVector(candidates, startInstant, curInstant, 
              lc, rc, n, q, res);
      }
      candidates.Clear();
      if(unitIterator1 != t2)
      {
        curUnit= ++unitIterator1;
        startInstant = (*curUnit).starttime;
        lc= (*curUnit).lc;
      }
      else
        break;
    }
    result.reserve(res.size());
    multimap< set<int>, DoubleInterval>::iterator i;
    for(i= res.begin(); i != res.end(); ++i)
    {
      InMemMSet mset;
      InMemUSet uset( (*i).first, (*i).second.start, (*i).second.end, 
          (*i).second.lc, (*i).second.rc);
      mset.units.push_back(uset);
      if(debugme)
      {
        cerr<<"Adding  \n"; mset.Print(cerr); 
      }
      result.push_back(mset);
    }
    if(debugme)
    {
      cerr<<result.size(); 
    }
  }
  
  list<InMemUSet>::iterator GetNextPariod(InMemMSet& accumlator, 
      list<InMemUSet>::iterator t1)
  {
    bool debugme=false;
    list<InMemUSet>::iterator t2=t1;
    if(t2 == accumlator.units.end())
      return t2;
    double lastinstant= (*t1).endtime;
    if(debugme) (*t1).Print(cerr);
    while(++t2 != accumlator.units.end())
    {
      if(debugme) (*t2).Print(cerr);
      if((*t2).starttime != lastinstant)
        break;
      lastinstant= (*t2).endtime;
    }
    return t2;
  }
  
  void AddAllSubSetsToVector(InMemUSet& candidates, double startInstant, 
      double curInstant, bool lc, bool rc, 
      int n, quantifier q, multimap< set<int>, DoubleInterval>& res)
  {
    bool debugme= false; 
    bool changed=false;
    vector< set<int> > sets(0);
    if(debugme)
    {
      cerr<<"AddAllSubSetsToVector: recieved interval= ["<<startInstant 
      <<"  "<<curInstant<<"  "<<lc<<"  "<<rc;
      candidates.Print(cerr);
    }
    if(q == GPattern::exactly)
    {
      GenerateAllCombinations(candidates, n, sets);
      changed = (sets.size() != 0);
    }
    else if (q== GPattern::atleast)
    {
      for(unsigned int i= n; i<= candidates.Count(); ++i)
        GenerateAllCombinations(candidates, i, sets);
      changed = (sets.size() != 0);
    }
    
    if(changed)
    {
      pair<multimap< set<int>, DoubleInterval>::iterator,
        multimap< set<int>, DoubleInterval>::iterator> ret;
      multimap< set<int>, DoubleInterval>::iterator i;
      bool consumed= false;
      DoubleInterval timeInterval(startInstant, curInstant, lc, rc);
      for(unsigned int k=0; k<sets.size(); ++k)
      {
        consumed= false;
        ret = res.equal_range(sets[k]);
        for (i=ret.first; i!=ret.second && !consumed; ++i)
        {
          if((*i).second.Inside(timeInterval))
          {
            (*i).second.Set(startInstant, curInstant, lc, rc);
            consumed= true;
          }
          else if (timeInterval.Inside((*i).second))
            consumed= true;
        }
        if(!consumed)
        {
          DoubleInterval tmp(startInstant, curInstant, lc, rc);
          res.insert( pair<set<int>, DoubleInterval>(sets[k],tmp) );
        }
      }
      
    }
  }
};



class GPatternSolver
{
public:  
  
  Supplier TheStream;
  
/*
The list of supported assignments

*/  
  vector< vector< pair< Interval<CcReal>, set<int> > > > SA;
  vector<Supplier> Agenda;
    
/*
A helper data structure to translate the string aliases into their integer
position in the Agenda, SA and ConstraintGeraph.

*/
  map<string, int> VarAliasMap;
  vector< vector< vector<Supplier> > >ConstraintGraph;
/*
The total number of variables in the CSP.
 
*/  
  int count;
    
/*
The iterator is used in the "start" and "end" operators to iterate over the SA

*/
  int iterator;
  Interval<CcReal> nullInterval;
    
/*
A list of the variable that have been consumed so far.

*/
    vector<int> assignedVars;
    
    
    GPatternSolver():count(0),iterator(-1), 
    nullInterval(CcReal(true,0.0),CcReal(true,0.0), true,true)
    {}
    
/* 
The AddVariable function.
Input: the alias of the lifted predicate and a pointer to the its node in the 
operator tree.
Process: adds the variable to the Agenda and resizes the ConstraintGraph.
Output: error code

*/ 
    int AddVariable(string alias, Supplier handle);

/* 
The AddConstraint function.
Input: the two aliases of the two lifted predicates and a pointer to the 
"stconstraint" node in the operator tree.
Process: adds the constraint to the ConstraintGraph.
Output: error code

*/ 
   
    int AddConstraint(string alias1, string alias2, Supplier handle);
    
/*
The Solve function. It implements the Solve Pattern algorithm in the paper.

*/
    
    bool Solve();
    
/*
The MoveNext function. It is used to iterate over the SA list. The function is
used by the "start" and "end" operators in the extended STPP.
    
*/
    bool MoveNext();
    
/*
The GetStart function. It is the impelementation of the "start" operator.
    
*/
    bool GetStart(string alias, Instant& result);
    
/*
The GetStart function. It is the impelementation of the "end" operator.
      
*/
    bool GetEnd(string alias, Instant& result);
    
/*
The Print function. It is used for debug purposes.
   
*/  
    ostream& Print(ostream& os);
/*
The Clear function. It is used to intialize the CSP. It is necessary to 
call it before processing every tuple in order to reintialize the CSP.

*/
    int Clear();
/*
The WriteTuple function writes the current SA entry to a tuple
 
*/  
    void WriteTuple(Tuple* tuple);

private:
/*
The IntervalInstant2IntervalCcReal helper function. It converts the 
Interval<Instant> to Internal<CcReal> for more efficient processing

*/  
  void IntervalInstant2IntervalCcReal(const Interval<Instant>& in, 
      Interval<CcReal>& out);


/* 
The Extend function as in the paper.  
    
*/  
  bool Extend(int index);
    
/*
The IsSupported function.
Input: a partial assignment sa and the index of the newly evaluated variable.
Process: It checks whether the constraints that involve the new variable are
fulfilled.
Output: whether the partial assignment is consistent.
   
*/  
  bool IsSupported(vector< pair<Interval<CcReal>, set<int> > >& sa, int index);

/*
The CheckConstraint helper function. It checks whether an STVector is fulfilled 
by two lifted predicates. 

*/

  bool CheckConstraint(Interval<CcReal>& p1, Interval<CcReal>& p2, 
      vector<Supplier> constraint);
/*
The PickVariable function. It implements the picking methodology based on the
Connectivity rank as in the paper.

*/
  int PickVariable();

}GPSolver;



} // namespace GPattern



#endif 
