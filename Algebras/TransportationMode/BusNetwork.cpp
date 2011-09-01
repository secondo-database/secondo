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
#include "PaveGraph.h"


string BusRoute::StreetSectionCellTypeInfo = 
"(rel (tuple ((Secid int)(Cellid int) (Cnt int) (Cover_area region))))";

string BusRoute::BusRoutesTmpTypeInfo = 
"(rel (tuple ((br_id int) (bus_route1 gline) (bus_route2 line)\
(start_loc point)(end_loc point) (route_type int))))";

string BusRoute::NewBusRoutesTmpTypeInfo =
"(rel (tuple ((br_id int) (bus_route1 line) (bus_route2 line) (route_type int)\
(br_uid int))))";

string BusRoute::FinalBusRoutesTypeInfo =
"(rel (tuple ((br_id int) (bus_route line) (route_type int) (br_uid int)\
(bus_direction bool) (startSmaller bool))))";


string BusRoute::BusStopTemp1TypeInfo =
"(rel (tuple ((br_id int) (bus_stop_id int) (bus_stop1 gpoint)\
(bus_stop2 point))))";


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

Berlin and Houston have different roads distribution.

*/
void BusRoute::CreateRoute1(int attr1,int attr2,int attr3,string type)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<" attr3 "<<attr3<<endl; 
//  cout<<"CreateBusRoute()"<<endl; 

  int max_cell_id = -1; 
  int max_cnt = -1;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_cell = rel1->GetTuple(i,false);
    CcInt* cell_id = (CcInt*)tuple_cell->GetAttribute(attr1);
    CcInt* cnt = (CcInt*)tuple_cell->GetAttribute(attr2); 
    
//    cout<<sec_id->GetIntval()<<" "
//        <<cell_id->GetIntval()<<" "<<cnt->GetIntval()<<endl; 
  
    if(cell_id->GetIntval() > max_cell_id)
        max_cell_id = cell_id->GetIntval(); 
    
    if(cnt->GetIntval() > max_cnt) 
      max_cnt = cnt->GetIntval(); 

    tuple_cell->DeleteIfAllowed(); 
  }

//  cout<<"max_cell_id "<<max_cell_id<<endl;
//  cout<<"max_cnt "<<max_cnt<<endl;

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

//  float div_number1 = 0.5; 
    float div_number1 = 0.4;

//    float div_number2 = 0.2;
    float div_number2 = 0.25;


  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple_cell = rel1->GetTuple(i,false);

    CcInt* cell_id = (CcInt*)tuple_cell->GetAttribute(attr1);
    CcInt* cnt = (CcInt*)tuple_cell->GetAttribute(attr2); 
    Region* reg = (Region*)tuple_cell->GetAttribute(attr3);
    
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

//  cout<<"cell list size "<<cell_list.size()<<endl;

  //create priority_queue 
  int count = 0;

  priority_queue<Section_Cell> cell_queue; //////////priority_queue
  ////////////cells are ordered by intersecting number from large to small////
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
  //3---13  2---49  1---204 //berlin roads---div1 0.5
  //3---21  2---41  1---204 //berlin roads---div1 0.4
  //3---27  2---21  1---218 //berlin roads---div1 0.4; div 0.25

  //3---10 2---8 1---619 //houston roads

//    cout<<cell_list3.size()<<" "
//        <<cell_list2.size()<<" "<<cell_list1.size()<<endl;

   if(type == "Berlin"){

    BuildRoute(cell_list3, cell_list1, 1, true);//type 1

    BuildRoute(cell_list2, cell_list1, 2, false);//type 2

  //  unsigned int limit_no = 20;
      unsigned int limit_no = 45;
//    unsigned int limit_no = 30;

    BuildRoute_Limit(cell_list1, cell_list1, limit_no);
   }else if(type == "Houston"){

    BuildRoute(cell_list3, cell_list1, 1, true);//type 1
    BuildRoute(cell_list2, cell_list1, 2, false);//type 2

    unsigned int limit_no = 70;

    BuildRoute_Limit2(cell_list1, cell_list1, limit_no);

   }else{
    cout<<"type: "<<type<<" not processed"<<endl;
    assert(false);
   }

}

/*
highest density area with low density area

*/
void BusRoute::BuildRoute(vector<Section_Cell>& from_cell_list,
                             vector<Section_Cell> to_cell_list,
                             int type, bool start)
{
  for(unsigned int i = 0;i < from_cell_list.size();i++){
    Section_Cell elem = from_cell_list[i];

//      float dist_val = 10000.0;//Euclidean distance between two cells

//      float dist_val = 12000.0;//Euclidean distance between two cells

      float dist_val = 16000.0;//Euclidean distance between two cells

      int end_cellid = 
          FindEndCell1(from_cell_list[i],to_cell_list,dist_val, start); 

      if(end_cellid >= 0){
//        cout<<"start cell "<<from_cell_list[i].cell_id
//          <<" end cell "<<to_cell_list[end_cellid].cell_id<<endl; 

          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id);


          bus_route_type.push_back(type);
      }
  }

}

/*
lown density area with low density area
with the limit number of bus routes 

*/
void BusRoute::BuildRoute_Limit(vector<Section_Cell>& from_cell_list,
                             vector<Section_Cell> to_cell_list,
                             unsigned int limit_no)
{
  unsigned int count = 0;
  for(unsigned int i = 0;i < from_cell_list.size();i++){
    Section_Cell elem = from_cell_list[i];

    if(from_cell_list[i].def == false) continue; 

//      float dist_val = 15000.0;//Euclidean distance between two cells
      float dist_val = 22000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell2(from_cell_list[i],to_cell_list,dist_val); 

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
          count++;
    }
    if(count > limit_no/2) break; 
  } 

  count = 0;
  for(int i = from_cell_list.size() - 1;i >= 0;i--){
    Section_Cell elem = from_cell_list[i];

    if(from_cell_list[i].def == false) continue; 

      float dist_val = 22000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell2(from_cell_list[i],to_cell_list,dist_val); 

      if(end_cellid >= 0){

          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id); 

          bus_route_type.push_back(3);//the lowest route 

          to_cell_list[i].def = false; 
          from_cell_list[end_cellid].def = false; 
          count++;
    }
    if(count > limit_no/2) break; 
  }

}

/*
lown density area with low density area
with the limit number of bus routes for Houston 
larger area than Berlin, so the distance value is also large

*/
void BusRoute::BuildRoute_Limit2(vector<Section_Cell>& from_cell_list,
                             vector<Section_Cell> to_cell_list,
                             unsigned int limit_no)
{
  unsigned int count = 0;
  for(unsigned int i = 0;i < from_cell_list.size();i++){
    Section_Cell elem = from_cell_list[i];

    if(from_cell_list[i].def == false) continue; 

      float dist_val = 40000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell2(from_cell_list[i],to_cell_list,dist_val); 

      if(end_cellid >= 0){

          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id); 

          bus_route_type.push_back(3);//the lowest route 

          to_cell_list[i].def = false; 
          from_cell_list[end_cellid].def = false; 
          count++;
    }
    if(count > limit_no/2) break; 

  }
  
    count = 0;
    for(int i = from_cell_list.size() - 1;i >= 0;i--){
      Section_Cell elem = from_cell_list[i];

      if(from_cell_list[i].def == false) continue; 

      float dist_val = 40000.0;//Euclidean distance between two cells
      int end_cellid = FindEndCell2(from_cell_list[i],to_cell_list,dist_val); 

      if(end_cellid >= 0){

          start_cells.push_back(from_cell_list[i].reg);
          end_cells.push_back(to_cell_list[end_cellid].reg);
          start_cell_id.push_back(from_cell_list[i].cell_id);
          end_cell_id.push_back(to_cell_list[end_cellid].cell_id); 

          bus_route_type.push_back(3);//the lowest route 

          to_cell_list[i].def = false; 
          from_cell_list[end_cellid].def = false; 
          count++;
    }
    if(count > limit_no/2) break; 
  }

}

/*
use the first paramter as the start cell and find a cell from the
second parameter that the distance between them is larger than a value 
if flag is true, from small index to large. otherwise from large to small

*/
int BusRoute::FindEndCell1(Section_Cell& start_cell, 
                           vector<Section_Cell>& cell_list, 
                           float dist_val, bool flag)
{
  if(flag){
    for(unsigned int i = 0;i < cell_list.size();i++){
      if(cell_list[i].def){
          if(start_cell.reg.Distance(cell_list[i].reg) > dist_val){
            cell_list[i].def = false;
            return i; 
          }
      }
    }
  }else{
      for(int i = cell_list.size() - 1;i >= 0;i--){
      if(cell_list[i].def){
          if(start_cell.reg.Distance(cell_list[i].reg) > dist_val){
            cell_list[i].def = false;
            return i; 
          }
      }
    }
  }

  return -1; 
}

/*
use the first paramter as the start cell and find a cell from the
second parameter that the distance between them is larger than a value 

*/
int BusRoute::FindEndCell2(Section_Cell& start_cell,
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

void BusRoute::CreateRoute2(Space* sp, int attr,int attr1,int attr2,int attr3)
{
//  cout<<"attr "<<attr<<" attr1 "<<attr1 <<" attr2 "<<attr2<<endl; 
//  cout<<"CreateRoute2()"<<endl; 

  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  if(rn == NULL){
    cout<<"load road network error"<<endl;
    return;
  }
  n = rn;
  
  RoadGraph* rg = sp->LoadRoadGraph();

  for(int i = 1;i <= rel2->GetNoTuples();i++){

    Tuple* tuple_cell_pair = rel2->GetTuple(i, false);
    int from_cell_id = 
        ((CcInt*)tuple_cell_pair->GetAttribute(attr1))->GetIntval();
    int end_cell_id =
        ((CcInt*)tuple_cell_pair->GetAttribute(attr2))->GetIntval();
        
    int route_type =
        ((CcInt*)tuple_cell_pair->GetAttribute(attr3))->GetIntval();
//    cout<<"from_cell "<<from_cell_id<<" end_cell "<<end_cell_id<<endl; 

//    cout<<"route_type "<<route_type<<endl; 

    ConnectCell(rg, attr,from_cell_id,end_cell_id,route_type, i);
    tuple_cell_pair->DeleteIfAllowed(); 
    

  }

  sp->CloseRoadGraph(rg);
  sp->CloseRoadNetwork(rn);

  ///////////////////add several special routes//////////////////////////
  /////////maybe not the shortest path, cycle route//////////////////////
  ////////////////////////////////////////////////////////////////////////


}

/*
randomly choose two positions in two cells and find the shortest path
connecting them. use the shortest path as the bus route 

*/

void BusRoute::ConnectCell(RoadGraph* rg, int attr,int from_cell_id,
                           int end_cell_id, int route_type, int seed)
{
    Relation* routes = n->GetRoutes();
    RoadNav* road_nav = new RoadNav();

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
    
    Tuple* road_tuple1 = routes->GetTuple(gp1->GetRouteId(), false);
    SimpleLine* sl1 = (SimpleLine*)road_tuple1->GetAttribute(ROUTE_CURVE);
    assert(sl1->GetStartSmaller());
    assert(sl1->AtPosition(gp1->GetPosition(), true, *location1));
    road_tuple1->DeleteIfAllowed();

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

    Tuple* road_tuple2 = routes->GetTuple(gp2->GetRouteId(), false);
    SimpleLine* sl2 = (SimpleLine*)road_tuple2->GetAttribute(ROUTE_CURVE);
    assert(sl2->GetStartSmaller());
    assert(sl2->AtPosition(gp2->GetPosition(), true, *location2));
    road_tuple2->DeleteIfAllowed();

    end_gp.push_back(*location2); 


    GLine* gl = new GLine(0);

 //   cout<<"gp1 "<<*gp1<<" gp2 "<<*gp2<<endl; 

//    road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);

//     if(route_type == 1 || route_type == 2){
//         road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
//     }else{ //////paths avoid city center area 
//       road_nav->ShortestPathSub2(gp1, gp2, rg, n, gl);
//       if(gl->IsDefined() == false){//in case cannot find such a path 
//         road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
//       }
//     }

    if(route_type == 1){
        if(GetRandom() % 2 == 0)
          road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
        else{
          road_nav->ShortestPathSub3(gp1, gp2, rg, n, gl);
        if(gl->IsDefined() == false)
          road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
        }
    }else if(route_type == 2){
       if(GetRandom() % 2 == 0){
          road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
       }else{
        road_nav->ShortestPathSub3(gp1, gp2, rg, n, gl);
        if(gl->IsDefined() == false)
          road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
       }
    }else{ //////paths avoid city center area 
      if(GetRandom() % 2 == 0){
        road_nav->ShortestPathSub2(gp1, gp2, rg, n, gl);
        if(gl->IsDefined() == false){//in case cannot find such a path 
          road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
        }
      }else{
        road_nav->ShortestPathSub3(gp1, gp2, rg, n, gl);
        if(gl->IsDefined() == false){//in case cannot find such a path 
          road_nav->ShortestPathSub(gp1, gp2, rg, n, gl);
        }
      }
    }

    delete location1; 
    delete gp1;

    delete location2; 
    delete gp2; 
    
    tuple_sec_1->DeleteIfAllowed();
    tuple_sec_2->DeleteIfAllowed(); 
    //////////////////////////////////////////////////////////////////////////

    bus_lines1.push_back(*gl);

    //////////////////////////////////////////////////////////////////////////
    Line* l = new Line(0);
    gl->Gline2line(l);
    bus_lines2.push_back(*l);
    delete l; 
    delete gl; 

    bus_route_type.push_back(route_type);

    delete road_nav;

} 


/*
there are bugs of computing shortest path in road network. 
if the result is not correct, return false

*/

bool BusRoute::ConvertGLine(GLine* gl, GLine* newgl)
{
   newgl->SetNetworkId(gl->GetNetworkId());
   
   
   vector<RouteInterval> ri_list; 
   for(int i = 0; i < gl->Size();i++){
    RouteInterval* ri = new RouteInterval();
    gl->Get(i, *ri); 
    

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
            }else {
              delete ri;

              return false;
            }
        }else
          ri_list.push_back(*ri);
      }

    }
    delete ri; 
   }

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

//        cout<<*gp1<<" "<<*gp2<<endl;

        delete gp_p;
        delete p2;
        delete gp2; 
        delete p1;
        delete gp1;
   }

  if(ri_list.size() < 2) return false;

   vector<bool> temp_start_from; 
   const double dist_delta = 0.001; 
   for(unsigned int i = 0; i < gp_p_list.size() - 1;i++){
    GP_Point gp_p1 = gp_p_list[i];
    GP_Point gp_p2 = gp_p_list[i + 1];

//     cout<<"gp1 loc1 "<<gp_p1.loc1<<" gp1 loc2 "<<gp_p1.loc2
//         <<"gp2 loc1 "<<gp_p2.loc1<<" gp2 loc2 "<<gp_p2.loc2<<endl; 


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
    }else {

        return false;
      }

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
    }else {
      return false;
    }

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
    return true;
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


    if(routes_def[br_id1 - 1] == false){
        tuple_bus_route1->DeleteIfAllowed();
        continue; 
    }
    GLine* gl1 = (GLine*)tuple_bus_route1->GetAttribute(attr2);
    
    for(int j = i + 1; j <= rel1->GetNoTuples();j++){
      Tuple* tuple_bus_route2 = rel1->GetTuple(j, false);
      int br_id2 = ((CcInt*)tuple_bus_route2->GetAttribute(attr1))->GetIntval();
      

      if(routes_def[br_id2 - 1] == false){
        tuple_bus_route2->DeleteIfAllowed();
        continue; 
      }
     GLine* gl2 = (GLine*)tuple_bus_route2->GetAttribute(attr2);

//     cout<<"br_id1 "<<br_id1<<" br_id2 "<<br_id2<<endl; 
//     cout<<gl1->GetLength()<<" "<<gl2->GetLength()<<endl;

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

      Tuple* tuple_bus_route = rel1->GetTuple(i, false);
      int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
      Line* l = (Line*)tuple_bus_route->GetAttribute(attr2);
      int route_type = 
          ((CcInt*)tuple_bus_route->GetAttribute(attr3))->GetIntval();

//       if(br_id != 38){
//         tuple_bus_route->DeleteIfAllowed();
//         continue;
//       }

      ////////////////////////translate/////////////////////////////////    
      SimpleLine* sl = new SimpleLine(0);
      sl->fromLine(*l);

      SpacePartition* sp = new SpacePartition();
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
      sp->ReorderLine(sl, seq_halfseg);

//      cout<<"l1 "<<l->Length()<<" l2 "<<sl->Length()<<endl; 
//      cout<<"br_id "<<br_id<<endl;

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

      ////////////////////////////////////////////////////////////

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

//   for(unsigned int i = 0;i < point_list.size() - 1;i++){
//     Point lp = point_list[i];
//     Point rp = point_list[i + 1];
// 
//     HalfSegment* hs = new HalfSegment(true, lp, rp);
//     hs->attr.edgeno = edgeno++;
//     *l += *hs;
// 
//     hs->SetLeftDomPoint(!hs->IsLeftDomPoint());
//     *l += *hs;
// 
//     delete hs;
// 
//   }
 
  for(unsigned int i = 0;i < point_list.size() - 1;i++){
    Point lp = point_list[i];
    Point rp = point_list[i + 1];
    HalfSegment hs1(true, lp, rp);
    unsigned int j = i + 2;
    for(; j < point_list.size() - 1;j++){
      Point p1 = point_list[j];
      Point p2 = point_list[j + 1];
      HalfSegment hs2(true, p1, p2);
      Point res;
      if(hs1.Intersection(hs2, res)){
//        cout<<"crossing"<<endl;
        point_list[j] = res;
        point_list[j + 1] = res;
        break;
      }
    }
    if(j >= point_list.size() - 1){
        HalfSegment* hs = new HalfSegment(true, lp, rp);
        hs->attr.edgeno = edgeno++;
       *l += *hs;
        hs->SetLeftDomPoint(!hs->IsLeftDomPoint());
        *l += *hs;
        delete hs;
    }else{
        rp = point_list[j];
        HalfSegment* hs = new HalfSegment(true, lp, rp);
        hs->attr.edgeno = edgeno++;
       *l += *hs;
        hs->SetLeftDomPoint(!hs->IsLeftDomPoint());
        *l += *hs;
        delete hs;
        i = j;
    }

  }


}

/*
for each bus route, it creates a sequence of points on it as bus stops 

*/
void BusRoute::CreateBusStop1(int attr1, int attr2, int attr3, 
                              int attr4, Relation* pave_rel, 
                              BTree* btree_pave, string type)
{
    ////////////initialize distance for stops///////////////////////
    vector<double> dist_stops1;//for type 1
    vector<double> dist_stops2;//for type 2
    vector<double> dist_stops3;//for type 3
    InitializeDistStop(dist_stops1, dist_stops2, dist_stops3, type);
    ///////////////////////////////////////////////////////
  
    for(int i = 1;i <= rel1->GetNoTuples();i++){

      Tuple* tuple_bus_route = rel1->GetTuple(i, false);
      int br_id = ((CcInt*)tuple_bus_route->GetAttribute(attr1))->GetIntval();
      GLine* gl = (GLine*)tuple_bus_route->GetAttribute(attr2); 
      Line* l = (Line*)tuple_bus_route->GetAttribute(attr3);
      int route_type = 
          ((CcInt*)tuple_bus_route->GetAttribute(attr4))->GetIntval();

//       if(br_id != 5){
//           tuple_bus_route->DeleteIfAllowed();
//           continue;
//       }

//      cout<<"br_id "<<br_id<<endl; 
//      cout<<"route_type "<<route_type<<endl; 

      unsigned int cur_size = bus_stop_loc_1.size(); 

      CreateStops(br_id, gl,l, route_type, 
                  dist_stops1, dist_stops2, dist_stops3);
      tuple_bus_route->DeleteIfAllowed();

      ///modify the bus stops if they are located on the zebracrossing/////
      CheckBusStopZC(cur_size, pave_rel, btree_pave); 
    }
}

/*
set the distance value for two adjacent bus stops

*/
void BusRoute::InitializeDistStop(vector<double>& dist_for_stops1,
                                  vector<double>& dist_for_stops2,
                                  vector<double>& dist_for_stops3, 
                                  string type)
{
  if(type == "Berlin"){
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

    dist_for_stops1.push_back(800.0);////////add smaller distance
    dist_for_stops1.push_back(500.0);////////add smaller distance
  }else if(type == "Houston"){
    dist_for_stops1.push_back(1900.0);
    dist_for_stops1.push_back(2000.0); 
    dist_for_stops1.push_back(2100.0); 
    dist_for_stops1.push_back(1800.0);
    dist_for_stops1.push_back(2300.0);
    dist_for_stops1.push_back(2400.0);
    dist_for_stops1.push_back(2200.0);
    dist_for_stops1.push_back(2500.0);

  }else {
    cout<<"not processed"<<endl;
    assert(false);
  }


  if(type == "Berlin"){
    dist_for_stops2.push_back(900.0);
    dist_for_stops2.push_back(1100.0);
    dist_for_stops2.push_back(1150.0); 
    dist_for_stops2.push_back(1250.0); 
    dist_for_stops2.push_back(1050.0); 
    dist_for_stops2.push_back(1000.0);
    dist_for_stops2.push_back(1200.0);

    dist_for_stops2.push_back(800.0);////////add smaller distance
    dist_for_stops2.push_back(500.0);////////add smaller distance
  }else if(type == "Houston"){
    dist_for_stops2.push_back(2000.0);
    dist_for_stops2.push_back(2200.0);
    dist_for_stops2.push_back(1800.0);
    dist_for_stops2.push_back(2100.0);
    dist_for_stops2.push_back(1900.0);
    dist_for_stops2.push_back(2300.0);
    dist_for_stops2.push_back(2400.0);
    dist_for_stops2.push_back(2500.0);

  }else{
    cout<<"not processed"<<endl;
    assert(false);
  }
  
  if(type == "Berlin"){
    dist_for_stops3.push_back(900.0);
    dist_for_stops3.push_back(1100.0);
    dist_for_stops3.push_back(950.0);
    dist_for_stops3.push_back(1300.0);
    dist_for_stops3.push_back(1000.0);
    dist_for_stops3.push_back(1200.0);
    dist_for_stops3.push_back(1150.0);

    dist_for_stops3.push_back(800.0); ////////add smaller distance
    dist_for_stops3.push_back(500.0);
  }else if(type == "Houston"){
  
    dist_for_stops3.push_back(2000.0);
    dist_for_stops3.push_back(2200.0); 
    dist_for_stops3.push_back(2100.0); 
    dist_for_stops3.push_back(1900.0);
    dist_for_stops3.push_back(1800.0);
    dist_for_stops3.push_back(2300.0); 
    dist_for_stops3.push_back(2400.0);
    dist_for_stops3.push_back(2500.0);

  }else{
    cout<<"not processed"<<endl;
    assert(false);
  }


}

/*
for such a gline, create a set of points on it 

*/

void BusRoute::CreateStops(int br_id, GLine* gl, Line* l, 
                           int route_type,vector<double> dist_for_stops1,
                           vector<double> dist_for_stops2, 
                           vector<double> dist_for_stops3)
{


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
//        next_stop_dist -= fabs(cur_end - cur_start);
        double temp_dist = next_stop_dist - fabs(cur_end - cur_start);
        if(temp_dist > 0.0){//if smaller than 0, keep the value, skip the part
          next_stop_dist = temp_dist;
        }
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
   
//   cout<<"stop1 "<<*gp<<endl; 

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
 //     cout<<"cur_start "<<cur_start<<" cur_end "<<cur_end<<endl; 

/*      cout<<"rid "<<sec_list[last_sec_index].rid
        <<" secid "<<sec_list[last_sec_index].secttid
        <<" start pos "<<sec_list[last_sec_index].start
        <<" end pos "<<sec_list[last_sec_index].end<<endl; */
 //     cout<<"next_stop_dist "<<next_stop_dist<<endl; 

      ////////////////////////////////////////////////////////////////
      if(fabs(cur_end - cur_start) < 2*dist_to_jun){//close to junction area
        double temp_dist = next_stop_dist - fabs(cur_end - cur_start);
        if(temp_dist > 0.0){//if smaller than 0, keep the value, skip the part
          next_stop_dist = temp_dist;
        }
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
//          cout<<"temp_pos "<<temp_pos<<endl;

          if(fabs(temp_pos - cur_end) < dist_to_jun)
              temp_pos = cur_end - dist_to_jun;
          if(fabs(temp_pos - cur_start) < dist_to_jun)
              temp_pos = cur_start + dist_to_jun;

          last_sec_gp_pos = temp_pos;   
        }

//        cout<<"last_sec_gp_pos: "<<last_sec_gp_pos<<endl;

        last_sec_start = cur_start;
        last_sec_end = cur_end; 
        rid = sec_list[last_sec_index].rid;      
        break; 
      }else
        next_stop_dist -= fabs(cur_end - cur_start); 
    
    }
      if(last_sec_index == sec_list.size()) return false; 
    
    }
    
    GPoint* gp = new GPoint(true,n->GetId(),rid,last_sec_gp_pos,None);
    bus_stop_loc_1.push_back(*gp);
    Point* p = new Point();
    gp->ToPoint(p);
    bus_stop_loc_2.push_back(*p);

//    cout<<" stop2 "<<*gp<<endl;

    delete p;
    delete gp;

    return true; 
  }

}

/*
if the bus stops are located on the zebra crossing, we modify the position by
  a small distance 

*/
void BusRoute::CheckBusStopZC(unsigned int cur_size, Relation* pave_rel, 
                    BTree* btree_pave)
{
  for(;cur_size < bus_stop_loc_1.size(); cur_size++){

    GPoint gp = bus_stop_loc_1[cur_size]; 
    Point p = bus_stop_loc_2[cur_size]; 
    int rid = gp.GetRouteId(); 

    CcInt* search_id = new CcInt(true, rid);
    BTreeIterator* btree_iter = btree_pave->ExactMatch(search_id);

    while(btree_iter->Next()){
        Tuple* pave_tuple = pave_rel->GetTuple(btree_iter->GetId(), false);
        Region* reg = (Region*)pave_tuple->GetAttribute(DualGraph::PAVEMENT);
        /////////////////bus stops are on the zebra crossing/////////////////
        if(p.Inside(*reg)){
//          cout<<"br id "<<br_id_list[cur_size]<<" stop id "
//              <<br_stop_id[cur_size]<<endl;

          Tuple* sec = n->GetSectionOnRoute(&gp);
          double meas1 = 
              ((CcReal*)sec->GetAttribute(SECTION_MEAS1))->GetRealval();
          double meas2 = 
              ((CcReal*)sec->GetAttribute(SECTION_MEAS2))->GetRealval();
//          cout<<gp<<endl;
//          cout<<"meas1 "<<meas1<<" meas2 "<<meas2<<endl;
          sec->DeleteIfAllowed(); 
          double m1 = MIN(meas1, meas2);
          double m2 = MAX(meas1, meas2); 
          double pos = gp.GetPosition();
          assert(pos > m1 && pos < m2); 
          double delta; 
          if((pos - m1) > (m2 - pos)) delta = -2.0; 
          else
            delta = 2.0; 

          GPoint* new_gp = new GPoint(gp);
          while(true){
            new_gp->SetPosition(new_gp->GetPosition() + delta);
            Point* new_p = new Point();
            new_gp->ToPoint(new_p);
            if(new_p->Inside(*reg) == false){
              bus_stop_loc_1[cur_size] = *new_gp;
              bus_stop_loc_2[cur_size] = *new_p; 
              delete new_p;
              break; 
            } 
            delete new_p; 
            assert(new_gp->GetPosition() > m1 && new_gp->GetPosition() < m2); 
          }
          delete new_gp;

        }
        pave_tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;    
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
//    cout<<"br_id "<<br_id<<*gp<<endl;

    Tuple* temp = n->GetSectionOnRoute(gp);
    assert(temp != NULL);

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

//    const double dist_val = 800.0; //the minimum distance between stops
    const double dist_val = 300.0; //the minimum distance between stops
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
//  const double dist_val = 500.0; //distance used to expand road section
//  const double dist_val = 300.0; //distance used to expand road section
  const double dist_val = 200.0; //distance used to expand road section

  int last_br_id = 0; 
  vector<SectTreeEntry> sec_list; 
  vector<bool> start_from; 
  for(unsigned int i = 0;i < bus_stop_list.size();i++){

//      if(bus_stop_list[i].br_id > 10) continue;


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

  assert((int)dist_list.size() == (end - start)); 
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

    if(sl1->Length() < 1.0 || sl2->Length() < 1.0){
      cout<<i<<endl;
      cout<<"not correct routes"<<endl;
      assert(false);
    }

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
get the intersecting points between the create small line and two bus routes 

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
          Point* loc = (Point*)bs_tuple->GetAttribute(RoadDenstiy::BUS_LOC);

//          cout<<"busstop_rid "<<br_id_s<<" stop id "<<s_id<<endl;

//          bus_stop_list.push_back(Bus_Stop(true, br_id_s, s_id));
          bus_stop_list.push_back(Bus_Stop(true, br_id_s, s_id, d));
          bus_stop_geodata.push_back(*loc);
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
//    cout<<"br_id "<<br_id<<endl;

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

  SpacePartition* sp_pt = new SpacePartition();
  vector<MyHalfSegment> mhs;
  sp_pt->ReorderLine(sl, mhs);
  delete sp_pt;


  for(unsigned int i = 0;i < bslist.size() - 1;i++){//ordered by stop id 
      Point loc1 = bslist[i].loc;
      Point loc2 = bslist[i + 1].loc;

//       cout<<"br_id "<<br_id<<endl;
//       cout<<"stop id1 "<<bslist[i].br_stop_id
//           <<" stop id2 "<<bslist[i + 1].br_stop_id<<endl;

//      cout<<"loc1 "<<bslist[i].loc<<" loc2 "<<bslist[i + 1].loc<<endl; 

      //////////////////////////////////////////////////////////////////
      /////////numeric problem, use the new implementation//////////////
      //////////////////////////////////////////////////////////////////
      double pos1 = -1.0;
      double pos2 = -1.0; 

      double newpos1, newpos2;

//      GetPosOnSL(sl, loc1, newpos1);
//      GetPosOnSL(sl, loc2, newpos2);

      //////////////////////////////////////////////////

      GetPosOnSL(sl, loc1, newpos1, mhs);
      GetPosOnSL(sl, loc2, newpos2, mhs);

      ///////////////////////////////////////////

        pos1 = newpos1;
        pos2 = newpos2; 

      /////////////////////////////////////////////////////////////////////
      ////////////////////////section before the first stop////////////////
      /////////////////////////////////////////////////////////////////////
      if(i == 0){
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

      if(sl->GetStartSmaller()){
        sl->SubLine(len1, len2, sl->GetStartSmaller(), sub_l);
      }else{
        sl->SubLine(len2, len1, sl->GetStartSmaller(), sub_l);
      }

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
  br_id_list.push_back(br_id);
  bus_route_list.push_back(*br); 
  delete br; 
}


/*
due to numeric problem, use the following method to find pos for a point on 
a sline

*/
void BusRoute::GetPosOnSL(SimpleLine* sl, Point loc, double& pos, 
                          vector<MyHalfSegment>& mhs)
{
  ////////get the start point of sline /////////////
  Point start_loc;
  assert(sl->AtPosition(0.0, sl->StartsSmaller(), start_loc));
  
  /////////convert to myhalfsegment/////////////////
  SpacePartition* sp = new SpacePartition();

  ////////use getclosespoint//////////////////////
  const double delta_d = 0.001;
  if(mhs[0].from.Distance(start_loc) < delta_d){
    double len = 0;
    unsigned int i = 0;
    for(;i < mhs.size();i++){
      HalfSegment hs(true, mhs[i].from, mhs[i].to);
      Point res;
      sp->GetClosestPoint_New(hs, loc, res);
      if(res.Distance(loc) < delta_d){
        len += res.Distance(mhs[i].from);
        pos = len;
        break;
      }else{
        len += hs.Length();
      }
    }
    if(i == mhs.size())assert(false);


  }else if(mhs[mhs.size() - 1].to.Distance(start_loc) < delta_d){

    double len = 0;
    int i = mhs.size() - 1;
    for(;i >= 0;i--){
      HalfSegment hs(true, mhs[i].from, mhs[i].to);
      Point res;
      sp->GetClosestPoint_New(hs, loc, res);
      if(res.Distance(loc) < delta_d){
        len += res.Distance(mhs[i].to);
        pos = len;
        break;
      }else{
        len += hs.Length();
      }
    }
    if(i < 0)assert(false);

  }else{
    cout<<"should not be here"<<endl;
    assert(false);
  }
  
  delete sp;

  //////////////////testing the result again////////////////
  Point test;
  assert(sl->AtPosition(pos,sl->StartsSmaller(), test));
  assert(test.Distance(loc) < delta_d);

}

/*void BusRoute::GetPosOnSL(SimpleLine* sl, Point loc, double& pos)
{
  ////////get the start point of sline /////////////
  Point start_loc;
  assert(sl->AtPosition(0.0, sl->StartsSmaller(), start_loc));
  
  /////////convert to myhalfsegment/////////////////
  SpacePartition* sp = new SpacePartition();
  vector<MyHalfSegment> mhs;
  sp->ReorderLine(sl, mhs);
  
  ////////use getclosespoint//////////////////////
  const double delta_d = 0.001;
  if(mhs[0].from.Distance(start_loc) < delta_d){
    double len = 0;
    unsigned int i = 0;
    for(;i < mhs.size();i++){
      HalfSegment hs(true, mhs[i].from, mhs[i].to);
      Point res;
      sp->GetClosestPoint_New(hs, loc, res);
      if(res.Distance(loc) < delta_d){
        len += res.Distance(mhs[i].from);
        pos = len;
        break;
      }else{
        len += hs.Length();
      }
    }
    if(i == mhs.size())assert(false);


  }else if(mhs[mhs.size() - 1].to.Distance(start_loc) < delta_d){

    double len = 0;
    int i = mhs.size() - 1;
    for(;i >= 0;i--){
      HalfSegment hs(true, mhs[i].from, mhs[i].to);
      Point res;
      sp->GetClosestPoint_New(hs, loc, res);
      if(res.Distance(loc) < delta_d){
        len += res.Distance(mhs[i].to);
        pos = len;
        break;
      }else{
        len += hs.Length();
      }
    }
    if(i < 0)assert(false);

  }else{
    cout<<"should not be here"<<endl;
    assert(false);
  }
  
  delete sp;

  //////////////////testing the result again////////////////
  Point test;
  assert(sl->AtPosition(pos,sl->StartsSmaller(), test));
  assert(test.Distance(loc) < delta_d);

}*/


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

string RoadDenstiy::rg_nodes_typeinfo = 
"(rel (tuple ((jun_id int) (jun_gp gpoint) (jun_p point) (rid int))))";


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
        if((i + 1) < (int) seq_halfseg.size()){
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
/*              int second_val = schedule_t.GetSecond(); 
              int msecond_val = schedule_t.GetMillisecond(); 
              double double_s = schedule_t.ToDouble() - 
                                second_val/(24.0*60.0*60.0) -
                                msecond_val/(24.0*60.0*60.0*1000.0);*/

              //we should consider second and millisecond 
              double double_s = schedule_t.ToDouble();

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
//  int msecond_val_s = st.GetMillisecond(); 
//   double double_s = st.ToDouble() - 
//                        second_val_s/(24.0*60.0*60.0) -
//                        msecond_val_s/(24.0*60.0*60.0*1000.0);

  double double_s = st.ToDouble();//we should consider second and millisecond

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
/*  int msecond_val_e = et.GetMillisecond(); 
  double double_e = et.ToDouble() - 
                       second_val_e/(24.0*60.0*60.0) -
                       msecond_val_e/(24.0*60.0*60.0*1000.0);*/

  double double_e = et.ToDouble();//we should consider second and millisecond

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

//   br_id_list.push_back(br_id);
//   bus_stop_id_list.push_back(stop_id);
//   br_direction.push_back(dir);

  Bus_Stop* bs = new Bus_Stop(true, br_id, stop_id, dir);
  bs_list.push_back(*bs); 
  bs_uoid_list.push_back(bs->GetUOid());
  delete bs; 


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
get the time that the bus arrives at this point(stop)

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

bool CompareGP_P(const GP_Point& gp_p1, const GP_Point& gp_p2)
{
   if(gp_p1.rid < gp_p2.rid) return true;
   else{
      if(gp_p1.rid > gp_p2.rid) return false;
      else{
        if(gp_p1.pos1 <  gp_p2.pos1) return true; 
        else return false;
      }
   } 

}

/*
create nodes relation for road graph jun id,  gpoint, point 

*/

void RoadDenstiy::GetRGNodes()
{

    Relation* juns = n->GetJunctions();
    
    vector<GP_Point> loc_list;
    
    for(int i = 1;i <= juns->GetNoTuples();i++){

      Tuple* jun_tuple = juns->GetTuple(i, false);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();
      Point* junp = (Point*)jun_tuple->GetAttribute(JUNCTION_POS);

      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

      double pos1 = meas1->GetRealval();
      double pos2 = meas2->GetRealval();


//       unique_id_list.push_back(oid);
//       oid++;
//       gp_list.push_back(*gp1);
//       jun_loc_list.push_back(*junp);
// 
// 
//       unique_id_list.push_back(oid);
//       oid++;
//       gp_list.push_back(*gp2);
//       jun_loc_list.push_back(*junp);
      GP_Point gp_p1(id1, pos1, -1.0, *junp, *junp);
      GP_Point gp_p2(id2, pos2, -1.0, *junp, *junp);
      
      loc_list.push_back(gp_p1);
      loc_list.push_back(gp_p2);

      jun_tuple->DeleteIfAllowed();
    }

    juns->Delete();


    int oid = 1;

    sort(loc_list.begin(), loc_list.end(), CompareGP_P);
    const double delta_dist = 0.001;
    for(unsigned int i = 0;i < loc_list.size();i++){
//      loc_list[i].Print();
        if(oid == 1){
          GPoint* gp = new GPoint(true, n->GetId(), loc_list[i].rid,
                                  loc_list[i].pos1, None);

          unique_id_list.push_back(oid);
          oid++;
          gp_list.push_back(*gp);
          delete gp;
          jun_loc_list.push_back(loc_list[i].loc1);
          rid_list.push_back(loc_list[i].rid);
        }else{
          GPoint last_gp = gp_list[gp_list.size() - 1];
          Point last_jun = jun_loc_list[jun_loc_list.size() - 1];


          GPoint* gp = new GPoint(true, n->GetId(), loc_list[i].rid,
                                  loc_list[i].pos1, None);
          if(gp->GetRouteId() == last_gp.GetRouteId() && 
             fabs(gp->GetPosition() - last_gp.GetPosition()) < delta_dist &&
             last_jun.Distance(loc_list[i].loc1) < delta_dist){
            delete gp;
            continue;
          }

          unique_id_list.push_back(oid);
          oid++;
          gp_list.push_back(*gp);
          delete gp;
          jun_loc_list.push_back(loc_list[i].loc1);
          rid_list.push_back(loc_list[i].rid);

        }

    }

}

/*
create one connection for road graph, two junction points having the same
spatial location 

*/
void RoadDenstiy::GetRGEdges1(Relation* rel, R_Tree<2,TupleId>* rtree)
{

    for(int i = 1;i <= rel->GetNoTuples();i++){
      Tuple* jun_tuple = rel->GetTuple(i, false);
      int id = ((CcInt*)jun_tuple->GetAttribute(RG_N_JUN_ID))->GetIntval();
      Point* loc = (Point*)jun_tuple->GetAttribute(RG_N_P);

      vector<int> neighbor_list;

      DFTraverse(rel, rtree, rtree->RootRecordId(), *loc, neighbor_list);
      for(unsigned int i = 0;i < neighbor_list.size();i++){
        if(neighbor_list[i] == id)continue;
        jun_id_list1.push_back(id);
        jun_id_list2.push_back(neighbor_list[i]);
      }
      jun_tuple->DeleteIfAllowed();
    }

}

/*
traverse rtree to find the points that have the same spatial location as input

*/
void RoadDenstiy::DFTraverse(Relation* rel,R_Tree<2,TupleId>* rtree, 
                             SmiRecordId adr, 
                          Point& loc, vector<int>& oid_list)
{
  const double delta_dist = 0.001;
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = rel->GetTuple(e.info, false);
              Point* q = (Point*)dg_tuple->GetAttribute(RG_N_P);

              if(q->Distance(loc) < delta_dist){
                  int id = 
                    ((CcInt*)dg_tuple->GetAttribute(RG_N_JUN_ID))->GetIntval();
                  oid_list.push_back(id);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc.Inside(e.box)){
                DFTraverse(rel, rtree, e.pointer, loc, oid_list);
            }
      }
  }
  delete node;

}

/*
two junction points are connected by glines 
converting gline to line takes a lot of time 
because the junction point (gpoint ) is get from curve -- dual --TRUE
use simpline method 2 seconds for berlin roads

use gline2line method 27 seconds 

*/
void RoadDenstiy::GetRGEdges2(Relation* rel)
{
  vector<GP_Point> gp_p_list;
  int NetId;
  for(int i = 1;i <= rel->GetNoTuples();i++){
    Tuple* jun_tuple = rel->GetTuple(i, false);
    int oid = ((CcInt*)jun_tuple->GetAttribute(RG_N_JUN_ID))->GetIntval();
    GPoint* gp = (GPoint*)jun_tuple->GetAttribute(RG_N_GP);
    Point* loc = (Point*)jun_tuple->GetAttribute(RG_N_P);
    NetId = gp->GetNetworkId();
    GP_Point gp_p(gp->GetRouteId(), gp->GetPosition(), oid, *loc, *loc);
    gp_p_list.push_back(gp_p);
    jun_tuple->DeleteIfAllowed();
  }
  sort(gp_p_list.begin(), gp_p_list.end(), CompareGP_P);

  
  
  
  for(unsigned int i = 0;i < gp_p_list.size();i++){

//      gp_p_list[i].Print();
    vector<GP_Point> sub_list;
    sub_list.push_back(gp_p_list[i]);
    int rid = gp_p_list[i].rid;
    unsigned int j = i + 1;
    while(j < gp_p_list.size() && 
          gp_p_list[j].rid == sub_list[sub_list.size() - 1].rid){
      sub_list.push_back(gp_p_list[j]);
      j++;
    }

//    cout<<"rid "<<rid<<" "<<sub_list.size()<<endl;
    i = j - 1;
    if(sub_list.size() > 1){
//      cout<<"rid "<<rid<<endl;

    Tuple* road_tuple = n->GetRoute(rid);
    SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);

     for(unsigned int k = 0;k < sub_list.size();k++){
       if(k == 0){
          jun_id_list1.push_back((int)sub_list[k].pos2);
          jun_id_list2.push_back((int)sub_list[k + 1].pos2);
          double pos1 = sub_list[k].pos1;
          double pos2 = sub_list[k + 1].pos1;

          GLine* gl = new GLine(0);
          gl->SetNetworkId(NetId);
          gl->AddRouteInterval(rid, pos1, pos2);
          gl->SetDefined(true);
          gl->SetSorted(false);
          gl->TrimToSize();
    
          gl_path_list.push_back(*gl);

          SimpleLine* sub_l = new SimpleLine(0);
          if(pos1 < pos2)
            sl->SubLine(pos1, pos2, true, *sub_l);
          else
            sl->SubLine(pos2, pos1, true, *sub_l);
          
          sline_path_list.push_back(*sub_l);
          delete sub_l;

    
          delete gl;

       }else if(k == sub_list.size() - 1){
         jun_id_list1.push_back((int)sub_list[k].pos2);
         jun_id_list2.push_back((int)sub_list[k - 1].pos2);

         double pos1 = sub_list[k].pos1;
         double pos2 = sub_list[k - 1].pos1;

         GLine* gl = new GLine(0);
         gl->SetNetworkId(NetId);
         gl->AddRouteInterval(rid, pos1, pos2);

         gl->SetDefined(true);
         gl->SetSorted(false);
         gl->TrimToSize();

         gl_path_list.push_back(*gl);

          SimpleLine* sub_l = new SimpleLine(0);
          if(pos2 < pos1)
            sl->SubLine(pos2, pos1, true, *sub_l);
          else
            sl->SubLine(pos1, pos2, true, *sub_l);
          
          sline_path_list.push_back(*sub_l);
          delete sub_l;

         delete gl;

       }else{
        jun_id_list1.push_back((int)sub_list[k].pos2);
        jun_id_list2.push_back((int)sub_list[k - 1].pos2);

        double pos1_1 = sub_list[k].pos1;
        double pos1_2 = sub_list[k - 1].pos1;

        jun_id_list1.push_back((int)sub_list[k].pos2);
        jun_id_list2.push_back((int)sub_list[k + 1].pos2);

        double pos2_1 = sub_list[k].pos1;
        double pos2_2 = sub_list[k + 1].pos1;


        GLine* gl1 = new GLine(0);
        gl1->SetNetworkId(NetId);
        gl1->AddRouteInterval(rid, pos1_1, pos1_2);
        gl1->SetDefined(true);
        gl1->SetSorted(false);
        gl1->TrimToSize();
        gl_path_list.push_back(*gl1);


        SimpleLine* sub_l1 = new SimpleLine(0);
        if(pos1_2 < pos1_1)
          sl->SubLine(pos1_2, pos1_1, true, *sub_l1);
        else
          sl->SubLine(pos1_1, pos1_2, true, *sub_l1);

        sline_path_list.push_back(*sub_l1);
        delete sub_l1;
          

        delete gl1;


        GLine* gl2 = new GLine(0);
        gl2->SetNetworkId(NetId);
        gl2->AddRouteInterval(rid, pos2_1, pos2_2);
        gl2->SetDefined(true);
        gl2->SetSorted(false);
        gl2->TrimToSize();

        gl_path_list.push_back(*gl2);

        SimpleLine* sub_l2 = new SimpleLine(0);
        if(pos2_1 < pos2_2)
          sl->SubLine(pos2_1, pos2_2, true, *sub_l2);
        else
          sl->SubLine(pos2_2, pos2_1, true, *sub_l2);

        sline_path_list.push_back(*sub_l2);
        delete sub_l2;

        delete gl2;
       }
 
      }
    
      road_tuple->DeleteIfAllowed();
    }

  }

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

  ListExpr list1 = nl->TwoElemList(nl->StringAtom("Route Id:"), 
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
    <<"direction "<<bs.GetUp();
  }else
    o<<"undef"<<endl;

  return o;
}

/*
for each bus stop, we assign a unique number. 
  the number is constrcuted by: brid + stopid + up 
  a number has three parts (string to int)
  not correct 1 11 1----11 1 1

*/
int Bus_Stop::GetUOid()
{
  if(!IsDefined()) return 0;

  char buf1[256], buf2[256], buf3[2];

  sprintf(buf1, "%d", br_id * 10);
  sprintf(buf2, "%d", stop_id);
  if(up_down)
    sprintf(buf3, "%d", 1);
  else
    sprintf(buf3, "%d", 0);

   strcat (buf1, buf2);
   strcat (buf1, buf3);
   
  int uoid = atoi(buf1);
//  cout<<" uoid "<<uoid<<endl;
  assert(uoid > 0); 
  return uoid; 

}

bool Bus_Stop::operator==(const Bus_Stop& bs)
{
  if(!IsDefined() || !bs.IsDefined()) return false; 
  if(br_id == bs.GetId() && stop_id == bs.GetStopId() && up_down == bs.GetUp())
    return true;
  return false; 
}

ostream& Bus_Stop::Print(ostream& os) const
{
  return os<<*this; 
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


const Rectangle<2> Bus_Route::BoundingBox(const Geoid* geoid) const
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
add a new element to the result, only a segment 

*/

void Bus_Route::Add2(HalfSegment hs, int count)
{
  BR_Elem br_elem;
  br_elem.br_seg_id = count;
  br_elem.start_pos = seg_list.Size(); 
  int no = 0;

  seg_list.Append(hs); 
  no++;

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

/*
get the point of a metro stop
it is different from the bus route. in metro network, the up and down direction
have the same route (geometry). the segment connecting two endpoints 

*/
void Bus_Route::GetMetroStopGeoData(Bus_Stop* ms, Point* p)
{
  if(GetId() != ms->GetId()){
    cout<<"route id is different for the bus stop and route"<<endl; 
    p->SetDefined(false);
    return; 
  }
  if(ms->GetStopId() < 1 || (int) ms->GetStopId() > Size() + 2 ){
    cout<<"invalid bus stop id"<<endl; 
    p->SetDefined(false);
    return;
  }

  if(ms->GetUp() != GetUp()){
    cout<<"bus stop and bus route have different directions"<<endl;
    p->SetDefined(false);
    return;
  }

   HalfSegment hs1, hs2;
   if(ms->GetStopId() == 1){

      if(ms->GetUp()){
        GetSeg(0, hs1);
        GetSeg(1, hs2);
      }else{
        GetSeg(SegSize() - 1, hs1);
        GetSeg(SegSize() - 2, hs2);
      }

      Point lp1 = hs1.GetLeftPoint();
      Point rp1 = hs1.GetRightPoint();
      Point lp2 = hs2.GetLeftPoint();
      Point rp2 = hs2.GetRightPoint();

      if(AlmostEqual(lp1, lp2) || AlmostEqual(lp1, rp2)){
        *p = rp1;
      }else if(AlmostEqual(rp1, lp2) || AlmostEqual(rp1, rp2)){
        *p = lp1;
      }else assert(false);

   }else if ((int)ms->GetStopId() == SegSize() + 1){

      if(ms->GetUp()){
        GetSeg(SegSize() - 1, hs1);
        GetSeg(SegSize() - 2, hs2);
      }else{
        GetSeg(0, hs1);
        GetSeg(1, hs2);
      }

      Point lp1 = hs1.GetLeftPoint();
      Point rp1 = hs1.GetRightPoint();
      Point lp2 = hs2.GetLeftPoint();
      Point rp2 = hs2.GetRightPoint();

      if(AlmostEqual(lp1, lp2) || AlmostEqual(lp1, rp2)){
        *p = rp1;
      }else if(AlmostEqual(rp1, lp2) || AlmostEqual(rp1, rp2)){
        *p = lp1;
      }else assert(false);

   }
   else{

     if(ms->GetUp()){
        int index1 = ms->GetStopId() - 2;
        int index2 = ms->GetStopId() - 1;
        GetSeg(index1, hs1);
        GetSeg(index2, hs2);
      }else{
        int index1 = ms->GetStopId();
        int index2 = ms->GetStopId() - 1;
        GetSeg(SegSize() - index1, hs1);
        GetSeg(SegSize() - index2, hs2);
      }

      Point lp1 = hs1.GetLeftPoint();
      Point rp1 = hs1.GetRightPoint();
      Point lp2 = hs2.GetLeftPoint();
      Point rp2 = hs2.GetRightPoint();

      if(AlmostEqual(lp1, lp2) || AlmostEqual(lp1, rp2)){
        *p = lp1;
      }else if(AlmostEqual(rp1, lp2) || AlmostEqual(rp1, rp2)){
        *p = rp1;
      }else assert(false); 

   }

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
"(rel (tuple ((bus_stop busstop) (stop_geodata point))))";
string BusNetwork::BusRoutesTypeInfo =
"(rel (tuple ((br_id int)(bus_route busroute)(oid int))))";
string BusNetwork::BusStopsInternalTypeInfo =
"(rel (tuple ((br_id int)(bus_stop busstop)(u_oid int)(geodata point))))";
string BusNetwork::BusStopsBTreeTypeInfo =
"(btree (tuple ((br_id int)(bus_stop busstop)(u_oid int)(geodata point))) int)";
string BusNetwork::BusStopsRTreeTypeInfo =  "(rtree (tuple ((br_id int)\
(bus_stop busstop)(u_oid int)(geodata point))) point FALSE)";

string BusNetwork::BusRoutesBTreeTypeInfo =
"(btree (tuple ((br_id int)(bus_route busroute)(oid int))) int)";

string BusNetwork::BusRoutesBTreeUOidTypeInfo =
"(btree (tuple ((br_id int)(bus_route busroute)(oid int))) int)";

string BusNetwork::BusTripsTypeInfo = 
"(rel (tuple ((bustrip1 genmo) (bustrip2 mpoint) (br_id int)(oid int))))";

string BusNetwork::BusTripBTreeTypeInfo =
"(btree (tuple ((bustrip1 genmo) (bustrip2 mpoint) (br_id int)(oid int))) int)";


ListExpr BusNetworkProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("busnetwork"),
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
  ////////////////////////////////////////////////////////////////////////
  ////////////////no output bus trips relation: too much data///////////////
  //////////////////////////////////////////////////////////////////////////
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
def(false), bn_id(0), graph_init(false), graph_id(0), max_bus_speed(0),
min_br_oid(0), min_bt_oid(0), 
stops_rel(NULL), btree_bs(NULL), 
routes_rel(NULL), btree_br(NULL), btree_bs_uoid(NULL), rtree_bs(NULL),
btree_br_uoid(NULL), bustrips_rel(NULL), btree_trip_oid(NULL),
btree_trip_br_id(NULL)
{

  
}

BusNetwork::BusNetwork(bool d, unsigned int i): def(d), bn_id(i), 
graph_init(false), graph_id(0), max_bus_speed(0),
min_br_oid(0), min_bt_oid(0), 
stops_rel(NULL), btree_bs(NULL), routes_rel(NULL), btree_br(NULL),
btree_bs_uoid(NULL), rtree_bs(NULL), btree_br_uoid(NULL), bustrips_rel(NULL),
btree_trip_oid(NULL), btree_trip_br_id(NULL)
{

}

/*
read the data from record 

*/
BusNetwork::BusNetwork(SmiRecord& valueRecord, size_t& offset, 
                       const ListExpr typeInfo):
def(false), bn_id(0), graph_init(false), graph_id(0), max_bus_speed(0),
min_br_oid(0), min_bt_oid(0), 
stops_rel(NULL), btree_bs(NULL), 
routes_rel(NULL), btree_br(NULL), btree_bs_uoid(NULL), rtree_bs(NULL),
btree_br_uoid(NULL), bustrips_rel(NULL), btree_trip_oid(NULL),
btree_trip_br_id(NULL)
{
  valueRecord.Read(&def, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&bn_id, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  valueRecord.Read(&graph_init, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&graph_id, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  valueRecord.Read(&max_bus_speed, sizeof(double), offset);
  offset += sizeof(double);

  valueRecord.Read(&min_br_oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  valueRecord.Read(&min_bt_oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for busstops*********************/
  nl->ReadFromString(BusStopsInternalTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  stops_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!stops_rel) {
   return;
  }
  ///////////////////btree on bus stops on brid///////////////////////////////
  nl->ReadFromString(BusStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bs = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_bs) {
    stops_rel->Delete(); 
   return;
  }

  /***********************Open relation for busroutes*********************/
  nl->ReadFromString(BusRoutesTypeInfo,xType);
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

  ///////////////////btree on bus stops on uoid//////////////////////////////
  nl->ReadFromString(BusStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bs_uoid = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_bs_uoid) {
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    delete btree_br; 
    return;
  }

  ///////////////////rtree on bus stops //////////////////////////////
  Word xValue;
  if(!(rtree_bs->Open(valueRecord,offset, BusStopsRTreeTypeInfo,xValue))){
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    delete btree_br; 
    delete btree_bs_uoid;
    return;
  }

  rtree_bs = ( R_Tree<2,TupleId>* ) xValue.addr;


   ///////////////////btree on bus routes unique id////////////////////////////
  nl->ReadFromString(BusRoutesBTreeUOidTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_br_uoid = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_br_uoid) {
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    delete btree_br; 
    delete btree_bs_uoid;
    delete rtree_bs; 
    return;
  }

  ///////////////open relation storing bus trips//////////////////////
  nl->ReadFromString(BusTripsTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bustrips_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!bustrips_rel) {
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    delete btree_br; 
    delete btree_bs_uoid;
    delete rtree_bs; 
    delete btree_br_uoid;
    return;
  }
  
  ///////////////////btree on bus trips unique id////////////////////////////
  nl->ReadFromString(BusTripBTreeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_trip_oid = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_trip_oid) {
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    delete btree_br; 
    delete btree_bs_uoid;
    delete rtree_bs; 
    delete btree_br_uoid;
    bustrips_rel->Delete();
    return;
  }
  ///////////////////btree on bus trips bus route id/////////////////////////
  nl->ReadFromString(BusTripBTreeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_trip_br_id = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_trip_br_id) {
    stops_rel->Delete(); 
    delete btree_bs;
    routes_rel->Delete();
    delete btree_br; 
    delete btree_bs_uoid;
    delete rtree_bs; 
    delete btree_br_uoid;
    bustrips_rel->Delete();
    delete btree_trip_oid;
    return;
  }

}

BusNetwork::~BusNetwork()
{
  if(stops_rel != NULL) stops_rel->Close();
  if(btree_bs != NULL) delete btree_bs; 
  if(routes_rel != NULL) routes_rel->Close();
  if(btree_br != NULL) delete btree_br;
  if(btree_bs_uoid != NULL) delete btree_bs_uoid; 
  if(rtree_bs != NULL) delete rtree_bs; 
  if(btree_br_uoid != NULL) delete btree_br_uoid;
  if(bustrips_rel != NULL) bustrips_rel->Close(); 
  if(btree_trip_oid != NULL) delete btree_trip_oid;
  if(btree_trip_br_id != NULL) delete btree_trip_br_id;

}

bool BusNetwork::Save(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo)
{
  
//  cout<<"BusNetwork::Save"<<endl; 

  valueRecord.Write(&def, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&bn_id, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  valueRecord.Write(&graph_init, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&graph_id, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  valueRecord.Write(&max_bus_speed, sizeof(double), offset);
  offset += sizeof(double); 

  valueRecord.Write(&min_br_oid, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  valueRecord.Write(&min_bt_oid, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  ListExpr xType;
  ListExpr xNumericType;

  ////////////////////bus stops relation/////////////////////////////
  nl->ReadFromString(BusStopsInternalTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!stops_rel->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////btree on bus stops on brid/////////////////////////
  nl->ReadFromString(BusStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bs->Save(valueRecord,offset,xNumericType))
      return false;

   ///////////////////bus routes relation/////////////////////////////
  nl->ReadFromString(BusRoutesTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!routes_rel->Save(valueRecord,offset,xNumericType))
      return false;
  
  ///////////////////////btree on bus routes////////////////////////////
  nl->ReadFromString(BusRoutesBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_br->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////btree on bus stops on uoid///////////////////////
  nl->ReadFromString(BusStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bs_uoid->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////rtree on bus stops ///////////////////////
  if(!rtree_bs->Save(valueRecord, offset)){
    return false;
  }
  
  ///////////////////////btree on bus routes on unique id//////////////////////
  nl->ReadFromString(BusRoutesBTreeUOidTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_br_uoid->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////bus trips relation/////////////////////////////
  nl->ReadFromString(BusTripsTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!bustrips_rel->Save(valueRecord,offset,xNumericType))
      return false;
  
  ///////////////////////btree on bus trips on unique id//////////////////////
  nl->ReadFromString(BusTripBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_trip_oid->Save(valueRecord,offset,xNumericType))
      return false;

  //////////////////btree on bus trips on bus route  id//////////////////////
  nl->ReadFromString(BusTripBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_trip_br_id->Save(valueRecord,offset,xNumericType))
      return false;
  
  return true; 
}

/*
store bus stops relation and the index

*/
void BusNetwork::LoadStops(Relation* r1)
{
  ListExpr xTypeInfo;
  nl->ReadFromString(BusStopsInternalTypeInfo, xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation* s_rel = new Relation(xNumType, true);
  for(int i = 1;i <= r1->GetNoTuples();i++){
    Tuple* bs_tuple = r1->GetTuple(i, false);
    if(bs_tuple->GetNoAttributes() != 2){
      cout<<"bus stops relation schema is wrong"<<endl;
      bs_tuple->DeleteIfAllowed();
      break; 
    }
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(NODE_BS1);
    int id = bs->GetId(); 

    Tuple* new_bs_tuple = new Tuple(nl->Second(xNumType)); 
    new_bs_tuple->PutAttribute(BN_ID1, new CcInt(true, id));
    new_bs_tuple->PutAttribute(BN_BS, new Bus_Stop(*bs));
    new_bs_tuple->PutAttribute(BS_U_OID, new CcInt(true, bs->GetUOid()));

    Point* bs_loc = (Point*)bs_tuple->GetAttribute(NODE_BS2);
//    cout<<*bs_loc<<endl;
    new_bs_tuple->PutAttribute(BS_GEO, new Point(*bs_loc));

    s_rel->AppendTuple(new_bs_tuple);
    new_bs_tuple->DeleteIfAllowed();
    bs_tuple->DeleteIfAllowed();
  }
//  cout<<s_rel->GetNoTuples()<<endl; 


  ListExpr ptrList1 = listutils::getPtrList(s_rel);

  string strQuery = "(consume(feed(" + BusStopsInternalTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";
 
//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  stops_rel = (Relation*)xResult.addr; 
  s_rel->Delete(); 
  
  //////////////////////////////////////////////////////////////////
  //////////////////////btree on bus stops br id/////////////////////
  ///////////////////////////////////////////////////////////////////
  ListExpr ptrList2 = listutils::getPtrList(stops_rel);
  
  strQuery = "(createbtree (" + BusStopsInternalTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "br_id)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bs = (BTree*)xResult.addr;


  //////////////////////btree on bus stops uoid/////////////////////
  ListExpr ptrList3 = listutils::getPtrList(stops_rel);

  strQuery = "(createbtree (" + BusStopsInternalTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "u_oid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bs_uoid = (BTree*)xResult.addr;

  //////////////////////rtree on bus stops//////////////////////////
  ListExpr ptrList4 = listutils::getPtrList(stops_rel);

  strQuery = "(bulkloadrtree(sortby(addid(feed (" + BusStopsInternalTypeInfo +
         " (ptr " + nl->ToString(ptrList4) + "))))((geodata asc))) geodata)";
  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  rtree_bs = ( R_Tree<2,TupleId>* ) xResult.addr;

}

/*
store bus routes relation as well as some indices 

*/
void BusNetwork:: LoadRoutes(Relation* r2)
{

  ListExpr ptrList1 = listutils::getPtrList(r2);

  string strQuery = "(consume(feed(" + BusRoutesTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  routes_rel = (Relation*)xResult.addr; 
  
  ///////////////////get the minimum bus route oid//////////////////////////
  int temp_id = numeric_limits<int>::max(); 
  for(int i = 1;i <= routes_rel->GetNoTuples();i++){
    Tuple* br_tuple = routes_rel->GetTuple(i, false);
    int id = ((CcInt*)br_tuple->GetAttribute(BN_BR_OID))->GetIntval();
    if(id < temp_id) temp_id = id;
    br_tuple->DeleteIfAllowed(); 
  }
  min_br_oid = temp_id; 
//  cout<<"min br oid "<<min_br_oid<<endl; 
  
   //////////////////////////////////////////////////////////////////
  //////////////////////btree on bus routes brid/////////////////////
  //////////////////////////////////////////////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(routes_rel);

  strQuery = "(createbtree (" + BusRoutesTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "br_id)";
//  cout<<strQuery<<endl; 
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_br = (BTree*)xResult.addr;


   //////////////////////////////////////////////////////////////////
  //////////////////////btree on bus routes unique oid///////////////
  //////////////////////////////////////////////////////////////////

  ListExpr ptrList3 = listutils::getPtrList(routes_rel);

  strQuery = "(createbtree (" + BusRoutesTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "oid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_br_uoid = (BTree*)xResult.addr;

}

/*
store moving buses and the maximum speed of all buses 
(for query processing of bus graph) 

*/
void BusNetwork:: LoadBuses(Relation* r3)
{

  ListExpr ptrList1 = listutils::getPtrList(r3);

  string strQuery = "(consume(feed(" + BusTripsTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  bustrips_rel = (Relation*)xResult.addr; 

//  cout<<"size "<<bustrips_rel->GetNoTuples()<<endl; 

 //////////////////////////////////////////////////////////////////////
 //////////////////////set maximum bus speed///////////////////////////
 /////////////////////////////////////////////////////////////////////
 int temp_id = numeric_limits<int>::max();
 for(int i = 1;i <= bustrips_rel->GetNoTuples();i++){
  Tuple* bus_tuple = bustrips_rel->GetTuple(i, false);
  GenMO* mo = (GenMO*)bus_tuple->GetAttribute(BN_BUSTRIP);

  for( int j = 0; j < mo->GetNoComponents(); j++ ){
      UGenLoc unit;
      mo->Get( j, unit );
      double pos1 = unit.gloc1.GetLoc().loc1;
      double pos2 = unit.gloc2.GetLoc().loc1;
      if(fabs(pos1 - pos2) > 1.0){
          double t = 
              unit.timeInterval.end.ToDouble()*86400.0 -
              unit.timeInterval.start.ToDouble()*86400.0;
          double speed = fabs(pos1 - pos2)/t; 
/*          cout<<"dist "<<fabs(pos1 - pos2)<<" t "<<t
               <<" speed "<<speed<<endl;*/
          if(speed > max_bus_speed){
            max_bus_speed = speed; 
          }
      }
  }
  int id = ((CcInt*)bus_tuple->GetAttribute(BN_BUS_OID))->GetIntval();
  if(id < temp_id) temp_id = id; 
  
  bus_tuple->DeleteIfAllowed();

 }
 min_bt_oid = temp_id; 
// cout<<"max bus speed "<<max_bus_speed*60*60/1000.0<<"km/h "<<endl;
// cout<<"min bus trip oid "<<min_bt_oid<<endl; 

  ////////////////btree on bus trips oid///////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(bustrips_rel);

  strQuery = "(createbtree (" + BusTripsTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "oid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_trip_oid = (BTree*)xResult.addr;


  ////////////////btree on bus trips bus route///////////////////////////

  ListExpr ptrList3 = listutils::getPtrList(bustrips_rel);

  strQuery = "(createbtree (" + BusTripsTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "br_id)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_trip_br_id = (BTree*)xResult.addr;

}

/*
create a bus network by bus stops relation and bus routes relation 

*/
void BusNetwork::Load(unsigned int i, Relation* r1, Relation* r2, Relation* r3)
{
  if(i < 1){
    def = false;
    return;
  }
  bn_id = i; 

  LoadStops(r1);  /////to get 2D points in space  
  LoadRoutes(r2); //first load bus routes because bus stops access bus routes
  LoadBuses(r3); //first load bus routes because bus stops access bus routes

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

/*
set the bus graph id for bus network 

*/
void BusNetwork::SetGraphId(int g_id)
{
  graph_id = g_id; 
  graph_init = true; 
}

/*
get the bus graph in bus network 

*/
BusGraph* BusNetwork::GetBusGraph()
{
  if(graph_init == false) return NULL;
  
  ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
  xObjectList = nl->Rest(xObjectList);
  while(!nl->IsEmpty(xObjectList))
  {
    // Next element in list
    ListExpr xCurrent = nl->First(xObjectList);
    xObjectList = nl->Rest(xObjectList);

    // Type of object is at fourth position in list
    ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
    if(nl->IsAtom(xObjectType) &&
       nl->SymbolValue(xObjectType) == "busgraph"){
      // Get name of the bus graph 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. 
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined 
        continue;
      }
      BusGraph* bg = (BusGraph*)xValue.addr;
      if(bg->bg_id == graph_id){
        // This is the bus graph we have been looking for
        return bg;
      }
    }
  }
  return NULL;
}

/*
close the bus graph 

*/
void BusNetwork::CloseBusGraph(BusGraph* bg)
{
  if(bg == NULL) return; 
  Word xValue;
  xValue.addr = bg;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "busgraph" ),
                                           xValue);
}

/*
given a bus stop and time, it returns oid of the moving bus belonging to 
that bus route and direction as well as coving the time

*/
struct Id_Time{
  int oid;
  double time;
  Id_Time(){}
  Id_Time(int id, double t):oid(id), time(t){}
  Id_Time(const Id_Time& id_t):oid(id_t.oid), time(id_t.time){}
  Id_Time& operator=(const Id_Time& id_time)
  {
    oid = id_time.oid;
    time = id_time.time;
    return *this;
  }
  bool operator<(const Id_Time& id_time) const
  {
    return time < id_time.time;
  }
  void Print()
  {
    cout<<"oid "<<oid<<"cost "<<time<<endl;
  }
};

int BusNetwork::GetMOBus_Oid(Bus_Stop* bs, Point* bs_loc, Instant& t)
{
//  cout<<"instant "<<t<<endl; 
  int br_id = bs->GetId();
  //////////////////////////////////////////////////////////////////
  ////////////get the unique id for the bus route/////////////////
  //////////////////////////////////////////////////////////////////
  CcInt* search_id1 = new CcInt(true, br_id);
  BTreeIterator* btree_iter1 = btree_br->ExactMatch(search_id1);
  int br_uoid = 0;
  while(btree_iter1->Next()){
      Tuple* tuple = routes_rel->GetTuple(btree_iter1->GetId(), false);
      Bus_Route* br = (Bus_Route*)tuple->GetAttribute(BusNetwork::BN_BR);
      if(br->GetUp() == bs->GetUp()){
        br_uoid = 
          ((CcInt*)tuple->GetAttribute(BusNetwork::BN_BR_OID))->GetIntval();
        tuple->DeleteIfAllowed();
        break;
      }
      tuple->DeleteIfAllowed();
  }
  delete btree_iter1;
  delete search_id1;
  assert(br_uoid > 0);

//  cout<<"br_uoid "<<br_uoid<<endl;
  ///////////////////////////////////////////////////////////////
  ///////////////get moving buses moving on the route///////////
  ///////////////////////////////////////////////////////////////
  int bus_oid = 0;
  CcInt* search_id2 = new CcInt(true, br_uoid);
  BTreeIterator* btree_iter2 = btree_trip_br_id->ExactMatch(search_id2);
  const double delta_dist = 0.01;
  vector<Id_Time> res_list;

  while(btree_iter2->Next()){
      Tuple* tuple = bustrips_rel->GetTuple(btree_iter2->GetId(), false);
      int brid = 
         ((CcInt*)tuple->GetAttribute(BN_REFBR_OID))->GetIntval();
      assert(brid == br_uoid);
      MPoint* mo_bus = (MPoint*)tuple->GetAttribute(BN_BUSTRIP_MP);
      Periods* peri = new Periods(0);
      mo_bus->DefTime(*peri);
//      cout<<"periods "<<*peri<<endl; 
      if(peri->Contains(t)){
//        cout<<"periods containt instant "<<endl;

        for(int i = 0;i < mo_bus->GetNoComponents();i++){
          UPoint unit;
          mo_bus->Get(i, unit);
          Point p0 = unit.p0;
          Point p1 = unit.p1;

//          cout<<unit.timeInterval<<" dist "<<bs_loc->Distance(p0)<<endl;
//          cout<<"dist1 "<<bs_loc->Distance(p0)
//              <<" dist2 "<<bs_loc->Distance(p1)<<endl;

          if(bs_loc->Distance(p0) < delta_dist &&
             bs_loc->Distance(p1) < delta_dist){
              bus_oid = ((CcInt*)tuple->GetAttribute(BN_BUS_OID))->GetIntval();
              double delta_t = 
                fabs(unit.timeInterval.start.ToDouble() - t.ToDouble());
              Id_Time* id_time = new Id_Time(bus_oid, delta_t);
              res_list.push_back(*id_time);
              delete id_time;
          }
        }
      }
      delete peri;
      tuple->DeleteIfAllowed();
  }
  delete btree_iter2;
  delete search_id2;
  sort(res_list.begin(), res_list.end());
//  for(unsigned int i = 0;i < res_list.size();i++)
//    res_list[i].Print();
//  cout<<"res_list size "<<res_list.size()<<endl;
  assert(res_list.size() > 0);
  bus_oid = res_list[0].oid;
  
  assert(bus_oid > 0);
  return bus_oid; 
}

/*
given a bus stop and time, it returns mpoint of the moving bus belonging to 
that bus route and direction as well as coving the time

*/
int BusNetwork::GetMOBus_MP(Bus_Stop* bs, Point* bs_loc, Instant t, MPoint& mp)
{
//  cout<<"GetMOBus_MP() bs "<<*bs<<endl;
  int br_id = bs->GetId();
  //////////////////////////////////////////////////////////////////
  ////////////get the unique id for the bus route/////////////////
  //////////////////////////////////////////////////////////////////
  CcInt* search_id1 = new CcInt(true, br_id);
  BTreeIterator* btree_iter1 = btree_br->ExactMatch(search_id1);
  int br_uoid = 0;
  while(btree_iter1->Next()){
      Tuple* tuple = routes_rel->GetTuple(btree_iter1->GetId(), false);
      Bus_Route* br = (Bus_Route*)tuple->GetAttribute(BusNetwork::BN_BR);
      if(br->GetUp() == bs->GetUp()){
        br_uoid = 
          ((CcInt*)tuple->GetAttribute(BusNetwork::BN_BR_OID))->GetIntval();
        tuple->DeleteIfAllowed();
        break;
      }
      tuple->DeleteIfAllowed();
  }
  delete btree_iter1;
  delete search_id1;
  assert(br_uoid > 0);

//  cout<<"br_uoid "<<br_uoid<<endl;
  ///////////////////////////////////////////////////////////////
  ///////////////get moving buses moving on the route///////////
  ///////////////////////////////////////////////////////////////
  int bus_oid = 0;
  CcInt* search_id2 = new CcInt(true, br_uoid);
  BTreeIterator* btree_iter2 = btree_trip_br_id->ExactMatch(search_id2);
  bool found = false;
  const double delta_dist = 0.01;
  vector<Id_Time> res_list;

  while(btree_iter2->Next() && found == false){
      Tuple* tuple = bustrips_rel->GetTuple(btree_iter2->GetId(), false);
      int brid = 
         ((CcInt*)tuple->GetAttribute(BN_REFBR_OID))->GetIntval();
      assert(brid == br_uoid);
      MPoint* mo_bus = (MPoint*)tuple->GetAttribute(BN_BUSTRIP_MP);
      Periods* peri = new Periods(0);
      mo_bus->DefTime(*peri);
      if(peri->Contains(t)){
//        cout<<"periods "<<*peri<<endl;
        for(int i = 0;i < mo_bus->GetNoComponents();i++){
          UPoint unit;
          mo_bus->Get(i, unit);
          Point p0 = unit.p0;
          Point p1 = unit.p1;
          if(bs_loc->Distance(p0) < delta_dist &&
              unit.timeInterval.Contains(t)){
              mp = *mo_bus;
              bus_oid = ((CcInt*)tuple->GetAttribute(BN_BUS_OID))->GetIntval();
              found = true;
              break;
          }
        }

      }
      delete peri;
      tuple->DeleteIfAllowed();
  }
  delete btree_iter2;
  delete search_id2;
  assert(bus_oid > 0);
  return bus_oid; 
}

/*
find the bus trip by input oid

*/
void BusNetwork::GetMOBUS(int trip_id, MPoint& mp, int& br_uid)
{

  CcInt* search_id = new CcInt(true, trip_id);
  BTreeIterator* btree_iter = btree_trip_oid->ExactMatch(search_id);
  while(btree_iter->Next()){
      Tuple* tuple = bustrips_rel->GetTuple(btree_iter->GetId(), false);
      MPoint* bus_trip = (MPoint*)tuple->GetAttribute(BN_BUSTRIP_MP);
      int id = ((CcInt*)tuple->GetAttribute(BN_BUS_OID))->GetIntval();
      assert(id == trip_id);
      br_uid = ((CcInt*)tuple->GetAttribute(BN_REFBR_OID))->GetIntval();
      mp = *bus_trip;
      tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;
  assert(br_uid > 0);
}

/*
from the input bus route unique id, find its geo data

*/
void BusNetwork::GetBusRouteGeoData(int br_uoid, SimpleLine& sl)
{
  CcInt* search_id = new CcInt(true, br_uoid);
  BTreeIterator* btree_iter = btree_br_uoid->ExactMatch(search_id);
  while(btree_iter->Next()){
      Tuple* tuple = routes_rel->GetTuple(btree_iter->GetId(), false);
      Bus_Route* br = (Bus_Route*)tuple->GetAttribute(BN_BR);
      br->GetGeoData(sl);
      tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id;
  
}

/////////////////////////////////////////////////////////////////////////////


string BN::BusStopsPaveTypeInfo =
"(rel (tuple ((bus_stop busstop) (pave_loc1 genloc)\
(pave_loc2 point)(bus_stop_loc point))))";

string BN::BusTimeTableTypeInfo = 
"(rel (tuple ((stop_loc point) (bus_stop busstop) (whole_time periods) \
(schedule_interval real) (loc_id int) (bus_uoid int))))";


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
  w = 2*w;
  ////////////////construct a line connecting the two bus stops//////
  Point p1;
  bn->GetBusStopGeoData(&bs1, &p1);

  Point p2;
  bn->GetBusStopGeoData(&bs2, &p2);

//  cout<<"bs1 "<<bs1<<" bs2 "<<bs2<<endl; 
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
//  cout<<p1<<" "<<p2<<endl;
//  cout<<it_p_list1.size()<<" "<<it_p_list2.size()<<endl;
  if(it_p_list1.size() == 0 || it_p_list2.size() == 0){
    cout<<"road width should be enlarged"<<endl;
    assert(false);

  }

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
//       if(q1.Inside(*reg1) == false){
//         cout<<"oid1 "<<"q1 "<<endl;
//         cout<<bs1<<" "<<bs2<<endl;
//       }

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
//       if(q2.Inside(*reg2) == false){
//           cout<<"oid2 "<<oid2<<" q2 "<<q2<<endl;
//           cout<<bs1<<" "<<bs2<<endl;
//       }

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
//       if(q1.Inside(*reg1) == false){
//         cout<<"oid1 "<<oid1<<" q1"<<q1<<endl;
//         cout<<bs1<<" "<<bs2<<endl;
//       }

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
//       if(q2.Inside(*reg2) == false){
//         cout<<"oid2 "<<oid2<<" q2 "<<q2<<endl;
//         cout<<bs1<<" "<<bs2<<endl;
//       }

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
/*                  Line* l1 = new Line(0);
                  candi_reg->Boundary(l1);
                  Points* ps = new Points(0);
                  l->Crossings(*l1, *ps); 
                  for(int i = 0;i < ps->Size();i++){
                    Point p;
                    ps->Get(i, p);
                    MyPoint_Tid mpt(p, 0.0, e.info);
                    it_p_list.push_back(mpt);
//                    assert(p.Inside(*candi_reg));
                  }
                  delete ps;
                  delete l1;*/

                 for(int i1 = 0;i1 < l->Size();i1++){
                    HalfSegment hs1;
                    l->Get(i1, hs1);
                    if(hs1.IsLeftDomPoint() == false) continue;
                    for(int i2 = 0;i2 < candi_reg->Size();i2++){
                      HalfSegment hs2;
                      candi_reg->Get(i2, hs2);
                      if(hs2.IsLeftDomPoint() == false) continue;
                      Point p;
                      if(hs1.Intersection(hs2, p)){
                         if(p.Inside(*candi_reg)){
                             MyPoint_Tid mpt(p, 0.0, e.info);
                             it_p_list.push_back(mpt);
                         }
//                        MyPoint_Tid mpt(p, 0.0, e.info);
//                        it_p_list.push_back(mpt);
                      }
                    }
                 }
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


/*
for each bus stop, we find its neighbor bus stops. the distance between them
  in the pavement area is smaller than a threshold value. e,g., 100m 

*/
void BN::BsNeighbors1(DualGraph* dg, VisualGraph* vg, Relation* rel1,
                   Relation* rel2, R_Tree<2,TupleId>* rtree)
{
//  const double neighbor_dist = 200.0;
  const double neighbor_dist = 100.0;

  SmiRecordId adr = rtree->RootRecordId();
  /////for each bus stop, find the neighbor candidates where the distance///
  /////between them is smaller than D in Euclidean space/////////////////
  ////////////this is done by traversing the RTree//////////////////////
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* bs_pave_tuple = rel2->GetTuple(i, false);
    Point* loc = (Point*)bs_pave_tuple->GetAttribute(BN_PAVE_LOC2);
    Bus_Stop* bs1 = (Bus_Stop*)bs_pave_tuple->GetAttribute(BN_BUSSTOP); 
    vector<int> neighbor_list; 
     ////////////Euclidean distance as a bound///////////////////////
    DFTraverse2(rtree,  rel2, adr , loc, neighbor_list, neighbor_dist); 
//    cout<<"bs "<<*bs<<" "<<neighbor_list.size()<<endl;

    int neighbor_no = 0;
    for(unsigned int j = 0;j < neighbor_list.size();j++){
      if(neighbor_list[j] != i){

        Line* path = new Line(0);
        if(FindNeighbor(i, neighbor_list[j], dg, vg, rel1, 
                       rel2, neighbor_dist, path)){
//          cout<<"a neighbor "<<endl; 
//            cout<<"tid1 "<<i<<" tid2 "<<neighbor_list[j]<<endl; 


           Tuple* tuple2 = rel2->GetTuple(neighbor_list[j], false);
           Bus_Stop* bs2 = (Bus_Stop*)tuple2->GetAttribute(BN_BUSSTOP); 

           if(bs1->GetId() != bs2->GetId()){ //different bus routes 
              bs_list1.push_back(*bs1);
              bs_list2.push_back(*bs2);
              SimpleLine* sl = new SimpleLine(0); 
              sl->fromLine(*path);
              path_sl_list.push_back(*sl); 


//              cout<<path->Length()<<endl; 
              //////////////////////////////////////////////////////////////
              /////connection between bus stops and their mapping points /// 
              ///////////////in the pavement///////////////////////////////
              //////this is a special connection, I defint it does not /////
              /////////belong to pavement area///////////////////////////////
              ///////////////////////////////////////////////////////////////
              Point* bus_loc1 = 
                      (Point*)bs_pave_tuple->GetAttribute(BN_BUSLOC); 
              Point* bus_loc2 = (Point*)tuple2->GetAttribute(BN_BUSLOC); 
              Point sl_sp;
              assert(sl->AtPosition(0.0, true, sl_sp)); 
              double d1 = sl_sp.Distance(*bus_loc1);
              double d2 = sl_sp.Distance(*bus_loc2);
//              cout<<"d1 "<<d1<<" d2 "<<d2<<endl; 


              if(d1 < d2){
                Point sl_ep;
                assert(sl->AtPosition(sl->Length(), true, sl_ep)); 
                HalfSegment hs1(true, *bus_loc1, sl_sp);
                HalfSegment hs2(true, *bus_loc2, sl_ep);


                SimpleLine* sub_sl1 = new SimpleLine(0);
                sub_sl1->StartBulkLoad();
                hs1.attr.edgeno = 0;
                *sub_sl1 += hs1;
                hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
                *sub_sl1 += hs1;
                sub_sl1->EndBulkLoad();
                sub_path1.push_back(*sub_sl1);

                SimpleLine* sub_sl2 = new SimpleLine(0);
                sub_sl2->StartBulkLoad();
                hs2.attr.edgeno = 0;
                *sub_sl2 += hs2;
                hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
                *sub_sl2 += hs2;
                sub_sl2->EndBulkLoad();
                sub_path2.push_back(*sub_sl2); 

                ///////////////////construct the whole path////////////////////
                SimpleLine* sl2 = new SimpleLine(0);  
                sl2->StartBulkLoad();
                int edgeno = 0;
                HalfSegment hs_1(true, *bus_loc1, sl_sp);
                HalfSegment hs_2(true, *bus_loc2, sl_ep);

                hs_1.attr.edgeno = edgeno++; 
                *sl2 += hs_1;
                hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
                *sl2 += hs_1; 
                hs_2.attr.edgeno = edgeno++;
                *sl2 += hs_2;
                hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
                *sl2 += hs_2; 
                for(int k = 0;k < sl->Size();k++){
                  HalfSegment hs;
                  sl->Get(k, hs);
                  if(!hs.IsLeftDomPoint()) continue; 
                  HalfSegment new_hs(true, hs.GetLeftPoint(), 
                                     hs.GetRightPoint());
                  new_hs.attr.edgeno = edgeno++; 
                  *sl2 += new_hs;
                  new_hs.SetLeftDomPoint(!new_hs.IsLeftDomPoint());
                  *sl2 += new_hs; 
                }
                sl2->EndBulkLoad(); 
                path2_sl_list.push_back(*sl2); 
                delete sl2; 
                //////////////////////////////////////////////////////////////

                delete sub_sl1; 
                delete sub_sl2; 

              }else{
                Point sl_ep;
                assert(sl->AtPosition(sl->Length(), true, sl_ep)); 
                HalfSegment hs1(true, *bus_loc2, sl_sp);
                HalfSegment hs2(true, *bus_loc1, sl_ep);

                SimpleLine* sub_sl1 = new SimpleLine(0);
                sub_sl1->StartBulkLoad();
                hs1.attr.edgeno = 0;
                *sub_sl1 += hs1;
                hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
                *sub_sl1 += hs1;
                sub_sl1->EndBulkLoad();
                sub_path1.push_back(*sub_sl1);


                SimpleLine* sub_sl2 = new SimpleLine(0);
                sub_sl2->StartBulkLoad();
                hs2.attr.edgeno = 0;
                *sub_sl2 += hs2;
                hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
                *sub_sl2 += hs2;
                sub_sl2->EndBulkLoad();
                sub_path2.push_back(*sub_sl2); 

                ///////////////////construct the whole path////////////////////
                SimpleLine* sl2 = new SimpleLine(0);  
                sl2->StartBulkLoad();
                int edgeno = 0;
                HalfSegment hs_1(true, *bus_loc2, sl_sp);
                HalfSegment hs_2(true, *bus_loc1, sl_ep);

                hs_1.attr.edgeno = edgeno++; 
                *sl2 += hs_1;
                hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
                *sl2 += hs_1; 
                hs_2.attr.edgeno = edgeno++;
                *sl2 += hs_2;
                hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
                *sl2 += hs_2; 
                for(int k = 0;k < sl->Size();k++){
                  HalfSegment hs;
                  sl->Get(k, hs);
                  if(!hs.IsLeftDomPoint()) continue; 
                  HalfSegment new_hs(true, hs.GetLeftPoint(), 
                                     hs.GetRightPoint());
                  new_hs.attr.edgeno = edgeno++; 
                  *sl2 += new_hs;
                  new_hs.SetLeftDomPoint(!new_hs.IsLeftDomPoint());
                  *sl2 += new_hs; 
                }
                sl2->EndBulkLoad(); 
                path2_sl_list.push_back(*sl2); 
                delete sl2; 
                //////////////////////////////////////////////////////////////

                delete sub_sl1; 
                delete sub_sl2; 

              }


              //////////////////////////////////////////////////////

              bs_uoid_list.push_back(bs1->GetUOid());
              delete sl; 
           }
           neighbor_no++;
           tuple2->DeleteIfAllowed(); 
        }

        delete path; 

      }
    }

    bs_pave_tuple->DeleteIfAllowed(); 

  }

}


/*
traverse R-tree to find neighbor points (bus stops), but does not include the
bus stops that have the same spatial location in space. because this kinds of
connection will be discovered later by another edge in bus network graph. 

*/
void BN::DFTraverse2(R_Tree<2,TupleId>* rtree, Relation* rel, 
                  SmiRecordId adr, Point* loc, vector<int>& neighbor_list,
                    double dist)
{
  const double delta_dist = 0.001; 
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple2 = rel->GetTuple(e.info,false);
              Point* bus_loc = (Point*)dg_tuple2->GetAttribute(BN_PAVE_LOC2);
              double d = loc->Distance(*bus_loc);
              if( d < dist && d > delta_dist){
                  neighbor_list.push_back(e.info);
              }
              dg_tuple2->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc->Distance(e.box) < dist){
              DFTraverse2(rtree, rel, e.pointer, loc, neighbor_list, dist);
            }
      }
  }
  delete node;


}

/*
calculate the obstructed distance between the two bus stops.
rel1: triangle relation 
rel2: bus stops and pavements location relation 
use the same procedure in PaveGraph.cpp WalkSP ShortestPath
(calculate the shortest path between two locations on the pavements)

*/
bool BN::FindNeighbor(int tid1, int tid2, DualGraph* dg, VisualGraph* vg,
                   Relation* rel1, Relation* rel2, 
                   double dist, Line* res)
{
//  cout<<"tid1 "<<tid1<<" tid2 "<<tid2<<endl; 

  Tuple* tuple1 = rel2->GetTuple(tid1, false);
  GenLoc* genloc1 = (GenLoc*)tuple1->GetAttribute(BN_PAVE_LOC1);
  Point* loc_1 = (Point*)tuple1->GetAttribute(BN_PAVE_LOC2); 

  Tuple* tuple2 = rel2->GetTuple(tid2, false);
  GenLoc* genloc2 = (GenLoc*)tuple2->GetAttribute(BN_PAVE_LOC1);
  Point* loc_2 = (Point*)tuple2->GetAttribute(BN_PAVE_LOC2);

  Point loc1 = *loc_1;
  Point loc2 = *loc_2;

  GenLoc gloc1 = *genloc1;
  GenLoc gloc2 = *genloc2;

  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();

//  cout<<gloc1<<" "<<gloc2<<endl; 
  ///////////////////////////////////////////////////////////////////////////
  /////////////////calculate the distance between the two bus stops//////////
  ///////////////////////////////////////////////////////////////////////////
  
  int no_node_graph = dg->No_Of_Node();
  int oid1 = gloc1.GetOid();
  int oid2 = gloc2.GetOid(); 
  oid1 -= dg->min_tri_oid_1;
  oid2 -= dg->min_tri_oid_1; 
  
  if(oid1 < 1 || oid1 > no_node_graph){
    cout<<"loc1 does not exist"<<endl;
    return false;
  }
  if(oid2 < 1 || oid2 > no_node_graph){
    cout<<"loc2 does not exist"<<endl;
    return false;
  }
  
  /////////this kind of connection will be found by another edge in the ///
  ///////////////bus network graph////////////////////////////////////////
  if(AlmostEqual(loc1,loc2)){
//    return true;
    return false; 
  }

  Tuple* tuple_1 = dg->GetNodeRel()->GetTuple(oid1, false);
  Region* reg1 = (Region*)tuple_1->GetAttribute(DualGraph::PAVEMENT);

  if(loc1.Inside(*reg1) == false){
    tuple_1->DeleteIfAllowed();
    cout<<"point1 is not inside the polygon"<<endl;
    return false;
  }
  Tuple* tuple_2 = dg->GetNodeRel()->GetTuple(oid2, false);
  Region* reg2 = (Region*)tuple_2->GetAttribute(DualGraph::PAVEMENT);

  if(loc2.Inside(*reg2) == false){
    tuple_1->DeleteIfAllowed();
    tuple_2->DeleteIfAllowed();
    cout<<"point2 is not inside the polygon"<<endl;
    return false;
  }
  tuple_1->DeleteIfAllowed();
  tuple_2->DeleteIfAllowed();
  ////////////////////////////////////////////////////////////////
  ////////////////find all visibility nodes to start node/////////
  ///////connect them to the visibility graph/////////////////////

  priority_queue<WPath_elem> path_queue;
  vector<WPath_elem> expand_path;

  VGraph* vg1 = new VGraph(dg, NULL, rel1, vg->GetNodeRel());

  vg1->GetVisibilityNode(oid1, loc1);

  assert(vg1->oids1.size() == vg1->p_list.size());

  if(vg1->oids1.size() == 1){//start point equasl to triangle vertex
    double w = loc1.Distance(loc2);
    path_queue.push(WPath_elem(-1, 0, vg1->oids1[0], w, loc1, 0.0));
    expand_path.push_back(WPath_elem(-1, 0, vg1->oids1[0], w, loc1, 0.0));

  }else{
    double w = loc1.Distance(loc2);
    path_queue.push(WPath_elem(-1, 0, -1, w,  loc1,0.0));//start location
    expand_path.push_back(WPath_elem(-1, 0, -1, w, loc1,0.0));//start location
    int prev_index = 0;

    for(unsigned int i = 0;i < vg1->oids1.size();i++){

      int expand_path_size = expand_path.size();
      double d = loc1.Distance(vg1->p_list[i]);
      w = d + vg1->p_list[i].Distance(loc2);
      path_queue.push(WPath_elem(prev_index, expand_path_size,
                      vg1->oids1[i], w, vg1->p_list[i], d));
      expand_path.push_back(WPath_elem(prev_index, expand_path_size,
                      vg1->oids1[i], w, vg1->p_list[i], d));
    }
  }

  delete vg1;
  ////////////////////////////////////////////////////////////////////////////
  ////////////////find all visibility nodes to the end node/////////
  VGraph* vg2 = new VGraph(dg, NULL, rel1, vg->GetNodeRel());

  vg2->GetVisibilityNode(oid2, loc2);

  assert(vg2->oids1.size() == vg2->p_list.size());
  //if the end node equals to triangle vertex.
  //it can be connected by adjacency list
  //we don't conenct it to the visibility graph
  
  Points* neighbor_end = new Points(0);
  neighbor_end->StartBulkLoad();
  if(vg2->oids1.size() > 1){
    for(unsigned int i = 0;i < vg2->oids1.size();i++){
        Tuple* loc_tuple = vg->GetNodeRel()->GetTuple(vg2->oids1[i], false);
        Point* loc = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
        *neighbor_end += *loc;
        loc_tuple->DeleteIfAllowed();
    }
    neighbor_end->EndBulkLoad();
  }

  /////////////////////searching path///////////////////////////////////
  bool find = false;

  vector<bool> mark_flag;
  for(int i = 1;i <= vg->GetNodeRel()->GetNoTuples();i++)
    mark_flag.push_back(false);
  
  WPath_elem dest;
  while(path_queue.empty() == false){
        WPath_elem top = path_queue.top();
        path_queue.pop();
//        top.Print();

        if(top.real_w > dist){ //already larger than the threshold distance 
              delete neighbor_end;
              delete vg2;
              return false; 
        }

        if(AlmostEqual(top.loc, loc2)){
          find = true;
          dest = top;
          break;
        }
        //do not consider the start point
        //if it does not equal to the triangle vertex
        //its adjacent nodes have been found already and put into the queue
        if(top.tri_index > 0 && mark_flag[top.tri_index - 1] == false){
          vector<int> adj_list;
          vg->FindAdj(top.tri_index, adj_list);
          int pos_expand_path = top.cur_index;
          for(unsigned int i = 0;i < adj_list.size();i++){
            if(mark_flag[adj_list[i] - 1]) continue;
            int expand_path_size = expand_path.size();

            Tuple* loc_tuple = vg->GetNodeRel()->GetTuple(adj_list[i], false);
            Point* loc = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);

            double w1 = top.real_w + top.loc.Distance(*loc);
            double w2 = w1 + loc->Distance(loc2);
            path_queue.push(WPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], w2 ,*loc, w1));
            expand_path.push_back(WPath_elem(pos_expand_path, expand_path_size,
                            adj_list[i], w2, *loc, w1));

            loc_tuple->DeleteIfAllowed();

            mark_flag[top.tri_index - 1] = true;
          }
        }

        ////////////check visibility points to the end point////////////
        if(neighbor_end->Size() > 0){
            const double delta_dist = 0.1;//in theory, it should be 0.0
            if(top.loc.Distance(neighbor_end->BoundingBox()) < delta_dist){
              for(unsigned int i = 0;i < vg2->oids1.size();i++){
                if(top.tri_index == vg2->oids1[i]){
                  int pos_expand_path = top.cur_index;
                  int expand_path_size = expand_path.size();

                  double w1 = top.real_w + top.loc.Distance(loc2);
                  double w2 = w1;
                  path_queue.push(WPath_elem(pos_expand_path, expand_path_size,
                                -1, w2 ,loc2, w1));
                  expand_path.push_back(WPath_elem(pos_expand_path,
                            expand_path_size,
                            -1, w2, loc2, w1));
                  break;
                }
              }
            }
        }
        ///////////////////////////////////////////////////////////////
  }

  delete neighbor_end;
  delete vg2;
  /////////////construct path///////////////////////////////////////////
  double len = 0.0;
  
  if(find){
    res->StartBulkLoad();
    while(dest.prev_index != -1){
      Point p1 = dest.loc;
      dest = expand_path[dest.prev_index];
      Point p2 = dest.loc;
      /////////////////////////////////////////////////////
      HalfSegment hs;
      hs.Set(true, p1, p2);
      hs.attr.edgeno = 0;
      *res += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *res += hs;
      /////////////////////////////////////////////////////
    }
    res->EndBulkLoad(); 
    len = res->Length(); 
  }
  if(len < dist) return true; 
  else return false; 
}

/*
for each bus stop, find the bus stops that have the same spatial location but
  belong to different bus routes. these are the places that people can do
  transfer. we also return the unique id of the first bus stop

*/
void BN::BsNeighbors2()
{

  R_Tree<2,TupleId>* rtree = bn->GetBS_RTree(); 
  
  SmiRecordId adr = rtree->RootRecordId();
  
  Relation* bs_rel = bn->GetBS_Rel();
  for(int i = 1;i <= bs_rel->GetNoTuples();i++){
    Tuple* bs_tuple = bs_rel->GetTuple(i, false); 
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BusNetwork::BN_BS);
    Point* bs_loc = (Point*)bs_tuple->GetAttribute(BusNetwork::BS_GEO);
    vector<int> neighbor_tid; 
    DFTraverse3(rtree, bs_rel, adr, bs_loc,neighbor_tid); 

//    cout<<"bs "<<*bs<<" neighbor size "<<neighbor_tid.size()<<endl; 
    for(unsigned int j = 0;j < neighbor_tid.size();j++){
      if(i != neighbor_tid[j]){

        Tuple* bs_tuple2 = bs_rel->GetTuple(neighbor_tid[j], false); 
        Bus_Stop* bs2 = (Bus_Stop*)bs_tuple2->GetAttribute(BusNetwork::BN_BS);
 //       cout<<"neighbor "<<*bs2<<endl; 
        
        bs_list1.push_back(*bs);
        bs_list2.push_back(*bs2); 
        bs_uoid_list.push_back(bs->GetUOid());
        bs_tuple2->DeleteIfAllowed(); 
      }

    }

    bs_tuple->DeleteIfAllowed(); 
  }

}

/*
find bus stops that map to the same spatial point 

*/
void BN::DFTraverse3(R_Tree<2,TupleId>* rtree, Relation* rel, 
                  SmiRecordId adr, Point* loc, vector<int>& neighbor_list)
{
  const double delta_dist = 0.001; 
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple2 = rel->GetTuple(e.info,false);
              Point* bus_loc = 
                    (Point*)dg_tuple2->GetAttribute(BusNetwork::BS_GEO);
              double d = loc->Distance(*bus_loc);
              if(d < delta_dist){
                  neighbor_list.push_back(e.info);
              }
              dg_tuple2->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc->Distance(e.box) < delta_dist){
              DFTraverse3(rtree, rel, e.pointer, loc, neighbor_list);
            }
      }
  }
  delete node;
}

/*
find all adjacent nodes of the given node id 

*/
void BN::GetAdjNodeBG1(BusGraph* bg, int nodeid)
{
  if(bg->node_rel == NULL){
    cout<<"no bus graph node rel"<<endl;
    return; 
  }
  if(nodeid < 1 || nodeid > bg->node_rel->GetNoTuples()){
      cout<<"invalid node id "<<endl; 
      return; 
  }

  cout<<"total "<<bg->node_rel->GetNoTuples()<<" nodes "<<endl;
  cout<<"total "<<bg->edge_rel1->GetNoTuples() +
                  bg->edge_rel2->GetNoTuples() +
                  bg->edge_rel3->GetNoTuples() <<" edges "<<endl;
  
  
  Tuple* bs_tuple = bg->node_rel->GetTuple(nodeid, false);
  Bus_Stop* bs1 = (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE);
//  cout<<*bs1<<endl; 

  /////////////////////////////////////////////////////////////////////////
  ///////////////the first kind of connection by pavements/////////////////
  /////////////////////////////////////////////////////////////////////////
  vector<int> tid_list1; 
  bg->FindAdj1(nodeid, tid_list1); 
  for(unsigned int i = 0;i < tid_list1.size();i++){
    Tuple* edge_tuple = bg->edge_rel1->GetTuple(tid_list1[i], false);
    Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2); 
    SimpleLine* path = 
             (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH1);
    bs_list1.push_back(*bs1);
    bs_list2.push_back(*bs2);
    path_sl_list.push_back(*path);

    edge_tuple->DeleteIfAllowed();
    type_list.push_back(1); 
  }

  ///////////////////////////////////////////////////////////////////////////
  //////the second kind of connection (no path; the same spatial location)////
  ////////////////////////////////////////////////////////////////////////////
  vector<int> tid_list2; 
  bg->FindAdj2(nodeid, tid_list2); 
  for(unsigned int i = 0;i < tid_list2.size();i++){
    Tuple* edge_tuple = bg->edge_rel2->GetTuple(tid_list2[i], false);
    Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2); 
    SimpleLine* path = new SimpleLine(0);
    path->StartBulkLoad();
    path->EndBulkLoad(); 
    bs_list1.push_back(*bs1);
    bs_list2.push_back(*bs2);
    path_sl_list.push_back(*path);
    delete path; 
    edge_tuple->DeleteIfAllowed();
    type_list.push_back(2); 
  }
  ////////////////////////////////////////////////////////////////////
  //////the third kind of connection (connected by moving buses)//////
  ////////////////////////////////////////////////////////////////////
  
  vector<int> tid_list3; 
  bg->FindAdj3(nodeid, tid_list3); 
//  cout<<"connection 3 size: "<<tid_list3.size()<<endl; 
  for(unsigned int i = 0;i < tid_list3.size();i++){
    Tuple* edge_tuple = bg->edge_rel3->GetTuple(tid_list3[i], false);
    Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2); 
    SimpleLine* path = 
          (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);
    bs_list1.push_back(*bs1);
    bs_list2.push_back(*bs2);
    path_sl_list.push_back(*path);

    edge_tuple->DeleteIfAllowed();
    type_list.push_back(3); 
  }

  bs_tuple->DeleteIfAllowed(); 
  
//  /* test  adjacency list1*/
//  for(int i = 1;i <= bg->node_rel->GetNoTuples();i++){
//    Tuple* node_tuple = bg->node_rel->GetTuple(i, false);
//    cout<<*node_tuple<<endl; 
//     vector<int> tid_list; 
//     bg->FindAdj1(i, tid_list); 
//    for(unsigned int j = 0;j < tid_list.size();j++){
//       Tuple* edge_tuple = bg->edge_rel1->GetTuple(tid_list[j], false);
//      Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2);
//       SimpleLine* path = 
//               (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH1);
//       cout<<i<<" "<<path->Length()<<endl; 
// 
//       edge_tuple->DeleteIfAllowed();
//     }
//    node_tuple->DeleteIfAllowed();
//   }

   /*test btree built on node relation*/
 
//  for(int i = 1;i <= bg->node_rel->GetNoTuples();i++){
//    Tuple* node_tuple = bg->node_rel->GetTuple(i, false);
//    Bus_Stop* bs = (Bus_Stop*)node_tuple->GetAttribute(BusGraph::BG_NODE);
// 
//     CcInt* search_id = new CcInt(true, bs->GetUOid());
//     BTreeIterator* btree_iter = bg->btree_node->ExactMatch(search_id);
// 
//     while(btree_iter->Next()){
//         cout<<"tid1 "<<node_tuple->GetTupleId()
//             <<"tid2 "<<btree_iter->GetId()<<endl;
//     }
//     delete btree_iter;
//     delete search_id;
// 
//    node_tuple->DeleteIfAllowed();
//   }


//  /* test  adjacency list2*/
//  for(int i = 1;i <= bg->node_rel->GetNoTuples();i++){
//    Tuple* node_tuple = bg->node_rel->GetTuple(i, false);
// 
//     vector<int> tid_list; 
//     bg->FindAdj2(i, tid_list); 
//    for(unsigned int j = 0;j < tid_list.size();j++){
//       Tuple* edge_tuple = bg->edge_rel2->GetTuple(tid_list[j], false);
//      Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2);
//       edge_tuple->DeleteIfAllowed();
//     }
// 
//    if(tid_list.size() > 0){
//       cout<<*node_tuple<<" neighbor size "<<tid_list.size()<<endl;
//    }
//    node_tuple->DeleteIfAllowed();
//   }

//  /* test  adjacency list3*/
//   for(int i = 1;i <= bg->node_rel->GetNoTuples();i++){
//    Tuple* node_tuple = bg->node_rel->GetTuple(i, false);
// 
//     vector<int> tid_list; 
//     bg->FindAdj3(i, tid_list); 
//     for(unsigned int j = 0;j < tid_list.size();j++){
//       Tuple* edge_tuple = bg->edge_rel3->GetTuple(tid_list[j], false);
//       edge_tuple->DeleteIfAllowed();
//     }
// 
//     if(tid_list.size() > 0){
//       cout<<" neighbor size "<<tid_list.size()<<endl;
//    }
//     node_tuple->DeleteIfAllowed();
//   }

}

/*
get the neighbor nodes of a given bus stop 
notes: the result may have the same value (type 3). this is becuase some bus
routes have night schedule (day schedule (workday and weekend))  
(night schedule (0:00-5:00 20:00-24:00)) 
there can be 4 tuples having the same value 

*/
void BN::GetAdjNodeBG2(BusGraph* bg, Bus_Stop* bs)
{
    int bs_tid = -1; 
    CcInt* search_id = new CcInt(true, bs->GetUOid());
    BTreeIterator* btree_iter = bg->btree_node->ExactMatch(search_id);

    while(btree_iter->Next()){
        bs_tid = btree_iter->GetId();
    }
    delete btree_iter;
    delete search_id;
    if(!(1 <= bs_tid && bs_tid <= bg->node_rel->GetNoTuples())){
      cout<<"invalid bus stop "<<*bs<<endl;
      return;
    }

    GetAdjNodeBG1(bg, bs_tid); 
}

/*
find connections of two adjacent bus stops in one route.
they are connected by the bus 

*/
void BN::BsNeighbors3(Relation* table_rel, Relation* mo_rel, BTree* btree_mo)
{
//  int count = 0; 
  for(int i = 1;i <= table_rel->GetNoTuples();i++){
    Tuple* bs_table_tuple = table_rel->GetTuple(i, false); 
    int bs_uoid = 
          ((CcInt*)bs_table_tuple->GetAttribute(BN_T_BUS_UOID))->GetIntval();
    vector<int> tid_list; 
    tid_list.push_back(i);
    int j = i + 1;
    while(j <= table_rel->GetNoTuples()){
      Tuple* tuple = table_rel->GetTuple(j, false);
      int bs_uoid2 = ((CcInt*)tuple->GetAttribute(BN_T_BUS_UOID))->GetIntval();
      tuple->DeleteIfAllowed();
      if(bs_uoid2 == bs_uoid){
        tid_list.push_back(j);
        j++; 
      }else{
        i = j - 1; 
        break; 
      } 
    }
//    cout<<tid_list.size()<<endl;
    ////////////////////////////////////////////
//    count += tid_list.size();
    ConnectionOneRoute(table_rel, tid_list, mo_rel, btree_mo);
    ////////////////////////////////////////////

    if(j > table_rel->GetNoTuples()){
        bs_table_tuple->DeleteIfAllowed();
        break;
    }

  }
//  cout<<count<<endl; 

}

/*
for a bus stop, find the connection to its consecutive bus stop in one route
for the last stop(no connection), 
we can set the length of the path (empty) as zero 

*/
void BN::ConnectionOneRoute(Relation* table_rel, vector<int> tid_list, 
                            Relation* mo_rel, BTree* btree_mo)
{
  const double delta_dist = 0.01;
  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* table_tuple = table_rel->GetTuple(tid_list[i], false);
    Point* bs_loc = (Point*)table_tuple->GetAttribute(BN_T_GEOBS);
    Bus_Stop* bs = (Bus_Stop*)table_tuple->GetAttribute(BN_T_BS);
    Periods* peri = (Periods*)table_tuple->GetAttribute(BN_T_P);
    double schedu = ((CcReal*)table_tuple->GetAttribute(BN_T_S))->GetRealval();
    int bs_uoid = 
        ((CcInt*)table_tuple->GetAttribute(BN_T_BUS_UOID))->GetIntval();


    /////////////////find the bus trip./////////////////////////////////
    /////////////////get the path to the next bus stop if exists////////////
//    int count = 0;
    CcInt* search_id = new CcInt(true, bs->GetId());
    BTreeIterator* btree_iter = btree_mo->ExactMatch(search_id);
    while(btree_iter->Next()){
      Tuple* mo_tuple = mo_rel->GetTuple(btree_iter->GetId(), false);
      unsigned int bus_r_id = 
            ((CcInt*)mo_tuple->GetAttribute(RoadDenstiy::BR_ID5))->GetIntval();
      assert(bus_r_id == bs->GetId()); 
      bool dir =  ((CcBool*)mo_tuple->GetAttribute( 
                  RoadDenstiy::MO_BUS_DIRECTION))->GetBoolval();

      bool find = false; 
      if(dir == bs->GetUp()){ //the same direction 
          MPoint* mo = (MPoint*)mo_tuple->GetAttribute(RoadDenstiy::BUS_TRIP);

          /////////////////////////////////////////////////////////////////
          ////////////////find the bus stop in units///////////////////////
          ////////////////////////////////////////////////////////////////
          int j = 0;
          for(;j < mo->GetNoComponents();j++){
            UPoint up;
            mo->Get(j, up);
            Point lp = up.p0;
            Point rp = up.p1;

            if(bs_loc->Distance(lp) < delta_dist && 
               bs_loc->Distance(rp) < delta_dist){
//              count++;
              break;
            }
          }
          assert(j < mo->GetNoComponents()); 

          if(j == mo->GetNoComponents() - 1){///last stop in the route 
//            cout<<"the last bus stop"<<endl; 
          }else{
            MPoint sub_move(0);
            sub_move.StartBulkLoad();

            double time_move = 0.0; 
            //////the leaving time of the bus, not arrival time 
            for(j++;j < mo->GetNoComponents();j++){//start from the next unit 
              UPoint up;
              mo->Get(j, up);
              Point lp = up.p0;
              Point rp = up.p1;

              if(lp.Distance(rp) < delta_dist)//next bus stop 
                break;
              else{
                sub_move.Add(up); 
                time_move += up.timeInterval.end.ToDouble() -
                           up.timeInterval.start.ToDouble(); 
              }
            }
            sub_move.EndBulkLoad();


//            cout<<sl->Length()<<endl; 
            if(dir){
              Bus_Stop* neighbor_bs = 
              new Bus_Stop(true, bs->GetId(), bs->GetStopId() + 1, bs->GetUp());
              /////////////////////////////////////////////////////////////////
              ///////////////////////store the result -2///////////////////////
              ////////////////////////////////////////////////////////////////
              bs_uoid_list.push_back(bs_uoid);
              bs_list1.push_back(*bs);
              bs_list2.push_back(*neighbor_bs); 
//              bus_trip_list.push_back(sub_move);
              duration.push_back(*peri);
              schedule_interval.push_back(schedu);

              Line traj(0);
              sub_move.Trajectory(traj); 
              SimpleLine sl_traj(0); 
              sl_traj.fromLine(traj); 
              path_sl_list.push_back(sl_traj); 
              delete neighbor_bs;
              time_cost_list.push_back(time_move); 
            }else{
              Bus_Stop* neighbor_bs =
              new Bus_Stop(true, bs->GetId(), bs->GetStopId() - 1, bs->GetUp());
              /////////////////////////////////////////////////////////////////
              ///////////////////////store the result -3///////////////////////
              ////////////////////////////////////////////////////////////////
              bs_uoid_list.push_back(bs_uoid);
              bs_list1.push_back(*bs);
              bs_list2.push_back(*neighbor_bs); 
//              bus_trip_list.push_back(sub_move);
              duration.push_back(*peri);
              schedule_interval.push_back(schedu);

              Line traj(0);
              sub_move.Trajectory(traj); 
              SimpleLine sl_traj(0); 
              sl_traj.fromLine(traj); 
              path_sl_list.push_back(sl_traj); 
              delete neighbor_bs; 
              time_cost_list.push_back(time_move); 
            }

            assert(j < mo->GetNoComponents()); 

          }
          ///////////////////////////////////////////////////////////////////
          find = true; 
      }
      mo_tuple->DeleteIfAllowed();
      if(find)break; 
    }
    delete btree_iter;
    delete search_id;
    /////////////////////////////////////////////////////////////////////
/*    cout<<"count "<<count<<endl;
    cout<<*bs<<endl;
    assert(count == 1);*/

    table_tuple->DeleteIfAllowed();

  }
}


///////////////////////////////////////////////////////////////////////////
////////////////////  Bus Network Graph  //////////////////////////////////
//////////////////////////////////////////////////////////////////////////
string BusGraph::NodeTypeInfo =
"(rel (tuple ((bus_stop busstop)(stop_geodata point))))";
string BusGraph::NodeInternalTypeInfo =
"(rel (tuple ((bus_stop busstop)(stop_geodata point)(bus_uoid int))))";
string BusGraph::NodeBTreeTypeInfo = 
"(btree (tuple ((bus_stop busstop)(stop_geodata point)(bus_uoid int))) int)";

string BusGraph::EdgeTypeInfo1 = 
"(rel (tuple ((bus_uoid int)(bus_stop1 busstop)(bus_stop2 busstop)(Path sline)\
(SubPath1 sline)(SubPath2 sline)(Path2 sline)(bus_stop2_tid int))))"; 

string BusGraph::EdgeTypeInfo2 = 
"(rel (tuple ((bus_uoid int)(bus_stop1 busstop)(bus_stop2 busstop)\
(bus_stop2_tid int))))"; 

/*
whole time:  time --- bus arrival time (30 seconds waiting)

*/
string BusGraph::EdgeTypeInfo3 = 
"(rel (tuple ((bus_uoid int)(bus_stop1 busstop)(bus_stop2 busstop)\
(whole_time periods)(schedule_interval real)(Path sline)(TimeCost real)\
(bus_stop2_tid int))))";



ListExpr BusGraph::BusGraphProp()
{
//    cout<<"BaseGraphProp()"<<endl;
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist,
               "createbusgraph(<id>,<edge-relation>,<node-relation>)");
    return nl->TwoElemList(
             nl->TwoElemList(nl->StringAtom("Creation"),
                              nl->StringAtom("Example Creation")),
             nl->TwoElemList(examplelist,
                   nl->StringAtom("let bg=createbusgraph(id,e-rel,n-rel)")));
}

bool BusGraph::CheckBusGraph(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckBusGraph()"<<endl;
  return nl->IsEqual(type, "busgraph");
}

int BusGraph::SizeOfBusGraph()
{
//  cout<<"SizeOfBusGraph()"<<endl;
  return 0;
}

void* BusGraph::CastBusGraph(void* addr)
{
//  cout<<"CastBusGraph()"<<endl;
  return 0;
}
Word BusGraph::CloneBusGraph(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneBusGraph()"<<endl;
  return SetWord(Address(0));
}

void BusGraph::CloseBusGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseBusGraph()"<<endl;
  delete static_cast<BusGraph*> (w.addr);
  w.addr = NULL;
}

Word BusGraph::CreateBusGraph(const ListExpr typeInfo)
{
//  cout<<"CreateBusGraph()"<<endl;
  return SetWord(new BusGraph());
}

void BusGraph::DeleteBusGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteBusGraph()"<<endl;
  BusGraph* bg = (BusGraph*)w.addr;
  delete bg;
  w.addr = NULL;
}

/*
input bus network graph 

*/
Word BusGraph::InBusGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
//  cout<<"InBusGraph()"<<endl;
  BusGraph* bg = new BusGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(bg);
  else{
    delete bg;
    return SetWord(Address(0));
  }
}

ListExpr BusGraph::OutBusGraph(ListExpr typeInfo, Word value)
{
//  cout<<"OutBusGraph()"<<endl;
  BusGraph* bg = (BusGraph*)value.addr;
  return bg->Out(typeInfo);
}

bool BusGraph::SaveBusGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveBusGraph()"<<endl;
  BusGraph* bg = (BusGraph*)value.addr;
  bool result = bg->Save(valueRecord, offset, typeInfo);

  return result;
}


bool BusGraph::OpenBusGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenBusGraph()"<<endl;
  value.addr = BusGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

BusGraph* BusGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
  return new BusGraph(valueRecord,offset,typeInfo);
}



BusGraph::BusGraph():bg_id(0), min_t(0),
node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0), 
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0),
edge_rel3(NULL), adj_list3(0), entry_adj_list3(0)
{
//  cout<<"BusGraph::BusGraph()"<<endl;
}

BusGraph::~BusGraph()
{
  if(node_rel != NULL) node_rel->Close();
  if(btree_node != NULL) delete btree_node; 
  if(edge_rel1 != NULL) edge_rel1->Close(); 
  if(edge_rel2 != NULL) edge_rel2->Close(); 
  if(edge_rel3 != NULL) edge_rel3->Close(); 
}


BusGraph::BusGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect):
bg_id(0), min_t(0),
node_rel(NULL), btree_node(NULL), 
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0), 
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0),
edge_rel3(NULL), adj_list3(0), entry_adj_list3(0)
{
//  cout<<"BusGraph::BusGraph(ListExpr)"<<endl;
}

BusGraph::BusGraph(SmiRecord& in_xValueRecord, size_t& inout_iOffset,
const ListExpr in_xTypeInfo):bg_id(0), min_t(0),
node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0), 
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0),
edge_rel3(NULL), adj_list3(0), entry_adj_list3(0)
{
  in_xValueRecord.Read(&bg_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Read(&min_t,sizeof(double),inout_iOffset);
  inout_iOffset += sizeof(double);

  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for node*********************/
  nl->ReadFromString(NodeInternalTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  node_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!node_rel) {
    return;
  }
  ///////////////////open btree built on nodes//////////////////////////
   nl->ReadFromString(NodeBTreeTypeInfo,xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
   btree_node = BTree::Open(in_xValueRecord, inout_iOffset, xNumericType);
   if(!btree_node) {
     node_rel->Delete();
     return;
   }

  /***********************Open relation for edge1*********************/
  nl->ReadFromString(EdgeTypeInfo1, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel1 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel1) {
    node_rel->Delete();
    delete btree_node; 
    return;
  }
  
  /////////////////open adjacency list1////////////////////////////////
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list1.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list1.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

   
  /***********************Open relation for edge2*********************/
  nl->ReadFromString(EdgeTypeInfo2, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel2 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel2) {
    node_rel->Delete();
    delete btree_node; 
    edge_rel1->Delete(); 
    adj_list1.clean();
    entry_adj_list1.clean(); 
    return;
  }
  
    /////////////////open adjacency list2////////////////////////////////
   bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list2.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list2.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

  /***********************Open relation for edge3*********************/
  nl->ReadFromString(EdgeTypeInfo3, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel3 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel3) {
    node_rel->Delete();
    delete btree_node; 
    edge_rel1->Delete(); 
    adj_list1.clean();
    entry_adj_list1.clean(); 
    edge_rel2->Delete();
    adj_list2.clean();
    entry_adj_list2.clean(); 
    return;
  }
  
   /////////////////open adjacency list3////////////////////////////////
   bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list3.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list3.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

}

ListExpr BusGraph::Out(ListExpr typeInfo)
{
//  cout<<"Out()"<<endl;
  ListExpr xNode = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= node_rel->GetNoTuples();i++){
      Tuple* node_tuple = node_rel->GetTuple(i, false);
      Bus_Stop* bs = (Bus_Stop*)node_tuple->GetAttribute(BG_NODE);
      xNext = OutBusStop(nl->TheEmptyList(),SetWord(bs));
      
      if(bFirst){
        xNode = nl->OneElemList(xNext);
        xLast = xNode;
        bFirst = false;
      }else
          xLast = nl->Append(xLast,xNext);
      node_tuple->DeleteIfAllowed();
  }

//   cout<< edge_rel1->GetNoTuples() + edge_rel2->GetNoTuples() + 
//          edge_rel3->GetNoTuples()<<endl;

  return nl->TwoElemList(nl->IntAtom(bg_id),xNode);

}

/*
save bus network graph 

*/
bool BusGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo)
{
//  cout<<"save "<<endl; 
  in_xValueRecord.Write(&bg_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Write(&min_t,sizeof(double),inout_iOffset);
  inout_iOffset += sizeof(double);

  ListExpr xType;
  ListExpr xNumericType;
  /************************save node****************************/
  nl->ReadFromString(NodeInternalTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  
  ////////////////save btree on nodes///////////////////////////
   nl->ReadFromString(NodeBTreeTypeInfo, xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType); 
   if(!btree_node->Save(in_xValueRecord, inout_iOffset, xNumericType))
     return false; 
   
  /************************save edge1****************************/
  nl->ReadFromString(EdgeTypeInfo1,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel1->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  
  /////////////////adjacency list 1//////////////////////////////
   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();


   adj_list1.saveToFile(rf, adj_list1);
   SmiSize offset = 0;
   size_t bufsize = adj_list1.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   adj_list1.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list1.saveToFile(rf, entry_adj_list1);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list1.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

   /////////////////////////////////////////////////////////////////////////

 /************************save edge2****************************/
  nl->ReadFromString(EdgeTypeInfo2,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel2->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /////////////////adjacency list 2//////////////////////////////

   adj_list2.saveToFile(rf, adj_list2);
   offset = 0;
   bufsize = adj_list2.headerSize()+ 2*sizeof(int);
   buf = (char*) malloc(bufsize);
   adj_list2.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list2.saveToFile(rf, entry_adj_list2);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list2.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

  /************************save edge3****************************/
  nl->ReadFromString(EdgeTypeInfo3,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel3->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

    /////////////////adjacency list 3//////////////////////////////

   adj_list3.saveToFile(rf, adj_list3);
   offset = 0;
   bufsize = adj_list3.headerSize()+ 2*sizeof(int);
   buf = (char*) malloc(bufsize);
   adj_list3.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list3.saveToFile(rf, entry_adj_list3);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list3.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

   return true; 
}

/*
load a bus graph 
edge1: the path in the pavement connecting two nearby bus stops
edge2: 

*/
void BusGraph::Load(int id, Relation* r1, Relation* edge1, 
                    Relation* edge2, Relation* edge3)
{
//   cout<<"BusGraph::Load()"<<endl; 

   bg_id = id;
  //////////////////node relation////////////////////

  ListExpr xTypeInfo;
  nl->ReadFromString(NodeInternalTypeInfo, xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation* s_rel = new Relation(xNumType, true);
  for(int i = 1;i <= r1->GetNoTuples();i++){
    Tuple* bs_tuple = r1->GetTuple(i, false);
    
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BG_NODE);
    Point* bs_loc = (Point*)bs_tuple->GetAttribute(BG_NODE_GEO);


    Tuple* new_bs_tuple = new Tuple(nl->Second(xNumType)); 
    new_bs_tuple->PutAttribute(BG_NODE, new Bus_Stop(*bs));
    new_bs_tuple->PutAttribute(BG_NODE_GEO, new Point(*bs_loc));
    new_bs_tuple->PutAttribute(BG_NODE_UOID, new CcInt(true, bs->GetUOid()));

    s_rel->AppendTuple(new_bs_tuple);

    new_bs_tuple->DeleteIfAllowed();
    bs_tuple->DeleteIfAllowed();
  }


  ListExpr ptrList1 = listutils::getPtrList(s_rel);

  string strQuery = "(consume(feed(" + NodeInternalTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";


  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;

  s_rel->Delete();

  ////////////////////////////btree on nodes/////////////////////////
  ListExpr ptrList2 = listutils::getPtrList(node_rel);

  strQuery = "(createbtree (" + NodeInternalTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "bus_uoid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  btree_node = (BTree*)xResult.addr;

  //////////////////////////////load edges///////////////////////////
  LoadEdge1(edge1);///connected by pavements path 
  LoadEdge2(edge2);///same spatial location, doing transfer 
  LoadEdge3(edge3);//moving buses in the same route 
  
}

/*
edges for pavement connectiong 

*/
void BusGraph::LoadEdge1(Relation* r)
{
  //////////////////////////////////////////////////////////////////////
  ////////////////////add the tid of the second bus stop/////////////////
  ////////////////////more efficient for query processing///////////////
  //////////////////////////////////////////////////////////////////////

  ListExpr ptrList1 = listutils::getPtrList(r);

  string strQuery = "(consume(feed(" + EdgeTypeInfo1 +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel1 = (Relation*)xResult.addr;

  GenericRelationIterator* edgeiter = edge_rel1->MakeScan();
  Tuple* edge_tuple; 
  while((edge_tuple = edgeiter->GetNextTuple()) != 0){

    //////////////////////use btree built on node rel////////////////////////

    Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BG_E_BS2);
    int bs2_tid = -1;
    CcInt* search_id = new CcInt(true, bs2->GetUOid());
    BTreeIterator* btree_iter = btree_node->ExactMatch(search_id);
    while(btree_iter->Next()){
      bs2_tid = btree_iter->GetId();
    }
    delete btree_iter;
    delete search_id;
    assert(bs2_tid > 0 && bs2_tid <= node_rel->GetNoTuples()); 
    /////////////////////////////////////////////////////////////////////////
/*    int bs2_tid_old = 
         ((CcInt*)edge_tuple->GetAttribute(BG_E_BS2_TID))->GetIntval();*/
    vector<int> xIndices;
    xIndices.push_back(BG_E_BS2_TID);
    vector<Attribute*> xAttrs;
    xAttrs.push_back(new CcInt(true, bs2_tid)); 
    edge_rel1->UpdateTuple(edge_tuple,xIndices,xAttrs);
//    cout<<"old "<<bs2_tid_old<<" new "<<bs2_tid<<endl; 
  }

//  cout<<"edge1 rel no "<<edge_rel1->GetNoTuples()<<endl; 

  //////////////////create adjacency list///////////////////////////////////
  
  ListExpr ptrList2 = listutils::getPtrList(edge_rel1);
  
  strQuery = "(createbtree (" + EdgeTypeInfo1 +
             "(ptr " + nl->ToString(ptrList2) + "))" + "bus_uoid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///////// visibility graph. it is the same as indoor graph ////////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* bs_tuple = node_rel->GetTuple(i, false);
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BG_NODE);

    CcInt* nodeid = new CcInt(true, bs->GetUOid());
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list1.Size();
    while(btree_iter->Next()){
      Tuple* edge_tuple = edge_rel1->GetTuple(btree_iter->GetId(), false);

      adj_list1.Append(edge_tuple->GetTupleId());//get the edge tuple id 

      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list1.Size();
    entry_adj_list1.Append(ListEntry(start, end));

    delete nodeid;

    bs_tuple->DeleteIfAllowed();

  }

  delete btree;

}


/*
edges for bus stops with the same spatial location but belong to different
bus routes 

*/
void BusGraph::LoadEdge2(Relation* r)
{

//  cout<<r->GetNoTuples()<<endl; 
  //////////////////////////////////////////////////////////////////////
  ////////////////////add the tid of the second bus stop/////////////////
  ////////////////////more efficient for query processing///////////////
  //////////////////////////////////////////////////////////////////////

  ListExpr ptrList1 = listutils::getPtrList(r);

  string strQuery = "(consume(feed(" + EdgeTypeInfo2 +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel2 = (Relation*)xResult.addr;

  GenericRelationIterator* edgeiter = edge_rel2->MakeScan();
  Tuple* edge_tuple; 
  while((edge_tuple = edgeiter->GetNextTuple()) != 0){

    //////////////////////use btree built on node rel////////////////////////

    Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BG_E2_BS2);
    int bs2_tid = -1;
    CcInt* search_id = new CcInt(true, bs2->GetUOid());
    BTreeIterator* btree_iter = btree_node->ExactMatch(search_id);
    while(btree_iter->Next()){
      bs2_tid = btree_iter->GetId();
    }
    delete btree_iter;
    delete search_id;
    assert(bs2_tid > 0 && bs2_tid <= node_rel->GetNoTuples()); 
    /////////////////////////////////////////////////////////////////////////
/*    int bs2_tid_old = 
         ((CcInt*)edge_tuple->GetAttribute(BG_E_BS2_TID))->GetIntval();*/
    vector<int> xIndices;
    xIndices.push_back(BG_E2_BS2_TID);
    vector<Attribute*> xAttrs;
    xAttrs.push_back(new CcInt(true, bs2_tid)); 
    edge_rel2->UpdateTuple(edge_tuple,xIndices,xAttrs);
//    cout<<"old "<<bs2_tid_old<<" new "<<bs2_tid<<endl; 
  }

//  cout<<"edge2 rel no "<<edge_rel2->GetNoTuples()<<endl; 


  //////////////////create adjacency list////////////////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(edge_rel2);

  strQuery = "(createbtree (" + EdgeTypeInfo2 +
             "(ptr " + nl->ToString(ptrList2) + "))" + "bus_uoid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///////// visibility graph. it is the same as indoor graph ////////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* bs_tuple = node_rel->GetTuple(i, false);
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BG_NODE);

    CcInt* nodeid = new CcInt(true, bs->GetUOid());
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list2.Size();
    while(btree_iter->Next()){
      Tuple* edge_tuple = edge_rel2->GetTuple(btree_iter->GetId(), false);

      adj_list2.Append(edge_tuple->GetTupleId());//get the edge tuple id 

      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list2.Size();
    entry_adj_list2.Append(ListEntry(start, end));

    delete nodeid;

    bs_tuple->DeleteIfAllowed();

  }

  delete btree;

}


/*
edges for bus stops belonging to the same route, the bus stops are connected
by moving buses 

*/
void BusGraph::LoadEdge3(Relation* e_rel)
{
//  cout<<e_rel->GetNoTuples()<<endl; 
  

  ListExpr ptrList1 = listutils::getPtrList(e_rel);

  string strQuery = "(consume(feed(" + EdgeTypeInfo3 +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel3 = (Relation*)xResult.addr;
//  Relation* edge_rel3 = (Relation*)xResult.addr;

  GenericRelationIterator* edgeiter = edge_rel3->MakeScan();
  Tuple* edge_tuple; 

  min_t = numeric_limits<double>::max(); 


  while((edge_tuple = edgeiter->GetNextTuple()) != 0){
     Periods* peri = (Periods*)edge_tuple->GetAttribute(BG_LIFETIME);
    Interval<Instant> periods;
    peri->Get(0, periods);
    double t = periods.start.ToDouble();
    if(t < min_t) min_t = t; 

    //////////////////////use btree built on node rel////////////////////////

    Bus_Stop* bs2 = (Bus_Stop*)edge_tuple->GetAttribute(BG_E3_BS2);
    int bs2_tid = -1;
    CcInt* search_id = new CcInt(true, bs2->GetUOid());
    BTreeIterator* btree_iter = btree_node->ExactMatch(search_id);
    while(btree_iter->Next()){
      bs2_tid = btree_iter->GetId();
    }
    delete btree_iter;
    delete search_id;
    assert(bs2_tid > 0 && bs2_tid <= node_rel->GetNoTuples()); 
//     int bs2_tid_old = 
//         ((CcInt*)edge_tuple->GetAttribute(BG_E3_BS2_TID))->GetIntval();

    /////////////////////////////////////////////////////////////////////////
    vector<int> xIndices;
    xIndices.push_back(BG_E3_BS2_TID);
    vector<Attribute*> xAttrs;
    xAttrs.push_back(new CcInt(true, bs2_tid)); 
    edge_rel3->UpdateTuple(edge_tuple,xIndices,xAttrs);
//    cout<<"old "<<bs2_tid_old<<" new "<<bs2_tid<<endl; 

  }

//   Instant min(instanttype);
//   min.ReadFrom(min_t);
//   printf("%.10f\n",min_t); 
//   cout<<min<<endl; 


 //////////////////create adjacency list////////////////////////////////////
  
  ListExpr ptrList2 = listutils::getPtrList(edge_rel3);

  strQuery = "(createbtree (" + EdgeTypeInfo3 +
             "(ptr " + nl->ToString(ptrList2) + "))" + "bus_uoid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///////// visibility graph. it is the same as indoor graph ////////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* bs_tuple = node_rel->GetTuple(i, false);
    Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BG_NODE);

    CcInt* nodeid = new CcInt(true, bs->GetUOid());
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list3.Size();
    while(btree_iter->Next()){
//      Tuple* edge_tuple = edge_rel2->GetTuple(btree_iter->GetId(), false);
      Tuple* edge_tuple = edge_rel3->GetTuple(btree_iter->GetId(), false);

      adj_list3.Append(edge_tuple->GetTupleId());//get the edge tuple id 

      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list3.Size();
    entry_adj_list3.Append(ListEntry(start, end));

    delete nodeid;

    bs_tuple->DeleteIfAllowed();

  }

  delete btree;

}


/*
adjacency list for the first kind of edge (pavements)

*/
void BusGraph::FindAdj1(int node_id, vector<int>& list)
{
  ListEntry list_entry;
  entry_adj_list1.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list1.Get(j, oid);
      j++;
      list.push_back(oid);
  }

}

/*
adjacency list for the second kind of edge 
(bus stops with the same spatial location but belong to different bus routes)

*/
void BusGraph::FindAdj2(int node_id, vector<int>& list)
{
  ListEntry list_entry;
  entry_adj_list2.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list2.Get(j, oid);
      j++;
      list.push_back(oid);
  }

}

/*
adjacency list for the third kind of edge 
two adjacent bus stops are connected by moving buses 

*/
void BusGraph::FindAdj3(int node_id, vector<int>& list)
{
  ListEntry list_entry;
  entry_adj_list3.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list3.Get(j, oid);
      j++;
      list.push_back(oid);
  }
}

/*
for a bus stop, find its tid in the node relation 

*/
int BusGraph::GetBusStop_Tid(Bus_Stop* bs)
{
  if(!bs->IsDefined()) return -1; 
  
  int bs_tid = -1;
  CcInt* search_id = new CcInt(true,bs->GetUOid());
  BTreeIterator* btree_iter = btree_node->ExactMatch(search_id);
  while(btree_iter->Next()){
     bs_tid = btree_iter->GetId();
  }
  delete btree_iter;
  delete search_id;
  assert(bs_tid > 0 && bs_tid <= node_rel->GetNoTuples()); 
  return bs_tid; 
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////navigation in bus network//////////////////
//////////////////////////////////////////////////////////////////////////

/*
shortest path from one bus stop to another in length 

*/
void BNNav::ShortestPath_Length(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{
//  cout<<"bus shortest path in length "<<endl; 

  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }

 if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
 }
 
  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

 if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
    cout<<"two bus stops equal to each other"<<endl;
    bn->CloseBusGraph(bg);
    return; 
 }

  priority_queue<BNPath_elem> path_queue;
  vector<BNPath_elem> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);
  
  ///////////  initialize the queue //////////////////////////////
  InitializeQueue1(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl; 


  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem dest;//////////destination
  
  while(path_queue.empty() == false){
    BNPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

//    top.Print();

    if(top.tri_index == bs2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 

    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavement ////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list1;
    bg->FindAdj1(top.tri_index, adj_list1);
    pos_expand_path = top.cur_index;
    for(unsigned int i = 0;i < adj_list1.size();i++){
      Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
      int neighbor_id1 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
      SimpleLine* path = 
                  (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

      if(visit_flag1[neighbor_id1 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

      cur_size = expand_queue.size();
      Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id1, false); 
      Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);

      double w = top.real_w + path->Length(); 
      double hw = p->Distance(end_p);
      BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w, 
                       *path, TM_WALK);
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      bs_node_tuple->DeleteIfAllowed(); 
      edge_tuple->DeleteIfAllowed();
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    vector<int> adj_list2;
    bg->FindAdj2(top.tri_index, adj_list2);
    for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
      int neighbor_id2 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();
      SimpleLine* path = new SimpleLine(0);
      path->StartBulkLoad();
      path->EndBulkLoad();

      if(visit_flag1[neighbor_id2 - 1]){
        edge_tuple->DeleteIfAllowed();
        delete path;
        continue; 
      }

      cur_size = expand_queue.size();
      double w = top.real_w; 

      Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id2, false); 
      Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
      double hw = p->Distance(end_p);
      bs_node_tuple->DeleteIfAllowed();


      BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w, 
                       *path, -1);
      path_queue.push(elem);
      expand_queue.push_back(elem);

      delete path; 

      edge_tuple->DeleteIfAllowed();
    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    for(unsigned int i = 0;i < adj_list3.size();i++){
      Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
      int neighbor_id3 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();
      SimpleLine* path = 
              (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);

      if(visit_flag1[neighbor_id3 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

      cur_size = expand_queue.size();
      double w = top.real_w + path->Length(); 

      Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id3, false); 
      Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
      double hw = p->Distance(end_p);
      bs_node_tuple->DeleteIfAllowed(); 

      BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w, 
                       *path, TM_BUS);
      path_queue.push(elem);
      expand_queue.push_back(elem);
      edge_tuple->DeleteIfAllowed();

    }

     visit_flag1[top.tri_index - 1] = true; 
  }
  ///////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem elem = expand_queue[id_list[i]];
      path_list.push_back(elem.path);
//      cout<<elem.tri_index<<endl; 
      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }

      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stop connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

    }
  }else{
//    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);

}


/*
put the start bus stop into the queue for minimum length  

*/
void BNNav::InitializeQueue1(Bus_Stop* bs1, Bus_Stop* bs2, 
                            priority_queue<BNPath_elem>& path_queue, 
                            vector<BNPath_elem>& expand_queue, BusNetwork* bn,
                            BusGraph* bg, Point& start_p, Point& end_p)
{

    int cur_size = expand_queue.size();
    double w = 0.0; 
    double hw = start_p.Distance(end_p);
    SimpleLine* sl = new SimpleLine(0);
    sl->StartBulkLoad();
    sl->EndBulkLoad();

    int bs_tid = bg->GetBusStop_Tid(bs1); 
//    cout<<"start bus stop tid "<<bs_tid<<endl;
    BNPath_elem elem(-1, cur_size, bs_tid, w + hw, w, *sl, TM_BUS);
    path_queue.push(elem);
    expand_queue.push_back(elem); 
    delete sl; 
}


/*
shortest path from one bus stop to another in time 

*/
void BNNav::ShortestPath_Time(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{
  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }
  
  if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
  }

  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

  if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   bn->CloseBusGraph(bg);
   return; 
  }
  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant bg_min(instanttype);
  bg_min.ReadFrom(bg->min_t); 

  assert(bg_min.GetWeekday() == 6);//start from Sunday 
  
  if(qt->GetWeekday() == 6){
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"Sunday"<<endl; 
    
  }else{ //Monday-Saturday 
    ////////////////////////////to Monday///////////////////////
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay() + 1, 
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"workday --->Monday"<<endl; 
  }
//  cout<<"mapping start time"<<new_st<<endl; 
  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem> path_queue;
  vector<BNPath_elem> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);
  
  //////////////////////////////////////////////////////////////////
  /////////////from bus network, get the maximum speed of the bus///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////
//  cout<<"max bus speed "<<bn->GetMaxSpeed()*60.0*60.0/1000.0<<"km/h"<<endl;

  ///////////  initialize the queue //////////////////////////////
  InitializeQueue2(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl; 

  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem dest;//////////destination
  double speed_human = 1.0; 

//   int elem_count = 0;

  while(path_queue.empty() == false){
    BNPath_elem top = path_queue.top();
    path_queue.pop();

//     elem_count++;

    if(visit_flag1[top.tri_index - 1])continue; 

//    top.Print();

    if(top.tri_index == bs2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 

    pos_expand_path = top.cur_index;
    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavements ///////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list1;
    bg->FindAdj1(top.tri_index, adj_list1);

    bool search_flag = true;
    BNPath_elem temp_elem = top;
    while(dest.prev_index != -1){
      if(temp_elem.tm == TM_BUS){
          break;
      }
      if(temp_elem.tm == TM_WALK){
          search_flag = false;
          break;
      }
      temp_elem = expand_queue[temp_elem.prev_index];
    }

    for(unsigned int i = 0;i < adj_list1.size() && search_flag;i++){
      Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
      int neighbor_id1 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
      SimpleLine* path = 
                  (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

      if(visit_flag1[neighbor_id1 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

      cur_size = expand_queue.size();

      double w = top.real_w + path->Length()/(speed_human*24.0*60.0*60.0); 

      Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id1, false); 
      Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
      double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
      bs_node_tuple->DeleteIfAllowed(); 


/*      BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w, 
                       *path, TM_WALK, true);*/
      SimpleLine temp_sl(0);
      BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w, 
                       temp_sl, TM_WALK, true);
      elem.type = TM_WALK;
      elem.edge_tid = adj_list1[i];


      path_queue.push(elem);
      expand_queue.push_back(elem);
      edge_tuple->DeleteIfAllowed();
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    vector<int> adj_list2;
    bg->FindAdj2(top.tri_index, adj_list2);

    for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
      int neighbor_id2 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();
      SimpleLine* path = new SimpleLine(0);
      path->StartBulkLoad();
      path->EndBulkLoad();

      if(visit_flag1[neighbor_id2 - 1]){
        edge_tuple->DeleteIfAllowed();
        delete path;
        continue; 
      }

      cur_size = expand_queue.size();
      double w = top.real_w; 
      Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id2, false); 
      Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
      double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
      bs_node_tuple->DeleteIfAllowed();


/*      BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                       *path, -1, false); //not useful for time cost */

      BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                       *path, -1, false); //not useful for time cost 
      elem.type = -1;
      elem.edge_tid = 0;

      path_queue.push(elem);
      expand_queue.push_back(elem); 

      delete path; 
      edge_tuple->DeleteIfAllowed();
    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    int64_t max_64_int = numeric_limits<int64_t>::max();

    for(unsigned int i = 0;i < adj_list3.size();i++){
      Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
       int neighbor_id3 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();
//        SimpleLine* path =
//                (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);

       if(visit_flag1[neighbor_id3 - 1]){
         edge_tuple->DeleteIfAllowed();
         continue; 
       }

       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current bus stop 
//       cout<<"time at bus stop "<<cur_inst<<endl; 

       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(BusGraph::BG_LIFETIME);
       Interval<Instant> periods;
       peri->Get(0, periods);

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_SCHEDULE))->GetRealval();
       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);

//       cout<<"st "<<periods.start<<" et "<<periods.end<<endl; 

       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
       double wait_time = 0.0;
       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         while(st_int < cur_t_int && st_int <= et_int){

/*          Instant temp(instanttype);
          temp.ReadFrom(st);
          cout<<"t1 "<<temp<<endl; */

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 
          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 

//           temp.ReadFrom(st);
//           cout<<"t2 "<<temp<<endl; 

         }
         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }

       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_TIMECOST))->GetRealval();
        double w = top.real_w + wait_time + weight; 

        Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id3, false);
        Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
        double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
        bs_node_tuple->DeleteIfAllowed(); 


/*       BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w,
                        *path, TM_BUS, true);*/
       SimpleLine temp_sl(0);
       BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w,
                        temp_sl, TM_BUS, true);
       elem.type = TM_BUS;
       elem.edge_tid = adj_list3[i];


       if(wait_time > 0.0){ //to the time waiting for bus 
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem); 


       edge_tuple->DeleteIfAllowed();

    }

    visit_flag1[top.tri_index - 1] = true; 

  }
/*  cout<<elem_count<<" elements poped from queue"<<endl;*/
  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    Instant t1 = *qt;
//    int no_transfer = 0; 

    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem elem = expand_queue[id_list[i]];
      ////////////////////////////////////////////////////////////
      if(elem.type == TM_WALK){
        assert(1 <= elem.edge_tid && 
              elem.edge_tid <= bg->edge_rel1->GetNoTuples());
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(elem.edge_tid, false);
        SimpleLine* path = 
                   (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);
        elem.path = *path;
        edge_tuple->DeleteIfAllowed();

      }else if(elem.type == TM_BUS){
        if(elem.edge_tid > 0){
          Tuple* edge_tuple = bg->edge_rel3->GetTuple(elem.edge_tid, false);
          SimpleLine* path =
            (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);
          elem.path = *path;
          edge_tuple->DeleteIfAllowed();
        }

      }else if(elem.type == -1){

      }else{
        cout<<"should not be here"<<endl;
        assert(false);
      }
      ////////////////////////////////////////////////////////////

      path_list.push_back(elem.path);

      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }


      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stops connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the bus 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = bs2_list[bs2_list.size() - 1];
          ////////the same as last bus stop //////////////////////
          bs2_list[bs2_list.size() - 1] = bs1_list[bs1_list.size() - 1];


          /////////////moving with bus////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          bs1_list.push_back(str1);
          bs2_list.push_back(str); 

        }

    }
//    cout<<" transfer "<<no_transfer<<" times "<<endl; 
  }else{
    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);
}

/*
shortest path from one bus stop to another in time 
edge without any cost, id does not connect walk edge and no cost edge because
its previous node has expanded these edges 
for the edge connected by moving buses, for a route which is accessed for the
  first time, it records the searching time of the whole periods. 
  in this case, it does not have to start from the begin of the periods for a 
  bus stop

*/
void BNNav::ShortestPath_TimeNew(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{
  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }
  
  if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
  }

  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

  if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   bn->CloseBusGraph(bg);
   return; 
  }
//  cout<<*bs1<<" "<<*bs2<<" "<<*qt<<endl;
  /////////////////////////////////////////////////////////////////////
  ////////// initialize counter for searhcing periods/////////////////
  ///////////////////////////////////////////////////////////////////
  vector<int> counter_up;
  vector<int> counter_down;
  for(int i = 0;i < bn->GetBR_Rel()->GetNoTuples();i++){
    counter_up.push_back(0);
    counter_down.push_back(0);
  }
//  cout<<counter_up.size()<<endl;

  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant bg_min(instanttype);
  bg_min.ReadFrom(bg->min_t); 

  assert(bg_min.GetWeekday() == 6);//start from Sunday 
  
  if(qt->GetWeekday() == 6){
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"Sunday"<<endl; 
    
  }else{ //Monday-Saturday 
    ////////////////////////////to Monday///////////////////////
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay() + 1, 
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"workday --->Monday"<<endl; 
  }
//  cout<<"mapping start time"<<new_st<<endl; 
  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem> path_queue;
  vector<BNPath_elem> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++)
    visit_flag1.push_back(false);
  
  //////////////////////////////////////////////////////////////////
  /////////////from bus network, get the maximum speed of the bus///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////
//  cout<<"max bus speed "<<bn->GetMaxSpeed()*60.0*60.0/1000.0<<"km/h"<<endl;

  ///////////  initialize the queue //////////////////////////////
  InitializeQueue2(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl; 

  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem dest;//////////destination
  double speed_human = 1.0; 
  
//  int elem_count = 0;

  while(path_queue.empty() == false){
    BNPath_elem top = path_queue.top();
    path_queue.pop();

//     elem_count++;

    if(visit_flag1[top.tri_index - 1])continue; 

//    top.Print();

    if(top.tri_index == bs2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 

    pos_expand_path = top.cur_index;
    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavements ///////////////////////
    //////////////////////////////////////////////////////////////////////
    if(top.tm == TM_BUS){
      vector<int> adj_list1;
      bg->FindAdj1(top.tri_index, adj_list1);

      bool search_flag = true;
      BNPath_elem temp_elem = top;
      while(dest.prev_index != -1){
        if(temp_elem.tm == TM_BUS){
          break;
        }
        if(temp_elem.tm == TM_WALK){
          search_flag = false;
          break;
        }
        temp_elem = expand_queue[temp_elem.prev_index];
      }

      for(unsigned int i = 0;i < adj_list1.size() && search_flag;i++){
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
        int neighbor_id1 = 
        ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
        SimpleLine* path = 
                  (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

        if(visit_flag1[neighbor_id1 - 1]){
          edge_tuple->DeleteIfAllowed();
          continue; 
        }

        cur_size = expand_queue.size();

        double w = top.real_w + path->Length()/(speed_human*24.0*60.0*60.0); 

        Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id1, false); 
        Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
        double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
        bs_node_tuple->DeleteIfAllowed(); 

/*        BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w, 
                       *path, TM_WALK, true);*/
        SimpleLine temp_sl(0);
        BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w, 
                       temp_sl, TM_WALK, true);
        elem.type = TM_WALK;
        elem.edge_tid = adj_list1[i];
        ////////////////////////////////////////////////////////////////

        path_queue.push(elem);
        expand_queue.push_back(elem);
        edge_tuple->DeleteIfAllowed();
      }
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    if(top.tm == TM_WALK || top.tm == TM_BUS){

      vector<int> adj_list2;
      bg->FindAdj2(top.tri_index, adj_list2);

      for(unsigned int i = 0;i < adj_list2.size();i++){
        Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
        int neighbor_id2 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();
        SimpleLine* path = new SimpleLine(0);
        path->StartBulkLoad();
        path->EndBulkLoad();

        if(visit_flag1[neighbor_id2 - 1]){
          edge_tuple->DeleteIfAllowed();
          delete path;
          continue; 
        }

        cur_size = expand_queue.size();
        double w = top.real_w; 
        Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id2, false); 
        Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
        double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
        bs_node_tuple->DeleteIfAllowed();


/*        BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                       *path, -1, false); //not useful for time cost */

        BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                       *path, -1, false); //not useful for time cost 
        elem.type = -1;
        elem.edge_tid = 0;
        ////////////////////////////////////////////////////////////////

        path_queue.push(elem);
        expand_queue.push_back(elem); 

        delete path; 
        edge_tuple->DeleteIfAllowed();
      }
    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    int64_t max_64_int = numeric_limits<int64_t>::max();

    for(unsigned int i = 0;i < adj_list3.size();i++){
      Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
       int neighbor_id3 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();
//        SimpleLine* path =
//                (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);

       if(visit_flag1[neighbor_id3 - 1]){
         edge_tuple->DeleteIfAllowed();
         continue; 
       }

       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current bus stop 
//       cout<<"time at bus stop "<<cur_inst<<endl; 

       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(BusGraph::BG_LIFETIME);
       Interval<Instant> periods;
       peri->Get(0, periods);

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_SCHEDULE))->GetRealval();
       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);


       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
       double wait_time = 0.0;
       Tuple* bs_top_tuple = bg->node_rel->GetTuple(top.tri_index, false);
       Bus_Stop* bs_top =
                 (Bus_Stop*)bs_top_tuple->GetAttribute(BusGraph::BG_NODE);


        if(bs_top->GetUp()){
          int last_record = counter_up[bs_top->GetId() - 1];
//          cout<<"up "<<last_record<<endl;

          st += sched * last_record;
          st_int = st * 86400000; 

        }else{
          int last_record = counter_down[bs_top->GetId() - 1];
//          cout<<"down "<<last_record<<endl;

          st += sched * last_record;
          st_int = st * 86400000; 
        }

//         temp.ReadFrom(st);
//         cout<<"t2 "<<temp<<endl; 
//         cout<<st_int<<" "<<cur_t_int<<endl;

       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         int record_count = 0;
         while(st_int < cur_t_int && st_int <= et_int){

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 

          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 
          record_count++;
         }

         if(bs_top->GetUp()){
          if(counter_up[bs_top->GetId() - 1] == 0) //for the first time 
             counter_up[bs_top->GetId() - 1] = record_count;
         }else{
          if(counter_down[bs_top->GetId() - 1] == 0) //for the first time 
             counter_down[bs_top->GetId() - 1] = record_count;
         }

         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }

        bs_top_tuple->DeleteIfAllowed();
       //////////////////////////////////////////////////////////////
 
       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_TIMECOST))->GetRealval();
        double w = top.real_w + wait_time + weight; 

        Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id3, false);
        Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
        double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
        bs_node_tuple->DeleteIfAllowed(); 


/*       BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w,
                        *path, TM_BUS, true);*/
       //////////////////////////////////////////////////////////////////
       SimpleLine temp_sl(0);
       BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w,
                        temp_sl, TM_BUS, true);
       elem.type = TM_BUS;
       elem.edge_tid = adj_list3[i];
       /////////////////////////////////////////////////////////////


       if(wait_time > 0.0){ //to the time waiting for bus 
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem); 


       edge_tuple->DeleteIfAllowed();

    }

    visit_flag1[top.tri_index - 1] = true; 

  }  
//   cout<<elem_count<<" elements poped from queue"<<endl;

  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    Instant t1 = *qt;
//    int no_transfer = 0; 

    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem elem = expand_queue[id_list[i]];
      ////////////////////////////////////////////////////////////
      if(elem.type == TM_WALK){
        assert(1 <= elem.edge_tid && 
              elem.edge_tid <= bg->edge_rel1->GetNoTuples());
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(elem.edge_tid, false);
        SimpleLine* path = 
                   (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);
        elem.path = *path;
        edge_tuple->DeleteIfAllowed();

      }else if(elem.type == TM_BUS){
        if(elem.edge_tid > 0){
          Tuple* edge_tuple = bg->edge_rel3->GetTuple(elem.edge_tid, false);
          SimpleLine* path =
            (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);
          elem.path = *path;
          edge_tuple->DeleteIfAllowed();
        }

      }else if(elem.type == -1){

      }else{
        cout<<"should not be here"<<endl;
        assert(false);
      }
      ////////////////////////////////////////////////////////////


      path_list.push_back(elem.path);

      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }


      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stops connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the bus 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = bs2_list[bs2_list.size() - 1];
          ////////the same as last bus stop //////////////////////
          bs2_list[bs2_list.size() - 1] = bs1_list[bs1_list.size() - 1];


          /////////////moving with bus////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          bs1_list.push_back(str1);
          bs2_list.push_back(str); 

        }

    }
//    cout<<" transfer "<<no_transfer<<" times "<<endl; 
  }else{
    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);
}

/*
initialize the queue: shortest path in time 

*/
void BNNav::InitializeQueue2(Bus_Stop* bs1, Bus_Stop* bs2, 
                            priority_queue<BNPath_elem>& path_queue, 
                            vector<BNPath_elem>& expand_queue, 
                            BusNetwork* bn, BusGraph* bg, 
                            Point& start_p, Point& end_p)
{
    int cur_size = expand_queue.size();
    double w = 0.0; 

    double hw = start_p.Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
//    double hw = 0.0; 

    SimpleLine* sl = new SimpleLine(0);
    sl->StartBulkLoad();
    sl->EndBulkLoad();

    int bs_tid = bg->GetBusStop_Tid(bs1); 
//    cout<<"start bus stop tid "<<bs_tid<<endl;
    //////////////////no time cost////////////////////////////////////
    BNPath_elem elem(-1, cur_size, bs_tid, w + hw, w, *sl, TM_BUS, false);
    
    elem.type = TM_BUS;//////start condition
    elem.edge_tid = 0;////initial condition
    
    path_queue.push(elem);
    expand_queue.push_back(elem); 
    delete sl;
}

/*
shortest path from one bus stop to another in time 
for the start bus stop, it does not consider the walk segment connection

*/
void BNNav::ShortestPath_Time2(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{

  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }

  if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
  }

  /////////////////////////////////////////////////////////////////////
  ////////// initialize counter for searhcing periods/////////////////
  ///////////////////////////////////////////////////////////////////
  vector<int> counter_up;
  vector<int> counter_down;
  for(int i = 0;i < bn->GetBR_Rel()->GetNoTuples();i++){
    counter_up.push_back(0);
    counter_down.push_back(0);
  }
  

  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

  if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   bn->CloseBusGraph(bg);
   return; 
  }
  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant bg_min(instanttype);
  bg_min.ReadFrom(bg->min_t); 

  assert(bg_min.GetWeekday() == 6);//start from Sunday 

  if(qt->GetWeekday() == 6){
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"Sunday"<<endl; 
    
  }else{ //Monday-Saturday 
    ////////////////////////////to Monday///////////////////////
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay() + 1, 
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"workday --->Monday"<<endl; 
  }
//  cout<<"mapping start time"<<new_st<<endl; 
  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem> path_queue;
  vector<BNPath_elem> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  vector<bool> visit_flag2;
  vector<int> visit_flag3;
  vector<string> tm_list1;
  vector<string> tm_list2;
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++){
    visit_flag1.push_back(false);
    visit_flag2.push_back(false);
    visit_flag3.push_back(0);
    tm_list1.push_back("empty");
    tm_list2.push_back("empty");
  }

  //////////////////////////////////////////////////////////////////
  /////////////from bus network, get the maximum speed of the bus///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////

  ///////////  initialize the queue //////////////////////////////
  InitializeQueue2(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl;

  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem dest;//////////destination
  double speed_human = 1.0;
  const double delta_t = 0.001;

  while(path_queue.empty() == false){
    BNPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

//    top.Print();

    if(top.tri_index == bs2_tid){
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 
    pos_expand_path = top.cur_index;

    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavements ///////////////////////
    //////////////////////////////////////////////////////////////////////

    ////for the first bus stop, do not consider walk segment////////////
    if(top.real_w > delta_t){

        if(top.tm == TM_BUS){
          bool search_flag = true;
          BNPath_elem temp_elem = top;

          while(temp_elem.prev_index != -1){
            if(temp_elem.tm == TM_BUS){
              break;
            }
            if(temp_elem.tm == TM_WALK){
                search_flag = false;
                break;
            }
            temp_elem = expand_queue[temp_elem.prev_index];
          }

          vector<int> adj_list1;
          bg->FindAdj1(top.tri_index, adj_list1);

        for(unsigned int i = 0;i < adj_list1.size() && search_flag;i++){
          Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
          int neighbor_id1 = 
        ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
          SimpleLine* path = 
                   (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

          if(visit_flag1[neighbor_id1 - 1]){
            edge_tuple->DeleteIfAllowed();
            continue; 
          }

          cur_size = expand_queue.size();

          double w = top.real_w + path->Length()/(speed_human*24.0*60.0*60.0); 

          Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id1, false); 
          Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
          double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
          bs_node_tuple->DeleteIfAllowed(); 


//           BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w,
//                        *path, TM_WALK, true);
          /////////////////////////////////////////////////////
          SimpleLine temp_sl(0);
          BNPath_elem elem(pos_expand_path, cur_size,neighbor_id1, w + hw, w, 
                       temp_sl, TM_WALK, true);
          elem.type = TM_WALK;
          elem.edge_tid = adj_list1[i];
          //////////////////////////////////////////////

          path_queue.push(elem);
          expand_queue.push_back(elem);
          edge_tuple->DeleteIfAllowed();


          if(visit_flag1[neighbor_id1 - 1] == 0)
              tm_list1[neighbor_id1 - 1] = "walk";
          else
              tm_list2[neighbor_id1 - 1] = "walk";

          visit_flag3[neighbor_id1 - 1]++;
        }

      }
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    if(top.tm == TM_WALK || top.tm == TM_BUS){

      vector<int> adj_list2;
      bg->FindAdj2(top.tri_index, adj_list2);

      for(unsigned int i = 0;i < adj_list2.size();i++){
        Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
        int neighbor_id2 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();

        if(visit_flag1[neighbor_id2 - 1]){
          edge_tuple->DeleteIfAllowed();
          continue; 
        }

        if(visit_flag2[neighbor_id2 - 1]){
          edge_tuple->DeleteIfAllowed();
          continue;
        }

        SimpleLine* path = new SimpleLine(0);
        path->StartBulkLoad();
        path->EndBulkLoad();

        cur_size = expand_queue.size();
        double w = top.real_w; 
        Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id2, false); 
        Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
        double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
        bs_node_tuple->DeleteIfAllowed();


/*        BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                       *path, -1, false); //-1, not useful for time cost */

         ///////////////////////////////////////
        BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                       *path, -1, false); //-1, not useful for time cost 
        elem.type = -1;
        elem.edge_tid = 0;
        //////////////////////////////////////////////
        path_queue.push(elem);
        expand_queue.push_back(elem);

        delete path; 
        edge_tuple->DeleteIfAllowed();


        if(visit_flag3[neighbor_id2 - 1] == 0)
            tm_list1[neighbor_id2 - 1] = "none";
        else
            tm_list2[neighbor_id2 - 1] = "none";

        visit_flag2[neighbor_id2 - 1] = true;

        visit_flag3[neighbor_id2 - 1]++;
      }

    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////

    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    int64_t max_64_int = numeric_limits<int64_t>::max();

    for(unsigned int i = 0;i < adj_list3.size();i++){
       Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
       int neighbor_id3 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();

       if(visit_flag1[neighbor_id3 - 1]){
         edge_tuple->DeleteIfAllowed();
         continue; 
       }


       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current bus stop 
//       cout<<"time at bus stop "<<cur_inst<<endl; 

       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(BusGraph::BG_LIFETIME);
       Interval<Instant> periods;
       peri->Get(0, periods);

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_SCHEDULE))->GetRealval();
       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);

//       cout<<"st "<<periods.start<<" et "<<periods.end<<endl; 

       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
       
       
       double wait_time = 0.0;
       ///////////////////////////////////////////////////////////////
       Tuple* bs_top_tuple = bg->node_rel->GetTuple(top.tri_index, false);
       Bus_Stop* bs_top = 
            (Bus_Stop*)bs_top_tuple->GetAttribute(BusGraph::BG_NODE);

       if(bs_top->GetUp()){
        int last_record = counter_up[bs_top->GetId() - 1];
        st += sched*last_record;
        st_int = st*86400000;
       }else{
         int last_record = counter_down[bs_top->GetId() - 1];
         st += sched*last_record;
         st_int = st*86400000;
       }

       ///////////////////////////////////////////////////////////////

       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         int record_count = 0;
         while(st_int < cur_t_int && st_int <= et_int){

/*          Instant temp(instanttype);
          temp.ReadFrom(st);
          cout<<"t1 "<<temp<<endl; */

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 
          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 
          record_count++;

//           temp.ReadFrom(st);
//           cout<<"t2 "<<temp<<endl; 
         }

         if(bs_top->GetUp()){
          if(counter_up[bs_top->GetId() - 1] == 0)
              counter_up[bs_top->GetId() - 1] = record_count;
         }else{
          if(counter_down[bs_top->GetId() - 1] == 0)
              counter_down[bs_top->GetId() - 1] = record_count;
         }

         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }////////end else

       bs_top_tuple->DeleteIfAllowed();

//        SimpleLine* path =
//             (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);

       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_TIMECOST))->GetRealval();
        double w = top.real_w + wait_time + weight; 

        Tuple* bs_node_tuple = bg->node_rel->GetTuple(neighbor_id3, false);
        Point* p = (Point*)bs_node_tuple->GetAttribute(BusGraph::BG_NODE_GEO);
        double hw = p->Distance(end_p)/(bn->GetMaxSpeed()*24.0*60.0*60.0);
        bs_node_tuple->DeleteIfAllowed(); 


/*       BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w,
                        *path, TM_BUS, true);*/
        /////////////////////////////////////////////////////
        SimpleLine temp_sl(0);
        BNPath_elem elem(pos_expand_path, cur_size,neighbor_id3, w + hw, w,
                        temp_sl, TM_BUS, true);
        elem.type = TM_BUS;
        elem.edge_tid = adj_list3[i];
        //////////////////////////////////////////////////////

       if(wait_time > 0.0){ //to the time waiting for bus 
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem); 
       edge_tuple->DeleteIfAllowed();
       
       if(visit_flag3[neighbor_id3 - 1] == 0)
         tm_list1[neighbor_id3 - 1] = "bus";
       else
         tm_list2[neighbor_id3 - 1] = "bus";
       
       visit_flag3[neighbor_id3 - 1]++;

    }

    visit_flag1[top.tri_index - 1] = true; 
  }




//   for(unsigned int i = 0;i < visit_flag3.size();i++){
//     if(visit_flag3[i] > 1) {
//       cout<<visit_flag3[i]<<" "<<tm_list1[i]<<" "<<tm_list2[i]<<endl;
//     }
//   }

  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ///constrcut the result 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    Instant t1 = *qt;
//    int no_transfer = 0;

    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem elem = expand_queue[id_list[i]];
      ////////////////////////////////////////////////////////////
      if(elem.type == TM_WALK){
        assert(1 <= elem.edge_tid && 
              elem.edge_tid <= bg->edge_rel1->GetNoTuples());
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(elem.edge_tid, false);
        SimpleLine* path = 
                   (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);
        elem.path = *path;
        edge_tuple->DeleteIfAllowed();

      }else if(elem.type == TM_BUS){
        if(elem.edge_tid > 0){
          Tuple* edge_tuple = bg->edge_rel3->GetTuple(elem.edge_tid, false);
          SimpleLine* path =
            (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);
          elem.path = *path;
          edge_tuple->DeleteIfAllowed();
        }

      }else if(elem.type == -1){

      }else{
        cout<<"should not be here"<<endl;
        assert(false);
      }

      ////////////////////////////////////////////////////////////
      path_list.push_back(elem.path);

      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }


      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stops connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the bus 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = bs2_list[bs2_list.size() - 1];
          ////////the same as last bus stop //////////////////////
          bs2_list[bs2_list.size() - 1] = bs1_list[bs1_list.size() - 1];


          /////////////moving with bus////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          bs1_list.push_back(str1);
          bs2_list.push_back(str); 

        }

    }
//    cout<<" transfer "<<no_transfer<<" times "<<endl; 
  }else{
//    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);
}


/*
shortest path from one bus stop to another in bus transfer

*/
void BNNav::ShortestPath_Transfer(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{
  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }
  
  if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
  }

  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

  if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   bn->CloseBusGraph(bg);
   return; 
  }
  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant bg_min(instanttype);
  bg_min.ReadFrom(bg->min_t); 

  assert(bg_min.GetWeekday() == 6);//start from Sunday 
  
  if(qt->GetWeekday() == 6){
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"Sunday"<<endl; 
    
  }else{ //Monday-Saturday 
    ////////////////////////////to Monday///////////////////////
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay() + 1, 
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"workday --->Monday"<<endl; 
  }
//  cout<<"mapping start time"<<new_st<<endl; 
  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem2> path_queue;
  vector<BNPath_elem2> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++){
    visit_flag1.push_back(false);
  }
  //////////////////////////////////////////////////////////////////
  /////////////from bus network, get the maximum speed of the bus///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////
//  cout<<"max bus speed "<<bn->GetMaxSpeed()*60.0*60.0/1000.0<<"km/h"<<endl;

  ///////////  initialize the queue //////////////////////////////
  InitializeQueue3(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl; 

//  ofstream output("debug.txt");
  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem2 dest;//////////destination
  double speed_human = 1.0; 
  
  while(path_queue.empty() == false){
    BNPath_elem2 top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 


//    top.Print();


    if(top.tri_index == bs2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 
    pos_expand_path = top.cur_index;

    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavements ///////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list1;
    bg->FindAdj1(top.tri_index, adj_list1);

    bool search_flag = true;
    BNPath_elem2 temp_elem = top;
    while(dest.prev_index != -1){
        if(temp_elem.tm == TM_BUS){
            break;
        }
        if(temp_elem.tm == TM_WALK){
            search_flag = false;
            break;
        }
        temp_elem = expand_queue[temp_elem.prev_index];
    }

    for(unsigned int i = 0;i < adj_list1.size() && search_flag;i++){
      Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
      int neighbor_id1 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
      SimpleLine* path = 
                  (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

      if(visit_flag1[neighbor_id1 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

      cur_size = expand_queue.size();

      double w2 = top.real_w + path->Length()/(speed_human*24.0*60.0*60.0);

      int w1 = top.weight;///walk is not bus transfer 

//       BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id1, w1, w2, 
//                        *path, TM_WALK, true);
      SimpleLine temp_sl(0);
      BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id1, w1, w2, 
                       temp_sl, TM_WALK, true);
      elem.type = TM_WALK;
      elem.edge_tid = adj_list1[i];

      path_queue.push(elem);
      expand_queue.push_back(elem);
      edge_tuple->DeleteIfAllowed();
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    vector<int> adj_list2;
    bg->FindAdj2(top.tri_index, adj_list2);

    for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
      int neighbor_id2 = 
      ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();
      SimpleLine* path = new SimpleLine(0);
      path->StartBulkLoad();
      path->EndBulkLoad();

      if(visit_flag1[neighbor_id2 - 1]){
        edge_tuple->DeleteIfAllowed();
        delete path;
        continue; 
      }

      cur_size = expand_queue.size();
      double w2 = top.real_w;

      int w1 = top.weight;
//       BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id2, w1, w2,
//                        *path, -1, false); //not useful for time cost 

      BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id2, w1, w2,
                       *path, -1, false); //not useful for time cost 
      elem.type = -1;
      elem.edge_tid = 0;

      
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      delete path; 
      edge_tuple->DeleteIfAllowed();
    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    int64_t max_64_int = numeric_limits<int64_t>::max();

    for(unsigned int i = 0;i < adj_list3.size();i++){
      Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
       int neighbor_id3 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();
//        SimpleLine* path =
//                (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);

      if(visit_flag1[neighbor_id3 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current bus stop 
//       cout<<"time at bus stop "<<cur_inst<<endl;


       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(BusGraph::BG_LIFETIME);
       Interval<Instant> periods;
       peri->Get(0, periods);

//       output<<"periods "<<periods<<"query time "<<cur_inst<<endl;

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_SCHEDULE))->GetRealval();
       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);

//       cout<<"st "<<periods.start<<" et "<<periods.end<<endl; 

       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
       double wait_time = 0.0;

       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         while(st_int < cur_t_int && st_int <= et_int){

/*          Instant temp(instanttype);
          temp.ReadFrom(st);
          cout<<"t1 "<<temp<<endl; */

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 
          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 

//           temp.ReadFrom(st);
//           cout<<"t2 "<<temp<<endl; 

         }
         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }

       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_TIMECOST))->GetRealval();
       double w2 = top.real_w + wait_time + weight;

      int w1;

      if(fabs(int64_t(wait_time*86400) - 30) <= 2 || 
        (int)(top.real_w*86400.0) == 0)
          w1 = top.weight;
       else w1 = top.weight + 1;


//        BNPath_elem2 elem(pos_expand_path, cur_size,neighbor_id3, w1, w2,
//                         *path, TM_BUS, true);
       SimpleLine temp_sl(0);
       BNPath_elem2 elem(pos_expand_path, cur_size,neighbor_id3, w1, w2,
                        temp_sl, TM_BUS, true);
       elem.type = TM_BUS;
       elem.edge_tid = adj_list3[i];


       if(wait_time > 0.0){ //to the time waiting for bus 
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem); 


       edge_tuple->DeleteIfAllowed();


    }

    visit_flag1[top.tri_index - 1] = true; 
  }
  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
//      cout<<dest.weight<<" bus transfers"<<endl; 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    Instant t1 = *qt;


    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem2 elem = expand_queue[id_list[i]];
      path_list.push_back(elem.path);

      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }


      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stops connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the bus 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = bs2_list[bs2_list.size() - 1];
          ////////the same as last bus stop //////////////////////
          bs2_list[bs2_list.size() - 1] = bs1_list[bs1_list.size() - 1];


          /////////////moving with bus////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          bs1_list.push_back(str1);
          bs2_list.push_back(str); 

        }

    }

  }else{
    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);
}


/*
shortest path from one bus stop to another in bus transfer
walk edge is not expanded twice; 
only bus edge is expanded by walk edge;
only walk edge and bus edge are expanded by no cost edge

*/
void BNNav::ShortestPath_TransferNew(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{
  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }
  
  if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
  }

  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

  if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   bn->CloseBusGraph(bg);
   return; 
  }
  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant bg_min(instanttype);
  bg_min.ReadFrom(bg->min_t); 

  assert(bg_min.GetWeekday() == 6);//start from Sunday 
  
  if(qt->GetWeekday() == 6){
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"Sunday"<<endl; 
    
  }else{ //Monday-Saturday 
    ////////////////////////////to Monday///////////////////////
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay() + 1, 
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"workday --->Monday"<<endl; 
  }
//  cout<<"mapping start time"<<new_st<<endl; 
  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem2> path_queue;
  vector<BNPath_elem2> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++){
    visit_flag1.push_back(false);
  }
  //////////////////////////////////////////////////////////////////
  /////////////from bus network, get the maximum speed of the bus///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////
//  cout<<"max bus speed "<<bn->GetMaxSpeed()*60.0*60.0/1000.0<<"km/h"<<endl;

  ///////////  initialize the queue //////////////////////////////
  InitializeQueue3(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl; 

//  ofstream output("debug.txt");
  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem2 dest;//////////destination
  double speed_human = 1.0; 
  
  while(path_queue.empty() == false){
    BNPath_elem2 top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 


//    top.Print();

    if(top.tri_index == bs2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 
    pos_expand_path = top.cur_index;

    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavements ///////////////////////
    //////////////////////////////////////////////////////////////////////
    if(top.tm == TM_BUS){
      vector<int> adj_list1;
      bg->FindAdj1(top.tri_index, adj_list1);

      bool search_flag = true;
      BNPath_elem2 temp_elem = top;
      while(dest.prev_index != -1){
        if(temp_elem.tm == TM_BUS){
            break;
        }
        if(temp_elem.tm == TM_WALK){
            search_flag = false;
            break;
        }
        temp_elem = expand_queue[temp_elem.prev_index];
      }

      for(unsigned int i = 0;i < adj_list1.size() && search_flag;i++){
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
        int neighbor_id1 = 
        ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
        SimpleLine* path = 
                  (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

        if(visit_flag1[neighbor_id1 - 1]){
          edge_tuple->DeleteIfAllowed();
          continue; 
        }

        cur_size = expand_queue.size();

        double w2 = top.real_w + path->Length()/(speed_human*24.0*60.0*60.0);

        int w1 = top.weight;///walk is not bus transfer 

//         BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id1, w1, w2, 
//                        *path, TM_WALK, true);
        SimpleLine temp_sl(0);
        BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id1, w1, w2, 
                       temp_sl, TM_WALK, true);
        elem.type = TM_WALK;
        elem.edge_tid = adj_list1[i];
        /////////////////////////////////////////////////////////////

        path_queue.push(elem);
        expand_queue.push_back(elem);
        edge_tuple->DeleteIfAllowed();
      }
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    if(top.tm == TM_WALK || top.tm == TM_BUS){
        vector<int> adj_list2;
        bg->FindAdj2(top.tri_index, adj_list2);

        for(unsigned int i = 0;i < adj_list2.size();i++){
          Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
          int neighbor_id2 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();
          SimpleLine* path = new SimpleLine(0);
          path->StartBulkLoad();
          path->EndBulkLoad();

          if(visit_flag1[neighbor_id2 - 1]){
            edge_tuple->DeleteIfAllowed();
            delete path;
            continue; 
          }

          cur_size = expand_queue.size();
          double w2 = top.real_w;

          int w1 = top.weight;
/*          BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id2, w1, w2,
                       *path, -1, false); //not useful for time cost */

          ///////////////////////////////////////////////////////////////
          BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id2, w1, w2,
                       *path, -1, false); //not useful for time cost 
          elem.type = -1;
          elem.edge_tid = 0;
          //////////////////////////////////////////////////////////

          path_queue.push(elem);
          expand_queue.push_back(elem); 

          delete path; 
          edge_tuple->DeleteIfAllowed();
      }
    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    int64_t max_64_int = numeric_limits<int64_t>::max();

    for(unsigned int i = 0;i < adj_list3.size();i++){
      Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
       int neighbor_id3 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();
//        SimpleLine* path =
//                (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);

      if(visit_flag1[neighbor_id3 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current bus stop 
//       cout<<"time at bus stop "<<cur_inst<<endl;


       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(BusGraph::BG_LIFETIME);
       Interval<Instant> periods;
       peri->Get(0, periods);

//       output<<"periods "<<periods<<"query time "<<cur_inst<<endl;

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_SCHEDULE))->GetRealval();
       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);

//       cout<<"st "<<periods.start<<" et "<<periods.end<<endl; 

       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
       double wait_time = 0.0;

       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         while(st_int < cur_t_int && st_int <= et_int){

/*          Instant temp(instanttype);
          temp.ReadFrom(st);
          cout<<"t1 "<<temp<<endl; */

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 
          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 

         }
         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }

       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_TIMECOST))->GetRealval();
       double w2 = top.real_w + wait_time + weight;

      int w1;

      if(fabs(int64_t(wait_time*86400) - 30) <= 2 || 
        (int)(top.real_w*86400.0) == 0)
          w1 = top.weight;
       else w1 = top.weight + 1;


//        BNPath_elem2 elem(pos_expand_path, cur_size,neighbor_id3, w1, w2,
//                         *path, TM_BUS, true);

       SimpleLine temp_sl(0);
       BNPath_elem2 elem(pos_expand_path, cur_size,neighbor_id3, w1, w2,
                        temp_sl, TM_BUS, true);
       elem.type = TM_BUS;
       elem.edge_tid = adj_list3[i];


       if(wait_time > 0.0){ //to the time waiting for bus 
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem); 


       edge_tuple->DeleteIfAllowed();

    }

    visit_flag1[top.tri_index - 1] = true; 
  }
  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
//      cout<<dest.weight<<" bus transfers"<<endl; 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    Instant t1 = *qt;


    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem2 elem = expand_queue[id_list[i]];

      ////////////////////////////////////////////////////////////
      if(elem.type == TM_WALK){
        assert(1 <= elem.edge_tid && 
              elem.edge_tid <= bg->edge_rel1->GetNoTuples());
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(elem.edge_tid, false);
        SimpleLine* path = 
                   (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);
        elem.path = *path;
        edge_tuple->DeleteIfAllowed();

      }else if(elem.type == TM_BUS){
        if(elem.edge_tid > 0){
          Tuple* edge_tuple = bg->edge_rel3->GetTuple(elem.edge_tid, false);
          SimpleLine* path =
            (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);
          elem.path = *path;
          edge_tuple->DeleteIfAllowed();
        }

      }else if(elem.type == -1){

      }else{
        cout<<"should not be here"<<endl;
        assert(false);
      }
      ////////////////////////////////////////////////////////////


      path_list.push_back(elem.path);

      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }


      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stops connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the bus 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = bs2_list[bs2_list.size() - 1];
          ////////the same as last bus stop //////////////////////
          bs2_list[bs2_list.size() - 1] = bs1_list[bs1_list.size() - 1];


          /////////////moving with bus////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          bs1_list.push_back(str1);
          bs2_list.push_back(str); 

        }

    }

  }else{
    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);
}

/*
shortest path from one bus stop to another in bus transfer
for the first bus stop, do not consider walk segment connection

*/
void BNNav::ShortestPath_Transfer2(Bus_Stop* bs1, Bus_Stop* bs2, Instant* qt)
{

  BusGraph* bg = bn->GetBusGraph(); 
  if(bg == NULL){
    cout<<"bus graph is invalid"<<endl; 
    return;
  }
  
  if(!bs1->IsDefined() || !bs2->IsDefined()){
   cout<<" bus stops are not defined"<<endl;
   return; 
  }

  Point start_p, end_p; 
  bn->GetBusStopGeoData(bs1, &start_p);
  bn->GetBusStopGeoData(bs2, &end_p);
  const double delta_dist = 0.01; 

  if(*bs1 == *bs2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   bn->CloseBusGraph(bg);
   return; 
  }
  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant bg_min(instanttype);
  bg_min.ReadFrom(bg->min_t); 

  assert(bg_min.GetWeekday() == 6);//start from Sunday 
  
  if(qt->GetWeekday() == 6){
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"Sunday"<<endl; 
    
  }else{ //Monday-Saturday 
    ////////////////////////////to Monday///////////////////////
    new_st.Set(bg_min.GetYear(),bg_min.GetMonth(),bg_min.GetGregDay() + 1, 
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());
//    cout<<"workday --->Monday"<<endl; 
  }
//  cout<<"mapping start time"<<new_st<<endl; 
  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem2> path_queue;
  vector<BNPath_elem2> expand_queue;

  vector<bool> visit_flag1;////////////bus stop visit 
  for(int i = 1; i <= bg->node_rel->GetNoTuples();i++){
    visit_flag1.push_back(false);
  }
  //////////////////////////////////////////////////////////////////
  /////////////from bus network, get the maximum speed of the bus///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////
//  cout<<"max bus speed "<<bn->GetMaxSpeed()*60.0*60.0/1000.0<<"km/h"<<endl;

  ///////////  initialize the queue //////////////////////////////
  InitializeQueue3(bs1, bs2, path_queue, expand_queue, bn, bg, start_p, end_p);

  int bs2_tid = bg->GetBusStop_Tid(bs2);
//  cout<<"end bus stop tid "<<bs2_tid<<endl; 

//  ofstream output("debug.txt");
  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the bus graph//////////////////////////
  /////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem2 dest;//////////destination
  double speed_human = 1.0; 
  const double delta_t = 0.001;
  
  while(path_queue.empty() == false){
    BNPath_elem2 top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1])continue; 

//    top.Print();

    if(top.tri_index == bs2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 
    pos_expand_path = top.cur_index;

    ///////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 by pavements ///////////////////////
    //////////////////////////////////////////////////////////////////////

    if(top.real_w > delta_t){
       if(top.tm == TM_BUS){
        ////////////////////////////////////////////////////
          bool search_flag = true;
          BNPath_elem2 temp_elem = top;
//          while(dest.prev_index != -1){
          while(temp_elem.prev_index != -1){
            if(temp_elem.tm == TM_BUS){
              break;
            }
            if(temp_elem.tm == TM_WALK){
              search_flag = false;
              break;
            }
            temp_elem = expand_queue[temp_elem.prev_index];
          }

        ////////////////////////////////////////////////////
          vector<int> adj_list1;
          bg->FindAdj1(top.tri_index, adj_list1);
          for(unsigned int i = 0;i < adj_list1.size() && search_flag;i++){
            Tuple* edge_tuple = bg->edge_rel1->GetTuple(adj_list1[i], false);
            int neighbor_id1 = 
        ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E_BS2_TID))->GetIntval();
            SimpleLine* path = 
                  (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);

            if(visit_flag1[neighbor_id1 - 1]){
              edge_tuple->DeleteIfAllowed();
              continue; 
            }

            cur_size = expand_queue.size();

           double w2 = top.real_w + path->Length()/(speed_human*24.0*60.0*60.0);

           int w1 = top.weight;///walk is not bus transfer 

//           BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id1, w1, w2,
//                       *path, TM_WALK, true);
           /////////////////////////////////////////////////////////////
           SimpleLine temp_sl(0);
           BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id1, w1, w2,
                       temp_sl, TM_WALK, true);
           elem.type = TM_WALK;
           elem.edge_tid = adj_list1[i];
           ////////////////////////////////////////////////////////////

           path_queue.push(elem);
           expand_queue.push_back(elem);
           edge_tuple->DeleteIfAllowed();
          }
      }
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 2 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    if(top.tm == TM_WALK || top.tm == TM_BUS){
        vector<int> adj_list2;
        bg->FindAdj2(top.tri_index, adj_list2);

        for(unsigned int i = 0;i < adj_list2.size();i++){
          Tuple* edge_tuple = bg->edge_rel2->GetTuple(adj_list2[i], false);
          int neighbor_id2 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E2_BS2_TID))->GetIntval();
          SimpleLine* path = new SimpleLine(0);
          path->StartBulkLoad();
          path->EndBulkLoad();

          if(visit_flag1[neighbor_id2 - 1]){
            edge_tuple->DeleteIfAllowed();
            delete path;
            continue; 
          }

          cur_size = expand_queue.size();
          double w2 = top.real_w;

          int w1 = top.weight;
/*          BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id2, w1, w2,
                       *path, -1, false); //not useful for time cost */
          /////////////////////////////////////////////////////////////
          BNPath_elem2 elem(pos_expand_path, cur_size, neighbor_id2, w1, w2,
                       *path, -1, false); //not useful for time cost 
          elem.type = -1;
          elem.edge_tid = 0;
          ///////////////////////////////////////////////////////////////

          path_queue.push(elem);
          expand_queue.push_back(elem); 

          delete path; 
          edge_tuple->DeleteIfAllowed();
      }
    }

    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 3 moving buses/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list3;
    bg->FindAdj3(top.tri_index, adj_list3);
    int64_t max_64_int = numeric_limits<int64_t>::max();

    for(unsigned int i = 0;i < adj_list3.size();i++){
      Tuple* edge_tuple = bg->edge_rel3->GetTuple(adj_list3[i], false);
       int neighbor_id3 = 
       ((CcInt*)edge_tuple->GetAttribute(BusGraph::BG_E3_BS2_TID))->GetIntval();

      if(visit_flag1[neighbor_id3 - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }

       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current bus stop 
//       cout<<"time at bus stop "<<cur_inst<<endl;


       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(BusGraph::BG_LIFETIME);
       Interval<Instant> periods;
       peri->Get(0, periods);

//       output<<"periods "<<periods<<"query time "<<cur_inst<<endl;

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_SCHEDULE))->GetRealval();
       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);

//       cout<<"st "<<periods.start<<" et "<<periods.end<<endl; 

       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
       double wait_time = 0.0;

       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at bus stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         while(st_int < cur_t_int && st_int <= et_int){

/*          Instant temp(instanttype);
          temp.ReadFrom(st);
          cout<<"t1 "<<temp<<endl; */

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 
          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 

//           temp.ReadFrom(st);
//           cout<<"t2 "<<temp<<endl; 

         }
         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }

       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(BusGraph::BG_TIMECOST))->GetRealval();
       double w2 = top.real_w + wait_time + weight;

      int w1;

      if(fabs(int64_t(wait_time*86400) - 30) <= 2 || 
        (int)(top.real_w*86400.0) == 0)
          w1 = top.weight;
       else w1 = top.weight + 1;


/*       BNPath_elem2 elem(pos_expand_path, cur_size,neighbor_id3, w1, w2,
                        *path, TM_BUS, true);*/
        ///////////////////////////////////////////////////////
        SimpleLine temp_sl(0);
        BNPath_elem2 elem(pos_expand_path, cur_size,neighbor_id3, w1, w2,
                        temp_sl, TM_BUS, true);
        elem.type = TM_BUS;
        elem.edge_tid = adj_list3[i];
        ///////////////////////////////////////////////////////
       if(wait_time > 0.0){ //to the time waiting for bus 
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem); 


       edge_tuple->DeleteIfAllowed();

//        output<<"wait time "<<wait_time*86400.0
//              <<" move time "<<weight*86400.0<<endl;
//        Tuple* bs_tuple = bg->node_rel->GetTuple(neighbor_id3, false); 
//      Bus_Stop* bs_cur = (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE);
//     output<<"extend elem; transfer: "<<elem.weight<<" "<<elem.real_w*86400.0
//          <<" "<<*bs_cur<<endl<<endl;
//        bs_tuple->DeleteIfAllowed();

    }

    visit_flag1[top.tri_index - 1] = true; 
  }
  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
//      cout<<dest.weight<<" bus transfers"<<endl; 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
      }
      id_list.push_back(dest.cur_index);

    Bus_Stop bs_last = *bs1; 
    Instant t1 = *qt;


    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem2 elem = expand_queue[id_list[i]];

      ////////////////////////////////////////////////////////////
      if(elem.type == TM_WALK){
        assert(1 <= elem.edge_tid && 
              elem.edge_tid <= bg->edge_rel1->GetNoTuples());
        Tuple* edge_tuple = bg->edge_rel1->GetTuple(elem.edge_tid, false);
        SimpleLine* path = 
                   (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH2);
        elem.path = *path;
        edge_tuple->DeleteIfAllowed();

      }else if(elem.type == TM_BUS){
        if(elem.edge_tid > 0){
          Tuple* edge_tuple = bg->edge_rel3->GetTuple(elem.edge_tid, false);
          SimpleLine* path =
            (SimpleLine*)edge_tuple->GetAttribute(BusGraph::BG_PATH3);
          elem.path = *path;
          edge_tuple->DeleteIfAllowed();
        }

      }else if(elem.type == -1){

      }else{
        cout<<"should not be here"<<endl;
        assert(false);
      }
      ////////////////////////////////////////////////////////////

      path_list.push_back(elem.path);

      if(elem.tm == TM_WALK){
          tm_list.push_back(str_tm[elem.tm]); 
      }else if(elem.tm == TM_BUS){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }


      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end bus stops connected by the path ////////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "br: %d ", bs_last.GetId());
      sprintf(buf2, "stop: %d", bs_last.GetStopId());
      strcat (buf1, buf2);   
      if(bs_last.GetUp())strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      bs1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        bs2_list.push_back(str2);

      }else{////////////////the end bus stop 

        Tuple* bs_tuple = bg->node_rel->GetTuple(elem.tri_index, false); 
        Bus_Stop* bs_cur = 
              (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "br: %d ", bs_cur->GetId());
        sprintf(buf_2, "stop: %d", bs_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(bs_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        bs2_list.push_back(str2);
        bs_last = *bs_cur; 
        bs_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the bus 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = bs2_list[bs2_list.size() - 1];
          ////////the same as last bus stop //////////////////////
          bs2_list[bs2_list.size() - 1] = bs1_list[bs1_list.size() - 1];


          /////////////moving with bus////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          bs1_list.push_back(str1);
          bs2_list.push_back(str); 

        }

    }

  }else{
//    cout<<"bs1 ("<<*bs1<<") bs2 ("<<*bs2<<") not reachable "<<endl;
  }

  bn->CloseBusGraph(bg);
}

/*
initialize the queue: shortest path in bus transfer 

*/
void BNNav::InitializeQueue3(Bus_Stop* bs1, Bus_Stop* bs2, 
                            priority_queue<BNPath_elem2>& path_queue, 
                            vector<BNPath_elem2>& expand_queue, 
                            BusNetwork* bn, BusGraph* bg, 
                            Point& start_p, Point& end_p)
{
    int cur_size = expand_queue.size();
    int w1 = 0;
    double w2 = 0.0;

    SimpleLine* sl = new SimpleLine(0);
    sl->StartBulkLoad();
    sl->EndBulkLoad();

    int bs_tid = bg->GetBusStop_Tid(bs1); 
//    cout<<"start bus stop tid "<<bs_tid<<endl;
    //////////////////no transfer cost////////////////////////////////////
    BNPath_elem2 elem(-1, cur_size, bs_tid, w1, w2, *sl, TM_BUS, false);
    
    elem.type = TM_BUS;
    elem.edge_tid = 0;

    path_queue.push(elem);
    expand_queue.push_back(elem); 
    delete sl;
}


/*
converting from moving buses (mpoint) to genmo 

*/
void BNNav::BusToGenMO(Relation* mo_rel, Relation* br_rel, BTree* btree)
{

  for(int i = 1;i <= mo_rel->GetNoTuples();i++){
    Tuple* bus_tuple = mo_rel->GetTuple(i, false);
    unsigned int br_id = 
        ((CcInt*)bus_tuple->GetAttribute(RoadDenstiy::BR_ID5))->GetIntval();
    bool dir =  ((CcBool*)bus_tuple->GetAttribute(
                 RoadDenstiy::MO_BUS_DIRECTION))->GetBoolval();
    MPoint* mp = (MPoint*)bus_tuple->GetAttribute(RoadDenstiy::BUS_TRIP);
    MPToGenMO(mp,br_id, dir, br_rel, btree);
    bus_tuple->DeleteIfAllowed(); 
  }

}

/*
from mpoint, bool (direction), line id to genmo 

*/

void BNNav::MPToGenMO(MPoint* mp,unsigned int br_id, bool dir, Relation* br_rel,
                      BTree* btree)
{
//    cout<<"br id "<<br_id<<"dir "<<dir<<endl;
    GenMO* genmo = new GenMO(0); 
    genmo->StartBulkLoad(); 
    CcInt* search_id = new CcInt(true, br_id);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    SimpleLine* sl = new SimpleLine(0);
    unsigned int bus_line_id = 0;
    int bus_line_uoid = -1;
    while(btree_iter->Next()){
        Tuple* tuple = br_rel->GetTuple(btree_iter->GetId(), false);
        Bus_Route* br = (Bus_Route*)tuple->GetAttribute(BusNetwork::BN_BR);
        if(br->GetUp() == dir){
          br->GetGeoData(*sl); 
          assert(br->GetId() == br_id); 
          bus_line_id = 
            ((CcInt*)tuple->GetAttribute(BusNetwork::BN_ID2))->GetIntval();
          assert(bus_line_id == br_id); 
          bus_line_uoid = 
          ((CcInt*)tuple->GetAttribute(BusNetwork::BN_BR_OID))->GetIntval();
        }
        tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;

    assert(bus_line_uoid > 0); 

    double pos1 = -1.0;
    double pos2 = -1.0;
    bool up;
     /////////////find it is increasing or decreasing///////////////////////
    UPoint u1, u2;
    mp->Get(0, u1);
    mp->Get(mp->GetNoComponents() - 1, u2); 

    assert(sl->AtPoint(u1.p0, true, pos1));
    assert(sl->AtPoint(u2.p0, true, pos2));
    if(pos1 < pos2)  up = true;
    else up = false;

     ////////////////////////////////////////////////////////////////////////
     ///////// for numeric problem, we may not map all points to the sline////
     ////////so we find the first point and just simply add the length value//
     //////first we have to know the relative positon on the route//////////
     //////////////////////is increasing or decreasing///////////////////////
     ////////////////////////////////////////////////////////////////////////
    for(int i = 0;i < mp->GetNoComponents();i++){
      UPoint unit1;
      mp->Get(i, unit1); 

      ////////////////////the same when we convert it back////////////////
//      cout<<"i "<<i<<"p0 "<<unit1.p0<<" p1 "<<unit1.p1<<endl;
      if(i == 0){
        assert(sl->AtPoint(unit1.p0, true, pos1));
        assert(sl->AtPoint(unit1.p1, true, pos2));

      }else{
        double d = unit1.p0.Distance(unit1.p1);
        pos1 = pos2;
        if(up){
          pos2 += d;
        }else{
          pos2 -= d;
        }
      }
//      cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl; 

      Loc loc1(pos1,-1); 
      Loc loc2(pos2,-1); 
      GenLoc gloc1(bus_line_uoid, loc1);
      GenLoc gloc2(bus_line_uoid, loc2);
      int tm = GetTM("Bus"); 
      //////////////////////////////////////////////////////////////////
      /////////////correct way to create UGenLoc///////////////////////
      //////////////////////////////////////////////////////////////////
      UGenLoc* unit2 = new UGenLoc(unit1.timeInterval, gloc1, gloc2, tm);

      genmo->Add(*unit2); 
      delete unit2; 

    }
    delete sl;

    genmo->EndBulkLoad();

    genmo_list.push_back(*genmo);
    br_id_list.push_back(bus_line_uoid);
    mp_list.push_back(*mp);
    delete genmo; 
}

////////////////////////////////////////////////////////////////////////////
//////////////////        Create UBahn Trains    ///////////////////////////
////////////////////////////////////////////////////////////////////////////


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
/*              int second_val = schedule_t.GetSecond(); 
              int msecond_val = schedule_t.GetMillisecond(); 
              double double_s = schedule_t.ToDouble() - 
                                second_val/(24.0*60.0*60.0) -
                                msecond_val/(24.0*60.0*60.0*1000.0);*/

              //we should consider second and millisecond  
              double double_s = schedule_t.ToDouble();

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

    ////////collect all metro stops mapping to the same 2D point in space/////
    unsigned int j = i + 1;
    BusStop_Ext bse = station_list_new[0]; 

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

//    cout<<"br_id "<<br_id<<" stop_id "<<stop_id<<" "<<endl;
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
//  int msecond_val_s = st.GetMillisecond(); 
//   double double_s = st.ToDouble() - 
//                        second_val_s/(24.0*60.0*60.0) -
//                        msecond_val_s/(24.0*60.0*60.0*1000.0);

  double double_s = st.ToDouble();


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
//  int msecond_val_e = et.GetMillisecond(); 
//   double double_e = et.ToDouble() - 
//                        second_val_e/(24.0*60.0*60.0) -
//                        msecond_val_e/(24.0*60.0*60.0*1000.0);

  double double_e = et.ToDouble();

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

string UBTrain::TrainsTypeInfo = 
"(rel (tuple ((lineid int) (Up bool) (Trip mpoint))))"; 

string UBTrain::UBahnLineInfo = 
"(rel (tuple ((lineid int) (oid int) (geoData sline))))";

string UBTrain:: UBahnTrainsTypeInfo =
"(rel (tuple ((Id int) (Line int) (Up bool) (Trip mpoint) (schedule_id int))))";

string UBTrain::TrainsStopTypeInfo = 
"(rel (tuple ((LineId int) (loc point) (stop_id int))))"; 

string UBTrain::TrainsStopExtTypeInfo = 
"(rel (tuple ((LineId int) (loc point) (stop_id int) (Up bool))))";

string UBTrain::UBahnTrainsTimeTable =
"(rel (tuple ((station_loc point) (line_id int) (stop_id int)\
(train_direction bool) (whole_time periods) (schedule_interval real)\
(loc_id int))))";


/*
convert berlintest trains to generic moving objects 

*/
void UBTrain::TrainsToGenMO()
{
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* train_tuple = rel1->GetTuple(i, false); 
    int line_id = 
       ((CcInt*)train_tuple->GetAttribute(TRAIN_LINE_ID))->GetIntval();
    bool dir = 
        ((CcBool*)train_tuple->GetAttribute(TRAIN_LINE_DIR))->GetBoolval();
    MPoint* mp = ((MPoint*)train_tuple->GetAttribute(TRAIN_TRIP)); 

    GenMO* genmo = new GenMO(0); 

    MPToGenMO(mp, genmo, line_id); 

    genmo_list.push_back(*genmo);
    mp_list.push_back(*mp);
    br_id_list.push_back(line_id);
    direction_list.push_back(dir);

    delete genmo; 
    train_tuple->DeleteIfAllowed(); 

//    break; 
  }

}

/*
convert a moving point to a generic moving object 

*/
void UBTrain::MPToGenMO(MPoint* mp, GenMO* mo, int l_id)
{
//    cout<<"line id "<<l_id<<endl;

    mo->StartBulkLoad(); 
    CcInt* search_id = new CcInt(true, l_id);
    BTreeIterator* btree_iter = btree1->ExactMatch(search_id);
    SimpleLine* sl = new SimpleLine(0);
    int ub_line_id = -1; 
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

//  cout<<"line length "<<sl->Length()<<endl;

    for(int i = 0;i < mp->GetNoComponents();i++){
      UPoint unit1;
      mp->Get(i, unit1); 
      double pos1;
      double pos2;
      ////////////////////////////////////////////////////////////////////
      //////////////the same when we convert it back  (TRUE) /////////////
      ////////////////////////////////////////////////////////////////////
      assert(sl->AtPoint(unit1.p0, true, pos1));
      assert(sl->AtPoint(unit1.p1, true, pos2));
//      cout<<"pos1 "<<pos1<<" pos2 "<<pos2<<endl; 

      Loc loc1(pos1,-1); 
      Loc loc2(pos2,-1); 
      GenLoc gloc1(ub_line_id, loc1);
      GenLoc gloc2(ub_line_id, loc2);
      int tm = GetTM("Metro"); 
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


/////////////////////////////////////////////////////////////////////////
////////////////////create metro from road network///////////////////////
/////////////////////////////////////////////////////////////////////////
string MetroStruct::MetroRouteInfo = 
"(rel (tuple ((mr_id int) (mroute busroute) (oid int))))";

string MetroStruct::MetroTripTypeInfo_Com = "(rel (tuple ((mtrip1 genmo)\
(mtrip2 mpoint) (mr_id int) (Up bool) (mr_oid int) (oid int))))";


/*
create metro routes 

*/
void MetroStruct::CreateMRoute(DualGraph* dg, string type)
{
//  int no_mroute = 10;
  
  int no_mroute;
  if(type == "Berlin")no_mroute = 10;
  else if(type == "Houston") no_mroute = 20;
  
  vector<bool> cell_flag;
  for(int i = 1;i <= dg->node_rel->GetNoTuples();i++){
    cell_flag.push_back(false);
  }
  
  //////////////////////////////////////////////////////////////////
  ///////////////get the center///////////////////////////////////
  ////////////////////////////////////////////////////////////////
  Rectangle<2> area_box = dg->rtree_node->BoundingBox();
  double center_x = (area_box.MinD(0) + area_box.MaxD(0))/2;
  double center_y = (area_box.MinD(1) + area_box.MaxD(1))/2;
  Point center(true, center_x, center_y);
  int center_index = -1;
  
  for(int i = 1;i <= dg->node_rel->GetNoTuples();i++){
     Tuple* cell_tuple = dg->node_rel->GetTuple(i, false);
     Region* reg = (Region*)cell_tuple->GetAttribute(DualGraph::PAVEMENT);
     if(center.Inside(reg->BoundingBox())){
        center_index = i;
        break;
     }
     cell_tuple->DeleteIfAllowed();
  }
  assert(center_index >= 1);


  vector<int> find_cell_list; 
  int count = 1;
  
//  double min_dist = 22000.0;//minimum distance for a ubahn 
  double min_dist;
  if(type == "Berlin"){
    min_dist = 25000.0;
  }else if(type == "Houston"){
    min_dist = 60000.0;
  }else{
    cout<<"not processed"<<endl;
    assert(false);
  }
  double min_dist2 = 1500.0;
  
  while(count <= no_mroute){
    int cell1 = GetRandom() % dg->node_rel->GetNoTuples() + 1;
    if(cell_flag[cell1 - 1]) continue;
    int cell2 = GetRandom() % dg->node_rel->GetNoTuples() + 1;
    if(cell_flag[cell2 - 1]) continue;


    Tuple* cell_tuple1 = dg->node_rel->GetTuple(cell1, false);
    Tuple* cell_tuple2 = dg->node_rel->GetTuple(cell2, false);
    Region* reg1 = (Region*)cell_tuple1->GetAttribute(DualGraph::PAVEMENT);
    Region* reg2 = (Region*)cell_tuple2->GetAttribute(DualGraph::PAVEMENT);

    if(reg1->BoundingBox().Distance(reg2->BoundingBox()) < min_dist){

      cell_tuple2->DeleteIfAllowed();
      cell_tuple1->DeleteIfAllowed();
      continue;
    }

    bool valid = true;
    for(unsigned int i = 0;i < find_cell_list.size();i++){
      Tuple* cell_tuple = dg->node_rel->GetTuple(find_cell_list[i], false);
      Region* reg = (Region*)cell_tuple->GetAttribute(DualGraph::PAVEMENT);
      if(reg->BoundingBox().Distance(reg1->BoundingBox()) < min_dist2){
          valid = false;
          break;
      }
      if(reg->BoundingBox().Distance(reg2->BoundingBox()) < min_dist2){
          valid = false;
          break;
      }
      cell_tuple->DeleteIfAllowed();

    }
    if(valid == false){
      cell_tuple2->DeleteIfAllowed();
      cell_tuple1->DeleteIfAllowed();
      continue;
    }

//    cell_reg_list1.push_back(*reg1);
//    cell_reg_list2.push_back(*reg2);

    /////////////////////////////////////////////////////////////////
    //////////////calculate the metro route connecting two cells////
    //////////////////////////////////////////////////////////////////
//    cout<<"start "<<cell1<<" end "<<cell2<<endl;

    vector<int> path_list;
    if(count <= 4){
      vector<int> path_list1;
      dg->Path_Weight(cell1, center_index, path_list1);
      vector<int> path_list2;
      dg->Path_Weight(center_index, cell2, path_list2);

      for(unsigned int i = 0;i < path_list1.size();i++)
        path_list.push_back(path_list1[i]);

      for(unsigned int i = 1;i < path_list2.size();i++)
        path_list.push_back(path_list2[i]);
    }else{
      dg->Path_Weight(cell1, cell2, path_list);
    }
    /////////////////////////////////////////////////////////////////
//    cout<<path_list.size()<<endl; 

    if(BuildMetroRoute(path_list, dg, count)){
    ///////////////////////////////////////////////////////////////////
        cell_flag[cell1 - 1] = true;
        cell_flag[cell2 - 1] = true;

        find_cell_list.push_back(cell1);
        find_cell_list.push_back(cell2);
        count++;
    }

    cell_tuple2->DeleteIfAllowed();
    cell_tuple1->DeleteIfAllowed();

//    break;
  }
}

/*
create the connection line for two cells

*/
bool MetroStruct::BuildMetroRoute(vector<int> path_list, DualGraph* dg, 
                                  int count)
{
    vector<Point> ps_list;
    for(unsigned int i = 0;i < path_list.size();i++){
      Tuple* cell_tuple = dg->node_rel->GetTuple(path_list[i], false);
      Region* reg = (Region*)cell_tuple->GetAttribute(DualGraph::PAVEMENT);
      Rectangle<2> bbox_reg = reg->BoundingBox();
      double x = (bbox_reg.MinD(0) + bbox_reg.MaxD(0))/2;
      double y = (bbox_reg.MinD(1) + bbox_reg.MaxD(1))/2;
      Point p(true, x ,y);
      ps_list.push_back(p);
      cell_tuple->DeleteIfAllowed();
    }

    SimpleLine* mroute = new SimpleLine(0);
    mroute->StartBulkLoad();
    int edgeno = 0;
    for(unsigned int i = 0;i < ps_list.size() - 1;i++){
      Point lp = ps_list[i];
      Point rp = ps_list[i + 1];
      HalfSegment hs(true, lp, rp);
      hs.attr.edgeno = edgeno++;
      *mroute += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *mroute += hs;
    }

    mroute->EndBulkLoad();
    const double min_len = 10000.0;
    if(mroute->Length() < min_len){
      delete mroute;
      return false;
    }

//    mroute_list.push_back(*mroute);
    //////////////////////////////////////////////////////////////
    ////////////////use data type bus route represent the data///
    /////////////////////////////////////////////////////////////
    SpacePartition* sp = new SpacePartition();
    vector<MyHalfSegment> seq_halfseg;
    sp->ReorderLine(mroute, seq_halfseg);
    
    delete mroute;
    
//    cout<<seq_halfseg.size()<<endl;
    ////////////////////////////////////////////////////////////
    ///////////////create two:up and down//////////////////////
    //////////////////////////////////////////////////////////
     Bus_Route* br1 = new Bus_Route(count, true);
     br1->StartBulkLoad(); 
     int no_1 = 0;
     for(unsigned int i = 0; i < seq_halfseg.size();i++){
        MyHalfSegment mhs = seq_halfseg[i];
        HalfSegment hs(true, mhs.from, mhs.to);
        br1->Add2(hs, no_1);
        no_1++;
     }

     br1->EndBulkLoad();
     mroute_list.push_back(*br1);
     delete br1;
     ///////////////////////////////////////////////////////////////
     Bus_Route* br2 = new Bus_Route(count, false);
     br2->StartBulkLoad(); 
     int no_2 = 0;
     for(int i = seq_halfseg.size() - 1;i >= 0;i--){
        MyHalfSegment mhs = seq_halfseg[i];
        HalfSegment hs(true, mhs.to, mhs.from);
        br2->Add2(hs, no_2);
        no_2++;
     }

     br2->EndBulkLoad();
     mroute_list.push_back(*br2);
     delete br2;


    id_list.push_back(count);
    id_list.push_back(count);

    delete sp;
    /////////////////////////////////////////////////////////////
    return true;

}

/*
create metro stops: very important format !!!
Up: stop id increases from small to large 
Down: stop id decreases from large to small 
1 2 3 4 5 6 true
1 2 3 4 5 6 false 

*/
void MetroStruct::CreateMStop(Relation* mr)
{
  for(int i = 1;i <= mr->GetNoTuples();i++){
    Tuple* mr_tuple = mr->GetTuple(i, false);
    int mr_id = ((CcInt*)mr_tuple->GetAttribute(MR_ID))->GetIntval();
    Bus_Route* mr = (Bus_Route*)mr_tuple->GetAttribute(MR_ROUTE);
    bool dir = mr->GetUp();
    assert(mr_id == (int) mr->GetId());

    SimpleLine sl(0);
    mr->GetGeoData(sl);

    SpacePartition* sp = new SpacePartition();
    vector<MyHalfSegment> seq_halfseg;
    sp->ReorderLine(&sl, seq_halfseg);

    int s_id = 1;
    for(unsigned int j = 0;j < seq_halfseg.size();j++){
        Bus_Stop ms_stop(true, mr_id, s_id, dir);
        Point geo_loc = seq_halfseg[j].from;
        s_id++;

        mstop_list.push_back(ms_stop);
        stop_geo_list.push_back(geo_loc);
        id_list.push_back(mr_id);

        if(j == seq_halfseg.size() - 1){
          Bus_Stop ms_stop2(true, mr_id, s_id, dir);
          Point geo_loc2 = seq_halfseg[j].to;
          s_id++;

          mstop_list.push_back(ms_stop2);
          stop_geo_list.push_back(geo_loc2);
          id_list.push_back(mr_id);
        }

    }

    delete sp; 

    mr_tuple->DeleteIfAllowed();
  }

}

/*
create moving metros 

*/
void MetroStruct::CreateMTrips(Relation* mr, Periods* peri)
{

  for(int i = 1;i <= mr->GetNoTuples();i++){
    Tuple* mr_tuple = mr->GetTuple(i, false);
    int mr_id = ((CcInt*)mr_tuple->GetAttribute(MR_ID))->GetIntval();
    Bus_Route* mr = (Bus_Route*)mr_tuple->GetAttribute(MR_ROUTE);
    bool dir = mr->GetUp();
    int mr_oid = ((CcInt*)mr_tuple->GetAttribute(MR_OID))->GetIntval();

    assert(mr_id == (int) mr->GetId());

    SimpleLine sl(0);
    mr->GetGeoData(sl);

    SpacePartition* sp = new SpacePartition();
    vector<MyHalfSegment> seq_halfseg;
    sp->ReorderLine(&sl, seq_halfseg);

    if(dir){
      CreateMetroUp(seq_halfseg, peri, mr_id, mr_oid, dir);
    }else{
      CreateMetroDown(seq_halfseg, peri, mr_id, mr_oid, dir);

    }
    delete sp;

  }
  
  ///////////////////////////////////////////////////////////////////////
  /////////////copy all trips one day before//////////////////////////////
  //////////////////////////////////////////////////////////////////////
  CopyTripOneDayBefore();

}

/*
create metros up direction 

*/
void MetroStruct::CreateMetroUp(vector<MyHalfSegment>& seg_list, 
                                Periods* peri, int mr_id, int mr_oid, bool dir)
{
  double metro_speed = 70.0*1000.0/3600.0;// -- 70km h, convert to meter second
  double time_interval = 30.0/(24.0*60.0*60.0); //time of waiting 

  Interval<Instant> metro_periods;
  peri->Get(0, metro_periods);

  Instant start_time = metro_periods.start;
  start_time.ReadFrom(start_time.ToDouble() + (GetRandom() % 30)/(24.0*60.0));


//  cout<<start_time<<endl;

  MPoint* mo = new MPoint(0);
  GenMO* genmo = new GenMO(0);
  mo->StartBulkLoad();
  genmo->StartBulkLoad();
  ////////////genmo/////////////
  ///////////mpoint/////////////
  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 

  for(unsigned int i = 0;i < seg_list.size();i++){
    Point from_loc = seg_list[i].from;
    Point to_loc = seg_list[i].to; 

    double dist = from_loc.Distance(to_loc);
    double time = dist/metro_speed; 

    //////////////////////static unit /////////////////////////////
//    et.ReadFrom(st.ToDouble() + 30.0/(24.0*60.0*60.0));//30 seconds
    et.ReadFrom(st.ToDouble() + time_interval);//30 seconds

    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up1 = new UPoint(up_interval,from_loc,from_loc);
  //    cout<<*up<<endl; 
    mo->Add(*up1);

    delete up1; 
    st = et; 
    /////////////////////////////////////////////////////////////
    /////////////generic location unit /////////////////////////
    ////////////////////////////////////////////////////////////
    Loc loc1(i + 1, 0.0);
    Loc loc2(i + 1, 0.0);
    GenLoc gloc1(mr_oid, loc1);
    GenLoc gloc2(mr_oid, loc2);
    int tm = GetTM("Metro"); 
    UGenLoc* unit1 = new UGenLoc(up_interval, gloc1, gloc2, tm);
    genmo->Add(*unit1);
    delete unit1;


    ////////////////moving unit /////////////////////////////////
    et.ReadFrom(st.ToDouble() + time/(24.0*60.0*60.0));

    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up2 = new UPoint(up_interval,from_loc,to_loc);
  //    cout<<*up<<endl; 
    mo->Add(*up2);
    delete up2; 
    st = et; 
    //////////////////////////////////////////////////////////////
    /////////////generic location unit //////////////////////////
    ////////////////////////////////////////////////////////////
    Loc loc3(i + 1, 0.0);
    Loc loc4(i + 1 + 1, 0.0);
    GenLoc gloc3(mr_oid, loc3);
    GenLoc gloc4(mr_oid, loc4);
    tm = GetTM("Metro"); 
    UGenLoc* unit2 = new UGenLoc(up_interval, gloc3, gloc4, tm);
    genmo->Add(*unit2);
    delete unit2;


    /////////////////////////////////////////////////////////////

    if(i == seg_list.size() - 1){//////the last stop 

      et.ReadFrom(st.ToDouble() + 30.0/(24.0*60.0*60.0));//30 seconds

      ////////////////////create a upoint////////////////////////
      up_interval.start = st;
      up_interval.lc = true;
      up_interval.end = et;
      up_interval.rc = false; 
      UPoint* up = new UPoint(up_interval,to_loc,to_loc);
  //    cout<<*up<<endl; 
      mo->Add(*up);
      delete up; 
      st = et; 
      //////////////////////////////////////////////////////////////
      /////////////generic location unit //////////////////////////
      ////////////////////////////////////////////////////////////

      Loc loc5(i + 1 + 1, 0.0);
      Loc loc6(i + 1 + 1, 0.0);
      GenLoc gloc5(mr_oid, loc5);
      GenLoc gloc6(mr_oid, loc6);
      tm = GetTM("Metro");
      UGenLoc* unit3 = new UGenLoc(up_interval, gloc5, gloc6, tm);
      genmo->Add(*unit3);
      delete unit3;

    }
  }

  mo->EndBulkLoad();
  genmo->EndBulkLoad();


  mtrip_list1.push_back(*genmo);
  mtrip_list2.push_back(*mo);
  id_list.push_back(mr_id);
  mr_oid_list.push_back(mr_oid);//unique object id for a metro route 
  
  dir_list.push_back(dir);

  CopyMetroTrip(genmo, mo, peri, start_time, mr_id, mr_oid, dir);

  delete mo;
  delete genmo;
}

/*
create metros down direction 

*/
void MetroStruct::CreateMetroDown(vector<MyHalfSegment>& seg_list, 
                                  Periods* peri, int mr_id, 
                                  int mr_oid, bool dir)
{
  double metro_speed = 70.0*1000.0/3600;// -- 70km h, convert to meter minute
  double time_interval = 30.0/(24.0*60.0*60.0); //time of waiting 


  Interval<Instant> metro_periods;
  peri->Get(0, metro_periods);

  Instant start_time = metro_periods.start;
  start_time.ReadFrom(start_time.ToDouble() + (GetRandom() % 30)/(24.0*60.0));

  MPoint* mo = new MPoint(0);
  GenMO* genmo = new GenMO(0);
  mo->StartBulkLoad();
  genmo->StartBulkLoad();


  ////////////genmo/////////////
  ///////////mpoint/////////////
  Instant st = start_time;
  Instant et = start_time; 
  Interval<Instant> up_interval; 


  for(int i = seg_list.size() - 1; i >= 0; i--){

    Point from_loc = seg_list[i].to;
    Point to_loc = seg_list[i].from; 

    double dist = from_loc.Distance(to_loc);
    double time = dist/metro_speed;

    //////////////////////static unit /////////////////////////////
//    et.ReadFrom(st.ToDouble() + 30.0/(24.0*60.0*60.0));//30 seconds
    et.ReadFrom(st.ToDouble() + time_interval);//30 seconds

    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up1 = new UPoint(up_interval,from_loc,from_loc);
  //    cout<<*up<<endl; 
    mo->Add(*up1);

    delete up1; 
    st = et; 

    /////////////////////////////////////////////////////////////
    /////////////generic location unit /////////////////////////
    ////////////////////////////////////////////////////////////

     Loc loc1(i + 1 + 1, 0.0);
     Loc loc2(i + 1 + 1, 0.0);
     GenLoc gloc1(mr_oid, loc1);
     GenLoc gloc2(mr_oid, loc2);
     int tm = GetTM("Metro"); 
     UGenLoc* unit1 = new UGenLoc(up_interval, gloc1, gloc2, tm);
     genmo->Add(*unit1);
     delete unit1;


    ////////////////moving unit /////////////////////////////////
    et.ReadFrom(st.ToDouble() + time/(24.0*60.0*60.0));

    ////////////////////create a upoint////////////////////////
    up_interval.start = st;
    up_interval.lc = true;
    up_interval.end = et;
    up_interval.rc = false; 
    UPoint* up2 = new UPoint(up_interval,from_loc,to_loc);
  //    cout<<*up<<endl; 
    mo->Add(*up2);
    delete up2; 
    st = et;
    ////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    /////////////generic location unit //////////////////////////
    ////////////////////////////////////////////////////////////
    
     Loc loc3(i + 1 + 1, 0.0);
     Loc loc4(i + 1, 0.0);
     GenLoc gloc3(mr_oid, loc3);
     GenLoc gloc4(mr_oid, loc4);
     tm = GetTM("Metro"); 
     UGenLoc* unit2 = new UGenLoc(up_interval, gloc3, gloc4, tm);
     genmo->Add(*unit2);
     delete unit2;


    if(i == 0){//////the last stop 

      et.ReadFrom(st.ToDouble() + 30.0/(24.0*60.0*60.0));//30 seconds

      ////////////////////create a upoint////////////////////////
      up_interval.start = st;
      up_interval.lc = true;
      up_interval.end = et;
      up_interval.rc = false; 
      UPoint* up = new UPoint(up_interval, to_loc, to_loc);
  //    cout<<*up<<endl; 
      mo->Add(*up);
      delete up; 
      st = et; 
      //////////////////////////////////////////////////////////////
      /////////////generic location unit //////////////////////////
      ////////////////////////////////////////////////////////////

       Loc loc5(i + 1, 0.0);
       Loc loc6(i + 1, 0.0);
       GenLoc gloc5(mr_oid, loc5);
       GenLoc gloc6(mr_oid, loc6);
       tm = GetTM("Metro");
       UGenLoc* unit3 = new UGenLoc(up_interval, gloc5, gloc6, tm);
       genmo->Add(*unit3);
       delete unit3;

    }

  }

  mo->EndBulkLoad();
  genmo->EndBulkLoad();

  mtrip_list1.push_back(*genmo);
  mtrip_list2.push_back(*mo);
  id_list.push_back(mr_id);
  mr_oid_list.push_back(mr_oid);//unique object id for a metro route 

  dir_list.push_back(dir);

  CopyMetroTrip(genmo, mo, peri, start_time, mr_id, mr_oid, dir);

  delete mo;
  delete genmo;

}

/*
copy the metro to have more schedules 

*/
void MetroStruct::CopyMetroTrip(GenMO* genmo, MPoint* mo, 
                                Periods* peri, Instant s_time, 
                                int mr_id, int mr_oid, bool dir)
{

  Interval<Instant> metro_periods;
  peri->Get(0, metro_periods);
  
  double schedule = 10.0/(24.0*60.0);
  Instant new_start = s_time;
  /////////////////10 minutes for each metro ////////////////
  new_start.ReadFrom(new_start.ToDouble() + schedule);

  int sched_id = 1;
  while(new_start < metro_periods.end){

//    cout<<"start time "<<new_start<<endl; 

    MPoint* new_mo = new MPoint(0);
    GenMO* new_genmo = new GenMO(0);
    new_mo->StartBulkLoad();
    new_genmo->StartBulkLoad();


    for(int i = 0;i < mo->GetNoComponents();i++){
        UPoint up;
        mo->Get(i, up);
        Instant st = up.timeInterval.start;
        st.ReadFrom(st.ToDouble() + schedule*sched_id );
        Instant et = up.timeInterval.end;
        et.ReadFrom(et.ToDouble() + schedule*sched_id);
        up.timeInterval.start = st;
        up.timeInterval.end = et; 
        new_mo->Add(up);

        /////////////////////////////////////////////////
        //////////genmo units  //////////////////////////
        /////////////////////////////////////////////////
        UGenLoc ugloc;
        genmo->Get(i, ugloc);
//        cout<<ugloc<<endl;

        UGenLoc* unit_new = new UGenLoc(up.timeInterval, ugloc.gloc1, 
                                ugloc.gloc2, ugloc.tm);
        new_genmo->Add(*unit_new);
        delete unit_new;

    }

    new_mo->EndBulkLoad();
    new_genmo->EndBulkLoad();


    mtrip_list1.push_back(*new_genmo);
    mtrip_list2.push_back(*new_mo);
    id_list.push_back(mr_id);
    mr_oid_list.push_back(mr_oid);//unique object id for a metro route 

    dir_list.push_back(dir);

    delete new_mo;
    delete new_genmo;

    new_start.ReadFrom(new_start.ToDouble() + schedule);
    sched_id++;

  }
}

/*
copy the metro trips on Sunday 

*/
void MetroStruct::CopyTripOneDayBefore()
{
  int start = 0;
  int end = mtrip_list1.size();

  for(;start < end; start++){

      GenMO* genmo = &mtrip_list1[start];
      MPoint* mo = &mtrip_list2[start];

      int mr_id = id_list[start];
      bool dir = dir_list[start];
      int mr_oid = mr_oid_list[start];


      MPoint* new_mo = new MPoint(0);
      GenMO* new_genmo = new GenMO(0);
      new_mo->StartBulkLoad();
      new_genmo->StartBulkLoad();
      double schedule = 1.0;///////// one day before

      for(int i = 0;i < mo->GetNoComponents();i++){
        UPoint up;
        mo->Get(i, up);
        Instant st = up.timeInterval.start;
        st.ReadFrom(st.ToDouble() - schedule);
        Instant et = up.timeInterval.end;
        et.ReadFrom(et.ToDouble() - schedule);
        up.timeInterval.start = st;
        up.timeInterval.end = et; 
        new_mo->Add(up);

        /////////////////////////////////////////////////
        //////////genmo units  //////////////////////////
        /////////////////////////////////////////////////
        UGenLoc ugloc;
        genmo->Get(i, ugloc);
//        cout<<ugloc<<endl;

        UGenLoc* unit_new = new UGenLoc(up.timeInterval, ugloc.gloc1, 
                                ugloc.gloc2, ugloc.tm);
        new_genmo->Add(*unit_new);
        delete unit_new;

      }

      new_mo->EndBulkLoad();
      new_genmo->EndBulkLoad();

      id_list.push_back(mr_id);
      dir_list.push_back(dir);
      mr_oid_list.push_back(mr_oid);

      mtrip_list1.push_back(*new_genmo);
      mtrip_list2.push_back(*new_mo);

      delete new_mo;
      delete new_genmo;
  
  }


}




/*
create one kind of edges for metro graph where two metro stops have the same
spatial location

*/
void MetroStruct::MsNeighbors1(Relation* r)
{
//   cout<<"MsNeighbors1"<<endl;
   vector<UBahn_Stop> ub_stop_list;
   for(int i = 1;i <= r->GetNoTuples();i++){
      Tuple* ub_tuple = r->GetTuple(i, false);

      int line_id = 
          ((CcInt*)ub_tuple->GetAttribute(MetroNetwork::M_R_ID))->GetIntval();

      Bus_Stop* mr_stop = 
          (Bus_Stop*)ub_tuple->GetAttribute(MetroNetwork::M_STOP);
      Point* loc = 
          (Point*)ub_tuple->GetAttribute(MetroNetwork::M_STOP_GEO);
      int stop_id = mr_stop->GetStopId();
      bool dir = mr_stop->GetUp();

      UBahn_Stop ub_stop(line_id, *loc, stop_id, dir, ub_tuple->GetTupleId());
      ub_stop_list.push_back(ub_stop);
      ub_tuple->DeleteIfAllowed();
   }

//   cout<<ub_stop_list.size()<<endl;
//  sort(ub_stop_list.begin(), ub_stop_list.end());

  const double delta_dist = 0.01;
  for(unsigned int i = 0; i < ub_stop_list.size();i++){

    for(unsigned int j = 0; j < ub_stop_list.size();j++){
      if(i != j){
        if(ub_stop_list[i].loc.Distance(ub_stop_list[j].loc) < delta_dist){
            ms_tid_list1.push_back(ub_stop_list[i].tid);
            ms_tid_list2.push_back(ub_stop_list[j].tid);
        }
      }
    }
  }

}

/*
create edges for metro graph where two metro stops are connected by moving
metros

*/
void MetroStruct::MsNeighbors2(MetroNetwork* mn, Relation* timetable, 
                               BTree* btree1,
                   Relation* metrotrip, BTree* btree2)
{
//  cout<<"MsNeighbors2 "<<endl;
  Relation* ms_rel = mn->GetMS_Rel();
  for(int i = 1;i <= ms_rel->GetNoTuples();i++){
    Tuple* ms_tuple = ms_rel->GetTuple(i, false);

/*    int line_id = ((CcInt*)ms_tuple->GetAttribute(T_LINEID_EXT))->GetIntval();
    Point *loc = (Point*)ms_tuple->GetAttribute(T_STOP_LOC_EXT);
    int stop_id = ((CcInt*)ms_tuple->GetAttribute(T_STOP_ID_EXT))->GetIntval();
    bool dir = ((CcBool*)ms_tuple->GetAttribute(T_DIRECTION))->GetBoolval();*/

   int line_id = 
    ((CcInt*)ms_tuple->GetAttribute(MetroNetwork::M_R_ID))->GetIntval();
   Bus_Stop* ms = (Bus_Stop*)ms_tuple->GetAttribute(MetroNetwork::M_STOP);
   Point *loc = (Point*)ms_tuple->GetAttribute(MetroNetwork::M_STOP_GEO);
   int stop_id = ms->GetStopId();
   bool dir = ms->GetUp();


    UBahn_Stop ms_stop(line_id, *loc, stop_id, dir, ms_tuple->GetTupleId());

    int neighbor_tid = mn->GetMS_Stop_Neighbor(&ms_stop);

    if(neighbor_tid > 0){
//      ms_stop.Print();
      Tuple* ms_tuple2 = ms_rel->GetTuple(neighbor_tid, false);
//      Point *loc2 = (Point*)ms_tuple2->GetAttribute(T_STOP_LOC_EXT);
      Point *loc2 = (Point*)ms_tuple2->GetAttribute(MetroNetwork::M_STOP_GEO);

      ConnectionOneRoute(&ms_stop, timetable, btree1, 
                         metrotrip, btree2, neighbor_tid, loc2);

      ms_tuple2->DeleteIfAllowed();

      ms_tid_list1.push_back(ms_tuple->GetTupleId());
      ms_tid_list2.push_back(neighbor_tid);
    }

    ms_tuple->DeleteIfAllowed();

/*if(neighbor_tid > 0){
      Tuple* ms_tuple2 = ms_rel->GetTuple(neighbor_tid, false);
    int line_id2 = ((CcInt*)ms_tuple2->GetAttribute(T_LINEID_EXT))->GetIntval();
      Point *loc2 = (Point*)ms_tuple2->GetAttribute(T_STOP_LOC_EXT);
   int stop_id2 = ((CcInt*)ms_tuple2->GetAttribute(T_STOP_ID_EXT))->GetIntval();
      bool dir2 = ((CcBool*)ms_tuple2->GetAttribute(T_DIRECTION))->GetBoolval();
  UBahn_Stop ms_stop2(line_id2, *loc2, stop_id2, dir2, ms_tuple2->GetTupleId());

      ms_tuple2->DeleteIfAllowed();
      ms_stop2.Print();
      cout<<endl;
    }else
      cout<<"no neighbor "<<endl; */

  }

}


/*
build the connection for a metro stop in one route, to its next stop

*/
void MetroStruct::ConnectionOneRoute(UBahn_Stop* ms_stop, Relation* timetable,
                         BTree* btree1, Relation* metrotrip, 
                                 BTree* btree2, 
                                 int Neighbor_tid, Point* Neighbor_loc)
{
    ///////////////////////////////////////////////////////////////
    //////////////////find the periods and schedule ///////////////
    ///////////////////////////////////////////////////////////////
    CcInt* search_cell_id1 = new CcInt(true, ms_stop->line_id);
    BTreeIterator* btree_iter1 = btree1->ExactMatch(search_cell_id1);
    while(btree_iter1->Next()){
        Tuple* tuple = timetable->GetTuple(btree_iter1->GetId(), false);
        int l_id = 
          ((CcInt*)tuple->GetAttribute(UBTrain::UB_LINE_ID_T))->GetIntval();
        assert(l_id == ms_stop->line_id);
        int s_id = 
          ((CcInt*)tuple->GetAttribute(UBTrain::UB_STOP_ID_T))->GetIntval();
        bool dir = 
          ((CcBool*)tuple->GetAttribute(UBTrain::UB_DIR_T))->GetBoolval();
        if(s_id == ms_stop->stop_id && dir == ms_stop->d){
          Periods* peri = (Periods*)tuple->GetAttribute(UBTrain::UB_PERIODS_T);
          double sche_int = 
           ((CcReal*)tuple->GetAttribute(UBTrain::UB_INTERVAL_T))->GetRealval();
//          cout<<"periods "<<*peri<<" interval "<<sche_int*86400.0<<endl; 

          period_list.push_back(*peri);
          schedule_interval.push_back(sche_int);
          break;
        }

        tuple->DeleteIfAllowed();
    }
    delete btree_iter1;
    delete search_cell_id1;

    ////////////////////////////////////////////////////////////////
    //////////////////find the trip connecting two metro stops /////
    ////////////////////////////////////////////////////////////////
    CcInt* search_cell_id2 = new CcInt(true, ms_stop->line_id);
    BTreeIterator* btree_iter2 = btree2->ExactMatch(search_cell_id2);
    bool found = false;
    while(btree_iter2->Next()){
      Tuple* tuple = metrotrip->GetTuple(btree_iter2->GetId(), false);
/*      int l_id = 
        ((CcInt*)tuple->GetAttribute(MetroNetwork::UB_M_LINE_ID))->GetIntval();
      assert(l_id == ms_stop->line_id);
      bool dir = 
   ((CcBool*)tuple->GetAttribute(MetroNetwork::UB_M_LINE_DIR))->GetBoolval();*/

      int l_id = ((CcInt*)tuple->GetAttribute(M_R_ID_COM))->GetIntval();
      assert(l_id == ms_stop->line_id);
      bool dir = ((CcBool*)tuple->GetAttribute(M_DIR_COM))->GetBoolval();


//      cout<<"l_id "<<l_id<<" dir "<<dir<<endl;
      if(dir == ms_stop->d){
        const double delta_dist = 0.01;
//        MPoint* mo = (MPoint*)tuple->GetAttribute(MetroNetwork::UB_TRIP2_MO);
        MPoint* mo = (MPoint*)tuple->GetAttribute(M_MP_COM);
        ///////////////////////////////////////////////////////////
        ///////////////find the stop units////////////////////////
        /////////////////////////////////////////////////////////
        int j = 0;
        for(;j < mo->GetNoComponents();j++){
          UPoint up;
          mo->Get(j, up);
          Point lp = up.p0;
          Point rp = up.p1;
          if(ms_stop->loc.Distance(lp) < delta_dist && 
             ms_stop->loc.Distance(rp) < delta_dist){
//              cout<<"find the stop "<<endl;
              break;
          }
        }
        assert(j < mo->GetNoComponents());
        if(j == mo->GetNoComponents() - 1){//last stop, need not process 

        }else{
          MPoint sub_move(0);
          sub_move.StartBulkLoad();
          double time_move = 0.0;
          for(j++;j < mo->GetNoComponents();j++){//start from next unit
            UPoint up;
            mo->Get(j, up);
            Point lp = up.p0;
            Point rp = up.p1;
            if(lp.Distance(rp) < delta_dist && 
               Neighbor_loc->Distance(lp) < delta_dist){ //next bus stop
              break;
            }else{
              sub_move.Add(up);
              time_move += up.timeInterval.end.ToDouble() - 
                           up.timeInterval.start.ToDouble();
            }
          }
          sub_move.EndBulkLoad();
//          cout<<" time cost  "<<time_move*86400.0<<endl;
          Line traj(0);
          sub_move.Trajectory(traj);
          SimpleLine sl_traj(0);
          sl_traj.fromLine(traj);

          geodata.push_back(sl_traj);
          time_cost_list.push_back(time_move);

        }

        found = true;
      }
      tuple->DeleteIfAllowed();

      if(found)break;
    }
    delete btree_iter2;
    delete search_cell_id2;

    assert(found);

}


/*
map a metro stop to a pavement area 

*/
void MetroStruct::MapMSToPave(Relation* rel1, Relation* rel2, 
                              R_Tree<2,TupleId>* rtree)
{
  Rectangle<2> bbox = rtree->BoundingBox();
 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* ms_tuple = rel1->GetTuple(i, false);
    Point* ms_loc = (Point*)ms_tuple->GetAttribute(MetroNetwork::M_STOP_GEO);
    vector<int> tri_tid_list;
    double dist = bbox.MaxD(0) - bbox.MinD(0); 
    DFTraverse(rtree, rel2, rtree->RootRecordId(), ms_loc, tri_tid_list, dist);
//    cout<<"min dist "<<dist<<" size "<<tri_tid_list.size()<<endl; 

    Tuple* pave_tuple = 
        rel2->GetTuple(tri_tid_list[tri_tid_list.size() - 1], false);
    int tri_oid = 
        ((CcInt*)pave_tuple->GetAttribute(DualGraph::OID))->GetIntval();
    Region* reg = (Region*)pave_tuple->GetAttribute(DualGraph::PAVEMENT);
    ///////////////////////////////////////////////////////////////////
    /////////////get the closet point on the region to the ms loc/////
    ///////////////////////////////////////////////////////////////////
    SpacePartition* sp = new SpacePartition();

    Point ms_loc1 = *ms_loc;
    Point ms_loc2;
    for(int j = 0;j < reg->Size();j++){
      HalfSegment hs;
      reg->Get(j, hs);
      if(!hs.IsLeftDomPoint()) continue;
      Point cp;
      sp->GetClosestPoint(hs, ms_loc1, cp);
      if(j == 0){
        ms_loc2 = cp;
      }else{
        if(ms_loc1.Distance(cp) < ms_loc1.Distance(ms_loc2)){
          ms_loc2 = cp;
        }
      }
    }

    delete sp;

    Rectangle<2> reg_box = reg->BoundingBox();
    Loc loc(ms_loc2.GetX() - reg_box.MinD(0), 
            ms_loc2.GetY() - reg_box.MinD(1));
    GenLoc gloc(tri_oid, loc);
    loc_list1.push_back(gloc);/////////generic location in the pavement 

//    id_list.push_back(tri_oid);
//    neighbor_list.push_back(*reg);
//    loc_list2.push_back(*ms_loc);

    Bus_Stop* ms_stop = (Bus_Stop*)ms_tuple->GetAttribute(MetroNetwork::M_STOP);
    Point* ms_stop_loc = 
        (Point*)ms_tuple->GetAttribute(MetroNetwork::M_STOP_GEO);

    stop_geo_list.push_back(*ms_stop_loc);
    mstop_list.push_back(*ms_stop);
    loc_list2.push_back(ms_loc2);

    pave_tuple->DeleteIfAllowed();
    ms_tuple->DeleteIfAllowed();
  }

}

/*
traverse the rtree built on pavement areas and find the clost point to the 
metro stop 

*/
void MetroStruct::DFTraverse(R_Tree<2,TupleId>* rtree, Relation* rel,
                             SmiRecordId adr, Point* loc, 
                             vector<int>& tri_tid_list, double& min_dist)
{
  
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));

  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = rel->GetTuple(e.info, false);
              Region* reg = 
                    (Region*)dg_tuple->GetAttribute(DualGraph::PAVEMENT);

              double d = reg->Distance(*loc);
              if(d < min_dist){
                  tri_tid_list.push_back(e.info);
                  min_dist = d;
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc->Distance(e.box) < min_dist){
                DFTraverse(rtree, rel, e.pointer, loc, tri_tid_list, min_dist);
            }
      }
  }
  delete node;
}
///////////////////////////////////////////////////////////////
//////////////////// metro network/////////////////////////////
///////////////////////////////////////////////////////////////

/////////////// used for the real data (ubahn, converting)///////////////////

string MetroNetwork::UBAHNStopsTypeInfo =
"(rel (tuple ((LineId int) (loc point) (stop_id int) (Up bool))))";

string MetroNetwork::UBAHNStopsBTreeTypeInfo =
"(btree (tuple ((LineId int) (loc point) (stop_id int) (Up bool))) int)";

string MetroNetwork::UBAHNStopsRTreeTypeInfo = "(rtree (tuple ((LineId int)\
(loc point) (stop_id int) (Up bool))) point FALSE)";

string MetroNetwork::UBAHNRoutesTypeInfo = "(rel (tuple ((lineid int)\
(oid int) (geoData sline))))";

string MetroNetwork::UBAHNRotuesBTreeTypeInfo = "(rel (tuple ((lineid int)\
(oid int) (geoData sline))) int)";


string MetroNetwork::MetroStopsTypeInfo =
"(rel (tuple ((ms_stop busstop) (stop_geodata point) (mr_id int))))";

string MetroNetwork::MetroStopsBTreeTypeInfo =
"(btree (tuple ((ms_stop busstop) (stop_geodata point) (mr_id int))) int)";

string MetroNetwork::MetroStopsRTreeTypeInfo = 
"(rtree (tuple ((ms_stop busstop)(stop_geodata point)\
(mr_id int))) point FALSE)";

string MetroNetwork::MetroRoutesTypeInfo = "(rel (tuple ((mr_id int) \
(mroute busroute) (oid int))))";

string MetroNetwork::MetroRoutesBTreTypeInfo = "(rel (tuple ((mr_id int)\
(mroute busroute) (oid int))) int)";

string MetroNetwork::MetroTripTypeInfo = "(rel (tuple ((mtrip1 genmo)\
(mtrip2 mpoint) (mr_oid int) (oid int))))";

string MetroNetwork::MetroTypeBTreeTypeInfo = "(btree (tuple ((mtrip1 genmo)\
(mtrip2 mpoint) (mr_id int) (oid int))) int)";

string MetroNetwork::MetroPaveTypeInfo =
"(rel (tuple ((loc1 genloc) (loc2 point) (ms_stop busstop)\
(ms_stop_loc point))))";

ListExpr MetroNetworkProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("metronetwork"),
         nl->StringAtom("((def, id))"),
           nl->StringAtom("((TRUE 1))"))));
}

/*
In function. there is no nested list expression here.

*/
Word InMetroNetwork( const ListExpr typeInfo, const ListExpr instance,
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
      cout<< "metronetwork(): metro network id must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int id = nl->IntValue(second);

    MetroNetwork* mn = new MetroNetwork(d, id); 

   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    return SetWord(mn);
  }

  correct = false;
  return SetWord(Address(0));
}

/*
output the metro network 

*/
ListExpr OutMetroNetwork( ListExpr typeInfo, Word value )
{
//  cout<<"OutMetroNetwork"<<endl; 
  MetroNetwork* mn = (MetroNetwork*)(value.addr);
  if(!mn->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  ListExpr list1 = nl->TwoElemList(
               nl->StringAtom("Metro Network Id:"), 
               nl->IntAtom(mn->GetId()));

//  return nl->OneElemList(list1);

  ////////////////out put bus stops relation////////////////////////////////
  ListExpr bs_list = nl->TheEmptyList();
  Relation* bs_rel = mn->GetMS_Rel();
  
  if(bs_rel != NULL){
      bool bFirst = true;
      ListExpr xNext = nl->TheEmptyList();
      ListExpr xLast = nl->TheEmptyList();
      for(int i = 1;i <= bs_rel->GetNoTuples();i++){
        Tuple* node_tuple = bs_rel->GetTuple(i, false);
        int line_id = 
       ((CcInt*)node_tuple->GetAttribute(MetroNetwork::M_R_ID))->GetIntval();
        Bus_Stop* ms = 
          (Bus_Stop*)node_tuple->GetAttribute(MetroNetwork::M_STOP);
        int stop_id = ms->GetStopId();
        bool direction = ms->GetUp();

        Point* loc = (Point*)node_tuple->GetAttribute(MetroNetwork::M_STOP_GEO);

        ListExpr stop_list = nl->FourElemList(
           nl->IntAtom(line_id), nl->IntAtom(stop_id), 
           nl-> BoolAtom(direction), 
           OutPoint( nl->TheEmptyList(), SetWord(loc) ));

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


 //////////////////////output metro routes relation///////////////////////////
  ListExpr br_list = nl->TheEmptyList();
  Relation* br_rel = mn->GetMR_Rel();

  if(br_rel != NULL){
      bool bFirst = true;
      ListExpr xNext = nl->TheEmptyList();
      ListExpr xLast = nl->TheEmptyList();
      for(int i = 1;i <= br_rel->GetNoTuples();i++){
        Tuple* node_tuple = br_rel->GetTuple(i, false);

        int lineid = 
      ((CcInt*)node_tuple->GetAttribute(MetroNetwork::M_ROUTE_ID))->GetIntval();
        Bus_Route* mr = 
          (Bus_Route*)node_tuple->GetAttribute(MetroNetwork::M_ROUTE);
        SimpleLine mr_sl(0);
        mr->GetGeoData(mr_sl);

        ListExpr route_list = nl->TwoElemList(
           nl->IntAtom(lineid), 
           OutSimpleLine( nl->TheEmptyList(), SetWord(&mr_sl)));

        xNext  = route_list;
        if(bFirst){
          br_list = nl->OneElemList(xNext);
          xLast = br_list;
          bFirst = false;
        }else
            xLast = nl->Append(xLast,xNext);
        node_tuple->DeleteIfAllowed();
      }
  }
  ////////////////////////////////////////////////////////////////////////
  ////////////////no output bus trips relation: too much data///////////////
  //////////////////////////////////////////////////////////////////////////
  return nl->ThreeElemList(list1, bs_list, br_list);

}

bool SaveMetroNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  MetroNetwork* mn = (MetroNetwork*)value.addr;
  return mn->Save(valueRecord, offset, typeInfo);
}

bool OpenMetroNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  value.addr = MetroNetwork::Open(valueRecord, offset, typeInfo);
  return value.addr != NULL; 
}

MetroNetwork* MetroNetwork::Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo)
{
  return new MetroNetwork(valueRecord, offset, typeInfo); 

}


Word CreateMetroNetwork(const ListExpr typeInfo)
{
// cout<<"CreateMetroNetwork()"<<endl;
  return SetWord (new MetroNetwork());
}


void DeleteMetroNetwork(const ListExpr typeInfo, Word& w)
{
// cout<<"DeleteMetroNetwork()"<<endl;
  MetroNetwork* mn = (MetroNetwork*)w.addr;
  delete mn;
   w.addr = NULL;
}


void CloseMetroNetwork( const ListExpr typeInfo, Word& w )
{
//  cout<<"CloseMetroNetwork"<<endl; 
  delete static_cast<MetroNetwork*>(w.addr); 
  w.addr = 0;
}

Word CloneMetroNetwork( const ListExpr typeInfo, const Word& w )
{
//  cout<<"CloneMetroNetwork"<<endl; 
  return SetWord( new Address(0));
}

void* MetroNetwork::Cast(void* addr)
{
  return NULL;
}

int SizeOfMetroNetwork()
{
//  cout<<"SizeOfMetroNetwork"<<endl; 
  return sizeof(MetroNetwork);
}

bool CheckMetroNetwork( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckMetroNetwork"<<endl; 
  return (nl->IsEqual( type, "metronetwork" ));
}


MetroNetwork::MetroNetwork():
def(false), mn_id(0), graph_init(false), graph_id(0), max_metro_speed(0),
min_mr_oid(0), min_mt_oid(0), 
stops_rel(NULL), btree_ms(NULL), rtree_ms(NULL),
routes_rel(NULL), btree_mr(NULL), btree_mr_uoid(NULL),
metrotrips_rel(NULL), btree_trip_br_id(NULL), btree_trip_oid(NULL)
{


}

MetroNetwork::MetroNetwork(bool d, unsigned int i): def(d), mn_id(i), 
graph_init(false), graph_id(0), max_metro_speed(0),
min_mr_oid(0), min_mt_oid(0),
stops_rel(NULL), btree_ms(NULL), rtree_ms(NULL),
routes_rel(NULL), btree_mr(NULL), btree_mr_uoid(NULL),
metrotrips_rel(NULL), btree_trip_br_id(NULL), btree_trip_oid(NULL)
{


}

/*
read the data from record 

*/
MetroNetwork::MetroNetwork(SmiRecord& valueRecord, size_t& offset, 
                       const ListExpr typeInfo):
def(false), mn_id(0), graph_init(false), graph_id(0), max_metro_speed(0),
min_mr_oid(0), min_mt_oid(0),
stops_rel(NULL), btree_ms(NULL), rtree_ms(NULL),
routes_rel(NULL), btree_mr(NULL), btree_mr_uoid(NULL),
metrotrips_rel(NULL), btree_trip_br_id(NULL), btree_trip_oid(NULL)
{
  valueRecord.Read(&def, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&mn_id, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  valueRecord.Read(&graph_init, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&graph_id, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  valueRecord.Read(&max_metro_speed, sizeof(double), offset);
  offset += sizeof(double);

  valueRecord.Read(&min_mr_oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  valueRecord.Read(&min_mt_oid, sizeof(unsigned int), offset);
  offset += sizeof(unsigned int);

  ListExpr xType;
  ListExpr xNumericType;
  ////////////////Open relation for metro stops///////////////////////
  nl->ReadFromString(MetroStopsTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  stops_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!stops_rel) {
    return;
  }

  ///////////////////btree on metro stops on brid///////////////////////////////
  nl->ReadFromString(MetroStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_ms = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_ms) {
    stops_rel->Delete(); 
   return;
  }

  ///////////////////rtree on metro stops //////////////////////////////
  Word xValue;
  if(!(rtree_ms->Open(valueRecord,offset, MetroStopsRTreeTypeInfo,xValue))){
    stops_rel->Delete(); 
    delete btree_ms;
    return;
  }

  rtree_ms = ( R_Tree<2,TupleId>* ) xValue.addr;

 ///////////////////////////////Open relation for metroroutes///////////////
  nl->ReadFromString(MetroRoutesTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  routes_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!routes_rel) {
    stops_rel->Delete();
    delete btree_ms;
    delete rtree_ms;
    return;
  }

  ///////////////////btree on metro routes//////////////////////////////////
  nl->ReadFromString(MetroRoutesBTreTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_mr = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_mr) {
    stops_rel->Delete();
    delete btree_ms;
    delete rtree_ms;
    routes_rel->Delete();
    return;
  }

   ///////////////////btree on metro routes unique oid///////////////////////
  nl->ReadFromString(MetroRoutesBTreTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_mr_uoid = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_mr_uoid) {
    stops_rel->Delete(); 
    delete btree_ms;
    routes_rel->Delete();
    delete btree_mr;
    delete rtree_ms;
    return;
  }

  ///////////////open relation storing metro trips//////////////////////
  nl->ReadFromString(MetroTripTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  metrotrips_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!metrotrips_rel) {
    stops_rel->Delete(); 
    delete btree_ms;
    routes_rel->Delete();
    delete btree_mr;
    delete rtree_ms;
    delete btree_mr_uoid;
    return;
  }

  ///////////////////btree on metro trips metro route id///////////////////////
  nl->ReadFromString(MetroTypeBTreeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_trip_br_id = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_trip_br_id) {
    stops_rel->Delete(); 
    delete btree_ms;
    routes_rel->Delete();
    delete btree_mr;
    delete rtree_ms; 
    delete btree_mr_uoid;
    metrotrips_rel->Delete();
    return;
  }
  
  ///////////////////btree on metro trips unique id////////////////////////////
  nl->ReadFromString(MetroTypeBTreeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_trip_oid = BTree::Open(valueRecord, offset, xNumericType);
  if(!btree_trip_oid) {
    stops_rel->Delete();
    delete btree_ms;
    routes_rel->Delete();
    delete btree_mr;
    delete rtree_ms;
    delete btree_mr_uoid;
    metrotrips_rel->Delete();
    delete btree_trip_br_id;
    return;
  }

}

MetroNetwork::~MetroNetwork()
{
  if(stops_rel != NULL) stops_rel->Close();
  if(btree_ms != NULL) delete btree_ms;
  if(rtree_ms != NULL) delete rtree_ms; 
  if(routes_rel != NULL) routes_rel->Close();
  if(btree_mr != NULL) delete btree_mr;
  if(btree_mr_uoid != NULL) delete btree_mr_uoid;
  if(metrotrips_rel != NULL) metrotrips_rel->Close();
  if(btree_trip_br_id != NULL) delete btree_trip_br_id;
  if(btree_trip_oid != NULL) delete btree_trip_oid;
}

bool MetroNetwork::Save(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo)
{
  
//  cout<<"MetroNetwork::Save"<<endl; 

  valueRecord.Write(&def, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&mn_id, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  valueRecord.Write(&graph_init, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&graph_id, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  valueRecord.Write(&max_metro_speed, sizeof(double), offset);
  offset += sizeof(double); 

  valueRecord.Write(&min_mr_oid, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  valueRecord.Write(&min_mt_oid, sizeof(unsigned int), offset); 
  offset += sizeof(unsigned int); 

  ListExpr xType;
  ListExpr xNumericType;

  ////////////////////metro stops relation/////////////////////////////
  nl->ReadFromString(MetroStopsTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!stops_rel->Save(valueRecord,offset,xNumericType))
      return false;

  
  /////////////////////btree on metro stops on lineid/////////////////////////
  nl->ReadFromString(MetroStopsBTreeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_ms->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////rtree on metro stops ///////////////////////
  if(!rtree_ms->Save(valueRecord, offset)){
    return false;
  }
  
   ///////////////////metro routes relation///////////////////////////
  nl->ReadFromString(MetroRoutesTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!routes_rel->Save(valueRecord,offset,xNumericType))
      return false;
  
  ///////////////////////btree on metro routes////////////////////////////
  nl->ReadFromString(MetroRoutesBTreTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_mr->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////btree on bus routes on unique id///////////////////
  nl->ReadFromString(MetroRoutesBTreTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_mr_uoid->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////metro trips relation/////////////////////////////
  nl->ReadFromString(MetroTripTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!metrotrips_rel->Save(valueRecord,offset,xNumericType))
      return false;

  //////////////////btree on metro trips on metro route id/////////////////
  nl->ReadFromString(MetroTypeBTreeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_trip_br_id->Save(valueRecord,offset,xNumericType))
      return false;

  ///////////////////////btree on metro trips on unique id///////////////////
  nl->ReadFromString(MetroTypeBTreeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_trip_oid->Save(valueRecord,offset,xNumericType))
      return false;

  return true; 
}

/*
load stops, routes, and moving metros relation 

*/
void MetroNetwork::Load(unsigned int i, Relation* r1, Relation* r2, 
                        Relation* r3)
{

  if(i < 1){
    def = false;
    return;
  }
  mn_id = i; 

  LoadStops(r1);  /////to get 2D points in space  
  LoadRoutes(r2); //first load bus routes because bus stops access bus routes
  LoadMetros(r3); //load moving metros 

  def = true; 
}

/*
load metro stops relation 

*/
void MetroNetwork::LoadStops(Relation* r)
{
//  cout<<"LoadStops"<<endl;
  
  ListExpr ptrList1 = listutils::getPtrList(r);
  
  string strQuery = "(consume(feed(" + MetroStopsTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  stops_rel = (Relation*)xResult.addr; 


  //////////////////////////////////////////////////////////////////
  //////////////////////btree on metro stops line id/////////////////////
  ///////////////////////////////////////////////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(stops_rel);

  strQuery = "(createbtree (" + MetroStopsTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "mr_id)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_ms = (BTree*)xResult.addr;


  //////////////////////rtree on metro stops//////////////////////////
  
  ListExpr ptrList3 = listutils::getPtrList(stops_rel);
  
  strQuery = "(bulkloadrtree(sortby(addid(feed (" + MetroStopsTypeInfo +
         " (ptr " + nl->ToString(ptrList3) + 
         "))))((stop_geodata asc))) stop_geodata)";

  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  rtree_ms = ( R_Tree<2,TupleId>* ) xResult.addr;

}

/*
load metro routes relation 

*/
void MetroNetwork::LoadRoutes(Relation* r2)
{
//    cout<<"LoadRotes"<<endl; 

    ListExpr ptrList1 = listutils::getPtrList(r2);

    string strQuery = "(consume(feed(" + MetroRoutesTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

//  cout<<strQuery<<endl; 

    Word xResult;
    int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
    assert(QueryExecuted);
    routes_rel = (Relation*)xResult.addr; 

  ///////////////////get the minimum metro route oid////////////////////////
  int temp_id = numeric_limits<int>::max(); 
  for(int i = 1;i <= routes_rel->GetNoTuples();i++){
    Tuple* mr_tuple = routes_rel->GetTuple(i, false);
    int id = ((CcInt*)mr_tuple->GetAttribute(M_R_OID))->GetIntval();
    if(id < temp_id) temp_id = id;
    mr_tuple->DeleteIfAllowed(); 
  }
  min_mr_oid = temp_id;
  
//  cout<<"min mr oid "<<min_mr_oid<<endl; 
  
   //////////////////////////////////////////////////////////////////
  //////////////////////btree on metro routes lineid/////////////////
  //////////////////////////////////////////////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(routes_rel);
  
  strQuery = "(createbtree (" + MetroRoutesTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "mr_id)";
//  cout<<strQuery<<endl; 
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_mr = (BTree*)xResult.addr;

  //////////////////////////////////////////////////////////////////
  //////////////////////btree on metro routes unique oid////////////
  //////////////////////////////////////////////////////////////////

  ListExpr ptrList3 = listutils::getPtrList(routes_rel);
  
  strQuery = "(createbtree (" + MetroRoutesTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "oid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_mr_uoid = (BTree*)xResult.addr;

}

/*
load moving metros 

*/
void MetroNetwork::LoadMetros(Relation* r3)
{
//  cout<<"LoadMetros"<<endl;

  ListExpr ptrList1 = listutils::getPtrList(r3);

  string strQuery = "(consume(feed(" + MetroTripTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

//  cout<<strQuery<<endl;

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  metrotrips_rel = (Relation*)xResult.addr;

//  cout<<"metro trips size "<<metrotrips_rel->GetNoTuples()<<endl; 

 //////////////////////////////////////////////////////////////////////
 //////////////////////set maximum metro speed///////////////////////////
 /////////////////////////////////////////////////////////////////////
 int temp_id = numeric_limits<int>::max();
 for(int i = 1;i <= metrotrips_rel->GetNoTuples();i++){
  Tuple* metro_tuple = metrotrips_rel->GetTuple(i, false);
  MPoint* mo = (MPoint*)metro_tuple->GetAttribute(M_TRIP_MP);

  for( int j = 0; j < mo->GetNoComponents(); j++ ){
      UPoint unit;
      mo->Get( j, unit );
      Point pos1 = unit.p0;
      Point pos2 = unit.p1;
      if(pos1.Distance(pos2) > 1.0){
          double t = 
              unit.timeInterval.end.ToDouble()*86400.0 -
              unit.timeInterval.start.ToDouble()*86400.0;
          double speed = pos1.Distance(pos2)/t; 
/*          cout<<"dist "<<fabs(pos1 - pos2)<<" t "<<t
               <<" speed "<<speed<<endl;*/
          if(speed > max_metro_speed){
            max_metro_speed = speed; 
          }
      }
  }
    int id = ((CcInt*)metro_tuple->GetAttribute(M_TRIP_OID))->GetIntval();
    if(id < temp_id) temp_id = id;

    metro_tuple->DeleteIfAllowed();
 }



  min_mt_oid = temp_id;
//  cout<<"max metro speed "<<max_metro_speed*60*60/1000.0<<"km/h "<<endl;
//  cout<<"min metro trip oid "<<min_mt_oid<<endl;


  ////////////////btree on metro trips metro route id ///////////////////////
  
  ListExpr ptrList2 = listutils::getPtrList(metrotrips_rel);
  
  strQuery = "(createbtree (" + MetroTripTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "mr_oid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_trip_br_id = (BTree*)xResult.addr;

  ////////////////btree on metro trips oid///////////////////////////

  ListExpr ptrList3 = listutils::getPtrList(metrotrips_rel);

  strQuery = "(createbtree (" + MetroTripTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "oid)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_trip_oid = (BTree*)xResult.addr;

}

/*
given a metro stop, find its neighbor stop, i.e., the one after it

*/

int MetroNetwork::GetMS_Stop_Neighbor(UBahn_Stop* ms_stop )
{
  int line_id = ms_stop->line_id;
  int stop_id = ms_stop->stop_id;
  bool dir = ms_stop->d;

  CcInt* search_cell_id = new CcInt(true, line_id);
  BTreeIterator* btree_iter = btree_ms->ExactMatch(search_cell_id);

  int neighbor_tid = 0;
  bool found = false;
  while(btree_iter->Next()){
      Tuple* tuple = stops_rel->GetTuple(btree_iter->GetId(), false);

      int l_id = ((CcInt*)tuple->GetAttribute(M_R_ID))->GetIntval();
      assert(l_id == line_id);
//        bool d = ((CcBool*)tuple->GetAttribute(UB_DIRECTION))->GetBoolval();
      Bus_Stop* ms = (Bus_Stop*)tuple->GetAttribute(M_STOP);
      bool d = ms->GetUp();
      if(dir == d){
//          int s_id = ((CcInt*)tuple->GetAttribute(UB_STOP_ID))->GetIntval();
        int s_id = ms->GetStopId(); 
        if(dir){
            if(s_id == stop_id + 1){
              neighbor_tid = tuple->GetTupleId();
              found = true;
            }
          }else{
            if(s_id == stop_id - 1){
              neighbor_tid = tuple->GetTupleId();
              found = true;
            }
          }
      }
      tuple->DeleteIfAllowed();
      if(found)break;
  }
  delete btree_iter;
  delete search_cell_id;

  return neighbor_tid; 
}


/*
set the metro graph id 

*/
void MetroNetwork::SetGraphId(int g_id)
{
  graph_id = g_id; 
  graph_init = true; 
}


/*
get the metro graph in metro network

*/
MetroGraph* MetroNetwork::GetMetroGraph()
{
  if(graph_init == false) return NULL;
  
  ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
  xObjectList = nl->Rest(xObjectList);
  while(!nl->IsEmpty(xObjectList))
  {
    // Next element in list
    ListExpr xCurrent = nl->First(xObjectList);
    xObjectList = nl->Rest(xObjectList);

    // Type of object is at fourth position in list
    ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
    if(nl->IsAtom(xObjectType) &&
       nl->SymbolValue(xObjectType) == "metrograph"){
      // Get name of the metro graph 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. 
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined 
        continue;
      }
      MetroGraph* mg = (MetroGraph*)xValue.addr;
      if(mg->GetMG_ID() == graph_id){
        // This is the metro graph we have been looking for
        return mg;
      }
    }
  }
  return NULL;
}

/*
close the metro graph 

*/
void MetroNetwork::CloseMetroGraph(MetroGraph* mg)
{
  if(mg == NULL) return; 
  Word xValue;
  xValue.addr = mg;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("metrograph"),
                                           xValue);
}

/*
get the 2d point of a metro stop

*/
void MetroNetwork::GetMetroStopGeoData(Bus_Stop* ms, Point* p)
{

  int id = ms->GetId(); 
  if(id < 1){
    cout<<"invalid metro stop"<<endl;
    return; 
  }

  CcInt* search_id = new CcInt(true, id);
  BTreeIterator* btree_iter = btree_mr->ExactMatch(search_id);
  while(btree_iter->Next()){
      Tuple* tuple = routes_rel->GetTuple(btree_iter->GetId(), false);
      Bus_Route* mr = (Bus_Route*)tuple->GetAttribute(M_ROUTE);
      if(mr->GetUp() == ms->GetUp()){
        mr->GetMetroStopGeoData(ms, p); 
        tuple->DeleteIfAllowed();
        break; 
      }
      tuple->DeleteIfAllowed();
  }
  delete btree_iter;
  delete search_id; 

}

/*
get the moving metro pass the metro stop at the input time 

*/
int MetroNetwork::GetMOMetro_Oid(Bus_Stop* ms, Point* ms_loc, Instant& t)
{
//  cout<<"metro stop "<<*ms<<" loc "<<*ms_loc<<" time "<<t<<endl; 
  
  int mr_id = ms->GetId();
  
  //////////////////////////////////////////////////////////////////
  ////////////get the unique id for the metro route/////////////////
  //////////////////////////////////////////////////////////////////
  CcInt* search_id1 = new CcInt(true, mr_id);
  BTreeIterator* btree_iter1 = btree_mr->ExactMatch(search_id1);
  int mr_uoid = 0;
  while(btree_iter1->Next()){
      Tuple* tuple = routes_rel->GetTuple(btree_iter1->GetId(), false);
      Bus_Route* mr = (Bus_Route*)tuple->GetAttribute(M_ROUTE);
      if(mr->GetUp() == ms->GetUp()){
        mr_uoid = ((CcInt*)tuple->GetAttribute(M_R_OID))->GetIntval();
        tuple->DeleteIfAllowed();
        break;
      }
      tuple->DeleteIfAllowed();
  }
  delete btree_iter1;
  delete search_id1;
  assert(mr_uoid > 0);

//  cout<<"mr_uoid "<<mr_uoid<<endl;
  
  ///////////////////////////////////////////////////////////////
  ///////////////get moving metros moving on the route///////////
  ///////////////////////////////////////////////////////////////
  int metro_oid = 0;
  CcInt* search_id2 = new CcInt(true, mr_uoid);
  BTreeIterator* btree_iter2 = btree_trip_br_id->ExactMatch(search_id2);
  const double delta_dist = 0.01;
  vector<Id_Time> res_list;
  
    while(btree_iter2->Next()){
      Tuple* tuple = metrotrips_rel->GetTuple(btree_iter2->GetId(), false);
      int mrid = 
         ((CcInt*)tuple->GetAttribute(M_REFMR_OID))->GetIntval();
      assert(mrid == mr_uoid);
      MPoint* mo_metro = (MPoint*)tuple->GetAttribute(M_TRIP_MP);
      Periods* peri = new Periods(0);
      mo_metro->DefTime(*peri);
//      cout<<"periods "<<*peri<<endl; 
      if(peri->Contains(t)){
//        cout<<"periods containt instant "<<endl;

        for(int i = 0;i < mo_metro->GetNoComponents();i++){
          UPoint unit;
          mo_metro->Get(i, unit);
          Point p0 = unit.p0;
          Point p1 = unit.p1;

//          cout<<unit.timeInterval<<" dist "<<bs_loc->Distance(p0)<<endl;
//          cout<<"dist1 "<<bs_loc->Distance(p0)
//              <<" dist2 "<<bs_loc->Distance(p1)<<endl;

          if(ms_loc->Distance(p0) < delta_dist &&
             ms_loc->Distance(p1) < delta_dist){
             metro_oid = ((CcInt*)tuple->GetAttribute(M_TRIP_OID))->GetIntval();
             double delta_t = 
                fabs(unit.timeInterval.start.ToDouble() - t.ToDouble());
              Id_Time* id_time = new Id_Time(metro_oid, delta_t);
              res_list.push_back(*id_time);
              delete id_time;
          }
        }
      }
      delete peri;
      tuple->DeleteIfAllowed();
  }

  delete btree_iter2;
  delete search_id2;


  sort(res_list.begin(), res_list.end());

  assert(res_list.size() > 0);
  metro_oid = res_list[0].oid;
  
  assert(metro_oid > 0);
  
  return metro_oid;

}

/*
given a metro stop and time, it returns mpoint of the moving metro belonging to
that metro route and direction as well as covering the time

*/

int MetroNetwork::GetMOMetro_MP(Bus_Stop* ms, Point* ms_loc, 
                                Instant t, MPoint& mp)
{

//  cout<<"metro stop "<<*ms<<" loc "<<*ms_loc<<" time "<<t<<endl; 

  int mr_id = ms->GetId();
  //////////////////////////////////////////////////////////////////
  ////////////get the unique id for the metro route/////////////////
  //////////////////////////////////////////////////////////////////
  CcInt* search_id1 = new CcInt(true, mr_id);
  BTreeIterator* btree_iter1 = btree_mr->ExactMatch(search_id1);
  int mr_uoid = 0;
  while(btree_iter1->Next()){
      Tuple* tuple = routes_rel->GetTuple(btree_iter1->GetId(), false);
      Bus_Route* mr = (Bus_Route*)tuple->GetAttribute(M_ROUTE);
      if(mr->GetUp() == ms->GetUp()){
        mr_uoid = ((CcInt*)tuple->GetAttribute(M_R_OID))->GetIntval();
        tuple->DeleteIfAllowed();
        break;
      }
      tuple->DeleteIfAllowed();
  }
  delete btree_iter1;
  delete search_id1;
  assert(mr_uoid > 0);

//  cout<<"mr_uoid "<<mr_uoid<<endl;

  ///////////////////////////////////////////////////////////////
  ///////////////get moving metro moving on the route///////////
  ///////////////////////////////////////////////////////////////
  int metro_oid = 0;
  CcInt* search_id2 = new CcInt(true, mr_uoid);
  BTreeIterator* btree_iter2 = btree_trip_br_id->ExactMatch(search_id2);
  bool found = false;
  const double delta_dist = 0.01;
  vector<Id_Time> res_list;

  while(btree_iter2->Next() && found == false){
      Tuple* tuple = metrotrips_rel->GetTuple(btree_iter2->GetId(), false);
      int mrid = ((CcInt*)tuple->GetAttribute(M_REFMR_OID))->GetIntval();
      assert(mrid == mr_uoid);
      MPoint* mo_metro = (MPoint*)tuple->GetAttribute(M_TRIP_MP);
      Periods* peri = new Periods(0);
      mo_metro->DefTime(*peri);
      if(peri->Contains(t)){
//        cout<<"periods "<<*peri<<endl;
        for(int i = 0;i < mo_metro->GetNoComponents();i++){
          UPoint unit;
          mo_metro->Get(i, unit);
          Point p0 = unit.p0;
          Point p1 = unit.p1;

          if(ms_loc->Distance(p0) < delta_dist &&
              unit.timeInterval.Contains(t)){
              mp = *mo_metro;
             metro_oid = ((CcInt*)tuple->GetAttribute(M_TRIP_OID))->GetIntval();
              found = true;
              break;
          }
        }

      }
      delete peri;
      tuple->DeleteIfAllowed();
  }
  delete btree_iter2;
  delete search_id2;
  assert(metro_oid > 0);
  return metro_oid; 

}

////////////////////////////////////////////////////////////////////////
////////////////////metro graph/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
string MetroGraph::MGNodeTypeInfo =
"(rel (tuple ((ms_stop busstop) (stop_geodata point) (mr_id int))))";

string MetroGraph::MGEdge1TypeInfo =
"(rel (tuple ((ms_stop1_tid int) (ms_stop2_tid int))))";

string MetroGraph::MGEdge2TypeInfo = "(rel (tuple ((ms_stop1_tid int)\
(ms_stop2_tid int) (whole_time periods)(schedule_interval real)\
(Path sline) (TimeCost real))))";

string MetroGraph::MGNodeBTreeTypeInfo = 
"(btree (tuple ((ms_stop busstop) (stop_geodata point) (mr_id int))) int)";


ListExpr MetroGraph::MetroGraphProp()
{
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist,
               "createmetrograph(<id>,<edge-relation>,<node-relation>)");
    return nl->TwoElemList(
             nl->TwoElemList(nl->StringAtom("Creation"),
                              nl->StringAtom("Example Creation")),
             nl->TwoElemList(examplelist,
                   nl->StringAtom("let mg=createmetrograph(id,e-rel,n-rel)")));
}


bool MetroGraph::CheckMetroGraph(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckMetroGraph()"<<endl;
  return nl->IsEqual(type, "metrograph");
}

int MetroGraph::SizeOfMetroGraph()
{
//  cout<<"SizeOfMetroGraph()"<<endl;
  return 0;
}

void* MetroGraph::CastMetroGraph(void* addr)
{
//  cout<<"CastMetroGraph()"<<endl;
  return 0;
}
Word MetroGraph::CloneMetroGraph(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneMetroGraph()"<<endl;
  return SetWord(Address(0));
}

void MetroGraph::CloseMetroGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseMetroGraph()"<<endl;
  delete static_cast<MetroGraph*> (w.addr);
  w.addr = NULL;
}

Word MetroGraph::CreateMetroGraph(const ListExpr typeInfo)
{
//  cout<<"MetroMetroGraph()"<<endl;
  return SetWord(new MetroGraph());
}

void MetroGraph::DeleteMetroGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteMetroGraph()"<<endl;
  MetroGraph* mg = (MetroGraph*)w.addr;
  delete mg;
  w.addr = NULL;
}

/*
input metro network graph 

*/
Word MetroGraph::InMetroGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
//  cout<<"InMetroGraph()"<<endl;
  MetroGraph* mg = new MetroGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(mg);
  else{
    delete mg;
    return SetWord(Address(0));
  }
}

ListExpr MetroGraph::OutMetroGraph(ListExpr typeInfo, Word value)
{
//  cout<<"OutMetroGraph()"<<endl;
  MetroGraph* mg = (MetroGraph*)value.addr;
  return mg->Out(typeInfo);
}

ListExpr MetroGraph::Out(ListExpr typeInfo)
{
//  cout<<"Out()"<<endl;
  ListExpr xNode = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= node_rel->GetNoTuples();i++){
      Tuple* node_tuple = node_rel->GetTuple(i, false);
/*      int l_id = 
        ((CcInt*)node_tuple->GetAttribute(MG_NODE_LINE_ID))->GetIntval();
      Point* loc = (Point*)node_tuple->GetAttribute(MG_NODE_LOC);
      int stop_id = 
        ((CcInt*)node_tuple->GetAttribute(MG_NODE_STOP_ID))->GetIntval();
      bool dir = 
        ((CcBool*)node_tuple->GetAttribute(MG_NODE_UP))->GetBoolval();*/

      int l_id = 
        ((CcInt*)node_tuple->GetAttribute(MG_NODE_MR_ID))->GetIntval();
      Point* loc = (Point*)node_tuple->GetAttribute(MG_NODE_STOP_GEO);
      Bus_Stop* ms_stop = (Bus_Stop*)node_tuple->GetAttribute(MG_NODE_STOP);
      int stop_id = ms_stop->GetStopId();
      bool dir = ms_stop->GetUp();

      xNext = nl->FourElemList(
           nl->IntAtom(l_id), nl->IntAtom(stop_id), 
           nl-> BoolAtom(dir), 
           OutPoint( nl->TheEmptyList(), SetWord(loc) ));

      if(bFirst){
        xNode = nl->OneElemList(xNext);
        xLast = xNode;
        bFirst = false;
      }else
          xLast = nl->Append(xLast,xNext);
      node_tuple->DeleteIfAllowed();
  }
  return nl->TwoElemList(nl->IntAtom(mg_id),xNode);

//  return nl->OneElemList(nl->IntAtom(mg_id));

}

bool MetroGraph::SaveMetroGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveMetroGraph()"<<endl;
  MetroGraph* mg = (MetroGraph*)value.addr;
  bool result = mg->Save(valueRecord, offset, typeInfo);

  return result;
}

bool MetroGraph::OpenMetroGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenMetroGraph()"<<endl;
  value.addr = MetroGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

MetroGraph* MetroGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
  return new MetroGraph(valueRecord,offset,typeInfo);
}


MetroGraph::MetroGraph():mg_id(0), min_t(0),
node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0),
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0)
{
//  cout<<"BusGraph::BusGraph()"<<endl;
}

MetroGraph::~MetroGraph()
{
   if(node_rel != NULL) node_rel->Close();
   if(btree_node != NULL) delete btree_node; 
   if(edge_rel1 != NULL) edge_rel1->Close(); 
   if(edge_rel2 != NULL) edge_rel2->Close(); 

}

MetroGraph::MetroGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect):
mg_id(0), min_t(0),
node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0),
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0)
{
//  cout<<"MetroGraph::MetroGraph(ListExpr)"<<endl;
}

MetroGraph::MetroGraph(SmiRecord& in_xValueRecord, size_t& inout_iOffset,
const ListExpr in_xTypeInfo):mg_id(0), min_t(0),
node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0),
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0)
{
  in_xValueRecord.Read(&mg_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Read(&min_t,sizeof(double),inout_iOffset);
  inout_iOffset += sizeof(double);

  ListExpr xType;
  ListExpr xNumericType;
  ///////////////////Open relation for node////////////////////////////
  nl->ReadFromString(MGNodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  node_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!node_rel) {
    return;
  }

  ///////////////////open btree built on nodes//////////////////////////
   nl->ReadFromString(MGNodeBTreeTypeInfo,xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
   btree_node = BTree::Open(in_xValueRecord, inout_iOffset, xNumericType);
   if(!btree_node) {
     node_rel->Delete();
     return;
   }

  ///////////////////////Open relation for edge1///////////////////////////
  nl->ReadFromString(MGEdge1TypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel1 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel1) {
    node_rel->Delete();
    delete btree_node; 
    return;
  }

  /////////////////open adjacency list1////////////////////////////////
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list1.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list1.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

  ///////////////////////Open relation for edge2//////////////////////////
  nl->ReadFromString(MGEdge2TypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel2 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel2) {
    node_rel->Delete();
    delete btree_node; 
    edge_rel1->Delete(); 
    adj_list1.clean();
    entry_adj_list1.clean(); 
    return;
  }

  /////////////////open adjacency list2////////////////////////////////
   bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list2.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list2.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

}


bool MetroGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo)
{

  //  cout<<"save "<<endl; 
  in_xValueRecord.Write(&mg_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);

  in_xValueRecord.Write(&min_t,sizeof(double),inout_iOffset);
  inout_iOffset += sizeof(double);

  ListExpr xType;
  ListExpr xNumericType;
  ///////////////////////////save node/////////////////////////////
  nl->ReadFromString(MGNodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  
  ////////////////save btree on nodes///////////////////////////
   nl->ReadFromString(MGNodeBTreeTypeInfo, xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType); 
   if(!btree_node->Save(in_xValueRecord, inout_iOffset, xNumericType))
     return false; 

  /////////////////////save edge1/////////////////////////////
  nl->ReadFromString(MGEdge1TypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel1->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /////////////////adjacency list 1//////////////////////////////
   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();


   adj_list1.saveToFile(rf, adj_list1);
   SmiSize offset = 0;
   size_t bufsize = adj_list1.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   adj_list1.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list1.saveToFile(rf, entry_adj_list1);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list1.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;


  ///////////////////////save edge2/////////////////////////////////////
  nl->ReadFromString(MGEdge2TypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel2->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /////////////////adjacency list 2//////////////////////////////////

   adj_list2.saveToFile(rf, adj_list2);
   offset = 0;
   bufsize = adj_list2.headerSize()+ 2*sizeof(int);
   buf = (char*) malloc(bufsize);
   adj_list2.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list2.saveToFile(rf, entry_adj_list2);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list2.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

   return true; 


}

/*
load metro graph from input relations 

*/

void MetroGraph::Load(int g_id, Relation* r1, Relation* edge1, Relation* edge2)
{
  if(g_id <= 0){
    cout<<"invalid graph id "<<g_id<<endl;
    return;
  }
  mg_id = g_id;

  //////////////////node relation////////////////////
  ListExpr ptrList1 = listutils::getPtrList(r1);

  string strQuery = "(consume(feed(" + MGNodeTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;

//  cout<<"nodes size "<<node_rel->GetNoTuples()<<endl;

  ///////////////////////////btree on nodes///////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(node_rel);

  strQuery = "(createbtree (" + MGNodeTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "mr_id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  btree_node = (BTree*)xResult.addr;


//  cout<<edge1->GetNoTuples()<<" "<<edge2->GetNoTuples()<<endl;

  LoadEdge1(edge1);
  LoadEdge2(edge2);

}

/*
metro graph edges: two metro stops have the same spatial location in space

*/
void MetroGraph::LoadEdge1(Relation* edge1)
{
  ListExpr ptrList1 = listutils::getPtrList(edge1);

  string strQuery = "(consume(feed(" + MGEdge1TypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel1 = (Relation*)xResult.addr;

  //////////////////create adjacency list////////////////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(edge_rel1);
  
  strQuery = "(createbtree (" + MGEdge1TypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "ms_stop1_tid)";
             
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ////// visibility graph. it is the same as indoor and bus graph //////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* ms_tuple = node_rel->GetTuple(i, false);

    CcInt* nodeid = new CcInt(true, ms_tuple->GetTupleId());
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list1.Size();
    while(btree_iter->Next()){
      Tuple* edge_tuple = edge_rel1->GetTuple(btree_iter->GetId(), false);

      adj_list1.Append(edge_tuple->GetTupleId());//get the edge tuple id 

      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list1.Size();
    entry_adj_list1.Append(ListEntry(start, end));

   //cout<<"start "<<start<<" end "<<end<<endl;

    delete nodeid;

    ms_tuple->DeleteIfAllowed();

  }

  delete btree;
}

/*
metro graph edges: two metro stops are connected by moving metros

*/
void MetroGraph::LoadEdge2(Relation* edge2)
{

  ListExpr ptrList1 = listutils::getPtrList(edge2);

  string strQuery = "(consume(feed(" + MGEdge2TypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel2 = (Relation*)xResult.addr;


  min_t = numeric_limits<double>::max();

  for(int i = 1;i <= edge_rel2->GetNoTuples();i++){
    Tuple* edge_tuple = edge_rel2->GetTuple(i, false);

    Periods* peri = (Periods*)edge_tuple->GetAttribute(MG_EDGE2_PERI);
    Interval<Instant> periods;
    peri->Get(0, periods);
    double t = periods.start.ToDouble();
    if(t < min_t) min_t = t; 

    edge_tuple->DeleteIfAllowed();

  }

//    Instant min_time(instanttype);
//    min_time.ReadFrom(min_t);
//    printf("%.10f\n",min_t); 
//    cout<<min_time<<endl; 


 //////////////////create adjacency list////////////////////////////////////

  ListExpr ptrList2 = listutils::getPtrList(edge_rel2);

  strQuery = "(createbtree (" + MGEdge2TypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "ms_stop1_tid)";


  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///// visibility graph. it is the same as indoor and bus graph /////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* ms_tuple = node_rel->GetTuple(i, false);

    CcInt* nodeid = new CcInt(true, ms_tuple->GetTupleId());
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list2.Size();
    while(btree_iter->Next()){
      Tuple* edge_tuple = edge_rel2->GetTuple(btree_iter->GetId(), false);
      adj_list2.Append(edge_tuple->GetTupleId());//get the edge tuple id 
      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list2.Size();
    entry_adj_list2.Append(ListEntry(start, end));

    delete nodeid;

    ms_tuple->DeleteIfAllowed();

  }

  delete btree;

}

/*
find the metro stop tid in the relation 

*/
int MetroGraph::GetMetroStop_Tid(Bus_Stop* ms)
{

  if(!ms->IsDefined()) return -1; 
  
  int ms_tid = -1;
  CcInt* search_id = new CcInt(true,ms->GetId());
  BTreeIterator* btree_iter = btree_node->ExactMatch(search_id);
  while(btree_iter->Next()){
    Tuple* ms_tuple = node_rel->GetTuple(btree_iter->GetId(), false);
    Bus_Stop* ms_stop = (Bus_Stop*)ms_tuple->GetAttribute(MG_NODE_STOP);
    int mr_id = ms->GetId();

    assert(mr_id == (int) ms->GetId());
    if(ms->GetStopId() == ms_stop->GetStopId() && 
       ms->GetUp() == ms_stop->GetUp()){
      ms_tid = ms_tuple->GetTupleId();
      ms_tuple->DeleteIfAllowed();
      break;
    }

    ms_tuple->DeleteIfAllowed();

  }
  delete btree_iter;
  delete search_id;
  assert(ms_tid > 0 && ms_tid <= node_rel->GetNoTuples()); 
  return ms_tid; 


}

/*
metro stops have the same spatial location in space

*/
void MetroGraph::FindAdj1(int node_id, vector<int>& list)
{
  ListEntry list_entry;
  entry_adj_list1.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list1.Get(j, oid);
      j++;
      list.push_back(oid);
  }
  
}

/*
metro stops are connected by moving metros 

*/
void MetroGraph::FindAdj2(int node_id, vector<int>& list)
{

  ListEntry list_entry;
  entry_adj_list2.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list2.Get(j, oid);
      j++;
      list.push_back(oid);
  }

}

////////////////////////////////////////////////////////////////////////////
////////////////// query processing on the metro graph and network//////////
////////////////////////////////////////////////////////////////////////////
/*
shortest path on metro network and graph 

*/
void MNNav::ShortestPath_Time(Bus_Stop* ms1, Bus_Stop* ms2, Instant* qt)
{
//  cout<<"shortest path on metro network"<<endl; 
  MetroGraph* mg = mn->GetMetroGraph(); 

  if(mg == NULL){
    cout<<"metro graph is invalid"<<endl; 
    return;
  }

//  cout<<mg->GetNode_Rel()->GetNoTuples()<<endl;

  if(!ms1->IsDefined() || !ms2->IsDefined()){
    cout<<" metro stops are not defined"<<endl;
    return; 
  }

//  cout<<*ms1<<" "<<*ms2<<endl;

  Point start_p, end_p; 
  mn->GetMetroStopGeoData(ms1, &start_p);
  mn->GetMetroStopGeoData(ms2, &end_p);
//  cout<<"start "<<start_p<<" end "<<end_p<<endl;

  
  const double delta_dist = 0.01; 

  if(*ms1 == *ms2 || start_p.Distance(end_p) < delta_dist){
   cout<<"two bus stops equal to each other"<<endl;
   mn->CloseMetroGraph(mg);
   return; 
  }

  /////////////////////////build the start time///////////////////////////
  Instant new_st(instanttype);
  Instant mg_min(instanttype);
  mg_min.ReadFrom(mg->GetMIN_T());

//  cout<<"mg_min "<<mg_min<<endl;

  new_st.Set(mg_min.GetYear(), mg_min.GetMonth(), mg_min.GetGregDay(),
           qt->GetHour(), qt->GetMinute(), qt->GetSecond(),
           qt->GetMillisecond());

//  cout<<"mapping start time"<<new_st<<endl; 

  //////////////////////////////////////////////////////////////////////////

  priority_queue<BNPath_elem> path_queue;
  vector<BNPath_elem> expand_queue;

  vector<bool> visit_flag1;////////////metro stop visit 
  for(int i = 1; i <= mg->GetNode_Rel()->GetNoTuples();i++)
    visit_flag1.push_back(false);

  //////////////////////////////////////////////////////////////////
  /////////from metro network, get the maximum speed of the metro///
  /////////////for setting heuristic value/////////////////////////
  ///////////////////////////////////////////////////////////////////
// cout<<"max metro speed "<<mn->GetMaxSpeed()*60.0*60.0/1000.0<<"km/h"<<endl;


  ///////////  initialize the queue //////////////////////////////
  InitializeQueue(ms1, ms2, path_queue, expand_queue, mn, mg, start_p, end_p);

  int ms2_tid = mg->GetMetroStop_Tid(ms2);
//  cout<<"end bus stop tid "<<ms2_tid<<endl;

  ////////////////////////////////////////////////////////////////////
  ////////////////////search on the metro graph///////////////////////
  ////////////////////////////////////////////////////////////////////
  bool find = false;
  BNPath_elem dest;//////////destination
 
  while(path_queue.empty() == false){
    BNPath_elem top = path_queue.top();
    path_queue.pop();

    if(visit_flag1[top.tri_index - 1]) continue;

//     cout<<"top elem "<<endl;
//     top.Print();


    if(top.tri_index == ms2_tid){
//       cout<<"find the shortest path"<<endl;
       find = true;
       dest = top;
       break;
    }
    int pos_expand_path;
    int cur_size; 

    pos_expand_path = top.cur_index;

    /////////////////////////////////////////////////////////////////////
    //////////////////////connection 1 same spatial location/////////////
    ////////////////////////////////////////////////////////////////////
    if(top.tm == TM_METRO){
      vector<int> adj_list1;
      mg->FindAdj1(top.tri_index, adj_list1);
//    cout<<"adj1 size "<<adj_list1.size()<<endl;

      for(unsigned int i = 0;i < adj_list1.size();i++){
        Tuple* edge_tuple = mg->GetEdge_Rel1()->GetTuple(adj_list1[i], false);
        int neighbor_id1 = 
     ((CcInt*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE1_TID2))->GetIntval();
        SimpleLine* path = new SimpleLine(0);
        path->StartBulkLoad();
        path->EndBulkLoad();
//      cout<<"neighbor_tid1 "<<neighbor_id1<<endl;

        if(visit_flag1[neighbor_id1 - 1]){
          edge_tuple->DeleteIfAllowed();
          delete path;
          continue; 
        }

        cur_size = expand_queue.size();
        double w = top.real_w; 
        Tuple* ms_node_tuple = mg->GetNode_Rel()->GetTuple(neighbor_id1, false);
        Point* p = 
          (Point*)ms_node_tuple->GetAttribute(MetroGraph::MG_NODE_STOP_GEO);
        double hw = p->Distance(end_p)/(mn->GetMaxSpeed()*24.0*60.0*60.0);
        ms_node_tuple->DeleteIfAllowed();


//         BNPath_elem elem(pos_expand_path, cur_size, neighbor_id1, w + hw, w,
//                        *path, -1, false); //no useful for time cost 


        BNPath_elem elem(pos_expand_path, cur_size, neighbor_id1, w + hw, w,
                       *path, -1, false); //no useful for time cost 
        elem.type = -1;
        elem.edge_tid = 0;

        path_queue.push(elem);
        expand_queue.push_back(elem); 

//       cout<<"neighbor1 ";
//       elem.Print();
//       cout<<endl; 

        delete path; 
        edge_tuple->DeleteIfAllowed();
      }
    }


    //////////////////////////////////////////////////////////////////////
    ////////////////////connection 2 moving metros/////////////////////////
    //////////////////////////////////////////////////////////////////////
    vector<int> adj_list2;
    mg->FindAdj2(top.tri_index, adj_list2);
    int64_t max_64_int = numeric_limits<int64_t>::max();

//    cout<<"ajd2 size "<<adj_list2.size()<<endl;

    for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = mg->GetEdge_Rel2()->GetTuple(adj_list2[i], false);
       int neighbor_id2 = 
     ((CcInt*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE2_TID2))->GetIntval();
//        SimpleLine* path =
//            (SimpleLine*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE2_PATH);

//      cout<<"neighbor_id2 "<<neighbor_id2<<endl;

       if(visit_flag1[neighbor_id2 - 1]){
         edge_tuple->DeleteIfAllowed();
         continue;
       }

       cur_size = expand_queue.size();
       double cur_t = new_st.ToDouble() + top.real_w; 
       Instant cur_inst = new_st;
       cur_inst.ReadFrom(cur_t); //time to arrive current metro stop
//       cout<<"time at metro stop "<<cur_inst<<endl; 

       int64_t cur_t_int = cur_t*86400000; 
       assert(cur_t_int <= max_64_int);

       Periods* peri = 
               (Periods*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE2_PERI);
       Interval<Instant> periods;
       peri->Get(0, periods);

       double sched = 
       ((CcReal*)edge_tuple->GetAttribute(MetroGraph::
                                          MG_EDGE2_SCHED))->GetRealval();

       double st = periods.start.ToDouble(); 
       double et = periods.end.ToDouble(); 
       int64_t st_int = st*86400000;
       int64_t et_int = et*86400000; 
       assert(st_int <= max_64_int);
       assert(et_int <= max_64_int);

//       cout<<"st "<<periods.start<<" et "<<periods.end<<endl; 

       if(et_int < cur_t_int){//end time smaller than curtime 
         edge_tuple->DeleteIfAllowed();
         continue;
       }
//       cout<<"st_int "<<st_int<<" cur_t_int "<<cur_t_int<<endl;

       double wait_time = 0.0;
       if(st_int > cur_t_int){//wait for the first start time 
            wait_time += st - cur_t; 
            wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at metro stop 
       }else if(st_int == cur_t_int){
          wait_time += 30.0/(24.0*60.0*60.0);//30 seconds at metro stop 
       }else{ //most times, it is here, wait for the next schedule 

         bool valid = false;
         while(st_int < cur_t_int && st_int <= et_int){

           Instant temp(instanttype);
           temp.ReadFrom(st);
//           cout<<"t1 "<<temp<<endl; 
//           cout<<"st_int "<<st_int<<" cur_t_int "<<cur_t_int<<endl;

          if((st_int + 30000) >= cur_t_int){//30 second
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break;
          }
          st += sched; 
          st_int = st * 86400000; 
          if(st_int >= cur_t_int){
            wait_time += st + 30.0/(24.0*60.0*60.0) - cur_t;
            valid = true;
            break; 
          }
          assert(st_int <= max_64_int); 

//           temp.ReadFrom(st);
//           cout<<"t2 "<<temp<<endl; 

         }
         if(valid == false){
           cout<<"should not arrive at here"<<endl; 
           assert(false); 
         }
       }

       double weight = 
       ((CcReal*)edge_tuple->GetAttribute(MetroGraph::
                                          MG_EDGE2_TIME_COST))->GetRealval();
        double w = top.real_w + wait_time + weight; 

        Tuple* ms_node_tuple = mg->GetNode_Rel()->GetTuple(neighbor_id2, false);
        Point* p = 
            (Point*)ms_node_tuple->GetAttribute(MetroGraph::MG_NODE_STOP_GEO);
        double hw = p->Distance(end_p)/(mn->GetMaxSpeed()*24.0*60.0*60.0);
        ms_node_tuple->DeleteIfAllowed();


//        BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
//                         *path, TM_METRO, true);

       SimpleLine temp_sl(0);
       BNPath_elem elem(pos_expand_path, cur_size, neighbor_id2, w + hw, w,
                        temp_sl, TM_METRO, true);
       elem.type = TM_METRO;
       elem.edge_tid = adj_list2[i];


       if(wait_time > 0.0){ //to the time waiting for metro
          elem.SetW(top.real_w + wait_time);
       }

       path_queue.push(elem);
       expand_queue.push_back(elem);

//       cout<<"neighbor2 "<<wait_time<<" "<<weight<<endl;
//       elem.Print();
//       cout<<endl;

      edge_tuple->DeleteIfAllowed();

    }

    visit_flag1[top.tri_index - 1] = true; 

  }

  //////////////////////////////////////////////////////////////////////
  ////////////////construct the result//////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  if(find){   ////////constrcut the result 
      vector<int> id_list; 
      while(dest.prev_index != -1){
       id_list.push_back(dest.cur_index);
       dest = expand_queue[dest.prev_index];
     }

    id_list.push_back(dest.cur_index);

    Bus_Stop ms_last = *ms1; 
    Instant t1 = *qt;
//    int no_transfer = 0; 

    for(int i = id_list.size() - 1;i >= 0;i--){
      BNPath_elem elem = expand_queue[id_list[i]];
      
      if(elem.type == TM_METRO){
        if(elem.edge_tid > 0){
         Tuple* edge_tuple = mg->GetEdge_Rel2()->GetTuple(elem.edge_tid, false);
         SimpleLine* path =
            (SimpleLine*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE2_PATH);
          elem.path = *path;
          edge_tuple->DeleteIfAllowed();
        }

      }else if(elem.type == -1){

      }else{
        cout<<"should not be here"<<endl;
        assert(false);
      }
      
      
      path_list.push_back(elem.path);

      if(elem.tm == TM_METRO){
          tm_list.push_back(str_tm[elem.tm]); 
      }else{
//        assert(false);
          tm_list.push_back("none"); 
      }

      ////////////////////////////////////////////////////////////////////
      ////////////////we also return///////////////////////////////////////
      ////////the start and end metro stops connected by the path /////////
      ////////////////////////////////////////////////////////////////////
      char buf1[256], buf2[256];

      sprintf(buf1, "mr: %d ", ms_last.GetId());
      sprintf(buf2, "stop: %d", ms_last.GetStopId());
      strcat (buf1, buf2);
      if(ms_last.GetUp()) strcat (buf1, " UP");
        else strcat (buf1, " DOWN");

      string str1(buf1);
      ms1_list.push_back(str1);

      if(i == (int)(id_list.size() - 1)){
        string str2(str1);
        ms2_list.push_back(str2);

      }else{////////////////the end metro stop 

        Tuple* ms_tuple = mg->GetNode_Rel()->GetTuple(elem.tri_index, false);
        Bus_Stop* ms_cur = 
              (Bus_Stop*)ms_tuple->GetAttribute(MetroGraph::MG_NODE_STOP); 
        char buf_1[256], buf_2[256];
        sprintf(buf_1, "mr: %d ", ms_cur->GetId());
        sprintf(buf_2, "stop: %d", ms_cur->GetStopId());
        strcat (buf_1, buf_2);   
        if(ms_cur->GetUp()) strcat (buf_1, " UP");
            else strcat (buf_1, " DOWN");

        string str2(buf_1);
        ms2_list.push_back(str2);
        ms_last = *ms_cur;
        ms_tuple->DeleteIfAllowed();
      }

        ////////////////time duration////////////////////////////////

        Instant t2(instanttype);
        if(elem.b_w == false){
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri = new Periods(0);
          peri->StartBulkLoad();
          if(elem.valid)
            peri->MergeAdd(time_span);
          peri->EndBulkLoad();
          peri_list.push_back(*peri); 
          t1 = t2; 
          delete peri; 
        }else{ //////////to dinstinguish time of waiting for the metro 
          t2.ReadFrom(elem.w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

        //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 
          Interval<Instant> time_span;
          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri1 = new Periods(0);
          peri1->StartBulkLoad();
          if(elem.valid)
            peri1->MergeAdd(time_span);
          peri1->EndBulkLoad();
          peri_list.push_back(*peri1); 
          t1 = t2; 
          delete peri1; 

          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl->EndBulkLoad();
          path_list[path_list.size() - 1] = *sl;
          delete sl; 

          tm_list[tm_list.size() - 1] = "none"; //waiting is no tm 
          string str = ms2_list[ms2_list.size() - 1];
          ////////the same as last metro stop //////////////////////
          ms2_list[ms2_list.size() - 1] = ms1_list[ms1_list.size() - 1];

          /////////////moving with metro////////////////////////////////
          t2.ReadFrom(elem.real_w + qt->ToDouble());
//        cout<<t1<<" "<<t2<<endl; 

          //time cost in seconds 
          if(elem.valid)
            time_cost_list.push_back((t2.ToDouble() - t1.ToDouble())*86400.0);
          else //doing transfer without moving 
            time_cost_list.push_back(0.0); 

          time_span.start = t1;
          time_span.lc = true;
          time_span.end = t2;
          time_span.rc = false; 

          Periods* peri2 = new Periods(0);
          peri2->StartBulkLoad();
          if(elem.valid)
            peri2->MergeAdd(time_span);
          peri2->EndBulkLoad();
          peri_list.push_back(*peri2); 
          t1 = t2; 
          delete peri2; 
          path_list.push_back(elem.path);
          tm_list.push_back(str_tm[elem.tm]);
          ms1_list.push_back(str1);
          ms2_list.push_back(str); 

        }

    }
//    cout<<" transfer "<<no_transfer<<" times "<<endl; 
  }else{
//    cout<<"ms1 ("<<*ms1<<") ms2 ("<<*ms2<<") not reachable "<<endl;
  }


  mn->CloseMetroGraph(mg);

}


/*
initialize the queue: shortest path in time 

*/
void MNNav::InitializeQueue(Bus_Stop* ms1, Bus_Stop* ms2,
                            priority_queue<BNPath_elem>& path_queue, 
                            vector<BNPath_elem>& expand_queue, 
                            MetroNetwork* mn, MetroGraph* mg,
                            Point& start_p, Point& end_p)
{
    int cur_size = expand_queue.size();
    double w = 0.0; 

    double hw = start_p.Distance(end_p)/(mn->GetMaxSpeed()*24.0*60.0*60.0);
//    double hw = 0.0; 

    SimpleLine* sl = new SimpleLine(0);
    sl->StartBulkLoad();
    sl->EndBulkLoad();

    int ms_tid = mg->GetMetroStop_Tid(ms1); 
//    cout<<"start metro stop tid "<<ms_tid<<endl;
    //////////////////no time cost////////////////////////////////////
    BNPath_elem elem(-1, cur_size, ms_tid, w + hw, w, *sl, TM_METRO, false);
    
    elem.type = TM_METRO;
    elem.edge_tid = 0;
    
    path_queue.push(elem);
    expand_queue.push_back(elem); 
    delete sl;
}

/*
get adjacent list for a metro graph node 

*/
void MNNav::GetAdjNodeMG(MetroGraph* mg, int nodeid)
{
 
  if(mg->GetNode_Rel() == NULL){
    cout<<"no metro graph node rel"<<endl;
    return; 
  }
  if(nodeid < 1 || nodeid > mg->GetNode_Rel()->GetNoTuples()){
      cout<<"invalid node id "<<endl; 
      return; 
  }

  cout<<"total "<<mg->GetNode_Rel()->GetNoTuples()<<" nodes "<<endl;
  cout<<"total "<<mg->GetEdge_Rel1()->GetNoTuples() + 
                  mg->GetEdge_Rel2()->GetNoTuples()<<" edges "<<endl;

  
  Relation* node_rel = mg->GetNode_Rel();

  Tuple* ms_tuple = node_rel->GetTuple(nodeid, false);
  Bus_Stop* ms1 = (Bus_Stop*)ms_tuple->GetAttribute(MetroGraph::MG_NODE_STOP);
//  cout<<*ms1<<endl; 

  ///////////////////////////////////////////////////////////////////////////
  //////the first kind of connection (no path; the same spatial location)////
  ////////////////////////////////////////////////////////////////////////////
  vector<int> tid_list1; 
  mg->FindAdj1(nodeid, tid_list1); 
  
  for(unsigned int i = 0;i < tid_list1.size();i++){
    Tuple* edge_tuple = mg->GetEdge_Rel1()->GetTuple(tid_list1[i], false);
    int neighbor_id = 
     ((CcInt*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE1_TID2))->GetIntval();
    edge_tuple->DeleteIfAllowed();

    Tuple* ms_neighbor1 = node_rel->GetTuple(neighbor_id, false);
    Bus_Stop* ms2 = 
        (Bus_Stop*)ms_neighbor1->GetAttribute(MetroGraph::MG_NODE_STOP);

    SimpleLine* path = new SimpleLine(0);
    path->StartBulkLoad();
    path->EndBulkLoad(); 
    ms_list1.push_back(*ms1);
    ms_list2.push_back(*ms2);
    path_list.push_back(*path);
    delete path; 

    ms_neighbor1->DeleteIfAllowed();
    type_list.push_back(1); 
  }
  
  ////////////////////////////////////////////////////////////////////
  //////the second kind of connection (connected by moving metros)////
  ////////////////////////////////////////////////////////////////////
  
  vector<int> tid_list2; 
  mg->FindAdj2(nodeid, tid_list2); 

  for(unsigned int i = 0;i < tid_list2.size();i++){

    Tuple* edge_tuple = mg->GetEdge_Rel2()->GetTuple(tid_list2[i], false);
    int neighbor_id = 
     ((CcInt*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE2_TID2))->GetIntval();
    Tuple* ms_neighbor2 = node_rel->GetTuple(neighbor_id, false);
    
    Bus_Stop* ms3 = 
      (Bus_Stop*)ms_neighbor2->GetAttribute(MetroGraph::MG_NODE_STOP);


    SimpleLine* path = 
          (SimpleLine*)edge_tuple->GetAttribute(MetroGraph::MG_EDGE2_PATH);
    ms_list1.push_back(*ms1);
    ms_list2.push_back(*ms3);
    path_list.push_back(*path);

    edge_tuple->DeleteIfAllowed();
    ms_neighbor2->DeleteIfAllowed();

    type_list.push_back(2); 
  }

  ms_tuple->DeleteIfAllowed();


}


///////////////////////////////////////////////////////////////////////
////////// improve join algorithm, not use symmjoin////////////////////
///////////////////////////////////////////////////////////////////////
string TM_Join::CellBoxTypeInfo = "(rel (tuple ((cellid int)\
(cover_area region) (x_id int) (y_id int))))";

string TM_Join::RoadSectionTypeInfo = "(rel (tuple ((rid int) (meas1 real)\
(meas2 real) (dual bool) (curve sline)(curveStartsSmaller bool)\
(rrc tid) (sid int))))";


/*
for each cell, it collects the number of route section intersecting it

*/
void TM_Join::Road_Cell_Join(Relation* rel1, Relation* rel2, 
                             R_Tree<2,TupleId>* rtree)
{
//  cout<<rel1->GetNoTuples()<<" "<<rel2->GetNoTuples()<<endl;

  
  vector<Region> cell_area_list(rel2->GetNoTuples(), Region(0));
  for(int i = 1;i <= rel2->GetNoTuples();i++){
      Tuple* cell_tuple = rel2->GetTuple(i, false);
      int cellid = ((CcInt*)cell_tuple->GetAttribute(TM_JOIN_ID))->GetIntval();
      Region* reg = (Region*)cell_tuple->GetAttribute(TM_JOIN_AREA);
      cell_area_list[cellid - 1] = *reg;
      cell_tuple->DeleteIfAllowed();
  }

  vector<int> cell_list;
  vector<int> sec_id_list(rel2->GetNoTuples(), 0);

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* road_tuple = rel1->GetTuple(i, false);
    int sec_id = ((CcInt*)road_tuple->GetAttribute(SECTION_SID))->GetIntval();
    SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(SECTION_CURVE);
    Line* l = new Line(0);
    sl->toLine(*l);

    vector<int> tid_list;
    DFTraverse(rel2, rtree, rtree->RootRecordId(), l, tid_list);

//    cout<<"rid "<<r_id<<" "<<tid_list.size()<<endl;
    if(tid_list.size() > 0){
      for(unsigned int j = 0;j < tid_list.size();j++){

        Tuple* cell_tuple = rel2->GetTuple(tid_list[j], false);
        int cell_id = 
          ((CcInt*)cell_tuple->GetAttribute(TM_JOIN_ID))->GetIntval();

        cell_list.push_back(cell_id);
        cell_tuple->DeleteIfAllowed();

        if(sec_id_list[cell_id - 1] == 0){
            sec_id_list[cell_id - 1] = sec_id;
        }
      }
    }
    delete l;
    road_tuple->DeleteIfAllowed();
  }

  sort(cell_list.begin(), cell_list.end());
  for(unsigned int i = 0;i < cell_list.size();i++){
    int cell_id = cell_list[i];
    unsigned int j = i + 1;
    int count = 1;
    while(j < cell_list.size() && cell_list[j] == cell_id){
      count++;
      j++;
    }

    i = j - 1;

    sec_list.push_back(sec_id_list[cell_id - 1]);
    id_list.push_back(cell_id);
    count_list.push_back(count);
    area_list.push_back(cell_area_list[cell_id - 1]);
  }

//    for(unsigned int i = 0;i < id_list.size();i++)
//      cout<<id_list[i]<<" no: "<<count_list[i]<<endl;

}

/*
find all cell box that intersect the road line

*/
void TM_Join::DFTraverse(Relation* rel, R_Tree<2,TupleId>* rtree,
                           SmiRecordId adr, Line* l, vector<int>& id_list)
{
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* tuple = rel->GetTuple(e.info,false);
              Region* reg =(Region*)tuple->GetAttribute(TM_JOIN_AREA);
              if(l->Intersects(*reg)){
                id_list.push_back(e.info);
              }
              tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(l->Intersects(e.box)){
              DFTraverse(rel, rtree, e.pointer, l, id_list);
            }
      }
  }
  delete node;
}

