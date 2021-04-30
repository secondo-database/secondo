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
#include "Algebras/FText/FTextAlgebra.h"

#include "PropertyGraphQueryProcessor.h"
#include "PropertyGraph2.h"

#include "../OrderedRelation/OrderedRelationAlgebra.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "Utils.h"

using namespace std;

namespace pgraph2 {

#define NO_REPEATED_EDGE_SEMANTIC

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
PGraphQueryProcessor::PGraphQueryProcessor(PGraph2 *apg, MemoryGraphObject *pgm)
{
    InputStreamState = InputStreamStateEnum::Closed;
    pg=apg;
    pgraphMem=pgm;
}

/*

 set the input Relation for the QueryProcessor

*/

//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetInputRelation(QueryTree *tree)
{
   string relname = tree->Root->TypeName;

   LOGOP(30,"PGraphQueryProcessor::SetInputRelation","relname: ", relname);
   Relation* rel=NULL;
   // node can be found by index directly
   if (tree->Root->Filters.size()==1 && (tree->Root->Filters.front()->Indexed))
   {
      QueryFilter *qf=tree->Root->Filters.front();
      string idxname=pgraphMem->name+"_idx_"+tree->Root->TypeName+"_"+qf->Name;
      string memtabname=pgraphMem->name+"_rel_"+tree->Root->TypeName;
      string query="";
      LOGOP(10,"PGraphQueryProcessor::SetInputRelation","use index : "+idxname);
      if (pg->structure =="memory"){
      query="(consume (mexactmatch "+idxname+" "+memtabname+" \""+
         qf->Value+"\"))";
      }
      else if (pg->structure=="orel"){
      query="(consume (exactmatch "+idxname+" "+relname+"P \""+
         qf->Value+"\"))";
      }
      rel=ExecuteQuery(query);
   }
   else
   {
      // open start node relation
      RelationInfo *ri = pgraphMem->RelRegistry.GetRelationInfo(relname);
      if (ri==NULL)
            throw PGraph2Exception("relation not defined");
      ListExpr reltype=SecondoSystem::GetCatalog()->GetObjectTypeExpr(ri->Name);
      if (nl->IsEmpty(reltype))
            throw PGraph2Exception("relation not found");
      ListExpr relinfo;
      LOGOP(10,"PGraphQueryProcessor::SetInputRelation","open relation : ",
         ri->Name);
      rel= OpenRelation(ri->Name, relinfo);
   }

   InputStreamState=InputStreamStateEnum::Reading;
   _InputRelationIterator = rel->MakeScan();

}

//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetInputStream(Address stream)
{
    if (InputStreamState==InputStreamStateEnum::Reading)
       throw PGraph2Exception("already reading input stream!");

    _InputStreamAddress=stream;

    Word rec;
    qp->Open(_InputStreamAddress);
    InputStreamState=InputStreamStateEnum::Reading;
}

/*

 checks if the given QueryTreeBase matches to the filters of the graph

*/
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
         //LOGOP(30, "check filter:",f->Name ," ",f->Value,"=",val);
         if (val!=f->Value) return false;
      }
   }
   return true;
}

//----------------------------------------------------------------------------
bool PGraphQueryProcessor::UsedEdgeAlready(QueryTreeEdge *queryedge,
   uint edgeid)
{
   for(auto &&edge: poslist)
   {
      if (edge==queryedge) break;
      if (edge->current_edgeid==edgeid) return true;
   }
   return false;
}

/*

 get the outgoing Nodes of the actual node

*/
//-----------------------------------------------------------------------------
void PGraphQueryProcessor::GetOutGoingNodes (std::string relname, \
    int localnodeid, bool reverse)
{
   Word resultword;
   bool defined;
   if (reverse) {
   relname=relname+"_B";
   }
   SecondoSystem::GetCatalog()->GetObject(relname, resultword, defined);

   OrderedRelation* orel = (OrderedRelation*) resultword.addr;

   std::vector<int> OutGoingNodes;

   OrderedRelationIterator* orelIt = 0;
   vector<void*> attributes(2);
   vector<SmiKey::KeyDataType> kElems(2);
   SmiKey test((int32_t) 0);
   kElems[0] = test.GetType();
   kElems[1] = test.GetType();

   CcInt* minNodeId = new CcInt(true,0);
   CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
   CcInt* actNodeInt = new CcInt(true,localnodeid);

   attributes[0] = actNodeInt;
   attributes[1] = minNodeId;
   CompositeKey actNodeLower(attributes,kElems,false);
   attributes[1] = maxNodeId;
   CompositeKey actNodeUpper(attributes,kElems,true);
   orelIt = (OrderedRelationIterator*) orel->MakeRangeScan(actNodeLower, \
    actNodeUpper);

   Tuple* tuple = orelIt->GetNextTuple();
   while (tuple != NULL) {
      if (!reverse) {
         int value = ((CcInt*)tuple->GetAttribute(1))->GetValue();
         OutGoingNodes.push_back(value);
      }
      else if (reverse) {
         int value = ((CcInt*)tuple->GetAttribute(0))->GetValue();
         OutGoingNodes.push_back(value);
      }
      tuple = orelIt->GetNextTuple();
   }
   GlobalOutGoingNodes[relname][localnodeid] = OutGoingNodes;
   if (orelIt!=NULL) {
   delete orelIt;
   orelIt=NULL;
   }
   if (orel!=NULL){
   delete orel;
   orel=NULL;
   }
}

/*

 check if the actual node has outgoing Edges or if they are already saved

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckGlobalOutGoingNodes (std::string relname, \
    int localnodeid, bool reverse)
{
   std::map<std::string, std::map<int, std::vector<int>>>::iterator it;
   std::map<int, std::vector<int>>::iterator it2;
   if (reverse) relname=relname+"_B";

   it = GlobalOutGoingNodes.find(relname);
   if (it != GlobalOutGoingNodes.end())
   {
      it2 = GlobalOutGoingNodes[relname].find(localnodeid);
      if (it2 != GlobalOutGoingNodes[relname].end())
      {
         return true;
      }

    }
   return false;
}

/*

 searching for resulttuple with the next edge

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::NextEdge(QueryTreeEdge *queryedge)
{
   //
   vector<int> *edgelist=NULL;
   if (!queryedge->Reverse)
      edgelist=&pgraphMem->AdjList.OutGoingRels.at(queryedge->FromNode
         ->current_nodeid);
   else
      edgelist=&pgraphMem->AdjList.InGoingRels.at(queryedge->FromNode
         ->current_nodeid);

   // find next edge
   for(uint i=queryedge->current_edgeindex+1; i<edgelist->size(); i++)
   {
      //
      Edge *einfo=pgraphMem->AdjList.EdgeInfo[edgelist->at(i)];
      LOGOP(30,"PGraphQueryProcessor::NextEdge", "checking edge index ",
          queryedge->current_edgeindex, " -> ",i);

      bool ok=true;

      // no-repeated edge semantics neo4j
#ifdef NO_REPEATED_EDGE_SEMANTIC
      if (UsedEdgeAlready(queryedge, einfo->EdgeId))
      {
         LOGOP(30,"PGraphQueryProcessor::MatchEdge", "edge already taken",
             einfo->EdgeId);
         ok=false;
      }
#endif

      if (ok) ok=CheckEdge(queryedge, einfo) ;

      if (ok) {
         LOGOP(30,"PGraphQueryProcessor::NextEdge", "next found edgeid:",
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

/*

 searching for resulttuple with the next edge

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::NextEdge2(QueryTreeEdge *queryedge)
{
   //
     vector<int> *edgelist=NULL;
     if (CheckGlobalOutGoingNodes(queryedge->TypeName,queryedge->FromNode->\
            current_nodeid,queryedge->Reverse)) {
        if (queryedge->Reverse){
           string relname_b=queryedge->TypeName+"_B";
           edgelist=&GlobalOutGoingNodes[relname_b][queryedge->FromNode->\
            current_nodeid];
        }
        else {
           edgelist=&GlobalOutGoingNodes[queryedge->TypeName][queryedge->\
            FromNode->current_nodeid];
        }
    }
    else
    {
        GetOutGoingNodes(queryedge->TypeName, queryedge->FromNode->\
            current_nodeid, queryedge->Reverse);
        if (queryedge->Reverse){
           string relname_b=queryedge->TypeName+"_B";
           edgelist=&GlobalOutGoingNodes[relname_b][queryedge->FromNode->\
            current_nodeid];
        }
        else {
           edgelist=&GlobalOutGoingNodes[queryedge->TypeName][queryedge->\
            FromNode->current_nodeid];
        }
    }
   // find next edge
   for(int i=queryedge->current_edgeindex+1; \
    i<static_cast<int>(edgelist->size()); i++)
   {
      //
     // Edge *einfo=pgraphMem->AdjList.EdgeInfo[edgelist->at(i)];
      LOGOP(30,"PGraphQueryProcessor::NextEdge", "checking edge index ",
          queryedge->current_edgeindex, " -> ",i);

      bool ok=true;

      if (ok) ok=CheckEdge2(queryedge, edgelist->at(i)) ;

      if (ok) {
         LOGOP(30,"PGraphQueryProcessor::NextEdge", "next found edgeid:", \
            queryedge->FromNode->current_nodeid);
         int nodeid=edgelist->at(i);
         queryedge->current_edgeindex=i;
         queryedge->current_edgeid=queryedge->FromNode->current_nodeid;
         queryedge->current_edge_tonodeid=nodeid;
         bool b=MatchNode2(nodeid, queryedge->ToNode);
         if (b) {
            return true;
            }
      }
   }
   return false;
}

/*

 recursive searching for result tuple

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchEdge(QueryTreeEdge *queryedge)
{
   LOGOP(30,"PGraphQueryProcessor::MatchEdge", "entering for node ",
       queryedge->FromNode->current_nodeid, " Direction: ",
       (queryedge->Reverse?"reverse":"forward"));

   //
   vector<int> *edgelist=NULL;
   if (!queryedge->Reverse)
      edgelist=&pgraphMem->AdjList.OutGoingRels[queryedge->FromNode
         ->current_nodeid];
   else
      edgelist=&pgraphMem->AdjList.InGoingRels[queryedge->FromNode
         ->current_nodeid];

   // try all edges
   LOGOP(30,"PGraphQueryProcessor::MatchEdge", "edgelist size: ",
      edgelist->size());
   for(uint i=0; i<edgelist->size(); i++)
   {
      Edge *einfo=pgraphMem->AdjList.EdgeInfo[edgelist->at(i)];
      bool ok=true;

      // no-repeated edge semantics neo4j
#ifdef NO_REPEATED_EDGE_SEMANTIC
      if (UsedEdgeAlready(queryedge, einfo->EdgeId))
      {
         LOGOP(30,"PGraphQueryProcessor::MatchEdge", "edge already taken",
             einfo->EdgeId);
         ok=false;
      }
#endif

      LOGOP(30,"PGraphQueryProcessor::MatchEdge", "checking edge index ",i,
         " [",einfo->FromNodeId," -> ",einfo->ToNodeId, "]");
      if (ok) ok=CheckEdge(queryedge, einfo) ;

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

/*

 recursive searching for result tuple

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchEdge2(QueryTreeEdge *queryedge)
{
   LOGOP(30,"PGraphQueryProcessor::MatchEdge", "entering for node ",
       queryedge->FromNode->current_nodeid, " Direction: ",
       (queryedge->Reverse?"reverse":"forward"));

   //
    vector<int> *edgelist=NULL;
     if (CheckGlobalOutGoingNodes(queryedge->TypeName,queryedge->FromNode->\
        current_nodeid,queryedge->Reverse)) {
        if (queryedge->Reverse){
           string relname_b=queryedge->TypeName+"_B";
           edgelist=&GlobalOutGoingNodes[relname_b][queryedge->FromNode->\
            current_nodeid];
        }
        else {
           edgelist=&GlobalOutGoingNodes[queryedge->TypeName][queryedge->\
            FromNode->current_nodeid];
        }
    }
    else
    {
        GetOutGoingNodes(queryedge->TypeName, queryedge->FromNode->\
            current_nodeid, queryedge->Reverse);
        if (queryedge->Reverse){
           string relname_b=queryedge->TypeName+"_B";
           edgelist=&GlobalOutGoingNodes[relname_b][queryedge->FromNode->\
            current_nodeid];
        }
        else {
           edgelist=&GlobalOutGoingNodes[queryedge->TypeName][queryedge->\
            FromNode->current_nodeid];
        }
    }

   // try all edges
   LOGOP(30,"PGraphQueryProcessor::MatchEdge", "edgelist size: ",
      edgelist->size());
   for(int i=0; i<static_cast<int>(edgelist->size()); i++)
   {
      //Edge *einfo=pgraphMem->AdjList.EdgeInfo[edgelist->at(i)];
      bool ok=true;

      LOGOP(30,"PGraphQueryProcessor::MatchEdge", "checking edge index ",i,
         " [",queryedge->FromNode->current_nodeid," -> ",edgelist->at(i), "]");
      if (ok) ok=CheckEdge2(queryedge, edgelist->at(i)) ;

      // edge ok - go deeper
      if (ok)
      {
         int tonode=edgelist->at(i);

         // memorize current edge index/id
         queryedge->current_edgeindex=i;
         queryedge->current_edgeid=queryedge->FromNode->current_nodeid;
         queryedge->current_edge_tonodeid=tonode;
         bool nodematched=MatchNode2(tonode, queryedge->ToNode);

         // found one
         if (nodematched) {
            return true;
            }

      };
   }
   return false;
}

/*

 checks if the given QueryEdge matches to the filter of the querytree

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckEdge(QueryTreeEdge *queryedge, Edge* edge)
{
   LOGOP(30,"PGraphQueryProcessor::CheckEdge");

   //queryedge->TypeName?
   RelationInfo* ri = pgraphMem->RelRegistry.GetRelationInfo(edge->RelId);
   Tuple *tuple=pgraphMem->AdjList.EdgeList[edge->EdgeId];

   // check edge type
   if ((queryedge->TypeName!="") && (queryedge->TypeName!=ri->Name))
   {
      LOGOP(30,"PGraphQueryProcessor::CheckEdge","typename failed");
      return false;
   }

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

/*

 checks if the given QueryEdge matches to the filter of the querytree

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckEdge2(QueryTreeEdge *queryedge, int toid)
{
   LOGOP(30,"PGraphQueryProcessor::CheckEdge");

   //queryedge->TypeName?
   RelationInfo* ri = pgraphMem->RelRegistry.\
    GetRelationInfo(queryedge->TypeName);

   Word resultword;
   bool defined;
   string relname = queryedge->TypeName;
   if (queryedge->Reverse) relname = relname+"_B";
   SecondoSystem::GetCatalog()->GetObject(relname, resultword, defined);
   OrderedRelation* orel = (OrderedRelation*) resultword.addr;

   OrderedRelationIterator* orelIt = 0;
   vector<void*> attributes(2);
   vector<SmiKey::KeyDataType> kElems(2);
   SmiKey test((int32_t) 0);
   kElems[0] = test.GetType();
   kElems[1] = test.GetType();

   CcInt* minNodeId = new CcInt(true,0);
   CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
   CcInt* actNodeInt = new CcInt(true,queryedge->FromNode->current_nodeid);

   attributes[0] = actNodeInt;
   attributes[1] = minNodeId;
   CompositeKey actNodeLower(attributes,kElems,false);
   attributes[1] = maxNodeId;
   CompositeKey actNodeUpper(attributes,kElems,true);
   orelIt = (OrderedRelationIterator*) orel->MakeRangeScan(actNodeLower, \
    actNodeUpper);

   Tuple* tuple = orelIt->GetNextTuple();

   while (tuple != NULL){
      if (!queryedge->Reverse) {
         if (((CcInt*)tuple->GetAttribute(1))->GetValue() == toid) {
         break;
         }
      }
      else if (queryedge->Reverse) {
         if (((CcInt*)tuple->GetAttribute(0))->GetValue() == toid) {
         break;
         }
      }
      tuple = orelIt->GetNextTuple();
   }

   if (tuple != NULL)
   {
   // check edge type
   if ((queryedge->TypeName!="") && (queryedge->TypeName!=ri->Name))
   {
      LOGOP(30,"PGraphQueryProcessor::CheckEdge","typename failed");
      tuple->DeleteIfAllowed();
      if (orelIt!=NULL) {
        delete orelIt;
        orelIt=NULL;
      }
      if (orel!=NULL){
        delete orel;
        orel=NULL;
      }
      return false;
   }

   // check node filters
   if (!MatchesFilters(queryedge, &ri->RelSchema, tuple)) {
      tuple->DeleteIfAllowed();
      if (orelIt!=NULL) {
        delete orelIt;
        orelIt=NULL;
      }
      if (orel!=NULL){
        delete orel;
        orel=NULL;
      }
      return false;
   }

   // check global filters
   if (queryedge->Alias!="") {
      if (!tree->filterList.Matches(queryedge->Alias, &ri->RelSchema, tuple)) {
         tuple->DeleteIfAllowed();
         if (orelIt!=NULL) {
           delete orelIt;
           orelIt=NULL;
         }
         if (orel!=NULL){
           delete orel;
           orel=NULL;
         }
         return false;
      }
   }


   // add alias mapping, if any
   if (queryedge->Alias!="") {
       aliases[queryedge->Alias]=queryedge;
   }
   tuple->DeleteIfAllowed();
   if (orelIt!=NULL) {
   delete orelIt;
   orelIt=NULL;
   }
   if (orel!=NULL){
   delete orel;
   orel=NULL;
   }

   return true;
   }
   if (orelIt!=NULL) {
   delete orelIt;
   orelIt=NULL;
   }
   if (orel!=NULL){
   delete orel;
   orel=NULL;
   }

   return false;
}

/*

 checks if the given QueryNode matches to the filter of the querytree

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckNode(int nodeid, QueryTreeNode *node)
{
   LOGOP(30,"PGraphQueryProcessor::CheckNode",nodeid);

   RelationInfo* ri = pgraphMem->RelRegistry.GetRelationInfo(
         pgraphMem->AdjList.NodeRelIdList[nodeid]);
   Tuple *tuple=pgraphMem->AdjList.NodeList[nodeid];

   // node type name matches
   if ((node->TypeName!="") && (node->TypeName!=ri->Name)) {
      LOGOP(30,"Type not expected");
      return false;
   }

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

/*

 checks if the given QueryNode matches to the filter of the querytree

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::CheckNode2(int nodeid, QueryTreeNode *node)
{
   LOGOP(30,"PGraphQueryProcessor::CheckNode",nodeid);

   RelationInfo* ri = pgraphMem->RelRegistry.GetRelationInfo(node->TypeName);

   Word resultword;
   bool defined;
   SecondoSystem::GetCatalog()->GetObject(node->TypeName, resultword, defined);
   OrderedRelation* orel = (OrderedRelation*) resultword.addr;

   OrderedRelationIterator* orelIt = 0;
   vector<void*> attributes(2);
   vector<SmiKey::KeyDataType> kElems(2);
   SmiKey test((int32_t) 0);
   kElems[0] = test.GetType();
   kElems[1] = test.GetType();

   CcInt* minNodeId = new CcInt(true,0);
   CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
   CcInt* actNodeInt = new CcInt(true,nodeid);

   attributes[0] = actNodeInt;
   attributes[1] = minNodeId;
   CompositeKey actNodeLower(attributes,kElems,false);
   attributes[1] = maxNodeId;
   CompositeKey actNodeUpper(attributes,kElems,true);
   orelIt = (OrderedRelationIterator*) orel->MakeRangeScan(actNodeLower, \
    actNodeUpper);

   Tuple* tuple = orelIt->GetNextTuple();

   if (tuple != NULL)
   {
   // node type name matches
   if ((node->TypeName!="") && (node->TypeName!=ri->Name)) {
      LOGOP(30,"Type not expected");
      tuple->DeleteIfAllowed();
      if (orelIt!=NULL) {
        delete orelIt;
        orelIt=NULL;
      }
      if (orel!=NULL){
        delete orel;
        orel=NULL;
      }
      return false;
   }

   // check node filters
   if (!MatchesFilters(node, &ri->RelSchema, tuple))
   {
      tuple->DeleteIfAllowed();
      if (orelIt!=NULL) {
        delete orelIt;
        orelIt=NULL;
      }
      if (orel!=NULL){
        delete orel;
        orel=NULL;
      }
      return false;
   }

   // check global filters
   if ( node->Alias!="")
      if (!tree->filterList.Matches(node->Alias, &ri->RelSchema, tuple)) {
         tuple->DeleteIfAllowed();
         if (orelIt!=NULL) {
           delete orelIt;
           orelIt=NULL;
         }
         if (orel!=NULL){
           delete orel;
           orel=NULL;
         }
         return false;
         }

   // keep nodeid and register alias (once necessary)
   node->current_nodeid=nodeid;

   // add alias mapping, if any
   if (node->Alias!="") {
      aliases[node->Alias]=node;
   }
   tuple->DeleteIfAllowed();
   if (orelIt!=NULL) {
     delete orelIt;
     orelIt=NULL;
   }
   if (orel!=NULL){
     delete orel;
     orel=NULL;
   }

   return true;
   }
   if (orelIt!=NULL) {
     delete orelIt;
     orelIt=NULL;
   }
   if (orel!=NULL){
     delete orel;
     orel=NULL;
   }
   return false;
}

/*

 searches for resulttuple with the next node

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::NextNode()
{
   LOGOP(30,"PGraphQueryProcessor::NextNode");

   bool found=false;
   int index=poslist.size()-1;
   while (true)
   {
      // highest entry already processed
      if (index<0) break;

      //
      LOGOP(30,"PGraphQueryProcessor::NextNode","increasing pos ",index);
      found = NextEdge(poslist.at(index));
      LOGOP(30,"PGraphQueryProcessor::NextNode","pos ",index, "increased: ",
         found?"true":"false");
      if (found) break;

      //
      index--;
   }


   if (found)
   {
      for(uint i=index+1;i<poslist.size(); i++)
      {
         LOGOP(30,"PGraphQueryProcessor::NextNode","reset pos ",i);
         MatchEdge(poslist.at(i));
      }
   }

   return found;

}

/*

 searches for resulttuple with the next node

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::NextNode2()
{
   LOGOP(30,"PGraphQueryProcessor::NextNode2");

   bool found=false;
   int index=poslist.size()-1;
   while (true)
   {
      // highest entry already processed
      if (index<0) break;

      //
      LOGOP(30,"PGraphQueryProcessor::NextNode","increasing pos ",index);
      found = NextEdge2(poslist.at(index));
      LOGOP(30,"PGraphQueryProcessor::NextNode","pos ",index, "increased: ",
         found?"true":"false");
      if (found) break;

      //
      index--;
   }


   if (found)
   {
      for(uint i=index+1;i<poslist.size(); i++)
      {
         LOGOP(30,"PGraphQueryProcessor::NextNode","reset pos ",i);
         MatchEdge2(poslist.at(i));
      }
   }

   return found;

}

/*

 recursive searching for resulttuple

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchNode(int nodeid, QueryTreeNode *node)
{
   LOGOP(30,"PGraphQueryProcessor::MatchNode",nodeid);

   // current node already set
   //if (node->current_nodeid==nodeid) return true;

   //
   bool ok=CheckNode(nodeid, node);
   if (ok)
   {
      LOGOP(30,"QueryTreeNode::Match","ok for ",nodeid, "(Type:",node->TypeName,
          ")","(Alias:",node->Alias,")");

      // try to match all edges
      for (auto&& queryedge:node->Edges)
      {
         bool edgefound = MatchEdge(queryedge);
         if (!edgefound)
         {
            LOGOP(30,"QueryTreeNode::Match"," no matching edge found !");
            return false;
         }
      }

      // OK
      return true;
   }
   else
   {
      LOGOP(30,"QueryTreeNode::Match"," node filter failed");
      return false;
   }

}

/*

 recursive searching for resulttuple

*/
//----------------------------------------------------------------------------
bool PGraphQueryProcessor::MatchNode2(int nodeid, QueryTreeNode *node)
{
   LOGOP(30,"PGraphQueryProcessor::MatchNode",nodeid);

   // current node already set
   //if (node->current_nodeid==nodeid) return true;

   //
   bool ok=CheckNode2(nodeid, node);
   if (ok)
   {
      LOGOP(30,"QueryTreeNode::Match","ok for ",nodeid, "(Type:",node->TypeName,
          ")","(Alias:",node->Alias,")");

      // try to match all edges
      for (auto&& queryedge:node->Edges)
      {
         bool edgefound = MatchEdge2(queryedge);
         if (!edgefound)
         {
            LOGOP(30,"QueryTreeNode::Match"," no matching edge found !");
            return false;
         }
      }

      // OK
      return true;
   }
   else
   {
      LOGOP(30,"QueryTreeNode::Match"," node filter failed");
      return false;
   }

}

/*

 setting the QueryTree

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::SetQueryTree(QueryTree *qtree)
{
   tree=qtree;
   tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;

   LOGOP(30, "PGraphQueryProcessor::SetQueryTree", "tn:",qtree->Root->TypeName,
    "alias:",qtree->Root->Alias);

    tree->Root->GetPosVector(&poslist);
    for (auto&& n:poslist)
    {
      LOGOP(30, "PGraphQueryProcessor::SetQueryTree", "tn:",n->TypeName,
         "alias:",n->Alias);
    }
}

/*

 continues searching with the actual state

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTreeNextMatch()
{
    LOGOP(30, "PGraphQueryProcessor::PrepareQueryTreeNextMatch");

   bool res= NextNode();
   if (res)
      tree->state=QueryTreeMatchStateEnum::MATCH_AVAILABLE;
   else
      tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;

}

/*

 continues searching with the actual state

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTreeNextMatch2()
{
    LOGOP(30, "PGraphQueryProcessor::PrepareQueryTreeNextMatch");

   bool res= NextNode2();
   if (res)
      tree->state=QueryTreeMatchStateEnum::MATCH_AVAILABLE;
   else
      tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;

}

/*

 preparing QueryTree

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTree(int id)
{
   LOGOP(30, "PGraphQueryProcessor::PrepareQueryTree","id:",id);
   //
   bool res=MatchNode(id, tree->Root);
   if (res)
      tree->state=QueryTreeMatchStateEnum::MATCH_AVAILABLE;
   else
      tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;
}

/*

 preparing QueryTree

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTree2(int id)
{
   LOGOP(30, "PGraphQueryProcessor::PrepareQueryTree","id:",id);
   //
   bool res=MatchNode2(id, tree->Root);
   if (res)
      tree->state=QueryTreeMatchStateEnum::MATCH_AVAILABLE;
   else
      tree->state=QueryTreeMatchStateEnum::NO_FURTHER_MATCH;
}

/*

 searching for results with the next input Node

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTreeForNextInputNode()
{
   Word rec;

   LOGOP(30, "PGraphQueryProcessor::PrepareQueryTreeForNextInputNode");

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
          throw PGraph2Exception("query root node needs a type name!");

      CcInt *vi=(CcInt*)tuple->GetAttribute( ri->IdAttrIndex );

      int id=ri->IdTranslationTable[vi->GetValue()];
      LOGOP(30, "PGraphQueryProcessor::PrepareQueryTreeForNextInputNode",
         "Input Node: ",ri->Name,"  local:",vi->GetValue()," glob: ",id );

      PrepareQueryTree(id);

      tuple->DeleteIfAllowed();

   }
}

/*

 searching for results with the next input Node

*/
//----------------------------------------------------------------------------
void PGraphQueryProcessor::PrepareQueryTreeForNextInputNode2()
{
   Word rec;

   LOGOP(30, "PGraphQueryProcessor::PrepareQueryTreeForNextInputNode");

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
          throw PGraph2Exception("query root node needs a type name!");

      CcInt *vi=(CcInt*)tuple->GetAttribute( ri->IdAttrIndex );

      int localid=vi->GetValue();
      LOGOP(30, "PGraphQueryProcessor::PrepareQueryTreeForNextInputNode",
         "Input Node: ",ri->Name,"  local:",vi->GetValue());

      PrepareQueryTree2(localid);

      tuple->DeleteIfAllowed();

   }
}

/*

 Reads the next ResultTuple

*/
//----------------------- -----------------------------------------------------
Tuple *PGraphQueryProcessor::ReadNextResultTuple()
{

   Tuple* res=NULL;
   LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple");

   if (tree->state==QueryTreeMatchStateEnum::NOT_INITIALIZED)
   {
       LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple","NOT INITIALIZED");
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
            LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple",
               "input stream closed");
            return NULL;
         }
         else
         {
            LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple",
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
   LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple","creating tuple");
   res = new Tuple(_OutputTupleType);
   for (auto&& outfield : tree->outputFields.Fields)
   {
      LOGOP(30,"prop:",outfield->PropertyName);
      AttrInfo *ainfo=NULL;
      Tuple *aliastuple=NULL;
      QueryTreeBase *item= aliases[outfield->NodeAlias];
      if (item!=NULL) {
         QueryTreeNode* node=dynamic_cast<QueryTreeNode*>(item);
         if (node!=NULL) {
            aliastuple = pgraphMem->AdjList.NodeList[node->current_nodeid];
//ord: aliastuple = pgraphMem->OrdRel.GetActualTuple(node->TypeName,node->
//current_nodeid);
            int relid = pgraphMem->AdjList.NodeRelIdList[node->current_nodeid];
// ord: int relid = pgraphMem->RelRegistry.GetRelationId(node->TypeName);
            RelationInfo *relinfo=pgraphMem->RelRegistry.GetRelationInfo(relid);
            ainfo=relinfo->RelSchema.GetAttrInfo(outfield->PropertyName);
         }
         QueryTreeEdge* edge=dynamic_cast<QueryTreeEdge*>(item);
         if (edge!=NULL)
         {
            Edge *edgeinfo = pgraphMem->AdjList.EdgeInfo[edge->current_edgeid];
            int relid = edgeinfo->RelId;
// ord: int relid = pgraphMem->RelRegistry.GetRelationId(edge->TypeName);
            RelationInfo *relinfo=pgraphMem->RelRegistry.GetRelationInfo(relid);
            aliastuple = pgraphMem->AdjList.EdgeList[edgeinfo->EdgeId];
// ord: aliastuple = pgraphMem->OrdRel.GetActualTuple(edge->TypeName, edge->
//current_edgeid);
            ainfo=relinfo->RelSchema.GetAttrInfo(outfield->PropertyName);
         }
      }
      if (ainfo!=NULL)
      {
         if (ainfo->TypeName=="text")
         {
            res->PutAttribute(outfield->index, new FText( true,
               ainfo->GetStringVal(aliastuple) ));
         }
         if (ainfo->TypeName=="int")
         {
            res->PutAttribute(outfield->index, new CcInt( true,
               std::stoi(ainfo->GetStringVal(aliastuple)) ));
         }
         if (ainfo->TypeName=="string")
         {
            res->PutAttribute(outfield->index, new CcString( true,
               ainfo->GetStringVal(aliastuple) ));
         }
      }
      else
      {
         res->PutAttribute(outfield->index, new CcString( true,"???" ));
      }
   }
   LOGOP(30,"/PGraphQueryProcessor::ReadNextResultTuple");
   return res;
}

/*

 Reads the next ResultTuple

*/
//----------------------- -----------------------------------------------------
Tuple *PGraphQueryProcessor::ReadNextResultTuple2()
{

   Tuple* res=NULL;
   LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple");

   if (tree->state==QueryTreeMatchStateEnum::NOT_INITIALIZED)
   {
       LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple","NOT INITIALIZED");
       return NULL;
   }

   // current tree hat further match?
   if (tree->state==QueryTreeMatchStateEnum::MATCH_AVAILABLE)
   {
      // try to get next match on existing node
      PrepareQueryTreeNextMatch2();
   }

   while (true)
   {
      if (tree->state==QueryTreeMatchStateEnum::NO_FURTHER_MATCH)
      {
         // no further start node in input stream
         if (InputStreamState==InputStreamStateEnum::Closed)
         {
            LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple",
               "input stream closed");
            return NULL;
         }
         else
         {
            LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple",
               "next input node");
            // continue with next match on existing node
            PrepareQueryTreeForNextInputNode2();
         }
      }

      if (tree->state==QueryTreeMatchStateEnum::MATCH_AVAILABLE)
      {
         break;
      }
   }

   // return tuple
   //TODO who releases these tuples
   LOGOP(30,"PGraphQueryProcessor::ReadNextResultTuple","creating tuple");
   res = new Tuple(_OutputTupleType);
   for (auto&& outfield : tree->outputFields.Fields)
   {
      LOGOP(30,"prop:",outfield->PropertyName);
      AttrInfo *ainfo=NULL;
      Tuple *aliastuple=NULL;
      QueryTreeBase *item= aliases[outfield->NodeAlias];
      OrderedRelationIterator* orelIt = NULL;
      OrderedRelation* orel = NULL;
      if (item!=NULL) {
         QueryTreeNode* node=dynamic_cast<QueryTreeNode*>(item);
         if (node!=NULL) {

            Word resultword;
            bool defined;
            SecondoSystem::GetCatalog()->GetObject(node->TypeName, \
                resultword, defined);
            orel = (OrderedRelation*) resultword.addr;

            orelIt = 0;
            vector<void*> attributes(2);
            vector<SmiKey::KeyDataType> kElems(2);
            SmiKey test((int32_t) 0);
            kElems[0] = test.GetType();
            kElems[1] = test.GetType();

            CcInt* minNodeId = new CcInt(true,0);
            CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
            CcInt* actNodeInt = new CcInt(true,node->current_nodeid);

            attributes[0] = actNodeInt;
            attributes[1] = minNodeId;
            CompositeKey actNodeLower(attributes,kElems,false);
            attributes[1] = maxNodeId;
            CompositeKey actNodeUpper(attributes,kElems,true);
            orelIt = (OrderedRelationIterator*) orel->\
                MakeRangeScan(actNodeLower,actNodeUpper);

            aliastuple = orelIt->GetNextTuple();

            int relid = pgraphMem->RelRegistry.GetRelationId(node->TypeName);
          RelationInfo *relinfo=pgraphMem->RelRegistry.GetRelationInfo(relid);
            ainfo=relinfo->RelSchema.GetAttrInfo(outfield->PropertyName);
         }
         QueryTreeEdge* edge=dynamic_cast<QueryTreeEdge*>(item);
         if (edge!=NULL)
         {
    //Edge *edgeinfo = pgraphMem->AdjList.EdgeInfo[edge->current_edgeid];
            //int relid = edgeinfo->RelId;
            int relid = pgraphMem->RelRegistry.GetRelationId(edge->TypeName);
            RelationInfo *relinfo=pgraphMem->RelRegistry.\
                GetRelationInfo(relid);
            Word resultword;
            bool defined;
            std::string relname=edge->TypeName;
            int toid = edge->current_edge_tonodeid;
            if (edge->Reverse) relname = relname+"_B";
            SecondoSystem::GetCatalog()->GetObject(relname, \
                resultword, defined);
            orel = (OrderedRelation*) resultword.addr;

            orelIt = 0;
            vector<void*> attributes(2);
            vector<SmiKey::KeyDataType> kElems(2);
            SmiKey test((int32_t) 0);
            kElems[0] = test.GetType();
            kElems[1] = test.GetType();

            CcInt* minNodeId = new CcInt(true,0);
            CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
            CcInt* actNodeInt = new CcInt(true,edge->current_edgeid);

            attributes[0] = actNodeInt;
            attributes[1] = minNodeId;
            CompositeKey actNodeLower(attributes,kElems,false);
            attributes[1] = maxNodeId;
            CompositeKey actNodeUpper(attributes,kElems,true);
            orelIt = (OrderedRelationIterator*) orel->\
                MakeRangeScan(actNodeLower,actNodeUpper);

            aliastuple = orelIt->GetNextTuple();

            while (aliastuple != NULL){
                if (!edge->Reverse) {
                    if (((CcInt*)aliastuple->GetAttribute(1))->\
                        GetValue() == toid) {
                    break;
                    }
                }
                else if (edge->Reverse) {
                    if (((CcInt*)aliastuple->GetAttribute(0))->\
                        GetValue() == toid) {
                    break;
                    }
                }
                aliastuple = orelIt->GetNextTuple();
            }
            ainfo=relinfo->RelSchema.GetAttrInfo(outfield->PropertyName);
         }
      }
      if (ainfo!=NULL)
      {
         if (ainfo->TypeName=="text")
         {
            res->PutAttribute(outfield->index, new FText( true,
               ainfo->GetStringVal(aliastuple) ));
         }
         if (ainfo->TypeName=="int")
         {
            res->PutAttribute(outfield->index, new CcInt( true,
               std::stoi(ainfo->GetStringVal(aliastuple)) ));
         }
         if (ainfo->TypeName=="string")
         {
            res->PutAttribute(outfield->index, new CcString( true,
               ainfo->GetStringVal(aliastuple) ));
         }
         aliastuple->DeleteIfAllowed();
         if (orelIt!=NULL) {
            delete orelIt;
            orelIt=NULL;
         }
         if (orel!=NULL){
            delete orel;
            orel=NULL;
         }
      }
      else
      {
         res->PutAttribute(outfield->index, new CcString( true,"???" ));
      }
   }
   LOGOP(30,"/PGraphQueryProcessor::ReadNextResultTuple");
   return res;
}

} // namespace
