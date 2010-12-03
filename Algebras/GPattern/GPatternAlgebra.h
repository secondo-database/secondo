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

template<class MSetClass, class IteratorClass>
static void FindSubGraphs(MSetClass& Accumlator,
    IteratorClass begin, IteratorClass end , 
    vector<intpair>& ids, double d, int n, string qts, 
    list<InMemMSet*>*& finalResStream)
{
  bool debugme= false;
  IteratorClass cur= begin;
  set<int> s;
  list<InMemMSet*>* resStream= 0, *localResStream= 0;
  finalResStream= new list<InMemMSet*>(0);

  FindDynamicComponents(Accumlator, begin, end, ids, d, n, qts, resStream);

  bool locallyChanged= true, globallyChanged= false;
  InMemMSet* curMSet=0;
  while(resStream->begin() != resStream->end())
  {
    curMSet= resStream->front();
    if(debugme)
      curMSet->Print(cerr);
    locallyChanged=true; globallyChanged= false;
    while(locallyChanged)
    {
      locallyChanged= curMSet->RemoveSmallUnits(n);
      locallyChanged= 
        (locallyChanged || curMSet->RemoveShortElemParts(d) );
      globallyChanged= (globallyChanged || locallyChanged);
    }
    if(debugme)
      curMSet->Print(cerr);
    if(globallyChanged)
    {     
      list<InMemUSet>::iterator begin2= curMSet->units.begin(), end2, tmp;
      while(begin2 != curMSet->units.end())
      {
        end2= curMSet->GetPeriodEndUnit(begin2);
        if(debugme)
        {
          (*begin2).Print(cerr);
          (*end2).Print(cerr);
        }
        if((*end2).endtime - (*begin2).starttime >= d)
        {
          ++end2;
          GPatternHelper::FindSubGraphs<InMemMSet, list<InMemUSet>::iterator>(
              *curMSet, begin2, end2 , ids, d, n, qts, localResStream);
          resStream->splice(resStream->end(), *localResStream);
          delete localResStream;
        }
        else
          ++end2;
        begin2= end2;
      }
      delete curMSet;
    }
    else
      finalResStream->push_back(curMSet);
    resStream->pop_front();
  }
  delete resStream;
}
  
static void FindDynamicComponents(InMemMSet& Accumlator,
    list<InMemUSet>::iterator begin, 
    list<InMemUSet>::iterator end , 
    vector<intpair>& ids, double d, int n, string qts, 
    list<InMemMSet*>*& finalResStream)
{
  bool debugme= false;  
  set<int> constValue;
  map<int, int> _map;
  vector<int> mergeIndex;
  double starttime, endtime;
  bool lc, rc;
  int cnt= 0;
  
  list<InMemMSet*>* resStream= new list<InMemMSet*>(0);
  finalResStream= new list<InMemMSet*>(0);
  list<InMemUSet>::iterator cur= begin;  
      
  if(debugme)
    Accumlator.Print(cerr);
    
  while(cur != end)
  {
    cerr<<endl<< "Processing unit number: " <<++cnt;
    starttime= (*cur).starttime; endtime= (*cur).endtime;
    lc= (*cur).lc; rc= (*cur).rc;
    
    DynamicGraphAppend((*cur).constValue, starttime, endtime, lc, rc,
        ids, d, n, qts, resStream, finalResStream);
    ++cur;
  }
  cerr<<"\n FindDynamicComponents finished successfully ";  
  if(debugme)
  {
    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
    for(list<InMemMSet*>::iterator 
        it= resStream->begin(); it != resStream->end(); ++it)
      (*it)->Print(cerr);
  }
  resStream->remove_if(bind2nd(checkShortDelete(), d));
  if(debugme)
  {
    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
    for(list<InMemMSet*>::iterator 
        it= resStream->begin(); it != resStream->end(); ++it)
      (*it)->Print(cerr);
  }
  finalResStream->splice(finalResStream->end(), *resStream);
  if(debugme)
  {
    cerr<< "\nfinalResStream contains "<< 
      finalResStream->size() << " results\n";
    for(list<InMemMSet*>::iterator 
        it= finalResStream->begin(); it != finalResStream->end(); ++it)
      (*it)->Print(cerr);
  }
  delete resStream;
}

struct 
checkShortDelete: binary_function< InMemMSet* ,double,bool>
{
public: 
  bool operator() (InMemMSet* _mset, double d) const
  {
    if(_mset->units.empty())
    {
      delete _mset;
      return true;
    }
    double starttime= _mset->units.front().starttime, 
    endtime= _mset->units.back().endtime;
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
    vector<intpair>& ids, double d, int n, string qts, 
    list<InMemMSet*>*& finalResStream)
{
  bool debugme= false;
  set<int> constValue;
  map<int, int> _map;
  vector<int> mergeIndex;
  double starttime, endtime;
  bool lc, rc;
  int cnt= 0;
  
  list<InMemMSet*>* resStream= new list<InMemMSet*>(0);
  finalResStream= new list<InMemMSet*>(0);
  list<CompressedInMemUSet>::iterator cur= begin;
  while(cur != end)
  {
    cerr<<"\n\nProcessing unit number: " <<++cnt;
    cerr<<"\nresStream has: " <<resStream->size();
    cerr<<" results, finalResStream has: " <<finalResStream->size();
    Accumlator.GetSet(cur, constValue);
    starttime= (*cur).starttime; endtime= (*cur).endtime;
    lc= (*cur).lc; rc= (*cur).rc;
    ++cur;
    DynamicGraphAppend(constValue, starttime, endtime, lc, rc,
        ids, d, n, qts, resStream, finalResStream);
  }
  cerr<<"\n FindDynamicComponents finished successfully ";
  if(debugme)
  {
    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
    for(list<InMemMSet*>::iterator 
        it= resStream->begin(); it != resStream->end(); ++it)
      (*it)->Print(cerr);
  }
  resStream->remove_if(bind2nd(checkShortDelete(), d));
  if(debugme)
  {
    cerr<< "\nresStream contains "<< resStream->size() << " results\n";
    for(list<InMemMSet*>::iterator 
        it= resStream->begin(); it != resStream->end(); ++it)
      (*it)->Print(cerr);
  }
  finalResStream->splice(finalResStream->end(), *resStream);
  if(debugme)
  {
    cerr<< "\nfinalResStream contains "<< 
      finalResStream->size() << " results\n";
    for(list<InMemMSet*>::iterator 
        it= finalResStream->begin(); it != finalResStream->end(); ++it)
      (*it)->Print(cerr);
  }
  delete resStream;
}

static void DynamicGraphAppend(set<int> &constValue, double starttime, 
    double endtime, bool lc, bool rc, vector<intpair>& ids, double d, 
    int n, string qts, 
    list<InMemMSet*>* resStream, list<InMemMSet*>* finalResStream)
{
  bool debugme= false;
  set<int> *subGraph;
  map<int, int> _map;
  igraph_t* graph;
  vector<set<int> >* subGraphsNodes, *subGraphsEdges;
  vector<int> mergeIndex;
  bool intersects;
  int cnt= 0;
  bool merged= false;
  
  graph= Set2Graph(constValue, ids, _map);
  if(graph == 0)
  {
    cerr<< "\n Empty set yields empty graph ";
    resStream->remove_if(bind2nd(checkShortDelete(), d));
    finalResStream->splice(finalResStream->end(), *resStream);
    resStream->clear();
    if(debugme)
    {
      cerr<< "\nresStream contains "<< resStream->size() << " results\n";
      for(list<InMemMSet*>::iterator 
          it= resStream->begin(); it != resStream->end(); ++it)
        (*it)->Print(cerr);
    }
    if(debugme)
    {
      cerr<< "\nfinalResStream contains "<< 
        finalResStream->size() << " results\n";
      for(list<InMemMSet*>::iterator 
          it= finalResStream->begin(); it != finalResStream->end(); ++it)
        (*it)->Print(cerr);
    }
    return;
  }
  
  subGraphsNodes= FindSubGraphs(graph, n, qts, _map);
  int size= subGraphsNodes->size();
  cerr<<"\n Number of subgraphs in this unit is: "<< size;
  subGraphsEdges= new vector<set<int> >(size);
  for(unsigned int i=0; i< subGraphsNodes->size(); ++i)
  {
    GraphNodes2Edges(
        (*subGraphsNodes)[i], constValue, ids, (*subGraphsEdges)[i]);
    (*subGraphsNodes)[i].clear();
  }
  delete subGraphsNodes;

  if(resStream->empty())
  {
    for(unsigned int i=0; i< subGraphsEdges->size(); ++i)
    {
      InMemMSet* newmset= new InMemMSet();
      newmset->MergeAdd((*subGraphsEdges)[i], starttime, endtime, lc, rc);
      resStream->push_back(newmset);
      if(debugme)
        newmset->Print(cerr);
    }
    if(debugme)
    {
      cerr<< "\nresStream contains "<< resStream->size() << " results\n";
      for(list<InMemMSet*>::iterator 
          it= resStream->begin(); it != resStream->end(); ++it)
        (*it)->Print(cerr);
    }
    if(debugme)
    {
      cerr<< "\nfinalResStream contains "<< resStream->size() << " results\n";
      for(list<InMemMSet*>::iterator 
          it= finalResStream->begin(); it != finalResStream->end(); ++it)
        (*it)->Print(cerr);
    }
  }
  else
  {
    if(subGraphsEdges->empty())
    {
      resStream->remove_if(bind2nd(checkShortDelete(), d));
      finalResStream->splice(finalResStream->end(), *resStream);
      resStream->clear();
      if(debugme)
      {
        cerr<< "\nresStream contains "<< resStream->size() << " results\n";
        for(list<InMemMSet*>::iterator 
            it= resStream->begin(); it != resStream->end(); ++it)
          (*it)->Print(cerr);
      }
      if(debugme)
      {
        cerr<< "\nfinalResStream contains "<< 
          finalResStream->size() << " results\n";
        for(list<InMemMSet*>::iterator 
            it= finalResStream->begin(); it != finalResStream->end(); ++it)
          (*it)->Print(cerr);
      }
    }
    else
    {    
      list<InMemMSet*>::iterator it= resStream->begin();
      while(it != resStream->end())
      {
        InMemMSet* _mset= (*it);
        InMemUSet* uset= &_mset->units.back();
        for(unsigned int i=0; i< subGraphsEdges->size(); ++i)
        {
          subGraph= &(*subGraphsEdges)[i];
          intersects = uset->Intersects(*subGraph);
          if(intersects)
            mergeIndex.push_back(i);
        }
        if(mergeIndex.empty())
        {
          if(debugme)
          {
            cerr<<endl<< resStream->size() <<endl << _mset << endl;
            _mset->Print(cerr);
          }
          double starttime= _mset->units.front().starttime, 
                endtime= _mset->units.back().endtime;
          if(endtime - starttime < d) 
            delete _mset;
          else
            finalResStream->push_back(_mset);
          resStream->erase(it++);  
          if(debugme)
          {
            cerr<< "\nresStream contains "<< resStream->size() << " results\n";
            for(list<InMemMSet*>::iterator 
                it= resStream->begin(); it != resStream->end(); ++it)
              (*it)->Print(cerr);
          }
          if(debugme)
          {
            cerr<< "\nfinalResStream contains "<< 
              finalResStream->size() << " results\n";
            for(list<InMemMSet*>::iterator 
                it= finalResStream->begin(); it != finalResStream->end(); ++it)
              (*it)->Print(cerr);
          }
          
        }
        else if(mergeIndex.size() == 1)
        {
          subGraph= &(*subGraphsEdges)[mergeIndex[0]];
          merged= _mset->MergeAdd(*subGraph, starttime, endtime, lc, rc);
          if(debugme)
            _mset->Print(cerr);
          assert(merged);
          ++it;
          mergeIndex.clear();
        }
        else
        {
          for(unsigned int k=0; k< mergeIndex.size(); ++k)
          {
            subGraph= &(*subGraphsEdges)[mergeIndex[k]];
            list<InMemUSet>::iterator msetEnd= _mset->units.end();
            InMemMSet* newMSet= 
              new InMemMSet(*_mset, _mset->units.begin(), --msetEnd);
            merged= newMSet->MergeAdd(*subGraph, starttime, endtime, lc, rc);
            if(debugme)
              newMSet->Print(cerr);
            assert(merged);  
            resStream->push_front(newMSet);
            if(debugme)
              cerr<<endl<< resStream->size() << endl;
          }
          mergeIndex.clear();
          delete _mset;
          resStream->erase(it++); 
          if(debugme)
          {
            cerr<< "\nresStream contains "<< resStream->size() << " results\n";
            for(list<InMemMSet*>::iterator 
                it= resStream->begin(); it != resStream->end(); ++it)
              (*it)->Print(cerr);
          }
          if(debugme)
          {
            cerr<< "\nfinalResStream contains "<< 
              finalResStream->size() << " results\n";
            for(list<InMemMSet*>::iterator 
                it= finalResStream->begin(); it != finalResStream->end(); ++it)
              (*it)->Print(cerr);
          }
        }
      }
    }
  }
  subGraphsEdges->clear();
  delete subGraphsEdges;
  subGraphsEdges=0;
  _map.clear();
  igraph_destroy(graph);
  delete graph;
  graph=0;

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

static bool Merge(InMemMSet *_mset, set<int> *subGraph, double starttime,
    double endtime, bool lc, bool rc)
{
  InMemUSet uset= _mset->units.back();
  bool intersects = uset.Intersects(*subGraph);
  if(intersects)
  {
    _mset->MergeAdd(*subGraph, starttime, endtime, lc, rc);
    return true;
  }
  return false;
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

  static InMemMSet* EdgeMSet2NodeMSet(InMemMSet* edgeMSet, vector<intpair>& ids)
  {
    InMemMSet* res= 0;
    intpair edge;
    set<int> nodeSet;
    if(edgeMSet == 0 || edgeMSet->GetNoComponents() == 0) return res;
    
    res= new InMemMSet();
    for(list<InMemUSet>::iterator 
        it1= edgeMSet->units.begin(); it1!= edgeMSet->units.end(); ++it1)
    {
      InMemUSet* edgeUSet= &(*it1);
      nodeSet.clear();
      for(set<int>::iterator it2= edgeUSet->constValue.begin(); 
        it2!= edgeUSet->constValue.end(); ++it2)
      {
        edge= ids[*it2];
        nodeSet.insert(edge.first);
        nodeSet.insert(edge.second);
      }
      res->MergeAdd(nodeSet, edgeUSet->starttime, edgeUSet->endtime,
          edgeUSet->lc, edgeUSet->rc);
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
