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

#include "MSet.h"
#include "vector"
namespace GPattern 
{
class LWGraph;
class Component;
class NewComponent;
class DelComponent;
class DelComponent
{
  
};

class NewComponent
{
public:
  std::vector<int> affectedComponents;
  std::set<int> newNodes;
  std::ostream& Print( std::ostream &os );
  bool AffectsComponent(int label);
  void Union(NewComponent& arg);
};

class Component
{
public:
  enum ComponentMessage{NotChanged=0, AddedEdges, RemovedEdges, AddRemoveMix,
    NewlyAdded, RemoveNow, SplitFromExtistingComponent,
    MergedFromExistingComponents, ReDistribute};
  ComponentMessage message;
  int label;
  int parentLabel;
  //parentLabel helps the finalize function to know the parent of a
  //component whose message is SplitFromExtistingComponent. This is the only
  //use for this attribute. The attribute's value is written in UpdateRemove
  //function, and read in the Finalize function.
  std::set<int> nodes;
  int resultPartIndex;
  std::vector< std::list< std::vector<int> >::iterator> associatedResults;
  std::set<int> addedEdges;
  std::set<int> removedEdges;
  bool ExtendedTillLastChange;

  
  Component():message(NotChanged), label(-1),
    ExtendedTillLastChange(false){}
  void SetMessage(ComponentMessage msg);
  bool UpdateMessage(ComponentMessage newMsg);
  void SynchronizeNodes(LWGraph* g);
  bool IsConnected(LWGraph* g, int node1, int node2);
  void Cluster(LWGraph* g, std::list<std::set<int> >& splitComponents);
  void GetEdges(LWGraph* g, std::set<int>& compEdges);
  void Union(Component* arg);
  std::ostream& Print( std::ostream &os );
  void ExtendResStreamsTillNow(
      std::vector<mset::CompressedMSet*>& resultsParts,
      int64_t endtime, bool rc);
  void Reset();
  bool Intersects(std::set<int>* arg);
};

class LWGraph
{
public:
  std::map<int, int> node_component; 
  std::map<int,std::list< 
     std::set< std::pair<int, int> > >::iterator> node_index;
  std::list< std::set< std::pair<int, int> > > neighbors;
  LWGraph(){clear();}
  int clear();  
  int copy_from(LWGraph* arg);
  //int create_graph(vector< pair<int,int> > *_edges, bool _directed);
  int insert_edge_directed(std::pair<int, int>* _edgeNodes, int& _edge);
  int insert_edge_directed(int& _first, int& _second, int& _edge);
  int remove_edge_directed(std::pair<int, int>& _edgeNodes, int& _edge);
  int remove_edge_directed(int& _first, int& _second, int& _edge);
  std::vector< LWGraph* >* cluster();
  std::vector< LWGraph* >* find_components_of(std::set<int>& _nodes);
  int nodes_count();
  int get_nodes(std::set<int>& res);
  std::ostream& print( std::ostream &os );
private:
  bool remove_node_if_isolated(int _node);
};

void InsertEdgesUndirected(
    LWGraph* g, std::set<int>& edges, 
        std::vector<std::pair<int,int> >& edge2nodes);
void InsertEdgesUndirected(
    LWGraph* g, std::set<int>& edges, 
    std::vector<std::pair<int,int> >& edge2nodes,
    std::set<int>& newNodes);

void FindComponentsOf(
    LWGraph* g, std::list<Component*>* components, std::set<int>& roots, 
    std::vector<std::pair<int,int> >& edge2nodes, 
    std::vector<NewComponent>& newComponents);

int RemoveEdgeUndirected(LWGraph* g, int edge, std::pair<int,int>* edgeNodes);
void RemoveEdgesUndirected(
    LWGraph* g, std::set<int>& edges, 
    std::vector<std::pair<int,int> >& edge2nodes);
void RemoveEdgesUndirected(
    LWGraph* g, std::set<int>& edges, 
    std::vector<std::pair<int,int> >& edge2nodes, 
    std::set<int>& removedNodes);
//void Cluster(set<int>& edges, vector<pair<int,int> >& edge2nodes,
//    vector<Component*>& Components);

void SetGraphNodesComponent(LWGraph* graph, std::set<int>& nodes, int label);
void RemoveGraphNodesComponent(LWGraph* g, std::set<int>& nodes);
void UpdateGraphNodesComponent(LWGraph* g, std::set<int>& nodes, int label);

int FindEdgeComponent(LWGraph* graph, std::pair<int, int>* _edge);

std::list<Component*>::iterator 
GetComponentIt(std::list<Component*>* components, int label);

void ExpandInGraph(LWGraph*  graph, std::pair<int, int>* edgeNodes, 
    std::set<int>& curAffectedComponents, std::set<int>& curNewNodes);

void MergeNewComponents(std::vector<NewComponent>& newComponents, 
    std::set<int>& affectedComponents);

std::ostream& PrintSet( std::set<int>& arg, std::ostream &os );
std::ostream& PrintVector( std::vector<int>& arg, std::ostream &os );

bool HasOneComponent(std::set<int> edges, 
      std::vector< std::pair<int,int> >& edge2nodes);
bool IsOneComponent(mset::CompressedMSet* _mset, int n, 
    std::vector< std::pair<int,int> > & edge2nodes);
int GetNumComponents(
    std::set<int>& edges, int n, 
    std::vector< std::pair<int,int> > & edge2nodes);

bool SetIntersects(std::set<int>* set1, std::set<int>* set2);
}

