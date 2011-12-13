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
#include "CollectionAlgebra.h"
#include "MSet.h"
#include <map>
#include <functional>
#include <algorithm>
#include "LightWeightGraph.h"
#include "Boids/Boid.h"

using namespace datetime;
using namespace mset;
typedef DateTime Instant;
extern NestedList *nl;
extern QueryProcessor* qp;  


namespace GPattern {
enum quantifier {exactly, atleast};

struct Int64Interval
{
  int64_t start, end;
  bool lc, rc;
  Int64Interval(int64_t s, int64_t e, bool l, bool r): start(s),
     end(e), lc(l), rc(r){}
  void Set(int64_t s, int64_t e, bool l, bool r){
    start=s;    end=e;    lc=l;    rc=r;    }
  bool Inside(int64_t s, int64_t e, bool l, bool r)
  {
    return ((s< start && e > end) ||
    (s== start && (l || !lc) && e > end) ||
    (s< start && e == end && (r || !rc)) ||
    (s== start && (l || !lc) && e == end && (r || !rc)));
  }
  bool Inside(Int64Interval& arg)
  {
    return ((arg.start< start && arg.end > end) ||
    (arg.start== start && (arg.lc || !lc) && arg.end > end) ||
    (arg.start< start && arg.end == end && (arg.rc || !rc)) ||
    (arg.start== start &&(arg.lc || !lc)&& arg.end == end &&(arg.rc || !rc)));
  }
};

/*
Class ChangeRecord. Used in the process of finding
LargeConnectedComponents given a ConnectedComponent

*/
class ChangeRecord
{
public:
  enum StatusCode{NotChanged=0, ElemsRemoved=1, UnitRemoved=2};
  StatusCode status;
  vector<int> removedNodes;
  vector<int> removedNodesInRemoved;
  ChangeRecord();
  bool UpdateStatus(StatusCode newCode);
  bool AppendToRemovedNodes(int node);
  ostream& Print(ostream& os);
  void Clear();
};

class MSetIndex
{
public:
  struct NodeLogEntry
  {
    int64_t starttime, endtime;
    list<CompressedInMemUSet>::iterator startUnitIt, endUnitIt;
    int startUnitIndex, endUnitIndex;
    set<int> associatedEdges;
    NodeLogEntry(
        int64_t st, list<CompressedInMemUSet>::iterator stIt, int sUI,
        set<int>& aE);
    ostream& Print(ostream& os);
  };
  struct NodeLog
  {
    list<NodeLogEntry> log;
    void Append(list<CompressedInMemUSet>::iterator& usetIt, int usetIndex,
        set<int>& edges);
    bool RemoveUnit(int index);
    ostream& Print(ostream& os);
  };
  map<int,  NodeLog> nodes;
  vector<int> units;
  MSetIndex(CompressedInMemMSet& mset, vector<pair<int, int> >& edge2Nodes );
  void ConstructIndex(
      CompressedInMemMSet& mset, vector<pair<int, int> >& edge2Nodes);


  bool RemoveShortNodeEntries(int64_t dMS, vector<ChangeRecord>& AllChanges);
  bool RemoveShortNodeEntries(int64_t dMS, map<int,  NodeLog>::iterator nodeIt,
      vector<ChangeRecord>& AllChanges);
  void DeleteNodePart(map<int,  NodeLog>::iterator& nodeIt,
      list<NodeLogEntry>::iterator nodeEntryIt,
      vector<ChangeRecord>& AllChanges);
  bool RemoveSmallUnits(int n, vector<ChangeRecord>& AllChanges);
  void RemoveUnit(int index, vector<ChangeRecord>& AllChanges);
  void FinalizeIndex(set<int>& nodesToBeFinalized,
      list<CompressedInMemUSet>::iterator endIt);
  ostream& Print(ostream& os);

private:
  void AppendInsertionsIntoNodeLog(set<int>& deltaNodes,
      vector<pair<int, int> >& edge2nodesMap,
      list<CompressedInMemUSet>::iterator& curUSet, int curUSetIndex);

  void AppendRemovalsIntoNodeLog(set<int>& deltaNodes,
      vector<pair<int, int> >& edge2nodesMap,
      list<CompressedInMemUSet>::iterator& curUSet, int curUSetIndex);
};

class CheckRemoveEntry {
  private:
    int nodeId;
    vector<ChangeRecord>* AllChanges;
    MSetIndex* index;
    int64_t dMS;
  public:
    static bool changed;
    void SetChanged(bool ch);
    bool GetChanged();
    CheckRemoveEntry(
        int id, vector<ChangeRecord>& ch, MSetIndex* i, int64_t _dMS);
    bool operator() (MSetIndex::NodeLogEntry& logEntry);
};
bool CheckRemoveEntry::changed = false;
/*
GPatternHelper

*/ 

class GPatternHelper
{
public:

GPatternHelper(){}
~GPatternHelper() {}

bool setCompareDesc (set<int>& i,set<int>& j);

void RemoveDuplicates(list<CompressedInMemUSet>& resStream);

void removeShortUnits(MBool &mbool, int64_t dMS);

bool RemoveShortNodeMembership(CompressedInMemMSet& Accumlator,
    vector<pair<int, int> >& edge2nodesMap, int64_t dMS);

ostream& PrintNodeHistory(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  ostream &os );

bool CheckRemoveNodeMembership(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  set<int> removedEdges, vector<pair<int, int> >& edge2nodesMap,
  list<CompressedInMemUSet>::iterator& cur, set<int>& deltaNodes, int64_t dMS);

void InPlaceSetDifference(set<int>& set1, set<int>& set2);

void InsertInNodeHistory(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  set<int> newEdges, vector<pair<int, int> >& edge2nodesMap,
  list<CompressedInMemUSet>::iterator& cur, set<int>& deltaNodes);

void FindNodeEdges(
    int newNode, set<int>& newEdges, vector<pair<int, int> >& edge2nodesMap,
    set<int>& newNodeEdges);

bool RemoveUnitsHavingFewNodes(
    CompressedInMemMSet& Accumlator,
    vector<pair<int, int> >& edge2nodesMap, int n);


void FindLargeDynamicComponents(CompressedInMemMSet& Accumlator,
    list<CompressedInMemUSet>::iterator begin,
    list<CompressedInMemUSet>::iterator end ,
    vector<pair<int,int> >& edge2nodesMap, int64_t dMS, int n, string& qts,
    list<CompressedMSet*>*& finalResStream, int depth);
  
CompressedMSet* CollectResultParts(
    vector<CompressedMSet*>& ResultParts, vector<int>&  partIndexs);

void FindDynamicComponents(CompressedInMemMSet& Accumlator,
    list<CompressedInMemUSet>::iterator begin, 
    list<CompressedInMemUSet>::iterator end , 
    vector<pair<int, int> >& edge2nodesMap, int64_t dMS, int n, string& qts,
    list<CompressedMSet*>*& FinalResultStream);

bool SetIntersects(set<int> &set1, set<int> &set2);

void CheckAdd( LWGraph* g, NewComponent& comp, int n ,
    list<Component*>* components, 
    map<int, list<Component*>::iterator>& compLabelsMap, 
    int& NextComponentLabel);

void UpdateMerge(LWGraph *g, NewComponent& newComp, set<int>& newEdges,
    vector<pair<int, int> >& edge2nodesMap, list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap);

void MergeComponents(LWGraph* g, NewComponent& newComp,
    list<Component*>* components, 
    map<int, list<Component*>::iterator>& compLabelsMap, 
    int& NextComponentLabel);

void UpdateRemove(LWGraph* graph, list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    vector<pair<int, int> >& edge2nodesMap, int n, int& NextComponentLabel,
    set< int>& affectedComponentsLabels);

void Finalize(LWGraph* graph, list<Component*>* components, int64_t dMS,
    CompressedInMemUSet& cur,
    vector<CompressedMSet*>& ResultParts,
    list<vector<int> > *ResultStream,
    list<CompressedMSet*>* FinalResultStream,
    vector<pair<int, int> >& edge2nodesMap);

void DynamicGraphAppend(LWGraph* graph, list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    list<CompressedInMemUSet>::iterator cur,
    vector<pair<int, int> >& edge2nodesMap, int64_t dMS,
    int n, string& qts, int& NextComponentLabel, 
    vector<CompressedMSet*>* ResultParts,
    list<vector<int> >* ResultStream,
    list<CompressedMSet*>*& FinalResultStream);

void GraphNodes2Edges(set<int>& subGraphNodes, set<int>& graphEdges,
    vector<pair<int, int> >& edge2Nodes, set<int>& res);

bool Merge(CompressedInMemMSet *_mset, set<int> *subGraph,
    int64_t starttime, int64_t endtime, bool lc, bool rc);

void SetAddRemove(set<int>& _set, set<int>& _add, set<int>& _remove);
  
void EdgeSet2NodeSet(
    set<int> &edges, set<int> &nodes, vector<pair<int, int> > edge2nodesMap);
  
CompressedMSet* EdgeMSet2NodeMSet(
    CompressedMSet* edgeMSet, vector<pair<int, int> >& edge2nodesMap);

void ComputeAddSubSets(InMemMSet& acc,
    list<InMemUSet>::iterator t1, list<InMemUSet>::iterator t2,
    unsigned int n,  int64_t dMS, vector<InMemMSet*>* result);

bool ApplyThresholds(MSetIndex& index, int n, int64_t dMS,
    vector<ChangeRecord>& Changes, bool debugme);

void UpdateResult(CompressedInMemMSet* curMSet,
    vector<pair<int,int> >& edge2nodesMap,int n, int64_t dMS, string qts,
    vector<ChangeRecord>& Changes,
    list<CompressedMSet*>& resStream, bool debugme);

void ApplyChanges(CompressedInMemMSet* msetPart,
    vector<ChangeRecord>& changesPart,
    vector<pair<int,int> >& edge2nodesMap,int n, int64_t dMS, string qts,
    list<CompressedMSet*>& resStream, list<CompressedInMemMSet*>& msetParts,
    list<vector<ChangeRecord> >& changeParts, bool debugme);
/*
Private members

*/ 
private:
  void GenerateAllCombinations(InMemUSet& cand, int select,
      vector< set<int> > & res);
      
  void AddAllSubSetsToVector(InMemUSet& candidates, int64_t startInstant,
      int64_t curInstant, bool lc, bool rc,
      int n, multimap< set<int>, Int64Interval>& res);
};



class GPatternSolver
{
public:  
  
  Supplier TheStream;
  
/*
The list of supported assignments

*/  
  vector< vector< pair< Interval<Instant>, MSet* > > > SA;
  vector<Supplier> Agenda;
    
/*
A helper data structure to translate the string aliases into their integer
position in the Agenda, SA and ConstraintGeraph.

*/
  map<string, int> VarAliasMap;
  vector< vector< vector<Supplier> > >ConstraintGraph;
  vector<MSet*> ToDelete;
/*
The total number of variables in the CSP.
 
*/  
  int count;
    
/*
The iterator is used in the "start" and "end" operators to iterate over the SA

*/
  int iterator;
  Interval<Instant> nullInterval;
    
/*
A list of the variable that have been consumed so far.

*/
    vector<int> assignedVars;
    
    
    GPatternSolver():count(0),iterator(-1), 
    nullInterval(Instant(0,0, instanttype ),
        Instant(0,0, instanttype ), true,true)
    {}
    
    ~GPatternSolver()
    {
      for(vector<MSet*>::iterator it= 
        ToDelete.begin(); it != ToDelete.end(); ++it)
      {
        (*it)->DeleteIfAllowed(true);
      }
    }
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
  bool IsSupported(vector< pair<Interval<Instant>, MSet* > >& sa, int index);

/*
The CheckConstraint helper function. It checks whether an STVector is fulfilled 
by two lifted predicates. 

*/

  bool CheckConstraint(Interval<Instant>& p1, Interval<Instant>& p2,
      vector<Supplier> constraint);
/*
The PickVariable function. It implements the picking methodology based on the
Connectivity rank as in the paper.

*/
  int PickVariable();

};

} // namespace GPattern



#endif 

