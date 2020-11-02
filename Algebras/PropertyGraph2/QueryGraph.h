/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

*/

#ifndef QUERYGRAPH_H
#define QUERYGRAPH_H

#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include <list> 
#include "QueryTree.h"


namespace pgraph {

class QueryGraphEdge;
class QueryGraphNode;

//-----------------------------------------------------------------------------
class QueryGraphNode
{
   
public:
   QueryGraphNode();

   int ID;
   int _uid;

   double Cost=1;

   std::string Alias="";
   std::string TypeName="";

   std::list<QueryFilter*> Filters;

};

//-----------------------------------------------------------------------------
class QueryGraphEdge
{
public:
   int ID;
   double CostFw=-1;
   double CostBw=-1;

   QueryGraphNode *FromNode=NULL;
   QueryGraphNode *ToNode=NULL;
   std::string Alias="";
   std::string TypeName="";
   std::list<QueryFilter*> Filters;
};

//-----------------------------------------------------------------------------
class QueryGraph
{
public:
   RelationRegistry *RelRegistry;
   int NextNodeID=0, NextEdgeID=0;

   // constructors and destructors
   QueryGraph(RelationRegistry *relreg);
   ~QueryGraph();

   std::list<QueryGraphEdge>  Edges;
   std::list<QueryGraphNode>  Nodes;
   QueryAliasList             AliasList;

   QueryGraphNode* addNode(std::string alias, std::string typename_,  
      ListExpr *props);
   void addEdge(std::string edegelias, std::string typename_,std::string alias, 
      std::string alias2, ListExpr *props);
   void readFilters(std::list<QueryFilter*> &filters, ListExpr list);

   QueryTree *CreateQueryTree(QueryGraphNode *n);
   QueryTree *CreateOptimalQueryTree(std::string forcealias="");

   QueryGraphNode *GetOptimalStartNode();

   bool IsConnected();
   void CompleteTypes();

   void ReadQueryGraph(std::string slist);
   void ReadQueryGraph(ListExpr alist);

   void DumpGraph();
   void DumpGraphDot(std::string fn);

   void Validate();

};

} // namespace
#endif