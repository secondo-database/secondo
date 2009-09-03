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

[1] Header File of the Transportation Mode-Bus Network Algebra

August, 2009 Jianqiu Xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef BusNetwork_H
#define BusNetwork_H


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
#include <list>
/*
Subclass for manage bus network

*/
struct Elem;

class BusNetwork{
public:
/*data description*/
  static string busrouteTypeInfo;//relation description for pre-defined paths
  enum BusRouteInfo{RID=0,LINENO,TRIP};
  static string btreebusrouteTypeInfo;//b-tree on pre-defined paths
  static string busstopTypeInfo; //relation description for bus stop
  enum BusStopInfo{SID=0,LOC};
  static string btreebusstopTypeInfo; //b-tree on bus stop
  static string rtreebusstopTypeInfo; //r-tree on bus stop
  static string busedgeTypeInfo; //relation description for edge
  enum BusEdgeInfo{EID=0,V1,V2,DEF_T,LINE,FEE,PID,MOVE};
  static string btreebusedgeTypeInfo; //b-tree on edge id

/*function for type constructor*/
  static ListExpr BusNetworkProp();
  static ListExpr OutBusNetwork(ListExpr,Word);
  static Word InBusNetwork(ListExpr,ListExpr,int,ListExpr&,bool&);
  static Word CreateBusNetwork(const ListExpr);
  static void DeleteBusNetwork(const ListExpr,Word&);
  static bool OpenBusNetwork(SmiRecord&,size_t&,const ListExpr,Word&);
  static bool SaveBusNetwork(SmiRecord&,size_t&,const ListExpr,Word&);
  static void CloseBusNetwork(const ListExpr,Word&);
  static Word CloneBusNetwork(const ListExpr,const Word&);
  static void* CastBusNetwork(void* addr);
  static int SizeOfBusNetwork();
  static bool CheckBusNetwork(ListExpr,ListExpr&);
  ListExpr Out(ListExpr typeInfo);
  void Load(int in_iId,const Relation* in_busRoute);
  static BusNetwork* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);
  bool Save(SmiRecord&,size_t&,const ListExpr);
/*Type Constructor and Deconstructor*/
  BusNetwork(ListExpr in_xValue,int in_iErrorPos,ListExpr& inout_xErrorInfo,
             bool& inout_bCorrect);
  BusNetwork();
  ~BusNetwork();
  BusNetwork(SmiRecord&,size_t&,const ListExpr);
  void FillBusNode(const Relation*);
  void FillBusEdge(const Relation*);
  void  Destory();
/*Interface function and Application function*/
  Relation* GetRelBus_Node(){return bus_node;}
  Relation* GetRelBus_Route(){return bus_route;}
  Relation* GetRelBus_Edge(){return bus_edge;}
  TupleId FindPointTid(Point& p);
  void FindPath_T_1(MPoint* result,MInt* query);
  void FindPath1(const UInt* ui1,const UInt* ui2,vector<int>& path);
  //more efficient
  void FindPath2(const UInt* ui1,const UInt* ui2,vector<int>& path);
  void FindPath_T_2(MPoint* result,MInt* query);//more efficient
  void Optimize1(priority_queue<Elem>& q_list,priority_queue<Elem>& temp_list);
  //time duration for middle stop
  bool FindPath3(const UInt* ui1,const UInt* ui2,vector<int>& path);
  void FindPath_T_3(MPoint* result,MInt* query);
private:
  int busnet_id;
  bool bus_def;
  Relation* bus_route;//relation storing bus route
  BTree* btree_bus_route; //b_tree on bus route
  Relation* bus_node; //relation storing bus stops
  BTree* btree_bus_node; //b-tree on bus stops
  R_Tree<2,TupleId>* rtree_bus_node; //r-tree on bus stops
  Relation* bus_edge;//relation storing edge
  BTree* btree_bus_edge; //b-tree on edge
  BTree* btree_bus_edge_v1; //b-tree on edge start node id
  BTree* btree_bus_edge_v2; //b-tree on edge end node id
};


#endif