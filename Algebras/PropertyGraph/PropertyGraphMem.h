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

#ifndef PGRAPHMEM_H
#define PGRAPHMEM_H

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"

#include "MPointer.h"
#include "MemoryObject.h"

#include "RelationSchemaInfo.h"
#include "Utils.h"

namespace pgraph {

class PGraph; // forward as mutual usage
class NodeRelInfo; // forward as mutual usage
class EdgeRelInfo; // forward as mutual usage
class RelationInfo;

enum RelationRoleType { RoleEdge, RoleNode };


//------------------------------------------------------------------
class RelStatistics
{
public:
    int  cardinality=-1;
    double  avgcardForward=-1;
    double  avgcardBackward=-1;
};

class RelationRegistry
{
   int Counter=0;
   std::map<std::string,int>  RelationIds;
   
public:
   std::vector<RelationInfo*>  RelationInfos;

   RelationInfo*  AddRelation(std::string name, RelationRoleType role);
   int  GetRelationId(std::string name);
   RelationInfo* GetRelationInfo(std::string name);
   RelationInfo* GetRelationInfo(int id);
   bool          IsIndexed(std::string relation,std::string attr);

   void Clear();
};


class RelationInfo
{
public:
   ~RelationInfo();

   RelStatistics   *statistics=NULL;
   RelationRoleType roleType=RelationRoleType::RoleEdge;
   std::string FromName=""; // only for edges - helps to tell the direction
   std::string ToName=""; // only for edges - helps to tell the direction
   std::string Name;
   std::string IdFieldName;
   int RelId;
   int IdAttrIndex=0;   // faster access to id field   
   int FromAttrIndex=0;   // faster access to id field
   int ToAttrIndex=0;
   RelationSchemaInfo RelSchema;
   std::map<int,int> IdTranslationTable;
   void InitRelSchema(ListExpr TupleInfo);
   void AddGlobalIndex(int global, int id);
};

struct Edge
{
   Edge(int edgeid, int relid,int from,int to) { EdgeId=edgeid; 
      RelId=relid; FromNodeId=from; ToNodeId=to; }
   int FromNodeId;
   int ToNodeId;
   int RelId;
   int EdgeId;
};

class AdjacencyList
{
public:   
   ~AdjacencyList();
   int NodeGlobalCounter=0;
   
   // both use the same index to get to the relId for further metadata
   std::vector<Tuple*> NodeList;
   std::vector<int> NodeRelIdList;

   //
   std::vector<std::vector<int>> OutGoingRels;
   std::vector<std::vector<int>> InGoingRels;

   // both use the same index to get to the relId for further metadata
   int EdgeGlobalCounter=0;
   std::vector<Tuple*> EdgeList;
   std::vector<Edge*> EdgeInfo;

   void AddNodeTuple(int relId, Tuple *tuple);
   void AddEdgeTuple(int relId, Tuple *tuple, RelationInfo *fromrel, 
      int fromID, RelationInfo *torel, int toID);
   
   void Clear();
};


class MemoryGraphObject : public mm2algebra::MemoryObject 
{
   public:
      MemoryGraphObject(){
         LOG(10, "MemoryObject constructor");
      }

      MemoryGraphObject (std::string _type, std::string db)
         :MemoryObject(false, db, _type) {
      };

      static std::string BasicType(){
         return "mpgraph";
      }
      
      static bool checkType( ListExpr list){
         if(!nl->HasLength(list,1)){
             return false;
         }
         if(!listutils::isSymbol(nl->First(list),BasicType())){
            return false;
         }

         return true;
      }
          
      MemoryObject* clone(){
         LOG(10, "MemoryObject* clone");
         return new MemoryGraphObject(objectTypeExpr, getDatabase());
      }

      std::string name;
      std::string mrelprefix;

   private:
   protected:
     
        ~MemoryGraphObject();

public:
   AdjacencyList AdjList;
   RelationRegistry RelRegistry;

   int initstate=0;
   std::string DumpInfo();
   void   DumpGraphDot(std::string filename);

   void   Clear();

   int    IsLoaded() { return initstate==1; }

   void   LoadData(PGraph *pg, bool forcerebuildStatistics);
   void   LoadNodeRelation(std::string memrelname, NodeRelInfo *relinfo, 
                           bool rebuildStatistics);
   void   LoadEdgeRelation(std::string memrelname, EdgeRelInfo *relinfo,
                           bool rebuildStatistics);
};
}


#endif
