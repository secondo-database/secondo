/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File for Creating Bus Network 

July, 2011 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef Road_Network_H
#define Road_Network_H


#include "Algebra.h"

#include "NestedList.h"

#include "QueryProcessor.h"
#include "RTreeAlgebra.h"
#include "BTreeAlgebra.h"
#include "TemporalAlgebra.h"
#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "NetworkAlgebra.h"
#include "SpatialAlgebra.h"

#include <fstream>
#include <stack>
#include <vector>
#include <queue>
#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "WinUnix.h"
#include "AvlTree.h"
#include "Symbols.h"
#include "Partition.h"
#include "PaveGraph.h"
#include "GeneralType.h"

struct GP_Point;

/////////////////////////////////////////////////////////////////////////
///////////////////road network graph////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*
a node corresponds to a junction 
two junctions are connected by 
(1) the same location on space
(2) route segments of one route (the two junctions belong to one route)

at this movement (2011.7.26) we assume each road has both up and down directions
so that for a gpoint it can go either direction along the route. 
but it is no problem to extend that if gpoint has up or down value, then only
one direction is available

*/
class RoadGraph{
  public:
    RoadGraph();
    RoadGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect);

    RoadGraph(SmiRecord&, size_t&, const ListExpr);

    ~RoadGraph();

    static string RGNodeTypeInfo;
    static string RGBTreeNodeTypeInfo;
    static string RGEdgeTypeInfo1;
    static string RGEdgeTypeInfo2;

    enum RG_NODE{RG_JUN_ID, RG_JUN_GP, RG_JUN_P, RG_ROAD_ID};
    enum RG_EDGE1{RG_JUN1, RG_JUN2};
    enum RG_EDGE2{RG_JUN_1, RG_JUN_2, RG_PATHA1, RG_PATH2};

    static ListExpr RoadGraphProp();
    static bool CheckRoadGraph(ListExpr type, ListExpr& errorInfo);
    static int SizeOfRoadGraph();
    static void* CastRoadGraph(void* addr);
    static Word CloneRoadGraph(const ListExpr typeInfo, const Word& w); 
    static void CloseRoadGraph(const ListExpr typeInfo, Word& w);
    static Word CreateRoadGraph(const ListExpr typeInfo);
    static void DeleteRoadGraph(const ListExpr typeInfo, Word& w);

    static bool SaveRoadGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);

    bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo);

    static bool OpenRoadGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
    static RoadGraph* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);
    
    static Word InRoadGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect);

    static ListExpr OutRoadGraph(ListExpr typeInfo, Word value); 
    ListExpr Out(ListExpr typeInfo); 

    unsigned int GetRG_ID(){return rg_id;}
    void Load(int, Relation*, Relation*, Relation*);
    void LoadEdge1(Relation* rel);
    void LoadEdge2(Relation* rel);

    Relation* GetNode_Rel(){return node_rel;}
    Relation* GetEdge_Rel1(){return edge_rel1;}
    Relation* GetEdge_Rel2(){return edge_rel2;}


    void FindAdj1(int node_id, vector<int>& list);
    void FindAdj2(int node_id, vector<int>& list);

    void GetJunctionsNode(int rid, vector<GP_Point>& res_list);


  private:

    unsigned int rg_id;//road graph id
    Relation* node_rel;//node relation

    BTree* btree_node;

    Relation* edge_rel1;
    DbArray<int> adj_list1;
    DbArray<ListEntry> entry_adj_list1;

    Relation* edge_rel2;
    DbArray<int> adj_list2;
    DbArray<ListEntry> entry_adj_list2;

};


/*
for shortest path searching  in road graph

*/
struct RNPath_elem:public Path_elem{
  double weight;
  double real_w;
  RouteInterval ri;

  bool len; //same spatial location or not
  Point to_loc;
  
  RNPath_elem(){}
  RNPath_elem(int p, int c, int t, double w1, double w2, RouteInterval& i,
              bool b, Point loc):Path_elem(p, c, t), weight(w1), real_w(w2), 
              ri(i), len(b), to_loc(loc){}
  RNPath_elem(const RNPath_elem& wp):Path_elem(wp),
            weight(wp.weight),real_w(wp.real_w),
            ri(wp.ri), len(wp.len), to_loc(wp.to_loc){}
  RNPath_elem& operator=(const RNPath_elem& wp)
  {
    Path_elem::operator=(wp);
    weight = wp.weight;
    real_w = wp.real_w;
    ri = wp.ri;  
    len = wp.len;
    to_loc = wp.to_loc;
    return *this;
  }
  bool operator<(const RNPath_elem& ip) const
  {
    return weight > ip.weight;
  }

  void Print()
  {
    cout<<" tri_index " <<tri_index<<" realweight "<<real_w
        <<" weight "<<weight<<endl;
    ri.Print(cout);
  }

};

struct GPoint_Dist{
  GPoint gp;
  Point p;
  double dist;
  GPoint_Dist(){}
  GPoint_Dist(GPoint& p1, Point& p2, double d):gp(p1), p(p2), dist(d){}
  GPoint_Dist(const GPoint_Dist& gpd):gp(gpd.gp), p(gpd.p), dist(gpd.dist){}
  GPoint_Dist& operator=(const GPoint_Dist& gpd)
  {
    gp = gpd.gp;
    dist = gpd.dist;
    p = gpd.p;
    return *this;
  }

  bool operator<(const GPoint_Dist& gpd) const
  {
    return dist < gpd.dist;
  }
  void Print()
  {
    cout<<gp<<endl;
    cout<<p<<" "<<dist<<endl;
  }
  
};

/*
query processing on the road graph

*/
struct RoadNav{
  
  
  vector<SimpleLine> path_list; 
  vector<GLine> gline_list;
  
  unsigned int count;
  TupleType* resulttype;
  
  RoadNav(){count = 0; resulttype = NULL;}
  

  ~RoadNav(){if(resulttype != NULL) delete resulttype;}
  
  void GenerateRoadLoc(Network* rn, int no, vector<GPoint>& gp_list, 
                                 vector<Point>& gp_loc_list);
  void ShortestPath(GPoint*, GPoint*, RoadGraph*, Network*, GLine* res);
  
  void ShortestPathSub(GPoint*, GPoint*, RoadGraph*, Network*, GLine* res);
  
  void ShortestPath2(GPoint*, GPoint*, RoadGraph*, Network*, GLine* res);
  void ShortestPathSub2(GPoint*, GPoint*, RoadGraph*, Network*, GLine* res);
  void DFTraverse(Network* rn, R_Tree<2,TupleId>* rtree, 
                          SmiRecordId adr, 
                          SimpleLine* line, vector<GPoint_Dist>& id_list);
};

#define LOOP_PRINT1(s) for(unsigned int i = 0; i < s.size();i++){ \
                      cout<<i<<" "<<s[i]<<endl;\
                      }

#define LOOP_PRINT2(s) for(unsigned int i = 0; i < s.size();i++){ \
                          s[i].Print(); \
                      }

#endif