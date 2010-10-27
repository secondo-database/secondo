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

[1] Source File of the Transportation Mode Algebra

Oct, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
creating bus network 

*/
#include "BusNetwork.h"

/*
create route bus route 
a pair of cells, start cell-end cell.
the start cell can be considered as the start location of a bus route
and the end cell is the end location of a bus route.
Then it randomly selectes two locations in these two cells 

*/
void BusRoute::CreateRoute1(int attr1,int attr2,int attr3,int attr4)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 
//  cout<<"CreateBusRoute()"<<endl; 

  int max_cell_id = -1; 
  int max_cnt = -1;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_cell = rel1->GetTuple(i,false);
    CcInt* cell_id = (CcInt*)tuple_cell->GetAttribute(attr2); 
    CcInt* cnt = (CcInt*)tuple_cell->GetAttribute(attr3); 
    
//    cout<<sec_id->GetIntval()<<" "
//        <<cell_id->GetIntval()<<" "<<cnt->GetIntval()<<endl; 
  
    if(cell_id->GetIntval() > max_cell_id)
        max_cell_id = cell_id->GetIntval(); 
    
    if(cnt->GetIntval() > max_cnt) 
      max_cnt = cnt->GetIntval(); 
      
    tuple_cell->DeleteIfAllowed(); 
  }

//  cout<<"max_cell_id "<<max_cell_id<<endl;
//    cout<<"max_cnt "<<max_cnt<<endl; 
    
  /********initialize the array storing section_cell structure**************/
  vector<Section_Cell> cell_list; //////////////////////array 
  
//  cell_list.resize(max_cell_id); 
//  for(int i = 0;i < max_cell_id;i++)
//    cell_list[i].def = false;  //crashes if all BuildRoute is called 3 times 
  
  for(int i = 0;i < max_cell_id;i++){
    double min[2], max[2];
    min[0] = 0;
    min[1] = 0;
    max[0] = 1;
    max[1] = 1;
    BBox<2>* reg = new BBox<2>(true,min,max);
    Section_Cell* sc_ptr = new Section_Cell(0,0,0,false,*reg);
    cell_list.push_back(*sc_ptr);
    delete reg;
    delete sc_ptr;
  }

  ///////////////////////////////////////////////////////////////////

  float div_number1 = 0.5; 
  float div_number2 = 0.2; 
  
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_cell = rel1->GetTuple(i,false);
//    CcInt* sec_id = (CcInt*)tuple_cell->GetAttribute(attr1); 
    CcInt* cell_id = (CcInt*)tuple_cell->GetAttribute(attr2); 
    CcInt* cnt = (CcInt*)tuple_cell->GetAttribute(attr3); 
    Region* reg = (Region*)tuple_cell->GetAttribute(attr4);
    
    int cellid = cell_id->GetIntval();
    cell_list[cellid - 1].cell_id = cellid; 
    cell_list[cellid - 1].sec_intersect_count = cnt->GetIntval();
    cell_list[cellid - 1].reg = reg->BoundingBox(); 
  
    if(cnt->GetIntval() > (int)(max_cnt*div_number1)){  
        cell_list[cellid - 1].count_1 = 3;
    }    
    else if(cnt->GetIntval() > (int)(max_cnt*div_number2) ){
        cell_list[cellid - 1].count_1 = 2;
    }    
    else{
        cell_list[cellid - 1].count_1 = 1;
    }
    
    cell_list[cellid - 1].def = true; 
      
    tuple_cell->DeleteIfAllowed(); 
  }
  
  //create priority_queue 
  int count = 0;
  
  priority_queue<Section_Cell> cell_queue; //////////priority_queue
  
  for(int i = 0;i < cell_list.size();i++)
      if(cell_list[i].def){
//          cell_list[i].Print(); 
          count++;
          cell_queue.push(cell_list[i]);
      }    
//  cout<<"count "<<count<<endl; 


  vector<Section_Cell> cell_list3; //////////////////////array
  vector<Section_Cell> cell_list2; //////////////////////array 
  vector<Section_Cell> cell_list1; //////////////////////array 

  while(cell_queue.empty() == false){
    Section_Cell top = cell_queue.top();
    cell_queue.pop();
//    top.Print();
    
    if(top.count_1 == 3)
      cell_list3.push_back(top);
    if(top.count_1 == 2)
      cell_list2.push_back(top);
    if(top.count_1 == 1)
      cell_list1.push_back(top);
  }
  //3---13  2---49  1---204
//  cout<<cell_list3.size()<<" "
//        <<cell_list2.size()<<" "<<cell_list1.size()<<endl;

  int bus_no;
   bus_no = 3;
   
   BuildRoute(cell_list3, cell_list1, attr1, bus_no);
   bus_no = 2;
   
   BuildRoute(cell_list2, cell_list1, attr1, bus_no);
  
  bus_no = 1;
  int limit_no = 20; 
  
  BuildRoute_Limit(cell_list1, cell_list1, attr1, bus_no, limit_no);
  
}

/*
highest density area with low density area

*/
void BusRoute::BuildRoute(vector<Section_Cell>& from_cell_list,
                             vector<Section_Cell> to_cell_list,
                             int attr1, int bus_no)
{
  for(unsigned int i = 0;i < from_cell_list.size();i++){
    Section_Cell elem = from_cell_list[i];
    
    int temp_bus_no = bus_no;
    
    while(temp_bus_no > 0){
      float dist_val = 10000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell(from_cell_list[i],to_cell_list,dist_val); 
    
      if(end_cellid >= 0){
//        cout<<"start cell "<<from_cell_list[i].cell_id
//          <<" end cell "<<to_cell_list[end_cellid].cell_id<<endl; 
        
          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id); 
          
      }
    
      temp_bus_no--; 
    
    }

    ////use btree to get the sections that this cell interesects////////
/*    CcInt* search_cell_id = new CcInt(true, elem.cell_id);
    BTreeIterator* btree_iter = btree->ExactMatch(search_cell_id);
    vector<int> sec_id_list; 
    while(btree_iter->Next()){
        Tuple* tuple_cell = rel1->GetTuple(btree_iter->GetId(), false);
        CcInt* sec_id = (CcInt*)tuple_cell->GetAttribute(attr1);
        sec_id_list.push_back(sec_id->GetIntval());
        tuple_cell->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_cell_id;
    ////////////////////////////////////////////////////////////////////
    cout<<"cell_id "<<cell_list3[i].cell_id
        <<"sec no "<<sec_id_list.size()<<endl; 
        */
  }

}

/*
lown density area with low density area
with the limit number of bus routes 

*/
void BusRoute::BuildRoute_Limit(vector<Section_Cell>& from_cell_list,
                             vector<Section_Cell> to_cell_list,
                             int attr1, int bus_no, int limit_no)
{
  for(unsigned int i = 0;i < from_cell_list.size();i++){
    Section_Cell elem = from_cell_list[i];
    
    if(from_cell_list[i].def == false) continue; 
    
    int temp_bus_no = bus_no;
    
    while(temp_bus_no > 0){
      float dist_val = 15000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell(from_cell_list[i],to_cell_list,dist_val); 
    
      if(end_cellid >= 0){
//        cout<<"start cell "<<from_cell_list[i].cell_id
//          <<" end cell "<<to_cell_list[end_cellid].cell_id<<endl; 
        
          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id); 
     
          to_cell_list[i].def = false; 
          from_cell_list[end_cellid].def = false; 
          
      }
     
      temp_bus_no--; 
    
    }
    if(i > limit_no) break; 
    
  }

}

/*
use the first paramter as the start cell and find a cell from the
second parameter that the distance between them is larger than a value 

*/
int BusRoute::FindEndCell(Section_Cell& start_cell,
                          vector<Section_Cell>& cell_list, float dist_val)
{
  for(unsigned int i = 0;i < cell_list.size();i++){
    if(cell_list[i].def){
        if(start_cell.reg.Distance(cell_list[i].reg) > dist_val){
          cell_list[i].def = false;
          return i; 
        }
    }
  }
  cout<<"do not find an end cell for cell "<<start_cell.cell_id<<endl; 
  return -1; 
}


/*
create bus routes, use the relation of a pair of cells 
1.given a pair of cells where the first is the start location and the second is
the end location of a bus route
2.for each cell, it (randomly) selects a point as the start (end) point.
3.use the shortest path connecting the two points as the bus route

*/

void BusRoute::CreateRoute2(int attr,int attr1,int attr2)
{
//  cout<<"attr "<<attr<<" attr1 "<<attr1 <<" attr2 "<<attr2<<endl; 
//  cout<<"CreateRoute2()"<<endl; 

  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple_cell_pair = rel2->GetTuple(i, false);
    int from_cell_id = 
        ((CcInt*)tuple_cell_pair->GetAttribute(attr1))->GetIntval();
    int end_cell_id =
        ((CcInt*)tuple_cell_pair->GetAttribute(attr2))->GetIntval();
    
//    cout<<"from_cell "<<from_cell_id<<" end_cell "<<end_cell_id<<endl; 
    ConnectCell(attr,from_cell_id,end_cell_id);
    tuple_cell_pair->DeleteIfAllowed(); 
  
  }
}

/*
randomly choose two positions in two cells and find the shortest path
connecting them. use the shortest path as the bus route 

*/
void BusRoute::ConnectCell(int attr,int from_cell_id,int end_cell_id)
{
     
    ////use btree to get the sections that this cell interesects////////
    CcInt* search_cell_id_from = new CcInt(true, from_cell_id);
    BTreeIterator* btree_iter1 = btree->ExactMatch(search_cell_id_from);
    vector<int> sec_id_list_from; 
    while(btree_iter1->Next()){
        Tuple* tuple_cell = rel1->GetTuple(btree_iter1->GetId(), false);
        CcInt* sec_id = (CcInt*)tuple_cell->GetAttribute(attr);
        sec_id_list_from.push_back(sec_id->GetIntval());
//        cout<<"sec id1 "<<sec_id->GetIntval()<<endl;
        tuple_cell->DeleteIfAllowed();
    }
    delete btree_iter1;
    delete search_cell_id_from;
/*    cout<<"from_cell_id "<<from_cell_id
        <<" sec no "<<sec_id_list_from.size()<<" "; */
        
    ////////////////////////////////////////////////////////////////////////
    CcInt* search_cell_id_end = new CcInt(true, end_cell_id);
    BTreeIterator* btree_iter2 = btree->ExactMatch(search_cell_id_end);
    vector<int> sec_id_list_end; 
    while(btree_iter2->Next()){
        Tuple* tuple_cell = rel1->GetTuple(btree_iter2->GetId(), false);
        CcInt* sec_id = (CcInt*)tuple_cell->GetAttribute(attr);
        sec_id_list_end.push_back(sec_id->GetIntval());
//        cout<<"sec id2 "<<sec_id->GetIntval()<<endl;
        tuple_cell->DeleteIfAllowed();
    }
    delete btree_iter2;
    delete search_cell_id_end;
/*    cout<<"end_cell_id "<<end_cell_id
        <<" sec no "<<sec_id_list_end.size()<<endl; */
    ///////// create two gpoints from selected two sections///////////////
    /////////choose the first road section for each cell/////////////////
    int index1 = 0;
    int index2 = 0; 
    ////////////////////create first gpoint////////////////////////
    int sec_id_1 = sec_id_list_from[index1];
    
//    cout<<"sec_id_1 "<<sec_id_1<<" "; 

    Tuple* tuple_sec_1 = n->GetSection(sec_id_1);  
    int rid1 = ((CcInt*)tuple_sec_1->GetAttribute(SECTION_RID))->GetIntval();
//    cout<<"section from "<<*tuple_sec_1<<endl; 
//    cout<<"rid1 "<<rid1<<endl;
    double loc1 = 
      ((CcReal*)tuple_sec_1->GetAttribute(SECTION_MEAS1))->GetRealval();

    GPoint* gp1 = new GPoint(true,n->GetId(),rid1,loc1,None);
    Point* location1 = new Point();
    gp1->ToPoint(location1);
    start_gp.push_back(*location1); 
    
    
    ///////////////////get the selected road section////////////////////
/*  SimpleLine* sl1 = (SimpleLine*)tuple_sec_1->GetAttribute(SECTION_CURVE);
    Line* busline1 = new Line(0);
    sl1->toLine(*busline1);
    bus_sections1.push_back(*busline1);
    delete busline1; */
    ///////////////////create second gpoint/////////////////////////////
    int sec_id_2 = sec_id_list_end[index2];
    
//    cout<<"sec_id_2 "<<sec_id_2<<endl; 
    
    Tuple* tuple_sec_2 = n->GetSection(sec_id_2);
    int rid2 = ((CcInt*)tuple_sec_2->GetAttribute(SECTION_RID))->GetIntval();
    double loc2 = 
        ((CcReal*)tuple_sec_2->GetAttribute(SECTION_MEAS1))->GetRealval();
    GPoint* gp2 = new GPoint(true,n->GetId(),rid2,loc2,None); 
    Point* location2 = new Point();
    gp2->ToPoint(location2); 
    end_gp.push_back(*location2); 
    
//    cout<<"section end "<<*tuple_sec_2<<endl; 

    ////////////////////////get the selected road section////////////////////
/*    SimpleLine* sl2 = (SimpleLine*)tuple_sec_2->GetAttribute(SECTION_CURVE);
    Line* busline2 = new Line(0);
    sl2->toLine(*busline2);
    bus_sections2.push_back(*busline2);
    delete busline2; */
    //////////////////////////////////////////////////////////////////////////
    //    cout<<"rid2 "<<rid2<<endl; 

    GLine* gl = new GLine(0);
    gp1->ShortestPath(gp2, gl);
    
    delete location1; 
    delete gp1;
    
    delete location2; 
    delete gp2; 
    
    tuple_sec_1->DeleteIfAllowed();
    tuple_sec_2->DeleteIfAllowed(); 
    //////////////////////////////////////////////////////////////////////////
    bus_lines.push_back(*gl);
    delete gl; 
    
}
