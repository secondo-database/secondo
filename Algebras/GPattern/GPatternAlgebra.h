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
#include "igraph/igraph.h"
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



class GPatternHelper
{
public:

GPatternHelper(){}
~GPatternHelper() {}

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


template<class MSetClass, class IteratorClass>
static void FindSubGraphs(MSetClass& Accumlator,
    IteratorClass begin, IteratorClass end , 
    vector<intpair>& ids, double d, int n, string& qts, 
    list<CompressedMSet*>*& finalResStream)
{
  bool debugme= false;
  IteratorClass cur= begin;
  set<int> s;
  list<CompressedMSet*>* resStream= 0, *localResStream= 0;
  finalResStream= new list<CompressedMSet*>(0);

  FindDynamicComponents(Accumlator, begin, end, ids, d, n, qts, resStream);

  bool locallyChanged= true, globallyChanged= false;
  CompressedInMemMSet* curInMemMSet= new CompressedInMemMSet();
  CompressedMSet* curMSet=0;
  while(resStream->begin() != resStream->end())
  {
    curMSet= resStream->front();
    curMSet->WriteToCompressedInMemMSet(*curInMemMSet);
    if(debugme)
      curInMemMSet->Print(cerr);
    locallyChanged=true; globallyChanged= false;
    while(locallyChanged)
    {
      locallyChanged= curInMemMSet->RemoveSmallUnits(n);
      locallyChanged= 
        (locallyChanged || curInMemMSet->RemoveShortElemParts(d) );
      globallyChanged= (globallyChanged || locallyChanged);
    }
    if(debugme)
      curInMemMSet->Print(cerr);
    if(globallyChanged)
    {     
      list<CompressedInMemUSet>::iterator begin2= 
        curInMemMSet->units.begin(), end2, tmp;
      while(begin2 != curInMemMSet->units.end())
      {
        end2= curInMemMSet->GetPeriodEndUnit(begin2);
        if(debugme)
        {
          (*begin2).Print(cerr);
          (*end2).Print(cerr);
        }
        if((*end2).endtime - (*begin2).starttime >= d)
        {
          ++end2;
          GPatternHelper::FindSubGraphs<CompressedInMemMSet, 
            list<CompressedInMemUSet>::iterator>(
              *curInMemMSet, begin2, end2 , ids, d, n, qts, localResStream);
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
  delete resStream;
  delete curInMemMSet;
}
  
//static void FindDynamicComponents(CompressedInMemMSet& Accumlator,
//    list<CompressedInMemUSet>::iterator begin, 
//    list<CompressedInMemUSet>::iterator end , 
//    vector<intpair>& ids, double d, int n, string qts, 
//    list<CompressedInMemMSet*>*& finalResStream)
//{
//  bool debugme= false;  
//  set<int> constValue;
//  map<int, int> _map;
//  vector<int> mergeIndex;
//  double starttime, endtime;
//  bool lc, rc;
//  int cnt= 0;
//  
//  list<CompressedInMemMSet*>* resStream= new list<CompressedInMemMSet*>(0);
//  finalResStream= new list<CompressedInMemMSet*>(0);
//  list<CompressedInMemUSet>::iterator cur= begin;  
//      
//  if(debugme)
//    Accumlator.Print(cerr);
//    
//  while(cur != end)
//  {
//    cerr<<endl<< "Processing unit number: " <<++cnt;
//    starttime= (*cur).starttime; endtime= (*cur).endtime;
//    lc= (*cur).lc; rc= (*cur).rc;
//    
//    DynamicGraphAppend((*cur).constValue, starttime, endtime, lc, rc,
//        ids, d, n, qts, resStream, finalResStream);
//    ++cur;
//  }
//  cerr<<"\n FindDynamicComponents finished successfully ";  
//  if(debugme)
//  {
//    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
//    for(list<CompressedInMemMSet*>::iterator 
//        it= resStream->begin(); it != resStream->end(); ++it)
//      (*it)->Print(cerr);
//  }
//  resStream->remove_if(bind2nd(checkShortDelete(), d));
//  if(debugme)
//  {
//    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
//    for(list<CompressedInMemMSet*>::iterator 
//        it= resStream->begin(); it != resStream->end(); ++it)
//      (*it)->Print(cerr);
//  }
//  finalResStream->splice(finalResStream->end(), *resStream);
//  if(debugme)
//  {
//    cerr<< "\nfinalResStream contains "<< 
//      finalResStream->size() << " results\n";
//    for(list<CompressedInMemMSet*>::iterator 
//        it= finalResStream->begin(); it != finalResStream->end(); ++it)
//      (*it)->Print(cerr);
//  }
//  delete resStream;
//}

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
  bool debugme= false;
  set<int> constValue;
  map<int, int> _map;
  vector<int> mergeIndex;
  double starttime, endtime;
  bool lc, rc;
  int cnt= 0;
  
  list<CompressedMSet*>* resStream= new list<CompressedMSet*>(0);
  finalResStream= new list<CompressedMSet*>(0);
  list<CompressedInMemUSet>::iterator cur= begin;
  LWGraph* graph= new LWGraph();
  list<Component*>* components= new list<Component*>();
  
  while(cur != end)
  {
    cerr<<"\n\nProcessing unit number: " <<++cnt <<" unit has: "
      <<(*cur).count<<" elems";
    cerr<<"\nresStream has: " <<resStream->size();
    cerr<<" results, finalResStream has: " <<finalResStream->size();
//    Accumlator.GetSet(cur, constValue);
//    starttime= (*cur).starttime; endtime= (*cur).endtime;
//    lc= (*cur).lc; rc= (*cur).rc;
    DynamicGraphAppend(graph, components, cur, ids, d, n, qts, 
        resStream, finalResStream);
    if(debugme)
    {
      cerr<< "\nresStream contains "<< resStream->size() << " results\n";
      for(list<CompressedMSet*>::iterator 
          it= resStream->begin(); it != resStream->end(); ++it)
        (*it)->Print(cerr);
    }
    ++cur;
  }
  cerr<<"\n FindDynamicComponents finished successfully ";
  if(debugme)
  {
    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
    for(list<CompressedMSet*>::iterator 
        it= resStream->begin(); it != resStream->end(); ++it)
      (*it)->Print(cerr);
  }
  resStream->remove_if(bind2nd(checkShortDelete(), d));
  if(debugme)
  {
    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
    for(list<CompressedMSet*>::iterator 
        it= resStream->begin(); it != resStream->end(); ++it)
      (*it)->Print(cerr);
  }
  finalResStream->splice(finalResStream->end(), *resStream);
  if(debugme)
  {
    cerr<< "\nfinalResStream contains "<< 
      finalResStream->size() << " results\n";
    for(list<CompressedMSet*>::iterator 
        it= finalResStream->begin(); it != finalResStream->end(); ++it)
      (*it)->Print(cerr);
  }
  delete resStream;
}

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

static void CheckAdd(Component* newComp, int n , list<Component*>* components)
{
  if(newComp->graph->nodes_count() >= n)
  {
    newComp->message = Component::NewlyAdded;
    newComp->resStreams.clear();
    components->push_back(newComp);
  }
  else
    delete newComp;
}

static void UpdateMerge(set<int>& newEdges, vector<intpair>& ids, 
    vector< list<Component*>::iterator>* affectedComponents)
{
  Component* comp= *(*affectedComponents)[0];
  int numNodesBeforeInsert= comp->graph->nodes_count();
  InsertEdgesUndirected(comp->graph, newEdges, ids);
  int numNodesAfterInsert= comp->graph->nodes_count();
  
  if(numNodesBeforeInsert != numNodesAfterInsert)
    comp->UpdateMessage( Component::NewlyAddedNodes );
  else
    comp->UpdateMessage( Component::NewlyAddedEdges );
}

static void MergeComponents(Component* newComp, 
    vector< list<Component*>::iterator>* affectedComponents, 
    list<Component*>* components)
{
  Component* bigComp= new Component();
  for(unsigned int i=0; i< affectedComponents->size(); ++i)
    bigComp->Union( *(*affectedComponents)[i]);
    //a union of graphs and resStreams
    
  for(unsigned int i=0; i< affectedComponents->size(); ++i)
  {
    delete *(*affectedComponents)[i];
    components->erase((*affectedComponents)[i]);
  }
  
  components->push_back(bigComp);
}

static void UpdateRemove(Component* delComp, int n, 
    vector< list<Component*>::iterator>* affectedComponents, 
    list<Component*>* components)
{
  Component* comp= *(*affectedComponents)[0];
  int sizeBeforeDel= comp->graph->nodes_count();
  comp->RemoveEdgesFrom(delComp);
  int sizeAfterDel= comp->graph->nodes_count();
  vector< LWGraph* >* newComponents= comp->graph->cluster();
  
  vector< LWGraph* >::iterator it= --newComponents->end(); 
  while(!newComponents->empty() && it >= newComponents->begin())
  {
    if((*it)->nodes_count() < n)
    {
      delete *it; 
      newComponents->erase(it--);
    }
    else
      --it;
  }
  
  if(newComponents->size() == 1)
  {
    //simple edge/node removal
    assert(sizeBeforeDel >= sizeAfterDel);
    if(sizeBeforeDel > sizeAfterDel)
    {
      //simple node removal 
      comp->UpdateMessage(Component::RemovedNodes);
      delete comp->graph;
      comp->graph= (*newComponents)[0];
    }
    else if (sizeBeforeDel == sizeAfterDel)
    {
      //simple edge removal
      comp->UpdateMessage(Component::RemovedEdges);
      delete comp->graph;
      comp->graph= (*newComponents)[0];
    }
    else
      assert(false);
  }
  else if(newComponents->empty())
  {
    //a deletion of a complete component
    if (comp->UpdateMessage(Component::RemoveNow));
    {
      delete comp->graph;
      delete comp;
      components->erase((*affectedComponents)[0]);
    }
  }
  else if(newComponents->size() > 1)
  {
    //a split of a component
    for(unsigned int i=0; i< newComponents->size(); ++i)
    {
      Component* nC= new Component();
      nC->resStreams.resize(comp->resStreams.size());
      copy(comp->resStreams.begin(), comp->resStreams.end(), 
          nC->resStreams.begin());
      nC->SetMessage(comp->message);
      nC->UpdateMessage(Component::SplitFromExtistingComponent);
      nC->graph= (*newComponents)[i];
      components->push_back(nC);
    }
    delete comp->graph;
    delete comp;
    components->erase((*affectedComponents)[0]);
  }
  delete newComponents;
  
}

static void Finalize(list<Component*>* components,double d, 
    CompressedInMemUSet& cur, list<CompressedMSet*>* resStream, 
    list<CompressedMSet*>* finalResStream)
{
  bool debugme= true;
  list<list<CompressedMSet*>::iterator> remainingResStreams;
  for(list<CompressedMSet*>::iterator res= resStream->begin(); res!= 
    resStream->end(); ++res)
    remainingResStreams.push_back(res);
    
  for(list<Component*>::iterator it= components->begin(); it!=
    components->end(); ++it)
  {
    Component* comp= *it;
    switch(comp->message)
    {
    case Component::NotChanged:
    case Component::NewlyAddedEdges:
    case Component::RemovedEdges:
    {
      CompressedUSetRef theUSet;
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        mset::CompressedMSet* theMSet= **res;
        theMSet->units.Get(theMSet->GetNoComponents()-1, theUSet);
        assert(AlmostEqual(theUSet.endtime, cur.starttime) && 
            (theUSet.rc || cur.lc));
        theUSet.endtime= cur.endtime;
        theUSet.rc= cur.rc;
        theMSet->units.Put(theMSet->GetNoComponents()-1, theUSet);
        remainingResStreams.remove(*res);
      }
    }break;
    case Component::NewlyAddedNodes:
    case Component::RemovedNodes:
    {
      set<int> compNodes;
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        CompressedMSet* theMSet= **res;
        comp->graph->get_nodes(compNodes);
        theMSet->MergeAdd(compNodes, cur.starttime, cur.endtime,
            cur.lc, cur.rc);
        remainingResStreams.remove(*res);
      }      
    }break;
    case Component::NewlyAdded:
    {
      set<int> compNodes;
      CompressedMSet* newMSet= new CompressedMSet(0);
      comp->graph->get_nodes(compNodes);
      newMSet->AddUnit(compNodes, cur.starttime, cur.endtime,
          cur.lc, cur.rc);
      resStream->push_back(newMSet);
      list<CompressedMSet*>::iterator newMSetIt= resStream->end(); 
      --newMSetIt;
      comp->resStreams.push_back(newMSetIt);
    }break;
    case Component::SplitFromExtistingComponent:
    {
      list< list<mset::CompressedMSet*>::iterator > resStreams;
      for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
      {
        set<int> compNodes;
        CompressedMSet* theMSet= **res;
        CompressedMSet* newMSet= new CompressedMSet(*theMSet);
        comp->graph->get_nodes(compNodes);
        newMSet->MergeAdd(compNodes, cur.starttime, cur.endtime,
            cur.lc, cur.rc);
        resStream->push_back(newMSet);
        list<CompressedMSet*>::iterator newMSetIt= resStream->end(); 
        --newMSetIt;
        resStreams.push_back(newMSetIt);
        remainingResStreams.remove(*res);
      }  
      comp->resStreams.clear();
      comp->resStreams.resize(resStreams.size());
      copy(resStreams.begin(), resStreams.end(), comp->resStreams.begin());
      
    }break;
    default:
    {
      assert(0);
    }
    
    };
  }
  
  list<list<CompressedMSet*>::iterator>::iterator res=
    remainingResStreams.begin(); 
  while(res!= remainingResStreams.end())
  {
    CompressedMSet* _mset= **res;
    if(_mset->DurationLength() > d)
    {
      if(debugme)
      {
        cerr<<"Moving the following mset from resStream to finalResStream:";
        _mset->Print(cerr);
      }
      finalResStream->push_back(_mset);
    }
    else
      delete _mset;
    resStream->erase(*res);
    ++res;
  }
}

static void DynamicGraphAppend(LWGraph* graph, list<Component*>* components,
    list<CompressedInMemUSet>::iterator cur, vector<intpair>& ids, double d, 
    int n, string& qts, list<CompressedMSet*>*& resStream, 
    list<CompressedMSet*>*& finalResStream)
{
  bool debugme= false;
  set<int> *subGraph;
  map<int, int> _map;
  
  
  vector<Component*> newComponents, delComponents;
  vector< list<Component*>::iterator> affectedComponents;
  vector<int> mergeIndex;
  bool intersects;
  bool merged= false;
  
  if(debugme)
    graph->print(cerr);
  for(list<Component*>::iterator it= 
    components->begin(); it!= components->end(); ++it)
    (*it)->SetMessage(Component::NotChanged);
  
  if(!(*cur).added.empty())
  {
    InsertEdgesUndirected(graph, (*cur).added, ids);

    FindComponentsOf(graph, (*cur).added, ids, newComponents);
    for(vector<Component*>::iterator newComp= 
      newComponents.begin(); newComp!= newComponents.end(); ++newComp)
    {
      if(debugme)
        (*newComp)->graph->print(cerr);
      affectedComponents.clear();
      for(list<Component*>::iterator comp= 
        components->begin(); comp!= components->end(); ++comp)
      {
        intersects= (*comp)->Intersects( *newComp );
        if(intersects)
          affectedComponents.push_back(comp);
      } 

      if( affectedComponents.empty())
        CheckAdd(*newComp, n , components);

      else if( affectedComponents.size() == 1) 
        UpdateMerge((*cur).added, ids, &affectedComponents);
      //new edges/nodes are added to an existing component without causing a 
      //merge with other component. This causes no change.

      else if( affectedComponents.size() > 1) 
        MergeComponents(*newComp, &affectedComponents, components);
      //the added edges caused some components to merge together.
      //delete *newComp;
    }
    //delete newComponents;
  }

  if(!(*cur).removed.empty())
  {
    RemoveEdgesUndirected(graph, (*cur).removed, ids);
    Cluster((*cur).removed, ids, delComponents);
    for(vector<Component*>::iterator delComp= 
      delComponents.begin(); delComp!= delComponents.end(); ++delComp)
    {
      affectedComponents.clear();
      intersects= false;
      for(list<Component*>::iterator comp= 
        components->begin(); comp!= components->end() && !intersects; ++comp)
      {
        intersects= (*comp)->Intersects( *delComp );
        if(intersects)
          affectedComponents.push_back(comp);
      } 
      
      if(affectedComponents.empty());
        // do nothing. This is a removal of some non-compoenent edges

      else if (affectedComponents.size() == 1)
        UpdateRemove(*delComp, n, &affectedComponents, components);
        // can be a simple edge removal, a split of a component, or a 
        // deletion of a complete component
      
      else if (affectedComponents.size() > 1)
        assert(false);
      //delete *delComp;
    }
    //delete delComponents;
  }
  
  Finalize(components, d, *cur, resStream, finalResStream);

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

static int my_igraph_clusters_weak(
    const igraph_t *graph, igraph_vector_t *membership,
       igraph_vector_t *csize, igraph_integer_t *no) {

//  long int no_of_nodes=igraph_vcount(graph);
//  char *already_added;
//  long int first_node, act_cluster_size=0, no_of_clusters=1;
//  
//  igraph_dqueue_t q=IGRAPH_DQUEUE_NULL;
//  
//  long int i;
//  igraph_vector_t neis=IGRAPH_VECTOR_NULL;
//
//  already_added=igraph_Calloc(no_of_nodes,char);
//  if (already_added==0) {
//    IGRAPH_ERROR("Cannot calculate clusters", IGRAPH_ENOMEM);
//  }
//  IGRAPH_FINALLY(igraph_free, already_added);
//
//  IGRAPH_DQUEUE_INIT_FINALLY(
//    &q, no_of_nodes > 100000 ? 10000 : no_of_nodes/10);
//  IGRAPH_VECTOR_INIT_FINALLY(&neis, 0);
//
//  // Memory for result, csize is dynamically allocated 
//  if (membership) { 
//    IGRAPH_CHECK(igraph_vector_resize(membership, no_of_nodes));
//  }
//  if (csize) { 
//    igraph_vector_clear(csize);
//  }
//
//  // The algorithm 
//
//  for (first_node=0; first_node < no_of_nodes; ++first_node) {
//    if (already_added[first_node]==1) continue;
//    IGRAPH_ALLOW_INTERRUPTION();
//
//    already_added[first_node]=1;
//    act_cluster_size=1;
//    if (membership) {
//      VECTOR(*membership)[first_node]=no_of_clusters-1;
//    }
//    IGRAPH_CHECK(igraph_dqueue_push(&q, first_node));
//    
//    while ( !igraph_dqueue_empty(&q) ) {
//      long int act_node=igraph_dqueue_pop(&q);
//      IGRAPH_CHECK(igraph_neighbors(graph, &neis, act_node, IGRAPH_ALL));
//      for (i=0; i<igraph_vector_size(&neis); i++) {
//  long int neighbor=VECTOR(neis)[i];
//  if (already_added[neighbor]==1) { continue; }
//  IGRAPH_CHECK(igraph_dqueue_push(&q, neighbor));
//  already_added[neighbor]=1;
//  act_cluster_size++;
//  if (membership) {
//    VECTOR(*membership)[neighbor]=no_of_clusters-1;
//  }
//      }
//    }
//    no_of_clusters++;
//    if (csize) {
//      IGRAPH_CHECK(igraph_vector_push_back(csize, act_cluster_size));
//    }
//  }
//  
//  // Cleaning up 
//  
//  if (no) { *no = no_of_clusters-1; }
//  
//  igraph_Free(already_added);
//  igraph_dqueue_destroy(&q);
//  igraph_vector_destroy(&neis);
//  IGRAPH_FINALLY_CLEAN(3);
  
  return 0;
}

static vector<set<int> >* FindSubGraphs(
    igraph_t* g, unsigned int n, string qts, map<int, int>& _map)
{
  if(qts == "cc")
  {
    igraph_vector_t membership, csize;
    igraph_vector_init(&membership, 0);
    igraph_vector_init(&csize, 0);
    igraph_integer_t no;
    vector<set<int> >* res= 0;
    map< int, int> rmap;
    pair< set<int>::iterator, bool > insertRes;
    int nodeName, clusterName;
    for(map<int, int>::iterator it= _map.begin(); it!= _map.end(); ++it)
      rmap.insert(make_pair((*it).second, (*it).first));
    //int rc= my_igraph_clusters_weak(g, &membership, &csize, &no);
    int rc= igraph_clusters(g, &membership, &csize, &no, IGRAPH_WEAK);
    if(rc==0)
    {
      res= new vector<set<int> >(static_cast<int>(no));
      for(int i=0; i< igraph_vector_size(&membership); ++i)
      {
        nodeName= rmap[i]; 
        clusterName= VECTOR(membership)[i];
        insertRes= (*res)[clusterName].insert(nodeName);
        assert(insertRes.second);
      }
    }
    vector<set<int> >::iterator it1= res->begin();
    while( it1 != res->end()) 
    {
      if((*it1).size() < n)
        it1= res->erase(it1);
        else
          ++it1;
    }

    igraph_vector_destroy(&membership);
    igraph_vector_destroy(&csize);
    return res;
  }
  else if(qts == "clique")
  {
    return 0;
  }
}

  static igraph_t* Set2Graph(set<int>& constValue, vector<intpair>& ids, 
      map<int, int>& _map)
  {
    bool debugme= false;
    igraph_t *graph= 0;
    if(constValue.empty()) return graph;
    graph= new igraph_t();
    igraph_vector_t edges;
    igraph_vector_init(&edges, constValue.size()* 2);
    intpair edge;
    int i=0, vertix= 0;
    pair< map<int,int>::iterator, bool > insertRes;
    for(set<int>::iterator it= constValue.begin(); it!= constValue.end(); ++it)
    {
      edge= ids[*it];
      insertRes= _map.insert(make_pair(edge.first, vertix));
      if(insertRes.second) ++vertix;
      VECTOR(edges)[i++] = (*insertRes.first).second;
      
      insertRes= _map.insert(make_pair(edge.second, vertix));
      if(insertRes.second) ++vertix;
      VECTOR(edges)[i++] = (*insertRes.first).second;
      if(debugme)
        cerr<<endl<<"Edge added ("<<VECTOR(edges)[i-2]
          <<','<<VECTOR(edges)[i-1]<<')';
    }
    igraph_create(graph, &edges, 0, IGRAPH_UNDIRECTED);
    igraph_vector_destroy(&edges);
    return graph;
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
    CompressedMSet* res= new CompressedMSet(0);
    CompressedUSetRef _uset;
    set<int> curEdgeSet, curNodeSet;
    if(edgeMSet == 0 || edgeMSet->GetNoComponents() == 0) return res;
    
    edgeMSet->units.Get(0, _uset);
    edgeMSet->GetSet(0, curEdgeSet);
    EdgeSet2NodeSet(curEdgeSet, curNodeSet, ids);
    res->MergeAdd(
        curNodeSet, _uset.starttime, _uset.endtime, _uset.lc, _uset.rc);
    int i=1, elem;
    while(i < edgeMSet->units.Size())
    {
      edgeMSet->units.Get(i, _uset);
      for(int j= _uset.addedstart; j<=_uset.addedend; ++j)
      {
        edgeMSet->added.Get(j, elem);
        curEdgeSet.insert(elem);
      }
      for(int j= _uset.removedstart; j<=_uset.removedend; ++j)
      {
        edgeMSet->removed.Get(j, elem);
        curEdgeSet.erase(elem);
      }
      assert(curEdgeSet.size() == _uset.count);
      EdgeSet2NodeSet(curEdgeSet, curNodeSet, ids);
      res->MergeAdd(
          curNodeSet, _uset.starttime, _uset.endtime, _uset.lc, _uset.rc);
      ++i;
    }
    return res;
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
