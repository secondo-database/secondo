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
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/STPattern/STPatternAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Collection/CollectionAlgebra.h"
#include "MSet.h"
#include <map>
#include <functional>
#include <algorithm>
#include "LightWeightGraph.h"
#include "Boids/Boid.h"

typedef datetime::DateTime Instant;
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
  std::vector<int> removedNodes;
  std::vector<int> removedNodesInRemoved;
  ChangeRecord();
  bool UpdateStatus(StatusCode newCode);
  bool AppendToRemovedNodes(int node);
  std::ostream& Print(std::ostream& os);
  void Clear();
};

class MSetIndex
{
public:
  struct NodeLogEntry
  {
    int64_t starttime, endtime;
    std::list<mset::CompressedInMemUSet>::iterator startUnitIt, endUnitIt;
    int startUnitIndex, endUnitIndex;
    std::set<int> associatedEdges;
    NodeLogEntry(
        int64_t st, std::list<mset::CompressedInMemUSet>::iterator stIt,
        int sUI,
        std::set<int>& aE);
    std::ostream& Print(std::ostream& os);
  };
  struct NodeLog
  {
    std::list<NodeLogEntry> log;
    void Append(std::list<mset::CompressedInMemUSet>::iterator& usetIt, 
         int usetIndex,
        std::set<int>& edges);
    bool RemoveUnit(int index);
    std::ostream& Print(std::ostream& os);
  };
  std::map<int,  NodeLog> nodes;
  std::vector<int> units;
  MSetIndex(mset::CompressedInMemMSet& mset, 
            std::vector<std::pair<int, int> >& edge2Nodes );
  void ConstructIndex(
      mset::CompressedInMemMSet& mset, 
      std::vector<std::pair<int, int> >& edge2Nodes);


  bool RemoveShortNodeEntries(int64_t dMS,
            std::vector<ChangeRecord>& AllChanges);
  bool RemoveShortNodeEntries(int64_t dMS, 
            std::map<int,  NodeLog>::iterator nodeIt,
      std::vector<ChangeRecord>& AllChanges);
  void DeleteNodePart(std::map<int,  NodeLog>::iterator& nodeIt,
      std::list<NodeLogEntry>::iterator nodeEntryIt,
      std::vector<ChangeRecord>& AllChanges);
  bool RemoveSmallUnits(int n, std::vector<ChangeRecord>& AllChanges);
  void RemoveUnit(int index, std::vector<ChangeRecord>& AllChanges);
  void FinalizeIndex(std::set<int>& nodesToBeFinalized,
      std::list<mset::CompressedInMemUSet>::iterator endIt);
  std::ostream& Print(std::ostream& os);

private:
  void AppendInsertionsIntoNodeLog(std::set<int>& deltaNodes,
      std::vector<std::pair<int, int> >& edge2nodesMap,
      std::list<mset::CompressedInMemUSet>::iterator& curUSet, 
      int curUSetIndex);

  void AppendRemovalsIntoNodeLog(std::set<int>& deltaNodes,
      std::vector<std::pair<int, int> >& edge2nodesMap,
      std::list<mset::CompressedInMemUSet>::iterator& curUSet, 
      int curUSetIndex);
};

class CheckRemoveEntry {
  private:
    int nodeId;
    std::vector<ChangeRecord>* AllChanges;
    MSetIndex* index;
    int64_t dMS;
  public:
    static bool changed;
    void SetChanged(bool ch);
    bool GetChanged();
    CheckRemoveEntry(
        int id, std::vector<ChangeRecord>& ch, MSetIndex* i, int64_t _dMS);
    bool operator() (MSetIndex::NodeLogEntry& logEntry);
};
/*
GPatternHelper

*/ 

class GPatternHelper
{
public:

GPatternHelper(){}
~GPatternHelper() {}

bool setCompareDesc (std::set<int>& i,std::set<int>& j);

void RemoveDuplicates(std::list<mset::CompressedInMemUSet>& resStream);

void removeShortUnits(temporalalgebra::MBool &mbool, int64_t dMS);

bool RemoveShortNodeMembership(mset::CompressedInMemMSet& Accumlator,
    std::vector<std::pair<int, int> >& edge2nodesMap, int64_t dMS);

std::ostream& PrintNodeHistory(
  std::map<int, 
      std::pair<std::list<mset::CompressedInMemUSet>::iterator, 
     std::set<int> > >* nodeHistory,
  std::ostream &os );

bool CheckRemoveNodeMembership(
  std::map<int, std::pair<
      std::list<mset::CompressedInMemUSet>::iterator, 
                std::set<int> > >* nodeHistory,
  std::set<int> removedEdges, std::vector<std::pair<int, int> >& edge2nodesMap,
  std::list<mset::CompressedInMemUSet>::iterator& cur, 
           std::set<int>& deltaNodes, int64_t dMS);

void InPlaceSetDifference(std::set<int>& set1, std::set<int>& set2);

void InsertInNodeHistory(
  std::map<int, std::pair<std::list<mset::CompressedInMemUSet>::iterator, 
         std::set<int> > >* nodeHistory,
  std::set<int> newEdges, std::vector<std::pair<int, int> >& edge2nodesMap,
  std::list<mset::CompressedInMemUSet>::iterator& cur, 
            std::set<int>& deltaNodes);

void FindNodeEdges(
    int newNode, std::set<int>& newEdges, 
               std::vector<std::pair<int, int> >& edge2nodesMap,
    std::set<int>& newNodeEdges);

bool RemoveUnitsHavingFewNodes(
    mset::CompressedInMemMSet& Accumlator,
    std::vector<std::pair<int, int> >& edge2nodesMap, int n);


void FindLargeDynamicComponents(mset::CompressedInMemMSet& Accumlator,
    std::list<mset::CompressedInMemUSet>::iterator begin,
    std::list<mset::CompressedInMemUSet>::iterator end ,
    std::vector<std::pair<int,int> >& edge2nodesMap, int64_t dMS, int n, 
    std::string& qts,
    std::list<mset::CompressedMSet*>*& finalResStream, int depth);
  
mset::CompressedMSet* CollectResultParts(
    std::vector<mset::CompressedMSet*>& ResultParts, 
    std::vector<int>&  partIndexs);

void FindDynamicComponents(mset::CompressedInMemMSet& Accumlator,
    std::list<mset::CompressedInMemUSet>::iterator begin, 
    std::list<mset::CompressedInMemUSet>::iterator end , 
    std::vector<std::pair<int, int> >& edge2nodesMap, int64_t dMS, int n,
    std::string& qts,
    std::list<mset::CompressedMSet*>*& FinalResultStream);

bool SetIntersects(std::set<int> &set1, std::set<int> &set2);

void CheckAdd( LWGraph* g, NewComponent& comp, int n ,
    std::list<Component*>* components, 
    std::map<int, std::list<Component*>::iterator>& compLabelsMap, 
    int& NextComponentLabel);

void UpdateMerge(LWGraph *g, NewComponent& newComp, std::set<int>& newEdges,
    std::vector<std::pair<int, int> >& edge2nodesMap, 
    std::list<Component*>* components,
    std::map<int, std::list<Component*>::iterator>& compLabelsMap);

void MergeComponents(LWGraph* g, NewComponent& newComp,
    std::list<Component*>* components, 
    std::map<int, std::list<Component*>::iterator>& compLabelsMap, 
    int& NextComponentLabel);

void UpdateRemove(LWGraph* graph, std::list<Component*>* components,
    std::map<int, std::list<Component*>::iterator>& compLabelsMap,
    std::vector<std::pair<int, int> >& edge2nodesMap, 
    int n, int& NextComponentLabel,
    std::set< int>& affectedComponentsLabels);

void Finalize(LWGraph* graph, std::list<Component*>* components, int64_t dMS,
    mset::CompressedInMemUSet& cur,
    std::vector<mset::CompressedMSet*>& ResultParts,
    std::list<std::vector<int> > *ResultStream,
    std::list<mset::CompressedMSet*>* FinalResultStream,
    std::vector<std::pair<int, int> >& edge2nodesMap);

void DynamicGraphAppend(LWGraph* graph, std::list<Component*>* components,
    std::map<int, std::list<Component*>::iterator>& compLabelsMap,
    std::list<mset::CompressedInMemUSet>::iterator cur,
    std::vector<std::pair<int, int> >& edge2nodesMap, int64_t dMS,
    int n, std::string& qts, int& NextComponentLabel, 
    std::vector<mset::CompressedMSet*>* ResultParts,
    std::list<std::vector<int> >* ResultStream,
    std::list<mset::CompressedMSet*>*& FinalResultStream);

void GraphNodes2Edges(std::set<int>& subGraphNodes, std::set<int>& graphEdges,
    std::vector<std::pair<int, int> >& edge2Nodes, std::set<int>& res);

bool Merge(mset::CompressedInMemMSet *_mset, std::set<int> *subGraph,
    int64_t starttime, int64_t endtime, bool lc, bool rc);

void SetAddRemove(std::set<int>& _set, std::set<int>& _add, 
                  std::set<int>& _remove);
  
void EdgeSet2NodeSet(
    std::set<int> &edges, std::set<int> &nodes, 
    std::vector<std::pair<int, int> > edge2nodesMap);
  
mset::CompressedMSet* EdgeMSet2NodeMSet(
    mset::CompressedMSet* edgeMSet, 
    std::vector<std::pair<int, int> >& edge2nodesMap);

void ComputeAddSubSets(mset::InMemMSet& acc,
    std::list<mset::InMemUSet>::iterator t1, 
    std::list<mset::InMemUSet>::iterator t2,
    unsigned int n,  int64_t dMS, std::vector<mset::InMemMSet*>* result);

bool ApplyThresholds(MSetIndex& index, int n, int64_t dMS,
    std::vector<ChangeRecord>& Changes, bool debugme);

void UpdateResult(mset::CompressedInMemMSet* curMSet,
    std::vector<std::pair<int,int> >& edge2nodesMap,int n, 
    int64_t dMS, std::string qts,
    std::vector<ChangeRecord>& Changes,
    std::list<mset::CompressedMSet*>& resStream, bool debugme);

void ApplyChanges(mset::CompressedInMemMSet* msetPart,
    std::vector<ChangeRecord>& changesPart,
    std::vector<std::pair<int,int> >& edge2nodesMap,int n, 
    int64_t dMS, std::string qts,
    std::list<mset::CompressedMSet*>& resStream, 
    std::list<mset::CompressedInMemMSet*>& msetParts,
    std::list<std::vector<ChangeRecord> >& changeParts, bool debugme);
/*
Private members

*/ 
private:
  void GenerateAllCombinations(mset::InMemUSet& cand, int select,
      std::vector< std::set<int> > & res);
      
  void AddAllSubSetsToVector(mset::InMemUSet& candidates, int64_t startInstant,
      int64_t curInstant, bool lc, bool rc,
      int n, std::multimap< std::set<int>, Int64Interval>& res);
};



class GPatternSolver
{
public:  
  
  Supplier TheStream;
  
/*
The list of supported assignments

*/  
  std::vector< std::vector< std::pair< temporalalgebra::Interval<Instant>, 
          mset::MSet* > > > SA;
  std::vector<Supplier> Agenda;
    
/*
A helper data structure to translate the string aliases into their integer
position in the Agenda, SA and ConstraintGeraph.

*/
  std::map<std::string, int> VarAliasMap;
  std::vector< std::vector< std::vector<Supplier> > >ConstraintGraph;
  std::vector<mset::MSet*> ToDelete;
/*
The total number of variables in the CSP.
 
*/  
  int count;
    
/*
The iterator is used in the "start" and "end" operators to iterate over the SA

*/
  int iterator;
  temporalalgebra::Interval<Instant> nullInterval;
    
/*
A list of the variable that have been consumed so far.

*/
    std::vector<int> assignedVars;
    
    
    GPatternSolver():count(0),iterator(-1), 
    nullInterval(Instant(0,0, datetime::instanttype ),
        Instant(0,0, datetime::instanttype ), true,true)
    {}
    
    ~GPatternSolver()
    {
      for(std::vector<mset::MSet*>::iterator it= 
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
    int AddVariable(std::string alias, Supplier handle);

/* 
The AddConstraint function.
Input: the two aliases of the two lifted predicates and a pointer to the 
"stconstraint" node in the operator tree.
Process: adds the constraint to the ConstraintGraph.
Output: error code

*/ 
   
    int AddConstraint(std::string alias1, std::string alias2, Supplier handle);
    
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
    bool GetStart(std::string alias, Instant& result);
    
/*
The GetStart function. It is the impelementation of the "end" operator.
      
*/
    bool GetEnd(std::string alias, Instant& result);
    
/*
The Print function. It is used for debug purposes.
   
*/  
    std::ostream& Print(std::ostream& os);
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
temporalalgebra::Interval<Instant> to Internal<CcReal> for more efficient 
processing

*/  
  void IntervalInstant2IntervalCcReal(
      const temporalalgebra::Interval<Instant>& in, 
      temporalalgebra::Interval<CcReal>& out);


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
  bool IsSupported(
       std::vector< std::pair<temporalalgebra::Interval<Instant>, 
           mset::MSet* > >& sa, 
       int index);

/*
The CheckConstraint helper function. It checks whether an STVector is fulfilled 
by two lifted predicates. 

*/

  bool CheckConstraint(temporalalgebra::Interval<Instant>& p1, 
                       temporalalgebra::Interval<Instant>& p2,
      std::vector<Supplier> constraint);
/*
The PickVariable function. It implements the picking methodology based on the
Connectivity rank as in the paper.

*/
  int PickVariable();

};

} // namespace GPattern



#endif 

