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
class Mode_RTree;

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

struct Seq_Mode{
  MInt seq_tm;
  bool status;
  Seq_Mode():seq_tm(0), status(false){}
  Seq_Mode(MInt& in):seq_tm(in), status(false){}
  Seq_Mode(const Seq_Mode& sm):seq_tm(sm.seq_tm), status(sm.status){}

  Seq_Mode& operator=(const Seq_Mode& sm)
  {
    seq_tm.Clear();
    seq_tm.SetDefined(sm.seq_tm.IsDefined());
    for(int i = 0;i < sm.seq_tm.GetNoComponents();i++){
      UInt u;
      sm.seq_tm.Get(i, u);
      seq_tm.Add(u);
    }
    status = sm.status;
    return *this;
  }
  bool Status(){return status;}
  inline void Update(double st, double et, int m);
  inline void CheckStatus(vector<long> seq_tm);
  void Print()
  {
      cout<<seq_tm;
  }

};

struct LocPair;

struct LocRef;
struct PairState;

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
  
  static string GenmoUnitsInfoExt;
  enum GenmoUnitsExt{GM_TRAJ_ID_EXT = 0, GM_BOX_EXT, GM_MODE_EXT, GM_REF,
                     GM_SUBTRIP_EXT, GM_ID_EXT};
  
  static string GenMOBench_Query;
  enum GenMOBench_QINFO{BENCH_QID = 0, BENCH_M, BENCH_QT, 
                        BENCH_REFID, BENCH_QPOINT};
  
  static string GenmoRelExtInfo;
  enum GenmoRelExt{GENMOEXT_OID = 0, GENMOEXT_TRIP1, GENMOEXT_TRIP2,
                GENMOEXT_DEF, GENMOEXT_M, GENMOEXT_UNIDEX};
				
  static string QueryTMPathRelInfo;
  enum QueryTMPathRel{QTMPath_REGID, QTMPath_BID, QTMPath_GeoData,
                      QTMPath_Path};
  				
  static string Bench12FloorRel;
  enum Bench12FloorInfo{B_FLOOR_OID = 0};
  
  static string Bench12BusStopsRel;
  enum Bench12BusStopInfo{B_STOP_BID = 0, B_STOP_GEO};
  
  static string GenmoRangeQuery2;
  enum GMORangeQuery2{GM_RQ2_TRAJ_ID = 0, GM_RQ2_MODE, GM_RQ2_SUBTRIP,
                      GM_RQ2_DIVPOS, GM_RQ2_BOX, GM_RQ2_ID};
  
  QueryTM(){ count = 0; resulttype = NULL;} 
  ~QueryTM(){if(resulttype != NULL) delete resulttype;}

  vector<int> oid_list;
  vector<Periods> time_list;
  vector<Rectangle<3> > box_list;
  vector<Rectangle<2> > box_list2;
  
  vector<int> tm_list;
  vector<int> ref_id_list;
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
  unsigned int indoor_id_no;

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
  void CollectFree_0(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
 void CollectWalk_0(int& i, int oid, GenMO* mo1, MPoint* mo2, double, int &pos);

  void CollectFree_1(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
 void CollectWalk_1(int& i, int oid, GenMO* mo1, MPoint* mo2, double, int &pos);

 
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
  ///////////////////////////////////////////////////////////////////////////
  /////////////////a sequence of modes /////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  void SeqMode_Filter1(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                       vector<bool> m_bit_list);
  void SeqMode_Filter2(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                       Relation* units_rel, vector<bool> m_bit_list);
  void SeqMode_Filter3(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box,
                       Relation* units_rel, vector<bool> m_bit_list);
  void SeqMode_Filter4(TM_RTree<3,TupleId>* tmrtree, Rectangle<3> box, 
                      vector<bool> bit_pos, Relation* units_rel);
  void SeqMode_Refinement(Rectangle<3> query_box, vector<bool> bit_pos, 
                          Relation* units_rel, vector<long> seq_tm,
                          vector<bool> m_bit_list);
  inline void MakeTMUnit(MInt& mp_tm, double st, double et, int m);
  /////////////////////////////////////////////////////////////////////////
  //////////////// simple(baseline) method for testing correctness/////////
  ////////////////////////////////////////////////////////////////////////
  void RangeQuery(Relation* rel1, Relation* rel2);
  void Sin_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box,
                             int m);
  void Mul_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box,
                             int m);
  void Seq_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box, 
                      vector<long> seq_tm);

  bool ContainMode1_Sin(MReal* mode_index, int m);
  bool ContainMode1_Mul(MReal* mode_index, vector<bool> bit_pos, int);
  
  bool ContainMode2_Sin(MReal* mode_index, Interval<Instant>& t, int m);
  void ContainMode2_Mul(map<int, Traj_Mode>& res_traj, MReal* mode_index, 
                        Interval<Instant>& t, vector<bool> bit_pos, int, int);

  void ContainMode3_Seq(map<int, Seq_Mode>& res_traj, MReal* mode_index,
                        vector<bool> bit_pos, Interval<Instant>& t, 
                        int, vector<long>);
  void RangeQuery2(R_Tree<4, TupleId>*, Relation* rel1, Relation* rel2);	
  void Sin_RangeQuery4DRTree(R_Tree<4, TupleId>* rtree, Relation* rel1, 
			 Periods* peri, Rectangle<2>* q_box,int m);
  void Mul_RangeQuery4DRTree(R_Tree<4, TupleId>* rtree, Relation* rel1, 
		Periods* peri, Rectangle<2>* q_box,
                             long m);
  void Seq_RangeQuery4DRTree(R_Tree<4, TupleId>* rtree, Relation* rel1, 
			Periods* peri, Rectangle<2>* q_box,
                             vector<long>& m_list);
  inline bool ModeExist(int, vector<int>&);	
  /////////////////////////////////////////////////////////////////////
  //////////////////////mode rtree////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  void DecomposeGenmo2(Relation*, double, Space*);
  void CreateMTuple_2(int oid, GenMO* mo1, MPoint* mo2, double);
  void CollectWalk_2(int& i, int oid, GenMO* mo1, MPoint* mo2, double, int&pos);
 void CollectIndoor2(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
  void CollectFree_2(int& i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
 void CollectBusMetro2(int& i, int oid, int m, GenMO* mo1, MPoint* mo2,int&pos);
   void CollectCBT2(int&i, int oid, int m, GenMO* mo1, MPoint* mo2, int& pos);
  void GenMOBenchQuery(Mode_RTree*, Relation*, Relation*, Space* sp,
					   Relation*, BTree*);
  void GenMOBenchQuery9(Mode_RTree*, Relation*, Relation*, Space* sp,
					   Relation*, BTree*, Relation*);
  void CollectTaxiMovement2(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
			Interval<Instant> q_time, Rectangle<2> q_box,
						    set<int>& res_list);
  void CollectIndoorMovement(Mode_RTree* modertree, Relation* rel1, 
			Interval<Instant> q_time, vector<int> id_list,
			map<int, int>& res_list, Space* sp);	
  void GenMOBenchQuery13(Mode_RTree*, Relation*, Relation*, Space* sp,
			   Relation*, BTree*, Relation*, Relation*);
  void CollectBikeMovement(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
			  Interval<Instant> q_time, set<int>& res_list);
  void DistanceMPoint( MPoint& p1, MPoint& p2, MReal& result);
  void GenMOBenchQuery12(Mode_RTree*, Relation*, Relation*, Space* sp,
			  Relation*, BTree*, Relation*, Relation*, Relation*);
  void CollectIndoorMovement2(Mode_RTree* modertree, Relation* rel1, 
					Relation* rel2, Space* sp, 
					Relation* rel3, BTree*, 
					Relation* rel4, vector<int>&);	
  void CollectBusMovement2(TM_RTree<3,TupleId>* tmrtree_bus, 
					   Relation* rel1, Relation* rel2,
						   Space* sp,
					Relation* rel_bs1, Relation* rel_bs2, 
						   set<int>& bus_candi);	
  int EqualToStop(vector<Point> bs_list, Point p);
  ///////////////////////////////////////////////////////////////////////
  //////////////// benchmark quries using modertree//////////////////////
  ///////////////////////////////////////////////////////////////////////
  int GetBusBit(int ref_id, Space* sp, vector<int>& br_id, map<int,int>&);
  void GetMetroBit(int ref_id, Space* sp, vector<int>& br_id, 
				  map<int,int>&, vector<int>&);
  void BenchQuery5(Mode_RTree* modertree, Relation* rel, string m,
			Interval<Instant> q_time, int q_dur, Space* sp);
  void BenchQuery5_Bus(TM_RTree<3,TupleId>* tmrtree, Relation* rel, 
						      Interval<Instant> q_time, 
						      int ref_id, Space* sp);
  void BenchQuery5_Metro(TM_RTree<3,TupleId>* tmrtree, Relation* rel, 
						      Interval<Instant> q_time, 
						      int ref_id, Space* sp);
  int SetRoadBit(Mode_RTree* modertree, int road_id, Space* sp, 
				 Rectangle<2>& bbox);
  void BenchQuery5_CTB(Mode_RTree* modertree, Relation* rel, 
	                  Interval<Instant> q_time, 
					  int ref_id, 
					  Space* sp,
	string m);
  void BenchQuery8(Mode_RTree* modertree, Relation* rel, 
	Interval<Instant> q_time, Space* sp,Relation* rel2,
					Relation* rel3, BTree* genmobtree);
  void BenchQuery10(Mode_RTree* modertree, Relation* rel1,  
					Interval<Instant> q_time, 
					Relation* rel2, Space* sp);
  void InsertElem(MPoint* mp, multimap<int, LocRef>&, vector<Point>, int, 
				  int, double);		
  void BenchQuery11(Mode_RTree* modertree, Relation* rel, 
				Interval<Instant> q_time, Relation* rel2, 
					Relation* rel3, 
		BTree* genmobtree);				
  void BenchQuery14(Mode_RTree* modertree, Relation* rel, string m,
					Interval<Instant> q_time, 
					Relation* rel2, double q_dur);
  void BenchQuery15(Mode_RTree* modertree, Relation* rel, 
					Interval<Instant> q_time, 
					Space* sp,Relation* rel2);
  void BenchQuery16(Mode_RTree* modertree, 
		Relation* rel, 
					Interval<Instant> q_time, 
					Space* sp,Relation* rel2,
					Relation* rel3, BTree* genmobtree);

  void BenchQuery17(Mode_RTree* modertree, Relation* rel, string m,
					Interval<Instant> q_time, 
					double q_dur);
  void BenchQuery18(Mode_RTree* modertree, Relation* rel, string m,
					Interval<Instant> q_time, 
					Relation* rel2, Space* sp);

  void BenchQuery18_Bus(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
						Interval<Instant> q_time, 
						Relation* rel2, 
	Space* sp);							  
  void BenchQuery18_Metro(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
						      Interval<Instant> q_time, 
						      Relation* rel2, 
						  Space* sp);
  int CheckRouteId(int route_ref, vector<int> bus_route_id1, 
				   vector<int> bus_route_id2);
  void BenchQuery19(Mode_RTree* modertree, Relation* rel, 
				Interval<Instant> q_time, Relation* rel2,
					Relation* rel3, BTree* genmobtree);
  void SetIndoorBit(Mode_RTree* modertree, map<int, int>& bit_map, 
		Space* sp, vector<int>, Rectangle<2>&);	
  void CollectBusMovement(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
						 Interval<Instant> q_time, 
		set<int>& res_list);
  void CollectMetroMovement(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
						 Interval<Instant> q_time, 
						 set<int>& res_list);
  void CollectTaxiMovement(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
						 Interval<Instant> q_time, 
		set<int>& res_list);
  void CollectWalkMovement(TM_RTree<3,TupleId>* tmrtree, Relation* rel1, 
		Interval<Instant> q_time, Relation* rel2,
	   vector<int>& res_list);
  void BenchQuery20(Mode_RTree* modertree, Relation* rel, string m,
		Interval<Instant> q_time, int q_dur, Space* sp);

  void BenchQuery20_Bus(TM_RTree<3,TupleId>* tmrtree, Relation* rel, 
						      Interval<Instant> q_time, 
						      int ref_id, Space* sp);
  void BenchQuery20_Metro(TM_RTree<3,TupleId>* tmrtree, Relation* rel, 
						      Interval<Instant> q_time, 
						      int ref_id, Space* sp);

};

struct BRID_TripID{
  int br_id;
  int bus_id;
  BRID_TripID():br_id(0), bus_id(0){}
  BRID_TripID(int a, int b):br_id(a), bus_id(b){}
  BRID_TripID(const BRID_TripID& br_trip):
  br_id(br_trip.br_id), bus_id(br_trip.bus_id){}
  BRID_TripID& operator=(const BRID_TripID& br_trip)
  {
	br_id = br_trip.br_id;
	bus_id = br_trip.bus_id;
	return *this;
  }
  bool operator<(const BRID_TripID& br_trip) const
  {
	if(br_id < br_trip.br_id) return true;
    else if(br_id == br_trip.br_id){
      if(bus_id < br_trip.bus_id) return true;
      else return false; 
    }else return false; 
  }
  bool operator==(const BRID_TripID& br_trip) const
  {
	if(br_id == br_trip.br_id && bus_id == br_trip.bus_id) return true;
	return false;
  }

};

/*
id and the counter

*/
struct ID_Count{
  int id;
  int count;
  ID_Count():id(0),count(0){}
  ID_Count(int a, int b):id(a), count(b){}
  ID_Count(const ID_Count& id_c):id(id_c.id), count(id_c.count){}
  ID_Count& operator=(const ID_Count& id_c)
  {
	id = id_c.id;
	count = id_c.count;
	return *this;
  }
  bool operator<(const ID_Count& id_c)const
  {
//	return count < id_c.count;
	return count > id_c.count;
  }

};

/*
bus route transfer

*/
struct Transfer{
  int id1, id2; //1 -- true, 0 -- false
  int64_t t1, t2;
  Point loc;
  bool status;
  Transfer():id1(0), id2(0), status(false){}
  Transfer(int a, int b, int64_t ta, int64_t tb):
  id1(a), id2(b), t1(ta), t2(tb){}
  Transfer(const Transfer& input){
	id1 = input.id1;
	id2 = input.id2;
	t1 = input.t1;
	t2 = input.t2;
	loc = input.loc;
	status = input.status;
  }
  Transfer& operator=(const Transfer& input)
  {
	id1 = input.id1;
	id2 = input.id2;
	t1 = input.t1;
	t2 = input.t2;
	loc = input.loc;
	status = input.status;
	return *this;
  }
  bool GetStatus(){return status;}
  void SetStatus(bool b){status = b;}
  void Print()
  {
  cout<<"route A: "<<id1<<" route B: "<<id2<<" t1: "<<t1<<" t2: "<<t2<<endl;
  }
};

/*
a pair of locations

*/
struct LocPair{
  int64_t t1, t2;
  Point loc1, loc2;
  LocPair():t1(0), t2(0){}
  LocPair(int64_t a, int64_t b, Point x, Point y):
  t1(a), t2(b), loc1(x), loc2(y){}
  LocPair(const LocPair& input){
	t1 = input.t1;
	t2 = input.t2;
	loc1 = input.loc1;
	loc2 = input.loc2;
  }
  LocPair& operator=(const LocPair& input)
  {
	t1 = input.t1;
	t2 = input.t2;
	loc1 = input.loc1;
	loc2 = input.loc2;
	return *this;
  }
  void Print()
  {
	cout<<"start: "<<t1<<" "<<loc1
	    <<" end: "<<t2<<" "<<loc2<<endl;
  }
};

struct PairState{
  bool b1, b2;
  bool status;
  PairState():b1(false), b2(false), status(false){}
  PairState(bool a, bool b):b1(a), b2(b), status(false){
	if(a && b) status = true;
  }
  PairState(const PairState& ps):b1(ps.b1), b2(ps.b2), status(ps.status){}
  PairState& operator=(const PairState& ps)
  {
	b1 = ps.b1;
	b2 = ps.b2;
	status = ps.status;
	return *this;
  }
  bool GetStatus(){return status;}
  void SetStatus(bool b){status = b;}
};

/*
time of referencing to an referenced object

*/
struct LocRef{
  int ref_id;
  int64_t t;
  Point loc;
  bool status;
  LocRef():ref_id(0), t(0){}
  LocRef(int id, int64_t time, Point p):ref_id(id), t(time), loc(p){}
  LocRef(const LocRef& input):ref_id(input.ref_id), 
  t(input.t), loc(input.loc), status(input.status){}
  LocRef& operator=(const LocRef& input)
  {
	ref_id = input.ref_id;
	t = input.t;
	loc = input.loc;
	status = input.status;
	return *this;
  }
  bool GetStatus(){return status;}
  void SetStatus(bool b){status = b;}
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
