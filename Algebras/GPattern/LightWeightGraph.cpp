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
bool Component::Intersects(Component* arg)
{
  set<int> set1, set2;
  this->graph->get_nodes(set1);
  arg->graph->get_nodes(set2);
  if(set1.empty() || set2.empty()) return false;
  set<int>::iterator first1= set1.begin(),
    first2= set2.begin(),
    last1 = --set1.end(),
    last2 = --set2.end();
    
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
void Component::SetMessage(ComponentMessage msg)
{
  this->message= msg;
}
bool Component::UpdateMessage(ComponentMessage newMsg)
{
  cerr<< "\ncur message: "<< this->message <<endl<< "argument message"<< newMsg;
  this->message= newMsg;
  return true;
}

int Component::Union(Component* argComp)
{
  this->UpdateMessage(argComp->message);
  list< list<mset::CompressedMSet*>::iterator >::iterator pos;
  for(list< list<mset::CompressedMSet*>::iterator >::iterator it= 
    argComp->resStreams.begin(); it!= argComp->resStreams.end(); ++it)
  {
    pos= find(this->resStreams.begin(), this->resStreams.end(), *it);
    if(pos != this->resStreams.end())
      this->resStreams.push_back(*it);
  }
  
  LWGraph* arg= argComp->graph;
  int argNode;
  set< pair<int, int> >* argNodeNeighbors;
  map<int,list< set< pair<int, int> > >::iterator>::iterator nodeNeighborsIt;
  for(map<int,list< set< pair<int, int> > >::iterator>::iterator argNodesIt= 
    arg->node_index.begin(); argNodesIt!= arg->node_index.end(); ++argNodesIt)
  {
    argNode= (*argNodesIt).first;
    argNodeNeighbors= &(*(*argNodesIt).second);
    nodeNeighborsIt= this->graph->node_index.find(argNode);
    if(nodeNeighborsIt != this->graph->node_index.end())
      (*(*nodeNeighborsIt).second).insert(argNodeNeighbors->begin(), 
          argNodeNeighbors->end());
    else
    {
      set< pair<int, int> > neighbors;
      neighbors.insert(argNodeNeighbors->begin(), argNodeNeighbors->end());
      this->graph->neighbors.push_back(neighbors);
      this->graph->node_index.insert(
          make_pair(argNode, --this->graph->neighbors.end()));
    }
  }  
  return 0;
}
void Component::CopyEdgesFrom(Component* comp)
{
  LWGraph* arg= comp->graph;
  int argNode;
  set< pair<int, int> >* argNodeNeighbors;
  map<int,list< set< pair<int, int> > >::iterator>::iterator nodeNeighborsIt;
  for(map<int,list< set< pair<int, int> > >::iterator>::iterator argNodesIt= 
    arg->node_index.begin(); argNodesIt!= arg->node_index.end(); ++argNodesIt)
  {
    argNode= (*argNodesIt).first;
    argNodeNeighbors= &(*(*argNodesIt).second);
    nodeNeighborsIt= this->graph->node_index.find(argNode);
    assert(nodeNeighborsIt != this->graph->node_index.end());
    (*(*nodeNeighborsIt).second).insert(argNodeNeighbors->begin(), 
        argNodeNeighbors->end());
  }
}
void Component::RemoveEdgesFrom(Component* comp)
{
  LWGraph* arg= comp->graph;
  int argNode;
  pair<int, int> neighbor;
  set< pair<int, int> >* argNodeNeighbors;
  map<int,list< set< pair<int, int> > >::iterator>::iterator nodeNeighborsIt;
  for(map<int,list< set< pair<int, int> > >::iterator>::iterator argNodesIt= 
    arg->node_index.begin(); argNodesIt!= arg->node_index.end(); ++argNodesIt)
  {
    argNode= (*argNodesIt).first;
    argNodeNeighbors= &(*(*argNodesIt).second);
    for(set< pair<int, int> >::iterator neighborIt= argNodeNeighbors->begin(); 
      neighborIt!= argNodeNeighbors->end(); ++neighborIt)
    {
      neighbor= *neighborIt;
      this->graph->remove_edge_directed(argNode, neighbor.first, 
          neighbor.second);
    }
  }
}


int LWGraph::clear()
{
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
  assert(node_pos != node_index.end());
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
  assert(node_pos != node_index.end());
  
  list< set< pair<int, int> > >::iterator node_neighbors_it= (*node_pos).second;
  unsigned int erased= (*node_neighbors_it).erase(make_pair(_second, _edge));
  assert(erased==1);
  
  remove_node_if_isolated(_first);
  remove_node_if_isolated(_second);
  return 0;
}

int LWGraph::nodes_count()
{
  assert(node_index.size() == neighbors.size());
  return neighbors.size();
}

int LWGraph::get_nodes(set<int>& res)
{
  res.clear();
  for(map<int, list< set< pair<int, int> > >::iterator>::iterator it= 
    node_index.begin(); it!= node_index.end(); ++it)
    res.insert((*it).first);
  assert(res.size() == node_index.size());
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

void FindComponentsOf(LWGraph* g, set<int>& roots, 
    vector<pair<int,int> >& edge2nodes, vector<Component*>& newComponents)
{
  int edge;
  pair<int, int> *edgeNodes;
  set<int> nodes;
  vector< LWGraph* >* subGraphs;
  for(set<int>::iterator it= roots.begin(); it!= roots.end(); ++it)
  {
    edge= *it;
    edgeNodes= &edge2nodes[edge];
    nodes.insert(edgeNodes->first);
    nodes.insert(edgeNodes->second);
  }
  subGraphs= g->find_components_of(nodes);
  for(vector< LWGraph* >::iterator it= subGraphs->begin(); it!= 
    subGraphs->end(); ++it)
  {
    Component* newComp= new Component();
    newComp->graph= *it;
    newComponents.push_back(newComp);
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

void Cluster(set<int>& edges, vector<pair<int,int> >& edge2nodes,
    vector<Component*>& Components)
{
  vector< LWGraph* >* subGraphs;
  LWGraph graph;
  InsertEdgesUndirected(&graph, edges, edge2nodes);
  subGraphs= graph.cluster();
  for(vector< LWGraph* >::iterator it= subGraphs->begin(); it!= 
    subGraphs->end(); ++it)
  {
    Component* newComp= new Component();
    newComp->graph= *it;
    Components.push_back(newComp);
  }
}
}
