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

string QueryTM::GenmoUnitsInfo = "(rel (tuple ((Traj_id int) (MT_box rect3)\
(Mode int) (SubTrip mpoint) (DivPos int) (Id int))))";

string QueryTM::GenmoRangeQuery = "(rel (tuple ((T periods) (BOX rect)\
(Mode string))))";

/*
decompose the units of genmo 
0: single mode in a movement tuple
1: combine walk + m in a movement tuple plus single mode in a movement tuple

*/
void QueryTM::DecomposeGenmo(Relation* genmo_rel, double len)
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

//    CreateMTuple_0(oid, mo1, mo2, len);
//    cout<<"oid "<<oid<<endl;
//      if(oid != 1234) {
//        tuple->DeleteIfAllowed();
//        continue;
//      }

    if(mt_type)
      CreateMTuple_1(oid, mo1, mo2, len);//single mode + a pair of modes
    else
      CreateMTuple_0(oid, mo1, mo2, len);//only single mode

    tuple->DeleteIfAllowed();

//    break;
  }

//   cout<<oid_list.size()<<" "<<box_list.size()
//       <<" "<<tm_list.size()<<" "<<mp_list.size()<<endl;

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
           CollectWalk_0(i, oid, mo1, mo2, len, up_pos);
           break;

      case TM_INDOOR:

           CollectIndoor(i, oid, TM_INDOOR, mo1, mo2, up_pos);
           break;

      case TM_CAR:

           CollectCBT(i, oid, TM_CAR, mo1, mo2, up_pos, len);
           break;
      case TM_BIKE:

           CollectCBT(i, oid, TM_BIKE, mo1, mo2, up_pos, len);
           break;

      case TM_TAXI:
           CollectCBT(i, oid, TM_TAXI, mo1, mo2, up_pos, len);
           break;

      case TM_METRO:

           CollectBusMetro(i, oid, TM_METRO, mo1, mo2, up_pos);
           break;

      case TM_FREE:

           CollectFree_0(i, oid, TM_FREE, mo1, mo2, up_pos);
           break;

      default:
        assert(false);
        break; 
    }
  }
}

/*
extend the method above to be able to process a pair of modes
combine walk and another

*/

void QueryTM::CreateMTuple_1(int oid, GenMO* mo1, MPoint* mo2, double len)
{
//  cout<<"oid "<<oid<<endl;

  int up_pos = 0;
  for(int j = 0;j < mo1->GetNoComponents();j++){
    UGenLoc unit;
    mo1->Get(j, unit);
    int tm = unit.GetTM(); 
    switch(tm){
      case TM_BUS:

           CollectBusMetro(j, oid, TM_BUS, mo1, mo2, up_pos);
            break;
      case TM_WALK:
           CollectWalk_1(j, oid, mo1, mo2, len, up_pos);//two mtuples
           break;

      case TM_INDOOR:

           CollectIndoor(j, oid, TM_INDOOR, mo1, mo2, up_pos);
           break;

      case TM_CAR:

           CollectCBT(j, oid, TM_CAR, mo1, mo2, up_pos, len);
           break;
      case TM_BIKE:

           CollectCBT(j, oid, TM_BIKE, mo1, mo2, up_pos, len);
           break;

      case TM_TAXI:
           CollectCBT(j, oid, TM_TAXI, mo1, mo2, up_pos, len);
           break;

      case TM_METRO:

           CollectBusMetro(j, oid, TM_METRO, mo1, mo2, up_pos);
           break;

      case TM_FREE:

           CollectFree_1(j, oid, TM_FREE, mo1, mo2, up_pos);//two mtuples

           break;

      default:
        assert(false);
        break; 
    }
  }
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////

   ///////store not only a single mode but also a pair of modes//////////////
//    cout<<"oid size "<<oid_list.size()<<endl;
//   cout<<Oid_list.size()<<" "<<Box_list.size()
//       <<" "<<Tm_list.size()<<" "<<Mp_list.size()<<endl;

      ///////////////// output information ///////////////////
//     for(unsigned int i = 0;i < oid_list.size();i++){
//       Periods* peri = new Periods(0);
//       mp_list[i].DefTime(*peri);
//       cout<<GetTMStrExt(tm_list[i])<<" "<<*peri<<endl;
//       delete peri;
//     }

    if(oid_list.size() == 1){//////indoor trips, as one movement tuple
         Oid_list.push_back(oid_list[0]);
         Tm_list.push_back(tm_list[0]);
         Box_list.push_back(box_list[0]);
         Mp_list.push_back(mp_list[0]);
         div_list.push_back(-1);//one mode
    }else{

        for(unsigned int i = 0;i < oid_list.size() - 1;i++){
          int j = i + 1;

          if(tm_list[i] == tm_list[j]){
            Oid_list.push_back(oid_list[i]);
            Tm_list.push_back(tm_list[i]);
            Box_list.push_back(box_list[i]);
            Mp_list.push_back(mp_list[i]);
            div_list.push_back(-1);//one mode

            if(i == oid_list.size() - 2){
                Oid_list.push_back(oid_list[j]);
                Tm_list.push_back(tm_list[j]);
                Box_list.push_back(box_list[j]);
                Mp_list.push_back(mp_list[j]);
                div_list.push_back(-1);//one mode
            }

          }else{

            Rectangle<3> box1 = box_list[i];
            Rectangle<3> box2 = box_list[j];
            Rectangle<3> box = box1.Union(box2);
            int m1 = tm_list[i];
            int m2 = tm_list[j];
            if(m1 == TM_WALK || m2 == TM_WALK){
              int m = -1;
              if(m1 == TM_WALK){
                m = m2 + 2*ARR_SIZE(str_tm);
              }else{
                m = m1 + ARR_SIZE(str_tm);
              }

              MPoint* mp1 = &mp_list[i];
              MPoint* mp2 = &mp_list[j];
              MPoint* mp = new MPoint(0);
              mp->StartBulkLoad();
              for(int k = 0;k < mp1->GetNoComponents();k++){
                UPoint u;
                mp1->Get(k, u);
                mp->Add(u);
              }
              for(int k = 0;k < mp2->GetNoComponents();k++){
                UPoint u;
                mp2->Get(k, u);
                mp->Add(u);
              }
              mp->EndBulkLoad();

              Oid_list.push_back(oid_list[i]);
              Tm_list.push_back(m); //bit index
              Box_list.push_back(box);
              Mp_list.push_back(*mp);
              div_list.push_back(mp1->GetNoComponents());//two modes, record pos

              delete mp;

              if(i < oid_list.size() - 2){
                int next_elem = j + 1;
                if(tm_list[j] == tm_list[next_elem]){
                  i++;
                }

              }
            }else{
              cout<<"should not be here"<<endl;
              assert(false);
            }

          }
        } 
    }

    oid_list.clear();
    box_list.clear();
    tm_list.clear();
    mp_list.clear();
}


/*
get movement tuples for bus and metro
collect a piece of continuous movement (bus or metro), get the units in mp
use start and end time instants 

*/
void QueryTM::CollectBusMetro(int& i, int oid, int m, GenMO* mo1, 
                              MPoint* mo2, int& up_pos)
{
//    cout<<"CollectBusMetro"<<endl;

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

  int pos1 = -1;
  int pos2 = -1;

  vector<MPoint*> mp_pointer_list;
  int counter = 0;
  mp_pointer_list.push_back(new MPoint(0));
  mp_pointer_list[counter]->StartBulkLoad();
  
  for(int index = up_pos; index < mo2->GetNoComponents();index++){
    UPoint u;
    mo2->Get(index, u);

    mp_pointer_list[counter]->Add(u);
    if(start_time < 0) 
      start_time = u.timeInterval.start.ToDouble()*ONEDAY_MSEC;

    if(pos1 < 0) pos1 = index;

    if(index == up_pos && u.p0.Distance(u.p1) < EPSDIST){ //waiting at the stop
        continue;
    }
    int64_t cur_t = u.timeInterval.end.ToDouble()*ONEDAY_MSEC;

    /////////////// at a bus stop or end /////////////////////////
    if(u.p0.Distance(u.p1) < EPSDIST || cur_t == end_time){

      mp_pointer_list[counter]->EndBulkLoad();

      mp_list.push_back(*mp_pointer_list[counter]);//store a subtrip
//      cout<<oid<<" "<<subtrip_list[subtrip_list.size() - 1]<<endl;


//      cout<<mp<<endl;

//      Periods* peri = new Periods(0);
//      mp.DefTime(*peri);
//      cout<<*peri<<endl;
//      delete peri;

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

       Rectangle<2> box_spatial = 
            mp_pointer_list[counter]->BoundingBoxSpatial();
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
//        index_list1.push_back(p1); //genmo index
//        index_list2.push_back(p2); //mpoint index
//        index_list.push_back(p2);


//       delete peri;
//       delete peri_t;

      ////////////////////////////////////////////
       counter++;
       mp_pointer_list.push_back(new MPoint(0));
       mp_pointer_list[counter]->StartBulkLoad();
       //////////////////////////////////////////////

       start_time = -1;
       pos1 = -1;
     }

     if(cur_t == end_time) {
        up_pos = index + 1;
        break;
     }

  }

  /////////reallocate memory/////////////////////
  for(int i = 0;i <= counter;i++)
    delete mp_pointer_list[i];


}

/*
collect all walking units 
collect a complete part of walking movement and use the time period to find 
the units in mp
use start and end time instants 

*/
void QueryTM::CollectWalk_0(int& i, int oid, GenMO* mo1, MPoint* mo2, 
                          double len, int& up_pos)
{
//  cout<<"CollectWalk"<<endl;

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
          mp_list.push_back(*mp);


//        Periods* peri = new Periods(0);
//        mp->DefTime(*peri);
//        delete peri;

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
//          index_list1.push_back(p1); //genmo index
//          index_list2.push_back(p2); //mpoint index

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
      mp_list.push_back(*mp);//store a sub trip

//    Periods* peri = new Periods(0);
//    mp->DefTime(*peri);
//    delete peri;

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
//      index_list1.push_back(p1); //genmo index
//      index_list2.push_back(p2); //mpoint index


//      delete peri;
      delete mp;

//      delete peri_t;
    }
    ///////////////////////////////////////////////////////////////////

    i = j - 1;
}

/*
almost the same as collectwalk 0, but be sure that there are at least two
mtules

*/
void QueryTM::CollectWalk_1(int& i, int oid, GenMO* mo1, MPoint* mo2, 
                          double len, int& up_pos)
{
//  cout<<"CollectWalk"<<endl;
    int mtuple = 0;

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
          mp_list.push_back(*mp);

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
          mtuple++;

          Point p1(true, index1, j);
          assert(pos1 >= 0 && pos2 >= pos1);
          Point p2(true, pos1, pos2);//index in mpoint
//          index_list1.push_back(p1); //genmo index
//          index_list2.push_back(p2); //mpoint index

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
      mp_list.push_back(*mp);//store a sub trip
      /////////////////////////////////////////////////////
      oid_list.push_back(oid);
      mtuple++;
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
//      index_list1.push_back(p1); //genmo index
//      index_list2.push_back(p2); //mpoint index


//      delete peri;
      delete mp;

//      delete peri_t;
    }
    ///////////////////////////////////////////////////////////////////

    i = j - 1;


    if(mtuple == 1){//decompose into two movements
      MPoint* mp = &(mp_list[mp_list.size() - 1]);

      if(mp->GetNoComponents() == 1){///////////only one unit, one mtuples
        /////doing nothing
      }else if(tm_list.size() > 1 && 
               tm_list[tm_list.size() - 2] == TM_WALK){ //two already
        //if the last movement is also walking, do not split
      }else{///////////two mtuples
//          cout<<"mp "<<*mp<<" "<<mp->GetNoComponents()<<endl;
          int div_pos = (int)(mp->GetNoComponents()/2.0);
//          cout<<"div_pos "<<div_pos<<endl;
          MPoint* mp1 = new MPoint(0);
          mp1->StartBulkLoad();
          double st1, et1;
          for(int k = 0;k < div_pos;k++){
            UPoint u;
            mp->Get(k, u);
            mp1->Add(u);
            if(k == 0) st1 = u.timeInterval.start.ToDouble();
            if(k == div_pos - 1)et1 = u.timeInterval.end.ToDouble();
          }

         mp1->EndBulkLoad();

//          mp_list[mp_list.size() - 1] = *mp1;//update the one in the list


         Rectangle<3> tm_box1(true, st1, et1,
                          mp1->BoundingBoxSpatial().MinD(0),
                          mp1->BoundingBoxSpatial().MaxD(0),
                          mp1->BoundingBoxSpatial().MinD(1),
                          mp1->BoundingBoxSpatial().MaxD(1)
                        );
//        box_list[box_list.size() - 1] = tm_box1;//update the one in the list
//        delete mp1;

        MPoint* mp2 = new MPoint(0);
        mp2->StartBulkLoad();
        double st2, et2;
//        cout<<" div_pos "<<div_pos<<" "<<mp->GetNoComponents()<<endl;
        for(int k = div_pos; k < mp->GetNoComponents();k++){
          UPoint u;
          mp->Get(k, u);
          mp2->Add(u);
          if(k == div_pos) st2 = u.timeInterval.start.ToDouble();
          if(k == mp->GetNoComponents() - 1) 
            et2 = u.timeInterval.end.ToDouble();

        }
        mp2->EndBulkLoad();

        box_list[box_list.size() - 1] = tm_box1;//update the one in the list
        mp_list[mp_list.size() - 1] = *mp1;//update the one in the list

        mp_list.push_back(*mp2);
        oid_list.push_back(oid);

        Rectangle<3> tm_box2(true, st2, et2,
                          mp2->BoundingBoxSpatial().MinD(0),
                          mp2->BoundingBoxSpatial().MaxD(0),
                          mp2->BoundingBoxSpatial().MinD(1),
                          mp2->BoundingBoxSpatial().MaxD(1)
                        );
        box_list.push_back(tm_box2);
        tm_list.push_back(TM_WALK);//use walk insead of free

        delete mp1;
        delete mp2;
      }
    }

}

/*
collect movement tuples for car, taxi and bike
for each movement on one road, get the units in mp
use start and end time instants 

*/
void QueryTM::CollectCBT(int&i, int oid, int m, GenMO* mo1, 
                         MPoint* mo2, int& up_pos, double len)
{
//   cout<<"CollectCBT"<<endl;

  int index1 = i;
  int j = i;

  int64_t start_time = -1;
  int64_t end_time = -1;

  double l = 0;

  int last_rid = -1;
  while(j < mo1->GetNoComponents()){
    UGenLoc unit;
    mo1->Get(j, unit);

    if(unit.GetTM() == m){
      if(start_time < 0) 
        start_time = unit.timeInterval.start.ToDouble()*ONEDAY_MSEC;

      end_time = unit.timeInterval.end.ToDouble()*ONEDAY_MSEC;

      l += fabs(unit.gloc1.GetLoc().loc1 - unit.gloc2.GetLoc().loc1);
//       cout<<unit.gloc1<<" "<<unit.gloc2<<endl;
//       cout<<"l "<<l<<endl;

      int cur_rid = unit.GetOid();
      if(last_rid == -1){
        last_rid = cur_rid;
//      }else if(last_rid != cur_rid){
      }else if(last_rid != cur_rid && l > len){//driving length is long

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

          mp_list.push_back(*mp);//store a subtrip

//        Periods* peri = new Periods(0);
//        mp->DefTime(*peri);
//        delete peri;

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
//          index_list1.push_back(p1); //genmo index
//          index_list2.push_back(p2); //mpoint index

          index1 = j;
          last_rid = cur_rid;

          start_time = -1;
          end_time = -1;
//          delete peri;
          delete mp;

//          delete peri_t;
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

      mp_list.push_back(*mp);

//    Periods* peri = new Periods(0);
//    mp->DefTime(*peri);
//    delete peri;

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
//      index_list1.push_back(p1);
//      index_list2.push_back(p2);

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
void QueryTM::CollectIndoor(int& i, int oid, int m,
                                GenMO* mo1, MPoint* mo2, int& up_pos)
{
//  cout<<"CollectIndoorFree"<<endl;

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

  mp_list.push_back(*mp);//store a sub trip

  ////////////////////////////////////////////////////////////////////
  oid_list.push_back(oid);

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

  ///////////////////////////////////////////////////////////////////
//  delete peri_t;
//  delete peri;
  delete mp;

  i = j - 1;
}

/*
ignore such a small piece of movement to compute movement tuples

*/

void QueryTM::CollectFree_0(int& i, int oid, int m,
                                GenMO* mo1, MPoint* mo2, int& up_pos)
{
//  cout<<"CollectIndoorFree"<<endl;

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

  ////////////////////////////////////////////////////////////////////

  Rectangle<3> tm_box(true,
                      start_time/ONEDAY_MSEC,
                      end_time/ONEDAY_MSEC,
                      mp->BoundingBoxSpatial().MinD(0),
                      mp->BoundingBoxSpatial().MaxD(0),
                      mp->BoundingBoxSpatial().MinD(1),
                      mp->BoundingBoxSpatial().MaxD(1)
                     );

  mp_list.push_back(*mp);//store a sub trip
  oid_list.push_back(oid);
  box_list.push_back(tm_box);
//  tm_list.push_back(m);
  tm_list.push_back(TM_WALK);//here, we use walk instead of free

  Point p1(true, index1, index2);

  assert(pos1 >= 0 && pos2 >= pos1);
  Point p2(true, pos1, pos2);//index in mpoint

  ///////////////////////////////////////////////////////////////////
//  delete peri_t;
//  delete peri;
  delete mp;

  i = j - 1;
}

/*
almost the same as collectfree 0 but here we get two movement tuples if the 
trip contains more than one units, this is to combine walk and m in the future

*/
void QueryTM::CollectFree_1(int& i, int oid, int m,
                                GenMO* mo1, MPoint* mo2, int& up_pos)
{
//  cout<<"CollectIndoorFree"<<endl;

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

  ////////////////////////////////////////////////////////////////////

  Rectangle<3> tm_box(true,
                      start_time/ONEDAY_MSEC,
                      end_time/ONEDAY_MSEC,
                      mp->BoundingBoxSpatial().MinD(0),
                      mp->BoundingBoxSpatial().MaxD(0),
                      mp->BoundingBoxSpatial().MinD(1),
                      mp->BoundingBoxSpatial().MaxD(1)
                     );

//  mp_list.push_back(*mp);//store a sub trip
//  oid_list.push_back(oid);
//  box_list.push_back(tm_box);
//  tm_list.push_back(TM_WALK);//be careful, use walk instead of free
//  cout<<tm_box<<endl;


    if(mp->GetNoComponents() == 1){///////////only one unit, one mtuples
      mp_list.push_back(*mp);//store a sub trip
      oid_list.push_back(oid);
      box_list.push_back(tm_box);
      tm_list.push_back(TM_WALK);//be careful, use walk instead of free

    }else if(tm_list.size() > 0 && 
             tm_list[tm_list.size() - 1] == TM_WALK){//do not split
      mp_list.push_back(*mp);//store a sub trip
      oid_list.push_back(oid);
      box_list.push_back(tm_box);
      tm_list.push_back(TM_WALK);//be careful, use walk instead of free

    }else{////////two mtuples
        int div_pos = (int)(mp->GetNoComponents()/2.0);
        MPoint* mp1 = new MPoint(0);
        mp1->StartBulkLoad();
        double st1, et1;
        for(int i = 0;i < div_pos;i++){
          UPoint u;
          mp->Get(i, u);
          mp1->Add(u);
          if(i == 0) st1 = u.timeInterval.start.ToDouble();
          if(i == div_pos - 1)et1 = u.timeInterval.end.ToDouble();
        }

        mp1->EndBulkLoad();

        mp_list.push_back(*mp1);
        oid_list.push_back(oid);

        Rectangle<3> tm_box1(true, st1, et1,
                          mp1->BoundingBoxSpatial().MinD(0),
                          mp1->BoundingBoxSpatial().MaxD(0),
                          mp1->BoundingBoxSpatial().MinD(1),
                          mp1->BoundingBoxSpatial().MaxD(1)
                        );
        box_list.push_back(tm_box1);
//    cout<<"sub box1 "<<tm_box1<<endl;
        tm_list.push_back(TM_WALK);//use walk instead of free
        delete mp1;

        MPoint* mp2 = new MPoint(0);
        mp2->StartBulkLoad();
        double st2, et2;
        for(int i = div_pos; i < mp->GetNoComponents();i++){
          UPoint u;
          mp->Get(i, u);
          mp2->Add(u);
          if(i == div_pos) st2 = u.timeInterval.start.ToDouble();
          if(i == mp->GetNoComponents() - 1) 
            et2 = u.timeInterval.end.ToDouble();
        }
        mp2->EndBulkLoad();

        mp_list.push_back(*mp2);
        oid_list.push_back(oid);

        Rectangle<3> tm_box2(true, st2, et2,
                          mp2->BoundingBoxSpatial().MinD(0),
                          mp2->BoundingBoxSpatial().MaxD(0),
                          mp2->BoundingBoxSpatial().MinD(1),
                          mp2->BoundingBoxSpatial().MaxD(1)
                        );
        box_list.push_back(tm_box2);
//    cout<<"sub box2 "<<tm_box2<<endl;

        tm_list.push_back(TM_WALK);//use walk insead of free
        delete mp2;
    }////////end for if else

  Point p1(true, index1, index2);

  assert(pos1 >= 0 && pos2 >= pos1);
  Point p2(true, pos1, pos2);//index in mpoint
  delete mp;

  i = j - 1;
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
//    bitset<ARR_SIZE(str_tm)> mode_bit(m);

    bitset<TM_SUM_NO> mode_bit(m);

//    assert(mode_bit.count() == 1);//one mode in a leaf node (before)

    assert(mode_bit.count() >= 1);//3D rtree and TMRtree
    ///////////////output result///////////////////////////////
    oid_list.push_back(nodeid);
    level_list.push_back(level);
    b_list.push_back(true);

    mode_list.push_back(m);// bit value to integer value
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

      mode_list.push_back(m);
      box_list.push_back(node->BoundingBox());
      entry_list.push_back(node->EntryCount());

      delete node;

      //////////////////////////////////////////////////////

  }

}

/*
range query on generic moving objects using TM-RTree
using a queue to store nodes
1) check mode; 2) check query window

*/
void QueryTM::RangeTMRTree(TM_RTree<3,TupleId>* tmrtree, Relation* units_rel, 
                           Relation* query_rel, int treetype)
{
//  cout<<units_rel->GetNoTuples()<<" "<<query_rel->GetNoTuples()<<endl;
//    cout<<"genmo no "<<genmo_rel->GetNoTuples()<<endl;

    if(query_rel->GetNoTuples() != 1){
      cout<<"one query tuple "<<endl;
      return;
    }
  for(int i = 1;i <= query_rel->GetNoTuples();i++){
    Tuple* tuple = query_rel->GetTuple(i, false);
    Periods* peri = (Periods*)tuple->GetAttribute(GM_TIME);
    Rectangle<2>* box = (Rectangle<2>*)tuple->GetAttribute(GM_SPATIAL);
    string mode = ((CcString*)tuple->GetAttribute(GM_Q_MODE))->GetValue();
//    cout<<*peri<<" "<<*box<<" "<<mode<<endl;
    //////////////////////////////////////////////////////////////////
    double min[3], max[3];
    Interval<Instant> time_span;
    peri->Get(0, time_span);

    min[0] = time_span.start.ToDouble();
    max[0] = time_span.end.ToDouble();
    min[1] = box->MinD(0);
    max[1] = box->MaxD(0);
    min[2] = box->MinD(1);
    max[2] = box->MaxD(1);
    Rectangle<3> query_box(true, min, max);
    //////////////////////////////////////////////////////////////////
    vector<long> seq_tm;
    int type = ModeType(mode, seq_tm); //seqtm bit value to integer
    if(type < 0){
      cout<<"mode error "<<mode<<endl;
      tuple->DeleteIfAllowed();
      break;
    }
    /////////////////////////////////////////////////////////////////////
    ////////////////////////single mode//////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    if(type == SINMODE){
      //////////////filter step ////////////////////////////

//      bitset<ARR_SIZE(str_tm)> modebits(seq_tm[0]);
      bitset<TM_SUM_NO> modebits(seq_tm[0]);//bit index to an anteger
      cout<<"single mode "<<GetModeStringExt(seq_tm[0])
          <<" "<<modebits.to_string()<<endl;
      assert(modebits.count() == 1);
      int bit_pos = -1;
      for(int i = 0;i < (int)modebits.size();i++){
        if(modebits.test(i)){
          bit_pos = i;
          break;
        }
      }
      int sin_mode_no = (int)ARR_SIZE(str_tm);
      assert(0 <= bit_pos && bit_pos <= sin_mode_no);

      if(treetype == TMRTREE)// TMRTree
        SinMode_Filter1(tmrtree, query_box, bit_pos, units_rel);
      else if(treetype == ADRTREE)//ADRTree
        SinMode_Filter2(tmrtree, query_box, bit_pos, units_rel);
      else if(treetype == RTREE3D){//3DRTRee
        SinMode_Filter3(tmrtree, query_box, bit_pos, units_rel);
      }else if(treetype == TMRTREETEST){ //only single mode in mtuples
        SinMode_Filter1(tmrtree, query_box, bit_pos, units_rel);
      } else{
        cout<<"wrong type "<<treetype<<endl;
        assert(false);
      }
      //////////////refinement ////////////////////////////////
      SinMode_Refinement(query_box, bit_pos, units_rel);
    }
    ///////////////////////////////////////////////////////////////
    ///////////////////// multiple modes //////////////////////////
    ////////////////////////////////////////////////////////////////
    if(type == MULMODE){
      //not necessary use the total length, because the value belongs to 
      // the first nine 
      bitset<ARR_SIZE(str_tm)> modebits(seq_tm[0]);//integer from bit index
      cout<<"multiple mode "<<GetModeStringExt(seq_tm[0])
          <<" "<<modebits.to_string()<<endl;
      assert(modebits.count() >= 2);

      vector<bool> bit_pos(ARR_SIZE(str_tm), false);// first part enough
      int mode_count = 0;
      for(unsigned int i = 0;i < modebits.size();i++){
        if(modebits.test(i)){
          bit_pos[i] = true;
          mode_count++;
        }
      }

      if(treetype == TMRTREE){//TMRTree filter multiple modes
       MulMode_Filter1(tmrtree, query_box, bit_pos, units_rel);
      }else if(treetype == ADRTREE) {//ADRTREE, simply stores the value
       MulMode_Filter2(tmrtree, query_box, bit_pos, units_rel);
      }else if(treetype == RTREE3D){//3DRTree
       MulMode_Filter3(tmrtree, query_box, bit_pos, units_rel);
      }else if(treetype == TMRTREETEST){//TMRTree only single mode
        MulMode_Filter1(tmrtree, query_box, bit_pos, units_rel);
      }else{
        cout<<"wrong tree type "<<endl;
        assert(false);
      }
       MulMode_Refinement(query_box, bit_pos, units_rel, mode_count);
    }
    ///////////////////////////////////////////////////////////////////
    /////////////////// a sequence of modes////////////////////////////
    //////////////////////////////////////////////////////////////////
    if(type == SEQMODE){//a sequence of modes
      cout<<"a sequence of modes "<<endl;
      vector<bool> bit_pos(ARR_SIZE(str_tm), false);//mark the index
      bitset<ARR_SIZE(str_tm)> modebits;
      modebits.reset();
      for(unsigned int i = 0;i < seq_tm.size();i++){
//         cout<<GetTMStrExt(seq_tm[i])<<" ";
          bit_pos[seq_tm[i]] = true;
          modebits.set(seq_tm[i]);
      }
//       cout<<endl;
//       cout<<modebits.to_string()<<" "
//           <<GetModeStringExt(modebits.to_ulong())<<endl;;

      ///////////split the mode value by walk + m///////////////
      assert(seq_tm.size() > 1);
      int sin_mode_no = ARR_SIZE(str_tm);
      vector<bool> m_bit_list(3*sin_mode_no, false);
      for(unsigned int i = 0;i < seq_tm.size();i++){
    //      cout<<GetTMStrExt(seq_tm[i])<<" ";
        if(i < seq_tm.size() - 1){
              unsigned int j = i + 1;
              int m1 = seq_tm[i];
              int m2 = seq_tm[j];
              assert(m1 != m2);
              if(m1 == TM_WALK){
                  m_bit_list[m2 + 2*sin_mode_no] = true;
              }else if(m2 == TM_WALK){
                  m_bit_list[m1 + sin_mode_no] = true;
              }else{
                  assert(false);
              }
        }
      }
      ///////////1 include these modes; 2 with a certain order/////////////
      if(treetype == TMRTREE){
          SeqMode_Filter1(tmrtree, query_box, m_bit_list);
      }else if(treetype == ADRTREE){
          SeqMode_Filter2(tmrtree, query_box, units_rel, m_bit_list);
      }else if(treetype == RTREE3D){
          SeqMode_Filter3(tmrtree, query_box, units_rel, m_bit_list);
      }else if(treetype == TMRTREETEST){//get all mtuples contain these modes
          SeqMode_Filter4(tmrtree, query_box, bit_pos, units_rel);
      }else{
        cout<<"wrong tree type "<<endl;
        assert(false);
      }
         SeqMode_Refinement(query_box, bit_pos, units_rel, seq_tm, m_bit_list);
    }

    tuple->DeleteIfAllowed();
  }

}

/*
get the type of range query: single mode or a sequence of modes
1: single mode or multiple modes but no order;
2: a sequence of modes
seq tm stores mode value(s)

*/
int QueryTM::ModeType(string mode, vector<long>& seq_tm)
{

  vector<int> pos_list1;//single or multiple modes
  vector<int> pos_list2;// a sequence of modes
  char buffer[16];
  int index = 0;
  for(unsigned int i = 0;i < mode.length();i++){
//    cout<<i<<" "<<mode[i]<<endl;
    if(isalpha(mode[i])){
      buffer[index] = mode[i];
      index++;
    }
    if(mode[i] == ','){
      buffer[index] = '\0';
//      cout<<i<<" "<<buffer<<endl;
      index = 0;
      string m(buffer);
//      cout<<m<<endl;
      pos_list1.push_back(GetTM(m));
    }
    if(mode[i] == ';'){
      buffer[index] = '\0';
//      cout<<i<<" "<<buffer<<endl;
      index = 0;
      string m(buffer);
//      cout<<m<<endl;
      pos_list2.push_back(GetTM(m));
    }
    if(i == mode.length() - 1){
      buffer[index] = '\0';
//      cout<<i<<" "<<buffer<<endl;
      index = 0;

      string m(buffer);
//      cout<<m<<endl;
      if(pos_list2.size() > 0){
        pos_list2.push_back(GetTM(m));
      }else{
        pos_list1.push_back(GetTM(m));
      }
    }
  }

//  cout<<pos_list1.size()<<" "<<pos_list2.size()<<endl;
  if(pos_list1.size() > 0 && pos_list2.size() > 0){//do not mix mul and seq
    return -1;
  }
  if(pos_list1.size() > 0){
//    bitset<ARR_SIZE(str_tm)> modebits;
    bitset<TM_SUM_NO> modebits;
    modebits.reset();
    for(unsigned int i = 0;i < pos_list1.size();i++){
//       cout<<"i "<<ARR_SIZE(str_tm)<<" "
//           <<pos_list1[i]<<" "
//           <<ARR_SIZE(str_tm) - 1 - pos_list1[i]<<endl;
//      cout<<pos_list1[i]<<endl;
      if(pos_list1[i] < 0) return -1;
      assert(pos_list1[i] >= 0);

      modebits.set((int)(pos_list1[i]), 1);
    }
//    cout<<modebits.to_ulong()<<endl;
    seq_tm.push_back(modebits.to_ulong());

    if(pos_list1.size() == 1)
        return SINMODE;
    else
      return MULMODE;
  }
  
  if(pos_list2.size() > 0){/////modes with a certain order
    // set the value in seqtm list//
    for(unsigned int i = 0;i < pos_list2.size();i++){
//      cout<<GetTMStrExt(pos_list2[i])<<" ";
        seq_tm.push_back(pos_list2[i]);//store the mode bit index
    }

    return SEQMODE;//a sequence of values
  }

  return -1;
}

inline bool ModeCheck1(bitset<TM_SUM_NO> node_bit, int bit_pos)
{
    if(node_bit.test(bit_pos)) return true;

    if(bit_pos == TM_WALK){//mode walk
      if(node_bit.to_ulong() >= (unsigned int)pow(2, ARR_SIZE(str_tm)))
        return true;
    }else{
      if(node_bit.test(bit_pos + ARR_SIZE(str_tm)) || 
         node_bit.test(bit_pos + 2*ARR_SIZE(str_tm)))return true;
    }
  return false;
}


/*
traverse TMRTree to find units that intersect the query box
query mode is a single mode 

*/
void QueryTM::SinMode_Filter1(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, int bit_pos,
                              Relation* units_rel)
{
    ////////////////////////////////////////////////////////////////
    Interval<Instant> time_span;
    Instant st(instanttype); 
    Instant et(instanttype);
    st.ReadFrom(query_box.MinD(0));
    time_span.start = st; 
    time_span.lc = true;
    et.ReadFrom(query_box.MaxD(0));
    time_span.end = et;
    time_span.rc = false;

    double min[2], max[2];
    min[0] = query_box.MinD(1);
    min[1] = query_box.MinD(2);
    max[0] = query_box.MaxD(1);
    max[1] = query_box.MaxD(2);
    Rectangle<2> spatial_box(true, min, max);
    Region query_reg(spatial_box);

    ////////////////////////////////////////////////////////////////
//    cout<<"SinModefilter query box "<<query_box<<endl;
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());

//    cout<<"bit pos "<<bit_pos<<endl;
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

       bitset<TM_SUM_NO> node_bit(node->GetTMValue());

//       cout<<nodeid<<" "<<GetModeStringExt(node->GetTMValue());

       bool res = ModeCheck1(node_bit, bit_pos);

//       if(node_bit.test(bit_pos)){///////transportation mode check
       if(res){///////transportation mode check
/*          cout<<"node id "<<nodeid
              <<" mode "<<GetModeString(node->GetTMValue())<<endl;*/
          if(node->IsLeaf()){
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];

                if(e.box.Intersects(query_box)){ //query window
                    unit_tid_list.push_back(e.info);
                }
            }

          }else{

            for(int j = 0;j < node->EntryCount();j++){//finish
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }
          node_count++;
       }

       delete node;
    }

    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;
}


/*
traverse ADRTree to find units that intersect the query box
query mode is a single mode 
in a leaf node it needs to acces units relation to get precise mode value

*/
void QueryTM::SinMode_Filter2(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, int bit_pos,
                              Relation* units_rel)
{
//    cout<<"SinModefilter 3DRtree query box "<<query_box<<endl;
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());

//    cout<<"bit pos "<<bit_pos<<endl;
    int sin_mode_no = (int)ARR_SIZE(str_tm);
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

       bitset<TM_SUM_NO> node_bit(node->GetTMValue());
       bool res = ModeCheck1(node_bit, bit_pos);
       if(res){
/*          cout<<"node id "<<nodeid
              <<" mode "<<GetModeString(node->GetTMValue())<<endl;*/
          if(node->IsLeaf()){
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];

                Tuple* mtuple = units_rel->GetTuple(e.info, false);
                int mode = ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
//              cout<<"bit_pos "<<bit_pos<<" "<< mode<<endl;
//              cout<<GetTMStr(bit_pos)<<" "<<GetTMStr(mode)<<endl;

//                 if(bit_pos == (total_size - mode) &&
//                    e.box.Intersects(query_box)){ //query window
//                    unit_tid_list.push_back(e.info);
//                 }
                if(bit_pos == TM_WALK){
                    if(bit_pos == mode || mode >= sin_mode_no){
                          if(e.box.Intersects(query_box)){
                            unit_tid_list.push_back(e.info);
                          }
                      }
                }else{
                  if(bit_pos == mode || (mode == bit_pos + sin_mode_no) ||
                     (mode == bit_pos + 2*sin_mode_no)){
                          if(e.box.Intersects(query_box)){
                            unit_tid_list.push_back(e.info);
                          }
                  }
                }
                mtuple->DeleteIfAllowed();
            }
          }else{

            for(int j = 0;j < node->EntryCount();j++){//finish
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }
          node_count++;
       }

       delete node;
    }
    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}

/*
traverse 3DRTree to find units that intersect the query box
no mode check for a node
query mode is a single mode 
in a leaf node it needs to acces units relation to get precise mode value

*/
void QueryTM::SinMode_Filter3(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, int bit_pos,
                              Relation* units_rel)
{

//    cout<<"SinModefilter 3DRtree query box "<<query_box<<endl;
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());

//    cout<<"bit pos "<<bit_pos<<endl;
    int sin_mode_no = (int)ARR_SIZE(str_tm);
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

/*          cout<<"node id "<<nodeid
              <<" mode "<<GetModeString(node->GetTMValue())<<endl;*/
          if(node->IsLeaf()){
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];

                Tuple* mtuple = units_rel->GetTuple(e.info, false);
                int mode = ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
//              cout<<"bit_pos "<<bit_pos<<" "<< total_size - mode<<endl;
//              cout<<GetTMStr(bit_pos)<<" "<<GetTMStr(mode)<<endl;

//                 if(bit_pos == (total_size - mode) &&
//                    e.box.Intersects(query_box)){ //query window
//                    unit_tid_list.push_back(e.info);
//                 }

                if(bit_pos == TM_WALK){
                    if(bit_pos == mode || mode >= sin_mode_no){
                          if(e.box.Intersects(query_box)){
                            unit_tid_list.push_back(e.info);
                          }
                      }
                }else{
                  if(bit_pos == mode || (mode == bit_pos + sin_mode_no) ||
                     (mode == bit_pos + 2*sin_mode_no)){
                          if(e.box.Intersects(query_box)){
                            unit_tid_list.push_back(e.info);
                          }
                  }
                }
                mtuple->DeleteIfAllowed();
            }

          }else{

            for(int j = 0;j < node->EntryCount();j++){//finish
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }
          node_count++;

       delete node;
    }

    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}

/*
from the result by traversing TMRtree, check the precise value
just return the complete trajectory if its sub trip satisfies the condition.
it needs much work to decompose the trajectory into pieces and it might not be
necessary

*/
void QueryTM::SinMode_Refinement(Rectangle<3> query_box, int bit_pos,
                                 Relation* units_rel)
{
//  cout<<"refinement candidate number "<<unit_tid_list.size()<<endl;

  Interval<Instant> time_span;
  Instant st(instanttype); 
  Instant et(instanttype);
  st.ReadFrom(query_box.MinD(0));
  time_span.start = st; 
  time_span.lc = true;
  et.ReadFrom(query_box.MaxD(0));
  time_span.end = et;
  time_span.rc = false;

  double min[2], max[2];
  min[0] = query_box.MinD(1);
  min[1] = query_box.MinD(2);
  max[0] = query_box.MaxD(1);
  max[1] = query_box.MaxD(2);
  Rectangle<2> spatial_box(true, min, max);
  Region query_reg(spatial_box);

  set<int> res_id;//store trajectory id;
  int sin_mode_no = (int)ARR_SIZE(str_tm);

  for(unsigned int i = 0;i < unit_tid_list.size();i++){

      Tuple* mtuple = units_rel->GetTuple(unit_tid_list[i], false);
      int traj_id = ((CcInt*)mtuple->GetAttribute(GM_TRAJ_ID))->GetIntval();

//       if(traj_id != 1247){
//         mtuple->DeleteIfAllowed();
//         continue;
//       }

      if(res_id.find(traj_id) != res_id.end()){
          mtuple->DeleteIfAllowed();
          continue;
      }

      int m = ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();//bit index
//      Rectangle<3>* mt_box = (Rectangle<3>*)mtuple->GetAttribute(GM_BOX);
//      printf("%.6f %.6f\n", mt_box->MinD(0), mt_box->MaxD(0));

//      assert((int)(ARR_SIZE(str_tm) - 1 - m) == bit_pos);
//     cout<<GetTMStrExt(m)<<" bit_pos "<<bit_pos<<"m "<<m<<endl;

      if(bit_pos == TM_WALK){
        assert(m == bit_pos || m >= sin_mode_no);
      }else{
        assert(m == bit_pos || m  == (bit_pos + sin_mode_no) ||
               m == (bit_pos + 2*sin_mode_no));
      }

//      cout<<"traj id "<<traj_id<<endl;
      /////////////////////////////////////////////////////////////
      //////////////////// get trajectory//////////////////////////
      /////////////////////////////////////////////////////////////
      MPoint* mo2 = (MPoint*)mtuple->GetAttribute(GM_SUBTRIP);
      int div_pos = ((CcInt*)mtuple->GetAttribute(GM_DIV))->GetIntval();

      int start = -1;
      int end = -1;
      if(div_pos < 0){//single mode case
        start = 0;
        end = mo2->GetNoComponents();
      }else{// a pair of modes
        if(bit_pos == TM_WALK){
            if(sin_mode_no <= m && m < 2*sin_mode_no){//second part
                start = div_pos;
                end = mo2->GetNoComponents();
            }else if(m >= 2*sin_mode_no && m < 3*sin_mode_no){//first part
                start = 0;
                end = div_pos;
            }else{
              assert(false);
            }
        }else{ ///////other modes

            if(sin_mode_no <= m && m < 2*sin_mode_no){
                start = 0;
                end = div_pos;
            }else if(m >= 2*sin_mode_no && m < 3*sin_mode_no){
                start = div_pos;
                end = mo2->GetNoComponents();
            }else{
              assert(false);
            }
        }
      }
      assert(0 <= start && start < end);

      bool query_res = CheckMPoint(mo2, start, end, time_span, &query_reg);

      if(query_res){//////return sub trips
          oid_list.push_back(traj_id);
          res_id.insert(traj_id);
      }

      ///////////////////////////////////////////////////////////
      mtuple->DeleteIfAllowed();
  }

}


/*
traverse TMRTree to find units that intersect the query box
query mode is multiple modes

*/
void QueryTM::MulMode_Filter1(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, vector<bool> bit_pos,
                              Relation* units_rel)
{
    ////////////////////////////////////////////////////////////////
    Interval<Instant> time_span;
    Instant st(instanttype); 
    Instant et(instanttype);
    st.ReadFrom(query_box.MinD(0));
    time_span.start = st; 
    time_span.lc = true;
    et.ReadFrom(query_box.MaxD(0));
    time_span.end = et;
    time_span.rc = false;

    double min[2], max[2];
    min[0] = query_box.MinD(1);
    min[1] = query_box.MinD(2);
    max[0] = query_box.MaxD(1);
    max[1] = query_box.MaxD(2);
    Rectangle<2> spatial_box(true, min, max);
    Region query_reg(spatial_box);

    ////////////////////////////////////////////////////////////////
//    cout<<"SinModefilter query box "<<query_box<<endl;
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());
    int sin_mode_no = ARR_SIZE(str_tm);

//    cout<<"bit pos "<<bit_pos<<endl;
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

       bitset<TM_SUM_NO> node_bit(node->GetTMValue());
//       cout<<nodeid<<" "<<node_bit.to_string()<<endl;
       bool node_mode = false;
       for(unsigned int i = 0;i < bit_pos.size();i++){//check existence
         if(bit_pos[i] == false || i == TM_WALK) continue;
          ////for other modes, connected to m + walk///////////
          if(node_bit.test(i) || node_bit.test(i + sin_mode_no) ||
              node_bit.test(i + 2*sin_mode_no)){
              node_mode = true;
              break;
          }
       }
       //////////////////////////////////////////////////////////////
       if(bit_pos[TM_WALK]){//////////including walking
            if(node_bit.test(TM_WALK) || 
               node_bit.test(sin_mode_no + TM_INDOORWALK) ||
               node_bit.test(sin_mode_no + TM_WALKINDOOR)){
              node_mode = true;
            }
       }

//     cout<<endl<<GetModeStringExt(node->GetTMValue())<<" "<<node_mode<<endl;

       if(node_mode){
/*          cout<<"node id "<<nodeid
              <<" mode "<<GetModeStringExt(node->GetTMValue())<<endl;*/
          if(node->IsLeaf()){
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];

                if(e.box.Intersects(query_box)){ //query window
                    unit_tid_list.push_back(e.info);
                }
            }

          }else{

            for(int j = 0;j < node->EntryCount();j++){//finish
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }
          node_count++;
       }

       delete node;
    }

    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;
}


/*
traverse ADRTree to find units that intersect the query box
query mode is multiple modes

*/
void QueryTM::MulMode_Filter2(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, vector<bool> bit_pos,
                              Relation* units_rel)
{
  //    cout<<"SinModefilter 3DRtree query box "<<query_box<<endl;
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());

    int sin_mode_no = (int)ARR_SIZE(str_tm);
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));
       bitset<TM_SUM_NO> node_bit(node->GetTMValue());

       bool node_mode = false;
       for(unsigned int i = 0;i < bit_pos.size();i++){//check existence
         if(bit_pos[i] == false || i == TM_WALK) continue;
          ////for other modes, connected to m + walk///////////
          if(node_bit.test(i) || node_bit.test(i + sin_mode_no) ||
              node_bit.test(i + 2*sin_mode_no)){
              node_mode = true;
              break;
          }
       }
       /////////////////more than two modes////////////////////////
       //////////////can not be only walk/////////////////////////
       if(bit_pos[TM_WALK]){//////////including walking
            if(node_bit.test(TM_WALK) || 
               node_bit.test(sin_mode_no + TM_INDOORWALK) ||
               node_bit.test(sin_mode_no + TM_WALKINDOOR)){
              node_mode = true;
            }
       }
       if(node_mode){

           if(node->IsLeaf()){
             for(int j = 0;j < node->EntryCount();j++){
                 R_TreeLeafEntry<3, TupleId> e =
                  (R_TreeLeafEntry<3, TupleId>&)(*node)[j];
                  Tuple* mtuple = units_rel->GetTuple(e.info, false);
//                   int mode = 
//                    ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
                   if(e.box.Intersects(query_box)){
//                        cout<<GetTMStrExt(mode)<<endl;
                        int mode = 
                        ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
                        bool res = false;
                        if(mode < sin_mode_no){
                          if(bit_pos[mode]) res = true;
                        }else if(sin_mode_no <= mode && mode < 2*sin_mode_no){
                          int m1 = mode - sin_mode_no;
                          int m2= TM_WALK;
                          if(bit_pos[m1] || bit_pos[m2]) res = true;
                        }else if(2*sin_mode_no <= mode && mode < 3*sin_mode_no){
                          int m1 = TM_WALK;
                          int m2 = mode - 2*sin_mode_no;
                          if(bit_pos[m1] || bit_pos[m2]) res = true;
                        }else{
                          assert(false);
                        }
                        if(res){
                            unit_tid_list.push_back(e.info);
                        }
                   }

//                  for(int i = 0;i < (int)bit_pos.size();i++){
//                   if(bit_pos[i] == false) continue;
//                   if(bit_pos[i] == TM_WALK){
//                     if(mode >= sin_mode_no && e.box.Intersects(query_box)){
//                         unit_tid_list.push_back(e.info);
//                     }
//                   }else{
//                     if( ((i == sin_mode_no) || (i + sin_mode_no) == mode ||
//                         (i + 2*sin_mode_no) == mode) &&
//                      e.box.Intersects(query_box)){//query window, mode check
//                         unit_tid_list.push_back(e.info);
//                     }
//                   }
// 
//                  }
                 mtuple->DeleteIfAllowed();
             }

           }else{

              for(int j = 0;j < node->EntryCount();j++){//finish
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
              }
           }

          node_count++;
       }

       delete node;
    }
    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}

/*
3DRtree for multiple modes, there are no modes information in the node

*/
void QueryTM::MulMode_Filter3(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, vector<bool> bit_pos,
                              Relation* units_rel)
{
  //    cout<<"SinModefilter 3DRtree query box "<<query_box<<endl;
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());

    int sin_mode_no = (int)ARR_SIZE(str_tm);
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

        if(node->IsLeaf()){
             for(int j = 0;j < node->EntryCount();j++){
                 R_TreeLeafEntry<3, TupleId> e =
                  (R_TreeLeafEntry<3, TupleId>&)(*node)[j];
                  Tuple* mtuple = units_rel->GetTuple(e.info, false);
                  int mode = 
                   ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
                   if(e.box.Intersects(query_box)){
//                        cout<<GetTMStrExt(mode)<<endl;
                        bool res = false;
                        if(mode < sin_mode_no){
                          if(bit_pos[mode]) res = true;
                        }else if(sin_mode_no <= mode && mode < 2*sin_mode_no){
                          int m1 = mode - sin_mode_no;
                          int m2= TM_WALK;
                          if(bit_pos[m1] || bit_pos[m2]) res = true;
                        }else if(2*sin_mode_no <= mode && mode < 3*sin_mode_no){
                          int m1 = TM_WALK;
                          int m2 = mode - 2*sin_mode_no;
                          if(bit_pos[m1] || bit_pos[m2]) res = true;
                        }else{
                          assert(false);
                        }
                        if(res){
                            unit_tid_list.push_back(e.info);
                        }
                   }

                 mtuple->DeleteIfAllowed();
             }

      }else{

              for(int j = 0;j < node->EntryCount();j++){//finish
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
              }
      }

       node_count++;
       delete node;
    }
    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}


/*
from the result by traversing TMRtree, check the precise value
just return the complete trajectory if its sub trip satisfies the condition.
it needs much work to decompose the trajectory into pieces and it might not be
necessary. multiple modes

*/
void QueryTM::MulMode_Refinement(Rectangle<3> query_box, vector<bool> bit_pos,
                                 Relation* units_rel, int mode_count)
{
//  cout<<"refinement candidate number "<<unit_tid_list.size()<<endl;

  Interval<Instant> time_span;
  Instant st(instanttype); 
  Instant et(instanttype);
  st.ReadFrom(query_box.MinD(0));
  time_span.start = st; 
  time_span.lc = true;
  et.ReadFrom(query_box.MaxD(0));
  time_span.end = et;
  time_span.rc = false;

  double min[2], max[2];
  min[0] = query_box.MinD(1);
  min[1] = query_box.MinD(2);
  max[0] = query_box.MaxD(1);
  max[1] = query_box.MaxD(2);
  Rectangle<2> spatial_box(true, min, max);
  Region query_reg(spatial_box);

  map<int, Traj_Mode> res_traj;//binary search tree
  int sin_mode_no = ARR_SIZE(str_tm);

  for(unsigned int i = 0;i < unit_tid_list.size();i++){

      Tuple* mtuple = units_rel->GetTuple(unit_tid_list[i], false);
      int traj_id = ((CcInt*)mtuple->GetAttribute(GM_TRAJ_ID))->GetIntval();

      int m = ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();

      MPoint* mp = (MPoint*)mtuple->GetAttribute(GM_SUBTRIP);
      int div_pos = ((CcInt*)mtuple->GetAttribute(GM_DIV))->GetIntval();
      map<int, Traj_Mode>::iterator iter = res_traj.find(traj_id);

//      cout<<"traj_id "<<traj_id<<" "<<GetTMStrExt(m)<<endl;
//      cout<<*mp<<endl;
      if(iter == res_traj.end()){//not exist, insert into the tree
//          cout<<"new tuple "<<endl;
          int start = -1;
          int end = -1;
          bitset<ARR_SIZE(str_tm)> m_bit;
          m_bit.reset();
          if(m < sin_mode_no){//////////0-8
              if(bit_pos[m]){ //such a mode exists in the query
                  start = 0;
                  end = mp->GetNoComponents();
              }else{//ignore if does not exist
                cout<<"should not occur"<<endl;
                mtuple->DeleteIfAllowed();
                assert(false);
                continue;
              }
              m_bit.set(m);
              if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                 Traj_Mode traj_mode(m_bit);
                 res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
              }
          }else if(sin_mode_no <= m && m < 2*sin_mode_no){ ///9-17
            int m1 = m - sin_mode_no;
            int m2 = TM_WALK;
            if(bit_pos[m1] && bit_pos[m2]){///both exist
/*              start = 0;
              end = mp->GetNoComponents();
              m_bit.set(m1);
              m_bit.set(m2);*/
                start = 0;
                end = div_pos;
                bool res1 = CheckMPoint(mp, start, end, time_span, &query_reg);
                if(res1) m_bit.set(m1);
                start = div_pos;
                end = mp->GetNoComponents();
                bool res2 = CheckMPoint(mp, start, end, time_span, &query_reg);
                if(res2) m_bit.set(m2);
                if(res1 || res2){
                    Traj_Mode traj_mode(m_bit);
                    res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
                }
            }else if(bit_pos[m1] && bit_pos[m2] == false){
              start = 0;
              end = div_pos;
              m_bit.set(m1);
              if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                Traj_Mode traj_mode(m_bit);
                res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
              }
            }else if(bit_pos[m1] == false && bit_pos[m2]){
              start = div_pos;
              end = mp->GetNoComponents();
              m_bit.set(m2);
              if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                Traj_Mode traj_mode(m_bit);
                res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
              }
            }else{
              assert(false);
            }
          }else if(2*sin_mode_no <= m && m < 3*sin_mode_no){//18-26
            int m1 = TM_WALK;
            int m2 = m - 2*sin_mode_no;
            if(bit_pos[m1] && bit_pos[m2]){
//               start = 0;
//               end = mp->GetNoComponents();
//               m_bit.set(m1);
//               m_bit.set(m2);
                start = 0;
                end = div_pos;
                bool res1 = CheckMPoint(mp, start, end, time_span, &query_reg);
                if(res1) m_bit.set(m1);
                start = div_pos;
                end = mp->GetNoComponents();
                bool res2 = CheckMPoint(mp, start, end, time_span, &query_reg);
                if(res2) m_bit.set(m2);
                if(res1 || res2){
                    Traj_Mode traj_mode(m_bit);
                    res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
                }
            }else if(bit_pos[m1] && bit_pos[m2] == false){
                start = 0;
                end = div_pos;
                m_bit.set(m1);
                if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                  Traj_Mode traj_mode(m_bit);
                  res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
                }
            }else if(bit_pos[m1] == false && bit_pos[m2]){
                start = div_pos;
                end = mp->GetNoComponents();
                m_bit.set(m2);
                if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                  Traj_Mode traj_mode(m_bit);
                  res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
                }
            }else{
              assert(false);
            }
          }else{
            assert(false);
          }
          //////temporal and spatial condition////////////////
//             if(CheckMPoint(mp, start, end, time_span, &query_reg)){
//                 Traj_Mode traj_mode(m_bit);
//                 res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
//             }
      }else{
//        iter->second.Print();
        if(iter->second.Mode_Count() == mode_count){//find the trajectory
              //do nothing
              //cout<<"exist already "<<endl;
        }else{
            assert(iter->second.Mode_Count() < mode_count);
            //check whether a new bit has to be marked///
            //satisfy temporal and spatial coniditon////
            /// need such a mode and does not exist or find already///
            int start = -1;
            int end = -1;
            bitset<ARR_SIZE(str_tm)> m_bit;
            m_bit.reset();
            if(m < sin_mode_no){//////////////0-8
              //already results
              if(bit_pos[m] == false || iter->second.modebits.test(m)){
                mtuple->DeleteIfAllowed();
                continue;
              }
              if(bit_pos[m]){
                  start = 0;
                  end = mp->GetNoComponents();
                  m_bit.set(m);
              }
              ///////////temporal and spatial checking//////////////////
              ////////mark the new bit////////////////////////
              if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                iter->second.modebits = iter->second.modebits | m_bit;
              }
            }else if(sin_mode_no <= m && m < 2*sin_mode_no){//9-17
              int m1 = m - sin_mode_no;
              int m2 = TM_WALK;
              if(bit_pos[m1] && iter->second.modebits.test(m1) == false){
                start = 0;
                end = div_pos;
                if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                  m_bit.set(m1);
                  iter->second.modebits = iter->second.modebits | m_bit;
                }
              }
              if(bit_pos[m2] && iter->second.modebits.test(m2) == false){
                start = div_pos;
                end = mp->GetNoComponents();
                if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                  m_bit.set(m2);
                  iter->second.modebits = iter->second.modebits | m_bit;
                }
              }
            }else if(2*sin_mode_no <= m && m < 3*sin_mode_no){//18-26
                int m1 = TM_WALK;
                int m2 = m - 2*sin_mode_no;
                if(bit_pos[m1] && iter->second.modebits.test(m1) == false){
                  start = 0;
                  end = div_pos;
                  if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                    m_bit.set(m1);
                    iter->second.modebits = iter->second.modebits | m_bit;
                  }
                }
                if(bit_pos[m2] && iter->second.modebits.test(m2) == false){
                  start = div_pos;
                  end = mp->GetNoComponents();
                  if(CheckMPoint(mp, start, end, time_span, &query_reg)){
                    m_bit.set(m2);
                    iter->second.modebits = iter->second.modebits | m_bit;
                  }
                }
            }else{
              assert(false);
            }

        }

      }

      ///////////////////////////////////////////////////////////
      mtuple->DeleteIfAllowed();
  }
  
  map<int, Traj_Mode>::iterator iter;
  for(iter = res_traj.begin(); iter != res_traj.end();iter++){
    if(iter->second.Mode_Count() == mode_count){
//        iter->second.Print();
        oid_list.push_back(iter->first);
    }
  }

}

/*
check the spatial and temporal condition of a sub trip

*/

bool QueryTM::CheckMPoint(MPoint* mp, int start, int end, 
                          Interval<Instant>& time_span, Region* query_reg)
{
//     cout<<"CheckMPoint "<<endl;
     for(;start < end; start++){
         UPoint up;
         mp->Get(start, up);
//         ///////////////precise value checking/////////////////
         if(time_span.Intersects(up.timeInterval)){
           UPoint up2;
           up.AtInterval(time_span, up2);///////////time interval
           if(!AlmostEqual(up2.p0, up2.p1)){
             HalfSegment hs(true, up2.p0, up2.p1);
             if(query_reg->Intersects(hs)){//find the result, terminate
              return true;
             }
           }else{
             if(up2.p0.Inside(*query_reg)){//find the result, terminate
              return true;
             }
           }
         }
       }
    return false;
}

/*
simple (baseline) method to test the correctness 

*/
void QueryTM::RangeQuery(Relation* rel1, Relation* rel2)
{
//  cout<<rel1->GetNoTuples()<<" "<<rel2->GetNoTuples()<<endl;
  assert(rel2->GetNoTuples() == 1);
  Tuple* q_tuple = rel2->GetTuple(1, false);
  Periods* peri = (Periods*)q_tuple->GetAttribute(GM_TIME);
  Rectangle<2>* q_box = (Rectangle<2>*)q_tuple->GetAttribute(GM_SPATIAL);
  string mode = ((CcString*)q_tuple->GetAttribute(GM_Q_MODE))->GetValue();

  vector<long> seq_tm;
  int type = ModeType(mode, seq_tm); //seqtm bit value to integer
  if(type < 0){
      cout<<"mode error "<<mode<<endl;
      q_tuple->DeleteIfAllowed();
      return;
  }

  if(type == SINMODE){
//      cout<<"single mode "<<endl;
      int m = GetTM(mode);
      Sin_RangeQuery(rel1, peri, q_box, m);
  }
  if(type == MULMODE){
      Mul_RangeQuery(rel1, peri, q_box, seq_tm[0]);
  }
  if(type == SEQMODE){
      Seq_RangeQuery(rel1, peri, q_box, seq_tm);
  }


  q_tuple->DeleteIfAllowed();
}

/*
a single mode: baseline method

*/
void QueryTM::Sin_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box,
                             int m)
{
  Interval<Instant> time_span;
  peri->Get(0, time_span);
  Region query_reg(*q_box);

//  cout<<*peri<<" "<<*q_box<<" "<<GetTMStrExt(m)<<endl;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple = rel1->GetTuple(i, false);
    int traj_id = ((CcInt*)tuple->GetAttribute(GENMO_OID))->GetIntval();
    GenMO* mo1 = (GenMO*)tuple->GetAttribute(GENMO_TRIP1);

//     if(traj_id != 1247){
//       tuple->DeleteIfAllowed();
//       continue;
//     }

    Periods* mo_t = new Periods(0);
    mo1->DefTime(*mo_t);
    if(mo_t->Intersects(*peri) == false){
      delete mo_t;
      tuple->DeleteIfAllowed();
      continue;
    }
    delete mo_t;

    MPoint* mo2 = (MPoint*)tuple->GetAttribute(GENMO_TRIP2);
    MPoint* sub_mo = new MPoint(0);
    mo2->AtPeriods(*peri, *sub_mo);
    if(sub_mo->BoundingBoxSpatial().Intersects(*q_box) == false){
      delete sub_mo;
      tuple->DeleteIfAllowed();
      continue;
    }

    MReal* mode_index_tmp = new MReal(0);
//    mo1->IndexOnUnits(mode_index_tmp);
    mo1->IndexOnUnits2(mode_index_tmp);

    MReal* mode_index = new MReal(0);
    mode_index_tmp->AtPeriods(*peri, *mode_index);
//    cout<<*mode_index<<endl;
    delete mode_index_tmp;

    if(ContainMode1_Sin(mode_index, m) == false){
      delete sub_mo;
      delete mode_index;
      tuple->DeleteIfAllowed();
      continue;
    }
    /////////////////////////////////////////////////////
    bool query_res = false;
    for(int j = 0;j < sub_mo->GetNoComponents();j++){
      UPoint up;
      sub_mo->Get(j, up);
      if(time_span.Intersects(up.timeInterval)){
           UPoint up2;
           up.AtInterval(time_span, up2);///////////time interval
           if(!AlmostEqual(up2.p0, up2.p1)){
             HalfSegment hs(true, up2.p0, up2.p1);
             if(query_reg.Intersects(hs)){//find the result, terminate
               if(ContainMode2_Sin(mode_index, up2.timeInterval, m)){
                  query_res = true;
                  break;
               }
             }
           }else{
             if(up2.p0.Inside(query_reg)){//find the result, terminate
               if(ContainMode2_Sin(mode_index, up2.timeInterval, m)){
                  query_res = true;
                  break;
               }
             }
           }
      }
    }

    delete sub_mo;
    delete mode_index;

    if(query_res){
      oid_list.push_back(traj_id);
    }

    tuple->DeleteIfAllowed();
  }

}

/*
baseline method: multiple modes

*/
void QueryTM::Mul_RangeQuery(Relation* rel1, Periods* peri, Rectangle<2>* q_box,
                             int m)
{
  Interval<Instant> time_span;
  peri->Get(0, time_span);
  Region query_reg(*q_box);
  
  // the first nine 
  bitset<ARR_SIZE(str_tm)> modebits(m);//integer from bit index
  cout<<"baseline method multiple mode "<<GetModeStringExt(m)
      <<" "<<modebits.to_string()<<endl;
  assert(modebits.count() >= 2);

  vector<bool> bit_pos(ARR_SIZE(str_tm), false);// first part enough
  int mode_count = 0;
  for(unsigned int i = 0;i < modebits.size();i++){
     if(modebits.test(i)){
        bit_pos[i] = true;
        mode_count++;
     }
  }

 map<int, Traj_Mode> res_traj;//binary search tree
  
 for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple = rel1->GetTuple(i, false);
    int traj_id = ((CcInt*)tuple->GetAttribute(GENMO_OID))->GetIntval();
    GenMO* mo1 = (GenMO*)tuple->GetAttribute(GENMO_TRIP1);

    Periods* mo_t = new Periods(0);
    mo1->DefTime(*mo_t);
    if(mo_t->Intersects(*peri) == false){
      delete mo_t;
      tuple->DeleteIfAllowed();
      continue;
    }
    delete mo_t;

    MPoint* mo2 = (MPoint*)tuple->GetAttribute(GENMO_TRIP2);
    MPoint* sub_mo = new MPoint(0);
    mo2->AtPeriods(*peri, *sub_mo);
    if(sub_mo->BoundingBoxSpatial().Intersects(*q_box) == false){
      delete sub_mo;
      tuple->DeleteIfAllowed();
      continue;
    }
    ///////////////////////////////////////////////////////////////
    MReal* mode_index_tmp = new MReal(0);
    mo1->IndexOnUnits(mode_index_tmp);

    MReal* mode_index = new MReal(0);
    mode_index_tmp->AtPeriods(*peri, *mode_index);
//    cout<<*mode_index<<endl;
    delete mode_index_tmp;

    if(ContainMode1_Mul(mode_index, bit_pos, mode_count) == false){
      delete sub_mo;
      delete mode_index;
      tuple->DeleteIfAllowed();
      continue;
    }
    ///////////////spatial and temporal checking//////////////////////////
    for(int j = 0;j < sub_mo->GetNoComponents();j++){
      UPoint up;
      sub_mo->Get(j, up);
      if(time_span.Intersects(up.timeInterval)){
           UPoint up2;
           up.AtInterval(time_span, up2);///////////time interval
           if(!AlmostEqual(up2.p0, up2.p1)){
             HalfSegment hs(true, up2.p0, up2.p1);
             if(query_reg.Intersects(hs)){//find the result, terminate
              ContainMode2_Mul(res_traj, mode_index, up2.timeInterval,
                            bit_pos, traj_id, mode_count);
             }
           }else{
             if(up2.p0.Inside(query_reg)){//find the result, terminate
              ContainMode2_Mul(res_traj, mode_index, up2.timeInterval,
                             bit_pos, traj_id, mode_count);

             }
           }
      }
    }

    delete sub_mo;
    delete mode_index;

    tuple->DeleteIfAllowed();

 }
 
  /////////////////////collect the result///////////////////////////
  map<int, Traj_Mode>::iterator iter;
  for(iter = res_traj.begin(); iter != res_traj.end();iter++){
    if(iter->second.Mode_Count() == mode_count){
        oid_list.push_back(iter->first);
    }
  }

}


/*
check whether a mode is included 

*/
bool QueryTM::ContainMode1_Sin(MReal* mode_index, int m)
{
    for(int i = 0;i < mode_index->GetNoComponents();i++){
      UReal ur;
      mode_index->Get(i, ur);
      if((int)(ur.a) == m){
        return true;
      }
    }
    return false;
}

/*
check several modes are included
all of modes have to be considered

*/
bool QueryTM::ContainMode1_Mul(MReal* mode_index, vector<bool> bit_pos,
                               int m_count)
{
     int m_no = 0;

     vector<bool> input_mode(ARR_SIZE(str_tm), false);
     for(int i = 0;i < mode_index->GetNoComponents();i++){
          UReal ur;
          mode_index->Get(i, ur);
          int m = (int)ur.a;
          input_mode[m] = true;
     }

     for(unsigned int i = 0;i < bit_pos.size();i++){
      if(input_mode[i] && bit_pos[i]) m_no++;
     }
     if(m_no == m_count) return true;

      return false;
}

/*
from mreal, get a sub unit, check the transportation mode

*/
bool QueryTM::ContainMode2_Sin(MReal* mode_index, Interval<Instant>& t, int m)
{
    for(int j = 0;j < mode_index->GetNoComponents();j++){
      UReal ur;
      mode_index->Get(j, ur);
      if(t.Intersects(ur.timeInterval)){
           UReal ur2;
           ur.AtInterval(t, ur2);///////////time interval
           if((int)(ur2.a) == m)return true;

      }
    }
    return false;
}

/*
from mreal, get a sub unit, check the transportation mode
only mark bit (modes) that are asked by the query

*/
void QueryTM::ContainMode2_Mul(map<int, Traj_Mode>& res_traj, MReal* mode_index,
                              Interval<Instant>& t, 
                              vector<bool> bit_pos, int traj_id, int mode_count)
{
    bool res = false;
    int m = -1;
    for(int j = 0;j < mode_index->GetNoComponents();j++){
      UReal ur;
      mode_index->Get(j, ur);
      if(t.Intersects(ur.timeInterval)){
           UReal ur2;
           ur.AtInterval(t, ur2);///////////time interval
           m = (int)(ur2.a);
           if(bit_pos[m]){ 
             res = true;
             break;
           }
      }
    }
    if(res == false) return;
    map<int, Traj_Mode>::iterator iter = res_traj.find(traj_id);
    if(iter == res_traj.end()){/////////a new one, insert
        bitset<ARR_SIZE(str_tm)> m_bit;
        m_bit.reset();
        m_bit.set(m);
        Traj_Mode traj_mode(m_bit);
        res_traj.insert(pair<int, Traj_Mode>(traj_id, traj_mode));
    }else{
        if(iter->second.Mode_Count() == mode_count)return;
        bitset<ARR_SIZE(str_tm)> m_bit;/////mark the new bit mode
        m_bit.reset();
        m_bit.set(m);

        iter->second.modebits = iter->second.modebits | m_bit;
    }

}

/*
baseline method to check a sequence of modes

*/
void QueryTM::Seq_RangeQuery(Relation* rel1, Periods* peri, 
                             Rectangle<2>* q_box, vector<long> seq_tm)
{
//   for(unsigned int i = 0;i < seq_tm.size();i++){
//     cout<<GetTMStrExt(seq_tm[i])<<" ";
//   }

  Interval<Instant> time_span;
  peri->Get(0, time_span);
  Region query_reg(*q_box);

  vector<bool> bit_pos(ARR_SIZE(str_tm), false);// first part enough
  bitset<ARR_SIZE(str_tm)> modebits;//different multiple modes
  modebits.reset();//indoor;walk;bus;walk, the result is 3 instead of 4
  for(unsigned int i = 0;i < seq_tm.size();i++){
      bit_pos[seq_tm[i]] = true;
      modebits.set(seq_tm[i], 1);
  }

  int mode_count =  modebits.count(); //some modes appear more then once

  map<int, Seq_Mode> res_traj;//binary search tree
  for(int i = 1;i <= rel1->GetNoTuples();i++){
     Tuple* tuple = rel1->GetTuple(i, false);
     int traj_id = ((CcInt*)tuple->GetAttribute(GENMO_OID))->GetIntval();
     GenMO* mo1 = (GenMO*)tuple->GetAttribute(GENMO_TRIP1);
      Periods* mo_t = new Periods(0);
      mo1->DefTime(*mo_t);
      if(mo_t->Intersects(*peri) == false){
        delete mo_t;
        tuple->DeleteIfAllowed();
        continue;
      }
      delete mo_t;
      MPoint* mo2 = (MPoint*)tuple->GetAttribute(GENMO_TRIP2);
      MPoint* sub_mo = new MPoint(0);
      mo2->AtPeriods(*peri, *sub_mo);
      if(sub_mo->BoundingBoxSpatial().Intersects(*q_box) == false){
       delete sub_mo;
       tuple->DeleteIfAllowed();
       continue;
     }

      ///////////////////////////////////////////////////////////////
      MReal* mode_index_tmp = new MReal(0);
      mo1->IndexOnUnits(mode_index_tmp);

      MReal* mode_index = new MReal(0);
      mode_index_tmp->AtPeriods(*peri, *mode_index);
    //    cout<<*mode_index<<endl;
      delete mode_index_tmp;
      //such a sub trip, all modes are included////////////
     if(ContainMode1_Mul(mode_index, bit_pos, mode_count) == false){
        delete sub_mo;
        delete mode_index;
        tuple->DeleteIfAllowed();
        continue;
     }

    ///////////////spatial and temporal checking//////////////////////////
     for(int j = 0;j < sub_mo->GetNoComponents();j++){
      UPoint up;
      sub_mo->Get(j, up);

      if(time_span.Intersects(up.timeInterval)){
           UPoint up2;
           up.AtInterval(time_span, up2);///////////time interval
           if(!AlmostEqual(up2.p0, up2.p1)){
              HalfSegment hs(true, up2.p0, up2.p1);
              if(query_reg.Intersects(hs)){//
                ContainMode3_Seq(res_traj, mode_index, bit_pos, 
                                 up2.timeInterval, traj_id, 
                                 seq_tm);

              }
           }else{
              if(up2.p0.Inside(query_reg)){//
                ContainMode3_Seq(res_traj, mode_index, bit_pos,
                                 up2.timeInterval, traj_id, 
                                 seq_tm);
              }
           }
      }
    }


      delete sub_mo;
      delete mode_index;
      tuple->DeleteIfAllowed();
  }

  /////////////////////collect the result///////////////////////////
  map<int, Seq_Mode>::iterator iter;
  for(iter = res_traj.begin(); iter != res_traj.end();iter++){
    if(iter->second.Status()){
        oid_list.push_back(iter->first);
    }
  }

}

/*
simple method: check a sequence of modes

*/
void QueryTM::ContainMode3_Seq(map<int, Seq_Mode>& res_traj, MReal* mode_index,
                                  vector<bool> bit_pos,
                                  Interval<Instant>& t, 
                                  int traj_id, vector<long> seq_tm)
{
    bool res = false;
    int m = -1;
    for(int j = 0;j < mode_index->GetNoComponents();j++){//check mode
      UReal ur;
      mode_index->Get(j, ur);
      if(t.Intersects(ur.timeInterval)){
           UReal ur2;
           ur.AtInterval(t, ur2);///////////time interval
           m = (int)(ur2.a);
           if(bit_pos[m]){ 
             res = true;
             break;
           }
      }
    }
    if(res == false) return;
    assert(0 <= m && m < (int)(ARR_SIZE(str_tm)));
    map<int, Seq_Mode>::iterator iter = res_traj.find(traj_id);
    if(iter == res_traj.end()){//a new one, insert
        double st = t.start.ToDouble();
        double et = t.end.ToDouble();
        MInt mp_tm(0);
        MakeTMUnit(mp_tm, st, et, m);
        Seq_Mode seq_m(mp_tm);
        res_traj.insert(pair<int, Seq_Mode>(traj_id, seq_m));
    }else{
      if(iter->second.Status()){//doing nothing, find the result already

      }else{
          double st = t.start.ToDouble();
          double et = t.end.ToDouble();
          iter->second.Update(st, et, m);
          iter->second.CheckStatus(seq_tm);
      }

    }

}

/*
TMRTree, filter step: a sequence of modes

*/
void QueryTM::SeqMode_Filter1(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, vector<bool> m_bit_list)
{

//     for(unsigned int i = 0;i < m_bit_list.size();i++){
//       cout<<" i "<<i<<" "<<m_bit_list[i]<<endl;
//       if(m_bit_list[i])cout<<GetTMStrExt(i)<<endl;
//     }
  /////////////////////////////////////////////////////////
    Interval<Instant> time_span;
    Instant st(instanttype); 
    Instant et(instanttype);
    st.ReadFrom(query_box.MinD(0));
    time_span.start = st; 
    time_span.lc = true;
    et.ReadFrom(query_box.MaxD(0));
    time_span.end = et;
    time_span.rc = false;

    double min[2], max[2];
    min[0] = query_box.MinD(1);
    min[1] = query_box.MinD(2);
    max[0] = query_box.MaxD(1);
    max[1] = query_box.MaxD(2);
    Rectangle<2> spatial_box(true, min, max);
    Region query_reg(spatial_box);

  /////////////////////////////////////////////////////////
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));
       bitset<TM_SUM_NO> node_bit(node->GetTMValue());
//       cout<<nodeid<<" "<<node_bit.to_string()<<endl;
       bool node_mode = false;
       for(unsigned int i = 0;i < node_bit.size();i++){
//          if(node_bit.test(i))cout<<"i "<<i<<endl;
          if(node_bit.test(i) && m_bit_list[i]){
            node_mode = true;
            break;
          }
       }

//       cout<<GetModeStringExt(node->GetTMValue())<<" "<<node_mode<<endl;

       if(node_mode){
          if(node->IsLeaf()){
              for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];

                if(e.box.Intersects(query_box)){ //query window
                    unit_tid_list.push_back(e.info);
                }
              }
          }else{
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }

          node_count++;
       }
        delete node;
    }

    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}

/*
filter step for a sequence of modes, ADRTree

*/
void QueryTM::SeqMode_Filter2(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box,
                              Relation* units_rel, vector<bool> m_bit_list)
{

//     for(unsigned int i = 0;i < m_bit_list.size();i++){
//       cout<<" i "<<i<<" "<<m_bit_list[i]<<endl;
//       if(m_bit_list[i])cout<<GetTMStrExt(i)<<endl;
//     }
  /////////////////////////////////////////////////////////
    Interval<Instant> time_span;
    Instant st(instanttype); 
    Instant et(instanttype);
    st.ReadFrom(query_box.MinD(0));
    time_span.start = st; 
    time_span.lc = true;
    et.ReadFrom(query_box.MaxD(0));
    time_span.end = et;
    time_span.rc = false;

    double min[2], max[2];
    min[0] = query_box.MinD(1);
    min[1] = query_box.MinD(2);
    max[0] = query_box.MaxD(1);
    max[1] = query_box.MaxD(2);
    Rectangle<2> spatial_box(true, min, max);
    Region query_reg(spatial_box);

  /////////////////////////////////////////////////////////
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));
       bitset<TM_SUM_NO> node_bit(node->GetTMValue());
//       cout<<nodeid<<" "<<node_bit.to_string()<<endl;
       bool node_mode = false;
       for(unsigned int i = 0;i < node_bit.size();i++){
//          if(node_bit.test(i))cout<<"i "<<i<<endl;
          if(node_bit.test(i) && m_bit_list[i]){
            node_mode = true;
            break;
          }
       }

//       cout<<GetModeStringExt(node->GetTMValue())<<" "<<node_mode<<endl;

       if(node_mode){
          if(node->IsLeaf()){
              for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];
                Tuple* mtuple = units_rel->GetTuple(e.info, false);

                if(e.box.Intersects(query_box)){ //query window
                   int mode = 
                   ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
                    if(m_bit_list[mode])
                      unit_tid_list.push_back(e.info);
                }

                mtuple->DeleteIfAllowed();
              }
          }else{
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }

          node_count++;
       }

      delete node;
    }

    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}

/*
filter step: a sequence of modes 3DRtree

*/
void QueryTM::SeqMode_Filter3(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box,
                              Relation* units_rel, vector<bool> m_bit_list)
{

//     for(unsigned int i = 0;i < m_bit_list.size();i++){
//       cout<<" i "<<i<<" "<<m_bit_list[i]<<endl;
//       if(m_bit_list[i])cout<<GetTMStrExt(i)<<endl;
//     }
  /////////////////////////////////////////////////////////
    Interval<Instant> time_span;
    Instant st(instanttype); 
    Instant et(instanttype);
    st.ReadFrom(query_box.MinD(0));
    time_span.start = st; 
    time_span.lc = true;
    et.ReadFrom(query_box.MaxD(0));
    time_span.end = et;
    time_span.rc = false;

    double min[2], max[2];
    min[0] = query_box.MinD(1);
    min[1] = query_box.MinD(2);
    max[0] = query_box.MaxD(1);
    max[1] = query_box.MaxD(2);
    Rectangle<2> spatial_box(true, min, max);
    Region query_reg(spatial_box);

  /////////////////////////////////////////////////////////
    queue<SmiRecordId> query_list;
    query_list.push(tmrtree->RootRecordId());
    int node_count = 0;
    while(query_list.empty() == false){
       SmiRecordId nodeid = query_list.front();
       query_list.pop();

       TM_RTreeNode<3, TupleId>* node = tmrtree->GetMyNode(nodeid,false,
                          tmrtree->MinEntries(0), tmrtree->MaxEntries(0));

//       cout<<GetModeStringExt(node->GetTMValue())<<" "<<node_mode<<endl;

          if(node->IsLeaf()){
              for(int j = 0;j < node->EntryCount();j++){
                R_TreeLeafEntry<3, TupleId> e =
                 (R_TreeLeafEntry<3, TupleId>&)(*node)[j];
                Tuple* mtuple = units_rel->GetTuple(e.info, false);

                if(e.box.Intersects(query_box)){ //query window
                   int mode = 
                   ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
                    if(m_bit_list[mode])
                      unit_tid_list.push_back(e.info);
                }

                mtuple->DeleteIfAllowed();
              }
          }else{
            for(int j = 0;j < node->EntryCount();j++){
                R_TreeInternalEntry<3> e =
                  (R_TreeInternalEntry<3>&)(*node)[j];
                if(e.box.Intersects(query_box)){ //query window
                    query_list.push(e.pointer);
                }
            }
          }

          node_count++;
          delete node;
    }

    cout<<node_count<<" nodes accessed "
        <<unit_tid_list.size()<<" candidates "<<endl;

}


/*
a sequence of transportation modes

*/

void QueryTM::SeqMode_Filter4(TM_RTree<3,TupleId>* tmrtree,
                              Rectangle<3> query_box, vector<bool> bit_pos,
                              Relation* units_rel)
{

    MulMode_Filter1(tmrtree, query_box, bit_pos, units_rel);
}

inline double GetStartTimeValue(MPoint* mp, int pos)
{
  assert(0 <= pos && pos < mp->GetNoComponents());
  UPoint u;
  mp->Get(pos, u);
  return u.timeInterval.start.ToDouble();
}

inline double GetEndTimeValue(MPoint* mp, int pos)
{
  assert(0 <= pos && pos < mp->GetNoComponents());
  UPoint u;
  mp->Get(pos, u);
  return u.timeInterval.end.ToDouble();
}

/*
refinement step for a sequence of modes

*/
void QueryTM::SeqMode_Refinement(Rectangle<3> query_box, vector<bool> bit_pos, 
                          Relation* units_rel, vector<long> seq_tm,
                                 vector<bool> m_bit_list)
{
  Interval<Instant> time_span;
  Instant st(instanttype); 
  Instant et(instanttype);
  st.ReadFrom(query_box.MinD(0));
  time_span.start = st; 
  time_span.lc = true;
  et.ReadFrom(query_box.MaxD(0));
  time_span.end = et;
  time_span.rc = false;

  double min[2], max[2];
  min[0] = query_box.MinD(1);
  min[1] = query_box.MinD(2);
  max[0] = query_box.MaxD(1);
  max[1] = query_box.MaxD(2);
  Rectangle<2> spatial_box(true, min, max);
  Region query_reg(spatial_box);

  map<int, Seq_Mode> res_traj;//binary search tree
  int sin_mode_no = ARR_SIZE(str_tm);

  for(unsigned int i = 0;i < unit_tid_list.size();i++){
      Tuple* mtuple = units_rel->GetTuple(unit_tid_list[i], false);
      int traj_id = ((CcInt*)mtuple->GetAttribute(GM_TRAJ_ID))->GetIntval();
      int m = ((CcInt*)mtuple->GetAttribute(GM_MODE))->GetIntval();
      int div_pos = ((CcInt*)mtuple->GetAttribute(GM_DIV))->GetIntval();

//        if(traj_id != 3594){
//            mtuple->DeleteIfAllowed();
//            continue;
//        }

      MPoint* mp = (MPoint*)mtuple->GetAttribute(GM_SUBTRIP);
//      cout<<"mtuple mode "<<GetTMStrExt(m)<<" "<<*mp<<endl;

//      cout<<"traj_id "<<traj_id<<" div_pos "<<div_pos<<endl;
      ///////////////////////////////////////////////////////////
      map<int, Seq_Mode>::iterator iter = res_traj.find(traj_id);
      if(iter == res_traj.end()){//a new mtuple or trajectory

        if(m < sin_mode_no){/////mode value 0-8
            assert(div_pos == -1);
            if(bit_pos[m] == false){//this should be done by filter step
              mtuple->DeleteIfAllowed();
              assert(false);
              continue;
            }
            int start = 0;
            int end = mp->GetNoComponents();
            ////temporal and spatial checking/////
            bool res = CheckMPoint(mp, start, end, time_span, &query_reg);
            if(res == false){
              mtuple->DeleteIfAllowed();
              continue;
            }

            double st = GetStartTimeValue(mp, start);
            double et = GetEndTimeValue(mp, end - 1);
//            cout<<st<<" "<<et<<endl;
            MInt mp_tm(0);
            MakeTMUnit(mp_tm, st, et, m);
//            cout<<mp_tm<<endl;
            Seq_Mode seq_m(mp_tm);
            res_traj.insert(pair<int, Seq_Mode>(traj_id, seq_m));

        }else if(sin_mode_no <= m && m < 2*sin_mode_no){

          if(m_bit_list[m] == false){//this should be done by filter step
             mtuple->DeleteIfAllowed();
             assert(false);
             continue;
          }
          int start = 0;
          int end = div_pos;
          bool res1 = CheckMPoint(mp, start, end, time_span, &query_reg);
          if(res1 == false){
              mtuple->DeleteIfAllowed();
              continue;
          }
          ///////////////////////////////////////////////////////////////////
          //////we split the two modes, store them separately in the tree////
          ///////////////////////////////////////////////////////////////////
          double st1 = GetStartTimeValue(mp, start);
          double et1 = GetEndTimeValue(mp, div_pos - 1);
          MInt mp_tm(0);
          MakeTMUnit(mp_tm, st1, et1, m - sin_mode_no);

          Seq_Mode seq_m(mp_tm);
          res_traj.insert(pair<int, Seq_Mode>(traj_id, seq_m));

          /////////////////////second part//////////////////////////
          start = div_pos;
          end = mp->GetNoComponents();
          bool res2 = CheckMPoint(mp, start, end, time_span, &query_reg);
          if(res2 == false){
              mtuple->DeleteIfAllowed();
              continue;
          }

          iter = res_traj.find(traj_id);

          double st2 = GetStartTimeValue(mp, div_pos);
          double et2 = GetEndTimeValue(mp,  mp->GetNoComponents() - 1);
          iter->second.Update(st2, et2, TM_WALK);


        }else if(2*sin_mode_no <= m && m < 3*sin_mode_no){

          if(m_bit_list[m] == false){//this should be done by filter step
             mtuple->DeleteIfAllowed();
             assert(false);
             continue;
          }
          int start = 0;
          int end = div_pos;
          bool res1 = CheckMPoint(mp, start, end, time_span, &query_reg);
          if(res1 == false){
              mtuple->DeleteIfAllowed();
              continue;
          }
          ///////////////////////////////////////////////////////////////////
          //////we split the two modes, store them separately in the tree////
          ///////////////////////////////////////////////////////////////////
          double st1 = GetStartTimeValue(mp, start);
          double et1 = GetEndTimeValue(mp, div_pos - 1);
          MInt mp_tm(0);
          MakeTMUnit(mp_tm, st1, et1, TM_WALK);

          Seq_Mode seq_m(mp_tm);
          res_traj.insert(pair<int, Seq_Mode>(traj_id, seq_m));
          //////////////////second part//////////////////////////////////
          start = div_pos;
          end = mp->GetNoComponents();
          bool res2 = CheckMPoint(mp, start, end, time_span, &query_reg);
          if(res2 == false){
              mtuple->DeleteIfAllowed();
              continue;
          }

          iter = res_traj.find(traj_id);

          double st2 = GetStartTimeValue(mp, div_pos);
          double et2 = GetEndTimeValue(mp,  mp->GetNoComponents() - 1);
          iter->second.Update(st2, et2, m - 2*sin_mode_no);

        }else{
          assert(false);
        }

      }else{
        if(iter->second.Status()){//as a result already
          ///doing nothing
        }else{/////need to update the value

          if(m < sin_mode_no){
//             cout<<"update the mode"<<endl;
            if(bit_pos[m] == false){//this should be done by filter step
              mtuple->DeleteIfAllowed();
              assert(false);
              continue;
            }
            int start = 0;
            int end = mp->GetNoComponents();
            ////temporal and spatial checking/////
            bool res = CheckMPoint(mp, start, end, time_span, &query_reg);
            if(res == false){
              mtuple->DeleteIfAllowed();
              continue;
            }
            double st = GetStartTimeValue(mp, start);
            double et = GetEndTimeValue(mp, end - 1);
            iter->second.Update(st, et, m);
            iter->second.CheckStatus(seq_tm);


          }else if(sin_mode_no <= m && m < 2*sin_mode_no){
            if(m_bit_list[m] == false){//this should be done by filter step
             mtuple->DeleteIfAllowed();
             assert(false);
             continue;
            }
            int start = 0;
            int end = div_pos;
            ////temporal and spatial checking/////
            bool res1 = CheckMPoint(mp, start, end, time_span, &query_reg);
            if(res1 == false){
              mtuple->DeleteIfAllowed();
              continue;
            }
            ///////////////////////////////////////////////////////////////////
            //////we split the two modes, store them separately in the tree////
            ///////////////////////////////////////////////////////////////////

            double st1 = GetStartTimeValue(mp, start);
            double et1 = GetEndTimeValue(mp, div_pos - 1);
            iter->second.Update(st1, et1, m - sin_mode_no);

            /////////////////////second part///////////////////
            start = div_pos;
            end = mp->GetNoComponents();
            ////temporal and spatial checking/////
            bool res2 = CheckMPoint(mp, start, end, time_span, &query_reg);
            if(res2 == false){
              mtuple->DeleteIfAllowed();
              iter->second.CheckStatus(seq_tm);
              continue;
            }

            double st2 = GetStartTimeValue(mp, div_pos);
            double et2 = GetEndTimeValue(mp, end - 1);
            iter->second.Update(st2, et2, TM_WALK);

            iter->second.CheckStatus(seq_tm);


          }else if(2*sin_mode_no <= m && m < 3*sin_mode_no){
            if(m_bit_list[m] == false){//this should be done by filter step
              mtuple->DeleteIfAllowed();
              assert(false);
              continue;
            }
            ///////////////////////////////////////////////////////////////////
            //////we split the two modes, store them separately in the tree////
            ///////////////////////////////////////////////////////////////////
            int start = 0;
            int end = div_pos;
            ////temporal and spatial checking/////
            bool res1 = CheckMPoint(mp, start, end, time_span, &query_reg);
            if(res1 == false){
              mtuple->DeleteIfAllowed();
              continue;
            }
            double st1 = GetStartTimeValue(mp, start);
            double et1 = GetEndTimeValue(mp, div_pos - 1);
            iter->second.Update(st1, et1, TM_WALK);

            //////////////////second part///////////////////////////
            start = div_pos;
            end = mp->GetNoComponents();
            bool res2 = CheckMPoint(mp, start, end, time_span, &query_reg);
            if(res2 == false){
              mtuple->DeleteIfAllowed();
              iter->second.CheckStatus(seq_tm);
              continue;
            }

            double st2 = GetStartTimeValue(mp, div_pos);
            double et2 = GetEndTimeValue(mp, end - 1);
            iter->second.Update(st2, et2, m - 2*sin_mode_no);
            iter->second.CheckStatus(seq_tm);

          }else{
            assert(false);
          }

        }

      }

      mtuple->DeleteIfAllowed();
  }

  /////////////////////output the result////////////////////
  map<int, Seq_Mode>::iterator iter;
//  cout<<"res count "<<res_traj.size()<<endl;
  for(iter = res_traj.begin(); iter != res_traj.end();iter++){
//      iter->second.Print();
       if(iter->second.Status()){
         oid_list.push_back(iter->first);
       }
  }

}

/*
update the moving integer: simply insert
1: simply insert and check the status later
2: insert, merge and the procedure of checking might be easy

*/
void Seq_Mode::Update(double st, double et, int m)
{
    //////////simply insert, there might be gaps////////////////
    UInt u;
    Instant start(instanttype);
    Instant end(instanttype);
    start.ReadFrom(st);
    end.ReadFrom(et);
    u.timeInterval.start = start;
    u.timeInterval.end = end;
    u.timeInterval.lc = true;
    u.timeInterval.rc = false;
    u.constValue.Set(m);
    u.SetDefined(true);
    seq_tm.Add(u);

//    seq_tm.SortbyUnitTime();
}

/*
check whether the sequence of mode is satisfied
the current mode is set found only when its previous has been found

*/
void Seq_Mode::CheckStatus(vector<long> m_list)
{
//  cout<<"check status "<<endl;
 if(seq_tm.GetNoComponents() < (int)m_list.size()) return;//impossible be true

//   vector<bool> flag(m_list.size(), false);
//   for(int i = 0;i < seq_tm.GetNoComponents();i++){
//     UInt u;
//     seq_tm.Get(i, u);
//     int index = tm_index[u.constValue.GetValue()];
//     assert(index >= 0 && index < (int)m_list.size());
//     if(index == 0)flag[index] = true;
//     else{
//       int prev = index - 1;
//       if(flag[prev]){//only when its previous has been found
//         flag[index] = true;
//         if(index == (int)m_list.size() - 1){//find the result
//           status = true;
//           break;
//         }
//       }
//     }
//   }
  seq_tm.SortbyUnitTime();
  ///////////this checking is or will be consistent with the mtuples////
  ////where the filter step only considers mtules with a pair of modes/////
  vector<int> tmp_res;
  for(int i = 0;i < seq_tm.GetNoComponents();i++){
    UInt u;
    seq_tm.Get(i, u);
    int m = u.constValue.GetValue();
    if(tmp_res.size() == 0){
      if(m == m_list[0]){//put the start value
        tmp_res.push_back(m);
      }
    }else{
        if(m == m_list[tmp_res.size()]){
          tmp_res.push_back(m);
          if(tmp_res.size() == m_list.size()){
            status = true;
            break;
          }
        }
    }
  }

}
/*
create a new moving integer, only one unit

*/
void QueryTM::MakeTMUnit(MInt& mp_tm, double st, double et, int m)
{
    UInt u;
    Instant start(instanttype);
    Instant end(instanttype);
    start.ReadFrom(st);
    end.ReadFrom(et);
    u.timeInterval.start = start;
    u.timeInterval.end = end;
    u.timeInterval.lc = true;
    u.timeInterval.rc = false;
    u.constValue.Set(m);
    u.SetDefined(true);
    mp_tm.Add(u); //simply insert

}