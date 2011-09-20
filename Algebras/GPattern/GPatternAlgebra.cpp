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

[1] Source File of the Spatiotemporal Group Pattern Algebra

JAN, 2010 Mahmoud Sakr

[TOC]

1 Overview

2 Defines and includes

*/

#include "GPatternAlgebra.h"
#include "Symbols.h"

using namespace mset;

namespace GPattern{


/*
GPatternHelper

*/

ChangeRecord::ChangeRecord():status(NotChanged){}

bool ChangeRecord::UpdateStatus(StatusCode newCode)
{
  if(status == NotChanged)
  {
    status = newCode;
    return true;
  }
  if(status == ElemsRemoved && newCode != NotChanged)
  {
    status = newCode;
    return true;
  }
  if(newCode == UnitRemoved)
    return true;

  return false;
}

bool ChangeRecord::AppendToRemovedNodes(int node)
{
  this->removedNodes.push_back(node);
  return true;
}

ostream& ChangeRecord::Print(ostream& os)
{
  string statusText[]={"Unchanged", "EdgesRemoved", "UnitDeleted"};
  os<<"\n"<<statusText[this->status];
  if(this->status == ElemsRemoved)
  {
    os<<": ";
    for(unsigned int i=0; i<this->removedNodes.size(); ++i)
      os<<this->removedNodes[i]<<",";
  }
  return os;
}

void ChangeRecord::Clear()
{
  this->status= NotChanged;
  this->removedNodes.clear();
  this->removedNodesInRemoved.clear();
}

MSetIndex::MSetIndex(
    CompressedInMemMSet& mset, vector<pair<int, int> >& edge2Nodes )
{
  nodes.clear(); units.clear();
  ConstructIndex(mset, edge2Nodes);
}
void MSetIndex::ConstructIndex(
    CompressedInMemMSet& mset, vector<pair<int, int> >& edge2Nodes)
{
  bool debugme= false;
  if(debugme&& 0)
    mset.Print(cerr);
  if(mset.units.size()==0) return;
  list<CompressedInMemUSet>::iterator i= mset.units.begin();
  LWGraph graph;
  set<int> deltaNodes;
  int unitIndex=0;
  int count=0;
  while(i!= mset.units.end())
  {
    if(debugme)
    {
//      cerr<<"\n graph:";
//      graph.print(cerr);
      if(!(*i).added.empty())
      {
        cerr<<"\n addedEdges:";
        PrintSet((*i).added, cerr);
      }
      if(!(*i).removed.empty())
      {
         cerr<<"\n removedEdges:";
         PrintSet((*i).removed, cerr); //cerr<<"\nAccumlator: ";
      }
//      _mset.Print(cerr);
    }
    InsertEdgesUndirected(&graph, (*i).added, edge2Nodes, deltaNodes);
    AppendInsertionsIntoNodeLog(deltaNodes, edge2Nodes, i, unitIndex);
    count+= deltaNodes.size();

    RemoveEdgesUndirected(&graph, (*i).removed, edge2Nodes, deltaNodes);
    AppendRemovalsIntoNodeLog(deltaNodes, edge2Nodes, i, unitIndex);
    count-= deltaNodes.size();

    units.push_back(count);
//      if(debugme)
//        Print(&nodeHistory, cerr);
    ++i;
    ++unitIndex;
  }
  set<int> nodesNotFinalized;
  graph.get_nodes(nodesNotFinalized);
  FinalizeIndex(nodesNotFinalized, i);
}

void MSetIndex::FinalizeIndex(set<int>& nodesToBeFinalized,
    list<CompressedInMemUSet>::iterator endIt)
{
  NodeLogEntry* log;
  list<CompressedInMemUSet>::iterator lastIt= endIt;
  --lastIt;
  for(set<int>::iterator it= nodesToBeFinalized.begin(); it!=
      nodesToBeFinalized.end(); ++it)
  {
    log= &this->nodes[*it].log.back();
    assertIf(log->endtime == -1);
    log->endUnitIndex= this->units.size() - 1;
    log->endUnitIt= lastIt;
    log->endtime= (*lastIt).endtime;
  }
}

void MSetIndex::AppendInsertionsIntoNodeLog(set<int>& deltaNodes,
    vector<pair<int, int> >& edge2nodesMap,
    list<CompressedInMemUSet>::iterator& curUSet, int curUSetIndex)
{
  map<int, set<int> > nodeNeighbors;
  int _edge, _node;
  pair<int, int> edgeNodes;
  map<int, set<int> >::iterator pos;
  for(set<int>::iterator it= (*curUSet).added.begin(); it !=
      (*curUSet).added.end(); ++it)
  {
    _edge= *it;
    edgeNodes= edge2nodesMap[_edge];
    _node= edgeNodes.first;
    pos= nodeNeighbors.find(_node);
    if(pos != nodeNeighbors.end())
      (*pos).second.insert(_edge);
    else
    {
      set<int> s; s.insert(_edge);
      assertIf(
          nodeNeighbors.insert(make_pair<int, set<int> >(_node, s)).second);
    }

    _node= edgeNodes.second;
    pos= nodeNeighbors.find(_node);
    if(pos != nodeNeighbors.end())
      (*pos).second.insert(_edge);
    else
    {
      set<int> s; s.insert(_edge);
      assertIf(
          nodeNeighbors.insert(make_pair<int, set<int> >(_node, s)).second);
    }
  }

  map<int, set<int> >::iterator nodeIt;
  for(nodeIt= nodeNeighbors.begin(); nodeIt!= nodeNeighbors.end(); ++nodeIt)
  {
    _node= (*nodeIt).first;
    if(deltaNodes.find(_node) == deltaNodes.end()){
      assertIf(this->nodes.find(_node) != this->nodes.end());}
    else{
      assertIf((this->nodes.find(_node) == this->nodes.end()) ||
          this->nodes[_node].log.back().endtime != -1);}
    this->nodes[_node].Append(curUSet, curUSetIndex, (*nodeIt).second);
  }
}
void MSetIndex::AppendRemovalsIntoNodeLog(set<int>& deltaNodes,
    vector<pair<int, int> >& edge2nodesMap,
    list<CompressedInMemUSet>::iterator& curUSet, int curUSetIndex)
{
  bool debugme=false;
  if(deltaNodes.empty()) return;
  list<CompressedInMemUSet>::iterator lastUSet= curUSet;
  --lastUSet;
  if(debugme)
    this->Print(cerr);
  for(set<int>::iterator it= deltaNodes.begin(); it!= deltaNodes.end(); ++it)
  {
    //assertIf(this->nodes.find(*it) != this->nodes.end());
    if(debugme)
      this->nodes[*it].Print(cerr);
    //assertIf(!this->nodes[*it].log.empty());
    //assertIf(this->nodes[*it].log.back().endtime == -1);
    if(debugme)
      cerr<<"\t"<<this->nodes[*it].log.back().endtime<<"\t";
    this->nodes[*it].log.back().endtime= (*lastUSet).endtime;
    this->nodes[*it].log.back().endUnitIt= lastUSet;
    this->nodes[*it].log.back().endUnitIndex= curUSetIndex -1 ;
  }
}

MSetIndex::NodeLogEntry::NodeLogEntry(
   int64_t st, list<CompressedInMemUSet>::iterator stIt, int sUI, set<int>& aE):
   starttime(st), endtime(-1), startUnitIt(stIt), startUnitIndex(sUI),
   endUnitIndex(-1), associatedEdges(aE){}

ostream& MSetIndex::NodeLogEntry::Print(ostream& os)
{
  os<<"\nunit indexes ["<< this->startUnitIndex<<"," <<this->endUnitIndex<<"]";
  os<<",\t time [";
  Instant i(instanttype);
  i.ReadFrom(this->starttime); i.Print(os);
  os<<",";
  i.ReadFrom(this->endtime); i.Print(os);
  os<<"]";
  os<<",\t timeFromUnit ["<< (*this->startUnitIt).starttime<<"," ;
  if(this->endUnitIndex == -1)
    os<< -1<<"]";
  else
    os<<(*this->endUnitIt).starttime;
  os<<"\nEdges"; PrintSet(this->associatedEdges, os);
  return os;
}

void MSetIndex::NodeLog::Append(
    list<CompressedInMemUSet>::iterator& usetIt, int usetIndex, set<int>& edges)
{
  if(log.empty()|| log.back().endtime != -1)
  {
    log.push_back(NodeLogEntry((*usetIt).starttime, usetIt, usetIndex, edges));
    return;
  }
  log.back().associatedEdges.insert(edges.begin(), edges.end());
}

bool MSetIndex::NodeLog::RemoveUnit(int index)
{
//needs tests
  bool debugme=false;
  if(index < this->log.front().startUnitIndex ||
      index > this->log.back().endUnitIndex) return false;

  if(debugme)
    this->Print(cerr);
  bool changed=false;
  list<NodeLogEntry>::iterator firstPartIt, logEntryIt= this->log.begin();
  NodeLogEntry* entry;
  while(logEntryIt != this->log.end() && !changed)
  {
    entry= &(*logEntryIt);
    assertIf(entry->endUnitIndex >= entry->startUnitIndex);

    if(index== entry->startUnitIndex &&
        entry->endUnitIndex == entry->startUnitIndex )
    {
      this->log.erase(logEntryIt++);
      changed= true;
      if(debugme)
        this->Print(cerr);
    }
    else if(index == entry->startUnitIndex)
    {
      ++(entry->startUnitIndex);
      ++(entry->startUnitIt);
      entry->starttime= (*(entry->startUnitIt)).starttime;
      assertIf(entry->starttime <= entry->endtime);
      ++logEntryIt;
      changed= true;
      if(debugme)
        this->Print(cerr);
    }
    else if(index == entry->endUnitIndex )
    {
      --entry->endUnitIndex;
      --entry->endUnitIt;
      entry->endtime= (*entry->endUnitIt).endtime;
      ++logEntryIt;
      changed= true;
      if(debugme)
        this->Print(cerr);
    }
    else if(index > entry->startUnitIndex && index < entry->endUnitIndex)
    {
      NodeLogEntry newEntry(*entry);
      firstPartIt= this->log.insert(logEntryIt, newEntry);
      (*firstPartIt).endUnitIndex= index - 1;
      (*firstPartIt).endUnitIt= (*firstPartIt).startUnitIt;
      for(int i=(*firstPartIt).startUnitIndex; i< index-1; ++i)
        ++(*firstPartIt).endUnitIt;
      (*firstPartIt).endtime= (*(*firstPartIt).endUnitIt).endtime;

      entry->startUnitIndex= index+1;
      for(int i=entry->startUnitIndex; i< index+1; ++i)
        ++entry->startUnitIt;
      entry->starttime= (*entry->startUnitIt).starttime;
      ++logEntryIt;
      changed= true;
      if(debugme)
        this->Print(cerr);
    }
    else
      ++logEntryIt;
  }
  return changed;
}

ostream& MSetIndex::NodeLog::Print(ostream& os)
{
  int i=0;
  list<NodeLogEntry>::iterator it= this->log.begin();
  while(it != this->log.end())
  {
    os<<"\n NodeLog "<< i++;
    (*it).Print(os);
    ++it;
  }
  return os;
}

bool MSetIndex::RemoveShortNodeEntries(
    int64_t dMS, vector<ChangeRecord>& AllChanges)
{
  map<int,  NodeLog>::iterator nodeEntry= this->nodes.begin();
  bool changed= false;
  while(nodeEntry != this->nodes.end())
  {
    changed= RemoveShortNodeEntries(dMS, nodeEntry, AllChanges) || changed;
    ++nodeEntry;
  }
  return changed;
}

void DecreaseUnitCount(int& cnt)
{
  assertIf(cnt > 0);
  --cnt;
}

class LogNodeRemoval {
  private:
    int nodeId;
  public:
    LogNodeRemoval(int id) : nodeId(id) {}
    void operator() (ChangeRecord& elem) {
      elem.UpdateStatus(ChangeRecord::ElemsRemoved);
      elem.AppendToRemovedNodes(nodeId);
    }
};

void CheckRemoveEntry::SetChanged(bool ch)
{
  CheckRemoveEntry::changed= ch;
}
bool CheckRemoveEntry::GetChanged()
{
  return CheckRemoveEntry::changed;
}
CheckRemoveEntry::CheckRemoveEntry(int id,
  vector<ChangeRecord>& ch, MSetIndex* i, int64_t _dMS):
  nodeId(id), AllChanges(&ch), index(i), dMS(_dMS){}
bool CheckRemoveEntry::operator() (MSetIndex::NodeLogEntry& logEntry)
{
  bool debugme= false;
  if(debugme)
  {
    cerr<< "\nbefore applying d:";
    logEntry.Print(cerr);
  }
  if((logEntry.endtime - logEntry.starttime) < dMS)
  {
    for_each(index->units.begin() + logEntry.startUnitIndex,
        index->units.begin()+ logEntry.endUnitIndex, DecreaseUnitCount);
    for_each(AllChanges->begin() + logEntry.startUnitIndex,
        AllChanges->begin()+ logEntry.endUnitIndex,
        LogNodeRemoval(nodeId));
    SetChanged(true);
    return true;
  }
  else
  {
    SetChanged(false);
    return false;
  }
}

bool MSetIndex::RemoveShortNodeEntries(int64_t dMS,
    map<int,  NodeLog>::iterator nodeIt, vector<ChangeRecord>& AllChanges)
{
  //bool debugme= false;
  bool changed= false;
  NodeLog* nodeLog= &(*nodeIt).second;
  int nodeId= (*nodeIt).first;
  list<NodeLogEntry>::iterator logEntryIt= nodeLog->log.begin();
  NodeLogEntry* logEntry;
  while(logEntryIt != nodeLog->log.end())
  {
    logEntry= &(*logEntryIt);
    if((logEntry->endtime - logEntry->starttime) < dMS)
    {
      for_each(this->units.begin() + logEntry->startUnitIndex,
        this->units.begin()+ logEntry->endUnitIndex + 1, DecreaseUnitCount);
      // +1 is there because for_each loops over the interval [start, end)
      for_each(AllChanges.begin() + logEntry->startUnitIndex,
        AllChanges.begin()+ logEntry->endUnitIndex + 1,
        LogNodeRemoval(nodeId));
      if(AllChanges.begin() + logEntry->endUnitIndex + 1 != AllChanges.end())
        AllChanges[logEntry->endUnitIndex + 1].removedNodesInRemoved.push_back(
            nodeId);
      nodeLog->log.erase(logEntryIt++);
      changed= true;
    }
    else
    {
      ++logEntryIt;
    }
  }
  return changed;
}

void MSetIndex::DeleteNodePart(map<int,  NodeLog>::iterator& nodeIt,
    list<NodeLogEntry>::iterator nodeEntryIt, vector<ChangeRecord>& AllChanges)
{
//  NodeLogEntry* logEntry= &(*nodeEntryIt);
//  for(int i= logEntry->startUnitIndex; i<=logEntry->endUnitIndex; ++i)
//  {
//    --(this->units[i]);
//    AllChanges[i].UpdateStatus(ChangeRecord::ElemsRemoved);
//    AllChanges[i].AppendRemovedEdges(logEntry->associatedEdges);
//  }
//  (*nodeIt).second.log.erase(nodeEntryIt);
}

bool MSetIndex::RemoveSmallUnits(int n, vector<ChangeRecord>& AllChanges)
{
  bool changed= false;
  for(unsigned int i=0; i<this->units.size(); ++i)
  {
    assertIf(this->units[i] >= -1);
    if(this->units[i] < n && this->units[i] != -1)
    {
      changed= true;
      RemoveUnit(i, AllChanges);
    }
  }
  return changed;
}

void MSetIndex::RemoveUnit(int index, vector<ChangeRecord>& AllChanges)
{
  AllChanges[index].UpdateStatus(ChangeRecord::UnitRemoved);
  this->units[index]= -1;
  map<int,  NodeLog>::iterator nodeIt= this->nodes.begin();
  while(nodeIt != this->nodes.end())
  {
    (*nodeIt).second.RemoveUnit(index);
    ++nodeIt;
  }
}

ostream& MSetIndex::Print(ostream& os)
{
  map<int,  NodeLog>::iterator nodesIt= this->nodes.begin();
  while(nodesIt != this->nodes.end())
  {
    os<<"\nIndex Node: "<<(*nodesIt).first;
    (*nodesIt).second.Print(os);
    ++nodesIt;
  }
  return os;
}


GPatternHelper::GPatternHelper();
GPatternHelper::~GPatternHelper();

bool GPatternHelper::setCompareDesc (set<int>& i,set<int>& j)
{
  if(includes(i.begin(), i.end(), j.begin(), j.end())) return true;
  if(i > j) return true;
  return false;
}
void GPatternHelper::RemoveDuplicates(list<CompressedInMemUSet>& resStream)
{
  //sort(resStream.begin(), resStream.end(), setCompareDesc);

}

void GPatternHelper::removeShortUnits(MBool &mbool, int64_t dMS)
{
  UBool ubool;
  int64_t starttime, endtime;
  for(int i= 0; i< mbool.GetNoComponents(); ++i)
  {
    mbool.Get(i, ubool);
    starttime= ubool.timeInterval.start.millisecondsToNull();
    endtime= ubool.timeInterval.end.millisecondsToNull();
    if(endtime - starttime < dMS)
    {
      ubool.constValue.Set(false, false);
      mbool.Put(i, ubool);
    }
  }
}

bool GPatternHelper::RemoveShortNodeMembership(
    CompressedInMemMSet& Accumlator,
    vector<pair<int, int> >& edge2nodesMap, int64_t dMS)
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
    InsertEdgesUndirected(&graph, (*i).added, edge2nodesMap, deltaNodes);
    InsertInNodeHistory(&nodeHistory, (*i).added, edge2nodesMap, i, deltaNodes);
    if(debugme)
      PrintNodeHistory(&nodeHistory, cerr);
    RemoveEdgesUndirected(&graph, (*i).removed, edge2nodesMap, deltaNodes);
    changed= (CheckRemoveNodeMembership(
      &nodeHistory, (*i).removed, edge2nodesMap, i, deltaNodes, dMS)||changed);
    if(debugme)
      PrintNodeHistory(&nodeHistory, cerr);
    ++i;
  }
  //Remove the units which have empty added and removed sets
  //Accumlator.RemoveConstUnits();
  return changed;
}

ostream& GPatternHelper::PrintNodeHistory(
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

bool GPatternHelper::CheckRemoveNodeMembership(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  set<int> removedEdges, vector<pair<int, int> >& edge2nodesMap,
  list<CompressedInMemUSet>::iterator& cur, set<int>& deltaNodes, int64_t dMS)
{
  int64_t nodeMembershipDuration;
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
    if(nodeMembershipDuration < dMS)
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

void GPatternHelper::InPlaceSetDifference(set<int>& set1, set<int>& set2)
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

void GPatternHelper::InsertInNodeHistory(
  map<int, pair<list<CompressedInMemUSet>::iterator, set<int> > >* nodeHistory,
  set<int> newEdges, vector<pair<int, int> >& edge2nodesMap,
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
    edgeNodes= &edge2nodesMap[_edge];
    pos= nodeHistory->find(edgeNodes->first);
    if(pos != nodeHistory->end())
    {
      nodeHistoryEdges= &(*pos).second.second;
      nodeHistoryEdges->insert(_edge);
    }
    else
    {
      assertIf(deltaNodes.find(edgeNodes->first) != deltaNodes.end());
      FindNodeEdges(edgeNodes->first, newEdges, edge2nodesMap, newNodeEdges);
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
      FindNodeEdges(edgeNodes->second, newEdges, edge2nodesMap, newNodeEdges);
      nodeHistory->insert(
          make_pair(edgeNodes->second, make_pair(cur, newNodeEdges)));
    }
  }
}

void GPatternHelper::FindNodeEdges(
    int newNode, set<int>& newEdges, vector<pair<int, int> >& edge2nodesMap,
    set<int>& newNodeEdges)
{
  newNodeEdges.clear();
  pair<int, int>* edgeNodes;
  for(set<int>::iterator it= newEdges.begin(); it!=newEdges.end(); ++it)
  {
    edgeNodes= &edge2nodesMap[*it];
    if(edgeNodes->first == newNode || edgeNodes->second == newNode)
      newNodeEdges.insert(*it);
  }
}
bool GPatternHelper::RemoveUnitsHavingFewNodes(
    CompressedInMemMSet& Accumlator,
    vector<pair<int, int> >& edge2nodesMap, int n)
{
  //bool debugme= false;
  bool changed=false;
  if(Accumlator.units.size()==0) return false;
  list<CompressedInMemUSet>::iterator i= Accumlator.units.begin();
  LWGraph graph;
  vector<list<CompressedInMemUSet>::iterator> unitsToRemove;
  while(i!= Accumlator.units.end())
  {
    InsertEdgesUndirected(&graph, (*i).added, edge2nodesMap);
    RemoveEdgesUndirected(&graph, (*i).removed, edge2nodesMap);
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

void GPatternHelper::FindLargeDynamicComponents(CompressedInMemMSet& Accumlator,
    list<CompressedInMemUSet>::iterator begin,
    list<CompressedInMemUSet>::iterator end ,
    vector<pair<int,int> >& edge2nodesMap, int64_t dMS, int n, string& qts,
    list<CompressedMSet*>*& finalResStream, int depth)
{
  bool debugme= false;
  list<CompressedInMemUSet>::iterator cur= begin;
  set<int> s;
  list<CompressedMSet*>* resStream= 0;
  finalResStream= new list<CompressedMSet*>(0);
  GPatternHelper GPHelper;
  if(debugme)
  {
    cerr<<"\nFindSubGraphs started at depth " << depth;
    cerr<<"\nAccumlator has " << Accumlator.GetNoComponents()<< " units";
  }
  FindDynamicComponents(Accumlator, begin, end, edge2nodesMap, dMS, n, qts,
      resStream);
  if(debugme && 0)
  {
    list<CompressedMSet*>::iterator it= resStream->begin();
    while(it != resStream->end())
    {
      (*it)->Print(cerr);
      CompressedMSet* tmp= EdgeMSet2NodeMSet(*it, edge2nodesMap);
      CompressedInMemMSet tmp1;
      tmp->WriteToCompressedInMemMSet(tmp1);
      MSet tmp2(true);
      tmp1.WriteToMSet(tmp2);
      tmp2.Print(cerr);
      delete tmp;
      ++it;
    }
  }
//To be deleted
//  finalResStream= resStream;
//  return;
//End of to be deleted

  if(debugme)
    cerr<<"\nFindDynamicComponents returned";
  bool changed= true;
  list<CompressedMSet*> localResStream;
  CompressedMSet* curMSet=0;
  vector<ChangeRecord> Changes;
  CompressedInMemMSet inMemMSet;
  while(resStream->begin() != resStream->end())
  {
    curMSet= resStream->front();
    if(debugme && 0)
    {
      curMSet->Print(cerr);
      CompressedMSet* tmp= EdgeMSet2NodeMSet(curMSet, edge2nodesMap);
      CompressedInMemMSet tmp1;
      tmp->WriteToCompressedInMemMSet(tmp1);
      MSet tmp2(true);
      tmp1.WriteToMSet(tmp2);
      tmp2.Print(cerr);
      delete tmp;
    }
    Changes.resize(curMSet->GetNoComponents());
    curMSet->WriteToCompressedInMemMSet(inMemMSet);
    MSetIndex index(inMemMSet, edge2nodesMap);
    changed=
        GPHelper.ApplyThresholds(index, n, dMS, Changes);
    if(changed)
    {
      GPHelper.UpdateResult(
          &inMemMSet, edge2nodesMap, n, dMS, qts, Changes, localResStream);
      resStream->splice(resStream->end(), localResStream);
      delete curMSet;
    }
    else
      finalResStream->push_back(curMSet);
    resStream->pop_front();
    if(debugme&& 0)
    {
      list<CompressedMSet*>::iterator it= resStream->begin();
      while(it != resStream->end())
      {
        //cerr<<(*it)->GetNoComponents();
        //(*it)->Print(cerr);
        CompressedMSet* tmp= EdgeMSet2NodeMSet(*it, edge2nodesMap);
        CompressedInMemMSet tmp1;
        tmp->WriteToCompressedInMemMSet(tmp1);
        MSet tmp2(true);
        tmp1.WriteToMSet(tmp2);
        tmp2.Print(cerr);
        delete tmp;
        ++it;
      }
    }
  }
  delete resStream;
}

struct
checkShortDelete: binary_function< CompressedMSet* , int64_t, bool>
{
public:
  bool operator() (CompressedMSet* _mset, int64_t dMS) const
  {
    if(_mset->units.Size() == 0)
    {
      delete _mset;
      return true;
    }
    CompressedUSetRef uset;
    _mset->units.Get(0, uset);
    int64_t starttime= uset.starttime;
    _mset->units.Get(_mset->units.Size() - 1, uset);
    int64_t endtime= uset.endtime;
    if(endtime - starttime < dMS)
    {
      delete _mset;
      return true;
    }
    return false;
  }
};

CompressedMSet* GPatternHelper::CollectResultParts(
    vector<CompressedMSet*>& ResultParts, vector<int>&  partIndexs)
{
  bool debugme= false;
  if(debugme)
  {
    cerr<<"\nCollecting parts:";
    for(unsigned int partIndex=0; partIndex < partIndexs.size(); ++partIndex)
        cerr<<partIndexs[partIndex]<< ", ";
  }

  CompressedMSet* result= new CompressedMSet(0);
  for(unsigned int partIndex=0; partIndex < partIndexs.size(); ++partIndex)
  {
    if(debugme)
      ResultParts[partIndexs[partIndex]]->Print(cerr);
    result->Concat( ResultParts[partIndexs[partIndex]]);
    if(debugme)
    {
      //ResultParts[partIndexs[partIndex]]->Print(cerr);
      result->Print(cerr);
    }
  }
  return result;
}

void GPatternHelper::FindDynamicComponents(CompressedInMemMSet& Accumlator,
    list<CompressedInMemUSet>::iterator begin,
    list<CompressedInMemUSet>::iterator end ,
    vector<pair<int, int> >& edge2nodesMap, int64_t dMS, int n, string& qts,
    list<CompressedMSet*>*& FinalResultStream)
{
  bool debugme= false;
  set<int> constValue;
  map<int, int> _map;
  vector<int> mergeIndex;
  int cnt= 0, totalNodesNum= Accumlator.GetNoComponents();

  vector<CompressedMSet*>* ResultParts= new vector<CompressedMSet*>(0);
  list<vector<int> > *ResultStream=   new  list<vector<int> >(0);
  FinalResultStream= new list<CompressedMSet*>(0);
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
      cerr<<"\nresStream has: " <<ResultStream->size();
      cerr<<" results, finalResStream has: " <<FinalResultStream->size();
    }

    DynamicGraphAppend(graph, components, compLabelsMap, cur,
        edge2nodesMap, dMS, n, qts, NextComponentLabel, ResultParts,
        ResultStream, FinalResultStream);
    ++cur;
  }

  if(debugme && 0)
  {
    cerr<< "\nfinalResStream contains "<<
      FinalResultStream->size() << " results\n";
    for(list<CompressedMSet*>::iterator
        it= FinalResultStream->begin(); it != FinalResultStream->end(); ++it)
      (*it)->Print(cerr);
  }


  //finalize the components
  --cur;
  for(list<Component*>::iterator compIt= components->begin(); compIt!=
    components->end(); ++compIt)
    (*compIt)->ExtendResStreamsTillNow(*ResultParts, (*cur).endtime, (*cur).rc);
  ++cur;

  list<vector<int> >::iterator resultIt= ResultStream->begin();
  while(!ResultStream->empty())
  {
    CompressedMSet* result=
        CollectResultParts(*ResultParts, ResultStream->front());
    FinalResultStream->push_back(result);
    ResultStream->pop_front();
  }
  if(debugme && 0)
  {
    cerr<< "\nfinalResStream contains "<<
      FinalResultStream->size() << " results\n";
    for(list<CompressedMSet*>::iterator
        it= FinalResultStream->begin(); it != FinalResultStream->end(); ++it)
      (*it)->Print(cerr);
  }
  while(!ResultParts->empty())
  {
    delete ResultParts->back();
    ResultParts->pop_back();
  }
  for(list<Component*>::iterator it= components->begin(); it!=
    components->end(); ++it)
    delete *it;
  delete components;
  delete graph;
  delete ResultParts;
  delete ResultStream;
}

/*
SetIntersects

*/
bool GPatternHelper::SetIntersects(set<int> &set1, set<int> &set2)
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

void GPatternHelper::CheckAdd( LWGraph* g, NewComponent& comp, int n ,
    list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    int& NextComponentLabel)
{
  if((comp.newNodes.size()* 1.0) >= n)
  {
    Component* newComp= new Component();
    newComp->message = Component::NewlyAdded;
    newComp->label= NextComponentLabel++;
    newComp->nodes.insert(comp.newNodes.begin(), comp.newNodes.end());
    components->push_back(newComp);
    compLabelsMap[newComp->label]= --components->end();
    SetGraphNodesComponent(g, comp.newNodes, newComp->label);
  }
}

void GPatternHelper::UpdateMerge(LWGraph *g, NewComponent& newComp,
    set<int>& newEdges, vector<pair<int, int> >& edge2nodesMap,
    list<Component*>* components,
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
    edgeNodes= &edge2nodesMap[*newEdge];
    if(((*affectedComp)->nodes.find(edgeNodes->first) !=
            (*affectedComp)->nodes.end())
      ||
      ((*affectedComp)->nodes.find(edgeNodes->second) !=
            (*affectedComp)->nodes.end()))
      (*affectedComp)->addedEdges.insert(*newEdge);
  }
  assert((*affectedComp)->UpdateMessage(Component::AddedEdges));
  (*affectedComp)->nodes.insert(
      newComp.newNodes.begin(), newComp.newNodes.end());
  for(set<int>::iterator nodeIt= newComp.newNodes.begin(); nodeIt !=
      newComp.newNodes.end(); ++nodeIt)
  {
    assertIf(g->node_component.find(*nodeIt) == g->node_component.end());
    g->node_component.insert(make_pair(*nodeIt, (*affectedComp)->label));
  }

}

void GPatternHelper::MergeComponents(LWGraph* g, NewComponent& newComp,
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
    //a union of nodes, messages, and associatedResults
  if(! newComp.newNodes.empty())
  {
    bigComp->nodes.insert(newComp.newNodes.begin(), newComp.newNodes.end());
    assert(bigComp->UpdateMessage(Component::AddedEdges));
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

void GPatternHelper::UpdateRemove(LWGraph* graph, list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    vector<pair<int, int> >& edge2nodesMap, int n, int& NextComponentLabel,
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
      edgeNodes= &edge2nodesMap[*edgeIt];
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
        assert((*affectedComponentIt)->UpdateMessage(Component::RemovedEdges));
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
          newComponent->associatedResults.assign(
              (*affectedComponentIt)->associatedResults.begin(),
              (*affectedComponentIt)->associatedResults.end());
          newComponent->label= NextComponentLabel++;
          newComponent->parentLabel= (*affectedComponentIt)->label;
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
void GPatternHelper::Finalize(LWGraph* graph, list<Component*>* components,
    int64_t dMS, CompressedInMemUSet& cur, vector<CompressedMSet*>& ResultParts,
    list<vector<int> > *ResultStream, list<CompressedMSet*>* FinalResultStream,
    vector<pair<int, int> >& edge2nodesMap)
{
  bool debugme= false;

  vector<list<Component*>::iterator> compsToErase;
  vector<list< vector<int> >::iterator> resToErase;
  set<int> processedParents;
  for(list<Component*>::iterator compIt= components->begin(); compIt!=
    components->end(); ++compIt)
  {
    Component* comp= *compIt;
    if(debugme)
      comp->Print(cerr);
    switch(comp->message)
    {
    case Component::NotChanged:
    {
      comp->ExtendedTillLastChange= false;
    }break;
    case Component::NewlyAdded:
    {
      set<int> compEdges;
      comp->GetEdges(graph, compEdges);
      CompressedMSet* newMSet= new CompressedMSet(0);
      newMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
      ResultParts.push_back(newMSet);
      comp->resultPartIndex= ResultParts.size() -1;

      vector<int> res(1, comp->resultPartIndex);
      ResultStream->push_back(res);
      list< vector<int> >::iterator newResIt= --ResultStream->end();
      comp->associatedResults.push_back(newResIt);

      comp->ExtendedTillLastChange= true;
      comp->Reset();
      if(debugme)
      {
        list<vector<int> >::iterator associatedResultIt;
        for(unsigned int associatedResultIndex=0; associatedResultIndex <
          comp->associatedResults.size(); ++associatedResultIndex)
        {
          associatedResultIt= comp->associatedResults[associatedResultIndex];
          vector<int>  associatedResult= *associatedResultIt;
          CompressedMSet* theMSet=
              CollectResultParts(ResultParts, associatedResult);
          theMSet->Print(cerr);
          delete theMSet;
        }
        if(!IsOneComponent(newMSet, 0, edge2nodesMap))
          cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
      }
    }break;
    case Component::AddedEdges:
    case Component::RemovedEdges:
    case Component::AddRemoveMix:
    {
      comp->ExtendResStreamsTillNow(ResultParts, cur.starttime, !cur.lc);
      CompressedMSet* theMSet= ResultParts[comp->resultPartIndex];
      theMSet->AddUnit(comp->addedEdges, comp->removedEdges, cur.starttime,
            cur.endtime, cur.lc, cur.rc);
      if(debugme)
        if(!IsOneComponent(theMSet, 0, edge2nodesMap))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//        remainingResStreams.remove(*res);

      comp->ExtendedTillLastChange= true;
      comp->Reset();
    }break;
    case Component::MergedFromExistingComponents:
    {
/*
comp->ExtendResStreamsTillNow(ResultParts, cur.starttime, !cur.lc);
This was called within MergeComponents. The component now contains the merge
result, and not the merged components. ExtendResStreamsTillNow is applied to
the merged/small components, before they are deleted inside the MergeComponents
function.

*/
      set<int> compEdges;
      comp->SynchronizeNodes(graph);
      comp->ExtendResStreamsTillNow(ResultParts, cur.starttime, !cur.lc);
      comp->GetEdges(graph, compEdges);
      CompressedMSet* newMSet= new CompressedMSet(0);
      newMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
      ResultParts.push_back(newMSet);
      comp->resultPartIndex= ResultParts.size() -1;
      for(unsigned int associatedResultIndex=0; associatedResultIndex <
        comp->associatedResults.size(); ++associatedResultIndex)
      {
        vector<int>*  associatedResult=
            &(*(comp->associatedResults[associatedResultIndex]));
        associatedResult->push_back(ResultParts.size() -1);
        if(debugme)
          if(!IsOneComponent(newMSet, 0, edge2nodesMap))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//        remainingResStreams.remove(*res);
      }
      comp->ExtendedTillLastChange= true;
      comp->Reset();
    }break;
    case Component::SplitFromExtistingComponent:
    {
      comp->ExtendResStreamsTillNow(ResultParts, cur.starttime, !cur.lc);
      vector<list< vector<int> >::iterator> resStreams;
      set<int> compEdges;
      comp->GetEdges(graph, compEdges);
      CompressedMSet* newMSet= new CompressedMSet(0);
      newMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
      ResultParts.push_back(newMSet);
      comp->resultPartIndex= ResultParts.size() -1;
      for(unsigned int associatedResultIndex=0; associatedResultIndex <
        comp->associatedResults.size(); ++associatedResultIndex)
      {
        vector<int>  associatedResult=
            (*(comp->associatedResults[associatedResultIndex]));
        vector<int> newResult(associatedResult);
        newResult.push_back(ResultParts.size() -1);
        ResultStream->push_back(newResult);
        resStreams.push_back( --ResultStream->end());
        if(find(resToErase.begin(), resToErase.end(),
            comp->associatedResults[associatedResultIndex]) ==
            resToErase.end())
          resToErase.push_back(comp->associatedResults[associatedResultIndex]);
        if(debugme)
          if(!IsOneComponent(newMSet, 0, edge2nodesMap))
            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//        remainingResStreams.remove(*res);
      }
      comp->associatedResults.clear();
      comp->associatedResults.resize(resStreams.size());
      copy(resStreams.begin(), resStreams.end(),
          comp->associatedResults.begin());
      comp->ExtendedTillLastChange= true;
      comp->Reset();
    }break;
    case Component::RemoveNow:
    {
      comp->ExtendResStreamsTillNow(ResultParts, cur.starttime, !cur.lc);
      for(unsigned int associatedResultIndex=0; associatedResultIndex <
        comp->associatedResults.size(); ++associatedResultIndex)
      {
        vector<int>*  associatedResult=
            &(*(comp->associatedResults[associatedResultIndex]));
        CompressedMSet* theMSet=
            CollectResultParts(ResultParts, *associatedResult);
        if(theMSet->DurationLength() >= dMS)
        {
          if(debugme)
          {
            cerr<<"Moving the following mset from resStream to finalResStream:";
            theMSet->Print(cerr);
          }
          FinalResultStream->push_back(theMSet);
        }
        else
          delete theMSet;
        ResultStream->erase(comp->associatedResults[associatedResultIndex]);
//        remainingResStreams.remove(*res);
      }
      delete comp;
      compsToErase.push_back(compIt);
    }break;
    case Component::ReDistribute: //very unexpected case
    {
      bool NotYetImplemented= false;
      assert(NotYetImplemented);
//      comp->ExtendResStreamsTillNow(ResultParts, cur.starttime, !cur.lc);
//      list< list<mset::CompressedMSet*>::iterator > resStreams;
//      bool matched=false;
//      set<int> compEdges;
//      comp->GetEdges(graph, compEdges);
//      for(list< list<mset::CompressedMSet*>::iterator >::iterator res=
//        comp->resStreams.begin(); res!= comp->resStreams.end(); ++res)
//      {
//
//        CompressedMSet* theMSet= **res;
//        set<int>* resFinalEdgeSet= theMSet->GetFinalSet(), resFinalNodeSet;
//        EdgeSet2NodeSet(*resFinalEdgeSet, resFinalNodeSet, edge2nodesMap);
//        if(debugme)
//        {
//          cerr<<endl;
//          PrintSet(*resFinalEdgeSet, cerr);
//          cerr<<endl;
//          PrintSet(resFinalNodeSet, cerr);
//        }
//        if(SetIntersects(comp->nodes, resFinalNodeSet))
//        {
//          matched= true;
//          CompressedMSet* newMSet= new CompressedMSet(*theMSet);
//          newMSet->MergeAdd(compEdges, cur.starttime, cur.endtime,
//              cur.lc, cur.rc);
//          resStream->push_back(newMSet);
//          list<CompressedMSet*>::iterator newMSetIt= --resStream->end();
//          resStreams.push_back(newMSetIt);
//          resToErase.push_back(*res);
//          if(debugme)
//            if(!IsOneComponent(newMSet, 0, edge2nodesMap))
//              cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//          //        remainingResStreams.remove(*res);
//        }
//      }
//
//      comp->resStreams.clear();
//      comp->resStreams.resize(resStreams.size());
//      copy(resStreams.begin(), resStreams.end(), comp->resStreams.begin());
//      comp->ExtendedTillLastChange= true;
//      comp->Reset();
//
//     if(!matched) // a new component that is split from an existing component
//       //this is an extremely unexpected case that should not happen under
//        //normal moving objects scenarios
//      {
//        CompressedMSet* newMSet= new CompressedMSet(0);
//      newMSet->AddUnit(compEdges, cur.starttime, cur.endtime, cur.lc, cur.rc);
//        resStream->push_back(newMSet);
//        list<CompressedMSet*>::iterator newMSetIt= --resStream->end();
//        comp->resStreams.push_back(newMSetIt);
//        comp->ExtendedTillLastChange= true;
//        if(debugme)
//          if(!IsOneComponent(newMSet, 0, edge2nodesMap))
//            cerr<<"Not One Component!!!!!!!!!!!!!!!!!!!!!";
//      }
    }break;
    default:
    {
      assert(0);
    }

    };
  }


  for(unsigned int i= 0; i< resToErase.size(); ++i)
    ResultStream->erase(resToErase[i]);
  resToErase.clear();

//  assert(remainingResStreams.empty());
  for(unsigned int i=0; i<compsToErase.size(); ++i)
    components->erase(compsToErase[i]);

}

void GPatternHelper::DynamicGraphAppend(LWGraph* graph,
    list<Component*>* components,
    map<int, list<Component*>::iterator>& compLabelsMap,
    list<CompressedInMemUSet>::iterator cur,
    vector<pair<int, int> >& edge2nodesMap, int64_t dMS,
    int n, string& qts, int& NextComponentLabel,
    vector<CompressedMSet*>* ResultParts,
    list<vector<int> >* ResultStream,
    list<CompressedMSet*>*& FinalResultStream)
{
  bool debugme= false;

  if(debugme)
    graph->print(cerr);

  if(!(*cur).added.empty())
  {
    vector<NewComponent> newComponents;
    InsertEdgesUndirected(graph, (*cur).added, edge2nodesMap);
    FindComponentsOf(
        graph, components, (*cur).added, edge2nodesMap, newComponents);

    for(vector<NewComponent>::iterator newComp=
      newComponents.begin(); newComp!= newComponents.end(); ++newComp)
    {
      if(debugme)
        (*newComp).Print(cerr);
      if( (*newComp).affectedComponents.empty())
        CheckAdd( graph, *newComp, n , components,
            compLabelsMap, NextComponentLabel);

      else if( (*newComp).affectedComponents.size() == 1)
        UpdateMerge(graph, *newComp, (*cur).added, edge2nodesMap, components,
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
      edgeNodes= &edge2nodesMap[*edgeIt];
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
      UpdateRemove(graph, components, compLabelsMap, edge2nodesMap, n,
          NextComponentLabel, affectedComponentsLabels);
  }

  Finalize(graph, components, dMS, *cur, *ResultParts, ResultStream,
      FinalResultStream, edge2nodesMap);

}

void GPatternHelper::GraphNodes2Edges(set<int>& subGraphNodes,
    set<int>& graphEdges,
    vector<pair<int, int> >& edge2Nodes, set<int>& res)
{
  res.clear();
  pair<int, int>  edge;
  for(set<int>::iterator it= graphEdges.begin(); it!= graphEdges.end(); ++it)
  {
    edge= edge2Nodes[*it];
    if(subGraphNodes.find(edge.first) != subGraphNodes.end() &&
        subGraphNodes.find(edge.second) != subGraphNodes.end())
      res.insert(*it);
  }

}

bool GPatternHelper::Merge(CompressedInMemMSet *_mset, set<int> *subGraph,
    int64_t starttime, int64_t endtime, bool lc, bool rc)
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


void GPatternHelper::SetAddRemove(
    set<int>& _set, set<int>& _add, set<int>& _remove)
{
  _set.insert(_add.begin(), _add.end());
  for(set<int>::iterator it= _remove.begin(); it!= _remove.end(); ++it)
    _set.erase(*it);
}

void GPatternHelper::EdgeSet2NodeSet(
    set<int> &edges, set<int> &nodes, vector<pair<int, int> > edge2nodesMap)
{
  pair<int, int>  edge;
  nodes.clear();
  for(set<int>::iterator it= edges.begin(); it != edges.end(); ++it)
  {
    edge= edge2nodesMap[*it];
    nodes.insert(edge.first);
    nodes.insert(edge.second);
  }
}

CompressedMSet* GPatternHelper::EdgeMSet2NodeMSet(
    CompressedMSet* edgeMSet, vector<pair<int, int> >& edge2nodesMap)
{
  //bool debugme= false;
  LWGraph graph;
  CompressedMSet* nodeMSet= new CompressedMSet(0);
  CompressedUSetRef edgeUnit(true), nodeUnit(true);
  set<int> newNodes, removedNodes, curNodeSet;
  int count=0;
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
  InsertEdgesUndirected(&graph, toAdd, edge2nodesMap, newNodes);
  nodeUnit.starttime= edgeUnit.starttime; nodeUnit.lc= edgeUnit.lc;
  nodeUnit.addedstart=0; nodeUnit.removedstart=0;
  count= newNodes.size();
  for(set<int>::iterator it= newNodes.begin(); it!=newNodes.end(); ++it)
    nodeMSet->added.Append(*it);
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
    InsertEdgesUndirected(&graph, toAdd, edge2nodesMap, newNodes);
    RemoveEdgesUndirected(&graph, toRemove, edge2nodesMap, removedNodes);

    if(! newNodes.empty() || ! removedNodes.empty())
    {
      nodeUnit.addedend= nodeMSet->added.Size()-1;
      nodeUnit.removedend= nodeMSet->removed.Size()-1;
      nodeUnit.endtime= edgeUnit.starttime;
      nodeUnit.rc= ! edgeUnit.lc;
      nodeUnit.count= count;
      nodeMSet->units.Append(nodeUnit);

      nodeUnit.starttime= edgeUnit.starttime; nodeUnit.lc= edgeUnit.lc;
      nodeUnit.addedstart= nodeMSet->added.Size();
      nodeUnit.removedstart= nodeMSet->removed.Size();
      count+= newNodes.size() - removedNodes.size();
      assertIf(count >= 0);
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
  nodeUnit.count= count;
  nodeMSet->units.Append(nodeUnit);

  return nodeMSet;
}

void GPatternHelper::ComputeAddSubSets(InMemMSet& acc,
    list<InMemUSet>::iterator t1, list<InMemUSet>::iterator t2,
    unsigned int n,  int64_t dMS, vector<InMemMSet>* result)
{
  bool debugme= false;
  assert(acc.GetNoComponents() > 0 );
  assert(dMS > 0);
  assert(((*t2).endtime - (*t1).starttime) > dMS);

  if(debugme)
  {
    cerr<<"\nComputeResultStreamPart Called: n= "<< n <<"---------------\n";
    for(list<InMemUSet>::iterator t= t1; t!=t2; ++t)
      (*t).Print(cerr);
    (*t2).Print(cerr);
    cerr<<"End of input -----------------------";
  }
  multimap< set<int>, Int64Interval> res;

  list<InMemUSet>::iterator unitIterator1=t1, unitIterator2=t2;
  int64_t startInstant= (*t1).starttime, curInstant=0,
  endInstant= (*t2).endtime;
  bool lc= (*t1).lc, rc=false;
  list<InMemUSet>::iterator curUnit;
  InMemUSet candidates;
  curUnit= unitIterator1;
  while( endInstant - startInstant >= dMS)
  {
    unitIterator2= unitIterator1;
    curInstant= (*curUnit).endtime;
    rc= (*curUnit).rc;
    candidates.CopyFrom(*curUnit);
    while( candidates.Count() >= n &&
        curInstant - startInstant < dMS && unitIterator2 != t2)
    {
      curUnit = ++unitIterator2;
      curInstant= (*curUnit).endtime;
      rc= (*curUnit).rc;
      candidates.Intersection((*curUnit).constValue);
    }
    if(candidates.Count() >= n && curInstant - startInstant >= dMS)
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
  multimap< set<int>, Int64Interval>::iterator i;
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

void GPatternHelper::GenerateAllCombinations(InMemUSet& cand, int select,
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

void GPatternHelper::AddAllSubSetsToVector(InMemUSet& candidates,
    int64_t startInstant,
    int64_t curInstant, bool lc, bool rc,
    int n, multimap< set<int>, Int64Interval>& res)
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
    pair<multimap< set<int>, Int64Interval>::iterator,
    multimap< set<int>, Int64Interval>::iterator> ret;
    multimap< set<int>, Int64Interval>::iterator i;
    bool consumed= false;
    Int64Interval timeInterval(startInstant, curInstant, lc, rc);
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
        Int64Interval tmp(startInstant, curInstant, lc, rc);
        res.insert( pair<set<int>, Int64Interval>(sets[k],tmp) );
      }
    }

  }
}
void ClearChangeRecord(ChangeRecord& rec)
{
  rec.Clear();
}
bool GPatternHelper::ApplyThresholds(MSetIndex& index, int n, int64_t dMS,
    vector<ChangeRecord>& AllChanges)
{
  bool debugme= false;
  for_each(AllChanges.begin(), AllChanges.end(), ClearChangeRecord);
  bool changed= true, globallyChanged= false;
  if(debugme)
    index.Print(cerr);
  while(changed)
  {
    changed= index.RemoveShortNodeEntries(dMS, AllChanges);
    globallyChanged |= changed;
    changed= index.RemoveSmallUnits(n, AllChanges);
    globallyChanged |= changed;
  }
  if(debugme)
    index.Print(cerr);
  return globallyChanged;
}


void GPatternHelper::UpdateResult(CompressedInMemMSet* curMSet,
    vector<pair<int,int> >& edge2nodesMap,int n, int64_t dMS, string qts,
    vector<ChangeRecord>& Changes,
    list<CompressedMSet*>& resStream)
{
/*
Find the ChangeRecoreds marked as UnitDeleted, and partition curMSet and Changes
accordingly.
Insert curMSet partitions into msetParts, and Changes partitions into
changesParts (two parallel arrays)
FOREACH msetPart in msetParts, and
FOREACH corresponding changesPart in changesParts
  ApplyChanges(msetPart, changesPart, resStream, msetParts, changesParts)

*/

  bool debugme= false;
  unsigned int offset=0, length=0, it=0;
  list<CompressedInMemUSet>::iterator begin= curMSet->units.begin(),
      usetIt= begin, end;
  list<CompressedInMemMSet*> msetParts;
  list<vector<ChangeRecord> > changeParts;
  bool lastUnitIsDeleted=(Changes[0].status == ChangeRecord::UnitRemoved);
  while(++it < Changes.size())
  {
    ++usetIt;
    if(lastUnitIsDeleted && Changes[it].status != ChangeRecord::UnitRemoved)
    {
      offset= it;
      begin= usetIt;
    }
    else
     if(!lastUnitIsDeleted && Changes[it].status == ChangeRecord::UnitRemoved)
    {
      length= it - offset;
      end= usetIt;
      CompressedInMemMSet* newMSetPart=
          new CompressedInMemMSet(*curMSet, begin, end);
      msetParts.push_back(newMSetPart);
      changeParts.resize(changeParts.size()+1);
      changeParts.back().insert(changeParts.back().end(),
          Changes.begin() + offset, Changes.begin() + offset + length);
    }
    lastUnitIsDeleted= (Changes[it].status == ChangeRecord::UnitRemoved);
  }

  if(!lastUnitIsDeleted)
  {
    CompressedInMemMSet* newMSetPart= new CompressedInMemMSet(
        *curMSet, begin, curMSet->units.end());
    msetParts.push_back(newMSetPart);

    changeParts.resize(changeParts.size()+1);
    changeParts.back().insert(changeParts.back().end(),
        Changes.begin() + offset, Changes.end());
  }

  assertIf(msetParts.size() == changeParts.size());

  while(!msetParts.empty())
  {
    if(debugme && 0)
    {
      MSet tmp(true);
      cerr<<"\nBefore Change: ";
      msetParts.front()->WriteToMSet(tmp);
      tmp.Print(cerr);
    }
    ApplyChanges(msetParts.front(), changeParts.front(), edge2nodesMap,
        n, dMS, qts, resStream, msetParts, changeParts);
    delete msetParts.front();
    msetParts.pop_front();
    changeParts.pop_front();
    if(debugme && 0)
    {
      cerr<<"\nAfter Change: ";
      list<CompressedMSet*>::iterator it= resStream.begin();
      while(it != resStream.end())
      {
        (*it)->Print(cerr);
        CompressedMSet* tmp= EdgeMSet2NodeMSet(*it, edge2nodesMap);
        CompressedInMemMSet tmp1;
        tmp->WriteToCompressedInMemMSet(tmp1);
        MSet tmp2(true);
        tmp1.WriteToMSet(tmp2);
        tmp2.Print(cerr);
        delete tmp;
        ++it;
      }
    }
  }
}

void GPatternHelper::ApplyChanges(CompressedInMemMSet* inMemMSet,
    vector<ChangeRecord>& changesPart,
    vector<pair<int,int> >& edge2nodesMap,int n, int64_t dMS, string qts,
    list<CompressedMSet*>& resStream, list<CompressedInMemMSet*>& msetParts,
    list<vector<ChangeRecord> >& changeParts)
{


//  localResStream: vector<CompressedMSet>
//
//  FOREACH uset in msetPart and
//  FOREACH corresponding changeRec in changesPart
//   ASSERT(changeRec.status != UnitRemoved)
//   IF changeRec.status == UnChanged continue
//   ELSE IF changeRec.status == NodesRemoved
//     Remove nodes from the uset
//     changeRes.status= Unchanged
//     IF uset has one large component continue
//     ELSE IF uset has no large components
//       resStream.pushback(msetPart[0..uset.index-1])
//       msetParts.pushback(msetPart[uset.index+1..end])
//       changesParts.pushback(changesParts[uset.index+1..end])
//       RETURN
//     ELSE IF uset has several large components
//       FindDynamicComponents(msetPart,..., localResStream)
//       resStream.splice(resStream.end(), localResStream)
//       RETURN
//  resStream.pushback(msetPart)



  bool debugme= false;
  list<CompressedMSet*>* localResStream= 0;
  CompressedInMemUSet* inMemUSet;
  list<CompressedInMemUSet>::iterator usetIt= inMemMSet->units.begin();
  vector<ChangeRecord>::iterator changeIt= changesPart.begin();
  ChangeRecord* ch;
  set<int> edges;
  int noComponents;
  bool changed=false;
  while(changeIt!= changesPart.end())
  {
    ch= &(*changeIt);
    assertIf(ch->status != ChangeRecord::UnitRemoved);

    if(!ch->removedNodesInRemoved.empty())
    {
      inMemUSet= &(*usetIt);
      changed= inMemUSet->EraseNodes(ch->removedNodesInRemoved, edge2nodesMap);
      assertIf(changed);
    }
    if(ch->status == ChangeRecord::NotChanged)
    {
      ++changeIt;      ++usetIt;            continue;
    }
    else if (ch->status == ChangeRecord::ElemsRemoved)
    {
      inMemUSet= &(*usetIt);
      changed= inMemUSet->EraseNodes(ch->removedNodes, edge2nodesMap);
      ch->status= ChangeRecord::NotChanged;
      if(!changed)
      {
        ++changeIt;        ++usetIt;        continue;
      }
      inMemMSet->GetSet(usetIt, edges);
      noComponents= GetNumComponents(edges, n, edge2nodesMap);
      if(noComponents == 1)
      {
        ++changeIt;        ++usetIt;        continue;
      }
      else if(noComponents == 0)
      {
        if(usetIt != inMemMSet->units.begin())
        {
          CompressedInMemMSet firstPart(
              *inMemMSet, inMemMSet->units.begin(), usetIt);
          firstPart.MakeMinimal();
          CompressedMSet* resPart= new CompressedMSet(0);
          resPart->ReadFromCompressedInMemMSet(firstPart);
          resStream.push_back(resPart);
          if(debugme && 0)
            resStream.back()->Print(cerr);
        }
        list<CompressedInMemUSet>::iterator last= inMemMSet->units.end();
        --last;
        if(usetIt != last)
        {
          list<CompressedInMemUSet>::iterator it= usetIt; ++it;
          CompressedInMemMSet* lastPart= new CompressedInMemMSet(*inMemMSet,
              it, inMemMSet->units.end());
          msetParts.push_back(lastPart);
          vector<ChangeRecord> chPart(changeIt +1, changesPart.end());
          changeParts.push_back(chPart);
        }
        return;
      }
      else if(noComponents > 1)
      {
        if(debugme)
        {
          inMemMSet->Print(cerr);
          CompressedMSet tmp1(0);
          tmp1.ReadFromCompressedInMemMSet(*inMemMSet);
          CompressedMSet* tmp= EdgeMSet2NodeMSet(&tmp1, edge2nodesMap);
          CompressedInMemMSet tmp2;
          tmp->WriteToCompressedInMemMSet(tmp2);
          MSet tmp3(true);
          tmp2.WriteToMSet(tmp3);
          tmp3.Print(cerr);
          delete tmp;
        }
        inMemMSet->MakeMinimal();
        FindDynamicComponents(*inMemMSet, inMemMSet->units.begin(),
            inMemMSet->units.end(), edge2nodesMap, dMS, n, qts, localResStream);
        resStream.splice(resStream.end(), *localResStream);
        delete localResStream;
        localResStream= 0;
        return;
      }
    }
  }

  inMemMSet->MakeMinimal();
  CompressedMSet* result= new CompressedMSet(0);
  result->ReadFromCompressedInMemMSet(*inMemMSet);
  resStream.push_back(result);
  if(debugme)
    resStream.back()->Print(cerr);
}



GPatternSolver GPSolver;
/*
3 Classes

*/


int GPatternSolver::PickVariable()
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

bool GPatternSolver::MoveNext()
{
  if(iterator < (signed int)SA.size()-1)
    iterator++;
  else
    return false;
  return true;
}


bool GPatternSolver::GetStart(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.CopyFrom(&(SA[iterator][index].first.start));
  return true;
}

bool GPatternSolver::GetEnd(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.CopyFrom(&(SA[iterator][index].first.end));
  return true;
}

ostream& GPatternSolver::Print(ostream &os)
{
  os<< "\n==========================\nSA.size() = "<< SA.size()<< endl;
  Instant tmp(instanttype);
  for(unsigned int i=0; i< SA.size(); i++)
  {
    os<< "Tuple number:" << i;
    for(unsigned int j=0; j<SA[i].size(); j++)
    {
      if(Agenda[j] !=0) continue;
      os<< "Attribute number:" << j;
      SA[i][j].second->Print(os);
//      tmp.ReadFrom(SA[i][j].first.start.GetRealval());
//      tmp.Print(os);
//      os<< "\t ";
//      tmp.ReadFrom(SA[i][j].first.end.GetRealval());
//      tmp.Print(os);
//      os<<"\t{";
//      for(set<int>::iterator itr= SA[i][j].second.begin(); 
//        itr!= SA[i][j].second.end(); ++itr)
//      {
//        os<< *itr << ", ";
//      }
//      os<<"} |";
    }
    os<<endl;
  }
  return os;
}

int GPatternSolver::Clear()
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

void GPatternSolver::IntervalInstant2IntervalCcReal(
    const Interval<Instant>& in, Interval<CcReal>& out)
{
  bool debugme=false;
  out.start.Set(in.start.IsDefined(), in.start.ToDouble());
  out.end.Set(in.end.IsDefined(), in.end.ToDouble());
  out.lc= in.lc;
  out.rc= in.rc;
  if(debugme)
  {
    in.Print(cerr);
    out.Print(cerr);
  }
  
}

/*
Extend

*/

bool GPatternSolver::Extend(int varIndex)
{
  bool debugme= false;
  vector< pair< Interval<Instant>, MSet* > > sa(count);
  qp->Open(Agenda[varIndex]);
  Interval<Instant> deftime;
  Interval<Instant> period;
  Periods periods(0);
  set<int> val;
  USetRef unit;
  Word Value;

  if(SA.size() == 0) //This is the first varirable to be evaluated
  {
    for(int i=0; i<count; i++)
      sa[i].first.CopyFrom(nullInterval);

    qp->Request(Agenda[varIndex], Value);
    while(qp->Received(Agenda[varIndex]))
    {
      MSet* res = static_cast<MSet*>(Value.addr)->Clone();
      ToDelete.push_back(res);
      if(res->IsDefined())
      {
        if(debugme) res->Print(cerr);
        res->DefTime(periods);
        periods.Get(0, period);
        sa[varIndex].first= period;
        sa[varIndex].second= res;
        SA.push_back(sa);
      }
      qp->Request(Agenda[varIndex], Value);
    }
    qp->Close(Agenda[varIndex]); 
  }
  else
  {
    vector< pair< Interval<Instant>, MSet* > > stream;
    pair< Interval<Instant>, MSet* > elem;
    qp->Request(Agenda[varIndex], Value);
    while(qp->Received(Agenda[varIndex]))
    {
      MSet* res = static_cast<MSet*>(Value.addr)->Clone() ;
      ToDelete.push_back(res);
      if(res->IsDefined())
      {
        res->DefTime(periods);
        periods.Get(0, period);
        elem.first= period;
        elem.second= res;
        stream.push_back(elem);
      }
      qp->Request(Agenda[varIndex], Value);
    }
    // SA has already entries 
    unsigned int SASize= SA.size();
    for(unsigned int i=0; i<SASize; i++)
    {
      sa= SA[0];
      
      for(unsigned int j=0; j<stream.size(); ++j)
      {
        sa[varIndex]= stream[j];
        if(IsSupported(sa, varIndex))
          SA.push_back(sa);
      } 
      SA.erase(SA.begin());
    }
  }
  if(debugme) Print(cerr);
  return true;
}

bool GPatternSolver::IsSupported(
    vector< pair<Interval<Instant>, MSet* > >& sa, int index)
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
          supported= CheckConstraint(sa[assignedVars[i]].first, 
              sa[assignedVars[j]].first, 
              ConstraintGraph[assignedVars[i]][assignedVars[j]]);
          if(!supported) return false;
        }

        if(ConstraintGraph[assignedVars[j]][assignedVars[i]].size() != 0)
        {
          supported= CheckConstraint(sa[assignedVars[j]].first, 
              sa[assignedVars[i]].first, 
              ConstraintGraph[assignedVars[j]][assignedVars[i]]);
          if(!supported) return false;
        }
      }
    }
  }
  return supported;
}

/*
CheckCopnstraint

*/

bool GPatternSolver::CheckConstraint(Interval<Instant>& p1,
    Interval<Instant>& p2, vector<Supplier> constraint)
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
    STP::STVector* vec= (STP::STVector*) Value.addr;
    satisfied= vec->ApplyVector(p1, p2);
    if(!satisfied) return false;
  }
  return true; 
}

int GPatternSolver::AddConstraint(string alias1, string alias2, Supplier handle)
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

int GPatternSolver::AddVariable(string alias, Supplier handle)
{
  Agenda.push_back(handle);
  VarAliasMap[alias]=count;
  count++;
  ConstraintGraph.resize(count);
  for(int i=0; i<count; i++)
    ConstraintGraph[i].resize(count);
  return 0;
}

bool GPatternSolver::Solve()
{
  int varIndex;
  while( (varIndex= PickVariable()) != -1)
  {
    if(! Extend(varIndex)) return false;
    if(SA.size() == 0)  return false;
    Agenda[varIndex]=0;
  }
  if(SA.size() == 0)
    return false;
  return true;
}

void GPatternSolver::WriteTuple(Tuple* tuple)
{
  vector< pair< Interval<Instant>, MSet* > > sa= SA[iterator];
  USet uset(true);
  Instant instant(instanttype);
  MSet mset(0);
  for(unsigned int i=0; i<sa.size(); ++i)
  {
//    MSet* mset= new MSet(0);
//    uset.constValue.Clear();
//    instant.ReadFrom(sa[i].first.start.GetValue());
//    uset.timeInterval.start= instant;
//    instant.ReadFrom(sa[i].first.end.GetValue());
//    uset.timeInterval.end= instant;
//    uset.timeInterval.lc= sa[i].first.lc;
//    uset.timeInterval.rc= sa[i].first.rc;
//    for(set<int>::iterator it=sa[i].second.begin(); 
//       it != sa[i].second.end(); ++it)
//      uset.constValue.Insert(*it);
//    uset.SetDefined(true);
//    uset.constValue.SetDefined(true);
//    assert(uset.IsValid());
//    mset->Add(uset);
    tuple->PutAttribute(i, sa[i].second->Clone());
  }
}

/*
4 Algebra Types and Operators 

*/


ListExpr RowColTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }
  ListExpr errorInfo;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 1 &&
      nl->IsAtom(nl->First(args)) &&
      am->CheckKind(Kind::TEMPORAL(), nl->First(args), errorInfo),
      "Operators row/col expect one argument of kind TEMPORAL \n but got: "
      + argstr + ".");

  if(debugme)
  {
    cout<<endl<<endl<<"Operator row/col accepted the input";
    cout.flush();
  }
  return nl->First(args);
}


ListExpr CrossPatternTM(ListExpr args)
{
  bool debugme= false;
  bool error=false;
  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if(nl->ListLength(args) != 7)
  {
    ErrorReporter::ReportError( 
      "Operator crosspattern expects 7 arguments \n but got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////  
  ListExpr streamTuple =  nl->First(args),
  tupleIdent1= nl->Second(args),
  tupleIdent2= nl->Third(args),
  liftedPred = nl->Fourth(args),
  duration = nl->Fifth(args),
  count  =   nl->Sixth(args),
  subGraph = nl->Nth(7,args),
  tupleType=0;
  
  nl->WriteToString(argstr, streamTuple);
  if(listutils::isTupleStream(streamTuple)) 
  {
    tupleType = nl->Second(streamTuple);
    if(!listutils::isTupleDescription(tupleType))
      error= true;
  }
  else
    error= true;
  
  if(error)
  {
    ErrorReporter::ReportError(
       "Operator crosspattern expects a stream(tuple) as the first "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////    
  nl->WriteToString(argstr, tupleIdent1);
  if(listutils::isMap<1>(tupleIdent1))
  {
    if(nl->Second(tupleIdent1) == tupleType)
    {
      if(!nl->Equal(nl->Third(tupleIdent1), nl->SymbolAtom(CcInt::BasicType())))
        error= true;
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator crosspattern: error in the tuple type passed to the map in "
        "the second argument. It must be the same as the tuple type in the "
        "stream(tuple) in the first arguement. "
        "\nOperator crosspattern got: " + argstr + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  else
    error= true;
  
  if(error)
  {
    ErrorReporter::ReportError(
       "Operator crosspattern expects a (map tuple int) as a second "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////    
  nl->WriteToString(argstr, tupleIdent2);
  if(listutils::isMap<1>(tupleIdent2))
  {
    if(nl->Second(tupleIdent2) == tupleType)
    {
      if(!nl->Equal(nl->Third(tupleIdent2), nl->SymbolAtom(CcInt::BasicType())))
        error= true;
    }
    else
    {
      ErrorReporter::ReportError(
          "Operator crosspattern: error in the tuple type passed to the map in "
          "the third argument. It must be the same as the tuple type in the "
          "stream(tuple) in the first arguement. "
          "\nOperator crosspattern got: " + argstr + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  else
    error= true;

  if(error)
  {
    ErrorReporter::ReportError(
        "Operator crosspattern expects a (map tuple int) as a third "
        "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////  
  nl->WriteToString(argstr, liftedPred);
  if(listutils::isMap<1>(liftedPred))
  {
    if(nl->Second(liftedPred) == tupleType)
    {
      if(!nl->Equal(nl->Third(liftedPred), nl->SymbolAtom(MBool::BasicType())))
        error= true;
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator crosspattern: error in the tuple type passed to the map in "
        "the fourth argument. It must be the same as the tuple type in the "
        "stream(tuple) in the first arguement. "
        "\nOperator gpattern got: " + argstr + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  else
    error= true;
  
  if(error)
  {
    ErrorReporter::ReportError(
       "Operator crosspattern expects a (map tuple mbool) as the fourth "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////        
  nl->WriteToString(argstr, duration);
  if( !nl->IsAtom(duration) ||
       nl->SymbolValue(duration) != Duration::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator crosspattern expects a duration as the fifth argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////          
  nl->WriteToString(argstr, count);
  if( !nl->IsAtom(count) || nl->SymbolValue(count) != CcInt::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator crosspattern expects an int as the sixth argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////        
  nl->WriteToString(argstr, subGraph);
  if( !nl->IsAtom(subGraph) )
  {
    ErrorReporter::ReportError(
        "Operator crosspattern expects a subgraph name (...) as the "
        "last argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr result =
    nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), nl->SymbolAtom("mset"));
  if(debugme)
  {
    cout<<endl<<endl<<"Operator gpattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr GPatternTM(ListExpr args)
{
  bool debugme= false;
  bool error=false;
  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if(nl->ListLength(args) != 6)
  {
    ErrorReporter::ReportError( 
        "Operator gpattern expects 6 arguments \n but got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////  
  ListExpr streamTuple =  nl->First(args),
  tupleIdent= nl->Second(args),
  liftedPred = nl->Third(args),
  duration = nl->Fourth(args),
  count  =   nl->Fifth(args),
  quantifier = nl->Sixth(args),
  tupleType=0;
  
  nl->WriteToString(argstr, streamTuple);
  if(listutils::isTupleStream(streamTuple)) 
  {
    tupleType = nl->Second(streamTuple);
    if(!listutils::isTupleDescription(tupleType))
      error= true;
  }
  else
    error= true;
  
  if(error)
  {
    ErrorReporter::ReportError(
       "Operator gpattern expects a stream(tuple) as the first "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////    
  nl->WriteToString(argstr, tupleIdent);
  if(listutils::isMap<1>(tupleIdent))
  {
    if(nl->Second(tupleIdent) == tupleType)
    {
      if(!nl->Equal(nl->Third(tupleIdent), nl->SymbolAtom(CcInt::BasicType())))
        error= true;
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator gpattern: error in the tuple type passed to the map in the "
        "second argument. It must be the same as the tuple type in the "
        "stream(tuple) in the first arguement. "
        "\nOperator gpattern got: " + argstr + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  else
    error= true;
  
  if(error)
  {
    ErrorReporter::ReportError(
       "Operator gpattern expects a (map tuple int) as a second "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////  
  nl->WriteToString(argstr, liftedPred);
  if(listutils::isMap<1>(liftedPred))
  {
    if(nl->Second(liftedPred) == tupleType)
    {
      if(!nl->Equal(nl->Third(liftedPred), nl->SymbolAtom(MBool::BasicType())))
        error= true;
    }
    else
    {
      ErrorReporter::ReportError(
        "Operator gpattern: error in the tuple type passed to the map in the "
        "third argument. It must be the same as the tuple type in the "
        "stream(tuple) in the first arguement. "
        "\nOperator gpattern got: " + argstr + ".");
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
  }
  else
    error= true;
  
  if(error)
  {
    ErrorReporter::ReportError(
       "Operator gpattern expects a (map tuple mbool) as the third "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////        
  nl->WriteToString(argstr, duration);
  if( !nl->IsAtom(duration) ||
       nl->SymbolValue(duration) != Duration::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects a duration as the fourth argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////          
  nl->WriteToString(argstr, count);
  if( !nl->IsAtom(count) || nl->SymbolValue(count) != CcInt::BasicType())
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects an int as the fifth argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }
//////////////////////////        
  nl->WriteToString(argstr, quantifier);
  if( !nl->IsAtom(quantifier) || 
    ( nl->SymbolValue(quantifier) != "exactly" && 
      nl->SymbolValue(quantifier) != "atleast"))
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects a quantifier (exactly or atleast) as the "
        "last argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr result =
    nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), nl->SymbolAtom("mset"));
  if(debugme)
  {
    cout<<endl<<endl<<"Operator gpattern accepted the input";
    cout.flush();
  }
  return result;
}

/*
Type map ReportPattern

*/

ListExpr ReportPatternTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 4,
      "Operator reportpattern expects 4 arguments \n but got: " 
      + argstr + ".");
  
  ListExpr streamExpr = nl->First(args),   //stream(tuple(int, mx))
  NamedPatternoidList  = nl->Second(args),  //named list of patternoids
  ConstraintList = nl->Third(args),    //TConstraint list
  BoolCondition  = nl->Fourth(args);    

  nl->WriteToString(argstr, streamExpr);
  CHECK_COND( listutils::isTupleStream(streamExpr) ,
      "Operator reportpattern expects stream(tuple(X)) as first argument."
      "\nBut got: " + argstr + ".");
  
  ListExpr errorInfo;
  ListExpr tuple = nl->Second(nl->Second(streamExpr));
  nl->WriteToString(argstr, streamExpr);
  CHECK_COND( nl->ListLength(tuple) == 2 &&
    nl->IsAtom     (nl->Second(nl->First (tuple))) &&
    nl->SymbolValue(nl->Second(nl->First (tuple)))== CcInt::BasicType() &&
    nl->IsAtom     (nl->Second(nl->Second(tuple))) &&
    am->CheckKind(Kind::TEMPORAL(), nl->Second(nl->Second(tuple)), errorInfo),
        "Operator reportpattern expects stream(tuple(int mx)) as first "
        "argument.\nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, NamedPatternoidList);
  CHECK_COND( ! nl->IsAtom(NamedPatternoidList) ,
      "Operator  reportpattern expects as second argument a "
      "list of aliased patternoid reporting functions.\n"
      "But got: '" + argstr + "'.\n" );
  
  ListExpr NamedPatternoidListRest = NamedPatternoidList;
  ListExpr NamedPatternoid;
  vector<ListExpr> aliases;
  vector<ListExpr>::iterator it; 
  while( !nl->IsEmpty(NamedPatternoidListRest) )
  {
    NamedPatternoid = nl->First(NamedPatternoidListRest);
    NamedPatternoidListRest = nl->Rest(NamedPatternoidListRest);
    nl->WriteToString(argstr, NamedPatternoid);

    CHECK_COND
    ((nl->ListLength(NamedPatternoid) == 2 &&
        nl->IsAtom(nl->First(NamedPatternoid))&&
        listutils::isMap<1>(nl->Second(NamedPatternoid))&&
        listutils::isDATAStream(nl->Third(nl->Second(NamedPatternoid)))&&
        nl->IsAtom((nl->Second(nl->Third(nl->Second(NamedPatternoid)))))&&
        nl->SymbolValue((nl->Second(nl->Third(nl->Second(NamedPatternoid)))))
        == "mset"),
        "Operator reportpattern expects a list of aliased patternoid "
        "reporting operators. \nBut got: " + argstr + ".");
    aliases.push_back(nl->First(NamedPatternoid));
  }

  nl->WriteToString(argstr, ConstraintList);
  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);

    CHECK_COND((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== CcBool::BasicType()),
        "Operator reportpattern expects a list of temporal connectors. "
        "\nBut got: " + argstr + ".");
  }

  nl->WriteToString(argstr, BoolCondition);
  CHECK_COND( nl->IsAtom(BoolCondition) &&
    nl->SymbolValue(BoolCondition) == CcBool::BasicType(),
        "Operator reportpattern expects a boolean condition. "
        "\nBut got: " + argstr + ".");
  
  
  it= aliases.begin();
  ListExpr attr = nl->TwoElemList(*it, nl->SymbolAtom("mset"));
  ListExpr attrList = nl->OneElemList(attr);
  ListExpr lastlistn = attrList;
  ++it;
  while (it != aliases.end())
  {
    attr = nl->TwoElemList(*it, nl->SymbolAtom("mset"));
    lastlistn = nl->Append(lastlistn, attr);
    ++it;
  }

  ListExpr result =
    nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList));
  if(debugme)
  {
    cout<<endl<<endl<<"Operator reportpattern accepted the input";
    cout<< "return type is "<< nl->ToString(result);
    cout.flush();
  }
  return result;
}

ListExpr EmptyMSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->IsEmpty(args),
      "Operator emptymset expects zero paramenters.\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr MBool2MSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2 &&
    nl->IsAtom(nl->First(args)) &&
    nl->SymbolValue(nl->First(args))== MBool::BasicType() &&
    nl->IsAtom(nl->Second(args)) &&
    nl->SymbolValue(nl->Second(args))== CcInt::BasicType(),
      "Operator mbool2mset expects (mbool int)\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr UnionMSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2 &&
    nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mset" &&
    nl->IsAtom(nl->Second(args)) &&  
    nl->SymbolValue(nl->Second(args))== "mset",
      "Operator union expects (mset mset)\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr Union2MSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2 &&
    nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mset" &&
    nl->IsAtom(nl->Second(args)) &&  
    nl->SymbolValue(nl->Second(args))== "mset",
      "Operator union expects (mset mset)\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr CardinalityMSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND( nl->ListLength(args)==1 &&  nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mset" ,
      "Operator cardinality expects mset\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom(MInt::BasicType());
  return result;
}

/*
TConstraint

*/

ListExpr TConstraintTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  ListExpr alias1 = nl->First(args),   
  alias2  = nl->Second(args),      
  temporalconnector = nl->Third(args); 

  nl->WriteToString(argstr, alias1);
  CHECK_COND(( nl->IsAtom(alias1)&&
      nl->SymbolValue(alias1)== CcString::BasicType()),
      "Operator tconstraint expects a predicate label as first "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, alias2);
  CHECK_COND(( nl->IsAtom(alias2)&&
      nl->SymbolValue(alias2)== CcString::BasicType()),
      "Operator tconstraint: expects a predicate label as second "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, temporalconnector);
  CHECK_COND(( nl->IsAtom(temporalconnector)&&
      nl->SymbolValue(temporalconnector)== "stvector"),
      "Operator tconstraint: expects a temporal connector as third "
      "argument.\n But got '" + argstr + "'.");

  ListExpr result = nl->SymbolAtom(CcBool::BasicType());
  if(debugme)
  {
    cout<<endl<<endl<<"Operator tconstraint accepted the input";
    cout.flush();
  }
  return result;
}

//operators
template <int Alfa>
ListExpr CreateAlfaSetTM(ListExpr args)
{
  bool debugme= false;
  //type names according to secondo type mapping
  string thetypes[]={CcInt::BasicType(), CcReal::BasicType(),
                    CcString::BasicType(), CcBool::BasicType()};
  string alfa(thetypes[Alfa]);
  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2,
      "Operator stream2set expects 2 arguments \n but got: " 
      + argstr + ".");
  
  ListExpr streamExpr = nl->First(args);   //stream(tuple(DATA))

  nl->WriteToString(argstr, nl->First(args));
  CHECK_COND( listutils::isTupleStream(streamExpr) ,
      "Operator stream2set expects stream(tuple(.)) as first "
      "argument.\nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) ,
        "Operator stream2set expects as second argument an "
        "attribute name.\nBut got: '" + argstr + "'.\n" );

  nl->WriteToString(argstr, args);
  ListExpr attrType;
  int attrIndex= 
    listutils::findAttribute(nl->Second(nl->Second(streamExpr)), 
        nl->ToString(nl->Second(args)), 
        attrType);
  
  
  CHECK_COND( attrIndex != 0,
        "Operator  stream2set expects as second argument an "
        "attribute name that belongs to the first argument.\n"
        "But got: '" + argstr + "'.\n" );
  
  CHECK_COND( nl->IsEqual(attrType, alfa),
        "Operator  stream2set expects a attribute of type " + alfa +
        ".\n But got: '" + argstr + "'.\n" );

  ListExpr res= nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(nl->IntAtom(attrIndex), attrType),
      nl->SymbolAtom("intset"));
  if(debugme)
  {
    cout<<endl<<endl<<"Operator create" + alfa + "set accepted the input";
    cout.flush();
  }
  return res;
}

ListExpr MSet2MRegionTM(ListExpr args)
{
  string msg= nl->ToString(args);
  if(nl->ListLength(args) != 3)
  {
    ErrorReporter::ReportError("Operator mset2mregion expects 3 arguments."
        "\nBut got: " + msg + ".");
    return nl->SymbolAtom("typeerror");
  };
  
  msg= nl->ToString(nl->First(args));
  if(!listutils::isTupleStream(nl->First(args)))
  {
    ErrorReporter::ReportError("Operator mset2mregion expects stream(tuple(X)) "
        "as first argument.\nBut got: " + msg + ".");
    return nl->SymbolAtom("typeerror");
  };

  msg= nl->ToString(nl->Second(args));
  if(!nl->IsAtom(nl->Second(args)||nl->SymbolValue(nl->Second(args))!="mset"))
  {
    ErrorReporter::ReportError("Operator mset2mregion expects an mset as second"
        " argument.\nBut got: " + msg + ".");
    return nl->SymbolAtom("typeerror");
  };

  msg= nl->ToString(nl->Third(args));
  if(!nl->IsAtom(nl->Third(args)||
      nl->SymbolValue(nl->Third(args))!= Duration::BasicType()))
  {
    ErrorReporter::ReportError("Operator mset2mregion expects duration as "
        "third argument.\nBut got: " + msg + ".");
    return nl->SymbolAtom("typeerror");
  };
  
  ListExpr tuple1 = nl->Second(nl->Second(nl->First(args)));
  msg= nl->ToString(tuple1);
  if(nl->ListLength(tuple1) != 2 ||
     !nl->IsAtom(nl->Second(nl->First (tuple1))) ||
     nl->SymbolValue(nl->Second(nl->First (tuple1)))!= CcInt::BasicType() ||
     !nl->IsAtom(nl->Second(nl->Second(tuple1))) ||
     nl->SymbolValue(nl->Second(nl->Second(tuple1)))!= MPoint::BasicType())
  {
    ErrorReporter::ReportError("Operator mset2mregion expects "
        "stream(tuple(int mpoint)) as first "
        "argument.\nBut got: stream(tuple(" + msg + ")).");
    return nl->SymbolAtom("typeerror");
  };
  
  return nl->SymbolAtom(MRegion::BasicType());
}

ListExpr ConvexHullTM(ListExpr args)
{
  string msg= nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 2 ,
      "Operator convexhull expects 2 arguments.\nBut got: " + msg + ".");
  
  msg= nl->ToString(nl->First(args));
  ListExpr strm= nl->First(args);
  CHECK_COND( nl->ListLength(strm) == 2 &&
      nl->IsAtom(nl->First(strm)) &&
      nl->SymbolValue(nl->First(strm)) == Symbol::STREAM() &&
      nl->IsAtom(nl->Second(strm)) &&
      nl->SymbolValue(nl->Second(strm)) == MPoint::BasicType(),
      "Operator convexhull expects stream(mpoint) as first argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) &&
      nl->SymbolValue(nl->Second(args)) == Instant::BasicType(),
      "Operator convexhull expects an instant as second argument."
      "\nBut got: " + msg + ".");

  return nl->SymbolAtom(Region::BasicType());
}

ListExpr MSet2MPointsTM(ListExpr args)
{
  string msg= nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 3 ,
      "Operator mset2mpoints expects 3 arguments.\nBut got: " + msg + ".");
  
  msg= nl->ToString(nl->First(args));
  CHECK_COND( listutils::isTupleStream(nl->First(args)) ,
      "Operator mset2mpoints expects stream(tuple(X)) as first argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) && 
      nl->SymbolValue(nl->Second(args)) == "mset",
      "Operator mset2mpoints expects an mset as second argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Third(args));
  CHECK_COND( nl->IsAtom(nl->Third(args)) &&
      nl->SymbolValue(nl->Third(args)) == CcBool::BasicType(),
      "Operator mset2mpoints expects a bool as third argument."
      "\nBut got: " + msg + ".");
    
  ListExpr tuple1 = nl->Second(nl->Second(nl->First(args)));
  msg= nl->ToString(tuple1);
  CHECK_COND( nl->ListLength(tuple1) == 2 &&
    nl->IsAtom     (nl->Second(nl->First (tuple1))) &&
    nl->SymbolValue(nl->Second(nl->First (tuple1)))== CcInt::BasicType() &&
    nl->IsAtom     (nl->Second(nl->Second(tuple1))) &&
    nl->SymbolValue(nl->Second(nl->Second(tuple1)))== MPoint::BasicType(),
        "Operator mset2mpoints expects stream(tuple(int mpoint)) as first "
        "argument.\nBut got: stream(tuple(" + msg + ")).");

  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->SymbolAtom(MPoint::BasicType()));
}


ListExpr
IntersectsTM( ListExpr args )
{
  string argstr;
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ), //first argument
    arg2 = nl->Second( args );//second argument
    if(nl->IsEqual( arg1, "intset" ) && nl->IsEqual( arg2, "intset" ))
      return nl->SymbolAtom( CcBool::BasicType() );
  }
  nl->WriteToString(argstr, args);
  ErrorReporter::ReportError("typemap error in operator intersects. "
      "Operator  received: " + argstr);
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

ListExpr TheUnitTM( ListExpr args )
{
  string argstr;
  nl->WriteToString(argstr, args);
  if (argstr == "(intset instant instant bool bool)")
    return nl->SymbolAtom( "uset" );
  ErrorReporter::ReportError
    ("Operator 'the_unit' expects a list with structure\n"
     "'(intset instant instant bool bool)'"
     ", but it gets a list of type \n'" + argstr + "'.");
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


ListExpr TheMValueTM( ListExpr args )
{
  string argstr;

  // quick check for signature (stream uT) --> mT
  nl->WriteToString(argstr, args);
  if ( argstr == "((stream uset))" )    return nl->SymbolAtom( "mset" );
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

ListExpr CollectIntSetTypeMap(ListExpr args) {
    string operatorName;
    string resultType;

    const string errMsg = "Operator " + operatorName
                                      + " expects (stream int)";
    ListExpr argStream;
    ListExpr argType;

    if ( nl->ListLength(args) == 1 )
    {
      argStream = nl->First(args);

      if ( !nl->IsAtom(argStream) && nl->ListLength(argStream) == 2)
      {
          argType = nl->Second(argStream);
          if ( nl->IsEqual(nl->First(argStream), Symbol::STREAM())
             && nl->IsEqual(nl->Second(argStream), CcInt::BasicType()))
              return nl->SymbolAtom("intset");
      }
    }
    ErrorReporter::ReportError(errMsg);
    return nl->TypeError();
}

ListExpr
NoComponentsTM( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mset" ))
      return nl->SymbolAtom( CcInt::BasicType() );
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


ListExpr GBoidsTM( ListExpr args )
{
  string errMsg= "";
  if ( nl->ListLength( args ) != 4 )
  {
    errMsg= "generateboids expects 4 arguments, but got:"
        + nl->ToString(args);
    ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom( Symbol::TYPEERROR() );
  }

  ListExpr BoidSizes = nl->First(nl->First( args )),
     Obstacles= nl->First(nl->Second( args )),
     ObstaclesExpr= nl->Second(nl->Second( args )),
     SimulationStartTime= nl->First(nl->Third( args )),
     SimulationDuration= nl->First(nl->Fourth( args ));

  ListExpr expected= nl->TwoElemList(nl->SymbolAtom(Vector::BasicType()),
      nl->SymbolAtom(CcInt::BasicType()));
  if(!nl->Equal(BoidSizes, expected))
  {
    errMsg= " generateboids expects "+ Vector::BasicType() +
        nl->ToString(expected) +
        " as first argument, but got:" + nl->ToString(BoidSizes);
    ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom( Symbol::TYPEERROR());
  }

  expected= nl->TwoElemList(nl->SymbolAtom(Vector::BasicType()),
        nl->SymbolAtom(CcReal::BasicType()) );
  if(! nl->Equal(Obstacles, expected))
  {
    errMsg= " generateboids expects "+
        nl->ToString(expected) +
        " as second argument, but got:" + nl->ToString(BoidSizes);
    ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom( Symbol::TYPEERROR());
  }

  if( nl->IsAtom(ObstaclesExpr) ||
      nl->ListLength(ObstaclesExpr) < 4 ||
      ((nl->ListLength(ObstaclesExpr) - 1) % 3) != 0)
  {
    errMsg= " generateboids expects as a second argument "
        "a vector of obstacles (circles) in the "
        "format x1, y1, r1, ..., xn, yn, rn, where (xk, yk) are the center of "
        "an obstacle, and rk is its radius in meters. There must be at least "
        "one obstacle representing the world boundaries. "
        "The operator got:" + nl->ToString(ObstaclesExpr);
    ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom( Symbol::TYPEERROR());
  }

  if(! nl->IsEqual( SimulationStartTime, DateTime::BasicType()))
  {
    errMsg= " generateboids expects "+ DateTime::BasicType() +
        " as third argument, but got:"  + nl->ToString(args);
    ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom( Symbol::TYPEERROR());
  }

  if(! nl->IsEqual( SimulationDuration, Duration::BasicType()))
  {
    errMsg= " generateboids expects " + Duration::BasicType() + ""
        " as fourth argument, but got:" + nl->ToString(args);
    ErrorReporter::ReportError(errMsg);
    return nl->SymbolAtom( Symbol::TYPEERROR());
  }

  NList resTupleType =
      NList(NList("BoidID"), NList(CcInt::BasicType())).enclose();
  resTupleType.append(NList(NList("T"),NList(Instant::BasicType())));
  resTupleType.append(NList(NList("X"),NList(CcReal::BasicType())));
  resTupleType.append(NList(NList("Y"),NList(CcReal::BasicType())));
  NList resType =
        NList(NList(Symbol::STREAM()),
              NList(NList(Tuple::BasicType()),resTupleType));

  return resType.listExpr();
}
/*
Value map ReportPattern

*/

int 
ReportPatternVM(Word* args, Word& result,int message, Word& local, Supplier s)
{
  bool debugme=false;
  if(debugme)
  {
    cout<<" Inside ReportPatternVM\n";
    cout<<" Message = " <<message<<endl;
    //    qp->ListOfTree(s, cout);
    cout.flush();
  }

  switch( message )
  {
  case OPEN: // Construct and Solve the CSP, then store it in the "local" 
  { 
    Supplier stream, namedpatternoidlist, namedpatternoid,alias, patternoid, 
    constraintlist, filter, constraint, alias1, alias2, tvector;
    Word Value;
    string aliasstr, alias1str, alias2str;
    int noofpatternoids, noofconstraints;

    stream = args[0].addr;
    namedpatternoidlist = args[1].addr;
    constraintlist= args[2].addr;
    filter= args[3].addr;

    noofpatternoids= qp->GetNoSons(namedpatternoidlist);
    noofconstraints= qp->GetNoSons(constraintlist);

    GPSolver.Clear();
    GPSolver.TheStream= stream;
    for(int i=0; i< noofpatternoids; i++)
    {
      namedpatternoid= qp->GetSupplierSon(namedpatternoidlist, i);
      alias= qp->GetSupplierSon(namedpatternoid, 0);
      patternoid = qp->GetSupplierSon(namedpatternoid, 1);
      aliasstr= nl->ToString(qp->GetType(alias));
      GPSolver.AddVariable(aliasstr,patternoid);
    }

    for(int i=0; i< noofconstraints; i++)
    {
      constraint = qp->GetSupplierSon(constraintlist, i);
      alias1= qp->GetSupplierSon(constraint, 0);
      alias2= qp->GetSupplierSon(constraint, 1);
      tvector= qp->GetSupplierSon(constraint, 2);

      qp->Request(alias1, Value);
      alias1str= ((CcString*) Value.addr)->GetValue();
      qp->Request(alias2, Value);
      alias2str= ((CcString*) Value.addr)->GetValue();
      GPSolver.AddConstraint(alias1str,alias2str, tvector);
    }

    GPSolver.Solve();
    return 0;
  }
  case REQUEST: { // return the next stream element
   bool Part2=false;
   Supplier filter= args[3].addr;
   Word Value;
   while(!Part2 && GPSolver.MoveNext())
   {
     qp->Request(filter, Value);
     Part2= ((CcBool*)Value.addr)->GetValue();
   }
   if(Part2)
   {
     result = qp->ResultStorage( s );
     Tuple* tuple= 
       new Tuple(static_cast<Tuple*>(result.addr)->GetTupleType() );
     GPSolver.WriteTuple(tuple);
     result.setAddr(tuple);
     return YIELD;
   }
   result.addr = 0;
   return CANCEL;
  }
  case CLOSE: { // free the local storage

    GPSolver.Clear();
    local.addr = 0;
  }

  return 0;
  }
  return 0;
}


int TConstraintVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  cerr<< "\nThe operator tconstraint is a facked operator. It may only be "
      "called within the reportpattern operator.";
  assert(0); //this function should never be invoked.
  return 0;
}


int GPatternVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  switch( message )
  {
  case OPEN: 
  { 
/*
Defining local variables

*/    
    Word t, value;
    Tuple* tup;
    MBool* mbool;
    int id;
    CompressedInMemMSet accumlator;
    vector<InMemMSet>* resStream = new vector<InMemMSet>();
    GPatternHelper GPHelper;

/*
Reading the operator arguments

*/
    Supplier TheStream= args[0].addr, 
    tupleID= args[1].addr,  
    liftedPred = args[2].addr;
    ArgVectorPointer tupleIDArgs = qp->Argument(tupleID),
      liftedPredArgs = qp->Argument(liftedPred);
    qp->Request(args[3].addr, value);
    int64_t dMS= static_cast<Instant*>(value.addr)->millisecondsToNull();
    qp->Request(args[4].addr, value);
    int n= static_cast<CcInt*>(value.addr)->GetIntval();
    string qts= nl->ToString(qp->GetType(args[5].addr));
    GPattern::quantifier q= (qts=="exactly")?
        GPattern::exactly : GPattern::atleast;

/*
Evaluating the gpattern results

*/    
    qp->Open(TheStream);
    qp->Request(TheStream, t);
    while (qp->Received(TheStream))
    {
      tup = static_cast<Tuple*>(t.addr);
      
      (*tupleIDArgs)[0] = tup;
      qp->Request(tupleID, value);
      id= static_cast<CcInt*>(value.addr)->GetIntval();
      
      (*liftedPredArgs)[0] = tup;
      qp->Request(liftedPred, value);
      mbool = static_cast<MBool*>(value.addr);
      if (mbool->IsDefined())
      {
        accumlator.Buffer(*mbool, id);
        //accumlator2.Union(*mbool, id);
      }
      tup->DeleteIfAllowed();
      qp->Request(TheStream, t);
    }

    accumlator.ConstructFromBuffer();
    qp->Close(TheStream);

    if(debugme)
    {
      MSet tmp1(0), tmp2(0);
      accumlator.WriteToMSet(tmp1);
      //accumlator2.WriteToMSet(tmp2);
      if(tmp1 != tmp2)
      {
        tmp1.Print(cerr);
        //tmp2.Print(cerr);
      }
    }
    
    bool changed= true;
    while(changed && accumlator.GetNoComponents() > 0)
    {
      //accumlator2.RemoveSmallUnits(n);
      changed= accumlator.RemoveSmallUnits(n);

      //accumlator2.RemoveShortElemParts(d);
      changed= (accumlator.RemoveShortElemParts(dMS) || changed );
    }
    if(debugme)
    {
      MSet tmp1(0), tmp2(0);
      accumlator.WriteToMSet(tmp1);
      //accumlator2.WriteToMSet(tmp2);
      if(tmp1 != tmp2)
      {
        tmp1.Print(cerr);
        //tmp2.Print(cerr);
      }
    }
    
    list<CompressedInMemUSet>::iterator begin= 
      accumlator.units.begin(), end, tmp;
    //cast the CompressedInMemMSet into an InMemMSet
    begin != accumlator.units.end();
    while(begin != accumlator.units.end())
    {
      end= accumlator.GetPeriodEndUnit(begin);
      if(debugme)
      {
        (*begin).Print(cerr);
        (*end).Print(cerr);
      }
      if(q == GPattern::atleast)
      {
        InMemMSet* mset= new InMemMSet();
        tmp= end;
        ++tmp;
        accumlator.WriteToInMemMSet(*mset, begin, tmp);
        if(debugme)
        {
          MSet tmp1(0);
          mset->WriteToMSet(tmp1);
          tmp1.Print(cerr);
        }        
        resStream->push_back(*mset);
      }
      else
      {
        InMemMSet* mset= new InMemMSet();
        tmp= end;
        ++tmp;
        accumlator.WriteToInMemMSet(*mset, begin, tmp);
        if(debugme)
        {
          MSet tmp1(0);
          mset->WriteToMSet(tmp1);
          tmp1.Print(cerr);
        } 

        list<InMemUSet>::iterator e= mset->units.end();
        --e;
        GPHelper.ComputeAddSubSets(*mset, mset->units.begin(), e,
            n, dMS, resStream);
      }
      begin= ++end;
    }
    local= SetWord(resStream);
    return 0;
  }
  case REQUEST: { // return the next stream element

    vector<InMemMSet>* resStreams= static_cast<vector<InMemMSet>*>(local.addr); 
    if ( resStreams->size() != 0)
    {
      MSet* res= new MSet(0);
      (*resStreams->begin()).WriteToMSet(*res);
      resStreams->erase(resStreams->begin());
      result= SetWord(res);  
      return YIELD;
    }
    else
    {
      // you should always set the result to null
      // before you return a CANCEL
      result.addr = 0;
      return CANCEL;
    }

  }
  case CLOSE: { // free the local storage
    vector<InMemMSet>* resStream= static_cast<vector<InMemMSet>* >(local.addr);
    resStream->clear();
    delete resStream;
    local.addr = 0;
  }

  return 0;
  }
  return 0;
}

/*
Value map CrossPattern

*/

int CrossPatternVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  switch( message )
  {
  case OPEN: 
  { 
/*
Defining local variables

*/    
    Word t, value;
    Tuple* tup;
    MBool* mbool;
    int id1, id2, tupleCnt=0;
    vector<pair<int, int> > edge2nodesMap(0);
    CompressedInMemMSet accumlator;
    //InMemMSet accumlator2;
    list<CompressedMSet*>* resStream= 
      new list<CompressedMSet*>(), *resStreamFinal=0,
      *localResStream=0;
    GPatternHelper GPHelper;

/*
Reading the operator arguments

*/
    Supplier TheStream= args[0].addr, 
    tupleID1= args[1].addr,  
    tupleID2= args[2].addr,
    liftedPred = args[3].addr;
    ArgVectorPointer tupleID1Args = qp->Argument(tupleID1),
      tupleID2Args = qp->Argument(tupleID2),
      liftedPredArgs = qp->Argument(liftedPred);
    qp->Request(args[4].addr, value);
    int64_t dMS= static_cast<Instant*>(value.addr)->millisecondsToNull();
    qp->Request(args[5].addr, value);
    int n= static_cast<CcInt*>(value.addr)->GetIntval();
    string qts= nl->ToString(qp->GetType(args[6].addr));
    
/*
Evaluating the gpattern results

*/    
    qp->Open(TheStream);
    qp->Request(TheStream, t);
    while (qp->Received(TheStream))
    {
      tup = static_cast<Tuple*>(t.addr);
      
      (*tupleID1Args)[0] = tup;
      qp->Request(tupleID1, value);
      id1= static_cast<CcInt*>(value.addr)->GetIntval();
      
      (*tupleID2Args)[0] = tup;
      qp->Request(tupleID2, value);
      id2= static_cast<CcInt*>(value.addr)->GetIntval();
     
      (*liftedPredArgs)[0] = tup;
      qp->Request(liftedPred, value);
      mbool = static_cast<MBool*>(value.addr);
      if (mbool->IsDefined())
      {
/*
Using bool inc= accumlator.Buffer(mbool, tupleCnt, d) is wrong. A series of
short edges might lead that a node is part of a component long enough. The
correct objective is to filter out short node appearance, which cannot be
performed in this step.

*/

        //bool inc= accumlator.Buffer(*mbool, tupleCnt, d);
        bool inc= accumlator.Buffer(*mbool, tupleCnt);
        //GPHelper.removeShortUnits(*mbool, d);
        //accumlator2.Union(*mbool, tupleCnt);
        if(inc) 
        {
          edge2nodesMap.push_back(make_pair(id1, id2));
          ++tupleCnt;
        }
        //accumlator.Buffer(*mbool, tupleCnt);
      }
      tup->DeleteIfAllowed();
      qp->Request(TheStream, t);
    }

    accumlator.ConstructFromBuffer();
    qp->Close(TheStream);

    if(debugme )
    {
      MSet tmp1(0), tmp2(0);
      accumlator.WriteToMSet(tmp1);
      //accumlator2.WriteToMSet(tmp2);
      if(tmp1 != tmp2)
      {
        tmp1.Print(cerr);
        //tmp2.Print(cerr);
      }
    }
    bool changed= true;
    while(changed && accumlator.GetNoComponents() > 0 && 0)
    {
      //accumlator2.RemoveSmallUnits(n);
   //changed= GPHelper.RemoveUnitsHavingFewNodes(accumlator, edge2nodesMap, n);

      //accumlator2.RemoveShortElemParts(d);
      //This function removes short edges. This is incorrect. It should instead
      //remove short nodes
      //changed= (accumlator.RemoveShortElemParts(d) || changed );
    }
    
    if(debugme && 0)
    {
      MSet tmp1(0), tmp2(0);
      accumlator.WriteToMSet(tmp1);
      //accumlator2.WriteToMSet(tmp2);
      if(tmp1 != tmp2)
      {
        tmp1.Print(cerr);
        tmp2.Print(cerr);
      }
    }
    
    list<CompressedInMemUSet>::iterator begin= 
      accumlator.units.begin(), end, tmp;
    //cast the CompressedInMemMSet into an InMemMSet
    while(begin != accumlator.units.end())
    {
      end= accumlator.GetPeriodEndUnit(begin);
      if(debugme)
      {
        accumlator.Print(cerr);
        (*begin).Print(cerr);
        (*end).Print(cerr);
      }
      if((*end).endtime - (*begin).starttime >= dMS)
      {
        ++end;
        if(begin != accumlator.units.begin())
        {
          CompressedInMemMSet accumlatorPart(accumlator, begin, end);
          if(debugme)
          {
            MSet tmp1(0);
            //accumlatorPart.WriteToMSet(tmp1);
            //accumlator2.WriteToMSet(tmp2);
            //tmp1.Print(cerr);
          }
          GPHelper.FindLargeDynamicComponents(accumlatorPart,
              accumlatorPart.units.begin(), accumlatorPart.units.end() , 
              edge2nodesMap, dMS, n, qts, localResStream, 0);
        }
        else
        {
          GPHelper.FindLargeDynamicComponents(accumlator, begin, end ,
                edge2nodesMap, dMS, n, qts, localResStream, 0);
        }
        resStream->splice(resStream->end(), *localResStream);
        delete localResStream;
        localResStream= 0;
      }
      else
        ++end;
      begin= end;
    }
    resStreamFinal= new list<CompressedMSet*>();
    for(list<CompressedMSet*>::iterator 
        it= resStream->begin(); it!= resStream->end(); ++it)
    {
      if(debugme)
        (*it)->Print(cerr);
      resStreamFinal->push_back(GPHelper.EdgeMSet2NodeMSet(*it, edge2nodesMap));
      delete *it;
      *it = 0;
    }
    delete resStream;
    
    local= SetWord(resStreamFinal);
    return 0;
  }
  case REQUEST: { // return the next stream element

    list<CompressedMSet*>* resStream= 
      static_cast< list<CompressedMSet*>* >(local.addr);
    CompressedInMemMSet tmp;  
    if ( resStream->size() != 0)
    {
      MSet* res= new MSet(0);
      resStream->front()->WriteToCompressedInMemMSet(tmp);
      tmp.WriteToMSet(*res);
      delete resStream->front();
      resStream->pop_front();
      result= SetWord(res);  
      return YIELD;
    }
    else
    {
      // you should always set the result to null
      // before you return a CANCEL
      result.addr = 0;
      return CANCEL;
    }

  }
  case CLOSE: { // free the local storage
    list<CompressedMSet*>* resStream= 
      static_cast<list<CompressedMSet*>* >(local.addr);
    for(list<CompressedMSet*>::iterator 
        it= resStream->begin(); it!= resStream->end(); ++it)
    {
      delete *it;
      *it = 0;
    }
    resStream->clear();
    delete resStream;
    local.addr = 0;
  }

  return 0;
  }
  return 0;
}

int EmptyMSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  static_cast<MSet*>(result.addr)->Clear();
  static_cast<MSet*>(result.addr)->SetDefined(true);
  return 0;
}

int MBool2MSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MSet* res= static_cast<MSet*>(result.addr);
  MBool * arg1= static_cast<MBool*>(args[0].addr);
  CcInt * arg2= static_cast<CcInt*>(args[1].addr);
  res->MBool2MSet(*arg1, arg2->GetIntval());
  return 0;
}

int UnionMSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MSet* res= static_cast<MSet*>(result.addr);
  MSet* op1= static_cast<MSet*>(args[0].addr);
  MSet* op2= static_cast<MSet*>(args[1].addr);
  op1->LiftedUnion(*op2, *res);
  return 0;
}

int Union2MSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MSet* res= static_cast<MSet*>(result.addr);
  MSet* op1= static_cast<MSet*>(args[0].addr);
  MSet* op2= static_cast<MSet*>(args[1].addr);
  op1->LiftedUnion2(*op2, *res);
  return 0;
}

int CardinalityMSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MInt* res= static_cast<MInt*>(result.addr);
  MSet* arg= static_cast<MSet*>(args[0].addr);
  arg->LiftedCount(*res);
  return 0;
}


template <bool Row>
int RowColVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  return 0;
}

template <class Alfa>
int CreateAlfaSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  //bool debugme= false;
  Word t, value;
  Tuple* tup;
  IntSet* res=0;
  int elem;
  
  int attrIndex= static_cast<CcInt*>(args[2].addr)->GetIntval();
  //string attrType= static_cast<CcString*>(args[3].addr)->GetValue();
  result = qp->ResultStorage( s );
  res= static_cast< IntSet* > (result.addr);
  res->Clear();
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, t);
  while (qp->Received(args[0].addr))
  {
    tup = (Tuple*)t.addr;
    elem= dynamic_cast< CcInt* > (tup->GetAttribute(attrIndex-1))->GetIntval();
    res->Insert(elem);
    qp->Request(args[0].addr, t);
    res->Print(cerr);
  }
  qp->Close(args[0].addr);
  return 0;
}

void 
MPointsSample(Instant& curTime, vector<MPoint*>& sourceMPoints, Points& res,
    bool checkDefined)
{
  bool debugme=false;
  if(debugme)
    curTime.Print(cerr);
  res.Clear();
  Point curPoint(0, 0);
  Intime<Point> pointIntime(curTime, curPoint);
  for(unsigned int i=0; i<sourceMPoints.size(); ++i)
  {
    sourceMPoints[i]->AtInstant(curTime, pointIntime);
//    if(checkDefined)
      assert(pointIntime.IsDefined());
    if(debugme)
      pointIntime.Print(cerr);
    res += pointIntime.value;
  }
}

void 
MPointsSample(Instant& curTime, set<int>& idset, 
    map<int, MPoint*>& sourceMPoints, Points& res, bool checkDefined)
{
  bool debugme=false;
  if(debugme)
    curTime.Print(cerr);
  res.Clear();
  Point curPoint(0, 0);
  Intime<Point> pointIntime(curTime, curPoint);
  set<int>::iterator id= idset.begin();
  for(; id != idset.end(); ++id)
  {
    sourceMPoints[*id]->AtInstant(curTime, pointIntime);
//    if(checkDefined)
//      assert(pointIntime.IsDefined());
    if(debugme)
      pointIntime.Print(cerr);
    if(pointIntime.IsDefined()) res += pointIntime.value;
  }
}


void AppendMRegionPart(set<int>& idset, map<int, MPoint*>& sourceMPoints, 
    Interval<Instant>& unitBoundary,
    Instant& samplingDuration,
    MRegion& res)
{
  bool debugme=false;
    vector<double> weigths(4);
    weigths[0] = 0.7;            // AreaWeight
    weigths[1] = 0.7;            // OverlapWeight
    weigths[2] = 0.5;            // HausdorffWeight
    weigths[3] = 1.0;            // LinearWeight
  
    Instant curTime(unitBoundary.start);
    Interval<Instant> 
      unitInterval(curTime, unitBoundary.end, unitBoundary.lc, false);
    
    Points ps(0);
    Region* reg1=new Region(0), *reg2=new Region(0), *regswap;
    RegionForInterpolation *reginter1, *reginter2, *reginterswap;
    Match *sm;
    mLineRep *lines;
    URegion *resUnit;
    bool firstIteration= true;
    
    MPointsSample(curTime, idset, sourceMPoints, ps, true);
    GrahamScan::convexHull(&ps,reg1);
    reginter1=new RegionInterpol::RegionForInterpolation(reg1);
    curTime+= samplingDuration;
    while(curTime < unitBoundary.end)
    {
      if(!firstIteration)
        unitInterval.lc= true;
      firstIteration= false;
      MPointsSample(curTime, idset, sourceMPoints, ps, true);
      GrahamScan::convexHull(&ps, reg2);
      unitInterval.end= curTime;
      reginter2=new RegionInterpol::RegionForInterpolation(reg2);
      sm=new OptimalMatch(reginter1, reginter2, weigths);
      lines=new mLineRep(sm);    
      resUnit= new URegion(lines->getTriangles(), unitInterval);
      if(debugme)
        unitInterval.Print(cerr);
      res.AddURegion(*resUnit);
/*
Copying the right part of this URegion to the left part of the next URegion

*/
      
      unitInterval.start= unitInterval.end;
      regswap= reg1;
      reg1= reg2;
      reg2= regswap;
      reginterswap= reginter1;
      reginter1= reginter2;
/*
Garbage collection

*/      
      delete resUnit;
      delete lines;
      delete reginterswap;
      curTime+= samplingDuration;
    }
/*
Adding the last instant in the unit

*/  
    Instant endI(unitBoundary.end);
    if(!unitBoundary.rc)
    {
      Instant milli(0, 1, durationtype);
      endI -= milli;
    } 
    MPointsSample(endI, idset, sourceMPoints, ps, false);
    unitInterval.rc= unitBoundary.rc;
    GrahamScan::convexHull(&ps, reg2);
    unitInterval.end= unitBoundary.end;
    reginter2=new RegionInterpol::RegionForInterpolation(reg2);
    sm=new OptimalMatch(reginter1, reginter2, weigths);
    lines=new mLineRep(sm);    
    resUnit= new URegion(lines->getTriangles(), unitInterval);
    if(debugme)
      unitInterval.Print(cerr);
    res.AddURegion(*resUnit);
    delete resUnit;
    delete lines;
    delete reg1;
    delete reg2;
    delete reginter1;
    delete reginter2;
}

/*
Value map MSet2MRegion

*/

int 
MSet2MRegionVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;
  result = qp->ResultStorage(s);
  MRegion* res = static_cast<MRegion*>( result.addr);
  res->Clear();
  Word Value;
  Supplier arg0= qp->GetSon(s,0);
  Supplier arg1= qp->GetSon(s,1);
  Supplier arg2= qp->GetSon(s,2);
  try{
  qp->Request(arg1, Value);
  MSet* mset= static_cast<MSet*>(Value.addr);
  if(debugme)
    mset->Print(cerr);

  
  if(!mset->IsDefined() || mset->GetNoComponents()==0)
  {
    res->SetDefined(false);
    return 0;
  }
  
  qp->Request(arg2, Value);
  Instant* d= static_cast<Instant*>(Value.addr);
  USetRef usetref;
  USet uset(true);
  set<int> idset;
  set<int>::iterator setIt;
  for(int i=0; i<mset->GetNoComponents(); ++i)
  {
    mset->Get(i, usetref);
    usetref.GetUnit(mset->data, uset);
    if(uset.constValue.Count()!= 0)
    {
      for(int k=0; k< uset.constValue.Count(); ++k)
        idset.insert(uset.constValue[k]);
    }
  }
  
  map<int, MPoint*> mpoints;
  vector<Tuple*> tuplesToDelete(0);
  qp->Open(arg0);
  qp->Request(arg0, Value);
  while(qp->Received(arg0) &&  (mpoints.size() < idset.size()))
  {
    Tuple* tuple= static_cast<Tuple*>(Value.addr);
    int id= dynamic_cast<CcInt*>(tuple->GetAttribute(0))->GetIntval();
    setIt= idset.find(id);
    if(setIt != idset.end())
    {
      MPoint* elem=dynamic_cast<MPoint*>(
          dynamic_cast<MPoint*>(tuple->GetAttribute(1)));
      mpoints[id]= elem;
      tuplesToDelete.push_back(tuple);
    }
    else
      tuple->DeleteIfAllowed();
      
    qp->Request(arg0, Value);
  }
  qp->Close(arg0);  
  if(mpoints.size() != idset.size())
  {
    res->SetDefined(false);
    cerr<<"mset2mregion: not all ids in the mset are "
    "found in the mpoint stream\n";
    for(unsigned int k=0; k<tuplesToDelete.size(); ++k)
      tuplesToDelete[k]->DeleteIfAllowed();
    return 0;
  }
  
  for(int i=0; i<mset->GetNoComponents(); ++i)
  {
    mset->Get(i, usetref);
    usetref.GetUnit(mset->data, uset);
    if(uset.constValue.Count()!= 0)
    {
      idset.clear();
      for(int k=0; k< uset.constValue.Count(); ++k)
        idset.insert(uset.constValue[k]);
      AppendMRegionPart(idset, mpoints, usetref.timeInterval, *d , *res);
    }
  }
  for(unsigned int k=0; k<tuplesToDelete.size(); ++k)
    tuplesToDelete[k]->DeleteIfAllowed();
  }
  catch(...)
  {
    cerr<<"\nUnhandeled exception in mset2mregion ValueMap. "
        "Yielding undefined result.\n";
    res->Clear();
    res->SetDefined(false);
    return 0;
  }
  return 0;
}

//int 
//MSet2MRegionVM(Word* args, Word& result, int message, Word& local, Supplier s)
//{
//  bool debugme=false;
//  result = qp->ResultStorage(s);
//  MRegion* res = static_cast<MRegion*>( result.addr);
//  res->Clear();
//  Word Value;
//  Supplier arg0= qp->GetSon(s,0);
//  Supplier arg1= qp->GetSon(s,1);
//  Supplier arg2= qp->GetSon(s,2);
//  
//  qp->Request(arg1, Value);
//  MSet* mset= static_cast<MSet*>(Value.addr);
//  if(debugme)
//    mset->Print(cerr);
//
//  
//  if(!mset->IsDefined() || mset->GetNoComponents()==0)
//  {
//    res->SetDefined(false);
//    return 0;
//  }
//  
//  qp->Request(arg2, Value);
//  Instant* d= static_cast<Instant*>(Value.addr);
//  USetRef usetref;
//  USet uset(true);
//  set<int> idset;
//  set<int>::iterator setIt;
//  for(int i=0; i<mset->GetNoComponents(); ++i)
//  {
//    mset->Get(i, usetref);
//    usetref.GetUnit(mset->data, uset);
//    if(uset.constValue.Count()!= 0)
//    {
//      idset.clear();
//      for(int k=0; k< uset.constValue.Count(); ++k)
//        idset.insert(uset.constValue[k]);
//      vector<MPoint*> mpoints(0);
//      qp->Open(arg0);
//      qp->Request(arg0, Value);
//      while(qp->Received(arg0))
//      {
//        Tuple* tuple= static_cast<Tuple*>(Value.addr);
//        int id= dynamic_cast<CcInt*>(tuple->GetAttribute(0))->GetIntval();
//        setIt= idset.find(id);
//        if(setIt != idset.end())
//        {
//          MPoint* elem=dynamic_cast<MPoint*>( 
//            dynamic_cast<MPoint*>(tuple->GetAttribute(1))->Clone());
//          mpoints.push_back(elem);
//        }
//        tuple->DeleteIfAllowed();
//        qp->Request(arg0, Value);
//      }
//      if(mpoints.size() != idset.size())
//      {
//        res->SetDefined(false);
//        cerr<<"mset2mregion: not all ids in the mset are "
//        "found in the mpoint stream\n";
//        return 0;
//      }
//      qp->Close(arg0);
//      AppendMRegionPart(mpoints, usetref.timeInterval, *d , *res);
//      for(unsigned int k=0; k<mpoints.size(); ++k)
//        delete mpoints[k];
//    }
//  }
//  return 0;
//}

/*
Value map MSet2MPoints

*/

int 
MSet2MPointsVM(Word* args, Word& result, int message, Word& local, Supplier s)
{

  //bool debugme=false;

  switch( message )
  {
  case OPEN: { // initialize the local storag
    map<int, Periods*>* msetElems= new map<int, Periods*>();      
    Word Value;
    Supplier arg1= qp->GetSon(s,1);
    qp->Request(arg1, Value);
    MSet* mset= static_cast<MSet*>(Value.addr);
    int nocomponents= mset->GetNoComponents();
    if(!mset->IsDefined() || nocomponents==0)
    {
      local=SetWord(msetElems);
      return 0;
    }
    Supplier arg2= qp->GetSon(s,2);
    qp->Request(arg2, Value);    
    bool restrictMPoints= static_cast<CcBool*>(Value.addr)->GetBoolval();  
    
    int nounitcomponents=0, idFromMSet=0;
    map<int, Periods*>::iterator msetElemsIt;
    
    USetRef usetref;
    USet uset(true);
    for(int i=0; i< nocomponents; ++i)
    {
      mset->Get(i, usetref);
      usetref.GetUnit(mset->data, uset);
      nounitcomponents= uset.constValue.Count();
      for(int e= 0; e<nounitcomponents; ++e)
      {
        idFromMSet= uset.constValue[e];
        msetElemsIt= msetElems->find(idFromMSet);
        if(msetElemsIt == msetElems->end())
        {
          Periods* periods= new Periods(1);
          periods->Add(uset.timeInterval);
          (*msetElems)[idFromMSet]= periods;
        }
        else
        {
          if(!restrictMPoints)
            continue;        
          (*msetElemsIt).second->MergeAdd(uset.timeInterval);
        }
      }
    }    
    local=SetWord(msetElems);
    qp->Open( args[0].addr );
    return 0;
  }
  case REQUEST: { // return the next stream element
    map<int, Periods*>* msetElems= static_cast<map<int, Periods*>*>(local.addr);
    if(msetElems->size() == 0)
    {
      result.addr = 0;
      return CANCEL;
    }
    //result = qp->ResultStorage(s);
    Word Value;
    map<int, Periods*>::iterator msetElemsIt; 
    Supplier arg2= qp->GetSon(s,2);
    qp->Request(arg2, Value);  
    bool restrictMPoints= static_cast<CcBool*>(Value.addr)->GetBoolval();  
    
    qp->Request(args[0].addr, Value);
    while(qp->Received(args[0].addr))
    {
      Tuple* tuple= static_cast<Tuple*>(Value.addr);
      int idFromTuple= 
        dynamic_cast<CcInt*>(tuple->GetAttribute(0))->GetIntval();
      msetElemsIt= msetElems->find(idFromTuple);
      if(msetElemsIt != msetElems->end())
      {
        MPoint* res= new MPoint(0);
        if(restrictMPoints)
        {
          MPoint* tmp = dynamic_cast<MPoint*>(tuple->GetAttribute(1));
          tmp->AtPeriods(*((*msetElemsIt).second), *res);
        }
        else
        {
          MPoint* tmp = dynamic_cast<MPoint*>(tuple->GetAttribute(1));
          res->CopyFrom(tmp);
        }
        msetElems->erase(msetElemsIt);
        tuple->DeleteIfAllowed();
        result= SetWord(res);
        return YIELD;
      }
      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, Value);
    }
    cerr<<"operator mset2mpoints: some mset elements are not found in "
        "the stream";
    return FAILURE;
  }
  case CLOSE: { // free the local storage

    map<int, Periods*>* msetElems= static_cast<map<int, Periods*>*>(local.addr);
    map<int, Periods*>::iterator msetElemsIt= msetElems->begin();
    for(; msetElemsIt != msetElems->end(); ++msetElemsIt)
      delete (*msetElemsIt).second;
    delete msetElems;
    result.addr=0;  
    return 0;
  }
  return 0;
  }
  return 0;
}

int 
ConvexHullVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  //bool debugme= false;
  Word Value;
  MPoint* mp=0;
  Instant* instant=0;
  vector<MPoint*> stream(0);
  result = qp->ResultStorage(s);
  
  qp->Open(args[0].addr);
  qp->Request(args[0].addr,Value);
  while (qp->Received(args[0].addr))
  {
    mp = static_cast<MPoint*>(Value.addr);
    stream.push_back(mp);
    qp->Request(args[0].addr, Value);
  }
  Supplier arg1= qp->GetSon(s,1);
  qp->Request(arg1, Value);
  instant= static_cast<Instant*>(Value.addr);
  
  Points ps(0);
  MPointsSample(*instant, stream, ps, true);
  Region* reg=static_cast<Region*>(result.addr);
  reg->Clear();
  GrahamScan::convexHull(&ps, reg);
  qp->Close(args[0].addr);
  return 0;
}

int 
IntersectsVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  Word Value;
  IntSet *arg1= static_cast<IntSet*>(args[0].addr),
         *arg2= static_cast<IntSet*>(args[1].addr);
  result = qp->ResultStorage(s);
  CcBool* res= static_cast<CcBool*>(result.addr);

  bool intersects = arg1->Intersects(*arg2);
  if(debugme)
  {
    cerr<< "\nInside intersects\n arg1:";
    arg1->Print(cerr);
    cerr<<"\narg2:"; 
    arg2->Print(cerr);
    cerr<<"\nresult: "<<intersects;
  }
  res->Set(true, intersects);
  return 0;
}

// the_unit for uset:
// intset instant instant bool bool -> uT
int TheUnitVM(Word* args, Word& result,
                        int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  USet *res = static_cast<USet *>(result.addr);
  IntSet  *value = static_cast<IntSet*>(args[0].addr);
  Instant *i1    = static_cast<DateTime*>(args[1].addr);
  Instant *i2    = static_cast<DateTime*>(args[2].addr);
  CcBool  *cl    = static_cast<CcBool*>(args[3].addr);
  CcBool  *cr    = static_cast<CcBool*>(args[4].addr);
  bool clb, crb;

  // Test arguments for definedness
  if ( !value->IsDefined() ||
       !i1->IsDefined() || !i2->IsDefined() ||
       !cl->IsDefined() || !cr->IsDefined()    ) {
    res->SetDefined( false );
    return 0;
  }

  clb = cl->GetBoolval();
  crb = cr->GetBoolval();

  if ( ( (*i1 == *i2) && (!clb || !crb) )   ||
       ( i1->Adjacent(i2) && !(clb || crb) )  ) { // illegal interval setting
    res->SetDefined( false );
    return 0;
  }
  if ( *i1 < *i2 ) {// sort instants
    Interval<Instant> interval( *i1, *i2, clb, crb );
    res =  new USet( interval, *value );
    result= SetWord(res);
  } else {
    Interval<Instant> interval( *i2, *i1, clb, crb );
    res = new USet( interval, *value );
    result= SetWord(res);
  }
  return 0;
}

int TheMValueVM(Word* args,Word& result,int message,
                      Word& local,Supplier s)
{
  Word currentUnit;
  USet* unit;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentUnit);

  result = qp->ResultStorage(s);
  MSet* m = static_cast<MSet*>(result.addr);
  m->Clear();
  m->SetDefined( true );

  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) ) { // get all tuples
      unit = static_cast<USet*>(currentUnit.addr);
      if(unit == 0) {
        cout << endl << __PRETTY_FUNCTION__ << ": Received Nullpointer!"
             << endl;
        assert( false );
      } else if(unit->IsDefined()) {
        m->MergeAdd( *unit );
      } else {
        cerr << endl << __PRETTY_FUNCTION__ << ": Dropping undef unit "
             << endl;
      }
      unit->DeleteIfAllowed();
      qp->Request(args[0].addr, currentUnit);
  }
  m->EndBulkLoad( true, true ); // force Mapping to sort the units
  qp->Close(args[0].addr);      // and mark invalid Mapping as undefined

  return 0;
}


int CollectIntSetValueMap(
    Word* args, Word& result, int message, Word& local, Supplier s) 
{
  result = qp->ResultStorage(s);
  IntSet* resColl = static_cast<IntSet*>(result.addr);
  resColl->Clear();
  resColl->SetDefined(true);
  CcInt* elemToInsert;
  Word elem = SetWord(Address(0));

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);

  while ( qp->Received(args[0].addr) )
  {
      elemToInsert = static_cast<CcInt*>(elem.addr);
      resColl->Insert(elemToInsert->GetIntval());
      //elemToInsert->DeleteIfAllowed();
      qp->Request(args[0].addr, elem);
  }
  qp->Close(args[0].addr);
  return 0;
}

int NoComponentsVM( 
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MSet* m = ((MSet*)args[0].addr);
  if( m->IsDefined() ){
    ((CcInt*)result.addr)->Set( true,
                              ((MSet*)args[0].addr)->GetNoComponents() );
  } else {
    ((CcInt*)result.addr)->Set( false, 0 );
  }
  return 0;
}

struct GBoidLocalInfo
{
  GBoidLocalInfo(BoidGenerator* bg, TupleType* tt):
    BGenerator(bg), resType(tt){}
  BoidGenerator* BGenerator;
  TupleType* resType;
};

int GBoidsVM(
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  //bool debugme= false;
  switch( message )
  {
  case OPEN:
  {
    Word value;
    collection::Collection* BoidSizes=
        static_cast<collection::Collection*>(args[0].addr);
    collection::Collection* Obstacles=
        static_cast<collection::Collection*>(args[1].addr);
    Instant* SimulationStartTime= static_cast<Instant*>(args[2].addr);
    Instant* SimulationDuration= static_cast<Instant*>(args[3].addr);

    vector<int> boidSizes;
    for(int i=0; i< BoidSizes->GetNoComponents(); ++i)
    {
      CcInt* elem= static_cast<CcInt*>(BoidSizes->GetComponent(i));
      boidSizes.push_back(elem->GetIntval());
      delete elem;
    }
    vector<double> obstacles;
    for(int i=0; i< Obstacles->GetNoComponents(); ++i)
    {
      CcReal* elem= static_cast<CcReal*>(Obstacles->GetComponent(i));
      obstacles.push_back(elem->GetRealval());
      delete elem;
    }

    BoidGenerator* BGenerator= new BoidGenerator( boidSizes,
        obstacles, SimulationStartTime, SimulationDuration);

    TupleType* resType =  new TupleType(nl->Second(GetTupleResultType(s)));
    GBoidLocalInfo* li= new GBoidLocalInfo(BGenerator, resType);
    local.setAddr(li);
    return 0;
  }
  case REQUEST:
  { // return the next stream element
    GBoidLocalInfo* li= static_cast<GBoidLocalInfo*>(local.addr);
    BoidGenerator* BGenerator= li->BGenerator;
    result = qp->ResultStorage( s );
    CcInt* BoidID;
    CcReal* X, *Y;
    int boidID;
    double x, y;
    DateTime SampleTime(0, 0, durationtype);
    if ( BGenerator->GetNext(boidID, SampleTime, x, y) != -1)
    {
      DateTime* sampleTime= new DateTime(SampleTime);
      BoidID= new CcInt(true, boidID);
      X= new CcReal(true, x);
      Y= new CcReal(true, y);
      Tuple* res= new Tuple(li->resType);
      res->PutAttribute(0, (Attribute*)BoidID);
      res->PutAttribute(1, (Attribute*)sampleTime);
      res->PutAttribute(2, (Attribute*)X);
      res->PutAttribute(3, (Attribute*)Y);
      result= SetWord(res);
      return YIELD;
    }
    else
    {
      // you should always set the result to null
      // before you return a CANCEL
      result.addr = 0;
      return CANCEL;
    }

  }
  case CLOSE: { // free the local storage
    GBoidLocalInfo* li= static_cast<GBoidLocalInfo*>(local.addr);
    delete li->BGenerator;
    delete li->resType;
    delete li;
    local.addr = 0;
  }
  return 0;
  }
  return 0;
}
/*
Operator properties

*/

const string RowColSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string CrossPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string GPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string ReportPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string TConstraintSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>string X string X stvector -> bool</text--->"
  "<text>_ stconstraint( string, string, vec(_))</text--->"
  "<text>The operator is used only within the stpattern and reportpattern "
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

const string CreateIntSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(.)) X symbol -> elemset</text--->"
  "<text>_ intstream2set(_)</text--->"
  "<text>The operator collects the values of a stream int attribute into "
  "a set.</text--->"
  "<text>query ten feed intstream2set[no] consume</text--->"
  ") )";

const string EmptyMSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> -> mset</text--->"
  "<text>_ emptymset()</text--->"
  "<text>The operator creates an empty mset (i.e. an mset with zero units)."
  "</text--->"
  "<text>query emptymset()"
  "</text--->"
  ") )";

const string MBool2MSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mbool x int -> mset</text--->"
  "<text>_ mbool2mset(_)</text--->"
  "<text>The operator creates an mset representation for the given mbool. "
  "For every true unit in the mbool argument, a corresponding uset is added "
  "to the result. The set within the uset will have one element, the int "
  "argument. The operator is used within the GPattern operator."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) nocomponents"
  "</text--->"
  ") )";

const string UnionMSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset x mset -> mset</text--->"
  "<text>_ union _</text--->"
  "<text>The operator is a lifted union for msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) union emptymset() nocomponents"
  "</text--->"
  ") )";

const string IntersectsIntSetSpec = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> intset x intset -> bool</text--->"
  "<text>_ intersects _</text--->"
  "<text>The predicate yields true of the intersection of the two sets is "
  "non-empty.</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) union emptymset() nocomponents"
  "</text--->"
  ") )";

const string Union2MSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset x mset -> mset</text--->"
  "<text>_ union2 _</text--->"
  "<text>The operator is a lifted union for msets. Unlike union, this operator "
  "treats undefined intervals as empty msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) union emptymset() nocomponents"
  "</text--->"
  ") )";

const string CardinalityMSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset -> mint</text--->"
  "<text>_ cardinality</text--->"
  "<text>The operator is the lifted count/cardinality for the msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) cardinality max"
  "</text--->"
  ") )";

const string MSet2MPointsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset -> mint</text--->"
  "<text>_ cardinality</text--->"
  "<text>The operator is the lifted count/cardinality for the msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) cardinality max"
  "</text--->"
  ") )";

const string ConvexHullSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset -> mint</text--->"
  "<text>_ cardinality</text--->"
  "<text>The operator is the lifted count/cardinality for the msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) cardinality max"
  "</text--->"
  ") )";


const string MSet2MRegionSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> stream(tuple(int mpoint)) x mset x duration -> movingregion"
  "</text--->"
  "<text>Trains feed addcounter[Cnt, 1] project[Cnt, Trip] Flocks feed "
  "mflocks2mregions[create_duration(0, 10000)]</text--->"
  "<text>Creates mving region representation for the mflocks. The resulting "
  "mregions are the interpolation of the convex hull regions taken at time "
  "intervals of duration at most.</text--->"
  "<text>query Trains feed addcounter[Cnt, 1] project[Cnt, Trip] Flocks feed "
  "mflocks2mregions[create_duration(0, 10000)] consume</text--->"
  ") )";


const string  TheUnitSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "("
  "<text>intset x instant x instant x bool x bool --> uset\n</text--->"
  "<text>the_unit( set, tstart, tend, cl, cr )\n </text--->"
  "<text>Creates a unit value from the argument list. The instants/ipoints"
  "/(point/instant)-pairs will be sorted automatically.</text--->"
  "<text>query the_unit(intstream(1,5) collectintset, create_instant(10,10), "
  "create_instant(100, 100), FALSE, TRUE)</text--->) )";

const string TheMValueSpec  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(stream uset) -> mset\n</text--->"
"<text>_ the_mvalue</text--->"
"<text>Create a moving object from a (not necessarily sorted) "
"object stream containing units. "
"No two unit timeintervals may overlap. Undefined units are "
"allowed and will be ignored. A stream with less than 1 defined "
"unit will result in an 'empty' moving object, not in an 'undef'.</text--->"
"<text>query units(zug5) the_mvalue</text---> ) )";

const string NoComponentsSpec =
  "( ( \"Signature\" \"Syntax\"\"Meaning\" \"Example\" ) "
  "( <text>mset -> int</text--->"
  "<text>no_components ( _ )</text--->"
  "<text>Number of units inside a moving object value. "
  "UNDEFINED argument yields an UNDEFINED result.</text--->"
  "<text>no_components ( mset1 )</text--->"
  ") )";

const string GBoidsSpec =
  "( ( \"Signature\" \"Syntax\"\"Meaning\" \"Example\" ) "
  "( <text>(vector int) x (vector real) x instant x duration"
  " -> stream(tuple((BoidID int)(T instant)(X real)(Y real)))</text--->"
  "<text>generateboids(group sizes, number of freely moving boids, "
  "obstacles, the rect defining the world, simulation start time, "
  "simulation period)</text--->"
  "<text>The operator uses the source code provided by Christopher John Kline "
  "for creating boids. Boids are flying birds. Originally the code generates "
  "3D coordinates. The operator, however, yields the X, Y only. You can create "
  "several groups of boids, where every group try to flock together. The "
  "first argument is a vector that specifies the sizes of these groups. The "
  "first entry in this vector is mandatory, and it specifies the number of "
  "freely moving boids (i.e., boids that move in the background without "
  "flocking). The second argument is a vector that specifies the obstacles "
  "that boids are required to avoid. Obstacles are described as circles, "
  "where every circle is described as x-coord of the center, y-coord of the "
  "center, raduis in meters. The number of reals within this vector must be "
  "a multiple of 3. The first three reals are mandatory and they describe the "
  "world. Boids will respect this world while flying, and should never cross "
  "its boundaries. The third argument is used as the initial time instant in "
  "the generated samples. There is one sample per boids per 5 seconds. The "
  "last argument specifies the simulation duration. The operator return a "
  "stream of (BoidID, T, X, Y) tuples, each of which is representing a single "
  "boid observation. This can be used in combination with other SECONDO "
  "operators to generate the boid trajectories.</text--->"
  "<text>query generateboids("
  "create_vector(40, 15, 15), "
  "create_vector(1000.0,-3000.0, 20000.0), "
  "now(), "
  "create_duration(0, 1000000)) "
  "count</text--->) )";

struct collectIntSetInfo : OperatorInfo {

  collectIntSetInfo()
  {
    name      = "collectintset";
    signature = "stream(int) -> intset";
    syntax    = "_ collectintset";
    meaning   = "Collects the stream if integers into a new intset";
  }

};

Operator row (
    "row",    //name
    RowColSpec,     //specification
    RowColVM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    RowColTM        //type mapping
);

Operator col (
    "col",    //name
    RowColSpec,     //specification
    RowColVM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    RowColTM        //type mapping
);

Operator crosspattern (
    "crosspattern",    //name
    CrossPatternSpec,     //specification
    CrossPatternVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CrossPatternTM        //type mapping
);

Operator gpattern (
    "gpattern",    //name
    GPatternSpec,     //specification
    GPatternVM,       //value mapping
    Operator::SimpleSelect,
    GPatternTM        //type mapping
);

Operator reportpattern (
    "reportpattern",    //name
    ReportPatternSpec,     //specification
    ReportPatternVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    ReportPatternTM        //type mapping
);

Operator tconstraint (
    "tconstraint",    //name
    TConstraintSpec,     //specification
    TConstraintVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    TConstraintTM        //type mapping
);

Operator emptymset (
    "emptymset",    //name
    EmptyMSetSpec,     //specification
    EmptyMSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    EmptyMSetTM        //type mapping
);

Operator mbool2mset (
    "mbool2mset",    //name
    MBool2MSetSpec,     //specification
    MBool2MSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    MBool2MSetTM        //type mapping
);

Operator unionmset (
    "union",    //name
    UnionMSetSpec,     //specification
    UnionMSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    UnionMSetTM        //type mapping
);

Operator intersects (
    "intersects",    //name
    IntersectsIntSetSpec,     //specification
    IntersectsVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    IntersectsTM        //type mapping
);

Operator union2mset (
    "union2",    //name
    Union2MSetSpec,     //specification
    Union2MSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    Union2MSetTM        //type mapping
);

Operator cardinalitymset (
    "cardinality",    //name
    CardinalityMSetSpec,     //specification
    CardinalityMSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CardinalityMSetTM        //type mapping
);

Operator intstream2set (
    "intstream2set",    //name
    CreateIntSetSpec,     //specification
    CreateAlfaSetVM<CcInt>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CreateAlfaSetTM<0>        //type mapping
);


Operator mset2mregion (
    "mset2mregion",               // name
    MSet2MRegionSpec,             // specification
    MSet2MRegionVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    MSet2MRegionTM          // type mapping
);

Operator mset2mpoints (
    "mset2mpoints",               // name
    MSet2MPointsSpec,             // specification
    MSet2MPointsVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    MSet2MPointsTM          // type mapping
);

Operator convexhull (
    "convexhull2",               // name
    ConvexHullSpec,             // specification
    ConvexHullVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    ConvexHullTM          // type mapping
);

Operator theunit( 
    "the_unit",
    TheUnitSpec,
    TheUnitVM,
    Operator::SimpleSelect,
    TheUnitTM);

Operator themvalue( 
    "the_mvalue",
    TheMValueSpec,
    TheMValueVM,
    Operator::SimpleSelect,
    TheMValueTM);

Operator nocomponents( "no_components",
    NoComponentsSpec,
    NoComponentsVM,
    Operator::SimpleSelect,
    NoComponentsTM);

Operator generateboids( "generateboids",
    GBoidsSpec,
    GBoidsVM,
    Operator::SimpleSelect,
    GBoidsTM);


TypeConstructor intSetTC(
        "intset",       //name
        IntSet::Property, //property function describing signature
        IntSet::Out,
        IntSet::In,     //Out and In functions
        0, 0,          //SaveToList and RestoreFromList functions
        IntSet::Create,
        IntSet::Delete, //object creation and deletion
        0, 0,
        IntSet::Close,
        IntSet::Clone,  //object close and clone
        IntSet::Cast,   //cast function
        IntSet::SizeOfObj, //sizeof function
        IntSet::KindCheck );  

TypeConstructor usetTC(
        "uset",        //name
        mset::USet::USetProperty,    //property function describing signature
        mset::USet::OutUSet,
        mset::USet::InUSet,    //Out and In functions
        0,     0,  //SaveToList and RestoreFromList functions
        mset::USet::CreateUSet,
        mset::USet::DeleteUSet,          //object creation and deletion
        OpenAttribute<USet>,
        SaveAttribute<USet>,  // object open and save
        mset::USet::CloseUSet,
        mset::USet::CloneUSet,           //object close and clone
        mset::USet::CastUSet,            //cast function
        mset::USet::SizeOfUSet,          //sizeof function
        mset::USet::CheckUSet );  

TypeConstructor msetTC(
        "mset", //name
        MSet::Property,  //property function describing signature
        MSet::OutMSet,
        MSet::InMSet,
       //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        MSet::CreateMSet,
        MSet::DeleteMSet,        //object creation and deletion
        0,
        0,          // object open and save
        MSet::CloseMSet,
        MSet::CloneMSet,     //object close and clone
        MSet::CastMSet,     //cast function
        MSet::SizeOfMSet,    //sizeof function
        MSet::KindCheck );   


class GPatternAlgebra : public Algebra
{
public:
  GPatternAlgebra() : Algebra()
  {

    AddTypeConstructor( &intSetTC );
    AddTypeConstructor( &usetTC );
    AddTypeConstructor( &msetTC );

    intSetTC.AssociateKind( Kind::DATA() );
    usetTC.AssociateKind(Kind::TEMPORAL() );
    usetTC.AssociateKind( Kind::DATA() );
    msetTC.AssociateKind( Kind::TEMPORAL() );
    msetTC.AssociateKind( Kind::DATA() );
/*
The spattern and reportpattern operators are registered as lazy variables.

*/
    reportpattern.SetRequestsArguments();
    gpattern.SetRequestsArguments();
    crosspattern.SetRequestsArguments();
    generateboids.SetUsesArgsInTypeMapping();
    
    AddOperator(&GPattern::tconstraint);
    AddOperator(&GPattern::reportpattern);
    AddOperator(&GPattern::emptymset);
    AddOperator(&GPattern::gpattern);
    AddOperator(&GPattern::crosspattern);
    AddOperator(&GPattern::row);
    AddOperator(&GPattern::col);
    AddOperator(&intstream2set);
    AddOperator(&emptymset);
    AddOperator(&mbool2mset);
    AddOperator(&unionmset);
    AddOperator(&union2mset);
    AddOperator(&cardinalitymset);
    AddOperator(&mset2mregion);
    AddOperator(&mset2mpoints);
    AddOperator(&convexhull);
    AddOperator(&theunit);
    AddOperator(&themvalue );
    AddOperator(collectIntSetInfo(), CollectIntSetValueMap,
                CollectIntSetTypeMap);
    AddOperator(&intersects);
    AddOperator(&nocomponents);
    AddOperator(&generateboids);
  }
  ~GPatternAlgebra() {};
};

};

/*
5 Initialization

*/



extern "C"
Algebra*
InitializeGPatternAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new GPattern::GPatternAlgebra;
    }
