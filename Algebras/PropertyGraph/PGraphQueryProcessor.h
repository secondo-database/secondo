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
#include "PGraphMem.h"


namespace pgraph {

enum InputStreamStateEnum { Reading, Closed };
enum MatchEdgeMode { First, Next };

//-----------------------------------------------------------------------------
class PGraphQueryProcessor
{
   void PrepareQueryTree(int id);
   void PrepareQueryTreeForNextInputNode();
   void PrepareQueryTreeNextMatch();

   QueryTree *tree=NULL;

   std::string OptionForceStartNode="";

public:

   bool OptionDumpQueryTree=false;
   bool OptionDumpQueryGraph=false;
   

   std::map<string,QueryTreeBase*> aliases;
   std::vector<QueryTreeEdge*> poslist;

   TupleType *_OutputTupleType;
   InputStreamStateEnum InputStreamState;
   MemoryGraphObject *pgraphMem;
          
   ListExpr _InputTupleType;
   Address  _InputStreamAddress=NULL;
   GenericRelationIterator *_InputRelationIterator=NULL;

   PGraphQueryProcessor();
   ~PGraphQueryProcessor();

   void SetInputTupleType(ListExpr inputstreamtype);
   void ReadOptionsFromString(string options);
   void SetInputStream(Address stream);
   void SetInputRelation(std::string relname);

   void SetQueryTree(QueryTree *tree);

   //
   bool  CheckNode(int nodeid, QueryTreeNode *node);
   bool  CheckEdge(QueryTreeEdge *queryedge, Edge* edge);
   bool  MatchesFilters(QueryTreeBase *item, RelationSchemaInfo *schema, 
         Tuple *tuple);

   //
   bool NextEdge(QueryTreeEdge *queryedge);
   bool NextNode();

   // 
   bool  MatchNode(int nodeid, QueryTreeNode *node);
   bool  MatchEdge(QueryTreeEdge *edge);

   Tuple *ReadNextResultTuple();

   void ProcessNextMatch();
   
   
};


} // namespace
#endif