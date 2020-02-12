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

#ifndef QUERYTREE_H
#define QUERYTREE_H

#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include <list> 
#include "PGraphMem.h"
#include "QueryOutputFields.h"
#include "QueryFilterFields.h"


namespace pgraph {

class QueryTreeEdge;
class PGraphQueryProcessor;
class QueryTreeNode;
class QueryTreeState;

enum QueryTreeMatchStateEnum { NOT_INITIALIZED, MATCH_AVAILABLE, 
       NO_FURTHER_MATCH };

//-----------------------------------------------------------------------------
class QueryFilter
{
public:
   std::string Name="";
   std::string Value="";
   std::string Index="";

   QueryFilter* Clone();
};

//-----------------------------------------------------------------------------
class QueryTreeBase
{
public:
   virtual ~QueryTreeBase() = default; 

   std::string Alias="";
   std::string TypeName="";
   double Cost=0;

   std::list<QueryFilter*> Filters;

};

//-----------------------------------------------------------------------------
class QueryTreeNode: public QueryTreeBase
{
public:
   QueryTreeNode();
   ~QueryTreeNode();

   // state while matching
   int current_nodeid=-1;

   int _uid; 
   list<QueryTreeEdge*> Edges;

   double CalcCost();

   void Reset();

   void GetPosVector(std::vector<QueryTreeEdge*> *posvector);
};

//-----------------------------------------------------------------------------
class QueryTreeEdge: public QueryTreeBase
{
public:
   // state while matching
   int current_edgeindex=-1;
   int current_edgeid=-1;

   QueryTreeNode *FromNode;
   QueryTreeNode *ToNode;
   bool   Reverse=false;
};

//-----------------------------------------------------------------------------
class QueryTree
{
public:

   // constructors and destructors
   QueryTree() {}
   ~QueryTree();

   QueryTreeMatchStateEnum state=QueryTreeMatchStateEnum::NOT_INITIALIZED;

   QueryTreeNode *Root=NULL;
   QueryOutputFields outputFields;
   QueryFilterFields filterList;

   void ReadQueryTree(ListExpr list);
   void ReadFilterList(ListExpr list);
   void ReadOutputFieldList(ListExpr list);

   void Validate();

   void Reset();

   void aliasList(std::list<std::string> *list);

   void Clear();
   void DumpTree(QueryTreeNode *n=NULL, int level=0);
   void DumpTreeDot(QueryTreeNode *n, std::string fn, 
       std::ostringstream *data=NULL);

   // static helpers
   static QueryTreeNode* ReadNode(ListExpr list);
   static QueryTreeNode* ReadNodeInfo(ListExpr list);
   static void ReadEdges(QueryTreeNode *n, ListExpr list);
   static void ReadFilters(list<QueryFilter*> &filters, ListExpr list);
};

} // namespace
#endif