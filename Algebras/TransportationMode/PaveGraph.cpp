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

April, 2010 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
creating the graph model for walk planning.


*/
#include "Partition.h"
#include "Triangulate.h"
#include "PaveGraph.h"
#include "GeneralType.h"
#include "Indoor.h"

/*
Decompose the pavement on one side of the road into a set of subregions

*/

void SpacePartition::DecomposePave(Region* reg1, Region* reg2,
                     vector<Region>& result)
{
    vector<Region> temp_result;

    vector<Region> result1;
//    int no_faces = reg1->NoComponents();
//    int no_faces = reg1->Size();//faceno in halfsegment is not recomputed 
    int no_faces = 0;
    for(int i = 0;i < reg1->Size();i++){
      HalfSegment hs;
      reg1->Get(i, hs);
      if(hs.attr.faceno >= no_faces) no_faces = hs.attr.faceno + 1;
    }

    for(int i = 0;i < no_faces;i++){
        Region* temp = new Region(0);

        result1.push_back(*temp);
        delete temp;
        result1[i].StartBulkLoad();
    }

//    cout<<"no faces "<<no_faces<<endl;

    for(int index = 0;index < reg1->Size();index++){
      HalfSegment temp_hs;
      reg1->Get(index, temp_hs);
      int face = temp_hs.attr.faceno;
      if(temp_hs.attr.cycleno == 0){/////do not need the hole 
        result1[face] += temp_hs;
      }
    }

    for(int i = 0;i < no_faces;i++){
        result1[i].SetNoComponents(1);
        result1[i].EndBulkLoad(false,false,false,false);
        if(result1[i].Size() >= 6){
          temp_result.push_back(result1[i]);
        }
    }


    vector<Region> result2;
//    no_faces = reg2->NoComponents();
//    no_faces = reg2->Size();

    no_faces = 0; /// original faceno in halfsegment is not recomputed 
    for(int i = 0;i < reg2->Size();i++){
      HalfSegment hs;
      reg2->Get(i, hs);
      if(hs.attr.faceno >= no_faces) no_faces = hs.attr.faceno + 1;
    }

    for(int i = 0;i < no_faces;i++){
        Region* temp = new Region(0);

        result2.push_back(*temp);
        delete temp;
        result2[i].StartBulkLoad();
    }
    for(int i = 0;i < reg2->Size();i++){
      HalfSegment hs;
      reg2->Get(i,hs);
      int face = hs.attr.faceno;
      if(hs.attr.cycleno == 0){/////do not need the hole
          result2[face] += hs;
      }
    }
    for(int i = 0;i < no_faces;i++){
        result2[i].SetNoComponents(1);
        result2[i].EndBulkLoad(false,false,false,false);
        if(result2[i].Size() >= 6)
          temp_result.push_back(result2[i]);
    }
    //////////////////////////////////////////////////////////

//    cout<<temp_result.size()<<endl;

    for(unsigned int i = 0;i < temp_result.size();i++){
        Line* line = new Line(0);
        temp_result[i].Boundary(line);
//        cout<<"i "<<i<<" len "<<line->Length()<<endl;
        SimpleLine* sline = new SimpleLine(0);
        sline->fromLine(*line);

        vector<MyHalfSegment> mhs;
        ReorderLine(sline, mhs);
        delete sline;
        delete line;
        vector<Point> ps;
        for(unsigned int j = 0;j < mhs.size();j++){
          Point p = mhs[j].from;
//          cout<<"before "<<setprecision(10)<<p;
          Modify_Point(p);
          ps.push_back(p);
//          cout<<"after "<<setprecision(10)<<p<<endl;
        }

        //////////////////////////////////
        vector<Point> newps;
        const double delta_dist = 0.1;
        for(unsigned int i = 0;i < ps.size();i++){
            if(i == 0){
              newps.push_back(ps[i]);
              continue;
            }
          if(i < ps.size() - 1){
            Point last_p = ps[i - 1];
            if(last_p.Distance(ps[i]) > delta_dist){
              newps.push_back(ps[i]);
              continue;
            }
          }
          if(i == ps.size() - 1){
            Point first_p = ps[0];
            if(first_p.Distance(ps[i]) > delta_dist){
              newps.push_back(ps[i]);
              continue;
            }
          }
        }

        ///////////////////////////////////
        vector<Region> regs;
//        ComputeRegion(ps, regs);
        ComputeRegion(newps, regs);
        result.push_back(regs[0]);
        //////////////////////////////////////////////////////

    }
    ///////////////////////////////////////////////////////////
}

/*
get the closest point in hs to p, and return it in cp
it also returns the distance between hs and p

*/
double SpacePartition::GetClosestPoint(HalfSegment& hs, Point& p, Point& cp)
{

  assert( p.IsDefined() );
  Coord xl = hs.GetLeftPoint().GetX(),
        yl = hs.GetLeftPoint().GetY(),
        xr = hs.GetRightPoint().GetX(),
        yr = hs.GetRightPoint().GetY(),
        X = p.GetX(),
        Y = p.GetY();

  double result, auxresult;

  if( xl == xr || yl == yr ){
    if( xl == xr){ //hs is vertical
      if( (yl <= Y && Y <= yr) || (yr <= Y && Y <= yl) ){
        result = fabs( X - xl );
        cp.Set(xl, Y); //store the closest point
      }
      else{
        result = p.Distance(hs.GetLeftPoint());
        auxresult = p.Distance(hs.GetRightPoint());
        if( result > auxresult ){
          result = auxresult;
          cp = hs.GetRightPoint(); //store the closest point
        }else{
          cp = hs.GetLeftPoint();  //store the closest point
        }
      }
    }else{         //hs is horizontal line: (yl==yr)
      if( xl <= X && X <= xr ){
        result = fabs( Y - yl );
        cp.Set(X,yl);//store the closest point
      }else{
        result = p.Distance(hs.GetLeftPoint());
        auxresult = p.Distance(hs.GetRightPoint());
        if( result > auxresult ){
          result = auxresult;
          cp = hs.GetRightPoint();//store the closest point
        }else{
          cp = hs.GetLeftPoint();//store the closest point
        }
      }
    }
  }else
  {
    double k = (yr - yl) / (xr - xl),
           a = yl - k * xl,
           xx = (k * (Y - a) + X) / (k * k + 1),
           yy = k * xx + a;
    Coord XX = xx,
          YY = yy;
    Point PP( true, XX, YY );
    if( xl <= XX && XX <= xr ){
      result = p.Distance( PP );
      cp = PP; //store the closest point
    }
    else
    {
      result = p.Distance( hs.GetLeftPoint() );
      auxresult = p.Distance( hs.GetRightPoint());
      if( result > auxresult ){
        result = auxresult;
        cp = hs.GetRightPoint();
      }else{
        cp = hs.GetLeftPoint();
      }
    }
  }
  return result;

}


double SpacePartition::GetClosestPoint_New(HalfSegment& hs, Point& p, Point& cp)
{

  assert( p.IsDefined() );
  Coord xl = hs.GetLeftPoint().GetX(),
        yl = hs.GetLeftPoint().GetY(),
        xr = hs.GetRightPoint().GetX(),
        yr = hs.GetRightPoint().GetY(),
        X = p.GetX(),
        Y = p.GetY();

  double result, auxresult;

  if( AlmostEqual(xl,xr) || AlmostEqual(yl ,yr) ){
    if( AlmostEqual(xl, xr)){ //hs is vertical
      if(((yl < Y || AlmostEqual(yl,Y)) && (Y < yr || AlmostEqual(Y, yr))) ||
          ((yr < Y || AlmostEqual(yr,Y)) && (Y < yl || AlmostEqual(Y, yl) ))){
        result = fabs( X - xl );
        cp.Set(xl, Y); //store the closest point
      }
      else{
        result = p.Distance(hs.GetLeftPoint());
        auxresult = p.Distance(hs.GetRightPoint());
        if( result > auxresult ){
          result = auxresult;
          cp = hs.GetRightPoint(); //store the closest point
        }else{
          cp = hs.GetLeftPoint();  //store the closest point
        }
      }
    }else{         //hs is horizontal line: (yl==yr)
//      if( xl <= X && X <= xr ){
      if( (xl < X || AlmostEqual(xl,X)) &&
          (X < xr || AlmostEqual(X, xr) ) ){
        result = fabs( Y - yl );
        cp.Set(X,yl);//store the closest point
      }else{
        result = p.Distance(hs.GetLeftPoint());
        auxresult = p.Distance(hs.GetRightPoint());
        if( result > auxresult ){
          result = auxresult;
          cp = hs.GetRightPoint();//store the closest point
        }else{
          cp = hs.GetLeftPoint();//store the closest point
        }
      }
    }
  }else{
    double k = (yr - yl) / (xr - xl),
           a = yl - k * xl,
           xx = (k * (Y - a) + X) / (k * k + 1),
           yy = k * xx + a;
    Coord XX = xx,
          YY = yy;
    Point PP( true, XX, YY );
//    if( xl <= XX && XX <= xr ){
    if( (xl < XX || AlmostEqual(xl, XX)) &&
        (XX < xr || AlmostEqual(XX, xr) ) ){
//      cout<<setprecision(16)<<XX<<" "<<YY<<endl;
      result = p.Distance( PP );
      cp = PP; //store the closest point
    }
    else
    {
      result = p.Distance( hs.GetLeftPoint() );
      auxresult = p.Distance( hs.GetRightPoint());
      if( result > auxresult ){
        result = auxresult;
        cp = hs.GetRightPoint();
      }else{
        cp = hs.GetLeftPoint();
      }
    }
  }
  return result;

}


/*
Decompose the pavement of one road into a set of subregions

*/

void SpacePartition::DecomposePavement1(Network* n, Relation* rel,
                                 int attr_pos1, int attr_pos2, int attr_pos3)
{
    vector<Region> paves1;
    vector<Region> paves2;
    vector<bool> route_flag;
    for(int i = 1;i <=  rel->GetNoTuples();i++){

      Tuple* pave_tuple = rel->GetTuple(i, false);
      Region* reg1 = (Region*)pave_tuple->GetAttribute(attr_pos2);
      Region* reg2 = (Region*)pave_tuple->GetAttribute(attr_pos3);
      paves1.push_back(*reg1);
      paves2.push_back(*reg2);
      pave_tuple->DeleteIfAllowed();
      route_flag.push_back(false);
    }


    vector<Region> pavements1;
    vector<Region> pavements2;

    int oid = 1;//object identifier

    assert(paves1.size() == paves2.size());
    for(int i = 1;i <=  rel->GetNoTuples();i++){

      Tuple* pave_tuple = rel->GetTuple(i, false);
      int rid = ((CcInt*)pave_tuple->GetAttribute(attr_pos1))->GetIntval();
//      cout<<"rid "<<rid<<endl;

/*      if(!(rid == 3116)){
          pave_tuple->DeleteIfAllowed();
          continue;
      }*/
//      cout<<"pave1 "<<paves1[rid - 1].NoComponents()<<endl;
//      cout<<"pave2 "<<paves2[rid - 1].NoComponents()<<endl;

      DecomposePave(&paves1[rid - 1], &paves2[rid - 1], pavements1);
      for(unsigned int j = 0;j < pavements1.size();j++){
          junid1.push_back(oid);
          junid2.push_back(rid);
          outer_regions1.push_back(pavements1[j]);
          oid++;
      }
      pave_tuple->DeleteIfAllowed();
      pavements1.clear();
    }
    ////////////check inside above//////////////////////////////
}


/*
Decompose the zebra crossings into a set of subregions

*/

void SpacePartition::DecomposePavement2(int start_oid, Relation* rel,
                                 int attr_pos1, int attr_pos2)
{
//    cout<<"start_oid "<<start_oid<<endl;
    int oid = start_oid + 1;//object identifier
    vector<Region> zc_regs;
    for(int i = 1;i <= rel->GetNoTuples();i++){

      Tuple* zc_tuple = rel->GetTuple(i, false);
      int rid = ((CcInt*)zc_tuple->GetAttribute(attr_pos1))->GetIntval();
      Region* zc_reg = (Region*)zc_tuple->GetAttribute(attr_pos2);
      Region* temp = new Region(0);
//      cout<<"rid "<<rid<<endl;
      DecomposePave(zc_reg, temp, zc_regs);
//      assert(zc_regs.size() > 0);
//      cout<<zc_regs.size()<<endl;
/*      for(unsigned int j = 0;j < zc_regs.size();j++){
          junid1.push_back(oid);
          junid2.push_back(rid);
          outer_regions1.push_back(zc_regs[j]);
          oid++;
      }*/

     //filter two zebra crossings are too close to each other
      const double delta_dist = 4.0;
      vector<Region> zc_regs_filter;
      for(unsigned int j1 = 0;j1 < zc_regs.size();j1++){
          unsigned int j2 = 0;
          for(;j2 < zc_regs_filter.size();j2++){
            if(zc_regs_filter[j2].Distance(zc_regs[j1]) < delta_dist)
              break;
          }
          if(j2 == zc_regs_filter.size())
            zc_regs_filter.push_back(zc_regs[j2]);
//          zc_regs_filter.push_back(zc_regs[j1]);
      }

      for(unsigned int j = 0;j < zc_regs_filter.size();j++){
          junid1.push_back(oid);
          junid2.push_back(rid);
          outer_regions1.push_back(zc_regs_filter[j]);
          oid++;
      }

      delete temp;
      zc_regs.clear();
      zc_tuple->DeleteIfAllowed();
    }
}

/*
get the commone line between two pavements (node in the graph model)

*/
void SpacePartition::GetPavementEdge1(Network* n, Relation* rel,
                                    BTree* btree_pave,
                                    int attr1, int attr2, int attr3)
{

    Relation* juns = n->GetJunctions();

    vector<Region_Oid> regs1;
    vector<Region_Oid> regs2;

    for(int i = 1;i <= n->GetNoJunctions();i++){

      Tuple* jun_tuple = juns->GetTuple(i, false);
      CcInt* rid1 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE1_ID);
      CcInt* rid2 = (CcInt*)jun_tuple->GetAttribute(JUNCTION_ROUTE2_ID);
      int id1 = rid1->GetIntval();
      int id2 = rid2->GetIntval();

/*      if(!(id1 == 3 && id2 == 6)){
          jun_tuple->DeleteIfAllowed();
          continue;
      }*/

//      cout<<"rid1 "<<id1<<" rid2 "<<id2<<endl;

      BTreeIterator* btreeiter1 = btree_pave->ExactMatch(rid1);
      while(btreeiter1->Next()){

        Tuple* pave_tup = rel->GetTuple(btreeiter1->GetId(), false);
        int oid = ((CcInt*)pave_tup->GetAttribute(attr1))->GetIntval();
        Region* pave = (Region*)pave_tup->GetAttribute(attr3);
        Region_Oid* ro = new Region_Oid(oid, *pave);
        regs1.push_back(*ro);
        delete ro;
        pave_tup->DeleteIfAllowed();
      }
      delete btreeiter1;

      BTreeIterator* btreeiter2 = btree_pave->ExactMatch(rid2);
      while(btreeiter2->Next()){

        Tuple* pave_tup = rel->GetTuple(btreeiter2->GetId(), false);
        int oid = ((CcInt*)pave_tup->GetAttribute(attr1))->GetIntval();
        Region* pave = (Region*)pave_tup->GetAttribute(attr3);
        Region_Oid* ro = new Region_Oid(oid, *pave);
        regs2.push_back(*ro);
        delete ro;
        pave_tup->DeleteIfAllowed();
      }
      delete btreeiter2;
//      cout<<regs1.size()<<" "<<regs2.size()<<endl;

      GetCommPave1(regs1, regs2, id1, id2);

      regs1.clear();
      regs2.clear();
      jun_tuple->DeleteIfAllowed();

    }

    juns->Delete();

}


/*
get the commone line between zc and pavement (node in the graph model)

*/
void SpacePartition::GetPavementEdge2(Relation* rel1,
                                    Relation* rel2, BTree* btree_pave,
                                    int attr1, int attr2, int attr3)
{
  vector<Region_Oid> reg_pave;

  for(int i = 1;i <= rel1->GetNoTuples();i++){

    Tuple* zc_tuple = rel1->GetTuple(i, false);
    CcInt* zc_oid = (CcInt*)zc_tuple->GetAttribute(attr1);
    CcInt* rid = (CcInt*)zc_tuple->GetAttribute(attr2);
    Region* reg = (Region*)zc_tuple->GetAttribute(attr3);


/*    if(!(rid->GetIntval() == 476)){
        zc_tuple->DeleteIfAllowed();
        continue;
    }*/

//    cout<<"oid "<<zc_oid->GetIntval()<<"rid "<<rid->GetIntval()<<endl;

    BTreeIterator* btreeiter = btree_pave->ExactMatch(rid);
    while(btreeiter->Next()){
        Tuple* pave_tuple = rel2->GetTuple(btreeiter->GetId(), false);
        int oid = ((CcInt*)pave_tuple->GetAttribute(attr1))->GetIntval();
        Region* pave = (Region*)pave_tuple->GetAttribute(attr3);
        Region_Oid* ro = new Region_Oid(oid, *pave);
        reg_pave.push_back(*ro);
        delete ro;
        pave_tuple->DeleteIfAllowed();
    }
    delete btreeiter;


    GetCommPave2(reg, zc_oid->GetIntval(),reg_pave);


    reg_pave.clear();
    zc_tuple->DeleteIfAllowed();
  }

}

/*
detect whether two pavements intersect, (pave1, pave2)

*/
void SpacePartition::GetCommPave1(vector<Region_Oid>& pave1,
                                 vector<Region_Oid>& pave2, int rid1,
                                 int rid2)
{
//  const double delta_dist = 0.01;
  const double delta_dist = 0.00001;
  for(unsigned int i = 0;i < pave1.size();i++){
      for(unsigned int j = 0;j < pave2.size();j++){

          if(pave1[i].reg.Inside(pave2[j].reg)){
              continue;
          }
          if(pave2[j].reg.Inside(pave1[i].reg)){
              continue;
          }
      //////////////////////////////////////////////////////////////
          if(MyRegIntersects(&pave1[i].reg, &pave2[j].reg)){

            vector<Point> common_ps;
            for(int index1 = 0; index1 < pave1[i].reg.Size();index1++){
                HalfSegment hs1;
                pave1[i].reg.Get(index1, hs1);
                if(!hs1.IsLeftDomPoint())continue;
                for(int index2 = 0;index2 < pave2[j].reg.Size();index2++){
                    HalfSegment hs2;
                    pave2[j].reg.Get(index2, hs2);
                    if(!hs2.IsLeftDomPoint())continue;
                    Point cp;
                    if(hs1.Intersection(hs2,cp)){
                      unsigned int index = 0;
                      for(;index < common_ps.size();index++){
                        if(cp.Distance(common_ps[index]) < delta_dist)
                          break;
                      }
                      if(index == common_ps.size())
                        common_ps.push_back(cp);
                    }
                }
            }
//            cout<<"oid1 "<<pave1[i].oid<<" oid2 "<<pave2[j].oid<<endl;
            assert(common_ps.size() > 1);
            if(common_ps.size() > 1){
                junid1.push_back(pave1[i].oid);
                junid2.push_back(pave2[j].oid);
                Line* l = new Line(0);
                pave_line1.push_back(*l);
                delete l;
            }
          }
      /////////////////////////////////////////////////////////////
      }
  }

}

/*
detect whether the zebra crossing intersects the pavement

*/
void SpacePartition::GetCommPave2(Region* reg, int oid,
                                  vector<Region_Oid>& pave2)
{
    for(unsigned int i = 0;i < pave2.size();i++){
        if(MyRegIntersects(reg, &pave2[i].reg)){
          Line* boundary = new Line(0);
          pave2[i].reg.Boundary(boundary);
          Line* result = new Line(0);
          boundary->Intersection(*reg, *result);
          if(result->Size() == 0){
            cout<<"zc oid1 "<<oid<<endl;
            cout<<"pave oid2 "<<pave2[i].oid<<endl;
          }
          assert(result->Size() > 0);
          if(result->Size() > 0){
              junid1.push_back(oid);
              junid2.push_back(pave2[i].oid);
              Line* l = new Line(0);
              pave_line1.push_back(*l);
              delete l;
          }
          delete result;
          delete boundary;
        }
    }
}

/*
doing triangulation for a polygon with and without hole

*/

CompTriangle::CompTriangle()
{
  count = 0;
  path = NULL;
  resulttype = NULL;
}
CompTriangle::CompTriangle(Region* r):reg(r),count(0),
path(NULL),resulttype(NULL)
{

}

CompTriangle:: ~CompTriangle()
{
  if(path != NULL) delete path;
  if(resulttype != NULL) delete resulttype;
}

/*
get the clockwise value 

*/
bool CompTriangle::GetClockwise(const vector<Point>& contour)
{
  float obj_area = Area(contour);
  if(0.0f < obj_area) return false;
  else return true; 
}

/*
Compute the area of a polygon. It is also used to compute the clockwise 
value of a list of points.
if area > 0.0f, counter-clockwise order
else clockwise order 
          
*/

float CompTriangle::Area(const vector<Point>& contour)
{
  int n = contour.size();

  float A=0.0f;

  for(int p=n-1,q=0; q<n; p=q++)
  {
    A+=
    contour[p].GetX()*contour[q].GetY() - contour[q].GetX()*contour[p].GetY();
  }
  return A*0.5f;

}

/*
  InsideTriangle decides if a point P is Inside of the triangle
  defined by A, B, C.
  If P equals to A, B or C, it returns true

*/
inline bool CompTriangle::InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py)

{
  float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
  float cCROSSap, bCROSScp, aCROSSbp;

  ax = Cx - Bx;  ay = Cy - By;
  bx = Ax - Cx;  by = Ay - Cy;
  cx = Bx - Ax;  cy = By - Ay;
  apx= Px - Ax;  apy= Py - Ay;
  bpx= Px - Bx;  bpy= Py - By;
  cpx= Px - Cx;  cpy= Py - Cy;

  aCROSSbp = ax*bpy - ay*bpx;
  cCROSSap = cx*apy - cy*apx;
  bCROSScp = bx*cpy - by*cpx;

  return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
};


bool CompTriangle::Snip(const vector<Point>& contour,int u,int v,int w,
                       int n,int *V)
{
  int p;
  float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

  Ax = contour[V[u]].GetX();
  Ay = contour[V[u]].GetY();

  Bx = contour[V[v]].GetX();
  By = contour[V[v]].GetY();

  Cx = contour[V[w]].GetX();
  Cy = contour[V[w]].GetY();

  if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) return false;

  for (p=0;p<n;p++)
  {
    if( (p == u) || (p == v) || (p == w) ) continue;
    Px = contour[V[p]].GetX();
    Py = contour[V[p]].GetY();
    if (InsideTriangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py)) return false;
  }

  return true;
}

/*
decompose a polygon into a set of triangles
Using the implementation by John W.Ratcliff

*/

bool CompTriangle::GetTriangles(const vector<Point>& contour,
                                vector<Point>& result)
{
    /* allocate and initialize list of Vertices in polygon */

  int n = contour.size();
  if ( n < 3 ) return false;

  int *V = new int[n];

  /* we want a counter-clockwise polygon in V */
//  cout<<"Area "<<Area(contour)<<endl;

  if ( 0.0f < Area(contour) )
    for (int v=0; v<n; v++) V[v] = v;
  else
    for(int v=0; v<n; v++) V[v] = (n-1)-v;

  int nv = n;

  /*  remove nv-2 Vertices, creating 1 triangle every time */
  int count = 2*nv;   /* error detection */

  for(int m=0, v=nv-1; nv>2; )
  {
    /* if we loop, it is probably a non-simple polygon */
    if (0 >= (count--))
    {
      //** Triangulate: ERROR - probable bad polygon!
      return false;
    }

    /* three consecutive vertices in current polygon, <u,v,w> */
    int u = v  ; if (nv <= u) u = 0;     /* previous */
    v = u+1; if (nv <= v) v = 0;     /* new v    */
    int w = v+1; if (nv <= w) w = 0;     /* next     */

    if ( Snip(contour,u,v,w,nv,V) )
    {
      int a,b,c,s,t;

      /* true names of the vertices */
      a = V[u]; b = V[v]; c = V[w];

      /* output Triangle */
      result.push_back( contour[a] );
      result.push_back( contour[b] );
      result.push_back( contour[c] );

      m++;

      /* remove v from remaining polygon */
      for(s=v,t=v+1;t<nv;s++,t++) V[s] = V[t]; nv--;

      /* resest error detection counter */
      count = 2*nv;
    }
  }

  delete []V;

  return true;
}

/*
Decompose the region into a set of triangles
it does not support polgyon with holes

*/
void CompTriangle::Triangulation()
{
  if(reg->NoComponents() == 0){
      cout<<"this is not a region"<<endl;
      return;
  }
  if(reg->NoComponents() > 1){
      cout<<"can't handle region with more than one faces"<<endl;
      return;
  }

  vector<int> no_cycles(reg->Size(), -1);
  for(int i = 0;i < reg->Size();i++){
      HalfSegment hs;
      reg->Get(i, hs);
      if(!hs.IsLeftDomPoint())continue;
      int cycleno = hs.attr.cycleno;
      no_cycles[cycleno] = cycleno;
  }

  unsigned int no_cyc = 0;
  for(unsigned int i = 0;i < no_cycles.size();i++){
    if(no_cycles[i] != -1) no_cyc++;
    else
      break;
  }

  if(no_cyc > 1){
    cout<<"polgyon has hole inside, please call NewTriangulation()"<<endl;
    return;
  }


  Line* boundary = new Line(0);
  reg->Boundary(boundary);
//  cout<<"boundary "<<*boundary<<endl;
  SimpleLine* sboundary = new SimpleLine(0);
  sboundary->fromLine(*boundary);
//  cout<<"sboundary size "<<sboundary->Size()<<endl;
  vector<MyHalfSegment> mhs;
  //get all the points of the region
  SpacePartition* sp = new SpacePartition();
  if(sboundary->Size() > 0)
    sp->ReorderLine(sboundary, mhs);
  else{
    cout<<"can't covert the boundary to a sline"<<endl;
    delete boundary;
    delete sboundary;
    return;
  }
  delete boundary;
  delete sboundary;

/*  for(unsigned int i = 0;i < mhs.size();i++)
        mhs[i].Print();*/

  vector<Point> ps;
  for(unsigned int i = 0;i < mhs.size();i++)
    ps.push_back(mhs[i].from);


/*  for(unsigned int i = 0;i < ps.size();i++){
    printf("%.8f %.8f\n",ps[i].GetX(), ps[i].GetY());
  }*/

  vector<Point> result;
  assert(GetTriangles(ps, result));

  unsigned int tcount = result.size()/3;

  for (unsigned int i=0; i<tcount; i++)
  {
     Point p1 = result[i*3+0];
     Point p2 = result[i*3+1];
     Point p3 = result[i*3+2];
/*     printf("Triangle %d => (%.5f,%.5f) (%.5f,%.5f) (%.5f,%.5f)\n", i + 1,
          p1.GetX(), p1.GetY(),
          p2.GetX(), p2.GetY(),
          p3.GetX(), p3.GetY());*/

    vector<Point> reg_ps;

    reg_ps.push_back(p1);
    reg_ps.push_back(p2);
    reg_ps.push_back(p3);

    sp->ComputeRegion(reg_ps, triangles);
  }

  delete sp;

}

bool CompTriangle::IsConvex(vector<Point> ps)
{
   int n = ps.size();
   int i,j,k;
   int flag = 0;
   double z;

   if (ps.size() < 3){
      cout<<"less than 3 points, it is not a region"<<endl;
      return false;
   }
   for (i=0;i<n;i++) {
      j = (i + 1) % n;
      k = (i + 2) % n;
//      z  = (p[j].x - p[i].x) * (p[k].y - p[j].y);
//      z -= (p[j].y - p[i].y) * (p[k].x - p[j].x);
      z  = (ps[j].GetX() - ps[i].GetX()) * (ps[k].GetY() - ps[j].GetY());
      z -= (ps[j].GetY() - ps[i].GetY()) * (ps[k].GetX() - ps[j].GetX());

      if (z < 0)
         flag |= 1;
      else if (z > 0)
         flag |= 2;
      if (flag == 3)
         return false;
   }
   if (flag != 0)
      return true;
   else
      return false;

}
/*
if the polygon is convex, returns true, otherwise (concave) returns false

*/
bool CompTriangle::PolygonConvex()
{
  if(reg->NoComponents() == 0){
      cout<<"error: this is not a region"<<endl;
      return false;
  }
  if(reg->NoComponents() > 1 || NoOfCycles() > 1){
      cout<<"error: there is hole inside or several subregions"<<endl;
      return false;
  }

  Line* boundary = new Line(0);
  reg->Boundary(boundary);
//  cout<<"boundary "<<*boundary<<endl;
  SimpleLine* sboundary = new SimpleLine(0);
  sboundary->fromLine(*boundary);
//  cout<<"sboundary size "<<sboundary->Size()<<endl;
  vector<MyHalfSegment> mhs;
  //get all the points of the region
  SpacePartition* sp = new SpacePartition();
  if(sboundary->Size() > 0)
    sp->ReorderLine(sboundary, mhs);
  else{
    cout<<"can't covert the boundary to a sline"<<endl;
    delete boundary;
    delete sboundary;
    delete sp;
    return false;
  }
  delete boundary;
  delete sboundary;
  delete sp;

/*  for(unsigned int i = 0;i < mhs.size();i++)
        mhs[i].Print();*/

  vector<Point> ps;
  for(unsigned int i = 0;i < mhs.size();i++)
    ps.push_back(mhs[i].from);
  ///////////////      convex/concave        /////////////////////////////
/*   int n = ps.size();
   int i,j,k;
   int flag = 0;
   double z;

   if (ps.size() < 3){
      cout<<"less than 3 points, it is not a region"<<endl;
      return false;
   }
   for (i=0;i<n;i++) {
      j = (i + 1) % n;
      k = (i + 2) % n;
//      z  = (p[j].x - p[i].x) * (p[k].y - p[j].y);
//      z -= (p[j].y - p[i].y) * (p[k].x - p[j].x);
      z  = (ps[j].GetX() - ps[i].GetX()) * (ps[k].GetY() - ps[j].GetY());
      z -= (ps[j].GetY() - ps[i].GetY()) * (ps[k].GetX() - ps[j].GetX());

      if (z < 0)
         flag |= 1;
      else if (z > 0)
         flag |= 2;
      if (flag == 3)
         return false;
   }
   if (flag != 0)
      return true;
   else
      return false;*/
  return IsConvex(ps);
  /////////////////////////////////////////////////////////////////////////

}

bool CompTriangle::PolygonConvex2(int& error)
{
  if(reg->NoComponents() == 0){
      cout<<"error: this is not a region"<<endl;
      error = 1;
      return false;
  }
  if(reg->NoComponents() > 1 || NoOfCycles() > 1){
      cout<<"error: there is hole inside or several subregions"<<endl;
      error = 2;
      return false;
  }

  Line* boundary = new Line(0);
  reg->Boundary(boundary);
//  cout<<"boundary "<<*boundary<<endl;
  SimpleLine* sboundary = new SimpleLine(0);
  sboundary->fromLine(*boundary);
//  cout<<"sboundary size "<<sboundary->Size()<<endl;
  vector<MyHalfSegment> mhs;
  //get all the points of the region
  SpacePartition* sp = new SpacePartition();
  if(sboundary->Size() > 0)
    sp->ReorderLine(sboundary, mhs);
  else{
    error = 3;
    cout<<"can't covert the boundary to a sline"<<endl;
    delete boundary;
    delete sboundary;
    return false;
  }
  delete boundary;
  delete sboundary;

/*  for(unsigned int i = 0;i < mhs.size();i++)
        mhs[i].Print();*/

  vector<Point> ps;
  for(unsigned int i = 0;i < mhs.size();i++)
    ps.push_back(mhs[i].from);
  ///////////////      convex/concave        /////////////////////////////
  error = 0;
  return IsConvex(ps);
  /////////////////////////////////////////////////////////////////////////

}

/*
number of vertices is larger than 100. 
it is a concave polgyon with holes inside 

if it is a complex region, when it needs to compute the shortest path inside.
  we do not call the simple method, but build the dual graph, visual graph and
  an auxiliary relation. 
  1:complex region 0 not complex region -1: error 

*/
int CompTriangle::ComplexRegion()
{
  int nocom = reg->NoComponents();
  if(nocom == 0){
      cout<<"error: this is not a region"<<endl;
      return -1;
  }
  if(nocom > 1){
      cout<<"error: several subregions"<<endl;
      return -1;
  }

  if(reg->Size() < 80) return 0; 

  if(reg->Size() > 150) return 1;//////////complex region 
  
  vector<int> no_cycles(reg->Size(), -1);

  for(int i = 0;i < reg->Size();i++){
      HalfSegment hs;
      reg->Get(i, hs);
      if(!hs.IsLeftDomPoint())continue;
      int cycleno = hs.attr.cycleno;
      no_cycles[cycleno] = cycleno;
  }

  unsigned int no_cyc = 0;
  for(unsigned int i = 0;i < no_cycles.size();i++){
    if(no_cycles[i] != -1) no_cyc++;
    else
      break;
  }
  if(no_cyc < 4) return 0;
  else return 1; /////////there are many holes inside 
}


/*
get a sequence of triangles where the shoretest path should pass through

*/
void CompTriangle::FindAdj(unsigned int index, vector<bool>& flag,
                           vector<int>& adj_list)
{
  vector<HalfSegment> cur_triangle;
  for(int i = 0;i < triangles[index].Size();i++){
    HalfSegment hs;
    triangles[index].Get(i, hs);
    if(!hs.IsLeftDomPoint())continue;
    cur_triangle.push_back(hs);
  }
  assert(cur_triangle.size() == 3);

  const double delta_dist = 0.00001;
  for(unsigned int i = 0;i < triangles.size();i++){
    if(flag[i] == true) continue;
    ///////////////////get the edges////////////////////////
    vector<HalfSegment> triangle;
    for(int j = 0;j < triangles[i].Size();j++){
      HalfSegment hs;
      triangles[i].Get(j, hs);
      if(!hs.IsLeftDomPoint())continue;
      triangle.push_back(hs);
    }
    assert(triangle.size() == 3);
    ////////////////////////////////////////////////////////////
    for(unsigned int k1 = 0;k1 < cur_triangle.size();k1++){
        Point p1 = cur_triangle[k1].GetLeftPoint();
        Point p2 = cur_triangle[k1].GetRightPoint();
        unsigned int k2 = 0;
      for(;k2 < triangle.size();k2++){
        Point p3 = triangle[k2].GetLeftPoint();
        Point p4 = triangle[k2].GetRightPoint();
        if(p1.Distance(p3) < delta_dist && p2.Distance(p4) < delta_dist){
          adj_list.push_back(i);
          flag[i] = true;
          break;
        }
        if(p1.Distance(p4) < delta_dist && p2.Distance(p3) < delta_dist){
          adj_list.push_back(i);
          flag[i] = true;
          break;
        }
      }
      if(k2 != triangle.size())break;
    }

  }

}

/*
get a sequence of triangles from the start point to the end point

*/
void CompTriangle::GetChannel(Point* start, Point* end)
{
  ////////// find the start triangle /////////////////////////
  int index1 = -1;
  int index2 = -1;
  for(unsigned int i = 0;i < triangles.size();i++){
      if(start->Inside(triangles[i])){
        index1 = i;
        break;
      }
  }
  ////////// find the end triangle /////////////////////////
  for(unsigned int i = 0;i < triangles.size();i++){
      if(end->Inside(triangles[i])){
        index2 = i;
        break;
      }
  }
  assert(index1 != -1 && index2 != -1);
  cout<<"index1 "<<index1<<" index2 "<<index2<<endl;
  vector<bool> triangle_flag;
  for(unsigned int i = 0;i < triangles.size();i++)
    triangle_flag.push_back(false);

  triangle_flag[index1] = true;

  ////////////////shortest path algorithm///////////////////////
  priority_queue<SPath_elem> path_queue;
  vector<SPath_elem> expand_path;

  path_queue.push(SPath_elem(-1, 0, index1, 1));
  expand_path.push_back(SPath_elem(-1,0, index1, 1));
  bool find = false;
  SPath_elem dest;//////////destination
  while(path_queue.empty() == false){
    SPath_elem top = path_queue.top();
    path_queue.pop();
//    top.Print();
    if(top.tri_index == index2){
       cout<<"find the path"<<endl;
       find = true;
       dest = top;
       break;
    }
   ////////find its adjacecy element, and push them into queue and path//////
    vector<int> adj_list;
    FindAdj(top.tri_index, triangle_flag, adj_list);
//    cout<<"adjcency_list size "<<adj_list.size()<<endl;
//    cout<<"expand_path_size "<<expand_path.size()<<endl;
    int pos_expand_path = top.cur_index;
    for(unsigned int i = 0;i < adj_list.size();i++){
      int expand_path_size = expand_path.size();
      path_queue.push(SPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.weight+1));
      expand_path.push_back(SPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.weight+1));
    }
  }
  ///////////////construct the path/////////////////////////////////
  if(find){
    vector<int> path_record;
    while(dest.prev_index != -1){
//      sleeve.push_back(triangles[dest.tri_index]);
      path_record.push_back(dest.tri_index);
      dest = expand_path[dest.prev_index];
    }
//    sleeve.push_back(triangles[dest.tri_index]);
    path_record.push_back(dest.tri_index);

    for(int i = path_record.size() - 1;i >= 0;i--)
      sleeve.push_back(triangles[path_record[i]]);
  }

}

/*
build the channel/funnel

*/
void CompTriangle::ConstructConvexChannel1(list<MyPoint>& funnel_front,
                              list<MyPoint>& funnel_back,
                              Point& newvertex,
                              vector<Point>& path, bool front_back)
{
//  cout<<"ConstructConvexChannel1 "<<endl;
  const double delta_dist = 0.00001;
  //push newvertex into funnel_back
  if(front_back){ //front = newvertex, check funnel_back first,
//    cout<<"front "<<"push into back "<<endl;
    //case1   case2 check another list
    MyPoint mp1;
    MyPoint mp2 = funnel_back.back();
    while(funnel_back.empty() == false){
        MyPoint elem = funnel_back.back();
//        elem.Print();
        HalfSegment hs;
        hs.Set(true, elem.loc, newvertex);
        if(reg->Contains(hs)){
          funnel_back.pop_back();
          mp1 = elem;
        }else break;
    }
    if(funnel_back.empty()){//case 1
//      cout<<"funnel_back empty"<<endl;
      funnel_back.push_back(mp1);
      funnel_back.push_back(MyPoint(newvertex,
                            newvertex.Distance(mp1.loc) + mp1.dist));
    }else{
      if(mp1.loc.Distance(mp2.loc) < delta_dist){ //case2
        double l1 = newvertex.Distance(mp2.loc) + mp2.dist;

        //////////////////////////////
        list<MyPoint>::iterator iter = funnel_front.begin();
        for(;iter != funnel_front.end();iter++){
          Point p = iter->loc;
          HalfSegment hs;
          hs.Set(true, p, newvertex);
          if(reg->Contains(hs))break;
        }
        double l2 = newvertex.Distance(iter->loc) + iter->dist;
        ///////////////////////////////
//        cout<<"l1 "<<l1<<" l2 "<<l2<<endl;
        if(l1 < l2 || AlmostEqual(l1, l2)){
          funnel_back.push_back(mp2);
          funnel_back.push_back(MyPoint(newvertex,l1));
        }else{
            while(funnel_front.empty() == false){
              MyPoint elem = funnel_front.front();
//              elem.Print();
              HalfSegment hs;
              hs.Set(true, elem.loc, newvertex);
              if(reg->Contains(hs))break;
              funnel_front.pop_front();
              if(path[path.size() - 1].Distance(elem.loc) > delta_dist)
                  path.push_back(elem.loc);
            }
            assert(funnel_front.empty() == false);
            MyPoint top = funnel_front.front();
            path.push_back(top.loc);


            //update funnel_back
            funnel_back.clear();
            funnel_back.push_back(top);
            funnel_back.push_back(MyPoint(newvertex,
                       newvertex.Distance(top.loc) + top.dist));
        }
      }else{//case 1,find a point in funnel_back directly connected to newvertex
        funnel_back.push_back(mp1);
        funnel_back.push_back(MyPoint(newvertex,
                            newvertex.Distance(mp1.loc) + mp1.dist));
      }
    }
  //push newvertex into funnel_front
  }else{ //back = newvertex, check funnel funnel_front first
//  cout<<"back "<<"push into front "<<endl;
 //case1   case2 check another list
    MyPoint mp1;
    MyPoint mp2 = funnel_front.back();
    while(funnel_front.empty() == false){
        MyPoint elem = funnel_front.back();
//        elem.Print();
        HalfSegment hs;
        hs.Set(true, elem.loc, newvertex);
        if(reg->Contains(hs)){
          funnel_front.pop_back();
          mp1 = elem;
        }else break;
    }
    if(funnel_front.empty()){//case 1
//      cout<<"funnel_front empty"<<endl;
      funnel_front.push_back(mp1);
      funnel_front.push_back(MyPoint(newvertex,
                            newvertex.Distance(mp1.loc) + mp1.dist));
    }else{
      if(mp1.loc.Distance(mp2.loc) < delta_dist){ //case2
        double l1 = newvertex.Distance(mp2.loc) + mp2.dist;
        ////////////////////////////////////////////////
        list<MyPoint>::iterator iter = funnel_back.begin();
        for(;iter != funnel_back.end();iter++){
          Point p = iter->loc;
          HalfSegment hs;
          hs.Set(true, p, newvertex);
          if(reg->Contains(hs))break;
        }
        double l2 = newvertex.Distance(iter->loc) + iter->dist;
        /////////////////////////////////////////////////

//        cout<<"l1 "<<l1<<" l2 "<<l2<<endl;
        if(l1 < l2 || AlmostEqual(l1, l2)){
          funnel_front.push_back(mp2);
          funnel_front.push_back(MyPoint(newvertex,l1));
        }
        else{
            while(funnel_back.empty() == false){
              MyPoint elem = funnel_back.front();
              HalfSegment hs;
              hs.Set(true, elem.loc, newvertex);
              if(reg->Contains(hs))break;
              funnel_back.pop_front();
              if(path[path.size() - 1].Distance(elem.loc) > delta_dist)
                  path.push_back(elem.loc);
            }
            assert(funnel_back.empty() == false);
            MyPoint top = funnel_back.front();
            path.push_back(top.loc);


            //update funnel_front
            funnel_front.clear();
            funnel_front.push_back(top);
            funnel_front.push_back(MyPoint(newvertex,
                       newvertex.Distance(top.loc) + top.dist));
        }
      }else{//case 1,find a point in funnel_back directly connected to newvertex
        funnel_front.push_back(mp1);
        funnel_front.push_back(MyPoint(newvertex,
                            newvertex.Distance(mp1.loc) + mp1.dist));
      }
    }

  }

/*  list<MyPoint>::iterator iter = funnel_front.begin();
  cout<<"front ";
  for(;iter != funnel_front.end();iter++){
        Point p = iter->loc;
        cout<<p<<" ";
  }
  cout<<endl<<"back ";
  iter = funnel_back.begin();
  for(;iter != funnel_back.end();iter++){
        Point p = iter->loc;
        cout<<p<<" ";
  }
  cout<<endl;
  cout<<"points in path "<<endl;
  for(unsigned int i = 0;i < path.size();i++)
      cout<<path[i]<<" ";
  cout<<endl;*/

}

/*
complete the final shortest path

*/

void CompTriangle::ConstructConvexChannel2(list<MyPoint> funnel_front,
                              list<MyPoint> funnel_back,
                              Point& newvertex,
                              vector<Point>& path, bool front_back)
{
//  cout<<"ConstructConvexChannel2 "<<endl;
  const double delta_dist = 0.00001;
  //push newvertex into funnel_back
  if(front_back){ //front = newvertex, check funnel_back first,
//    cout<<"front "<<"push into back "<<endl;
    //case1   case2 check another list

    MyPoint mp1 = funnel_back.back();
    MyPoint mp2 = funnel_back.front();
    while(funnel_back.empty() == false){
        MyPoint elem = funnel_back.back();
//        elem.Print();
        HalfSegment hs;

        if(elem.loc.Distance(newvertex) < delta_dist){
            funnel_back.pop_back();
            mp1 = elem;
            continue;
        }

        hs.Set(true, elem.loc, newvertex);
        if(reg->Contains(hs)){
          funnel_back.pop_back();
          mp1 = elem;
        }else break;
    }

      double l1 = newvertex.Distance(mp1.loc) + mp1.dist;
        ////////////////////////////////////////////////
        list<MyPoint>::iterator iter = funnel_front.begin();
        for(;iter != funnel_front.end();iter++){
          Point p = iter->loc;
          HalfSegment hs;

          if(p.Distance(newvertex) < delta_dist) break;

          hs.Set(true, p, newvertex);
          if(reg->Contains(hs))break;
        }
        double l2 = newvertex.Distance(iter->loc) + iter->dist;
        /////////////////////////////////////////////////

//      cout<<"l1 "<<l1<<" l2 "<<l2<<endl;
      if(l1 < l2 || AlmostEqual(l1, l2)){

        if(funnel_back.empty() == false)
          funnel_back.pop_front();

        while(funnel_back.empty() == false){
          MyPoint elem = funnel_back.front();
          path.push_back(elem.loc);
          funnel_back.pop_front();
        }
        if(mp1.loc.Distance(mp2.loc) > delta_dist)
          path.push_back(mp1.loc);
      }else{
          MyPoint top = funnel_front.front();

          if(funnel_front.empty() == false)
            funnel_front.pop_front();

          HalfSegment hs;

          if(top.loc.Distance(newvertex) < delta_dist) return;

          hs.Set(true, top.loc, newvertex);
          if(reg->Contains(hs)) return;

          while(funnel_front.empty() == false){
            MyPoint elem = funnel_front.front();
            path.push_back(elem.loc);
            ////////////////////////////////////////
            HalfSegment hs;

            if(elem.loc.Distance(newvertex) < delta_dist) break;

            hs.Set(true, elem.loc, newvertex);
            if(reg->Contains(hs))break;
            ///////////////////////////////////////
            funnel_front.pop_front();
          }
      }
    //push newvertex into funnel_front
  }else{ //back = newvertex, check funnel funnel_front first
//  cout<<"back "<<"push into front "<<endl;
 //case1   case2 check another list

    MyPoint mp1 = funnel_front.back();
    MyPoint mp2 = funnel_front.front();
    while(funnel_front.empty() == false){
        MyPoint elem = funnel_front.back();
//        elem.Print();
        HalfSegment hs;

        if(elem.loc.Distance(newvertex) < delta_dist){
          funnel_front.pop_back();
          mp1 = elem;
          continue;
        }

        hs.Set(true, elem.loc, newvertex);
        if(reg->Contains(hs)){
          funnel_front.pop_back();
          mp1 = elem;
        }else break;
    }

        double l1 = newvertex.Distance(mp1.loc) + mp1.dist;
        /////////////////////////////////////////////
        list<MyPoint>::iterator iter = funnel_back.begin();
        for(;iter != funnel_back.end();iter++){
          Point p = iter->loc;
          HalfSegment hs;

          if(p.Distance(newvertex) < delta_dist){
            break;
          }

          hs.Set(true, p, newvertex);
          if(reg->Contains(hs))break;
        }
        double l2 = newvertex.Distance(iter->loc) + iter->dist;
        ////////////////////////////////////////////

//        cout<<"l1 "<<l1<<" l2 "<<l2<<endl;
        if(l1 < l2 || AlmostEqual(l1, l2)){
            if(funnel_front.empty() == false)
                funnel_front.pop_front();
            while(funnel_front.empty() == false){
              MyPoint elem = funnel_front.front();
              path.push_back(elem.loc);
              funnel_front.pop_front();
            }
            if(mp1.loc.Distance(mp2.loc) > delta_dist)
                path.push_back(mp1.loc);
        }
        else{
            MyPoint top = funnel_back.front();
            if(funnel_back.empty() == false){
                funnel_back.pop_front();
            }

            HalfSegment hs;

            if(top.loc.Distance(newvertex) < delta_dist)return;

            hs.Set(true, top.loc, newvertex);
            if(reg->Contains(hs)) return;

            while(funnel_back.empty() == false){
              MyPoint elem = funnel_back.front();
              path.push_back(elem.loc);
              /////////////////////////////
              HalfSegment hs;
              if(elem.loc.Distance(newvertex) < delta_dist)break;

              hs.Set(true, elem.loc, newvertex);
              if(reg->Contains(hs))break;
              //////////////////////////////
              funnel_back.pop_front();
            }
        }
  }

}

/*
geometrical shortest path in a polygon
Apply the Funnel Algorithm, front point---->point
The polygon should not have hole inside. !!!
If it has hole inside, it has to check every possible path.

*/
void CompTriangle::GeoShortestPath(Point* start, Point* end)
{

    if(start->Inside(*reg) == false || end->Inside(*reg) == false){
      cout<<"points are not inside the polygon"<<endl;
      return;
    }
    if(AlmostEqual(start->GetX(), end->GetX()) &&
       AlmostEqual(start->GetY(), end->GetY())){
        cout<<"two points are equal"<<endl;
        return;
    }

    if(PolygonConvex()){ //convex, just use euclidean distance
//      cout<<"a convex polygon"<<endl;
      int edgeno = 0;
      path = new Line(0);
      path->StartBulkLoad();
      HalfSegment hs;
      hs.Set(true, *start, *end);
      hs.attr.edgeno = edgeno++;
      *path += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *path += hs;
      path->EndBulkLoad();
      sleeve.push_back(*reg);
      return;
    }

//    Triangulation();//get a set of triangles, does not support hole
    NewTriangulation();
    if(triangles.size() < 3){
      cout<<"triangulation is not correct"<<endl;
      return;
    }
    ///////////////  find the channel /////////////////////////////

    GetChannel(start, end);
    /////////////////get a sequence of diagonals////////////////////////
    vector<HalfSegment> diagonals;
    const double delta_dist = 0.00001;
//    cout<<"channel size "<<sleeve.size()<<endl;
    for(unsigned int i = 0;i < sleeve.size() - 1;i++){
        vector<HalfSegment> cur_triangle;
        for(int j = 0;j < sleeve[i].Size();j++){
          HalfSegment hs;
          sleeve[i].Get(j, hs);
            if(!hs.IsLeftDomPoint())continue;
          cur_triangle.push_back(hs);
        }

        vector<HalfSegment> next_triangle;
        for(int j = 0;j < sleeve[i + 1].Size();j++){
          HalfSegment hs;
          sleeve[i + 1].Get(j, hs);
            if(!hs.IsLeftDomPoint())continue;
          next_triangle.push_back(hs);
        }
        for(unsigned int k1 = 0;k1 < cur_triangle.size();k1++){
            Point p1 = cur_triangle[k1].GetLeftPoint();
            Point p2 = cur_triangle[k1].GetRightPoint();

            unsigned int k2 = 0;
            for(;k2 < next_triangle.size();k2++){
              Point p3 = next_triangle[k2].GetLeftPoint();
              Point p4 = next_triangle[k2].GetRightPoint();

              if(p1.Distance(p3) < delta_dist && p2.Distance(p4) < delta_dist){
                HalfSegment hs;
                hs.Set(true,p1, p2);
                diagonals.push_back(hs);
                break;
              }
              if(p1.Distance(p4) < delta_dist && p2.Distance(p3) < delta_dist){
                HalfSegment hs;
                hs.Set(true,p1, p2);
                diagonals.push_back(hs);
                break;
              }
            }
            if(k2 != next_triangle.size())break;
        }
    }
//    cout<<"diagnoals size "<<diagonals.size()<<endl;

    list<MyPoint> funnel_front;
    list<MyPoint> funnel_back;
    Point apex = *start;
    vector<Point> shortest_path;

    shortest_path.push_back(apex);
    funnel_front.push_back(MyPoint(apex, 0.0));
    funnel_back.push_back(MyPoint(apex, 0.0));

    bool funnel_front_flag = true;

    for(unsigned int i = 0;i < diagonals.size();i++){
      Point lp = diagonals[i].GetLeftPoint();
      Point rp = diagonals[i].GetRightPoint();
      if(i == 0){
        funnel_front.push_back(MyPoint(lp, lp.Distance(apex)));
        funnel_back.push_back(MyPoint(rp, rp.Distance(apex)));
        continue;
      }
      Point last_lp = diagonals[i - 1].GetLeftPoint();
      Point last_rp = diagonals[i - 1].GetRightPoint();
      Point newvertex;
      if(lp.Distance(last_lp) < delta_dist ||
         lp.Distance(last_rp) < delta_dist){
          newvertex = rp;

//          cout<<"newvertex rp" <<newvertex<<endl;
          MyPoint front = funnel_front.back();
          MyPoint back = funnel_back.back();


          if(front.loc.Distance(lp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, true);
            funnel_front_flag = true;
          }else if(back.loc.Distance(lp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, false);
            funnel_front_flag = false;
          }else assert(false);
      }
      else if(rp.Distance(last_lp) < delta_dist ||
              rp.Distance(last_rp) < delta_dist){
              newvertex = lp;

//          cout<<"newvertex lp" <<newvertex<<endl;
          MyPoint front = funnel_front.back();
          MyPoint back = funnel_back.back();


          if(front.loc.Distance(rp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, true);
            funnel_front_flag = true;
          }else if(back.loc.Distance(rp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, false);
            funnel_front_flag = false;
          }else assert(false);
      }
      else assert(false);
    }

/*    for(unsigned int i = 0;i < shortest_path.size();i++)
            cout<<"point in path "<<shortest_path[i]<<endl;*/

    ////////////////// last point //////////////////////////////
    ConstructConvexChannel2(funnel_front, funnel_back,
                                 *end, shortest_path, funnel_front_flag);
    shortest_path.push_back(*end);
    ////////////////////////////////////////////////////////////
//    cout<<"shortest path segments size "<<shortest_path.size()<<endl;
    path = new Line(0);
    path->StartBulkLoad();
    int edgeno = 0;
    for(unsigned int i = 0;i < shortest_path.size() - 1;i++){
//      cout<<"point1 "<<shortest_path[i]<<endl;
//      cout<<"point2 "<<shortest_path[i + 1]<<endl;
      HalfSegment hs;
      Point p1 = shortest_path[i];
      Point p2 = shortest_path[i + 1];
      hs.Set(true, p1, p2);
      hs.attr.edgeno = edgeno++;
      *path += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *path += hs;
    }
    path->EndBulkLoad();
}

/*
select the point on segment to compute the shortest path.
it goes through the funnel to check the vertex stored and try to find
sucn an segment which is contained by the polygon and it consists of
the vertex and the closest point to the vertex in the input segement.

*/
void CompTriangle::SelectPointOnSeg(list<MyPoint> funnel_front,
                        list<MyPoint> funnel_back, HalfSegment* end,
                        Point& vertex, Point& cp)

{
  SpacePartition* sp = new SpacePartition();
  const double delta_dist = 0.00001;
  MyPoint elem_front = funnel_front.back();
  MyPoint elem_back = funnel_back.back();
  if(elem_front.loc.Distance(vertex) < delta_dist){
    sp->GetClosestPoint(*end, elem_front.loc, cp);
    funnel_front.pop_back();
    while(funnel_front.empty() == false){
        MyPoint elem = funnel_front.back();
        Point p;
        sp->GetClosestPoint(*end, elem.loc, p);
        HalfSegment hs;
        hs.Set(true, elem.loc, p);
        if(reg->Contains(hs) == false)break;
        cp = p;
        funnel_front.pop_back();
    }
  }else if(elem_back.loc.Distance(vertex) < delta_dist){
    sp->GetClosestPoint(*end, elem_back.loc, cp);
    funnel_back.pop_back();
    while(funnel_back.empty() == false){
        MyPoint elem = funnel_back.back();
        Point p;
        sp->GetClosestPoint(*end, elem.loc, p);
        HalfSegment hs;
        hs.Set(true, elem.loc, p);
        if(reg->Contains(hs) == false)break;
        cp = p;
        funnel_back.pop_back();
    }
  }else assert(false);

  delete sp;
}

/*
compute the shortest path from start point to the end segment

*/
void CompTriangle::PtoSegSPath(Point* start, HalfSegment* end,
                              vector<Region>& temp_sleeve, Line* res_line)
{

  /////////////////get the channel////////////////////////////
  vector<HalfSegment> diagonals;
  const double delta_dist = 0.00001;

    for(unsigned int i = 0;i < temp_sleeve.size() - 1;i++){
        vector<HalfSegment> cur_triangle;
        for(int j = 0;j < temp_sleeve[i].Size();j++){
          HalfSegment hs;
          temp_sleeve[i].Get(j, hs);
            if(!hs.IsLeftDomPoint())continue;
          cur_triangle.push_back(hs);
        }

        vector<HalfSegment> next_triangle;
        for(int j = 0;j < temp_sleeve[i + 1].Size();j++){
          HalfSegment hs;
          temp_sleeve[i + 1].Get(j, hs);
            if(!hs.IsLeftDomPoint())continue;
          next_triangle.push_back(hs);
        }
        for(unsigned int k1 = 0;k1 < cur_triangle.size();k1++){
            Point p1 = cur_triangle[k1].GetLeftPoint();
            Point p2 = cur_triangle[k1].GetRightPoint();

            unsigned int k2 = 0;
            for(;k2 < next_triangle.size();k2++){
              Point p3 = next_triangle[k2].GetLeftPoint();
              Point p4 = next_triangle[k2].GetRightPoint();

              if(p1.Distance(p3) < delta_dist && p2.Distance(p4) < delta_dist){
                HalfSegment hs;
                hs.Set(true,p1, p2);
                diagonals.push_back(hs);
                break;
              }
              if(p1.Distance(p4) < delta_dist && p2.Distance(p3) < delta_dist){
                HalfSegment hs;
                hs.Set(true,p1, p2);
                diagonals.push_back(hs);
                break;
              }
            }
            if(k2 != next_triangle.size())break;
        }
    }
//    cout<<"diagnoals size "<<diagonals.size()<<endl;

 ///////////////////////////////////////////////////////////////

    list<MyPoint> funnel_front;
    list<MyPoint> funnel_back;
    Point apex = *start;
    vector<Point> shortest_path;

    shortest_path.push_back(apex);
    funnel_front.push_back(MyPoint(apex, 0.0));
    funnel_back.push_back(MyPoint(apex, 0.0));

    bool funnel_front_flag = true;
    Point newvertex;
    Point newvertex_pair;
    for(unsigned int i = 0;i < diagonals.size();i++){
      Point lp = diagonals[i].GetLeftPoint();
      Point rp = diagonals[i].GetRightPoint();
      if(i == 0){
        funnel_front.push_back(MyPoint(lp, lp.Distance(apex)));
        funnel_back.push_back(MyPoint(rp, rp.Distance(apex)));
        continue;
      }
      Point last_lp = diagonals[i - 1].GetLeftPoint();
      Point last_rp = diagonals[i - 1].GetRightPoint();

      if(lp.Distance(last_lp) < delta_dist ||
         lp.Distance(last_rp) < delta_dist){
          newvertex = rp;
          newvertex_pair = lp;

//          cout<<"newvertex rp" <<newvertex<<endl;
          MyPoint front = funnel_front.back();
          MyPoint back = funnel_back.back();

          if(front.loc.Distance(lp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, true);
            funnel_front_flag = true;
          }else if(back.loc.Distance(lp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, false);
            funnel_front_flag = false;
          }else assert(false);
      }
      else if(rp.Distance(last_lp) < delta_dist ||
              rp.Distance(last_rp) < delta_dist){
              newvertex = lp;
              newvertex_pair = rp;

//          cout<<"newvertex lp" <<newvertex<<endl;
          MyPoint front = funnel_front.back();
          MyPoint back = funnel_back.back();


          if(front.loc.Distance(rp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, true);
            funnel_front_flag = true;
          }else if(back.loc.Distance(rp) < delta_dist){
            ConstructConvexChannel1(funnel_front, funnel_back,
                                 newvertex, shortest_path, false);
            funnel_front_flag = false;
          }else assert(false);
      }
      else assert(false);
    }
  //////////////////////////////////////////////////////////////
//  cout<<"end halfsegment "<<*end<<endl;
  vector<Point> shortest_path_another;
  for(unsigned int i = 0;i < shortest_path.size();i++)
      shortest_path_another.push_back(shortest_path[i]);

//  cout<<"newvertex "<<newvertex<<" cp1 "<<cp1<<endl;
  ////////////////////////////////////////////
  list<MyPoint> copy_funnel_front;
  list<MyPoint> copy_funnel_back;
  list<MyPoint>::iterator iter;
  for(iter = funnel_front.begin(); iter != funnel_front.end();iter++)
    copy_funnel_front.push_back(*iter);
  for(iter = funnel_back.begin(); iter != funnel_back.end();iter++)
    copy_funnel_back.push_back(*iter);

  Point cp1, cp2;

  //////////////////////////////////////////////////////////////////

  SelectPointOnSeg(funnel_front, funnel_back, end, newvertex, cp1);

  if(newvertex.Distance(cp1) > delta_dist){
    ConstructConvexChannel2(funnel_front, funnel_back,
                                 cp1, shortest_path, funnel_front_flag);
    shortest_path.push_back(cp1);
  }else{
    MyPoint top_front = funnel_front.back();
    MyPoint top_back = funnel_back.back();
    if(top_front.loc.Distance(cp1) < delta_dist){
      while(funnel_front.size() > 1){
        MyPoint elem = funnel_front.back();
        funnel_front.pop_back();
        shortest_path.push_back(elem.loc);
      }
    }else if(top_back.loc.Distance(cp1) < delta_dist){
      while(funnel_back.size() > 1){
        MyPoint elem = funnel_back.back();
        funnel_back.pop_back();
        shortest_path.push_back(elem.loc);
      }
    }else assert(false);
  }
  ///////////////////////////////////////////////////////////////////////
//  cout<<"path1 size "<<shortest_path.size()<<endl;

  SelectPointOnSeg(funnel_front, funnel_back, end, newvertex_pair, cp2);

//  cout<<"newvertex_pair "<<newvertex_pair<<" cp2 "<<cp2<<endl;
  if(newvertex_pair.Distance(cp2) > delta_dist){
    ConstructConvexChannel2(copy_funnel_front, copy_funnel_back,
                                 cp2, shortest_path_another,
                                 !funnel_front_flag);
    shortest_path_another.push_back(cp2);
  }else{
      MyPoint top_front = copy_funnel_front.back();
      MyPoint top_back = copy_funnel_back.back();
      if(top_front.loc.Distance(cp2) < delta_dist){
        while(copy_funnel_front.size() > 1){
          MyPoint elem = copy_funnel_front.back();
          copy_funnel_front.pop_back();
          shortest_path_another.push_back(elem.loc);
        }
      }else if(top_back.loc.Distance(cp2) < delta_dist){
          while(copy_funnel_back.size() > 1){
            MyPoint elem = copy_funnel_back.back();
            copy_funnel_back.pop_back();
            shortest_path_another.push_back(elem.loc);
          }
      }else assert(false);
  }

//  cout<<"path2 size "<<shortest_path_another.size()<<endl;

  //////////////////////////////////////////////////////////////
  Line* l1 = new Line(0);

  l1->StartBulkLoad();


  int edgeno1 = 0;
  for(unsigned int i = 0;i < shortest_path.size() - 1;i++){
      HalfSegment hs;
      Point p1 = shortest_path[i];
      Point p2 = shortest_path[i + 1];
      hs.Set(true, p1, p2);
      hs.attr.edgeno = edgeno1++;
      *l1 += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *l1 += hs;
  }
  l1->EndBulkLoad();

  Line* l2 = new Line(0);
  l2->StartBulkLoad();
  int edgeno2 = 0;
  for(unsigned int i = 0;i < shortest_path_another.size() - 1;i++){
      HalfSegment hs;
      Point p1 = shortest_path_another[i];
      Point p2 = shortest_path_another[i + 1];
      hs.Set(true, p1, p2);
      hs.attr.edgeno = edgeno2++;
      *l2 += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *l2 += hs;
  }
  l2->EndBulkLoad();
  if(l1->Length() < l2->Length())
    *res_line = *l1;
  else
    *res_line = *l2;

  delete l1;
  delete l2;
}


/*The following describes a method for determining whether or not a polygon has
its vertices ordered clockwise or anticlockwise for both convex and concave
polygons. A polygon will be assumed to be described by N vertices, ordered
(x0,y0), (x1,y1), (x2,y2), . . . (xn-1,yn-1)

A simple test of vertex ordering for convex polygons is based on considerations
of the cross product between adjacent edges. If the crossproduct is positive
then it rises above the plane (z axis up out of the plane) and if negative then
the cross product is into the plane.

cross product = ((xi - xi-1),(yi - yi-1)) x ((xi+1 - xi),(yi+1 - yi))
= (xi - xi-1) * (yi+1 - yi) - (yi - yi-1) * (xi+1 - xi)

A positive cross product means we have a counterclockwise polygon.

To determine the vertex ordering for concave polygons one can use a result
from the calculation of polygon areas, where the area is given by

A = 1/2*Sum(for (i=0;i<N-1;i++) (xiyi+1 - xi+1yi))

If the above expression is positive then the polygon is ordered counter
clockwise otherwise if it is negative then the polygon vertices are ordered
clockwise.*/

/*
calculate the number of cycles in a region

*/
unsigned int CompTriangle::NoOfCycles()
{
  vector<int> no_cycles(reg->Size(), -1);

  for(int i = 0;i < reg->Size();i++){
      HalfSegment hs;
      reg->Get(i, hs);
      if(!hs.IsLeftDomPoint())continue;
      int cycleno = hs.attr.cycleno;
      no_cycles[cycleno] = cycleno;
  }

  unsigned int no_cyc = 0;
  for(unsigned int i = 0;i < no_cycles.size();i++){
    if(no_cycles[i] != -1) no_cyc++;
    else
      break;
  }
  return no_cyc;
}
/*
Initialize the point list

*/

void CompTriangle::PolygonContourPoint(unsigned int no_cyc, int no_p_contour[],
                           vector<double>& ps_contour_x,
                           vector<double>& ps_contour_y)
{
  ps_contour_x.push_back(0.0);
  ps_contour_y.push_back(0.0);

  vector<SimpleLine*> sl_contour;

  for(unsigned int i = 0;i < no_cyc;i++){
      SimpleLine* sl = new SimpleLine(0);
      sl->StartBulkLoad();
      sl_contour.push_back(sl);
  }
  vector<int> edgenos(no_cyc, 0);
  for(int j = 0;j < reg->Size();j++){
    HalfSegment hs1;
    reg->Get(j, hs1);
    if(!hs1.IsLeftDomPoint()) continue;
    HalfSegment hs2;
    hs2.Set(true, hs1.GetLeftPoint(), hs1.GetRightPoint());

    hs2.attr.edgeno = edgenos[hs1.attr.cycleno]++;
    *sl_contour[hs1.attr.cycleno] += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *sl_contour[hs1.attr.cycleno] += hs2;

  }
//  cout<<"get all boundary line"<<endl;
  SpacePartition* sp = new SpacePartition();
  for(unsigned int i = 0;i < no_cyc;i++){
      sl_contour[i]->EndBulkLoad();
      vector<MyHalfSegment> mhs;
      sp->ReorderLine(sl_contour[i], mhs);
      vector<Point> ps;
      for(unsigned int j = 0;j < mhs.size();j++)
        ps.push_back(mhs[j].from);

      bool clock;
      if(0.0f < Area(ps)){//points counter-clockwise order
        clock = false;
      }else{// points clockwise
        clock = true;
      }
      no_p_contour[i] = ps.size();
      if(i == 0){//outer contour, counter_clockwise
        if(clock == false){
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[index].GetX());
                ps_contour_y.push_back(ps[index].GetY());
            }
        }else{
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[ps.size() - 1 - index].GetX());
                ps_contour_y.push_back(ps[ps.size() - 1 - index].GetY());
            }
        }
      }else{//hole points, should be clockwise
        if(clock == false){
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[ps.size() -1 - index].GetX());
                ps_contour_y.push_back(ps[ps.size() -1 - index].GetY());
            }
        }else{
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[index].GetX());
                ps_contour_y.push_back(ps[index].GetY());
            }
        }
      }

      delete sl_contour[i];
  }
  delete sp;

}

/*
the same as PolygonContourPoint, but do not insert (0,0) for the first point
for each cycle, it inserts the first point to the end to close the region

*/

void CompTriangle::PolygonContourPoint2(unsigned int no_cyc, int no_p_contour[],
                           vector<double>& ps_contour_x,
                           vector<double>& ps_contour_y)
{
  
  vector<SimpleLine*> sl_contour;

  for(unsigned int i = 0;i < no_cyc;i++){
      SimpleLine* sl = new SimpleLine(0);
      sl->StartBulkLoad();
      sl_contour.push_back(sl);
  }
  vector<int> edgenos(no_cyc, 0);
  for(int j = 0;j < reg->Size();j++){
    HalfSegment hs1;
    reg->Get(j, hs1);
    if(!hs1.IsLeftDomPoint()) continue;
    HalfSegment hs2;
    hs2.Set(true, hs1.GetLeftPoint(), hs1.GetRightPoint());

    hs2.attr.edgeno = edgenos[hs1.attr.cycleno]++;
    *sl_contour[hs1.attr.cycleno] += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *sl_contour[hs1.attr.cycleno] += hs2;
  }
//  cout<<"get all boundary line"<<endl;
  SpacePartition* sp = new SpacePartition();
  for(unsigned int i = 0;i < no_cyc;i++){
      sl_contour[i]->EndBulkLoad();
      vector<MyHalfSegment> mhs;
      sp->ReorderLine(sl_contour[i], mhs);
      vector<Point> ps;
      for(unsigned int j = 0;j < mhs.size();j++)
        ps.push_back(mhs[j].from);

      bool clock;
      if(0.0f < Area(ps)){//points counter-clockwise order
        clock = false;
      }else{// points clockwise
        clock = true;
      }
      no_p_contour[i] = ps.size();
      if(i == 0){//outer contour, counter_clockwise
        if(clock == false){
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[index].GetX());
                ps_contour_y.push_back(ps[index].GetY());
            }
            //////////insert the start point again to close the region///////
//             unsigned int index = 0;
//             ps_contour_x.push_back(ps[index].GetX());
//             ps_contour_y.push_back(ps[index].GetY());

        }else{
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[ps.size() - 1 - index].GetX());
                ps_contour_y.push_back(ps[ps.size() - 1 - index].GetY());
            }
            //////////insert the start point again to close the region///////
//             unsigned int index = 0;
//             ps_contour_x.push_back(ps[ps.size() - 1 - index].GetX());
//             ps_contour_y.push_back(ps[ps.size() - 1 - index].GetY());
        }
      }else{//hole points, should be clockwise
        if(clock == false){
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[ps.size() -1 - index].GetX());
                ps_contour_y.push_back(ps[ps.size() -1 - index].GetY());
            }
            //////////insert the start point again to close the region///////
//             unsigned int index = 0;
//             ps_contour_x.push_back(ps[ps.size() -1 - index].GetX());
//             ps_contour_y.push_back(ps[ps.size() -1 - index].GetY());
        }else{
            for(unsigned int index = 0;index < ps.size();index++){
                ps_contour_x.push_back(ps[index].GetX());
                ps_contour_y.push_back(ps[index].GetY());
            }
            //////////insert the start point again to close the region///////
//             unsigned int index = 0;
//             ps_contour_x.push_back(ps[index].GetX());
//             ps_contour_y.push_back(ps[index].GetY());
        }
      }

      delete sl_contour[i];
  }
  delete sp;

}

/*
Decompose a polygon with and without holes into a set of triangles
Using the implementation by Atul Narkhede and Dinesh Manocha

*/

void CompTriangle::NewTriangulation()
{

  if(reg->NoComponents() == 0){
      cout<<"this is not a region"<<endl;
      return;
  }

  if(reg->NoComponents() > 1){
      cout<<"can't handle region with more than one face"<<endl;
      return;
  }


  ////////////////////get the number of cycles////////////////////
  unsigned int no_cyc = NoOfCycles();

/*  if(no_cyc == 1){
    Triangulation();
    return;
  }*/

  //the first is the outer cycle
//  cout<<"polgyon with "<<no_cyc - 1<<" holes inside "<<endl;


  const int ncontours = no_cyc;
  int no_p_contour[ncontours];

  vector<double> ps_contour_x;
  vector<double> ps_contour_y;

  PolygonContourPoint(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);

 //  cout<<"finish creating contour for the polgyon"<<endl;
//   for(unsigned int i = 0;i < no_cyc;i++)
//     cout<<no_p_contour[i]<<endl;

//  cout<<"no vertices "<<ps_contour_x.size()<<endl;

  ///call the algorithm implemented by Atul Narkhede and Dinesh Manocha ///////
  int result_trig[SEGSIZE][3];
  int (*res_triangles)[3] = &result_trig[0];

  int no_triangle;
  no_triangle = triangulate_polygon(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y, res_triangles);

//  cout<<"no_triangle "<<no_triangle<<endl;

  assert(0 < no_triangle && no_triangle < SEGSIZE);


  SpacePartition* spacepart = new SpacePartition();

  for (int i = 0; i < no_triangle; i++){
//    printf("triangle #%d: %d %d %d\n", i,
//       res_triangles[i][0], res_triangles[i][1], res_triangles[i][2]);

    vector<Point> ps_reg;
    Point p1, p2, p3;
    Coord x, y;
    x = ps_contour_x[res_triangles[i][0]];
    y = ps_contour_y[res_triangles[i][0]];
    p1.Set(x, y);
    x = ps_contour_x[res_triangles[i][1]];
    y = ps_contour_y[res_triangles[i][1]];
    p2.Set(x, y);
    x = ps_contour_x[res_triangles[i][2]];
    y = ps_contour_y[res_triangles[i][2]];
    p3.Set(x, y);
    ps_reg.push_back(p1);
    ps_reg.push_back(p2);
    ps_reg.push_back(p3);
    vector<Region> reg;
    spacepart->ComputeRegion(ps_reg, reg);
    triangles.push_back(reg[0]);
  }
  delete spacepart;
  ////////////////////////////////////////////////////////////////////

}

/*
Decompose a polygon with and without holes into a set of triangles
Using the implementation by from codeproject
holes: number of vertices - 2
without holes: number of vertices - 1 - 2

*/

void CompTriangle::NewTriangulation2()
{

  if(reg->NoComponents() == 0){
      cout<<"this is not a region"<<endl;
      return;
  }

  if(reg->NoComponents() > 1){
      cout<<"can't handle region with more than one face"<<endl;
      return;
  }


  ////////////////////get the number of cycles////////////////////
  unsigned int no_cyc = NoOfCycles();

  if(no_cyc == 1){
    Triangulation();
    return;
  }

  //the first is the outer cycle
//  cout<<"polgyon with "<<no_cyc - 1<<" holes inside "<<endl;


  const int ncontours = no_cyc;
  int no_p_contour[ncontours];

  vector<double> ps_contour_x;
  vector<double> ps_contour_y;

  PolygonContourPoint2(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);

 //  cout<<"finish creating contour for the polgyon"<<endl;
//   for(unsigned int i = 0;i < no_cyc;i++)
//     cout<<no_p_contour[i]<<endl;

//  cout<<"no vertices "<<ps_contour_x.size()<<endl;

  HPolygon poly;
  int no_triangle = poly.Triangulation2(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y);

//  cout<<"no_triangle "<<no_triangle<<endl;


  SpacePartition* spacepart = new SpacePartition();

  for (int i = 0; i < no_triangle; i++){

//    printf("triangle #%d: %d %d %d\n", i,
//       poly.mtabCell[i].Index(0) + 1,
//       poly.mtabCell[i].Index(1) + 1,
//       poly.mtabCell[i].Index(2) + 1);

      int index1 = poly.mtabCell[i].Index(0);
      int index2 = poly.mtabCell[i].Index(1);
      int index3 = poly.mtabCell[i].Index(2);

//       cout<<"( "<<poly.mtabPnt[index1].X()<<" "
//           <<poly.mtabPnt[index1].Y()<<") "
//           <<"( "<<poly.mtabPnt[index2].X()<<" "
//           <<poly.mtabPnt[index2].Y()<<") "
//           <<"( "<<poly.mtabPnt[index3].X()<<" "
//           <<poly.mtabPnt[index3].Y()<<") "<<endl;

    vector<Point> ps_reg;
    Point p1, p2, p3;
    Coord x, y;
    x = poly.mtabPnt[index1].X();
    y = poly.mtabPnt[index1].Y();
    p1.Set(x, y);
    x = poly.mtabPnt[index2].X();
    y = poly.mtabPnt[index2].Y();
    p2.Set(x, y);
    x = poly.mtabPnt[index3].X();
    y = poly.mtabPnt[index3].Y();
    p3.Set(x, y);
    ps_reg.push_back(p1);
    ps_reg.push_back(p2);
    ps_reg.push_back(p3);
    vector<Region> reg;
//    cout<<p1<<" "<<p2<<" "<<p3<<endl;
    spacepart->ComputeRegion(ps_reg, reg);
    if(reg.size() > 0) //2012.3.9 rounding problem
      triangles.push_back(reg[0]);

  }
  delete spacepart;

  ////////////////////////////////////////////////////////////////////

}

/*
collect all vertices of a polygon together with its two neighbors
the two halfsegments
One big region with a lot of holes inside. it collects the vertices of all
these holes

*/
void CompTriangle::GetAllPoints()
{
  if(reg->NoComponents() == 0){
      cout<<"this is not a region"<<endl;
      return;
  }

  ////////////////////get the number of cycles, exclude outer contour//////////
  unsigned int no_cyc = NoOfCycles();

//  cout<<"polgyon with "<<no_cyc<<" holes inside "<<endl;

  vector<double> ps_contour_x;
  vector<double> ps_contour_y;


  ps_contour_x.push_back(0.0);
  ps_contour_y.push_back(0.0);

  vector<SimpleLine*> sl_contour;

  for(unsigned int i = 0;i < no_cyc;i++){
      SimpleLine* sl = new SimpleLine(0);
      sl->StartBulkLoad();
      sl_contour.push_back(sl);
  }
  vector<int> edgenos(no_cyc, 0);
  for(int j = 0;j < reg->Size();j++){
    HalfSegment hs1;
    reg->Get(j, hs1);
//    if(!hs1.IsLeftDomPoint() || hs1.attr.cycleno == 0) continue;
    if(!hs1.IsLeftDomPoint()) continue;
    HalfSegment hs2;
    hs2.Set(true, hs1.GetLeftPoint(), hs1.GetRightPoint());
    int cycle_no = hs1.attr.cycleno;

    hs2.attr.edgeno = edgenos[cycle_no]++;
    *sl_contour[cycle_no] += hs2;
    hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
    *sl_contour[cycle_no] += hs2;
  }
//  cout<<"get all boundary line"<<endl;
  SpacePartition* sp = new SpacePartition();
  for(unsigned int i = 0;i < no_cyc;i++){
      sl_contour[i]->EndBulkLoad();
      vector<MyHalfSegment> mhs;
      sp->ReorderLine(sl_contour[i], mhs);
      for(unsigned int j = 0;j < mhs.size();j++){
        plist1.push_back(mhs[j].from);
        reg_id.push_back(i + 1);//cycle_no id, start from 1;
        if(j == 0){
          plist2.push_back(mhs[j + 1].from);
          plist3.push_back(mhs[mhs.size() - 1].from);
        }else if(j == mhs.size() - 1){
          plist2.push_back(mhs[j - 1].from);
          plist3.push_back(mhs[0].from);
        }else{
          plist2.push_back(mhs[j - 1].from);
          plist3.push_back(mhs[j + 1].from);
        }
      }
  }
  delete sp;
//  cout<<plist1.size()<<" "<<plist2.size()<<" "<<plist3.size()<<endl;
}

/*
get all visible point for the given query point
using rotational plane sweep algorithm, developed by
M.Sharir and A.Schorr.
On Shortest Paths in Polyhedral Spaces, SIAM, Journal of Computing,15(1),1986

*/
string CompTriangle::AllPointsInfo =
"(rel(tuple((V point)(Neighbor1 point)(Neighbor2 point)(Regid int))))";


ostream& operator<<(ostream& o, const RPoint& rp)
{
  o<<setprecision(12);
  o<<"p "<<rp.p<<"angle "<<rp.angle<<" dist "<<rp.dist<<endl;
  return o;
}

ostream& operator<<(ostream& o, const MySegDist& seg)
{
  o<<"from "<<seg.from<<" to "<<seg.to<<" dist "<<seg.dist<<endl;
  return o;
}

/*
Initialize the event queue, points are sorted by the angle in counter-clockwise

*/
void CompTriangle::InitializeQueue(priority_queue<RPoint>& allps,
                                   Point& query_p, Point& hp, Point& p,
                                   SpacePartition* sp, Point* q1,
                                   Point* q2, int reg_id)
{
    double angle = sp->GetAngle(query_p, hp, p);
    double dist = query_p.Distance(p);
    if(sp->GetClockwise(query_p, hp, p)){// clockwise
        angle = 2*TM_MYPI - angle;
    }
    RPoint rp(p, angle, dist, reg_id);
    rp.SetNeighbor(*q1, *q2);
    allps.push(rp);
}


/*
insert segments that intersect the original sweep line into AVLTree

*/
void CompTriangle::InitializeAVL( multiset<MySegDist>& sss,
                                      Point& query_p, Point& hp,
                                      Point& p, Point& neighbor,
                                      SpacePartition* sp)
{
    HalfSegment hs1;
    hs1.Set(true, query_p, hp);
    HalfSegment hs2;
    hs2.Set(true, p, neighbor);
    Point ip;
    /////////////////do not insert such segment//////////////////////////////
    ///////as it needs more work to determin counter-clockwise or clockwise//
    ///////we collect the two endpoints as visible and determine the region
    ///face locates on which side of the segment
    if(hs2.Contains(query_p))return;

    if(hs1.Intersection(hs2, ip)){
        double angle1 = sp->GetAngle(query_p, hp, p);
        double angle2 = sp->GetAngle(query_p, hp, neighbor);
        if(sp->GetClockwise(query_p, hp, p))
          angle1 = 2*TM_MYPI-angle1;
        if(sp->GetClockwise(query_p, hp, neighbor))
          angle2 = 2*TM_MYPI-angle2;
        if(AlmostEqual(angle1, angle2)){
            double d1 = query_p.Distance(p);
            double d2 = query_p.Distance(neighbor);
            if(d1 < d2){
              MySegDist myseg(true, p, neighbor, d1);
      //          if(!(*(sss.find(myseg)) == myseg))
              sss.insert(myseg);
            }
        }else{
          double dist = hs2.Distance(query_p);
          MySegDist myseg(true, p, neighbor, dist);
//          if(!(*(sss.find(myseg)) == myseg))
          sss.insert(myseg);
        }
    }
}

void CompTriangle::PrintAVLTree(multiset<MySegDist>& sss, ofstream& outfile)
{
  outfile<<"AVLTree"<<endl;
  multiset<MySegDist>::iterator start;
  multiset<MySegDist>::iterator end = sss.end();
  for(start = sss.begin();start != end; start++)
    outfile<<*start<<endl;
}

/*
process the neighbor point of current point from queue,i.e.,
whether it inserts or deletes the segment

*/
/*void CompTriangle::ProcessNeighbor(multiset<MySegDist>& sss,
                            RPoint& rp, Point& query_p,
                            Point& neighbor, Point& hp,
                            SpacePartition* sp, ofstream& outfile)*/

void CompTriangle::ProcessNeighbor(multiset<MySegDist>& sss,
                            RPoint& rp, Point& query_p,
                            Point& neighbor, Point& hp,
                            SpacePartition* sp)
{
    HalfSegment hs;
    hs.Set(true, rp.p, neighbor);
//    outfile<<"rp.p "<<rp.p<<"neighbor "<<neighbor<<endl;
    if(rp.p.Distance(neighbor) < EPSDIST) return;

    /////////////////do not insert such segment//////////////////////////////
    ///////as it needs more work to determine counter-clockwise or clockwise//
    ///////we collect the two endpoints as visible and determine the region
    ///face locates on which side of the segment
    if(hs.Contains(query_p)) return;

    double angle = sp->GetAngle(query_p, hp, neighbor);
    if(sp->GetClockwise(query_p, hp, neighbor))
      angle = 2*TM_MYPI - angle;

    if(AlmostEqual(angle, rp.angle)){
//        outfile<<"on the same line as query_p "<<endl;
        double d1 = rp.dist;
        double d2 = query_p.Distance(neighbor);
        assert(!AlmostEqual(d1, d2));
        if(d1 < d2){ //insert
            double dist = hs.Distance(query_p);
            MySegDist myseg(true, rp.p, neighbor, dist);
//            outfile<<"insert1 "<<myseg<<endl;
//            if(!(*(sss.find(myseg)) == myseg))
                sss.insert(myseg);
        }else{
            double dist = hs.Distance(query_p);
            MySegDist myseg(true, rp.p, neighbor, dist);
//            outfile<<"remove1 "<<myseg<<endl;
            //erase the element
            multiset<MySegDist>::iterator iter = sss.find(myseg);

//            assert(iter != sss.end());
            if(iter != sss.end()){
              while(iter != sss.end()){
                if(*iter == myseg){
                    sss.erase(iter);
                    break;
                  }else
                  iter++;
              }
            }
        }
      return;
    }

    if(sp->GetClockwise(query_p, rp.p, neighbor) == false){
        double dist = hs.Distance(query_p);
        MySegDist myseg(true, rp.p, neighbor, dist);
//        outfile<<"insert2 "<<myseg<<endl;
//        if(!(*(sss.find(myseg)) == myseg))
            sss.insert(myseg);
    }else{
//        outfile<<"clockwise"<<endl;
        double dist = hs.Distance(query_p);
        MySegDist myseg(true, rp.p, neighbor, dist);
//        outfile<<"remove2 "<<myseg<<endl;
        //erase the element
        multiset<MySegDist>::iterator iter = sss.find(myseg);
//        assert(iter != sss.end());
        if(iter != sss.end()){
          while(iter != sss.end()){
            if(*iter == myseg){
                sss.erase(iter);
                break;
            }else
            iter++;
          }
        }
    }

}
/*
using rotational plane sweep algorithm to find visible points.
the query point should not locate on the outer contour boundary
Micha Sharir and Amir Schorr
On Shortest Paths in Polyhedral Spaces, SIAM Journal on Computing. 1986

*/

void CompTriangle::GetVPoints(Relation* rel1, Relation* rel2,
                              Rectangle<2>* bbox, Relation* rel3, int attr_pos)
{
//  cout<<"attr_pos "<<attr_pos<<endl;
//  ofstream outfile("tmrecord");

  if(rel1->GetNoTuples() < 1){
    cout<<"query relation is empty"<<endl;
    return;
  }
  Tuple* query_tuple = rel1->GetTuple(1, false);
  Point* query_p =
          new Point(*((Point*)query_tuple->GetAttribute(VisualGraph::QLOC2)));
  query_tuple->DeleteIfAllowed();
//  outfile<<"query point "<<*query_p<<endl;
  Coord xmax = bbox->MaxD(0) + 1.0;

  Point hp;
  hp.Set(xmax, query_p->GetY());
  HalfSegment hs1;
  hs1.Set(true, *query_p, hp);
//  cout<<"horizonal p "<<hp<<endl;

  ///////////// sort all points by counter clockwise order//////////////////
  priority_queue<RPoint> allps;
  multiset<MySegDist> sss;

  vector<Point> neighbors;

  //one segment contains query_p
  //it represents query_p equals to the endpoint(=2) or not(=3)
  int no_contain = 0;

  int hole_id = 0; //record whether the query point locates on the hole boundary
  SpacePartition* sp = new SpacePartition();
  for(int i = 1;i <= rel2->GetNoTuples();i++){
      Tuple* p_tuple = rel2->GetTuple(i, false);
      Point* p = (Point*)p_tuple->GetAttribute(CompTriangle::V);
      if(AlmostEqual(*query_p, *p)){//equal points are ignored
        p_tuple->DeleteIfAllowed();
        continue;
      }
      Point* q1 = (Point*)p_tuple->GetAttribute(CompTriangle::NEIGHBOR1);
//      outfile<<"vertex "<<*p<<endl;
//      outfile<<sp->GetAngle(*query_p, hp, *p)<<endl;
      Point* q2 = (Point*)p_tuple->GetAttribute(CompTriangle::NEIGHBOR2);
      int reg_id =
          ((CcInt*)p_tuple->GetAttribute(CompTriangle::REGID))->GetIntval();

      HalfSegment hs1;
      hs1.Set(true, *p, *q1);
      HalfSegment hs2;
      hs2.Set(true, *p, *q2);
      if(no_contain < 2 && hs1.Contains(*query_p)){
/*        cout<<"segment contains query points "<<endl;
        cout<<"current it is not supported"<<endl;
        p_tuple->DeleteIfAllowed();
        return;*/
//        cout<<"hs1 "<<hs1<<"q1 "<<*q1<<endl;
        hole_id = reg_id;

        if(AlmostEqual(*query_p, hs1.GetLeftPoint()))
          neighbors.push_back(hs1.GetRightPoint());
        else if(AlmostEqual(*query_p, hs1.GetRightPoint()))
          neighbors.push_back(hs1.GetLeftPoint());
        else{
          neighbors.push_back(hs1.GetLeftPoint());
          neighbors.push_back(hs1.GetRightPoint());
          no_contain = 2;
        }
        no_contain++;
      }
      if(no_contain < 2 && hs2.Contains(*query_p)){
//        cout<<"hs2 "<<hs2<<"q2 "<<*q2<<endl;
        hole_id = reg_id;

        if(AlmostEqual(*query_p, hs2.GetLeftPoint()))
          neighbors.push_back(hs2.GetRightPoint());
        else if(AlmostEqual(*query_p, hs2.GetRightPoint()))
          neighbors.push_back(hs2.GetLeftPoint());
        else{
          neighbors.push_back(hs2.GetLeftPoint());
          neighbors.push_back(hs2.GetRightPoint());
          no_contain = 2;
        }
        no_contain++;
      }

      InitializeAVL(sss,*query_p, hp, *p, *q1, sp);
      InitializeAVL(sss,*query_p, hp, *p, *q2, sp);
      InitializeQueue(allps,*query_p, hp, *p, sp, q1, q2, reg_id);

      p_tuple->DeleteIfAllowed();
  }
  delete sp;

//  PrintAVLTree(sss, outfile);
  double last_angle = -1.0;

  double angle1 = -1.0;
  double angle2 = -1.0;
  bool face_direction; //false counter-clockwise, true clockwise

  //determine the region face on which side of the segment
  if(hole_id != 0){
//    cout<<neighbors.size()<<endl;
    assert(neighbors.size() == 2);
//    cout<<neighbors[0]<<" "<<neighbors[1]<<endl;
    assert(!AlmostEqual(neighbors[0], neighbors[1]));
    Tuple* reg_tuple = rel3->GetTuple(hole_id, false);
    Region* r = (Region*)reg_tuple->GetAttribute(attr_pos);

    Line* boundary = new Line(0);
    r->Boundary(boundary);
    SimpleLine* sboundary = new SimpleLine(0);
    sboundary->fromLine(*boundary);
    vector<MyHalfSegment> mhs;
    //get all the points of the region
    SpacePartition* sp = new SpacePartition();
    if(sboundary->Size() > 0)
      sp->ReorderLine(sboundary, mhs);
    else{
      cout<<"can't covert the boundary to a sline, maybe there is a hole"<<endl;
      delete boundary;
      delete sboundary;
      return;
    }
    delete boundary;
    delete sboundary;

    vector<Point> ps;
    for(unsigned int i = 0;i < mhs.size();i++)
      ps.push_back(mhs[i].from);

    angle1 = sp->GetAngle(*query_p, hp, neighbors[0]);
    angle2 = sp->GetAngle(*query_p, hp, neighbors[1]);
    if(sp->GetClockwise(*query_p, hp, neighbors[0]))
      angle1 = 2*TM_MYPI - angle1;
    if(sp->GetClockwise(*query_p, hp, neighbors[1]))
      angle2 = 2*TM_MYPI - angle2;

    if(angle1 > angle2){
      Point temp = neighbors[0];
      neighbors[0] = neighbors[1];
      neighbors[1] = temp;
      double temp_angle = angle1;
      angle1 = angle2;
      angle2 = temp_angle;
    }

    //combine with index1 and index2
    //the point with smaller angle and larger angle
    //from smaller angle to larger angle, whether it is counter clockwisr or not
    if(0.0 < Area(ps)) face_direction = false;//counter-clockwise
    else face_direction = true;
    //query_p locates in one segment but not equal to endpoint
    if(no_contain > 2){
      unsigned int index1 = ps.size();
      unsigned int index2 = ps.size();
      for(unsigned int i = 0;i < ps.size();i++){
          if(AlmostEqual(ps[i], neighbors[0])) index1 = i;
          if(AlmostEqual(ps[i], neighbors[1])) index2 = i;
      }
      assert(index1 != ps.size() && index2 != ps.size());
      if(hole_id == 1){
        if(index1 == index2 + 1){
          face_direction = !face_direction;
        }else if(index1 == 0 && index2 == ps.size() - 1){
          face_direction = !face_direction;
        }
      }else{
        if(index1  + 1 == index2){
          face_direction = !face_direction;
        }else if(index1 == ps.size() - 1 && index2 == 0){
          face_direction = !face_direction;
        }
      }
    }else if(no_contain == 2){//query_p equals to one endpoint of a segment
      unsigned int index1 = ps.size();
      unsigned int index2 = ps.size();
      for(unsigned int i = 0;i < ps.size();i++){
        if(AlmostEqual(ps[i], neighbors[0])) index1 = i;
        if(AlmostEqual(ps[i], *query_p)) index2 = i;
      }
      assert(index1 != ps.size() && index2 != ps.size());
//      outfile<<"index1 "<<index1<<" index2 "<<index2<<endl;
//      outfile<<"face direction "<<face_direction<<endl;
      if(hole_id == 1){
        if(index1 == index2 + 1){
          face_direction = !face_direction;
        }else if(index1 == 0 && index2 == ps.size() - 1){
          face_direction = !face_direction;
        }
      }else{
        if(index1 + 1 == index2){
          face_direction = !face_direction;
        }else if(index1 == ps.size() - 1 && index2 == 0){
          face_direction = !face_direction;
        }
      }
    }else assert(false);
//    outfile<<"direction "<<face_direction<<" 1 "<<angle1<<" 2 "<<angle2<<endl;
//    outfile<<"hole_id "<<hole_id<<endl;
    reg_tuple->DeleteIfAllowed();
  }

  Tuple* reg_tuple = rel3->GetTuple(1, false);
  Region* outer_region = (Region*)reg_tuple->GetAttribute(attr_pos);

  while(allps.empty() == false){
      RPoint top = allps.top();
      allps.pop();
      ////////////////debug/////////////////
//      PrintAVLTree(sss, outfile);
//      outfile<<top<<" "<<top.regid<<endl;
      //////////////////////////////////////
      if(sss.empty()){
//          outfile<<"avltree is emtpy find visible point1"<<endl;
          bool visible = true;
          if(top.regid == 1){//outer boundary
              HalfSegment hs;
              hs.Set(true, *query_p, top.p);
              if(RegContainHS(outer_region, hs) == false)
                visible = false;
          }
          if(visible){
            plist1.push_back(top.p);
            Line* l = new Line(0);
            l->StartBulkLoad();
            HalfSegment hs;
            hs.Set(true, *query_p, top.p);
            hs.attr.edgeno = 0;
            *l += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *l += hs;
            l->EndBulkLoad();
            connection.push_back(*l);
            delete l;
          }
          last_angle = top.angle;
/*          ProcessNeighbor(sss, top, *query_p,
                            top.n1, hp, sp, outfile);
          ProcessNeighbor(sss, top, *query_p,
                            top.n2, hp, sp, outfile);*/
          ProcessNeighbor(sss, top, *query_p,
                            top.n1, hp, sp);
          ProcessNeighbor(sss, top, *query_p,
                            top.n2, hp, sp);
          continue;
      }
      ////////////// visibility checking  ///////////////////////////
      if(!AlmostEqual(top.angle, last_angle)){
          multiset<MySegDist>::iterator start = sss.begin();
          double d = query_p->Distance(top.p);
          double mindist = start->dist;
          bool visible = true;
//          outfile<<"d "<<d<<" mindist "<<mindist<<endl;

          if(top.regid == hole_id){//filter some points on the boundary
              if(face_direction == false){
                 if(angle1 < top.angle && top.angle < angle2)visible = false;
              }else{
                 if(top.angle < angle1 || top.angle > angle2)visible = false;
              }
          }

          if(visible){
              if(d < mindist || AlmostEqual(d, mindist)){
//                outfile<<"find visible point2"<<endl;
              }else{
                  HalfSegment test_hs;
                  test_hs.Set(true, *query_p, top.p);
                  while(start->dist < d && start != sss.end()){
                    HalfSegment cur_hs;
                    cur_hs.Set(true, start->from, start->to);
                    Point ip;
                    if(test_hs.Intersection(cur_hs, ip)){
//                    outfile<<"test_hs "<<test_hs<<"cur_hs "<<cur_hs<<endl;
  //                  outfile<<ip<<endl;
                      if(!AlmostEqual(ip, top.p)){
//                        outfile<<"visible false"<<endl;
//                        outfile<<"test_hs "<<test_hs<<"cur_hs "<<cur_hs<<endl;
                        visible = false;
                        break;
                      }
                    }
                    start++;
                  }
              }
          }

          if(visible && top.regid == 1){//outer boundary
              HalfSegment hs;
              hs.Set(true, *query_p, top.p);
              if(RegContainHS(outer_region, hs) == false)
                visible = false;
          }

          if(visible){
//              outfile<<"find visible point4"<<endl;
              plist1.push_back(top.p);
              Line* l = new Line(0);
              l->StartBulkLoad();
              HalfSegment hs;
              hs.Set(true, *query_p, top.p);
              hs.attr.edgeno = 0;
              *l += hs;
              hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
              *l += hs;
              l->EndBulkLoad();
              connection.push_back(*l);
              delete l;
          }
      }
//      ProcessNeighbor(sss, top, *query_p,top.n1, hp, sp, outfile);
//      ProcessNeighbor(sss, top, *query_p,top.n2, hp, sp, outfile);
      ProcessNeighbor(sss, top, *query_p, top.n1, hp, sp);
      ProcessNeighbor(sss, top, *query_p, top.n2, hp, sp);
      //////////////////////////////////////////////////////////////////
      last_angle = top.angle;
      //////////////////////////////////////////////////////////////
  }

  reg_tuple->DeleteIfAllowed();
  delete query_p;

}

/*********************implementation of basgraph********************/
BaseGraph::BaseGraph():g_id(0),
node_rel(NULL),
edge_rel(NULL),
adj_list(0),
entry_adj_list(0)
{
//  cout<<"BaseGraph::BaseGraph()"<<endl;
}

BaseGraph::BaseGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect):g_id(0),
node_rel(NULL),
edge_rel(NULL),
adj_list(0),
entry_adj_list(0)
{
//  cout<<"BaseGraph::BaseGraph(ListExpr)"<<endl;
}

BaseGraph::BaseGraph(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo):
g_id(0), node_rel(NULL), edge_rel(NULL),
adj_list(0), entry_adj_list(0)
{
//  cout<<"BaseGraph::BaseGraph(SmiRecord)"<<endl;
}

BaseGraph::~BaseGraph()
{
//    cout<<"~BaseGraph()"<<endl;
    if(node_rel != NULL)
      node_rel->Close();

    if(edge_rel != NULL)
      edge_rel->Close();

//    adj_list.clean();
//    entry_adj_list.clean();

}

void BaseGraph::Destroy()
{
//  cout<<"Destroy()"<<endl;
  if(node_rel != NULL){
      node_rel->Delete();
      node_rel = NULL;
  }
  if(edge_rel != NULL){
    edge_rel->Delete();
    edge_rel = NULL;
  }

}

ListExpr BaseGraph::BaseGraphProp()
{
//    cout<<"BaseGraphProp()"<<endl;
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist,
               "createdualgraph(<id>,<edge-relation>,<node-relation>)");
    return nl->TwoElemList(
             nl->TwoElemList(nl->StringAtom("Creation"),
                              nl->StringAtom("Example Creation")),
             nl->TwoElemList(examplelist,
                   nl->StringAtom("let dg=createdualgraph(id,e-rel,n-rel)")));
}

void* BaseGraph::CastBaseGraph(void* addr)
{
//  cout<<"CastBaseGraph()"<<endl;
  return 0;
}

Word BaseGraph::CloneBaseGraph(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneBaseGraph()"<<endl;
  return SetWord(Address(0));
}

int BaseGraph::SizeOfBaseGraph()
{
//  cout<<"SizeOfBaseGraph()"<<endl;
  return 0;
}

/*
Given a nodeid, find all its adjacecny nodes

*/
void BaseGraph::FindAdj(int node_id, vector<bool>& flag, vector<int>& list)
{

  ListEntry list_entry;
  entry_adj_list.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list.Get(j, oid);
      j++;
      if(flag[oid - 1] == false){
        list.push_back(oid);
        flag[oid - 1] = true;
      }
  }

}

void BaseGraph::FindAdj(int node_id, vector<int>& list)
{

  ListEntry list_entry;
  entry_adj_list.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list.Get(j, oid);
      j++;
      list.push_back(oid);
  }

}

/*How can we find the shortest path for any polygon (with or without holes) and
any startpoint and endpoint?  As far as I know, an exhaustive search is
required.  However, the search can be optimized a bit so that it doesn’t take
as long as testing every possible path. With the dual graph and visbility graph,
it can be found efficiently*/

/*build a visibility graph, connect the start and end point to the graph*/

string DualGraph::NodeTypeInfo =
  "(rel(tuple((Oid int)(Rid int)(Pavement region))))";
string DualGraph::BTreeNodeTypeInfo = 
  "(btree(tuple((Oid int)(Rid int)(Pavement region))) int)";
string DualGraph::EdgeTypeInfo =
  "(rel(tuple((Oid1 int)(Oid2 int)(Commarea line))))";
string DualGraph::NodeRTreeTypeInfo =  "(rtree (tuple ((oid int)(rid int)\
(pavement region))) region FALSE)";
string DualGraph::TriangleTypeInfo1 =
  "(rel(tuple((V1 int)(V2 int)(V3 int)(Centroid point)(Oid int))))";
string DualGraph::TriangleTypeInfo2 =
  "(rel(tuple((Cycleno int)(Vertex point))))";
string DualGraph::TriangleTypeInfo3 =
  "(rel(tuple((V1 int)(V2 int)(V3 int)(Centroid point))))";
string DualGraph::TriangleTypeInfo4 =
  "(rel(tuple((Vid int)(Triid int))))";

ListExpr DualGraph::OutDualGraph(ListExpr typeInfo, Word value)
{
//  cout<<"OutDualGraph()"<<endl;
  DualGraph* dg = (DualGraph*)value.addr;
  return dg->Out(typeInfo);
}

ListExpr DualGraph::Out(ListExpr typeInfo)
{
//  cout<<"Out()"<<endl;
  ListExpr xNode = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= node_rel->GetNoTuples();i++){
      Tuple* node_tuple = node_rel->GetTuple(i, false);
      CcInt* oid = (CcInt*)node_tuple->GetAttribute(OID);
      CcInt* rid = (CcInt*)node_tuple->GetAttribute(RID);
      Region* reg = (Region*)node_tuple->GetAttribute(PAVEMENT);

      ListExpr xRegion = OutRegion(nl->TheEmptyList(),SetWord(reg));
      xNext = nl->FourElemList(nl->IntAtom(g_id),
                               nl->IntAtom(oid->GetIntval()),
                               nl->IntAtom(rid->GetIntval()),
                               xRegion);
      if(bFirst){
        xNode = nl->OneElemList(xNext);
        xLast = xNode;
        bFirst = false;
      }else
          xLast = nl->Append(xLast,xNext);
      node_tuple->DeleteIfAllowed();
  }

  return nl->TwoElemList(nl->IntAtom(g_id),xNode);
}

Word DualGraph::InDualGraph(ListExpr in_xTypeInfo, ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
//  cout<<"InDualGraph()"<<endl;
  DualGraph* dg = new DualGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(dg);
  else{
    delete dg;
    return SetWord(Address(0));
  }
}

Word DualGraph::CreateDualGraph(const ListExpr typeInfo)
{
//  cout<<"CreateDualGraph()"<<endl;
  return SetWord(new DualGraph());
}

void DualGraph::CloseDualGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseDualGraph()"<<endl;
  delete static_cast<DualGraph*> (w.addr);
  w.addr = NULL;
}


void DualGraph::DeleteDualGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteDualGraph()"<<endl;
  DualGraph* dg = (DualGraph*)w.addr;
//  dg->Destroy();
  delete dg;
  w.addr = NULL;
}

bool DualGraph::CheckDualGraph(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckDualGraph()"<<endl;
  return nl->IsEqual(type, "dualgraph");
}



bool DualGraph::SaveDualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveDualGraph()"<<endl;
  DualGraph* dg = (DualGraph*)value.addr;
  bool result = dg->Save(valueRecord, offset, typeInfo);

  return result;
}

bool DualGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
//  cout<<"Save()"<<endl;
  /********************Save graph id ****************************/
  in_xValueRecord.Write(&g_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);
 
  ////////////minimum oid - 1/////////////////////////////////////
  in_xValueRecord.Write(&min_tri_oid_1,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);
  
  ListExpr xType;
  ListExpr xNumericType;
  /************************save node****************************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /************************save sorted node****************************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel_sort->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  
  ///////////////////////save rtree on dual graph nodes///////////////////
  if(!rtree_node->Save(in_xValueRecord, inout_iOffset)){
    return false;
  }
  
  /************************save edge****************************/
  nl->ReadFromString(EdgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;


   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();
   adj_list.saveToFile(rf, adj_list);
   SmiSize offset = 0;
   size_t bufsize = adj_list.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   adj_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list.saveToFile(rf, entry_adj_list);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

   
  return true;
}

bool DualGraph::OpenDualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenDualGraph()"<<endl;
  value.addr = DualGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

DualGraph* DualGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
//  cout<<"Open()"<<endl;
  return new DualGraph(valueRecord,offset,typeInfo);
}

DualGraph::~DualGraph()
{
//  cout<<"~DualGraph()"<<endl;
  if(node_rel_sort != NULL) node_rel_sort->Close();
  if(rtree_node != NULL) delete rtree_node;
}

DualGraph::DualGraph():node_rel_sort(NULL), rtree_node(NULL)
{
//  cout<<"DualGraph::DualGraph()"<<endl;
}

DualGraph::DualGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect):node_rel_sort(NULL), rtree_node(NULL)

{
//  cout<<"DualGraph::DualGraph(ListExpr)"<<endl;

}

DualGraph::DualGraph(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo):node_rel_sort(NULL), rtree_node(NULL)
{
//   cout<<"DualGraph::DualGraph(SmiRecord)"<<endl;
   /***********************Read graph id********************************/
  in_xValueRecord.Read(&g_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);

   /***********************minimum oid -1********************************/
  in_xValueRecord.Read(&min_tri_oid_1,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);
  
  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for node*********************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  node_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!node_rel) {
    return;
  }
     
   //////////////////////node relation after sorting//////////////////////////
   nl->ReadFromString(NodeTypeInfo,xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
   node_rel_sort = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
   if(!node_rel_sort) {
    node_rel->Delete();
    return;
   }

   ///////////////////rtree on dual graph sorted nodes////////////////////
   Word xValue;
   if(!(rtree_node->Open(in_xValueRecord,inout_iOffset, 
        NodeRTreeTypeInfo,xValue))){
    node_rel->Delete();
    node_rel_sort->Delete();
    return;
  }

  rtree_node = ( R_Tree<2,TupleId>* ) xValue.addr;

  /***********************Open relation for edge*********************/
  nl->ReadFromString(EdgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel) {
    node_rel->Delete();
    node_rel_sort->Delete();
    return;
  }

  ////////////////////adjaency list////////////////////////////////
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);


}



/*
let dg1 = createdualgraph(1, graph_node, graph_edge);
delete dg1;
close database;
quit; //very important, otherwise, it can't create it again

start Secondo
let dg1 = createdualgraph(1, graph_node, graph_edge);

*/

void DualGraph::Load(int id, Relation* r1, Relation* r2)
{
//  cout<<"Load()"<<endl;
  g_id = id;
  //////////////////node relation////////////////////
  ListExpr ptrList1 = listutils::getPtrList(r1);
  
  string strQuery = "(consume(sort(feed(" + NodeTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;
  ////////////////////////////////////////////////////////
  /////////////create sorting node relation///////////////
  ////////////////////////////////////////////////////////
  LoadSortNode(r1); 

  /////////////////edge relation/////////////////////
  ListExpr ptrList2 = listutils::getPtrList(r2);
  
  strQuery = "(consume(sort(feed(" + EdgeTypeInfo +
                "(ptr " + nl->ToString(ptrList2) + ")))))";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel = (Relation*)xResult.addr;

  ////////////adjacency list ////////////////////////////////
  ListExpr ptrList3 = listutils::getPtrList(edge_rel);

  strQuery = "(createbtree (" + EdgeTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "Oid1)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree_node_oid1 = (BTree*)xResult.addr;


  ListExpr ptrList4 = listutils::getPtrList(edge_rel);

  strQuery = "(createbtree (" + EdgeTypeInfo +
             "(ptr " + nl->ToString(ptrList4) + "))" + "Oid2)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  BTree* btree_node_oid2 = (BTree*)xResult.addr;

  ////////////////////////////////////////////////////////////////////////
  /////////////////get minimum triangle oid///////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  int min_tri_oid = numeric_limits<int>::max(); 
  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* tri_tuple = node_rel->GetTuple(i, false);
    int tri_oid = ((CcInt*)tri_tuple->GetAttribute(OID))->GetIntval();
    if(tri_oid < min_tri_oid) min_tri_oid = tri_oid; 
    tri_tuple->DeleteIfAllowed(); 
  }
//  cout<<"min tri oid "<<min_tri_oid<<endl; 
  min_tri_oid--; 
  min_tri_oid_1 = min_tri_oid; 
  assert(min_tri_oid_1 >= 0); 
//  cout<<"b-tree on edge is finished....."<<endl;

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* tri_tuple = node_rel->GetTuple(i, false);
    int tri_oid = ((CcInt*)tri_tuple->GetAttribute(OID))->GetIntval();
    tri_tuple->DeleteIfAllowed(); 

    CcInt* nodeid = new CcInt(true, tri_oid);

    BTreeIterator* btree_iter1 = btree_node_oid1->ExactMatch(nodeid);
    int start = adj_list.Size();
//    cout<<"start "<<start<<endl;
    while(btree_iter1->Next()){
      Tuple* edge_tuple = edge_rel->GetTuple(btree_iter1->GetId(), false);
      int oid = ((CcInt*)edge_tuple->GetAttribute(OIDSECOND))->GetIntval();
//      adj_list.Append(oid);
      adj_list.Append(oid - min_tri_oid_1);//actually, it is node tuple id >= 1
      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter1;

    BTreeIterator* btree_iter2 = btree_node_oid2->ExactMatch(nodeid);
    while(btree_iter2->Next()){
      Tuple* edge_tuple = edge_rel->GetTuple(btree_iter2->GetId(), false);
      int oid = ((CcInt*)edge_tuple->GetAttribute(OIDFIRST))->GetIntval();
//      adj_list.Append(oid);
      adj_list.Append(oid - min_tri_oid_1);//actually, it is node tuple id >= 1
      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter2;
    int end = adj_list.Size();
    entry_adj_list.Append(ListEntry(start, end));

    delete nodeid;
  }

  delete btree_node_oid1;
  delete btree_node_oid2;

/*  for(int i = 0;i < entry_adj_list.Size();i++){
      cout<<"i "<<i + 1<<endl;
      ListEntry list_entry;
      entry_adj_list.Get(i, list_entry);
      int low = list_entry.low;
      int high = list_entry.high;
      int j = low;
      cout<<"adjaency list low"<<low<<"high "<<high<<endl;
      while(j < high){
        int oid;
        adj_list.Get(j, oid);
        cout<<"oid "<<oid<<" ";
        j++;
      }
      cout<<endl;
  }*/
  
}

/*
sort the dual graph node by the bounding box of the region (triangle)

*/
void DualGraph::LoadSortNode(Relation* r1)
{
 
  ListExpr ptrList1 = listutils::getPtrList(r1);
  string strQuery = "(consume(sortby(feed(" + NodeTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + ")))((Pavement asc))))";

//  cout<<strQuery<<endl;
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel_sort = (Relation*)xResult.addr;


//   for(int i = 1;i <= node_rel_sort->GetNoTuples();i++){
//     Tuple* tuple = node_rel_sort->GetTuple(i, false);
//     int oid = ((CcInt*)tuple->GetAttribute(OID))->GetIntval();
//     cout<<"oid "<<oid<<endl;
//     tuple->DeleteIfAllowed();
//   }

  ListExpr ptrList2 = listutils::getPtrList(node_rel_sort);

  strQuery = "(bulkloadrtree(addid(feed (" + NodeTypeInfo +
         " (ptr " + nl->ToString(ptrList2) + ")))) Pavement)";

  QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, xResult );
  assert ( QueryExecuted );
  rtree_node = ( R_Tree<2,TupleId>* ) xResult.addr;

}

/*
find all triangles that intersect the input line and return all tid of these
traingles 

*/
void DualGraph::LineIntersectTri(Line* l, vector<Line>& line_list)
{
    DFTraverse(rtree_node, rtree_node->RootRecordId(), l, line_list);
}

/*
Using depth first method to travese the R-tree to find all triangles
intersecting the line and record the triangle tuple id 

*/

void DualGraph::DFTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* line, vector<Line>& line_list)
{
  const double delta_dist = 0.001;
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = node_rel_sort->GetTuple(e.info, false);
              Region* candi_reg =
                     (Region*)dg_tuple->GetAttribute(DualGraph::PAVEMENT);

              Line* res = new Line(0);
              line->Intersection(*candi_reg, *res);

              if(res->Length() > delta_dist){
                line_list.push_back(*res);
              }
              delete res;
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(e.box.Intersects(line->BoundingBox())){
                DFTraverse(rtree, e.pointer, line, line_list);
            }
      }
  }
  delete node;

}

/*
find which triangle contains the point 

*/
int DualGraph::GetTriId_OfPoint(Point& loc)
{
    vector<int> tri_oid_list;
    DFTraverse2(rtree_node, rtree_node->RootRecordId(), loc, tri_oid_list);
    if(tri_oid_list.size() > 0) return tri_oid_list[0];
    else
      return -1;
}

/*
find which triangle contains the point 

*/
void DualGraph::DFTraverse2(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Point& loc, vector<int>& tri_oid_list)
{
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = node_rel_sort->GetTuple(e.info, false);
              Region* candi_reg =
                     (Region*)dg_tuple->GetAttribute(PAVEMENT);

              if(loc.Inside(*candi_reg)){
                  int oid = ((CcInt*)dg_tuple->GetAttribute(OID))->GetIntval();
                  tri_oid_list.push_back(oid);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc.Inside(e.box)){
                DFTraverse2(rtree, e.pointer, loc, tri_oid_list);
            }
      }
  }
  delete node;

}

/*
find all triangles that their distances to the input point is smaller than 
the given distance threshold 

*/
void DualGraph::DFTraverse3(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Point& loc, vector<int>& tri_oid_list, double dist)
{
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = node_rel_sort->GetTuple(e.info, false);
              Region* candi_reg =
                     (Region*)dg_tuple->GetAttribute(PAVEMENT);

              if(candi_reg->Distance(loc) < dist){
                  tri_oid_list.push_back(e.info);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(loc.Distance(e.box) < dist){
                DFTraverse3(rtree, e.pointer, loc, tri_oid_list, dist);
            }
      }
  }
  delete node;

}

/*
calculate a path connecting two cells for generating metro routes 

*/
void DualGraph::Path_Weight(int start, int end, vector<int>& path)
{
//  cout<<"start "<<start<<" end "<<end<<endl; 
  
  vector<bool> area_flag;
  for(int i = 1;i <= node_rel->GetNoTuples();i++)
    area_flag.push_back(false);

  area_flag[start - 1] = true;

  ////////////////shortest path algorithm///////////////////////
  priority_queue<SPath_elem> path_queue;
  vector<SPath_elem> expand_path;
  
  path_queue.push(SPath_elem(-1, 0, start, 1));
  expand_path.push_back(SPath_elem(-1, 0, start, 1));
  bool find = false;
  SPath_elem dest;//////////destination
  
   while(path_queue.empty() == false){
    SPath_elem top = path_queue.top();
    path_queue.pop();
//    top.Print();

    if(top.tri_index == end){
//       cout<<"find the path"<<endl;
       find = true;
       dest = top;
       break;
    }
   ////////find its adjacecy element, and push them into queue and path//////
    vector<int> adj_list;
    FindAdj(top.tri_index, area_flag, adj_list);

    int pos_expand_path = top.cur_index;
    for(unsigned int i = 0;i < adj_list.size();i++){
      int expand_path_size = expand_path.size();
      path_queue.push(SPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.weight+1));
      expand_path.push_back(SPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.weight+1));
    }


     area_flag[top.tri_index - 1] = true;

  }

  if(find){
    vector<int> path_record;
    while(dest.prev_index != -1){
//      sleeve.push_back(triangles[dest.tri_index]);
      path_record.push_back(dest.tri_index);
      dest = expand_path[dest.prev_index];
    }
//    sleeve.push_back(triangles[dest.tri_index]);
    path_record.push_back(dest.tri_index);

    for(int i = path_record.size() - 1;i >= 0;i--){
//      cout<<"oid "<<path_record[i]<<endl;
      path.push_back(path_record[i]);
    }
  }

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
Walk_SP::Walk_SP()
{
  dg = NULL;
  vg = NULL;
  rel1 = NULL;
  rel2 = NULL;
  count = 0;
  resulttype = NULL;
  rel3 = NULL;
  rel4 = NULL;
  btree = NULL;
}
Walk_SP::Walk_SP(DualGraph* g1, VisualGraph* g2, Relation* r1,
Relation* r2):dg(g1), vg(g2),rel1(r1), rel2(r2), count(0),
resulttype(NULL), rel3(NULL), rel4(NULL), btree(NULL)
{

}

Walk_SP:: ~Walk_SP()
{
  if(resulttype != NULL) delete resulttype;
}




/*
using visiblity graph to find the shortest path
first connects the start and end point to the VG, and then applies A star
algorithm
1. (rel (tuple ((v1 int) (v2 int) (v3 int) (centroid point)))):
for each triangle, we represent it by its three vertices id
the value of v1 v2 and v3 is the tuple id of the vertex in visibility graph 
with v1, v2 and v3, we can get the point of the vertex

2. (rel (tuple ((vid int) (triid int))))
it records the vertex id and the triangle id

these two parameters are used to find all triangles that a vertex belongs to 

*/
void Walk_SP::WalkShortestPath(Line* res)
{
//  cout<<"WalkShortestPath"<<endl;
  if(rel1->GetNoTuples() != 1 || rel2->GetNoTuples() != 1){
    cout<<"input query relation is not correct"<<endl;
    return;
  }
  Tuple* t1 = rel1->GetTuple(1, false);
  int oid1 = ((CcInt*)t1->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* p1 = (Point*)t1->GetAttribute(VisualGraph::QLOC2);
  Point loc1(*p1);
  t1->DeleteIfAllowed(); 
  
  Tuple* t2 = rel2->GetTuple(1, false);
  int oid2 = ((CcInt*)t2->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* p2 = (Point*)t2->GetAttribute(VisualGraph::QLOC2);
  Point loc2(*p2);
  t2->DeleteIfAllowed(); 

//  cout<<"tri_id1 "<<oid1<<" tri_id2 "<<oid2<<endl;
//  cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl;
  oid1 -= dg->min_tri_oid_1;
  oid2 -= dg->min_tri_oid_1; 

  int no_node_graph = dg->No_Of_Node();
  if(oid1 < 1 || oid1 > no_node_graph){
    cout<<"loc1 does not exist"<<endl;
    return;
  }
  if(oid2 < 1 || oid2 > no_node_graph){
    cout<<"loc2 does not exist"<<endl;
    return;
  }
  if(AlmostEqual(loc1,loc2)){
    cout<<"start location equals to end location"<<endl;
    return;
  }
  Tuple* tuple1 = dg->GetNodeRel()->GetTuple(oid1, false);
  Region* reg1 = (Region*)tuple1->GetAttribute(DualGraph::PAVEMENT);
  
  CompTriangle* ct1 = new CompTriangle(reg1);
  assert(ct1->PolygonConvex()); 
  delete ct1; 
  
  if(loc1.Inside(*reg1) == false){
    tuple1->DeleteIfAllowed();
    cout<<"point1 is not inside the polygon"<<endl;
    return;
  }
  Tuple* tuple2 = dg->GetNodeRel()->GetTuple(oid2, false);
  Region* reg2 = (Region*)tuple2->GetAttribute(DualGraph::PAVEMENT);

  CompTriangle* ct2 = new CompTriangle(reg2);
  assert(ct2->PolygonConvex()); 
  delete ct2; 
  
  if(loc2.Inside(*reg2) == false){
    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();
    cout<<"point2 is not inside the polygon"<<endl;
    return;
  }
  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();
  
  ////////////////////////////////////////////////////
  ////////debuging euclidean connection//////////////
//   oid1 = 34450 - dg->min_tri_oid_1;
//   oid2 = 34430 - dg->min_tri_oid_1;
//   loc1.Set(24164.01, 15711.896);
//   loc2.Set(24170.12, 15720.85);

  //////////////////////////////////////////////////
  
  WalkShortestPath2(oid1, oid2, loc1, loc2, res);

}

/*
test shortest path for pedestrian 

*/
void Walk_SP::TestWalkShortestPath(int tid1, int tid2)
{
//  cout<<"WalkShortestPath"<<endl;
  clock_t start, finish;
  start = clock();
  
  Tuple* t1 = rel1->GetTuple(tid1, false);
  int oid1 = ((CcInt*)t1->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* p1 = (Point*)t1->GetAttribute(VisualGraph::QLOC2);
  Point loc1(*p1);

  Tuple* t2 = rel2->GetTuple(tid2, false);
  int oid2 = ((CcInt*)t2->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* p2 = (Point*)t2->GetAttribute(VisualGraph::QLOC2);
  Point loc2(*p2);

//  cout<<"tri_id1 "<<oid1<<" tri_id2 "<<oid2<<endl;
//  cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl;
  oid1 -= dg->min_tri_oid_1;
  oid2 -= dg->min_tri_oid_1; 

  int no_node_graph = dg->No_Of_Node();
  if(oid1 < 1 || oid1 > no_node_graph){
    cout<<"loc1 does not exist"<<endl;
    return;
  }
  if(oid2 < 1 || oid2 > no_node_graph){
    cout<<"loc2 does not exist"<<endl;
    return;
  }
  if(AlmostEqual(loc1,loc2)){
    cout<<"start location equals to end location"<<endl;
    return;
  }
  Tuple* tuple1 = dg->GetNodeRel()->GetTuple(oid1, false);
  Region* reg1 = (Region*)tuple1->GetAttribute(DualGraph::PAVEMENT);

  CompTriangle* ct1 = new CompTriangle(reg1);
  assert(ct1->PolygonConvex()); 
  delete ct1; 
  
  if(loc1.Inside(*reg1) == false){
    tuple1->DeleteIfAllowed();
    cout<<"point1 is not inside the polygon"<<endl;
    return;
  }
  Tuple* tuple2 = dg->GetNodeRel()->GetTuple(oid2, false);
  Region* reg2 = (Region*)tuple2->GetAttribute(DualGraph::PAVEMENT);

  CompTriangle* ct2 = new CompTriangle(reg2);
  assert(ct2->PolygonConvex()); 
  delete ct2; 
  
  if(loc2.Inside(*reg2) == false){
    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();
    cout<<"point2 is not inside the polygon"<<endl;
    return;
  }
  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();
  
  if(oid1 == oid2){/////////////start and end point are located on the same tri
      Line* res = new Line(0);
      res->StartBulkLoad();
      /////////////////////////////////////////////////////
      HalfSegment hs;
      hs.Set(true, loc1, loc2);
      hs.attr.edgeno = 0;
      *res += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *res += hs;
      /////////////////////////////////////////////////////
      res->EndBulkLoad(); 
      oids1.push_back(tid1); //vertex id 
      oids2.push_back(tid2); //vertex id 
      path.push_back(*res);
//   double len = res->Length(); 
// printf("Euclidean length: %.4f Walk length: %.4f\n",loc1.Distance(loc2),len);
      delete res; 
      return; 
  }

  ///////////////////////////////////////////////////////////////////////
  ///////////////////check Eudlidean Connection//////////////////////////
  //////////////////////////////////////////////////////////////////////

  if(EuclideanConnect2(oid1, loc1, oid2, loc2)){
      Line* res = new Line(0);
      res->StartBulkLoad();
      /////////////////////////////////////////////////////
      HalfSegment hs;
      hs.Set(true, loc1, loc2);
      hs.attr.edgeno = 0;
      *res += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *res += hs;
      /////////////////////////////////////////////////////
      res->EndBulkLoad(); 
      oids1.push_back(tid1); //vertex id 
      oids2.push_back(tid2); //vertex id 
      path.push_back(*res);
      delete res;
//    double len = res->Length(); 
// printf("Euclidean length: %.4f Walk length: %.4f\n",loc1.Distance(loc2),len);
    return; 
  }

  ///////////////////////////////////////////////////////////////////////
  priority_queue<WPath_elem> path_queue;
  vector<WPath_elem> expand_path;
  ////////////////find all visibility nodes to start node/////////
  ///////connect them to the visibility graph/////////////////////
  VGraph* vg1 = new VGraph(dg, NULL, rel3, vg->GetNodeRel());
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
//    cout<<"vnode id ";
    for(unsigned int i = 0;i < vg1->oids1.size();i++){
//      cout<<vg1->oids1[i]<<" ";
      int expand_path_size = expand_path.size();
      double d = loc1.Distance(vg1->p_list[i]);
      w = d + vg1->p_list[i].Distance(loc2);
      path_queue.push(WPath_elem(prev_index, expand_path_size,
                      vg1->oids1[i], w, vg1->p_list[i], d));
      expand_path.push_back(WPath_elem(prev_index, expand_path_size,
                      vg1->oids1[i], w, vg1->p_list[i], d));
    }
  }
//  cout<<endl;
  delete vg1;
  ////////////////find all visibility nodes to the end node/////////
  VGraph* vg2 = new VGraph(dg, NULL, rel3, vg->GetNodeRel());
  vg2->GetVisibilityNode(oid2, loc2);

  assert(vg2->oids1.size() == vg2->p_list.size());
  //if the end node equals to triangle vertex.
  //it can be connected by adjacency list
  //we don't conenct it to the visibility graph
  if(vg2->oids1.size() == 1){
//      cout<<"end point id "<<vg2->oids1[0]<<endl;
  }
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

        if(AlmostEqual(top.loc, loc2)){
//          cout<<"find the path"<<endl;
          find = true;
          dest = top;
          break;
        }
        //do not consider the start point
        //if it is equal to the triangle vertex
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
    Line* l =  new Line(0);
    l->StartBulkLoad();
    while(dest.prev_index != -1){
      Point p1 = dest.loc;
      dest = expand_path[dest.prev_index];
      Point p2 = dest.loc;
      /////////////////////////////////////////////////////
      HalfSegment hs;
      hs.Set(true, p1, p2);
      hs.attr.edgeno = 0;
      *l += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *l += hs;
      /////////////////////////////////////////////////////
    }
    oids1.push_back(tid1); //vertex id 
    oids2.push_back(tid2); //vertex id 

    l->EndBulkLoad();
    path.push_back(*l);
    len = l->Length();
    delete l;

    finish = clock();
    printf("tid1: %d, tid2: %d, Euclidean: %.4f Walk: %.4f Time: %f\n",
           tid1, tid2, loc1.Distance(loc2),len, 
           (double)(finish - start) / CLOCKS_PER_SEC);
  }
}

/*
called by WalkShortestPath

*/
void Walk_SP::WalkShortestPath2(int oid1, int oid2, Point loc1, Point loc2,
                                Line* res)
{
//  cout<<"WalkShortestPath2"<<endl;
//  cout<<"tri_id1 "<<oid1<<" tri_id2 "<<oid2<<endl;
//  cout<<"loc1 "<<loc1<<" loc2 "<<loc2<<endl;

  if(oid1 == oid2){/////////////start and end point are located on the same tri
      res->StartBulkLoad();
      /////////////////////////////////////////////////////
      if(!AlmostEqual(loc1, loc2)){/////////two locations are equal 
        HalfSegment hs;
        hs.Set(true, loc1, loc2);
        hs.attr.edgeno = 0;
        *res += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        *res += hs;
      }
      /////////////////////////////////////////////////////
      res->EndBulkLoad(); 
//    double len = res->Length(); 
// printf("Euclidean length: %.4f Walk length: %.4f\n",loc1.Distance(loc2),len);
    return; 
  }
  ///////////////////////////////////////////////////////////////////////
  ///////////////////check Eudlidean Connection//////////////////////////
  ///////////////////////////////////////////////////////////////////////
//   clock_t start, finish;
//   start = clock();

  if(EuclideanConnect2(oid1, loc1, oid2, loc2)){
      res->StartBulkLoad();
      /////////////////////////////////////////////////////
      HalfSegment hs;
      hs.Set(true, loc1, loc2);
      hs.attr.edgeno = 0;
      *res += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *res += hs;
      /////////////////////////////////////////////////////
      res->EndBulkLoad(); 
//    double len = res->Length(); 
// printf("Euclidean length: %.4f Walk length: %.4f\n",loc1.Distance(loc2),len);
    return; 
  }
//     finish = clock();
//     printf("Time: %f\n",(double)(finish - start) / CLOCKS_PER_SEC);

  ///////////////////////////////////////////////////////////////////////
  priority_queue<WPath_elem> path_queue;
  vector<WPath_elem> expand_path;
  ////////////////find all visibility nodes to start node/////////
  ///////connect them to the visibility graph/////////////////////
  VGraph* vg1 = new VGraph(dg, NULL, rel3, vg->GetNodeRel());

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
//    cout<<"vnode id ";
    for(unsigned int i = 0;i < vg1->oids1.size();i++){
//      cout<<vg1->oids1[i]<<" ";
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
  ////////////////find all visibility nodes to the end node/////////
  VGraph* vg2 = new VGraph(dg, NULL, rel3, vg->GetNodeRel());

  vg2->GetVisibilityNode(oid2, loc2);

  assert(vg2->oids1.size() == vg2->p_list.size());
  //if the end node equals to triangle vertex.
  //it can be connected by adjacency list
  //we don't conenct it to the visibility graph
  if(vg2->oids1.size() == 1){
//      cout<<"end point id "<<vg2->oids1[0]<<endl;
  }
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

//   clock_t start, finish;
//   start = clock();

  WPath_elem dest;
  while(path_queue.empty() == false){
        WPath_elem top = path_queue.top();
        path_queue.pop();
//         cout<<"top elem "<<endl; 
//         top.Print();

        if(AlmostEqual(top.loc, loc2)){
//          cout<<"find the path"<<endl;
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
//             cout<<"neighbor "<<endl; 
//             expand_path[expand_path.size() - 1].Print(); 
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


//     finish = clock();
//     printf("Time: %f\n",(double)(finish - start) / CLOCKS_PER_SEC);

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
}

/*
check whether the two points can be directly reachable by a line 

*/
bool Walk_SP::EuclideanConnect(Point loc1, Point loc2)
{
    Line* l = new Line(0);
    l->StartBulkLoad();
    HalfSegment hs;
    hs.Set(true, loc1, loc2);
    hs.attr.edgeno = 0;
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;
    l->EndBulkLoad(); 
    double len1 = l->Length();
    double len2 = 0.0;
    DFTraverse2(dg->rtree_node, dg->rtree_node->RootRecordId(), 
                l, len2);
    delete l;

//    cout<<"l1: "<<len1<<"l2: "<<len2<<endl;

    if(fabs(len1 - len2) < 0.01) return true;

    return false;
}

/*
Using depth first method to travese the R-tree to find all triangles
intersecting the line and calculate the total intersection length

*/
void Walk_SP::DFTraverse2(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* line, double& l)
{
  const double delta_area = 0.01;
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = dg->node_rel_sort->GetTuple(e.info, false);
              Region* candi_reg =
                     (Region*)dg_tuple->GetAttribute(DualGraph::PAVEMENT);
              Line* len = new Line(0);

/*              int oid = 
                ((CcInt*)dg_tuple->GetAttribute(DualGraph::OID))->GetIntval();
              cout<<oid<<endl;*/

//              line->Intersection(*candi_reg, *len); //crashes once for Houston
              MyIntersection(*line, *candi_reg, *len); 

              if(candi_reg->Area() > delta_area && len->Length() > 0.0){
                l += len->Length();
              }
              delete len;
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(e.box.Intersects(line->BoundingBox())){
                DFTraverse2(rtree, e.pointer, line, l);
            }
      }
  }
  delete node;

}


/*
check whether the two points can be directly reachable by a line 

*/
bool Walk_SP::EuclideanConnect2(int oid1, Point loc1, int oid2, Point loc2)
{
//    cout<<"EuclideanConnect2 oid1 "<<oid1<<" oid2 "<<oid2<<endl; 
    Line* l = new Line(0);
    l->StartBulkLoad();
    HalfSegment hs;
    hs.Set(true, loc1, loc2);
    hs.attr.edgeno = 0;
    *l += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    *l += hs;
    l->EndBulkLoad(); 

    Tuple* node_tuple = dg->GetNodeRel()->GetTuple(oid1, false);
    Region* reg = (Region*)node_tuple->GetAttribute(DualGraph::PAVEMENT);
    Line* res = new Line(0);
    MyIntersection(*l, *reg, *res);
    delete res;
    node_tuple->DeleteIfAllowed();

    bool flag = true;
//    oid1 -= dg->min_tri_oid_1;
//    oid2 -= dg->min_tri_oid_1;

    int start_tri = oid1;
    int last_tri = oid1;

    bool find = false;
    while(flag){
        vector<int> adj_list;
//        cout<<"tri "<<start_tri + dg->min_tri_oid_1 <<endl;
        dg->FindAdj(start_tri, adj_list);////find all neighbor triangles
        unsigned int i = 0;
/*        cout<<start_tri + dg->min_tri_oid_1 
            <<" neighbor size "<<adj_list.size()<<endl;*/
        for(;i < adj_list.size();i++){

           Tuple* tuple = dg->GetNodeRel()->GetTuple(adj_list[i], false);
           Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);
           int tri_id = 
              ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval();
//           cout<<"neighbor id "<<tri_id<<endl;
           tri_id -= dg->min_tri_oid_1;
           if(tri_id == last_tri){
              tuple->DeleteIfAllowed();
              continue;
           }
           Line* res = new Line(0);
           MyIntersection(*l, *reg, *res);
           if(res->Length() > 0.0){

              last_tri = start_tri;
              start_tri = tri_id;
              delete res;
              tuple->DeleteIfAllowed();
              break;
           }
           delete res;
           tuple->DeleteIfAllowed();
        }
        if(i == adj_list.size()){
          flag = false;
        }else if(start_tri == oid2){
          find = true;
          flag = false;
        }
   }

    delete l;

    return find;
}

/*
Randomly generates points inside pavement polygon
1) randomly selects a polygon/polygon
2) randomly generates a number between [xmin,xmax], and [ymin,ymax]

*/

void Walk_SP::GenerateData1(int no_p)
{
  int no_node_graph = rel1->GetNoTuples();
  
//  struct timeval tval;
//  struct timezone tzone;
//  gettimeofday(&tval, &tzone);
//  srand48(tval.tv_sec);

  for (int i = 1; i <= no_p;){
//      int  m = lrand48() % no_node_graph + 1;

      int m = GetRandom() % no_node_graph + 1; 

      Tuple* tuple = rel1->GetTuple(m, false);
      int oid = ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);

      if(reg->Area() < 0.5){ //too small area, not useful for a human
          tuple->DeleteIfAllowed();
          continue;
      }

      /*neighbor triangles for testing Euclidean connection*/
//       if(!(oid == 96621 || oid == 96619 || oid == 97639)){
//           tuple->DeleteIfAllowed();
//           continue; 
//       }

      BBox<2> bbox = reg->BoundingBox();
      int xx = (int)(bbox.MaxD(0) - bbox.MinD(0)) + 1;
      int yy = (int)(bbox.MaxD(1) - bbox.MinD(1)) + 1;

      Point p1;
      Point p2;
      bool inside = false;
      int count = 1;
      while(inside == false && count <= 100){

        int x = GetRandom() % (xx*100);
        int y = GetRandom() % (yy*100);


        double coord_x = x/100.0;
        double coord_y = y/100.0;

        Coord x_cord = coord_x + bbox.MinD(0);
        Coord y_cord = coord_y + bbox.MinD(1);
        p2.Set(x_cord, y_cord);
        p1.Set(coord_x, coord_y); //set back to relative position
        //lower the precision
        Modify_Point_3(p1);
        Modify_Point_3(p2);
        inside = p2.Inside(*reg);
        count++;
      }
      if(inside){
//        oids.push_back(m);
        oids.push_back(oid); 
        q_loc1.push_back(p1);
        q_loc2.push_back(p2);
        i++;
      }
      tuple->DeleteIfAllowed();
  }
}

/*
Randomly generates points inside pavement polygon
1) randomly selects a polygon/polygon
2) randomly generates a vertex of the triangle
3) the point equals to one of triangle vertices 

*/

void Walk_SP::GenerateData2(int no_p)
{
  int no_node_graph = rel1->GetNoTuples();

  for (int i = 1; i <= no_p;i++){
      int m = GetRandom() % no_node_graph + 1; 

      Tuple* tuple = rel1->GetTuple(m, false);
      int oid = ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);
      Points* ps = new Points(0);
      reg->Vertices(ps);


      unsigned index = GetRandom() % 3;

      Point p2;
      ps->Get(index, p2);
      BBox<2> bbox = reg->BoundingBox();
      Point p1(true, p2.GetX() - bbox.MinD(0), p2.GetY() - bbox.MinD(1));

//      oids.push_back(m);
      oids.push_back(oid); 
      q_loc1.push_back(p1);//relative position in the polygon
      q_loc2.push_back(p2);//absolute position

      delete ps;
      tuple->DeleteIfAllowed();
  }
}

/*
Randomly generates points inside pavement polygon
1) randomly selects a polygon/polygon
2) randomly generates a vertex of the triangle
3) the vertex is the internal point of a triangle. not on the edge

*/

void Walk_SP::GenerateData3(int no_p)
{
  int no_node_graph = rel1->GetNoTuples();
//  struct timeval tval;
//  struct timezone tzone;

//  gettimeofday(&tval, &tzone);
//  srand48(tval.tv_sec);

 for (int i = 1; i <= no_p;){

//      int  m = lrand48() % no_node_graph + 1;

      int  m = GetRandom() % no_node_graph + 1;

      Tuple* tuple = rel1->GetTuple(m, false);
      int oid = ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval(); 
      Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);

      if(reg->Area() < 0.5){ //too small area, not useful for a human
          tuple->DeleteIfAllowed();
          continue;
      }
      BBox<2> bbox = reg->BoundingBox();
      int xx = (int)(bbox.MaxD(0) - bbox.MinD(0)) + 1;
      int yy = (int)(bbox.MaxD(1) - bbox.MinD(1)) + 1;

      Point p1;
      Point p2;
      bool inside = false;
      int count = 1;
      vector<HalfSegment> segs;
      for(int j = 0;j < reg->Size();j++){
          HalfSegment hs;
          reg->Get(j,hs);
          if(!hs.IsLeftDomPoint())continue;
          segs.push_back(hs);
      }

      while(inside == false && count <= 100){

        int x = GetRandom() % (xx*100);
        int y = GetRandom() % (yy*100); 

        double coord_x = x/100.0;
        double coord_y = y/100.0;

        Coord x_cord = coord_x + bbox.MinD(0);
        Coord y_cord = coord_y + bbox.MinD(1);
        p2.Set(x_cord, y_cord);
        p1.Set(coord_x, coord_y); //set back to relative position
        //lower the precision
        Modify_Point_3(p1);
        Modify_Point_3(p2);
        inside = p2.Inside(*reg);
        for(unsigned int k = 0;k < segs.size();k++){
            if(segs[k].Contains(p2)){
              inside = false;
              break;
            }
        }
        count++;
      }
      if(inside){
//        oids.push_back(m);
        oids.push_back(oid); 
        q_loc1.push_back(p1);
        q_loc2.push_back(p2);
        i++;
      }
      tuple->DeleteIfAllowed();
  }
}

/*
create a random point on the given triangle 

*/
bool Walk_SP::GenerateData4(int tid)
{
  if(rel1 == NULL){
    cout<<"realtion no initialize "<<endl;
    return false;
  }
  int no_node_graph = rel1->GetNoTuples();
  assert(1 <= tid && tid <= no_node_graph);

  Tuple* tuple = rel1->GetTuple(tid, false);
  int oid = ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval(); 
  Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);

  if(reg->Area() < 0.5){ //too small area, not useful for a human
      tuple->DeleteIfAllowed();
      return false;
  }


  BBox<2> bbox = reg->BoundingBox();
  int xx = (int)(bbox.MaxD(0) - bbox.MinD(0)) + 1;
  int yy = (int)(bbox.MaxD(1) - bbox.MinD(1)) + 1;

  Point p1;
  Point p2;
  bool inside = false;
  int count_tmp = 1;
  while(inside == false && count_tmp <= 100){

        int x = GetRandom() % (xx*100);
        int y = GetRandom() % (yy*100);


        double coord_x = x/100.0;
        double coord_y = y/100.0;

        Coord x_cord = coord_x + bbox.MinD(0);
        Coord y_cord = coord_y + bbox.MinD(1);
        p2.Set(x_cord, y_cord);
        p1.Set(coord_x, coord_y); //set back to relative position
        //lower the precision
        Modify_Point_3(p1);
        Modify_Point_3(p2);
        inside = p2.Inside(*reg);
        count_tmp++;
  }
  tuple->DeleteIfAllowed();
  if(inside){
//        oids.push_back(m);
        oids.push_back(oid); 
        q_loc1.push_back(p1);
        q_loc2.push_back(p2);
        return true;
  }
  
  return false;

}


/*
set pavement rid 

*/
void Walk_SP::SetPaveRid(R_Tree<2,TupleId>* rtree)
{
  
  SmiRecordId adr = rtree->RootRecordId();
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tri_tuple = rel1->GetTuple(i, false);
    int oid = ((CcInt*)tri_tuple->GetAttribute(DualGraph::OID))->GetIntval();
    Region* reg = (Region*)tri_tuple->GetAttribute(DualGraph::PAVEMENT);
    vector<int> r_id_list; 

    DFTraverse(rtree, adr, reg, r_id_list);

//    cout<<"tid "<<i <<" oid "<<oid<<endl; 

    if(r_id_list.size() == 0){
      cout<<"do not find road "<<endl; 
      cout<<"tid "<<i <<" oid "<<oid<<endl;
//      break; 
      continue;
    }

    for(unsigned int j = 0;j < r_id_list.size();j++){
        oid_list.push_back(oid);
        rid_list.push_back(r_id_list[j]); 
        reg_list.push_back(*reg); 
    }

    tri_tuple->DeleteIfAllowed(); 
  }

}

/*
Using depth first method to travese the R-tree to find the big pavement 
intersects the triangle (decomposed from the big pavement)

*/
void Walk_SP::DFTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr, Region* reg,
                  vector<int>& r_id_list)
{
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple2 = rel2->GetTuple(e.info, false);
              Region* candi_reg =
                     (Region*)dg_tuple2->GetAttribute(DualGraph::PAVEMENT);
              if(reg->Intersects(*candi_reg)){
                 int rid = 
                 ((CcInt*)dg_tuple2->GetAttribute(DualGraph::RID))->GetIntval();
                 unsigned int i = 0;
                 for(;i < r_id_list.size();i++)
                   if(r_id_list[i] == rid)break;
                 if(i == r_id_list.size())
                    r_id_list.push_back(rid);
              }
              dg_tuple2->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(e.box.Intersects(reg->BoundingBox())){
                DFTraverse(rtree, e.pointer, reg, r_id_list);
            }
      }
  }
  delete node;

}


/*
map points in pavements to gpoints 

*/
void Walk_SP::PaveLocToGP(Network* n)
{
  //rel1 query locations , rel2 dgnode 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple1 = rel1->GetTuple(i, false);
    int oid = ((CcInt*)tuple1->GetAttribute(VisualGraph::QOID))->GetIntval(); 
    Point* loc = (Point*)tuple1->GetAttribute(VisualGraph::QLOC2); 

    /////////////////find which road the pavement corresponds to////////
    CcInt* search_id = new CcInt(true, oid);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    vector<int> route_id_list;
    while(btree_iter->Next()){
        Tuple* tuple = rel2->GetTuple(btree_iter->GetId(), false);
        int rid = ((CcInt*)tuple->GetAttribute(DualGraph::RID))->GetIntval();
        route_id_list.push_back(rid); 
        tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;

//    for(unsigned int i = 0;i < route_id_list.size();i++)
//      cout<<"rid "<<route_id_list[i]<<endl; 

//    cout<<"oid "<<oid<<" rid "<<rid<<endl;

    PaveLocToGPoint(loc, n, route_id_list);//more robustness

    tuple1->DeleteIfAllowed(); 

//    break; 
  }


}

/*
for a point, location in pavement, find its mapping gpoint 

*/
struct MyPoint_Id:public MyPoint{
    int id; 
    MyPoint_Id(){}
    MyPoint_Id(const Point& p, double d, int i):
    MyPoint(p,d),id(i){}
    MyPoint_Id(const MyPoint_Id& mpi):
    MyPoint(mpi), id(mpi.id){}
    MyPoint_Id& operator=(const MyPoint_Id& mpi)
    {
        MyPoint::operator=(mpi);
        id = mpi.id; 
        return *this; 
    }
     void Print()
    {
        cout<<" loc " <<loc<<" dist "<<dist<<" id "<<id<<endl; 
    }
};


/*
pavement location to gpoint 

*/
bool Walk_SP::PaveLocToGPoint(Point* loc, Network* n, 
                              vector<int> route_id_list)
{
  vector<MyPoint_Id> mp_list; 
  for(unsigned int i = 0;i < route_id_list.size();i++){
    int rid = route_id_list[i];
    Tuple* route_tuple = n->GetRoute(rid);
    SimpleLine* l = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
//    cout<<"l : "<<l->Length()<<endl; 

    for(int j = 0;j < l->Size();j++){
      HalfSegment hs;
      l->Get(j, hs);
      if(!hs.IsLeftDomPoint())continue; 

      SpacePartition* sp = new SpacePartition();
      Point res; 
      double dist = sp->GetClosestPoint(hs, *loc, res); 
      delete sp;
      MyPoint_Id mp(res, dist, rid); 
      mp_list.push_back(mp); 
    }
    route_tuple->DeleteIfAllowed(); 
  }
  assert(mp_list.size() > 0); 
  sort(mp_list.begin(), mp_list.end());

//  cout<<"rid "<<mp_list[0].id<<" neighbor "<<mp_list[0].loc<<endl;

  Tuple* route_tuple = n->GetRoute(mp_list[0].id); 
  SimpleLine* sl = (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE); 
  bool s = 
      ((CcBool*)route_tuple->GetAttribute(ROUTE_STARTSSMALLER))->GetBoolval();
  double pos;
  bool ok = sl->AtPoint(mp_list[0].loc, s, pos);
  ////////////MyAtPoint()////////////////
  
  if(ok){
    GPoint gp(true, n->GetId(), mp_list[0].id, pos);
    route_tuple->DeleteIfAllowed();
    p_list.push_back(mp_list[0].loc); 
    gp_list.push_back(gp);
  }
  
  return ok;

}

/*
type constructors 

*/
VGraph::VGraph()
{
  dg = NULL;
  rel1 = NULL;
  rel2 = NULL;
  rel3 = NULL;
  count = 0;
  resulttype = NULL;
  vg = NULL;
}
VGraph::VGraph(DualGraph* g, Relation* r1, Relation* r2, Relation* r3):dg(g),
rel1(r1), rel2(r2), rel3(r3), count(0), resulttype(NULL), vg(NULL)
{

}
VGraph::VGraph(VisualGraph* g):dg(NULL),
rel1(NULL), rel2(NULL), rel3(NULL), count(0), resulttype(NULL), vg(g)
{

}
VGraph:: ~VGraph()
{
  if(resulttype != NULL) delete resulttype;
}
/*
get all adjacent nodes for a given node. dual graph

*/

void VGraph::GetAdjNodeDG(int oid)
{
    if(oid <= dg->min_tri_oid_1 ||
       oid > dg->min_tri_oid_1 + dg->node_rel->GetNoTuples()){
        cout<<"invalid oid "<<oid<<endl;
        cout<<"minimum oid "<<dg->min_tri_oid_1 + 1<<endl;
        cout<<"maximum oid "
            <<dg->min_tri_oid_1 + dg->node_rel->GetNoTuples()<<endl;
        return; 
    }

    cout<<"total "<< dg->GetNodeRel()->GetNoTuples()<<" nodes "<<endl;

    vector<int> adj_list;
    assert(dg->min_tri_oid_1 >= 0); 
    dg->FindAdj(oid - dg->min_tri_oid_1, adj_list);

    for(unsigned int i = 0;i < adj_list.size();i++){

      Tuple* node_tuple = dg->GetNodeRel()->GetTuple(adj_list[i], false);
      CcInt* tri_id = (CcInt*)node_tuple->GetAttribute(DualGraph::OID);
      Region* reg = (Region*)node_tuple->GetAttribute(DualGraph::PAVEMENT);
      oids1.push_back(tri_id->GetIntval());
      regs.push_back(*reg);
      node_tuple->DeleteIfAllowed();

    }

}

/*
get all adjacent nodes for a given node. visibility graph

*/

void VGraph::GetAdjNodeVG(int oid)
{
    if(oid < 1 || oid > vg->GetNodeRel()->GetNoTuples()){
      cout<<"invalid oid "<<oid<<endl;
      return;
    }
    cout<<"total "<<vg->GetNodeRel()->GetNoTuples()<<" nodes "<<endl;

    Tuple* node_tuple = vg->GetNodeRel()->GetTuple(oid, false);
    Point* query_p = (Point*)node_tuple->GetAttribute(VisualGraph::LOC);
    Point query_loc(*query_p);
    node_tuple->DeleteIfAllowed();

    vector<int> adj_list;
    vg->FindAdj(oid, adj_list);
    for(unsigned int i = 0;i < adj_list.size();i++){
      node_tuple = vg->GetNodeRel()->GetTuple(adj_list[i], false);
      CcInt* id = (CcInt*)node_tuple->GetAttribute(VisualGraph::OID);
      Point* loc = (Point*)node_tuple->GetAttribute(VisualGraph::LOC);
      oids1.push_back(id->GetIntval());
      p_list.push_back(*loc);
      ////////////////////////////////////////////////
      Line* l =  new Line(0);
      l->StartBulkLoad();
      HalfSegment hs;
      hs.Set(true, query_loc, *loc);
      hs.attr.edgeno = 0;
      *l += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *l += hs;
      l->EndBulkLoad();
      line.push_back(*l);
      delete l;
      ////////////////////////////////////////////////
      node_tuple->DeleteIfAllowed();
    }

}


/*
get the visibile points for one vertex

*/

void VGraph::GetVNodeOnVertex(int vid, Point* query_p)
{
  CcInt* vertex_id = new CcInt(true, vid);
  BTreeIterator* btreeiter = btree->ExactMatch(vertex_id);
  while(btreeiter->Next()){
      Tuple* ver_tri = rel4->GetTuple(btreeiter->GetId(), false);
      int triangle_id =
          ((CcInt*)ver_tri->GetAttribute(DualGraph::TRIID))->GetIntval();

//       cout<<"tid_start "<<tid_start<<endl; 
//       cout<<"triangle_id "<<triangle_id<<endl;
      ///////////for debuging///////////////////////
      assert((triangle_id - dg->min_tri_oid_1) >= 1); 
      triangle_id = triangle_id - dg->min_tri_oid_1; 
      assert(triangle_id >= 1 && triangle_id <= rel2->GetNoTuples()); 
      Tuple* tri_tuple = rel2->GetTuple(triangle_id, false);
      int v1 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
      int v2 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
      int v3 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
      tri_tuple->DeleteIfAllowed();

      vector<int> vid_list;
      if(v1 == vid){
          vid_list.push_back(v2);
          vid_list.push_back(v3);
          for(unsigned int i = 0;i < vid_list.size();i++){
              unsigned int j = 0;
              for(;j < oids1.size();j++){
                if(vid_list[i] == oids1[j]) break;
              }
              if(j == oids1.size()){
                 oids1.push_back(vid_list[i]);
              }
          }
          GetVisibleNode2(triangle_id, query_p, 3);

      }else if(v2 == vid){
          vid_list.push_back(v1);
          vid_list.push_back(v3);
          for(unsigned int i = 0;i < vid_list.size();i++){
              unsigned int j = 0;
              for(;j < oids1.size();j++){
                if(vid_list[i] == oids1[j]) break;
              }
              if(j == oids1.size()){
                 oids1.push_back(vid_list[i]);
              }
          }
          GetVisibleNode2(triangle_id, query_p, 2);
      }else if(v3 == vid){
          vid_list.push_back(v1);
          vid_list.push_back(v2);
          for(unsigned int i = 0;i < vid_list.size();i++){
              unsigned int j = 0;
              for(;j < oids1.size();j++){
                if(vid_list[i] == oids1[j]) break;
              }
              if(j == oids1.size()){
                 oids1.push_back(vid_list[i]);
              }
          }
          GetVisibleNode2(triangle_id, query_p, 1);
      }else assert(false);

      ver_tri->DeleteIfAllowed();
  }
  delete btreeiter;
  delete vertex_id;
}


/*
create the edge relation for the visibility graph

*/
void VGraph::GetVGEdge()
{
//  cout<<dg->min_tri_oid_1<<endl; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){

      Tuple* vertex_tuple = rel1->GetTuple(i, false);
      int vid =
          ((CcInt*)vertex_tuple->GetAttribute(VisualGraph::OID))->GetIntval();
      Point* p1 = (Point*)vertex_tuple->GetAttribute(VisualGraph::LOC);

//       if(vid != 1200){
//         vertex_tuple->DeleteIfAllowed();
//         continue; 
//       }

      GetVNodeOnVertex(vid, p1);
      for(unsigned int j = 0;j < oids1.size();j++){
          oids2.push_back(vid);
          oids3.push_back(oids1[j]);
          /////////////////////////
          Tuple* loc_tuple2 = rel3->GetTuple(oids1[j], false);
          Point* p2 = (Point*)loc_tuple2->GetAttribute(VisualGraph::LOC);
          /////////////////////////
          Line* l =  new Line(0);
          l->StartBulkLoad();
          HalfSegment hs;
          hs.Set(true, *p1, *p2);
          hs.attr.edgeno = 0;
          *l += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          *l += hs;
          l->EndBulkLoad();
          line.push_back(*l);
          delete l;
          loc_tuple2->DeleteIfAllowed();
      }
      vertex_tuple->DeleteIfAllowed();
      oids1.clear();
  }

}

inline bool VGraph::Collineation(Point& p1, Point& p2, Point& p3)
{
      if(AlmostEqual(p1.GetX(), p2.GetX())){
          if(AlmostEqual(p2.GetX(), p3.GetX())) return true;
      }
      if(AlmostEqual(p1.GetY(), p2.GetY())){
          if(AlmostEqual(p2.GetY(), p3.GetY())) return true;
      }
      double k1 = (p1.GetY() - p2.GetY())/(p1.GetX() - p2.GetX());
      double k2 = (p2.GetY() - p3.GetY())/(p2.GetX() - p3.GetX());
      if(AlmostEqual(k1, k2)) return true;
      return false;
}

/*
it checks whether vertex (vp) is inside the clamp structure constructed by
vertex and two points endp1, endp2

*/
bool VGraph::CheckVisibility1(Clamp& clamp, Point& checkp, int vp)
{
//    cout<<"CheckVisibility1()"<<endl;
//    clamp.Print();
//    cout<<"checkp " <<checkp<<endl;

    bool visibility;
    /////////it checks whether the angle1 + angle2 is equal to clamp angle
    //but it has some drawbacks that when angle1(2) is 0
    //it can't well determine whether checkp is inside the clamp or not

/*    SpacePartition* sp = new SpacePartition();

    double angle1 = sp->GetAngle(clamp.apex, clamp.foot1, checkp);
    double angle2 = sp->GetAngle(clamp.apex, checkp, clamp.foot2);

    cout<<setprecision(8);
    cout<<"angle1 "<<angle1<<" angle2 "<<angle2<<endl;
    cout<<"clamp angle "<<clamp.angle<<endl;

    //it is possible that d1 is smaller than d2 and in this case
    //the angle is also very small. it occurs that when the two feet are very
    //very,very far away from the apex. so, the angle is almost equal to 0.

    if(AlmostEqual(angle1, 0.0)){
        delete sp;
        return false;
    }
    if(AlmostEqual(angle2, 0.0)){
        delete sp;
        return false;
    }
    const double pi = 3.14159;
    const double delta = 1.0*pi/360.0; //deviation 1 degree
    ////////////////////////////////////////////////////////////////////////
    //  if the degree is smaller than 1 degree. we think it is the same  ///
    ///////////////////////////////////////////////////////////////////////

    if(fabs(clamp.angle - (angle1 + angle2) ) < delta)
      visibility = true;
    else
      visibility = false;

    delete sp; */

   ////it checks whether the polygon constructed by apex, foot1(2),
   ///checkp,foot2(1) is a convex or concave.
   // if checkp is outside the angle formed by two line (apex,foot1)(apex,foot2)
   //then, the angle apex-foot1(2)-checkp is larger than 180.

    vector<Point> ps;
    ps.push_back(clamp.apex);
    ps.push_back(clamp.foot1);
    ps.push_back(checkp);
    ps.push_back(clamp.foot2);
    CompTriangle* ct = new CompTriangle();
    visibility = ct->IsConvex(ps);//convex. checkp is inside
    delete ct;

    if(visibility){
      //it is impossible that clamp.apex, foot1(2), checkp are on the same line
      //and checkp locates between clamp.apex and foot1(2)
      //because in this case, checkp should be found first
      //because we start from the triangle that queryp locates and the inside
      //checking condition

      bool flag1 = Collineation(clamp.apex, clamp.foot1, checkp);
      bool flag2 = Collineation(clamp.apex, clamp.foot2, checkp);
//      cout<<"flag1 "<<flag1<<" flag2 "<<flag2<<endl;
      if(flag1 || flag2) return false;
      return true;
    }else return false;

}
/*
Get the intersection point between halfsegment (p1,p2) and
radial p3->p4, store the results in ip
if it has, returns true, otherwise returns false

*/

bool VGraph::GetIntersectionPoint(Point& p1, Point& p2,
                                 Clamp& clamp, Point& ip, bool flag)
{
  Point p3;
  Point p4;
  if(flag){
      p3 = clamp.apex;
      p4 = clamp.foot1;
  }else{
      p3 = clamp.apex;
      p4 = clamp.foot2;
  }

  Coord x, y;
  //get the intersection point
  if(AlmostEqual(p1.GetX(), p2.GetX())){
    if(AlmostEqual(p3.GetX(), p4.GetX())) return false; //parallel

    double a = (p3.GetY() - p4.GetY())/(p3.GetX() - p4.GetX());
    double b = p3.GetY() - a*p3.GetX();
    x = p1.GetX();
    y = a*x + b;
  }else if(AlmostEqual(p1.GetY(), p2.GetY())){
    if(AlmostEqual(p3.GetY(), p4.GetY())) return false; //parallel
    if(AlmostEqual(p3.GetX(), p4.GetX())){
      x = p3.GetX();
      y = p1.GetY();
    }else{
      double a = (p3.GetY() - p4.GetY())/(p3.GetX() - p4.GetX());
      double b = p3.GetY() - a*p3.GetX();
      y = p1.GetY();
      x = (y - b)/a;
    }
  }else{
        double a1 = (p1.GetY() - p2.GetY())/(p1.GetX() - p2.GetX());
        double b1 = p1.GetY() - a1*p1.GetX();
        if(AlmostEqual(p3.GetX(), p4.GetX())){
            x = p3.GetX();
            y = a1*x + b1;
        }else{
          double a2 = (p3.GetY() - p4.GetY())/(p3.GetX() - p4.GetX());
          double b2 = p3.GetY() - a2*p3.GetX();

          if(AlmostEqual(a1, a2)) return false;//parallel
          x = (b2-b1)/(a1-a2);
          y = a1*x + b1;
        }
  }
  ip.Set(x, y);
  //check whether it is available
  HalfSegment hs;
  hs.Set(true,p1,p2);
  if(hs.Contains(ip)){
    if(AlmostEqual(p3.GetX(), p4.GetX())){
        if(p3.GetY() < p4.GetY()){
            if(AlmostEqual(y, p3.GetY()) || y > p3.GetY()) return true;
              else
                return false;
        }else{
            if(AlmostEqual(y, p3.GetY()) || y < p3.GetY()) return true;
              else
                return false;
        }
    }else{
        if(p3.GetX() < p4.GetX()){
          if(x > p3.GetX() || AlmostEqual(x, p3.GetX())) return true;
          else
            return false;
        }else{
          if(x < p3.GetX() || AlmostEqual(x, p3.GetX())) return true;
          else
            return false;
        }
    }
  }else
    return false;
}

/*
it checks whether the segment(checkp1, checkp2) is inside the clamp structure
Different from CheckVisibility1, now if checkp is on the line (apex,foot1) or
(apex, foot2), it might also return false.
one is outside and one is on the line. it returns false.

*/

bool VGraph::CheckVisibility2(Clamp& clamp, Point& checkp1, Point& checkp2)
{
//    cout<<"CheckVisibility2()"<<endl;
//    clamp.Print();
//    cout<<"checkp1 "<<checkp1<<" checkp2 "<<checkp2<<endl;

    //checkp1 is outside clamp
    //it checks whether seg(checkp1,checkp2) is outside clamp
    bool visibility;
    //get the intersection point between seg and clamp
    vector<Point> intersection_points;

    Point p1;
    if(GetIntersectionPoint(checkp1, checkp2, clamp, p1, true))
        intersection_points.push_back(p1);
    Point p2;
    if(GetIntersectionPoint(checkp1, checkp2, clamp, p2, false))
        intersection_points.push_back(p2);

    //the number of intersection point
    //0  return false
    //1  if intersection point is not checkp2, return true, otherwise false
    //2  return true

//    cout<<"intersection_points.size() "<<intersection_points.size()<<endl;
    if(intersection_points.size() == 0) visibility = false;
    if(intersection_points.size() == 2) visibility = true;
    if(intersection_points.size() == 1){
//        cout<<"checkp2 "<<checkp2<<endl;
//        cout<<"intersection [0] "<<intersection_points[0]<<endl;
        if(AlmostEqual(checkp2, intersection_points[0])) visibility = true;
        else
          visibility = false;
//        assert(!AlmostEqual(checkp1, intersection_points[0]));
//  it can be equal when checkp1 is on the line but further than foot1 or foot2
    }

    return visibility;
}

struct HSNode{
  HalfSegment hs;
  HSNode* next;
  HSNode(){next=NULL;}
  HSNode(HalfSegment& seg,HSNode* pointer):hs(seg),next(pointer){}
};

/*
Check whether two halfsemgnets cross

*/
bool VGraph::MyCross(const HalfSegment& hs1, const HalfSegment& hs2)
{
  Point p1 = hs1.GetLeftPoint();
  Point p2 = hs1.GetRightPoint();

  Point p3 = hs2.GetLeftPoint();
  Point p4 = hs2.GetRightPoint();

  Coord x, y;
  Point ip;
  if(MyAlmostEqual(p1.GetX(), p2.GetX())){
    if(MyAlmostEqual(p3.GetX(), p4.GetX())) return false; //parallel

    double a = (p3.GetY() - p4.GetY())/(p3.GetX() - p4.GetX());
    double b = p3.GetY() - a*p3.GetX();
    x = p1.GetX();
    y = a*x + b;
  }else if(MyAlmostEqual(p1.GetY(), p2.GetY())){
    if(MyAlmostEqual(p3.GetY(), p4.GetY())) return false; //parallel
    if(MyAlmostEqual(p3.GetX(), p4.GetX())){
      x = p3.GetX();
      y = p1.GetY();
    }else{
      double a = (p3.GetY() - p4.GetY())/(p3.GetX() - p4.GetX());
      double b = p3.GetY() - a*p3.GetX();
      y = p1.GetY();
      x = (y - b)/a;
    }
  }else{
        double a1 = (p1.GetY() - p2.GetY())/(p1.GetX() - p2.GetX());
        double b1 = p1.GetY() - a1*p1.GetX();
        if(MyAlmostEqual(p3.GetX(), p4.GetX())){
            x = p3.GetX();
            y = a1*x + b1;
        }else{
          double a2 = (p3.GetY() - p4.GetY())/(p3.GetX() - p4.GetX());
          double b2 = p3.GetY() - a2*p3.GetX();

          if(MyAlmostEqual(a1, a2)) return false;//parallel
          x = (b2-b1)/(a1-a2);
          y = a1*x + b1;
        }
  }

  if(MyAlmostEqual(x,p1.GetX()) && MyAlmostEqual(y,p1.GetY())) return false;
  if(MyAlmostEqual(x,p2.GetX()) && MyAlmostEqual(y,p2.GetY())) return false;
  if(MyAlmostEqual(x,p3.GetX()) && MyAlmostEqual(y,p3.GetY())) return false;
  if(MyAlmostEqual(x,p4.GetX()) && MyAlmostEqual(y,p4.GetY())) return false;
  ip.Set(x,y);
  if(hs1.Contains(ip) && hs2.Contains(ip))
    return true;
  return false;
}

/*
check whether the halfsegment is inside the triangle region

1) it can't use the method that first union all triangles and
region.contain(line). because it doesn't work for some very small triangles
(edges are very close to each other). it reports redundant segments.
2) it can't use the method that it compares the length of the intersection
line between (hs and all triangle regions). because if the point is very close
to the triangle edge. it can't create a segment (the length is very small)

*/
bool VGraph::PathContainHS(vector<int> tri_list, HalfSegment hs)
{
//  cout<<"PathContainHS() "<<"HS "<<hs<<endl;

/*  double len1 = hs.Length();
  assert(tri_list.size() > 0);
  Line* l = new Line(0);
  l->StartBulkLoad();
  hs.attr.edgeno = 0;
  *l += hs;
  hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
  *l += hs;
  l->EndBulkLoad();
  double len2 = 0.0;
  for(unsigned int i = 0;i < tri_list.size();i++){
    cout<<"tri id "<<tri_list[i]<<endl;
    Tuple* tri = dg->GetNodeRel()->GetTuple(tri_list[i], false);
    Region* reg = (Region*)tri->GetAttribute(DualGraph::PAVEMENT);
    if(reg->Area() > 0.0){
      Line* inter_l = new Line(0);
      l->Intersection(*reg,*inter_l);//it has numeric problem
      if(inter_l->Size() > 0){
        cout<<"inter_l length "<<inter_l->Length()<<endl;
        len2 += inter_l->Length();
      }
      delete inter_l;
    }
    tri->DeleteIfAllowed();
  }
  delete l;
  cout<<"len1 "<<len1<<" len2 "<<len2<<endl;

  if(AlmostEqual(len1, len2)) return true;
  return false;*/

  ///////////////////another implementation ////////////////////////////
  ////////the method above has numeric problem ////////////////////////

  HSNode* head = new HSNode();
  HSNode* prev = head;
  HSNode* cur = prev->next;

  Tuple* tri = dg->GetNodeRel()->GetTuple(tri_list[0], false);
  Region* reg = (Region*)tri->GetAttribute(DualGraph::PAVEMENT);
  for(int i = 0;i < reg->Size();i++){
      HalfSegment hs;
      reg->Get(i, hs);
      if(!hs.IsLeftDomPoint()) continue;
      HSNode* hsnode = new HSNode(hs,NULL);
      cur = hsnode;
      prev->next = cur;
      prev = cur;
      cur = cur->next;
  }
  tri->DeleteIfAllowed();


  for(unsigned int i = 1;i < tri_list.size();i++){

    tri = dg->GetNodeRel()->GetTuple(tri_list[i], false);
    reg = (Region*)tri->GetAttribute(DualGraph::PAVEMENT);
    for(int j = 0;j < reg->Size();j++){
      HalfSegment hs;
      reg->Get(j, hs);
      if(!hs.IsLeftDomPoint())continue;
      prev = head;
      cur = prev->next;
      Point lp1 = hs.GetLeftPoint();
      Point rp1 = hs.GetRightPoint();
      bool insert = true;
      while(cur != NULL && insert){//to find all block segments
        Point lp2 = cur->hs.GetLeftPoint();
        Point rp2 = cur->hs.GetRightPoint();
        if((AlmostEqual(lp1,lp2) && AlmostEqual(rp1, rp2))||
           (AlmostEqual(lp1,rp2) && AlmostEqual(rp1, lp2))){ //delete it
          HSNode* temp = cur;
          cur = cur->next;
          prev->next = cur;
          delete temp;
          insert = false;
        }else{
          prev = cur;
          cur = cur->next;
        }
      }
      if(insert){ //insert the halfsegment
          assert(cur == NULL);
          HSNode* node = new HSNode(hs, NULL);
          cur = node;
          prev->next = cur;
      }
    }
    tri->DeleteIfAllowed();
  }

  bool cross = false;
  prev = head;
  cur = prev->next;

  while(cur != NULL){
//      cout<<"cur hs "<<cur->hs<<endl;
      if(cross == false)
        cross = MyCross(hs,cur->hs);
//      cout<<"cross "<<MyCross(hs,cur->hs)<<endl;
      HSNode* node = prev;
      prev = cur;
      cur = cur->next;
      delete node;
  }

  return !cross;
}

/*
Depth first searching on the dual graph to find all visible vertices

*/
void VGraph::DFTraverse(int tri_id, Clamp& clamp, int pre_id, int type1)
{
//    cout<<"DFTraverse()"<<endl;
//    cout<<"tri_id "<<tri_id<<" pre_id "<<pre_id<<" type "<<type1<<endl;
//    clamp.Print();

    vector<int> adj_list;
    dg->FindAdj(tri_id, adj_list);

    //get the vertex
    Tuple* tri_tuple = rel2->GetTuple(tri_id, false);
    int v1 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
    int v2 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
    int v3 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
    tri_tuple->DeleteIfAllowed();
    Triangle tri1(v1, v2, v3);
//    cout<<"v1 "<<v1<<"v2 "<<v2<<" v3 "<<v3<<endl; 
//    cout<<"adj_list size "<<adj_list.size()<<endl;
    for(unsigned int i = 0;i < adj_list.size();i++){
//      cout<<" adj_list DF "<<adj_list[i]<<" i "<<i<<endl;
      if(adj_list[i] == pre_id) continue;
      tri_tuple = rel2->GetTuple(adj_list[i], false);
      int ver1 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
      int ver2 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
      int ver3 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
      tri_tuple->DeleteIfAllowed();
      ///////////////////////////////////////////////////////////////////////
      Tuple* loc_tuple1 = rel3->GetTuple(ver1, false);
      Point* p1 = (Point*)loc_tuple1->GetAttribute(VisualGraph::LOC);
      Point foot1(*p1);
      loc_tuple1->DeleteIfAllowed();
      Tuple* loc_tuple2 = rel3->GetTuple(ver2, false);
      Point* p2 = (Point*)loc_tuple2->GetAttribute(VisualGraph::LOC);
      Point foot2(*p2);
      loc_tuple2->DeleteIfAllowed();
      Tuple* loc_tuple3 = rel3->GetTuple(ver3, false);
      Point* p3 = (Point*)loc_tuple3->GetAttribute(VisualGraph::LOC);
      Point foot3(*p3);
      loc_tuple3->DeleteIfAllowed();
      ///////////////////////////////////////////////////////////////////////
      Triangle tri2(ver1, ver2, ver3);
      int sharetype1 = tri2.ShareEdge(tri1);
      int sharetype2 = tri1.ShareEdge(tri2);
//      cout<<"sharetype1 "<<sharetype1<<" sharetype2 "<<sharetype2<<endl;
      if(type1 != 4){
        if(sharetype2 != type1) continue;
      }

      bool visibility;
      switch(sharetype1){
        case 1:
//                cout<<"case1"<<endl;

                visibility = CheckVisibility1(clamp, foot3, ver3);
                if(visibility){
//                  cout<<"visibility"<<endl;
                  HalfSegment hs;
                  hs.Set(true, clamp.apex, foot3);
                  //split the clamp into 2

                  oids1.push_back(ver3);
                  p_list.push_back(foot3);
                  Point ip;

                  Clamp clamp1(clamp.apex, foot3, clamp.foot1);
                  if(GetIntersectionPoint(foot3,foot1,clamp1,ip,false))
                      DFTraverse(adj_list[i], clamp1, tri_id, 2);
                  if(GetIntersectionPoint(foot3,foot2,clamp1,ip,false))
                      DFTraverse(adj_list[i], clamp1, tri_id, 3);

                  Clamp clamp2(clamp.apex, foot3, clamp.foot2);
                  if(GetIntersectionPoint(foot3,foot1,clamp2,ip,false))
                      DFTraverse(adj_list[i], clamp2, tri_id, 2);
                  if(GetIntersectionPoint(foot3,foot2,clamp2,ip,false))
                      DFTraverse(adj_list[i], clamp2, tri_id, 3);

                }else{
//                      bool flag1 = CheckVisibility2(clamp, foot3, foot1);
//                      bool flag2 = CheckVisibility2(clamp, foot3, foot2);
//                      cout<<"flag1 "<<flag1<<" flag2 "<<flag2<<endl;

                      if(CheckVisibility2(clamp, foot3, foot1)){
                        DFTraverse(adj_list[i], clamp, tri_id, 2);
                      }
                      if(CheckVisibility2(clamp, foot3, foot2)){
                        DFTraverse(adj_list[i], clamp, tri_id, 3);
                      }
                }
              break;
        case 2:
//                cout<<"case2"<<endl;
                visibility = CheckVisibility1(clamp, foot2, ver2);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp.apex, foot2);
                    //split the clamp into 2
                    oids1.push_back(ver2);
                    p_list.push_back(foot2);
                    Point ip;

                    Clamp clamp1(clamp.apex, foot2, clamp.foot1);
                    if(GetIntersectionPoint(foot2,foot1,clamp1,ip,false))
                        DFTraverse(adj_list[i], clamp1, tri_id, 1);
                    if(GetIntersectionPoint(foot2,foot3,clamp1,ip,false))
                        DFTraverse(adj_list[i], clamp1, tri_id, 3);

                    Clamp clamp2(clamp.apex, foot2, clamp.foot2);
                    if(GetIntersectionPoint(foot2,foot1,clamp2,ip,false))
                        DFTraverse(adj_list[i], clamp2, tri_id, 1);
                    if(GetIntersectionPoint(foot2,foot3,clamp2,ip,false))
                        DFTraverse(adj_list[i], clamp2, tri_id, 3);

                }else{
                    if(CheckVisibility2(clamp, foot2, foot1)){
                        DFTraverse(adj_list[i], clamp, tri_id, 1);
                    }

                    if(CheckVisibility2(clamp, foot2, foot3)){
                        DFTraverse(adj_list[i], clamp, tri_id, 3);
                    }
                }
              break;
        case 3:
//                cout<<"case3"<<endl;
                visibility = CheckVisibility1(clamp, foot1, ver1);

                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp.apex, foot1);
                    //split the clamp into 2


                    oids1.push_back(ver1);
                    p_list.push_back(foot1);
                    Point ip;
                    Clamp clamp1(clamp.apex, foot1, clamp.foot2);
                    if(GetIntersectionPoint(foot1,foot2,clamp1,ip,false))
                      DFTraverse(adj_list[i], clamp1, tri_id, 1);
                    if(GetIntersectionPoint(foot1,foot3,clamp1,ip,false))
                      DFTraverse(adj_list[i], clamp1, tri_id, 2);

                    Clamp clamp2(clamp.apex, foot1, clamp.foot1);
                    if(GetIntersectionPoint(foot1,foot2,clamp2,ip,false))
                        DFTraverse(adj_list[i], clamp2, tri_id, 1);
                    if(GetIntersectionPoint(foot1,foot3,clamp2,ip,false))
                        DFTraverse(adj_list[i], clamp2, tri_id, 2);

                }else{

                  if(CheckVisibility2(clamp, foot1, foot2)){
                      DFTraverse(adj_list[i], clamp, tri_id, 1);
                  }

                  if(CheckVisibility2(clamp, foot1, foot3)){
                      DFTraverse(adj_list[i], clamp, tri_id, 2);
                  }

                }
              break;
        default: assert(false);
      }
//      cout<<"out of switch"<<endl;
    }
//    cout<<"finish loop"<<endl;

}

/*
for each vertex, it returns which triangle it belongs to (vid,triid)

*/
void VGraph::DecomposeTriangle()
{
  for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* tri_tuple = rel1->GetTuple(i, false);
      int v1 = ((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
      int v2 = ((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
      int v3 = ((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
      int triid =
                ((CcInt*)tri_tuple->GetAttribute(DualGraph::TOID))->GetIntval();
      oids1.push_back(v1);
      oids2.push_back(triid);
      oids1.push_back(v2);
      oids2.push_back(triid);
      oids1.push_back(v3);
      oids2.push_back(triid);
      tri_tuple->DeleteIfAllowed();
  }

}

/*
find all triangles that contain vertex (vid) and depth first searching

*/
void VGraph::FindTriContainVertex(int vid, int tri_id, Point* query_p)
{
//    cout<<"FindTriContainVertex() "<<endl;
//    cout<<"vid "<<vid<<"tri_id "<<tri_id<<endl;

    CcInt* vertex_id = new CcInt(true, vid);
    BTreeIterator* btreeiter = btree->ExactMatch(vertex_id);
    while(btreeiter->Next()){
      Tuple* ver_tri = rel4->GetTuple(btreeiter->GetId(), false);
      int triangle_id =
          ((CcInt*)ver_tri->GetAttribute(DualGraph::TRIID))->GetIntval();
      if(triangle_id != tri_id){
//        cout<<"triangle_id "<<triangle_id<<endl;
        Tuple* tri_tuple = rel2->GetTuple(triangle_id, false);
        int v1 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
        int v2 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
        int v3 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
        /////////////////////////////////////////////////////////////////////
        Tuple* loc_tuple1 = rel3->GetTuple(v1, false);
        Point* p1 = (Point*)loc_tuple1->GetAttribute(VisualGraph::LOC);
        Point foot1(*p1);
        loc_tuple1->DeleteIfAllowed();
        Tuple* loc_tuple2 = rel3->GetTuple(v2, false);
        Point* p2 = (Point*)loc_tuple2->GetAttribute(VisualGraph::LOC);
        Point foot2(*p2);
        loc_tuple2->DeleteIfAllowed();
        Tuple* loc_tuple3 = rel3->GetTuple(v3, false);
        Point* p3 = (Point*)loc_tuple3->GetAttribute(VisualGraph::LOC);
        Point foot3(*p3);
        loc_tuple3->DeleteIfAllowed();
        ///////////////////////////////////////////////////////////////////////

//        cout<<"v1 "<<v1<<" v2 "<<v2<<" v3 "<<v3<<endl;

        vector<int> vid_list;
        if(v1 == vid){
            vid_list.push_back(v2);
            vid_list.push_back(v3);

            for(unsigned int i = 0;i < vid_list.size();i++){
              unsigned int j = 0;
              for(;j < oids1.size();j++){
                if(vid_list[i] == oids1[j]) break;
              }
              if(j == oids1.size()){
                 oids1.push_back(vid_list[i]);

                Tuple* loc_tuple = rel3->GetTuple(vid_list[i], false);
                Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
                p_list.push_back(*p);
                loc_tuple->DeleteIfAllowed();

              }
            }

            Clamp clamp(*query_p, foot2, foot3);
            DFTraverse(triangle_id, clamp, tri_id, 3);

        }else if(v2 == vid){
            vid_list.push_back(v1);
            vid_list.push_back(v3);
            for(unsigned int i = 0;i < vid_list.size();i++){
              unsigned int j = 0;
              for(;j < oids1.size();j++){
                if(vid_list[i] == oids1[j]) break;
              }
              if(j == oids1.size()){

                  oids1.push_back(vid_list[i]);
                  Tuple* loc_tuple = rel3->GetTuple(vid_list[i], false);
                  Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
                  p_list.push_back(*p);
                  loc_tuple->DeleteIfAllowed();

              }
            }

            Clamp clamp(*query_p, foot1, foot3);
            DFTraverse(triangle_id, clamp, tri_id, 2);

        }else if(v3 == vid){
            vid_list.push_back(v1);
            vid_list.push_back(v2);
            for(unsigned int i = 0;i < vid_list.size();i++){
              unsigned int j = 0;
              for(;j < oids1.size();j++){
                if(vid_list[i] == oids1[j]) break;
              }
              if(j == oids1.size()){
                 oids1.push_back(vid_list[i]);

                Tuple* loc_tuple = rel3->GetTuple(vid_list[i], false);
                Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
                p_list.push_back(*p);
                loc_tuple->DeleteIfAllowed();

              }
            }

            Clamp clamp(*query_p, foot1, foot2);
            DFTraverse(triangle_id, clamp, tri_id, 1);
        }else assert(false);
        tri_tuple->DeleteIfAllowed();
      }
      ver_tri->DeleteIfAllowed();
    }
    delete btreeiter;
    delete vertex_id;
}

/*
check the triangle with the only given type

*/

void VGraph::GetVisibleNode2(int tri_id, Point* query_p, int type)
{

//cout<<"GetVisibleNode2() "<<"query tri_id "<<tri_id + dg->min_tri_oid_1<<endl;

  const double pi = 3.14159;
  const double delta = 0.0001;

  vector<int> adj_list;
  dg->FindAdj(tri_id, adj_list);

  Tuple* tri_tuple1 = rel2->GetTuple(tri_id, false);
  int v1 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V1))->GetIntval();
  int v2 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V2))->GetIntval();
  int v3 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V3))->GetIntval();
  tri_tuple1->DeleteIfAllowed();
  Triangle tri1(v1, v2, v3);
//  cout<<"v1 "<<v1<<" v2 "<<v2<<" v3 "<<v3<<endl; 
  ///////////////////////////////////////////////////////////////////////
  for(unsigned int i = 0;i < adj_list.size();i++){
//    cout<<"adj_list GVN "<<adj_list[i] + dg->min_tri_oid_1<<endl;
    Tuple* tri_tuple = rel2->GetTuple(adj_list[i], false);
    int ver1 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
    int ver2 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
    int ver3 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
    tri_tuple->DeleteIfAllowed();
//    cout<<"ver1 "<<ver1<<" ver2 "<<ver2<<" ver3 "<<ver3<<endl; 
    ///////////////////////////////////////////////////////////////////////
    Tuple* loc_tuple1 = rel3->GetTuple(ver1, false);
    Point* p1 = (Point*)loc_tuple1->GetAttribute(VisualGraph::LOC);
    Point foot1(*p1);
    loc_tuple1->DeleteIfAllowed();
    Tuple* loc_tuple2 = rel3->GetTuple(ver2, false);
    Point* p2 = (Point*)loc_tuple2->GetAttribute(VisualGraph::LOC);
    Point foot2(*p2);
    loc_tuple2->DeleteIfAllowed();
    Tuple* loc_tuple3 = rel3->GetTuple(ver3, false);
    Point* p3 = (Point*)loc_tuple3->GetAttribute(VisualGraph::LOC);
    Point foot3(*p3);
    loc_tuple3->DeleteIfAllowed();
    ///////////////////////////////////////////////////////////////////////
    Triangle tri2(ver1, ver2, ver3);
    int sharetype1 = tri2.ShareEdge(tri1);
    int sharetype2 = tri1.ShareEdge(tri2);
    if(sharetype2 != type) continue;


    Point clamp_foot1;
    Point clamp_foot2;
    if(sharetype1 == 1){
      clamp_foot1 = foot1;
      clamp_foot2 = foot2;
    }
    else if(sharetype1 == 2){
      clamp_foot1 = foot1;
      clamp_foot2 = foot3;
    }
    else if(sharetype1 == 3){
      clamp_foot1 = foot2;
      clamp_foot2 = foot3;
    }
    else assert(false);

    double d1 = query_p->Distance(clamp_foot1);
    double d2 = query_p->Distance(clamp_foot2);

    //apex foot1(2),checkp on the same line because apex=foot1(2)
    if(AlmostEqual(d1*d2, 0.0)){
       continue;
    }

    Clamp clamp(*query_p, clamp_foot1, clamp_foot2);

    //DFTraverse(adj_list[i], clamp1, tri_id,0);
    //1. triangle to be expanded 2. clamp 3. previous triangle
    //4. edge can't be expanded
    //a special case that the clamp's angle can be 180. but this can only
    //happen at the start time. because when the procedure proceeds, the
    //clamp angle always becomes smaller and smaller
    bool visibility;
    switch(sharetype1){
      case 1:
//                cout<<"case1"<<endl;

                if(fabs(clamp.angle - pi) < delta )
                  visibility = true;
                else
                  visibility = CheckVisibility1(clamp, foot3, ver3);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp.apex, foot3);
                    //split the clamp into 2
                         //insert the point into the result;
                    oids1.push_back(ver3);
                    p_list.push_back(foot3);
                    //split the clamp
                    Clamp clamp1(*query_p, clamp.foot1, foot3);
                    DFTraverse(adj_list[i], clamp1, tri_id, 2);
                    Clamp clamp2(*query_p, foot3, clamp.foot2);
                    DFTraverse(adj_list[i], clamp2, tri_id, 3);
                }else{
                      if(CheckVisibility2(clamp, foot3, foot1)){
                        DFTraverse(adj_list[i], clamp, tri_id, 2);
                      }

                      if(CheckVisibility2(clamp, foot3, foot2)){
                        DFTraverse(adj_list[i], clamp, tri_id, 3);
                      }
                }
              break;
      case 2:
//                cout<<"case2"<<endl;
                if(fabs(clamp.angle - pi) < delta)
                  visibility = true;
                else
                  visibility = CheckVisibility1(clamp, foot2, ver2);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp.apex, foot2);
                    //split the clamp into 2

                    oids1.push_back(ver2);
                    p_list.push_back(foot2);
                    Clamp clamp1(*query_p, clamp.foot1, foot2);
                    DFTraverse(adj_list[i], clamp1, tri_id,  1);
                    Clamp clamp2(*query_p, foot2, clamp.foot2);
                    DFTraverse(adj_list[i], clamp2, tri_id,  3);

                }else{

                  if(CheckVisibility2(clamp, foot2, foot1)){
                    DFTraverse(adj_list[i], clamp, tri_id, 1);
                  }

                  if(CheckVisibility2(clamp, foot2, foot3)){
                    DFTraverse(adj_list[i], clamp, tri_id, 3);
                  }

                }
              break;
      case 3:
//                cout<<"case3"<<endl;
                if(fabs(clamp.angle - pi) < delta)
                  visibility = true;
                else
                  visibility = CheckVisibility1(clamp, foot1, ver1);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp.apex, foot1);
                    //split the clamp into 2

                    oids1.push_back(ver1);
                    p_list.push_back(foot1);
                    Clamp clamp1(*query_p, clamp.foot2, foot1);
                    DFTraverse(adj_list[i], clamp1, tri_id,  2);
                    Clamp clamp2(*query_p, foot1, clamp.foot1);
                    DFTraverse(adj_list[i], clamp2, tri_id,  1);

                }else{
                  if(CheckVisibility2(clamp, foot1, foot2)){
                      DFTraverse(adj_list[i], clamp, tri_id, 1);
                  }

                  if(CheckVisibility2(clamp, foot1, foot3)){
                      DFTraverse(adj_list[i], clamp, tri_id, 2);
                  }
                }
                break;
      default: assert(false);
    }
  }

}

/*
check whether the query point equals to one of the vertices
if it is. it uses another method to find the clamp.
each triangle having the query point is considered as a clamp

*/
bool VGraph::GetVNode_QV(int tri_id, Point* query_p, int v1, int v2, int v3)
{
  Tuple* tuple1 = rel3->GetTuple(v1, false);
  Point* p1 = (Point*)tuple1->GetAttribute(VisualGraph::LOC);
  Point vp1(*p1);
  Tuple* tuple2 = rel3->GetTuple(v2, false);
  Point* p2 = (Point*)tuple2->GetAttribute(VisualGraph::LOC);
  Point vp2(*p2);
  Tuple* tuple3 = rel3->GetTuple(v3, false);
  Point* p3 = (Point*)tuple3->GetAttribute(VisualGraph::LOC);
  Point vp3(*p3);
  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();
  tuple3->DeleteIfAllowed();
  int type = 0;
  if(AlmostEqual(vp1, *query_p)) type = 1;
  else if(AlmostEqual(vp2, *query_p)) type = 2;
  else if(AlmostEqual(vp3, *query_p)) type = 3;

  switch(type){
    case 1:
          oids1.push_back(v2);
          oids1.push_back(v3);

          for(unsigned int i = 0;i < oids1.size();i++){
            Tuple* loc_tuple = rel3->GetTuple(oids1[i], false);
            Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
            p_list.push_back(*p);
            loc_tuple->DeleteIfAllowed();
          }

          FindTriContainVertex(v1, tri_id, query_p);
          GetVisibleNode2(tri_id, query_p, 3);
          break;
    case 2:
          oids1.push_back(v1);
          oids1.push_back(v3);
          for(unsigned int i = 0;i < oids1.size();i++){
            Tuple* loc_tuple = rel3->GetTuple(oids1[i], false);
            Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
            p_list.push_back(*p);
            loc_tuple->DeleteIfAllowed();
          }
          FindTriContainVertex(v2, tri_id, query_p);
          GetVisibleNode2(tri_id, query_p, 2);

          break;
    case 3:
          oids1.push_back(v1);
          oids1.push_back(v2);
          for(unsigned int i = 0;i < oids1.size();i++){
            Tuple* loc_tuple = rel3->GetTuple(oids1[i], false);
            Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
            p_list.push_back(*p);
            loc_tuple->DeleteIfAllowed();
          }
          FindTriContainVertex(v3, tri_id, query_p);
          GetVisibleNode2(tri_id, query_p, 1);
          break;
  }
  if(type != 0)return true;
  else  return false;

}

/*
find all visible nodes for query_p
in some cases, there are numberic problems or java display numberic.

*/


void VGraph::GetVisibleNode1(int tri_id, Point* query_p)
{

//  cout<<"GetVisibleNode1() "<<"query tri_id "<<tri_id<<endl;

  const double pi = 3.14159;
  const double delta = 0.0001;

  vector<int> adj_list;
  dg->FindAdj(tri_id, adj_list);
  assert(oids1.size() == 3);
  int v1 = oids1[0];
  int v2 = oids1[1];
  int v3 = oids1[2];
  Triangle tri1(v1, v2, v3);

  ///////////////////////////////////////////////////////////////////////
  for(unsigned int i = 0;i < adj_list.size();i++){
//    cout<<"adj_list GVN "<<adj_list[i]<<endl;
    Tuple* tri_tuple = rel2->GetTuple(adj_list[i], false);
    int ver1 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V1))->GetIntval();
    int ver2 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V2))->GetIntval();
    int ver3 =((CcInt*)tri_tuple->GetAttribute(DualGraph::V3))->GetIntval();
    tri_tuple->DeleteIfAllowed();

    ///////////////////////////////////////////////////////////////////////
    Tuple* loc_tuple1 = rel3->GetTuple(ver1, false);
    Point* p1 = (Point*)loc_tuple1->GetAttribute(VisualGraph::LOC);
    Point foot1(*p1);
    loc_tuple1->DeleteIfAllowed();
    Tuple* loc_tuple2 = rel3->GetTuple(ver2, false);
    Point* p2 = (Point*)loc_tuple2->GetAttribute(VisualGraph::LOC);
    Point foot2(*p2);
    loc_tuple2->DeleteIfAllowed();
    Tuple* loc_tuple3 = rel3->GetTuple(ver3, false);
    Point* p3 = (Point*)loc_tuple3->GetAttribute(VisualGraph::LOC);
    Point foot3(*p3);
    loc_tuple3->DeleteIfAllowed();
    ///////////////////////////////////////////////////////////////////////
    Triangle tri2(ver1, ver2, ver3);
    int sharetype = tri2.ShareEdge(tri1);
    Clamp clamp12(*query_p, foot1, foot2);
    Clamp clamp13(*query_p, foot1, foot3);
    Clamp clamp23(*query_p, foot2, foot3);

    //DFTraverse(adj_list[i], clamp1, tri_id,0);
    //1. triangle to be expanded 2. clamp 3. previous triangle
    //4. edge can't be expanded
    //a special case that the clamp's angle can be 180. but this can only
    //happen at the start time. because when the procedure proceeds, the
    //clamp angle always becomes smaller and smaller
    bool visibility;
    switch(sharetype){
      case 1:

//                cout<<"case1"<<endl;
                if(fabs(clamp12.angle - pi) < delta )
                  visibility = true;
                else
                  visibility = CheckVisibility1(clamp12, foot3, ver3);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp12.apex, foot3);
                    //split the clamp into 2

                         //insert the point into the result;
                    oids1.push_back(ver3);
                    p_list.push_back(foot3);
                    //split the clamp
                    Clamp clamp1(*query_p, clamp12.foot1, foot3);
                    DFTraverse(adj_list[i], clamp1, tri_id, 2);
                    Clamp clamp2(*query_p, foot3, clamp12.foot2);
                    DFTraverse(adj_list[i], clamp2, tri_id, 3);

                }else{
                      if(CheckVisibility2(clamp12, foot3, foot1)){
                        DFTraverse(adj_list[i], clamp12, tri_id, 2);
                      }

                      if(CheckVisibility2(clamp12, foot3, foot2)){
                        DFTraverse(adj_list[i], clamp12, tri_id, 3);
                      }
                }
              break;
      case 2:
//                cout<<"case2"<<endl;
                if(fabs(clamp13.angle - pi) < delta)
                  visibility = true;
                else
                  visibility = CheckVisibility1(clamp13, foot2, ver2);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp13.apex, foot2);
                    //split the clamp into 2

                    oids1.push_back(ver2);
                    p_list.push_back(foot2);
                    Clamp clamp1(*query_p, clamp13.foot1, foot2);
                    DFTraverse(adj_list[i], clamp1, tri_id,  1);
                    Clamp clamp2(*query_p, foot2, clamp13.foot2);
                    DFTraverse(adj_list[i], clamp2, tri_id,  3);

                }else{

                  if(CheckVisibility2(clamp13, foot2, foot1)){
                    DFTraverse(adj_list[i], clamp13, tri_id, 1);
                  }

                  if(CheckVisibility2(clamp13, foot2, foot3)){
                    DFTraverse(adj_list[i], clamp13, tri_id, 3);
                  }

                }
              break;
      case 3:
//                cout<<"case3"<<endl;
                if(fabs(clamp23.angle - pi) < delta)
                  visibility = true;
                else
                  visibility = CheckVisibility1(clamp23, foot1, ver1);
                if(visibility){
//                    cout<<"visibility"<<endl;
                    HalfSegment hs;
                    hs.Set(true, clamp23.apex, foot1);
                    //split the clamp into 2
                    oids1.push_back(ver1);
                    p_list.push_back(foot1);
                    Clamp clamp1(*query_p, clamp23.foot2, foot1);
                    DFTraverse(adj_list[i], clamp1, tri_id,  2);
                    Clamp clamp2(*query_p, foot1, clamp23.foot1);
                    DFTraverse(adj_list[i], clamp2, tri_id,  1);

                }else{
                  if(CheckVisibility2(clamp23, foot1, foot2)){
                      DFTraverse(adj_list[i], clamp23, tri_id, 1);
                  }

                  if(CheckVisibility2(clamp23, foot1, foot3)){
                      DFTraverse(adj_list[i], clamp23, tri_id, 2);
                  }
                }
                break;
      default: assert(false);
    }
  }

}

/*
for walk shortest path algorithm
it tries to connect the query point to the visiblity graph.
if the query point locates in the visiblity graph.
then it does not connect. because the adjacency can be later found by the
adjacency list

*/
void VGraph::GetVisibilityNode(int tri_id, Point query_p)
{
  assert(1 <= tri_id && tri_id <= rel2->GetNoTuples());
  /////////three vertices of the triangle////////////////////////
  Tuple* tri_tuple1 = rel2->GetTuple(tri_id, false);
  int v1 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V1))->GetIntval();
  int v2 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V2))->GetIntval();
  int v3 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V3))->GetIntval();
  tri_tuple1->DeleteIfAllowed();

  ///if the query_p equals to one of the triangle vertices ///

  Tuple* tuple1 = rel3->GetTuple(v1, false);
  Point* p1 = (Point*)tuple1->GetAttribute(VisualGraph::LOC);
  Point vp1(*p1);
  Tuple* tuple2 = rel3->GetTuple(v2, false);
  Point* p2 = (Point*)tuple2->GetAttribute(VisualGraph::LOC);
  Point vp2(*p2);
  Tuple* tuple3 = rel3->GetTuple(v3, false);
  Point* p3 = (Point*)tuple3->GetAttribute(VisualGraph::LOC);
  Point vp3(*p3);
  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();
  tuple3->DeleteIfAllowed();
  int type = 0;
  if(AlmostEqual(vp1, query_p)) type = 1;
  else if(AlmostEqual(vp2, query_p)) type = 2;
  else if(AlmostEqual(vp3, query_p)) type = 3;

  /////////////////////////////////////////////////////////////
  switch(type){
      case 0:
        oids1.push_back(v1);
        oids1.push_back(v2);
        oids1.push_back(v3);
        p_list.push_back(vp1);
        p_list.push_back(vp2);
        p_list.push_back(vp3);
        ///////searching in the dual graph to find all visible vertices////
        GetVisibleNode1(tri_id, &query_p);
        break;
      //if it equals to one of the vertices of the triangle, it directly
      //put the vertex and return.
      //its visibile vertices can be found by adjacency list
      case 1:
        oids1.push_back(v1);
        p_list.push_back(vp1);
        break;
      case 2:
        oids1.push_back(v2);
        p_list.push_back(vp2);
        break;
      case 3:
        oids1.push_back(v3);
        p_list.push_back(vp3);
        break;
      default:assert(false);
  }

}

/*
for a given point, find all its visible nodes
connect the point to the visibility graph node
rel1--query location relation
rel2--triangle relation v1 int v2 int v3 int
rel3--vertex relation1 (oid int) (loc point)
rel4--vertex relation2 (vid(oid) int) (triid int)

*/
void VGraph::GetVNode()
{
  if(rel1->GetNoTuples() < 1){
    cout<<"query relation is empty"<<endl;
    return;
  }
  Tuple* query_tuple = rel1->GetTuple(1, false);
  int query_oid =
          ((CcInt*)query_tuple->GetAttribute(VisualGraph::QOID))->GetIntval();
  Point* query_p =
          new Point(*((Point*)query_tuple->GetAttribute(VisualGraph::QLOC2)));
  query_tuple->DeleteIfAllowed();

  query_oid -= dg->min_tri_oid_1; 
  assert(1 <= query_oid && query_oid <= dg->node_rel->GetNoTuples()); 
  /////////three vertices of the triangle////////////////////////
  Tuple* tri_tuple1 = rel2->GetTuple(query_oid, false);
  int v1 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V1))->GetIntval();
  int v2 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V2))->GetIntval();
  int v3 =((CcInt*)tri_tuple1->GetAttribute(DualGraph::V3))->GetIntval();
  tri_tuple1->DeleteIfAllowed();

  ///if the query_p equals to one of the triangle vertices ///
  if(GetVNode_QV(query_oid, query_p,v1,v2,v3)){
  }else{
    //////////////////////////////////////////////////////////////////////////
    ////////three vertices of the triangle that the point is located in///////
    //////////////////////////////////////////////////////////////////////////
    oids1.push_back(v1);
    oids1.push_back(v2);
    oids1.push_back(v3);
    for(unsigned int i = 0;i < oids1.size();i++){
      Tuple* loc_tuple = rel3->GetTuple(oids1[i], false);
      Point* p = (Point*)loc_tuple->GetAttribute(VisualGraph::LOC);
      p_list.push_back(*p);
      loc_tuple->DeleteIfAllowed();
    }
    ///////////searching in the dual graph to find all visible vertices//////
    GetVisibleNode1(query_oid, query_p);
  }

  ///////build a halfsegment between query point and its visibility point////
  for(unsigned int i = 0;i < p_list.size();i++){
      Line* l =  new Line(0);
      l->StartBulkLoad();
      HalfSegment hs;
      hs.Set(true, p_list[i], *query_p);
      hs.attr.edgeno = 0;
      *l += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *l += hs;
      l->EndBulkLoad();
      line.push_back(*l);
      delete l;
  }
  delete query_p;
}


/*
create a relation for the vertices of the region with the cycleno

*/
void RegVertex::CreateVertex()
{
/*   //naive method to compute the cycle of a region
      vector<int> cycle;
      for(int i = 0;i < reg->Size();i++){
        HalfSegment hs;
        reg->Get(i, hs);
        if(!hs.IsLeftDomPoint())continue;
        int cycle_no = hs.attr.cycleno;
        unsigned int j = 0;
        for(;j < cycle.size();j++)
          if(cycle[j] == cycle_no) break;
        if(j == cycle.size()) cycle.push_back(cycle_no);
      }
      cout<<"polgyon with "<<cycle.size()<<" cycles inside "<<endl;*/
      CompTriangle* ct = new CompTriangle(reg);
      unsigned int no_cyc = ct->NoOfCycles();
      assert(no_cyc > 0);

      const int ncontours = no_cyc;
      int no_p_contour[ncontours];

      vector<double> ps_contour_x;
      vector<double> ps_contour_y;

      vector<SimpleLine*> sl_contour;

      for(unsigned int i = 0;i < no_cyc;i++){
          SimpleLine* sl = new SimpleLine(0);
          sl->StartBulkLoad();
          sl_contour.push_back(sl);
      }
      vector<int> edgenos(no_cyc, 0);
      for(int j = 0;j < reg->Size();j++){
        HalfSegment hs1;
        reg->Get(j, hs1);
        if(!hs1.IsLeftDomPoint()) continue;
        HalfSegment hs2;
        hs2.Set(true, hs1.GetLeftPoint(), hs1.GetRightPoint());

        hs2.attr.edgeno = edgenos[hs1.attr.cycleno]++;
        *sl_contour[hs1.attr.cycleno] += hs2;
        hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
        *sl_contour[hs1.attr.cycleno] += hs2;

      }

      SpacePartition* sp = new SpacePartition();

      for(unsigned int i = 0;i < no_cyc;i++){
//        cout<<"i "<<i<<endl;
        sl_contour[i]->EndBulkLoad();
        vector<MyHalfSegment> mhs;

        sp->ReorderLine(sl_contour[i], mhs);
        vector<Point> ps;
        for(unsigned int j = 0;j < mhs.size();j++)
          ps.push_back(mhs[j].from);

        bool clock;
        if(0.0f < ct->Area(ps)){//points counter-clockwise order
          clock = false;
        }else{// points clockwise
          clock = true;
        }
        no_p_contour[i] = ps.size();
        if(i == 0){//outer contour, counter_clockwise
          if(clock == false){
              for(unsigned int index = 0;index < ps.size();index++){
//                  ps_contour_x.push_back(ps[index].GetX());
//                  ps_contour_y.push_back(ps[index].GetY());
                    regnodes.push_back(ps[index]);
                    cycleno.push_back(i);

              }
          }else{
              for(unsigned int index = 0;index < ps.size();index++){
//                  ps_contour_x.push_back(ps[ps.size() - 1 - index].GetX());
//                  ps_contour_y.push_back(ps[ps.size() - 1 - index].GetY());
                    regnodes.push_back(ps[ps.size() - 1 - index]);
                    cycleno.push_back(i);
              }
          }
        }else{//hole points, should be clockwise
          if(clock == false){
              for(unsigned int index = 0;index < ps.size();index++){
//                ps_contour_x.push_back(ps[ps.size() -1 - index].GetX());
//                ps_contour_y.push_back(ps[ps.size() -1 - index].GetY());
                  regnodes.push_back(ps[ps.size() - 1 - index]);
                  cycleno.push_back(i);
              }
          }else{
              for(unsigned int index = 0;index < ps.size();index++){
//                ps_contour_x.push_back(ps[index].GetX());
//                ps_contour_y.push_back(ps[index].GetY());
                  regnodes.push_back(ps[index]);
                  cycleno.push_back(i);
              }
          }
        }

        delete sl_contour[i];
      }
      delete ct;
      delete sp;
}

/*
for each triangle, it returns the number of each point and the centroid

*/
void RegVertex::TriangulationNew()
{
      CompTriangle* ct = new CompTriangle(reg);
      unsigned int no_cyc = ct->NoOfCycles();
      assert(no_cyc > 0);

      const int ncontours = no_cyc;
      int no_p_contour[ncontours];

      vector<double> ps_contour_x;//start from 1
      vector<double> ps_contour_y;//start from 1

      ct->PolygonContourPoint(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);
      int result_trig[SEGSIZE][3];
      int (*res_triangles)[3] = &result_trig[0];

      int no_triangle;
      no_triangle = triangulate_polygon(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y, res_triangles);

//      cout<<"no_triangle "<<no_triangle<<endl;

      assert(0 < no_triangle && no_triangle < SEGSIZE);
      for (int i = 0; i < no_triangle; i++){
          Coord x, y;

          x = ps_contour_x[res_triangles[i][0]];
          y = ps_contour_y[res_triangles[i][0]];

          x += ps_contour_x[res_triangles[i][1]];
          y += ps_contour_y[res_triangles[i][1]];

          x += ps_contour_x[res_triangles[i][2]];
          y += ps_contour_y[res_triangles[i][2]];
          v1_list.push_back(res_triangles[i][0]);
          v2_list.push_back(res_triangles[i][1]);
          v3_list.push_back(res_triangles[i][2]);
          //calculate the centroid point
          Point p;
          p.Set(x/3.0, y/3.0);
          regnodes.push_back(p);
      }

      delete ct;
}

/*
for each triangle, it returns the number of each point and the triangle

*/
void RegVertex::TriangulationExt()
{
      CompTriangle* ct = new CompTriangle(reg);
      unsigned int no_cyc = ct->NoOfCycles();
      assert(no_cyc > 0);

      const int ncontours = no_cyc;
      int no_p_contour[ncontours];

      vector<double> ps_contour_x;//start from 1
      vector<double> ps_contour_y;//start from 1

      ct->PolygonContourPoint(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);
      int result_trig[SEGSIZE][3];
      int (*res_triangles)[3] = &result_trig[0];

      int no_triangle;
      no_triangle = triangulate_polygon(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y, res_triangles);

//      cout<<"no_triangle "<<no_triangle<<endl;

      assert(0 < no_triangle && no_triangle < SEGSIZE);
      for (int i = 0; i < no_triangle; i++){
          Coord x1, y1;
          Coord x2, y2;
          Coord x3, y3;

          x1 = ps_contour_x[res_triangles[i][0]];
          y1 = ps_contour_y[res_triangles[i][0]];

          x2 = ps_contour_x[res_triangles[i][1]];
          y2 = ps_contour_y[res_triangles[i][1]];

          x3 = ps_contour_x[res_triangles[i][2]];
          y3 = ps_contour_y[res_triangles[i][2]];

          v1_list.push_back(res_triangles[i][0]);
          v2_list.push_back(res_triangles[i][1]);
          v3_list.push_back(res_triangles[i][2]);

          Point p1(true, x1, y1);
          Point p2(true, x2, y2);
          Point p3(true, x3, y3);

          vector<Point> ps_list; 
          ps_list.push_back(p1);
          ps_list.push_back(p2);
          ps_list.push_back(p3);
          SpacePartition* sp = new SpacePartition();
          vector<Region> reg_list;
          sp->ComputeRegion(ps_list, reg_list);
          tri_list.push_back(reg_list[0]);
          delete sp;
      }
      delete ct;
}

/*
for each triangle, it returns the number of each point and the centroid

*/
void RegVertex::TriangulationNew2()
{
    CompTriangle* ct = new CompTriangle(reg);
    unsigned int no_cyc = ct->NoOfCycles();
    assert(no_cyc > 0);

    const int ncontours = no_cyc;
    int no_p_contour[ncontours];

    vector<double> ps_contour_x;//start from 0
    vector<double> ps_contour_y;//start from 0

   ct->PolygonContourPoint2(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);

   HPolygon poly;
   int no_triangle = poly.Triangulation2(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y);

//   cout<<"no_triangle "<<no_triangle<<endl;
    SpacePartition* spacepart = new SpacePartition();

      for (int i = 0; i < no_triangle; i++){
          Coord x, y;
          int index1 = poly.mtabCell[i].Index(0);
          int index2 = poly.mtabCell[i].Index(1);
          int index3 = poly.mtabCell[i].Index(2);

          x = poly.mtabPnt[index1].X();
          y = poly.mtabPnt[index1].Y();

          x += poly.mtabPnt[index2].X();
          y += poly.mtabPnt[index2].Y();;

          x += poly.mtabPnt[index3].X();
          y += poly.mtabPnt[index3].Y();;

//          v1_list.push_back(index1 + 1);
//          v2_list.push_back(index2 + 1);
//          v3_list.push_back(index3 + 1);

//           v1_list.push_back(poly.p_id_list[index1]);
//           v2_list.push_back(poly.p_id_list[index2]);
//           v3_list.push_back(poly.p_id_list[index3]);

//           //calculate the centroid point
//           Point p;
//           p.Set(x/3.0, y/3.0);
//           regnodes.push_back(p);

          ///////////////////2012.3.9 rounding problem //////////
          Point p1(true, poly.mtabPnt[index1].X(), poly.mtabPnt[index1].Y());
          Point p2(true, poly.mtabPnt[index2].X(), poly.mtabPnt[index2].Y());
          Point p3(true, poly.mtabPnt[index3].X(), poly.mtabPnt[index3].Y());
          vector<Point> ps_list;
          ps_list.push_back(p1);
          ps_list.push_back(p2);
          ps_list.push_back(p3);
          vector<Region> reg;
          spacepart->ComputeRegion(ps_list, reg);
          ///////////////////////////////////////////////
//          if(reg.size() > 0){
            //calculate the centroid point
            Point p;
            p.Set(x/3.0, y/3.0);
            regnodes.push_back(p);
            v1_list.push_back(poly.p_id_list[index1]);
            v2_list.push_back(poly.p_id_list[index2]);
            v3_list.push_back(poly.p_id_list[index3]);
//          }
      }

      delete ct;
      delete spacepart;
}

void RegVertex::TriangulationExt2()
{
    CompTriangle* ct = new CompTriangle(reg);
    unsigned int no_cyc = ct->NoOfCycles();
    assert(no_cyc > 0);

    const int ncontours = no_cyc;
    int no_p_contour[ncontours];

    vector<double> ps_contour_x;//start from 0
    vector<double> ps_contour_y;//start from 0

   ct->PolygonContourPoint2(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);

   HPolygon poly;
   int no_triangle = poly.Triangulation2(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y);

//   cout<<"no_triangle "<<no_triangle<<endl;

      for (int i = 0; i < no_triangle; i++){
          Coord x1, y1;
          Coord x2, y2;
          Coord x3, y3;
          int index1 = poly.mtabCell[i].Index(0);
          int index2 = poly.mtabCell[i].Index(1);
          int index3 = poly.mtabCell[i].Index(2);

          x1 = poly.mtabPnt[index1].X();
          y1 = poly.mtabPnt[index1].Y();

          x2 = poly.mtabPnt[index2].X();
          y2 = poly.mtabPnt[index2].Y();;

          x3 = poly.mtabPnt[index3].X();
          y3 = poly.mtabPnt[index3].Y();;

//          v1_list.push_back(index1 + 1);
//          v2_list.push_back(index2 + 1);
//          v3_list.push_back(index3 + 1);

//           v1_list.push_back(poly.p_id_list[index1]);
//           v2_list.push_back(poly.p_id_list[index2]);
//           v3_list.push_back(poly.p_id_list[index3]);


          Point p1(true, x1, y1);
          Point p2(true, x2, y2);
          Point p3(true, x3, y3);

          vector<Point> ps_list; 
          ps_list.push_back(p1);
          ps_list.push_back(p2);
          ps_list.push_back(p3);
          SpacePartition* sp = new SpacePartition();
          vector<Region> reg_list;
          sp->ComputeRegion(ps_list, reg_list);
//          tri_list.push_back(reg_list[0]);
//          if(reg_list.size() > 0){ /////////2012.3.9 rounding problem 
            tri_list.push_back(reg_list[0]);
            v1_list.push_back(poly.p_id_list[index1]);
            v2_list.push_back(poly.p_id_list[index2]);
            v3_list.push_back(poly.p_id_list[index3]);
//          }
          delete sp;
      }

      delete ct;
}

/*
For each triangle, it sets the number of neighbors it has already.
If the numbers of two vertices are consecutive and they belong to the same
cycle, then the edge connecting them is the boundary. Thus, there is no triangle
adjacent to it by this edge. So we increase the number of neighbor. As for a
triangle, the maximum neighbor it can have is 3 (three edges).

*/

void SetNeighbor(Triangle& tri, vector<int>& no_points_cycles,
                 vector<int>& index_contour)
{
  //v1 and v2 are consecutive boundary points
  if(tri.c1 == tri.c2){
    if(fabs(tri.v1 - tri.v2) == 1){
      tri.neighbor_no++;
    }

    else if(tri.v1 == index_contour[tri.c1] && //first
          tri.v2 == index_contour[tri.c1] + no_points_cycles[tri.c2] - 1)//last
      tri.neighbor_no++;

    else if(tri.v2 == index_contour[tri.c1] &&
            tri.v1 == index_contour[tri.c1] + no_points_cycles[tri.c1] - 1)
      tri.neighbor_no++;
  }
  //v1 and v3 are consecutive boundary points
  if(tri.c1 == tri.c3){
      if(fabs(tri.v1 - tri.v3) == 1){
        tri.neighbor_no++;
      }

      else if(tri.v1 == index_contour[tri.c1] &&
              tri.v3 == index_contour[tri.c3] + no_points_cycles[tri.c3] - 1){
        tri.neighbor_no++;
      }

      else if(tri.v3 == index_contour[tri.c3] &&
              tri.v1 == index_contour[tri.c1] + no_points_cycles[tri.c1] - 1)
        tri.neighbor_no++;
  }
  //v2 and v3 are consecutive boundary points
  if(tri.c2 == tri.c3){
      if(fabs(tri.v2 - tri.v3) == 1)
        tri.neighbor_no++;

      else if(tri.v2 == index_contour[tri.c2] &&
              tri.v3 == index_contour[tri.c3] + no_points_cycles[tri.c3] - 1)
        tri.neighbor_no++;

      else if(tri.v3 == index_contour[tri.c3] &&
              tri.v2 == index_contour[tri.c2] + no_points_cycles[tri.c2] - 1){
        tri.neighbor_no++;
      }
  }
}

/*
get the dual graph edge relation. if two triangles have the same edge, an edge
in dual graph is created. Each node corresponds to a triangle.
1. number the vertex. order them by z-order
2. use a list
3. find the neighbor by the number of vertex instead of intersection detecting

*/
void RegVertex::GetDGEdge()
{
  //the number it stores is the number of points inside
  //cycleno->number of points
  vector<int> no_points_cycles(rel2->GetNoTuples(), 0);

  vector<int> vertex_cycleno; //vertex -> cycleno
  vector<Point>  vertex_point;
  //relation (cycleno int)(vertex point)
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* t = rel2->GetTuple(i, false);
    int cycleno = ((CcInt*)t->GetAttribute(DualGraph::CYCLENO))->GetIntval();
    Point* p = (Point*)t->GetAttribute(DualGraph::VERTEX);
    vertex_cycleno.push_back(cycleno);
    vertex_point.push_back(*p);
    t->DeleteIfAllowed();
    no_points_cycles[cycleno]++;
  }

  vector<int> index_contour;
  unsigned int no_cyc = 0;
  for(unsigned int i = 0;i < no_points_cycles.size();i++){
//    cout<<"no_cyc "<<i<<" points "<<no_points_cycles[i]<<endl;
    if(i == 0) index_contour.push_back(1);
    else index_contour.push_back(index_contour[index_contour.size() - 1] +
                                 no_points_cycles[i - 1]);

//    cout<<"start index "<<index_contour[index_contour.size() - 1]<<endl;
    if(no_points_cycles[i] > 0) no_cyc++;
    else
      break;
  }

//  cout<<"no of cycles "<<no_cyc<<endl;

  TriNode* head = new TriNode();
  TriNode* trinode;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
      Tuple* t = rel1->GetTuple(i, false);
      int v1 = ((CcInt*)t->GetAttribute(DualGraph::V1))->GetIntval();
      int v2 = ((CcInt*)t->GetAttribute(DualGraph::V2))->GetIntval();
      int v3 = ((CcInt*)t->GetAttribute(DualGraph::V3))->GetIntval();
      int oid = ((CcInt*)t->GetAttribute(DualGraph::TOID))->GetIntval();
      int c1 = vertex_cycleno[v1 - 1];
      int c2 = vertex_cycleno[v2 - 1];
      int c3 = vertex_cycleno[v3 - 1];

/*      if(!(oid == 93 || oid == 94 || oid == 95)){
        t->DeleteIfAllowed();
        continue;
      }*/

      Triangle tri(oid, v1, v2, v3, c1, c2, c3);
      SetNeighbor(tri, no_points_cycles, index_contour);

//      cout<<"new elem ";
//      tri.Print();

      trinode = new TriNode(tri,NULL);
      ////////////////traverse the list //////////////////////////////
      if(i == 1){
        head->next = trinode;
        t->DeleteIfAllowed();
        continue;
      }
      TriNode* prev = head;
      TriNode* cur = prev->next;
      bool insert = true;
      while(cur != NULL && insert){
//          cout<<" cur elem in list ";
//          cur->tri.Print();

          int sharetype = trinode->tri.ShareEdge(cur->tri);
//          cout<<"sharetype "<<sharetype<<endl;
          if(sharetype > 0){ //find a common edge
//            cout<<"oid1 "<<trinode->tri.oid<<"oid2 "<<cur->tri.oid<<endl;

            ////////////adjacent list///////////////////////////////
            v1_list.push_back(trinode->tri.oid);
            v2_list.push_back(cur->tri.oid);
            HalfSegment hs;
            Point p1, p2;
            switch(sharetype){
              case 1:
                    p1 = vertex_point[tri.v1 - 1];
                    p2 = vertex_point[tri.v2 - 1];
                    break;
              case 2:
                    p1 = vertex_point[tri.v1 - 1];
                    p2 = vertex_point[tri.v3 - 1];
                    break;
              case 3:
                    p1 = vertex_point[tri.v2 - 1];
                    p2 = vertex_point[tri.v3 - 1];
                    break;
              default:
                    assert(false);
            }

/*            hs.Set(true, p1, p2);
            Line* l = new Line(0);
            l->StartBulkLoad();
            hs.attr.edgeno = 0;
            *l += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *l += hs;
            l->EndBulkLoad();
            line.push_back(*l);
            delete l;*/


            Line* l = new Line(0);
            l->StartBulkLoad();
            if(!AlmostEqual(p1, p2)){////////////2012.3.9
              hs.Set(true, p1, p2);
              hs.attr.edgeno = 0;
              *l += hs;
              hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
              *l += hs;
            }
            l->EndBulkLoad();
            line.push_back(*l);
            delete l;


            //////////////////////////////////////////////////////
            //trinode
            trinode->tri.neighbor_no++;
            if(trinode->tri.neighbor_no == 3){
//              cout<<"do not insert new"<<endl;
              insert = false;
              delete trinode;
            }

            //cur
            cur->tri.neighbor_no++;
            if(cur->tri.neighbor_no == 3){//find all neighbors
//              cout<<"delete cur "<<endl;
              TriNode* temp = cur;
              prev->next = cur->next;
              cur = cur->next;
              delete temp;
            }else{
                 prev = cur;
                 cur = cur->next;
            }
          }else{
                prev = cur;
                cur = cur->next;
          }
      }
      if(insert){
        prev->next = trinode;
      }
      ///////////////////////////////////////////////////////////////
      t->DeleteIfAllowed();
  }
  //for debuging, detect whether there are some elements not processed
  if(head->next){
      cout<<"it should not come here. ";
      cout<<"there are some elements not processed in the list"<<endl;
      TriNode* temp = head->next;
      while(temp){
        temp->tri.Print();
        cout<<vertex_point[temp->tri.v1 - 1]<<" "
            <<vertex_point[temp->tri.v2 - 1]<<" "
            <<vertex_point[temp->tri.v3 - 1]<<" "<<endl;

        temp = temp->next;
      }
      assert(false);
  }
  delete head;

}

/*
two triangles sharing the same edge

*/
void RegVertex::ShareEdge(Region* reg1, Region* reg2, int oid1,
                int oid2, vector<vector<int> >& adj_node)
{
  for(int i = 0;i < reg1->Size();i++){
      HalfSegment hs1;
      reg1->Get(i, hs1);
     for(int j = 0;j < reg2->Size();j++){
        HalfSegment hs2;
        reg2->Get(j, hs2);
        if((AlmostEqual(hs1.GetLeftPoint(),  hs2.GetLeftPoint())&&
            AlmostEqual(hs1.GetRightPoint(), hs2.GetRightPoint())) ||
           (AlmostEqual(hs1.GetRightPoint(), hs2.GetLeftPoint()) &&
            AlmostEqual(hs1.GetLeftPoint(),  hs2.GetRightPoint()))){
            if(adj_node[oid2 - 1].size() == 0){
              Line* l =  new Line(0);
              l->StartBulkLoad();
              hs1.attr.edgeno = 0;
              *l += hs1;
              hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
              *l += hs1;
              l->EndBulkLoad();
              line.push_back(*l);
              delete l;
              v1_list.push_back(oid1);
              v2_list.push_back(oid2);
              adj_node[oid1 - 1].push_back(oid2);
            }else{
              unsigned int index = 0;
              for(;index < adj_node[oid2 - 1].size();index++){
                if(adj_node[oid2 - 1][index] == oid1)break;
              }
              if(index == adj_node[oid2 - 1].size()){
                Line* l =  new Line(0);
                l->StartBulkLoad();
                hs1.attr.edgeno = 0;
                *l += hs1;
                hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
                *l += hs1;
                l->EndBulkLoad();
                line.push_back(*l);
                delete l;
                v1_list.push_back(oid1);
                v2_list.push_back(oid2);
                adj_node[oid1 - 1].push_back(oid2);
              }
            }
            return;
        }
    }
  }

}

/*
Using depth first method to travese the R-tree to find all neighbors for the
triangle (oid,reg)

*/
void RegVertex::DFTraverse(R_Tree<2,TupleId>* rtree,
                           SmiRecordId adr,int oid, Region* reg,
                           vector<vector<int> >& adj_node)
{
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple2 = rel1->GetTuple(e.info,false);
              Region* candi_reg =
                     (Region*)dg_tuple2->GetAttribute(DualGraph::PAVEMENT);
//              cout<<"e.info "<<e.info<<endl;
//              cout<<"Area "<<candi_reg->Area()<<endl;
              int cand_oid = 
                 ((CcInt*)dg_tuple2->GetAttribute(DualGraph::OID))->GetIntval();
//              if(oid != (int)e.info){
              if(oid != cand_oid){
//                  ShareEdge(reg, candi_reg, oid, e.info, adj_node);
                  ShareEdge(reg, candi_reg, oid, cand_oid, adj_node);
              }
              dg_tuple2->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
//            if(reg->Intersects(e.box)){
            if(reg->Intersects(Region(e.box))){
              DFTraverse(rtree, e.pointer, oid, reg, adj_node);
            }
      }
  }
  delete node;
}

/*
for each triangle, it searches all its neighbors by traversing the RTree
it creates the edge relation for dual graph.
depth-first is a little faster than breadth-first
in create berlin and houston data, we have an unique oid for each IFOB.
so the value of oid may not start from 1. not the same as tuple id 
!!!Be Careful!!!

*/
void RegVertex::GetDGEdgeRTree(R_Tree<2,TupleId>* rtree)
{
  SmiRecordId adr = rtree->RootRecordId();
  
  int max_no = 0;
  for(int i = 1;i <= rel1->GetNoTuples();i++){
     Tuple* dg_tuple = rel1->GetTuple(i, false);
     int oid = ((CcInt*)dg_tuple->GetAttribute(DualGraph::OID))->GetIntval();
     if(oid > max_no) max_no = oid;
     dg_tuple->DeleteIfAllowed();
  }

//  vector<vector<int> > adj_node(rel1->GetNoTuples());
  vector<vector<int> > adj_node(max_no);//oid may not start from 1, e.g., 3000

  for(int i = 1;i <= rel1->GetNoTuples();i++){
//  for(int i = 1;i <= 500;i++){
      Tuple* dg_tuple1 = rel1->GetTuple(i, false);
      int oid = ((CcInt*)dg_tuple1->GetAttribute(DualGraph::OID))->GetIntval();
      Region* reg =
             (Region*)dg_tuple1->GetAttribute(DualGraph::PAVEMENT);
      ///////////////////traverse RTree//////////////////////////////////
/*      queue<SmiRecordId> record_list;
      record_list.push(adr);
      while(record_list.empty() == false){ // width first method
          SmiRecordId node_recid = record_list.front();
          record_list.pop();
          R_TreeNode<2,TupleId>* node = rtree->GetMyNode(node_recid,false,
                          rtree->MinEntries(0), rtree->MaxEntries(0));
          for(int j = 0;j < node->EntryCount();j++){
            if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple2 = rel1->GetTuple(e.info,false);
              Region* candi_reg =
                     (Region*)dg_tuple2->GetAttribute(DualGraph::PAVEMENT);
              if(oid != e.info){
//                  cout<<"find neighbor "<<oid<<" "<<e.info<<endl;
                  ShareEdge(reg, candi_reg,oid,e.info, adj_node);
              }
              dg_tuple2->DeleteIfAllowed();
            }else{
              R_TreeInternalEntry<2> e =
                  (R_TreeInternalEntry<2>&)(*node)[j];
              if(reg->Intersects(e.box))
                  record_list.push(e.pointer);
            }
          }
        delete node;
      }*/
      /////////////////depth first method////////////////////////////////////
      DFTraverse(rtree, adr, oid, reg, adj_node);
      //////////////////////////////////////////////////////////////////////
      dg_tuple1->DeleteIfAllowed();
  }
}

string VisualGraph::NodeTypeInfo =
  "(rel(tuple((Oid int)(Loc point))))";
string VisualGraph::EdgeTypeInfo =
  "(rel(tuple((Oid1 int)(Oid2 int)(Connection line))))";
string VisualGraph::QueryTypeInfo =
  "(rel(tuple((Oid int)(Loc1 point)(Loc2 point))))";

VisualGraph::~VisualGraph()
{
//  cout<<"~VisualGraph()"<<endl;
}

VisualGraph::VisualGraph()
{
//  cout<<"VisualGraph::VisualGraph()"<<endl;
}

VisualGraph::VisualGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect)
{
//  cout<<"VisualGraph::VisualGraph(ListExpr)"<<endl;
}

VisualGraph::VisualGraph(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
//   cout<<"VisualGraph::VisualGraph(SmiRecord)"<<endl;
   /***********************Read graph id********************************/
  in_xValueRecord.Read(&g_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);


  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for node*********************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  node_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!node_rel) {
    return;
  }
  /***********************Open relation for edge*********************/
  nl->ReadFromString(EdgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel) {
    node_rel->Delete();
    return;
  }

  ////////////////////adjaency list////////////////////////////////
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

}

void VisualGraph::Load(int id, Relation* r1, Relation* r2)
{
//  cout<<"VisualGraph::Load()"<<endl;
  g_id = id;
  //////////////////node relation////////////////////
  ListExpr ptrList1 = listutils::getPtrList(r1);

  string strQuery = "(consume(sort(feed(" + NodeTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;

  /////////////////edge relation/////////////////////
  ListExpr ptrList2 = listutils::getPtrList(r2);
  
  strQuery = "(consume(sort(feed(" + EdgeTypeInfo +
                "(ptr " + nl->ToString(ptrList2) + ")))))";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel = (Relation*)xResult.addr;

  ////////////adjacency list ////////////////////////////////
  ListExpr ptrList3 = listutils::getPtrList(edge_rel);

  strQuery = "(createbtree (" + EdgeTypeInfo +
             "(ptr " + nl->ToString(ptrList3) + "))" + "Oid1)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree_node_oid1 = (BTree*)xResult.addr;


//  cout<<"b-tree on edge is finished....."<<endl;

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    CcInt* nodeid = new CcInt(true, i);
    BTreeIterator* btree_iter1 = btree_node_oid1->ExactMatch(nodeid);
    int start = adj_list.Size();
//    cout<<"start "<<start<<endl;
    while(btree_iter1->Next()){
      Tuple* edge_tuple = edge_rel->GetTuple(btree_iter1->GetId(), false);
      int oid = ((CcInt*)edge_tuple->GetAttribute(OIDSECOND))->GetIntval();
      adj_list.Append(oid);
      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter1;

    int end = adj_list.Size();
    entry_adj_list.Append(ListEntry(start, end));
//    cout<<"end "<<end<<endl;
    delete nodeid;
  }

  delete btree_node_oid1;

}

ListExpr VisualGraph::OutVisualGraph(ListExpr typeInfo, Word value)
{
//  cout<<"OutVisualGraph()"<<endl;
  VisualGraph* vg = (VisualGraph*)value.addr;
  return vg->Out(typeInfo);
}

ListExpr VisualGraph::Out(ListExpr typeInfo)
{
//  cout<<"Out()"<<endl;
  ListExpr xNode = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= edge_rel->GetNoTuples();i++){
      Tuple* edge_tuple = edge_rel->GetTuple(i, false);
      CcInt* oid1 = (CcInt*)edge_tuple->GetAttribute(OIDFIRST);
      CcInt* oid2 = (CcInt*)edge_tuple->GetAttribute(OIDSECOND);
      Line* connection = (Line*)edge_tuple->GetAttribute(CONNECTION);

      ListExpr xline = OutLine(nl->TheEmptyList(),SetWord(connection));
      xNext = nl->FourElemList(nl->IntAtom(g_id),
                               nl->IntAtom(oid1->GetIntval()),
                               nl->IntAtom(oid2->GetIntval()),
                               xline);
      if(bFirst){
        xNode = nl->OneElemList(xNext);
        xLast = xNode;
        bFirst = false;
      }else
          xLast = nl->Append(xLast,xNext);
      edge_tuple->DeleteIfAllowed();
  }
  return nl->TwoElemList(nl->IntAtom(g_id),xNode);
}

bool VisualGraph::CheckVisualGraph(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckVisualGraph()"<<endl;
  return nl->IsEqual(type, "visualgraph");
}

void VisualGraph::CloseVisualGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseVisualGraph()"<<endl;
  delete static_cast<VisualGraph*> (w.addr);
  w.addr = NULL;
}

void VisualGraph::DeleteVisualGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteVisualGraph()"<<endl;
  VisualGraph* vg = (VisualGraph*)w.addr;
  delete vg;
  w.addr = NULL;
}

Word VisualGraph::CreateVisualGraph(const ListExpr typeInfo)
{
//  cout<<"CreateVisualGraph()"<<endl;
  return SetWord(new VisualGraph());
}

Word VisualGraph::InVisualGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
//  cout<<"InVisualGraph()"<<endl;
  VisualGraph* vg = new VisualGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(vg);
  else{
    delete vg;
    return SetWord(Address(0));
  }
}


bool VisualGraph::OpenVisualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenVisualGraph()"<<endl;
  value.addr = VisualGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

VisualGraph* VisualGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{

  return new VisualGraph(valueRecord,offset,typeInfo);
}

bool VisualGraph::SaveVisualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveVisualGraph()"<<endl;
  VisualGraph* vg = (VisualGraph*)value.addr;
  bool result = vg->Save(valueRecord, offset, typeInfo);

  return result;
}

bool VisualGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo)
{

//  cout<<"Save()"<<endl;
  /********************Save graph id ****************************/
  in_xValueRecord.Write(&g_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);


  ListExpr xType;
  ListExpr xNumericType;
  /************************save node****************************/
  nl->ReadFromString(NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /************************save edge****************************/
  nl->ReadFromString(EdgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;


   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();
   adj_list.saveToFile(rf, adj_list);
   SmiSize offset = 0;
   size_t bufsize = adj_list.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   adj_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list.saveToFile(rf, entry_adj_list);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;

  return true;

}

/*
checks whether the region halfsegment self intersects

*/
bool Hole::NoSelfIntersects(Region* r)
{
    for(int i = 0;i < r->Size();i++){
        HalfSegment hs1;
        r->Get(i, hs1);
        if(!hs1.IsLeftDomPoint())continue;
        for(int j = 0;j < r->Size();j++){
          HalfSegment hs2;
          r->Get(j, hs2);
          if(!hs2.IsLeftDomPoint() || i == j)continue;
          Point ip;
          if(hs1.Intersection(hs2, ip)){
              if(!AlmostEqual(ip, hs1.GetLeftPoint()) &&
                 !AlmostEqual(ip, hs1.GetRightPoint()) &&
                 !AlmostEqual(ip, hs2.GetLeftPoint()) &&
                 !AlmostEqual(ip, hs2.GetRightPoint()))
                  return false;
          }
        }
    }
    return true;
}
/*
collect the holes of a region

*/
void Hole::GetHole(Region* r)
{
  CompTriangle* ct = new CompTriangle(r);
  unsigned int no_cyc = ct->NoOfCycles();//ignore the first cycle
  delete ct;

//  cout<<"holes "<<no_cyc<<endl;

  vector<int> edgeno;
  vector<int> partnerno;
  for(unsigned int i = 0;i < no_cyc;i++){
      Region* new_reg = new Region(0);
      new_reg->StartBulkLoad();
      new_reg->EndBulkLoad();
      regs.push_back(*new_reg);
      delete new_reg;
      edgeno.push_back(0);
      partnerno.push_back(0);
  }
//  cout<<"here "<<r->Size()<<endl;
  for(unsigned int i = 0;i < no_cyc;i++){
      regs[i].StartBulkLoad();
  }

  for(int i = 0;i < r->Size();i++){
//      cout<<"i1 "<<i<<endl;
      HalfSegment hs;
      r->Get(i, hs);
      if(!hs.IsLeftDomPoint()) continue;
      HalfSegment temp_hs;
      temp_hs.Set(true, hs.GetLeftPoint(), hs.GetRightPoint());
      temp_hs.SetLeftDomPoint(hs.IsLeftDomPoint());
      temp_hs.attr.faceno = 0;
      int cycle_no = hs.attr.cycleno;
//      cout<<"cycle_no "<<cycle_no<<endl;
      temp_hs.attr.cycleno = 0;
      temp_hs.attr.edgeno = edgeno[cycle_no]++;
      temp_hs.attr.partnerno = partnerno[cycle_no]++;
      temp_hs.attr.insideAbove = !temp_hs.attr.insideAbove;
      regs[cycle_no] += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      regs[cycle_no] += hs;
//      cout<<hs<<endl;
  }
  for(unsigned int i = 0;i < no_cyc;i++){
//      cout<<"i "<<i<<endl;
      regs[i].SetNoComponents(1);
      regs[i].EndBulkLoad(true, false, false, false);
  }

}

/*
from the input data file, it creates a lot of contours
collect regions from original data, where each tuple is a halfsegment

*/
struct MHSNode{
  MyHalfSegment mhs;
  MHSNode* next;
  MHSNode(MyHalfSegment hs, MHSNode* pointer):mhs(hs),next(pointer){}
};

bool RegionCom(const Region& r1, const Region& r2)
{
  return r1.Area() > r2.Area();
}

/*
query getcontour('Transportationmode/Polygon-Data/GermanyPPPoly') count;
read polygon data from a file

*/
void Hole::GetContour()
{
    Coord x1, y1, x2, y2;
    int index = 0;
    MHSNode* head = new MHSNode(MyHalfSegment(),NULL);

    Points* ps = new Points(0);
    ps->StartBulkLoad();

    while( !in.eof()){
      in>>index;
      if(!in.good())
          break;
      in>>x1;
      if(!in.good())
          break;
      in>>y1;
      if(!in.good())
          break;
      in>>x2;
      if(!in.good())
          break;
      in>>y2;
      if(!in.good())
        break;
//      printf("%.12f %.12f %.12f %.12f\n",x1,y1,x2,y2);
      Point p1(true,x1,y1);
      Point p2(true,x2,y2);
      MyHalfSegment hs(true,p1,p2);
      if(AlmostEqual(p1,p2)) continue;
//      if(index > 10000)break;

      MHSNode* node = new MHSNode(hs,NULL);
      node->next = head->next;
      head->next = node;

      *ps += p1;
      *ps += p2;
    }
    ps->EndBulkLoad();

    BBox<2> bbox;
    double min[2];
    double max[2];
    const double delta = 5.0;

    min[0] = ps->BoundingBox().MinD(0) - delta;
    min[1] = ps->BoundingBox().MinD(1) - delta;
    max[0] = ps->BoundingBox().MaxD(0) + delta;
    max[1] = ps->BoundingBox().MaxD(1) + delta;
    bbox.Set(true, min, max);

    Region* outer_contour = new Region(bbox);
//    GrahamScan::convexHull(ps,outer_contour);
    regs.push_back(*outer_contour);//the first is the outer contour
    delete outer_contour;


  ////////////////////////////////////////////////
    while(head->next != NULL){
        list<MyHalfSegment> contours;
        MHSNode* prev = head;
        MHSNode* cur = head->next;
        contours.push_back(cur->mhs);

        prev->next = cur->next;
        MHSNode* temp = cur;
        delete temp;
        MyHalfSegment front = contours.front();
        MyHalfSegment back = contours.back();

        while(1){
          cur = head->next;
          prev = head;
          while(cur != NULL){
            if(AlmostEqual(front.from, cur->mhs.to)){
                contours.push_front(cur->mhs);
                prev->next = cur->next;
                delete cur;
                break;
            }
            if(AlmostEqual(front.from, cur->mhs.from)){
                cur->mhs.Exchange();
                contours.push_front(cur->mhs);
                prev->next = cur->next;
                delete cur;
                break;
            }
            if(AlmostEqual(back.to, cur->mhs.from)){
                contours.push_back(cur->mhs);
                prev->next = cur->next;
                delete cur;
                break;
            }
            if(AlmostEqual(back.to, cur->mhs.to)){
                cur->mhs.Exchange();
                contours.push_back(cur->mhs);
                prev->next = cur->next;
                delete cur;
                break;
            }
            prev = cur;
            cur = cur->next;
          }
          front = contours.front();
          back = contours.back();
//          cout<<"front ";front.Print();
//          cout<<"back "; back.Print();
          if(AlmostEqual(front.from, back.to) && contours.size() > 2){
//              cout<<"cycle found "<<contours.size()<<endl;
              vector<Point> ps;
              while(contours.empty() == false){
                  MyHalfSegment top = contours.front();
                  contours.pop_front();
                  ps.push_back(top.from);
              }
              SpacePartition* sp = new SpacePartition();
              CompTriangle* ct = new CompTriangle();
              vector<Region> regions;
              if(ct->Area(ps) > 0){
                sp->ComputeRegion(ps,regions);
                regs1.push_back(regions[0]);
                regs2.push_back(regions[0]);
              }else{
                vector<Point> temp_ps;
                for(int i = ps.size() - 1;i >= 0;i--)
                  temp_ps.push_back(ps[i]);
                sp->ComputeRegion(temp_ps,regions);
                regs1.push_back(regions[0]);
                regs2.push_back(regions[0]);
              }
              delete ct;
              delete sp;
              contours.clear();
              break;
          }
          if(cur == NULL){
              contours.clear();
              break;
          }
        }
    }
    sort(regs1.begin(),regs1.end(), RegionCom);
    sort(regs2.begin(),regs2.end(), RegionCom);

    for(unsigned int i = 0;i < regs1.size();i++){
        unsigned int j = 0;
//        cout<<regs1[i].Area()<<endl;
        for(;j < regs2.size();j++){
          if(i == j) continue;
/*          if(regs1[i].Inside(regs2[j]) ||
             regs1[i].Intersects(regs2[j]) ||
             regs2[j].Inside(regs1[i])) break;*/
          if(regs1[i].Intersects(regs2[j])) break;

        }
        //no self-intersection segments are allowed
        if(j == regs2.size() &&
           NoSelfIntersects(&regs1[i]))regs.push_back(regs1[i]);
    }
    delete head;
    delete ps;
}

/*
create no_reg regions(cycles)

*/
void Hole::GetContour(unsigned int no_reg)
{
    vector<Point> contour;
    vector<Region> regions;
    SpacePartition* sp = new SpacePartition();
    //////////////////the outer contour/////////////////////////////////
    contour.push_back(Point(true,0.0,0.0));
    contour.push_back(Point(true,100000.0,0.0));
    contour.push_back(Point(true,100000.0,100000.0));
    contour.push_back(Point(true,0.0,100000.0));
    sp->ComputeRegion(contour,regions);
    regs.push_back(regions[0]);
    ////////////////////////////////////////////////////////
//    struct timeval tval;
//    struct timezone tzone;
//    gettimeofday(&tval, &tzone);
//    srand48(tval.tv_sec);

    unsigned int reg_count = no_reg;

    while(no_reg > 1){

//      int  px = lrand48() % 98700 + 600;
//      int  py = lrand48() % 98700 + 600;
      int px = GetRandom() % 98700 + 600;
      int py = GetRandom() % 98700 + 600;

//      int radius = lrand48() % 496 + 5;//5-500 10-1000

      int radius = GetRandom() % 496 + 5;//5-500 10-1000 

      contour.clear();
      regions.clear();
      contour.push_back(Point(true, px - radius, py - radius));
      contour.push_back(Point(true, px + radius, py - radius));
      contour.push_back(Point(true, px + radius, py + radius));
      contour.push_back(Point(true, px - radius, py + radius));
      sp->ComputeRegion(contour,regions);
      unsigned int i = 1;
      for(;i < regs.size();i++){
        if(regs[i].Intersects(regions[0]))break;
      }
      if(i == regs.size()){
          ///create convex hull
          Points* outer_ps = new Points(0);
//          int outer_ps_no =  lrand48() % 191 + 10;///10-200

          int outer_ps_no =  GetRandom() % 191 + 10;///10-200

//          int outer_ps_no =  radius;///10-200
          double xmin = px - radius;
          double ymin = py - radius;
          outer_ps->StartBulkLoad();
          vector<Point> temp_ps;
          while(outer_ps_no > 0){
//            int x =  lrand48() %(2*radius*100);
//            int y =  lrand48() %(2*radius*100);

            int x =  GetRandom() %(2*radius*100);
            int y =  GetRandom() %(2*radius*100);

            double coord_x = x/100.0 + xmin;
            double coord_y = y/100.0 + ymin;
            Point newp(true,coord_x,coord_y);

            if(newp.Inside(regions[0])){
              outer_ps_no--;
              *outer_ps += newp;
              temp_ps.push_back(newp);
            }
          }
          outer_ps->EndBulkLoad();

          if(no_reg % 3 == 0){
            Region* newregion = new Region(0);
            GrahamScan::convexHull(outer_ps,newregion);
            regs.push_back(*newregion);
            delete newregion;
          }
          else if(no_reg % 3 == 1){
            Region* newregion = new Region(0);
            DiscoverContour(outer_ps,newregion);
            regs.push_back(*newregion);
            delete newregion;
          }
          else{
            Region* newregion = new Region(outer_ps->BoundingBox());
            regs.push_back(*newregion);
            delete newregion;
          }
          delete outer_ps;
          /////////////////////////////////////////////////
//          regs.push_back(regions[0]);
          no_reg--;
      }
    }
    delete sp;
    assert(reg_count == regs.size());
}

void Hole::SpacePartitioning(vector<Point> ps, vector<HalfSegment>& hs_segs,
                         Point sf, Point sl)
{
//    cout<<"sf "<<sf<<" sl "<<sl<<endl;
/*    for(unsigned int i = 0;i < ps.size();i++)
        cout<<ps[i]<<endl; */

    if(ps.size() == 0){
        HalfSegment hs;
        hs.Set(true, sf, sl);
        hs_segs.push_back(hs);
    }else{
        Point mid_p1 = ps[0];
        Point mid_p2(true, (sf.GetX()+sl.GetX())/2,(sf.GetY()+sl.GetY())/2);

        if(AlmostEqual(mid_p1.GetX(), mid_p2.GetX())){
          //it seldomly happens
          if(AlmostEqual(sf.GetX(), sl.GetX())) assert(false);
          vector<Point> ls;
          vector<Point> rs;
          for(unsigned int i = 0;i < ps.size();i++){
            if(AlmostEqual(ps[i], mid_p1) ||
               AlmostEqual(ps[i], mid_p1)) continue;
            if(ps[i].GetX() < mid_p1.GetX() ||
               AlmostEqual(ps[i].GetX(), mid_p1.GetX()))
                ls.push_back(ps[i]);
            else
                rs.push_back(ps[i]);
          }

          if(sf.GetX() < mid_p1.GetX()){
//              cout<<"1 "<<"left "<<endl;
              SpacePartitioning(ls, hs_segs, sf, mid_p1);
//              cout<<"1 "<<"right "<<endl;
              SpacePartitioning(rs, hs_segs, mid_p1, sl);
          }else{
//              cout<<"1 "<<"left "<<endl;
              SpacePartitioning(ls, hs_segs, sl, mid_p1);
//              cout<<"1 "<<"right "<<endl;
              SpacePartitioning(rs, hs_segs, mid_p1, sf);
          }

        }else if(AlmostEqual(mid_p1.GetY(), mid_p2.GetY())){
            //it seldomly happens
            if(AlmostEqual(sf.GetY(), sl.GetY())) assert(false);
            vector<Point> ls;
            vector<Point> rs;
            for(unsigned int i = 0;i < ps.size();i++){
              if(AlmostEqual(ps[i], mid_p1) ||
                 AlmostEqual(ps[i], mid_p1)) continue;
              if(ps[i].GetY() < mid_p1.GetY() ||
                 AlmostEqual(ps[i].GetY(), mid_p1.GetY()))
                ls.push_back(ps[i]);
              else
                rs.push_back(ps[i]);
            }
            if(sf.GetY() < mid_p1.GetY()){
//                cout<<"2 "<<"left "<<endl;
                SpacePartitioning(ls, hs_segs, sf, mid_p1);
//                cout<<"2 "<<"right "<<endl;
                SpacePartitioning(rs, hs_segs, mid_p1, sl);
            }else{
//                cout<<"2 "<<"left "<<endl;
                SpacePartitioning(ls, hs_segs, sl, mid_p1);
//                cout<<"2 "<<"right "<<endl;
                SpacePartitioning(rs, hs_segs, mid_p1, sf);
            }
        }else{
            vector<Point> ls;
            vector<Point> rs;
            double a = (mid_p1.GetY() - mid_p2.GetY())/
                       (mid_p1.GetX() - mid_p2.GetX());
            double b = mid_p1.GetY() - a*mid_p1.GetX();
            for(unsigned int i = 0;i < ps.size();i++){
              if(AlmostEqual(ps[i], mid_p1) ||
                 AlmostEqual(ps[i], mid_p1)) continue;
              if(ps[i].GetY() < (a*ps[i].GetX() + b)||
                AlmostEqual(ps[i].GetY(), (a*ps[i].GetX() + b)))
                ls.push_back(ps[i]);
              else
                rs.push_back(ps[i]);
            }
            if(sf.GetY() < a*sf.GetX() + b){
//              cout<<"3 "<<"left "<<endl;
              SpacePartitioning(ls, hs_segs, sf, mid_p1);
//              cout<<"3 "<<"right "<<endl;
              SpacePartitioning(rs, hs_segs, mid_p1, sl);
            }else{
//              cout<<"3 "<<"left "<<endl;
              SpacePartitioning(ls, hs_segs, sl, mid_p1);
//              cout<<"3 "<<"right "<<endl;
              SpacePartitioning(rs, hs_segs, mid_p1, sf);
            }
          }
    }
}

/*
create a polygon by SpacePartitioning method

*/

void Hole::SpacePartitioning(Points* gen_ps, vector<HalfSegment>& hs_segs)
{
  vector<Point> ps1;
  for(int i = 0;i < gen_ps->Size();i++){
      Point temp_p;
      gen_ps->Get(i, temp_p);
      ps1.push_back(temp_p);
  }

  Point sf = ps1[0];
  Point sl = ps1[1];
//cout<<"sf "<<sf<<" sl "<<sl<<endl;
/*  for(unsigned int i = 0;i < ps1.size();i++)
    cout<<"ps1 "<<ps1[i]<<endl; */

  if(AlmostEqual(sf.GetX(), sl.GetX())){
      vector<Point> ls;
      vector<Point> rs;
      for(unsigned int i = 0;i < ps1.size();i++){
        if(AlmostEqual(ps1[i], sf) || AlmostEqual(ps1[i],sl)) continue;
        if(ps1[i].GetX() < sf.GetX() || AlmostEqual(ps1[i].GetX(), sf.GetX()))
            ls.push_back(ps1[i]);
        else
            rs.push_back(ps1[i]);
      }
//      cout<<"1 "<<"left "<<endl;
      SpacePartitioning(ls, hs_segs, sf, sl);
//      cout<<"1 "<<"right "<<endl;
      SpacePartitioning(rs, hs_segs, sf, sl);
  }else if(AlmostEqual(sf.GetY(), sl.GetY())){
      vector<Point> ls;
      vector<Point> rs;
      for(unsigned int i = 0;i < ps1.size();i++){
        if(AlmostEqual(ps1[i], sf) || AlmostEqual(ps1[i],sl)) continue;
        if(ps1[i].GetY() < sf.GetY() || AlmostEqual(ps1[i].GetY(), sf.GetY()))
            ls.push_back(ps1[i]);
        else
            rs.push_back(ps1[i]);
      }
//      cout<<"2 "<<"left "<<endl;
      SpacePartitioning(ls, hs_segs, sf, sl);
//      cout<<"2 "<<"right "<<endl;
      SpacePartitioning(rs, hs_segs, sf, sl);
  }else{
      vector<Point> ls;
      vector<Point> rs;
      double a = (sf.GetY() - sl.GetY())/(sf.GetX() - sl.GetX());
      double b = sf.GetY() - a*sf.GetX();
      for(unsigned int i = 0;i < ps1.size();i++){
        if(AlmostEqual(ps1[i], sf) || AlmostEqual(ps1[i],sl)) continue;
        if(ps1[i].GetY() < (a*ps1[i].GetX() + b)||
           AlmostEqual(ps1[i].GetY(), (a*ps1[i].GetX() + b)))
            ls.push_back(ps1[i]);
        else
            rs.push_back(ps1[i]);
      }
//      cout<<"3 "<<"left "<<endl;
      SpacePartitioning(ls, hs_segs, sf, sl);
//      cout<<"3 "<<"right "<<endl;
      SpacePartitioning(rs, hs_segs, sf, sl);
  }

}
void Hole::GetPolygon(int no_ps)
{
    struct timeval tval;
    struct timezone tzone;

    gettimeofday(&tval, &tzone);
    srand48(tval.tv_sec);
    vector<Point> ps1;
    if(no_ps < 3){
       cout<<"at least three points"<<endl;
       return;
    }
    Points* gen_ps = new Points(0);
    gen_ps->StartBulkLoad();
    while(no_ps > 0){
//        int x =  lrand48() %100000;
//        int y =  lrand48() %100000;
        int x = GetRandom() % 100000;
        int y = GetRandom() % 100000;

        double coord_x = x/100.0;
        double coord_y = y/100.0;
        Point p(true,coord_x, coord_y);
        *gen_ps += p;
        no_ps--;
    }
    gen_ps->EndBulkLoad();
    Region* reg = new Region(0);
    DiscoverContour(gen_ps, reg);//store the polygon in regs
    regs.push_back(*reg);
    delete reg;
    delete gen_ps;
}

/*
using the space partitining method to create randomly polygon by a given set
of vertices

*/
void Hole::DiscoverContour(Points* gen_ps, Region* r)
{
  vector<HalfSegment> hs_segs;
  SpacePartitioning(gen_ps, hs_segs);
  MHSNode* head = new MHSNode(MyHalfSegment(),NULL);
  for(unsigned int i = 0;i < hs_segs.size();i++){
       MHSNode* node = new MHSNode(
              MyHalfSegment(true,hs_segs[i].GetLeftPoint(),
                                 hs_segs[i].GetRightPoint()),NULL);
        node->next = head->next;
        head->next = node;
  }
  DiscoverContour(head, r);
  delete head;
}
/*
given a set of segments, it discovers whether a cycle exists. if yes, a region
is returned

*/
void Hole::DiscoverContour(MHSNode* head, Region* r)
{
    list<MyHalfSegment> contours;
    MHSNode* prev = head;
    MHSNode* cur = head->next;
    contours.push_back(cur->mhs);

    prev->next = cur->next;
    MHSNode* temp = cur;
    delete temp;
    MyHalfSegment front = contours.front();
    MyHalfSegment back = contours.back();

    while(1){
        cur = head->next;
        prev = head;
        while(cur != NULL){
          if(AlmostEqual(front.from, cur->mhs.to)){
              contours.push_front(cur->mhs);
              prev->next = cur->next;
              delete cur;
              break;
          }
          if(AlmostEqual(front.from, cur->mhs.from)){
              cur->mhs.Exchange();
              contours.push_front(cur->mhs);
              prev->next = cur->next;
              delete cur;
              break;
          }
          if(AlmostEqual(back.to, cur->mhs.from)){
              contours.push_back(cur->mhs);
              prev->next = cur->next;
              delete cur;
              break;
          }
          if(AlmostEqual(back.to, cur->mhs.to)){
              cur->mhs.Exchange();
              contours.push_back(cur->mhs);
              prev->next = cur->next;
              delete cur;
              break;
          }
          prev = cur;
          cur = cur->next;
        }
        front = contours.front();
        back = contours.back();
        if(AlmostEqual(front.from, back.to) && contours.size() > 2){
            vector<Point> ps;
            while(contours.empty() == false){
                MyHalfSegment top = contours.front();
                contours.pop_front();
                ps.push_back(top.from);
            }
            SpacePartition* sp = new SpacePartition();
            vector<Region> regions;
            sp->ComputeRegion(ps,regions);
            *r = regions[0];
            delete sp;
            contours.clear();
            break;
        }
        if(cur == NULL){
            contours.clear();
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////
////////////////// get the maximum area rectangle in a region/////////
//////////////////////////////////////////////////////////////////////
bool SortByY(const MyPoint& mp1, const MyPoint& mp2)
{
  if(mp1.loc.GetY() < mp2.loc.GetY() ||
    AlmostEqual(mp1.loc.GetY(), mp2.loc.GetY())) return false; 
  return true; 
}

/*
locate which Quadrant the point belongs to 

*/
int GetQuadrant(double x, double y, Rectangle<2> reg_box)
{
  double m_x = (reg_box.MinD(0) + reg_box.MaxD(0))/2;
  double m_y = (reg_box.MinD(1) + reg_box.MaxD(1))/2; 
  if(x < m_x || AlmostEqual(x, m_x)){
    if(y < m_y || AlmostEqual(y, m_y ))return 3; 
    else
      return 2;
  }else{
    if(y < m_y || AlmostEqual(y, m_y ))return 4;
    else
      return 1;
  }
  return -1; 
}


/*
the region should only have one face. 
it can be convex or concave, but without holes 
if the polygon is a triangle, we can directly get the maximum rectangle 

*/
Rectangle<2> GetMaxRect(Region* reg)
{
  if(reg->NoComponents() != 1){
    cout<<"only one face is allowed"<<endl; 
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 

  }
  
  CompTriangle* ct = new CompTriangle(reg);
  unsigned int no_cyc = ct->NoOfCycles();
  
  if(no_cyc != 1){
    cout<<"has holes inside or no cycle"<<endl; 
    delete ct; 
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);

  }


  Line* boundary = new Line(0);
  reg->Boundary(boundary);

  SimpleLine* sboundary = new SimpleLine(0);
  sboundary->fromLine(*boundary);

  vector<MyHalfSegment> mhs;

  SpacePartition* sp = new SpacePartition();
  if(sboundary->Size() > 0)
    sp->ReorderLine(sboundary, mhs);
  else{
    cout<<"can't covert the boundary to a sline"<<endl;
    delete boundary;
    delete sboundary;
    delete sp;
    delete ct; 
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);

  }

  vector<Point> ps;
  for(unsigned int i = 0;i < mhs.size();i++)
    ps.push_back(mhs[i].from);

  if(ct->IsConvex(ps) == false){
    cout<<"concave polygon"<<endl; 
    delete ct; 
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
    
  }
  
  ///////////////////////////////////////////////////////////////////////
  /////////////// finish checking //////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
 
  Rectangle<2> reg_box = reg->BoundingBox(); 
//  cout<<"ps size "<<ps.size()<<endl;
  for(unsigned int i = 0;i < ps.size();i++){
    double x = ps[i].GetX();
    double y = ps[i].GetY(); 
    assert(!(x < 0.0 || y < 0.0));
    int range = GetQuadrant(x, y, reg_box);
    assert(range > 0); 
    switch(range){
      case 1:
            x = floor(x);
            y = floor(y);
            break;
      case 2:
            x = ceil(x);
            y = floor(y);
            break;
      case 3:
            x = ceil(x);
            y = ceil(y);
            break;
      case 4:
            x = floor(x);
            y = ceil(y); 
            break; 
    }
    ps[i].Set(x,y); 
//    cout<<"x "<<x<<" y "<<y<<endl;

    if(i > 0){
      Point p1 = ps[i - 1];
      Point p2 = ps[i]; 
      if(AlmostEqual(p1, p2)){
          delete boundary;
          delete sboundary;

          delete sp;
          delete ct; 

          return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
      }
    }

    /////////////////////////////////////////////////////////////////
    //more checking, three points collineation, cannot be a region///
    ////////////////////////////////////////////////////////////////
    if(ps.size() == 3){
      SpacePartition* sp = new SpacePartition();
      if(sp->Collineation(ps[0], ps[1], ps[2])){
        delete sp;
        return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 
      }

      delete sp;
    }
  }

  delete boundary;
  delete sboundary;

  if(AlmostEqual(ps[0], ps[ps.size() - 1])){
      delete sp;
      delete ct; 
      return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 
  }
  
  vector<Region> regs;
  sp->ComputeRegion(ps, regs);

  delete sp; 
  delete ct;
  
  
  if(ct->GetClockwise(ps) == false){
    vector<Point> temp_ps;
    for(int i = ps.size() - 1; i >= 0;i--)
      temp_ps.push_back(ps[i]); 
    ps.clear();
    for(unsigned int i = 0;i < temp_ps.size();i++)
      ps.push_back(temp_ps[i]); 
  }
  
  if(ps.size() >= 3){
      vector<GeomPoint> polygon; 
      for(unsigned int i = 0;i < ps.size();i++){
      GeomPoint gp(ps[i].GetX(), ps[i].GetY()); 
      polygon.push_back(gp); 
    }

    MaxRect max_rect; 
    max_rect.Init(); 
    max_rect.SetPoint(polygon); 
    if(max_rect.computeEdgeList()){
        max_rect.computeLargestRectangle(); 
        if(max_rect.result != -1)
          return max_rect.RectList[max_rect.result]; 
        else
          return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 
     }else
        return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 
  }

     return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 
}

/*
simple and approximate method to get a rectangle from a polygon.
the method above takes too much time

*/
Rectangle<2> GetMaxRect2(Region* reg)
{
  if(reg->NoComponents() != 1){
    cout<<"only one face is allowed"<<endl; 
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 

  }
  
  CompTriangle* ct = new CompTriangle(reg);
  unsigned int no_cyc = ct->NoOfCycles();
  
  if(no_cyc != 1){
    cout<<"has holes inside or no cycle"<<endl; 
    delete ct; 
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);

  }
  delete ct;

  Rectangle<2> bbox = reg->BoundingBox();
  double mid_x = (bbox.MinD(0) + bbox.MaxD(0))/2;
  double mid_y = (bbox.MinD(1) + bbox.MaxD(1))/2;


  double dist_delta1; 
  double dist_delta2; 
  if(GetRandom() % 2 ==0){
    dist_delta1 = 5.0;
    dist_delta2 = 3.0;
  }else{
    dist_delta1 = 3.0;
    dist_delta2 = 5.0;
  }

  double min[2], max[2];
  min[0] = mid_x - dist_delta1;
  max[0] = mid_x + dist_delta1;
  min[1] = mid_y - dist_delta2;
  max[1] = mid_y + dist_delta2;
  Rectangle<2> max_bbox(true, min, max);
  if(RegContainRect(reg, max_bbox)){
    bool valid = true;
    Rectangle<2> last_box = max_bbox;
    while(valid){

      dist_delta1 += dist_delta1;
      dist_delta2 += dist_delta2;

      min[0] = mid_x - dist_delta1;
      max[0] = mid_x + dist_delta1;
      min[1] = mid_y - dist_delta2;
      max[1] = mid_y + dist_delta2;
      Rectangle<2> temp_bbox(true, min, max);
      if(RegContainRect(reg, temp_bbox) && temp_bbox.Area() < maxi_rect_area){
        last_box = temp_bbox;
      }else{
        valid = false;
      }
    }
    return last_box;
  }else{
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0); 
  }

}

/////////////////////////////////////////////////////////////////////////
//////////// the  following implementation from /////////////////////////
////H. Alt, D. Hsu, and J. Snoeyink. ////////////////////////////////////
////////Computing the largest inscribed isothetic rectangle. ///////////
//////////In Proc. 7th Canadian Conf. Comput. Geom.,/////////////////// 
////////////Universit'e Laval, Qu'ebec, August 1995, pp. 67--72./////////
//////// the original code is by Java, I conver it to C, c++/////////////
/////////////////////////////////////////////////////////////////////////

void MaxRect::SetPoint(vector<GeomPoint>& list)
{
  for(unsigned int i = 0;i < list.size();i++){
//    geo_p_list.push_back(list[i]); 
      int x = list[i].x;
      int y = list[i].y;
      GeomPoint p(x,y);

      // this.polygon.add(p);
      if (geo_p_list.size() <2){
            geo_p_list.push_back(p);
            status = 1;
            changed = true;
      }else if (geo_p_list.size() == 2){
          GeomPoint ha = geo_p_list[0];
          GeomPoint hb = geo_p_list[1];
          if (onLeft(ha, hb, p)){
              geo_p_list.push_back(p);
              status = 2;
              changed = true;
          }else{
                GeomPoint q = geo_p_list[1];
                geo_p_list[1] = p;
                geo_p_list.push_back(q); 
                status = 2;
            }
        }else{
            geo_p_list.push_back(p); 
        }
  }


}


bool MaxRect::onLeft(GeomPoint a, GeomPoint b, GeomPoint c)
{
  int area = (b.x -a.x)*(c.y - a.y) - (c.x - a.x)*(b.y - a.y);
  return (area<0);
}

bool MaxRect::pointOutside(GeomPoint p)
{//, int start, int stop){

   bool ptIn = true, currIn, prevIn = true;

   GeomPoint a = geo_p_list[0];
   GeomPoint b;

   for(unsigned int i=0; i<geo_p_list.size(); i++){
      b = geo_p_list[(i+1)%geo_p_list.size()];
      currIn = onLeft(a, b, p);
      ptIn = ptIn && currIn;
      a = b;

     if(prevIn && !currIn){ start = i;} //next point outside, 1st tangent found
         if(!prevIn && currIn){ stop = i;}  // 2nd tangent 
         prevIn = currIn;

  }
        return !ptIn;
}


bool MaxRect::computeEdgeList()
{
        GeomPoint a,b;
        a = geo_p_list[geo_p_list.size()-1];
        for(unsigned int i = 0; i< geo_p_list.size(); i++){
            b = (GeomPoint)geo_p_list[i];
            //b = (GeomPoint)this.elementAt(i+1);

            if (i==0){
                g_xmin = a.x;
                g_xmax = a.x;
                g_ymin = a.y;
                g_ymax = a.y;
            }
            else{
                if (a.x < g_xmin){
                    g_xmin = a.x;
                }
                if (a.x > g_xmax){
                    g_xmax  = a.x;
                    yxmax = a.y;
                }
                if (a.y < g_ymin){
                    g_ymin = a.y;
                }
                if (a.y > g_ymax){
                    g_ymax  = a.y;
                }
            }
            if(a.x == b.x) return false; 
            
            GeomEdge* e = new GeomEdge(a,b);
            geo_e_list.push_back(*e);
            a = b;
            delete e; 
        } //for
  return true; 
}


/* 
compute y intersection with an edge
      first pixel completely inside
      ceil function if edge is on top, floor otherwise
      (+y is down)
      
*/
inline int MaxRect::yIntersect(int xi, GeomEdge e)
{

        int y;
        double yfirst = (e.m) * (xi-0.5) + e.b;
        double ylast = (e.m) * (xi+0.5) + e.b;

        if (!e.isTop){
            y = (int)floor(MIN(yfirst, ylast));
        }
        else {
            y = (int)ceil(MAX(yfirst, ylast));
        }
        return y;
    }

/* 
find largest pixel completely inside
look through all edges for intersection

*/
int MaxRect::xIntersect(int y){
        int x=0;
        double x0=0, x1=0;
        for(unsigned int i=0; i< geo_p_list.size(); i++){
            GeomEdge e = geo_e_list[i];
            if (e.isRight && e.ymin <= y && e.ymax >= y){
                x0 = (double)(y+0.5 - e.b)/e.m;
                x1 = (double)(y-0.5 - e.b)/e.m;
            }
        }
        x = (int)floor(MIN(x0,x1));
        //System.out.println("xIntersect, x is " + x);
        return x;
    }


GeomEdge MaxRect::findEdge(int x, bool isTop)
{
        GeomEdge e;
        GeomEdge emax= geo_e_list[0];
        //int count = 0;
        for (unsigned int i=0; i< geo_p_list.size(); i++){
            e = geo_e_list[i];
            if (e.xmin == x){
                if (e.xmax != e.xmin){
                    if ((e.isTop && isTop)||(!e.isTop && !isTop)){
                        emax = e;
                    }
                }
            }

        }
        return emax;
    }
/*
to get more precise data: 
for the coordinates of original point, we can multiply the value by 1000. 
because the original code only uses the type int. 
in the end, we have to div the value by 1000. 
This depends on the application requirment 

*/
void MaxRect::computeLargestRectangle(){

        changed = false;

        GeomEdge top, bottom;
        int ymax, ymin, xright, xlo, xhi;
        int area, maxArea = 0;
        int width, height, maxh=0, maxw=0;

        /* all 2-corner and 3-corner largest rectangles */
        int aAC=0,aBD=0,aABC=0,aABD=0,aACD=0,aBCD=0;
        GeomPoint pAC, pBD, pABC, pABD, pACD, pBCD;
        int hAC=0,wAC=0,hBD=0,wBD=0,hABC=0,wABC=0,hABD=0,wABD=0;
        int hACD=0,wACD=0,hBCD=0,wBCD=0;
        bool onA, onB, onC, onD;

        GeomPoint maxp(0, 0);
        pAC = maxp; 
        pBD = maxp; 
        pABC = maxp; 
        pABD = maxp; 
        pACD = maxp; 
        pBCD = maxp;


        vector<GeomPoint> xint;

        for(int i=0;i< g_ymax;i++){
            int x = xIntersect(i);
            GeomPoint px(x, i);
            xint.push_back(px);
//            cout<<px.x<<" "<<px.y<<endl; 
        }

//        cout<<"g_xmin "<<g_xmin<<" g_xmax "<<g_xmax
//            <<" g_ymin "<<g_ymin<<" g_ymax "<<g_ymax<<endl; 

        //find first top and bottom edges
        top = findEdge(g_xmin, true);
        bottom = findEdge(g_xmin, false);

        //scan for rectangle left position
        for(int xi= g_xmin; xi< g_xmax;xi++){

            ymin = yIntersect(xi, top);
            ymax = yIntersect(xi, bottom);
//            cout<<"xi "<<xi<<" ymin "<<ymin<<" ymax "<<ymax<<endl;

            for(int ylo = ymax;ylo>= ymin;ylo--){//ylo from to to bottom
//                cout<<"ylo "<<ylo<<"ymin "<<ymin<<"ymax "<<ymax<<endl;
                for(int yhi = ymin; yhi<= ymax; yhi++){

//                    cout<<"yhi "<<yhi<<endl; 

                    if (yhi>ylo){

                        onA = (yhi == ymax && !bottom.isRight);
                        onD = (ylo == ymin && !top.isRight);

                        //xIntersect(ylo,edgeList);
                        xlo = (int)((GeomPoint)xint[ylo]).x;
                        //xIntersect(yhi,edgeList);
                        xhi = (int)((GeomPoint)xint[yhi]).x;

                        xright = maxp.min(xlo,xhi);
                        onC = (xright == xlo && yxmax >= ylo);
                        onB = (xright == xhi && yxmax <= yhi);

                        height = yhi-ylo;
                        width = xright - xi;

                        if (!fixed){
                        }//!fixed
                        else{
                          int fixedWidth = 
                          (int)ceil( ((double)height*fixedX)/((double)fixedY));
                          if (fixedWidth <= width){
                              width = fixedWidth;
                          }
                          else{
                              width = 0;
                          }
                       }
                        area = width * height;
                        //AC 
                        if (onA && onC && !onB && !onD){
                            if (area > aAC){
                                aAC = area;
//                                pAC = new GeomPoint(xi, ylo);
                                pAC.x = xi;
                                pAC.y = ylo; 
//                                cout<<" xi "<<xi<<" ylo "<<ylo<<endl; 
                                hAC = height;
                                wAC = width;
                            }
                        }
                        //BD
                        if (onB && onD && !onA && !onC){
                            if (area > aBD){
                                aBD = area;
//                                pBD = new GeomPoint(xi, ylo);
                                pBD.x = xi;
                                pBD.y = ylo; 

                                hBD = height;
                                wBD = width;
                            }
                        }
                        //ABC
                        if (onA && onB && onC){
                            if (area > aABC){
                                aABC = area;
//                                pABC = new GeomPoint(xi, ylo);
                                pABC.x = xi;
                                pABC.y = ylo; 

                                hABC = height;
                                wABC = width;
                            }
                        }
                        //ABD
                        if (onA && onB && onD){
                            if (area > aABD){
                                aABD = area;
//                                pABD = new GeomPoint(xi, ylo);
                                pABD.x = xi;
                                pABD.y = ylo; 

                                hABD = height;
                                wABD = width;
                            }
                        }
                        //ACD
                        if (onA && onC && onD){
                            if (area > aACD){
                                aACD = area;
//                                pACD = new GeomPoint(xi, ylo);
                                pACD.x = xi;
                                pACD.y = ylo; 

                                hACD = height;
                                wACD = width;
                            }
                        }
                        //BCD
                        if (onB && onC && onD){
                            if (area > aBCD){
                                aBCD = area;
//                                pBCD = new GeomPoint(xi, ylo);
                                pBCD.x = xi;
                                pBCD.y = ylo; 

                                hBCD = height;
                                wBCD = width;
                            }
                        }

                        if(area>maxArea){
                            maxArea = area;
//                            maxp = new GeomPoint(xi, ylo);
                            maxp.x = xi;
                            maxp.y = ylo; 

                            maxw = width;
                            maxh = height;

                        }
                    }//yhi > ylo
                }//for yhi
            }//for ylo
            if (xi == top.xmax){
                top = findEdge(xi,  true);
            }
            if(xi == bottom.xmax){
                bottom = findEdge(xi, false);
            }
        }//xi

        rectp = maxp;
        recth = maxh;
        rectw = maxw;


/*        RectList.push_back(Rectangle<2>(true, pAC.x, pAC.y, wAC, hAC));
        RectList.push_back(Rectangle<2>(true, pBD.x, pBD.y, wBD, hBD));
        RectList.push_back(Rectangle<2>(true, pABC.x, pABC.y, wABC, hABC));
        RectList.push_back(Rectangle<2>(true, pABD.x, pABD.y, wABD, hABD));
        RectList.push_back(Rectangle<2>(true, pACD.x, pACD.y, wACD, hACD));
        RectList.push_back(Rectangle<2>(true, pBCD.x, pBCD.y, wBCD, hBCD));
        RectList.push_back(Rectangle<2>(true, maxp.x, maxp.y, maxw, maxh));*/

        double x_1 = pAC.x;
        double x_2 = pAC.x + wAC;
        double y_1 = pAC.y;
        double y_2 = pAC.y + hAC; 
//        Rectangle<2> rect(true, x_1, x_2, y_1, y_2); 

//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2)); 

        x_1 = pBD.x; x_2 = x_1 + wBD; y_1 = pBD.y; y_2 = pBD.y + hBD; 
//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2));  

        x_1 = pABC.x; x_2 = x_1 + wABC; y_1 = pABC.y; y_2 = y_1 + hABC;
//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2)); 

        x_1 = pABD.x; x_2 = x_1 + wABD; y_1 = pABD.y; y_2 = y_1 + hABD;
//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2)); 

        x_1 = pACD.x; x_2 = x_1 + wACD; y_1 = pACD.y; y_2 = y_1 + hACD;
//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2));

        x_1 = pBCD.x; x_2 = x_1 + wBCD; y_1 = pBCD.y; y_2 = y_1 + hBCD;
//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2));

        x_1 = maxp.x; x_2 = x_1 + maxw; y_1 = maxp.y; y_2 = y_1 + maxh;
//        cout<<"x1 "<<x_1<<" x2 "<<x_2<<" y1 "<<y_1<<" y2 "<<y_2<<endl; 
        RectList.push_back(Rectangle<2>(true, x_1, x_2, y_1, y_2));


    float rect_area = 0.0;
    result = -1; 
    for(unsigned int i = 0;i < RectList.size();i++){
//      cout<<RectList[i]<<" area "<<RectList[i].Area()<<endl; 
      if(RectList[i].IsDefined() == false){
        result = -1;
        return; 
      }
      if(RectList[i].Area() > rect_area){
        rect_area = RectList[i].Area();
        result = i;
      }
    }
//    cout<<RectList[index]<<endl; 
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
string MaxRect::BuildingRectTypeInfo = 
"(rel (tuple ((Reg_id int) (GeoData rect) (Poly_id int) (Reg_type int))))"; 

string MaxRect::RegionElemTypeInfo = 
"(rel (tuple ((Id int) (Covarea region))))";

string MaxRect::BuildingRectExtTypeInfo =
"(rel (tuple ((Reg_id int) (GeoData rect) (Poly_id int) (Reg_type int) \
(Building_type int) (Building_type2 string) (Building_id int))))";

string MaxRect::BuildingParaInfo = "(rel (tuple ((Para real))))";

/*
remove some dirty data for regions. two subregions intersect at a point 

*/
void MaxRect::RemoveDirty(int attr1, int attr2)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<endl; 
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* region_tuple = rel1->GetTuple(i, false); 
    int reg_id = ((CcInt*)region_tuple->GetAttribute(attr1))->GetIntval();
    Region* r = (Region*)region_tuple->GetAttribute(attr2); 
    if(reg_id == 1){ //the outer cycle 
        reg_id_list.push_back(reg_id);
        reg_list.push_back(*r);
        region_tuple->DeleteIfAllowed(); 
        continue; 
    }

    CompTriangle* ct = new CompTriangle(r);
    int error; //0-concave 1 non-region 2 hole or faces 3 dirty region 
    bool convex = ct->PolygonConvex2(error);
    if(convex){ //convex polygon 
        reg_id_list.push_back(reg_id);
        reg_list.push_back(*r);
    }else if(convex == false && error == 0){  //concave polygon
        reg_id_list.push_back(reg_id);
        reg_list.push_back(*r);
    }else{
//      cout<<"dirty data"<<endl; 
        if(error == 3){ //ignore the first outer cycle, non-region dirty
          RemoveDirtyRegion(reg_id, r);

//          break; 
        }
    }

    delete ct; 
    region_tuple->DeleteIfAllowed(); 
  }

}

/*
a point that appears four times. it is a vertex that two regions intersect 

*/
void MaxRect::RemoveDirtyRegion(int regid, Region* reg)
{
  vector<MyPoint> ps_list;
  vector<HalfSegment> hs_list; 
  for(int i = 0;i < reg->Size();i++){
    HalfSegment hs;
    reg->Get(i, hs);
    if(!hs.IsLeftDomPoint())continue; 
    Point lp = hs.GetLeftPoint();
    Point rp = hs.GetRightPoint();
    if(ps_list.size() == 0){
      ps_list.push_back(MyPoint(lp, 1));
      ps_list.push_back(MyPoint(rp, 1)); 
    }else{
      unsigned int j; 
      for(j = 0;j < ps_list.size();j++){
        if(AlmostEqual(ps_list[j].loc, lp)){
          ps_list[j].dist++; 
          break;
        }
      }
      if(j == ps_list.size())ps_list.push_back(MyPoint(lp, 1));

      for(j = 0;j < ps_list.size();j++){
        if(AlmostEqual(ps_list[j].loc, rp)){
          ps_list[j].dist++; 
          break;
        }
      }
      if(j == ps_list.size())ps_list.push_back(MyPoint(rp, 1)); 
    }
    HalfSegment temp_hs(true, hs.GetLeftPoint(), hs.GetRightPoint()); 
    hs_list.push_back(temp_hs); 
  }

  Point dirty_p; 
  for(unsigned int i = 0;i <ps_list.size();i++){
//    ps_list[i].Print(); 
    if((int)(ps_list[i].dist) == 4)dirty_p = ps_list[i].loc; 
  }

//  cout<<"dirty point "<<dirty_p<<endl; 
  
  vector<unsigned int> index_list; 
  for(unsigned int i = 0;i < hs_list.size();i++){
    HalfSegment hs = hs_list[i]; 
    if(AlmostEqual(hs.GetLeftPoint(), dirty_p))index_list.push_back(i);
    if(AlmostEqual(hs.GetRightPoint(), dirty_p))index_list.push_back(i);
  }
  
  assert(index_list.size() == 4); //four halfsegments 
//  cout<<hs_list.size()<<endl;
  vector<HalfSegment> new_hs_list;
  for(unsigned int i = 0;i < hs_list.size();i++){
    unsigned int j = 0;
    for(;j < index_list.size();j++)
      if(i == index_list[j])break; 

    if(j == index_list.size())new_hs_list.push_back(hs_list[i]); 
  }
//  cout<<new_hs_list.size()<<endl; 


  vector<SimpleLine> sl_contour;

  for(unsigned int i = 0;i < index_list.size();i++){

      for(unsigned int j = i + 1;j < index_list.size();j++){
        SimpleLine* sl = new SimpleLine(0);
        sl->StartBulkLoad();

        vector<HalfSegment> sl_hs_list; 

        HalfSegment hs1 = hs_list[index_list[i]]; 
        HalfSegment hs2 = hs_list[index_list[j]]; 

        vector<HalfSegment> cut_hs_list;
        for(unsigned int k = 0;k < index_list.size();k++){
          if(index_list[k] != index_list[i] && 
             index_list[k] != index_list[j])
            cut_hs_list.push_back(hs_list[index_list[k]]); 
        }


        int edgeno = 0; 
        for(unsigned int k1 = 0;k1 < new_hs_list.size();k1++){
          HalfSegment hs = new_hs_list[k1];

          unsigned int k2 = 0;
          for(;k2 < cut_hs_list.size();k2++){
            if(hs.Intersects(cut_hs_list[k2]))break;
          }
          if(k2 == cut_hs_list.size()){
            hs.attr.edgeno = edgeno++;
            *sl += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *sl += hs; 
          }
        }
        *sl += hs1;
        hs1.SetLeftDomPoint(!hs1.IsLeftDomPoint());
        *sl += hs1;

        *sl += hs2;
        hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
        *sl += hs2; 


        sl->EndBulkLoad(); 
        if(sl->Length() > 0)
          sl_contour.push_back(*sl);
//        cout<<sl->Length()<<endl; 
        delete sl; 
      }
  }

  //////////// get the refinement region ///////////////////
  for(unsigned int i = 0;i < sl_contour.size();i++){
    if(IsCycle(&sl_contour[i])){
//        cout<<"a cycle"<<endl; 
//        reg_id_list.push_back(regid);
//        sl_list.push_back(sl_contour[i]);
        SpacePartition* sp = new SpacePartition();
        vector<MyHalfSegment> mhs;
        sp->ReorderLine(&sl_contour[i], mhs);

        vector<Point> ps;
        for(unsigned int j = 0;j < mhs.size();j++){
          ps.push_back(mhs[j].from);
        }
        vector<Region> regs;
        sp->ComputeRegion(ps, regs);
        if(regs.size() > 0 ){
          reg_id_list.push_back(regid); 
          reg_list.push_back(regs[0]);
        }
        delete sp; 
    }
  }

}

/*
check whether the simpleline is a cycle 

*/
bool MaxRect::IsCycle(SimpleLine* sl)
{

  vector<MyPoint> ps_list;
 
  for(int i = 0;i < sl->Size();i++){
    HalfSegment hs;
    sl->Get(i, hs);
    if(!hs.IsLeftDomPoint())continue; 
//    cout<<hs<<endl; 
    Point lp = hs.GetLeftPoint();
    Point rp = hs.GetRightPoint();
    if(ps_list.size() == 0){
      ps_list.push_back(MyPoint(lp, 1));
      ps_list.push_back(MyPoint(rp, 1)); 
    }else{
      unsigned int j; 
      for(j = 0;j < ps_list.size();j++){
        if(AlmostEqual(ps_list[j].loc, lp)){
          ps_list[j].dist++; 
          break;
        }
      }
      if(j == ps_list.size())ps_list.push_back(MyPoint(lp, 1));

      for(j = 0;j < ps_list.size();j++){
        if(AlmostEqual(ps_list[j].loc, rp)){
          ps_list[j].dist++; 
          break;
        }
      }
      if(j == ps_list.size())ps_list.push_back(MyPoint(rp, 1)); 
    }
  }
  for(unsigned int i = 0;i < ps_list.size();i++){
      if((int)ps_list[i].dist != 2) return false; 
  }
  
  return true; 
}


/*
get neighbor triangles to get convex polygons 

*/
void MaxRect::ConvexReg(int attr1, int attr2)
{
//  cout<<"attr1 "<<attr1<<" attr2 "<<attr2<<endl; 
  
  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* region_tuple = rel1->GetTuple(i, false); 
    int reg_id = ((CcInt*)region_tuple->GetAttribute(attr1))->GetIntval();
//    cout<<"reg_id "<<reg_id<<endl; 

/*    if(reg_id != 2572){
      region_tuple->DeleteIfAllowed();
      continue; 
    }*/

    Region* r = (Region*)region_tuple->GetAttribute(attr2); 
    CompTriangle* ct = new CompTriangle(r);
    int error; 
    bool convex = ct->PolygonConvex2(error);
    if(convex){ //convex polygon 
        reg_id_list.push_back(reg_id); 
        reg_list.push_back(*r);

    }else if(convex == false && error == 0){  //concave polygon
        ///decompose the convex polygon into several convex polygons ///
//        cout<<"reg_id "<<reg_id<<endl; 
        CompTriangle* ct1 = new CompTriangle(r);
        ct1->NewTriangulation();
//        cout<<ct1->triangles.size()<<endl; 
        MergeTriangle(ct1, reg_id); 
        delete ct1; 

    }
    delete ct; 
    region_tuple->DeleteIfAllowed(); 
  }

}

/*
merge triangles to get convex polygons 

*/

void MaxRect::MergeTriangle(CompTriangle* ct, int reg_id)
{
  vector<Region> convex_list; 
  vector<bool> mark_list; 
  for(unsigned int i = 0;i < ct->triangles.size();i++)
    mark_list.push_back(false); 
  
  for(unsigned int i = 0;i < ct->triangles.size();i++){
    if(mark_list[i]) continue; 
    Region* r1 = &(ct->triangles[i]);

    if(r1->Area() > mini_reg_area){
        convex_list.push_back(*r1);
        mark_list[i] = true; 
        continue; 
    }

    unsigned int j = i + 1;
    for(;j < ct->triangles.size();j++){
      Region* r2 = &(ct->triangles[j]); 
      if(mark_list[j] == false && NeighborTriangle(r1,r2)){
        Region* res = new Region(0);
        r1->Union(*r2, *res);
        convex_list.push_back(*res);
        delete res; 

        mark_list[j] = true;
        break; 
      }
    }
    if(j != ct->triangles.size())
      mark_list[i] = true; 
  }

  for(unsigned int i = 0;i < mark_list.size();i++)
    if(mark_list[i] == false)convex_list.push_back(ct->triangles[i]); 
  
  
  vector<Region> res_list; 
  for(unsigned int i = 0;i < convex_list.size();i++){
    CompTriangle* ct1 = new CompTriangle(&convex_list[i]);
    if(ct1->PolygonConvex()){
        res_list.push_back(convex_list[i]);
    }
    delete ct1; 
  }

  vector<bool> flag_list;
  for(unsigned int i = 0;i < res_list.size();i++){
    flag_list.push_back(true); 
  }
  
  for(unsigned int i = 0;i < res_list.size();i++){
    if(flag_list[i] == false)continue; 
    Region* r1 = &(res_list[i]);
    if(r1->Area() > mini_reg_area){
//      cout<<"mini area "<<mini_reg_area<<" r1 "<<r1->Area()<<endl; 
      reg_id_list.push_back(reg_id); 
      reg_list.push_back(*r1); 
      flag_list[i] = false; 

      for(unsigned int j = i + 1;j < res_list.size();j++){
        Region* r2 = &(res_list[j]);
        if(r1->BoundingBox().Distance(r2->BoundingBox()) < 50.0){
            flag_list[j] = false; 
        }
      }
    }
  }
}


/*
two triangles share one edge 

*/
bool MaxRect::NeighborTriangle(Region* reg1, Region* reg2)
{
  for(int i = 0;i < reg1->Size();i++){
      HalfSegment hs1;
      reg1->Get(i, hs1);
     for(int j = 0;j < reg2->Size();j++){
        HalfSegment hs2;
        reg2->Get(j, hs2);
        if((AlmostEqual(hs1.GetLeftPoint(),  hs2.GetLeftPoint())&&
            AlmostEqual(hs1.GetRightPoint(), hs2.GetRightPoint())) ||
           (AlmostEqual(hs1.GetRightPoint(), hs2.GetLeftPoint()) &&
            AlmostEqual(hs1.GetLeftPoint(),  hs2.GetRightPoint()))){
          return true; 
        }
     }
  }   
  return false; 
}

/*
check whether a region contains a rectangle 

*/
bool RegContainRect(Region* reg, Rectangle<2>& rect)
{
    Point p1(true, rect.MinD(0), rect.MinD(1));
    Point p2(true, rect.MaxD(0), rect.MinD(1));
    Point p3(true, rect.MaxD(0), rect.MaxD(1));
    Point p4(true, rect.MinD(0), rect.MaxD(1));

    HalfSegment hs1(true, p1, p2);
    HalfSegment hs2(true, p2, p3);
    HalfSegment hs3(true, p3, p4);
    HalfSegment hs4(true, p1, p4); 

    if(RegContainHS(reg, hs1) && RegContainHS(reg, hs2) &&
        RegContainHS(reg, hs3) && RegContainHS(reg, hs4))return true;
    return false; 
}

/*
for the input relation, it gets a maximum rectangle for each region 

*/
void MaxRect::GetRectangle1(int attr1, int attr2, Relation* building_para)
{
   if(building_para->GetNoTuples() == 0){
      cout<<"para relation empty"<<endl;
      return;
   }
   Tuple* para_tuple = building_para->GetTuple(1, false);
   float area_para = ((CcReal*)para_tuple->GetAttribute(0))->GetRealval();
   para_tuple->DeleteIfAllowed();
  
  
  int reg_id = 1; 
  for(int i = 1;i <= rel1->GetNoTuples();i++){
//  for(int i = 1;i <= 50;i++){

    Tuple* poly_tuple = rel1->GetTuple(i, false); 
    int poly_id = ((CcInt*)poly_tuple->GetAttribute(attr1))->GetIntval();
    Region* poly = (Region*)poly_tuple->GetAttribute(attr2); 

    if(poly_id == 1){
       poly_tuple->DeleteIfAllowed();
       continue; 
     }

//    cout<<"poly_id "<<poly_id<<endl;

    CompTriangle* ct = new CompTriangle(poly);
    ct->NewTriangulation();
//    cout<<ct->triangles.size()<<endl; 

    vector<Rectangle<2> > rect_box_list; 

    for(unsigned int j = 0;j < ct->triangles.size();j++){
      if(ct->triangles[j].Area() > mini_tri_area && 
         ct->triangles[j].Area() < maxi_tri_area){
//          printf("%f\n",ct->triangles[j].Area());
//          if(ct->triangles[j].Area() > 28000.0) poly_count++;

          if(ValidRegion(&ct->triangles[j])){//coordinates should be positive 

            Rectangle<2> rect_box(false);
            
//             if(type == "Berlin"){
//               rect_box = GetMaxRect(&ct->triangles[j]); 
//             }else if(type == "Houston"){
//               if(ct->triangles[j].Area() > 0.9 * maxi_tri_area){
//                 rect_box = GetMaxRect(&ct->triangles[j]);
//               }else{
//                 rect_box = GetMaxRect2(&ct->triangles[j]);
//               }
//             }else {
//               assert(false);
//             }

              if(ct->triangles[j].Area() > area_para * maxi_tri_area){
                 rect_box = GetMaxRect(&ct->triangles[j]);
              }else{
                 rect_box = GetMaxRect2(&ct->triangles[j]);
              }


//            cout<<rect_box<<endl;
            if(rect_box.IsDefined()){
                double x = fabs(rect_box.MaxD(0) - rect_box.MinD(0));
                double y = fabs(rect_box.MaxD(1) - rect_box.MinD(1));
                double min = MIN(x,y);
                double max = MAX(x,y); 
              if(rect_box.Area() > mini_rect_area && 
                 rect_box.Area() < maxi_rect_area && (max / min < 5.0)){

                    if(RegContainRect(poly, rect_box))
                        rect_box_list.push_back(rect_box); 
               }
            }
          }else{
            Rectangle<2> bbox = ct->triangles[j].BoundingBox();
            double minx = bbox.MinD(0);
            double miny = bbox.MinD(1);
            double tran_x = fabs(minx) + 1.0;
            double tran_y = fabs(miny) + 1.0; 
            Region* r = new Region(0);
            ct->triangles[j].Translate(tran_x, tran_y, *r); 
            assert(ValidRegion(r));

            Rectangle<2> rect_box(false);

//             if(type == "Berlin"){
//                 rect_box = GetMaxRect(r);
//             }else if(type == "Houston"){
//                 if(r->Area() > 0.9 * maxi_tri_area){
//                   rect_box = GetMaxRect(r);
//                 }else{
//                   rect_box = GetMaxRect2(r);
//                 }
//             }else{
//               assert(false);
// 
//             }

                if(r->Area() > area_para * maxi_tri_area){
                  rect_box = GetMaxRect(r);
                }else{
                  rect_box = GetMaxRect2(r);
                }

            if(rect_box.IsDefined()){
                double x = fabs(rect_box.MaxD(0) - rect_box.MinD(0));
                double y = fabs(rect_box.MaxD(1) - rect_box.MinD(1));
                double min = MIN(x,y);
                double max = MAX(x,y); 
              if(rect_box.Area() > mini_rect_area && 
                 rect_box.Area() < maxi_rect_area && (max / min < 5.0)){
                    double mini[2], maxi[2];
                    mini[0] = rect_box.MinD(0) - tran_x;
                    mini[1] = rect_box.MinD(1) - tran_y;
                    maxi[0] = rect_box.MaxD(0) - tran_x;
                    maxi[1] = rect_box.MaxD(1) - tran_y; 
                    Rectangle<2>* res_box = new Rectangle<2>(true, mini, maxi);

                    if(RegContainRect(poly, rect_box))
                        rect_box_list.push_back(res_box); 

                    delete res_box; 
               }
            }
            delete r;
          }
      }

    }
    delete ct; 
    poly_tuple->DeleteIfAllowed(); 

      //////////the distance between two buildings should be large////////
      for(unsigned int k1 = 0;k1 < rect_box_list.size();k1++){
        Rectangle<2> bbox1 = rect_box_list[k1];
        unsigned int k2 = k1;
        for(k2 += 1;k2 < rect_box_list.size();k2++){
          Rectangle<2> bbox2 = rect_box_list[k2];
          if(bbox1.Distance(bbox2) < mini_dist_build) break; 
        }
        if(k2 == rect_box_list.size()){

/*            reg_id_list.push_back(reg_id);
            rect_list.push_back(bbox1);
            poly_id_list.push_back(poly_id); 
            reg_id++;*/

//          if(bbox1.Area() < 4000.0 || bbox1.Area() > 8500.0){
          if(bbox1.Area() < 4000.0){/////we do not have the airport now. 2011.8
            reg_id_list.push_back(reg_id);
            rect_list.push_back(bbox1);
            poly_id_list.push_back(poly_id); 
            reg_id++;
          }else{//set this value considering airport
            /////////decompose the large rectangle to have more ////////////
            /////////keep large region for airport ///////////////////
            float w = bbox1.MaxD(0) - bbox1.MinD(0);
            float h = bbox1.MaxD(1) - bbox1.MinD(1);
            int row = w/length_limit;
            int col = h/length_limit;

            double x_min = bbox1.MinD(0);
//            double x_max = bbox1.MaxD(0);
            double y_min = bbox1.MinD(1);
//            double y_max = bbox1.MaxD(1);
            ///////35 x 30 ///////////////////
            double x = 35.0;
            double y = 30.0;
//            cout<<"row "<<row<<" col "<<col<<endl;
            if(row == 0 || col == 0){
                reg_id_list.push_back(reg_id);
                rect_list.push_back(bbox1);
                poly_id_list.push_back(poly_id); 
                reg_id++;
            }else{

              for(int index1 = 0;index1 < row;index1++){
                double x1 = x_min + index1*length_limit;
                double x2 = x1 + x;
                for(int index2 = 0;index2 < col;index2++){
                  double y1 = y_min + index2*length_limit;
                  double y2 = y1 + y;

                  double min[2], max[2];
                  min[0] = x1; max[0] = x2;
                  min[1] = y1; max[1] = y2;
                  Rectangle<2> temp_box(true, min, max);

                  if(bbox1.Contains(temp_box)){
                    reg_id_list.push_back(reg_id);
                    rect_list.push_back(temp_box);
                    poly_id_list.push_back(poly_id); 
                    reg_id++;
                  }
                }
              }/////end for 
            }///end else

          }
        }
      }
  }

}

/*
check whether all coordinates are positive. because for the function 
GetMaxRect(), all coordinates should be positive 

*/
bool MaxRect::ValidRegion(Region* r)
{
  for(int i = 0;i < r->Size();i++){
    HalfSegment hs;
    r->Get(i, hs);
    if(!hs.IsLeftDomPoint())continue; 
    Point lp = hs.GetLeftPoint();
    Point rp = hs.GetRightPoint(); 
    if(lp.GetX() < 0.0)return false;
    if(lp.GetY() < 0.0)return false;
    if(rp.GetX() < 0.0)return false;
    if(rp.GetY() < 0.0)return false;
  }
  return true; 
}

/*
for each start point, 
choose the one that is the closest to the polygon boundary
record which side it belongs to

*/
void MaxRect::SetStartAndEndPoint(Region* r,  
                             vector<Point>& build_sp_list, 
                             vector<Point>& build_ep_list)
{
  SpacePartition* s_p = new SpacePartition();

  for(unsigned int i = 0;i < build_sp_list.size();i++){
      vector<MyPoint> point_dist_list;
      for(int j = 0;j < r->Size();j++){
        HalfSegment hs;
        r->Get(j, hs);
        if(!hs.IsLeftDomPoint())continue;

        double d;
        Point cp(true, 0, 0);

        d = s_p->GetClosestPoint(hs, build_sp_list[i], cp);
        MyPoint mpe(cp, d);
        point_dist_list.push_back(mpe);
      }
    sort(point_dist_list.begin(), point_dist_list.end());

//     for(unsigned int k = 0;k < point_dist_list.size();k++)
//        point_dist_list[k].Print();

    build_ep_list.push_back(point_dist_list[0].loc);
  }
  delete s_p;
 
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*
load the pointers to all types of buildings 

*/
void MaxRect::OpenBuilding()
{

  for(unsigned int i = 0;i < ARR_SIZE(str_build_type);i++){
    if(i == 0){
      build_pointer.push_back(NULL);
      continue;
    }
    bool found = false;
    ListExpr xObjectList = SecondoSystem::GetCatalog()->ListObjects();
      xObjectList = nl->Rest(xObjectList);
      while(!nl->IsEmpty(xObjectList)){
          // Next element in list
          ListExpr xCurrent = nl->First(xObjectList);
          xObjectList = nl->Rest(xObjectList);
          // Type of object is at fourth position in list
          ListExpr xObjectType = nl->First(nl->Fourth(xCurrent));
          if(nl->IsAtom(xObjectType) &&
             nl->SymbolValue(xObjectType) == "building"){

            // Get name of the graph 
            ListExpr xObjectName = nl->Second(xCurrent);
            string strObjectName = nl->SymbolValue(xObjectName);

            // Load object to find out the id 
            Word xValue;
            bool bDefined;
            bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
            if(!bDefined || !bOk){
              // Undefined
              continue;
            }
            Building* building = (Building*)xValue.addr;

            if(building->GetType() == i){
              build_pointer.push_back(building);
              found = true;
              break;
            }else{
          SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "building" ),
                                           xValue); 
            }
          }
      }
    if(found == false)
      build_pointer.push_back(NULL);

  }
}

void MaxRect::CloseBuilding()
{
  for(unsigned int i = 0;i < ARR_SIZE(str_build_type);i++){
    if(build_pointer[i] != NULL){
      Word xValue;
      xValue.addr = build_pointer[i];
      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "building" ),
                                           xValue);
    }
  }
}

/*
for each building, it opens and closes the indoor graph 

*/
void MaxRect::OpenIndoorGraph()
{
  for(unsigned int i = 0;i < build_pointer.size();i++){
    if(build_pointer[i] == NULL){
      igraph_pointer.push_back(NULL);
      continue;
    }
    igraph_pointer.push_back(build_pointer[i]->OpenIndoorGraph());
  }
}

/*
read indoor paths of each building from disk files 

*/
void MaxRect::LoadIndoorPaths(vector< map<int, Line3D> >& paths,
                              vector< map<int, Line3D> >& rooms)
{
  for(unsigned int i = 0;i < build_pointer.size();i++){
      if(build_pointer[i] != NULL){
//      cout<<i<<" "<<str_build_type[build_pointer[i]->GetType()]<<endl;

        map<int, Line3D> path_list;
        map<int, Line3D> room_id_list;

        build_pointer[i]->LoadPaths(path_list, room_id_list);

        paths.push_back(path_list);
        rooms.push_back(room_id_list);//store groom id for each point3d 

      }else{
          map<int, Line3D> path_list;
          map<int, Line3D> room_id_list;

          paths.push_back(path_list);
          rooms.push_back(room_id_list);//store groom id for each point3d 
      }
  }

}

void MaxRect::CloseIndoorGraph()
{
  for(unsigned int i = 0;i < igraph_pointer.size();i++){
    if(igraph_pointer[i] == NULL){
      continue;
    }
    build_pointer[i]->CloseIndoorGraph(igraph_pointer[i]);
  }
}

/*

build the path between the entrance of the building and pavement area
which point to select on the polygon boundary:
which point to set as the entrance of the building on the rectangle 
create the path from building entrance to pavement area 

*/
void MaxRect::PathToBuilding(Space* gl_sp)
{
//  cout<<"build the connection between the entrance of building and pavement"
//      <<endl;

  Pavement* pave = gl_sp->LoadPavement(IF_REGION);
  DualGraph* dg = pave->GetDualGraph();

  ////////////////load all buildings and indoor graphs////////////////////////
  OpenBuilding();
  OpenIndoorGraph();

//   for(unsigned int i = 0;i < build_pointer.size();i++){
//     if(build_pointer[i] != NULL)cout<<GetBuildingStr(i)<<endl;
//     if(igraph_pointer[i] != NULL)cout<<GetBuildingStr(i)<<endl;
//   }

 ////////////////////////////////////////////////////////////////////

  /////for each original polygon, it collects all rectangles inside /////
  //////select point from the polygon boundary //////
  /////for each rectangle, it also selects a point on its boundary///////
  for(int i = 1;i <= rel2->GetNoTuples();i++){
    Tuple* poly_tuple = rel2->GetTuple(i, false);
    int poly_id = ((CcInt*)poly_tuple->GetAttribute(REGID))->GetIntval(); 

    CcInt* search_id = new CcInt(true, poly_id);
    BTreeIterator* btree_iter = btree->ExactMatch(search_id);
    vector<int> rect_tid_list;
    while(btree_iter->Next()){
        Tuple* tuple = rel1->GetTuple(btree_iter->GetId(), false);
        int poly_id2 = ((CcInt*)tuple->GetAttribute(POLY_ID_EXT))->GetIntval();
        assert(poly_id == poly_id2);
        rect_tid_list.push_back(btree_iter->GetId());
        tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;

    if(rect_tid_list.size() > 0){
      Region* poly = (Region*)poly_tuple->GetAttribute(COVAREA); 

      CreateEntranceforBuilding(poly, rect_tid_list, dg);
    }

    poly_tuple->DeleteIfAllowed();

  }
  

  pave->CloseDualGraph(dg);
  gl_sp->ClosePavement(pave);

  /////////////close all buildings and indoor graphs//////////////////////
  CloseIndoorGraph();
  CloseBuilding();

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
create the path in the polygon for the building entrance where the entrances of
a building is selected from the indoor floor plan

*/
void MaxRect::CreateEntranceforBuilding(Region* r, 
                                        vector<int>& tid_list, DualGraph* dg)
{
  HalfSegment hs;
  r->Get(0, hs);
  Point boundary_p = hs.GetLeftPoint(); 
  vector<Rectangle<2> > hole_list; 

  for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* rect_tuple = rel1->GetTuple(tid_list[i], false);
    Rectangle<2>* rect = (Rectangle<2>*)rect_tuple->GetAttribute(GEODATA_EXT);
    hole_list.push_back(*rect);
    rect_tuple->DeleteIfAllowed();
  }

  ////////////////create a region using these rectangels as holes/////////////
 for(unsigned int i = 0;i < tid_list.size();i++){
    Tuple* rect_tuple = rel1->GetTuple(tid_list[i], false);

    int reg_id = ((CcInt*)rect_tuple->GetAttribute(REG_ID_EXT))->GetIntval();
    Rectangle<2>* rect = (Rectangle<2>*)rect_tuple->GetAttribute(GEODATA_EXT);
    int build_type = 
        ((CcInt*)rect_tuple->GetAttribute(BUILDING_TYPE))->GetIntval();
    if(build_type == 0 || 
      (build_type > 1 && build_pointer[build_type] == NULL)){
      rect_tuple->DeleteIfAllowed();
      continue;
    }

    vector<Point> build_sp_list;
     vector<Point> build_ep_list;
    if(build_type > 1){
      BuildingEntrance(build_type, rect, build_sp_list);
      SetStartAndEndPoint(r, build_sp_list, build_ep_list);
    }else{/////////////entrance for personal apartment
      BuildingEntranceHouse(rect, build_sp_list, build_ep_list, r);
    }

    for(unsigned int j = 0;j < build_sp_list.size();j++){
        Point sp = build_sp_list[j];
        Point ep = build_ep_list[j];

        reg_id_list.push_back(reg_id);
        sp_list.push_back(sp);
//        ep_list.push_back(ep);
//        MapToPavement(dg, ep);
        /////build the path from building entrance to pavement area//////
        Path_BuildingPave(sp, ep, rect, r, dg);
        /////////////which entrance it is//////////////////////////
        sp_index_list.push_back(j + 1);
    }

    rect_tuple->DeleteIfAllowed();
  }

}


/*
set the building entrance point according to the floor plan

*/
void MaxRect::BuildingEntrance(int build_type, Rectangle<2>* rect, 
                          vector<Point>& build_sp_list)
{
  double min[2]={0,0};
  double max[2]={1,1};
  Rectangle<2> build_area(true, min, max);
  vector<Point> door_list;
  ///////////////get the floor plan for 2D areas and entrance point//////////
  Get2DAreadAndDoor(build_type, build_area, door_list);

  double length = build_area.MaxD(0) - build_area.MinD(0);
  double width = build_area.MaxD(1) - build_area.MinD(1);
  
  double length_new = rect->MaxD(0) - rect->MinD(0);
  double width_new = rect->MaxD(1) - rect->MinD(1);
  
  const double delta = 0.001;
  for(unsigned int i = 0;i < door_list.size();i++){
    double scale_x = (door_list[i].GetX() - build_area.MinD(0))/length;
    double scale_y = (door_list[i].GetY() - build_area.MinD(1))/width;
    double new_x;
    if(fabs(scale_x) < delta){
      new_x = rect->MinD(0);
    }else{
      new_x = rect->MinD(0) + scale_x*length_new;
    }

    double new_y;
    if(fabs(scale_y) < delta){
      new_y = rect->MinD(1);
    }else{
      new_y = rect->MinD(1) + scale_y* width_new;
    }

    Point new_door(true, new_x, new_y);
    build_sp_list.push_back(new_door);
  }

}

/*
set the building entrance point for personal apartments

*/
void MaxRect::BuildingEntranceHouse(Rectangle<2>* rect, 
                                    vector<Point>& build_sp_list,
                                    vector<Point>& build_ep_list, Region* r)
{
  double min_x = rect->MinD(0);
  double max_x = rect->MaxD(0);

  double min_y = rect->MinD(1);
  double max_y = rect->MaxD(1);

  Point p1(true, (min_x + max_x)/2, min_y);
  Point p2(true, (min_x + max_x)/2, max_y);
  Point p3(true, min_x, (min_y + max_y)/2);
  Point p4(true, max_x, (min_y + max_y)/2);

  build_sp_list.push_back(p1);
  build_sp_list.push_back(p2);
  build_sp_list.push_back(p3);
  build_sp_list.push_back(p4);
  
  
  SpacePartition* s_p = new SpacePartition();
  vector<MyPoint_Ext> point_dist_list;

  for(unsigned int i = 0;i < build_sp_list.size();i++){
      for(int j = 0;j < r->Size();j++){
        HalfSegment hs;
        r->Get(j, hs);
        if(!hs.IsLeftDomPoint())continue;

        double d;
        Point cp(true, 0, 0);

        d = s_p->GetClosestPoint(hs, build_sp_list[i], cp);
        MyPoint_Ext mpe(build_sp_list[i], cp, d, 0.0);
        point_dist_list.push_back(mpe);
      }
  }
  delete s_p;
  sort(point_dist_list.begin(), point_dist_list.end());

  build_sp_list.clear();
  build_ep_list.clear();

  build_sp_list.push_back(point_dist_list[0].loc);
  build_ep_list.push_back(point_dist_list[0].loc2);

}

/*
get the 2d area for a building and the location of entrance door from the 
floor plan(indoor graph)

*/
void MaxRect::Get2DAreadAndDoor(int build_type, Rectangle<2>& build_area, 
                           vector<Point>& door_list)
{
  Building* build = build_pointer[build_type];
  IndoorGraph* ig = igraph_pointer[build_type];
  if(build == NULL || ig == NULL){
    cout<<"such a building or indoor graph are not loaded"<<endl;
    return;
  }
  R_Tree<3, TupleId>* rtree = build->GetRTree();
  Rectangle<3> bbox = rtree->BoundingBox();

  double min[2], max[2];
  min[0] = bbox.MinD(0);
  min[1] = bbox.MinD(1);
  max[0] = bbox.MaxD(0);
  max[1] = bbox.MaxD(1);
  build_area.Set(true, min, max);
  
//  cout<<"2D building area "<<build_area;
//  cout<<"building graph: "<<GetBuildingStr(ig->GetGraphType())<<endl;

  ig->GetEntranceDoor(door_list);

//  cout<<"number of entrance "<<door_list.size()<<endl;
  
//   for(unsigned int i = 0;i < door_list.size();i++)
//     cout<<door_list[i]<<endl;

}



/*
find the closest point in pavement for the input location 

*/
void MaxRect::MapToPavement(DualGraph* dg, Point loc)
{
  SmiRecordId root_id = dg->rtree_node->RootRecordId();
  vector<int> tri_oid_list;
  const double dist = 5.0;
  dg->DFTraverse3(dg->rtree_node, root_id, loc, tri_oid_list, dist);

  if(tri_oid_list.size() == 0){
    cout<<loc<<endl;
    cout<<"!!! should not happen: no mapping point"<<endl;
  }


  double d = dist + dist;
  unsigned int index = 0;
  for(unsigned int i = 0;i < tri_oid_list.size();i++){
    Tuple* tuple = dg->node_rel_sort->GetTuple(tri_oid_list[i], false);
    Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);
//    int oid = ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval();
    double d2 = reg->Distance(loc);
//    cout<<"oid "<<oid<<" dist "<<d2<<endl; 
    if(d2 < d){
        d = d2;
        index = i;
    }
    tuple->DeleteIfAllowed();
  }

  Tuple* tuple = dg->node_rel_sort->GetTuple(tri_oid_list[index], false);
  Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);

  vector<MyPoint> point_dist_list;
  SpacePartition* s_p = new SpacePartition();
  for(int i = 0;i < reg->Size();i++){
    HalfSegment hs;
    reg->Get(i, hs);
    if(!hs.IsLeftDomPoint())continue;

    double d1;
    Point cp(true, 0, 0);
    d1 = s_p->GetClosestPoint(hs, loc, cp);
    MyPoint mpe(cp, d1);
    point_dist_list.push_back(mpe);

  }
  delete s_p;

  sort(point_dist_list.begin(), point_dist_list.end());
  Point map_loc = point_dist_list[0].loc; 
  ep_list2.push_back(map_loc);
  int tri_oid = ((CcInt*)tuple->GetAttribute(DualGraph::OID))->GetIntval();
//  cout<<"tri_oid "<<tri_oid<<endl;
  
  Rectangle<2> bbox = reg->BoundingBox();
  tuple->DeleteIfAllowed();
  
  //  if(map_loc.Distance(loc) > 1.0) cout<<map_loc.Distance(loc)<<endl;
  
  ////////////////generic location////////////////////////////////
  ///////relative position inside the triangle///////////////////
  //////////////////////////////////////////////////////////////////
  Loc temp_loc(map_loc.GetX() - bbox.MinD(0), map_loc.GetY() - bbox.MinD(1));
  GenLoc gloc(tri_oid, temp_loc);

  genloc_list.push_back(gloc); 
}

/*
create the path from building entrance to pavement area
the path should not accorss the building rectangle 

*/
void MaxRect::Path_BuildingPave(Point sp, Point ep, Rectangle<2>* rect, 
                                Region* r, DualGraph* dg)
{
   //////////////////////////////////////////////////////////////////////////
   //////////get the rectangle boundary line excluding the segment//////////
   ////////////containing the start point///////////////////////////////////
   /////////////////////////////////////////////////////////////////////////
    Region* temp_reg = new Region(*rect);
    Line* boundary_temp = new Line(0);
    temp_reg->Boundary(boundary_temp);
    
    Line* boundary = new Line(0);
    boundary->StartBulkLoad();
    int edgeno = 0;
    for(int i = 0;i < boundary_temp->Size();i++){
      HalfSegment hs;
      boundary_temp->Get(i, hs);
      if(!hs.IsLeftDomPoint() || hs.Contains(sp)) continue;
      HalfSegment new_hs(true, hs.GetLeftPoint(), hs.GetRightPoint());
      new_hs.attr.edgeno = edgeno++;
      *boundary += new_hs;
//      new_hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      new_hs.SetLeftDomPoint(!new_hs.IsLeftDomPoint());
      *boundary += new_hs;
    }
    boundary->EndBulkLoad();
    
    //////////////////////////////////////////////////////////////
    //////////////direction connection between sp and ep/////////
    /////////////////////////////////////////////////////////////
    Line* path = new Line(0);
    if(AlmostEqual(sp, ep)){
        path_list.push_back(*path);

        ep_list.push_back(ep);
        MapToPavement(dg, ep);
    }else{
        path->StartBulkLoad();
        HalfSegment hs(true, sp, ep);
        hs.attr.edgeno = 0;
        *path += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        *path += hs;
        path->EndBulkLoad();
        assert(path->Length() > 0.0);
        Points* ps = new Points(0);
        path->Crossings(*boundary, *ps);

//        cout<<ps->Size()<<endl;

        if(ps->Size() == 0){
          path_list.push_back(*path);
          ep_list.push_back(ep);
          MapToPavement(dg, ep);
        }
        else if(ps->Size() == 1){////////a new end point 
          //////////////////////////////////////////////////////////////////
          ////////////select one of the closest vertex to sp////////////////
          /////////plus the path from sp to the closet point on pavement///
          ////////////////////////////////////////////////////////////////

          Point p1(true, rect->MinD(0), rect->MinD(1));
          Point p2(true, rect->MaxD(0), rect->MinD(1));
          Point p3(true, rect->MaxD(0), rect->MaxD(1));
          Point p4(true, rect->MinD(0), rect->MaxD(1));
          vector<Point> p_list;
          p_list.push_back(p1);
          p_list.push_back(p2);
          p_list.push_back(p3);
          p_list.push_back(p4);

          vector<MyPoint_Ext> mp_list;
          SpacePartition* s_p = new SpacePartition();
          for(unsigned int i = 0;i < p_list.size();i++){

            for(int j = 0;j < r->Size();j++){
                HalfSegment temp_hs;
                r->Get(j, temp_hs);
                if(!temp_hs.IsLeftDomPoint())continue;

                double d;
                Point cp(true, 0, 0);
                d = s_p->GetClosestPoint(temp_hs, p_list[i], cp);

                MyPoint_Ext mpe(p_list[i], cp, d, 0.0);
                mp_list.push_back(mpe);
            }
          }
          delete s_p;
          sort(mp_list.begin(), mp_list.end());
          Point cp_border = mp_list[0].loc;
          Point new_ep = mp_list[0].loc2;

          //////////////////////////////////////////////////////////////////
          ///////////////new path has two parts: sp to cpborder,/////////// 
          //////////////// cpborder to ep /////////////////////////////////
          /////////////////////////////////////////////////////////////////
          Line* path2 = new Line(0);
          path2->StartBulkLoad();
          int edgeno = 0;

          if(!AlmostEqual(cp_border, new_ep)){
            HalfSegment hs(true, cp_border, new_ep);
            hs.attr.edgeno = edgeno++;
            *path2 += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *path2 += hs;
          }

          PathOnBorder(boundary_temp, sp, cp_border, path2, edgeno);

          path2->EndBulkLoad();
          path_list.push_back(*path2);
          delete path2;

          ep_list.push_back(new_ep);
          MapToPavement(dg, new_ep);
        }else assert(false);

        delete ps;
    }

    ///////////////////////////////////////////////////////////////
    delete boundary_temp;
    delete temp_reg;
    delete path;
    delete boundary; 

}

/*
calculate the shortest path between two points on a line. 
the line is a cycle so there are two options

*/
void MaxRect::PathOnBorder(Line* boundary_temp, Point sp, Point cp_border, 
                      Line* path2, int& edgeno)
{
   SpacePartition* s_p = new SpacePartition();
   //////////////////find the closest point to sp along the line///////////////
   if(sp.Inside(*boundary_temp) == false){
      vector<MyPoint> mp_list;
      for(int j = 0;j < boundary_temp->Size();j++){
        HalfSegment temp_hs;
        boundary_temp->Get(j, temp_hs);
        if(!temp_hs.IsLeftDomPoint())continue;

        double d;
        Point cp(true, 0, 0);
        d = s_p->GetClosestPoint(temp_hs, sp, cp);
        MyPoint mpe(cp, d);
        mp_list.push_back(mpe);
      }

     sort(mp_list.begin(), mp_list.end());
     Point sp_map = mp_list[0].loc;

     HalfSegment hs(true, sp, sp_map);
     hs.attr.edgeno = edgeno++;
     *path2 += hs;
     hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
     *path2 += hs;

     sp = sp_map;
   }
   ///////////////////////////////////////////////////////////////////////
   ////////////////connect sp and cp border along the line///////////////
   ///////////////as the boundary is a cycle, there are two paths///////
   ///////////////     select the shorter one    ///////////////////////
   //////////////////////////////////////////////////////////////////////

   SimpleLine* sline = new SimpleLine(0);
   sline->fromLine(*boundary_temp);
   vector<MyHalfSegment> seq_halfseg; 
   s_p->ReorderLine(sline, seq_halfseg);
   delete sline;

   int index1 = -1;
   for(unsigned int i = 0;i < seq_halfseg.size();i++){
    HalfSegment hs(true, seq_halfseg[i].from, seq_halfseg[i].to);
    if(hs.Contains(sp)){
      index1 = i;
      break;
    }
   }

   int index2 = -1;
   for(unsigned int i = 0;i < seq_halfseg.size();i++){
    HalfSegment hs(true, seq_halfseg[i].from, seq_halfseg[i].to);
    if(hs.Contains(cp_border)){
      index2 = i;
      break;
    }
   }
    assert(0 <= index1 && index1 < (int)seq_halfseg.size());
    assert(0 <= index2 && index2 < (int)seq_halfseg.size());

 //   cout<<"index1 "<<index1<<" index2 "<<index2<<endl;
    if(index1 == index2){

     HalfSegment hs(true, sp, cp_border);
     hs.attr.edgeno = edgeno++;
     *path2 += hs;
     hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
     *path2 += hs;
    }else{
      ////////////////////////////////////////////////////////////////
      ///////////////////find one possible connection/////////////////
      ///////////////////////////////////////////////////////////////
      Line* temp = new Line(0);
      temp->StartBulkLoad();
      int no = 0;
      if(index1 < index2 ){
        for(int i = index1;i <= index2;i++){
          Point p1, p2;
          if(i == index1){
            p1 = sp;
            p2 = seq_halfseg[i].to;
          }else if(i == index2){
            p1 = seq_halfseg[i].from;
            p2 = cp_border;
          }else{
            p1 = seq_halfseg[i].from;
            p2 = seq_halfseg[i].to;
          }
          if(!AlmostEqual(p1,p2)){
            HalfSegment hs(true, p1, p2);
            hs.attr.edgeno = no++;
            *temp += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *temp += hs;
          }
        }
      }else{
         for(int i = index2;i <= index1;i++){
          Point p1, p2;
          if(i == index2){
            p1 = cp_border;
            p2 = seq_halfseg[i].to;
          }else if(i == index1){
            p1 = seq_halfseg[i].from;
            p2 = sp;
          }else{
            p1 = seq_halfseg[i].from;
            p2 = seq_halfseg[i].to;
          }
          if(!AlmostEqual(p1,p2)){
            HalfSegment hs(true, p1, p2);
            hs.attr.edgeno = no++;
            *temp += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *temp += hs;
          }
        }
      }
      temp->EndBulkLoad();
      ///////////////////////////////////////////////////////////////////
      double l1 = temp->Length();
      double l2 = boundary_temp->Length() - l1;
//      cout<<"l1 "<<l1<<" l2 "<<l2<<endl;
      if(l1 < l2){/////////////select temp
        for(int i = 0;i < temp->Size();i++){
          HalfSegment hs1;
          temp->Get(i, hs1);
          if(!hs1.IsLeftDomPoint())continue;
          HalfSegment hs2(true, hs1.GetLeftPoint(), hs1.GetRightPoint());
          hs2.attr.edgeno = edgeno++;
          *path2 += hs2;
          hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
          *path2 += hs2;
        }
      }else{///////////////select the other one, use bounday minus temp 
          Line* res = new Line(0);
          boundary_temp->Minus(*temp, *res);
          for(int i = 0;i < res->Size();i++){
              HalfSegment hs1;
              res->Get(i, hs1);
              if(!hs1.IsLeftDomPoint())continue;
              HalfSegment hs2(true, hs1.GetLeftPoint(), hs1.GetRightPoint());
              hs2.attr.edgeno = edgeno++;
              *path2 += hs2;
              hs2.SetLeftDomPoint(!hs2.IsLeftDomPoint());
              *path2 += hs2;
          }
          delete res;
      }
      delete temp;
    }
    delete s_p;
}


/*
create a new region where r is outer cycle and the rectangles inside are holes

1. we have all points of all cycle so that we can call the function
buildRegion(). but very important: for each cycle, 
the first and the last point should be the same. 

2. we can directly use the outer region minus these rectangles which is much
simpler 

*/
bool MaxRect::RegionWithHole(vector<Rectangle<2> >& hole_list, Region* reg)
{

/*    for(unsigned int i = 0;i < hole_list.size();i++){
        Region* temp = new Region(hole_list[i]);
//        cout<<*temp<<endl; 
        Region* res = new Region(0);
        reg->Minus(*temp, *res);
        *reg = *res;
        delete res;
        delete temp; 
    }*/


  //////////////////////////////////////////////////////////////////////////
  Region* r = new Region(0);

  for(unsigned int i = 0;i < hole_list.size();i++){
      Region* temp = new Region(hole_list[i]);
      Region* res = new Region(0);
      temp->Union(*r, *res);
      *r = *res;
      delete res;
      delete temp; 
  }

  Region* res = new Region(0);
  reg->Minus(*r, *res);
  *reg = *res;

  delete res;
  delete r;

  return true;
}

/*
for each rectangle, it sets which kind of building it is, 
e.g., office or cinema
1 airport, 1-2 trainstations, 8 cinemas, 32 hotels,  48 shopping malls,
20 hospitals, 32 schooles, 4 libraries, 16 universities, 

200 office 24, 

houses and apartments: 1000


the left are of type none 

*/
void MaxRect::SetBuildingType(R_Tree<2,TupleId>* rtree, Space* gl_sp)
{
  Rectangle<2> bbox = rtree->BoundingBox();
  Point center(true, (bbox.MinD(0) + bbox.MaxD(0))/2, 
                     (bbox.MinD(1) + bbox.MaxD(1))/2);

  vector<Build_Rect> build_rect_list;

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* tuple = rel1->GetTuple(i, false);
    int reg_id = ((CcInt*)tuple->GetAttribute(REG_ID))->GetIntval();
    Rectangle<2>* rect = (Rectangle<2>*)tuple->GetAttribute(GEODATA);
    int poly_id = ((CcInt*)tuple->GetAttribute(POLY_ID))->GetIntval();
    int reg_type = ((CcInt*)tuple->GetAttribute(REG_TYPE))->GetIntval();

    Build_Rect br(reg_id, *rect, poly_id, reg_type);
    Point p(true, (rect->MinD(0) + rect->MaxD(0))/2,
                  (rect->MinD(1) + rect->MaxD(1))/2);
    if(p.GetX() > center.GetX()){
        if(p.GetY() > center.GetY()) br.quadrant = 1;
        else br.quadrant = 4;
    }else{
        if(p.GetY() > center.GetY()) br.quadrant = 2;
         else br.quadrant = 3;
    }

    build_rect_list.push_back(br);
    tuple->DeleteIfAllowed();
  }
  
//  cout<<build_rect_list.size()<<endl; 
  
  ////////////////////////////////////////////////////////////////////
  /////////////////set up building type//////////////////////////////
  ///////////////////////////////////////////////////////////////////
  SetAirPort(build_rect_list, rtree);//1 airport 
  SetTrainStation(build_rect_list); // a trainstation 

  /////////////////these buildings can be close to each other//////////////
  SetCinema(build_rect_list, 8, bbox);//maximum 8 cinemas 
  SetHotel(build_rect_list, 40, bbox);//maximum 40 hotels
  SetShopMall(build_rect_list, 80, bbox);//maximum 80 shopping malls

  SetOffice24(build_rect_list, 600);//maximum 500 office24 
  SetOffice38(build_rect_list, 600);//maximum 500 office38

  ////////////can not be close to the above three/////////////////////////
  /////////////but may be close to school or houses, apartments//////////
//  SetHospital(build_rect_list, 20);//maximum 20 hospitals
  SetHospital(build_rect_list, 50);//maximum 50 hospitals

  ///////////////////////////////////////////////////////////////////////
  //////////////////school library//////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SetLibrary(build_rect_list, 4);//maximum 4 libraries 
//  SetSchool(build_rect_list, 32);//maximum 32 schools, no school yet
  SetUniversity(build_rect_list, 20);//maximum 20 universities  

  /////////////////////////////////////////////////////////////////////
//  SetHouse(build_rect_list, 3000);//maximum 3000 houses and apartments 
  SetHouse(build_rect_list, 5000);//maximum 5000 houses and apartments 

  ///////////////////////////////////////////////////////////////////
  int cur_max_ref_id = gl_sp->MaxRefId() + 1;
    ///////////////////////////////////////////////////////////////
  ///////////////max int 21474 83647//////////////////////////////
  /////////////max int64_t 9223372036854775807///////////////////
  ////////////////////////////////////////////////////////////////

//  cout<<"max int "<<numeric_limits<int>::max()<<endl;
//  cout<<"max int64_t "<<numeric_limits<int64_t>::max()<<endl;

  if(cur_max_ref_id == 0){
    cout<<"!!! the space is empty !!!"<<endl;
    return;
  }

  ///////////////////////////////////////////////////////////////////
  ////////////////to check the minimum and maximum number///////////
  ///////////////have the same number of digits////////////////////
  /////////////for example, min:12345 max: 34567///////////////////
  ////////////but min:12345 max 234567 are wrong!!!////////////////
  //////////////////////////////////////////////////////////////////
  int min_id = cur_max_ref_id;
  int build_no = 0;
  for(unsigned int i = 0;i < build_rect_list.size();i++){
    if(build_rect_list[i].build_type > 0){
        build_no++;
    }
  }
  int max_id = min_id + build_no;
  char buf1[64], buf2[64];
  sprintf(buf1, "%d", min_id);
  sprintf(buf2, "%d", max_id);
//  cout<<"min_id "<<min_id<<" max_id "<<max_id<<endl;
  if(strlen(buf1) < strlen(buf2)){////////perhaps + 1000000
    cout<<"we need to reset the starting number for building id"<<endl;
//    cur_max_ref_id += pow(10, strlen(buf2) + 1);
    double base = 10;
    double exponent = strlen(buf2) + 1;
    cur_max_ref_id += pow(base, exponent);
    assert(cur_max_ref_id + build_no < numeric_limits<int>::max());
    assert(false);///////// can be deleted the above code should be correct
  }
  //////////////////////////////////////////////////////////////////
  
  for(unsigned int i = 0;i < build_rect_list.size();i++){
    if(build_rect_list[i].build_type > 0){
      reg_id_list.push_back(build_rect_list[i].reg_id);
      rect_list.push_back(build_rect_list[i].rect);
      poly_id_list.push_back(build_rect_list[i].poly_id);
      reg_type_list.push_back(build_rect_list[i].reg_type);
      build_type_list.push_back(build_rect_list[i].build_type);
      build_type2_list.push_back(GetBuildingStr(build_rect_list[i].build_type));
      build_id_list.push_back(cur_max_ref_id);
      cur_max_ref_id++;
    }
  }

  if(cur_max_ref_id > numeric_limits<int>::max() - 1){
    cout<<"!!!! building id number overflow !!!"<<endl;
    assert(false);
  }

}

/*
set the airport 

*/
#define AIRPORT_AREA 9000.0
bool Build_Rect_COM1(const Build_Rect& br1, const Build_Rect& br2)
{
  return br1.reg_type < br2.reg_type;
}
/*
an airport is an place far away from the city center and big enough, and 
there are no other buildings nearby. 

*/
void MaxRect::SetAirPort(vector<Build_Rect>& list, R_Tree<2,TupleId>* rtree)
{
  sort(list.begin(), list.end(), Build_Rect_COM1);
/*  for(unsigned int i = 0;i < list.size();i++)
    list[i].Print();*/
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > AIRPORT_AREA && list[i].reg_type == 3 &&
      list[i].init == false && NoNeighbor(list[i], rtree)){
//      cout<<"find an airport site"<<endl;
      list[i].build_type = BUILD_AIRPORT;
      list[i].init = true;
      return;
    }
  }

  cout<<"no avaialbe place for an airport"<<endl; 
}


/*
there are no rectangles whose distance to the input rect is smalelr than a 
distance threshold 

*/
bool MaxRect::NoNeighbor(Build_Rect& br, R_Tree<2,TupleId>* rtree)
{
  
  vector<int> tid_list;
  DFTraverse1(rtree, rtree->RootRecordId(), br.rect, tid_list);
//  cout<<"reg id "<<br.reg_id<<" neighbor size "<<tid_list.size()<<endl;
//   for(unsigned int i = 0;i < tid_list.size();i++){
//     cout<<"neighbor "<<tid_list[i]<<" ";
//   }
//  cout<<endl;

  if(tid_list.size() == 0) return true;
  else return false;
}

void MaxRect::DFTraverse1(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                    Rectangle<2>& rect, vector<int>& tri_oid_list)
{
  const double dist = 1000.0; 
  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* dg_tuple = rel1->GetTuple(e.info, false);
              Rectangle<2>* reg = 
                      (Rectangle<2>*)dg_tuple->GetAttribute(GEODATA);
              int reg_id = 
                   ((CcInt*)dg_tuple->GetAttribute(REG_ID))->GetIntval();
              double d = rect.Distance(*reg);
              if(d > 0.0 && d < dist){
                  tri_oid_list.push_back(reg_id);
              }
              dg_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(rect.Distance(e.box) < dist){
                DFTraverse1(rtree, e.pointer, rect, tri_oid_list);
            }
      }
  }
  delete node;
}


/*
set the train station 

*/
#define TRAINSTATION_AREA_MIN 800.0
#define TRAINSTATION_AREA_MAX 2000.0

void MaxRect::SetTrainStation(vector<Build_Rect>& list)
{
  sort(list.begin(), list.end(), Build_Rect_COM1);
/*  for(unsigned int i = 0;i < list.size();i++)
    list[i].Print();*/
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > TRAINSTATION_AREA_MIN &&
       list[i].rect.Area() < TRAINSTATION_AREA_MAX && 
       list[i].reg_type == 1 &&
       list[i].init == false){
//      cout<<"find a train station site"<<endl;
      list[i].build_type = BUILD_TRAINSTATION;
      list[i].init = true;
      return;
    }
  }

  cout<<"no avaialbe place for a train station"<<endl; 
}


/*
set the places for cinemas, uniformly distributed in each quadrant

*/
#define CINEMA_AREA_MIN 600.0
#define CINEMA_AREA_MAX 1200.0

void MaxRect::SetCinema(vector<Build_Rect>& list, unsigned int no,
                        Rectangle<2> bbox)
{
  vector<Build_Rect> cand_list; 
  
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);

  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > CINEMA_AREA_MIN && 
       list[i].rect.Area() < CINEMA_AREA_MAX &&
      (list[i].reg_type == 1 || list[i].reg_type == 2) &&
      list[i].init == false && NoNeighborCinema(cand_list, list[i], bbox)){
//      cout<<"find a cinema site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_CINEMA;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
cinemas should not be close to each other 

*/
bool MaxRect::NoNeighborCinema(vector<Build_Rect>& list, Build_Rect br,
                               Rectangle<2> bbox)
{
  const double min_dist = 5000.0;
//   Point p1(true, bbox.MinD(0), bbox.MinD(1));
//   Point p2(true, bbox.MaxD(0), bbox.MaxD(1));
//   
//   const double min_dist = p1.Distance(p2)/4;
  
  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}


/*
set the places for hotels,  uniformly distributed in each quadrant 

*/
#define HOTEL_AREA_MIN 800.0
#define HOTEL_AREA_MAX 1600.0

void MaxRect::SetHotel(vector<Build_Rect>& list, unsigned int no,
                       Rectangle<2> bbox)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  
  vector<Build_Rect> cand_list; 
  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > HOTEL_AREA_MIN && 
       list[i].rect.Area() < HOTEL_AREA_MAX &&
      list[i].init == false && NoNeighborHotel(cand_list, list[i], bbox)){
//      cout<<"find a hotel site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_HOTEL;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
hotels should not be close to each other 

*/
bool MaxRect::NoNeighborHotel(vector<Build_Rect>& list, Build_Rect br,
                              Rectangle<2> bbox)
{
  const double min_dist = 3000.0;

//   Point p1(true, bbox.MinD(0), bbox.MinD(1));
//   Point p2(true, bbox.MaxD(0), bbox.MaxD(1));
//   
//   const double min_dist = p1.Distance(p2)/8;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}


/*
set the places for shopping malls,  uniformly distributed in each quadrant 

*/
#define SHOPMALL_AREA_MIN 1000.0
#define SHOPMALL_AREA_MAX 2000.0

void MaxRect::SetShopMall(vector<Build_Rect>& list, unsigned int no,
                       Rectangle<2> bbox)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  
  vector<Build_Rect> cand_list; 
  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > SHOPMALL_AREA_MIN && 
       list[i].rect.Area() < SHOPMALL_AREA_MAX &&
      list[i].init == false && NoNeighborShopMall(cand_list, list[i], bbox)){
//      cout<<"find a hotel site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_SHOPPINGMALL;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
shopping malls should not be close to each other 

*/
bool MaxRect::NoNeighborShopMall(vector<Build_Rect>& list, Build_Rect br,
                              Rectangle<2> bbox)
{
  const double min_dist = 1500.0;

//   Point p1(true, bbox.MinD(0), bbox.MinD(1));
//   Point p2(true, bbox.MaxD(0), bbox.MaxD(1));
//   
//   const double min_dist = p1.Distance(p2)/8;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}

/*
set the places for office24,  uniformly distributed in each quadrant 

*/
#define OFFICE24_AREA_MIN 600.0
#define OFFICE24_AREA_MAX 2500.0

void MaxRect::SetOffice24(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);

  vector<Build_Rect> cand_list; 

  vector<unsigned int> reg_type_list;
  reg_type_list.push_back(0);
  reg_type_list.push_back(0);
  reg_type_list.push_back(0);

  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > OFFICE24_AREA_MIN && 
       list[i].rect.Area() < OFFICE24_AREA_MAX &&
      list[i].init == false){
//      cout<<"find a office24 site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4 && 
         reg_type_list[list[i].reg_type - 1] < no /3){
        list[i].build_type = BUILD_OFFICE24;
        list[i].init = true;
        cand_list.push_back(list[i]);

        quad_list[list[i].quadrant - 1]++;
        reg_type_list[list[i].reg_type - 1]++;

        if(cand_list.size() == no)return;
      }
    }
  }
  
}

/*
set the places for office38,  uniformly distributed in each quadrant 

*/
#define OFFICE38_AREA_MIN 1000.0
#define OFFICE38_AREA_MAX 3500.0


void MaxRect::SetOffice38(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);

  vector<Build_Rect> cand_list; 

  vector<unsigned int> reg_type_list;
  reg_type_list.push_back(0);
  reg_type_list.push_back(0);
  reg_type_list.push_back(0);

  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > OFFICE38_AREA_MIN && 
       list[i].rect.Area() < OFFICE38_AREA_MAX &&
      list[i].init == false){
//      cout<<"find a office38 site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4 && 
         reg_type_list[list[i].reg_type - 1] < no /3){
        list[i].build_type = BUILD_OFFICE38;
        list[i].init = true;
        cand_list.push_back(list[i]);

        quad_list[list[i].quadrant - 1]++;
        reg_type_list[list[i].reg_type - 1]++;

        if(cand_list.size() == no)return;
      }
    }
  }

}


/*
set the places for hospitals ,  uniformly distributed in each quadrant 

*/
#define HOSPITAL_AREA_MIN 800.0
#define HOSPITAL_AREA_MAX 2000.0

void MaxRect::SetHospital(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  
  vector<Build_Rect> cand_list; 
  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > HOSPITAL_AREA_MIN && 
       list[i].rect.Area() < HOSPITAL_AREA_MAX &&
      list[i].init == false && NoNeighborHospital(cand_list, list[i]) &&
       NoNearbyShopMallAndCinema(list, list[i])){
//      cout<<"find a hotel site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_HOSPITAL;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
hosptials should not be close to each other 

*/
bool MaxRect::NoNeighborHospital(vector<Build_Rect>& list, Build_Rect br)
{
  const double min_dist = 2000.0;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}

/*
a hospital should not have a shopping mall and cinema so close 
it is also not too close to a train station 

*/
bool MaxRect::NoNearbyShopMallAndCinema(vector<Build_Rect>& list, Build_Rect br)
{
  const double min_dist1 = 1000.0;
  const double min_dist2 = 500.0;
  const double min_dist3 = 300.0;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++){
    if((list[i].build_type == BUILD_SHOPPINGMALL || 
        list[i].build_type == BUILD_CINEMA) &&
        list[i].rect.Distance(br.rect) < min_dist1) return false;

    if(list[i].build_type == BUILD_HOTEL &&
       list[i].rect.Distance(br.rect) < min_dist2) return false;

    if(list[i].build_type == BUILD_TRAINSTATION &&
       list[i].rect.Distance(br.rect) < min_dist3) return false;
  }
  return true;
}

/*
set the places for libraries ,  uniformly distributed in each quadrant 

*/
#define LIBRARY_AREA_MIN 1000.0
#define LIBRARY_AREA_MAX 2000.0

void MaxRect::SetLibrary(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  
  vector<Build_Rect> cand_list; 
  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > LIBRARY_AREA_MIN && 
       list[i].rect.Area() < LIBRARY_AREA_MAX &&
       list[i].reg_type == 2 &&
       list[i].init == false && NoNeighborLibrary(cand_list, list[i]) && 
       NoNearbyCommercialBuilding(list, list[i])){
//      cout<<"find a hotel site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_LIBRARY;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
library should not be close to each other 

*/
bool MaxRect::NoNeighborLibrary(vector<Build_Rect>& list, Build_Rect br)
{
  const double min_dist = 3000.0;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}

/*
a library should not so close to cinemas, hotels, shopping malls 

*/
bool MaxRect::NoNearbyCommercialBuilding(vector<Build_Rect>& list, 
                                         Build_Rect br)
{
  const double min_dist1 = 1000.0;
  const double min_dist2 = 500.0;
  const double min_dist3 = 300.0;

  if(list.size() == 0) return true;
  for(unsigned int i = 0;i < list.size();i++){
    if((list[i].build_type == BUILD_SHOPPINGMALL ||
        list[i].build_type == BUILD_CINEMA) &&
        list[i].rect.Distance(br.rect) < min_dist1) return false;

    if((list[i].build_type == BUILD_HOTEL || 
        list[i].build_type == BUILD_HOSPITAL )&&
       list[i].rect.Distance(br.rect) < min_dist2) return false;

    if(list[i].build_type == BUILD_TRAINSTATION &&
       list[i].rect.Distance(br.rect) < min_dist3) return false;

  }
  return true;
}


/*
set the places for schools,  uniformly distributed in each quadrant 

*/
#define SCHOOL_AREA_MIN 2000.0
#define SCHOOL_AREA_MAX 3000.0

void MaxRect::SetSchool(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  
  vector<Build_Rect> cand_list; 
  
  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > SCHOOL_AREA_MIN && 
       list[i].rect.Area() < SCHOOL_AREA_MAX &&
       (list[i].reg_type == 1 || list[i].reg_type == 2)&&
       list[i].init == false && NoNeighborSchool(cand_list, list[i]) && 
       NoNearbyCommercialBuilding(list, list[i])){
//      cout<<"find a school site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_SCHOOL;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
schools should not be too close to each other 

*/
bool MaxRect::NoNeighborSchool(vector<Build_Rect>& list, Build_Rect br)
{
  const double min_dist = 600.0;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}

/*
set the places for universities,  uniformly distributed in each quadrant 

*/
#define UNIVERSITY_AREA_MIN 2000.0
#define UNIVERSITY_AREA_MAX 5000.0

void MaxRect::SetUniversity(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);

  vector<Build_Rect> cand_list;

  for(int i = list.size() - 1;i >= 0;i--){
    if(list[i].rect.Area() > UNIVERSITY_AREA_MIN && 
       list[i].rect.Area() < UNIVERSITY_AREA_MAX &&
       (list[i].reg_type == 2 || list[i].reg_type == 3)&&
       list[i].init == false && NoNeighborUniversity(cand_list, list[i]) && 
       NoNearbyCommercialBuilding(list, list[i])){
//      cout<<"find a hotel site"<<endl;
      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_UNIVERSITY;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
      }
    }
  }
}

/*
universities should not be too close to each other 

*/
bool MaxRect::NoNeighborUniversity(vector<Build_Rect>& list, Build_Rect br)
{
  const double min_dist = 500.0;

  if(list.size() == 0)return true;
  for(unsigned int i = 0;i < list.size();i++)
    if(list[i].rect.Distance(br.rect) < min_dist) return false;

  return true;
}



/*
set the places for houses,  uniformly distributed in each quadrant 

*/
#define HOUSE_AREA_MAX 1500.0

void MaxRect::SetHouse(vector<Build_Rect>& list, unsigned int no)
{
  vector<unsigned int> quad_list;
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  quad_list.push_back(0);
  
  vector<Build_Rect> cand_list; 
  
  for(int i = list.size() - 1;i >= 0;i--){
    if( list[i].rect.Area() < HOUSE_AREA_MAX &&

       list[i].init == false && NoNearbyNeighbors1(list, list[i])){

//      if(quad_list[list[i].quadrant - 1] < no / 4){
        list[i].build_type = BUILD_HOUSE;
        list[i].init = true;
        cand_list.push_back(list[i]);
        quad_list[list[i].quadrant - 1]++;
        if(cand_list.size() == no)return;
//      }
    }
  }
}

/*
neighbor building limitations for personal houses and apartments 

*/
bool MaxRect::NoNearbyNeighbors1(vector<Build_Rect>& list, Build_Rect br)
{

//   const double min_dist1 = 800.0;/////////cinema, hotel
//   const double min_dist2 = 400.0;//////shopping mall
//   const double min_dist3 = 200.0;////////others 

  const double min_dist1 = 500.0;/////////cinema,
  const double min_dist2 = 300.0;//////shopping mall, hotel
  const double min_dist3 = 100.0;////////others 

  if(list.size() == 0){ 
    return true;
  }

  for(unsigned int i = 0;i < list.size();i++){
    if((list[i].build_type == BUILD_CINEMA) &&
        list[i].rect.Distance(br.rect) < min_dist1) return false;

    if((list[i].build_type == BUILD_SHOPPINGMALL ||
        list[i].build_type == BUILD_HOTEL) &&
        list[i].rect.Distance(br.rect) < min_dist2) return false;

    if(( list[i].build_type == BUILD_SCHOOL || 
         list[i].build_type == BUILD_HOSPITAL ||
         list[i].build_type == BUILD_LIBRARY || 
         list[i].build_type == BUILD_OFFICE24 || 
         list[i].build_type == BUILD_UNIVERSITY || 
         list[i].build_type == BUILD_TRAINSTATION ) &&
       list[i].rect.Distance(br.rect) < min_dist3) return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
///////////////Infrastructure Region Based Outdoor Pavement////////////////
///////////////////////////////////////////////////////////////////////////
ListExpr PavementProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom("pavement"),
         nl->StringAtom("((def, id))"),
           nl->StringAtom("((TRUE 1))"))));
}
/*
output the pavement infrastructure 

*/
ListExpr OutPavement( ListExpr typeInfo, Word value )
{
//  cout<<"OutPavement"<<endl; 
  Pavement* pn = (Pavement*)(value.addr);
  if(!pn->IsDefined()){
    return nl->SymbolAtom("undef");
  }

  ListExpr list1 = nl->TwoElemList(
               nl->StringAtom("Pavement Id:"), 
               nl->IntAtom(pn->GetId()));
  
  ListExpr list2 = nl->TheEmptyList(); 
  if(pn->IsDGInit()){
      list2 = nl->TwoElemList(
               nl->StringAtom("Dual Graph Id:"), 
               nl->IntAtom(pn->GetDGId()));
  }else
    list2 = nl->OneElemList( nl->StringAtom("Dual Graph undef"));
    
  ListExpr list3 = nl->TheEmptyList(); 
  if(pn->IsVGInit()){
      list3 = nl->TwoElemList(
               nl->StringAtom("Visbility Graph Id:"), 
               nl->IntAtom(pn->GetVGId()));
  }else
    list3 = nl->OneElemList( nl->StringAtom("Visibility Graph undef"));


  ListExpr list4 = nl->TheEmptyList(); 
  
  return nl->FourElemList(list1, list2, list3, list4);
}

/*
In function. there is not nested list expression here.

*/
Word InPavement( const ListExpr typeInfo, const ListExpr instance,
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
      cout<< "pavenetwork(): definition must be bool type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    bool d = nl->BoolValue(first);


    if(!nl->IsAtom(second) || nl->AtomType(second) != IntType){
      cout<< "pavenetwork(): pavement infrastructure id must be int type"<<endl;
      correct = false;
      return SetWord(Address(0));
    }
    unsigned int id = nl->IntValue(second);

    Pavement* pn = new Pavement(d, id); 

   ////////////////very important /////////////////////////////
    correct = true; 
  ///////////////////////////////////////////////////////////
    return SetWord(pn);
  }

  correct = false;
  return SetWord(Address(0));
}

bool SavePavement(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  Pavement* pn = (Pavement*)value.addr;
  return pn->Save(valueRecord, offset, typeInfo);
}

bool OpenPavement(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value)
{
  value.addr = Pavement::Open(valueRecord, offset, typeInfo);
  return value.addr != NULL; 
}

Word CreatePavement(const ListExpr typeInfo)
{
// cout<<"CreatePavement()"<<endl;
  return SetWord (new Pavement());
}


void DeletePavement(const ListExpr typeInfo, Word& w)
{
// cout<<"DeletePavement()"<<endl;
  Pavement* pn = (Pavement*)w.addr;
  delete pn;
   w.addr = NULL;
}

void ClosePavement( const ListExpr typeInfo, Word& w )
{
//  cout<<"ClosePavement"<<endl; 
  delete static_cast<Pavement*>(w.addr); 
  w.addr = 0;
}

Word ClonePavement( const ListExpr typeInfo, const Word& w )
{
//  cout<<"ClonePavement"<<endl; 
  return SetWord( new Address(0));
}


void* Pavement::Cast(void* addr)
{
  return NULL;
}

int SizeOfPavement()
{
//  cout<<"SizeOfPavement"<<endl; 
  return sizeof(Pavement);
}

bool CheckPavement( ListExpr type, ListExpr& errorInfo )
{
//  cout<<"CheckPavement"<<endl; 
  return (nl->IsEqual( type, "pavenetwork" ));
}


Pavement::Pavement():def(false), pave_id(0), dg_init(false), dg_id(0), 
vg_init(false), vg_id(0), pave_rel(NULL)
{


}


Pavement::Pavement(bool d, unsigned int i):def(d), pave_id(i),
dg_init(false), dg_id(0), vg_init(false), vg_id(0), pave_rel(NULL)
{



}

Pavement::Pavement(SmiRecord& valueRecord, size_t& offset, 
                   const ListExpr typeInfo):def(false), pave_id(0), 
dg_init(false), dg_id(0), vg_init(false), vg_id(0), pave_rel(NULL)
{


  valueRecord.Read(&def, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&pave_id, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Read(&dg_init, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&dg_id, sizeof(int), offset);
  offset += sizeof(int);

  valueRecord.Read(&vg_init, sizeof(bool), offset);
  offset += sizeof(bool);

  valueRecord.Read(&vg_id, sizeof(int), offset);
  offset += sizeof(int);

  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for triangles*********************/
//  nl->ReadFromString(PaveTypeInfo,xType);
  nl->ReadFromString(DualGraph::NodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  pave_rel = Relation::Open(valueRecord, offset, xNumericType);
  if(!pave_rel) {
   return;
  }

}

Pavement::~Pavement()
{
  if(pave_rel != NULL) pave_rel->Close(); 

}

/*
create the pavement infrastructure by a relation storing all triangles 

*/
void Pavement::Load(unsigned int i, Relation* r)
{
  if(i < 1){
    cout<<"invalid id "<<i<<endl; 
    def = false;
    return;
  }
  pave_id = i; 

  def = true; 
  


  ListExpr ptrList = listutils::getPtrList(r);

//  string strQuery = "(consume(feed(" + PaveTypeInfo +
  string strQuery = "(consume(feed(" + DualGraph::NodeTypeInfo +
                "(ptr " + nl->ToString(ptrList) + "))))";

//  cout<<strQuery<<endl; 

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  pave_rel = (Relation*)xResult.addr; 
  
//  cout<<pave_rel->GetNoTuples()<<endl; 

}

bool Pavement::Save(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo)
{
  
//  cout<<"Pavement::Save"<<endl; 

  valueRecord.Write(&def, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&pave_id, sizeof(int), offset); 
  offset += sizeof(int); 

  valueRecord.Write(&dg_init, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&dg_id, sizeof(int), offset); 
  offset += sizeof(int); 

  valueRecord.Write(&vg_init, sizeof(bool), offset); 
  offset += sizeof(bool); 

  valueRecord.Write(&vg_id, sizeof(int), offset); 
  offset += sizeof(int); 

  
  ListExpr xType;
  ListExpr xNumericType;

  ////////////////////triangles relation/////////////////////////////
//  nl->ReadFromString(PaveTypeInfo, xType);
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!pave_rel->Save(valueRecord,offset,xNumericType))
      return false;

  return true; 
}

/*
open a pavement object 

*/
Pavement* Pavement::Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo)
{
  return new Pavement(valueRecord, offset, typeInfo); 

}

/*
set the dual graph id for the pavement 

*/
void Pavement::SetDualGraphId(int id)
{
  dg_id = id;
  dg_init = true;
}


/*
set the visual graph id for the pavement 

*/
void Pavement::SetVisualGraphId(int id)
{
  vg_id = id;
  vg_init = true; 
}

/*
load dual graph if it is stored 

*/
DualGraph* Pavement::GetDualGraph()
{
  if(dg_init == false) return NULL;
  
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
       nl->SymbolValue(xObjectType) == "dualgraph"){
      // Get name of the dual graph 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the dual graph. 
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
      DualGraph* dg = (DualGraph*)xValue.addr;
      if(dg->g_id == dg_id){
        // This is the dual graph we have been looking for
        return dg;
      }
    }
  }
  return NULL;
}

void Pavement::CloseDualGraph(DualGraph* dg)
{
  if(dg == NULL) return; 
  Word xValue;
  xValue.addr = dg;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "dualgraph" ),
                                           xValue);
}

/*
load visual graph if it is stored 

*/
VisualGraph* Pavement::GetVisualGraph()
{
  if(vg_init == false) return NULL;
  
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
       nl->SymbolValue(xObjectType) == "visualgraph"){
      // Get name of the visual graph 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the visual graph. 
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
      VisualGraph* vg = (VisualGraph*)xValue.addr;
      if(vg->g_id == vg_id){
        // This is the dual graph we have been looking for
        return vg;
      }
    }
  }
  return NULL;

}


void Pavement::CloseVisualGraph(VisualGraph* vg)
{
  if(vg == NULL) return; 
  Word xValue;
  xValue.addr = vg;
  SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom( "visualgraph" ),
                                           xValue);
}

Relation* Pavement::GetPaveRel()
{
  if(IsDefined()) return pave_rel;
  else return NULL;
}

/*
check the maximum distance between two lines, if the value is larger than d
return false

*/

bool SmallerD(Line* l1, Line* l2, float d)
{
  if(l1->IsEmpty() || l2->IsEmpty()){
    cout<<"line empty"<<endl;
    return false;
  }

  assert( l1->IsOrdered() );
  assert( l2->IsOrdered() );
  HalfSegment hs1, hs2;
  for( int i = 0; i < l1->Size(); ++i ){
    l1->Get( i, hs1 );
    Point lp1 = hs1.GetLeftPoint();
    Point rp1 = hs1.GetRightPoint();
    if(hs1.IsLeftDomPoint() == false) continue;
      for( int j = 0; j < l2->Size(); ++j ) {
        l2->Get( j, hs2 );
        if(hs2.IsLeftDomPoint() == false) continue;
        Point lp2 = hs2.GetLeftPoint();
        Point rp2 = hs2.GetRightPoint();
        if(lp1.Distance(lp2) > d) return false;
        if(lp1.Distance(rp2) > d) return false;
        if(rp1.Distance(lp2) > d) return false;
        if(rp1.Distance(rp2) > d) return false;
      }
  }

  return true;
}
