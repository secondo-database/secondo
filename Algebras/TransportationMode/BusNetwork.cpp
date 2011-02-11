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

it creates three kinds of routes determined by cells 
high density - low density;
middle denstiy - low density ; 
low density - lowdenstiy; 


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
  
  for(unsigned int i = 0;i < cell_list.size();i++)
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
  unsigned int limit_no = 20; 
  
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
//      float dist_val = 10000.0;//Euclidean distance between two cells
      
//      float dist_val = 12000.0;//Euclidean distance between two cells
            
      float dist_val = 16000.0;//Euclidean distance between two cells
      
      int end_cellid = FindEndCell(from_cell_list[i],to_cell_list,dist_val); 
    
      if(end_cellid >= 0){
//        cout<<"start cell "<<from_cell_list[i].cell_id
//          <<" end cell "<<to_cell_list[end_cellid].cell_id<<endl; 
        
          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id);
          
          if(bus_no == 3)bus_route_type.push_back(1);//the first route
          if(bus_no == 2)bus_route_type.push_back(2);//the second route
          
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
                             int attr1, int bus_no, unsigned int limit_no)
{
  for(unsigned int i = 0;i < from_cell_list.size();i++){
    Section_Cell elem = from_cell_list[i];
    
    if(from_cell_list[i].def == false) continue; 
    
    int temp_bus_no = bus_no;
    
    while(temp_bus_no > 0){
//      float dist_val = 15000.0;//Euclidean distance between two cells
      float dist_val = 25000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell(from_cell_list[i],to_cell_list,dist_val); 
    
      if(end_cellid >= 0){
//        cout<<"start cell "<<from_cell_list[i].cell_id
//          <<" end cell "<<to_cell_list[end_cellid].cell_id<<endl; 
        
          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id); 
     
          bus_route_type.push_back(3);//the lowest route 
          
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
//  cout<<"do not find an end cell for cell "<<start_cell.cell_id<<endl; 
  return -1; 
}


/*
create bus routes, use the relation of a pair of cells 
1.given a pair of cells where the first is the start location and the second is
the end location of a bus route
2.for each cell, it (randomly) selects a point as the start (end) point.
3.use the shortest path connecting the two points as the bus route

*/

void BusRoute::CreateRoute2(int attr,int attr1,int attr2,int attr3)
{
//  cout<<"attr "<<attr<<" attr1 "<<attr1 <<" attr2 "<<attr2<<endl; 
//  cout<<"CreateRoute2()"<<endl; 

  for(int i = 1;i <= rel2->GetNoTuples();i++){
    
//    if(i != 6 )continue; 
    
    Tuple* tuple_cell_pair = rel2->GetTuple(i, false);
    int from_cell_id = 
        ((CcInt*)tuple_cell_pair->GetAttribute(attr1))->GetIntval();
    int end_cell_id =
        ((CcInt*)tuple_cell_pair->GetAttribute(attr2))->GetIntval();
        
    int route_type =
        ((CcInt*)tuple_cell_pair->GetAttribute(attr3))->GetIntval();
//    cout<<"from_cell "<<from_cell_id<<" end_cell "<<end_cell_id<<endl; 

//    cout<<"route_type "<<route_type<<endl; 
    
    ConnectCell(attr,from_cell_id,end_cell_id,route_type, i);
    tuple_cell_pair->DeleteIfAllowed(); 
  
  }
  ///////////////////add several special routes//////////////////////////
  /////////maybe not the shortest path, cycle route//////////////////////
  ////////////////////////////////////////////////////////////////////////
  
  
}

/*
randomly choose two positions in two cells and find the shortest path
connecting them. use the shortest path as the bus route 

*/
void BusRoute::ConnectCell(int attr,int from_cell_id,
                           int end_cell_id, int route_type, int seed)
{
     
    ////use btree to get the sections that this cell interesects////////
    CcInt* search_cell_id_from = new CcInt(true, from_cell_id);
    BTreeIterator* btree_iter1 = btree->ExactMatch(search_cell_id_from);
    vector<int> sec_id_list_from; 
    while(btree_iter1->Next()){
        Tuple* tuple_cell = rel1->GetTuple(btree_iter1->GetId(), false);
        CcInt* sec_id = (CcInt*)tuple_cell->GetAttribute(attr);
        sec_id_list_from.push_back(sec_id->GetIntval());
        tuple_cell->DeleteIfAllowed();
    }
    delete btree_iter1;
    delete search_cell_id_from;
/*    cout<<"from_cell_id "<<from_cell_id
        <<" sec no "<<sec_id_list_from.size()<<" "; */
    
    ////////////////////create first gpoint////////////////////////
    int index1 = 0;
    index1 = (index1 + seed) % sec_id_list_from.size(); 
    int sec_id_1 = sec_id_list_from[index1];
        

    ///////// create two gpoints from selected two sections///////////////
    /////////choose the first road section for each cell/////////////////
    
    Tuple* tuple_sec_1 = n->GetSection(sec_id_1);  
    int rid1 = ((CcInt*)tuple_sec_1->GetAttribute(SECTION_RID))->GetIntval();
    double loc1 = 
      ((CcReal*)tuple_sec_1->GetAttribute(SECTION_MEAS1))->GetRealval();

    GPoint* gp1 = new GPoint(true,n->GetId(),rid1,loc1,None);
    Point* location1 = new Point();
    gp1->ToPoint(location1);
    start_gp.push_back(*location1); 
    

    ////////////////////////////////////////////////////////////////////////
    CcInt* search_cell_id_end = new CcInt(true, end_cell_id);
    BTreeIterator* btree_iter2 = btree->ExactMatch(search_cell_id_end);
    vector<int> sec_id_list_end; 
    while(btree_iter2->Next()){
        Tuple* tuple_cell = rel1->GetTuple(btree_iter2->GetId(), false);
        CcInt* sec_id = (CcInt*)tuple_cell->GetAttribute(attr);
        sec_id_list_end.push_back(sec_id->GetIntval());
        tuple_cell->DeleteIfAllowed();
    }
    delete btree_iter2;
    delete search_cell_id_end;
/*    cout<<"end_cell_id "<<end_cell_id
        <<" sec no "<<sec_id_list_end.size()<<endl; */
    
    ///////////////////get the selected road section////////////////////
/*  SimpleLine* sl1 = (SimpleLine*)tuple_sec_1->GetAttribute(SECTION_CURVE);
    Line* busline1 = new Line(0);
    sl1->toLine(*busline1);
    bus_sections1.push_back(*busline1);
    delete busline1; */
    
    ///////////////////create second gpoint/////////////////////////////
    int index2 = 0; 
    index2 = (index2 + seed) % sec_id_list_end.size(); 
    int sec_id_2 = sec_id_list_end[index2];
    
    Tuple* tuple_sec_2 = n->GetSection(sec_id_2);
    int rid2 = ((CcInt*)tuple_sec_2->GetAttribute(SECTION_RID))->GetIntval();
    double loc2 = 
        ((CcReal*)tuple_sec_2->GetAttribute(SECTION_MEAS1))->GetRealval();
    GPoint* gp2 = new GPoint(true,n->GetId(),rid2,loc2,None); 
    Point* location2 = new Point();
    gp2->ToPoint(location2); 
    end_gp.push_back(*location2); 
    

    ////////////////////////get the selected road section////////////////////
/*    SimpleLine* sl2 = (SimpleLine*)tuple_sec_2->GetAttribute(SECTION_CURVE);
    Line* busline2 = new Line(0);
    sl2->toLine(*busline2);
    bus_sections2.push_back(*busline2);
    delete busline2; */
    //////////////////////////////////////////////////////////////////////////
    //    cout<<"rid2 "<<rid2<<endl; 

    GLine* gl = new GLine(0);
    
//    cout<<"gp1 "<<*gp1<<" gp2 "<<*gp2<<endl; 
    
    gp1->ShortestPath(gp2, gl);
    
    delete location1; 
    delete gp1;
    
    delete location2; 
    delete gp2; 
    
    tuple_sec_1->DeleteIfAllowed();
    tuple_sec_2->DeleteIfAllowed(); 
    //////////////////////////////////////////////////////////////////////////
    GLine* newgl = new GLine(0);
    ConvertGLine(gl, newgl);
    
    bus_lines1.push_back(*newgl);
    /////////////////////////////////////////////////////////////////////////
//    bus_lines1.push_back(*gl);
    
    delete newgl; 
    //////////////////////////////////////////////////////////////////////////
    Line* l = new Line(0);
    gl->Gline2line(l);
    bus_lines2.push_back(*l);
    delete l; 
    delete gl; 
    
    bus_route_type.push_back(route_type);
}

/*
reorder the route interval in each gline 
the end point of i interval is connected to the start point of (i+1) interval 

Note: 2010.11.8. Currently, it only works with the old version of computing
intersectiong points of halfsegments in Spatial Algebra. The new version of
computing has some problems in Network Algebra. Because the rounding error of
computing junction points, it can't get the correct adjacent section list. 

*/
void BusRoute::ConvertGLine(GLine* gl, GLine* newgl)
{
   newgl->SetNetworkId(gl->GetNetworkId());
   
   
   vector<RouteInterval> ri_list; 
   for(int i = 0; i < gl->Size();i++){
    RouteInterval* ri = new RouteInterval();
    gl->Get(i, *ri); 
    
//    cout<<"rid "<<ri->GetRouteId()
//        <<"start "<<ri->GetStartPos()
//        <<"end "<<ri->GetEndPos()<<endl;
     
    /////////////////merge connected intervals///////////////////////////// 
    /////////////////but are properply not ordered correctly//////////////
    if(!AlmostEqual(ri->GetStartPos(), ri->GetEndPos())){
      if(ri_list.size() == 0)ri_list.push_back(*ri); 
      else{
        int last_rid = ri_list[ri_list.size() - 1].GetRouteId();
        if(last_rid == ri->GetRouteId()){
            double last_start = ri_list[ri_list.size() - 1].GetStartPos();
            double last_end = ri_list[ri_list.size() - 1].GetEndPos();
            
            double cur_start = ri->GetStartPos();
            double cur_end = ri->GetEndPos(); 
            
            if(AlmostEqual(last_start, cur_start)){
              ri_list[ri_list.size() - 1].SetStartPos(cur_end);
            }else if(AlmostEqual(last_start, cur_end)){
              ri_list[ri_list.size() - 1].SetStartPos(cur_start);
            }else if(AlmostEqual(last_end, cur_start)){
              ri_list[ri_list.size() - 1].SetEndPos(cur_end);
            }else if(AlmostEqual(last_end, cur_end)){ 
              ri_list[ri_list.size() - 1].SetEndPos(cur_start);
            }else assert(false);   
        }else
          ri_list.push_back(*ri);
      }
       
    }  
    delete ri; 
   }
   
   assert(ri_list.size() >= 2); //for bus route, long section 
   
   vector<GP_Point> gp_p_list; 

 //  cout<<"after processing "<<endl; 
   
   for(unsigned int i = 0;i < ri_list.size();i++){     
     
        int rid = ri_list[i].GetRouteId();
        double start = ri_list[i].GetStartPos();
        double end = ri_list[i].GetEndPos();
        GPoint* gp1 = new GPoint(true,n->GetId(),rid, start,None);
        Point* p1 = new Point();
        gp1->ToPoint(p1);
        
        GPoint* gp2 = new GPoint(true,n->GetId(),rid, end, None);
        Point* p2 = new Point();
        gp2->ToPoint(p2);
        
        GP_Point* gp_p = new GP_Point(rid,start,end,*p1,*p2);        
        gp_p_list.push_back(*gp_p);
        
        
        delete gp_p;
        delete p2;
        delete gp2; 
        delete p1;
        delete gp1;
   }  
   
   
   vector<bool> temp_start_from; 
   const double dist_delta = 0.001; 
   for(unsigned int i = 0; i < gp_p_list.size() - 1;i++){
    GP_Point gp_p1 = gp_p_list[i];
    GP_Point gp_p2 = gp_p_list[i + 1];
    
//    cout<<"gp1 loc1 "<<gp_p1.loc1<<" gp1 loc2 "<<gp_p1.loc2
//        <<"gp2 loc1 "<<gp_p2.loc1<<" gp2 loc2 "<<gp_p2.loc2<<endl; 
    
    if(gp_p1.loc1.Distance(gp_p2.loc1) < dist_delta || 
       gp_p1.loc1.Distance(gp_p2.loc2) < dist_delta){
      if(gp_p1.pos1 < gp_p1.pos2)
        temp_start_from.push_back(false);
      else
        temp_start_from.push_back(true); 
    
      
    }else if(gp_p1.loc2.Distance(gp_p2.loc1) < dist_delta ||
             gp_p1.loc2.Distance(gp_p2.loc2) < dist_delta){
     if(gp_p1.pos2 < gp_p1.pos1)
        temp_start_from.push_back(false);
      else
        temp_start_from.push_back(true); 
    
    }else assert(false);
    
   }
   
    GP_Point gp_p1 = gp_p_list[gp_p_list.size() - 1];
    GP_Point gp_p2 = gp_p_list[gp_p_list.size() - 2];
    if(gp_p1.loc1.Distance(gp_p2.loc1) < dist_delta || 
       gp_p1.loc1.Distance(gp_p2.loc2) < dist_delta){
      if(gp_p1.pos1 < gp_p1.pos2)
        temp_start_from.push_back(true);
      else
        temp_start_from.push_back(false); 
    
      
    }else if(gp_p1.loc2.Distance(gp_p2.loc1) < dist_delta ||
             gp_p1.loc2.Distance(gp_p2.loc2) < dist_delta){
     if(gp_p1.pos2 < gp_p1.pos1)
        temp_start_from.push_back(true);
      else
        temp_start_from.push_back(false); 
    
    }else assert(false);
    
    assert(temp_start_from.size() == ri_list.size());
    
/*    for(unsigned int i = 0;i < ri_list.size();i++){
    
        cout<<"rid "<<ri_list[i].GetRouteId()
        <<" start "<<ri_list[i].GetStartPos()
        <<" end "<<ri_list[i].GetEndPos()
        <<" start_from "<<temp_start_from[i]<<endl;
    }*/

    for(unsigned int i = 0;i < ri_list.size();i++){
      int rid = ri_list[i].GetRouteId();
      double start = ri_list[i].GetStartPos();
      double end = ri_list[i].GetEndPos();
      if(temp_start_from[i]){
        if(start < end)
          newgl->AddRouteInterval(rid, start, end);
        else
          newgl->AddRouteInterval(rid, end, start);
      
      }else{
        if(start < end)
          newgl->AddRouteInterval(rid, end, start);
        else
          newgl->AddRouteInterval(rid, start, end);
      
      }
    }

    newgl->SetDefined(true);
    newgl->SetSorted(false);
    newgl->TrimToSize();
    
//    cout<<*newgl<<endl; 
}

/*
Refine bus routes: filter some routes which are very similar to each other 
map each bus route to the road line, gline (rid,pos1)-(rid,pos2)
(rel (tuple ((br id int) (bus route1 gline) (bus route2 line) 
(start loc point) (end loc point) (route type int))))

*/
void BusRoute::RefineBusRoute(int attr1, int attr2, int attr3, int attr4,
                              int attr5, int attr6)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3
//      <<" attr4 "<<attr4<<" "<<attr5<<" attr6 " <<attr6<<endl; 

  vector<bool> routes_def;
  for(int i = 0;i < rel1->GetNoTuples();i++)
    routes_def.push_back(true);
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_route1 = rel1->GetTuple(i, false);
    int br_id1 = ((CcInt*)tuple_bus_route1->GetAttribute(attr1))->GetIntval();
    
//    if(br_id1 != 52){
//        tuple_bus_route1->DeleteIfAllowed();
//        continue; 
//    }
    
    
    if(routes_def[br_id1 - 1] == false){
        tuple_bus_route1->DeleteIfAllowed();
        continue; 
    }
    GLine* gl1 = (GLine*)tuple_bus_route1->GetAttribute(attr2);
    
    for(int j = i + 1; j <= rel1->GetNoTuples();j++){
      Tuple* tuple_bus_route2 = rel1->GetTuple(j, false);
      int br_id2 = ((CcInt*)tuple_bus_route2->GetAttribute(attr1))->GetIntval();
      
//      if(br_id2 != 148){
//        tuple_bus_route2->DeleteIfAllowed();
//        continue; 
//      }
    
      if(routes_def[br_id2 - 1] == false){
        tuple_bus_route2->DeleteIfAllowed();
        continue; 
      }
     GLine* gl2 = (GLine*)tuple_bus_route2->GetAttribute(attr2);
     
     int filter_br_id = FilterBusRoute(gl1,gl2,br_id1,br_id2);
     if(filter_br_id > 0){
//        cout<<" br_1 "<<br_id1<<" br_2 "<<br_id2
//            <<" filter "<<filter_br_id<<endl;  
        routes_def[filter_br_id - 1] = false;
        if(filter_br_id == br_id1){
          tuple_bus_route2->DeleteIfAllowed();
          break; 
        }  
     }

     tuple_bus_route2->DeleteIfAllowed();
     
    }  
    tuple_bus_route1->DeleteIfAllowed();
  }
  /////////////////////collect the non-filtered bus routes/////////////////
/* unsigned int final_no = 0;
   for(unsigned int i = 0;i < routes_def.size();i++){
    if(routes_def[i])final_no++;
   }
  cout<<"final bus routes no "<<final_no<<endl; */

  int new_br_id = 1; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
    if(routes_def[br_id - 1] == false){
        tuple_bus_route->DeleteIfAllowed();
        continue; 
    }
    
    GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr2);
    Line* l = (Line*)tuple_bus_route->GetAttribute(attr3);
    Point* start_p = (Point*)tuple_bus_route->GetAttribute(attr4);
    Point* end_p = (Point*)tuple_bus_route->GetAttribute(attr5);
    int route_type = 
        ((CcInt*)tuple_bus_route->GetAttribute(attr6))->GetIntval();
    
    br_id_list.push_back(new_br_id); 
    bus_lines1.push_back(*gl);
    bus_lines2.push_back(*l);
    start_gp.push_back(*start_p);
    end_gp.push_back(*end_p); 
    bus_route_type.push_back(route_type); 

    new_br_id++; 
    tuple_bus_route->DeleteIfAllowed(); 
  } 


}

/*
compare two bus routes represented by gline
it checkes whether one of them needs to be filtered 

*/
bool CompareRouteInterval(const RouteInterval& ri1, const RouteInterval& ri2)
{
   if(ri1.GetRouteId() < ri2.GetRouteId()) return true;
   else if(ri1.GetRouteId() > ri2.GetRouteId()) return false; 
   else{
    double start1 = ri1.GetStartPos();
    double end1 = ri1.GetEndPos(); 
    if(start1 > end1){
      double temp = start1;
      start1 = end1;
      end1 = temp;
    }
    double start2 = ri2.GetStartPos();
    double end2 = ri2.GetEndPos(); 
    if(start2 > end2){
      double temp = start2;
      start2 = end2;
      end2 = temp; 
    }
    if(AlmostEqual(start1, start2)){
      if(AlmostEqual(end1, end2))return true;
      else if(end1 < end2) return true;
      else return false; 
    }
    else if(start1 < start2 )return true; 
    else return false;
   }
}

int BusRoute::FilterBusRoute(GLine* gl1, GLine* gl2, int br_id1, int br_id2)
{
  //////////////////////convert GLine to RouteInterval////////////////////////
  vector<RouteInterval> ri_list1; 
  for(int i = 0; i < gl1->Size();i++){
    RouteInterval* ri = new RouteInterval();
    gl1->Get(i, *ri); 
    ri_list1.push_back(*ri);
  }


  sort(ri_list1.begin(), ri_list1.end(), CompareRouteInterval);
  double length1 = 0.0;
//  cout<<"GLine1"<<endl; 
  for(unsigned int i = 0;i < ri_list1.size();i++){
//    ri_list1[i].Print(cout);
    length1 += fabs(ri_list1[i].GetStartPos() - ri_list1[i].GetEndPos());
  }
//  cout<<"GLine1 br_id1 "<<br_id1<<" length1 "<<length1<<endl; 
  
  vector<RouteInterval> ri_list2; 
  for(int i = 0; i < gl2->Size();i++){
    RouteInterval* ri = new RouteInterval();
    gl2->Get(i, *ri); 
    ri_list2.push_back(*ri);
  }
  
  
  sort(ri_list2.begin(), ri_list2.end(), CompareRouteInterval);
  double length2 = 0.0; 
//  cout<<"GLine2"<<endl; 
  for(unsigned int i = 0;i < ri_list2.size();i++){
//    ri_list2[i].Print(cout);
    length2 += fabs(ri_list2[i].GetStartPos() - ri_list2[i].GetEndPos());
  }
  
//  cout<<"GLine2 br_id2 "<<br_id2<<" length2 "<<length2<<endl; 
  
  ////use a factor and choose the length of the shorter bus route/////////
  const double comm_factor = 0.8;// 0.9, 0.7 
 
  double comm_length;
  if(length1 < length2) comm_length = length1*comm_factor;
  else comm_length = length2*comm_factor; 

//  cout<<"comm length "<<comm_length<<endl; 

  ///////////////find the common length////////////////////////////
  unsigned int index2 = 0;
  double find_comm = 0.0;
  for(unsigned int i = 0;i < ri_list1.size();){
      int rid1 = ri_list1[i].GetRouteId();
      int rid2 = ri_list2[index2].GetRouteId();
//      cout<<"rid1 "<<rid1<<" rid2 "<<rid2<<endl; 
      if(rid1 < rid2){
          i++;
      }
      else if(rid1 == rid2){ //find the common route interval 
          double s1 = ri_list1[i].GetStartPos();
          double e1 = ri_list1[i].GetEndPos();
          if(s1 > e1){
              double temp = s1;
              s1 = e1;
              e1 = temp;
          }
          double s2 = ri_list2[index2].GetStartPos();
          double e2 = ri_list2[index2].GetEndPos();
          if(s2 > e2){
            double temp = s2;
            s2 = e2;
            e2 = temp; 
          }
//          cout<<"s1 "<<s1<<"s2 "<<s2<<endl; 
          //////////////////////////////////////////////////////////////
          if(AlmostEqual(s1,s2)){ //the same start position 
//              cout<<"AlmostEqual() "<<endl; 
              if(AlmostEqual(e1,e2)){
                find_comm += fabs(e1-s1);
              }else if(e1 > e2){
                find_comm += fabs(e2-s2);
              }else{//e1 < e2 
                find_comm += fabs(e1-s1);
              }
          }else if(s1 < s2){//check the position between e1, s2
//            cout<<"s1 < s2"<<endl; 
            if(e1 > s2 && e1 < e2){ // exists common part
              find_comm += fabs(e1-s2); 
            }
          }else{//s1 > s2 
//            cout<<"s1 > s2 "<<endl; 
            if(e2 > s1 && e2 < e1){ // exists common part
              find_comm += fabs(e2-s1); 
            }
          }
          if(find_comm > comm_length){
//             if(length1 < length2) return br_id1;//filter shorter one 
//               else return br_id2; 
              break; 
          }
          i++;
          if(index2 + 1 < ri_list2.size())
            index2++;
      }
      else if(rid1 > rid2){
          while((index2 + 1) < ri_list2.size() && rid1 > rid2){
            index2++; 
            rid2 = ri_list2[index2].GetRouteId();
          }  
          if(index2 + 1 == ri_list2.size())break; 
      }
      else assert(false); 
  //    cout<<"find comm "<<find_comm<<endl; 
  }

  if(find_comm > comm_length){
//      cout<<"br_id1 "<<br_id1<<" br_id2 "<<br_id2
//      <<" comm length "<<comm_length<<" real comm length "<<find_comm<<endl; 

      if(length1 < length2) return br_id1;//filter shorter one 
        else return br_id2; 
  }
  return 0; 
}

/*
calculate the total length of bus routes. of course it does not include 
redundant one. and get the percentage of bus routes in road network 

*/
float BusRoute::BusRouteInRoad(int attr1)
{
  vector<RouteInterval> ri_list;

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr1); 

    for(int j = 0; j < gl->Size();j++){
      RouteInterval* ri = new RouteInterval();
      gl->Get(j, *ri); 
      if(ri->GetStartPos() > ri->GetEndPos()){
          double temp = ri->GetStartPos();
          ri->SetStartPos(ri->GetEndPos());
          ri->SetEndPos(temp); 
      }
      ri_list.push_back(*ri);
    }
    tuple_bus_route->DeleteIfAllowed();
  }
  sort(ri_list.begin(), ri_list.end(), CompareRouteInterval);
  //////////////// merge intervals from the same route ////////////////////
  int last_rid = -1; 
  vector<RouteInterval> ri_list1; 
  vector<RouteInterval> ri_list2; 

  const double dist_delta = 0.01; 
  for(unsigned int i = 0;i < ri_list.size();i++){
//      ri_list[i].Print(cout); 

      if(ri_list[i].GetRouteId() != last_rid){
  
        last_rid = ri_list[i].GetRouteId();   
        for(unsigned int j = 0;j < ri_list1.size();j++){
          ri_list2.push_back(ri_list1[j]);
        }
        ri_list1.clear();
        ri_list1.push_back(ri_list[i]);
      }else{

           unsigned int index = ri_list1.size() - 1; 
            if(ri_list[i].GetStartPos() > ri_list1[index].GetEndPos()){
              ri_list1.push_back(ri_list[i]);
           }else if(ri_list1[index].GetStartPos() < ri_list[i].GetStartPos() &&
                   ri_list[i].GetStartPos() < ri_list1[index].GetEndPos() &&
                   ri_list[i].GetEndPos() > ri_list1[index].GetEndPos()){
              ri_list1[index].SetEndPos(ri_list[i].GetEndPos());
           }else if((fabs(ri_list[i].GetStartPos() - 
                         ri_list1[index].GetStartPos()) < dist_delta ||
                    fabs(ri_list[i].GetStartPos() - 
                         ri_list1[index].GetEndPos()) < dist_delta)  &&
                  ri_list[i].GetEndPos() > ri_list1[index].GetEndPos()){
              ri_list1[index].SetEndPos(ri_list[i].GetEndPos());
           }
      }
  }
  //////////////////////////////////////////////////////////////////////////
  double length1 = 0.0;
  for(unsigned int i = 0;i < ri_list2.size();i++){
//    ri_list2[i].Print(cout); 
    length1 += fabs(ri_list2[i].GetEndPos() - ri_list2[i].GetStartPos());
  }

  for(unsigned int i = 0;i < ri_list1.size();i++){
//    ri_list1[i].Print(cout); 
    length1 += fabs(ri_list1[i].GetEndPos() - ri_list1[i].GetStartPos());
  }

  double length2 = 0.0; 
  Relation* roads_rel = n->GetRoutes();
  for(int i = 1;i <= roads_rel->GetNoTuples();i++){
    Tuple* road_tuple = roads_rel->GetTuple(i, false);
    SimpleLine* curve = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);
    length2 += curve->Length();
    road_tuple->DeleteIfAllowed();
  }
  length1 = length1/1000.0;
  length2 = length2/1000.0;
  
  
  cout.setf(ios::fixed);
  cout<<setprecision(4)
  <<"bus route length "<<length1<<"km road length "<<length2<<"km"<<endl; 
  return length1/length2; 
}


/*
translate a bus route into two routes, down and up

*/

void BusRoute::CreateRoute3(int attr1, int attr2, int attr3, int w)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2
//      <<" attr3 "<<attr3<<" width "<<w<<endl; 
      
  int bus_route_uid = 1; 
  //////////////   relation  ///////////////////////////
  /////////////  bus route uid = br id * 2 -1///////////
  ////////////   bus route uid = br id * 2 /////////////
  //////////////////////////////////////////////////////
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    
//      if(i != 1) continue; 
      
      Tuple* tuple_bus_route = rel1->GetTuple(i, false);
      int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
      Line* l = (Line*)tuple_bus_route->GetAttribute(attr2);
      int route_type = 
          ((CcInt*)tuple_bus_route->GetAttribute(attr3))->GetIntval();
  
      ////////////////////////translate/////////////////////////////////    
      SimpleLine* sl = new SimpleLine(0);
      sl->fromLine(*l);
      
      SpacePartition* sp = new SpacePartition();
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
      sp->ReorderLine(sl, seq_halfseg);
      
  
//      cout<<"l1 "<<l->Length()<<" l2 "<<sl->Length()<<endl; 
      
      vector<Point> outer_l;
      vector<Point> outer_r;
      sp->ExtendSeg3(seq_halfseg, w, true, outer_l);
      sp->ExtendSeg3(seq_halfseg, w, false, outer_r);
      /////////////////////////////////////////////////////////////
      Line* l1 = new Line(0);
      l1->StartBulkLoad();
      ComputeLine(outer_l, l1);
      l1->EndBulkLoad();
      bus_sections1.push_back(*l1);
      
   
      br_id_list.push_back(br_id);
      bus_lines2.push_back(*l);
      bus_route_type.push_back(route_type);
      br_uid_list.push_back(bus_route_uid);
      bus_route_uid++;
      
      Line* l2 = new Line(0);
      l2->StartBulkLoad();
      ComputeLine(outer_r, l2);
      l2->EndBulkLoad();
      bus_sections1.push_back(*l2); 
       
      
      br_id_list.push_back(br_id);
      bus_lines2.push_back(*l);
      bus_route_type.push_back(route_type);
      br_uid_list.push_back(bus_route_uid);
      bus_route_uid++;
      
      delete l1;
      delete l2;
      delete sp; 
      delete sl; 
      tuple_bus_route->DeleteIfAllowed();
      
      
  }
  
}


/*
it inputs a list of points and computes a line value

*/

void BusRoute::ComputeLine(vector<Point>& point_list, Line* l)
{
  assert(point_list.size() > 0);
  int edgeno = 0;
  for(unsigned int i = 0;i < point_list.size() - 1;i++){
    Point lp = point_list[i];
    Point rp = point_list[i + 1];
    HalfSegment* hs = new HalfSegment(true, lp, rp);
    hs->attr.edgeno = edgeno++;
    *l += *hs;
    hs->SetLeftDomPoint(!hs->IsLeftDomPoint());
    *l += *hs;
    delete hs;
  }
}

/*
for each bus route, it creates a sequence of points on it as bus stops 

*/
void BusRoute::CreateBusStop1(int attr1, int attr2, int attr3, int attr4)
{
    for(int i = 1;i <= rel1->GetNoTuples();i++){
            
      Tuple* tuple_bus_route = rel1->GetTuple(i, false);
      int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
      GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr2); 
      Line* l = (Line*)tuple_bus_route->GetAttribute(attr3);
      int route_type = 
          ((CcInt*)tuple_bus_route->GetAttribute(attr4))->GetIntval();
      
//      cout<<"br_id "<<br_id<<endl; 
//      cout<<"route_type "<<route_type<<endl; 
      
      CreateStops(br_id, gl,l, route_type); 
      tuple_bus_route->DeleteIfAllowed(); 
      
    }
}

/*
for such a gline, create a set of points on it 

*/

void BusRoute::CreateStops(int br_id, GLine* gl, Line* l, int route_type)
{
  vector<double> dist_for_stops1;//for type 1
  dist_for_stops1.push_back(1000.0);
  dist_for_stops1.push_back(900.0);
  dist_for_stops1.push_back(1100.0);
  dist_for_stops1.push_back(1050.0);
  dist_for_stops1.push_back(1500.0);
  dist_for_stops1.push_back(1300.0);
  dist_for_stops1.push_back(950.0);
  dist_for_stops1.push_back(1200.0);
  dist_for_stops1.push_back(1400.0);
  dist_for_stops1.push_back(1300.0);
  
  ////////////////////////////////////////////////////////////////////
  vector<double> dist_for_stops2;//for type 2
  dist_for_stops2.push_back(900.0);
  dist_for_stops2.push_back(1100.0);
  dist_for_stops2.push_back(1150.0); 
  dist_for_stops2.push_back(1250.0); 
  dist_for_stops2.push_back(1050.0); 
  dist_for_stops2.push_back(1000.0);
  dist_for_stops2.push_back(1200.0);
  ///////////////////////////////////////////////////////////////////
  vector<double> dist_for_stops3;//for type 3
  dist_for_stops3.push_back(900.0);
  dist_for_stops3.push_back(1100.0); 
  dist_for_stops3.push_back(950.0); 
  dist_for_stops3.push_back(1300.0); 
  dist_for_stops3.push_back(1000.0);  
  dist_for_stops3.push_back(1200.0); 
  dist_for_stops3.push_back(1150.0); 
  //////////////////////////////////////////////////////////////////////
  
 
////we have to know it goes from start to end or the other direction/////
  vector<bool> start_from; //or end_from 
  
  vector<SectTreeEntry> sec_list; 
  
  GetSectionList(gl, sec_list, start_from);
  
/*  for(unsigned int i = 0;i < sec_list.size();i++){
    SectTreeEntry nEntry = sec_list[i]; 
    cout<<"i "<<i<<" rid "<<nEntry.rid<<" secid "<<nEntry.secttid
        <<"start "<<nEntry.start<<" end "<<nEntry.end<<endl; 
  }*/
  
  if(sec_list.size() < 2) return; 
  
  const double dist_to_jun = 15.0; //minimum distance between bus stop and jun 
  /////////////////create the first bus stop///////////////////////////
  int stop_count = 0; 
  unsigned int last_sec_index = 0; 
  double last_sec_start;
  double last_sec_end; 
  double last_sec_gp_pos; 
  
    
  assert(start_from.size() == sec_list.size()); 
  
/*  for(int i = 0;i < sec_list.size();i++){
    cout<<"rid "<<sec_list[i].rid
        <<"secid "<<sec_list[i].secttid
        <<"start pos "<<sec_list[i].start
        <<"end pos "<<sec_list[i].end<<endl; 
    cout<<start_from[i]<<endl; 
  }*/
  
 
  //////////////////////////////////////////////////////////////////////////
  while(true){
    SectTreeEntry entry = sec_list[last_sec_index]; 
    last_sec_start = entry.start;
    last_sec_end = entry.end; 
    
    if(fabs(last_sec_start - last_sec_end) < 2*dist_to_jun){
      last_sec_index++;
      continue; 
    }
    

    if(start_from[last_sec_index]){ //start from small
      last_sec_gp_pos = last_sec_start + dist_to_jun;  
    }else{ //start from big 
      last_sec_gp_pos = last_sec_end - dist_to_jun; 
    }
      
      
      GPoint* gp = 
        new GPoint(true,n->GetId(),entry.rid,last_sec_gp_pos,None);
      bus_stop_loc_1.push_back(*gp);
      Point* p = new Point();
      gp->ToPoint(p);
      bus_stop_loc_2.push_back(*p);
      
//      cout<<"gp start "<<*gp<<endl; 
      
      delete p;
      delete gp;
      
      br_id_list.push_back(br_id);
      br_stop_id.push_back(stop_count + 1);

      stop_count++;
      break; 
  }
  //////////////create more bus stops////////////////////////////////////////
  while(true){
    double next_stop_dist;
    if(route_type == 1)
      next_stop_dist = dist_for_stops1[(stop_count-1)% dist_for_stops1.size()];
    else if(route_type == 2)
      next_stop_dist = dist_for_stops2[(stop_count-1)% dist_for_stops2.size()];
    else if(route_type == 3)
      next_stop_dist = dist_for_stops3[(stop_count-1)% dist_for_stops3.size()];
    else assert(false);
      
/*    cout<<"stop_id "<<stop_count
        <<" current dist stop "<<next_stop_dist<<endl; */
    //////////////////////////////////////////////////////////////////////
    if(FindNextStop(sec_list,last_sec_index,last_sec_start,
                 last_sec_end, last_sec_gp_pos,next_stop_dist,
                 dist_to_jun,start_from)){
    
      br_id_list.push_back(br_id);
      br_stop_id.push_back(stop_count + 1);
      stop_count++;
    }else
      break; 
    //////////////////////////////////////////////////////////////////////
  }
  
}

/*
create more bus stops along the given gline
use nextstopdist to control the distance between two consecutive bus stops  

*/
bool BusRoute::FindNextStop(vector<SectTreeEntry> sec_list,
                    unsigned int& last_sec_index,double& last_sec_start,
                    double& last_sec_end, double& last_sec_gp_pos,
                    double next_stop_dist, 
                    double dist_to_jun, vector<bool> start_from)
{
  
  if(start_from[last_sec_index]){ //from small to big 
    
//    cout<<"from small to big "<<endl; 
    
    int rid; 
    //still in the same section, very long road section 
    if(fabs(last_sec_gp_pos - last_sec_end) > (next_stop_dist + dist_to_jun)){
      last_sec_gp_pos += next_stop_dist;
      rid = sec_list[last_sec_index].rid; 
  
    }else{//move to the next road section 
      next_stop_dist -= fabs(last_sec_gp_pos - last_sec_end); 
      for(last_sec_index++;last_sec_index < sec_list.size();last_sec_index++){
      
      double cur_start = sec_list[last_sec_index].start;
      double cur_end = sec_list[last_sec_index].end; 
      //////////////////////////////////////////////////////
/*      cout<<"rid "<<sec_list[last_sec_index].rid
        <<" secid "<<sec_list[last_sec_index].secttid
        <<" start pos "<<sec_list[last_sec_index].start
        <<" end pos "<<sec_list[last_sec_index].end<<endl; */
        
      /////////////////////////////////////////////////////////
      if(fabs(cur_end - cur_start) < 2*dist_to_jun){
        next_stop_dist -= fabs(cur_end - cur_start);
        continue; 
      }
        
      if(next_stop_dist - fabs(cur_end - cur_start) < 0){
        
         if(start_from[last_sec_index]){ //from small to big 
          double temp_pos = cur_start + next_stop_dist;
          if(fabs(temp_pos - cur_start) < dist_to_jun)
              temp_pos = cur_start + dist_to_jun;
          if(fabs(cur_end - temp_pos) < dist_to_jun)
              temp_pos = cur_end - dist_to_jun;
        
          last_sec_gp_pos = temp_pos; 
        }else{ //from big to small 
          double temp_pos = cur_end - next_stop_dist;
          if(fabs(temp_pos - cur_end) < dist_to_jun)
              temp_pos = cur_end - dist_to_jun;
          if(fabs(temp_pos - cur_start) < dist_to_jun)
              temp_pos = cur_start + dist_to_jun;
        
          last_sec_gp_pos = temp_pos;   
        }
        
        last_sec_start = cur_start;
        last_sec_end = cur_end; 
        rid = sec_list[last_sec_index].rid;      
        break; 
      }else
        next_stop_dist -= fabs(cur_end - cur_start); 
    
    }
    if(last_sec_index == sec_list.size()) return false; 
  }
    
//   cout<<"last_sec_index "<<last_sec_index<<" rid " <<rid<<endl; 
    
   GPoint* gp = 
   new GPoint(true,n->GetId(),rid,last_sec_gp_pos,None);
   bus_stop_loc_1.push_back(*gp);
   Point* p = new Point();
   gp->ToPoint(p);
   bus_stop_loc_2.push_back(*p);   
   
//   cout<<"stop "<<*gp<<endl; 
   
   delete p;
   delete gp;
         
   return true; 
   
  }else{///from big to small 
  
//    cout<<"from big to small"<<endl;
    
    int rid; 
    //still in the same section, very long road section 
    if(fabs(last_sec_gp_pos - last_sec_start) > (next_stop_dist + dist_to_jun)){
      last_sec_gp_pos -= next_stop_dist;
      rid = sec_list[last_sec_index].rid; 
      
    }else{
    
     next_stop_dist -= fabs(last_sec_gp_pos - last_sec_start); 
      for(last_sec_index++;last_sec_index < sec_list.size();last_sec_index++){
      
      double cur_start = sec_list[last_sec_index].start;
      double cur_end = sec_list[last_sec_index].end; 
      ////////////////////////////////////////////////////////////////
/*      cout<<"cur_start "<<cur_start<<" cur_end "<<cur_end<<endl; 
      
      cout<<"rid "<<sec_list[last_sec_index].rid
        <<" secid "<<sec_list[last_sec_index].secttid
        <<" start pos "<<sec_list[last_sec_index].start
        <<" end pos "<<sec_list[last_sec_index].end<<endl; */

      ////////////////////////////////////////////////////////////////
      if(fabs(cur_end - cur_start) < 2*dist_to_jun){
        next_stop_dist -= fabs(cur_end - cur_start);
        continue; 
      }
        
      if(next_stop_dist - fabs(cur_end - cur_start) < 0){
        
         if(start_from[last_sec_index]){ //from small to big 
          double temp_pos = cur_start + next_stop_dist;
          if(fabs(temp_pos - cur_start) < dist_to_jun)
              temp_pos = cur_start + dist_to_jun;
          if(fabs(cur_end - temp_pos) < dist_to_jun)
              temp_pos = cur_end - dist_to_jun;
        
          last_sec_gp_pos = temp_pos; 
        }else{ //from big to small 
//          cout<<"next_stop_dist "<<next_stop_dist<<endl; 

          double temp_pos = cur_end - next_stop_dist;
          if(fabs(temp_pos - cur_end) < dist_to_jun)
              temp_pos = cur_end - dist_to_jun;
          if(fabs(temp_pos - cur_start) < dist_to_jun)
              temp_pos = cur_start + dist_to_jun;

          last_sec_gp_pos = temp_pos;   
        }
        
        last_sec_start = cur_start;
        last_sec_end = cur_end; 
        rid = sec_list[last_sec_index].rid;      
        break; 
      }else
        next_stop_dist -= fabs(cur_end - cur_start); 
    
    }
      if(last_sec_index == sec_list.size()) return false; 
    
    }
    
    GPoint* gp = 
    new GPoint(true,n->GetId(),rid,last_sec_gp_pos,None);
    bus_stop_loc_1.push_back(*gp);
    Point* p = new Point();
    gp->ToPoint(p);
    bus_stop_loc_2.push_back(*p);   
  
//    cout<<" stop "<<*gp<<endl; 
    
    delete p;
    delete gp;

    return true; 
  }
   
}

/*
merge bus stops 
for several bus stops on the same road section, it might be merge them into one
 the minimum length of road section is 0.055
 the maximum length of road section is 5046.577  

we merge bus stops that locate on the same road section 

*/


void BusRoute::CreateBusStop2(int attr1,int attr2,int attr3)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 
//  cout<<"CreateBusStop2()"<<endl; 
  
  vector<BusStop> bus_stop_list; 
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_stop = rel1->GetTuple(i, false);
        
    int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr1))->GetIntval();
    
/*    if(br_id != 108){ 
      tuple_bus_stop->DeleteIfAllowed();
      continue; 
    }*/
    
    int stop_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr2))->GetIntval();
    GPoint* gp = (GPoint*)tuple_bus_stop->GetAttribute(attr3); 

    int sid = n->GetSectionOnRoute(gp)->GetTupleId(); 
    
//    cout<<*tuple_bus_stop<<endl; 
//    cout<<*n->GetSectionOnRoute(gp)<<endl;
    
    BusStop* bs = new BusStop(br_id,stop_id,gp->GetRouteId(),
                              gp->GetPosition(), sid, true);
    bus_stop_list.push_back(*bs);
    delete bs; 

    tuple_bus_stop->DeleteIfAllowed();
  }
  sort(bus_stop_list.begin(),bus_stop_list.end());
  
/*  for(int i = 0;i < bus_stop_list.size();i++){
      bus_stop_list[i].Print(); 
  }*/
  ////////////////take all stops in one route////////////////////////
  int count = 0;
  for(unsigned int i = 0;i < bus_stop_list.size();i++){
    unsigned int sid = bus_stop_list[i].sid; 
    
    vector<BusStop> temp_list; 
    temp_list.push_back(bus_stop_list[i]); 
    
    while((i + 1)< bus_stop_list.size() && 
          bus_stop_list[i + 1].sid == sid){
        i++;
        temp_list.push_back(bus_stop_list[i]); 
    }    
    
    MergeBusStop1(temp_list); 
    
    count += temp_list.size(); 
  }

//  cout<<bus_stop_list.size()<<" count "<<count<<endl; 

}

/*
take all bus stops in the same section and merge some of them if they are close
to each other. there is one condition:
after merging, all bus stops should still locate on their original bus routes.
they can't move to another different route

*/
void BusRoute::MergeBusStop1(vector<BusStop>& stop_list)
{
//  cout<<"stop_list size "<<stop_list.size()<<endl; 
//  cout<<stop_list.size()<<endl; 
  if(stop_list.size() == 1){
    br_id_list.push_back(stop_list[0].br_id);
    br_stop_id.push_back(stop_list[0].br_stop_id); 
    GPoint* gp = new GPoint(true,n->GetId(),stop_list[0].rid,stop_list[0].pos);
    bus_stop_loc_1.push_back(*gp); 
  
    Point* p = new Point();
    gp->ToPoint(p);
    bus_stop_loc_2.push_back(*p);      
    delete p; 
    delete gp;   
    
//    point_id_list.push_back(point_id);
    sec_id_list.push_back(stop_list[0].sid);

  }else{//there are several bus stops on the same road section 
    
    //to avoid that two bus stops coming from the same bus route are mereged
    //into one position 

    const double dist_val = 800.0; //the minimum distance between stops
    const double dist_to_jun = 15.0;//minimum distance between bus stop and jun 
    
    int sec_id = stop_list[0].sid; 
    Tuple* tuple_sec = n->GetSection(sec_id);
    SimpleLine* curve = (SimpleLine*)tuple_sec->GetAttribute(SECTION_CURVE);
    
    if(curve->Length() < dist_val){//merge all stops in one position
      double final_pos = 0.0; 
      for(unsigned int i = 0;i < stop_list.size();i++){
//        cout<<"stop_list pos "<<stop_list[i].pos<<" "; 
        final_pos += stop_list[i].pos;
      }
      
      final_pos = final_pos / stop_list.size(); 
//      cout<<endl<<" final pos "<<final_pos<<endl; 
      
      int count1 = 0; 
      for(unsigned int i = 0;i < stop_list.size();i++){
      
        br_id_list.push_back(stop_list[i].br_id);
        br_stop_id.push_back(stop_list[i].br_stop_id); 
        GPoint* gp = new GPoint(true,n->GetId(),stop_list[i].rid,final_pos);
        bus_stop_loc_1.push_back(*gp); 
          
//        cout<<"gp "<<*gp<<endl 
        
        Point* p = new Point();
        gp->ToPoint(p);
        bus_stop_loc_2.push_back(*p);      
        delete p; 
        delete gp;   
        count1++;
        
        sec_id_list.push_back(stop_list[i].sid);
      }
      
//      cout<<"count1 "<<count1<<endl; 
      
    }else{//partition a road section into severl parts and there is a bus stop
          //for each part 
      
      double sec_pos1 = 
          ((CcReal*)tuple_sec->GetAttribute(SECTION_MEAS1))->GetRealval();
      double sec_pos2 =
          ((CcReal*)tuple_sec->GetAttribute(SECTION_MEAS2))->GetRealval();
      if(sec_pos1 > sec_pos2){
        double temp = sec_pos1;
        sec_pos1 = sec_pos2;
        sec_pos2 = temp; 
      }


//      int interval = curve->Length() / dist_val;
      int interval = (int) (curve->Length() / dist_val);

/*      cout<<"interval "<<interval
          <<"curve length "<<curve->Length()<<endl; */

      for(unsigned int i = 0;i < stop_list.size();i++){
        double stop_pos = stop_list[i].pos;
//        int interval_pos = stop_pos / dist_val;
//        int interval_pos = (stop_pos - sec_pos1)/ dist_val;
        int interval_pos = (int)((stop_pos - sec_pos1)/ dist_val);

/*        stop_list[i].Print();
        cout<<"stop_pos "<<stop_pos
            <<" interval_pos "<<interval_pos<<endl; */

        assert(interval_pos >= 0 && interval_pos <= interval);
        double new_pos = dist_val / 4 + interval_pos*dist_val; 
        if(new_pos > curve->Length()){
//          new_pos = interval_pos*dist_val; 
            new_pos = curve->Length() - dist_to_jun;
        }
        /////////////////////////////////
        new_pos += sec_pos1; 
        /////////////////////////////////
        
        br_id_list.push_back(stop_list[i].br_id);
        br_stop_id.push_back(stop_list[i].br_stop_id); 
        GPoint* gp = new GPoint(true,n->GetId(),stop_list[i].rid,new_pos);
        bus_stop_loc_1.push_back(*gp); 
  
        Point* p = new Point();
        gp->ToPoint(p);
        bus_stop_loc_2.push_back(*p);      
        delete p; 
        delete gp; 

        sec_id_list.push_back(stop_list[i].sid);
      }
      
    }
  
    tuple_sec->DeleteIfAllowed();
  }


}

/*
Merge bus stops locating on different road sections (adjacent)
use a distance threshold value to expand the bus stop (gpoint) to a gline value 

after changing the position of some bus stops, they should still locate on 
their original bus route

Note: each bus route covers a complete road section or none of it.
it can't only cover part of a road section 

It also calculates the startSmaller value for each bus route.

very important value startSmaller. It records whether the simpleline starts
from the small or big. The simpleline comes from coverting gline. 
gline records the shortest path. We have to know after converting where the
start point is. 

*/
void BusRoute::CreateBusStop3(int attr,int attr1,int attr2,int attr3)
{
//  cout<<"attr "<<attr<<endl;
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 

  /////////////////  collect all the bus stops ////////////////////////
  vector<BusStop> bus_stop_list;
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple_bus_stop = rel2->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr1))->GetIntval();
    int stop_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr2))->GetIntval();
    GPoint* gp = (GPoint*)tuple_bus_stop->GetAttribute(attr3);
           

    int sid = n->GetSectionOnRoute(gp)->GetTupleId(); 
    
    BusStop* bs = 
      new BusStop(br_id,stop_id,gp->GetRouteId(),gp->GetPosition(),sid,true);
    
    bus_stop_list.push_back(*bs);
    delete bs;
    tuple_bus_stop->DeleteIfAllowed();
  }

//  cout<<"bus_stop_list size "<<bus_stop_list.size()<<endl; 

  //////////////////////////////////////////////////////////////////////////
  const double dist_val = 500.0; //distance used to expand road section 
  int last_br_id = 0; 
  vector<SectTreeEntry> sec_list; 
  vector<bool> start_from; 
  for(unsigned int i = 0;i < bus_stop_list.size();i++){
      
//      if(bus_stop_list[i].br_id > 10) continue;

//      if(i != 500) continue; 
        
      if(bus_stop_list[i].def == false) continue; 
      
    ///////////////////////////////////////////////////////////////////////////
    //collect road sections that are with the distance distval to the bus stop/
    ///////////////////////////////////////////////////////////////////////////
      //if it is false, it means the section starts from the big value
      //instead of the small value 
      
      if(bus_stop_list[i].br_id != last_br_id){
        sec_list.clear();
        start_from.clear();
        Tuple* tuple_bus_route = rel1->GetTuple(bus_stop_list[i].br_id,false);
        GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr);
        GetSectionList(gl, sec_list, start_from);
        tuple_bus_route->DeleteIfAllowed();
        last_br_id = bus_stop_list[i].br_id; 
        
        ///////////////////////////////////////////////////////
        assert(sec_list.size() == start_from.size());
/*        for(unsigned int j = 0;j < sec_list.size();j++){
          cout<<" rid "<<sec_list[j].rid<<" secid "<<sec_list[j].secttid
          <<" start "<<sec_list[j].start<<" end "<<sec_list[j].end
          <<" start from small "<<start_from[j]<<endl;       
        }*/
        /////////////////////////////////////////////////////////
      }
      /////////find the position of this bus stop in section/////////////
      int sec_index = -1;
      for(unsigned int j = 0;j < sec_list.size();j++){
        if(sec_list[j].secttid == bus_stop_list[i].sid){
          sec_index = j;
          break; 
        } 
      }
      assert(sec_index != -1); 

      /////////////// find the up adjacent sections //////////////////////////
      vector<SectTreeEntry> sec_down;
      vector<SectTreeEntry> sec_up;
      //we only consider adjacent section.
      //the same section has been processed before 
      if(start_from[sec_index])
      {
        if(fabs(bus_stop_list[i].pos - sec_list[sec_index].start) < dist_val){
          FindDownSection(bus_stop_list[i].pos,sec_list,sec_index,
                        dist_val,sec_down,start_from);
        }
        /////////////// find the down adjacent sections ///////////////////////
        
        if(fabs(bus_stop_list[i].pos - sec_list[sec_index].end) < dist_val){
          FindUpSection(bus_stop_list[i].pos,sec_list,sec_index,
                        dist_val,sec_up,start_from);
        }
        
      }else{
      
        if(fabs(bus_stop_list[i].pos - sec_list[sec_index].end) < dist_val){
          FindDownSection(bus_stop_list[i].pos,sec_list,sec_index,
                        dist_val,sec_down,start_from);
        }
        /////////////// find the down adjacent sections ///////////////////////
        if(fabs(bus_stop_list[i].pos - sec_list[sec_index].start) < dist_val){
          FindUpSection(bus_stop_list[i].pos,sec_list,sec_index,
                        dist_val,sec_up,start_from);
        }      
      }
      //////////////////////////////////////////////////////////////////////
/*      cout<<"bus route down direction"<<endl;
      for(unsigned int j = 0;j < sec_down.size();j++){
          cout<<" rid "<<sec_down[j].rid<<" secid "<<sec_down[j].secttid
          <<" start "<<sec_down[j].start<<" end "<<sec_down[j].end<<endl;       
      }
      
      cout<<"bus route up direction"<<endl;
      for(unsigned int j = 0;j < sec_up.size();j++){
          cout<<" rid "<<sec_up[j].rid<<" secid "<<sec_up[j].secttid
          <<" start "<<sec_up[j].start<<" end "<<sec_up[j].end<<endl;       
      }*/
      /////////////////////////////////////////////////////////////////////
      ////////collect all bus stops on these sections/////////////////////
      ////////////////////////////////////////////////////////////////////
      MergeBusStop2(sec_down,sec_up,bus_stop_list,i,attr);
  }
  
  ///////////////////////////////////////////////////////////////////////
  ///////////////clear some redundant stops after merging////////////////
  ///////////////clear some stops are too close or equal to each other////
  ////////////////////////////////////////////////////////////////////////
   vector<BusStop> temp_bus_stop_list; 
   int last_br = 0;
   const double delta_dist = 50.0;
   for(unsigned int i = 0;i < bus_stop_list.size();i++){
      if(temp_bus_stop_list.size() == 0){
        temp_bus_stop_list.push_back(bus_stop_list[i]); 
        last_br = bus_stop_list[i].br_id; 
      }else{
        if(bus_stop_list[i].br_id != last_br){
            temp_bus_stop_list.push_back(bus_stop_list[i]); 
            last_br = bus_stop_list[i].br_id; 
        }else{
            BusStop tmp_bs = temp_bus_stop_list[temp_bus_stop_list.size() - 1]; 
            if(fabs(tmp_bs.pos - bus_stop_list[i].pos) > delta_dist){
              int sid1 = tmp_bs.br_stop_id;
              int sid2 = bus_stop_list[i].br_stop_id; 
              if(fabs(sid1 - sid2) > 1){
                if(sid1 < sid2)
                  bus_stop_list[i].br_stop_id = sid1 + 1;
                else if(sid1 > sid2)
                  bus_stop_list[i].br_stop_id = sid1 - 1;
                else assert(false);
              }
              temp_bus_stop_list.push_back(bus_stop_list[i]); 
              last_br = bus_stop_list[i].br_id; 
            }
        }
      }
   }
   bus_stop_list.clear();
   for(unsigned int i = 0;i < temp_bus_stop_list.size();i++)
     bus_stop_list.push_back(temp_bus_stop_list[i]); 
   /////////////////////////////////////////////////////////////////////
   ///////////////////////get the result///////////////////////////////
   for(unsigned int i = 0;i < bus_stop_list.size();i++){
      br_id_list.push_back(bus_stop_list[i].br_id);
      br_stop_id.push_back(bus_stop_list[i].br_stop_id);
      sec_id_list.push_back(bus_stop_list[i].sid); 
      int rid = bus_stop_list[i].rid;
      double pos = bus_stop_list[i].pos;
      GPoint* gp = new GPoint(true,n->GetId(),rid,pos,None);
      bus_stop_loc_1.push_back(*gp);

      Point* p = new Point();
      gp->ToPoint(p);
      bus_stop_loc_2.push_back(*p);
      delete p;
      delete gp;
   }
   ///////////////////////calculate the startsmaller value/////////////

    unsigned int index = 0;
    for(int i = 1;i <= rel1->GetNoTuples();i++){

    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr);
  
    vector<bool> start_from; //or end_from 
    vector<SectTreeEntry> sec_list; 
    
    GetSectionList(gl, sec_list, start_from); 
    /////collect bus stops from the same bus route /////
    int start = index; 
    while(bus_stop_list[index].br_id == i && index < bus_stop_list.size()){
      index++;
    }
    int end = index; 
//    cout<<"br_id "<<i<<" "; 
    vector<double> dist_list; 
    
    ////////////get the startsmaller value //////////////////////////////
    Line* l = new Line(0);
    gl->Gline2line(l);
    SimpleLine* sl = new SimpleLine(0);
    sl->fromLine(*l);
    
    
    CalculateStartSmaller(bus_stop_list,start,end,sec_list,
                             start_from, dist_list, sl);
    delete sl;
    delete l;
    
    /////////////////////////////////////////////////////////////////////
    
    tuple_bus_route->DeleteIfAllowed();
  }
  
  assert(bus_stop_list.size() == startSmaller.size()); 
}

/*
Merge bus stops that locate on adjacent road sections

*/
void BusRoute::MergeBusStop2(vector<SectTreeEntry> sec_down,
                             vector<SectTreeEntry> sec_up,
                             vector<BusStop>& bus_stop_list, 
                             int cur_index, int attr)
{
  vector<SectTreeEntry> sec_all;
  for(unsigned int i = 0;i < sec_down.size();i++)
    sec_all.push_back(sec_down[i]);
  
  for(unsigned int i = 0;i < sec_up.size();i++){
    if(sec_down.size() > 0 &&
      sec_up[i].secttid == sec_down[0].secttid)continue; //the same section 
      
    sec_all.push_back(sec_up[i]);
  }  

  if(sec_all.size() > 0){
      vector<int> bus_stop_id_list; 
      for(unsigned int i = 0;i < sec_all.size();i++){
          int sec_id = sec_all[i].secttid; 
          double start = sec_all[i].start;
          double end = sec_all[i].end; 
          /////////collect all bus stops on these sections/////////////
        
        CcInt* search_cell_id = new CcInt(true, sec_id);
        BTreeIterator* btree_iter = btree->ExactMatch(search_cell_id);
        while(btree_iter->Next()){
          
          int tuple_id = btree_iter->GetId();
          if(tuple_id - 1 != cur_index){//do not include itself 
            if(start <= bus_stop_list[tuple_id - 1].pos &&
              bus_stop_list[tuple_id - 1].pos <= end){//bus stop locate on it 
            

              bus_stop_id_list.push_back(tuple_id - 1);
            
/*              if(bus_stop_list[tuple_id - 1].rid != 
                 bus_stop_list[cur_index].rid && 
                 !AlmostEqual(bus_stop_list[tuple_id - 1].pos,
                              bus_stop_list[cur_index].pos)){
                cout<<"tuple_id "<<tuple_id<<endl; 
                bus_stop_list[cur_index].Print();
                bus_stop_list[tuple_id - 1].Print();

              }*/
              ///////////////////////////////////////////////////////
/*              Tuple* tuple_bus_stop = 
                    rel2->GetTuple(btree_iter->GetId(), false);
//              cout<<*tuple_bus_stop<<endl; 
              tuple_bus_stop->DeleteIfAllowed();*/
              ///////////////////////////////////////////////////////    
            }
          }
        }
        delete btree_iter;
        delete search_cell_id;

      }
      //check whether the route of the bus stop changed position /////////////
      /////////////////has the section id //////////
      //do not merge bus stops come from the same route /////////////
      vector<int> bus_stop_id_list_new;
      vector<int> br_id_array;
      br_id_array.push_back(bus_stop_list[cur_index].br_id); 
      
      for(unsigned int i = 0;i < bus_stop_id_list.size();i++){
        
        unsigned int j = 0;
        for(;j < br_id_array.size();j++){
          int index = bus_stop_id_list[i];
          BusStop bs = bus_stop_list[index]; 
          if(bs.br_id == br_id_array[j])
            break;
        }
        if(j == br_id_array.size()){
          bus_stop_id_list_new.push_back(bus_stop_id_list[i]);
          BusStop bs = bus_stop_list[bus_stop_id_list[i]];
          br_id_array.push_back(bs.br_id); 
        }
      
      }
      

      
//      for(unsigned int i = 0;i < bus_stop_id_list.size();i++){
      for(unsigned int i = 0;i < bus_stop_id_list_new.size();i++){
//          int index = bus_stop_id_list[i];
          int index = bus_stop_id_list_new[i]; 
          BusStop bs = bus_stop_list[index]; 
          int br_id = bs.br_id;
          Tuple* tuple_bus_route = rel1->GetTuple(br_id,false);
          GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr);
          
          vector<SectTreeEntry> sec_list; 
          vector<bool> start_from;
          GetSectionList(gl, sec_list, start_from);
          for(unsigned int j = 0;j < sec_list.size();j++){
            if(sec_list[j].secttid == bus_stop_list[cur_index].sid){
              //do not move bus stops that come from the same bus route 

              bus_stop_list[index].rid = bus_stop_list[cur_index].rid;
              bus_stop_list[index].pos = bus_stop_list[cur_index].pos;
              bus_stop_list[index].sid = bus_stop_list[cur_index].sid;
              
          
              bus_stop_list[cur_index].def = false;
              bus_stop_list[index].def = false; 
              break; 
            }    
          }
          tuple_bus_route->DeleteIfAllowed();
      }
  }
  
}

/*
find the section to down direction. bus route goes down 

*/

void BusRoute::FindDownSection(double cur_bus_stop_pos,
                     vector<SectTreeEntry> sec_list,int sec_index,
                     const double dist_val,
                     vector<SectTreeEntry>& sec_down,vector<bool> start_from)

{
  double temp_dist; 
  if(start_from[sec_index]){
    int secttid = sec_list[sec_index].secttid;
    int rid = sec_list[sec_index].rid;
    double start = sec_list[sec_index].start;
    double end = cur_bus_stop_pos; 
    SectTreeEntry* tmp_entry = 
        new SectTreeEntry(secttid,rid,start,end,true,true);
    sec_down.push_back(*tmp_entry);
    delete tmp_entry;
    temp_dist = dist_val - fabs(end - start);
  }else{
    int secttid = sec_list[sec_index].secttid;
    int rid = sec_list[sec_index].rid;
    double start = cur_bus_stop_pos; 
    double end = sec_list[sec_index].end;
    SectTreeEntry* tmp_entry = 
        new SectTreeEntry(secttid,rid,start,end,true,true);
    sec_down.push_back(*tmp_entry);
    delete tmp_entry; 
    temp_dist = dist_val - fabs(end - start);
  }

  int temp_index = sec_index;
  while(temp_dist > 0.0){
      temp_index--;
      if(temp_index >= 0){
            SectTreeEntry entry = sec_list[temp_index];

            if(temp_dist - fabs(entry.start - entry.end) > 0){
              sec_down.push_back(entry);
              temp_dist -= fabs(entry.start - entry.end);
            }else{
              if(start_from[temp_index] == false){

                int secttid = entry.secttid;
                int rid = entry.rid;
                double start = entry.start;
                double end = start + temp_dist; 
                SectTreeEntry* temp_entry = 
                  new SectTreeEntry(secttid,rid,start,end,true,true);
                sec_down.push_back(*temp_entry);
                delete temp_entry; 
                temp_dist -= fabs(entry.start - entry.end);

              }else{ 
                int secttid = entry.secttid;
                int rid = entry.rid;
                double start = entry.end - temp_dist;
                double end = entry.end; 
                SectTreeEntry* temp_entry = 
                  new SectTreeEntry(secttid,rid,start,end,true,true);
                sec_down.push_back(*temp_entry);
                delete temp_entry; 
                temp_dist -= fabs(entry.start - entry.end);
              }
            }
      }else break;
  }
  
}                       
  
/*
find the section to up direction. bus route goes up

*/

void BusRoute::FindUpSection(double cur_bus_stop_pos,
                     vector<SectTreeEntry> sec_list,int sec_index,
                     const double dist_val,
                     vector<SectTreeEntry>& sec_up,vector<bool> start_from)
                       
{
  double temp_dist;
  if(start_from[sec_index]){
    int secttid = sec_list[sec_index].secttid;
    int rid = sec_list[sec_index].rid;
    double start = cur_bus_stop_pos;
    double end = sec_list[sec_index].end;
    SectTreeEntry* tmp_entry = 
        new SectTreeEntry(secttid,rid,start,end,true,true);
    sec_up.push_back(*tmp_entry);
    delete tmp_entry; 
    
    temp_dist = dist_val - fabs(end - start);
    
  }else{
    int secttid = sec_list[sec_index].secttid;
    int rid = sec_list[sec_index].rid;
    double start = sec_list[sec_index].start;
    double end = cur_bus_stop_pos;
    SectTreeEntry* tmp_entry = 
        new SectTreeEntry(secttid,rid,start,end,true,true);
    sec_up.push_back(*tmp_entry);
    delete tmp_entry; 
    
    temp_dist = dist_val - fabs(end - start);
  
  }  
        
  unsigned int temp_index = sec_index;
  while(temp_dist > 0.0){
      temp_index++;
      if(temp_index < sec_list.size()){
        
            SectTreeEntry entry = sec_list[temp_index];
            
            if(temp_dist - fabs(entry.start - entry.end) > 0){
              sec_up.push_back(entry);
              temp_dist -= fabs(entry.start - entry.end);
            }else{
              if(start_from[temp_index]){
                
                int secttid = entry.secttid;
                int rid = entry.rid;
                double start = entry.start;
                double end = start + temp_dist; 
                SectTreeEntry* temp_entry = 
                  new SectTreeEntry(secttid,rid,start,end,true,true);
                sec_up.push_back(*temp_entry);
                delete temp_entry; 
                temp_dist -= fabs(entry.start - entry.end);
                
              }else{ 
                int secttid = entry.secttid;
                int rid = entry.rid;
                double start = entry.end - temp_dist;
                double end = entry.end; 
                SectTreeEntry* temp_entry = 
                  new SectTreeEntry(secttid,rid,start,end,true,true);
                sec_up.push_back(*temp_entry);
                delete temp_entry; 
                temp_dist -= fabs(entry.start - entry.end);
              }
            }
      }else break;
  }
  
}                       

/*
convert a gline object to a list of section representations 

*/

void BusRoute::GetSectionList(GLine* gl,vector<SectTreeEntry>& sec_list, 
                              vector<bool>& start_from)
{
  //////////////////////////////////////////////////////////////////////////
  
   for(int i = 0; i < gl->Size();i++){
    RouteInterval* ri = new RouteInterval();
    gl->Get(i, *ri); 
    
//    cout<<"rid "<<ri->GetRouteId()
//        <<"start "<<ri->GetStartPos()
//        <<"end "<<ri->GetEndPos()<<endl;
    
    if(!AlmostEqual(ri->GetStartPos(), ri->GetEndPos())){
      
/*      DbArray<SectTreeEntry>* actSections = new DbArray<SectTreeEntry>(0);
      n->GetSectionsOfRouteInterval(ri,actSections);

      SectTreeEntry nEntry;
      
      vector<SectTreeEntry> sec_list_temp; 
      
      for(int j = 0;j < actSections->Size();j++){
          actSections->Get(j, nEntry);

          ////////////////////////////////////////////////////
//          cout<<"entry "<<nEntry.rid
//              <<" start "<<nEntry.start
//              <<" end "<<nEntry.end<<endl; 
              
          if(AlmostEqual(nEntry.start,nEntry.end))continue; 

          sec_list_temp.push_back(nEntry); 
          
          if(ri->GetStartPos() < ri->GetEndPos()) start_from.push_back(true);
          else
            start_from.push_back(false); 
          
      }  
      
      if(ri->GetStartPos() < ri->GetEndPos()){
        for(unsigned int k = 0;k < sec_list_temp.size();k++)
          sec_list.push_back(sec_list_temp[k]);
      }else{
        for(int k = sec_list_temp.size() - 1;k >= 0;k--)
          sec_list.push_back(sec_list_temp[k]);
      }
      
      
      delete actSections;*/

      vector<SectTreeEntry> actSections;
      n->GetSectionsOfRouteInterval(ri,actSections);

      SectTreeEntry nEntry;

      vector<SectTreeEntry> sec_list_temp; 

      for(unsigned int j = 0;j < actSections.size();j++){
          nEntry = actSections[j];

          ////////////////////////////////////////////////////
//          cout<<"entry "<<nEntry.rid
//              <<" start "<<nEntry.start
//              <<" end "<<nEntry.end<<endl; 
              
          if(AlmostEqual(nEntry.start,nEntry.end))continue; 

          sec_list_temp.push_back(nEntry); 
          
          if(ri->GetStartPos() < ri->GetEndPos()) start_from.push_back(true);
          else
            start_from.push_back(false); 
          
      }  
      
      if(ri->GetStartPos() < ri->GetEndPos()){
        for(unsigned int k = 0;k < sec_list_temp.size();k++)
          sec_list.push_back(sec_list_temp[k]);
      }else{
        for(int k = sec_list_temp.size() - 1;k >= 0;k--)
          sec_list.push_back(sec_list_temp[k]);
      }
      

    }  
      delete ri; 
  }

}



/*
change the representation for bus stop, get the pos value.
i.e., the relative position on its bus route 

*/
void BusRoute::CalculateStartSmaller(vector<BusStop>& bus_stop_list,
                                int start, int end, 
                                vector<SectTreeEntry>& sec_list,
                                vector<bool>& start_from,
                                vector<double>& dist_list, SimpleLine* sl)
{
//  cout<<"start "<<start<<" end "<<end<<endl; 
  double dist = 0.0;
  unsigned int bus_stop_index = start;
  
  bool startsmaller; 
  bool initialize = false; 
  ///////////use the section list of the bus route/////////////////
  //////////bus stops are ordered from id small to id big///////////
  
  for(unsigned int i = 0;i < sec_list.size();i++){
    unsigned int sec_id = sec_list[i].secttid; 
    double start = sec_list[i].start;
    double end = sec_list[i].end; 
    
 //   cout<<"sec_id "<<sec_id
 //       <<" busstop sec id "<<bus_stop_list[bus_stop_index].sid<<endl; 
    
    //find the section where the bus stop locates 
    while(bus_stop_list[bus_stop_index].sid == sec_id){
        double temp_dist = 0.0; 
        if(start_from[i]){
           temp_dist = fabs(bus_stop_list[bus_stop_index].pos - start);
           dist += temp_dist;
//           cout<<"pos1 "<<dist<<endl; 
           dist_list.push_back(dist);
        }else{
            temp_dist = fabs(bus_stop_list[bus_stop_index].pos - end);
            dist += temp_dist;
//            cout<<"pos2 "<<dist<<endl; 
            dist_list.push_back(dist);
        }
        ///////////////////////////////////////////////////////////////
        
        int rid = bus_stop_list[bus_stop_index].rid; 
        double pos = bus_stop_list[bus_stop_index].pos;
        GPoint* gp = new GPoint(true,n->GetId(),rid,pos,None); 
        Point* p = new Point();
        gp->ToPoint(p);

        //////////////////////////////////////////////////////////////
        if(initialize == false){
          Point temp_p;
          assert(sl->AtPosition(dist, true, temp_p));
          const double dist_delta = 1.0; 
          if(temp_p.Distance(*p) < dist_delta)
              startsmaller = true;
          else
              startsmaller = false;
          
          initialize = true; 
/*           cout<<"loc1 "<<*p<<" loc2 "<<temp_p
              <<" startsmaller "<<startsmaller<<endl; */
        }

        startSmaller.push_back(startsmaller); 

        //////////////////////////////////////////////////////////////
        delete p; 
        delete gp; 
        //////////////////////////////////////////////////////////////
        bus_stop_index++; 
        dist -= temp_dist; 
    }
    
    if(bus_stop_list[bus_stop_index].sid != sec_id)  
        dist += fabs(start - end);
  }
  
  assert(dist_list.size() == (end - start)); 
}

/*
change the position of bus stops after translate the bus route: down and up 
and for each route, it gets pos value.
bus stops on up direction are from small to big
bus tops on down direction are from big to small, but the pos value is set
according to the smaller start point 

*/
void BusRoute::CreateBusStop4(int attr_a,int attr_b,int attr1,int attr2,
                              int attr3, int attr4)
{
//  cout<<"attr_a "<<attr_a<<" attr_b "<<attr_b<<" attr1 "<<attr1
//      <<" attr2 "<<attr2<<" attr3 "<<attr3<<" attr4 "<<attr4<<endl; 
      
  vector<BusStop_Ext> bus_stop_list;
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple_bus_stop = rel2->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr1))->GetIntval();
    int stop_id = 
                ((CcInt*)tuple_bus_stop->GetAttribute(attr2))->GetIntval();
    Point* loc = (Point*)tuple_bus_stop->GetAttribute(attr3);
    
    bool start_small = 
          ((CcBool*)tuple_bus_stop->GetAttribute(attr4))->GetBoolval(); 

    BusStop_Ext* bse = new BusStop_Ext(br_id,stop_id,0,*loc,start_small);
    bus_stop_list.push_back(*bse);
    delete bse; 
    tuple_bus_stop->DeleteIfAllowed();
  }    

  sort(bus_stop_list.begin(), bus_stop_list.end());

/*  for(unsigned int i = 0;i < bus_stop_list.size();i++)    
    bus_stop_list[i].Print(); */

  vector<Line> line_list1;
  
  vector<Line> line_list2;
  vector<SimpleLine> sline_list;
  
  
  vector<bool> bus_route_direction; 
  
  for(int i = 1;i <= rel1->GetNoTuples();i+= 2){
    Tuple* tuple_bus_route1 = rel1->GetTuple(i, false);
    Tuple* tuple_bus_route2 = rel1->GetTuple(i + 1, false);
    Line* l = (Line*)tuple_bus_route1->GetAttribute(attr_a); 
    Line* l1 = (Line*)tuple_bus_route1->GetAttribute(attr_b);
    Line* l2 = (Line*)tuple_bus_route2->GetAttribute(attr_b);
    SimpleLine* sl1 = new SimpleLine(0);
    SimpleLine* sl2 = new SimpleLine(0);
    sl1->fromLine(*l1);
    sl2->fromLine(*l2);
    
    line_list1.push_back(*l);
    
    line_list2.push_back(*l1);
    line_list2.push_back(*l2);
    sline_list.push_back(*sl1);
    sline_list.push_back(*sl2);
        

    delete sl1;
    delete sl2; 
    
    tuple_bus_route1->DeleteIfAllowed();
    tuple_bus_route2->DeleteIfAllowed(); 
    
  }
  
//  cout<<"line list size "<<line_list1.size()
//      <<" sline list size "<<sline_list.size()<<endl; 
      
  const double dist_delta = 0.01; 
  int stop_loc_id = 1; //unique spatial location before transfer up and down  
  
  for(unsigned int i = 0;i < bus_stop_list.size();i++){
    vector<BusStop_Ext> bus_stop_list_new; 
  
    bus_stop_list_new.push_back(bus_stop_list[i]);
  
    ////////collect all bus stops mapping to the same 2D point in space/////
    unsigned int j = i + 1;
    BusStop_Ext bse = bus_stop_list_new[0]; 

    while(j < bus_stop_list.size() &&
        bus_stop_list[j].loc.Distance(bse.loc) < dist_delta ){
        bus_stop_list_new.push_back(bus_stop_list[j]);
        j++; 
    }
    i = j - 1; 
    
    ////////////get the intersection point//////////////////////////////
    BusStop_Ext bse_0 = bus_stop_list_new[0]; 
    
//    bse_0.Print(); 
    
    Line* l = &line_list1[bse_0.br_id - 1];
    HalfSegment hs;
    int index = -1;
    
    double hs_p_dist = numeric_limits<double>::max();
    int temp_index = -1; 
    
    for(index = 0;index < l->Size();index++){
      l->Get(index, hs);
      if(hs.IsLeftDomPoint() == false)continue;
      if(hs.Contains(bse_0.loc))break; 
      if(hs.Distance(bse_0.loc) < hs_p_dist){ //solve numeric problem 
          hs_p_dist = hs.Distance(bse_0.loc);
          temp_index = index; 
      }
    }
    
    if(index == -1 || index == l->Size()){
        if(temp_index != -1){
          l->Get(temp_index, hs);
        }else{
          cout<<"can't find the point (might be numeric problem)"<<endl;
          assert(false); 
        }    
    }
    
//    cout<<"hs "<<hs<<endl; 

//    Line* l1 = &line_list2[(bse_0.br_id * 2 - 1) - 1];
//    Line* l2 = &line_list2[(bse_0.br_id * 2) - 1];
     
    //for each translated bus route simple 
    
    for(unsigned int k = 0;k < bus_stop_list_new.size();k++){

      BusStop_Ext bs_ext = bus_stop_list_new[k]; 
      
//      SimpleLine* sl1 = &sline_list[(bse_0.br_id * 2 - 1) - 1];
//      SimpleLine* sl2 = &sline_list[(bse_0.br_id * 2 ) - 1];

//      bool sm = bse_0.start_small;
      bool sm = bs_ext.start_small; 
      double pos1,pos2;

//      cout<<"br_id "<<bs_ext.br_id<<"bs_stop_id "<<bs_ext.br_stop_id<<endl;

      Line* l1 = &line_list2[(bs_ext.br_id * 2 - 1) - 1];
      Line* l2 = &line_list2[(bs_ext.br_id * 2) - 1];

      vector<MyPoint> intersect_ps;
      GetInterestingPoints(hs,bse_0.loc,intersect_ps,l1,l2);
    
      assert(intersect_ps.size() == 2); 

//      cout<<"0 "<<intersect_ps[0].loc
//          <<" 1 "<<intersect_ps[1].loc<<endl; 
              
      SimpleLine* sl1 = &sline_list[(bs_ext.br_id * 2 - 1) - 1];
      SimpleLine* sl2 = &sline_list[(bs_ext.br_id * 2 ) - 1];
      
//      assert(sl1->AtPoint(intersect_ps[0].loc,sm,pos1));
//      assert(sl2->AtPoint(intersect_ps[1].loc,sm,pos2));

      //to solve numeric problem
      if(sl1->AtPoint(intersect_ps[0].loc,sm,pos1) == false)
        assert(MyAtPoint(sl1,intersect_ps[0].loc,sm,pos1,dist_delta));
      //to solve numeric problem
      if(sl2->AtPoint(intersect_ps[1].loc,sm,pos2) == false)
        assert(MyAtPoint(sl2,intersect_ps[1].loc,sm,pos2,dist_delta));

//      cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl; 
    
      br_id_list.push_back(bs_ext.br_id);
      br_uid_list.push_back(bs_ext.br_id*2 - 1);//line l1
      br_stop_id.push_back(bs_ext.br_stop_id);
      start_gp.push_back(bs_ext.loc);
      end_gp.push_back(intersect_ps[0].loc);
      bus_stop_loc_3.push_back(pos1); 
//      bus_sections1.push_back(*l1);  /////////////for debuging/////////

      /// unique spatial location: no up and down 
      stop_loc_id_list.push_back(stop_loc_id);
      
      
      br_id_list.push_back(bs_ext.br_id);
      br_stop_id.push_back(bs_ext.br_stop_id);
      br_uid_list.push_back(bs_ext.br_id*2);//line l2
      start_gp.push_back(bs_ext.loc);
      end_gp.push_back(intersect_ps[1].loc);
      bus_stop_loc_3.push_back(pos2); 
//      bus_sections1.push_back(*l2);  /////////////for debuging/////////

      /// unique spatial location: no up and down 
      stop_loc_id_list.push_back(stop_loc_id);///// unique spatial location 
      
    }

    stop_loc_id++;//another differnt spatial location in space 
  }
      
    //////////////   relation  ///////////////////////////
  /////////////  br uid = br id * 2 -1////////line l1 ///
  ////////////   br uid = br id * 2 //////////line l2 ///
  //////////////////////////////////////////////////////
}


/*
get the intersecting point between the create small line and two bus routes 

*/

void BusRoute::GetInterestingPoints(HalfSegment hs, Point ip, 
                                    vector<MyPoint>& list,
                                    Line* reg1, Line* reg2)
{
    Point p1 = hs.GetLeftPoint();
    Point p2 = hs.GetRightPoint();

    const double delta_dist = 10.0;
    Line* line1 = new Line(0);

    if(MyAlmostEqual(p1.GetX(), p2.GetX())){
        double y = ip.GetY();
        double x1 = ip.GetX() - delta_dist;
        double x2 = ip.GetX() + delta_dist;
        
        line1->StartBulkLoad();
        HalfSegment hs;
        Point lp,rp;
        lp.Set(x1, y);
        rp.Set(x2, y);
        hs.Set(true,lp,rp);
        int edgeno = 0;
        hs.attr.edgeno = edgeno++;
        *line1 += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        *line1 += hs;
        line1->EndBulkLoad();

      }else if(MyAlmostEqual(p1.GetY(), p2.GetY())){
            double y1 = ip.GetY() - delta_dist;
            double y2 = ip.GetY() + delta_dist;
            double x = ip.GetX();

            line1->StartBulkLoad();
            HalfSegment hs;
            Point lp,rp;
            lp.Set(x, y1);
            rp.Set(x, y2);
            hs.Set(true,lp,rp);
            int edgeno = 0;
            hs.attr.edgeno = edgeno++;
            *line1 += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *line1 += hs;
            line1->EndBulkLoad();

            
      }else{//not vertical
        double k1 = (p2.GetY() - p1.GetY())/(p2.GetX() - p1.GetX());
        double k2 = -1/k1;
        double c2 = ip.GetY() - ip.GetX()*k2;

        double x1 = ip.GetX() - delta_dist;
        double x2 = ip.GetX() + delta_dist;

        
        line1->StartBulkLoad();
        HalfSegment hs;
        Point lp,rp;
        lp.Set(x1, x1*k2 + c2);
        rp.Set(x2, x2*k2 + c2);
        hs.Set(true,lp,rp);
        int edgeno = 0;
        hs.attr.edgeno = edgeno++;
        *line1 += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        *line1 += hs;
        line1->EndBulkLoad();
      }
        
//        cout<<"ip "<<ip<<" created line "<<*line1<<endl;

        Points* ps1 = new Points(0);
        Points* ps2 = new Points(0);
        line1->Crossings(*reg1, *ps1);
        line1->Crossings(*reg2, *ps2);
//        cout<<l1->Size()<<" "<<l2->Size()<<endl; 

        assert(ps1->Size() >= 1 && ps2->Size() >= 1);
        
//        cout<<*ps1<<" "<<*ps2<<endl; 
        
        /////////////calculate the distanc to ip and get the two closest/////
        vector<MyPoint> temp1; 
        for(int i = 0;i <ps1->Size();i++){
          Point p;
          ps1->Get(i,p);
          MyPoint mp(p, p.Distance(ip));
          temp1.push_back(mp);
        }
        sort(temp1.begin(),temp1.end());
        list.push_back(temp1[0]); 

        
        vector<MyPoint> temp2; 
        for(int i = 0;i <ps2->Size();i++){
          Point p;
          ps2->Get(i,p);
          MyPoint mp(p, p.Distance(ip));
          temp2.push_back(mp);
        }
        sort(temp2.begin(),temp2.end());
        list.push_back(temp2[0]); 
        
        delete ps1;
        delete ps2;
        delete line1; 
}

/*
If the original SimpleLine::AtPoint does not work.
use this one. It uses the distance (a very small distance) 
between the point and halfsegment to
determine whether the point locates on the simpleline 

*/
bool BusRoute::MyAtPoint(SimpleLine* sl, Point& loc, 
                         bool sm, double& res, double dist_delta)
{
  SpacePartition* sp = new SpacePartition();
   
  vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
  sp->ReorderLine(sl, seq_halfseg);
  bool find = false;
  double pos = 0.0;
  if(sm){
    for(unsigned int i = 0;i < seq_halfseg.size();i++){
      HalfSegment hs(true,seq_halfseg[i].from,seq_halfseg[i].to);
      if(hs.Distance(loc) < dist_delta){
        pos += loc.Distance(seq_halfseg[i].from);
        res = pos;      
        find = true; 
        break; 
      }else
        pos += hs.Length();
    }  
  }else{
    for(int i = seq_halfseg.size() - 1;i >= 0;i--){
      HalfSegment hs(true,seq_halfseg[i].from,seq_halfseg[i].to);
      if(hs.Distance(loc) < dist_delta){
        pos += loc.Distance(seq_halfseg[i].to);
        res = pos;      
        find = true; 
        break; 
      }else
        pos += hs.Length();
    }  
  }
  
  delete sp; 

  return find; 
}

/*
for the two new generated bus routes, it sets the up and down value 

*/

void BusRoute::CreateRoute4(int attr1, int attr2, int attr3, int attr4, 
                            int attr_a, int attr_b)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3
//      <<"attr4 "<<attr4<<"attr_a "<<attr_a<<" attr_b "<<attr_b<<endl; 

  vector<bool> start_small_list;
  int last_br_id = 0;
  for(int i = 1;i <= rel2->GetNoTuples();i++){
      Tuple* tuple_bus_stop = rel2->GetTuple(i, false);
      int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr_a))->GetIntval();
      bool sm = ((CcBool*)tuple_bus_stop->GetAttribute(attr_b))->GetBoolval();
      if(start_small_list.size() == 0){
        start_small_list.push_back(sm);
        last_br_id = br_id; 
      }  
      else{
        if(br_id != last_br_id){
          last_br_id = br_id; 
          start_small_list.push_back(sm);
        }
      }
      
      tuple_bus_stop->DeleteIfAllowed();
  }
//  cout<<"start_small_list size "<<start_small_list.size()<<endl; 

  assert((int)start_small_list.size() == rel1->GetNoTuples() / 2); 
  
  for(int i = 1;i <= rel1->GetNoTuples();i+=2){
    Tuple* tuple_bus_route1 = rel1->GetTuple(i, false);
    Tuple* tuple_bus_route2 = rel1->GetTuple(i + 1, false);
    
    int br_id1 = ((CcInt*)tuple_bus_route1->GetAttribute(attr1))->GetIntval();
    int br_id2 = ((CcInt*)tuple_bus_route2->GetAttribute(attr1))->GetIntval();
    assert(br_id1 == br_id2);
    
    Line* l1 = (Line*)tuple_bus_route1->GetAttribute(attr2);
    Line* l2 = (Line*)tuple_bus_route2->GetAttribute(attr2);
    
    SimpleLine* sl1 = new SimpleLine(0);
    SimpleLine* sl2 = new SimpleLine(0);
    sl1->fromLine(*l1);
    sl2->fromLine(*l2);
    
    
    CalculateUpandDown(sl1, sl2, start_small_list[br_id1 - 1]); 
    
    delete sl1;
    delete sl2; 
    
    int route_type1 = 
        ((CcInt*)tuple_bus_route1->GetAttribute(attr3))->GetIntval();
    int route_type2 = 
        ((CcInt*)tuple_bus_route2->GetAttribute(attr3))->GetIntval();
        
        
    int br_uid_1 = 
        ((CcInt*)tuple_bus_route1->GetAttribute(attr4))->GetIntval();
    int br_uid_2 = 
        ((CcInt*)tuple_bus_route2->GetAttribute(attr4))->GetIntval();
    
        
    br_id_list.push_back(br_id1);
    br_id_list.push_back(br_id2); 
    
    bus_sections1.push_back(*l1);
    bus_sections1.push_back(*l2);
    
    bus_route_type.push_back(route_type1);
    bus_route_type.push_back(route_type2);
    
    br_uid_list.push_back(br_uid_1);
    br_uid_list.push_back(br_uid_2);
    
    ///////////////////add the startsmaller value /////////////////////////
    startSmaller.push_back(start_small_list[br_id1 - 1]);
    startSmaller.push_back(start_small_list[br_id1 - 1]);
    
    tuple_bus_route1->DeleteIfAllowed();
    tuple_bus_route2->DeleteIfAllowed();
    
  }
 
  assert(startSmaller.size() == br_id_list.size()); 
  
}


/*
define the up and down for each route. 
if it is up, the bus stop id increases
if it is down, the bus stop id decreases
use l1, and l2 to build a cycle. use the cycle (counter-clockwise or clockwise)
to determine the direction for l1 and l2.
we also have to the start smaller value because it needs to identify where the
route starts 

We assume all buses moving on the right side of the road (Germany,China)

*/

void BusRoute::CalculateUpandDown( SimpleLine* sl1, SimpleLine* sl2, bool sm)
{
  
  SpacePartition* sp = new SpacePartition();
   
  vector<MyHalfSegment> seq_halfseg1; //reorder it from start to end
  sp->ReorderLine(sl1, seq_halfseg1);
  
  vector<MyHalfSegment> seq_halfseg2; //reorder it from start to end
  sp->ReorderLine(sl2, seq_halfseg2);
 
  
  assert(sl1->Size() >= 2);
  assert(sl2->Size() >= 2);
  vector<Point> point_list;
  
  //assume we start from sl1 (up direction)
  if(sm){
    unsigned int index1 = 0;
    unsigned int index2 = 1;
    
    point_list.push_back(seq_halfseg1[index1].from);
    point_list.push_back(seq_halfseg1[index2].from);
    point_list.push_back(seq_halfseg1[index2].to);
    point_list.push_back(seq_halfseg2[index2].to);
    point_list.push_back(seq_halfseg2[index2].from);
    point_list.push_back(seq_halfseg2[index1].from);
    

  }else{
    unsigned int index1 = seq_halfseg1.size() - 1;
    unsigned int index2 = seq_halfseg1.size() - 2;
  
    point_list.push_back(seq_halfseg1[index1].to);
    point_list.push_back(seq_halfseg1[index1].from);
    point_list.push_back(seq_halfseg1[index2].from);
    point_list.push_back(seq_halfseg2[index2].from);
    point_list.push_back(seq_halfseg2[index1].from);
    point_list.push_back(seq_halfseg2[index1].to);
    
  }  
   
    vector<Region> regs;
    sp->ComputeRegion(point_list,regs);
    assert(regs.size() == 1);
    delete sp; 
  
    CompTriangle* ct = new CompTriangle();
    bool clock_wise = ct->GetClockwise(point_list);
    if(clock_wise){
  //    cout<<"l1 "<<false<<"l2 "<<true<<endl;
      direction_flag.push_back(false);
      direction_flag.push_back(true);
    }else{
  //    cout<<"l1 "<<true<<" l2 "<<false<<endl; 
      direction_flag.push_back(true);
      direction_flag.push_back(false);
    }
    delete ct; 

}

/*
set the up and down value for each bus stop 

*/
void BusRoute::CreateBusStop5(int attr,int attr1,int attr2,
                      int attr3,int attr4, int attr5)
{
//  cout<<"attr "<<attr<<"attr1 "<<attr1<<" attr2 "<<attr2
//      <<" attr3 "<<attr3<<" attr4 "<<attr4<<" attr5 "<<attr5<<endl; 

  vector<bool> br_direction; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* tuple_bus_route = rel1->GetTuple(i, false);
      bool direction = 
        ((CcBool*)tuple_bus_route->GetAttribute(attr))->GetBoolval();
      br_direction.push_back(direction);
      tuple_bus_route->DeleteIfAllowed();
  }
  
//  cout<<"direction size "<<br_direction.size()<<endl; 
  
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple_bus_stop = rel2->GetTuple(i, false);
    int br_id =  ((CcInt*)tuple_bus_stop->GetAttribute(attr1))->GetIntval();
    int br_uid = ((CcInt*)tuple_bus_stop->GetAttribute(attr2))->GetIntval();
    int stop_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr3))->GetIntval();
    Point* loc = (Point*)tuple_bus_stop->GetAttribute(attr4);
    double pos = ((CcReal*)tuple_bus_stop->GetAttribute(attr5))->GetRealval();
    assert(br_uid == br_id *2 - 1 || br_uid == br_id * 2);
    
    bool direction = br_direction[br_uid - 1];
    
    br_id_list.push_back(br_id);
    br_uid_list.push_back(br_uid);
    br_stop_id.push_back(stop_id);
    start_gp.push_back(*loc);
    bus_stop_loc_3.push_back(pos);
    bus_stop_flag.push_back(direction); 
    
    tuple_bus_stop->DeleteIfAllowed();
  }
  
}

/*
create bus stops relations with the data type busstop 

*/
void BusRoute::GetBusStops()
{
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* br_tuple = rel2->GetTuple(i, false);
    int br_id = 
        ((CcInt*)br_tuple->GetAttribute(RoadDenstiy::BR_ID6))->GetIntval();
    bool direction = 
     ((CcBool*)br_tuple->GetAttribute(RoadDenstiy::BR_DIRECTION))->GetBoolval();

     int br_uid = 
        ((CcInt*)br_tuple->GetAttribute(RoadDenstiy::BR_RUID))->GetIntval();

    CcInt* search_id = new CcInt(true, br_id);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    int count = 0; 
    while(btree_iter->Next()){
        Tuple* bs_tuple = rel1->GetTuple(btree_iter->GetId(), false);
        bool d = 
   ((CcBool*)bs_tuple->GetAttribute(RoadDenstiy::STOP_DIRECTION))->GetBoolval();
       if(direction == d){
          int br_id_s = 
            ((CcInt*)bs_tuple->GetAttribute(RoadDenstiy::BR_ID4))->GetIntval();

          int br_uid_s = 
            ((CcInt*)bs_tuple->GetAttribute(RoadDenstiy::BR_UID))->GetIntval();
          assert(br_id_s == br_id); 
          assert(br_uid == br_uid_s); 
          int s_id = 
        ((CcInt*)bs_tuple->GetAttribute(RoadDenstiy::BUS_STOP_ID))->GetIntval();
//          Point* loc = (Point*)bs_tuple->GetAttribute(RoadDenstiy::BUS_LOC);

//          cout<<"busstop_rid "<<br_id_s<<" stop id "<<s_id<<endl;

//          bus_stop_list.push_back(Bus_Stop(true, br_id_s, s_id));
          bus_stop_list.push_back(Bus_Stop(true, br_id_s, s_id, d));
//          bus_stop_geodata.push_back(*loc);
          count++;
       }
        bs_tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;

    br_tuple->DeleteIfAllowed();
  }

}

/*
create bus routes represented by the data type busroute 

*/
void BusRoute::GetBusRoutes()
{
  for(int i = 1;i <= rel2->GetNoTuples();i++){
  Tuple* br_tuple = rel2->GetTuple(i, false);
    int br_id = 
        ((CcInt*)br_tuple->GetAttribute(RoadDenstiy::BR_ID6))->GetIntval();
    bool direction = 
     ((CcBool*)br_tuple->GetAttribute(RoadDenstiy::BR_DIRECTION))->GetBoolval();

    bool small = 
 ((CcBool*)br_tuple->GetAttribute(RoadDenstiy::BR_STARTSMALLER))->GetBoolval();

    Line* l = (Line*)br_tuple->GetAttribute(RoadDenstiy::BR_GEODATA);
    CcInt* search_id = new CcInt(true, br_id);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    vector<TupleId> tid_list; 
    while(btree_iter->Next()){
        Tuple* bs_tuple = rel1->GetTuple(btree_iter->GetId(), false);
        bool d = 
   ((CcBool*)bs_tuple->GetAttribute(RoadDenstiy::STOP_DIRECTION))->GetBoolval();
       if(direction == d){
          tid_list.push_back(bs_tuple->GetTupleId());
       }
        bs_tuple->DeleteIfAllowed();
    }
    SimpleLine* sl = new SimpleLine(0); 
    sl->fromLine(*l);

    CreateRoutes(tid_list, br_id, sl, small, direction);

    delete sl; 
    delete btree_iter;
    delete search_id;

    br_tuple->DeleteIfAllowed();

//    break; 
  }

}

/*
sort bus stops of one route by stop id 

*/
bool CompareBusStopExt(const BusStop_Ext& bs1, const BusStop_Ext& bs2)
{
   if(bs1.br_stop_id < bs2.br_stop_id) return true;
    else return false;

}

/*
create a bus route consisting of a set of bus segments.
there is a restriction of bus routes. 
before the first stop and after the last stop, there are also some bus segments

*/
void BusRoute::CreateRoutes(vector<TupleId>& tid_list, int br_id,
                            SimpleLine* sl, bool small, bool direction)
{
  vector<BusStop_Ext> bslist; 
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* bs_tuple = rel1->GetTuple(tid_list[i], false);
    int bs_id = 
      ((CcInt*)bs_tuple->GetAttribute(RoadDenstiy::BR_ID4))->GetIntval(); 

    assert(bs_id == br_id);

    double pos = 
      ((CcReal*)bs_tuple->GetAttribute(RoadDenstiy::BUS_POS))->GetRealval(); 

    Point* loc = (Point*)bs_tuple->GetAttribute(RoadDenstiy::BUS_LOC); 
    int s_id = 
      ((CcInt*)bs_tuple->GetAttribute(RoadDenstiy::BUS_STOP_ID))->GetIntval(); 

    BusStop_Ext bs(bs_id, s_id, pos, *loc, small);
    bslist.push_back(bs);
    bs_tuple->DeleteIfAllowed();
  }

  const double delta_dist = 0.001; 
  sort(bslist.begin(), bslist.end(), CompareBusStopExt); 
  Bus_Route* br = new Bus_Route(br_id, direction);
  br->StartBulkLoad(); 
  int count = 0;
  
  for(unsigned int i = 0;i < bslist.size() - 1;i++){//ordered by stop id 
//      bslist[i].Print(); 
      Point loc1 = bslist[i].loc;
      Point loc2 = bslist[i + 1].loc;

//      cout<<"stop id1 "<<bslist[i].br_stop_id
//          <<" stop id2 "<<bslist[i + 1].br_stop_id<<endl;

//      cout<<"loc1 "<<bslist[i].loc<<" loc2 "<<bslist[i + 1].loc<<endl; 

      double pos1, pos2; 
      assert(sl->AtPoint(loc1, sl->GetStartSmaller(), pos1));
      assert(sl->AtPoint(loc2, sl->GetStartSmaller(), pos2));
      /////////////////////////////////////////////////////////////////////
      ////////////////////////section before the first stop////////////////
      /////////////////////////////////////////////////////////////////////
      if(i == 0){
//        cout<<"pos1 "<<pos1<<" length "<<sl->Length()<<endl; 
        assert(pos1 > 0.0 && !AlmostEqual(pos1, sl->Length())); 

        double l1;
        double l2;
        if(pos1 > pos2){
          l1 = pos1;
          l2 = sl->Length(); 
        }else{
           l1 = 0.0;
           l2 = pos1; 
        }

          SimpleLine sub_line(0);

          if(sl->GetStartSmaller())
            sl->SubLine(l1, l2, sl->GetStartSmaller(), sub_line);
          else
            sl->SubLine(l2, l1, sl->GetStartSmaller(), sub_line);

          assert(AlmostEqual(sub_line.Length(), l2 - l1)); 

          Point p1, p2;
          sub_line.AtPosition(0.0, sub_line.GetStartSmaller() ,p1); 
         sub_line.AtPosition(sub_line.Length(), sub_line.GetStartSmaller(), p2);
          bool startSmaller;
          if(loc1.Distance(p1) < delta_dist){
            startSmaller = !sub_line.GetStartSmaller();
          }else if(loc1.Distance(p2) < delta_dist){
            startSmaller = sub_line.GetStartSmaller();

          }else assert(false); 

          br->Add(&sub_line, count);
          count++;
       }
      //////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////
      double len1, len2; 
      if(pos1 > pos2){
        len1 = pos2;
        len2 = pos1;
      }else{
        len1 = pos1;
        len2 = pos2; 
      }

      SimpleLine sub_l(0);

      if(sl->GetStartSmaller())
//        sl->SubLine(pos1, pos2, sl->GetStartSmaller(), sub_l);
        sl->SubLine(len1, len2, sl->GetStartSmaller(), sub_l);
      else
//        sl->SubLine(pos2, pos1, sl->GetStartSmaller(), sub_l);
        sl->SubLine(len2, len1, sl->GetStartSmaller(), sub_l);

      assert(AlmostEqual(sub_l.Length(), fabs(pos2 - pos1))); 
      Point sp, ep;
      sub_l.AtPosition(0.0, sub_l.GetStartSmaller() ,sp); 
      sub_l.AtPosition(sub_l.Length(), sub_l.GetStartSmaller(), ep);

//      cout<<"sp "<<sp<<" ep "<<ep<<endl<<endl; 

      bool startSmaller; 
      if(loc1.Distance(sp) < delta_dist && loc2.Distance(ep) < delta_dist){
        startSmaller = sub_l.GetStartSmaller();
      }else if(loc1.Distance(ep) < delta_dist && 
               loc2.Distance(sp) < delta_dist){
        startSmaller = !sub_l.GetStartSmaller(); 
      }else assert(false); 

      br->Add(&sub_l, count);
      count++;
      ////////////////////////////////////////////////////////////////////////
     ////////////////////the segment after the last stop//////////////////////
     /////////////////////////////////////////////////////////////////////////
      if(i == bslist.size() - 2){
        assert(pos2 > 0.0 && !AlmostEqual(pos2, sl->Length())); 
        double l1;
        double l2;
        if(pos1 > pos2){
          l1 = 0.0;
          l2 = pos2; 
        }else{
           l1 = pos2;
           l2 = sl->Length(); 
        }

         SimpleLine sub_line(0);

         if(sl->GetStartSmaller())
            sl->SubLine(l1, l2, sl->GetStartSmaller(), sub_line);
         else
            sl->SubLine(l2, l1, sl->GetStartSmaller(), sub_line);

          assert(AlmostEqual(sub_line.Length(), l2 - l1)); 

         Point p1, p2;
         sub_line.AtPosition(0.0, sub_line.GetStartSmaller() ,p1); 
         sub_line.AtPosition(sub_line.Length(), sub_line.GetStartSmaller(), p2);
          bool startSmaller;
          //for the last bus segment, the point corresponds to the start point
          if(loc2.Distance(p1) < delta_dist){//the last segment is different 
            startSmaller = sub_line.GetStartSmaller();
          }else if(loc2.Distance(p2) < delta_dist){
            startSmaller = !sub_line.GetStartSmaller();
          }else assert(false); 

          br->Add(&sub_line, count);
          count++;
      }
  }
  
  /////////////////////////////////////////////////////////////////////////
  br->EndBulkLoad(); 
  bus_route_list.push_back(*br); 
  delete br; 
}

//////////////////////////////////////////////////////////////////////////////
//      use the road density data to set time schedule for each bus route  ///
/////////////////////////////////////////////////////////////////////////////
string RoadDenstiy::bus_route_speed_typeinfo = 
"(rel (tuple ((br_id int) (br_pos real) (speed_limit real)\
(route_segment line))))";

string RoadDenstiy::bus_stop_typeinfo = 
"(rel (tuple ((br_id int) (br_uid int) (bus_stop_id int) (bus_stop point)\
(bus_pos real) (stop_direction bool))))"; 


string RoadDenstiy::night_sched_typeinfo = 
"(rel (tuple ((br_id int) (duration1 periods) (duration2 periods) (br_interval\
real))))"; 

string RoadDenstiy::day_sched_typeinfo = 
"(rel (tuple ((br_id int) (duration1 periods) (duration2 periods) (br_interval1\
real) (br_interval2 real))))";

string RoadDenstiy::mo_bus_typeinfo = 
"(rel (tuple ((br_id int) (bus_direction bool) (bus_trip mpoint) (bus_type \
string) (bus_day string) (schedule_id int))))";

string RoadDenstiy::bus_route_typeinfo = 
"(rel (tuple ((br_id int) (bus_route line) (route_type int) (br_uid int)\
(bus_direction bool) (startSmaller bool))))"; 


string RoadDenstiy::bus_route_old_typeinfo = 
"(rel (tuple ((br_id int) (bus_route1 gline) (bus_route2 line)\
(start_loc point) (end_loc point) (route_type int))))"; 


/*
get night buses. use the network flow value.
for night bus, we don't dinstinguish Sunday and Workday 

*/

void RoadDenstiy::GetNightRoutes(int attr1, int attr2, int attr_a,
                                    int attr_b,Periods* peri1, Periods* peri2)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2
//      <<"attr_a "<<attr_a<<" attr_b "<<attr_b<<endl; 
//  cout<<*peri1<<" "<<*peri2<<endl; 

  Interval<Instant> periods1;
  Interval<Instant> periods2;
  peri1->Get(0, periods1);
  peri2->Get(0, periods2);
  
//  cout<<periods1<<" "<<periods2<<endl; 
  
  for(int i = 1;i <= rel2->GetNoTuples();i++){
     
    Tuple* tuple_bus_stop = rel2->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(attr_a))->GetIntval();
    GLine* gl = (GLine*)tuple_bus_stop->GetAttribute(attr_b);
    
//    cout<<*gl<<endl; 
    
    BusRoute* br = new BusRoute(n,NULL,NULL);

    vector<bool> start_from; 
    vector<SectTreeEntry> sec_list; 
    br->GetSectionList(gl, sec_list, start_from);
    
    int traffic_num = 0;
    
    Interval<Instant> time_span1 = periods1;
    Interval<Instant> time_span2 = periods2; 
    
    for(unsigned int j = 0;j < sec_list.size();j++){
        int secid = sec_list[j].secttid; 
        /////////////use btree to find the section/////////////////
        CcInt* search_sec_id = new CcInt(true, secid);
        BTreeIterator* btree_iter = btree->ExactMatch(search_sec_id);
        while(btree_iter->Next()){
          Tuple* tuple_traffic = rel1->GetTuple(btree_iter->GetId(), false);
          MInt* flow = (MInt*)tuple_traffic->GetAttribute(attr2);
//          cout<<*flow<<endl; 
          for(int k = 0;k < flow->GetNoComponents();k++){
            UInt cur_uint;
            flow->Get(k,cur_uint);
            
            if(periods1.Intersects(cur_uint.timeInterval)){
//              cout<<"1"<<endl; 
//              cur_uint.Print(cout);  
              traffic_num += cur_uint.constValue.GetValue();
              
              if(cur_uint.timeInterval.start > time_span1.start && 
                 cur_uint.timeInterval.start < time_span1.end){
                time_span1.start = cur_uint.timeInterval.start;
              }  
              if(cur_uint.timeInterval.end < time_span1.end && 
                 cur_uint.timeInterval.end > time_span1.start)
                time_span1.end = cur_uint.timeInterval.end; 
            }

            if(periods2.Intersects(cur_uint.timeInterval)){
//              cout<<"2"<<endl; 
//              cur_uint.Print(cout);  
              traffic_num += cur_uint.constValue.GetValue();
              
              if(cur_uint.timeInterval.start > time_span2.start &&
                 cur_uint.timeInterval.start < time_span2.end)
                time_span2.start = cur_uint.timeInterval.start;
              if(cur_uint.timeInterval.end < time_span2.end &&
                 cur_uint.timeInterval.end > time_span2.start)
                time_span2.end = cur_uint.timeInterval.end; 
            }
          }
          
          tuple_traffic->DeleteIfAllowed();
        }
        delete btree_iter;
        delete search_sec_id;
        //////////////////////////////////////////////////////////
    }
    delete br; 
    
    tuple_bus_stop->DeleteIfAllowed();
    
    br_id_list.push_back(br_id); 
    traffic_no.push_back(traffic_num);
    
    Periods* p1 = new Periods(0);
    p1->StartBulkLoad();
    p1->MergeAdd(time_span1);
    p1->EndBulkLoad();
    duration1.push_back(*p1);
    
    delete p1; 
    
    Periods* p2 = new Periods(0);
    p2->StartBulkLoad();
    p2->MergeAdd(time_span2);
    p2->EndBulkLoad();
    duration2.push_back(*p2);
    
    delete p2; 
  }
  
}

/*
create time interval for each night bus
it returns brid, time interval for each schedule  and two intervals
each interval records the first and last schedule time instant of this route 

*/
void RoadDenstiy::SetTSNightBus(int attr1,int attr2, int attr3,
                                Periods* peri1, Periods* peri2)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl;
//  cout<<*peri1<<" "<<*peri2<<endl; 
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
    Periods* time1 = (Periods*)tuple_bus_route->GetAttribute(attr2);
    Periods* time2 = (Periods*)tuple_bus_route->GetAttribute(attr3);
    
    
    Interval<Instant> time_span1;
    Interval<Instant> time_span2;
    
    Interval<Instant> interval1;
    Interval<Instant> interval2;
    
    time1->Get(0, interval1);
    time2->Get(0, interval2);
    
    int h1 = interval1.start.GetHour();
    int h2 = interval2.start.GetHour();
    
    if(h1 > h2){
      double m = 
        CalculateTimeSpan1(time1, peri1,time_span1,i); 
        CalculateTimeSpan2(peri2,time_span2,m); 
    }else{
      double m = 
        CalculateTimeSpan1(time2, peri2,time_span2,i); 
        CalculateTimeSpan2(peri1,time_span1,m); 
    }  

    br_id_list.push_back(br_id);
    
//    duration1.push_back(*time1);
//    duration2.push_back(*time2);

//    cout<<time_span1<<endl;
//    cout<<time_span2<<endl; 
    
    Periods* p1 = new Periods(0);
    p1->StartBulkLoad();
    p1->MergeAdd(time_span1);
    p1->EndBulkLoad();
    duration1.push_back(*p1);
    
    delete p1; 
    
    Periods* p2 = new Periods(0);
    p2->StartBulkLoad();
    p2->MergeAdd(time_span2);
    p2->EndBulkLoad();
    duration2.push_back(*p2);
    
    delete p2; 
    
    time_interval.push_back(1.0/24.0); //1 means 1 day , 1.0/24.0 = 1 hour 
    tuple_bus_route->DeleteIfAllowed();
    
  }  
}


/*
create time interval for each daytime bus
it returns brid, time interval for each schedule  and two intervals
each interval records the first and last schedule time instant of this route 
the time interval for Monday and Sunday is different
the start time for Monday and Sunday is also different 

streets have high density are closeer to city and longer than those low density
streets 

*/
void RoadDenstiy::SetTSDayTimeBus(int attr1,int attr2, int attr3,
                                Periods* peri1, Periods* peri2)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl;
//  cout<<*peri1<<" "<<*peri2<<endl; 
  
  const double factor = 0.3; 
  int num_high_freq = (int)(rel1->GetNoTuples() * factor); 
  
  //////////////////////////////////////////////////////
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
//    cout<<"br_id "<<br_id<<endl; 
    
    Periods* time1 = (Periods*)tuple_bus_route->GetAttribute(attr2);//Monday
    Periods* time2 = (Periods*)tuple_bus_route->GetAttribute(attr3);//Sunday
    double interval = 15.0/(24.0*60.0);//15 minutes 
    
    if(i > num_high_freq)
      interval = 30.0/(24.0*60.0); //30 minutes 
    
    Interval<Instant> time_span1;//For Monday  
//    cout<<"Monday"<<endl; 
    CalculateTimeSpan3(time1, peri1, time_span1,i, interval); 
    
    Interval<Instant> time_span2;//For Sunday 
//    cout<<"Sunday"<<endl; 
//    cout<<*time2<<" "<<*peri2<<endl; 
    CalculateTimeSpan3(time2, peri2, time_span2, i, 2.0*interval); 

    
    /////////////////////////////////////////////////////////////////////////
    br_id_list.push_back(br_id);
    Periods* p1 = new Periods(0);
    p1->StartBulkLoad();
    p1->MergeAdd(time_span1);
    p1->EndBulkLoad();
    duration1.push_back(*p1);
    
    delete p1; 
    
//    cout<<time_span2<<endl; 
    
    Periods* p2 = new Periods(0);
    p2->StartBulkLoad();
    p2->MergeAdd(time_span2);
    p2->EndBulkLoad();
    duration2.push_back(*p2);
    
    delete p2; 
    time_interval.push_back(interval);
    time_interval2.push_back(2.0*interval);
    /////////////////////////////////////////////////////////////////////////
    
    tuple_bus_route->DeleteIfAllowed();
    
  }  
}



/*
calculate the first and end time schedule of bus route 
if the start minute is larger than 30. we reduce the start hour by one 
e.g., 21:57 ---> 20:57 
else keep it e.g., 21:14 --> 21:14 

*/
double RoadDenstiy::CalculateTimeSpan1(Periods* t, Periods* p, 
                                    Interval<Instant>& span,
                                    int index)
{
   const int bti = 60; 
  Interval<Instant> p1;
  Interval<Instant> t1;
  p->Get(0, p1);
  t->Get(0, t1); 
  int m1 = t1.start.GetMinute();
//  cout<<"start minute "<<m1<<endl; 
  if(m1 == 0){
//      m1 = lrand48() % bti;
      m1 = index % bti; 
  }
  
  /////////////set the time for the first schedule //////////////////////////
  Instant start = t1.start;
//  cout<<"old "<<start<<endl; 
  if(m1 > bti/2){  // larger than half an hour 
    start = p1.start; // 1 means 1 day. 1.0/24.0 1 hour 1.0/(24*60) 1 minute
    double d = start.ToDouble() - 1.0/24.0 +  m1*1.0/(24.0*60);
    start.ReadFrom(d);  
  }else{
    start = p1.start;
    double d = start.ToDouble() + m1*1.0/(24.0*60);
    start.ReadFrom(d);  
  }
  
//  cout<<"start "<<start; 
  /////////////set the time for the last schedule///////////////////////////
  
  double d_s = start.ToDouble();
  double d_e = p1.end.ToDouble(); 
  double one_hour = 1/24.0; 
  while(d_s < d_e){
    d_s += one_hour; 
    
  }
  d_s -= one_hour; 
  Instant end = p1.end;//initialize value 
  end.ReadFrom(d_s);
//  cout<<" end "<<end<<endl; 
  
  span.start = start;
  span.lc = true;
  span.end = end;
  span.rc = false; 
  
  return m1*1.0/(24.0*60);//minute e.g., 20, 15 
}


/*
calculate the first and end time schedule of bus route 
use the minute in the last time span plus the hour 0:00 

*/
void RoadDenstiy::CalculateTimeSpan2(Periods* p, 
                                    Interval<Instant>& span, double m)
{
  Interval<Instant> p1;
  p->Get(0, p1);
   
  /////////////set the time for the first schedule //////////////////////////
  Instant start = p1.start;
  double d = start.ToDouble() + m;  
  start.ReadFrom(d);  
  
//  cout<<"start "<<start; 
  /////////////set the time for the last schedule///////////////////////////
  
  double d_s = start.ToDouble();
  double d_e = p1.end.ToDouble(); 
  double one_hour = 1/24.0; 
  while(d_s < d_e){
    d_s += one_hour; 
    
  }
  d_s -= one_hour; 
  Instant end = p1.end;//initialize value 
  end.ReadFrom(d_s);
//  cout<<" end "<<end<<endl; 

  span.start = start;
  span.lc = true;
  span.end = end;
  span.rc = false; 
  
}

/*
calculate the first and end time schedule of daytime bus route 
if the start minute is larger than 30. we reduce the start hour by one 
e.g., 21:57 ---> 20:57 
else keep it e.g., 21:14 --> 21:14 
For Monday   

*/
void RoadDenstiy::CalculateTimeSpan3(Periods* t, Periods* p, 
                                    Interval<Instant>& span,
                                    int index, double interval)
{
  const int bti = 60; 
   
  Interval<Instant> p1;
  Interval<Instant> t1;
  p->Get(0, p1);
  t->Get(0, t1); 
  int m1 = t1.start.GetMinute();
  if(m1 == 0){
      m1 = index % bti; 
  }
  
  /////////////set the time for the first schedule //////////////////////////
  Instant start = t1.start;
//  cout<<"old "<<start<<endl; 
  if(m1 > bti/2){  // larger than half an hour 
    start = p1.start; // 1 means 1 day. 1.0/24.0 1 hour 1.0/(24*60) 1 minute
    double d = start.ToDouble() - 1.0/24.0 +  m1*1.0/(24.0*60);
    start.ReadFrom(d);  
  }else{
    start = p1.start;
    double d = start.ToDouble() + m1*1.0/(24.0*60);
    start.ReadFrom(d);  
  }
  
//  cout<<"start "<<start; 
  /////////////set the time for the last schedule///////////////////////////
  
  double d_s = start.ToDouble();
  double d_e = p1.end.ToDouble(); 
  while(d_s < d_e){
    d_s += interval; 
  }
  d_s -= interval; 
  Instant end = p1.end;//initialize value 
  end.ReadFrom(d_s);
//  cout<<" end "<<end<<endl; 
  
  span.start = start;
  span.lc = true;
  span.end = end;
  span.rc = false; 
  
}

/*
set up the speed value for each bus route 

*/

void RoadDenstiy::SetBRSpeed(int attr1, int attr2, int attr, int attr_sm)
{
  ///////////////collect the startsmaller value for each route//////////
  vector<bool> sm_list;
  for(int i = 1;i <= rel3->GetNoTuples();i+= 2){
    Tuple* tuple_route1 = rel3->GetTuple(i, false);
    bool sm1 = ((CcBool*)tuple_route1->GetAttribute(attr_sm))->GetBoolval();
    sm_list.push_back(sm1);
    Tuple* tuple_route2 = rel3->GetTuple(i + 1, false);
    bool sm2 = ((CcBool*)tuple_route2->GetAttribute(attr_sm))->GetBoolval();
    assert(sm1 == sm2);
    tuple_route1->DeleteIfAllowed();
    tuple_route2->DeleteIfAllowed();
  }
  
  
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr "<<attr<<endl; 
  vector<double> route_speed; 
  //// take the speed limit value from the relation///////////////
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* tuple_route = rel2->GetTuple(i, false);
    double speed_val = ((CcReal*)tuple_route->GetAttribute(attr))->GetRealval();
    route_speed.push_back(speed_val);
    tuple_route->DeleteIfAllowed();
  }
  
//  cout<<"size "<<route_speed.size()<<endl; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    
//    if(i > 3 ) continue; 
    
    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
    GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr2); 
    
    ///////////////collect all road sections/////////////////////////
    ///////////////get the route id and the corresponding speed value////////
    BusRoute* br = new BusRoute(n,NULL,NULL);

    vector<bool> start_from; 
    vector<SectTreeEntry> sec_list; 
    br->GetSectionList(gl, sec_list, start_from);
    
    vector<Pos_Speed> pos_speed1;
    double dist = 0.0; 
    for(unsigned int j = 0;j < sec_list.size();j++){
      dist += fabs(sec_list[j].end - sec_list[j].start);
      double s = route_speed[sec_list[j].rid - 1];
      /////////////we define -1 --- 10km/h //////////////////////////
      ////////the street that is not valid for cars//////////////
      if(s < 0.0) s = 10.0; 
      ///////////////////////////////////////////////////////////
      Pos_Speed* ps = new Pos_Speed(dist, s);
      pos_speed1.push_back(*ps);
      delete ps;
    }  

//    cout<<" size "<<pos_speed1.size()<<endl; 

    delete br; 
    /////////////// merge value /////////////////////////////////
    vector<Pos_Speed> pos_speed2;
    pos_speed2.push_back(pos_speed1[0]);
//    cout<<"before "<<endl; 
    for(unsigned int j = 1;j < pos_speed1.size();j++){
//        pos_speed1[j].Print();
        Pos_Speed ps1 = pos_speed1[j];
        Pos_Speed ps2 = pos_speed2[pos_speed2.size() - 1];
        if(AlmostEqual(ps1.speed_val, ps2.speed_val)){
          pos_speed2[pos_speed2.size() - 1].pos = ps1.pos;
        }else
          pos_speed2.push_back(ps1);
    }
    
//    cout<<"after "<<endl; 

      Line* l = new Line(0);
      gl->Gline2line(l);
      SimpleLine* sl = new SimpleLine(0);
      sl->fromLine(*l);
      
    for(unsigned int j = 0;j < pos_speed2.size();j++){
//      pos_speed2[j].Print();
      br_id_list.push_back(br_id);
      br_pos.push_back(pos_speed2[j].pos);
      speed_limit.push_back(pos_speed2[j].speed_val);

      /////////////get the sub line of the speed//////////////////////////

      SimpleLine* sub_line = new SimpleLine(0);   
      double pos1, pos2; 
      if(j == 0){
        pos1 = 0.0;;
        pos2 = pos_speed2[j].pos; 
      }else{
        pos1 = pos_speed2[j - 1].pos;
        pos2 = pos_speed2[j].pos;
      }
      
      sl->SubLine(pos1, pos2, sm_list[br_id - 1] , *sub_line);
      Line* sub_l = new Line(0);
      sub_line->toLine(*sub_l);
      br_subroute.push_back(*sub_l);
      delete sub_l; 
      delete sub_line;
      /////////////////////////////////////////////////////////////////////
    }  
    delete sl;
    delete l;
    
    //////////////////////////////////////////////////////////////
    tuple_bus_route->DeleteIfAllowed();
  }
  
}

bool CompareBusStop(const BusStop_Ext& bs1, const BusStop_Ext& bs2)
{
  if(bs1.rid < bs2.rid) return true;
    else if(bs1.rid == bs2.rid){
      if(bs1.pos < bs2.pos) return true;
      else if(AlmostEqual(bs1.pos,bs2.pos))return true;
      else return false; 
      
    }else return false; 

}

/*
for each bus route segment, it sets the speed value 

*/
void RoadDenstiy::CreateSegmentSpeed(int attr1, int attr2, int attr3, int attr4,
                          int attr_a, int attr_b)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3
//      <<" attr4 "<<attr4
//      <<" attr_a "<<attr_a<<" attr_b "<<attr_b<<endl; 

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_route = rel1->GetTuple(i, false);
    /////collect br id, line, up down, startSmaller value of each route/////
    int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
    
    
/*    if(br_id != 48){
      tuple_bus_route->DeleteIfAllowed();
      continue; 
    }*/
    
    
    Line* l = (Line*)tuple_bus_route->GetAttribute(attr2);
    bool up_down = 
        ((CcBool*)tuple_bus_route->GetAttribute(attr3))->GetBoolval();
    bool startsmaller = 
        ((CcBool*)tuple_bus_route->GetAttribute(attr4))->GetBoolval();
    
//    cout<<"br_id "<<br_id<<" up_down "<<up_down
//        <<" smaller "<<startsmaller<<endl;
    
    ////////////////get the speed limit of this bus route ///////////////////
    CcInt* search_speed_id = new CcInt(true, br_id);
    BTreeIterator* btree_iter2 = btree_b->ExactMatch(search_speed_id);
    vector<Pos_Speed> br_speed_list; 
    
    while(btree_iter2->Next()){
      Tuple* tuple_bs = rel3->GetTuple(btree_iter2->GetId(), false);
      double bs_pos = 
              ((CcReal*)tuple_bs->GetAttribute(BR_POS))->GetRealval();
      double speed_val = 
              ((CcReal*)tuple_bs->GetAttribute(BR_SPEED))->GetRealval();
      Pos_Speed* pos_speed = new Pos_Speed(bs_pos, speed_val);
      br_speed_list.push_back(*pos_speed);
      delete pos_speed; 
      tuple_bs->DeleteIfAllowed();
    }
      
    
    delete btree_iter2;
    delete search_speed_id;
    
/*    cout<<"speed limit"<<endl; 
    for(unsigned int j = 0;j < br_speed_list.size();j++){
      br_speed_list[j].Print();
    }*/
    
    /////////collect all bus stops of this route/////////////////////////////
    
    CcInt* search_bs_id = new CcInt(true, br_id);
    BTreeIterator* btree_iter1 = btree_a->ExactMatch(search_bs_id);
    vector<BusStop_Ext> bus_stop_list; 
    
    while(btree_iter1->Next()){
        Tuple* tuple_bs = rel2->GetTuple(btree_iter1->GetId(), false);
        double bs_pos = 
              ((CcReal*)tuple_bs->GetAttribute(attr_a))->GetRealval();
        bool direction = 
              ((CcBool*)tuple_bs->GetAttribute(attr_b))->GetBoolval();
        //take bus stops on the same side of the route   
        if(direction == up_down){   
          Point p(true,0.0,0.0);
          BusStop_Ext* bs_ext = new BusStop_Ext(br_id,0,bs_pos,p,direction);
          bus_stop_list.push_back(*bs_ext);
          delete bs_ext; 
        }
        tuple_bs->DeleteIfAllowed();
    }
    delete btree_iter1;
    delete search_bs_id;
    
    sort(bus_stop_list.begin(),bus_stop_list.end(),CompareBusStop);
    
/*    cout<<"bus stop list "<<endl; 
    for(unsigned int j = 0;j < bus_stop_list.size();j++){
      bus_stop_list[j].Print();
    }*/
      
    /////////////////////////////////////////////////////////////////////////
    SimpleLine* sl = new SimpleLine(0);
    sl->fromLine(*l);
      
    
    CalculateRouteSegment(sl, br_speed_list, bus_stop_list, startsmaller);

    delete sl;

    ////////////////////////////////////////////////////////////////////////
    tuple_bus_route->DeleteIfAllowed();
    
  }
  
}

/*
calculate the bus segment combined with speed limit value 
the returned lines are already ordered in the same way as bus stops 
from bus stop id small to big.
It also writes down the start location (point) of each bus route for each
direction 
return segment id (start from 1)
bus stop id (starts from 1)

*/
void RoadDenstiy::CalculateRouteSegment(SimpleLine* sl, 
                                        vector<Pos_Speed> br_speed_list, 
                             vector<BusStop_Ext> bus_stop_list, bool sm)
{
  BusStop_Ext last_stop = bus_stop_list[0];
//  last_stop.Print();
  
  unsigned int index_speed = 0; 
//  cout<<"length "<<sl->Length()<<endl; 
  
  vector<SimpleLine> sub_line_list; 
  vector<double> speed_val_list; 
  vector<int> bus_segment_id_list; 
  
  for(unsigned int i = 1;i < bus_stop_list.size();i++){
      BusStop_Ext cur_stop = bus_stop_list[i];
      
//      cur_stop.Print();
      
      ////////////advance the index ///////////////////////////
      while(br_speed_list[index_speed].pos < last_stop.pos ||
            AlmostEqual(br_speed_list[index_speed].pos,last_stop.pos))
        index_speed++;
      
      //////////////////////////////////////////////////////////
      if(cur_stop.pos < br_speed_list[index_speed].pos || 
          AlmostEqual(cur_stop.pos,br_speed_list[index_speed].pos)){
          SimpleLine* sub_line1 = new SimpleLine(0);
          sl->SubLine(last_stop.pos,cur_stop.pos,sm,*sub_line1);
//          cout<<"sub length1 "<<sub_line1->Length()<<endl;
          sub_line_list.push_back(*sub_line1);
          delete sub_line1; 
          speed_val_list.push_back(br_speed_list[index_speed].speed_val);
          bus_segment_id_list.push_back(i);
          
      }else{
          //between last stop and cur stop 
          assert(last_stop.pos < br_speed_list[index_speed].pos &&
                 br_speed_list[index_speed].pos < cur_stop.pos); 
                 
          SimpleLine* sub_line2 = new SimpleLine(0);
          sl->SubLine(last_stop.pos,br_speed_list[index_speed].pos,
                      sm,*sub_line2);
//          cout<<"sub length2 "<<sub_line2->Length()<<endl;
          sub_line_list.push_back(*sub_line2);
          delete sub_line2; 
          speed_val_list.push_back(br_speed_list[index_speed].speed_val);
          bus_segment_id_list.push_back(i); 
          
          
          unsigned int index_speed_tmp = index_speed + 1;
          while(index_speed_tmp < br_speed_list.size() && 
                br_speed_list[index_speed_tmp].pos < cur_stop.pos){
          
            SimpleLine* sub_line3 = new SimpleLine(0);
            sl->SubLine(br_speed_list[index_speed].pos,
                      br_speed_list[index_speed_tmp].pos,
                      sm,*sub_line3);
//            cout<<"sub length3 "<<sub_line3->Length()<<endl;
            sub_line_list.push_back(*sub_line3);
            delete sub_line3; 
            speed_val_list.push_back(br_speed_list[index_speed_tmp].speed_val);

            bus_segment_id_list.push_back(i); 
            
            index_speed = index_speed_tmp;
            index_speed_tmp = index_speed + 1;
          }

          SimpleLine* sub_line4 = new SimpleLine(0);
          sl->SubLine(br_speed_list[index_speed].pos,cur_stop.pos, 
                      sm,*sub_line4);
//          cout<<"sub length4 "<<sub_line4->Length()<<endl;
          sub_line_list.push_back(*sub_line4);
          delete sub_line4; 
          
          if(index_speed_tmp < br_speed_list.size())
            speed_val_list.push_back(br_speed_list[index_speed_tmp].speed_val);
          else
            speed_val_list.push_back(br_speed_list[index_speed].speed_val);
          
          bus_segment_id_list.push_back(i); 
          
      }
      last_stop = cur_stop; 
  }

  BusStop_Ext temp_stop = bus_stop_list[0];
  assert(speed_val_list.size() == sub_line_list.size());
  assert(speed_val_list.size() == bus_segment_id_list.size());
  
  ////////////get start location ///////////////////
  Point sp;
  if(temp_stop.start_small){ //up and down bus stop
    assert(sl->AtPosition(temp_stop.pos,sm,sp));
  }else{
    BusStop_Ext temp_stop_end = bus_stop_list[bus_stop_list.size() - 1];
    assert(sl->AtPosition(temp_stop_end.pos,sm,sp));
  }
    
  //////////////////////////////////////////////////
  for(unsigned int i = 0;i < sub_line_list.size();i++){
    br_id_list.push_back(temp_stop.br_id);
    br_direction.push_back(temp_stop.start_small);//!!actually up and down 
    
    Line* temp_l = new Line(0);
    sub_line_list[i].toLine(*temp_l);
    br_subroute.push_back(*temp_l);
    delete temp_l; 


    speed_limit.push_back(speed_val_list[i]);
    startSmaller.push_back(sm); 
    
    //write down the location of first bus stop 
    start_loc_list.push_back(sp); 
    
     //segmend id starts from 1 
    segment_id_list.push_back(bus_segment_id_list[i]);
  }  
  
  
}

/*
create night moving bus. it has two time intervals 0-5 21-24 

*/
void RoadDenstiy::CreateNightBus()
{
  for(int i = 1; i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_ts = rel1->GetTuple(i, false);
    int br_id= ((CcInt*)tuple_bus_ts->GetAttribute(BR_ID2))->GetIntval();
    
//    if(br_id != 41){
//        tuple_bus_ts->DeleteIfAllowed();
//        continue; 
//    }

    Periods* peri1 = (Periods*)tuple_bus_ts->GetAttribute(DURATION1);
    Periods* peri2 = (Periods*)tuple_bus_ts->GetAttribute(DURATION2);
    double time_interval = 
          ((CcReal*)tuple_bus_ts->GetAttribute(BR_INTERVAL))->GetRealval();

//    cout<<"br_id "<<br_id<<" Periods1 "<<*peri1
//        <<" Periods2 "<<*peri2<<" interval "<<time_interval<<endl; 

    CreateMovingBus(br_id,peri1,time_interval, false);    
    CreateMovingBus(br_id,peri2,time_interval, false);    
    tuple_bus_ts->DeleteIfAllowed();
    
//    break; 
  }

}

/*
create moving objects during the time interval.peri
actually, it creates a set of moving objects. the start time deviation 
between each two consequent schedule is time interval 
70 km h 1166.67 m minute  19.44 m second 
50 km h 8333.3m minute  13.89 m second 
30 km h 500 m minute  8.33 m second 
10 km h 166.67 m minute 2.78 m second 

*/
void RoadDenstiy::CreateMovingBus(int br_id,Periods* peri,
                                  double time_interval, bool daytime)
{
    
    CreateUp(br_id, peri, time_interval, daytime);
    CreateDown(br_id, peri, time_interval, daytime);
}

/*
create moving bus in up direction 

*/
void RoadDenstiy::CreateUp(int br_id, Periods* peri, 
                           double time_interval, bool daytime)
{
//  cout<<"create bus trip: route "<<br_id<<" up "<<endl; 
      
  vector<SimpleLine> sub_sline_list; 
  vector<double> speed_list; 
  vector<int> seg_id_list; 
  
  //////////////////////collect all bus segments of up direction//////////////
  CcInt* search_bs_id = new CcInt(true, br_id);
  BTreeIterator* btree_iter = btree->ExactMatch(search_bs_id);
  Point start_p; 
  while(btree_iter->Next()){
      Tuple* tuple_bs = rel2->GetTuple(btree_iter->GetId(), false);
      bool direction = 
            ((CcBool*)tuple_bs->GetAttribute(BUS_DIRECTION))->GetBoolval();
      if(direction){   //up direction 
        Line* l = (Line*)tuple_bs->GetAttribute(SUB_ROUTE_LINE);
        double speed_val = 
            ((CcReal*)tuple_bs->GetAttribute(SPEED_LIMIT))->GetRealval();
        SimpleLine* sl = new SimpleLine(0);
        sl->fromLine(*l);
        sub_sline_list.push_back(*sl);
        delete sl; 
        speed_list.push_back(speed_val); 
  
        int segid = ((CcInt*)tuple_bs->GetAttribute(SEGMENT_ID))->GetIntval();
        seg_id_list.push_back(segid); 

        ////get the start point/// 
        Point* p = (Point*)tuple_bs->GetAttribute(START_LOC);
        start_p = *p; 
      }
      tuple_bs->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_bs_id;
  

  /////////////////create one bus trip movement///////////////////////////////
  const double dist_delta = 0.01; 
  MPoint* bus_mo = new MPoint(0);
  bus_mo->StartBulkLoad();
  Interval<Instant> bus_periods;
  peri->Get(0, bus_periods);
  
//  cout<<"seg_id_list size "<<seg_id_list.size()
//      <<"sub sline size "<<sub_sline_list.size()<<endl; 
      
//  for(unsigned int i = 0;i < seg_id_list.size();i++)
//    cout<<"seg id "<<seg_id_list[i]<<endl; 
  
  
//  int last_seg_id = seg_id_list[0];
  int last_seg_id = 0; //initialize, real segment starts from 1. 
  UPoint last_upoint; 
  
  for(unsigned int i = 0;i < sub_sline_list.size();i++){
      SpacePartition* sp = new SpacePartition();
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
//      cout<<sub_sline_list[i].Length()<<endl; 
//      cout<<"i "<<i<<"last_seg_id "<<last_seg_id
//         <<"cur seg id "<<seg_id_list[i]<<endl; 
          
      if(!(sub_sline_list[i].Length() > 0.0)){
          delete sp;
          continue; 
      }
      sp->ReorderLine(&sub_sline_list[i], seq_halfseg);
      assert(seq_halfseg.size() > 0);

      Point temp_sp1 = seq_halfseg[0].from; 
      Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 

//      cout<<"last start p "<<start_p
//          <<"endpoint1 "<<temp_sp1
//          <<" endpoint2 "<<temp_sp2<<endl; 

      bool start;
      double real_speed = speed_list[i] * 1000 / 60;// convert to meter minute 
      if(start_p.Distance(temp_sp1) < dist_delta){
        ////////moving bus moves to the bus stop and wait for a while//////
        if(last_seg_id != seg_id_list[i]){
//            cout<<"trip 1"
//                <<"last_seg_id "<<last_seg_id
//                <<"seg_id_list[i] "<<seg_id_list[i]
//                <<"i "<<i<<endl;
                
            last_seg_id = seg_id_list[i]; 
            Interval<Instant> up_interval; 
            up_interval.start = bus_periods.start;
            up_interval.lc = true;
            Instant newend = bus_periods.start; 
            ////////////////pause for 30 seconds /////////////////////////////
            newend.ReadFrom(bus_periods.start.ToDouble() + 30.0/(24.0*60*60)); 
            up_interval.end = newend; 
            up_interval.rc = false; 
            UPoint* up = new UPoint(up_interval,temp_sp1,temp_sp1);
            bus_mo->Add(*up);
            
            delete up; 
            bus_periods.start = newend; 
        }
        
        start = true;
        start_p = temp_sp2; 
        CreateBusTrip1(bus_mo, seq_halfseg, bus_periods.start, 
                       real_speed, last_upoint);
      }else if(start_p.Distance(temp_sp2) < dist_delta){
        ////////moving bus moves to the bus stop and wait for a while//////
        if(last_seg_id != seg_id_list[i]){
//          cout<<"trip 2"
//                <<"last_seg_id "<<last_seg_id
//                <<"seg_id_list[i] "<<seg_id_list[i]
//                <<"i "<<i<<endl;
                
            last_seg_id = seg_id_list[i]; 
            Interval<Instant> up_interval; 
            up_interval.start = bus_periods.start;
            up_interval.lc = true;
            Instant newend = bus_periods.start; 
            ////////////////pause for 30 seconds //////////////////////////
            newend.ReadFrom(bus_periods.start.ToDouble() + 30.0/(24.0*60*60)); 
            up_interval.end = newend; 
            up_interval.rc = false; 
            UPoint* up = new UPoint(up_interval,temp_sp2,temp_sp2);
            bus_mo->Add(*up);
            last_upoint = *up; 
            delete up; 
            bus_periods.start = newend; 
        }
        start = false;
        start_p = temp_sp1; 
        CreateBusTrip2(bus_mo, seq_halfseg, bus_periods.start, 
                       real_speed, last_upoint);
      }else assert(false);

      delete sp; 
  }
  ////////////////add 30 seconds wait for the last bus stop/////////////////
  Instant new_end = last_upoint.timeInterval.end;
  last_upoint.timeInterval.start = new_end; 
  new_end.ReadFrom(new_end.ToDouble() + 30.0/(24.0*60*60));
  last_upoint.timeInterval.end = new_end; 
  last_upoint.p0 = last_upoint.p1; 
  bus_mo->Add(last_upoint); 
  /////////////////////////////////////////////////////////////////////
  bus_mo->EndBulkLoad(); 
  ///////////////////////////add to the result ///////////////////////
  br_id_list.push_back(br_id);
  br_direction.push_back(true); //UP direction
  bus_trip.push_back(*bus_mo); 
  ////////////////////////////////////////////////////////////////////
  AddTypeandDay(bus_mo,daytime); 
  //////////////////////////////////////////////////////////////////
  int schedule_id = 1; 
  schedule_id_list.push_back(schedule_id);
  schedule_id++;
  //////////////////copy the trip by time interval//////////////////////////
  CopyBusTrip(br_id, true, bus_mo, peri, time_interval,daytime,schedule_id);
  //////////////////////////////////////////////////////////////////////////
  delete bus_mo; 
}

/*
create moving bus units and add them into the trip, 
it travers from index small to big in myhalfsegment list 
use the maxspeed as bus speed 

*/

void RoadDenstiy::CreateBusTrip1(MPoint* mo,
                                vector<MyHalfSegment> seq_halfseg, 
                                Instant& start_time,
                                double speed_val, UPoint& last_up)
{
  
//  cout<<"trip1 max speed "<<speed_val<<" start time "<<start_time<<endl;
  
  const double dist_delta = 0.01; 
  
  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 
  for(unsigned int i = 0;i < seq_halfseg.size();i++){
    Point from_loc = seq_halfseg[i].from;
    Point to_loc = seq_halfseg[i].to; 
    
    double dist = from_loc.Distance(to_loc);
    double time = dist/speed_val; 
    
 //   cout<<"dist "<<dist<<" time "<<time<<endl; 
//    printf("%.10f, %.10f",dist, time); 
    //////////////////////////////////////////////////////////////
    if(dist < dist_delta){//ignore such small segment 
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i+1].from = from_loc; 
        }
        continue; 
    }
    /////////////////////////////////////////////////////////////////
    
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0));//double 1.0 means 1 day 
//    cout<<st<<" "<<et<<endl;
    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up = new UPoint(up_interval,from_loc,to_loc);
//    cout<<*up<<endl; 
    mo->Add(*up);
    last_up = *up; 
    delete up; 
    //////////////////////////////////////////////////////////////
    st = et; 
  }
  
  start_time = et; 
  
}
                     
/*
create moving bus units and add them into the trip, 
!!! it traverse from index big to small in myhalfsegment list 
use the maxspeed as bus speed 

*/
void RoadDenstiy::CreateBusTrip2(MPoint* mo,
                                vector<MyHalfSegment> seq_halfseg, 
                                Instant& start_time,
                                double speed_val, UPoint& last_up)
{
  const double dist_delta = 0.01; 
//  cout<<"trip2 max speed "<<speed_val<<" start time "<<start_time<<endl; 
  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 
  for(int i = seq_halfseg.size() - 1;i >= 0;i--){
    Point from_loc = seq_halfseg[i].to;
    Point to_loc = seq_halfseg[i].from; 
    double dist = from_loc.Distance(to_loc);
    double time = dist/speed_val; 

 //   cout<<"dist "<<dist<<" time "<<time<<endl; 
    ///////////////////////////////////////////////////////////////////
    if(dist < dist_delta){//ignore such small segment 
        if((i + 1) < seq_halfseg.size()){
          seq_halfseg[i+1].from = from_loc; 
        }
        continue; 
    }
    
    et.ReadFrom(st.ToDouble() + time*1.0/(24.0*60.0));//double 1.0 means 1 day 
//    cout<<st<<" "<<et<<endl;
    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up = new UPoint(up_interval,from_loc,to_loc);
    mo->Add(*up);
    last_up = *up; 
    delete up; 
    //////////////////////////////////////////////////////////////
    st = et; 
  }
  
  start_time = et; 
}



/*
create moving bus in down direction.
the bus moves from large bus stop id to small bus stop id 

*/
void RoadDenstiy::CreateDown(int br_id, Periods* peri, 
                             double time_interval, bool daytime)
{

//  cout<<"create bus trip: route "<<br_id<<" down "<<endl; 

  vector<SimpleLine> sub_sline_list; 
  vector<double> speed_list; 
  vector<int> seg_id_list; 
  
  //////////////////////collect all bus segments of up direction//////////////
  CcInt* search_bs_id = new CcInt(true, br_id);
  BTreeIterator* btree_iter = btree->ExactMatch(search_bs_id);
  Point start_p; 
  while(btree_iter->Next()){
      Tuple* tuple_bs = rel2->GetTuple(btree_iter->GetId(), false);
      bool direction = 
            ((CcBool*)tuple_bs->GetAttribute(BUS_DIRECTION))->GetBoolval();
      if(direction == false){   //down direction 
        Line* l = (Line*)tuple_bs->GetAttribute(SUB_ROUTE_LINE);
        double speed_val = 
            ((CcReal*)tuple_bs->GetAttribute(SPEED_LIMIT))->GetRealval();
        SimpleLine* sl = new SimpleLine(0);
        sl->fromLine(*l);
        sub_sline_list.push_back(*sl);
        delete sl; 
        speed_list.push_back(speed_val); 
  
        int segid = ((CcInt*)tuple_bs->GetAttribute(SEGMENT_ID))->GetIntval();
        seg_id_list.push_back(segid); 

        ////get the start point/// 
        Point* p = (Point*)tuple_bs->GetAttribute(START_LOC);
        start_p = *p; 
      }
      tuple_bs->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_bs_id;
  
  
  /////////////////create one bus trip movement///////////////////////////////
  const double dist_delta = 0.01; 
  MPoint* bus_mo = new MPoint(0);
  bus_mo->StartBulkLoad();
  Interval<Instant> bus_periods;
  peri->Get(0, bus_periods);
  
//  cout<<"seg_id_list size "<<seg_id_list.size()
//      <<"sub sline size "<<sub_sline_list.size()<<endl; 
      
//  for(unsigned int i = 0;i < seg_id_list.size();i++)
//    cout<<"seg id "<<seg_id_list[i]<<endl; 

  UPoint last_upoint; 
//  int last_seg_id = seg_id_list[seg_id_list.size() - 1]; //from the last 
  int last_seg_id = 0; //initialize, real segment starts from 1 

  for(int i = sub_sline_list.size() - 1;i >= 0 ;i--){
      SpacePartition* sp = new SpacePartition();
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
//      cout<<sub_sline_list[i].Length()<<endl; 
//      cout<<"i "<<i<<"last_seg_id "<<last_seg_id
//          <<" cur seg id "<<seg_id_list[i]
//          <<"length "<<sub_sline_list[i].Length()<<endl; 

      if(!(sub_sline_list[i].Length() > 0.0)){
          delete sp;
          continue; 
      }

      sp->ReorderLine(&sub_sline_list[i], seq_halfseg);
      assert(seq_halfseg.size() > 0);

      Point temp_sp1 = seq_halfseg[0].from; 
      Point temp_sp2 = seq_halfseg[seq_halfseg.size() - 1].to; 

      double real_speed = speed_list[i] * 1000 / 60;// convert to meter minute 
      if(start_p.Distance(temp_sp1) < dist_delta){
        ////////moving bus moves to the bus stop and wait for a while//////
        if(last_seg_id != seg_id_list[i]){
//            cout<<"trip 1"
//                <<"last_seg_id "<<last_seg_id
//                <<"seg_id_list[i] "<<seg_id_list[i]
//                <<"i "<<i<<endl;
                
            last_seg_id = seg_id_list[i]; 
            Interval<Instant> up_interval; 
            up_interval.start = bus_periods.start;
            up_interval.lc = true;
            Instant newend = bus_periods.start; 
            ////////////////pause for 30 seconds /////////////////////////////
            newend.ReadFrom(bus_periods.start.ToDouble() + 30.0/(24.0*60*60)); 
            up_interval.end = newend; 
            up_interval.rc = false; 
            UPoint* up = new UPoint(up_interval,temp_sp1,temp_sp1);
            bus_mo->Add(*up);
            
            delete up; 
            bus_periods.start = newend; 
        }
        
        
        start_p = temp_sp2; 
        CreateBusTrip1(bus_mo, seq_halfseg, bus_periods.start, 
                       real_speed, last_upoint);
      }else if(start_p.Distance(temp_sp2) < dist_delta){
        ////////moving bus moves to the bus stop and wait for a while//////
        if(last_seg_id != seg_id_list[i]){
//          cout<<"trip 2"
//                <<"last_seg_id "<<last_seg_id
//                <<"seg_id_list[i] "<<seg_id_list[i]
//                <<"i "<<i<<endl;
                
            last_seg_id = seg_id_list[i]; 
            Interval<Instant> up_interval; 
            up_interval.start = bus_periods.start;
            up_interval.lc = true;
            Instant newend = bus_periods.start; 
            ////////////////pause for 30 seconds //////////////////////////
            newend.ReadFrom(bus_periods.start.ToDouble() + 30.0/(24.0*60*60)); 
            up_interval.end = newend; 
            up_interval.rc = false; 
            UPoint* up = new UPoint(up_interval,temp_sp2,temp_sp2);
            bus_mo->Add(*up);
            
            delete up; 
            bus_periods.start = newend; 
        }
        
        start_p = temp_sp1; 
        CreateBusTrip2(bus_mo, seq_halfseg, bus_periods.start, 
                       real_speed, last_upoint);
      }else assert(false);

      delete sp; 
  }
  ////////////////add 30 seconds wait for the last bus stop/////////////////
  Instant new_end = last_upoint.timeInterval.end;
  last_upoint.timeInterval.start = new_end; 
  new_end.ReadFrom(new_end.ToDouble() + 30.0/(24.0*60*60));
  last_upoint.timeInterval.end = new_end; 
  last_upoint.p0 = last_upoint.p1; 
  bus_mo->Add(last_upoint); 
  /////////////////////////////////////////////////////////////////////////
  bus_mo->EndBulkLoad(); 
  ///////////////////////////add to the result ///////////////////////
  br_id_list.push_back(br_id);
  br_direction.push_back(false); //DOWN direction
  bus_trip.push_back(*bus_mo); 
  ////////////////////////////////////////////////////////////////////
  AddTypeandDay(bus_mo, daytime);
  ///////////////////////add schedule counter/////////////////////////////
  int schedule_id = 1; 
  schedule_id_list.push_back(schedule_id);
  schedule_id++;
  //////////////////copy the trip by time interval///////////////////////////
  CopyBusTrip(br_id, false, bus_mo, peri, time_interval,daytime,schedule_id); 
  ///////////////////////////////////////////////////////////////////////////
  delete bus_mo; 
}

/*
create a moving bus trip for every time interval 

*/
void RoadDenstiy::CopyBusTrip(int br_id,bool direction, 
                              MPoint* mo, Periods* peri, 
                              double time_interval, bool daytime, 
                              int schedule_id)
{
//  cout<<"periods "<<*peri<<endl; 
  
  Interval<Instant> bus_periods;
  peri->Get(0, bus_periods);
  Instant st = bus_periods.start;
  st.ReadFrom(st.ToDouble() + time_interval);
  /////////////calculate the number of copies /////////////////////
  unsigned int copy_no = 0; 
  while(st <= bus_periods.end){
      copy_no++;
//      cout<<"new st "<<st<<endl; 
      st.ReadFrom(st.ToDouble() + time_interval);
  }
  
  
  for(unsigned int i = 0;i < copy_no;i++){
        MPoint* mo1 = new MPoint(0);
        mo1->StartBulkLoad();
        for(int j = 0;j < mo->GetNoComponents();j++){
          UPoint up1;
          mo->Get(j, up1);
          Interval<Instant> up_interval;
          
          Instant st = up1.timeInterval.start;
          st.ReadFrom(st.ToDouble() + (i+1)*time_interval);
          up_interval.start = st;
          up_interval.lc = up1.timeInterval.lc; 
          
          
          Instant et = up1.timeInterval.end;
          et.ReadFrom(et.ToDouble() + (i+1)*time_interval);
          up_interval.end = et;
          up_interval.rc = up1.timeInterval.rc; 
          
          
          UPoint* up2 = new UPoint(up_interval, up1.p0,up1.p1);
          mo1->Add(*up2); 
          delete up2; 
        }
        mo1->EndBulkLoad();
        
        br_id_list.push_back(br_id);
        br_direction.push_back(direction); 
        bus_trip.push_back(*mo1); 
        ////////Sunday,Monday,Night,Daytime////////////////////
        AddTypeandDay(mo1, daytime); 
        /////////Schedule ID//////////////////////////////////
        schedule_id_list.push_back(schedule_id);
        schedule_id++;
        ////////////////////////////////////////////////////////
        delete mo1; 
  }
  //////////////////////////copy the trip one day more//////////////////////
  if(daytime == false){ //only for night bus 
    for(unsigned int i = 0;i < copy_no + 1;i++){
        MPoint* mo1 = new MPoint(0);
        mo1->StartBulkLoad();
        for(int j = 0;j < mo->GetNoComponents();j++){
          UPoint up1;
          mo->Get(j, up1);
          Interval<Instant> up_interval;
          
          Instant st = up1.timeInterval.start;
          st.ReadFrom(st.ToDouble() + i*time_interval + 1.0);//1 day more 
          up_interval.start = st;
          up_interval.lc = up1.timeInterval.lc; 
          
          
          Instant et = up1.timeInterval.end;
          et.ReadFrom(et.ToDouble() + i*time_interval + 1.0);//1 day more 
          up_interval.end = et;
          up_interval.rc = up1.timeInterval.rc; 
          
          
          UPoint* up2 = new UPoint(up_interval, up1.p0,up1.p1);
          mo1->Add(*up2); 
          delete up2; 
        }
        mo1->EndBulkLoad();
        
        br_id_list.push_back(br_id);
        br_direction.push_back(direction); 
        bus_trip.push_back(*mo1); 
        //////////Sunday,Monday,Night,Daytime///////////////////////
        AddTypeandDay(mo1, daytime);
        /////////Schedule ID////////////////////
        schedule_id_list.push_back(schedule_id);
        schedule_id++;
        //////////////////////////////////////////////
        delete mo1; 
    }
  }
}

/*
add type of the bus: night or daytime 
add whether it is Monday, Sunday or ...

*/
void RoadDenstiy::AddTypeandDay(MPoint* mo, bool daytime)
{
 
  if(daytime)
    trip_type.push_back("daytime");
  else
    trip_type.push_back("night");
 
  assert(mo->GetNoComponents() > 0);
  UPoint up; 
  mo->Get(0, up);
  Instant st = up.timeInterval.start; 
  int d = st.GetWeekday();//0--Monday 
//  cout<<"start time "<<st<<"d "<<d<<endl; 
  string str;
  switch(d){
    case 0: str = "Monday";
            break;
    case 1: str = "Tuesday";
            break;
    case 2: str = "Wednesday";
            break;
    case 3: str = "Thursday";
            break;
    case 4: str = "Friday";
            break;
    case 5: str = "Saturday";
            break;
    case 6: str = "Sunday";
            break;
    default:str = " ";
            break; 
  }
  trip_day.push_back(str); 
}

/*
create daytime moving bus. it has two types of movement: Monday and Sunday 
different time intervals 

*/

void RoadDenstiy::CreateDayTimeBus()
{

  for(int i = 1; i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_ts = rel1->GetTuple(i, false);
    int br_id= ((CcInt*)tuple_bus_ts->GetAttribute(BR_ID3))->GetIntval();
    
//    cout<<"br_id "<<br_id<<endl; 
    
//    if(br_id != 1){
//        tuple_bus_ts->DeleteIfAllowed();
//        continue; 
//    }

    Periods* peri1 = (Periods*)tuple_bus_ts->GetAttribute(DURATION1);
    Periods* peri2 = (Periods*)tuple_bus_ts->GetAttribute(DURATION2);
    double time_interval1 = 
          ((CcReal*)tuple_bus_ts->GetAttribute(BR_INTERVAL1))->GetRealval();
    double time_interval2 = 
          ((CcReal*)tuple_bus_ts->GetAttribute(BR_INTERVAL2))->GetRealval();
    
//    cout<<"br_id "<<br_id<<" Periods1 "<<*peri1
//        <<" Periods2 "<<*peri2<<" interval "<<time_interval<<endl; 

    CreateMovingBus(br_id, peri1, time_interval1, true);    
    CreateMovingBus(br_id, peri2, time_interval2, true);    
    tuple_bus_ts->DeleteIfAllowed();

  }
}

/*
create time table at each spatial location. several bus stops from different
bus routes can locate at the same spatial location 

*/
void RoadDenstiy::CreateTimeTable()
{
  ////////////collect all bus stops /////////////////////////////////
  vector<BusStop_Ext> bus_stop_list;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_stop = rel1->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(BR_ID4))->GetIntval();
    int stop_id = 
        ((CcInt*)tuple_bus_stop->GetAttribute(BUS_STOP_ID))->GetIntval();
    Point* loc = (Point*)tuple_bus_stop->GetAttribute(BUS_LOC);
    bool start_small = 
          ((CcBool*)tuple_bus_stop->GetAttribute(STOP_DIRECTION))->GetBoolval();

    BusStop_Ext* bse = new BusStop_Ext(br_id,stop_id,0,*loc,start_small);
    bus_stop_list.push_back(*bse);
    delete bse; 
    tuple_bus_stop->DeleteIfAllowed();
  }    
  
  sort(bus_stop_list.begin(), bus_stop_list.end());
//  cout<<"bus_stop_list size "<<bus_stop_list.size()<<endl; 
  
  const double dist_delta = 0.01; 
  unsigned int temp_count = 1;
  for(unsigned int i = 0;i < bus_stop_list.size();i++){
    vector<BusStop_Ext> bus_stop_list_new; 
  
    bus_stop_list_new.push_back(bus_stop_list[i]);
  
    ////////collect all bus stops mapping to the same 2D point in space/////
    unsigned int j = i + 1;
    BusStop_Ext bse = bus_stop_list_new[0]; 
//    bse.Print();
    
    while(j < bus_stop_list.size() &&
        bus_stop_list[j].loc.Distance(bse.loc) < dist_delta ){
        bus_stop_list_new.push_back(bus_stop_list[j]);
        j++; 
    }
    i = j - 1; 
    ///////////////////process bus stop list new ///////////////////////////
//    cout<<"sub size "<<bus_stop_list_new.size()<<endl; 
    CreateLocTable(bus_stop_list_new,temp_count);
    
    temp_count++; 
//    if(temp_count > 2)break; 
  }  
}

/*
create time table for such spatial location 

*/
void RoadDenstiy::CreateLocTable(vector<BusStop_Ext> bus_stop_list_new, 
                                 int count_id)
{
  Point loc = bus_stop_list_new[0].loc; 
  
//  cout<<"bus stop location "<<loc<<endl; 
  
  const double dist_delta = 0.01; 
  const double stop_time = 30.0/(24.0*60.0*60.0); //30 seconds 
  
  for(unsigned int i = 0;i < bus_stop_list_new.size();i++){
      int br_id = bus_stop_list_new[i].br_id;
      bool direction = bus_stop_list_new[i].start_small;
      int stop_id = bus_stop_list_new[i].br_stop_id;
      /////use btree to find all bus trips of this route ///////
     CcInt* search_trip_id = new CcInt(true, br_id);
     BTreeIterator* btree_iter = btree->ExactMatch(search_trip_id);
     while(btree_iter->Next()){
        Tuple* tuple_trip = rel2->GetTuple(btree_iter->GetId(), false);
        int br_trip_id = 
            ((CcInt*)tuple_trip->GetAttribute(BR_ID5))->GetIntval();
        bool trip_direction = 
            ((CcBool*)tuple_trip->GetAttribute(MO_BUS_DIRECTION))->GetBoolval();
        assert(br_id == br_trip_id);
        if(direction == trip_direction){
          MPoint* mo = (MPoint*)tuple_trip->GetAttribute(BUS_TRIP);
          string bus_day = 
            ((CcString*)tuple_trip->GetAttribute(BUS_DAY))->GetValue();
//          cout<<*mo<<endl; 
          ////////////////////traverse the trip to find the point/////////////  
          int j = 0;
          for(;j < mo->GetNoComponents();j++){
            UPoint up;
            mo->Get(j, up);
            Point loc1 = up.p0;
            Point loc2 = up.p1; 
//            cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl; 
            bool find = false; 
            Instant schedule_t; 
            if(loc1.Distance(loc2) < dist_delta && 
               loc1.Distance(loc) < dist_delta){ //find the place 
              Instant st = up.timeInterval.start;
              Instant et = up.timeInterval.end; 
              double d_st = st.ToDouble();
              double d_et = et.ToDouble();
              assert(AlmostEqual(fabs(d_st-d_et), stop_time));//check 30 seconds
              schedule_t = st; 
              find = true;
            }  
            //////////after add waiting 30 seconds for the last stop //////
            //////////revmoe the following code  //////////////////////////
/*            if(j == mo->GetNoComponents() - 1 &&  //end location 
               loc2.Distance(loc) < dist_delta){
               find = true;
               schedule_t = up.timeInterval.end; 
            }
            /////////////////////////////////////////////////////////////////
            if(j == 0 && loc1.Distance(loc) < dist_delta){//start location 
               find = true;
               schedule_t = up.timeInterval.start; 
            }*/
            /////////////////////add result////////////////////////
            if(find){
              bus_stop_loc.push_back(loc);
              br_id_list.push_back(br_id);
              br_direction.push_back(direction);
              trip_day.push_back(bus_day);
              bus_stop_id_list.push_back(stop_id);
              
              ///////////////cut second and millisecond value/////////////////
              int second_val = schedule_t.GetSecond(); 
              int msecond_val = schedule_t.GetMillisecond(); 
              double double_s = schedule_t.ToDouble() - 
                                second_val/(24.0*60.0*60.0) -
                                msecond_val/(24.0*60.0*60.0*1000.0);
              
              schedule_t.ReadFrom(double_s);
              schedule_time.push_back(schedule_t);
              unique_id_list.push_back(count_id);
              break; 
            }
          }  
          assert(j != mo->GetNoComponents());

        }
        tuple_trip->DeleteIfAllowed();
     }
     delete btree_iter;
     delete search_trip_id;
    ////////////////////////////////////////////////////////////////////
  }
}

/*
create time table at each spatial location. several bus stops from different
bus routes can locate at the same spatial location 
Compact Storage:
loc:point lineid:int stopid:int direction:bool deftime:periods
locid:int scheduleinterval:double 

for one route, one direction, one stop, it has at most four tuples
night1 night2 sunday monday 

or only two tuples: sunday mondy 

*/
void RoadDenstiy::CreateTimeTable_Compact(Periods* peri1, Periods* peri2)
{
  ////////////collect all bus stops /////////////////////////////////
  vector<BusStop_Ext> bus_stop_list;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_bus_stop = rel1->GetTuple(i, false);
    int br_id = ((CcInt*)tuple_bus_stop->GetAttribute(BR_ID4))->GetIntval();
    int stop_id = 
        ((CcInt*)tuple_bus_stop->GetAttribute(BUS_STOP_ID))->GetIntval();
    Point* loc = (Point*)tuple_bus_stop->GetAttribute(BUS_LOC);
    bool start_small = 
          ((CcBool*)tuple_bus_stop->GetAttribute(STOP_DIRECTION))->GetBoolval();

    BusStop_Ext* bse = new BusStop_Ext(br_id,stop_id,0,*loc,start_small);
    bus_stop_list.push_back(*bse);
    delete bse; 
    tuple_bus_stop->DeleteIfAllowed();
  }    
  
  sort(bus_stop_list.begin(), bus_stop_list.end());
//  cout<<"bus_stop_list size "<<bus_stop_list.size()<<endl; 
  
  const double dist_delta = 0.01; 
  unsigned int temp_count = 1;
  for(unsigned int i = 0;i < bus_stop_list.size();i++){
    vector<BusStop_Ext> bus_stop_list_new; 
  
    bus_stop_list_new.push_back(bus_stop_list[i]);
  
    ////////collect all bus stops mapping to the same 2D point in space/////
    unsigned int j = i + 1;
    BusStop_Ext bse = bus_stop_list_new[0]; 
//    bse.Print();
    
    while(j < bus_stop_list.size() &&
        bus_stop_list[j].loc.Distance(bse.loc) < dist_delta ){
        bus_stop_list_new.push_back(bus_stop_list[j]);
        j++; 
    }
    i = j - 1; 
    ///////////////////process bus stop list new ///////////////////////////
//    cout<<"sub size "<<bus_stop_list_new.size()<<endl; 

    CreateLocTable_Compact(bus_stop_list_new,temp_count,peri1,peri2);

    temp_count++; 
    
//    break; 
  }  
}

/*
get the time table for one spatial location 

*/
void RoadDenstiy::CreateLocTable_Compact(vector<BusStop_Ext> bus_stop_list_new, 
                                 int count_id, Periods* peri1, Periods* peri2)
{
//  cout<<"bus_stop_list_new size "<<bus_stop_list_new.size()<<endl; 

  Point loc = bus_stop_list_new[0].loc; 

//  cout<<"bus stop location "<<loc<<endl; 
  
  for(unsigned int i = 0;i < bus_stop_list_new.size();i++){
      int br_id = bus_stop_list_new[i].br_id;
      bool direction = bus_stop_list_new[i].start_small;
      int stop_id = bus_stop_list_new[i].br_stop_id;
      
//      if( !(br_id == 11 && direction)) continue; 

//      cout<<"br_id "<<br_id<<" direction "<<direction<<endl; 

      /////use btree to find all bus trips of this route ///////
     CcInt* search_trip_id = new CcInt(true, br_id);
     BTreeIterator* btree_iter = btree->ExactMatch(search_trip_id);

     vector<MPoint> night_mo;
     vector<MPoint> day_mo1; //Monday
     vector<MPoint> day_mo2; //Sunday 

     while(btree_iter->Next()){
        Tuple* tuple_trip = rel2->GetTuple(btree_iter->GetId(), false);
        int br_trip_id = 
            ((CcInt*)tuple_trip->GetAttribute(BR_ID5))->GetIntval();
        bool trip_direction = 
            ((CcBool*)tuple_trip->GetAttribute(MO_BUS_DIRECTION))->GetBoolval();
        assert(br_id == br_trip_id);
        if(direction == trip_direction){
          MPoint* mo = (MPoint*)tuple_trip->GetAttribute(BUS_TRIP);
          string busday = 
            ((CcString*)tuple_trip->GetAttribute(BUS_DAY))->GetValue();
          string bus_type = 
            ((CcString*)tuple_trip->GetAttribute(BUS_TYPE))->GetValue();
          UPoint up;
          mo->Get(0, up);
          ////////////////////////////////////////////////////////////////////
          ////////  classify the buses into groups according to time  ///////
          //////////////////////////////////////////////////////////////////

          if(bus_type.compare("night") == 0){// only need one day for night bus
              if(SameDay(up,peri1))
                night_mo.push_back(*mo);
          }else{
            if(busday.compare("Monday") == 0){
              day_mo1.push_back(*mo);
            }
            else if(busday.compare("Sunday") == 0){
              day_mo2.push_back(*mo);
            }
            else assert(false);
          }
          ////////////////////////////////////////////////////////////
        }
        tuple_trip->DeleteIfAllowed();
     }
     delete btree_iter;
     delete search_trip_id;
    ////////////////////////////////////////////////////////////////////
//    cout<<"night_mo "<<night_mo.size()<<endl;
//    cout<<"workday_mo "<<day_mo1.size()<<endl;
//    cout<<"weekend_mo "<<day_mo2.size()<<endl; 

    ////////////////////////////////////////////////////////////////////
    if(night_mo.size() > 0)
      CreatTableAtStopNight(night_mo,loc,br_id,stop_id,direction,
                     peri1,peri2,count_id);
    CreatTableAtStop(day_mo1,loc,br_id,stop_id,direction,count_id);
    CreatTableAtStop(day_mo2,loc,br_id,stop_id,direction,count_id);
    ////////////////////////////////////////////////////////////////////
  }
}

/*
check whether thay are in the same day 

*/
bool RoadDenstiy::SameDay(UPoint& up, Periods* peri1)
{
    Interval<Instant> periods1;
    peri1->Get(0, periods1);
    int day1 = up.timeInterval.start.GetDay();
    int day2 = periods1.start.GetDay();
    if(day1 == day2)return true;
    else
      return false; 
}

/*
create table at one location for night buses 

*/
void RoadDenstiy::CreatTableAtStopNight(vector<MPoint>& mo_list, Point& loc, 
                                   int br_id, int stop_id, bool dir,
                                   Periods* night1, Periods* night2,
                                   int count_id)
{
    Interval<Instant> periods1;
    night1->Get(0, periods1);
    Interval<Instant> periods2;
    night2->Get(0, periods2);

//    cout<<"peri1 "<<*night1<<" peri2 "<<*night2<<endl; 
    
    double e1 = periods1.end.ToDouble();

    vector<MPoint> mo_list1;
    vector<MPoint> mo_list2; 
    for(unsigned int i = 0;i < mo_list.size();i++){
      MPoint mo = mo_list[i];
      UPoint up;
      mo.Get(0, up);
      double s = up.timeInterval.start.ToDouble(); 
      if( s < e1 || AlmostEqual(s, e1))
        mo_list1.push_back(mo_list[i]);
      else 
        mo_list2.push_back(mo_list[i]);
    }
//    cout<<"mo_list1 size "<<mo_list1.size()
//        <<" mo_list2 size "<<mo_list2.size()<<endl; 

    CreatTableAtStop(mo_list1,loc,br_id,stop_id,dir,count_id);
    CreatTableAtStop(mo_list2,loc,br_id,stop_id,dir,count_id);
}

/*
create time table for one location 

*/
void RoadDenstiy::CreatTableAtStop(vector<MPoint> trip_list, Point& loc,
                                   int br_id, int stop_id, bool dir,
                                   int count_id)
{

  assert(trip_list.size() >= 2);

  MPoint start_mo_0 = trip_list[0];
  UPoint up1;
  start_mo_0.Get(0, up1);//the start time of first trip 
  Instant start_time_0 = up1.timeInterval.start;
  Instant st = start_time_0;
  GetTimeInstantStop(start_mo_0, loc,st); 
  
  /////////////cut second and millsecond /////////////////////////////
  int second_val_s = st.GetSecond(); 
  int msecond_val_s = st.GetMillisecond(); 
  double double_s = st.ToDouble() - 
                       second_val_s/(24.0*60.0*60.0) -
                       msecond_val_s/(24.0*60.0*60.0*1000.0);
  st.ReadFrom(double_s);
  /////////////////////////////////////////////////////////////////
  
  MPoint start_mo_1 = trip_list[1];
  UPoint up2;
  start_mo_1.Get(0, up1);//the start time of second trip 
  Instant start_time_1 = up1.timeInterval.start;
  

  ///////////  get time interval for schedule  ///////////////////
  double sch_interval = start_time_1.ToDouble() - start_time_0.ToDouble();
  assert(sch_interval > 0.0);
//  cout<<"schedule "<<sch_interval*24.0*60.0*60.0<<" seconds"<<endl; 
  
  ////////////////last bus trip ////////////////////////////////////
  MPoint end_mo = trip_list[trip_list.size() - 1];
  UPoint up3;
  end_mo.Get(0, up3);//the start time of last trip 
  Instant end_time  = up3.timeInterval.start; 
  /////////////////cut second and millsecond ////////////////////////////////
  Instant et = end_time;
  
  GetTimeInstantStop(end_mo, loc, et); 

  int second_val_e = et.GetSecond(); 
  int msecond_val_e = et.GetMillisecond(); 
  double double_e = et.ToDouble() - 
                       second_val_e/(24.0*60.0*60.0) -
                       msecond_val_e/(24.0*60.0*60.0*1000.0);
  et.ReadFrom(double_e);
  ////////////////////////////////////////////////////////////////////////////
  assert(second_val_s == second_val_e); 
  Interval<Instant> time_span;
  time_span.start = st;
  time_span.lc = true;
  time_span.end = et;
  time_span.rc = true; 

   Periods* peri = new Periods(0);
   peri->StartBulkLoad();
   peri->MergeAdd(time_span);
   peri->EndBulkLoad();
//   duration.push_back(*peri);
//   cout<<"periods "<<*peri<<endl; 


  bus_stop_loc.push_back(loc);
  br_id_list.push_back(br_id);
  bus_stop_id_list.push_back(stop_id);
  br_direction.push_back(dir);
  duration1.push_back(*peri);
  unique_id_list.push_back(count_id);
  schedule_interval.push_back(sch_interval);
  delete peri; 

  
    ////////////////////////    check time  ////////////////////////////// 
  for(unsigned int i = 0;i < trip_list.size();i++){ 
    MPoint mo = trip_list[i];
    Instant temp_instant;
    GetTimeInstantStop(mo, loc, temp_instant);
    int second_val = temp_instant.GetSecond();
    assert(second_val == second_val_s);
  }
  //////////////////////////////////////////////////////////////////////// 
}

/*
get the time that the train arrives at this point(stop)

*/
void RoadDenstiy::GetTimeInstantStop(MPoint& mo, Point loc, Instant& arrove_t)
{
  const double dist_delta = 0.01; 
  const double stop_time = 30.0/(24.0*60.0*60.0); //30 seconds for buses  
  for(int j = 0;j < mo.GetNoComponents();j++){
      UPoint up;
      mo.Get(j, up);
      Point loc1 = up.p0;
      Point loc2 = up.p1; 

      if(loc1.Distance(loc2) < dist_delta && 
         loc1.Distance(loc) < dist_delta){ //find the place 
            Instant st = up.timeInterval.start;
            Instant et = up.timeInterval.end; 
            double d_st = st.ToDouble();
            double d_et = et.ToDouble();
            assert(AlmostEqual(fabs(d_st-d_et), stop_time));//check 30 seconds
            arrove_t = st; 
            return; 
      }
 }
  assert(false);
}

//////////////////////////////////////////////////////////////
///////////////////Bus Stop///////////////////////////////////
//////////////////////////////////////////////////////////////

ListExpr BusStopProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("busstop"),
           nl->StringAtom("(<brid, stopid, up>) (int int bool)"),
           nl->StringAtom("((1 2 TRUE))"))));
}

/*
Output  (brid, stopid) 

*/

ListExpr OutBusStop( ListExpr typeInfo, Word value )
{
//  cout<<"OutBusStop"<<endl; 
  Bus_Stop* bs = (Bus_Stop*)(value.addr);
  if(!bs->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  ListExpr list1 = nl->TwoElemList(nl->StringAtom("Bus Route Id:"), 
                         nl->IntAtom(bs->GetId()));

  ListExpr list2 = nl->TwoElemList(nl->StringAtom("Stop Id:"), 
                         nl->IntAtom(bs->GetStopId()));

  ListExpr list3 = nl->TwoElemList(nl->StringAtom("UP:"), 
                         nl->BoolAtom(bs->GetUp()));
  return nl->ThreeElemList(list1,list2, list3);

}

/*
In function

*/
Word InBusStop( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  
//  cout<<"length "<<nl->ListLength(instance)<<endl;

  if( !nl->IsAtom( instance ) ){

    if(nl->ListLength(instance) != 3){
      cout<<"length should be 3"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    ListExpr first = nl->First(instance);
    if(!nl->IsAtom(first) || nl->AtomType(first) != IntType){
      cout<< "busstop(): brid must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int id1 = nl->IntValue(first);

    ListExpr second = nl->Second(instance);
    if(!nl->IsAtom(second) || nl->AtomType(second) != IntType){
      cout<< "busstop(): stop id must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
   unsigned int id2 = nl->IntValue(second);
   
    ListExpr third = nl->Third(instance);
    if(!nl->IsAtom(third) || nl->AtomType(third) != BoolType){
      cout<< "busstop(): up/down must be bool type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
   bool d = nl->BoolValue(third);
   
   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    Bus_Stop* bs = new Bus_Stop(true, id1, id2, d);
    return SetWord(bs);
  }

  correct = false;
  return SetWord(Address(0));
}

/*
Open an reference object 

*/
bool OpenBusStop(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenBusStop()"<<endl; 

  Bus_Stop* bs = (Bus_Stop*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(bs);
  return true; 
}

/*
Save an reference object 

*/
bool SaveBusStop(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveBusStop"<<endl; 
  Bus_Stop* bs = (Bus_Stop*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, bs);
  return true; 
}

Word CreateBusStop(const ListExpr typeInfo)
{
// cout<<"CreateBusStop()"<<endl;
  return SetWord (new Bus_Stop(false));
}


void DeleteBusStop(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteBusStop()"<<endl;
  Bus_Stop* bs = (Bus_Stop*)w.addr;
  delete bs;
   w.addr = NULL;
}


void CloseBusStop( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseBusStop"<<endl; 
  ((Bus_Stop*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneBusStop( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneBusStop"<<endl; 
  return SetWord( new Bus_Stop( *((Bus_Stop*)w.addr) ) );
}

int SizeOfBusStop()
{
//  cout<<"SizeOfBusStop"<<endl; 
  return sizeof(Bus_Stop);
}

bool CheckBusStop( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckBusStop"<<endl; 
  return (nl->IsEqual( type, "busstop" ));
}

/*
type constructure functions 

*/
Bus_Stop::Bus_Stop():Attribute(){}

Bus_Stop::Bus_Stop(bool def, unsigned int id1, unsigned int id2, bool d):
Attribute(def), br_id(id1), stop_id(id2), up_down(d)
{
  SetDefined(def); 
}
Bus_Stop::Bus_Stop(const Bus_Stop& bs):
Attribute(bs.IsDefined()), br_id(bs.br_id), 
stop_id(bs.stop_id), up_down(bs.up_down)
{
  SetDefined(bs.IsDefined()); 
}

Bus_Stop& Bus_Stop::operator=(const Bus_Stop& bs)
{
    SetDefined(bs.IsDefined());
    if(IsDefined()){
          br_id = bs.GetId();
          stop_id = bs.GetStopId();
          up_down = bs.up_down; 
    }
    return *this;
}


void* Bus_Stop::Cast(void* addr)
{
  return new (addr)Bus_Stop; 

}

ostream& operator<<(ostream& o, const Bus_Stop& bs)
{
  if(bs.IsDefined()){
    o<<"br_id "<< bs.GetId()<<" "
    <<"stop id "<<bs.GetStopId()<<" "
    <<"direction "<<bs.GetUp()<<endl; 
  }else
    o<<"undef"<<endl;

  return o;
}

//////////////////////////////////////////////////////////////
//////////////////////bus route///////////////////////////////
//////////////////////////////////////////////////////////////
ListExpr BusRouteProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("busroute"),
         nl->StringAtom("((id, up) (<segment list>*))"),
           nl->StringAtom("((1 true)((((2.0 2.0 3.0 3.0)))))"))));
}

/*
Open an reference object 

*/
bool OpenBusRoute(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenBusRoute()"<<endl; 

  Bus_Route* br = (Bus_Route*)Attribute::Open(valueRecord, offset, typeInfo);
  value = SetWord(br);
  return true; 
}

/*
Save an reference object 

*/
bool SaveBusRoute(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveBusRoute"<<endl; 
  Bus_Route* br = (Bus_Route*)value.addr; 
  Attribute::Save(valueRecord, offset, typeInfo, br);
  return true; 
}

Word CreateBusRoute(const ListExpr typeInfo)
{
// cout<<"CreateBusRoute()"<<endl;
  return SetWord (new Bus_Route(0));
}


void DeleteBusRoute(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteBusRoute()"<<endl;
  Bus_Route* br = (Bus_Route*)w.addr;
  delete br;
   w.addr = NULL;
}


void CloseBusRoute( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseBusRoute"<<endl; 
  ((Bus_Route*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word CloneBusRoute( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneBusRoute"<<endl; 
  return SetWord( new Bus_Route( *((Bus_Route*)w.addr) ) );
}

int SizeOfBusRoute()
{
//  cout<<"SizeOfBusRoute"<<endl; 
  return sizeof(Bus_Route);
}

bool CheckBusRoute( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckBusRoute"<<endl; 
  return (nl->IsEqual( type, "busroute" ));
}

/*
In function

*/
Word InBusRoute( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  
//  cout<<"length "<<nl->ListLength(instance)<<endl;

  if( !nl->IsAtom( instance ) ){

    if(nl->ListLength(instance) != 2){
      cout<<"length should be 2"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    ListExpr first = nl->First(instance);
    if(nl->ListLength(first) != 2){
      cout<< "busroute(): the first part should have two parameters"<<endl;
      correct = false;
      return SetWord(Address(0));
    }

    ListExpr first1 = nl->First(first); 
    if(!nl->IsAtom(first1) || nl->AtomType(first1) != IntType){
      cout<< "busroute(): bus route id must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int br_id = nl->IntValue(first1);

    ListExpr first2 = nl->Second(first); 
    if(!nl->IsAtom(first2) || nl->AtomType(first2) != BoolType){
      cout<< "busroute(): bus route up/down must be bool type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    bool d = nl->BoolValue(first2);

    Bus_Route* br = new Bus_Route(br_id, d);


    ListExpr geo_list = nl->Second(instance);
    br->StartBulkLoad();
    int count = 0; 
    while(!nl->IsEmpty(geo_list)){
        ListExpr geo_first = nl->First(geo_list);
        geo_list = nl->Rest(geo_list);
        if(nl->ListLength(geo_first) != 1){
            cout<< "busroute(): segment list should have length 1"<<endl;
            correct = false;
            return SetWord(Address(0));
        }

        SimpleLine* sl = (SimpleLine*)InSimpleLine(typeInfo, 
               nl->First(geo_first), errorPos, errorInfo, correct).addr;
//        cout<<s<<" "<<sl->Length()<<endl;
        br->Add(sl, count);
        count++;
    }
    br->EndBulkLoad();
   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    return SetWord(br);
  }

  correct = false;
  return SetWord(Address(0));
}


/*
Output  a sequence of bus segments 

*/

ListExpr OutBusRoute( ListExpr typeInfo, Word value )
{
//  cout<<"OutBusRoute"<<endl; 
  Bus_Route* br = (Bus_Route*)(value.addr);
  if(!br->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  ListExpr list1 = nl->TwoElemList(
               nl->IntAtom(br->GetId()), nl->BoolAtom(br->GetUp()));
  ListExpr list2 = nl->TheEmptyList(); 
  if(!br->IsEmpty()){

    ListExpr last = list2;
    bool first = true;
    for(int i = 0;i < br->Size();i++){
      SimpleLine sl(0);
      br->Get(i, sl); 

      ListExpr geo_list = OutSimpleLine(nl->TheEmptyList(), SetWord(&sl));
      ListExpr flatseg = nl->OneElemList(geo_list);

      if(first == true){
          list2 = nl->OneElemList( flatseg );
          last = list2;
          first = false; 
      }else{
          last = nl->Append( last, flatseg );
      }
    }
  }

  return nl->TwoElemList(list1,list2);

}

Bus_Route::Bus_Route()
{

}

Bus_Route::Bus_Route(const Bus_Route& br):
StandardSpatialAttribute<2>(br.IsDefined()), elem_list(0), seg_list(0),
br_id(br.GetId()), up_down(br.GetUp())
{
  if(IsDefined()){
    elem_list.clean();
    seg_list.clean();
    Bus_Route* b_r = const_cast<Bus_Route*>(&br);
    for(int i = 0;i < b_r->Size();i++){
        BR_Elem elem;
        b_r->GetElem(i, elem);
        elem_list.Append(elem);

    }

    for(int i = 0;i < b_r->SegSize();i++){
        HalfSegment hs;
        b_r->GetSeg(i, hs);
        seg_list.Append(hs);
    }
  }

}

Bus_Route& Bus_Route::operator=(const Bus_Route& br)
{
    if(!br.IsDefined()) return *this; 
    
    br_id = br.GetId();
    up_down = br.GetUp(); 

    elem_list.clean();
    seg_list.clean();

    Bus_Route* b_r = const_cast<Bus_Route*>(&br);
    for(int i = 0;i < b_r->Size();i++){
        BR_Elem elem;
        b_r->GetElem(i, elem);
        elem_list.Append(elem);

    }

    for(int i = 0;i < b_r->SegSize();i++){
        HalfSegment hs;
        b_r->GetSeg(i, hs);
        seg_list.Append(hs);
    }
    
    SetDefined(true);
    return *this; 
}

void Bus_Route::GetElem(int i, BR_Elem& elem)
{
  assert(0 <= i && i < elem_list.Size());
  elem_list.Get(i, elem);

}
void Bus_Route::GetSeg(int i, HalfSegment& hs)
{
  assert(0 <= i && i < seg_list.Size()); 
  seg_list.Get(i, hs);
}

void* Bus_Route::Cast(void* addr)
{
  return new (addr)Bus_Route; 

}


const Rectangle<2> Bus_Route::BoundingBox() const
{
    Rectangle<2> bbox;
    for( int i = 0; seg_list.Size(); i++ ){
        HalfSegment hs ;
        seg_list.Get(i, hs);
        if( i == 0 ){
          bbox = hs.BoundingBox();
        }else
          bbox = bbox.Union(hs.BoundingBox());
    }
    return bbox;
}

/*
add a new element to the result 

*/
void Bus_Route::Add(SimpleLine* sl, int count)
{
  BR_Elem br_elem;
  br_elem.br_seg_id = count;
  br_elem.start_pos = seg_list.Size(); 
  int no = 0;
  for(int i = 0;i < sl->Size();i++){
    HalfSegment hs;
    sl->Get(i, hs);
    if(!hs.IsLeftDomPoint()) continue; 
    seg_list.Append(hs); 
    no++;
  }
  br_elem.no = no;
  elem_list.Append(br_elem); 
}

/*
get a bus segment from the dbarray 

*/
void Bus_Route::Get(int i, SimpleLine& sl)
{
  assert(0 <= i && i < elem_list.Size()); 

  BR_Elem br_elem;
  elem_list.Get(i, br_elem); 

  sl.StartBulkLoad();
  
  for(unsigned int j = br_elem.start_pos; j < br_elem.start_pos + br_elem.no;
      j++){
    HalfSegment hs; 
    seg_list.Get(j ,hs);
    sl += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    sl += hs;
  }
  sl.EndBulkLoad();

}

/*
get the geometrical line of a bus route 

*/
void Bus_Route::GetGeoData(SimpleLine& sl)
{
  sl.StartBulkLoad(); 
  int edgeno = 0;
  for(int i = 0;i < seg_list.Size();i++){
    HalfSegment hs1;
    seg_list.Get(i, hs1);

    HalfSegment hs2(true, hs1.GetLeftPoint(), hs1.GetRightPoint());
    hs2.attr.edgeno = edgeno++; 
    sl += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    sl += hs2; 
  }
  sl.EndBulkLoad(); 
}

/*
get the point of a bus stop. it is the end point of last segment and the 
start point of next segment 

*/
void Bus_Route::GetBusStopGeoData(Bus_Stop* bs, Point* p)
{
  if(GetId() != bs->GetId()){
    cout<<"route id is different for the bus stop and route"<<endl; 
    p->SetDefined(false);
    return; 
  }
  if(bs->GetStopId() < 1 || (int)bs->GetStopId() > Size()){
    cout<<"invalid bus stop id"<<endl; 
    p->SetDefined(false);
    return;
  }

  if(bs->GetUp() != GetUp()){
    cout<<"bus stop and bus route have different directions"<<endl;
    p->SetDefined(false);
    return;
  }

  SimpleLine sl1(0);
  Get(bs->GetStopId() - 1, sl1);
  SimpleLine sl2(0);
  Get(bs->GetStopId(), sl2);

  Point sp1, sp2, ep1, ep2;
  assert(sl1.AtPosition(0.0, sl1.GetStartSmaller(), sp1));
  assert(sl1.AtPosition(sl1.Length(), sl1.GetStartSmaller(), ep1));
  assert(sl2.AtPosition(0.0, sl2.GetStartSmaller(), sp2));
  assert(sl2.AtPosition(sl2.Length(), sl2.GetStartSmaller(), ep2));
  if(AlmostEqual(sp1, sp2) || AlmostEqual(sp1, ep2)){
    *p = sp1;
  }else if(AlmostEqual(ep1, sp2) || AlmostEqual(ep1, ep2)){
    *p = ep1;
  }else assert(false); 

}

double Bus_Route::Length()
{
  if(!IsDefined() || IsEmpty() == true) return 0.0; 

  double l = 0.0;
  for(int i = 0;i < seg_list.Size();i++){
    HalfSegment hs;
    seg_list.Get(i, hs);
    l += hs.Length(); 
  }
  return l; 
}


void Bus_Route::StartBulkLoad()
{

}

/*
check whether two consequent segments are connected 

*/
void Bus_Route::EndBulkLoad()
{
  for(int i = 0;i < Size() - 1;i++){

      SimpleLine sl1(0), sl2(0); 
      Get(i, sl1); 
      Get(i + 1, sl2); 
      ////////////check the end point of the first bus segment///////.
      ////////////and the start point of the second bus segment//////
      Point sp1, ep1;
      Point sp2, ep2; 
      assert(sl1.AtPosition(0.0, sl1.GetStartSmaller(), sp1));
      assert(sl1.AtPosition(sl1.Length(), sl1.GetStartSmaller(), ep1));
      assert(sl2.AtPosition(0.0, sl2.GetStartSmaller(), sp2));
      assert(sl2.AtPosition(sl2.Length(), sl2.GetStartSmaller(), ep2));
//      cout<<" sp1 "<<sp1<<" ep1 "<<ep1<<" sp2 "<<sp2<<" ep2 "<<ep2<<endl; 
      if(!(AlmostEqual(sp1, sp2)|| AlmostEqual(sp1, ep2) ||
           AlmostEqual(ep1, sp2)|| AlmostEqual(ep1, ep2))){
        cout<<"not valid bus segments"<<endl;
        cout<<"the end point of the first segment should be equal to \
              the start point of the second segment"<<endl; 
        break;
      }
  }

}
//////////////////////////////////////////////////////////////////
////////////////  Bus Network   //////////////////////////////////
//////////////////////////////////////////////////////////////////
string BusNetwork::BusStopsTypeInfo = 
"(rel (tuple ((bus_stop busstop))))"; 
string BusNetwork::BusRoutesTypeInfo = 
"(rel (tuple ((bus_route busroute))))";
string BusNetwork::BusStopsInternal = 
"(rel (tuple ((br_id int)(bus_stop busstop))))"; 
string BusNetwork::BusStopsBTreeTypeInfo = 
"(btree (tuple ((br_id int)(bus_stop busstop))) int)"; 
string BusNetwork::BusRoutesInternal = 
"(rel (tuple ((br_id int)(bus_route busroute))))";
string BusNetwork::BusRoutesBTreeTypeInfo = 
"(btree (tuple ((br_id int)(bus_route busroute))) int)";



ListExpr BusNetworkProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("busroute"),
         nl->StringAtom("((def, id))"),
           nl->StringAtom("((TRUE 1))"))));
}

/*
output the bus network 

*/
ListExpr OutBusNetwork( ListExpr typeInfo, Word value )
{
//  cout<<"OutBusNetwork"<<endl; 
  BusNetwork* bn = (BusNetwork*)(value.addr);
  if(!bn->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  ListExpr list1 = nl->TwoElemList(
               nl->StringAtom("Bus Network Id:"), 
               nl->IntAtom(bn->GetId()));
//  return nl->OneElemList(list1);

  ////////////////out put bus stops relation////////////////////////////////
  ListExpr bs_list = nl->TheEmptyList();
  Relation* bs_rel = bn->GetBS_Rel();
  
  if(bs_rel != NULL){
      bool bFirst = true;
      ListExpr xNext = nl->TheEmptyList();
      ListExpr xLast = nl->TheEmptyList();
      for(int i = 1;i <= bs_rel->GetNoTuples();i++){
        Tuple* node_tuple = bs_rel->GetTuple(i, false);
        Bus_Stop* bs = (Bus_Stop*)node_tuple->GetAttribute(BusNetwork::BN_BS);

        ListExpr stop_list = OutBusStop(nl->TheEmptyList(),SetWord(bs));
        xNext  = stop_list;
        if(bFirst){
          bs_list = nl->OneElemList(xNext);
          xLast = bs_list;
          bFirst = false;
        }else
            xLast = nl->Append(xLast,xNext);
        node_tuple->DeleteIfAllowed();
      }
  }
  //  return nl->TwoElemList(list1, bs_list);
  //////////////////////output bus routes relation///////////////////////////
  ListExpr br_list = nl->TheEmptyList();
  Relation* br_rel = bn->GetBR_Rel();
  
  if(br_rel != NULL){
      bool bFirst = true;
      ListExpr xNext = nl->TheEmptyList();
      ListExpr xLast = nl->TheEmptyList();
      for(int i = 1;i <= br_rel->GetNoTuples();i++){
        Tuple* node_tuple = br_rel->GetTuple(i, false);
        Bus_Route* br = (Bus_Route*)node_tuple->GetAttribute(BusNetwork::BN_BR);

        ListExpr stop_list = OutBusRoute(nl->TheEmptyList(),SetWord(br));
        xNext  = stop_list;
        if(bFirst){
          br_list = nl->OneElemList(xNext);
          xLast = br_list;
          bFirst = false;
        }else
            xLast = nl->Append(xLast,xNext);
        node_tuple->DeleteIfAllowed();
      }
  }

  return nl->ThreeElemList(list1, bs_list, br_list);
}

/*
In function. there is not nested list expression here.

*/
Word InBusNetwork( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{

//  cout<<"length "<<nl->ListLength(instance)<<endl;

  if( !nl->IsAtom( instance ) ){

    if(nl->ListLength(instance) != 2){
      cout<<"length should be 2"<<endl; 
      correct = false;
      return SetWord(Address(0));
    }
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);

    if(!nl->IsAtom(first) || nl->AtomType(first) != BoolType){
      cout<< "busnetwork(): definition must be bool type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    bool d = nl->BoolValue(first);


    if(!nl->IsAtom(second) || nl->AtomType(second) != IntType){
      cout<< "busnetwork(): bus network id must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int id = nl->IntValue(second);

    BusNetwork* bn = new BusNetwork(d, id); 

   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    return SetWord(bn);
  }

  correct = false;
  return SetWord(Address(0));
}

bool SaveBusNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  BusNetwork* bn = (BusNetwork*)value.addr;
  return bn->Save(valueRecord, offset, typeInfo);
}

BusNetwork* BusNetwork::Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo)
{
  return new BusNetwork(valueRecord, offset, typeInfo); 

}

bool OpenBusNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  value.addr = BusNetwork::Open(valueRecord, offset, typeInfo);
  return value.addr != NULL; 
}


Word CreateBusNetwork(const ListExpr typeInfo)
{
// cout<<"CreateBusNetwork()"<<endl;
  return SetWord (new BusNetwork());
}


void DeleteBusNetwork(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteBusNetwork()"<<endl;
  BusNetwork* bn = (BusNetwork*)w.addr;
  delete bn;
   w.addr = NULL;
}


void CloseBusNetwork( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseBusNetwork"<<endl; 
  delete static_cast<BusNetwork*>(w.addr); 
  w.addr = 0;
}

Word CloneBusNetwork( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneBusNetwork"<<endl; 
  return SetWord( new Address(0));
}

void* BusNetwork::Cast(void* addr)
{
  return NULL;
}

int SizeOfBusNetwork()
{
//  cout<<"SizeOfBusNetwork"<<endl; 
  return sizeof(BusNetwork);
}

bool CheckBusNetwork( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckBusNetwork"<<endl; 
  return (nl->IsEqual( type, "busnetwork" ));
}

BusNetwork::BusNetwork():
def(false), bn_id(0), stops_rel(NULL), btree_bs(NULL), 
routes_rel(NULL), btree_br(NULL)
{

}

BusNetwork::BusNetwork(bool d, unsigned int i): def(d), bn_id(i), 
stops_rel(NULL), btree_bs(NULL), routes_rel(NULL), btree_br(NULL)
{

}

/*
read the data from record 

*/
BusNetwork::BusNetwork(SmiRecord& valueRecord, size_t& offset, 
                       const ListExpr typeInfo):
def(false), bn_id(0), stops_rel(NULL), btree_bs(NULL), 
routes_rel(NULL), btree_br(NULL)
{
  valueRecord.Read(&def, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&bn_id, sizeof(int), offset);
  offset += sizeof(int);

  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for busstops*********************/
  nl->ReadFromString(BusStopsInternal,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  stops_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!stops_rel) {
   return;
  }
  ///////////////////btree on bus stops//////////////////////////////////
  nl->ReadFromString(BusStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bs = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_bs) {
    stops_rel->Delete(); 
   return;
  }

  /***********************Open relation for busroutes*********************/
  nl->ReadFromString(BusRoutesInternal,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  routes_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!routes_rel) {
   stops_rel->Delete();
   delete btree_bs; 
   return;
  }

  ///////////////////btree on bus routes//////////////////////////////////
  nl->ReadFromString(BusRoutesBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_br = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_br) {
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    return;
  }

}

BusNetwork::~BusNetwork()
{
  if(stops_rel != NULL) stops_rel->Close();
  if(btree_bs != NULL) delete btree_bs; 
  if(routes_rel != NULL) routes_rel->Close();
  if(btree_br != NULL) delete btree_br;

}

bool BusNetwork::Save(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo)
{

  valueRecord.Write(&def, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&bn_id, sizeof(int), offset); 
  offset += sizeof(int); 

  ListExpr xType;
  ListExpr xNumericType;
  
  ////////////////////bus stops relation/////////////////////////////
  nl->ReadFromString(BusStopsInternal,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!stops_rel->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////btree on bus stops////////////////////////////
  nl->ReadFromString(BusStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bs->Save(valueRecord,offset,xNumericType))
      return false;

   ///////////////////bus routes relation/////////////////////////////
  nl->ReadFromString(BusRoutesInternal,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!routes_rel->Save(valueRecord,offset,xNumericType))
      return false;
  
  ///////////////////////btree on bus routes////////////////////////////
  nl->ReadFromString(BusRoutesBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_br->Save(valueRecord,offset,xNumericType))
      return false;

  return true; 
}

/*
store bus stops relation and the index 

*/
void BusNetwork:: LoadStops(Relation* r1)
{
  ListExpr xTypeInfo;
  nl->ReadFromString(BusStopsInternal, xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation* s_rel = new Relation(xNumType, true);
  for(int i = 1;i <= r1->GetNoTuples();i++){
    Tuple* bs_tuple = r1->GetTuple(i, false);
    if(bs_tuple->GetNoAttributes() != 1){
      cout<<"bus stops relation schema is wrong"<<endl;
      bs_tuple->DeleteIfAllowed();
      break; 
    }
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(0); 
    int id = bs->GetId(); 

    Tuple* new_bs_tuple = new Tuple(nl->Second(xNumType)); 
    new_bs_tuple->PutAttribute(BN_ID1, new CcInt(true, id));
    new_bs_tuple->PutAttribute(BN_BS, new Bus_Stop(*bs));
    s_rel->AppendTuple(new_bs_tuple);
    new_bs_tuple->DeleteIfAllowed(); 
    bs_tuple->DeleteIfAllowed();
  }
//  cout<<s_rel->GetNoTuples()<<endl; 
  
  ostringstream xStopsStream;
  xStopsStream << (long)s_rel;
  string strQuery = "(consume(feed(" + BusStopsInternal +
                "(ptr " + xStopsStream.str() + "))))";
 
//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  stops_rel = (Relation*)xResult.addr; 
  s_rel->Delete(); 
  
  //////////////////////////////////////////////////////////////////
  //////////////////////btree on bus stops//////////////////////////
  //////////////////////////////////////////////////////////////////
  ostringstream xNodeOidPtrStream;
  xNodeOidPtrStream << (long)stops_rel;
  strQuery = "(createbtree (" + BusStopsInternal +
             "(ptr " + xNodeOidPtrStream.str() + "))" + "br_id)";
  
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bs = (BTree*)xResult.addr;

}

/*
store bus routes relation as well as some indices 

*/
void BusNetwork:: LoadRoutes(Relation* r2)
{
  ListExpr xTypeInfo;
  nl->ReadFromString(BusRoutesInternal, xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation* r_rel = new Relation(xNumType, true);
  for(int i = 1;i <= r2->GetNoTuples();i++){
    Tuple* br_tuple = r2->GetTuple(i, false);
    if(br_tuple->GetNoAttributes() != 1){
      cout<<"bus routes relation schema is wrong"<<endl;
      br_tuple->DeleteIfAllowed();
      break; 
    }
    Bus_Route* br = (Bus_Route*)br_tuple->GetAttribute(0); 
    int id = br->GetId(); 

    Tuple* new_br_tuple = new Tuple(nl->Second(xNumType)); 
    new_br_tuple->PutAttribute(BN_ID2, new CcInt(true, id));
    new_br_tuple->PutAttribute(BN_BR, new Bus_Route(*br));
    r_rel->AppendTuple(new_br_tuple);
    new_br_tuple->DeleteIfAllowed(); 
    br_tuple->DeleteIfAllowed();
  }
//  cout<<r_rel->GetNoTuples()<<endl; 

  ostringstream xRoutesStream;
  xRoutesStream << (long)r_rel;
  string strQuery = "(consume(feed(" + BusRoutesInternal +
                "(ptr " + xRoutesStream.str() + "))))";

//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  routes_rel = (Relation*)xResult.addr; 
  r_rel->Delete(); 

   //////////////////////////////////////////////////////////////////
  //////////////////////btree on bus routes//////////////////////////
  //////////////////////////////////////////////////////////////////
  ostringstream xEdgeOidPtrStream;
  xEdgeOidPtrStream << (long)routes_rel;
  strQuery = "(createbtree (" + BusRoutesInternal +
             "(ptr " + xEdgeOidPtrStream.str() + "))" + "br_id)";
//  cout<<strQuery<<endl; 
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_br = (BTree*)xResult.addr;

}

/*
create a bus network by bus stops relation and bus routes relation 

*/
void BusNetwork::Load(unsigned int i, Relation* r1, Relation* r2)
{
  if(i < 1){
    def = false;
    return;
  }
  bn_id = i; 

  LoadStops(r1);
  LoadRoutes(r2); 

  def = true; 
}

/*
get the point of a bus stop by: busstop x busnetwork 

*/
void BusNetwork::GetBusStopGeoData(Bus_Stop* bs, Point* p)
{
  int id = bs->GetId(); 
  if(id < 1){
    cout<<"invalid bus stop"<<endl;
    return; 
  }

  CcInt* search_id = new CcInt(true, id);
  BTreeIterator* btree_iter = btree_br->ExactMatch(search_id);
  while(btree_iter->Next()){
        Tuple* tuple = routes_rel->GetTuple(btree_iter->GetId(), false);
        Bus_Route* br = (Bus_Route*)tuple->GetAttribute(BN_BR);
        if(br->GetUp() == bs->GetUp()){
          br->GetBusStopGeoData(bs, p); 
          tuple->DeleteIfAllowed();
          break; 
        }
        tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;
}


BN::BN(BusNetwork* n):bn(n), count(0), resulttype(NULL)
{
}
BN::~BN()
{
  if(resulttype != NULL) delete resulttype; 
}

void BN::GetStops()
{
  Relation* rel = bn->GetBS_Rel();
  if(rel == NULL || rel->GetNoTuples() == 0) return; 
  for(int i = 1; i <= rel->GetNoTuples();i++){
    Tuple* bs_tuple = rel->GetTuple(i, false); 
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BusNetwork::BN_BS);
    bs_list.push_back(*bs); 
    bs_tuple->DeleteIfAllowed();
  }
}

/*
copy bus routes from busnetwork 

*/
void BN::GetRoutes()
{
  Relation* rel = bn->GetBR_Rel();
  if(rel == NULL || rel->GetNoTuples() == 0) return; 
  for(int i = 1; i <= rel->GetNoTuples();i++){
    Tuple* br_tuple = rel->GetTuple(i, false);
    Bus_Route* br = (Bus_Route*)br_tuple->GetAttribute(BusNetwork::BN_BR);
    br_list.push_back(*br);
    br_tuple->DeleteIfAllowed();
  }
}


bool BSCompare(const Bus_Stop& bs1, const Bus_Stop& bs2)
{
  if(bs1.GetId() < bs2.GetId()) return true;
  else if(bs1.GetId() == bs2.GetId()){
    if(bs1.GetStopId() < bs2.GetStopId()){
        return true; 
    }else
      return false; 
  }else
    return false; 
}

/*
map bus stops to the pavements 

*/

void BN::MapBSToPavements(R_Tree<2,TupleId>* rtree, Relation* pave_rel, int w)
{
  vector<Bus_Stop> stop_list;

  for(int i = 1;i <= bn->GetBS_Rel()->GetNoTuples();i++){
    Tuple* bs_tuple = bn->GetBS_Rel()->GetTuple(i, false);
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BusNetwork::BN_BS);
    stop_list.push_back(*bs);
    bs_tuple->DeleteIfAllowed(); 
  }

//  cout<<stop_list.size()<<endl; 

  sort(stop_list.begin(), stop_list.end(), BSCompare); 

  for(unsigned int i = 0;i < stop_list.size();){
    Bus_Stop bs1 = stop_list[i];
    Bus_Stop bs2 = stop_list[i + 1];
    assert(bs1.GetId() == bs2.GetId() && bs1.GetStopId() == bs2.GetStopId()); 
    i += 2;
    MapToPavment(bs1, bs2, rtree, pave_rel, w);

//    break;
  }

}

/*
map a bus stop to a point in the pavement. It creates a line connecting the two
bus stops and extends the line. Then, it gets the intersection points of the 
line and pavements. 

*/
void BN::MapToPavment(Bus_Stop& bs1, Bus_Stop& bs2, R_Tree<2,TupleId>* rtree, 
                    Relation* pave_rel, int w)
{
  ////////////////construct a line connecting the two bus stops//////
  Point p1;
  bn->GetBusStopGeoData(&bs1, &p1);

  Point p2;
  bn->GetBusStopGeoData(&bs2, &p2);

//  cout<<"bs1 "<<bs1<<"bs2 "<<bs2<<endl; 
//  cout<<"loc1 "<<p1<<" loc2 "<<p2<<endl; 

  Line* l = new Line(0);
  l->StartBulkLoad();
  if(AlmostEqual(p1.GetX(), p2.GetX())){
    double miny = MIN(p1.GetY(), p2.GetY());
    double maxy = MAX(p1.GetY(), p2.GetY());
    Point lp(true, p1.GetX(), miny - w);
    Point rp(true, p1.GetX(), maxy + w);
    HalfSegment hs(true, lp, rp);
    hs.attr.edgeno = 0; 
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;
  }else if(AlmostEqual(p1.GetY(), p2.GetY())){
    double minx = MIN(p1.GetX(), p2.GetX());
    double maxx = MAX(p1.GetX(), p2.GetX());
    Point lp(true, minx - w, p1.GetY());
    Point rp(true, maxx + w, p1.GetY());
    HalfSegment hs(true, lp, rp);
    hs.attr.edgeno = 0; 
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;
  }else{
    double a = (p1.GetY() - p2.GetY())/(p1.GetX() - p2.GetX()); 
    double b = p1.GetY() - a*p1.GetX();
    double minx = MIN(p1.GetX(), p2.GetX());
    double maxx = MAX(p1.GetX(), p2.GetX());
    minx -= w;
    maxx += w; 
    Point lp(true, minx, a*minx + b);
    Point rp(true, maxx, a*maxx + b);
    HalfSegment hs(true, lp, rp);
    hs.attr.edgeno = 0; 
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;
  }

  l->EndBulkLoad();
  //////////////////////////////////////////////////////////////////////////// 
  ////////////////get the intersection point of the line and pavements///////
  ////////////////////////////////////////////////////////////////////////////
//  cout<<"l length: "<<l->Length()<<" dist p1 p2 "<<p1.Distance(p2)<<endl; 

  vector<MyPoint_Tid> it_p_list; 

  SmiRecordId adr = rtree->RootRecordId();
  DFTraverse(rtree, pave_rel, adr, l, it_p_list);
  delete l;

//  cout<<it_p_list.size()<<endl;
  
  assert(it_p_list.size() >= 2); 
  Point midp(true, (p1.GetX() + p2.GetX())/2, (p1.GetY() + p2.GetY())/2); 
  
/*  for(unsigned int i = 0;i < it_p_list.size();i++){
      bs_list.push_back(bs1);
      Loc loc(0.0, 0.0); 
      GenLoc genloc(1, loc); 
      genloc_list.push_back(genloc); 
      geo_list.push_back(it_p_list[i].loc); 
  }*/

  ////////////get the two closest points ////////////////////////////
  for(unsigned int i = 0;i < it_p_list.size();i++){
      it_p_list[i].dist = it_p_list[i].loc.Distance(midp); 
  }
  
  vector<MyPoint_Tid> it_p_list1; 
  vector<MyPoint_Tid> it_p_list2; 
  
  if(AlmostEqual(p1.GetX(), p2.GetX())){
    for(unsigned int i = 0;i < it_p_list.size();i++){
        if(it_p_list[i].loc.GetY() > midp.GetY()){
          it_p_list1.push_back(it_p_list[i]);
        }else
          it_p_list2.push_back(it_p_list[i]);
    }
  }else{
    for(unsigned int i = 0;i < it_p_list.size();i++){
        if(it_p_list[i].loc.GetX() > midp.GetX()){
          it_p_list1.push_back(it_p_list[i]);
        }else
          it_p_list2.push_back(it_p_list[i]);
    }
  }

  sort(it_p_list1.begin(), it_p_list1.end());
  sort(it_p_list2.begin(), it_p_list2.end());

//  it_p_list1[0].Print();
//  it_p_list2[0].Print();
  //////////////////////////////////////////////////////////////////////////
  ///////////////map each point to its corresponding bus stop///////////////
  /////////////////////////////////////////////////////////////////////////
  
  double d1 = it_p_list1[0].loc.Distance(p1);
  double d2 = it_p_list1[0].loc.Distance(p2);
  if(d1 < d2){
      // it_p_list1[0] -- p1 bs1, it_p_list2[0] -- p2 bs2
      Tuple* pave_tuple1 = pave_rel->GetTuple(it_p_list1[0].tid, false);
      int oid1 = 
          ((CcInt*)pave_tuple1->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg1 = (Region*)pave_tuple1->GetAttribute(DualGraph::PAVEMENT);
      Point q1 = it_p_list1[0].loc; 

      Loc loc1(q1.GetX() - reg1->BoundingBox().MinD(0), 
               q1.GetY() - reg1->BoundingBox().MinD(1)); 

      GenLoc genloc1(oid1, loc1); 
      assert(q1.Inside(*reg1));
      pave_tuple1->DeleteIfAllowed(); 
      bs_list.push_back(bs1);
      genloc_list.push_back(genloc1); 
      geo_list.push_back(it_p_list1[0].loc); 

      /////////////////////////////////////////////////////////////////////
      Tuple* pave_tuple2 = pave_rel->GetTuple(it_p_list2[0].tid, false);
      int oid2 = 
          ((CcInt*)pave_tuple2->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg2 = (Region*)pave_tuple2->GetAttribute(DualGraph::PAVEMENT);
      Point q2 = it_p_list2[0].loc; 

      Loc loc2(q2.GetX() - reg2->BoundingBox().MinD(0), 
               q2.GetY() - reg2->BoundingBox().MinD(1)); 

      GenLoc genloc2(oid2, loc2); 
      assert(q2.Inside(*reg2));
      pave_tuple2->DeleteIfAllowed(); 

      bs_list.push_back(bs2); 
      genloc_list.push_back(genloc2); 
      geo_list.push_back(it_p_list2[0].loc);

//      cout<<"tid1 "<<it_p_list1[0].tid<<" tid2 "<<it_p_list2[0].tid<<endl;

  }else{
     //it_p_list1[0] -- p2 bs2, it_p_list2[0] -- p1 bs1

      Tuple* pave_tuple1 = pave_rel->GetTuple(it_p_list1[0].tid, false);
      int oid1 = 
          ((CcInt*)pave_tuple1->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg1 = (Region*)pave_tuple1->GetAttribute(DualGraph::PAVEMENT);
      Point q1 = it_p_list1[0].loc; 

      Loc loc1(q1.GetX() - reg1->BoundingBox().MinD(0),
               q1.GetY() - reg1->BoundingBox().MinD(1));

      GenLoc genloc1(oid1, loc1); 
      assert(q1.Inside(*reg1));
      pave_tuple1->DeleteIfAllowed(); 
      bs_list.push_back(bs2);
      genloc_list.push_back(genloc1); 
      geo_list.push_back(it_p_list1[0].loc); 

      /////////////////////////////////////////////////////////////////////
      Tuple* pave_tuple2 = pave_rel->GetTuple(it_p_list2[0].tid, false);
      int oid2 = 
          ((CcInt*)pave_tuple2->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg2 = (Region*)pave_tuple2->GetAttribute(DualGraph::PAVEMENT);
      Point q2 = it_p_list2[0].loc; 

      Loc loc2(q2.GetX() - reg2->BoundingBox().MinD(0), 
               q2.GetY() - reg2->BoundingBox().MinD(1)); 

      GenLoc genloc2(oid2, loc2); 
      assert(q2.Inside(*reg2));
      pave_tuple2->DeleteIfAllowed(); 

      bs_list.push_back(bs1); 
      genloc_list.push_back(genloc2); 
      geo_list.push_back(it_p_list2[0].loc);

//      cout<<"tid1 "<<it_p_list1[0].tid<<" tid2 "<<it_p_list2[0].tid<<endl;
  }

}

/*
Using depth first method to travese the R-tree to find all pavements intersect
the line and get the intersection points 

*/
void BN::DFTraverse(R_Tree<2,TupleId>* rtree, Relation* rel, 
                           SmiRecordId adr, Line* l,
                           vector<MyPoint_Tid>& it_p_list)
{
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple2 = rel->GetTuple(e.info,false);
              Region* candi_reg =
                     (Region*)dg_tuple2->GetAttribute(DualGraph::PAVEMENT);
              if(l->Intersects(candi_reg->BoundingBox())){
                  Line* l1 = new Line(0);
                  candi_reg->Boundary(l1);
                  Points* ps = new Points(0);
                  l->Crossings(*l1, *ps); 
                  for(int i = 0;i < ps->Size();i++){
                    Point p;
                    ps->Get(i, p);
                    MyPoint_Tid mpt(p, 0.0, e.info); 
                    it_p_list.push_back(mpt);
                  }
                  delete ps;
                  delete l1;
              }
              dg_tuple2->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(l->Intersects(e.box)){
              DFTraverse(rtree, rel, e.pointer, l, it_p_list);
            }
      }
  }
  delete node;
}


////////////////////////////////////////////////////////////////////////////
//////////////////        Create UBahn Trains    ///////////////////////////
////////////////////////////////////////////////////////////////////////////

/*
create UBahan Trains. use the original data. translate the time and copy 
more trips 

*/

void UBTrain::CreateUBTrains(int attr1,int attr2,int attr3, Periods* peri)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3
//      <<" Periods "<<*peri<<endl; 

  //1.  take all bus trips from one line, one direction 
  //2.  find the first schedule 
  //3.  change the time according to peri
  
  vector<UBTrainTrip> trip_list; 
  id_count = 1; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_trip = rel1->GetTuple(i, false);
    int l_id = ((CcInt*)tuple_trip->GetAttribute(attr1))->GetIntval();
    bool d = ((CcBool*)tuple_trip->GetAttribute(attr2))->GetBoolval();
    MPoint* trip = (MPoint*)tuple_trip->GetAttribute(attr3); 
    
//    if(l_id != 15){
//      tuple_trip->DeleteIfAllowed();
//      continue; 
//    }
    UBTrainTrip* train_trip = new UBTrainTrip(l_id,d,*trip);
    if(trip_list.size() == 0)
      trip_list.push_back(*train_trip);
    else{
      UBTrainTrip last_trip = trip_list[trip_list.size() - 1];
      if(train_trip->line_id == last_trip.line_id)
        trip_list.push_back(*train_trip);
      else{
         ////////////process the train from the same line////////////////
//        cout<<"l_id "<<l_id<<" d "<<d<<endl; 
        CreateUBTrainTrip(trip_list, peri); 

        trip_list.clear();
        trip_list.push_back(*train_trip);
      }

    }

    delete train_trip; 
    tuple_trip->DeleteIfAllowed();
  }

  CreateUBTrainTrip(trip_list, peri);//process the last  

}

/*
create the UBahn Train Trip for each route 

*/
void UBTrain::CreateUBTrainTrip(vector<UBTrainTrip> trip_list, Periods* peri)
{
//  cout<<" size "<<trip_list.size()<<endl; 

  vector<UBTrainTrip> trip_list_up;
  vector<UBTrainTrip> trip_list_down;
  for(unsigned int i = 0;i < trip_list.size();i++){
    if(trip_list[i].direction)
        trip_list_up.push_back(trip_list[i]);
    else
        trip_list_down.push_back(trip_list[i]);
  }
  
  sort(trip_list_up.begin(), trip_list_up.end());
  sort(trip_list_down.begin(), trip_list_down.end()); 

  CreateTrainTrip(trip_list_up, peri); ///////// UP direction 
  CreateTrainTrip(trip_list_down, peri);  ///////// DOWN direction 
}

/*
create UBahn Trip 

*/
void UBTrain::CreateTrainTrip(vector<UBTrainTrip> trip_list, Periods* time_peri)
{
    /////////////create the time difference in days ////////////////////////
    Periods* peri1 = new Periods(0);
    trip_list[0].train_trip.DefTime(*peri1);
    Interval<Instant> periods1;
    peri1->Get(0, periods1);
    delete peri1; 

    Interval<Instant> periods2;
    time_peri->Get(0, periods2);

    int day1 = periods1.start.GetDay();
    int day2 = periods2.start.GetDay();
    
    double waittime = 10.0/(24.0*60.0*60.0);
    const double dist_delta = 0.01; 
    ////////////get the first trip of new time ////////////////////////////
    MPoint new_trip(0);
    for(unsigned int i = 0;i < trip_list.size();i++){
      Periods* peri = new Periods(0);
      trip_list[i].train_trip.DefTime(*peri);
      Interval<Instant> periods;
      peri->Get(0, periods);
      
//      cout<<"old "<<periods<<endl; 
    
      MPoint mo = trip_list[i].train_trip;
      MPoint* new_mo = new MPoint(0);
      new_mo->StartBulkLoad();
      
      Instant start_time = periods.start; 
//      cout<<"old start time "<<start_time<<endl;
      start_time.ReadFrom(start_time.ToDouble() + (double)(day2-day1));
//      cout<<"new start time "<<start_time<<endl; 
      for(int j = 0;j < mo.GetNoComponents();j++){
        if(j == 0 ){// add stop 10 seconds for waiting, the first stop  
            UPoint up_temp;
            mo.Get(0, up_temp);
            Instant st = start_time;
            st.ReadFrom(st.ToDouble() - waittime); 
            Instant et = start_time;
            up_temp.timeInterval.start = st;
            up_temp.timeInterval.end = et;
            if(up_temp.p0.Distance(up_temp.p1) > dist_delta){
              up_temp.p1 = up_temp.p0; 
              new_mo->Add(up_temp);
            }else{
              double delva = up_temp.timeInterval.end.ToDouble() - 
                      up_temp.timeInterval.start.ToDouble();
              assert(AlmostEqual(delva,waittime)); //check time deviation 
            }  
//            cout<<up_temp<<endl; 
        }
            
        UPoint up;
        mo.Get(j, up);
        double time_interval = 
             up.timeInterval.end.ToDouble() - up.timeInterval.start.ToDouble();
        
        UPoint* new_up = new UPoint(up);
        new_up->timeInterval.start = start_time;
        
        Instant end_time = start_time;
        end_time.ReadFrom(start_time.ToDouble() + time_interval);
        new_up->timeInterval.end = end_time; 
        start_time = end_time; 
        new_mo->Add(*new_up);
        delete new_up; 

        if(j == mo.GetNoComponents() - 1){//last station, wait for 10 seconds 
            UPoint up_temp;
            mo.Get(j, up_temp);
            Instant st = end_time;
            Instant et = end_time;
            et.ReadFrom(et.ToDouble() + waittime); 
            up_temp.timeInterval.start = st;
            up_temp.timeInterval.end = et;
            if(up_temp.p0.Distance(up_temp.p1) > dist_delta){//the last stop 
              up_temp.p0 = up_temp.p1; 
              new_mo->Add(up_temp);
            }else{
              double delva = up_temp.timeInterval.end.ToDouble() - 
                      up_temp.timeInterval.start.ToDouble();
              assert(AlmostEqual(delva,waittime)); //check time deviation 
            }  
        }

      }
      new_mo->EndBulkLoad();
      new_mo->DefTime(*peri);
      peri->Get(0, periods);
//      cout<<"new "<<periods<<endl; 
      new_trip = *new_mo; 
      delete new_mo;
      delete peri; 
      break;  ////////////////  only need to create the first trip 
    }
    
    ////////////////////get the time schedule ///////////////////////////
    assert(trip_list.size() > 2); 
    
    Periods* peri_1 = new Periods(0);
    trip_list[0].train_trip.DefTime(*peri_1);
    Interval<Instant> periods_1;
    peri_1->Get(0, periods_1);

    
    Periods* peri_2 = new Periods(0);
    trip_list[1].train_trip.DefTime(*peri_2);
    Interval<Instant> periods_2;
    peri_2->Get(0, periods_2);

    delete peri_1;
    delete peri_2; 
    
    double schedule_time = 
        periods_2.start.ToDouble() - periods_1.start.ToDouble();
        
    int w_day = periods2.start.GetWeekday(); 
    if(w_day != 0){
      cout<<"the day should be Monday"<<endl; 
      return; 
    }

    ///////////////copy the train trip//////////////////////////////////
    int start_pos = id_list.size(); //record the start position 
    
    int schedule_id = 1; 
    id_list.push_back(id_count);
    id_count++; 
    int train_line_id = trip_list[0].line_id;
    bool train_direction = trip_list[0].direction;
    line_id_list.push_back(train_line_id);
    direction_list.push_back(train_direction); 
    train_trip.push_back(new_trip); 
    schedule_id_list.push_back(schedule_id);
    schedule_id++; 

    
    Periods* peri_a = new Periods(0);
    new_trip.DefTime(*peri_a);
    peri_a->Get(0, periods_1);
    Instant start_time = periods_1.start; 
    
    time_peri->Get(0, periods_2);
    Instant end_time = periods_2.end; 
    
    while(start_time < end_time){
        MPoint* mo = new MPoint(0);
        mo->StartBulkLoad();
        for(int i = 0;i < new_trip.GetNoComponents();i++){
            UPoint up;
            new_trip.Get(i, up);
            Instant st = up.timeInterval.start;
            st.ReadFrom(st.ToDouble() + (schedule_id - 1)*schedule_time);
            Instant et = up.timeInterval.end;
            et.ReadFrom(et.ToDouble() + (schedule_id - 1)*schedule_time);
            up.timeInterval.start = st;
            up.timeInterval.end = et;
            mo->Add(up);
        }
        mo->EndBulkLoad();
        /////////////////////////////////////////////////////////////////
        mo->DefTime(*peri_a);
        peri_a->Get(0, periods_1);
        start_time = periods_1.start; 
        start_time.ReadFrom(start_time.ToDouble() + schedule_time); 
        ///////////////////////////////////////////////////////////////////
        id_list.push_back(id_count);
        id_count++; 
        line_id_list.push_back(train_line_id);
        direction_list.push_back(train_direction); 
        train_trip.push_back(*mo); 
        schedule_id_list.push_back(schedule_id);
        schedule_id++; 
        ///////////////////////////////////////////////////////////////////
        delete mo; 
    }

    delete peri_a; 

    int end_pos = id_list.size();//end position 
    ////////////  make it one day more //////////////////////////
    CopyTrainTrip(start_pos, end_pos, train_line_id,train_direction); 
}

/*
let the UBahn train move one day more ---Sunday 

*/
void UBTrain::CopyTrainTrip(int start_pos, int end_pos,int line_id, bool d)
{
//  cout<<"start_pos "<<start_pos<<" end_pos "<<end_pos<<endl; 

  int schedule_id = 1; 
  for(int i = start_pos; i< end_pos;i++){//the same schedule 
    MPoint mo = train_trip[i];
    MPoint* new_mo = new MPoint(0);
    new_mo->StartBulkLoad();
    for(int j = 0;j < mo.GetNoComponents();j++){
      UPoint up;
      mo.Get(j, up);
      Instant st = up.timeInterval.start;
      st.ReadFrom(st.ToDouble() - 1.0);//one day before 
      Instant et = up.timeInterval.end;
      et.ReadFrom(et.ToDouble() - 1.0);//one day before
      up.timeInterval.start = st;
      up.timeInterval.end = et; 
      new_mo->Add(up);
    }
    new_mo->EndBulkLoad();
    /////////////////////////////////////////////////////////////////////
    id_list.push_back(id_count);
    id_count++; 
    line_id_list.push_back(line_id);
    direction_list.push_back(d); 
    train_trip.push_back(*new_mo); 
    schedule_id_list.push_back(schedule_id);
    schedule_id++; 
    /////////////////////////////////////////////////////////////////////
    delete new_mo; 
  }
}

/*
extract train stops from train trip 

*/

void UBTrain::CreateUBTrainStop(int attr1, int attr2, int attr3)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 
  const double delta_dist = 0.01; 

  int last_line_id = 0; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_train = rel1->GetTuple(i, false);
    int lineid = 
          ((CcInt*)tuple_train->GetAttribute(attr1))->GetIntval();
    bool direction =       
          ((CcBool*)tuple_train->GetAttribute(attr2))->GetBoolval(); 
    /////////we define the stop id increases along up direction
    /////// it means id decreases along down direction 
    int stopid = 1; 
    if(last_line_id != lineid && direction){ //should be the same result 
//    if(last_line_id != lineid && direction == false){
        MPoint* mo = (MPoint*)tuple_train->GetAttribute(attr3); 
        /////  extract bus stop from mo  ///// 
        for(int j = 0;j < mo->GetNoComponents();j++){
          UPoint up;
          mo->Get(j, up);
          if(up.p0.Distance(up.p1) < delta_dist){ //a stop position 
              line_id_list.push_back(lineid);
              stop_loc_list.push_back(up.p0);
              stop_id_list.push_back(stopid);
              stopid++;
          }
        }
        last_line_id = lineid; 
    }
    tuple_train->DeleteIfAllowed();
  }
}

/*
create time tables for trains 

*/
void UBTrain::CreateTimeTable()
{
  ////////////////////    collect all train stations   /////////////////
  vector<BusStop_Ext> station_list; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* tuple_stop = rel1->GetTuple(i, false);
      int lineid = ((CcInt*)tuple_stop->GetAttribute(T_LINEID))->GetIntval();
      Point* loc = (Point*)tuple_stop->GetAttribute(T_STOP_LOC);
      int stopid = ((CcInt*)tuple_stop->GetAttribute(T_STOP_ID))->GetIntval(); 
      BusStop_Ext* bs_e = new BusStop_Ext(lineid,stopid,0.0,*loc,true);
      station_list.push_back(*bs_e);
      delete bs_e; 
      tuple_stop->DeleteIfAllowed(); 
  }
  sort(station_list.begin(), station_list.end());
  
  const double dist_delta = 0.01; 
  unsigned int temp_count = 1;
  for(unsigned int i = 0;i < station_list.size();i++){
    vector<BusStop_Ext> station_list_new; 
  
    station_list_new.push_back(station_list[i]);
  
    ////////collect all bus stops mapping to the same 2D point in space/////
    unsigned int j = i + 1;
    BusStop_Ext bse = station_list_new[0]; 
//    bse.Print();
    
    while(j < station_list.size() &&
        station_list[j].loc.Distance(bse.loc) < dist_delta ){
        station_list_new.push_back(station_list[j]);
        j++; 
    }
    i = j - 1; 
    ///////////////////process train station list new /////////////////////
    CreateLocTable(station_list_new,temp_count);
    temp_count++; 

  }  

//  cout<<"different spatial locations "<<temp_count - 1<<endl; 

}

/*
find the time table for one spatial location 
Time tables for trains are the same on Sunday and Monday 

*/

void UBTrain::CreateLocTable(vector<BusStop_Ext> station_list_new,int count_id)
{
//  cout<<"size "<<station_list_new.size()<<endl; 

  Point loc = station_list_new[0].loc; 
  
//  cout<<"bus stop location "<<loc<<endl; 
  
  const double dist_delta = 0.01; 
  const double stop_time = 10.0/(24.0*60.0*60.0); //10 seconds for trains 
  
  for(unsigned int i = 0;i < station_list_new.size();i++){
      int br_id = station_list_new[i].br_id;
      int stop_id = station_list_new[i].br_stop_id;
      /////use btree to find all bus trips of this route ///////
     CcInt* search_trip_id = new CcInt(true, br_id);
     BTreeIterator* btree_iter = btree1->ExactMatch(search_trip_id);
     while(btree_iter->Next()){
        Tuple* tuple_trip = rel2->GetTuple(btree_iter->GetId(), false);
        int br_trip_id = 
            ((CcInt*)tuple_trip->GetAttribute(T_LINE))->GetIntval();
        bool trip_direction = 
            ((CcBool*)tuple_trip->GetAttribute(T_UP))->GetBoolval();
        assert(br_id == br_trip_id);

        MPoint* mo = (MPoint*)tuple_trip->GetAttribute(T_TRIP);
//        cout<<"br_trip_id "<<br_trip_id
//            <<"direction "<<trip_direction<<endl; 

//          cout<<*mo<<endl; 
        ////////////////////traverse the trip to find the point/////////////  
          int j = 0;
          for(;j < mo->GetNoComponents();j++){
            UPoint up;
            mo->Get(j, up);
            Point loc1 = up.p0;
            Point loc2 = up.p1; 
//            cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl; 
            bool find = false; 
            Instant schedule_t; 
            if(loc1.Distance(loc2) < dist_delta && 
               loc1.Distance(loc) < dist_delta){ //find the place 
              Instant st = up.timeInterval.start;
              Instant et = up.timeInterval.end; 
              double d_st = st.ToDouble();
              double d_et = et.ToDouble();
//              cout<<"st "<<st<<" et "<<et<<endl; 
              assert(AlmostEqual(fabs(d_st-d_et), stop_time) || 
                     fabs(d_st-d_et) > stop_time);//check 10 seconds
              schedule_t = st; 
              find = true;
            }  
            //////////after add waiting 10 seconds for the last stop //////
            /////////////////////add result////////////////////////
            if(find){
              stop_loc_list.push_back(loc);
              line_id_list.push_back(br_id);
              direction_list.push_back(trip_direction);
              stop_id_list.push_back(stop_id);

              ///////////////cut second and millisecond value/////////////////
              int second_val = schedule_t.GetSecond(); 
              int msecond_val = schedule_t.GetMillisecond(); 
              double double_s = schedule_t.ToDouble() - 
                                second_val/(24.0*60.0*60.0) -
                                msecond_val/(24.0*60.0*60.0*1000.0);
              
              schedule_t.ReadFrom(double_s);
              schedule_time.push_back(schedule_t);
              loc_id_list.push_back(count_id);
              break; 
            }
          }  
          assert(j != mo->GetNoComponents());

        tuple_trip->DeleteIfAllowed();
     }
     delete btree_iter;
     delete search_trip_id;
    ////////////////////////////////////////////////////////////////////
  }

}

/*
compact storage of time tables.
Instead of storing each time instant for every bus trip, 
it stores the time periods and time interval of schedule for each route 

*/

void UBTrain::CreateTimeTable_Compact()
{
  vector<BusStop_Ext> station_list; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* tuple_stop = rel1->GetTuple(i, false);
      int lineid = ((CcInt*)tuple_stop->GetAttribute(T_LINEID))->GetIntval();
      Point* loc = (Point*)tuple_stop->GetAttribute(T_STOP_LOC);
      int stopid = ((CcInt*)tuple_stop->GetAttribute(T_STOP_ID))->GetIntval(); 
      BusStop_Ext* bs_e = new BusStop_Ext(lineid,stopid,0.0,*loc,true);
      station_list.push_back(*bs_e);
      delete bs_e; 
      tuple_stop->DeleteIfAllowed(); 
  }
  sort(station_list.begin(), station_list.end());
  
  const double dist_delta = 0.01; 
  unsigned int temp_count = 1;
  for(unsigned int i = 0;i < station_list.size();i++){
    vector<BusStop_Ext> station_list_new; 
  
    station_list_new.push_back(station_list[i]);
  
    ////////collect all bus stops mapping to the same 2D point in space/////
    unsigned int j = i + 1;
    BusStop_Ext bse = station_list_new[0]; 
//    bse.Print();
    
    while(j < station_list.size() &&
        station_list[j].loc.Distance(bse.loc) < dist_delta ){
        station_list_new.push_back(station_list[j]);
        j++; 
    }
    i = j - 1; 
    ///////////////////process train station list new ///////////////////////
    CreateLocTable_Compact(station_list_new,temp_count);
    temp_count++; 

//     break; 
  }  

}


/*
find the time table for one spatial location 
Time tables for trains are the same on Sunday and Monday 
Compact Storage -- [start time of first trip, start time of last trip]
schedule time interval 

*/

void UBTrain::CreateLocTable_Compact(vector<BusStop_Ext> station_list_new,
                                     int count_id)
{
//  cout<<"size "<<station_list_new.size()<<endl; 

  Point loc = station_list_new[0].loc; 
  
//  cout<<"bus stop location "<<loc<<endl; 
  
  for(unsigned int i = 0;i < station_list_new.size();i++){
      int br_id = station_list_new[i].br_id;
      int stop_id = station_list_new[i].br_stop_id;
      /////use btree to find all bus trips of this route ///////
     CcInt* search_trip_id = new CcInt(true, br_id);
     BTreeIterator* btree_iter = btree1->ExactMatch(search_trip_id);
     
     vector<MPoint> trip_up;
     vector<MPoint> trip_down; 
     
     bool one_day = false; 
     int bus_day = -1; 
     while(btree_iter->Next()){
        Tuple* tuple_trip = rel2->GetTuple(btree_iter->GetId(), false);
        int br_trip_id = 
            ((CcInt*)tuple_trip->GetAttribute(T_LINE))->GetIntval();
        bool trip_direction = 
            ((CcBool*)tuple_trip->GetAttribute(T_UP))->GetBoolval();
        assert(br_id == br_trip_id);

        MPoint* mo = (MPoint*)tuple_trip->GetAttribute(T_TRIP);
        UPoint up;
        mo->Get(0, up);
        if(one_day == false){
          bus_day = up.timeInterval.start.GetDay();
          one_day = true; 
          if(trip_direction)
            trip_up.push_back(*mo);
          else
            trip_down.push_back(*mo);
        }else if(up.timeInterval.start.GetDay() == bus_day){//the same day 
          assert(bus_day != -1);
//          cout<<"bus day "<<bus_day<<endl; 
          if(trip_direction)
            trip_up.push_back(*mo);
          else
            trip_down.push_back(*mo);
        }
        tuple_trip->DeleteIfAllowed();
     }
     delete btree_iter;
     delete search_trip_id;
    ////////////////////////////////////////////////////////////////////
    
//    cout<<"up size "<<trip_up.size()<<endl;
//    cout<<"down size "<<trip_up.size()<<endl; 
    
    TimeTableCompact(trip_up,loc,br_id,stop_id,true,count_id);
    TimeTableCompact(trip_down,loc,br_id,stop_id,false,count_id);
  }

}

/*
extract the time for one route in one direction 

*/
void UBTrain::TimeTableCompact(vector<MPoint>& trip_list, Point loc,int br_id,
                               int stop_id, bool dir, int count_id)
{
  assert(trip_list.size() >= 2);
  
  MPoint start_mo_0 = trip_list[0];
  UPoint up1;
  start_mo_0.Get(0, up1);//the start time of first trip 
  Instant start_time_0 = up1.timeInterval.start;
  Instant st = start_time_0;
  GetTimeInstantStop(start_mo_0, loc,st); 
  
  /////////////cut second and millsecond ///////////////////////////////
  int second_val_s = st.GetSecond(); 
  int msecond_val_s = st.GetMillisecond(); 
  double double_s = st.ToDouble() - 
                       second_val_s/(24.0*60.0*60.0) -
                       msecond_val_s/(24.0*60.0*60.0*1000.0);
  st.ReadFrom(double_s);
  /////////////////////////////////////////////////////////////////
  
  MPoint start_mo_1 = trip_list[1];
  UPoint up2;
  start_mo_1.Get(0, up1);//the start time of second trip 
  Instant start_time_1 = up1.timeInterval.start;
  

  ///////////  get time interval for schedule  ///////////////////
  double sch_interval = start_time_1.ToDouble() - start_time_0.ToDouble();
  assert(sch_interval > 0.0);
//  cout<<"schedule "<<sch_interval*24.0*60.0*60.0<<" seconds"<<endl; 
  
  ////////////////last bus trip ////////////////////////////////////
  MPoint end_mo = trip_list[trip_list.size() - 1];
  UPoint up3;
  end_mo.Get(0, up3);//the start time of last trip 
  Instant end_time  = up3.timeInterval.start; 
  /////////////////cut second and millsecond ////////////////////////////////
  Instant et = end_time;
  
  GetTimeInstantStop(end_mo, loc, et); 

  int second_val_e = et.GetSecond(); 
  int msecond_val_e = et.GetMillisecond(); 
  double double_e = et.ToDouble() - 
                       second_val_e/(24.0*60.0*60.0) -
                       msecond_val_e/(24.0*60.0*60.0*1000.0);
  et.ReadFrom(double_e);
  ////////////////////////////////////////////////////////////////////////////
  assert(second_val_s == second_val_e); 
  Interval<Instant> time_span;
  time_span.start = st;
  time_span.lc = true;
  time_span.end = et;
  time_span.rc = true; 

   Periods* peri = new Periods(0);
   peri->StartBulkLoad();
   peri->MergeAdd(time_span);
   peri->EndBulkLoad();
//   duration.push_back(*peri);
//   cout<<"periods "<<*peri<<endl; 


//   const double dist_delta = 0.01; 
//   const double stop_time = 10.0/(24.0*60.0*60.0); //10 seconds for trains 
  
  stop_loc_list.push_back(loc);
  line_id_list.push_back(br_id);
  stop_id_list.push_back(stop_id);
  direction_list.push_back(dir);
  duration.push_back(*peri);
  loc_id_list.push_back(count_id);
  schedule_interval.push_back(sch_interval);
  delete peri; 
   
  ////////////////////////    check time  ////////////////////////////// 
  for(unsigned int i = 0;i < trip_list.size();i++){ 
    MPoint mo = trip_list[i];
    Instant temp_instant;
    GetTimeInstantStop(mo, loc, temp_instant);
    int second_val = temp_instant.GetSecond();
    assert(second_val == second_val_s);
  }
  //////////////////////////////////////////////////////////////////////// 
}

/*
get the time that the train arrives at this point(stop)

*/
void UBTrain::GetTimeInstantStop(MPoint& mo, Point loc, Instant& arrove_t)
{
  const double dist_delta = 0.01; 
  const double stop_time = 10.0/(24.0*60.0*60.0); //10 seconds for trains 
  for(int j = 0;j < mo.GetNoComponents();j++){
      UPoint up;
      mo.Get(j, up);
      Point loc1 = up.p0;
      Point loc2 = up.p1; 

      if(loc1.Distance(loc2) < dist_delta && 
         loc1.Distance(loc) < dist_delta){ //find the place 
            Instant st = up.timeInterval.start;
            Instant et = up.timeInterval.end; 
            double d_st = st.ToDouble();
            double d_et = et.ToDouble();
            assert(AlmostEqual(fabs(d_st-d_et), stop_time) || 
                   fabs(d_st-d_et) > stop_time);//check 10 seconds
            arrove_t = st; 
            return; 
      }
 }
  assert(false);
}

/*
split the UBhan represent to get the line id 
the name for the UBahn is the form   (U1, U12, U15)

*/
void UBTrain::SplitUBahn(int attr1, int attr2)
{
  vector<UBhan_Id_Geo> ub_lines; 
  
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<endl; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* ubahn_tuple = rel1->GetTuple(i, false);
    string name = ((CcString*)ubahn_tuple->GetAttribute(attr1))->GetValue(); 
//    cout<<"name "<<name<<endl; 
    char* str = new char[name.size() + 1];
    strcpy(str, name.c_str()); 
    char* sub_str = strtok(str, ","); 
    while(sub_str != NULL){
      string number1(sub_str);
//      cout<<number1<<endl; 
      string number2(number1,1);
//      cout<<number2<<endl; 
      int id = atoi(number2.c_str()); 
//      cout<<"id "<<id<<endl; 
      sub_str=strtok(NULL, ",");
      Line* l = (Line*)ubahn_tuple->GetAttribute(attr2); 
      AddToUBahn(id, l, ub_lines);
    } 
    delete[] str; 

    ubahn_tuple->DeleteIfAllowed();
  }

  for(unsigned int i = 0;i < ub_lines.size();i++){
    line_id_list.push_back(ub_lines[i].lineid);
    SimpleLine* sl = new SimpleLine(0);
    sl->fromLine(ub_lines[i].geodata);
    geodata.push_back(*sl); 
    delete sl;
  }

}

/*
add the line with id to the result vector 

*/
void UBTrain::AddToUBahn(int id, Line* l, vector<UBhan_Id_Geo>& ub_lines)
{
  if(ub_lines.size() == 0){
    UBhan_Id_Geo* idgeo = new UBhan_Id_Geo(id, *l);
    ub_lines.push_back(*idgeo);
    delete idgeo; 
  }else{
    unsigned int i = 0;
    for(;i < ub_lines.size();i++){
        if(ub_lines[i].lineid == id){
          Line* res = new Line(0);
          ub_lines[i].geodata.Union(*l, *res); 
          ub_lines[i].geodata = *res; 
          delete res; 
          break; 
        }
    }
    if(i == ub_lines.size()){
        UBhan_Id_Geo* idgeo = new UBhan_Id_Geo(id, *l);
        ub_lines.push_back(*idgeo);
        delete idgeo; 
    }
  }
}

string UBTrain::TrainsTypeInfo = 
"(rel (tuple ((Id int) (Line int) (Up bool) (Trip mpoint))))"; 

string UBTrain::UBahnLineInfo = 
"(rel (tuple ((lineid int) (geoData sline))))";

/*
convert berlintest trains to generic moving objects 

*/
void UBTrain::TrainsToGenMO()
{
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* train_tuple = rel1->GetTuple(i, false); 
    int id = ((CcInt*)train_tuple->GetAttribute(TRAIN_ID))->GetIntval();
    int line_id = ((CcInt*)train_tuple->GetAttribute(TRAIN_LINE))->GetIntval();
    bool up = ((CcBool*)train_tuple->GetAttribute(TRAIN_UP))->GetBoolval();
    MPoint* mp = ((MPoint*)train_tuple->GetAttribute(TRAIN_TRIP)); 
    
    GenMO* genmo = new GenMO(0); 
    
    MPToGenMO(mp, genmo, line_id, up); 
    
    id_list.push_back(id); 
    line_id_list.push_back(line_id); 
    direction_list.push_back(up);
    genmo_list.push_back(*genmo);

    delete genmo; 
    train_tuple->DeleteIfAllowed(); 


//    break; 
  }

}

/*
convert a moving point to a generic moving object 

*/
void UBTrain::MPToGenMO(MPoint* mp, GenMO* mo, int l_id, bool up)
{
    mo->StartBulkLoad(); 
    CcInt* search_id = new CcInt(true, l_id);
    BTreeIterator* btree_iter = btree1->ExactMatch(search_id);
    SimpleLine* sl = new SimpleLine(0);
    int ub_line_id; 
    while(btree_iter->Next()){
        Tuple* tuple = rel2->GetTuple(btree_iter->GetId(), false);
        ub_line_id = ((CcInt*)tuple->GetAttribute(UB_LINE_ID))->GetIntval();
        SimpleLine* l = (SimpleLine*)tuple->GetAttribute(UB_LINE_GEODATA); 
        *sl = *l; 
        tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;
    assert(ub_line_id == l_id); 
//    cout<<"line length "<<sl->Length()<<endl; 
    
    for(int i = 0;i < mp->GetNoComponents();i++){
      UPoint unit1;
      mp->Get(i, unit1); 
      double pos1;
      double pos2;
      assert(sl->AtPoint(unit1.p0, up, pos1));
      assert(sl->AtPoint(unit1.p1, up, pos2));
//      cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl; 


      Loc loc1(pos1,-1); 
      Loc loc2(pos2,-1); 
      GenLoc gloc1(ub_line_id, loc1);
      GenLoc gloc2(ub_line_id, loc2);
      int tm = GetTM("Tube"); 
      //////////////////////////////////////////////////////////////////
      /////////////correct way to create UGenLoc///////////////////////
      //////////////////////////////////////////////////////////////////
      UGenLoc* unit2 = new UGenLoc(unit1.timeInterval, gloc1, gloc2, tm);
//      if(i %2 == 0) //for debuging time intervals are not consequent 
        mo->Add(*unit2); 
      delete unit2; 
      
    }
    delete sl; 
    
    mo->EndBulkLoad();
}



