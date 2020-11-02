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

#include "MPointer.h"
#include "Mem.h"
#include "MemoryObject.h"
#include "MemCatalog.h"

#include "Utils.h"
#include "PropertyGraphMem.h"
#include "PropertyGraph.h"
#include "RelationSchemaInfo.h"

using namespace std;


extern NestedList* nl;
extern QueryProcessor *qp;

namespace pgraph {


//-----------------------------------------------------------------------------
void RelationRegistry::Clear()
{
   Counter=0;
   RelationIds.clear();

   //
   for(auto&& r : RelationInfos) delete r;
   RelationInfos.clear();      

}

//-----------------------------------------------------------------------------
bool RelationRegistry::IsIndexed(std::string relation,std::string attr)
{
    RelationInfo *ri=GetRelationInfo(relation);
    if (ri!=NULL)
    {
       AttrInfo *ai =ri->RelSchema.GetAttrInfo(attr);
       if (ai!=NULL)
          return ai->Indexed;
          
    }
    return false;
}

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
   LOGOP(30, "AdjacencyList::AddNodeTuple","relId ",relId);

   tuple->IncReference();

   NodeList.push_back(tuple);
   NodeRelIdList.push_back(relId);

   NodeGlobalCounter++;
}

//-----------------------------------------------------------------------------
void AdjacencyList::AddEdgeTuple(int relId, Tuple *tuple, 
    RelationInfo *fromrel, int fromID, RelationInfo *torel, int toID)
{
   LOGOP(30, "AdjacencyList::AddEdgeTuple","relId ",relId, " | ",
      fromID,"->",toID);

   tuple->IncReference();

   // ignore if not exiting
   if (fromrel->IdTranslationTable.find(fromID)==fromrel->
          IdTranslationTable.end()) 
      return;
   if (torel->IdTranslationTable.find(toID)==torel->IdTranslationTable.end()) 
      return;

   // get from object
   int globfrom = fromrel->IdTranslationTable[fromID];
   int globto = torel->IdTranslationTable[toID];
   LOGOP(30, "AdjacencyList::AddEdgeTuple","translated ",globfrom,"->",globto);

   // keep tuple
   EdgeList.push_back(tuple);
   EdgeInfo.push_back(new Edge(EdgeGlobalCounter, relId, globfrom, globto));

   // add to outgoing/incoming lists
   LOGOP(30, "AdjacencyList::AddEdgeTuple","In/OutGoingRels ");
   vector<int> *list;

   //
   list = &OutGoingRels[globfrom];
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
   Clear();
}

//-----------------------------------------------------------------------------
void AdjacencyList::Clear()
{
   LOGOP(30,"AdjacencyList::Clear");
   NodeGlobalCounter=0;
   EdgeGlobalCounter=0;

   NodeRelIdList.clear();
   OutGoingRels.clear();
   InGoingRels.clear();

   //
   for(auto&& tuple : NodeList)  tuple->DeleteIfAllowed();
   NodeList.clear();      

    //
   for(auto&& tuple : EdgeList) tuple->DeleteIfAllowed();
   EdgeList.clear();

   //
   for(auto&& edge : EdgeInfo) delete edge;
   EdgeInfo.clear();      
}

//-----------------------------------------------------------------------------
RelationInfo::~RelationInfo()
{
   if (statistics!=NULL) { delete statistics; statistics=NULL; };
}

//-----------------------------------------------------------------------------
void RelationInfo::InitRelSchema(ListExpr tupleinfo)
{
   RelSchema.LoadFromList(tupleinfo);
}

//-----------------------------------------------------------------------------
void RelationInfo::AddGlobalIndex(int global, int id)
{
   LOGOP(30, "NodeRelationInfo::AddGlobalIndex","id ",id," -> glob: ", global);
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
   Clear();
}

//-----------------------------------------------------------------------------
void MemoryGraphObject::LoadNodeRelation(string memrelname, 
      NodeRelInfo *nrelinfo, bool rebuildStatistics)
{
   LOGOP(10,"MemoryGraphObject::LoadNodeRelation", "loading "+
      nrelinfo->name);

   // get relation info
   ListExpr reltype = 
      SecondoSystem::GetCatalog()->GetObjectTypeExpr(nrelinfo->name);
   if (nl->IsEmpty(reltype))
      throw SecondoException("relation not found");

   // register relation
   RelationInfo* relinfo=RelRegistry.AddRelation(nrelinfo->name, 
      RelationRoleType::RoleNode);
   relinfo->InitRelSchema(reltype);
   relinfo->IdFieldName=nrelinfo->idattr;

   // 
   relinfo->IdAttrIndex=
      relinfo->RelSchema.GetAttrInfo(nrelinfo->idattr)->Index;

   //
   mm2algebra::MemoryRelObject* relation=QueryMemRelation(memrelname);
   if (relation==NULL) PGraphException("couldn't open relation:"+memrelname);

   int counter=0;
   for(auto&& tuple : *relation->getmmrel() )
   {
      // get nodeid
      int id=((CcInt*)tuple->GetAttribute(relinfo->IdAttrIndex))->GetValue();

      // add to adjacency list and put global id to translation table
      AdjList.AddNodeTuple(relinfo->RelId, tuple);
      relinfo->AddGlobalIndex(AdjList.NodeGlobalCounter-1, id);
      counter++;

      memSize+=tuple->GetSize();
   }
   
   // update as counted anyways (will be persistent)
   nrelinfo->StatCardinality=counter;

   relinfo->statistics=new RelStatistics();
   relinfo->statistics->cardinality=counter;
}

//-----------------------------------------------------------------------------
void MemoryGraphObject::LoadEdgeRelation(string memrelname,  
       EdgeRelInfo *erelinfo, bool rebuildStatistics)
{
   LOGOP(10,"MemoryGraphObject::LoadEdgeRelation", "loading "+erelinfo
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
   RelationInfo *fromrel = RelRegistry.GetRelationInfo(erelinfo->FromRelName);
   RelationInfo *torel = RelRegistry.GetRelationInfo(erelinfo->ToRelName);

   if (fromrel==NULL) 
       throw PGraphException("Relation not found: "+erelinfo->FromRelName);
   if (torel==NULL) 
       throw PGraphException("Relation not found: "+erelinfo->ToRelName);

   mm2algebra::MemoryRelObject* relation=QueryMemRelation(memrelname);
   if (relation==NULL) PGraphException("could not open relation: "+memrelname);

   relinfo->FromAttrIndex=
      relinfo->RelSchema.GetAttrInfo(erelinfo->FromIdName)->Index;
   relinfo->ToAttrIndex=
      relinfo->RelSchema.GetAttrInfo(erelinfo->ToIdName)->Index;

   for(auto&& tuple : *relation->getmmrel() )
   {
      // get fromid
      int fromid=((CcInt*)
         tuple->GetAttribute(relinfo->FromAttrIndex))->GetValue();
      int toid=((CcInt*)tuple->GetAttribute(relinfo->ToAttrIndex))->GetValue();

      // add to adjacency list 
      // (from,toid are relative and need to be translated to global id)
      AdjList.AddEdgeTuple(relinfo->RelId, tuple, fromrel, fromid,
            torel, toid);
   }
   
   // get persistent statistics
   relinfo->statistics=new RelStatistics();
   relinfo->statistics->avgcardForward=erelinfo->StatAvgForward;
   relinfo->statistics->avgcardBackward=erelinfo->StatAvgBackward;

   // Loading statistics if necessary
   if(rebuildStatistics)
   {
      LOGOP(10,"MemoryGraphObject::LoadEdgeRelation","getting edge statistics");
      double d=0;
      string query="(avg (groupby2 (smouterjoin (rename (mfeed "+mrelprefix+
          fromrel->Name+") n) (rename (mfeed "+mrelprefix+erelinfo->EdgeRelName+
          ") e) "+fromrel->IdFieldName+"_n "+erelinfo->FromIdName+"_e) ("+
          fromrel->IdFieldName+"_n) ( (C (fun (t TUPLE) (agg int) "  
          " (ifthenelse (isempty (attr t "+erelinfo->FromIdName+"_e)) agg "
          "(+ agg 1)) ) 0 ) ) ) C) ";
      LOGOP(30,"MemoryGraphObject::LoadEdgeRelation",
         "getting avg of forward relations per "+fromrel->Name,query);
      if (!queryValueDouble(query,d))
      {
         cout << "failed." << endl;
      };
      relinfo->statistics->avgcardForward=d;

      d=0;
      query="(avg (groupby2 (smouterjoin (rename (mfeed "+
         mrelprefix+torel->Name+") n) (rename (mfeed "+
         mrelprefix+erelinfo->EdgeRelName+") e) "+torel->IdFieldName+"_n "+
         erelinfo->ToIdName+"_e) ("+torel->IdFieldName+
         "_n) ( (C (fun (t TUPLE) (agg int) (ifthenelse (isempty (attr t "+
         erelinfo->ToIdName+"_e)) agg (+ agg 1)) ) 0 ) ) ) C) ";
      LOGOP(30,"MemoryGraphObject::LoadEdgeRelation",
         "getting avg of backward relations per "+torel->Name,query);
      queryValueDouble(query,d);
      relinfo->statistics->avgcardBackward=d;

      // keep persistent
      erelinfo->StatAvgForward=relinfo->statistics->avgcardForward;
      erelinfo->StatAvgBackward=relinfo->statistics->avgcardBackward;
   }

   LOGOP(30,"MemoryGraphObject::LoadEdgeRelation","finished" );


}

//-----------------------------------------------------------------------------
void MemoryGraphObject::Clear()
{
   initstate=0;
   AdjList.Clear();
   RelRegistry.Clear();

   //
   memSize=0;
}


//-----------------------------------------------------------------------------
void MemoryGraphObject::LoadData(PGraph *pg, bool forcerebuildStatistics)
{
   if (initstate==1)
   {
      Clear();
   }
   
   mrelprefix=pg->name+"_rel_";

   // load relations as memory objects
   for(auto&& item:pg->_nodeRelations)
   {
      LOGOP(10,"MemoryGraphObject::LoadData", " load to memory relation: "+
         item.second->name);
      DoLet( mrelprefix +item.second->name, "(mconsume (feed "+
         item.second->name+"))");
   }
   for(auto&& item:pg->_edgeRelations)
   {
      LOGOP(10,"MemoryGraphObject::LoadData", " load to memory relation: "+
         item.second->EdgeRelName);
      DoLet( mrelprefix+item.second->EdgeRelName, "(mconsume (feed "+
         item.second->EdgeRelName+"))");
   }

   // load relations as memory objects
   for(auto&& item:pg->_Indexes)
   {
      string idxname=pg->name+"_idx_"+item.first;
      ReplaceStringInPlace(idxname,".","_");
      string expr="(mcreateAVLtree "+mrelprefix+item.second->name+" "+
         item.second->attr+")";
      LOGOP(10,"MemoryGraphObject::LoadData", " create index: "+idxname);
      //LOGOP(10,"MemoryGraphObject::LoadData", " create index: "+expr);
      DoLet( idxname, expr);
   }

   // check for missing statistics
   if (!forcerebuildStatistics)
   {
      for(auto&& item:pg->_nodeRelations)
         if (item.second->StatCardinality<0) 
            { forcerebuildStatistics=true; break;};
      for(auto&& item:pg->_edgeRelations)
         if (item.second->StatAvgForward<0 || item.second->StatAvgBackward<0) 
            { forcerebuildStatistics=true; break;};
   }

   if (forcerebuildStatistics)
      LOGOP(10,"MemoryGraphObject::LoadData", "needs to update statistics ...");


   // load node relations
   for(auto&& item:pg->_nodeRelations)
      LoadNodeRelation( mrelprefix+item.second->name, item.second, 
         forcerebuildStatistics);

   // preallocate vectors
   LOGOP(10,"MemoryGraphObject::LoadData", " resize adjlist ", 
      AdjList.NodeGlobalCounter);
   AdjList.OutGoingRels.resize( AdjList.NodeGlobalCounter );
   AdjList.InGoingRels.resize( AdjList.NodeGlobalCounter );
   
   // load edge relations
   for(auto&& item : pg->_edgeRelations)
      LoadEdgeRelation( mrelprefix+item.second->EdgeRelName, item.second, 
         forcerebuildStatistics);

   // mark attributes as indexed
   for(auto&& item:pg->_Indexes) 
   {
      RelationInfo *ri=RelRegistry.GetRelationInfo(item.second->name);
      if (ri!=NULL)
      {
            AttrInfo *ai = ri->RelSchema.GetAttrInfo(item.second->attr);
            if (ai!=NULL)
               ai->Indexed=true;
      }
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
         for (unsigned int a=0; a<AdjList.OutGoingRels[i].size(); a++)
         {
            Edge *e= AdjList.EdgeInfo[AdjList.OutGoingRels[i].at(a)];
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
