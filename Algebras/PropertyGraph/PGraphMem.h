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

#include "../MainMemory2/MPointer.h"
#include "../MainMemory2/MemoryObject.h"

#include "RelationInfo.h"
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
   map<string,int>  RelationIds;
   
public:
   vector<RelationInfo*>  RelationInfos;

   RelationInfo*  AddRelation(string name, RelationRoleType role);
   int  GetRelationId(string name);
   RelationInfo* GetRelationInfo(string name);
   RelationInfo* GetRelationInfo(int id);
};

class RelIndexInfo
{
public:
   string FieldName;
   string IndexName;
};

class RelationInfo
{
public:
   RelStatistics   *statistics;
   RelationRoleType roleType=RelationRoleType::RoleEdge;
   string FromName=""; // only for edges - helps to tell the direction
   string ToName=""; // only for edges - helps to tell the direction
   string Name;
   int RelId;
   map<string,RelIndexInfo*> Indexes;
   RelationSchemaInfo RelSchema;
   map<int,int> IdTranslationTable;
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
   vector<Tuple*> NodeList;
   vector<int> NodeRelIdList;

   //
   vector<vector<int>> OutgoingRels;
   vector<vector<int>> InGoingRels;

   // both use the same index to get to the relId for further metadata
   int EdgeGlobalCounter=0;
   vector<Tuple*> EdgeList;
   vector<Edge*> EdgeInfo;

   void AddNodeTuple(int relId, Tuple *tuple);
   void AddEdgeTuple(int relId, Tuple *tuple, RelationInfo *fromrel, 
      int fromID, RelationInfo *torel, int toID);
   
};


class MemoryGraphObject : public mm2algebra::MemoryObject {

    public:
        MemoryGraphObject(){
          LOG(10, "MemoryObject constructor");
        }

      MemoryGraphObject (string _type, string db)
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
        }
          
        MemoryObject* clone(){
          LOG(10, "MemoryObject* clone");
          return new MemoryGraphObject(objectTypeExpr, getDatabase());
        }


    private:
    protected:
      
        ~MemoryGraphObject();

public:
   AdjacencyList AdjList;
   RelationRegistry RelRegistry;

   int initstate=0;
   string DumpInfo();
   void   DumpGraphDot(string filename);

   void   LoadData(PGraph *pg);
   void   LoadNodeRelation(NodeRelInfo *relinfo, 
             map<string,IndexInfo*> &indexinfo);
   void   LoadEdgeRelation(EdgeRelInfo *relinfo);


};


}


#endif