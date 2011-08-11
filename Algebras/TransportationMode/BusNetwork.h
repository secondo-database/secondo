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
#include "GeneralType.h"
#include "RoadNetwork.h"

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
  unsigned int sid; 
  bool def;
  BusStop(){}
  BusStop(int id1, int id2, int r, double p, unsigned int id3, bool d):
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
  int oid;
  GP_Point(){}
  GP_Point(int r, double p1,double p2, Point q1, Point q2):
  rid(r),pos1(p1),pos2(p2),loc1(q1),loc2(q2), oid(0){}
  GP_Point(const GP_Point& gp_p):
  rid(gp_p.rid),pos1(gp_p.pos1),pos2(gp_p.pos2),
  loc1(gp_p.loc1),loc2(gp_p.loc2), oid(gp_p.oid){}
  GP_Point& operator=(const GP_Point& gp_p)
  {
    rid = gp_p.rid;
    pos1 = gp_p.pos1;
    pos2 = gp_p.pos2;
    loc1 = gp_p.loc1;
    loc2 = gp_p.loc2; 
    oid = gp_p.oid;
    return *this;
  }
  void Print()
  {
    cout<<"rid "<<rid<<" pos1 "<<pos1<<" pos2 "<<pos2
         <<" loc1 "<<loc1<<" loc2 "<<loc2<<" oid "<<oid<<endl;
  }

};
class Bus_Stop;
class Bus_Route; 

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
  vector<Bus_Stop> bus_stop_list; 
  vector<Point> bus_stop_geodata; 
  
  vector<Bus_Route> bus_route_list; 
  ////////////////////////////////////////////////////////////////
  BusRoute(Network* net,Relation* r1,BTree* b):
  n(net),rel1(r1),btree(b)
  {count = 0;resulttype=NULL;}
  
  BusRoute(Network* net,Relation* r1,BTree* b,Relation* r2):
  n(net), rel1(r1), btree(b), rel2(r2)
  {count = 0;resulttype=NULL;}
  
  
  BusRoute(){count=0;resulttype = NULL;}
  ~BusRoute(){if(resulttype != NULL) delete resulttype;}
  
  static string StreetSectionCellTypeInfo;
  static string BusRoutesTmpTypeInfo;
  static string NewBusRoutesTmpTypeInfo;
  static string FinalBusRoutesTypeInfo;
  static string BusStopTemp1TypeInfo;
  
  ////////////rough description of bus routes/////////////////////////////
  void CreateRoute1(int attr1,int attr2,int attr3,int attr4); 
  void BuildRoute(vector<Section_Cell>& cell_list3,
                     vector<Section_Cell> cell_list1, int attr1, 
                  int bus_no, int type);
  void BuildRoute_Limit(vector<Section_Cell>& cell_list3,
                     vector<Section_Cell> cell_list1, int attr1, 
                     int bus_no, unsigned int limit_no);
  int FindEndCell(Section_Cell& start_cell,
                  vector<Section_Cell>& cell_list, float dist_val); 

  bool ConvertGLine(GLine* gl1, GLine* gl2); 
  /////////////////////////////create bus routes//////////////////////////
  void CreateRoute2(Space*, int attr,int attr1,int attr2,int attr3); 
  void ConnectCell(RoadGraph*, int attr,int from_cell_id,int end_cell_id, 
                   int route_type, int seed);
  /////////////////////refine bus routes////////////////////////////////
  void RefineBusRoute(int, int, int, int, int, int);
  int FilterBusRoute(GLine* gl1, GLine* gl2, int id1, int id2);
  float BusRouteInRoad(int);
  /////////////////////////////create bus stops/////////////////////
  void CreateBusStop1(int attr1,int attr2,int attr3, int attr4,
                      Relation*, BTree*); 
  void CreateStops(int br_id, GLine* gl, Line* l, int route_type); 
  bool FindNextStop(vector<SectTreeEntry> sec_list,
                    unsigned int& last_sec_index,double& last_sec_start,
                    double& last_sec_end, double& last_sec_gp_pos,
                    double next_stop_dist, 
                    double dist_to_jun, vector<bool> start_from); 

  void CheckBusStopZC(unsigned int cur_size, Relation*,BTree*); 
  ////////////////////////////////////////////////////////////////////
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
  ///////////////////////////////////////////////////////////////////////
  ///////////////bus stops and bus routes////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  void GetBusStops();
  void GetBusRoutes(); 
  void CreateRoutes(vector<TupleId>& tid_list, int br_id,
                            SimpleLine* sl, bool small, bool d); 

  void GetPosOnSL(SimpleLine* sl, Point loc, double& pos);

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
  vector<Bus_Stop> bs_list;
  vector<int> bs_uoid_list; 
  
  vector<Point> jun_loc_list;
  vector<GPoint> gp_list;
  vector<int> rid_list;
  
  vector<int> jun_id_list1;
  vector<int> jun_id_list2;
  vector<GLine> gl_path_list;
  vector<SimpleLine> sline_path_list;
  
  static string night_sched_typeinfo;
  static string day_sched_typeinfo;
  
  static string bus_route_speed_typeinfo; 
  static string bus_stop_typeinfo;
   
  static string mo_bus_typeinfo;
  static string bus_route_typeinfo;
  static string bus_route_old_typeinfo;
  
  
  static string rg_nodes_typeinfo;
  
  //for bus route speed relation  
  enum BR_SPEED{BR_ID = 0, BR_POS, BR_SPEED, BR_SPEED_SEG}; 
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

  ///////////////////bus routes relation////////////////////////////////
  enum BR_ROUTE{BR_ID6 = 0, BR_GEODATA, BR_ROUTE_TYPE, BR_RUID, 
                BR_DIRECTION, BR_STARTSMALLER}; 

  //////////////////////////////////////////////////////////////////////
  ////////////////initial bus routes relation//////////////////////////
  ///////////////with gline information///////////////////////////////
  enum BR_ROUTE_OLD{BR_ID_OLD = 0, BR_GEODATA1, BR_GEODATA2, BR_START_LOC, 
                BR_END_LOC, BR_ROUTE_TYPE_OLD}; 


  /////////////////////////////////////////////////////////////////////
  ///////////////road graph information//////////////////////////////
  ////////////////////////////////////////////////////////////////////
  enum RG_NODES{RG_N_JUN_ID,RG_N_GP,RG_N_P, RG_RID};


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
  ////////////////////////////////////////////////////////////////////////
  ////////////////construct road graph///////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  void GetRGNodes();
  void GetRGEdges1(Relation* rel, R_Tree<2,TupleId>* rtree);
  void DFTraverse(Relation*, R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Point& loc, vector<int>& _oid_list);

  void GetRGEdges2(Relation* rel);

};

/*
!!!!!!!!!!!!! Name BusStop has been used already !!!!!!!!!!!!!!!!!!!!!!!!!!

*/
class Bus_Stop:public Attribute{
  public:
     Bus_Stop(); 
     Bus_Stop(bool def, unsigned int id1 = 0, unsigned int id2 = 0, 
              bool d = true);
     Bus_Stop(const Bus_Stop& bs);
     ~Bus_Stop(){}
     Bus_Stop& operator=(const Bus_Stop& bs);

     void Set(unsigned int id1, unsigned int id2, bool d){
        SetDefined(true);
        br_id = id1;
        stop_id = id2;
        up_down = d;
     }
     static void* Cast(void* addr); 
     unsigned int GetId() const{return br_id;}
     unsigned int GetStopId() const{return stop_id;}
     bool GetUp() const {return up_down;}

     inline size_t Sizeof() const{return sizeof(*this);}
     inline bool IsEmpty() const{return !IsDefined();}
     int Compare(const Attribute* arg) const{return 0;}

     inline bool Adjacent(const Attribute* arg)const{return false;}
     Bus_Stop* Clone() const {return new Bus_Stop(*this);}
     size_t HashValue() const{return (size_t)0;}
     void CopyFrom(const Attribute* right){*this = *(const Bus_Stop*)right;}
     int GetUOid();
     bool operator==(const Bus_Stop& bs);
     ostream& Print(ostream& os) const; 
     static const string BasicType(){
       return "busstop";
     }

  private:
    unsigned int br_id;
    unsigned int stop_id; 
    bool up_down; 
};

ListExpr BusStopProperty();
ListExpr OutBusStop( ListExpr typeInfo, Word value ); 
Word InBusStop( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 
bool OpenBusStop(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
bool SaveBusStop(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value);
Word CreateBusStop(const ListExpr typeInfo);
void DeleteBusStop(const ListExpr typeInfo, Word& w); 
void CloseBusStop( const ListExpr typeInfo, Word& w ); 
Word CloneBusStop( const ListExpr typeInfo, const Word& w );
int SizeOfBusStop(); 
bool CheckBusStop( ListExpr type, ListExpr& errorInfo );
ostream& operator<<(ostream& o, const Bus_Stop& bs); 


/*
for bus routes, it records the segid in a route, start position in dbarray,
  number of halfsegments and start smaller value (sline)

*/
struct BR_Elem{
  unsigned int br_seg_id;
  unsigned int start_pos;
  unsigned int no;
  BR_Elem(){}
  BR_Elem(unsigned int seg_id, unsigned int pos, unsigned int num, bool s):
  br_seg_id(seg_id), start_pos(pos), no(num){}
  BR_Elem(const BR_Elem& br_elem):
  br_seg_id(br_elem.br_seg_id), start_pos(br_elem.start_pos), no(br_elem.no){}
  BR_Elem& operator=(const BR_Elem& br_elem)
  {
    br_seg_id = br_elem.br_seg_id;
    start_pos = br_elem.start_pos;
    no = br_elem.no;
    return *this; 
  }
  void Print()
  {
    cout<<"seg id "<<br_seg_id<<" start_pos "<<start_pos
        <<"num "<<no<<endl; 
  }
};


class Bus_Route:public StandardSpatialAttribute<2>{
  public:
    Bus_Route(); 
    Bus_Route(const int initsize):StandardSpatialAttribute<2>(true),
    elem_list(initsize), seg_list(initsize), br_id(0), up_down(true){}
    
    Bus_Route(unsigned int id1, bool d):
    StandardSpatialAttribute<2>(true),
    elem_list(0), seg_list(0), br_id(id1), up_down(d){}
    Bus_Route(const Bus_Route& br); 
    Bus_Route& operator=(const Bus_Route& br); 

    ~Bus_Route(){}


    inline size_t Sizeof() const{return sizeof(*this);}
    int Compare(const Attribute* arg) const{return 0;}

    inline bool Adjacent(const Attribute* arg)const{return false;}
    Bus_Route* Clone() const {return new Bus_Route(*this);}
    size_t HashValue() const{return (size_t)0;}
    void CopyFrom(const Attribute* right){*this = *(const Bus_Route*)right;}

    inline int Size() const {return elem_list.Size();}
    inline bool IsEmpty() const{return Size() == 0;}
    static void* Cast(void* addr); 
    void Add(SimpleLine* sl, int count);
    void Add2(HalfSegment hs, int count);
    void Get(int i, SimpleLine& sl); 
    void GetGeoData(SimpleLine& sl);

    void GetBusStopGeoData(Bus_Stop* bs, Point* p);
    void GetMetroStopGeoData(Bus_Stop* bs, Point* p);

    void GetElem(int i, BR_Elem& elem);
    void GetSeg(int i, HalfSegment& hs);
    int SegSize(){return seg_list.Size();}

    double Length();
    const Rectangle<2> BoundingBox(const Geoid* geoid=0) const;
    double Distance(const Rectangle<2>& r, const Geoid* geoid=0)const
    {
        return BoundingBox().Distance(r);
    }
    void StartBulkLoad();
    void EndBulkLoad();
    unsigned int GetId() const { return br_id;}
    bool GetUp() const { return up_down;}
    static const string BasicType(){
       return "busroute";
    }
    /////////////very important two functions////////////////////
   ////////especially genrange is an attribute in a relation/////
  inline int NumOfFLOBs() const { 
    return 2;
  }
  inline Flob* GetFLOB(const int i) { 
     if(i < 1)
      return &elem_list;
    else 
      return &seg_list;
  }
  
  private:
    DbArray<BR_Elem> elem_list;//one bus segment 
    DbArray<HalfSegment> seg_list;
    unsigned int br_id;
    bool up_down;
};

ListExpr BusRouteProperty();
bool OpenBusRoute(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value); 
bool SaveBusRoute(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value); 
Word CreateBusRoute(const ListExpr typeInfo); 

void DeleteBusRoute(const ListExpr typeInfo, Word& w); 
void CloseBusRoute( const ListExpr typeInfo, Word& w ); 
Word CloneBusRoute( const ListExpr typeInfo, const Word& w );
int SizeOfBusRoute(); 
bool CheckBusRoute( ListExpr type, ListExpr& errorInfo ); 
ListExpr OutBusRoute( ListExpr typeInfo, Word value );
Word InBusRoute( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 

class BusGraph; 
/*
Bus Network 

*/
class BusNetwork{
  public:
  BusNetwork();
  BusNetwork(bool d, unsigned int i);
  BusNetwork(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);

  ~BusNetwork();
  
  static string BusStopsTypeInfo;
  static string BusRoutesTypeInfo;
  static string BusStopsInternalTypeInfo;
  static string BusStopsBTreeTypeInfo; 
  static string BusStopsRTreeTypeInfo; 

  static string BusRoutesBTreeTypeInfo; 
  static string BusRoutesBTreeUOidTypeInfo; 
  ////////br id, unique bus route id, large number ///////////////
  static string BusTripsTypeInfo; 
  static string BusTripBTreeTypeInfo;


  bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
  static BusNetwork* Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo);
  enum BN_INODE{NODE_BS1 = 0, NODE_BS2};
  enum BN_BS{BN_ID1 = 0, BN_BS, BS_U_OID, BS_GEO};//internal representation 
  enum BN_BR{BN_ID2 = 0, BN_BR, BN_BR_OID}; //representation 
  enum BN_BTP{BN_BUSTRIP = 0, BN_BUSTRIP_MP, BN_REFBR_OID, BN_BUS_OID};

  void Load(unsigned int i, Relation* r1, Relation* r2, Relation* r3); 
  void LoadStops(Relation* r);
  void LoadRoutes(Relation* r); 
  void LoadBuses(Relation* r);
  static void* Cast(void* addr);
  bool IsDefined() const { return def;}
  unsigned int GetId() const {return bn_id;}
  Relation* GetBS_Rel(){return stops_rel;}
  Relation* GetBR_Rel(){return routes_rel;}
  Relation* GetBT_Rel(){return bustrips_rel;}
  double GetMaxSpeed(){return max_bus_speed;}
  R_Tree<2,TupleId>* GetBS_RTree() { return rtree_bs;}
  
  void GetBusStopGeoData(Bus_Stop* bs, Point* p);
  void SetGraphId(int g_id);
  bool IsGraphInit(){return graph_init;}
  unsigned int GraphId(){return graph_id;}
  BusGraph* GetBusGraph();
  void CloseBusGraph(BusGraph* bg);

  int GetMOBus_Oid(Bus_Stop* bs, Point*, Instant& t);
  int GetMOBus_MP(Bus_Stop* bs, Point*, Instant t, MPoint& mp);
  void GetMOBUS(int oid, MPoint& mp, int& br_uoid);
  void GetBusRouteGeoData(int br_uoid, SimpleLine& sl);

  private:
    bool def; 
    unsigned int bn_id;
    bool graph_init; 
    unsigned int graph_id; 
    double max_bus_speed;//maximum speed of all moving buses (heuristic value)
    unsigned int min_br_oid;//smallest bus route oid
    unsigned int min_bt_oid; //smallest bus trip oid 
    
    Relation* stops_rel; //a relation for bus stops
    BTree* btree_bs; //a btree on bus stops
    Relation* routes_rel;  //a relaton for bus routes
    BTree* btree_br; //a btree on bus routes

    BTree* btree_bs_uoid; //a btree on bus stops for unique oid 
    R_Tree<2,TupleId>* rtree_bs; //an rtree on bus stops

    BTree* btree_br_uoid; //a btree on bus routes for unique oid 

    Relation* bustrips_rel; //a relation for moving buses 

    BTree* btree_trip_oid;  //a btree on bus trips on unique oid 
    BTree* btree_trip_br_id; //a btree on bus trips on bus route id 
};

ListExpr BusNetworkProperty();
ListExpr OutBusNetwork( ListExpr typeInfo, Word value );
Word InBusNetwork( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct ); 
bool SaveBusNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
bool OpenBusNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
void DeleteBusNetwork(const ListExpr typeInfo, Word& w); 
Word CreateBusNetwork(const ListExpr typeInfo);
void CloseBusNetwork( const ListExpr typeInfo, Word& w );
Word CloneBusNetwork( const ListExpr typeInfo, const Word& w ); 
int SizeOfBusNetwork();
bool CheckBusNetwork( ListExpr type, ListExpr& errorInfo ); 


struct MyPoint_Tid:public MyPoint{
    int tid; 
    MyPoint_Tid(){}
    MyPoint_Tid(const Point& p1, double d1, int i):
    MyPoint(p1,d1),tid(i){}
    MyPoint_Tid(const MyPoint_Tid& mpt):MyPoint(mpt), tid(mpt.tid){}
    MyPoint_Tid& operator=(const MyPoint_Tid& mpt)
    {
        MyPoint::operator=(mpt);
        tid = mpt.tid; 
        return *this; 
    }
     void Print()
    {
        cout<<" loc " <<loc<<" dist "<<dist<<" tid "<<tid<<endl; 
    }
};

class BusGraph; 

struct BN{

  BusNetwork* bn;
  vector<Bus_Stop> bs_list; 
  vector<Bus_Route> br_list; 
  
  vector<GenLoc> genloc_list;
  vector<Point> geo_list; 
  unsigned int count;
  TupleType* resulttype;
  
  static string BusStopsPaveTypeInfo;
  enum BusStopPave{BN_BUSSTOP = 0, BN_PAVE_LOC1, BN_PAVE_LOC2, BN_BUSLOC}; 
  static string BusTimeTableTypeInfo; 
  enum BusStopTimeTable{BN_T_GEOBS = 0, BN_T_BS, BN_T_P, 
                        BN_T_S,BN_T_LOC_ID, BN_T_BUS_UOID}; 

  vector<Bus_Stop> bs_list1;
  vector<Bus_Stop> bs_list2;
  vector<Line> path_list; 
  vector<SimpleLine> path_sl_list; 
  vector<int> type_list; 
  vector<SimpleLine> sub_path1;
  vector<SimpleLine> sub_path2; 
  vector<SimpleLine> path2_sl_list; 
  
  vector<int> bs_uoid_list; 
  vector<double> schedule_interval;
  vector<Periods> duration;
  vector<MPoint> bus_trip_list; 
  vector<double> time_cost_list; 
  
  
  BN(BusNetwork* n);
  ~BN();
  void GetStops();
  void GetRoutes();
  
  void MapBSToPavements(R_Tree<2,TupleId>* rtree, Relation* pave_rel, int w);
  void MapToPavment(Bus_Stop& bs1, Bus_Stop& bs2, R_Tree<2,TupleId>* rtree, 
                    Relation* pave_rel, int w);
  void DFTraverse(R_Tree<2,TupleId>* rtree, Relation* rel, 
                           SmiRecordId adr, Line* l,
                           vector<MyPoint_Tid>& it_p_list);
  ///////////////for each bus stop, find the neighbor bus stops///////////////
  void BsNeighbors1(DualGraph* dg, VisualGraph* vg, Relation* rel1,
                   Relation* rel2, R_Tree<2,TupleId>* rtree);
  void DFTraverse2(R_Tree<2,TupleId>* rtree, Relation* rel, 
                 SmiRecordId adr, Point* l, vector<int>& neighbor_list, double);
  bool FindNeighbor(int tid1, int tid2, DualGraph* dg, VisualGraph* vg,
                   Relation* rel1, Relation* rel2, double dist, Line* res);

  
  void BsNeighbors2();
  void DFTraverse3(R_Tree<2,TupleId>* rtree, Relation* rel,
                   SmiRecordId adr, Point* loc, vector<int>& neighbor_list);

  /////////////////////////////////////////////////////////////////////
  void GetAdjNodeBG1(BusGraph*, int);
  void GetAdjNodeBG2(BusGraph*, Bus_Stop* bs);
  
  void BsNeighbors3(Relation*, Relation*, BTree*);
  void ConnectionOneRoute(Relation* table_rel, vector<int> tid_list, 
                            Relation* mo_rel, BTree* btree_mo); 
 
}; 

/*
bus network graph: three kinds of edges 
1. neighbor bus stops connected by path in the pavements
the path between bus stops and their pavements (subpath1, subpath2)
this kind of connection can be defined as in free space 
2. bus stops with the same spatial location but belong to different routes
3. moving buses from one bus stop to its next one in the same route 
for the last stop, it has no neigbor stops

three kinds of trip planning:shortest distance, minimum travelling time,
minimum number of bus transfers 

*/
class BusGraph{
  public:
    static string NodeTypeInfo;
    static string NodeInternalTypeInfo;
    static string NodeBTreeTypeInfo; 

    static string EdgeTypeInfo1; 
    static string EdgeTypeInfo2;
    static string EdgeTypeInfo3; 

    enum BGNodeTypeInfo{BG_NODE = 0, BG_NODE_GEO, BG_NODE_UOID}; 
    enum BGEdgeTypeInfo1{BG_UOID1 = 0, BG_E_BS1,BG_E_BS2,BG_PATH1,
                         BG_SUBPATH1, BG_SUBPATH2, BG_PATH2, BG_E_BS2_TID};
    enum BGEdgeTypeInfo2{BG_UOID2 = 0, BG_E2_BS1, BG_E2_BS2, BG_E2_BS2_TID};
    enum BGEdgeTypeInfo3{BG_UOID3 = 0, BG_E3_BS1, BG_E3_BS2, BG_LIFETIME, 
                         BG_SCHEDULE, BG_PATH3, BG_TIMECOST, BG_E3_BS2_TID};

    BusGraph();
    BusGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect);
    BusGraph(SmiRecord&, size_t&, const ListExpr);

    ~BusGraph();
    static ListExpr BusGraphProp();
    static bool CheckBusGraph(ListExpr type, ListExpr& errorInfo);
    static int SizeOfBusGraph();
    static void* CastBusGraph(void* addr);
    static Word CloneBusGraph(const ListExpr typeInfo, const Word& w); 
    static void CloseBusGraph(const ListExpr typeInfo, Word& w); 
    static Word CreateBusGraph(const ListExpr typeInfo);
    static void DeleteBusGraph(const ListExpr typeInfo, Word& w); 
    static Word InBusGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect);
    static ListExpr OutBusGraph(ListExpr typeInfo, Word value); 
    static bool SaveBusGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
    static bool OpenBusGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value); 
    static BusGraph* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);

    ListExpr Out(ListExpr typeInfo); 
    bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo); 

    void Load(int, Relation*, Relation*, Relation*, Relation*);
    void LoadEdge1(Relation* edge1); 
    void LoadEdge2(Relation* edge2); 
    void LoadEdge3(Relation* edge3); 

    void FindAdj1(int node_id, vector<int>& list);
    void FindAdj2(int node_id, vector<int>& list);
    void FindAdj3(int node_id, vector<int>& list);
    int GetBusStop_Tid(Bus_Stop* bs);
    
    unsigned int bg_id;
    double min_t; //minimum time of all bus stops time duration 
    Relation* node_rel;
    BTree* btree_node; //btree on bus stop unique id 

    Relation* edge_rel1;//edges: pavement path 
    DbArray<int> adj_list1;
    DbArray<ListEntry> entry_adj_list1;

    Relation* edge_rel2;//edges: with the same spatial location 
    DbArray<int> adj_list2;
    DbArray<ListEntry> entry_adj_list2;


    Relation* edge_rel3; //edges: moving buses in the same route 
    DbArray<int> adj_list3;
    DbArray<ListEntry> entry_adj_list3;

};

/*
for shortest path searching  in bus network and metro network

*/
struct BNPath_elem:public Path_elem{
  double weight;
  double real_w;
  SimpleLine path;
  int tm;
  bool valid; //false: transfering without moving 
  
  bool b_w;
  double w;//special case for time waiting for the bus or metro
  BNPath_elem():path(0){}
  BNPath_elem(int p, int c, int t, double w1, double w2, SimpleLine& sl,
              int m, bool b = true):Path_elem(p, c, t), weight(w1), real_w(w2), 
              path(sl), tm(m), valid(b), b_w(false), w(0){}
  BNPath_elem(const BNPath_elem& wp):Path_elem(wp),
            weight(wp.weight),real_w(wp.real_w),
            path(wp.path), tm(wp.tm), valid(wp.valid), b_w(wp.b_w), w(wp.w){}
  BNPath_elem& operator=(const BNPath_elem& wp)
  {
    Path_elem::operator=(wp);
    weight = wp.weight;
    real_w = wp.real_w;
    path = wp.path;  
    tm = wp.tm;
    valid = wp.valid; 
    b_w = wp.b_w;
    w = wp.w;
    return *this;
  }
  void SetW(double d)
  {
    if(d > 0.0){
      b_w = true;
      w = d; 
    }
  }
  bool operator<(const BNPath_elem& ip) const
  {
    return weight > ip.weight;
  }

  void Print()
  {
    cout<<" tri_index " <<tri_index<<" realweight "<<real_w
        <<" weight "<<weight;
    if(tm >= 0)cout<<" tm "<<str_tm[tm]<<endl;
    else cout<<"tm: none"<<endl;
  }

};

/*
for shortest path in bus transfer. two kinds of comparison 
  because the number of bus transfer does not change so frequently and in 
  big extent 

*/

struct BNPath_elem2:public Path_elem{
  int weight;
  double real_w;
  SimpleLine path;
  int tm;
  bool valid; //false: transfering without moving 
  
  bool b_w;
  double w;//special case for time waiting for the bus 
  BNPath_elem2():path(0){}
  BNPath_elem2(int p, int c, int t, int w1, double w2, SimpleLine& sl,
              int m, bool b = true):Path_elem(p, c, t), weight(w1), real_w(w2), 
              path(sl), tm(m), valid(b), b_w(false), w(0){}
  BNPath_elem2(const BNPath_elem2& wp):Path_elem(wp),
            weight(wp.weight),real_w(wp.real_w),
            path(wp.path), tm(wp.tm), valid(wp.valid), b_w(wp.b_w), w(wp.w){}
  BNPath_elem2& operator=(const BNPath_elem2& wp)
  {
    Path_elem::operator=(wp);
    weight = wp.weight;
    real_w = wp.real_w;
    path = wp.path;  
    tm = wp.tm;
    valid = wp.valid; 
    b_w = wp.b_w;
    w = wp.w;
    return *this;
  }
  void SetW(double d)
  {
    if(d > 0.0){
      b_w = true;
      w = d; 
    }
  }
  bool operator<(const BNPath_elem2& ip) const
  {
    if(weight > ip.weight) return true;
    else if(weight < ip.weight) return false; 
    else if(weight == ip.weight){
      return real_w > ip.real_w; 
    }else assert(false);
  }

  void Print()
  {
    cout<<" tri_index " <<tri_index<<" realweight "<<real_w
        <<" weight "<<weight;
    if(tm >= 0)cout<<" tm "<<str_tm[tm]<<endl;
    else cout<<"tm: none"<<endl;
  }
};

/*
navigation for bus network 

*/
struct BNNav{
  BusNetwork* bn;
  
  vector<SimpleLine> path_list; 
  vector<string> tm_list; 
  vector<string> bs1_list;
  vector<string> bs2_list; 
  vector<Periods> peri_list; 
  vector<double> time_cost_list; 

  vector<Bus_Stop> bs_list;
  vector<Point> bs_geo_list; 
  
  vector<GenMO> genmo_list;
  vector<MPoint> mp_list;
  
  vector<Bus_Stop> bs_list1;
  vector<Bus_Stop> bs_list2;
  vector<int> br_id_list;
  
  unsigned int count;
  TupleType* resulttype;
  
  int type; 
  BNNav(){count = 0; resulttype = NULL;}
  BNNav(BusNetwork* n):bn(n)
  { count = 0; 
    resulttype = NULL;
  }

  ~BNNav(){if(resulttype != NULL) delete resulttype;}

  void ShortestPath_Length(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);
  void ShortestPath_Time(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);
  void ShortestPath_Transfer(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);

  void ShortestPath_Time2(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);
  void ShortestPath_Transfer2(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);

  //////////////////////optimization on filtering edges////////////////////
  void ShortestPath_TimeNew(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);
  void ShortestPath_TransferNew(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);

  
  void ShortestPath_TimeNew2(Bus_Stop* bs1, Bus_Stop* bs2, Instant*);
  
  void InitializeQueue1(Bus_Stop* bs1, Bus_Stop* bs2, 
                            priority_queue<BNPath_elem>& path_queue, 
                            vector<BNPath_elem>& expand_queue, 
                            BusNetwork* bn, BusGraph* bg,
                            Point&, Point&);
  void InitializeQueue2(Bus_Stop* bs1, Bus_Stop* bs2, 
                            priority_queue<BNPath_elem>& path_queue, 
                            vector<BNPath_elem>& expand_queue, 
                            BusNetwork* bn, BusGraph* bg,
                            Point&, Point&);

  void InitializeQueue3(Bus_Stop* bs1, Bus_Stop* bs2, 
                            priority_queue<BNPath_elem2>& path_queue, 
                            vector<BNPath_elem2>& expand_queue, 
                            BusNetwork* bn, BusGraph* bg,
                            Point&, Point&);

  ///////////////////////moving buses (mpoint) to genmo////////////////////
  void BusToGenMO(Relation* r1, Relation* r2, BTree* btree);
  void MPToGenMO(MPoint* mp, unsigned int br_id, bool dir, Relation* br_rel,
                 BTree* btree);

};
//////////////////////////////////////////////////////////////////////////
///////////////////////underground trains////////////////////////////////
/////////////////////////////////////////////////////////////////////////

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

struct UBhan_Id_Geo{
  int lineid;
  Line geodata;
  UBhan_Id_Geo():geodata(0){}
  UBhan_Id_Geo(int id, const Line& l):lineid(id), geodata(l){}
  UBhan_Id_Geo(const UBhan_Id_Geo& ub):
  lineid(ub.lineid), geodata(ub.geodata){}
  UBhan_Id_Geo& operator=(const UBhan_Id_Geo& ub)
  {
    lineid = ub.lineid;
    geodata = ub.geodata;
    return *this; 
  }
};

/*
metro stop data

*/
struct UBahn_Stop{
  int line_id;
  Point loc;
  int stop_id;
  bool d;
  int tid;
  UBahn_Stop(){}
  UBahn_Stop(int l, Point& p, int s, bool dir, int id):
  line_id(l), loc(p), stop_id(s), d(dir), tid(id){}
  UBahn_Stop(const UBahn_Stop& ub_stop):
  line_id(ub_stop.line_id), loc(ub_stop.loc), 
  stop_id(ub_stop.stop_id), d(ub_stop.d), tid(ub_stop.tid){}
  UBahn_Stop& operator=(const UBahn_Stop& ub_stop)
  {
    line_id = ub_stop.line_id;
    loc = ub_stop.loc;
    stop_id = ub_stop.stop_id;
    d = ub_stop.d;
    tid = ub_stop.tid;
    return *this;
  }
  bool operator<(const UBahn_Stop& ub_stop) const
  {
    return loc < ub_stop.loc;
  }
  void Print()
  {
    cout<<"line id "<<line_id<<" stop id "<<stop_id<<" dir "<<d
        <<" loc "<<loc<<" tid "<<tid<<endl;
  }

};

class MetroNetwork;

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
  
  vector<SimpleLine> geodata; 
  
  vector<GenMO> genmo_list; 
  vector<MPoint> mp_list;
  vector<int> br_id_list;
  

  vector<int> ms_tid_list1;
  vector<int> ms_tid_list2;
  vector<Periods> period_list;
  vector<double> time_cost_list;



  static string TrainsTypeInfo;
  static string UBahnLineInfo;
  static string UBahnTrainsTypeInfo; 
  static string TrainsStopTypeInfo;
  static string TrainsStopExtTypeInfo; 
  static string UBahnTrainsTimeTable;


  UBTrain(){count = 0;resulttype = NULL;}
  UBTrain(Relation* r):rel1(r),count(0),resulttype(NULL){}
  UBTrain(Relation* r1,Relation* r2,BTree* b):
  rel1(r1), rel2(r2), btree1(b), count(0), resulttype(NULL){}
  ~UBTrain(){if(resulttype != NULL) delete resulttype;}

  //////////    for UBahn Trips ///////////////////////////////////////
  enum UBAHN_TRAIN{T_ID,T_LINE,T_UP,T_TRIP,T_SCHEDULE};
  /////////////// Train Stops ///////////////////////////////////////
  enum UBAHN_STOP{T_LINEID,T_STOP_LOC,T_STOP_ID};

  enum UBAHN_STOP_EXT{T_LINEID_EXT, T_STOP_LOC_EXT,
                      T_STOP_ID_EXT, T_DIRECTION};

  //////////////////// Trains and UBahn Line////////////////////
  ///////////////////convert trains to genmo///////////////////////
  enum UBAHN_TRAINS{TRAIN_LINE_ID = 0, TRAIN_LINE_DIR, TRAIN_TRIP};
  enum UBAHN_LINE{UB_LINE_ID = 0, UB_LINE_OID, UB_LINE_GEODATA}; 
  enum UBAHN_TIMETABLE{UB_STATION_LOC_T,UB_LINE_ID_T, UB_STOP_ID_T,
                       UB_DIR_T,UB_PERIODS_T,UB_INTERVAL_T,UB_LOC_ID_T};
  
  unsigned int count;
  TupleType* resulttype;

  void CreateTimeTable();
  void CreateLocTable(vector<BusStop_Ext> station_list_new,int count_id);
  ////////////////  Compact Storage of Time Tables //////////////
  void CreateTimeTable_Compact();
  void CreateLocTable_Compact(vector<BusStop_Ext> station_list_new,
                              int count_id);
  void TimeTableCompact(vector<MPoint>&,Point,int,int,bool,int);
  void GetTimeInstantStop(MPoint& mo, Point loc, Instant& st);

 ////////////////////////////////////////////////////////////////////////////
 ////////////////// covert berlin trains to generic moving objects//////////
 //////////////////////////////////////////////////////////////////////////
 void TrainsToGenMO(); 
 void MPToGenMO(MPoint* mp, GenMO* mo, int l_id); 

};

/*
create Metro: create metro routes and stops from road data 

*/
struct MetroStruct{

  MetroStruct(){count = 0;resulttype = NULL;}
  ~MetroStruct(){if(resulttype != NULL) delete resulttype;}
  
  unsigned int count;
  TupleType* resulttype;

  static string MetroRouteInfo; 
  static string MetroTripTypeInfo_Com;

  enum MetroRouteInfo{MR_ID,MR_ROUTE,MR_OID};
  enum MetroTripInfoCom{M_GENMO_COM,M_MP_COM,M_R_ID_COM,M_DIR_COM,M_R_OID_COM,
                        M_OID_COM};

  vector<int> id_list;
  vector<bool> dir_list;
  vector<int> mr_oid_list;
  vector<Bus_Route> mroute_list; 
  vector<Region> cell_reg_list1;
  vector<Region> cell_reg_list2;

  vector<Bus_Stop> mstop_list;
  vector<Point> stop_geo_list;

  vector<GenMO> mtrip_list1;
  vector<MPoint> mtrip_list2;
  
  
  vector<int> ms_tid_list1;
  vector<int> ms_tid_list2;
  vector<Periods> period_list;
  vector<double> time_cost_list;
  vector<double> schedule_interval;
  vector<SimpleLine> geodata;
   
  vector<GenLoc> loc_list1;
  vector<Point> loc_list2; 
  vector<Region> neighbor_list;
  

 /////////////////////////////////////////////////////////////////////////////
 //////////////////create railway routes and stops/////////////////////////
 //////////////////not use the data from berlintest or berlinmod////////////
 ////////////////////////////////////////////////////////////////////////////
 void CreateMRoute(DualGraph* dg);
 bool BuildMetroRoute(vector<int> path_list, DualGraph* dg, int count);

 void CreateMStop(Relation*);
 
 ////////////////////////////////////////////////////////////
 ////////////create moving metros////////////////////////////
 ////////////////////////////////////////////////////////////
 void CreateMTrips(Relation*, Periods*);
 void CreateMetroUp(vector<MyHalfSegment>& seg_list, 
                    Periods* peri,int mr_id, int mr_oid, bool d);
 void CreateMetroDown(vector<MyHalfSegment>& seg_list, 
                      Periods* peri,int mr_id, int mr_oid, bool d);

 void CopyMetroTrip(GenMO* genmo, MPoint* mo, Periods* peri, 
                    Instant st,int,int,bool);
 void CopyTripOneDayBefore();
 /////////////////////////////////////////////////////////////////////////
 ////////////////create stops and routes relation////////////////////
 /////////////////////////////////////////////////////////////////////
 void MsNeighbors1(Relation* r);
 void MsNeighbors2(MetroNetwork* mn, Relation* timetable, BTree* btree1,
                   Relation* metrotrip, BTree* btree2);
 void ConnectionOneRoute(UBahn_Stop* ms_stop, Relation* timetable, 
                         BTree* btree1, Relation* metrotrip, 
                         BTree* btree2, int Neighbor_tid, Point* Neighbor_loc);
 //////////////////////////////////////////////////////////////////////////
 ////////////map metro stops to pavement areas/////////////////////////////
 /////////////////////////////////////////////////////////////////////////
 void MapMSToPave(Relation*, Relation*, R_Tree<2,TupleId>*);
 void DFTraverse(R_Tree<2,TupleId>* rtree, Relation* rel,
                             SmiRecordId adr, Point* loc, 
                             vector<int>& tri_tid_list, double& min_dist);
};


class MetroGraph;

/*
metro network: underground trains 

*/
class MetroNetwork{
  public:
    MetroNetwork();
    MetroNetwork(bool d, unsigned int i);
    MetroNetwork(SmiRecord& valueRecord, size_t& offset, 
                 const ListExpr typeInfo);
    ~MetroNetwork();

    bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
    static MetroNetwork* Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo);

    bool IsDefined() const { return def;}
    unsigned int GetId() const {return mn_id;}
    Relation* GetMS_Rel(){return stops_rel;}
    Relation* GetMR_Rel(){return routes_rel;}
    Relation* GetMetro_Rel(){return metrotrips_rel;}

    static void* Cast(void* addr);
    void Load(unsigned int i, Relation* r1, Relation* r2, Relation* r3); 
    void LoadStops(Relation* r);
    void LoadRoutes(Relation* r);
    void LoadMetros(Relation* r);
    double GetMaxSpeed(){return max_metro_speed;}
    int GetMS_Stop_Neighbor(UBahn_Stop* ms_stop);

    R_Tree<2,TupleId>* GetMS_RTree() { return rtree_ms;}
    void SetGraphId(int g_id);

    bool IsGraphInit(){return graph_init;}
    unsigned int GraphId(){return graph_id;}
    MetroGraph* GetMetroGraph();
    void CloseMetroGraph(MetroGraph* mg);
    void GetMetroStopGeoData(Bus_Stop* ms, Point* p);
    int GetMOMetro_Oid(Bus_Stop* ms, Point*, Instant& t);
    int GetMOMetro_MP(Bus_Stop* bs, Point*, Instant t, MPoint& mp);

    ///////////////used for the real data, (ubahn converting)////////////////
    static string UBAHNStopsTypeInfo;
    static string UBAHNStopsBTreeTypeInfo;
    static string UBAHNStopsRTreeTypeInfo;

    static string UBAHNRoutesTypeInfo;
    static string UBAHNRotuesBTreeTypeInfo;

    enum UBAHN_STOP_INFO{UB_LINEID, UB_STOP_LOC,UB_STOP_ID, UB_DIRECTION};


    ////////////////////////////////////////////////////////////////////////

    static string MetroStopsTypeInfo; 
    static string MetroStopsBTreeTypeInfo;
    static string MetroStopsRTreeTypeInfo;

    static string MetroRoutesTypeInfo;
    static string MetroRoutesBTreTypeInfo;

    ////// mr id is the unique route id, large number /////
    static string MetroTripTypeInfo;
    static string MetroTypeBTreeTypeInfo;

    static string MetroPaveTypeInfo;

    enum METRO_STOP_INFO{M_STOP, M_STOP_GEO, M_R_ID};
    enum METRO_ROUTE_INFO{M_ROUTE_ID,M_ROUTE,M_R_OID};
    enum METRO_TRIP_INFO{M_TRIP_GENMO, M_TRIP_MP, M_REFMR_OID,M_TRIP_OID};

    enum METRO_PAVE_INFO{METRO_PAVE_LOC1, METRO_PAVE_LOC2,
                         METRO_PAVE_MS_STOP, METRO_PAVE_MS_STOP_LOC};

  private:

    bool def;
    unsigned int mn_id;/////metro network id
    bool graph_init; 
    unsigned int graph_id; 
    double max_metro_speed;//maximum speed of all moving buses (heuristic value)
    unsigned int min_mr_oid;//smallest metro route oid
    unsigned int min_mt_oid; //smallest metro trip oid 

    Relation* stops_rel; //a relation for metro stops
    BTree* btree_ms; //a btree on metro stops on line id 
    R_Tree<2,TupleId>* rtree_ms; //an rtree on metro stops

    Relation* routes_rel;  //a relaton for metro routes
    BTree* btree_mr; //a btree on metro routes line id
    BTree* btree_mr_uoid; //a btree on metro routes for unique oid 


    Relation* metrotrips_rel; //a relation for moving buses 
    BTree* btree_trip_br_id; //a btree on metro trips on  unique route linid
    BTree* btree_trip_oid;  //a btree on metro trips on unique oid 

};

bool SaveMetroNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
bool OpenMetroNetwork(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
ListExpr MetroNetworkProperty();
Word InMetroNetwork( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
ListExpr OutMetroNetwork( ListExpr typeInfo, Word value );
Word CreateMetroNetwork(const ListExpr typeInfo);
void DeleteMetroNetwork(const ListExpr typeInfo, Word& w);
void CloseMetroNetwork( const ListExpr typeInfo, Word& w );
Word CloneMetroNetwork( const ListExpr typeInfo, const Word& w );
int SizeOfMetroNetwork();
bool CheckMetroNetwork( ListExpr type, ListExpr& errorInfo );


/*
metro graph. 
nodes correspond to metro stops 
there are two kinds of edges: 
(1) two metro stops have the same spatial location in space
(2) two consequent metro stops are connected by moving metros
this graph is similar as bus graph

metro graph edges stores node tuple id 

*/
class MetroGraph{
  public:

    MetroGraph();
    MetroGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect);
    MetroGraph(SmiRecord&, size_t&, const ListExpr);
    ~MetroGraph();

    static string MGNodeTypeInfo;
    static string MGNodeBTreeTypeInfo; 
    static string MGEdge1TypeInfo;
    static string MGEdge2TypeInfo;

    enum MG_NODE_INFO{MG_NODE_STOP,MG_NODE_STOP_GEO,MG_NODE_MR_ID};
    enum MG_EDGE1_INFO{MG_EDGE1_TID1, MG_EDGE1_TID2};
    enum MG_EDGE2_INFO{MG_EDGE2_TID1, MG_EDGE2_TID2, MG_EDGE2_PERI,
                       MG_EDGE2_SCHED, MG_EDGE2_PATH, MG_EDGE2_TIME_COST};

    static ListExpr MetroGraphProp();
    static bool CheckMetroGraph(ListExpr type, ListExpr& errorInfo);
    static int SizeOfMetroGraph();
    static void* CastMetroGraph(void* addr);
    static Word CloneMetroGraph(const ListExpr typeInfo, const Word& w); 
    static void CloseMetroGraph(const ListExpr typeInfo, Word& w);
    static Word CreateMetroGraph(const ListExpr typeInfo);
    static void DeleteMetroGraph(const ListExpr typeInfo, Word& w);
    static Word InMetroGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect);
    static ListExpr OutMetroGraph(ListExpr typeInfo, Word value); 
    ListExpr Out(ListExpr typeInfo); 
    static bool SaveMetroGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
    bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo);
    static bool OpenMetroGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value); 
    static MetroGraph* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);
    unsigned int GetG_ID(){return mg_id;}

    void Load(int, Relation*, Relation*, Relation*);
    void LoadEdge1(Relation* edge1); 
    void LoadEdge2(Relation* edge2); 
    
    Relation* GetNode_Rel(){return node_rel;}
    Relation* GetEdge_Rel1(){return edge_rel1;}
    Relation* GetEdge_Rel2(){return edge_rel2;}
    
    unsigned int GetMG_ID(){return mg_id;}
    double GetMIN_T(){return min_t;}
    
    int GetMetroStop_Tid(Bus_Stop* ms);
    
    void FindAdj1(int node_id, vector<int>& list);
    void FindAdj2(int node_id, vector<int>& list);


  private:
    unsigned int mg_id;
    double min_t; //minimum time of all metro stops time duration 

    Relation* node_rel;
    BTree* btree_node; //btree on metro stop line id 

    Relation* edge_rel1;//edges: with the same spatial location 
    DbArray<int> adj_list1;
    DbArray<ListEntry> entry_adj_list1;

    Relation* edge_rel2;//edges: connected by moving metros 
    DbArray<int> adj_list2;
    DbArray<ListEntry> entry_adj_list2;

};

/*
query processing on the metro graph
navigation on the metro network

*/
struct MNNav{
  MetroNetwork* mn;
  
  vector<SimpleLine> path_list; 
  vector<string> tm_list; 
  vector<string> ms1_list;
  vector<string> ms2_list; 
  vector<Periods> peri_list; 
  vector<double> time_cost_list; 

  vector<Bus_Stop> ms_list;
  vector<Point> ms_geo_list; 
  
  vector<GenMO> genmo_list;
  vector<MPoint> mp_list;
  
  vector<Bus_Stop> ms_list1;
  vector<Bus_Stop> ms_list2;
  vector<int> mr_id_list;
  
  unsigned int count;
  TupleType* resulttype;
  
  MNNav(){count = 0; resulttype = NULL;}
  MNNav(MetroNetwork* n):mn(n)
  { count = 0; 
    resulttype = NULL;
  }


  ~MNNav(){if(resulttype != NULL) delete resulttype;}
  
  void ShortestPath_Time(Bus_Stop* ms1, Bus_Stop* ms2, Instant*);
  void InitializeQueue(Bus_Stop* ms1, Bus_Stop* ms2, 
                            priority_queue<BNPath_elem>& path_queue, 
                            vector<BNPath_elem>& expand_queue,
                            MetroNetwork* mn, MetroGraph* mg,
                            Point& start_p, Point& end_p);
};



#endif

