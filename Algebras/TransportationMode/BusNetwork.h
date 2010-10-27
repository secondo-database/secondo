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

Oct, 2010 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef Bus_Network_H
#define Bus_Network_H


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

/*
store cell id and the number of road sections intersecting it.
count1 records how many bus route it should cover, e.g., 2 or 3
count2 records how many bus route it already covers

*/

struct Section_Cell{
  int cell_id;
  int sec_intersect_count;
  int count_1;//maximum
  bool def; 
  BBox<2> reg; 
  Section_Cell(){}
  Section_Cell(int id, int c, int c1,bool d,BBox<2>& r):
  cell_id(id),sec_intersect_count(c),count_1(c1),def(d), reg(r){}
  Section_Cell(const Section_Cell& sc):
  cell_id(sc.cell_id),sec_intersect_count(sc.sec_intersect_count),
  count_1(sc.count_1),def(sc.def), reg(sc.reg){}
  
  Section_Cell& operator=(const Section_Cell& sc)
  {
    cell_id = sc.cell_id;
    sec_intersect_count = sc.sec_intersect_count;
    count_1 = sc.count_1;
    def = sc.def;
    reg = sc.reg; 
    return *this; 
  }
  
  bool operator<(const Section_Cell& sc) const
  {
//    return sec_intersect_count > sc.sec_intersect_count; //from small to big
    return sec_intersect_count < sc.sec_intersect_count; //from big to small
  }
  void Print()
  {
    cout<<"cell_id "<<cell_id
        <<" sec_intersect_count "<<sec_intersect_count
        <<" minimum bus route "<<count_1<<endl;
  }
};

/*
structure that is used to create bus network 

*/
struct BusRoute{
  Network* n;
  Relation* rel1; //store sec_id,cell_id,cnt
  BTree* btree;
  
  Relation* rel2;//store cell paris, start cell -- end cell 
  
  unsigned int count;
  TupleType* resulttype;
  
  vector<GLine> bus_lines; 
  vector<Point> start_gp;
  vector<Point> end_gp; 
  vector<Line> bus_sections1;
  vector<Line> bus_sections2; 
  
  
  vector<BBox<2> > start_cells;
  vector<BBox<2> > end_cells; 
  vector<int> start_cell_id;
  vector<int> end_cell_id;
  
  BusRoute(Network* net,Relation* r1,BTree* b):
  n(net),rel1(r1),btree(b)
  {count = 0;resulttype=NULL;}
  
  BusRoute(Network* net,Relation* r1,BTree* b,Relation* r2):
  n(net), rel1(r1), btree(b), rel2(r2)
  {count = 0;resulttype=NULL;}
  
  BusRoute(){count=0;resulttype = NULL;}
  ~BusRoute(){if(resulttype != NULL) delete resulttype;}
  
  void CreateRoute1(int attr1,int attr2,int attr3,int attr4); 
  void BuildRoute(vector<Section_Cell>& cell_list3,
                     vector<Section_Cell> cell_list1, int attr1, int bus_no);
  void BuildRoute_Limit(vector<Section_Cell>& cell_list3,
                     vector<Section_Cell> cell_list1, int attr1, 
                     int bus_no,int limit_no);
  int FindEndCell(Section_Cell& start_cell,
                  vector<Section_Cell>& cell_list, float dist_val); 
  
  void CreateRoute2(int attr,int attr1,int attr2); 
  void ConnectCell(int attr,int from_cell_id,int end_cell_id);
};
#endif


