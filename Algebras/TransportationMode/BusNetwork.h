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
#include "Partition.h"
#include "PaveGraph.h"

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
structure that is used for bus stop 

*/
struct BusStop{
  int br_id;
  int br_stop_id; 
  int rid;
  double pos;
  int sid; 
  bool def;
  BusStop(){}
  BusStop(int id1, int id2, int r, double p, int id3, bool d):
  br_id(id1),br_stop_id(id2),rid(r),pos(p), sid(id3), def(d){}
  BusStop(const BusStop& bs):
  br_id(bs.br_id),br_stop_id(bs.br_stop_id),rid(bs.rid),
  pos(bs.pos),sid(bs.sid), def(bs.def){}
  BusStop& operator=(const BusStop& bs)
  {
    br_id = bs.br_id;
    br_stop_id = bs.br_stop_id;
    rid = bs.rid;
    pos = bs.pos; 
    sid = bs.sid; 
    def = bs.def; 
    return *this; 
  }
  bool operator<(const BusStop& bs)const
  {
/*    if(rid > bs.rid) return true;
    else if(rid == bs.rid){
      if(pos > bs.pos) return true;
      else if(AlmostEqual(pos,bs.pos))return true;
      else return false; 
      
    }else return false; */

    if(rid < bs.rid) return true;
    else if(rid == bs.rid){
      if(pos < bs.pos) return true;
      else if(AlmostEqual(pos,bs.pos))return true;
      else return false; 
      
    }else return false; 

  }
  void Print()
  {
    cout<<"br_id "<<br_id<<" stop_id "<<br_stop_id
        <<" rid "<<rid<<" pos "<<pos<<" sid "<<sid<<endl; 
  }
}; 

struct BusStop_Ext:public BusStop{
  Point loc;
  bool start_small;
  BusStop_Ext(){}
  BusStop_Ext(int id1, int id2, double p, Point q, bool s):
  BusStop(id1,id2,0,p,0,true),loc(q), start_small(s){}
  BusStop_Ext(const BusStop_Ext& bse):
  BusStop(bse),loc(bse.loc), start_small(bse.start_small){}
  BusStop_Ext& operator=(const BusStop_Ext& bse)
  {
    BusStop::operator=(bse);
    loc = bse.loc;
    start_small = bse.start_small; 
    return *this; 
  }
  void Print()
  {
    BusStop::Print(); 
    cout<<"loc "<<loc<<" start_small "<<start_small<<endl; 
  }
  bool operator<(const BusStop_Ext& bse) const
  {
    return loc < bse.loc; 
  
  }
};

struct GP_Point{
  int rid;
  double pos1; 
  double pos2;
  Point loc1;
  Point loc2;
  GP_Point(){}
  GP_Point(int r, double p1,double p2, Point q1, Point q2):
  rid(r),pos1(p1),pos2(p2),loc1(q1),loc2(q2){}
  GP_Point(const GP_Point& gp_p):
  rid(gp_p.rid),pos1(gp_p.pos1),pos2(gp_p.pos2),
  loc1(gp_p.loc1),loc2(gp_p.loc2){}
  GP_Point& operator=(const GP_Point& gp_p)
  {
    rid = gp_p.rid;
    pos1 = gp_p.pos1;
    pos2 = gp_p.pos2;
    loc1 = gp_p.loc1;
    loc2 = gp_p.loc2; 
    return *this;
  }
  void Print()
  {
    cout<<"rid "<<rid<<" pos1 "<<pos1<<" pos2 "<<pos2
         <<"loc1 "<<loc1<<" loc2 "<<loc2<<endl; 
  }

};

/*
structure that is used to create bus network 

*/
struct BusRoute{
  Network* n;
  Relation* rel1; //store sec_id,cell_id,cnt 
                  //also used for bus routes br_id, bus_route1,bus_route2 
                  //also used for bus stops, br_id, bus_stop_id,bus_stop2 
  BTree* btree;
  
  Relation* rel2;//store cell paris, start cell -- end cell 
                 //store bus stops for opeartor createbusstop3 
  
  unsigned int count;
  TupleType* resulttype;
  
  vector<GLine> bus_lines1; //gline
  vector<Line> bus_lines2; //line 
  
  vector<Point> start_gp;
  vector<Point> end_gp; 
  vector<Line> bus_sections1;
  vector<Line> bus_sections2; 
  vector<int> br_uid_list; 
  
  vector<BBox<2> > start_cells;
  vector<BBox<2> > end_cells; 
  vector<int> start_cell_id;
  vector<int> end_cell_id;
  vector<int> bus_route_type; 
  vector<bool> direction_flag; 
  vector<bool> bus_stop_flag; 
  ////////////bus stops structure////////////////////////
  vector<int> br_id_list;          //starts from 1 
  vector<int> br_stop_id;
  vector<GPoint> bus_stop_loc_1; 
  vector<Point> bus_stop_loc_2; 
  vector<int> sec_id_list; 
  vector<double> bus_stop_loc_3; 
  vector<bool> startSmaller; //used for simpleline
  vector<int> stop_loc_id_list; //different spatial locations for bus stops 
  /////////////////////////////////////
  
  BusRoute(Network* net,Relation* r1,BTree* b):
  n(net),rel1(r1),btree(b)
  {count = 0;resulttype=NULL;}
  
  BusRoute(Network* net,Relation* r1,BTree* b,Relation* r2):
  n(net), rel1(r1), btree(b), rel2(r2)
  {count = 0;resulttype=NULL;}
  
  
  BusRoute(){count=0;resulttype = NULL;}
  ~BusRoute(){if(resulttype != NULL) delete resulttype;}
  
  ////////////rough description of bus routes/////////////////////////////
  void CreateRoute1(int attr1,int attr2,int attr3,int attr4); 
  void BuildRoute(vector<Section_Cell>& cell_list3,
                     vector<Section_Cell> cell_list1, int attr1, int bus_no);
  void BuildRoute_Limit(vector<Section_Cell>& cell_list3,
                     vector<Section_Cell> cell_list1, int attr1, 
                     int bus_no, unsigned int limit_no);
  int FindEndCell(Section_Cell& start_cell,
                  vector<Section_Cell>& cell_list, float dist_val); 
  void ConvertGLine(GLine* gl1, GLine* gl2); 
  /////////////////////////////create bus routes//////////////////////////
  void CreateRoute2(int attr,int attr1,int attr2,int attr3); 
  void ConnectCell(int attr,int from_cell_id,int end_cell_id, 
                   int route_type, int seed);
  /////////////////////refine bus routes////////////////////////////////
  void RefineBusRoute(int, int, int, int, int, int);
  int FilterBusRoute(GLine* gl1, GLine* gl2, int id1, int id2);
  float BusRouteInRoad(int);
  /////////////////////////////create bus stops/////////////////////
  void CreateBusStop1(int attr1,int attr2,int attr3, int attr4); 
  void CreateStops(int br_id, GLine* gl, Line* l, int route_type); 
  bool FindNextStop(vector<SectTreeEntry> sec_list,
                    unsigned int& last_sec_index,double& last_sec_start,
                    double& last_sec_end, double& last_sec_gp_pos,
                    double next_stop_dist, 
                    double dist_to_jun, vector<bool> start_from); 
  void CreateBusStop2(int attr1,int attr2,int attr3); 
  void MergeBusStop1(vector<BusStop>& temp_list); 
  void CreateBusStop3(int attr,int attr1,int attr2,int attr3); 
  void GetSectionList(GLine* gl,vector<SectTreeEntry>&, vector<bool>&);
  void FindDownSection(double,vector<SectTreeEntry>,int sec_index,
                       const double dist_val,
                       vector<SectTreeEntry>&,vector<bool>);
  void FindUpSection(double,vector<SectTreeEntry>,int sec_index,
                       const double dist_val,
                       vector<SectTreeEntry>&,vector<bool>);
  void MergeBusStop2(vector<SectTreeEntry>,vector<SectTreeEntry>,
                     vector<BusStop>&, int cur_index, int attr);
  ///////////////////////translate bus route//////////////////////////////
  void CreateRoute3(int attr1, int attr2, int attr3, int w);
  void CalculateUpandDown(SimpleLine* l1, SimpleLine* l2, bool sm);
  
  void ComputeLine(vector<Point>& point_list, Line* l);
  
  void CalculateStartSmaller(vector<BusStop>& bus_stop_list,int,int,
                                vector<SectTreeEntry>& sec_list,
                                vector<bool>& start_from,
                                vector<double>& dist_list,SimpleLine* sl);
  ///////////////////change the position of bus stops///////////////////
  /////////////////////change the representation for bus stops////////////
  void CreateBusStop4(int attr_a,int attr_b,int attr1,int attr2,
                      int attr3,int attr4); 
  void GetInterestingPoints(HalfSegment hs,Point loc, vector<MyPoint>& list,
                            Line* l1, Line* l2);
  bool MyAtPoint(SimpleLine* sl, Point& loc, bool startSmaller, 
                 double& res, double dist_delta);                            
  //////////////////////set the up and down value/////////////////////////
  void CreateRoute4(int attr1, int attr2, int attr3, int attr4, 
                    int attr_a, int attr_b);
  void CreateBusStop5(int attr,int attr1,int attr2,
                      int attr3,int attr4, int attr5); 
  
};

/*
get the road section density. use it to calculate the time schedule of each
bus route 

*/

struct Pos_Speed{
  double pos;
  double speed_val; 
  Pos_Speed(){}
  Pos_Speed(double p, double s):pos(p),speed_val(s){}
  Pos_Speed(const Pos_Speed& ps):pos(ps.pos),speed_val(ps.speed_val){}
  Pos_Speed& operator=(const Pos_Speed& ps)
  {
    pos = ps.pos;
    speed_val = ps.speed_val;
    return *this;
  }
  void Print()
  {
    cout<<"pos "<<pos<<" speed "<<speed_val<<endl; 
  }
};

struct RoadDenstiy{
  Network* n;
  Relation* rel1; //1. density relation  2. bus route relation
  Relation* rel2; //1. bus route relation 2. street data with speed limit  
  Relation* rel3; // brspeed relation 
  
  BTree* btree; 
  BTree* btree_a;//build on bus stops relation 
  BTree* btree_b;//build on bus route speed relation 
  
  unsigned int count;
  TupleType* resulttype;
  
  vector<int> br_id_list; 
  vector<int> traffic_no;
  vector<Periods> duration1;
  vector<Periods> duration2; 
  vector<double> time_interval; //minute 
  vector<double> time_interval2; //minute for daytime bus (Monday and Sunday)
  
  
  vector<double> br_pos;
  vector<double> speed_limit; 
  
  vector<Line> br_subroute; 
  vector<bool> br_direction;//up and down  
  vector<bool> startSmaller; //for simpleline 
  vector<int> segment_id_list; //the relationship betwee bus stop id 
  
  vector<Point> start_loc_list;
  vector<int> bus_stop_id_list; 
  
  vector<MPoint> bus_trip; 
  vector<string> trip_type;//type: night or daytime 
  vector<string> trip_day;//monday or sunday 
  vector<int> schedule_id_list; //schedule id 1, 2, ,3 
  
  vector<Point> bus_stop_loc; 
  vector<Instant> schedule_time; 
  vector<int> unique_id_list; 
  vector<double> schedule_interval;
  
  //for bus route speed relation  
  enum BR_SPEED{BR_ID = 0, BR_POS, BR_SPEED,BR_SPEED_SEG}; 
  //for bus route segment speed relation
  enum BR_SEGMENTD_SPEED{BR_ID1,BUS_DIRECTION,SUB_ROUTE_LINE,
                         SPEED_LIMIT,START_SMALLER,START_LOC,SEGMENT_ID};
  //for night bus schedule
  enum NIGHT_SCHEDULE{BR_ID2,DURATION1,DURATION2,BR_INTERVAL};
  
  //for daytime bus schedule 
  enum DAY_SCHEDULE{BR_ID3,DURATION_1,DURATION_2,BR_INTERVAL1,BR_INTERVAL2};
  
  ///////////////////////bus stops relation///////////////////////////
  enum BR_STOP{BR_ID4 = 0,BR_UID,BUS_STOP_ID,BUS_LOC,BUS_POS,STOP_DIRECTION};

  //////////////////////moving bus relation///////////////////////////
  enum MO_BUS{BR_ID5 = 0, MO_BUS_DIRECTION,BUS_TRIP,BUS_TYPE,
              BUS_DAY,SCHEDULE_ID}; 

  RoadDenstiy(){count=0;resulttype = NULL;}
  RoadDenstiy(Network* net,Relation* r1,Relation* r2,BTree* bt):
  n(net),rel1(r1),rel2(r2),btree(bt)
  {count=0;resulttype = NULL;}
  RoadDenstiy(Network* net,Relation* r1,Relation* r2,Relation* r3,BTree* bt):
  n(net),rel1(r1),rel2(r2),rel3(r3),btree(bt)
  {count=0;resulttype = NULL;}
  RoadDenstiy(Relation* r1,Relation* r2, Relation* r3,BTree* b1,BTree* b2):
  rel1(r1),rel2(r2), rel3(r3), btree_a(b1), btree_b(b2)
  {count=0;resulttype = NULL;}
  
  ~RoadDenstiy(){if(resulttype != NULL) delete resulttype;}

  void GetNightRoutes(int attr1, int attr2, int attr_a, int attr_b,
                                Periods*, Periods*);
  void SetTSNightBus(int attr1,int attr2, int attr3, Periods*, Periods*);
  void SetTSDayTimeBus(int attr1,int attr2, int attr3, Periods*, Periods*);
  double CalculateTimeSpan1(Periods* t, Periods* p1,
                         Interval<Instant>& span,int index);
  void CalculateTimeSpan2(Periods* p1,Interval<Instant>& span,double m);
  void CalculateTimeSpan3(Periods* t, Periods* p1,
                         Interval<Instant>& span,
                         int index, double interval);

  void SetBRSpeed(int attr1, int attr2, int attr, int attr_sm); 
  void CreateSegmentSpeed(int attr1, int attr2, int attr3, int attr4,
                          int attr_a, int attr_b);
  void CalculateRouteSegment(SimpleLine* sl, vector<Pos_Speed> br_speed_list, 
                             vector<BusStop_Ext> bus_stop_list, bool sm);
  ////////////////////////create moving bus//////////////////////////////
  void CreateNightBus();
  void CreateMovingBus(int br_id,Periods* peri,
                       double time_interval, bool daytime);    
  void CreateUp(int br_id,Periods* peri,
                double time_interval, bool daytime);
  void CreateDown(int br_id,Periods* peri,
                  double time_interval, bool daytime);
  //traverse halfsegmet to create moving points, from index small to big
  void CreateBusTrip1(MPoint*,vector<MyHalfSegment>, Instant&, 
                      double, UPoint&);
  //traverse halfsegmet to create moving points, from index big to small 
  void CreateBusTrip2(MPoint*,vector<MyHalfSegment>, Instant&, 
                      double, UPoint&);
  void CopyBusTrip(int br_id,bool direction, MPoint* mo, 
                   Periods*, double, bool daytime, int scheduleid);
  void CreateDayTimeBus(); 
  //////add night or daytime, and Sunday or Monday/////////////////
  void AddTypeandDay(MPoint* mo, bool daytime);
  ////////////////////////create time table/////////////////////////////////
  void CreateTimeTable();
  void CreateLocTable(vector<BusStop_Ext> bus_stop_list_new,int count_id);
  ///////////////////////Compact Storage ///////////////////////////////
  void CreateTimeTable_Compact(Periods*,Periods*);
  void CreateLocTable_Compact(vector<BusStop_Ext>,int count_id,
                              Periods*,Periods*);
  bool SameDay(UPoint& up, Periods* peri1);
  void CreatTableAtStopNight(vector<MPoint>& mo_list, Point& loc, 
                                   int br_id, int stop_id, bool dir,
                                   Periods* night1, Periods* night2,
                                   int count_id);
  void CreatTableAtStop(vector<MPoint> mo_list, Point& loc,
                                   int br_id, int stop_id, bool dir,
                                   int count_id); 
  void GetTimeInstantStop(MPoint& mo, Point loc, Instant& arrove_t); 
};


/*
To create UBahn trains 

*/
struct UBTrainTrip{
  int line_id;
  bool direction;
  MPoint train_trip;
  UBTrainTrip(){}
  UBTrainTrip(int id,bool d,MPoint tr):line_id(id),direction(d),train_trip(tr){}
  UBTrainTrip(const UBTrainTrip& train_tr):
  line_id(train_tr.line_id),direction(train_tr.direction),
  train_trip(train_tr.train_trip){}
  UBTrainTrip& operator=(const UBTrainTrip& train_tr)
  {
    line_id = train_tr.line_id;
    direction = train_tr.direction;
    train_trip = train_tr.train_trip;
    return *this;
  }
  void Print()
  {
    cout<<"line id "<<line_id<<" direction "<<direction
        <<" trip "<<train_trip<<endl; 
  }
  
  bool operator<(const UBTrainTrip& ubtrain) const
  {
    Periods* peri1 = new Periods(0);
    Periods* peri2 = new Periods(0);
    this->train_trip.DefTime(*peri1);
    ubtrain.train_trip.DefTime(*peri2);
    
    Interval<Instant> periods1;
    Interval<Instant> periods2;
    peri1->Get(0, periods1);
    peri2->Get(0, periods2);
    
    delete peri1;
    delete peri2; 
    
    return periods1.start < periods2.start; 
  }

};

struct UBTrain{
  Relation* rel1;
  Relation* rel2;
  BTree* btree1; 
  
  unsigned int id_count; 
  vector<int> id_list;
  vector<int> line_id_list;
  vector<bool> direction_list;
  vector<MPoint> train_trip; 
  vector<int> schedule_id_list; 
  
  vector<Point> stop_loc_list; 
  vector<int> stop_id_list; 
  
  vector<Instant> schedule_time;
  vector<int> loc_id_list; 

  vector<Periods> duration; //the whole life time for one day 
  vector<double> schedule_interval; //time interval for each bus trip 
  
  UBTrain(){count = 0;resulttype = NULL;}
  UBTrain(Relation* r):rel1(r),count(0),resulttype(NULL){}
  UBTrain(Relation* r1,Relation* r2,BTree* b):
  rel1(r1), rel2(r2), btree1(b), count(0), resulttype(NULL){}
  ~UBTrain(){if(resulttype != NULL) delete resulttype;}

  //////////    for UBahn Trips ///////////////////////////////////////
  enum UBAHN_TRIN{T_ID,T_LINE,T_UP,T_TRIP,T_SCHEDULE};
  /////////////// Train Stops ///////////////////////////////////////
  enum UBAHN_STOP{T_LINEID,T_STOP_LOC,T_STOP_ID}; 
  
  unsigned int count;
  TupleType* resulttype;
  
  void CreateUBTrains(int,int,int,Periods*);
  void CreateUBTrainTrip(vector<UBTrainTrip> trip_list, Periods* peri); 
  void CreateTrainTrip(vector<UBTrainTrip>, Periods*);
  void CopyTrainTrip(int start_pos, int end_pos,int line_id, bool d);
  void CreateUBTrainStop(int,int,int);
  void CreateTimeTable();
  void CreateLocTable(vector<BusStop_Ext> station_list_new,int count_id);
  ////////////////  Compact Storage of Time Tables //////////////
  void CreateTimeTable_Compact();
  void CreateLocTable_Compact(vector<BusStop_Ext> station_list_new,
                              int count_id);
  void TimeTableCompact(vector<MPoint>&,Point,int,int,bool,int);
  void GetTimeInstantStop(MPoint& mo, Point loc, Instant& st);
};


#endif

