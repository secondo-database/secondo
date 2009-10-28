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
#include <sys/timeb.h>
#include <bitset>

/*
Subclass for manage bus network

*/
struct Elem;
struct ListEntry{
  ListEntry(){}
  ListEntry(int l,int h):low(l),high(h){}
  ListEntry(const ListEntry& le):low(le.low),high(le.high){}
  int low;
  int high;
};
/*path1(e1) switch to path2(e2)*/
struct SwithEntry{
  SwithEntry(){}
  SwithEntry(int a,int b):e1(a),e2(b){}
  SwithEntry(const SwithEntry& le):e1(le.e1),e2(le.e2){}
  int e1; //TupleId
  int e2; //TupleId
};

/*store bus node as a tree structure*/
struct Node_Tree{
  int id;
  int pathid;
  int pos;
  double t;
  Point p;
  double zorder;
  int left;
  int right;
  unsigned int diff_bus; //number of different bus routes
  int busline; //bus route
  Node_Tree(){}
  Node_Tree(int n_id,int n_pathid,int n_pos,double n_t,Point n_p,
            double n_zorder,int n_left,int n_right,int diff,int bus_line):
          id(n_id),pathid(n_pathid),pos(n_pos),t(n_t),p(n_p),zorder(n_zorder),
      left(n_left),right(n_right),diff_bus(diff),busline(bus_line){}
  Node_Tree(const Node_Tree& node):id(node.id),pathid(node.pathid),
    pos(node.pos),t(node.t),p(node.p),zorder(node.zorder),
    left(node.left),right(node.right),
    diff_bus(node.diff_bus),busline(node.busline){}
  void Print()
  {
    cout<<"id "<<id<<" pathid "<<pathid<<" pos "<<pos<<" zorder "<<zorder
        <<"left "<<left<<" right "<<right<<" loc <"<<p.GetX()
        <<" "<<p.GetY()<<">"<<endl;
  }
  Node_Tree& operator=(const Node_Tree& node)
  {
    id = node.id;
    pathid = node.pathid;
    pos = node.pos;
    t = node.t;
    p = node.p;
    zorder = node.zorder;
    left = node.left;
    right = node.right;
    diff_bus = node.diff_bus;
    busline = node.busline;
    return *this;
  }
};
struct E_Node_Tree;
struct BusNode{
  int id;
  double zval;
  int diff_bus;
  BusNode(){}
  BusNode(int i,double v,int d):id(i),zval(zval),diff_bus(d){}
  BusNode(const BusNode& node):id(node.id),
                                zval(node.zval),diff_bus(node.diff_bus){}
};
struct BusAdj{
  int tid;//edge tid
  double t;//arrive time
  int busroute;//which bus route
  int pathid;//path id
  double cost; //cost of this edge
  BusAdj(){}

};

//#define graph_model //old representation


class BusNetwork{
public:
/*data description*/

  static string busrouteTypeInfo;//relation description for pre-defined paths
  static string btreebusrouteTypeInfo;//b-tree on pre-defined paths
#ifdef graph_model
  enum BusRouteInfo{RID=0,LINENO,TRIP};
#else
  enum BusRouteInfo{RID=0,LINENO,UP,TRIP};
#endif

  static string busstopTypeInfo; //relation description for bus stop
  enum BusStopInfo{SID=0,LOC};
  static string btreebusstopTypeInfo; //b-tree on bus stop
  static string rtreebusstopTypeInfo; //r-tree on bus stop
  static string busedgeTypeInfo; //relation description for edge
  enum BusEdgeInfo{EID=0,V1,V2,DEF_T,LINE,FEE,PID,MOVE,RPID,P1,P2};
  static string btreebusedgeTypeInfo; //b-tree on edge id


/*new schema for bus stop*/
  static string newbusstopTypeInfo; //relation description for bus stop
  enum newBusStopInfo{NEWSID=0,NEWLOC,BUSPATH,POS,ZVAL,ATIME,BUSLINE};
  static string newbtreebusstopTypeInfo1;
  static string newbtreebusstopTypeInfo2;
  static string busstoptreeTypeInfo;
  enum busstoptreeInfo{NODETID=0,LEFTTID,RIGHTTID};//tid,left,right,arrive-t


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
  void FillAdjacency();
  void FillAdjacencyPath();
  void Destroy();
  void CalculateMaxSpeed();

  void FillBusNode_New(const Relation*);

  inline double ZValue(Point& p);
  void ConstructBusNodeTree();
  void CreateNodeTreeIndex(vector<bool>&);
/*Interface function and Application function*/
  Relation* GetRelBus_Node(){return bus_node;}
  Relation* GetRelBus_Route(){return bus_route;}
  Relation* GetRelBus_Edge(){return bus_edge;}
  Relation* GetRelBus_NodeNew(){return bus_node_new;}


  TupleId FindPointTid(Point& p);
  TupleId FindPointTidNew(int pathid,Point& p);
  void FindPath_T_1(MPoint* result,Relation* query,int attrpos,Instant*);
  void FindPath1(int id1,int id2,vector<int>& path,Instant*);
  //more efficient
  void FindPath2(int id1,int id2,vector<int>& path,Instant&);
  //more efficient than FindPath_t_1
  void FindPath_T_2(MPoint* result,Relation* query,int attrpos,Instant&);
  void Optimize1(priority_queue<Elem>& q_list,priority_queue<Elem>& temp_list);
  //time duration for middle stop ,optimize-1
  bool FindPath3(int id1,int id2,vector<int>& path,Relation*,BTree*,
                 Instant&,double);
  void FindPath_T_3(MPoint* result,Relation* query,Relation*,BTree*,int,int,
                    Instant&);
  void Optimize2(priority_queue<Elem>& q_list,priority_queue<Elem>& temp_list,
                list<Elem>& end_node_edge,double&);
  //input relation and b-tree, optimize-1,3
  bool FindPath4(int id1,int id2,vector<Elem>& path,Instant&,double&);
  void FindPath_T_4(MPoint* result,Relation* query,int,int,Instant&);

  void TestFunction(Relation*,BTree*);
  //optimize-2
  bool FindPath5(int id1,int id2,vector<Elem>& path,Instant&,double&);
  void FindPath_T_5(MPoint* result,Relation* query,
                    int,int,Instant&);
  ///////////////////// new representation for bus node /////////////////////
  bool Bus_Tree1(vector<int>& stops,vector<double>& duration,
       vector<E_Node_Tree>& path,Instant&);
  void FindPath_Bus_Tree1(MPoint* result,Relation* query,int,int,Instant&);
//  inline double Second_Cost(E_Node_Tree& node,Point& p);
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
  BTree* btree_bus_edge_path; //b-tree on edge pid, path
  double maxspeed;
  //similar network adjacency
  //record the start and end index for each edge in adjacencylist
  //the following two structurs work together and record for each edge which
  //edge can be expanded (time after it and has one common position in space)
  //1-- with edge_tuple_id to access adjacencylist_index
  //2-- get the start and end position in adjacencylist
  //3-- get the edge tuple id
  DBArray<ListEntry> adjacencylist_index;
  DBArray<int> adjacencylist;

  //DBArray<BusNode> db_bus_node; //id, zval, no_diff_bus
  //DBArray<> new_adjlist;
  //DBArray<int> entry_new_adjlist;

  //path adjacency, for each path, which paths it can switch to and how
//  DBArray<ListEntry> adj_path_index;
//  DBArray<SwithEntry> adj_path;
  Relation* bus_node_new;
  BTree* btree_bus_node_new1;//on attribute path
  BTree* btree_bus_node_new2;//on attribute zval

  DBArray<Node_Tree> bus_tree;
  DBArray<double> ps_zval; // point_id --> z-order val

};
double difftimeb(struct timeb* t1,struct timeb* t2);


#endif

