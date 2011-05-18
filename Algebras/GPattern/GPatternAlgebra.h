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
#include <functional>
#include <algorithm>
#include "LightWeightGraph.h"

using namespace datetime;
using namespace mset;
typedef DateTime Instant;
extern NestedList *nl;
extern QueryProcessor* qp;  


namespace GPattern {
enum quantifier {exactly, atleast};
typedef pair<int, int> intpair;

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


/*
GPatternHelper

*/ 

class GPatternHelper
{
public:

GPatternHelper(){}
~GPatternHelper() {}

static bool setCompareDesc (set<int>& i,set<int>& j) 
{ 
  if(includes(i.begin(), i.end(), j.begin(), j.end())) return true;
  if(i > j) return true;
  return false;
}
static void RemoveDuplicates(list<CompressedInMemUSet>& resStream)
{
  //sort(resStream.begin(), resStream.end(), setCompareDesc);
  
}

static void removeShortUnits(MBool &mbool, double d)
{
  UBool ubool;
  double starttime, endtime;
  for(int i= 0; i< mbool.GetNoComponents(); ++i)
  {
    mbool.Get(i, ubool);
    starttime= ubool.timeInterval.start.ToDouble() * day2min;
    endtime= ubool.timeInterval.end.ToDouble() * day2min;
    if(endtime - starttime < d)
    {
      ubool.constValue.Set(false, false);
      mbool.Put(i, ubool);
    }
  }
}

static bool RemoveShortNodeMembership(
    CompressedInMemMSet& Accumlator,
    vector<pair<int, int> >& ids, double d)
{
  bool debugme= false;
  if(debugme)
    Accumlator.Print(cerr);
  bool changed=false;
  if(Accumlator.units.size()==0) return false;
  list<CompressedInMemUSet>::iterator i= Accumlator.units.begin();
  LWGraph graph;
  typedef pair<list<CompressedInMemUSet>::iterator, set<int> > NodeItNeighbors;
  map<int, NodeItNeighbors> nodeHistory;
  set<int> deltaNodes;
  while(i!= Accumlator.units.end())
  {
    if(debugme)
    {
      cerr<<"\n graph:";
      graph.print(cerr);
      cerr<<"\n addedEdges:";
      PrintSet((*i).added, cerr);
      cerr<<"\n removedEdges:";
      PrintSet((*i).removed, cerr); cerr<<"\nAccumlator: "; 
      Accumlator.Print(cerr);
    }
    InsertEdgesUndirected(&graph, (*i).added, ids, deltaNodes);
    InsertInNodeHistory(&nodeHistory, (*i).added, ids, i, deltaNodes);
    if(debugme)
      PrintNodeHistory(&nodeHistory, cerr);
    RemoveEdgesUndirected(&graph, (*i).removed, ids, deltaNodes);
    changed= (changed || CheckRemoveNodeMembership(
        &nodeHistory, (*i).removed, ids, i, deltaNodes, d));
    if(debugme)
      PrintNodeHistory(&nodeHistory, cerr);
    ++i;
  }
  //Remove the units which have empty added and removed sets
  //Accumlator.RemoveConstUnits();
  return changed;
}

static ostream& PrintNodeHistory(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  ostream &os )
{
  map<int, pair<list<CompressedInMemUSet>::iterator,set<int> > >::iterator i;
  i= nodeHistory->begin();
  while(i != nodeHistory->end())
  {
    os<<endl<<"node "<< (*i).first;
    os<<" started at time "<< (*((*i).second.first)).starttime;
    os<<", and it is connected by edges ";
    PrintSet((*i).second.second, cerr);
    ++i;
  }
  return os;
}


static bool CheckRemoveNodeMembership(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  set<int> removedEdges, vector<pair<int, int> >& ids,
  list<CompressedInMemUSet>::iterator& cur, set<int>& deltaNodes, double d)
{
  double nodeMembershipDuration;
  bool changed=false;
  map<int, pair<list<CompressedInMemUSet>::iterator,set<int> > >::iterator pos;
  list<CompressedInMemUSet>::iterator nodeFirstUnitIt, curUnitIt;
  set<int>* nodeEdges;
  int _node;
  for(set<int>::iterator it= deltaNodes.begin(); it!=deltaNodes.end(); ++it)
  {
    _node= *it;
    pos= nodeHistory->find(_node);
    assert(pos != nodeHistory->end());
    nodeFirstUnitIt= (*pos).second.first;
    nodeMembershipDuration= (*cur).starttime - (*nodeFirstUnitIt).starttime;
    if(nodeMembershipDuration < d)
    {
      changed= true;
      nodeEdges= & (*pos).second.second;
      curUnitIt= nodeFirstUnitIt;
      while(curUnitIt != cur)
      {
        InPlaceSetDifference((*curUnitIt).added, *nodeEdges);
        InPlaceSetDifference((*curUnitIt).removed, *nodeEdges);
        ++curUnitIt;
      }
      InPlaceSetDifference((*curUnitIt).added, *nodeEdges);
      InPlaceSetDifference((*curUnitIt).removed, *nodeEdges);
    }
    nodeHistory->erase(pos);
  }
  return changed;
}

static void InPlaceSetDifference(set<int>& set1, set<int>& set2)
{
  if(set1.empty() || set2.empty()) return;
  set<int>::iterator first1, first2, last1, last2;
  first1= set1.begin(); first2= set2.begin();
  last1 = --set1.end();  last2= --set2.end();
  if( (*last1 < *first2) || (*last2 < *first1) )  return;
  ++last1; ++last2;
  
  while (first1 != last1 && first2 != last2)
  {
    if (*first1 < *first2)
      ++first1;
    else if (*first2 < *first1)
      ++first2;
    else 
    {
      set1.erase(first1++);
      ++first2;
    }
  }  

}
static void InsertInNodeHistory(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  set<int> newEdges, vector<pair<int, int> >& ids,
  list<CompressedInMemUSet>::iterator& cur, set<int>& deltaNodes)
{
  int _edge;
  set<int> newNodeEdges;
  pair<int, int>* edgeNodes;
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >::iterator pos;
  set<int>* nodeHistoryEdges;
  for(set<int>::iterator it= newEdges.begin(); it!=newEdges.end(); ++it)
  {
    _edge= *it;
    edgeNodes= &ids[_edge];
    pos= nodeHistory->find(edgeNodes->first);
    if(pos != nodeHistory->end())
    {
      nodeHistoryEdges= &(*pos).second.second;
      nodeHistoryEdges->insert(_edge);
    }
    else
    {
      assertIf(deltaNodes.find(edgeNodes->first) != deltaNodes.end());
      FindNodeEdges(edgeNodes->first, newEdges, ids, newNodeEdges);
      nodeHistory->insert(
          make_pair(edgeNodes->first, make_pair(cur, newNodeEdges)));       
    }
    
    pos= nodeHistory->find(edgeNodes->second);
    if(pos != nodeHistory->end())
    {
      nodeHistoryEdges= &(*pos).second.second;
      nodeHistoryEdges->insert(_edge);
    }
    else
    {
      assertIf(deltaNodes.find(edgeNodes->second) != deltaNodes.end());
      FindNodeEdges(edgeNodes->second, newEdges, ids, newNodeEdges);
      nodeHistory->insert(
          make_pair(edgeNodes->second, make_pair(cur, newNodeEdges)));          
    }
  }
}

static void FindNodeEdges(
    int newNode, set<int>& newEdges, vector<pair<int, int> >& ids, 
    set<int>& newNodeEdges)
{
  newNodeEdges.clear();
  pair<int, int>* edgeNodes;
  for(set<int>::iterator it= newEdges.begin(); it!=newEdges.end(); ++it)
  {
    edgeNodes= &ids[*it];
    if(edgeNodes->first == newNode || edgeNodes->second == newNode)
      newNodeEdges.insert(*it);
  }
}
static bool RemoveUnitsHavingFewNodes(
    CompressedInMemMSet& Accumlator,
    vector<pair<int, int> >& ids, int n)
{
  //bool debugme= false;
  bool changed=false;
  if(Accumlator.units.size()==0) return false;
  list<CompressedInMemUSet>::iterator i= Accumlator.units.begin();
  LWGraph graph;
  vector<list<CompressedInMemUSet>::iterator> unitsToRemove;
  while(i!= Accumlator.units.end())
  {
    InsertEdgesUndirected(&graph, (*i).added, ids);
    RemoveEdgesUndirected(&graph, (*i).removed, ids);
    if(graph.nodes_count() < n)
      unitsToRemove.push_back(i);
    ++i;
  }
  
  if(!unitsToRemove.empty())
  {
    changed=true;
    for(unsigned int j=0; j< unitsToRemove.size(); ++j)
      Accumlator.EraseUnit(unitsToRemove[j]);
  }
  return changed;
}

template<class MSetClass, class IteratorClass>
static void FindSubGraphs(MSetClass& Accumlator,
    IteratorClass begin, IteratorClass end , 
    vector<pair<int,int> >& ids, double d, int n, string& qts, 
    list<CompressedMSet*>*& finalResStream, int depth)
{
  bool debugme= false;
  IteratorClass cur= begin;
  set<int> s;
  list<CompressedMSet*>* resStream= 0, *localResStream= 0;
  finalResStream= new list<CompressedMSet*>(0);
  if(debugme)
  {
    cerr<<"\nFindSubGraphs started at depth " << depth;
    cerr<<"\nAccumlator has " << Accumlator.GetNoComponents()<< " units";
  }
  FindDynamicComponents(Accumlator, begin, end, ids, d, n, qts, resStream);
  if(debugme)
    cerr<<"\nFindDynamicComponents returned";
  bool locallyChanged= true, globallyChanged= false;
  list<CompressedInMemMSet*> tmpResStream;
  CompressedMSet* curMSet=0;
  while(resStream->begin() != resStream->end())
  {
    curMSet= resStream->front();
    CompressedInMemMSet* curInMemMSet= new CompressedInMemMSet();
    curMSet->WriteToCompressedInMemMSet(*curInMemMSet);
    if(debugme)
      curInMemMSet->Print(cerr);
    locallyChanged=true; globallyChanged= false;
    while(locallyChanged && 0)
    {
      locallyChanged= GPatternHelper::RemoveShortNodeMembership(
          *curInMemMSet, ids, d);
      locallyChanged= (locallyChanged || 
          GPatternHelper::RemoveUnitsHavingFewNodes(*curInMemMSet, ids, n));
      globallyChanged= (globallyChanged || locallyChanged);
      if(debugme)
        curInMemMSet->Print(cerr);
    }
    if(debugme&& 0)
      curInMemMSet->Print(cerr);
    tmpResStream.push_back(curInMemMSet);  
    /////////////tmp lines
//    finalResStream->push_back(curMSet);  
    curMSet->ReadFromCompressedInMemMSet(*curInMemMSet);
    /////////////tmp lines
      
    if(globallyChanged &&0)
    {     
      list<CompressedInMemUSet>::iterator begin2= 
        curInMemMSet->units.begin(), end2, tmp;
      while(begin2 != curInMemMSet->units.end())
      {
        end2= curInMemMSet->GetPeriodEndUnit(begin2);
        if(debugme&& 0)
        {
          (*begin2).Print(cerr);
          (*end2).Print(cerr);
        }
        if((*end2).endtime - (*begin2).starttime >= d)
        {
          ++end2;
          GPatternHelper::FindSubGraphs<CompressedInMemMSet, 
            list<CompressedInMemUSet>::iterator>(
              *curInMemMSet, begin2, end2 , ids, d, n, qts, localResStream, 
              depth+1);
          resStream->splice(resStream->end(), *localResStream);
          delete localResStream;
        }
        else
          ++end2;
        begin2= end2;
      }
      delete curMSet;
      curInMemMSet->Clear();
    }
    else
      finalResStream->push_back(curMSet);
    resStream->pop_front();
  }
  //GPatternHelper::RemoveDuplicates(*finalResStream);
  delete resStream;
  //delete curInMemMSet;
}
  
struct 
checkShortDelete: binary_function< CompressedMSet* ,double,bool>
{
public: 
  bool operator() (CompressedMSet* _mset, double d) const
  {
    if(_mset->units.Size() == 0)
    {
      delete _mset;
      return true;
    }
    CompressedUSetRef uset;
    _mset->units.Get(0, uset);
    double starttime= uset.starttime;
    _mset->units.Get(_mset->units.Size() - 1, uset);
    double endtime= uset.endtime;
    if(endtime - starttime < d) 
    {
      delete _mset;
      return true;
    }
    return false;
  }
};

static void FindDynamicComponents(CompressedInMemMSet& Accumlator,
    list<CompressedInMemUSet>::iterator begin, 
    list<CompressedInMemUSet>::iterator end , 
    vector<intpair>& ids, double d, int n, string& qts, 
    list<CompressedMSet*>*& finalResStream)
{
  bool debugme= true;
  set<int> constValue;
  map<int, int> _map;
  vector<int> mergeIndex;
  int cnt= 0, totalNodesNum= Accumlator.GetNoComponents();
  
  list<CompressedMSet*>* resStream= new list<CompressedMSet*>(0);
  finalResStream= new list<CompressedMSet*>(0);
  list<CompressedInMemUSet>::iterator cur= begin;
  LWGraph* graph= new LWGraph();
  list<Component*>* components= new list<Component*>();
  map<int, list<Component*>::iterator> compLabelsMap;
  int NextComponentLabel= 0;
  
  while(cur != end)
  {
    if(debugme )
    {
      cerr<<"\n\nProcessing unit number: " <<++cnt <<"/" << totalNodesNum;
      cerr<<". Unit has: " <<(*cur).count<<" elems";
      cerr<<"\nresStream has: " <<resStream->size();
      cerr<<" results, finalResStream has: " <<finalResStream->size();
    }
    DynamicGraphAppend(graph, components, compLabelsMap, cur, ids, d, n, qts, 
        NextComponentLabel,resStream, finalResStream);
    ++cur;
  }
  //finalize the components
  --cur;
  for(list<Component*>::iterator compIt= components->begin(); compIt!=
    components->end(); ++compIt)
    (*compIt)->ExtendResStreamsTillNow((*cur).endtime, (*cur).rc);
  ++cur;
  
  resStream->remove_if(bind2nd(checkShortDelete(), d));
  finalResStream->splice(finalResStream->end(), *resStream);
  if(debugme )
  {
    cerr<< "\nfinalResStream contains "<< 
      finalResStream->size() << " results\n";
    for(list<CompressedMSet*>::iterator 
        it= finalResStream->begin(); it != finalResStream->end(); ++it);
      //(*it)->Print(cerr);
  }
  for(list<Component*>::iterator it= components->begin(); it!= 
    components->end(); ++it)
    delete *it;
  delete components;
  delete graph;
  delete resStream;
}

/*
SetIntersects

*/ 
static bool SetIntersects(set<int> &set1, set<int> &set2)
{
  if(set1.empty() || set2.empty()) return false;
  set<int>::iterator first1= set1.begin(), first2= set2.begin(),
    last1 = set1.end(), last2 = set2.end();
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

static void CheckAdd( LWGraph* g, NewComponent& comp, int n , 
    list<Component*>* components, 
    map<int, list<Component*>::iterator>& compLabelsMap, 
    int& NextComponentLabel)
{
  if((comp.newNodes.size()* 1.0) >= n)
  {
    Component* newComp= new Component(); 
    newComp->message = Component::NewlyAdded;
    newComp->resStreams.clear();
    newComp->label= NextComponentLabel++;
    newComp->nodes.insert(comp.newNodes.begin(), comp.newNodes.end());
    components->push_back(newComp);
    compLabelsMap[newComp->label]= --components->end();
    SetGraphNodesComponent(g, comp.newNodes, newComp->label);
  }
}

static void UpdateMerge(LWGraph *g, NewComponent& newComp, set<int>& newEdges, 
    vector<pair<int, int> >& ids, list<Component*>* components, 
    map<int, list<Component*>::iterator>& compLabelsMap)
{
  map<int, list<Component*>::iterator>::iterator affectedCompMap= 
    compLabelsMap.find(newComp.affectedComponents[0]);
  assertIf(affectedCompMap !=  compLabelsMap.end());
  list<Component*>::iterator affectedComp= (*affectedCompMap).second;
  
  assertIf(affectedComp != components->end());
  assertIf((*affectedComp)->addedEdges.empty());
  
  for(set<int>::iterator newNodesIt= newComp.newNodes.begin(); newNodesIt!= 
    newComp.newNodes.end(); ++newNodesIt)
  {
    set<pair<int,int> >* newNodeNeighbors=  &(*(g->node_index[*newNodesIt]));
    for(set<pair<int,int> >::iterator neis= newNodeNeighbors->begin(); neis!=
      newNodeNeighbors->end(); ++neis)
      (*affectedComp)->addedEdges.insert((*neis).second);
  }
  pair<int,int>* edgeNodes;
  for(set<int>::iterator newEdge= newEdges.begin(); newEdge!= 
    newEdges.end(); ++newEdge)
  {
    edgeNodes= &ids[*newEdge];
    if(((*affectedComp)->nodes.find(edgeNodes->first) != 
            (*affectedComp)->nodes.end()) 
      ||
      ((*affectedComp)->nodes.find(edgeNodes->second) != 
            (*affectedComp)->nodes.end()))
      (*affectedComp)->addedEdges.insert(*newEdge);
  }
  
  if(newComp.newNodes.empty())
    assert((*affectedComp)->UpdateMessage(Component::NewlyAddedEdges));
  else
  {
    assert((*affectedComp)->UpdateMessage(Component::NewlyAddedNodes));
    (*affectedComp)->nodes.insert(
        newComp.newNodes.begin(), newComp.newNodes.end());
    for(set<int>::iterator nodeIt= newComp.newNodes.begin(); nodeIt !=
      newComp.newNodes.end(); ++nodeIt)
    {
      assertIf(g->node_component.find(*nodeIt) == g->node_component.end());
      g->node_component.insert(make_pair(*nodeIt, (*affectedComp)->label));
    }
  }  
}

static void MergeComponents(LWGraph* g, NewComponent& newComp, 
    list<Component*>* components, 
    map<int, list<Component*>::iterator>& compLabelsMap, 
    int& NextComponentLabel)
{
  vector< list<Component*>::iterator> affectedComponents;
  map<int, list<Component*>::iterator>::iterator affectedCompMap;
  list<Component*>::iterator affectedComp;

  for(unsigned int i=0; i< newComp.affectedComponents.size(); ++i)
  {
    affectedCompMap= compLabelsMap.find(newComp.affectedComponents[i]);
    assertIf(affectedCompMap !=  compLabelsMap.end());
    affectedComp= (*affectedCompMap).second;
    affectedComponents.push_back(affectedComp);
  } 
  
  Component* bigComp= new Component();
  for(unsigned int i=0; i< affectedComponents.size(); ++i)
    bigComp->Union(*affectedComponents[i]);
    //a union of graphs and resStreams
  if(! newComp.newNodes.empty())
  {
    bigComp->nodes.insert(newComp.newNodes.begin(), newComp.newNodes.end());
    assert(bigComp->UpdateMessage(Component::NewlyAddedNodes));
  }
  bigComp->label= NextComponentLabel++;
  assert(bigComp->UpdateMessage(Component::MergedFromExistingComponents));
  for(unsigned int i=0; i< affectedComponents.size(); ++i)
  {
    compLabelsMap.erase((*affectedComponents[i])->label);
    delete *affectedComponents[i];
    components->erase(affectedComponents[i]);
  }
  
  for(set<int>::iterator nodeIt= bigComp->nodes.begin(); nodeIt !=
    bigComp->nodes.end(); ++nodeIt)
    g->node_component[*nodeIt] = bigComp->label;
  
  components->push_back(bigComp);
  compLabelsMap[bigComp->label]= --components->end();
}

static void UpdateRemove(LWGraph* graph, list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    vector<intpair>& ids, int n, int& NextComponentLabel, 
    set< int>& affectedComponentsLabels)
{
  list<Component*>::iterator affectedComponentIt;
  pair<int, int>* edgeNodes;
  list<set<int> > splitComponents;
  set<int>* splitComponent;
  bool disconnected= false;
  list<Component*>::iterator affectedComponentsIt;
  map<int, list<Component*>::iterator>::iterator affectedCompMap;
  
  for(set<int>::iterator affectedComponentLabelIt= 
    affectedComponentsLabels.begin(); affectedComponentLabelIt!= 
      affectedComponentsLabels.end(); ++affectedComponentLabelIt)
  {
    affectedCompMap= compLabelsMap.find(*affectedComponentLabelIt);
    assertIf(affectedCompMap !=  compLabelsMap.end());
    affectedComponentIt= (*affectedCompMap).second;
    assertIf(affectedComponentIt != components->end());
    (*affectedComponentIt)->SynchronizeNodes(graph);
    if((*affectedComponentIt)->nodes.size() < (unsigned int) n)
    {
      assert((*affectedComponentIt)->UpdateMessage(Component::RemoveNow));
      RemoveGraphNodesComponent(graph, (*affectedComponentIt)->nodes);
        continue;
    }
    
    disconnected= false;
    for(set<int>::iterator edgeIt= 
      (*affectedComponentIt)->removedEdges.begin(); edgeIt!=
        (*affectedComponentIt)->removedEdges.end() && !disconnected; ++edgeIt)
    {
      edgeNodes= &ids[*edgeIt];
      disconnected= disconnected || !(*affectedComponentIt)->IsConnected(graph,
          edgeNodes->first, edgeNodes->second);
    }
    if(disconnected)
    {
      splitComponents.clear();
      (*affectedComponentIt)->Cluster(graph, splitComponents);
      list<set<int> >::iterator splitComponentIt= splitComponents.begin();
      while(splitComponentIt != splitComponents.end())
      {
        splitComponent= &(*splitComponentIt);
        if(splitComponent->size()< (unsigned int) n)
        {
          RemoveGraphNodesComponent(graph, *splitComponent);
          splitComponents.erase(splitComponentIt++);
        }
        else
          ++splitComponentIt;       
      }
      
      if(splitComponents.empty())
      {
        //a deletion of a complete component
        assert((*affectedComponentIt)->UpdateMessage(Component::RemoveNow));
      }
      else if(splitComponents.size() ==1 )
      {
        splitComponent= &(*splitComponents.begin());
        assert((*affectedComponentIt)->UpdateMessage(Component::RemovedNodes));
        vector<int> removedNodes(
            (*affectedComponentIt)->nodes.size()- (*splitComponent).size()+1);
        vector<int>::iterator removedNodesEndIt=
          set_difference(
            (*affectedComponentIt)->nodes.begin(), 
            (*affectedComponentIt)->nodes.end(),
            (*splitComponent).begin(), (*splitComponent).end(), 
            removedNodes.begin());
        
        int curNode;
        list< set< pair<int, int> > >::iterator curNodeNeighborsIt;
        set< pair<int, int> >* curNodeNeighbors;
        
        for(vector<int>::iterator rn= removedNodes.begin(); rn!= 
          removedNodesEndIt; ++rn)
        {
          curNode= *rn;
          (*affectedComponentIt)->nodes.erase(curNode);
          curNodeNeighborsIt= graph->node_index[curNode];
          curNodeNeighbors= &(*curNodeNeighborsIt);
          for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); 
            nIt!= curNodeNeighbors->end(); ++nIt)
            (*affectedComponentIt)->removedEdges.insert((*nIt).second); 
        }
      }
      else//   If(splitComponents.count > 1)
      {
        //a split of a component
        list<set<int> >::iterator splitComponentIt= splitComponents.begin();
        splitComponent= &(*splitComponentIt);
        assert((*affectedComponentIt)->UpdateMessage(
            Component::SplitFromExtistingComponent));
        (*affectedComponentIt)->nodes.clear();
        (*affectedComponentIt)->nodes = *splitComponent ;
        ++splitComponentIt;
        while(splitComponentIt != splitComponents.end())
        {
          splitComponent= &(*splitComponentIt);
          Component* newComponent= new Component();
          newComponent->SetMessage((*affectedComponentIt)->message);
          newComponent->nodes= *splitComponent;
          newComponent->resStreams.assign(
              (*affectedComponentIt)->resStreams.begin(), 
              (*affectedComponentIt)->resStreams.end());
          newComponent->label= NextComponentLabel++;
          UpdateGraphNodesComponent(
              graph, *splitComponent, newComponent->label);
          components->push_back(newComponent);
          compLabelsMap[newComponent->label]= --components->end();
          ++splitComponentIt;
        }
      }
    }
  }
}


/*
Finalize

*/ 
static void Finalize(LWGraph* graph, list<Component*>* components,double d, 
    CompressedInMemUSet& cur, list<CompressedMSet*>* resStream, 
    list<CompressedMSet*>* finalResStream, vector<intpair>& ids)
{
  bool debugme= false;
  
//  list<list<CompressedMSet*>::iterator> remainingResStreams;
//  for(list<CompressedMSet*>::iterator res= resStream->begin(); res!= 
//    resStream->end(); ++res)
//    remainingResStreams.push_back(res);
  
  vector<list<Component*>::iterator> compsToErase;  
  list<list<CompressedMSet*>::iterator> resToErase;
  for(list<Component*>::iterator compIt= components->begin(); compIt!=
    components->end(); ++compIt)
  {
    Component* comp= *compIt;
    if(debugme)
      comp->Print(cerr);
    switch(comp->message)
    {
    case Component::NotChanged:
    case Component::NoMessage:  
    {
      comp->ExtendedTillLastChange= false;
      //comp->Reset();
//      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
//        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
//      {
//					remainingResStreams.remove(*res);
//        mset::CompressedMSet* theMSet= **res;
//        theMSet->units.Get(theMSet->GetNoComponents()-1, theUSet);
//        assert(AlmostEqual(theUSet.endtime, cur.starttime) && 
//            (theUSet.rc || cur.lc));
//        theUSet.endtime= cur.endtime;
//        theUSet.rc= cur.rc;
//        theMSet->units.Put(theMSet->GetNoComponents()-1, theUSet);
//      }
    }break;
    case Component::NewlyAddedEdges:
    case Component::RemovedEdges:
    case Component::NewlyAddedNodes:
    case Component::RemovedNodes:
    case Component::AddRemoveMix:
    {
      comp->ExtendResStreamsTillNow(cur.starttime, !cur.lc);
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        CompressedMSet* theMSet= **res;
        theMSet->AddUnit(comp->addedEdges, comp->removedEdges, cur.starttime, 
            cur.endtime, cur.lc, cur.rc);
        if(debugme)
          if(!IsOneComponent(theMSet, 0, ids))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//        remainingResStreams.remove(*res);
      }   
      comp->ExtendedTillLastChange= true;
      comp->Reset();
    }break;
    case Component::NewlyAdded:
    {
      set<int> compEdges;
      comp->GetEdges(graph, compEdges);
      CompressedMSet* newMSet= new CompressedMSet(0);
      newMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
      resStream->push_back(newMSet);
      list<CompressedMSet*>::iterator newMSetIt= --resStream->end(); 
      comp->resStreams.push_back(newMSetIt);
      comp->ExtendedTillLastChange= true;
      if(debugme)
        if(!IsOneComponent(newMSet, 0, ids))
          cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
      comp->Reset();
    }break;
    case Component::SplitFromExtistingComponent:
    {
      comp->ExtendResStreamsTillNow(cur.starttime, !cur.lc);
      list< list<mset::CompressedMSet*>::iterator > resStreams;
      set<int> compEdges;
      comp->GetEdges(graph, compEdges);
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        CompressedMSet* theMSet= **res;
        CompressedMSet* newMSet= new CompressedMSet(*theMSet);
        newMSet->MergeAdd(compEdges, cur.starttime, cur.endtime,
            cur.lc, cur.rc);
        resStream->push_back(newMSet);
        list<CompressedMSet*>::iterator newMSetIt= --resStream->end(); 
        resStreams.push_back(newMSetIt);
        resToErase.push_back(*res);
        if(debugme)
          if(!IsOneComponent(newMSet, 0, ids))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//        remainingResStreams.remove(*res);
      }  
      comp->resStreams.clear();
      comp->resStreams.resize(resStreams.size());
      copy(resStreams.begin(), resStreams.end(), comp->resStreams.begin());
      comp->ExtendedTillLastChange= true;
      comp->Reset();
      
    }break;
    case Component::MergedFromExistingComponents:
    {
      comp->ExtendResStreamsTillNow(cur.starttime, !cur.lc);
      set<int> compEdges;
      comp->SynchronizeNodes(graph);
      comp->GetEdges(graph, compEdges);
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        CompressedMSet* theMSet= **res;
        theMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
        if(debugme)
          if(!IsOneComponent(theMSet, 0, ids))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//        remainingResStreams.remove(*res);
      }
      comp->ExtendedTillLastChange= true;
      comp->Reset();
    }break;
    case Component::RemoveNow:
    {
      comp->ExtendResStreamsTillNow(cur.starttime, !cur.lc);
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        CompressedMSet* theMSet= **res;
        if(theMSet->DurationLength() >= d)
        {
          if(debugme && 0)
          {
            cerr<<"Moving the following mset from resStream to finalResStream:";
            theMSet->Print(cerr);
          }
          finalResStream->push_back(theMSet);
        }
        else
          delete theMSet;
        resStream->erase(*res);
//        remainingResStreams.remove(*res);
      }
      delete comp;
      compsToErase.push_back(compIt);
    }break;
    case Component::ReDistribute: //very unexpected case
    {
      comp->ExtendResStreamsTillNow(cur.starttime, !cur.lc);
      list< list<mset::CompressedMSet*>::iterator > resStreams;
      bool matched=false;
      set<int> compEdges;
      comp->GetEdges(graph, compEdges);
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        
        CompressedMSet* theMSet= **res;
        set<int>* resFinalEdgeSet= theMSet->GetFinalSet(), resFinalNodeSet;
        EdgeSet2NodeSet(*resFinalEdgeSet, resFinalNodeSet, ids);
        if(debugme)
        {
          cerr<<endl;
          PrintSet(*resFinalEdgeSet, cerr);
          cerr<<endl;
          PrintSet(resFinalNodeSet, cerr);
        }
        if(SetIntersects(comp->nodes, resFinalNodeSet))
        {
          matched= true;
          CompressedMSet* newMSet= new CompressedMSet(*theMSet);
          newMSet->MergeAdd(compEdges, cur.starttime, cur.endtime,
              cur.lc, cur.rc);
          resStream->push_back(newMSet);
          list<CompressedMSet*>::iterator newMSetIt= --resStream->end(); 
          resStreams.push_back(newMSetIt);
          resToErase.push_back(*res);
          if(debugme)
            if(!IsOneComponent(newMSet, 0, ids))
              cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
          //        remainingResStreams.remove(*res);
        }
      }  
      
      comp->resStreams.clear();
      comp->resStreams.resize(resStreams.size());
      copy(resStreams.begin(), resStreams.end(), comp->resStreams.begin());
      comp->ExtendedTillLastChange= true;
      comp->Reset();
      
      if(!matched) // a new component that is split from an existing component
        //this is an extremely unexpected case that should not happen under
        //normal moving objects scenarios
      {
        CompressedMSet* newMSet= new CompressedMSet(0);
        newMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
        resStream->push_back(newMSet);
        list<CompressedMSet*>::iterator newMSetIt= --resStream->end(); 
        comp->resStreams.push_back(newMSetIt);
        comp->ExtendedTillLastChange= true;
        if(debugme)
          if(!IsOneComponent(newMSet, 0, ids))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
      }
    }break;
    default:
    {
      assert(0);
    }
    
    };
  }


  while(resToErase.begin() != resToErase.end())
  {
//    remainingResStreams.remove(*resToErase.begin());
    resStream->erase(*resToErase.begin());
    resToErase.remove(*resToErase.begin());
  }
//  assert(remainingResStreams.empty());
  for(unsigned int i=0; i<compsToErase.size(); ++i)
    components->erase(compsToErase[i]);
    
}

static void DynamicGraphAppend(LWGraph* graph, list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    list<CompressedInMemUSet>::iterator cur, vector<intpair>& ids, double d, 
    int n, string& qts, int& NextComponentLabel, 
    list<CompressedMSet*>*& resStream, list<CompressedMSet*>*& finalResStream)
{
  bool debugme= false;
   
  if(debugme)
    graph->print(cerr);
  
  if(!(*cur).added.empty())
  {
    vector<NewComponent> newComponents;
    InsertEdgesUndirected(graph, (*cur).added, ids);
    FindComponentsOf(graph, components, (*cur).added, ids, newComponents);

    for(vector<NewComponent>::iterator newComp= 
      newComponents.begin(); newComp!= newComponents.end(); ++newComp)
    {
      if(debugme)
        (*newComp).Print(cerr);
      if( (*newComp).affectedComponents.empty())
        CheckAdd( graph, *newComp, n , components, 
            compLabelsMap, NextComponentLabel);

      else if( (*newComp).affectedComponents.size() == 1) 
        UpdateMerge(graph, *newComp, (*cur).added, ids, components, 
            compLabelsMap);
      //new edges/nodes are added to an existing component without causing a 
      //merge with other component. This causes no change.

      else if( (*newComp).affectedComponents.size() > 1) 
        MergeComponents(graph, *newComp, components, compLabelsMap, 
            NextComponentLabel);
      //the added edges caused some components to merge together.
      //delete *newComp;
    }
    //delete newComponents;
  }

  if(!(*cur).removed.empty())
  {

    set<int> affectedComponentsLabels;
    list<Component*>::iterator affectedComponentIt;
    pair<int, int>* edgeNodes;
    int componentLabel;
    map<int, list<Component*>::iterator>::iterator affectedCompMap;
    for(set<int>::iterator edgeIt= (*cur).removed.begin(); edgeIt!= 
      (*cur).removed.end(); ++edgeIt)
    {
      edgeNodes= &ids[*edgeIt];
      if(debugme)
        cerr<<"\n Removing edge ("<<edgeNodes->first <<", "<<edgeNodes->second
        <<")";
      componentLabel= FindEdgeComponent(graph, edgeNodes);
      RemoveEdgeUndirected(graph, *edgeIt, edgeNodes);
      if(componentLabel != -1)
      {
        affectedCompMap= compLabelsMap.find(componentLabel);
        assertIf(affectedCompMap !=  compLabelsMap.end());
        affectedComponentIt= (*affectedCompMap).second;
        assertIf(affectedComponentIt != components->end());
        affectedComponentsLabels.insert(componentLabel);
        (*affectedComponentIt)->removedEdges.insert(*edgeIt);
        assert((*affectedComponentIt)->UpdateMessage(Component::RemovedEdges));
      }
    }

    if(!affectedComponentsLabels.empty())
      UpdateRemove(graph, components, compLabelsMap, ids, n, 
          NextComponentLabel, affectedComponentsLabels);
  }

  Finalize(graph, components, d, *cur, resStream, finalResStream, ids);

}

static void GraphNodes2Edges(set<int>& subGraphNodes, set<int>& graphEdges, 
    vector<intpair>& edge2Nodes, set<int>& res)
{
  res.clear();
  intpair edge;
  for(set<int>::iterator it= graphEdges.begin(); it!= graphEdges.end(); ++it)
  {
    edge= edge2Nodes[*it];
    if(subGraphNodes.find(edge.first) != subGraphNodes.end() &&
        subGraphNodes.find(edge.second) != subGraphNodes.end())
      res.insert(*it);
  }

}

static bool Merge(CompressedInMemMSet *_mset, set<int> *subGraph, 
    double starttime, double endtime, bool lc, bool rc)
{
  set<int>* finalSet= _mset->GetFinalSet();
  bool intersects = SetIntersects(*finalSet, *subGraph);
  if(intersects)
  {
    _mset->MergeAdd(*subGraph, starttime, endtime, lc, rc);
    return true;
  }
  return false;
}


static void SetAddRemove(set<int>& _set, set<int>& _add, set<int>& _remove)
{
  _set.insert(_add.begin(), _add.end());
  for(set<int>::iterator it= _remove.begin(); it!= _remove.end(); ++it)
    _set.erase(*it);
}
  
static void EdgeSet2NodeSet(
    set<int> &edges, set<int> &nodes, vector<intpair> ids)
{
  intpair edge;
  nodes.clear();
  for(set<int>::iterator it= edges.begin(); it != edges.end(); ++it)
  {
    edge= ids[*it];
    nodes.insert(edge.first);
    nodes.insert(edge.second);
  }
}
  
static CompressedMSet* 
EdgeMSet2NodeMSet(CompressedMSet* edgeMSet, vector<intpair>& ids)
{
  //bool debugme= false;
  LWGraph graph;
  CompressedMSet* nodeMSet= new CompressedMSet(0);
  CompressedUSetRef edgeUnit(true), nodeUnit(true);
  set<int> newNodes, removedNodes, curNodeSet;
  int numUnits= edgeMSet->GetNoComponents();
  int elem;
  if(edgeMSet == 0 || numUnits == 0) return nodeMSet;

  set<int> toAdd, toRemove;

  edgeMSet->units.Get( 0, edgeUnit );
  for(int j=edgeUnit.addedstart; j<= edgeUnit.addedend; ++j)
  {
    edgeMSet->added.Get(j, elem);
    toAdd.insert(elem);
  }
  InsertEdgesUndirected(&graph, toAdd, ids, newNodes);
  nodeUnit.starttime= edgeUnit.starttime; nodeUnit.lc= edgeUnit.lc;
  nodeUnit.addedstart=0; nodeUnit.removedstart=0;
  for(set<int>::iterator it= newNodes.begin(); it!=newNodes.end(); ++it)
  {
    nodeMSet->added.Append(*it);
  }
  for(int i= 1; i< edgeMSet->units.Size(); ++i)
  {
    edgeMSet->units.Get( i, edgeUnit );
    toAdd.clear(); toRemove.clear();
    for(int j=edgeUnit.addedstart; j<= edgeUnit.addedend; ++j)
    {
      edgeMSet->added.Get(j, elem);
      toAdd.insert(elem);
    }
    for(int j=edgeUnit.removedstart; j<= edgeUnit.removedend; ++j)
    {
      edgeMSet->removed.Get(j, elem);
      toRemove.insert(elem);
    }
    InsertEdgesUndirected(&graph, toAdd, ids, newNodes); 
    RemoveEdgesUndirected(&graph, toRemove, ids, removedNodes);

    if(! newNodes.empty() || ! removedNodes.empty())
    {
      nodeUnit.addedend= nodeMSet->added.Size()-1;
      nodeUnit.removedend= nodeMSet->removed.Size()-1;
      nodeUnit.endtime= edgeUnit.starttime;
      nodeUnit.rc= ! edgeUnit.lc;
      nodeMSet->units.Append(nodeUnit);

      nodeUnit.starttime= edgeUnit.starttime; nodeUnit.lc= edgeUnit.lc;
      nodeUnit.addedstart= nodeMSet->added.Size(); 
      nodeUnit.removedstart= nodeMSet->removed.Size();
    }
    if(!newNodes.empty())
    {
      for(set<int>::iterator it= newNodes.begin(); it!=newNodes.end(); ++it)
        nodeMSet->added.Append(*it);
    }
    if(!removedNodes.empty())
    {
      for(set<int>::iterator it= removedNodes.begin(); it!=
        removedNodes.end(); ++it)
        nodeMSet->removed.Append(*it);
    }
  }

  nodeUnit.addedend= nodeMSet->added.Size()-1;
  nodeUnit.removedend= nodeMSet->removed.Size()-1;
  nodeUnit.endtime= edgeUnit.endtime;
  nodeUnit.rc= edgeUnit.rc;
  nodeMSet->units.Append(nodeUnit);

  return nodeMSet;
}
  
static void ComputeAddSubSets(InMemMSet& acc,
    list<InMemUSet>::iterator t1, list<InMemUSet>::iterator t2,
    unsigned int n,  double d, vector<InMemMSet>* result)
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
          lc, rc, n, res);

    while( curInstant < endInstant && candidates.Count() >=n &&
        unitIterator2 != t2)
    {
      curUnit= ++unitIterator2;
      curInstant= (*curUnit).endtime;
      rc= (*curUnit).rc;
      candidates.Intersection( (*curUnit).constValue);
      if(candidates.Count() >= n )
        AddAllSubSetsToVector(candidates, startInstant, curInstant, 
            lc, rc, n, res);
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
  //result.reserve(res.size());
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
    result->push_back(mset);
  }
  if(debugme)
  {
    cerr<<result->size(); 
  }
}
/*
Private members

*/ 
private:
  static void GenerateAllCombinations(InMemUSet& cand, int select, 
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
      
  static void AddAllSubSetsToVector(InMemUSet& candidates, double startInstant, 
      double curInstant, bool lc, bool rc, 
      int n, multimap< set<int>, DoubleInterval>& res)
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
    
    GenerateAllCombinations(candidates, n, sets);
    changed = (sets.size() != 0);
    
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
  vector< vector< pair< Interval<CcReal>, MSet* > > > SA;
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
  Interval<CcReal> nullInterval;
    
/*
A list of the variable that have been consumed so far.

*/
    vector<int> assignedVars;
    
    
    GPatternSolver():count(0),iterator(-1), 
    nullInterval(CcReal(true,0.0),CcReal(true,0.0), true,true)
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
  bool IsSupported(vector< pair<Interval<CcReal>, MSet* > >& sa, int index);

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

};



} // namespace GPattern



#endif 

