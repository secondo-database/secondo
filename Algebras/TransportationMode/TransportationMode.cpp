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
#include "Partition.h"

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
  vector<Line> pave_line1;
  vector<Line> pave_line2;
  LinePartiton()
  {
      l = NULL;
      resulttype = NULL;
      count = 0;
  }
  LinePartiton(Relation* in_line):l(in_line),count(0){}

  void GetDeviation(Point center, double a, double b,double& x1,
                   double& x2, int delta)
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
    double x,y;
    x = p.GetX();
    y = p.GetY();
//    printf("%.10f %.10f\n",x, y);
    x = ((int)(x*100.0 +0.5))/100.0;
    y = ((int)(y*100.0 +0.5))/100.0;
//    printf("%.10f %.10f\n",x, y);
    p.Set(x,y);
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

      if(add == false) return;

      if(clock_wise){
        for(unsigned int i = 0;i < boundary.size();i++){
          ///////////////outer segments////////////////////////////////
          Point p = boundary[i].GetLeftPoint();
          ModifyPoint(p);

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
                vector<Point>& outer, bool& add)
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

      regs.push_back(*cr);
      delete cr;

  }

  void ExtendSegment(int attr_pos, int w)
  {
    if(w < 3){
      cout<<"road width should be larger than 2"<<endl;
      return;
    }
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

  void CheckPaveRegion(Region& reg, Region& pave1, Region& pave2,
                       vector<Region>& paves,int rid, Region* inborder)
  {
    //cut the intersection region between pavement and road

//    cout<<"area1 "<<reg1->Area()<<" area2 "<<reg2->Area()<<endl;
//    cout<<"size1 "<<reg1->Size()<<" size2 "<<reg2->Size()<<endl;

//    if((reg1->Size() >= 6 && reg2->Size() < 6) ||
//       (reg2->Size() >= 6 && reg1->Size() < 6)){

      Region* comm_reg = new Region(0);
//      reg.Intersection(*inborder,*comm_reg);
      MyIntersection(reg,*inborder,*comm_reg);

      Region* result = new Region(0);
//      reg.Minus(*comm_reg,*result);
      MyMinus(reg,*comm_reg,*result);

      paves[rid - 1] = *result;
      delete result;

      delete comm_reg;
//    }

  }


  void Getpavement(Network* n, Relation* rel1, int attr_pos,
                  Relation* rel2, int attr_pos1, int attr_pos2)
  {
   //cut the pavement for each junction
    vector<Region> pavements1;
    vector<Region> pavements2;
    Relation* routes = n->GetRoutes();
    vector<double> routes_length;
    for(int i = 1;i <= rel2->GetNoTuples();i++){
      Tuple* tuple = rel2->GetTuple(i);
      Region* reg1 = (Region*)tuple->GetAttribute(attr_pos1);
      Region* reg2 = (Region*)tuple->GetAttribute(attr_pos2);
      pavements1.push_back(*reg1);
      pavements2.push_back(*reg2);
      tuple->DeleteIfAllowed();

      Tuple* route_tuple = routes->GetTuple(i);
      CcReal* len = (CcReal*)route_tuple->GetAttribute(ROUTE_LENGTH);
      double length = len->GetRealval();
      route_tuple->DeleteIfAllowed();
      routes_length.push_back(length);

    }

//    routes->Delete();

    Relation* juns = n->GetJunctions();

    for(int i = 1;i <= n->GetNoJunctions();i++){
      Tuple* jun_tuple = juns->GetTuple(i);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();


/*      if(!(id1 == 1754 || id2 == 1754)) {
          jun_tuple->DeleteIfAllowed();
          continue;
      }*/
//      cout<<"rid1 "<<id1<<" rid2 "<<id2<<endl;

//      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
//      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

//      double len1 = meas1->GetRealval();
//      double len2 = meas2->GetRealval();
//      double delta = 0.05;

//      printf("len1 %.8f, len %.8f\n", len1, len2);
//      printf("route1 len %.8f route2 len %.8f\n",routes_length[id1 - 1],
//          routes_length[id2 - 1]);

      Region rid1_pave1 = pavements1[id1 - 1];
      Region rid1_pave2 = pavements2[id1 - 1];

      Region rid2_pave1 = pavements1[id2 - 1];
      Region rid2_pave2 = pavements2[id2 - 1];

      Tuple* inborder_tuple1 = rel1->GetTuple(id1);
      Tuple* inborder_tuple2 = rel1->GetTuple(id2);
      Region* reg1 = (Region*)inborder_tuple1->GetAttribute(attr_pos);
      Region* reg2 = (Region*)inborder_tuple2->GetAttribute(attr_pos);

/*      if((delta < len1 && (routes_length[id1 - 1] - len1) > delta) &&
         (delta < len2 && (routes_length[id2 - 1] - len2) > delta)){
        inborder_tuple1->DeleteIfAllowed();
        inborder_tuple2->DeleteIfAllowed();
        jun_tuple->DeleteIfAllowed();
        continue;
      }*/

/*      if((len1 < delta|| fabs(len1 - routes_length[id1 - 1]) < delta)
        && (delta < len2 && fabs(len2 - routes_length[id2-1]) > delta)){*/
        CheckPaveRegion(rid1_pave1,rid2_pave1,rid2_pave2,pavements1,id1,reg2);
        CheckPaveRegion(rid1_pave2,rid2_pave1,rid2_pave2,pavements2,id1,reg2);
//      }


/*      if((len2 < delta|| fabs(len2 - routes_length[id2 - 1]) < delta)
        && (delta < len1 && fabs(len1 - routes_length[id1-1]) > delta)){*/
        CheckPaveRegion(rid2_pave1,rid1_pave1,rid1_pave2,pavements1,id2,reg1);
        CheckPaveRegion(rid2_pave2,rid1_pave1,rid1_pave2,pavements2,id2,reg1);
//      }

      inborder_tuple1->DeleteIfAllowed();
      inborder_tuple2->DeleteIfAllowed();
      jun_tuple->DeleteIfAllowed();
    }

    juns->Delete();

    for(unsigned int i = 0;i < pavements1.size();i++){
        outer_regions1.push_back(pavements1[i]);
        outer_regions2.push_back(pavements2[i]);
    }

  }
  void CrossHalfSegment(HalfSegment& hs, int delta, bool flag)
  {
//      cout<<"before hs "<<hs<<endl;
      Point lp = hs.GetLeftPoint();
      Point rp = hs.GetRightPoint();
//      cout<<"delta "<<delta<<endl;
      if(MyAlmostEqual(lp.GetY(), rp.GetY())){
          Point p1, p2;

          if(flag){
            p1.Set(lp.GetX(), lp.GetY() + delta);
            p2.Set(rp.GetX(), rp.GetY() + delta);
          }else{
            p1.Set(lp.GetX(), lp.GetY() - delta);
            p2.Set(rp.GetX(), rp.GetY() - delta);
          }

          hs.Set(true,p1,p2);
      }else if(MyAlmostEqual(lp.GetX(), rp.GetX())){
          Point p1, p2;
          if(flag){
            p1.Set(lp.GetX() + delta, lp.GetY());
            p2.Set(rp.GetX() + delta, rp.GetY());
          }else{
            p1.Set(lp.GetX() - delta, lp.GetY());
            p2.Set(rp.GetX() - delta, rp.GetY());
          }
          hs.Set(true,p1,p2);
      }else{

          double k1 = (lp.GetY() - rp.GetY())/(lp.GetX() - rp.GetX());
          double k2 = -1/k1;
          double c1 = lp.GetY() - lp.GetX()*k2;
          double c2 = rp.GetY() - rp.GetX()*k2;

          double x1 , x2;
          x1 = 0.0; x2 = 0.0;
          GetDeviation(lp, k2, c1, x1, x2, delta);
          double y1 = x1*k2 + c1;
          double y2 = x2*k2 + c1;

          double x3, x4;
          x3 = 0.0; x4 = 0.0;
          GetDeviation(rp, k2, c2, x3, x4, delta);
          double y3 = x3*k2 + c2;
          double y4 = x4*k2 + c2;
          Point p1,p2;

//          cout<<"x1 "<<x1<<" x2 "<<x2<<" y1 "<<y1<<" y2 "<<y2<<endl;

          if(flag){
            p1.Set(x1, y1);
            p2.Set(x3, y3);
          }else{
            p1.Set(x2, y2);
            p2.Set(x4, y4);
          }
          hs.Set(true, p1, p2);
      }
//      cout<<"after hs "<<hs<<endl;
  }

  void GetSubCurve(SimpleLine* curve, Line* newcurve,
                    int roadwidth, bool clock)
  {
      newcurve->StartBulkLoad();
      for(int i = 0; i < curve->Size();i++){
          HalfSegment hs;
          curve->Get(i,hs);
          CrossHalfSegment(hs, roadwidth, clock);
          *newcurve += hs;
      }
      newcurve->EndBulkLoad();
  }
/*
for the road line around the junction position, it creates the zebra crossing

*/
  void GetZebraCrossing(SimpleLine* subcurve,
                        Region* reg_pave1, Region* reg_pave2,
                        int roadwidth, Line* pave1, double delta_l,
                        Point p1, Region* crossregion)
  {
    vector<MyPoint> endpoints1;
    vector<MyPoint> endpoints2;
    double delta = 1;

    Line* subline1 = new Line(0);
    Line* subline2 = new Line(0);

    Point junp = p1;

    GetSubCurve(subcurve, subline1, roadwidth + 1, true);
    GetSubCurve(subcurve, subline2, roadwidth + 1, false);

//    *pave1 = *subline1;

    Point p2;
    double l;

/*    if(flag)
      l = delta;
    else
      l = subcurve->Length() - delta;*/

    //////////////////////////////////////////////////////
    Point startp, endp;
    assert(subcurve->AtPosition(0.0, true, startp));
    assert(subcurve->AtPosition(subcurve->Length(), true, endp));
    bool flag1;
    if(startp.Distance(p1) < endp.Distance(p1)){
      flag1 = true;
      l = delta;
    }
    else{
      l = subcurve->Length() - delta;
      flag1 = false;
    }

//    cout<<"l "<<l<<endl;
//    cout<<"p1 "<<p1<<" startp "<<startp<<"endp "<<endp<<endl;
    ///////////////////////////////////////////////////////

    bool find = false;
    while(find == false && (0 < l && l < subcurve->Length())){
/*      if(flag)
        l = l + delta;
      else
        l = l - delta;*/

      if(flag1)
        l = l + delta;
      else
        l = l - delta;

      assert(subcurve->AtPosition(l,true,p2));


      if(MyAlmostEqual(p1.GetX(), p2.GetX())){
        double y = p2.GetY();
        double x1 = p2.GetX() - (roadwidth + 2);
        double x2 = p2.GetX() + (roadwidth + 2);
        Line* line1 = new Line(0);
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


        Points* ps1 = new Points(0);
        Points* ps2 = new Points(0);
        subline1->Crossings(*line1,*ps1);
        subline2->Crossings(*line1,*ps2);

        if(ps1->Size() > 0 && ps2->Size() > 0 &&
             ((ps1->Inside(*reg_pave1) && ps2->Inside(*reg_pave2)) ||
             (ps1->Inside(*reg_pave2) && ps2->Inside(*reg_pave1)))){

//        if(ps1->Size() > 0 && ps2->Size() > 0){
//          cout<<"1 "<<p2<<endl;

          for(int i = 0;i < ps1->Size();i++){
            Point p;
            ps1->Get(i,p);
            MyPoint mp(p,p.Distance(p2));
            endpoints1.push_back(mp);

          }
          for(int i = 0;i < ps2->Size();i++){
            Point p;
            ps2->Get(i,p);
            MyPoint mp(p, p.Distance(p2));
            endpoints2.push_back(mp);
          }

          sort(endpoints1.begin(),endpoints1.end());
          sort(endpoints2.begin(),endpoints2.end());
          find = true;
        }
        p1 = p2;

        delete ps1;
        delete ps2;

        delete line1;

      }else if(MyAlmostEqual(p1.GetY(), p2.GetY())){
            double y1 = p2.GetY() - (roadwidth + 2);
            double y2 = p2.GetY() + (roadwidth + 2);
            double x = p2.GetX();

            Line* line1 = new Line(0);
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


            Points* ps1 = new Points(0);
            Points* ps2 = new Points(0);
            subline1->Crossings(*line1,*ps1);
            subline2->Crossings(*line1,*ps2);

            if(ps1->Size() > 0 && ps2->Size() > 0 &&
             ((ps1->Inside(*reg_pave1) && ps2->Inside(*reg_pave2)) ||
              (ps1->Inside(*reg_pave2) && ps2->Inside(*reg_pave1)))){
//            if(ps1->Size() > 0 && ps2->Size() > 0){

//              cout<<"2 "<<p2<<endl;

            for(int i = 0;i < ps1->Size();i++){
              Point p;
              ps1->Get(i,p);
              MyPoint mp(p,p.Distance(p2));
              endpoints1.push_back(mp);
            }
            for(int i = 0;i < ps2->Size();i++){
              Point p;
              ps2->Get(i,p);
              MyPoint mp(p, p.Distance(p2));
              endpoints2.push_back(mp);
            }

            sort(endpoints1.begin(),endpoints1.end());
            sort(endpoints2.begin(),endpoints2.end());
            find = true;
          }
          p1 = p2;

          delete ps1;
          delete ps2;
          delete line1;
      }else{//not vertical
        double k1 = (p2.GetY() - p1.GetY())/(p2.GetX() - p1.GetX());
        double k2 = -1/k1;
        double c2 = p2.GetY() - p2.GetX()*k2;

        double x1 = p2.GetX() - (roadwidth + 2);
        double x2 = p2.GetX() + (roadwidth + 2);

        Line* line1 = new Line(0);
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

        Points* ps1 = new Points(0);
        Points* ps2 = new Points(0);
        subline1->Crossings(*line1,*ps1);
        subline2->Crossings(*line1,*ps2);


        if(ps1->Size() > 0 && ps2->Size() > 0 &&
            ((ps1->Inside(*reg_pave1) && ps2->Inside(*reg_pave2)) ||
             (ps1->Inside(*reg_pave2) && ps2->Inside(*reg_pave1)))){

//        if(ps1->Size() > 0 && ps2->Size() > 0){
//          cout<<"3 "<<p2<<endl;
          for(int i = 0;i < ps1->Size();i++){
            Point p;
            ps1->Get(i,p);
            MyPoint mp(p, p.Distance(p2));
            endpoints1.push_back(mp);

          }
          for(int i = 0;i < ps2->Size();i++){
            Point p;
            ps2->Get(i,p);
            MyPoint mp(p, p.Distance(p2));
            endpoints2.push_back(mp);
          }

          sort(endpoints1.begin(),endpoints1.end());
          sort(endpoints2.begin(),endpoints2.end());
          find = true;
        }

        p1 = p2;

        delete ps1;
        delete ps2;
        delete line1;
      }

    }


    delete subline1;
    delete subline2;


    if(endpoints1.size() > 0 && endpoints2.size() > 0){

      MyPoint lp = endpoints1[0];
      MyPoint rp = endpoints2[0];
      Line* pave = new Line(0);
      pave->StartBulkLoad();
      HalfSegment hs;
      hs.Set(true,lp.loc,rp.loc);
      int edgeno = 0;
      hs.attr.edgeno = edgeno++;
      *pave += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *pave += hs;
      pave->EndBulkLoad();

      *pave1 = *pave;

      delete pave;
      //////////////extend it to a region/////////////////////////////
      HalfSegment hs1 = hs;
      HalfSegment hs2 = hs;
      CrossHalfSegment(hs1, 2, true);
      CrossHalfSegment(hs2, 2, false);
      vector<Point> outer_ps;
      vector<Region> result;
      Point lp1 = hs1.GetLeftPoint();
      Point rp1 = hs1.GetRightPoint();
      Point lp2 = hs2.GetLeftPoint();
      Point rp2 = hs2.GetRightPoint();
      Point mp1, mp2;
      mp1.Set((lp1.GetX() + rp1.GetX())/2, (lp1.GetY() + rp1.GetY())/2);
      mp2.Set((lp2.GetX() + rp2.GetX())/2, (lp2.GetY() + rp2.GetY())/2);


      if(mp1.Distance(junp) > mp2.Distance(junp)){
          lp1 = hs.GetLeftPoint();
          rp1 = hs.GetRightPoint();
          lp2 = hs1.GetLeftPoint();
          rp2 = hs1.GetRightPoint();

      }else{
          lp1 = hs.GetLeftPoint();
          rp1 = hs.GetRightPoint();
          lp2 = hs2.GetLeftPoint();
          rp2 = hs2.GetRightPoint();

      }

      if((lp2.Inside(*reg_pave1) && rp2.Inside(*reg_pave2)) ||
              (lp2.Inside(*reg_pave2) && rp2.Inside(*reg_pave1)) ){
          if(GetClockwise(lp2, lp1, rp2)){
            outer_ps.push_back(lp2);
            outer_ps.push_back(rp2);
            outer_ps.push_back(rp1);
            outer_ps.push_back(lp1);
          }else{
            outer_ps.push_back(lp1);
            outer_ps.push_back(rp1);
            outer_ps.push_back(rp2);
            outer_ps.push_back(lp2);
          }
        ComputeRegion(outer_ps, result);
        *crossregion = result[0];
      }
      ///////////////////////////////////////////////////////////////
    }

  }

/*
Extend the line in decreasing direction

*/
  void Decrease(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave,
                      int roadwidth, Region* crossregion)
  {
    double l = len;
    double delta_l = 20;
//    double delta_l = 30;
    Point p1;
    assert(curve->AtPosition(l, true, p1));

    while(1){
      SimpleLine* subcurve = new SimpleLine(0);
      if(l - delta_l > 0.0)
        curve->SubLine(l - delta_l, l, true, *subcurve);
      else
        curve->SubLine(0.0, l, true, *subcurve);

//    subcurve2->toLine(*pave2); ///crossing at junction

//    cout<<"decrease subcurve1 len"<<subcurve->Length()<<endl;

      GetZebraCrossing(subcurve, reg_pave1,
                   reg_pave2, roadwidth, pave, delta_l, p1, crossregion);
      delete subcurve;
      if(pave->Size() > 0 || delta_l >= curve->Length()) break;

      delta_l += delta_l;

    }
  }

/*
Extend the line in increasing direction

*/

  void Increase(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave,
                      int roadwidth, Region* crossregion)
  {

    double route_length = curve->Length();
    double l = len;
    double delta_l = 20;
//    double delta_l = 30;
    Point p1;
    assert(curve->AtPosition(l, true, p1));

    while(1){
      SimpleLine* subcurve = new SimpleLine(0);
      if(l + delta_l < route_length)
        curve->SubLine(l, l+delta_l, true, *subcurve);
      else
        curve->SubLine(l, route_length, true, *subcurve);

  //    subcurve1->toLine(*pave1);/////////crossing at junction

        GetZebraCrossing(subcurve, reg_pave1,
                reg_pave2, roadwidth, pave, delta_l, p1, crossregion);

      delete subcurve;
      if(pave->Size() > 0 || delta_l >= route_length) break;

      delta_l += delta_l;

    }

  }


  void CreatePavement(SimpleLine* curve, Region* reg_pave1,
                      Region* reg_pave2, double len, Line* pave1,
                      Line* pave2, int roadwidth, Region* crossregion)
  {
    Region* crossreg1 = new Region(0);
    Region* crossreg2 = new Region(0);
    if(MyAlmostEqual(curve->Length(), len))
      Decrease(curve, reg_pave1, reg_pave2, len, pave2,
               roadwidth, crossreg2);//--
    else if(MyAlmostEqual(len, 0.0))
      Increase(curve, reg_pave1, reg_pave2, len, pave1,
              roadwidth, crossreg1);//++
    else{
      Increase(curve, reg_pave1, reg_pave2, len, pave1,
              roadwidth, crossreg1);
      Decrease(curve, reg_pave1, reg_pave2, len, pave2,
              roadwidth, crossreg2);
    }

    MyUnion(*crossreg1, *crossreg2, *crossregion);
    delete crossreg1;
    delete crossreg2;

  }

  void Junpavement(Network* n, Relation* rel, int attr_pos1,
                  int attr_pos2, int width)
  {
    //get the pavement for each junction
    Relation* routes = n->GetRoutes();
    vector<double> routes_length;
    double min_length = numeric_limits<double>::max();
    double max_length = numeric_limits<double>::min();

    for(int i = 1;i <= routes->GetNoTuples();i++){

      Tuple* route_tuple = routes->GetTuple(i);
      CcReal* len = (CcReal*)route_tuple->GetAttribute(ROUTE_LENGTH);
      double length = len->GetRealval();

      if(length < min_length) min_length = length;
      if(length > max_length) max_length = length;

      route_tuple->DeleteIfAllowed();
      routes_length.push_back(length);

    }

    Relation* juns = n->GetJunctions();

    for(int i = 1;i <= n->GetNoJunctions();i++){
      Tuple* jun_tuple = juns->GetTuple(i);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();

/*      if(!(id1 == 7 && id2 == 8)){
          jun_tuple->DeleteIfAllowed();
          continue;
      }*/

//      cout<<"rid1 "<<id1<<" rid2 "<<id2<<endl;

      CcReal* meas1 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_MEAS);
      CcReal* meas2 = (CcReal*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_MEAS);

      double len1 = meas1->GetRealval();
      double len2 = meas2->GetRealval();
//      double delta = 0.05;

//      cout<<"len1 "<<len1<<" len2 "<<len2<<endl;

      Tuple* inborder_tuple1 = rel->GetTuple(id1);
      Tuple* inborder_tuple2 = rel->GetTuple(id2);
      Region* reg1_in = (Region*)inborder_tuple1->GetAttribute(attr_pos1);
      Region* reg1_out = (Region*)inborder_tuple1->GetAttribute(attr_pos2);
      Region* reg2_in = (Region*)inborder_tuple2->GetAttribute(attr_pos1);
      Region* reg2_out = (Region*)inborder_tuple2->GetAttribute(attr_pos2);


      Tuple* route_tuple1 = routes->GetTuple(id1);
      SimpleLine* curve1 = (SimpleLine*)route_tuple1->GetAttribute(ROUTE_CURVE);
      int input_w1 = width;
      if(curve1->Length() > (min_length + max_length) /2)
          input_w1 = 2*input_w1;

      Line* pave1 = new Line(0);
      Line* pave2 = new Line(0);
      Line* result1 = new Line(0);

      Region* crossregion1 = new Region(0);
      CreatePavement(curve1, reg1_in, reg1_out, len1, pave1,
                     pave2, input_w1, crossregion1);
      pave1->Union(*pave2,*result1);

      junid1.push_back(id1);
      pave_line1.push_back(*result1);
      outer_regions1.push_back(*crossregion1);

      delete crossregion1;
      delete result1;
      delete pave1;
      delete pave2;


      int input_w2 = width;
      Tuple* route_tuple2 = routes->GetTuple(id2);
      SimpleLine* curve2 = (SimpleLine*)route_tuple2->GetAttribute(ROUTE_CURVE);
      if(curve2->Length() > (min_length + max_length) /2)
          input_w2 = 2*input_w2;
      Line* pave3 = new Line(0);
      Line* pave4 = new Line(0);
      Line* result2 = new Line(0);

      Region* crossregion2 = new Region(0);
      CreatePavement(curve2, reg2_in, reg2_out, len2, pave3,
                     pave4, input_w2, crossregion2);
      pave3->Union(*pave4,*result2);

      junid2.push_back(id2);
      pave_line2.push_back(*result2);
      outer_regions2.push_back(*crossregion2);

      delete crossregion2;
      delete result2;
      delete pave3;
      delete pave4;


      route_tuple1->DeleteIfAllowed();

      inborder_tuple1->DeleteIfAllowed();
      inborder_tuple2->DeleteIfAllowed();
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
    "<text>extend the boundary of road network by a small value</text--->"
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
    "( <text>network x rel1 x attr x rel2 x attr1 x attr2->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>paveregion(n,rel1,attr,rel2,attr1,attr2)</text--->"
    "<text>cut the intersection region between road and pavement</text--->"
    "<text>query paveregion(n,allregions_in,inborder,allregions_pave"
    ",pave1,pave2) count;</text--->"
    ") )";

const string OpNetJunRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x int->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>junregion(n,rel,attr1,attr2,int)</text--->"
    "<text>get the pavement region (zebra crossing) at junctions</text--->"
    "<text>query junregion(n,allregions,inborder,outborder,roadwidth) count;"
    "</text--->"
    ") )";

const string OpNetDecomposeRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>decomposeregion(region)</text--->"
    "<text>decompose a region by its faces</text--->"
    "<text>query decomposeregion(partition_regions) count; </text--->"
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
  if ( nl->ListLength ( args ) != 6 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr attrName = nl->Third(args);
  ListExpr param4 = nl->Fourth(args);
  ListExpr attrName1 = nl->Fifth(args);
  ListExpr attrName2 = nl->Sixth(args);

  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname,attrType);

  if(j1 == 0 || !listutils::isSymbol(attrType,"region")){
      return listutils::typeError("attr name" + aname + "not found"
                      "or not of type region");
  }

  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname1,attrType1);


  if(j2 == 0 || !listutils::isSymbol(attrType1,"region")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname2,attrType2);


  if(j3 == 0 || !listutils::isSymbol(attrType2,"region")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

    if (listutils::isRelDescription(param2) &&
        listutils::isRelDescription(param4) &&
        nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "network"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("pavement1"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("pavement2"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
//                         nl->OneElemList(nl->IntAtom(j1)),result);
     nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),result);
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator junregion
get the pavement at each junction area

*/

ListExpr OpNetJunRegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr attrName1 = nl->Third(args);
  ListExpr attrName2 = nl->Fourth(args);
  ListExpr param5 = nl->Fifth(args);

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
        nl->SymbolValue(param1) == "network" &&
        nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&
        nl->SymbolValue(param5) == "int"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
//                      nl->FourElemList(
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("rid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("rid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("crosspave1"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("crosspave2"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("crossreg1"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("crossreg2"),
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

/*
TypeMap fun for operator decomposeregion,
decompose the faces of the input region

*/

ListExpr OpNetDecomposeRegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );

    if (nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "region"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("id"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("covarea"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );

    return result;
  }
  return nl->SymbolAtom ( "typeerror" );
}
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

        l_partition->ExtendSegment(attr_pos, width);
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
get the region for pavement at each junction
cut the dirty area

*/

int OpNetPaveRegionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  LinePartiton* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel1 = (Relation*)args[1].addr;
        Relation* rel2 = (Relation*)args[3].addr;
        int attr_pos = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr_pos1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[8].addr)->GetIntval() - 1;

        l_partition = new LinePartiton();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->Getpavement(n,rel1,attr_pos,rel2,attr_pos1,attr_pos2);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (LinePartiton*)local.addr;
          if(l_partition->count == l_partition->outer_regions1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,new CcInt(true,l_partition->count+1));
          tuple->PutAttribute(1,
                  new Region(l_partition->outer_regions1[l_partition->count]));
          tuple->PutAttribute(2,
                  new Region(l_partition->outer_regions2[l_partition->count]));
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
Value Mapping for the junregion operator
get the region for pavement at each junction area

*/

int OpNetJunRegionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  LinePartiton* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;
        int width = ((CcInt*)args[4].addr)->GetIntval();

        int attr_pos1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[6].addr)->GetIntval() - 1;

        l_partition = new LinePartiton();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->Junpavement(n, rel, attr_pos1, attr_pos2, width);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (LinePartiton*)local.addr;
          if(l_partition->count == l_partition->pave_line1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,l_partition->junid1[l_partition->count]));
          tuple->PutAttribute(1,
                new CcInt(true,l_partition->junid2[l_partition->count]));
          tuple->PutAttribute(2,
                new Line(l_partition->pave_line1[l_partition->count]));
          tuple->PutAttribute(3,
                new Line(l_partition->pave_line2[l_partition->count]));
          tuple->PutAttribute(4,
               new Region(l_partition->outer_regions1[l_partition->count]));
          tuple->PutAttribute(5,
               new Region(l_partition->outer_regions2[l_partition->count]));


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



struct DecomposeRegion{
  Region* reg;
  TupleType* resulttype;
  unsigned int count;
  vector<Region> result;
  DecomposeRegion()
  {
      reg = NULL;
      resulttype = NULL;
      count = 0;
  }
  DecomposeRegion(Region* r):reg(r),count(0){}
  void Decompose()
  {
    int no_faces = reg->NoComponents();
//    cout<<"Decompose() no_faces "<<no_faces<<endl;
    for(int i = 0;i < no_faces;i++){
        Region* temp = new Region(0);

        result.push_back(*temp);
        delete temp;
        result[i].StartBulkLoad();
    }
    for(int i = 0;i < reg->Size();i++){
      HalfSegment hs;
      reg->Get(i,hs);
      int face = hs.attr.faceno;
//      cout<<"face "<<face<<endl;
//      cout<<"hs "<<hs<<endl;
      result[face] += hs;

    }

    for(int i = 0;i < no_faces;i++){
        result[i].EndBulkLoad(false,false,false,false);
//        result[i].EndBulkLoad();
//        cout<<"Area "<<result[i].Area()<<endl;
    }
  }

};

/*
Value Mapping for the decomposeregion operator

*/

int OpNetDecomposeRegionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  DecomposeRegion* dr;

  switch(message){
      case OPEN:{
        Region* r = (Region*)args[0].addr;

        dr= new DecomposeRegion(r);
        dr->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        dr->Decompose();
        local.setAddr(dr);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          dr = (DecomposeRegion*)local.addr;
          if(dr->count == dr->result.size()) return CANCEL;
          Tuple* tuple = new Tuple(dr->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,dr->count + 1));
          tuple->PutAttribute(1,
                new Region(dr->result[dr->count]));

          result.setAddr(tuple);
          dr->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            dr = (DecomposeRegion*)local.addr;
            dr->resulttype->DeleteIfAllowed();
            delete dr;
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

Operator junregion(
    "junregion",               // name
    OpNetJunRegionSpec,          // specification
    OpNetJunRegionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetJunRegionTypeMap        // type mapping
);

Operator decomposeregion(
    "decomposeregion",               // name
    OpNetDecomposeRegionSpec,          // specification
    OpNetDecomposeRegionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpNetDecomposeRegionTypeMap        // type mapping
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
    AddOperator(&junregion);
    AddOperator(&decomposeregion);

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
