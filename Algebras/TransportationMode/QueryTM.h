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
  enum GenmoUnits{GM_TRAJ_ID = 0, GM_BOX, GM_MODE, GM_INDEX1, GM_INDEX2, GM_ID};

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
  void DecomposeGenmo_0(Relation*, double);
  void CreateMTuple_0(int oid, GenMO* mo1, MPoint* mo2, double);
  void CollectBusMetro(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, 
                       int& pos);
  void CollectIndoorFree(int& i, int oid, int m, GenMO* mo1, 
                         MPoint* mo2, int& pos);
  void CollectWalk(int& i, int oid, GenMO* mo1, MPoint* mo2, double, int &pos);
  void CollectCBT(int&i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
  void DecomposeGenmo_1(Relation*);
  ////////////////////get tm values for TM-Rtree nodes /////////////////////

  unsigned long Node_TM(R_Tree<3, TupleId>* tmrtree, Relation* rel, 
              SmiRecordId nodeid, int attr_pos);
  ////////////////////////////////////////////////////////////////////////
  void TM_RTreeNodes(TM_RTree<3,TupleId>*);
  void GetNodes(TM_RTree<3, TupleId>* tmrtree, SmiRecordId nodeid, int level);
};

#define TEN_METER 10.0

#endif
