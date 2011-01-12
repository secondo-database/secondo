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

[1] Source File of a standard graph implementation

Dec, 2010 Mahmoud Sakr

[TOC]

1 Overview

2 Defines and includes

*/

#include<map>
#include<vector>
#include<set>
#include<algorithm>
#include"LightWeightGraph.h"

using namespace std;
namespace GPattern {

ostream& NewComponent::Print( ostream &os )
{
  os<<endl<<"This NewComponent consists of components:";
  for(vector<int>::iterator it= affectedComponents.begin(); it!= 
    affectedComponents.end(); ++it)
    os<< *it<<", ";
  os<<endl<<"  It has additionally the nodes :";
  for(set<int>::iterator it= newNodes.begin(); it!= newNodes.end(); ++it)
    os<<*it<< ", ";
  return os;
}

bool NewComponent::AffectsComponent(int label)
{
  return(
      find(this->affectedComponents.begin(), this->affectedComponents.end(), 
        label ) != this->affectedComponents.end());
}
void NewComponent::Union(NewComponent& arg)
{
  for(unsigned int i= 0; i< arg.affectedComponents.size(); ++i)
  {
    if(
        find(this->affectedComponents.begin(), this->affectedComponents.end(), 
              arg.affectedComponents[i]) == this->affectedComponents.end() )
      this->affectedComponents.push_back(arg.affectedComponents[i]);
  }
  
  this->newNodes.insert( 
      arg.newNodes.begin(), arg.newNodes.end());
}


void Component::SetMessage(ComponentMessage msg)
{
  this->message= msg;
}
bool Component::UpdateMessage(ComponentMessage newMsg)
{
  bool debugme= false;
  if(newMsg == this->message) return true;
  string msgs[]= {"NoMessage", "NotChanged", "NewlyAddedEdges", 
      "NewlyAddedNodes", "RemovedEdges", "RemovedNodes", "AddRemoveMix", 
      "NewlyAdded", "RemoveNow", "SplitFromExtistingComponent", 
      "MergedFromExistingComponents", "ReDistribute"};
  if(debugme)
    cerr<< "\ncur message: "<< msgs[this->message] <<endl<< 
      "argument message"<< msgs[newMsg];
  switch (this->message)
  {
  case Component::NoMessage:
  case Component::NotChanged:
  {
    this->message= newMsg;
    return true;
  }break;
  case Component::NewlyAddedEdges:
  {
    if(newMsg== NoMessage || newMsg== NotChanged || newMsg== NewlyAddedEdges)
      return true;
    if(newMsg== NewlyAddedNodes)
    {
      this->message= NewlyAddedNodes;
      return true;
    }
    if(newMsg== RemovedEdges || newMsg== RemovedNodes || newMsg== AddRemoveMix)
    {
      this->message=AddRemoveMix;
      return true;
    }
    if(newMsg == NewlyAdded)
      return false;
    if(newMsg == SplitFromExtistingComponent || newMsg== RemoveNow || 
        newMsg== MergedFromExistingComponents)
    {
      this->message= newMsg;
      return true;
    }
  }break;
  case Component::NewlyAddedNodes:
  {
    if(newMsg== NoMessage || newMsg== NotChanged || newMsg== NewlyAddedEdges||
        newMsg== NewlyAddedNodes)
    {
      this->message= NewlyAddedNodes;
      return true;
    }
    if(newMsg== RemovedEdges || newMsg== RemovedNodes || newMsg== AddRemoveMix)
    {
      this->message=AddRemoveMix;
      return true;
    }
    if(newMsg == NewlyAdded)
      return false;
    if(newMsg == SplitFromExtistingComponent || newMsg== RemoveNow ||
        newMsg== MergedFromExistingComponents)
    {
      this->message= newMsg;
      return true;
    }
  }break;
  case Component::RemovedEdges:
  {
    if(newMsg== NoMessage || newMsg== NotChanged || newMsg== RemovedEdges)
      return true;
    if(newMsg== RemovedNodes)
    {
      this->message= RemovedNodes;
      return true;
    }
    if(newMsg== NewlyAddedEdges || newMsg== NewlyAddedNodes || 
        newMsg== AddRemoveMix)
    {
      this->message=AddRemoveMix;
      return true;
    }
    if(newMsg == NewlyAdded)
      return false;
    if(newMsg == SplitFromExtistingComponent || newMsg== RemoveNow || 
        newMsg== MergedFromExistingComponents)
    {
      this->message= newMsg;
      return true;
    }
  }break;
  case Component::RemovedNodes:
  {
    if(newMsg== NoMessage || newMsg== NotChanged || newMsg== RemovedEdges||
        newMsg==RemovedNodes)
      return true;
    if(newMsg== NewlyAddedEdges || newMsg== NewlyAddedNodes || 
        newMsg== AddRemoveMix)
    {
      this->message=AddRemoveMix;
      return true;
    }
    if(newMsg == NewlyAdded)
      return false;
    if(newMsg == SplitFromExtistingComponent || newMsg== RemoveNow ||
        newMsg== MergedFromExistingComponents)
    {
      this->message= newMsg;
      return true;
    }
  }break;
  case Component::AddRemoveMix:
  {
    if(newMsg== NoMessage || newMsg== NotChanged || newMsg== RemovedEdges||
        newMsg==RemovedNodes || newMsg== NewlyAddedEdges || 
        newMsg== NewlyAddedNodes || newMsg== AddRemoveMix)
    {
      this->message=AddRemoveMix;
      return true;
    }
    if(newMsg == NewlyAdded)
      return false;
    if(newMsg == SplitFromExtistingComponent || newMsg== RemoveNow ||
        newMsg== MergedFromExistingComponents)
    {
      this->message= newMsg;
      return true;
    }
  }break;
  case Component::NewlyAdded:
  case Component::SplitFromExtistingComponent:
  case Component::RemoveNow:
    return false; break;
  case Component::MergedFromExistingComponents:
  {
    if(newMsg== NoMessage || newMsg== NotChanged || newMsg== RemovedEdges||
        newMsg==RemovedNodes || newMsg== AddRemoveMix)
    {
      this->message= MergedFromExistingComponents;
      return true;
    }
    if(newMsg== RemoveNow)
    {
      this->message= RemoveNow;
      return true;
    }
    if(newMsg== SplitFromExtistingComponent)
    {
      this->message= ReDistribute; 
      return true;
    }
  }break;
  default:
    return false;
  }
  return false;
}

void Component::Cluster(LWGraph* g, list<set<int> >& splitComponents)
{
  splitComponents.clear();
  
  list<int> frontier;
  set<int> already_added;
  int curNode;
  list< set< pair<int, int> > >::iterator curNodeNeighborsIt;
  set< pair<int, int> >* curNodeNeighbors;
  for(set<int>::iterator it= this->nodes.begin(); it!= this->nodes.end(); ++it)
  {
    curNode= *it;
    if( already_added.find(curNode) != already_added.end()) continue;
    frontier.push_back(curNode);
    set<int> comp;
    comp.insert(curNode);
    while(!frontier.empty())
    {
      curNode= frontier.back();
      frontier.pop_back();
      if( already_added.find(curNode) != already_added.end()) continue;
      already_added.insert(curNode);
      curNodeNeighborsIt= g->node_index[curNode];
      curNodeNeighbors= &(*curNodeNeighborsIt);

      for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); nIt!=
        curNodeNeighbors->end(); ++nIt)
      {
        frontier.push_front( (*nIt).first);
        comp.insert((*nIt).first);
      }
    }
    splitComponents.push_back(comp);
  }
}

void Component::SynchronizeNodes(LWGraph* g)
{
  map<int, int>::iterator nodeComp;
  set<int>::iterator nodeIt= nodes.begin();
  while( nodeIt!= nodes.end())
  {
    nodeComp= g->node_component.find(*nodeIt); 
    if(nodeComp == g->node_component.end())
      nodes.erase(nodeIt++);
    else
      ++nodeIt;
    //may raise 
  }
}
bool Component::IsConnected(LWGraph* g, int node1, int node2)
{  
  list<int> frontier;
  set<int> already_added;
  list< set< pair<int, int> > >::iterator curNodeNeighborsIt;
  set< pair<int, int> >* curNodeNeighbors;
  map<int,list< set< pair<int, int> > >::iterator>::iterator nodeIndexIt;
  int curNode= node1;
  frontier.push_back(curNode);
  while(!frontier.empty())
  {
    curNode= frontier.back();
    frontier.pop_back();
    if( already_added.find(curNode) != already_added.end()) continue;
    already_added.insert(curNode);
    nodeIndexIt= g->node_index.find(curNode);
    if(nodeIndexIt == g->node_index.end()) return false;
    curNodeNeighborsIt= (*nodeIndexIt).second;
    curNodeNeighbors= &(*curNodeNeighborsIt);
    for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); nIt!=
      curNodeNeighbors->end(); ++nIt)
    {
      if((*nIt).first == node2)
        return true;
      frontier.push_front( (*nIt).first);
    }
  }
  return false;
}

void Component::GetEdges(LWGraph* g, set<int>& compEdges)
{
  compEdges.clear();
  int curNode;
  list< set< pair<int, int> > >::iterator curNodeNeighborsIt;
  set< pair<int, int> >* curNodeNeighbors;
  for(set<int>::iterator it= this->nodes.begin(); it!= this->nodes.end(); ++it)
  {
    curNode= *it;
    curNodeNeighborsIt= g->node_index[curNode];
    curNodeNeighbors= &(*curNodeNeighborsIt);

    for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); nIt!=
      curNodeNeighbors->end(); ++nIt)
      compEdges.insert( (*nIt).second);
  }
}

void Component::Union(Component* arg)
{
  assert(this->UpdateMessage(arg->message));
  
  this->nodes.insert(arg->nodes.begin(), arg->nodes.end());
  this->resStreams.insert(this->resStreams.end(), arg->resStreams.begin(), 
      arg->resStreams.end());
  
}

ostream& Component::Print( ostream &os )
{
  os<<endl<<"Printing component "<<label<<": ";
  for(set<int>::iterator it= nodes.begin(); it!= nodes.end(); ++it)
    os<<*it<< ", ";
  os<<"\n  Component is associated with "<<resStreams.size()<<" resStreams";
  return os;
}


void Component::ExtendResStreamsTillNow(double endtime, bool rc)
{
  if(ExtendedTillLastChange) return;
  mset::CompressedUSetRef theUSet;
  for(list< list<mset::CompressedMSet*>::iterator >::iterator res= 
    this->resStreams.begin(); res!= this->resStreams.end(); ++res)
  {
    mset::CompressedMSet* theMSet= **res;
    theMSet->units.Get(theMSet->GetNoComponents()-1, theUSet);
    theUSet.endtime= endtime;
    theUSet.rc= rc;
    theMSet->units.Put(theMSet->GetNoComponents()-1, theUSet);
  }
}

bool Component::Intersects(set<int>* arg)
{
  return SetIntersects(&this->nodes, arg);
}

void Component::Reset()
{
  this->SetMessage(Component::NotChanged);
  this->addedEdges.clear();
  this->removedEdges.clear();
}

int LWGraph::clear()
{
  node_component.clear();
  node_index.clear(); 
  //(node label, index): stores the index of a node n in the "node" vector
  neighbors.clear();
  //the neighbors of a node (i.e. nodes that are directly connected 
  //with an edge). It is a parallel arary to "node"
  return 0;
}

int LWGraph::copy_from(LWGraph* arg)
{
  assert(arg->neighbors.size()== arg->node_index.size());
  this->clear();
  this->neighbors.assign(arg->neighbors.begin(), arg->neighbors.end());
  list<set< pair<int, int> > >::iterator neighbor= this->neighbors.begin();
  for(map<int, list< set< pair<int, int> > >::iterator>::iterator argIt= 
    arg->node_index.begin(); argIt != arg->node_index.end(); ++argIt)
  {
    this->node_index.insert(make_pair((*argIt).first, neighbor));
    ++neighbor;
  }
  return 0;
}
  
int LWGraph::insert_edge_directed(pair<int, int>* _edgeNodes, int& _edge)
{
  int _first= _edgeNodes->first, _second= _edgeNodes->second;
  return insert_edge_directed(_first, _second, _edge);
}

int LWGraph::insert_edge_directed(int& _first, int& _second, int& _edge)
{
  map<int, list< set< pair<int, int> > >::iterator>::iterator _pos= 
    node_index.find(_first); 
  if(_pos != node_index.end())
    (*(*_pos).second).insert(make_pair(_second, _edge)); 
  else
  {
    set< pair<int, int> > adj; adj.insert(make_pair(_second, _edge));
    neighbors.push_back(adj);
    list< set< pair<int, int> > >::iterator it= --neighbors.end();
    node_index.insert(make_pair(_first, it));
  }
  
  _pos= node_index.find(_second);
  if(_pos == node_index.end())
  {
    set< pair<int, int> > tmp;
    neighbors.push_back(tmp);
    list< set< pair<int, int> > >::iterator it= --neighbors.end();
    node_index.insert(make_pair(_second, it));
  }
  return 0;
}

int LWGraph::remove_node_if_isolated(int _node)
{
  map<int, list< set< pair<int, int> > >::iterator>::iterator node_pos=
    node_index.find(_node);
  assertIf(node_pos != node_index.end());
  list< set< pair<int, int> > >::iterator node_neighbors_it= (*node_pos).second;
  
  //check whether the node has outbound edges
  if(! (*node_neighbors_it).empty()) return 0;
  
  //check whether the node has inbound edges 
  for(list< set< pair<int, int> > >::iterator it= 
    this->neighbors.begin(); it!= this->neighbors.end(); ++it)
  {
    for(set<pair<int,int> >::iterator it2= (*it).begin(); it2!= 
      (*it).end(); ++it2)  
      if((*it2).first == _node) return 0;
  }

  //there are no inbound or outbound edges for the node. We remove 
  //the node from the graph
  neighbors.erase(node_neighbors_it);
  node_index.erase(node_pos);
  node_component.erase(_node);
  
  return 0;
}

int LWGraph::remove_edge_directed(pair<int, int>& _edgeNodes, int& _edge)
{
  int _first= _edgeNodes.first, _second= _edgeNodes.second;
  return remove_edge_directed(_first, _second, _edge);
}

int LWGraph::remove_edge_directed(int& _first, int& _second, int& _edge)
{
  bool debugme= false;
  if(debugme)
  {
    this->print(cerr);
    cerr<< endl<< _first << " -> ("<< _second << ", "<< _edge<< ")"<< endl;
  }
  map<int, list< set< pair<int, int> > >::iterator>::iterator node_pos=
    node_index.find(_first);
  assertIf(node_pos != node_index.end());
  
  list< set< pair<int, int> > >::iterator node_neighbors_it= (*node_pos).second;
  unsigned int erased= (*node_neighbors_it).erase(make_pair(_second, _edge));
  assertIf(erased==1);
  
  remove_node_if_isolated(_first);
  remove_node_if_isolated(_second);
  return 0;
}

int LWGraph::nodes_count()
{
  assertIf(node_index.size() == neighbors.size());
  return neighbors.size();
}

int LWGraph::get_nodes(set<int>& res)
{
  res.clear();
  for(map<int, list< set< pair<int, int> > >::iterator>::iterator it= 
    node_index.begin(); it!= node_index.end(); ++it)
    res.insert((*it).first);
  assertIf(res.size() == node_index.size());
  return 0;
}

vector< LWGraph* >* LWGraph::cluster()
{
  vector< LWGraph* >* ress= new vector< LWGraph* >();
  list<int> frontier;
  set<int> already_added;
  int curNode;
  list< set< pair<int, int> > >::iterator curNodeNeighborsIt, 
    frontierNodeNeighborsIt;
  set< pair<int, int> >* curNodeNeighbors, fromtierNodeNeighbors;
  for(map<int, list< set< pair<int, int> > >::iterator>::iterator it= 
    node_index.begin(); it!= node_index.end(); ++it)
  {
    curNode= (*it).first;
    if( already_added.find(curNode) != already_added.end()) continue;
    frontier.push_back(curNode);
    //already_added.insert(curNode);
    LWGraph* res= new LWGraph();
    
    while(!frontier.empty())
    {
      curNode= frontier.back();
      frontier.pop_back();
      if( already_added.find(curNode) != already_added.end()) continue;
      already_added.insert(curNode);
      curNodeNeighborsIt= node_index[curNode];
      curNodeNeighbors= &(*curNodeNeighborsIt);
      
      res->neighbors.push_back(*curNodeNeighbors);
      list< set< pair<int, int> > >::iterator resNeighborsIt= 
        --res->neighbors.end();
      res->node_index.insert(make_pair(curNode, resNeighborsIt));
      for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); nIt!=
        curNodeNeighbors->end(); ++nIt)
        frontier.push_front( (*nIt).first);  
    }
    ress->push_back(res);
  }
  return ress;  
}

vector< LWGraph* >* LWGraph::find_components_of(set<int>& _nodes)
{
  vector< LWGraph* >* ress= new vector< LWGraph* >();
  list<int> frontier;
  set<int> already_added;
  int curNode;
  list< set< pair<int, int> > >::iterator curNodeNeighborsIt, 
    frontierNodeNeighborsIt;
  set< pair<int, int> >* curNodeNeighbors, fromtierNodeNeighbors;
  for(set<int>::iterator it= _nodes.begin(); it!= _nodes.end(); ++it)
  {
    curNode= *it;
    if( already_added.find(curNode) != already_added.end()) continue;
    frontier.push_back(curNode);
    //already_added.insert(curNode);
    LWGraph* res= new LWGraph();
    
    while(!frontier.empty())
    {
      curNode= frontier.back();
      frontier.pop_back();
      if( already_added.find(curNode) != already_added.end()) continue;
      already_added.insert(curNode);
      curNodeNeighborsIt= node_index[curNode];
      curNodeNeighbors= &(*curNodeNeighborsIt);
      
      res->neighbors.push_back(*curNodeNeighbors);
      list< set< pair<int, int> > >::iterator resNeighborsIt= 
        --res->neighbors.end();
      res->node_index.insert(make_pair(curNode, resNeighborsIt));
      for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); nIt!=
        curNodeNeighbors->end(); ++nIt)
        frontier.push_front( (*nIt).first);  
    }
    ress->push_back(res);
  }
  return ress;
}

ostream& LWGraph::print( ostream &os )
{
  os<<endl;
  for(map<int, list< set< pair<int, int> > >::iterator>::iterator it= 
    node_index.begin(); it!= node_index.end(); ++it)
  {
    os<< (*it).first<<" : ";
    for(set< pair<int, int> >::iterator n= (*(*it).second).begin(); n!= 
      (*(*it).second).end(); ++n)
      os<<"("<<(*n).first<<", "<< (*n).second<< "), ";
    os<<endl;
  }
  os<< "\n node/component pairs:";
  for(map< int, int >::iterator it= 
    node_component.begin(); it!= node_component.end(); ++it)
    os<< "("<< (*it).first<<", " <<(*it).second<<")," ;
  return os;
}

void InsertEdgesUndirected(
    LWGraph* g, set<int>& edges, vector<pair<int,int> >& edge2nodes)
{
  int edge;
  pair<int, int> *edgeNodes;
  for(set<int>::iterator it= edges.begin(); it!= edges.end(); ++it)
  {
    edge= *it;
    edgeNodes= &edge2nodes[edge];
    g->insert_edge_directed(edgeNodes, edge);
    g->insert_edge_directed(edgeNodes->second, edgeNodes->first, edge);
  }
}

list<Component*>::iterator 
GetComponentIt(list<Component*>* components, int label)
{
  list<Component*>::iterator it= components->begin();
  while((*it)->label != label && it != components->end()) ++it;
  return it;
}

void FindComponentsOf(LWGraph* g, list<Component*>* components, 
    set<int>& newEdges, vector<pair<int,int> >& edge2nodes, 
    vector<NewComponent>& newComponents)
{
  bool debugme= false;
  if(debugme)
    g->print(cerr);
  newComponents.clear();

  int edge;
  pair<int, int> *edgeNodes;
  set<int> nodes;
  set<int> newNodes, curNewNodes;
  set<int> affectedComponents, curAffectedComponents;
  NewComponent* newComp;
  
  for(set<int>::iterator it= newEdges.begin(); it!= newEdges.end(); ++it)
  {
    edge= *it;
    edgeNodes= &edge2nodes[edge];
    if(newNodes.find(edgeNodes->first) != newNodes.end()||
        newNodes.find(edgeNodes->second) != newNodes.end()) continue;
    curAffectedComponents.clear();  curNewNodes.clear();
    ExpandInGraph(g, edgeNodes, curAffectedComponents, curNewNodes);
    newNodes.insert(curNewNodes.begin(), curNewNodes.end());
    affectedComponents.insert(
        curAffectedComponents.begin(), curAffectedComponents.end());
    newComponents.resize(newComponents.size() +1);
    newComp= &newComponents.back();
    newComp->affectedComponents.assign(
        curAffectedComponents.begin(), curAffectedComponents.end());
    newComp->newNodes= curNewNodes;
    if(debugme)
      newComp->Print(cerr);
  }
  MergeNewComponents(newComponents, affectedComponents);
}

void ExpandInGraph(LWGraph*  graph, pair<int, int>* edgeNodes, 
    set<int>& affectedComponents, set<int>& newNodes)
{
  set<int> frontier;
  set<int> already_added;
  frontier.insert(edgeNodes->first);
  frontier.insert(edgeNodes->second);
  int curNode;
  map<int,list< set< pair<int, int> > >::iterator>::iterator nodeIndexIt;
  list< set< pair<int, int> > >::iterator curNodeNeighborsIt;
  set< pair<int, int> >* curNodeNeighbors;
  while (!frontier.empty())
  {
    curNode = *(frontier.begin());
    frontier.erase(frontier.begin());
    if(already_added.find(curNode)!= already_added.end()) continue;
    already_added.insert(curNode);
    map<int, int>::iterator nodeComp= graph->node_component.find(curNode);
    if(nodeComp != graph->node_component.end())
      affectedComponents.insert((*nodeComp).second);
    else
    {
      newNodes.insert(curNode);
      nodeIndexIt= graph->node_index.find(curNode);
      assertIf(nodeIndexIt != graph->node_index.end());
      curNodeNeighborsIt= (*nodeIndexIt).second;
      curNodeNeighbors= &(*curNodeNeighborsIt);
      for(set<pair<int,int> >::iterator nIt= curNodeNeighbors->begin(); nIt!=
        curNodeNeighbors->end(); ++nIt)
        frontier.insert( (*nIt).first);
    }
  }
}

void MergeNewComponents(vector<NewComponent>& newComponents, 
    set<int>& affectedComponents)
{
  bool debugme= false;
  if(debugme)
  {
    cerr<<"\n affectedComponents: ";
    PrintSet(affectedComponents, cerr);
    cerr<<"\n newComponents: ";
    for(vector<NewComponent>::iterator it= newComponents.begin(); it!= 
      newComponents.end(); ++it)
      (*it).Print(cerr);
  }
  if(newComponents.size() <= 1) return;
  vector<int> intersectingNewComponents;
  for(set<int>::iterator it= affectedComponents.begin(); it!= 
    affectedComponents.end(); ++it)
  {
    intersectingNewComponents.clear();
    for(unsigned int i=0; i<newComponents.size(); ++i) 
      if(newComponents[i].AffectsComponent(*it)) 
        intersectingNewComponents.push_back(i);
    
    assertIf(!intersectingNewComponents.empty());
    if(intersectingNewComponents.size() == 1) continue;
    NewComponent bigComp= newComponents[intersectingNewComponents.back()];
    for(int j= intersectingNewComponents.size()-2;  j>=0; --j)
    {
      bigComp.Union(newComponents[intersectingNewComponents[j]]);
      newComponents.erase(newComponents.begin()+ intersectingNewComponents[j]);
    }
  }
}

void RemoveEdgesUndirected(
    LWGraph* g, set<int>& edges, vector<pair<int,int> >& edge2nodes)
{
  int edge;
  pair<int, int> *edgeNodes;
  for(set<int>::iterator it= edges.begin(); it!= edges.end(); ++it)
  {
    edge= *it;
    edgeNodes= &edge2nodes[edge];
    g->remove_edge_directed(*edgeNodes, edge);
    g->remove_edge_directed(edgeNodes->second, edgeNodes->first, edge);
  }
}

void RemoveEdgeUndirected(LWGraph* g, int edge, pair<int,int>* edgeNodes)
{
  g->remove_edge_directed(*edgeNodes, edge);
  g->remove_edge_directed(edgeNodes->second, edgeNodes->first, edge);  
}

void SetGraphNodesComponent(LWGraph* g, set<int>& nodes, int label)
{
  for(set<int>::iterator nodeIt= nodes.begin(); nodeIt!= nodes.end(); ++nodeIt)
  {
    assertIf(g->node_component.find(*nodeIt) == g->node_component.end());
    g->node_component.insert(make_pair(*nodeIt, label));
  }
}

void RemoveGraphNodesComponent(LWGraph* g, set<int>& nodes)
{
  map<int, int>::iterator pos;

  for(set<int>::iterator nodeIt= nodes.begin(); nodeIt!= 
    nodes.end(); ++nodeIt)
  {
    pos= g->node_component.find(*nodeIt);
    assertIf(pos != g->node_component.end());
    g->node_component.erase(pos);
  }
}

void UpdateGraphNodesComponent(LWGraph* g, set<int>& nodes, int label)
{
  map<int, int>::iterator pos;
  for(set<int>::iterator nodeIt= nodes.begin(); nodeIt!= 
      nodes.end(); ++nodeIt)
  {
    pos= g->node_component.find(*nodeIt);
    assertIf(pos != g->node_component.end());
    (*pos).second= label;
  }
}

int FindEdgeComponent(LWGraph* g, pair<int, int>* _edge)
{
  map<int, int>::iterator pos1= g->node_component.find(_edge->first), 
  pos2= g->node_component.find(_edge->second);;
  if(pos1 == g->node_component.end() && pos2 == g->node_component.end()) 
    return -1;
    
  int comp1= (*pos1).second;
  int comp2= (*pos2).second;
  assertIf(comp1 == comp2);
  return comp1;
}

ostream& PrintSet( set<int>& arg, ostream &os )
{
  for(set<int>::iterator it= arg.begin(); it!= arg.end(); ++it)
    os<< *it<<", ";
  return os;
}

ostream& PrintVector( vector<int>& arg, ostream &os )
{
  for(vector<int>::iterator it= arg.begin(); it!= arg.end(); ++it)
    os<< *it<<", ";
  return os;
}

bool HasOneComponent(set<int> edges, vector<pair<int,int> >& edge2nodes)
{
  bool debugme=true;
  bool res=false;
  LWGraph* graph= new LWGraph();
  InsertEdgesUndirected(graph, edges, edge2nodes);
  vector< LWGraph* >* comps= graph->cluster();
  if(comps->size()==1) res= true;
  else
  {
    if(debugme)
    {
      cerr<<"\nThe graph is:\n";
      graph->print(cerr);
      for(vector< LWGraph* >::iterator it= comps->begin(); it!= comps->end(); 
        ++it)
      {
        cerr<<"\nFound Component:\n";
        (*it)->print(cerr);
      }
    }
  }
    
  
  for(vector< LWGraph* >::iterator it= comps->begin(); it!= comps->end(); ++it)
    delete *it;
  delete comps;
  return res;
}

bool IsOneComponent(mset::CompressedMSet* _mset, int n, 
    vector<pair<int,int> >& edge2nodes)
{
  set<int> lastSet;
  mset::CompressedInMemUSet uset;
  mset::CompressedUSetRef unitRef;
  int elem, cnt;
  set<int> toAdd, toRemove;
  int i= 0;
  int size= _mset->units.Size();
  for(; i< size; ++i)
  {
    _mset->units.Get( i, unitRef );
    toAdd.clear(); toRemove.clear();
    for(int j=unitRef.addedstart; j<= unitRef.addedend; ++j)
    {
      _mset->added.Get(j, elem);
      lastSet.insert(elem);
    }
    for(int j=unitRef.removedstart; j<= unitRef.removedend; ++j)
    {
      _mset->removed.Get(j, elem);
      lastSet.erase(elem);
    }
    cnt= lastSet.size();
    assertIf(cnt == unitRef.count);
    if(! HasOneComponent(lastSet, edge2nodes)) 
      return false;
  }
  return true;
}

bool SetIntersects(set<int>* set1, set<int>* set2)
{
  if(set1->empty() || set2->empty()) return false;
  set<int>::iterator first1= set1->begin(),
    first2= set2->begin(),
    last1 = --set1->end(),
    last2 = --set2->end();
    
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

}
