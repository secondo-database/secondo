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

#include <string>

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "Symbols.h"

#include "../Relation-C++/RelationAlgebra.h"

#include "../MainMemory2/MPointer.h"
#include "../MainMemory2/Mem.h"
#include "../MainMemory2/MemoryObject.h"
#include "../MainMemory2/MemCatalog.h"

#include "Utils.h"
#include "PGraphMem.h"
#include "PGraph.h"
#include "RelationInfo.h"

using namespace std;


extern NestedList* nl;
extern QueryProcessor *qp;

namespace pgraph {

//-----------------------------------------------------------------------------
RelationInfo *RelationRegistry::AddRelation(string name,  
     RelationRoleType role)
{
    RelationInfo *ri=new RelationInfo();
    ri->Name=name;
    ri->roleType = role;
    ri->RelId=Counter++;
     
    RelationInfos.push_back(ri);
    RelationIds[ri->Name]=ri->RelId;

    return ri;
}

//----------------------------------------------------------------------------
int RelationRegistry::GetRelationId(string name)
{
   if (RelationIds.find(name)==RelationIds.end())
      return -1;

   return RelationIds[name];
}
//----------------------------------------------------------------------------
RelationInfo* RelationRegistry::GetRelationInfo(string name)
{
   int id=GetRelationId(name);
   if (id<0) return NULL;

   return RelationInfos[id];
}

//----------------------------------------------------------------------------
RelationInfo* RelationRegistry::GetRelationInfo(int id)
{
   return RelationInfos[id];
}

//----------------------------------------------------------------------------
void AdjacencyList::AddNodeTuple(int relId, Tuple *tuple)
{
   LOGOP(10, "AdjacencyList::AddNodeTuple","relId ",relId);

   NodeList.push_back(tuple);
   NodeRelIdList.push_back(relId);

   NodeGlobalCounter++;
}

//-----------------------------------------------------------------------------
void AdjacencyList::AddEdgeTuple(int relId, Tuple *tuple, 
    RelationInfo *fromrel, int fromID, RelationInfo *torel, int toID)
{
   LOGOP(10, "AdjacencyList::AddEdgeTuple","relId ",relId, " | ",
      fromID,"->",toID);

   // ignore if not exiting
   if (fromrel->IdTranslationTable.find(fromID)==fromrel->
          IdTranslationTable.end()) 
      return;
   if (torel->IdTranslationTable.find(toID)==torel->IdTranslationTable.end()) 
      return;

   // get from object
   int globfrom = fromrel->IdTranslationTable[fromID];
   int globto = torel->IdTranslationTable[toID];
   LOGOP(10, "AdjacencyList::AddEdgeTuple","translated ",globfrom,"->",globto);

   // keep tuple
   EdgeList.push_back(tuple);
   EdgeInfo.push_back(new Edge(EdgeGlobalCounter, relId, globfrom, globto));

   // add to outgoing/incoming lists
   LOGOP(10, "AdjacencyList::AddEdgeTuple","In/OutGoingRels ");
   vector<int> *list;

   //
   list = &OutgoingRels[globfrom];
   list->push_back(EdgeGlobalCounter);

   //
   list = &InGoingRels[globto];
   list->push_back(EdgeGlobalCounter);

   //
   EdgeGlobalCounter++;
}

//-----------------------------------------------------------------------------
AdjacencyList::~AdjacencyList()
{
    for(auto&& tuple : NodeList) {
        tuple->DeleteIfAllowed();
    }
    NodeList.clear();      

    for(auto&& tuple : EdgeList) {
        tuple->DeleteIfAllowed();
    }
    EdgeList.clear();      
    for(auto&& edge : EdgeInfo) {
        delete edge;
    }
    EdgeInfo.clear();      
}

//-----------------------------------------------------------------------------
void RelationInfo::InitRelSchema(ListExpr tupleinfo)
{
   RelSchema.LoadFromList(tupleinfo);
}

//-----------------------------------------------------------------------------
void RelationInfo::AddGlobalIndex(int global, int id)
{
   LOGOP(10, "NodeRelationInfo::AddGlobalIndex","id ",id," -> glob: ", global);
   IdTranslationTable[id]=global;
}

//-----------------------------------------------------------------------------
string MemoryGraphObject::DumpInfo()
{
   string info="PGRAPHMEM Information\n";
   info += " - initstate: "+std::to_string(initstate) + "\n";

   info += " - statistics: \n";
   info += "   - noderelations: \n";
   
   for(auto&& n: RelRegistry.RelationInfos)
   {
      RelStatistics *rs=n->statistics;
      if (n->roleType==RoleNode) {
         info += "    - "+n->Name;
         if (rs!=NULL)
            info+=+"   CARD: "+to_string(rs->cardinality); 
         info += " \n";
      }
   }
   info += "   - edgerelations: \n";
   for(auto&& n: RelRegistry.RelationInfos)
   {
      RelStatistics *rs=n->statistics;
      if (n->roleType==RoleEdge) {
         info += "    - "+n->Name;
         if (rs!=NULL)
            info+=+"   CARDFW: "+to_string(rs->avgcardForward)+
               " CARDBW: "+to_string(rs->avgcardBackward); 
         info += "\n";
      }
   }

   return info;
}


//-----------------------------------------------------------------------------
MemoryGraphObject::~MemoryGraphObject()
{
}

//-----------------------------------------------------------------------------
void MemoryGraphObject::LoadNodeRelation(NodeRelInfo *nrelinfo, 
   map<string,IndexInfo*> &indexinfo)
{
      LOGOP(10,"MemoryGraphObject::LoadNodeRelation", "loading "+
         nrelinfo->name);

      //SecondoCatalog* sc = SecondoSystem::GetCatalog();

      // get relation info
      ListExpr reltype= SecondoSystem::GetCatalog()->GetObjectTypeExpr(
            nrelinfo->name);
      if (nl->IsEmpty(reltype))
         throw SecondoException("relation not found");

      // register relation
      RelationInfo* relinfo=RelRegistry.AddRelation(nrelinfo->name, 
         RelationRoleType::RoleNode);
      relinfo->InitRelSchema(reltype);

      // indexes
      for(auto&& idx:indexinfo)
      {
         if (idx.second->NodeType==nrelinfo->name)
         {
             for(int i=0; i<relinfo->RelSchema.GetAttrCount(); i++) {
                if (relinfo->RelSchema.GetAttrInfo(i)->Name==
                     idx.second->PropName)
                {
                   RelIndexInfo *rel=new RelIndexInfo();
                   rel->FieldName=idx.second->PropName;
                   rel->IndexName=idx.second->IndexName;
                   relinfo->Indexes[idx.second->PropName]=rel;
                }
             }
         }
      }

      //
      GenericRelation* relation=QueryRelation(nrelinfo->name);

      GenericRelationIterator *iter = relation->MakeScan();
      Tuple* tuple;
      int counter=0;
      while( (tuple = iter->GetNextTuple()) != 0 )
      {
         // get nodeid
         int id=((CcInt*)tuple->GetAttribute(relinfo->RelSchema.
            GetAttrInfo(nrelinfo->idattr)->Index))->GetValue();

         // add to adjacency list and put global id to translation table
         AdjList.AddNodeTuple(relinfo->RelId, tuple);
         relinfo->AddGlobalIndex(AdjList.NodeGlobalCounter-1, id);
         counter++;

         memSize+=tuple->GetSize();
      }

      relinfo->statistics=new RelStatistics();
      relinfo->statistics->cardinality=counter;

      delete iter;
}

//-----------------------------------------------------------------------------
void MemoryGraphObject::LoadEdgeRelation(EdgeRelInfo *erelinfo)
{
      LOGOP(10,"MemoryGraphObject::LoadNodeRelation", "loading "+erelinfo
          ->EdgeRelName);

      ListExpr reltype= SecondoSystem::GetCatalog()
          ->GetObjectTypeExpr(erelinfo->EdgeRelName);
      if (nl->IsEmpty(reltype))
         throw SecondoException("relation not found");

      // register relation
      RelationInfo* relinfo=RelRegistry.AddRelation(erelinfo->EdgeRelName, 
         RelationRoleType::RoleEdge);
      relinfo->InitRelSchema(reltype);
      relinfo->FromName=erelinfo->FromRelName;
      relinfo->ToName=erelinfo->ToRelName;

      //
      GenericRelation* relation=QueryRelation(erelinfo->EdgeRelName);

      GenericRelationIterator *iter = relation->MakeScan();
      Tuple* tuple;

    RelationInfo *fromrel = RelRegistry.GetRelationInfo(erelinfo->FromRelName);
    RelationInfo *torel = RelRegistry.GetRelationInfo(erelinfo->ToRelName);
      
      while( (tuple = iter->GetNextTuple()) != 0 )
      {
         // get fromid
         int fromid=((CcInt*)tuple->GetAttribute(relinfo->RelSchema
             .GetAttrInfo(erelinfo->FromIdName)->Index))->GetValue();
         int toid=((CcInt*)tuple->GetAttribute(relinfo->RelSchema
             .GetAttrInfo(erelinfo->ToIdName)->Index))->GetValue();

         // add to adjacency list 
         // (from,toid are relative and need to be translated)
         AdjList.AddEdgeTuple(relinfo->RelId, tuple, fromrel, fromid,
              torel, toid);

         //
         memSize+=tuple->GetSize();
      }
      delete iter;

      //
      relinfo->statistics=new RelStatistics();

      // Loading statistics
      // query L2L3 feed groupby[IdFrom; C: group count] avg[C]
      // (avg (groupby (feed L1L2) (IdFrom) ((C (fun 
      // (group1 GROUP) (count (feed group1) ))))) C )
      double d=0;
      string query="(avg (groupby (feed "+erelinfo->EdgeRelName+") ("+
           erelinfo->FromIdName+") ((C (fun (group1 GROUP) (count "
              " (feed group1) ))))) C )";
      //cout << query<<endl;
      queryValueDouble(query,d);
      relinfo->statistics->avgcardForward=d;

      d=0;
      query="(avg (groupby (feed "+erelinfo->EdgeRelName+") ("+
           erelinfo->ToIdName+") ((C (fun (group1 GROUP) (count "
               "(feed group1) ))))) C )";
      //cout << query<<endl;
      queryValueDouble(query,d);
      relinfo->statistics->avgcardBackward=d;


      LOGOP(10,"MemoryGraphObject::LoadNodeRelation","finished" );


}
//-----------------------------------------------------------------------------
void MemoryGraphObject::LoadData(PGraph *pg)
{
   if (initstate==1)
   {
      //TODO clear and reload
      return;
   }

 for(std::map<string, NodeRelInfo*>::iterator itr = pg->_nodeRelations.begin();
      itr != pg->_nodeRelations.end(); itr++)
   {
      LoadNodeRelation(itr->second, pg->_nodeIndexes);
   }

   // preallocate vectors
   LOGOP(10,"MemoryGraphObject::LoadData", " resize adjlist ", 
      AdjList.NodeGlobalCounter);
   AdjList.OutgoingRels.resize( AdjList.NodeGlobalCounter );
   AdjList.InGoingRels.resize( AdjList.NodeGlobalCounter );


 for(std::map<string, EdgeRelInfo*>::iterator itr = pg->_edgeRelations.begin();
       itr != pg->_edgeRelations.end(); itr++)
   {
      LoadEdgeRelation(itr->second);
   }


   //
   initstate=1;
}

//----------------------------------------------------------------------------
void MemoryGraphObject::DumpGraphDot(string filename)
{

   ostringstream *data=new ostringstream(); 
   *data << "digraph g {\n";
   *data << "  rankdir = TB; \n";
   *data << "  subgraph { \n";

   map<string,vector<string>> ranks;

   for(unsigned int i=0; i<AdjList.NodeList.size(); i++) {
      
         RelationInfo *relinfo=RelRegistry.GetRelationInfo(
               AdjList.NodeRelIdList[i]);
         Tuple *tuple=AdjList.NodeList[i];

         // node         
         *data <<"n"<< i << + "[label=<:"+relinfo->Name << "["
            << to_string(i) << "]" "<BR/>";
         for (int a=0; a<relinfo->RelSchema.GetAttrCount();a++)
         {
            AttrInfo *ai=relinfo->RelSchema.GetAttrInfo(a);
            *data << ai->Name <<" =  " << ai->GetStringVal(tuple) << "<BR/>\n";

         }
         *data<<">]\n";    

         // rank
         ranks[relinfo->Name].push_back("n"+to_string(i));

         // outgoing 
         for (unsigned int a=0; a<AdjList.OutgoingRels[i].size(); a++)
         {
            Edge *e= AdjList.EdgeInfo[AdjList.OutgoingRels[i].at(a)];
            *data<<"n"<<i<<" -> n"<< e->ToNodeId <<"\n";
            *data << " [label=<";
            RelationInfo *reledge=RelRegistry.GetRelationInfo(e->RelId);
            *data<<":"<<reledge->Name << " ["<< e->EdgeId <<"]";
            *data<<">]\n";    
        }

    }

   for(auto&& r : ranks) 
   {
       *data << "{rank = same; ";
       for(auto&& rn : r.second) 
          *data<<rn<<";";
       *data<<" }\n";
   }

    //
   *data<<"}}\n";
   std::ofstream outfile;
   outfile.open(filename, std::ios_base::trunc);
   outfile << data->str();     
   delete data;
   
}

}//namespace
