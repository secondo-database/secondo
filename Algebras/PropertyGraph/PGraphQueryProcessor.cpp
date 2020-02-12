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

#include "PGraphQueryProcessor.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "Utils.h"

namespace pgraph {

//----------------------------------------------------------------------------
PGraphQueryProcessor::~PGraphQueryProcessor()
{
    if (_InputStreamAddress!=0)
       qp->Close(_InputStreamAddress);
    if (_InputRelationIterator!=NULL)
       _InputRelationIterator=NULL;
    if (tree!=NULL) 
       delete tree;
}

//----------------------------------------------------------------------------
PGraphQueryProcessor::PGraphQueryProcessor()
{
    InputStreamState = InputStreamStateEnum::Closed;
}

//----------------------------------------------------------------------------
void  PGraphQueryProcessor::ReadOptionsFromString(string options)
{
   if (options=="") return;
   ListExpr argslist=0;
   nl->ReadFromString(options, argslist);

   LOGOP(10, "PGraphQueryProcessor::ReadOptionsFromString","arg: ",
      nl->ToString(argslist));

   while(true)
   {
      if (nl->IsEmpty(argslist)) break;
      ListExpr item=nl->First(argslist);
      argslist=nl->Rest(argslist);

      string name= nl->ToString(nl->First(item));
      string value= nl->ToString(nl->Second(item));

      if (name=="log")
         debugLevel=stoi(value);

      if (name=="dumpgraph") {
         OptionDumpQueryGraph = (value.find("graph") != string::npos);
         OptionDumpQueryTree = (value.find("tree") != string::npos);
      }

      if (name=="forcestart")
         OptionForceStartNode=value;

   }
}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetInputRelation(string relname)
{
   LOGOP(10,"PGraphQueryProcessor::SetInputRelation","relname: ", relname);
   // open start node relation
   RelationInfo *ri = pgraphMem->RelRegistry.GetRelationInfo(relname);
   if (ri==NULL)
         throw PGraphException("relation not defined");
   ListExpr reltype= SecondoSystem::GetCatalog()->GetObjectTypeExpr(ri->Name);
   if (nl->IsEmpty(reltype))
         throw PGraphException("relation not found");
   ListExpr relinfo;  
   LOGOP(10,"PGraphQueryProcessor::SetInputRelation","relnodes: ", ri->Name);
   Relation* rel= QueryRelation(ri->Name, relinfo);

   InputStreamState=InputStreamStateEnum::Reading;
   _InputRelationIterator = rel->MakeScan();
   SetInputTupleType(relinfo);
  
}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetInputTupleType(ListExpr inputstreamtype)
{
   LOGOP(10, "PGraphQueryProcessor::SetInputTupleType", "input tupletype:",   
       nl->ToString(inputstreamtype) );
    _InputTupleType=inputstreamtype;
}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetInputStream(Address stream)
{
    if (InputStreamState==InputStreamStateEnum::Reading) 
       throw PGraphException("already reading input stream!");

    _InputStreamAddress=stream;

    Word rec;
    qp->Open(_InputStreamAddress);
    InputStreamState=InputStreamStateEnum::Reading;
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchesFilters(QueryTreeBase *item, 
      RelationSchemaInfo *schema, Tuple *tuple)
{
   for (auto&& f:item->Filters)
   {
      AttrInfo *ai=schema->GetAttrInfo(f->Name);
      if (ai!=NULL)
      {
         string val=ai->GetStringVal(tuple);
      LOGOP(10, "check filter:",f->Name ," ",f->Value,"=",val);
         if (val!=f->Value) return false;
      }
   }
   return true; 
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::NextEdge(QueryTreeEdge *queryedge)
{
   //
   vector<int> *edgelist=NULL;
   if (!queryedge->Reverse)
      edgelist=&pgraphMem->AdjList.OutgoingRels.at(queryedge->FromNode
         ->current_nodeid);
   else   
      edgelist=&pgraphMem->AdjList.InGoingRels.at(queryedge->FromNode
         ->current_nodeid);

   // find next edge
   for(uint i=queryedge->current_edgeindex+1; i<edgelist->size(); i++)
   {
      Edge *einfo=pgraphMem->AdjList.EdgeInfo[edgelist->at(i)];
      LOGOP(10,"PGraphQueryProcessor::NextEdge", "checking edge index ",
          queryedge->current_edgeindex, " -> ",i);

      bool ok=CheckEdge(queryedge, einfo) ;

      if (ok) {
         LOGOP(10,"PGraphQueryProcessor::NextEdge", "next found edgeid:",
             einfo->EdgeId);
         int nodeid=queryedge->Reverse ? einfo->FromNodeId : einfo->ToNodeId;
         queryedge->current_edgeindex=i;
         queryedge->current_edgeid=einfo->EdgeId;
         bool b=MatchNode(nodeid, queryedge->ToNode);
         if (b) return true;
      }
   }


   return false;
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchEdge(QueryTreeEdge *queryedge)
{
   LOGOP(10,"PGraphQueryProcessor::MatchEdge", "entering for node ", 
       queryedge->FromNode->current_nodeid, " Direction: ", 
       (queryedge->Reverse?"reverse":"forward"));

   // 
   vector<int> *edgelist=NULL;
   if (!queryedge->Reverse)
      edgelist=&pgraphMem->AdjList.OutgoingRels[queryedge->FromNode
         ->current_nodeid];
   else   
      edgelist=&pgraphMem->AdjList.InGoingRels[queryedge->FromNode
         ->current_nodeid];

   // try all edges
   for(uint i=0; i<edgelist->size(); i++)
   {
      Edge *einfo=pgraphMem->AdjList.EdgeInfo[edgelist->at(i)];

      LOGOP(10,"PGraphQueryProcessor::MatchEdge", "checking edge index ",i, 
         " [",einfo->FromNodeId," -> ",einfo->ToNodeId, "]");
      bool ok=CheckEdge(queryedge, einfo) ;

      // edge ok - go deeper
      if (ok) 
      {
         int tonode= queryedge->Reverse ? einfo->FromNodeId : einfo->ToNodeId;

         // memorize current edge index/id
         queryedge->current_edgeindex=i;
         queryedge->current_edgeid=einfo->EdgeId;

         bool nodematched=MatchNode(tonode, queryedge->ToNode);

         // found one
         if (nodematched)
            return true;

      };
   }

   return false;
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckEdge(QueryTreeEdge *queryedge, Edge* edge)
{
   LOGOP(10,"PGraphQueryProcessor::CheckEdge");

   
   RelationInfo* ri = pgraphMem->RelRegistry.GetRelationInfo(edge->RelId);
   Tuple *tuple=pgraphMem->AdjList.EdgeList[edge->EdgeId];

   // check edge type
   if ((queryedge->TypeName!="") && (queryedge->TypeName!=ri->Name))
      return false;

   // check node filters
   if (!MatchesFilters(queryedge, &ri->RelSchema, tuple)) return false;

   // check global filters
   if (queryedge->Alias!="") {
      if (!tree->filterList.Matches(queryedge->Alias, &ri->RelSchema, tuple)) 
         return false;
   }


   // add alias mapping, if any
   if (queryedge->Alias!="") {
       aliases[queryedge->Alias]=queryedge;
   }

   return true;
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckNode(int nodeid, QueryTreeNode *node)
{
   LOGOP(10,"PGraphQueryProcessor::CheckNode",nodeid);

   //
   RelationInfo* ri = pgraphMem->RelRegistry.GetRelationInfo(
         pgraphMem->AdjList.NodeRelIdList[nodeid]);
   Tuple *tuple=pgraphMem->AdjList.NodeList[nodeid];

   // node type name matches
   if ((node->TypeName!="") && (node->TypeName!=ri->Name))
      return false;

   // check node filters
   if (!MatchesFilters(node, &ri->RelSchema, tuple)) return false;
   
   // check global filters
   if ( node->Alias!="")
      if (!tree->filterList.Matches(node->Alias, &ri->RelSchema, tuple)) 
         return false;

   // keep nodeid and register alias (once necessary)
   node->current_nodeid=nodeid;

   // add alias mapping, if any
   if (node->Alias!="") {
      aliases[node->Alias]=node;
   }

   return true;
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::NextNode()
{
   LOGOP(10,"PGraphQueryProcessor::NextNode");

   bool found=false;
   int index=poslist.size()-1;
   while (true)
   {
      // highest entry already processed
      if (index<0) break;

      //
      LOGOP(10,"PGraphQueryProcessor::NextNode","increasing pos ",index);
      found = NextEdge(poslist.at(index));
      LOGOP(10,"PGraphQueryProcessor::NextNode","pos ",index, "increased: ",
         found?"true":"false");
      if (found) break;
      
      //
      index--;
   }


   if (found)   
   {
      for(uint i=index+1;i<poslist.size(); i++)
      {
         LOGOP(10,"PGraphQueryProcessor::NextNode","reset pos ",i);
         MatchEdge(poslist.at(i));
      }
   }

   return found;

}


//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchNode(int nodeid, QueryTreeNode *node)
{
   LOGOP(10,"PGraphQueryProcessor::MatchNode",nodeid);

   // current node already set
   if (node->current_nodeid==nodeid) return true;

   //
   bool ok=CheckNode(nodeid, node);
   if (ok)
   {
      LOGOP(10,"QueryTreeNode::Match","ok for ",nodeid, "(Type:",node->TypeName,
          ")","(Alias:",node->Alias,")");

      // try to match all edges
      for (auto&& queryedge:node->Edges)
      {
         bool edgefound = MatchEdge(queryedge);
         if (!edgefound)
         {
            LOGOP(10,"QueryTreeNode::Match"," no matching edge found !");
            return false;
         }
      }

      // OK
      return true;
   }
   else
   {
      LOGOP(10,"QueryTreeNode::Match"," node filter failed");
      return false;
   }

}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetQueryTree(QueryTree *qtree)
{
   tree=qtree;
   tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;

   LOGOP(10, "PGraphQueryProcessor::SetQueryTree", "tn:",qtree->Root->TypeName, 
    "alias:",qtree->Root->Alias);

    tree->Root->GetPosVector(&poslist);
    for (auto&& n:poslist)
    {
      LOGOP(10, "PGraphQueryProcessor::SetQueryTree", "tn:",n->TypeName, 
         "alias:",n->Alias);
    }
}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTreeNextMatch()
{
    LOGOP(10, "PGraphQueryProcessor::PrepareQueryTreeNextMatch");

   bool res= NextNode();
   if (res)
      tree->state=QueryTreeMatchStateEnum::MATCH_AVAILABLE;
   else
      tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;
     
}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTree(int id)
{
   LOGOP(10, "PGraphQueryProcessor::PrepareQueryTree","id:",id);
   // 

   bool res=MatchNode(id, tree->Root);
   if (res)
      tree->state=QueryTreeMatchStateEnum::MATCH_AVAILABLE;
   else
      tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;
}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTreeForNextInputNode()
{
   Word rec;

   LOGOP(10, "PGraphQueryProcessor::PrepareQueryTreeForNextInputNode");

   Tuple *tuple = NULL;

   // read from relation
   if (_InputRelationIterator!=NULL)
   {
      tuple = _InputRelationIterator->GetNextTuple();
      if (tuple==NULL)
         InputStreamState=InputStreamStateEnum::Closed;
   }
   else
   {
      // read from stream
      qp->Request(_InputStreamAddress, rec);
      if ( qp->Received(_InputStreamAddress) ) 
         tuple=((Tuple*)rec.addr);
      else
         InputStreamState=InputStreamStateEnum::Closed;
   }  

   if (tuple!=NULL)
   {
      // get global id
      RelationInfo *ri=pgraphMem->RelRegistry.GetRelationInfo(tree->Root
         ->TypeName);
      if (ri==NULL)
          throw PGraphException("query root node needs a type name!");

      CcInt *vi=(CcInt*)tuple->GetAttribute(0 );
      
      int id=ri->IdTranslationTable[vi->GetValue()];
      LOGOP(10, "PGraphQueryProcessor::PrepareQueryTreeForNextInputNode", 
         "Input Node: ",ri->Name,"  local:",vi->GetValue()," glob: ",id );

      PrepareQueryTree(id);

      tuple->DeleteIfAllowed();        

   }
}

//----------------------- -----------------------------------------------------
Tuple *PGraphQueryProcessor::ReadNextResultTuple()
{

   Tuple* res=NULL;
   LOGOP(10,"PGraphQueryProcessor::ReadNextResultTuple");

   if (tree->state==QueryTreeMatchStateEnum::NOT_INITIALIZED)
   {
       LOGOP(10,"PGraphQueryProcessor::ReadNextResultTuple","NOT INITIALIZED");
       return NULL;
   }

   // current tree hat further match? 
   if (tree->state==QueryTreeMatchStateEnum::MATCH_AVAILABLE)
   {
      // try to get next match on existing node
      PrepareQueryTreeNextMatch();
   }

   while (true)
   {
      if (tree->state==QueryTreeMatchStateEnum::NO_FURTHER_MATCH)
      {
         // no further start node in input stream
         if (InputStreamState==InputStreamStateEnum::Closed)
         {
            LOGOP(10,"PGraphQueryProcessor::ReadNextResultTuple",
               "input stream closed");
            return NULL;
         }
         else 
         {
            LOGOP(10,"PGraphQueryProcessor::ReadNextResultTuple",
               "next input node");
            // continue with next match on existing node
            PrepareQueryTreeForNextInputNode();
         }
      }

      if (tree->state==QueryTreeMatchStateEnum::MATCH_AVAILABLE)
      {
         break;
      }
   }

   // return tuple
   //TODO who releases these tuples
   LOGOP(10,"PGraphQueryProcessor::ReadNextResultTuple","creating tuple");
   res = new Tuple(_OutputTupleType);
   for (auto&& outfield : tree->outputFields.Fields)
   {
      AttrInfo *ainfo=NULL;
      Tuple *aliastuple=NULL;
      QueryTreeBase *item= aliases[outfield->NodeAlias];
      if (item!=NULL) {
         QueryTreeNode* node=dynamic_cast<QueryTreeNode*>(item);
         if (node!=NULL) {
            aliastuple = pgraphMem->AdjList.NodeList[node->current_nodeid];
            int relid = pgraphMem->AdjList.NodeRelIdList[node->current_nodeid];
            RelationInfo *relinfo=pgraphMem->RelRegistry.GetRelationInfo(relid);
            ainfo=relinfo->RelSchema.GetAttrInfo(outfield->PropertyName);
         }
         QueryTreeEdge* edge=dynamic_cast<QueryTreeEdge*>(item);
         if (edge!=NULL) 
         {
            Edge *edgeinfo = pgraphMem->AdjList.EdgeInfo[edge->current_edgeid];
            int relid = edgeinfo->RelId;
            RelationInfo *relinfo=pgraphMem->RelRegistry.GetRelationInfo(relid);
            aliastuple = pgraphMem->AdjList.EdgeList[edgeinfo->EdgeId];
            ainfo=relinfo->RelSchema.GetAttrInfo(outfield->PropertyName);
         }
      }
      if (ainfo!=NULL)
      {
         res->PutAttribute(outfield->index, new CcString( true, 
            ainfo->GetStringVal(aliastuple) ));    
      }
      else
      {
         res->PutAttribute(outfield->index, new CcString( true,"???" ));    
      }
   }
   return res;
}


} // namespace