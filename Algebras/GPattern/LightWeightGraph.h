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

namespace GPattern 
{
class LWGraph;
class Component;

class Component
{
public:
  enum ComponentMessage{NewlyAdded, NotChanged, NewlyAddedEdges, 
    NewlyAddedNodes, RemovedEdges, RemovedNodes, RemoveInFinalize, 
    RemoveNow, SplitFromExtistingComponent, NoMessage};
  LWGraph* graph;
  list< list<mset::CompressedMSet*>::iterator > resStreams;
  ComponentMessage message;
  Component():message(NoMessage){}
  bool Intersects(Component* arg);
  void SetMessage(ComponentMessage msg);
  bool UpdateMessage(ComponentMessage newMsg);
  int Union(Component* arg);
  void CopyEdgesFrom(Component* comp);
  void RemoveEdgesFrom(Component* comp);
};

class LWGraph
{
public:
  map<int,list< set< pair<int, int> > >::iterator> node_index;
  list< set< pair<int, int> > > neighbors;
  LWGraph(){clear();}
  int clear();  
  int copy_from(LWGraph* arg);
  //int create_graph(vector< pair<int,int> > *_edges, bool _directed);
  int insert_edge_directed(pair<int, int>* _edgeNodes, int& _edge);
  int insert_edge_directed(int& _first, int& _second, int& _edge);
  int remove_edge_directed(pair<int, int>& _edgeNodes, int& _edge);
  int remove_edge_directed(int& _first, int& _second, int& _edge);
  vector< LWGraph* >* cluster();
  vector< LWGraph* >* find_components_of(set<int>& _nodes);
  int nodes_count();
  int get_nodes(set<int>& res);
  ostream& print( ostream &os );
private:
  int remove_node_if_isolated(int _node);
};

void InsertEdgesUndirected(
    LWGraph* g, set<int>& edges, vector<pair<int,int> >& edge2nodes);

void FindComponentsOf(LWGraph* g, set<int>& roots, 
    vector<pair<int,int> >& edge2nodes, vector<Component*>& newComponents);

void RemoveEdgesUndirected(
    LWGraph* g, set<int>& edges, vector<pair<int,int> >& edge2nodes);

void Cluster(set<int>& edges, vector<pair<int,int> >& edge2nodes,
    vector<Component*>& Components);
}
