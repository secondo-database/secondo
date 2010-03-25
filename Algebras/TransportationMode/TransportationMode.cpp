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

August, 2009 Jianqiu Xu

March, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
queries moving objects with transportation modes.

2 Defines and includes

*/

#include "TransportationMode.h"


extern NestedList* nl;
extern QueryProcessor *qp;

namespace TransportationMode{

/*
struct for partition space

*/

struct LinePartiton{
  Relation* l;
  TupleType* resulttype;
  unsigned int count;
  vector<int> junid1;
  vector<int> junid2;
  vector<Region> outer_regions_s;
  vector<Region> outer_regions1;
  vector<Region> outer_regions2;
  vector<Region> outer_regions_l;
  vector<Region> outer_regions4;
  vector<Region> outer_regions5;
  LinePartiton()
  {
      l = NULL;
      resulttype = NULL;
      count = 0;
  }
  LinePartiton(Relation* in_line):l(in_line),count(0){}

  void GetDeviation(Point center, double a, double b,double& x1,
                   double& x2,int delta)
  {
    double x0 = center.GetX();
    double y0 = center.GetY();
    double A = 1 + a*a;
    double B = 2*a*(b-y0)-2*x0;
//    double C = x0*x0 + (b-y0)*(b-y0) - 1;
    double C = x0*x0 + (b-y0)*(b-y0) - delta*delta;
    x1 = (-B - sqrt(B*B-4*A*C))/(2*A);
    x2 = (-B + sqrt(B*B-4*A*C))/(2*A);
  }

  bool GetClockwise(Point& p0,Point& p1, Point& p2)//from p0-p1 to p0-p2
  {
      double x0 = p0.GetX();
      double y0 = p0.GetY();
      double x1 = p1.GetX();
      double y1 = p1.GetY();
      double x2 = p2.GetX();
      double y2 = p2.GetY();
      bool result;
      if(AlmostEqual(x0,x1)){
        if(y1 >= y0){
//          if(x2 < x0) result = false;
          if(x2 < x0 || AlmostEqual(x2,x0)) result = false;
          else result = true;
        }else{
          if(x2 < x0) result = true;
          else result = false;
        }
      }else{
          double slope = (y1-y0)/(x1-x0);

/*          if(AlmostEqual(y1,y0))
            slope = 0;
          else
            slope = (y1-y0)/(x1-x0);*/

          double intercept = y1-slope*x1;
          if(x1 < x0){
            if(y2 < (slope*x2 + intercept)) result = false;
            else result = true;
          }else{
            if(y2 < (slope*x2 + intercept)) result = true;
            else result = false;
          }
      }

//      if(result) cout<<"clockwise "<<endl;
//     else cout<<"counterclokwise "<<endl;
      return result;
  }

    double GetAngle(Point& p0,Point& p1, Point& p2)
    {
      /////cosne theorem ///
      double angle; //radian [0-pi]
      double b = p0.Distance(p1);
      double c = p0.Distance(p2);
      double a = p1.Distance(p2);
      assert(AlmostEqual(b*c,0.0) == false);
      double value = (b*b+c*c-a*a)/(2*b*c);

      if(AlmostEqual(value,-1.0)) value = -1;
      if(AlmostEqual(value,1.0)) value = 1;
      angle = acos(value);
//      cout<<"angle "<<angle<<" degree "<<angle*180.0/pi<<endl;
      assert(0.0 <= angle && angle <= 3.1416);
      return angle;
    }

  void ModifySegment(MyHalfSegment& mhs,vector<MyHalfSegment>& boundary,
                     int delta,bool clock_flag)
  {

    Point from = mhs.GetLeftPoint();
    Point to = mhs.GetRightPoint();
    Point next_from1;
    Point next_to1;

    Point p1,p2,p3,p4;

    if(AlmostEqual(from.GetX(),to.GetX())){
 /*     next_from1.Set(from.GetX() + delta, from.GetY());
      next_to1.Set(to.GetX() + delta, to.GetY());
      MyHalfSegment* seg = new MyHalfSegment(true,next_from1,next_to1);
      boundary.push_back(*seg);
      delete seg;
      return;*/
      p1.Set(from.GetX() - delta,from.GetY());
      p2.Set(from.GetX() + delta,from.GetY());
      p3.Set(to.GetX() - delta, to.GetY());
      p4.Set(to.GetX() + delta, to.GetY());
    }
    else if(AlmostEqual(from.GetY(), to.GetY())){
/*      next_from1.Set(from.GetX(),from.GetY() - delta);
      next_to1.Set(to.GetX(), to.GetY() - delta);
      MyHalfSegment* seg = new MyHalfSegment(true,next_from1,next_to1);
      boundary.push_back(*seg);
      delete seg;
      return;*/
      p1.Set(from.GetX(), from.GetY() - delta);
      p2.Set(from.GetX(), from.GetY() + delta);
      p3.Set(to.GetX(), to.GetY() - delta);
      p4.Set(to.GetX(), to.GetY() + delta);
    }else{

      double k1 = (from.GetY() - to.GetY())/(from.GetX() - to.GetX());

      double k2 = -1/k1;
      double b1 = from.GetY() - k2*from.GetX();

      double x1,x2;
      GetDeviation(from,k2,b1,x1,x2,delta);

      double y1 = x1*k2 + b1;
      double y2 = x2*k2 + b1;

      double x3,x4;
      double b2 = to.GetY() - k2*to.GetX();
      GetDeviation(to,k2,b2,x3,x4,delta);

      double y3 = x3*k2 + b2;
      double y4 = x4*k2 + b2;

      p1.Set(x1,y1);
      p2.Set(x2,y2);
      p3.Set(x3,y3);
      p4.Set(x4,y4);
    }

    vector<Point> clock_wise;
    vector<Point> counterclock_wise;
    if(GetClockwise(from,to,p1)) clock_wise.push_back(p1);
    else counterclock_wise.push_back(p1);

    if(GetClockwise(from,to,p2)) clock_wise.push_back(p2);
    else counterclock_wise.push_back(p2);

    if(GetClockwise(from,to,p3)) clock_wise.push_back(p3);
      else counterclock_wise.push_back(p3);

    if(GetClockwise(from,to,p4)) clock_wise.push_back(p4);
      else counterclock_wise.push_back(p4);

    assert(clock_wise.size() == 2 && counterclock_wise.size() == 2);
    if(clock_flag){
      next_from1 = clock_wise[0];
      next_to1 = clock_wise[1];
    }else{
      next_from1 = counterclock_wise[0];
      next_to1 = counterclock_wise[1];
    }

    MyHalfSegment* seg = new MyHalfSegment(true,next_from1,next_to1);
    boundary.push_back(*seg);
    delete seg;

  }
  inline void ModifyPoint(Point& p)
  {
//    cout<<"ModifyPoint"<<endl;
    double a, b;
    int x, y;
    a = p.GetX();
    b = p.GetY();

    x = static_cast<int>(GetCloser(a));
    y = static_cast<int>(GetCloser(b));
    p.Set(x,y);

//    printf("%.8f %.8f\n",a,b);
//    printf("%d %d\n",x,y);
  }
  inline void Modify_Point(Point& p)
  {

//    printf("%.10f %.10f\n",a, b);
//    x = ((int)(a*1000.0 +0.5))/1000.0;
//    y = ((int)(b*1000.0 +0.5))/1000.0;
//    printf("%.10f %.10f\n",x, y);


  }

  void AddHalfSegmentResult(MyHalfSegment hs, Line* res, int& edgeno)
  {
      const double delta_dist = 0.1;
      double a1,b1,a2,b2;
      int x1,y1,x2,y2;
      a1 = hs.GetLeftPoint().GetX();
      b1 = hs.GetLeftPoint().GetY();
      a2 = hs.GetRightPoint().GetX();
      b2 = hs.GetRightPoint().GetY();

      x1 = static_cast<int>(GetCloser(a1));
      y1 = static_cast<int>(GetCloser(b1));
      x2 = static_cast<int>(GetCloser(a2));
      y2 = static_cast<int>(GetCloser(b2));

      HalfSegment hs2;
      Point p1,p2;
      p1.Set(x1,y1);
      p2.Set(x2,y2);

      if(p1.Distance(p2) > delta_dist){//////////////final check
        hs2.Set(true,p1,p2);
        hs2.attr.edgeno = edgeno++;
        *res += hs2;
        hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
        *res += hs2;
      }
  }
  //////outer does not include points from road, but outer_half includes////
  void ExtendSeg1(vector<MyHalfSegment>& segs,int delta, bool clock_wise,
                vector<Point>& outer,bool& add, vector<Point>& outer_half)
  {
    const double delta_dist = 0.1;
//    cout<<"ExtendSeg() "<<endl;
    for(unsigned int i = 0;i < segs.size();i++){
 //     cout<<"start "<<segs[i].GetLeftPoint()<<" to "
 //         <<segs[i].GetRightPoint()<<endl;
      if(i < segs.size() - 1){
        Point to1 = segs[i].GetRightPoint();
        Point from2 = segs[i+1].GetLeftPoint();
        assert(to1.Distance(from2) < delta_dist);
      }
    }


    vector<MyHalfSegment> boundary;

    for(unsigned int i = 0;i < segs.size();i++){
      ModifySegment(segs[i],boundary,delta,clock_wise);
//      break;
    }
//    cout<<"after modify segment "<<endl;
    ////////  connect the new boundary ////////////////////////
    for(unsigned int i = 0;i < boundary.size() - 1; i++){
      Point p1_1 = boundary[i].GetLeftPoint();
      Point p1_2 = boundary[i].GetRightPoint();
      Point p2_1 = boundary[i+1].GetLeftPoint();
      Point p2_2 = boundary[i+1].GetRightPoint();
//      cout<<p1_2<<p2_1<<endl;
      if(p1_2.Distance(p2_1) < delta_dist) continue;


      if(AlmostEqual(p1_1.GetX(),p1_2.GetX())){
        assert(!AlmostEqual(p2_1.GetX(),p2_2.GetX()));
        double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
        double b2 = p2_2.GetY() - a2*p2_2.GetX();

        double x = p1_1.GetX();
        double y = a2*x + b2;
        boundary[i].to.Set(x,y);
        boundary[i+1].from.Set(x,y);
      }else{
        if(AlmostEqual(p2_1.GetX(),p2_2.GetX())){

          assert(!AlmostEqual(p1_1.GetX(),p1_2.GetX()));
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double x = p2_1.GetX();
          double y = a1*x + b1;
          boundary[i].to.Set(x,y);
          boundary[i+1].from.Set(x,y);

        }else{
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
          double b2 = p2_2.GetY() - a2*p2_2.GetX();

//          assert(!AlmostEqual(a1,a2));
          if(AlmostEqual(a1,a2)) assert(AlmostEqual(b1,b2));

          double x = (b2-b1)/(a1-a2);
          double y = a1*x + b1;
          ////////////process speical case///////angle too small////////////
          Point q1;
          q1.Set(x,y);
          Point q2 = segs[i].GetRightPoint();
          if(q1.Distance(q2) > 5*delta){
//              assert(false);
            cout<<"special case"<<endl;
            add = false;
          }
          /////////////////////////////////////////////////////
          boundary[i].to.Set(x,y);
          boundary[i+1].from.Set(x,y);
        }
//        cout<<boundary[i].to<<" "<<boundary[i+1].from<<endl;

      }// end if

    }//end for
    ///////////////////////add two more segments ///////////////////////
      Point old_start = segs[0].GetLeftPoint();
      Point old_end = segs[segs.size() - 1].GetRightPoint();

      Point new_start = boundary[0].GetLeftPoint();
      Point new_end = boundary[boundary.size() - 1].GetRightPoint();


      MyHalfSegment* mhs = new MyHalfSegment(true,old_start,new_start);
      boundary.push_back(*mhs);
      for(int i = boundary.size() - 2; i >= 0;i --)
          boundary[i+1] = boundary[i];

      boundary[0] = *mhs;
      delete mhs;


      mhs = new MyHalfSegment(true,new_end,old_end);
      boundary.push_back(*mhs);
      delete mhs;
     /////////////for debuging see the result////////////////////////////////
/*      for(unsigned int i = 0;i < boundary.size();i++){
        Line* l = new Line(0);
        l->StartBulkLoad();
        int edgeno = 0;

        Point p1 = boundary[i].GetLeftPoint();
        Point p2 = boundary[i].GetRightPoint();
        HalfSegment* seg = new HalfSegment(true,p1,p2);

        seg->attr.edgeno = edgeno++;
        *l += *seg;
        seg->SetLeftDomPoint(!seg->IsLeftDomPoint());
        *l += *seg;

        l->EndBulkLoad();
        res_line.push_back(*l);
        delete l;
      }*/
      ///////////////////////////////////////////////////////////////////////
      if(add == false) return;

      if(clock_wise){
        for(unsigned int i = 0;i < boundary.size();i++){
          ///////////////outer segments////////////////////////////////
          Point p = boundary[i].GetLeftPoint();

          ModifyPoint(p);

          /////////////////////////////////////////////////////////
          if(i == 0){
              outer.push_back(boundary[i].GetLeftPoint());
              outer_half.push_back(boundary[i].GetLeftPoint());
          }
          else{
            outer.push_back(p);
            outer_half.push_back(p);
          }
        }

        for(int i = segs.size() - 1; i >= 0; i--)
          outer_half.push_back(segs[i].GetRightPoint());

      }else{
        for(unsigned int i = 0; i < segs.size(); i++)
          outer_half.push_back(segs[i].GetLeftPoint());

        for(int i = boundary.size() - 1;i >= 0;i--){
          /////////////////////////////////////////////////////////////
          Point p = boundary[i].GetRightPoint();
          ModifyPoint(p);
         /////////////////////////////////////////////////////////////
          if((unsigned)i == boundary.size() - 1){
              outer.push_back(boundary[i].GetRightPoint());
              outer_half.push_back(boundary[i].GetRightPoint());
          }else{
              outer.push_back(p);
              outer_half.push_back(p);
          }
        }

      }

  }
  void ExtendSeg2(vector<MyHalfSegment>& segs,int delta, bool clock_wise,
                vector<Point>& outer,bool& add)
  {
    const double delta_dist = 0.1;

    for(unsigned int i = 0;i < segs.size();i++){
 //     cout<<"start "<<segs[i].GetLeftPoint()<<" to "
 //         <<segs[i].GetRightPoint()<<endl;
      if(i < segs.size() - 1){
        Point to1 = segs[i].GetRightPoint();
        Point from2 = segs[i+1].GetLeftPoint();
        assert(to1.Distance(from2) < delta_dist);
      }
    }


    vector<MyHalfSegment> boundary;

    for(unsigned int i = 0;i < segs.size();i++){
      ModifySegment(segs[i],boundary,delta,clock_wise);
//      break;
    }
//    cout<<"after modify segment "<<endl;
    ////////  connect the new boundary ////////////////////////
    for(unsigned int i = 0;i < boundary.size() - 1; i++){
      Point p1_1 = boundary[i].GetLeftPoint();
      Point p1_2 = boundary[i].GetRightPoint();
      Point p2_1 = boundary[i+1].GetLeftPoint();
      Point p2_2 = boundary[i+1].GetRightPoint();
//      cout<<p1_2<<p2_1<<endl;
      if(p1_2.Distance(p2_1) < delta_dist) continue;


      if(AlmostEqual(p1_1.GetX(),p1_2.GetX())){
        assert(!AlmostEqual(p2_1.GetX(),p2_2.GetX()));
        double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
        double b2 = p2_2.GetY() - a2*p2_2.GetX();

        double x = p1_1.GetX();
        double y = a2*x + b2;
        boundary[i].to.Set(x,y);
        boundary[i+1].from.Set(x,y);
      }else{
        if(AlmostEqual(p2_1.GetX(),p2_2.GetX())){

          assert(!AlmostEqual(p1_1.GetX(),p1_2.GetX()));
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double x = p2_1.GetX();
          double y = a1*x + b1;
          boundary[i].to.Set(x,y);
          boundary[i+1].from.Set(x,y);

        }else{
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
          double b2 = p2_2.GetY() - a2*p2_2.GetX();

//          assert(!AlmostEqual(a1,a2));
          if(AlmostEqual(a1,a2)) assert(AlmostEqual(b1,b2));

          double x = (b2-b1)/(a1-a2);
          double y = a1*x + b1;
          ////////////process speical case///////angle too small////////////
          Point q1;
          q1.Set(x,y);
          Point q2 = segs[i].GetRightPoint();
          if(q1.Distance(q2) > 5*delta){
//              assert(false);
            cout<<"special case"<<endl;
            add = false;
          }
          /////////////////////////////////////////////////////
          boundary[i].to.Set(x,y);
          boundary[i+1].from.Set(x,y);
        }
//        cout<<boundary[i].to<<" "<<boundary[i+1].from<<endl;

      }// end if

    }//end for
    ///////////////////////add two more segments ///////////////////////
      Point old_start = segs[0].GetLeftPoint();
      Point old_end = segs[segs.size() - 1].GetRightPoint();

      Point new_start = boundary[0].GetLeftPoint();
      Point new_end = boundary[boundary.size() - 1].GetRightPoint();


      MyHalfSegment* mhs = new MyHalfSegment(true,old_start,new_start);
      boundary.push_back(*mhs);
      for(int i = boundary.size() - 2; i >= 0;i --)
          boundary[i+1] = boundary[i];

      boundary[0] = *mhs;
      delete mhs;


      mhs = new MyHalfSegment(true,new_end,old_end);
      boundary.push_back(*mhs);
      delete mhs;

      if(add == false) return;

      if(clock_wise){
        for(unsigned int i = 0;i < boundary.size();i++){
          ///////////////outer segments////////////////////////////////
          Point p = boundary[i].GetLeftPoint();

          ModifyPoint(p);

          /////////////////////////////////////////////////////////
          if(i == 0){
              outer.push_back(boundary[i].GetLeftPoint());
          }
          else{
            outer.push_back(p);

          }
        }


      }else{

        for(int i = boundary.size() - 1;i >= 0;i--){
          /////////////////////////////////////////////////////////////
          Point p = boundary[i].GetRightPoint();
          ModifyPoint(p);
         /////////////////////////////////////////////////////////////
          if((unsigned)i == boundary.size() - 1){
              outer.push_back(boundary[i].GetRightPoint());

          }else{
              outer.push_back(p);
          }
        }

      }

  }
  /////////from start point to end point ////////////////////////////
  void ReorderLine(SimpleLine* sline, vector<MyHalfSegment>& seq_halfseg)
  {
    Point sp;
    assert(sline->AtPosition(0.0,true,sp));
    vector<MyHalfSegment> copyline;
    for(int i = 0;i < sline->Size();i++){
      HalfSegment hs;
      sline->Get(i,hs);
      if(hs.IsLeftDomPoint()){
        Point lp = hs.GetLeftPoint();
        Point rp = hs.GetRightPoint();
        MyHalfSegment* mhs = new MyHalfSegment(true,lp,rp);
        copyline.push_back(*mhs);
        delete mhs;
      }
    }
    ////////////////reorder /////////////////////////////////////
/*    cout<<"before"<<endl;
    for(unsigned i = 0;i < copyline.size();i++)
      copyline[i].Print();*/

    const double delta_dist = 0.1;
    unsigned int count = 0;
    int index = 0;
    while(count < copyline.size()){
      if(copyline[index].def == false){
        index = (index + 1) % copyline.size();
        continue;
      }
      Point from = copyline[index].GetLeftPoint();
      Point to = copyline[index].GetRightPoint();

      if(from.Distance(sp) > delta_dist && to.Distance(sp) > delta_dist){
        index = (index + 1) % copyline.size();
        continue;
      }
      if(from.Distance(sp) < delta_dist){
        sp = to;
        count++;
        copyline[index].def = false;
        index = (index + 1) % copyline.size();
        MyHalfSegment* mhs = new MyHalfSegment(true,from,to);
        seq_halfseg.push_back(*mhs);
        delete mhs;
        continue;
      }
      if(to.Distance(sp) < delta_dist){
        sp = from;
        count++;
        copyline[index].def = false;
        index = (index + 1) % copyline.size();
        MyHalfSegment* mhs = new MyHalfSegment(true,to,from);
        seq_halfseg.push_back(*mhs);
        delete mhs;
        continue;
      }
    }
    ///////////////////////////////////////////////////////////////////////
/*    cout<<"after"<<endl;
    for(unsigned i = 0;i < copyline.size();i++)
      copyline[i].Print();*/

  }
  /////////////compute a region from the array of points/////////
  void ComputeRegion(vector<Point>& outer_region,vector<Region>& regs)
  {
    /////////note that points are counter_clock_wise ordered///////////////
//    for(unsigned i = 0;i < outer_region.size();i++)
//      cout<<outer_region[i];


      Region* cr = new Region( 0 );
      cr->StartBulkLoad();

      int fcno=-1;
      int ccno=-1;
      int edno= 0;
      int partnerno = 0;

      fcno++;
      ccno++;
      bool isCycle = true;

      Point firstPoint = outer_region[0];
      Point prevPoint = firstPoint;

      //Starting to compute a new cycle

      Points *cyclepoints= new Points( 8 ); // in memory
      Point currvertex, p1, p2, firstP;
      Region *rDir = new Region(32);
      rDir->StartBulkLoad();
      currvertex = firstPoint;


      cyclepoints->StartBulkLoad();
      *cyclepoints += currvertex;
      p1 = currvertex;
      firstP = p1;
      cyclepoints->EndBulkLoad();

      for(unsigned int i = 1;i < outer_region.size();i++){
        currvertex = outer_region[i];

//        if(cyclepoints->Contains(currvertex))assert(false);
        if(cyclepoints->Contains(currvertex))continue;

        ////////////////step -- 1/////////////////////////////
        p2 = currvertex;
        cyclepoints->StartBulkLoad();
        *cyclepoints += currvertex;
        cyclepoints->EndBulkLoad(true,false,false);
        /////////////step --- 2 create halfsegment/////////////////////////

        HalfSegment* hs = new HalfSegment(true,prevPoint, currvertex);
        hs->attr.faceno=fcno;
        hs->attr.cycleno=ccno;
        hs->attr.edgeno=edno;
        hs->attr.partnerno=partnerno;
        partnerno++;
        hs->attr.insideAbove = (hs->GetLeftPoint() == p1);
        ////////////////////////////////////////////////////////
        p1 = p2;
        edno++;
        prevPoint= currvertex;

        if(cr->InsertOk(*hs)){
           *cr += *hs;
//           cout<<"cr+1 "<<*hs<<endl;
           if( hs->IsLeftDomPoint()){
              *rDir += *hs;
//              cout<<"rDr+1 "<<*hs<<endl;
              hs->SetLeftDomPoint( false );
           }else{
                hs->SetLeftDomPoint( true );
//                cout<<"rDr+2 "<<*hs<<endl;
                (*rDir) += (*hs);
                }
            (*cr) += (*hs);
//            cout<<"cr+2 "<<*hs<<endl;
            delete hs;
        }else assert(false);
      }//end for

      delete cyclepoints;
      ////////////////////last segment//////////////////////////
      edno++;
      HalfSegment* hs = new HalfSegment(true, firstPoint,currvertex);
      hs->attr.faceno=fcno;
      hs->attr.cycleno=ccno;
      hs->attr.edgeno=edno;
      hs->attr.partnerno=partnerno;
      hs->attr.insideAbove = (hs->GetRightPoint() == firstP);
      partnerno++;

      //////////////////////////////////////////////////////////
      if (cr->InsertOk(*hs)){
//          cout<<"insert last segment"<<endl;
          *cr += *hs;
//          cout<<"cr+3 "<<*hs<<endl;
          if(hs->IsLeftDomPoint()){
             *rDir += *hs;
//            cout<<"rDr+3 "<<*hs<<endl;
            hs->SetLeftDomPoint( false );
          }else{
              hs->SetLeftDomPoint( true );
//              cout<<"rDr+4 "<<*hs<<endl;
              *rDir += *hs;
            }
          *cr += *hs;
//          cout<<"cr+4 "<<*hs<<endl;
          delete hs;
          rDir->EndBulkLoad(true, false, false, false);


          //To calculate the inside above attribute
//          bool direction = rDir->GetCycleDirection();
          ////explicitly define it for all regions, false -- area > 0////
          bool direction = false;//counter_wise
//          cout<<"direction "<<direction<<endl;
          int h = cr->Size() - ( rDir->Size() * 2 );
          while(h < cr->Size()){
            //after each left half segment of the region is its
            //correspondig right half segment
            HalfSegment hsIA;
            bool insideAbove;
            cr->Get(h,hsIA);

            if (direction == hsIA.attr.insideAbove)
               insideAbove = false;
            else
               insideAbove = true;
            if (!isCycle)
                insideAbove = !insideAbove;
            HalfSegment auxhsIA( hsIA );
            auxhsIA.attr.insideAbove = insideAbove;
            cr->UpdateAttr(h,auxhsIA.attr);
            //Get right half segment
            cr->Get(h+1,hsIA);
            auxhsIA = hsIA;
            auxhsIA.attr.insideAbove = insideAbove;
            cr->UpdateAttr(h+1,auxhsIA.attr);
            h+=2;
          }
          delete rDir;
        }else assert(false);

      cr->SetNoComponents( fcno+1 );
      cr->EndBulkLoad( true, true, true, false );

      //////////////////////////////////////////////////
/*      for(unsigned int i = 0;i < cr->Size();i++){
          HalfSegment hs;
          cr->Get(i,hs);
          if(hs.IsLeftDomPoint()){
            Point lp = hs.GetLeftPoint();
            Point rp = hs.GetRightPoint();
            printf("%.10f %.10f %.10f %.10f\n",lp.GetX(),lp.GetY(),
                                               rp.GetX(),rp.GetY());
          }
      }*/
      /////////////////////////////////////////////
      regs.push_back(*cr);
      delete cr;

  }

  void ExtendSegment(int attr_pos, int w)
  {
    double min_length = numeric_limits<double>::max();
    double max_length = numeric_limits<double>::min();

    for(int i = 1;i <= l->GetNoTuples();i++){
      Tuple* t = l->GetTuple(i);
      SimpleLine* sline = (SimpleLine*)t->GetAttribute(attr_pos);
      if(sline->Length() < min_length) min_length = sline->Length();
      if(sline->Length() > max_length) max_length = sline->Length();
      t->DeleteIfAllowed();
    }


    for(int i = 1;i <= l->GetNoTuples();i++){
//      cout<<"line "<<i<<endl;
      Tuple* t = l->GetTuple(i);
      SimpleLine* sline = (SimpleLine*)t->GetAttribute(attr_pos);
      vector<MyHalfSegment> seq_halfseg; //reorder it from start to end
      ReorderLine(sline,seq_halfseg);

      int delta1;//width of road on each side, depend on the road length
      int delta2;
      //main road-2w, side road--w, pavement -- w/2
      if(sline->Length() < (min_length+max_length)/2){
           delta1 = w;

           int delta;
           delta = w/2;
           if(delta < 2) delta = 2;
           delta2 = w + delta;
      }
      else{
          delta1 = 2*w;

          int delta;
          delta = w/2;
          if(delta < 2) delta = 2;
          delta2 = 2*(w + delta);
      }
      vector<Point> outer_s;
      vector<Point> outer1;
      vector<Point> outer2;
      vector<Point> outer4;
      vector<Point> outer_l;
      vector<Point> outer5;

      assert(delta1 != delta2);


      bool add = true;
//      cout<<"small1"<<endl;
      ExtendSeg1(seq_halfseg,delta1,true,outer_s,add,outer1);
//      cout<<"large1"<<endl;
      ExtendSeg2(seq_halfseg,delta2,true,outer_l,add);
      /////////////////////////////////////////////////////////////

      outer_l.push_back(outer_s[1]);
      for(int j = outer_l.size() - 1;j > 0;j--)
        outer_l[j] = outer_l[j-1];
      outer_l[1] = outer_s[1];
      outer_l.push_back(outer_s[outer_s.size() - 1]);
      int point_count1 = outer_s.size();
      int point_count2 = outer_l.size();
      //////////////////paveroad--larger/////////////////////////////////

      for(unsigned int j = 0;j < outer_l.size();j++)
          outer4.push_back(outer_l[j]);
      for(int j = seq_halfseg.size() - 1; j >= 0; j--)
          outer4.push_back(seq_halfseg[j].GetRightPoint());

      //////////////////////////////////////////////////////////
      ExtendSeg1(seq_halfseg,delta1,false,outer_s,add,outer2);
      ExtendSeg2(seq_halfseg,delta2,false,outer_l,add);
      ////////////////////////////////////////////////////////////
      outer_l.push_back(outer_s[point_count1+1]);
      for(int j = outer_l.size() - 1;j > point_count2;j--)
        outer_l[j] = outer_l[j-1];
      outer_l[point_count2 + 1] = outer_s[point_count1 + 1];
      outer_l.push_back(outer_s[outer_s.size() - 1]);
      //////////////////paveroad---larger////////////////////////////

      for(unsigned int j = 0; j < seq_halfseg.size(); j++)
          outer5.push_back(seq_halfseg[j].GetLeftPoint());
      for(unsigned int j = point_count2; j < outer_l.size();j++)
          outer5.push_back(outer_l[j]);
      /////////////////////////////////////////////////////////////
      t->DeleteIfAllowed();
      /////////////get the boundary//////////////////
      if(add){
        ComputeRegion(outer_s, outer_regions_s);
        ComputeRegion(outer1, outer_regions1);
        ComputeRegion(outer2, outer_regions2);
        ComputeRegion(outer_l, outer_regions_l);
        ComputeRegion(outer4, outer_regions4);
        ComputeRegion(outer5, outer_regions5);
      }
      assert(add); //it should be true, dirty data is preprocessed already
    }
  }

  ////////////union the pavement for each junction/////////////////
  void Getpavement(Network* n, Relation* rel, int attr_pos1, int attr_pos2)
  {
    cout<<"attr1 "<<attr_pos1<<" attr2 "<<attr_pos2<<endl;

/*    vector<Region> regions;
    for(int i = 1;i <= n->GetNoRoutes();i++){
        cout<<"route "<<i<<endl;
        CcInt* rid_id = new CcInt(true,i);
        vector<JunctionSortEntry> juns;
        n->GetJunctionsOnRoute(rid_id,juns);

        //////////get all its intersection routes//////////////////////
        vector<int> rids;

        for(unsigned int j = 0;j < juns.size();j++){
            CcInt* rid1 =
                (CcInt*)juns[j].m_pJunction->GetAttribute(JUNCTION_ROUTE1_ID);
            CcInt* rid2 =
                (CcInt*)juns[j].m_pJunction->GetAttribute(JUNCTION_ROUTE2_ID);
            if(rid1->GetIntval() != i) rids.push_back(rid1->GetIntval());
            if(rid2->GetIntval() != i) rids.push_back(rid2->GetIntval());
        }
        Tuple* tup_pave = rel->GetTuple(i);
        Region* tup_reg = (Region*)tup_pave->GetAttribute(attr_pos);
        Region* all_reg = new Region(*tup_reg);
        for(unsigned int j = 0;j < rids.size();j++){
            cout<<"intersection route "<<rids[j]<<endl;
            Tuple* tuple = rel->GetTuple(rids[j]);
            Region* reg = (Region*)tuple->GetAttribute(attr_pos);
            Region* result = new Region(0);
            all_reg->Union(*reg,*result);
            *all_reg = *result;
            delete result;
            tuple->DeleteIfAllowed();
        }

        regions.push_back(*all_reg);
        delete all_reg;
        tup_pave->DeleteIfAllowed();
        delete rid_id;
    }*/
    vector<Region> regions;
    for(int i = 1;i <= n->GetNoRoutes();i++){
        Region* temp = new Region(0);
        regions.push_back(*temp);
        delete temp;
    }

      Relation* juns = n->GetJunctions();
      for(int i = 1;i <= n->GetNoJunctions();i++){
//        cout<<"jun "<<i<<endl;
        Tuple* jun_tuple = juns->GetTuple(i);
        CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
        CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);

        Tuple* border1 = rel->GetTuple(rid1->GetIntval());
        Tuple* border2 = rel->GetTuple(rid2->GetIntval());
        Region* reg1_s = (Region*)border1->GetAttribute(attr_pos1);
        Region* reg1_l = (Region*)border1->GetAttribute(attr_pos2);

        Region* reg2_s = (Region*)border2->GetAttribute(attr_pos1);
        Region* reg2_l = (Region*)border2->GetAttribute(attr_pos2);

        Region* reg_inter_l = new Region(0);
        reg1_l->Intersection(*reg2_l,*reg_inter_l);
        Region* reg_inter_s = new Region(0);
        reg1_s->Intersection(*reg2_s,*reg_inter_s);

        Region* result = new Region(0);
        reg_inter_l->Minus(*reg_inter_s,*result);

//        outer_regions1.push_back(*reg_inter_s);
//        outer_regions2.push_back(*reg_inter_l);
        outer_regions_s.push_back(*result);

/*        Region* result_rid1 = new Region(0);
        regions[rid1->GetIntval() - 1].Union(*result1,*result_rid1);
        regions[rid1->GetIntval() - 1] = *result_rid1;
        delete result_rid1;

        Region* result_rid2 = new Region(0);
        regions[rid2->GetIntval() - 1].Union(*result1,*result_rid2);
        regions[rid2->GetIntval() - 1] = *result_rid2;
        delete result_rid2;*/

        delete result;
        delete reg_inter_l;
        delete reg_inter_s;
        border1->DeleteIfAllowed();
        border2->DeleteIfAllowed();
        jun_tuple->DeleteIfAllowed();

      }

    juns->Delete();
  }
};

////////////string for Operator Spec //////////////////////////////////
const string OpNetCheckSlineSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>sline-> sline</text--->"
    "<text>checksline(sline)</text--->"
    "<text>correct dirty route line </text--->"
    "<text>query routes(n) feed extend[newcurve: checksline(.curve)] count"
    "</text--->"
    ") )";
const string OpNetModifyBoundarySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rectangle x int -> region</text--->"
    "<text>modifyboundary(rectangle,2)</text--->"
    "<text>modify the boundary of road network</text--->"
    "<text>query modifyboundary(bbox(rtreeroad),2)"
    "</text--->"
    ") )";

const string OpNetSegment2RegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>relation x attr_name x int->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>segment2region(rel,attr, int)</text--->"
    "<text>extend the halfsegment to a small region </text--->"
    "<text>query segment2region(allroutes,curve,2) count"
    "</text--->"
    ") )";

const string OpNetPaveRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x relation x attr_name->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>paveregion(n,rel,attr)</text--->"
    "<text>get the intersection pavement region for each junction </text--->"
    "<text>query paveregion(n,allregions_paves,curve) count"
    "</text--->"
    ") )";

////////////////TypeMap function for operators//////////////////////////////

/*
TypeMap fun for operator checksline

*/

ListExpr OpNetCheckSlineTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );


  if (nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
      nl->SymbolValue(param1) == "sline")

  {
    return nl->SymbolAtom ( "sline" );
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator boundaryregion

*/

ListExpr OpNetModifyBoundaryTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );


  if (nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
      nl->SymbolValue(param1) == "rect" &&
      nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&
      nl->SymbolValue(param2) == "int")
  {
    return nl->SymbolAtom ( "region" );
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator segment2region

*/

ListExpr OpNetSegment2RegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr attrName = nl->Second(args);
  ListExpr param3 = nl->Third(args);
  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname,attrType);

  if(j == 0 || !listutils::isSymbol(attrType,"sline")){
      return listutils::typeError("attr name" + aname + "not found"
                      "or not of type sline");
  }

    if (listutils::isRelDescription(param1) &&
        nl->IsAtom(param3) && nl->AtomType(param3) == SymbolType &&
        nl->SymbolValue(param3) == "int"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                    nl->Cons(
                      nl->TwoElemList(nl->SymbolAtom("oid"),
                                    nl->SymbolAtom("int")),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("road1"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("road2"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("inborder"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("paveroad1"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("paveroad2"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("outborder"),
                                    nl->SymbolAtom("region"))

                  )
                 )
                )
          );

//    return result;
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                             nl->OneElemList(nl->IntAtom(j)),result);
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator paveregion

*/

ListExpr OpNetPaveRegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr attrName1 = nl->Third(args);
  ListExpr attrName2 = nl->Fourth(args);

  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"region")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);


  if(j2 == 0 || !listutils::isSymbol(attrType2,"region")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

    if (listutils::isRelDescription(param2) &&
        nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "network"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("pavement"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
//                         nl->OneElemList(nl->IntAtom(j1)),result);
                  nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);
  }
  return nl->SymbolAtom ( "typeerror" );
}
////////////////////////////////////////////////////////////////////////////
/*
Correct road with dirt data, two segment are very close to each other and the
angle is relatively small.
In Berlin road network, there are three, 454,542 and 2324.

*/
int OpNetCheckSlinemap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  static int count = 1;

  result = qp->ResultStorage(in_pSupplier);
  SimpleLine* input_line = (SimpleLine*)args[0].addr;
  SimpleLine* res = static_cast<SimpleLine*>(result.addr);

  const double delta_dist = 0.1;
  vector<MyHalfSegment> segs;
  LinePartiton* lp = new LinePartiton();
  lp->ReorderLine(input_line,segs);
  vector<MyHalfSegment> boundary;
  int delta = 1;
  bool clock_wise = true;
  for(unsigned int i = 0;i < segs.size();i++)
    lp->ModifySegment(segs[i],boundary,delta,clock_wise);


  for(unsigned int i = 0;i < boundary.size() - 1; i++){
      Point p1_1 = boundary[i].GetLeftPoint();
      Point p1_2 = boundary[i].GetRightPoint();
      Point p2_1 = boundary[i+1].GetLeftPoint();
      Point p2_2 = boundary[i+1].GetRightPoint();

      if(p1_2.Distance(p2_1) < delta_dist) continue;

      if(AlmostEqual(p1_1.GetX(),p1_2.GetX())){
        assert(!AlmostEqual(p2_1.GetX(),p2_2.GetX()));
      }else{
        if(AlmostEqual(p2_1.GetX(),p2_2.GetX())){
          assert(!AlmostEqual(p1_1.GetX(),p1_2.GetX()));
        }else{
          double a1 = (p1_2.GetY()-p1_1.GetY()) /(p1_2.GetX()-p1_1.GetX());
          double b1 = p1_2.GetY() - a1*p1_2.GetX();

          double a2 = (p2_2.GetY()-p2_1.GetY()) /(p2_2.GetX()-p2_1.GetX());
          double b2 = p2_2.GetY() - a2*p2_2.GetX();

//          assert(!AlmostEqual(a1,a2));
          if(AlmostEqual(a1,a2)) assert(AlmostEqual(b1,b2));

          double x = (b2-b1)/(a1-a2);
          double y = a1*x + b1;
          ////////////process speical case///////angle too small////////////
          Point q1;
          q1.Set(x,y);
          Point q2 = segs[i].GetRightPoint();
          if(q1.Distance(q2) > 5*delta){
            cout<<"line "<<count<<" dirty road data, modify it"<<endl;
            segs[i+1].def = false;
            if(i < segs.size() - 2){
              Point newp;
              Point lp = segs[i+1].GetLeftPoint();
              Point rp = segs[i+1].GetRightPoint();
              newp.Set((lp.GetX()+rp.GetX())/2, (lp.GetY()+rp.GetY())/2);
              segs[i].to = newp;
              segs[i+2].from = newp;
              i++;
            }
          }
        }
      }// end if

    }//end for

  delete lp;

  SimpleLine* sline = new SimpleLine(0);
  sline->StartBulkLoad();
  int edgeno = 0;
  for(unsigned int i = 0;i < segs.size();i++){
    if(segs[i].def == false)  continue;
    HalfSegment* seg =
         new HalfSegment(true,segs[i].GetLeftPoint(),segs[i].GetRightPoint());
    seg->attr.edgeno = edgeno++;
    *sline += *seg;
    seg->SetLeftDomPoint(!seg->IsLeftDomPoint());
    *sline += *seg;
    delete seg;
  }
  sline->EndBulkLoad();
  *res = *sline;
  delete sline;

  count++;
  return 0;
}
/*
Extend the bbox of a region by a small value, it is given in the input as a
paramter, e.g., roadwidth
Value Mapping for the modifyboundary operator

*/

int OpNetModifyBoundarymap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Rectangle<2>* rect = (Rectangle<2>*)args[0].addr;
  int wide = ((CcInt*)args[1].addr)->GetIntval();
  result = qp->ResultStorage( in_pSupplier );
  Region *pResult = (Region *)result.addr;


  double x1 = rect->MinD(0);
  double x2 = rect->MaxD(0);
  double y1 = rect->MinD(1);
  double y2 = rect->MaxD(1);
  Point p;

  wide = wide * 2;

  LinePartiton* lp = new LinePartiton();
  vector<Point> ps;
  vector<Region> regions;
  int x, y;
  x = static_cast<int>(GetCloser(x1 - wide));
  y = static_cast<int>(GetCloser(y1 - wide));
  p.Set(x,y);
  ps.push_back(p);

  x = static_cast<int>(GetCloser(x2 + wide));
  y = static_cast<int>(GetCloser(y1 - wide));
  p.Set(x,y);
  ps.push_back(p);

  x = static_cast<int>(GetCloser(x2 + wide));
  y = static_cast<int>(GetCloser(y2 + wide));
  p.Set(x,y);
  ps.push_back(p);

  x = static_cast<int>(GetCloser(x1 - wide));
  y = static_cast<int>(GetCloser(y2 + wide));
  p.Set(x,y);
  ps.push_back(p);


  lp->ComputeRegion(ps,regions);
  delete lp;

  *pResult = regions[0];
  return 0;
}

/*
Value Mapping for the segment2region operator
for each road, get the stripe for street plus pavement

*/

int OpNetSegment2Regionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  LinePartiton* l_partition;

  switch(message){
      case OPEN:{
//        cout<<"open "<<endl;
        Relation* l = ( Relation* ) args[0].addr;
        int width = ((CcInt*)args[2].addr)->GetIntval();
        int attr_pos = ((CcInt*)args[3].addr)->GetIntval() - 1;

        l_partition = new LinePartiton(l);
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->ExtendSegment(attr_pos,width);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (LinePartiton*)local.addr;
          if(l_partition->count == l_partition->outer_regions_s.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,new CcInt(true,l_partition->count+1));
          tuple->PutAttribute(1,
                  new Region(l_partition->outer_regions1[l_partition->count]));
          tuple->PutAttribute(2,
                  new Region(l_partition->outer_regions2[l_partition->count]));
          tuple->PutAttribute(3,
                  new Region(l_partition->outer_regions_s[l_partition->count]));
          tuple->PutAttribute(4,
                   new Region(l_partition->outer_regions4[l_partition->count]));
          tuple->PutAttribute(5,
                   new Region(l_partition->outer_regions5[l_partition->count]));
          tuple->PutAttribute(6,
                  new Region(l_partition->outer_regions_l[l_partition->count]));
          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (LinePartiton*)local.addr;
            l_partition->resulttype->DeleteIfAllowed();
            delete l_partition;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
Value Mapping for the paveregion operator
get the region for pavement

*/

int OpNetPaveRegionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  LinePartiton* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;
        int attr_pos1 = ((CcInt*)args[4].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[5].addr)->GetIntval() - 1;

        l_partition = new LinePartiton();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->Getpavement(n,rel,attr_pos1,attr_pos2);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (LinePartiton*)local.addr;
          if(l_partition->count == l_partition->outer_regions_s.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,new CcInt(true,l_partition->count+1));
          tuple->PutAttribute(1,
                  new Region(l_partition->outer_regions_s[l_partition->count]));

          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (LinePartiton*)local.addr;
            l_partition->resulttype->DeleteIfAllowed();
            delete l_partition;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}
////////////////Operator Constructor///////////////////////////////////////
Operator checksline(
    "checksline",               // name
    OpNetCheckSlineSpec,          // specification
    OpNetCheckSlinemap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetCheckSlineTypeMap        // type mapping
);

Operator modifyboundary (
    "modifyboundary",               // name
    OpNetModifyBoundarySpec,          // specification
    OpNetModifyBoundarymap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetModifyBoundaryTypeMap        // type mapping
);

Operator segment2region(
    "segment2region",               // name
    OpNetSegment2RegionSpec,          // specification
    OpNetSegment2Regionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetSegment2RegionTypeMap        // type mapping
);

Operator paveregion(
    "paveregion",               // name
    OpNetPaveRegionSpec,          // specification
    OpNetPaveRegionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetPaveRegionTypeMap        // type mapping
);

/*
Main Class for Transportation Mode

*/
class TransportationModeAlgebra : public Algebra
{
 public:
  TransportationModeAlgebra() : Algebra()
  {
    ////operators for partition regions////
    AddOperator(&checksline);
    AddOperator(&modifyboundary);
    AddOperator(&segment2region);
    AddOperator(&paveregion);
  }
  ~TransportationModeAlgebra() {};
 private:



};

};


extern "C"
Algebra* InitializeTransportationModeAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
    nl = nlRef;
    qp = qpRef;
  // The C++ scope-operator :: must be used to qualify the full name
  return new TransportationMode::TransportationModeAlgebra();
    }
