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

#include "BusNetwork.h"
#include "QueryTM.h"
#include <bitset>

/*
convert a genrange object to a 2D line in space or 3D line in a building 

*/
void QueryTM::GetLineOrLine3D(GenRange* gr, Space* sp)
{
  BusNetwork* bn = sp->LoadBusNetwork(IF_BUSNETWORK);
  
  for(int i = 0;i < gr->Size();i++){
    Line l(0);
    GenRangeElem grelem;
    gr->Get( i, grelem, l);
    int infra_type = sp->GetInfraType(grelem.oid);

    switch(infra_type){
      case IF_LINE:
//        cout<<"road network "<<endl;
        GetLineInRoad(grelem.oid, &l, sp);
        break;

      case IF_REGION:
//        cout<<"region based outdoor"<<endl;
        GetLineInRegion(grelem.oid, &l, sp);
        break; 

      case IF_FREESPACE:
//        cout<<"free space"<<endl;
        GetLineInFreeSpace(&l);
        break;

      case IF_BUSNETWORK:
//        cout<<"bus network"<<endl;
        GetLineInBusNetwork(grelem.oid, &l, bn);
        break;

      case IF_GROOM:
//        cout<<"indoor "<<endl;
        GetLineInGRoom(grelem.oid, &l, sp);
        break;

      default:
        assert(false);
        break;
    }
  }

  sp->CloseBusNetwork(bn);

}

/*
get the overall line in road network 

*/
void QueryTM::GetLineInRoad(int oid, Line* l, Space* sp)
{
  Network* rn = sp->LoadRoadNetwork(IF_LINE);
  Tuple* route_tuple = rn->GetRoute(oid);
  SimpleLine* sl = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
  Rectangle<2> bbox = sl->BoundingBox();
  route_tuple->DeleteIfAllowed();
  sp->CloseRoadNetwork(rn);

  Line* newl = new Line(0);
  newl->StartBulkLoad();
  
  int edgeno = 0;
  for(int i = 0;i < l->Size();i++){
    HalfSegment hs1;
    l->Get(i, hs1);
    if(!hs1.IsLeftDomPoint())continue;
    Point lp = hs1.GetLeftPoint();
    Point rp = hs1.GetRightPoint();
    Point newlp(true, lp.GetX() + bbox.MinD(0), lp.GetY() + bbox.MinD(1));
    Point newrp(true, rp.GetX() + bbox.MinD(0), rp.GetY() + bbox.MinD(1));
    HalfSegment hs2(true, newlp, newrp);
    hs2.attr.edgeno = edgeno++;
    *newl += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *newl += hs2;
  }
  
  newl->EndBulkLoad();
  
  line_list1.push_back(*newl);
  delete newl;

/*  Line3D* l3d = new Line3D(0);
  line3d_list.push_back(*l3d);
  delete l3d;*/

}


/*
get the overall line in region based outdoor 

*/
void QueryTM::GetLineInRegion(int oid, Line* l, Space* sp)
{
  Pavement* pm = sp->LoadPavement(IF_REGION);
  DualGraph* dg = pm->GetDualGraph();
  Rectangle<2> bbox = dg->rtree_node->BoundingBox();
  pm->CloseDualGraph(dg);
  sp->ClosePavement(pm);

  Line* newl = new Line(0);
  newl->StartBulkLoad();

  int edgeno = 0;
  for(int i = 0;i < l->Size();i++){
    HalfSegment hs1;
    l->Get(i, hs1);
    if(!hs1.IsLeftDomPoint())continue;
    Point lp = hs1.GetLeftPoint();
    Point rp = hs1.GetRightPoint();
    Point newlp(true, lp.GetX() + bbox.MinD(0), lp.GetY() + bbox.MinD(1));
    Point newrp(true, rp.GetX() + bbox.MinD(0), rp.GetY() + bbox.MinD(1));
    HalfSegment hs2(true, newlp, newrp);
    hs2.attr.edgeno = edgeno++;
    *newl += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *newl += hs2;
  }
  
  newl->EndBulkLoad();
  
  line_list1.push_back(*newl);
  delete newl;

//   Line3D* l3d = new Line3D(0);
//   line3d_list.push_back(*l3d);
//   delete l3d;

}

/*
get the overall line in free space 

*/
void QueryTM::GetLineInFreeSpace(Line* l)
{
  line_list1.push_back(*l);

//   Line3D* l3d = new Line3D(0);
//   line3d_list.push_back(*l3d);
//   delete l3d;

}

/*
get the overall line in bus network. oid corresponds to a bus route

*/
void QueryTM::GetLineInBusNetwork(int oid, Line* l, BusNetwork* bn)
{
  SimpleLine br_sl(0);
  bn->GetBusRouteGeoData(oid, br_sl);
  Rectangle<2> bbox = br_sl.BoundingBox();
  Line* newl = new Line(0);
  newl->StartBulkLoad();

  int edgeno = 0;
  for(int i = 0;i < l->Size();i++){
    HalfSegment hs1;
    l->Get(i, hs1);
    if(!hs1.IsLeftDomPoint())continue;
    Point lp = hs1.GetLeftPoint();
    Point rp = hs1.GetRightPoint();
    Point newlp(true, lp.GetX() + bbox.MinD(0), lp.GetY() + bbox.MinD(1));
    Point newrp(true, rp.GetX() + bbox.MinD(0), rp.GetY() + bbox.MinD(1));
    HalfSegment hs2(true, newlp, newrp);
    hs2.attr.edgeno = edgeno++;
    *newl += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *newl += hs2;
  }

  newl->EndBulkLoad();

  line_list1.push_back(*newl);
  delete newl;

/*  Line3D* l3d = new Line3D(0);
  line3d_list.push_back(*l3d);
  delete l3d;*/

}


/*
get the overall line for indoor environment (line3D)

*/
void QueryTM::GetLineInGRoom(int oid, Line* l, Space* sp)
{
//  cout<<"indoor not implemented"<<endl;

}

/////////////////////////////////////////////////////////////////////////
//////////////////// range query for generic moving objects/////////////
////////////////////////////////////////////////////////////////////////
string QueryTM::GenmoRelInfo = "(rel (tuple ((Oid int) (Trip1 genmo) \
(Trip2 mpoint))))";

string QueryTM::GenmoUnitsInfo = "(rel (tuple ((Traj_id int) (MT_box rect3) \
(Mode int) (Index1 point) (Index2 point) (Id int))))";

/*
decompose the units of genmo 
0: single mode in a movement tuple
1: combine walk + m in a movement tuple plus single mode in a movement tuple

*/
void QueryTM::DecomposeGenmo_0(Relation* genmo_rel, double len)
{
  ///indoor: group all units inside one building //
  // walk: a walking segment //
  // car, taxi, bicycle: a road segment ///
  // bus: a bus route segment ////
  // metro: 
  // free space: a unit

//   cout<<"oid maxsize "<<oid_list.max_size()<<endl
//       <<"Periods maxsize "<<time_list.max_size()<<endl
//       <<"Bos maxsize "<<box_list.max_size()<<endl;

 /// oid maxsize 1,07374,1823
 //// Periods maxsize 8947,8485
 /// Bos maxsize 1,0737,4182


  for(int i = 1;i <= genmo_rel->GetNoTuples();i++){
    Tuple* tuple = genmo_rel->GetTuple(i, false);
    int oid = ((CcInt*)tuple->GetAttribute(GENMO_OID))->GetIntval();
    GenMO* mo1 = (GenMO*)tuple->GetAttribute(GENMO_TRIP1);
    MPoint* mo2 = (MPoint*)tuple->GetAttribute(GENMO_TRIP2);
    CreateMTuple_0(oid, mo1, mo2, len);
    tuple->DeleteIfAllowed();

  }

}

/*
create movement tuples: single mode
original method us atperiods to find mpoint, 5500 genmo need 60sec
new method use unit index (record last access position), 5500 genmo need 20 sec

*/
void QueryTM::CreateMTuple_0(int oid, GenMO* mo1, MPoint* mo2, double len)
{
//  cout<<"oid "<<oid<<endl;

  int up_pos = 0;
  for(int i = 0;i < mo1->GetNoComponents();i++){
    UGenLoc unit;
    mo1->Get(i, unit);
    int tm = unit.GetTM(); 
    switch(tm){
      case TM_BUS:

           CollectBusMetro(i, oid, TM_BUS, mo1, mo2, up_pos);
            break;
      case TM_WALK:
           CollectWalk(i, oid, mo1, mo2, len, up_pos);
           break;

      case TM_INDOOR:

           CollectIndoorFree(i, oid, TM_INDOOR, mo1, mo2, up_pos);
           break;

      case TM_CAR:

           CollectCBT(i, oid, TM_CAR, mo1, mo2, up_pos);
           break;
      case TM_BIKE:

           CollectCBT(i, oid, TM_BIKE, mo1, mo2, up_pos);
           break;

      case TM_TAXI:
           CollectCBT(i, oid, TM_TAXI, mo1, mo2, up_pos);
           break;

      case TM_METRO:

           CollectBusMetro(i, oid, TM_METRO, mo1, mo2, up_pos);
           break;

      case TM_FREE:

           CollectIndoorFree(i, oid, TM_FREE, mo1, mo2, up_pos);
           break;

      default:
        assert(false);
        break; 
    }
  }

}

/*
get movement tuples for bus and metro
collect a piece of continuous movement (bus or metro), get the units in mp
use start and end time instants 

*/
void QueryTM::CollectBusMetro(int& i, int oid, int m, GenMO* mo1, 
                              MPoint* mo2, int& up_pos)
{
//   cout<<"bus metro "<<up_pos<<endl;
//   int j = i;
//   int index1 = i;
// 
//   int64_t st = -1;
//   int64_t et = -1;
//   while(j < mo1->GetNoComponents()){
//     UGenLoc unit;
//     mo1->Get(j, unit);
//     if(unit.GetTM() == m){
//         if(st < 0){
//           st = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;
//         }
//         j++;
//     }else{
//       et = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;
//       break;
//     }
//   }
// 
//   i = j -1;
//   int index2 = j - 1;
// 
// //  cout<<st<<" "<<et<<endl;
//   int pos = -1;
//   for(int i = 0;i < mo2->GetNoComponents();i++){
//     UPoint u;
//     mo2->Get(i, u);
//     int64_t cur_t = u.timeInterval.start.ToDouble()*ONEDAY_MSEC;
//     if(cur_t == st){
//       pos = i;
//       break;
//     }
//   }
// 
//   ///////////////////get pieces of movement from mpoint///////////////////
//   MPoint* mp = new MPoint(0);
//   mp->StartBulkLoad();
// 
//   for(int i = pos; i < mo2->GetNoComponents(); i++){
//      UPoint u;
//      mo2->Get(i, u);
//      mp->Add(u);
//      if(i == pos && u.p0.Distance(u.p1) < EPSDIST){ //waiting at the stop
//         continue;
//      }
// 
//      int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;
//      if(u.p0.Distance(u.p1) < EPSDIST || cur_t == et){ //at a bus stop or end
// 
//        mp->EndBulkLoad();
// 
//        Periods* peri = new Periods(0);
//        mp->DefTime(*peri);
//        oid_list.push_back(oid);
//        time_list.push_back(*peri);
// 
// //       box_list.push_back(mp->BoundingBoxSpatial());
//        Rectangle<2> box_spatial = mp->BoundingBoxSpatial();
// //        cout<<box_spatial.MinD(0)<<" "<<box_spatial.MaxD(0)<<" "
// //            <<box_spatial.MinD(1)<<" "<<box_spatial.MaxD(1)<<endl;
// 
//        double min[2], max[2];
//        min[0] = box_spatial.MinD(0);
//        min[1] = box_spatial.MinD(1);
//        max[0] = box_spatial.MaxD(0);
//        max[1] = box_spatial.MaxD(1);
//        if(fabs(box_spatial.MinD(0) - box_spatial.MaxD(0)) < EPSDIST){
//             min[0] -= TEN_METER;
//             max[0] += TEN_METER;
//        }
//       if(fabs(box_spatial.MinD(1) - box_spatial.MaxD(1)) < EPSDIST){
//            min[1] -= TEN_METER;
//            max[1] += TEN_METER;
//        }
//        box_spatial.Set(true, min, max);
// 
// 
//        box_list.push_back(box_spatial);
//        tm_list.push_back(m);
// 
//        Point p1(true, index1, index2);
//        Point p2(true, 0, 0);//not used yet
//        index_list1.push_back(p1);
//        index_list2.push_back(p2);
//        delete peri;
// 
//        mp->Clear();
//        mp->StartBulkLoad();
//      }
// 
//      if(cur_t == et) break;
//   }
// 
//   delete mp;


  int j = i;
  int index1 = i;

  int64_t start_time = -1;
  int64_t end_time = -1;
  while(j < mo1->GetNoComponents()){
    UGenLoc unit;
    mo1->Get(j, unit);
    if(unit.GetTM() == m){
        if(start_time < 0){
            start_time = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;

          end_time = unit.timeInterval.end.ToDouble()*ONEDAY_MSEC;
        }
        j++;
    }else{
      break;
    }
  }

  i = j -1;
  int index2 = j - 1;

  ///////////////////get pieces of movement from mpoint///////////////////
  MPoint* mp = new MPoint(0);
  mp->StartBulkLoad();
  int pos1 = -1;
  int pos2 = -1;
  
  for(int index = up_pos; index < mo2->GetNoComponents();index++){
    UPoint u;
    mo2->Get(index, u);
    mp->Add(u);
    if(start_time < 0) 
      start_time = u.timeInterval.start.ToDouble()*ONEDAY_MSEC;
   
    if(pos1 < 0) pos1 = index;

    if(index == up_pos && u.p0.Distance(u.p1) < EPSDIST){ //waiting at the stop
        continue;
    }
    int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;

    /////////////// at a bus stop or end /////////////////////////
    if(u.p0.Distance(u.p1) < EPSDIST || cur_t == end_time){

      mp->EndBulkLoad();

//      Periods* peri = new Periods(0);
//      mp->DefTime(*peri);
      /////////////////////////////////////////////////
/*      Instant st(instanttype); 
       Instant et(instanttype);

      Periods* peri_t = new Periods(0);
      peri_t->StartBulkLoad();
      Interval<Instant> time_span;
      st.ReadFrom(start_time/ONEDAY_MSEC);
      et.ReadFrom(cur_t/ONEDAY_MSEC);

      time_span.start = st;
      time_span.lc = true;
      time_span.end = et;
      time_span.rc = false;

      peri_t->MergeAdd(time_span);
      peri_t->EndBulkLoad();*/
//      cout<<*peri<<" "<<*peri_t<<endl;
      /////////////////////////////////////////////////////

       Rectangle<2> box_spatial = mp->BoundingBoxSpatial();
//        cout<<box_spatial.MinD(0)<<" "<<box_spatial.MaxD(0)<<" "
//            <<box_spatial.MinD(1)<<" "<<box_spatial.MaxD(1)<<endl;

       double min[2], max[2];
       min[0] = box_spatial.MinD(0);
       min[1] = box_spatial.MinD(1);
       max[0] = box_spatial.MaxD(0);
       max[1] = box_spatial.MaxD(1);
       if(fabs(box_spatial.MinD(0) - box_spatial.MaxD(0)) < EPSDIST){
            min[0] -= TEN_METER;
            max[0] += TEN_METER;
       }
      if(fabs(box_spatial.MinD(1) - box_spatial.MaxD(1)) < EPSDIST){
           min[1] -= TEN_METER;
           max[1] += TEN_METER;
       }
       box_spatial.Set(true, min, max);

        oid_list.push_back(oid);
//        time_list.push_back(*peri);
//         time_list.push_back(*peri_t);
//         box_list.push_back(box_spatial);
        Rectangle<3> tm_box(true, 
                            start_time/ONEDAY_MSEC,
                            end_time/ONEDAY_MSEC,
                            min[0], max[0],
                            min[1], max[1]
                            );

        box_list.push_back(tm_box);
        tm_list.push_back(m);
 
        Point p1(true, index1, index2);

        pos2 = index;
        assert(pos1 >= 0 && pos2 >= pos1);
        Point p2(true, pos1, pos2);//index in mpoint
        index_list1.push_back(p1);
        index_list2.push_back(p2);

//       delete peri;
//       delete peri_t;

       mp->Clear();
       mp->StartBulkLoad();

       start_time = -1;
       pos1 = -1;
     }

     if(cur_t == end_time) {
        up_pos = index + 1;
        break;
     }

  }

  delete mp;

}

/*
collect all walking units 
collect a complete part of walking movement and use the time period to find 
the units in mp
use start and end time instants 

*/
void QueryTM::CollectWalk(int& i, int oid, GenMO* mo1, MPoint* mo2, 
                          double len, int& up_pos)
{
//  cout<<"walk "<<up_pos<<"i "<<i<<endl;
//   int index1 = i;
//   int j = i;
// 
//   Periods* peri = new Periods(0);
//   peri->StartBulkLoad();
//   double l = 0.0;
//   while(j < mo1->GetNoComponents()){
//     UGenLoc unit;
//     mo1->Get(j, unit);
// 
//     if(unit.GetTM() == TM_WALK){
// 
//       peri->MergeAdd(unit.timeInterval);
//       Point p1(true, unit.gloc1.GetLoc().loc1, unit.gloc1.GetLoc().loc2);
//       Point p2(true, unit.gloc2.GetLoc().loc1, unit.gloc2.GetLoc().loc2);
//       l += p1.Distance(p2);
//       if(l > len){
//           peri->EndBulkLoad();
//           MPoint* mp = new MPoint(0);
//           mo2->AtPeriods(*peri, *mp);
// 
//           oid_list.push_back(oid);
//           time_list.push_back(*peri);
//           box_list.push_back(mp->BoundingBoxSpatial());
//           tm_list.push_back(TM_WALK);
// 
//           Point p1(true, index1, j);
//           Point p2(true, 0, 0);//not used yet
//           index_list1.push_back(p1);
//           index_list2.push_back(p2);
// 
//           index1 = j;
// 
//           peri->Clear();
//           peri->StartBulkLoad();
//           l = 0.0;
//       }
//       j++;
//     }else{
//       break;
//     }
//   }
// 
//   int index2 = j - 1;
// 
//   peri->EndBulkLoad();
//   if(peri->GetNoComponents() > 0){
//       MPoint* mp = new MPoint(0);
//       mo2->AtPeriods(*peri, *mp);
// 
//       oid_list.push_back(oid);
//       time_list.push_back(*peri);
//       box_list.push_back(mp->BoundingBoxSpatial());
//       tm_list.push_back(TM_WALK);
// 
//       Point p1(true, index1, index2);
//       Point p2(true, 0, 0);//not used yet
//       index_list1.push_back(p1);
//       index_list2.push_back(p2);
//       delete mp;
//   }
//   ///////////////////////////////////////////////////////////////////
//   delete peri;
//   i = j - 1;

    int index1 = i;
    int j = i;


    double l = 0.0;
    int64_t start_time = -1;
    int64_t end_time = -1;
    while(j < mo1->GetNoComponents()){
      UGenLoc unit;
      mo1->Get(j, unit);

      if(unit.GetTM() == TM_WALK){
        if(start_time < 0)
          start_time = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;

        end_time = unit.timeInterval.end.ToDouble()*ONEDAY_MSEC;


        Point p1(true, unit.gloc1.GetLoc().loc1, unit.gloc1.GetLoc().loc2);
        Point p2(true, unit.gloc2.GetLoc().loc1, unit.gloc2.GetLoc().loc2);
        l += p1.Distance(p2);
        if(l > len){

          MPoint* mp = new MPoint(0);
          mp->StartBulkLoad();

          int pos1 = up_pos;
          int pos2 = -1;
          for(int index = up_pos; index < mo2->GetNoComponents();index++){
            UPoint u;
            mo2->Get(index, u);
            mp->Add(u);
            int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;
            if(cur_t == end_time){
              up_pos = index + 1;
              pos2 = index;
              break;
            }
          }
          mp->EndBulkLoad();

//          Periods* peri = new Periods(0);
//          mp->DefTime(*peri);
         ///////////////////////////////////////////////////////
//           Instant st(instanttype); 
//           Instant et(instanttype);
// 
//           Periods* peri_t = new Periods(0);
//           peri_t->StartBulkLoad();
//           Interval<Instant> time_span;
//           st.ReadFrom(start_time/ONEDAY_MSEC);
//           et.ReadFrom(end_time/ONEDAY_MSEC);
// 
//           time_span.start = st;
//           time_span.lc = true;
//           time_span.end = et;
//           time_span.rc = false;
// 
//           peri_t->MergeAdd(time_span);
//           peri_t->EndBulkLoad();
//          cout<<*peri<<" "<<*peri_t<<endl;
         /////////////////////////////////////////////////////////

          oid_list.push_back(oid);
//          time_list.push_back(*peri);
/*          time_list.push_back(*peri_t);
          box_list.push_back(mp->BoundingBoxSpatial());*/
          Rectangle<3> tm_box(true,
                              start_time/ONEDAY_MSEC,
                              end_time/ONEDAY_MSEC,
                              mp->BoundingBoxSpatial().MinD(0),
                              mp->BoundingBoxSpatial().MaxD(0),
                              mp->BoundingBoxSpatial().MinD(1),
                              mp->BoundingBoxSpatial().MaxD(1)
                             );

          box_list.push_back(tm_box);

          tm_list.push_back(TM_WALK);

          Point p1(true, index1, j);
          assert(pos1 >= 0 && pos2 >= pos1);
          Point p2(true, pos1, pos2);//index in mpoint
          index_list1.push_back(p1);
          index_list2.push_back(p2);

          index1 = j;

//          delete peri;
          delete mp;

//          delete peri_t;

          start_time = -1;
          end_time = -1;

          l = 0.0;
        }
        j++;
      }else{
        break;
      }
    }

    int index2 = j - 1;

    if(start_time > 0 && start_time < end_time){

      MPoint* mp = new MPoint(0);
      mp->StartBulkLoad();

      int pos1 = up_pos;
      int pos2 = -1;
      for(int index = up_pos;index < mo2->GetNoComponents();index++){
        UPoint u;
        mo2->Get(index, u);
        mp->Add(u);
        if(index == up_pos) 
          start_time = u.timeInterval.start.ToDouble()*ONEDAY_MSEC;

        int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;
        if(cur_t == end_time){
          up_pos = index + 1;
          pos2 = index;
          break;
        }

      }
      mp->EndBulkLoad();

//      Periods* peri = new Periods(0);
//      mp->DefTime(*peri);

      /////////////////////////////////////////////////////
//        Instant st(instanttype); 
//        Instant et(instanttype);
// 
//        Periods* peri_t = new Periods(0);
//        peri_t->StartBulkLoad();
//        Interval<Instant> time_span;
//        st.ReadFrom(start_time/ONEDAY_MSEC);
//        et.ReadFrom(end_time/ONEDAY_MSEC);
// 
//        time_span.start = st;
//        time_span.lc = true;
//        time_span.end = et;
//        time_span.rc = false;
// 
//        peri_t->MergeAdd(time_span);
//        peri_t->EndBulkLoad();
//       cout<<*peri<<" "<<*peri_t<<endl;
      /////////////////////////////////////////////////////
      oid_list.push_back(oid);
//      time_list.push_back(*peri);
/*      time_list.push_back(*peri_t);
      box_list.push_back(mp->BoundingBoxSpatial());*/

      Rectangle<3> tm_box(true,
                          start_time/ONEDAY_MSEC,
                          end_time/ONEDAY_MSEC,
                          mp->BoundingBoxSpatial().MinD(0),
                          mp->BoundingBoxSpatial().MaxD(0),
                          mp->BoundingBoxSpatial().MinD(1),
                          mp->BoundingBoxSpatial().MaxD(1)
                         );

      box_list.push_back(tm_box);
      
      tm_list.push_back(TM_WALK);

      Point p1(true, index1, index2);
      assert(pos1 >=0 && pos2 >= pos1);
      Point p2(true, pos1, pos2);//index in mpoint
      index_list1.push_back(p1);
      index_list2.push_back(p2);


//      delete peri;
      delete mp;

//      delete peri_t;
    }
    ///////////////////////////////////////////////////////////////////

    i = j - 1;
}

/*
collect movement tuples for car, taxi and bike
for each movement on one road, get the units in mp
use start and end time instants 

*/
void QueryTM::CollectCBT(int&i, int oid, int m, GenMO* mo1, 
                         MPoint* mo2, int& up_pos)
{
//    cout<<"car bike taxi "<<up_pos<<endl;
//   int index1 = i;
//   int j = i;
// 
//   Periods* peri = new Periods(0);
//   peri->StartBulkLoad();
//   int last_rid = -1;
//   while(j < mo1->GetNoComponents()){
//     UGenLoc unit;
//     mo1->Get(j, unit);
// 
//     if(unit.GetTM() == m){
// 
//       peri->MergeAdd(unit.timeInterval);
//       int cur_rid = unit.GetOid();
//       if(last_rid == -1){
//         last_rid = cur_rid;
//       }else if(last_rid != cur_rid){
//           peri->EndBulkLoad();
//           MPoint* mp = new MPoint(0);
//           mo2->AtPeriods(*peri, *mp);
// 
//           oid_list.push_back(oid);
//           time_list.push_back(*peri);
//           box_list.push_back(mp->BoundingBoxSpatial());
//           tm_list.push_back(m);
// 
//           Point p1(true, index1, j);
//           Point p2(true, 0, 0);//not used yet
//           index_list1.push_back(p1);
//           index_list2.push_back(p2);
// 
//           index1 = j;
// 
//           peri->Clear();
//           peri->StartBulkLoad();
//           last_rid = cur_rid;
//       }
// 
//       j++;
//     }else{
//       break;
//     }
//   }
// 
//   int index2 = j - 1;
// 
//   peri->EndBulkLoad();
//   if(peri->GetNoComponents() > 0){
//       MPoint* mp = new MPoint(0);
//       mo2->AtPeriods(*peri, *mp);
// 
//       oid_list.push_back(oid);
//       time_list.push_back(*peri);
//       box_list.push_back(mp->BoundingBoxSpatial());
//       tm_list.push_back(m);
// 
//       Point p1(true, index1, index2);
//       Point p2(true, 0, 0);//not used yet
//       index_list1.push_back(p1);
//       index_list2.push_back(p2);
//       delete mp;
//   }
//   ///////////////////////////////////////////////////////////////////
//   delete peri;
//   i = j - 1;


  int index1 = i;
  int j = i;

  int64_t start_time = -1;
  int64_t end_time = -1;


  int last_rid = -1;
  while(j < mo1->GetNoComponents()){
    UGenLoc unit;
    mo1->Get(j, unit);

    if(unit.GetTM() == m){
      if(start_time < 0) 
        start_time = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;

      end_time = unit.timeInterval.end.ToDouble()*ONEDAY_MSEC;

      int cur_rid = unit.GetOid();
      if(last_rid == -1){
        last_rid = cur_rid;
      }else if(last_rid != cur_rid){

          MPoint* mp = new MPoint(0);
          mp->StartBulkLoad();
          int pos1 = up_pos;
          int pos2 = -1;
          for(int index = up_pos;index < mo2->GetNoComponents();index++){
            UPoint u;
            mo2->Get(index, u);
            mp->Add(u);
            int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;
            if(cur_t == end_time){
              up_pos = index + 1;
              pos2 = index;
              break;
            }
          }
          mp->EndBulkLoad();

//           Periods* peri = new Periods(0);
//           mp->DefTime(*peri);

          //////////////////////////////////////////////
//           Instant st(instanttype); 
//           Instant et(instanttype);
// 
//           Periods* peri_t = new Periods(0);
//           peri_t->StartBulkLoad();
//           Interval<Instant> time_span;
//           st.ReadFrom(start_time/ONEDAY_MSEC);
//           et.ReadFrom(end_time/ONEDAY_MSEC);
// 
//           time_span.start = st;
//           time_span.lc = true;
//           time_span.end = et;
//           time_span.rc = false;
// 
//           peri_t->MergeAdd(time_span);
//           peri_t->EndBulkLoad();

//          cout<<*peri<<" "<<*peri_t<<endl;
          ////////////////////////////////////
          oid_list.push_back(oid);
//          time_list.push_back(*peri);
//           time_list.push_back(*peri_t);
//           box_list.push_back(mp->BoundingBoxSpatial());
          Rectangle<3> tm_box(true,
                              start_time/ONEDAY_MSEC,
                              end_time/ONEDAY_MSEC,
                              mp->BoundingBoxSpatial().MinD(0),
                              mp->BoundingBoxSpatial().MaxD(0),
                              mp->BoundingBoxSpatial().MinD(1),
                              mp->BoundingBoxSpatial().MaxD(1)
                              );

          box_list.push_back(tm_box);

          tm_list.push_back(m);

          Point p1(true, index1, j);
          assert(pos1 >= 0 && pos2 >= pos1);
          Point p2(true, pos1, pos2);//index in mpoint
          index_list1.push_back(p1);
          index_list2.push_back(p2);

          index1 = j;
          last_rid = cur_rid;

          start_time = -1;
          end_time = -1;
//          delete peri;
          delete mp;

//          delete peri_t;
      }

      j++;
    }else{
      break;
    }
  }

  int index2 = j - 1;

  if(start_time > 0 && start_time < end_time){

      MPoint* mp = new MPoint(0);
      mp->StartBulkLoad();

      int pos1 = up_pos;
      int pos2 = -1;
      for(int index = up_pos;index < mo2->GetNoComponents();index++){
        UPoint u;
        mo2->Get(index, u);
        mp->Add(u);
        if(index == up_pos) 
          start_time = u.timeInterval.start.ToDouble()*ONEDAY_MSEC;

        int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;
        if(cur_t == end_time){
          up_pos = index + 1;
          pos2 = index;
          break;
        }
      }
      mp->EndBulkLoad();

//       Periods* peri = new Periods(0);
//       mp->DefTime(*peri);

      //////////////////////////////////////////////////
//       Instant st(instanttype); 
//       Instant et(instanttype);
// 
//       Periods* peri_t = new Periods(0);
//       peri_t->StartBulkLoad();
//       Interval<Instant> time_span;
//       st.ReadFrom(start_time/ONEDAY_MSEC);
//       et.ReadFrom(end_time/ONEDAY_MSEC);
// 
//       time_span.start = st;
//       time_span.lc = true;
//       time_span.end = et;
//       time_span.rc = false;
// 
//       peri_t->MergeAdd(time_span);
//       peri_t->EndBulkLoad();
//      cout<<*peri<<" "<<*peri_t<<endl;
      ////////////////////////////////////////////////////

      oid_list.push_back(oid);
/*      time_list.push_back(*peri_t);
      box_list.push_back(mp->BoundingBoxSpatial());*/

      Rectangle<3> tm_box(true,
                          start_time/ONEDAY_MSEC,
                          end_time/ONEDAY_MSEC,
                          mp->BoundingBoxSpatial().MinD(0),
                          mp->BoundingBoxSpatial().MaxD(0),
                          mp->BoundingBoxSpatial().MinD(1),
                          mp->BoundingBoxSpatial().MaxD(1)
                         );
      box_list.push_back(tm_box);
      tm_list.push_back(m);

      Point p1(true, index1, index2);
      assert(pos1 >= 0 && pos2 >= pos1);
      Point p2(true, pos1, pos2);//index in mpoint
      index_list1.push_back(p1);
      index_list2.push_back(p2);

//      delete peri;
      delete mp;

//      delete peri_t;
  }
  ///////////////////////////////////////////////////////////////////

  i = j - 1;

}

/*
collect all indoor units inside one building as one movement tuple
use start and end time instants 

*/
void QueryTM::CollectIndoorFree(int& i, int oid, int m,
                                GenMO* mo1, MPoint* mo2, int& up_pos)
{
//    cout<<"indoor free "<<up_pos<<" i "<<i<<endl;
//   int index1 = i;
//   int j = i;
//   Periods* peri = new Periods(0);
//   peri->StartBulkLoad();
//   while(j < mo1->GetNoComponents()){
//     UGenLoc unit;
//     mo1->Get(j, unit);
// 
//     if(unit.GetTM() == m){
//       peri->MergeAdd(unit.timeInterval);
//       j++;
//     }else{
// 
//       break;
//     }
//   }
//   int index2 = j - 1;
// 
//   peri->EndBulkLoad();
//   MPoint* mp = new MPoint(0);
//   mo2->AtPeriods(*peri, *mp);
// 
//   ////////////////////////////////////////////////////////////////////
//   oid_list.push_back(oid);
//   time_list.push_back(*peri);
//   box_list.push_back(mp->BoundingBoxSpatial());
//   tm_list.push_back(m);
// 
//   Point p1(true, index1, index2);
//   Point p2(true, 0, 0);//not used yet
//   index_list1.push_back(p1);
//   index_list2.push_back(p2);
// 
//   ///////////////////////////////////////////////////////////////////
//   delete mp;
//   delete peri;
// 
//   i = j - 1;


  int index1 = i;
  int j = i;
  int64_t start_time = -1;
  int64_t end_time = -1;

  while(j < mo1->GetNoComponents()){
    UGenLoc unit;
    mo1->Get(j, unit);

    if(unit.GetTM() == m){
      if(start_time < 0)
        start_time = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;

      end_time = unit.timeInterval.end.ToDouble()*ONEDAY_MSEC;
      j++;
    }else{
      break;
    }
  }
  int index2 = j - 1;

//  cout<<start_time<<" "<<end_time<<endl;
  MPoint* mp = new MPoint(0);
  mp->StartBulkLoad();
  int pos1 = up_pos;
  int pos2 = -1;
  for(int index = up_pos; index < mo2->GetNoComponents(); index++){
    UPoint u;
    mo2->Get(index, u);
    mp->Add(u);
    int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;
    if(cur_t == end_time){
//      cout<<"find end time "<<endl;
      up_pos = index + 1;
      pos2 = index;
      break;
    }

  }
  mp->EndBulkLoad();
  //  Instant st(instanttype); 
//  Instant et(instanttype);

//   Periods* peri = new Periods(0);
//   mp->DefTime(*peri);

//   Periods* peri_t = new Periods(0);
//   peri_t->StartBulkLoad();
//   Interval<Instant> time_span;
//   st.ReadFrom(start_time/ONEDAY_MSEC);
//   et.ReadFrom(end_time/ONEDAY_MSEC);
// 
//   time_span.start = st;
//   time_span.lc = true;
//   time_span.end = et;
//   time_span.rc = false;
// 
//   peri_t->MergeAdd(time_span);
//   peri_t->EndBulkLoad();
  
//  cout<<*peri<<" "<<*peri_t<<endl;
  ////////////////////////////////////////////////////////////////////
  oid_list.push_back(oid);
//  time_list.push_back(*peri);
//  time_list.push_back(*peri_t);
//  box_list.push_back(mp->BoundingBoxSpatial());

  Rectangle<3> tm_box(true,
                      start_time/ONEDAY_MSEC,
                      end_time/ONEDAY_MSEC,
                      mp->BoundingBoxSpatial().MinD(0),
                      mp->BoundingBoxSpatial().MaxD(0),
                      mp->BoundingBoxSpatial().MinD(1),
                      mp->BoundingBoxSpatial().MaxD(1)
                     );
  box_list.push_back(tm_box);
  tm_list.push_back(m);

  Point p1(true, index1, index2);

  assert(pos1 >= 0 && pos2 >= pos1);
  Point p2(true, pos1, pos2);//index in mpoint
  index_list1.push_back(p1);
  index_list2.push_back(p2);

  ///////////////////////////////////////////////////////////////////
//  delete peri_t;
//  delete peri;
  delete mp;

  i = j - 1;
}

/*
decompose the genmo considering a sequence of modes

*/
void QueryTM::DecomposeGenmo_1(Relation* genmo_rel)
{
  cout<<genmo_rel->GetNoTuples()<<endl;

}

/*
for each node calculate tm values using an integer, stops when there are several
  values because this can not be used to prune tm-rtree nodes

*/
unsigned long QueryTM::Node_TM(R_Tree<3, TupleId>* tmrtree, Relation* rel,
                      SmiRecordId nodeid, int attr_pos)
{

  R_TreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                  tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

  if(node->IsLeaf()){
    cout<<"leaf node "<<nodeid<<endl;
    int pos = -1;
    //////////////for testing////////////////
    for(int j = 0;j < node->EntryCount();j++){

      R_TreeLeafEntry<3, TupleId> e =
                (R_TreeLeafEntry<3, TupleId>&)(*node)[j];
      Tuple* tuple = rel->GetTuple(e.info, false);
      int m = ((CcInt*)tuple->GetAttribute(attr_pos))->GetIntval();
      tuple->DeleteIfAllowed();
//      cout<<"j "<<j<<" tm "<<GetTMStr(m)<<endl;
//      pos = (int)ARR_SIZE(str_tm) - 1 - m;
      if(pos < 0) pos = (int)(ARR_SIZE(str_tm) - 1 - m);
      else assert(pos == (int)(ARR_SIZE(str_tm) - 1 - m));
    }


    delete node;
    bitset<ARR_SIZE(str_tm)> modebits;
    modebits.reset();
    modebits.set(pos, 1);
    cout<<modebits.to_ulong()<<" "<<modebits.to_string()<<endl;
    ////////////////////output the result///////////////////
    oid_list.push_back(nodeid);
    mode_list.push_back(modebits.to_ulong());

    return modebits.to_ulong();

  }else{
     cout<<"non leaf node "<<nodeid<<endl;
     bitset<ARR_SIZE(str_tm)> modebits;
     modebits.reset();
     for(int j = 0;j < node->EntryCount();j++){

        R_TreeInternalEntry<3> e =
               (R_TreeInternalEntry<3>&)(*node)[j];
        int son_tm = Node_TM(tmrtree, rel, e.pointer, attr_pos);
        if(son_tm < 0){//if sontm < 0, stops
          delete node;
          return -1;
        }
        bitset<ARR_SIZE(str_tm)> m_bit(son_tm);
        ///////////// union value of each son tm to tm//////////////
/*        cout<<"new one "<<m_bit.to_string()
            <<" before "<<modebits.to_string()<<endl;*/
        modebits = modebits | m_bit;
//        cout<<"after"<<modebits.to_string()<<endl;

      }

      delete node;
      ///////////////////output the result ///////////////////
      cout<<modebits.to_ulong()<<" "<<modebits.to_string()<<endl;
      oid_list.push_back(nodeid);
      mode_list.push_back(modebits.to_ulong());

      //////////////////////////////////////////////////////
      if(modebits.count() > 1) return -1;

      return modebits.to_ulong();
  }

}

/*
get nodes of TM-RTree

*/
void QueryTM::TM_RTreeNodes(TM_RTree<3,TupleId>* tmrtree)
{

  SmiRecordId node_id = tmrtree->RootRecordId();
  int level = 0;
  GetNodes(tmrtree, node_id, level);
}

/*
get nodes information 

*/
void QueryTM::GetNodes(TM_RTree<3, TupleId>* tmrtree, SmiRecordId nodeid, 
                       int level)
{

  TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                  tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

  if(node->IsLeaf()){
    long m = node->GetTMValue();
//    cout<<"leaf node "<<nodeid<<" m "<<m<<" "<<GetModeString(m)<<endl;
    bitset<ARR_SIZE(str_tm)> mode_bit(m);
    assert(mode_bit.count() == 1);//one mode in a leaf node

    ///////////////output result///////////////////////////////
    oid_list.push_back(nodeid);
    level_list.push_back(level);
    b_list.push_back(true);
    str_list.push_back(GetModeString(m));
    mode_list.push_back(m);
    box_list.push_back(node->BoundingBox());
    entry_list.push_back(node->EntryCount());

    delete node;

  }else{

     for(int j = 0;j < node->EntryCount();j++){

        R_TreeInternalEntry<3> e =
               (R_TreeInternalEntry<3>&)(*node)[j];
        GetNodes(tmrtree, e.pointer, level + 1);
      }
      long m = node->GetTMValue();
//      cout<<"non leaf node "<<nodeid<<" m "<<m<<endl;

      oid_list.push_back(nodeid);
      level_list.push_back(level);
      b_list.push_back(false);
      str_list.push_back(GetModeString(m));
      mode_list.push_back(m);
      box_list.push_back(node->BoundingBox());
      entry_list.push_back(node->EntryCount());

      delete node;

      //////////////////////////////////////////////////////

  }

}