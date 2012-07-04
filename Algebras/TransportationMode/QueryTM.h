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

[1] Header File of the Transportation Mode Algebra

March, 2011 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/
#ifndef __QUERYTM_H__
#define __QUERYTM_H__

#include <fstream>
#include <stack>
#include <vector>
#include <queue>
#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "RectangleAlgebra.h"
#include "WinUnix.h"
#include "AvlTree.h"
#include "Symbols.h"
#include "AlmostEqual.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../Temporal/TemporalAlgebra.h"


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
#include "Partition.h"
#include "PaveGraph.h"
#include "Indoor.h"
#include "GeneralType.h"
#include "TMRTree.h"

struct Traj_Mode{

  bitset<ARR_SIZE(str_tm)> modebits;

  Traj_Mode(){}
  Traj_Mode(bitset<ARR_SIZE(str_tm)> in):modebits(in){

  }
  Traj_Mode(const Traj_Mode& tm):modebits(tm.modebits){}
  Traj_Mode& operator=(const Traj_Mode& tm)
  {
    modebits = tm.modebits;
    return *this;
  }
  inline int Mode_Count(){return modebits.count();}
  void Print()
  {
    cout<<GetModeStringExt(modebits.to_ulong())<<endl;
  }
};


/*
query processing on generic moving objects and data types 

*/
struct QueryTM{
  unsigned int count;
  TupleType* resulttype; 
  vector<Line> line_list1;
  vector<Line3D> line3d_list; 

  static string GenmoRelInfo;
  enum GenmoRel{GENMO_OID = 0, GENMO_TRIP1, GENMO_TRIP2};
  static string GenmoUnitsInfo;
  enum GenmoUnits{GM_TRAJ_ID = 0, GM_BOX, GM_MODE, GM_SUBTRIP, GM_DIV, GM_ID};
  static string GenmoRangeQuery;
  enum GMORangeQuery{GM_TIME = 0, GM_SPATIAL, GM_Q_MODE};
  
  QueryTM(){ count = 0; resulttype = NULL;} 
  ~QueryTM(){if(resulttype != NULL) delete resulttype;}

  vector<int> oid_list;
  vector<Periods> time_list;
  vector<Rectangle<3> > box_list;
  
  vector<int> tm_list;
  vector<Point> index_list1;
  vector<Point> index_list2;
  vector<unsigned long> mode_list;
  
  vector<bool> b_list;
  vector<string> str_list;
  vector<int> entry_list;
  vector<int> level_list;

  vector<GenMO> genmo_list;
  vector<MPoint> mp_list;
  vector<int> unit_tid_list;//store movement tuple id from filter step

  set<int> res_traj_id;//store trajectory id;

  ////////////extension to the previous method//////////////////
  vector<int> Oid_list;
  vector<Rectangle<3> > Box_list;
  vector<int> Tm_list;
  vector<MPoint> Mp_list;
  vector<int> div_list;
  bool mt_type;
  ////////////////////////////////////////////////////////////////////////////
  //////////get 2D line in space or 3D line in a building///////////////////
  ////////////////////////////////////////////////////////////////////////////
  void GetLineOrLine3D(GenRange* gr, Space* sp);
  
  void GetLineInRoad(int , Line* l, Space* sp);
  void GetLineInRegion(int , Line* l, Space* sp);
  void GetLineInFreeSpace(Line* l);
  void GetLineInBusNetwork(int, Line* l, BusNetwork* bn);
  void GetLineInGRoom(int, Line* l, Space* sp);
  ////////////////////////////////////////////////////////////////////////////
  ///////////////// range queries on generic moving objects /////////////////
  ///////////////////////////////////////////////////////////////////////////
  void DecomposeGenmo(Relation*, double);
  void CreateMTuple_0(int oid, GenMO* mo1, MPoint* mo2, double);
  void CreateMTuple_1(int oid, GenMO* mo1, MPoint* mo2, double);

  void CollectBusMetro(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, 
                       int& pos);
  void CollectIndoor(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
  void CollectFree(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
  void CollectWalk(int& i, int oid, GenMO* mo1, MPoint* mo2, double, int &pos);
  void CollectCBT(int&i, int oid, int m, GenMO* mo1, MPoint* mo2, 
                  int& pos, double len);
  ////////////////////get tm values for TM-Rtree nodes /////////////////////

  unsigned long Node_TM(R_Tree<3, TupleId>* tmrtree, Relation* rel, 
              SmiRecordId nodeid, int attr_pos);
  ////////////////////////////////////////////////////////////////////////
  void TM_RTreeNodes(TM_RTree<3,TupleId>*);
  void GetNodes(TM_RTree<3, TupleId>* tmrtree, SmiRecordId nodeid, int level);
  ////////////////range query using tmrtree//////////////////////////////////
  void RangeTMRTree(TM_RTree<3,TupleId>*, Relation*, Relation*, int );
  void SinMode_Filter1(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                      int bit_pos, Relation* units_rel);
  void SinMode_Filter2(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                      int bit_pos, Relation*);
  void SinMode_Filter3(TM_RTree<3, TupleId>* tmrtree, Rectangle<3> box, 
                       int bit_pos, Relation*);

  void SinMode_Refinement(Rectangle<3> query_box, int bit_pos,
                          Relation* units_rel);
  /////////////////////////////////////////////////////////////////////////
  ///////////////////// multile modes ////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  void MulMode_Filter1(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                      vector<bool> bit_pos, Relation* units_rel);
  void MulMode_Filter2(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                      vector<bool> bit_pos, Relation* units_rel);
  void MulMode_Filter3(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                      vector<bool> bit_pos, Relation* units_rel);
  void MulMode_Refinement(Rectangle<3> query_box, vector<bool> bit_pos,
                          Relation* units_rel, int mode_count);
  int ModeType(string mode, vector<long>& seq_tm);
  inline bool CheckMPoint(MPoint* mp, int start, int end, 
                          Interval<Instant>& time_span, Region* query_reg);
  /////////////////////////////////////////////////////////////////////////
  //////////////// simple(baseline) method for testing correctness/////////
  ////////////////////////////////////////////////////////////////////////
  void RangeQuery(Relation* rel1, Relation* rel2);
  void Sin_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box,
                             int m);
  void Mul_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box,
                             int m);

  bool ContainMode1_Sin(MReal* mode_index, int m);
  bool ContainMode1_Mul(MReal* mode_index, vector<bool> bit_pos, int);
  
  bool ContainMode2_Sin(MReal* mode_index, Interval<Instant>& t, int m);
  void ContainMode2_Mul(map<int, Traj_Mode>& res_traj, MReal* mode_index, 
                        Interval<Instant>& t, vector<bool> bit_pos, int, int);
};

#define TEN_METER 10.0
#define SINMODE 1
#define MULMODE 2
#define SEQMODE 3

#define TMRTREE 1
#define ADRTREE 2
#define RTREE3D 3
#define TMRTREETEST 4
#endif
