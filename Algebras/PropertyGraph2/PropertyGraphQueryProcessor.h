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

#ifndef PGRAPHQUERYPROCESSOR_H
#define PGRAPHQUERYPROCESSOR_H

#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include <list>
#include "../Relation-C++/RelationAlgebra.h"

#include "QueryTree.h"
#include "PropertyGraphMem.h"


namespace pgraph2 {

enum InputStreamStateEnum { Reading, Closed };
enum MatchEdgeMode { First, Next };

//-----------------------------------------------------------------------------
class PGraphQueryProcessor
{
   void PrepareQueryTree(int id);
   void PrepareQueryTree2(int id);
   void PrepareQueryTreeForNextInputNode();
   void PrepareQueryTreeForNextInputNode2();
   void PrepareQueryTreeNextMatch();
   void PrepareQueryTreeNextMatch2();

   QueryTree *tree=NULL;


public:

   std::map<std::string,QueryTreeBase*> aliases;
   std::vector<QueryTreeEdge*> poslist;
   std::set<int> UsedEdgesList;
   std::map<std::string, std::map<int, std::vector<int>>> GlobalOutGoingNodes;

   TupleType *_OutputTupleType;
   InputStreamStateEnum InputStreamState;
   MemoryGraphObject *pgraphMem;
   PGraph2 *pg;

   ListExpr _InputTupleType;
   Address  _InputStreamAddress=NULL;
   GenericRelationIterator *_InputRelationIterator=NULL;

   PGraphQueryProcessor(PGraph2 *apg, MemoryGraphObject *pgm);
   ~PGraphQueryProcessor();

   void SetInputTupleType(ListExpr inputstreamtype);
   void SetInputStream(Address stream);
   void SetInputRelation(QueryTree *tree);

   void SetQueryTree(QueryTree *tree);

//Funktionen für Ordered Relations

   void GetOutGoingNodes(std::string relname, int localnodeid, bool reverse);
   bool CheckGlobalOutGoingNodes(std::string relname,
    int localnodeid, bool reverse);

   bool  CheckNode(int nodeid, QueryTreeNode *node);
   bool  CheckNode2(int nodeid, QueryTreeNode *node);
   bool  CheckEdge(QueryTreeEdge *queryedge, Edge* edge);
   bool  CheckEdge2(QueryTreeEdge *queryedge, int toid);
   bool  MatchesFilters(QueryTreeBase *item, RelationSchemaInfo *schema,
         Tuple *tuple);

   //
   bool NextEdge(QueryTreeEdge *queryedge);
   bool NextEdge2(QueryTreeEdge *queryedge);
   bool NextNode();
   bool NextNode2();

   bool UsedEdgeAlready(QueryTreeEdge *queryedge, uint edgeid);

   //
   bool  MatchNode(int nodeid, QueryTreeNode *node);
   bool  MatchNode2(int nodeid, QueryTreeNode *node);
   bool  MatchEdge(QueryTreeEdge *edge);
   bool  MatchEdge2(QueryTreeEdge *edge);

   Tuple *ReadNextResultTuple();
   Tuple *ReadNextResultTuple2();

   void ProcessNextMatch();


};

} // namespace
#endif
