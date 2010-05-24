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

/*
Decompose the pavement on one side of the road into a set of subregions

*/

void SpacePartition::DecomposePave(Region* reg1, Region* reg2,
                     vector<Region>& result)
{
    vector<Region> temp_result;

    vector<Region> result1;
    int no_faces = reg1->NoComponents();
    for(int i = 0;i < no_faces;i++){
        Region* temp = new Region(0);

        result1.push_back(*temp);
        delete temp;
        result1[i].StartBulkLoad();
    }
    for(int i = 0;i < reg1->Size();i++){
      HalfSegment hs;
      reg1->Get(i,hs);
      int face = hs.attr.faceno;
      result1[face] += hs;
    }

    for(int i = 0;i < no_faces;i++){
        result1[i].SetNoComponents(1);
        result1[i].EndBulkLoad(false,false,false,false);
        if(result1[i].Size() >= 6)
//          result.push_back(result1[i]);
          temp_result.push_back(result1[i]);
    }


    vector<Region> result2;
    no_faces = reg2->NoComponents();
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
      result2[face] += hs;
    }
    for(int i = 0;i < no_faces;i++){
        result2[i].SetNoComponents(1);
        result2[i].EndBulkLoad(false,false,false,false);
        if(result2[i].Size() >= 6)
//          result.push_back(result2[i]);
          temp_result.push_back(result2[i]);
    }
    //////////////////////////////////////////////////////////

    for(unsigned int i = 0;i < temp_result.size();i++){
        Line* line = new Line(0);
        temp_result[i].Boundary(line);
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
//      Tuple* pave_tuple = rel->GetTuple(i);
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

//      Tuple* pave_tuple = rel->GetTuple(i);
      Tuple* pave_tuple = rel->GetTuple(i, false);
      int rid = ((CcInt*)pave_tuple->GetAttribute(attr_pos1))->GetIntval();

/*      if(!(rid == 1306 || rid == 1626)){
          pave_tuple->DeleteIfAllowed();
          continue;
      }*/

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
Compute the area of a polygon

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
bool CompTriangle::InsideTriangle(float Ax, float Ay,
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
  if(reg->NoComponents() > 1){
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
    cout<<"can't covert the boundary to a sline, maybe there is a hole"<<endl;
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
/*
structure for shortest path searching in a polygon
the graph is build the decomposed triangles.
it finds the path with minimum number of triangles connecting the start point
and the end point

*/
struct Path_elem{
  int prev_index;//previous in expansion list
  int cur_index; //current entry  in expansion list
  int tri_index; //object id
  Path_elem(){}
  Path_elem(int p, int c, int t):prev_index(p), cur_index(c),
                   tri_index(t){}
  Path_elem(const Path_elem& pe):prev_index(pe.prev_index),
                  cur_index(pe.cur_index), tri_index(pe.tri_index){}
  Path_elem& operator=(const Path_elem& pe)
  {
//    cout<<"Path_elem ="<<endl;
    prev_index = pe.prev_index;
    cur_index = pe.cur_index;
    tri_index = pe.tri_index;
    return *this;
  }

};

struct SPath_elem:public Path_elem{
  unsigned int weight;
  SPath_elem(){}
  SPath_elem(int p, int c, int t, int w):Path_elem(p, c, t), weight(w){}
  SPath_elem(const SPath_elem& se):Path_elem(se),
                       weight(se.weight){}
  SPath_elem& operator=(const SPath_elem& se)
  {
//    cout<<"SPath_elem ="<<endl;
    Path_elem::operator=(se);
    weight = se.weight;
    return *this;
  }
  bool operator<(const SPath_elem& se) const
  {
    return weight > se.weight;
  }

  void Print()
  {
    cout<<"prev "<<prev_index<<" cur "<<cur_index
        <<"tri_index" <<tri_index<<"weight "<<weight<<endl;
  }
};


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
  ///////////////construct the path/////////////////////////////
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
The polygon should not have hole inside

*/
void CompTriangle::GeoShortestPath(Point* start, Point* end)
{
    cout<<"GeoShortestPath point to point"<<endl;
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
      cout<<"a convex polygon"<<endl;
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
retrieve the channel/sleeve from a point to a line segment

*/
void CompTriangle::GetChannel(Point* start, HalfSegment* end,
                             vector<Region>& temp_sleeve)
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
      if(triangles[i].Contains(*end)){
        index2 = i;
        break;
      }
  }
  assert(index1 != -1 && index2 != -1);
//  cout<<"index1 "<<index1<<" index2 "<<index2<<endl;

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
  ///////////////construct the path/////////////////////////////
      if(find){
        vector<int> path_record;
        while(dest.prev_index != -1){
            path_record.push_back(dest.tri_index);
            dest = expand_path[dest.prev_index];
        }
        path_record.push_back(dest.tri_index);
        for(int i = path_record.size() - 1;i >= 0;i--)
          temp_sleeve.push_back(triangles[path_record[i]]);

      }

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

/*
geometrical shortest path in a polygon
Apply the Funnel Algorithm, find the shortest path from a point to a line

*/
void CompTriangle::GeoShortestPath(Point* start, Line* end)
{
  cout<<"GeoShortestPath point to sline"<<endl;

  if(start->Inside(*reg) == false || end->Intersects(*reg) == false){
      cout<<"point is not inside the polygon"<<endl;
      cout<<"or line does not intersect the region"<<endl;
      return;
  }
  if(PolygonConvex()){ //convex, just use euclidean distance
      cout<<"a convex polygon"<<endl;
      double dist = numeric_limits<float>::max();
      SpacePartition* sp = new SpacePartition();
      Point end_point;
      for(int i = 0;i < end->Size();i++){
        HalfSegment hs;
        end->Get(i, hs);
        if(!hs.IsLeftDomPoint()) continue;
        Point cp;
        sp->GetClosestPoint(hs, *start, cp);
        assert(cp.IsDefined());
        double d = start->Distance(cp);
        if(d < dist){
          end_point = cp;
          dist = d;
        }
      }
      int edgeno = 0;
      path = new Line(0);
      path->StartBulkLoad();
      HalfSegment hs;
      hs.Set(true, *start, end_point);
      hs.attr.edgeno = edgeno++;
      *path += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *path += hs;
      path->EndBulkLoad();
      sleeve.push_back(*reg);
      delete sp;
      return;
  }
  ////////////////concave polgyon////////////////////////////////////////
  Line* reg_boundary = new Line(0);
  reg->Boundary(reg_boundary);
  if(end->Inside(*reg_boundary) == false){
    cout<<"the end line should be covered by the border of the polygon"<<endl;
    delete reg_boundary;
    assert(false);
  }
  ///////////////////////////////////////////////////////////////////////
//  Triangulation();//get a set of triangles
  NewTriangulation();//get a set of triangles
  vector<Line*> temp_sp; //store all possible shortest path
  double shortest_path_len = numeric_limits<float>::max();
  path = new Line(0);
  for(int i = 0;i < end->Size();i++){
    HalfSegment hs;
    end->Get(i, hs);
    if(!hs.IsLeftDomPoint()) continue;
//    cout<<"i "<<i<<"hs "<<hs<<endl;
    Line* l = new Line(0);
    temp_sp.push_back(l);

    vector<Region> temp_sleeve;
    GetChannel(start, &hs, temp_sleeve);
    PtoSegSPath(start, &hs, temp_sleeve, l);

    if(l->Length() < shortest_path_len){
      shortest_path_len = l->Length();
      sleeve.clear();
      for(unsigned int j = 0;j < temp_sleeve.size();j++)
        sleeve.push_back(temp_sleeve[j]);
      *path = *l;
    }
  }

  for(unsigned int i = 0;i < temp_sp.size();i++)
    delete temp_sp[i];
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
  cout<<"polgyon with "<<no_cyc - 1<<" holes inside "<<endl;


  const int ncontours = no_cyc;
  int no_p_contour[ncontours];

  vector<double> ps_contour_x;
  vector<double> ps_contour_y;

  PolygonContourPoint(no_cyc, no_p_contour, ps_contour_x, ps_contour_y);

/*  for(unsigned int i = 0;i < no_cyc;i++)
    cout<<no_p_contour[i]<<endl;*/


  cout<<"finish creating contour for the polgyon"<<endl;
//  cout<<"no vertices "<<ps_contour_x.size()<<endl;

  ///call the algorithm implemented by Atul Narkhede and Dinesh Manocha ///////
  int result_trig[SEGSIZE][3];
  int (*res_triangles)[3] = &result_trig[0];

  int no_triangle;
  no_triangle = triangulate_polygon(no_cyc, no_p_contour,
                ps_contour_x, ps_contour_y, res_triangles);

  cout<<"no_triangle "<<no_triangle<<endl;

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
/*********************implementation of basgraph********************/
BaseGraph::BaseGraph():dg_id(0),
node_rel(NULL),
edge_rel(NULL),
adj_list(0),
entry_adj_list(0)
{
  cout<<"BaseGraph::BaseGraph()"<<endl;
}

BaseGraph::BaseGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect):dg_id(0),
node_rel(NULL),
edge_rel(NULL),
adj_list(0),
entry_adj_list(0)
{
  cout<<"BaseGraph::BaseGraph(ListExpr)"<<endl;
}

BaseGraph::BaseGraph(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo):
dg_id(0), node_rel(NULL), edge_rel(NULL),
adj_list(0), entry_adj_list(0)
{
  cout<<"BaseGraph::BaseGraph(SmiRecord)"<<endl;
}

BaseGraph::~BaseGraph()
{
    cout<<"~BaseGraph()"<<endl;
    if(node_rel != NULL)
      node_rel->Close();

    if(edge_rel != NULL)
      edge_rel->Close();

//    adj_list.clean();
//    entry_adj_list.clean();

}

void BaseGraph::Destroy()
{
  cout<<"Destroy()"<<endl;
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
    cout<<"BaseGraphProp()"<<endl;
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
  cout<<"CastBaseGraph()"<<endl;
  return 0;
}

Word BaseGraph::CloneBaseGraph(const ListExpr typeInfo, const Word& w)
{
  cout<<"CloneBaseGraph()"<<endl;
  return SetWord(Address(0));
}

int BaseGraph::SizeOfBaseGraph()
{
  cout<<"SizeOfBaseGraph()"<<endl;
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
as long as testing every possible path.*/

/*build a visibility graph, connect the start and end point to the graph*/

string DualGraph::NodeTypeInfo =
  "(rel(tuple((oid int)(rid int)(pavement region))))";
string DualGraph::EdgeTypeInfo =
  "(rel(tuple((oid1 int)(oid2 int)(commarea line))))";
string DualGraph::QueryTypeInfo =
  "(rel(tuple((oid int)(loc point))))";
string DualGraph::TriangleTypeInfo1 =
  "(rel(tuple((v1 int)(v2 int)(v3 int)(centroid point)(oid int))))";
string DualGraph::TriangleTypeInfo2 =
  "(rel(tuple((cycleno int)(vertex point))))";


ListExpr DualGraph::OutDualGraph(ListExpr typeInfo, Word value)
{
  cout<<"OutDualGraph()"<<endl;
  DualGraph* dg = (DualGraph*)value.addr;
  return dg->Out(typeInfo);
}

ListExpr DualGraph::Out(ListExpr typeInfo)
{
  cout<<"Out()"<<endl;
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
      xNext = nl->FourElemList(nl->IntAtom(dg_id),
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

  return nl->TwoElemList(nl->IntAtom(dg_id),xNode);
}

Word DualGraph::InDualGraph(ListExpr in_xTypeInfo, ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
  cout<<"InDualGraph()"<<endl;
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
  cout<<"CreateDualGraph()"<<endl;
  return SetWord(new DualGraph());

}

void DualGraph::CloseDualGraph(const ListExpr typeInfo, Word& w)
{
  cout<<"CloseDualGraph()"<<endl;
  delete static_cast<DualGraph*> (w.addr);
  w.addr = NULL;
}



void DualGraph::DeleteDualGraph(const ListExpr typeInfo, Word& w)
{
  cout<<"DeleteDualGraph()"<<endl;
  DualGraph* dg = (DualGraph*)w.addr;
//  dg->Destroy();
  delete dg;
  w.addr = NULL;
}

bool DualGraph::CheckDualGraph(ListExpr type, ListExpr& errorInfo)
{
  cout<<"CheckDualGraph()"<<endl;
  return nl->IsEqual(type, "dualgraph");
}



bool DualGraph::SaveDualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
  cout<<"SaveDualGraph()"<<endl;
  DualGraph* dg = (DualGraph*)value.addr;
  bool result = dg->Save(valueRecord, offset, typeInfo);

  return result;
}

bool DualGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
  cout<<"Save()"<<endl;
  /********************Save graph id ****************************/
  in_xValueRecord.Write(&dg_id,sizeof(int),inout_iOffset);
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

bool DualGraph::OpenDualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
  cout<<"OpenDualGraph()"<<endl;
  value.addr = DualGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

DualGraph* DualGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
  cout<<"Open()"<<endl;
  return new DualGraph(valueRecord,offset,typeInfo);
}

DualGraph::~DualGraph()
{
  cout<<"~DualGraph()"<<endl;
}

DualGraph::DualGraph()
{
  cout<<"DualGraph::DualGraph()"<<endl;
}

DualGraph::DualGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect)

{
  cout<<"DualGraph::DualGraph(ListExpr)"<<endl;

}

DualGraph::DualGraph(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
   cout<<"DualGraph::DualGraph(SmiRecord)"<<endl;
   /***********************Read graph id********************************/
  in_xValueRecord.Read(&dg_id,sizeof(int),inout_iOffset);
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
  dg_id = id;
  //////////////////node relation////////////////////

  ostringstream xNodePtrStream;
  xNodePtrStream<<(long)r1;
  string strQuery = "(consume(sort(feed(" + NodeTypeInfo +
                "(ptr " + xNodePtrStream.str() + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;

  /////////////////edge relation/////////////////////
  ostringstream xEdgePtrStream;
  xEdgePtrStream<<(long)r2;
  strQuery = "(consume(sort(feed(" + EdgeTypeInfo +
                "(ptr " + xEdgePtrStream.str() + ")))))";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel = (Relation*)xResult.addr;

  ////////////adjacency list ////////////////////////////////

  ostringstream xNodeOidPtrStream1;
  xNodeOidPtrStream1 << (long)edge_rel;
  strQuery = "(createbtree (" + EdgeTypeInfo +
             "(ptr " + xNodeOidPtrStream1.str() + "))" + "oid1)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree_node_oid1 = (BTree*)xResult.addr;


  ostringstream xNodeOidPtrStream2;
  xNodeOidPtrStream2 << (long)edge_rel;
  strQuery = "(createbtree (" + EdgeTypeInfo +
             "(ptr " + xNodeOidPtrStream2.str() + "))" + "oid2)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  BTree* btree_node_oid2 = (BTree*)xResult.addr;


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

    BTreeIterator* btree_iter2 = btree_node_oid2->ExactMatch(nodeid);
    while(btree_iter2->Next()){
      Tuple* edge_tuple = edge_rel->GetTuple(btree_iter2->GetId(), false);
      int oid = ((CcInt*)edge_tuple->GetAttribute(OIDFIRST))->GetIntval();
      adj_list.Append(oid);
      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter2;
    int end = adj_list.Size();
    entry_adj_list.Append(ListEntry(start, end));
//    cout<<"end "<<end<<endl;
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
Computing the shortest path of two points inside polgyon representing pavement

*/

void DualGraph::WalkShortestPath(int oid1, int oid2, Point loc1, Point loc2,
                                 Line* walk_sp, vector<Region>& walkregs)
{
  ////////////////point must be located inside the polygon//////////
  Tuple* tuple1 = node_rel->GetTuple(oid1, false);
  Region* reg1 = (Region*)tuple1->GetAttribute(PAVEMENT);

  if(loc1.Inside(*reg1) == false){
    tuple1->DeleteIfAllowed();
    cout<<"point1 is not inside the polygon"<<endl;
    return;
  }
  Tuple* tuple2 = node_rel->GetTuple(oid2, false);
  Region* reg2 = (Region*)tuple2->GetAttribute(PAVEMENT);

  if(loc2.Inside(*reg2) == false){
    tuple1->DeleteIfAllowed();
    tuple2->DeleteIfAllowed();
    cout<<"point2 is not inside the polygon"<<endl;
    return;
  }
  tuple1->DeleteIfAllowed();
  tuple2->DeleteIfAllowed();
  ///////////////////////////////////////////////////

  int no_node_graph = node_rel->GetNoTuples();
  vector<bool> mark_oid;
  for(int i = 1;i <= no_node_graph;i++){
    mark_oid.push_back(false);
  }

  priority_queue<SPath_elem> path_queue;
  vector<SPath_elem> expand_path;

  path_queue.push(SPath_elem(-1, 0, oid1, 1));
  expand_path.push_back(SPath_elem(-1,0, oid1, 1));
  mark_oid[oid1 - 1] = true;

  bool find = false;
  SPath_elem dest;//////////destination
  if(oid1 != oid2){
    while(path_queue.empty() == false){
      SPath_elem top = path_queue.top();
      path_queue.pop();
  //    top.Print();
      if(top.tri_index == oid2){
         cout<<"find the path"<<endl;
         find = true;
        dest = top;
        break;
      }
      //////find its adjacecy element, and push them into queue and path//////
      vector<int> adj_list;
      FindAdj(top.tri_index, mark_oid, adj_list);

      int pos_expand_path = top.cur_index;
      for(unsigned int i = 0;i < adj_list.size();i++){
        int expand_path_size = expand_path.size();
        path_queue.push(SPath_elem(pos_expand_path, expand_path_size,
                                adj_list[i], top.weight + 1));
        expand_path.push_back(SPath_elem(pos_expand_path, expand_path_size,
                            adj_list[i], top.weight + 1));
      }
    }
  }else{
    find = true;
    dest = SPath_elem(-1, 0, oid1, 1);
  }

  ///////////////construct the path/////////////////////////////
  //////walkregs stores the path consisting of a sequence of regions/triangles
  if(find){
    vector<int> path_record;
    while(dest.prev_index != -1){
      path_record.push_back(dest.tri_index);
      dest = expand_path[dest.prev_index];
    }
    path_record.push_back(dest.tri_index);


    for(int i = path_record.size() - 1;i >= 0;i--){
//      cout<<path_record[i]<<endl;
      Tuple* t = node_rel->GetTuple(path_record[i], false);
      Region* reg = (Region*)t->GetAttribute(PAVEMENT);
      walkregs.push_back(*reg);

      t->DeleteIfAllowed();
    }
  }

}


Walk_SP::Walk_SP()
{
  dg = NULL;
  rel1 = NULL;
  rel2 = NULL;
  count = 0;
  walk_sp = NULL;
  resulttype = NULL;
}
Walk_SP::Walk_SP(const DualGraph* g, const Relation* r1,
const Relation* r2):dg(g), rel1(r1), rel2(r2), count(0),
resulttype(NULL), walk_sp(NULL)
{

}

Walk_SP:: ~Walk_SP()
{
  if(walk_sp != NULL) delete walk_sp;
  if(resulttype != NULL) delete resulttype;
}

void Walk_SP::WalkShortestPath()
{
  cout<<"WalkShortestPath"<<endl;
  if(rel1->GetNoTuples() != 1 || rel2->GetNoTuples() != 1){
    cout<<"input query relation is not correct"<<endl;
    return;
  }
  Tuple* t1 = rel1->GetTuple(1, false);
  int oid1 = ((CcInt*)t1->GetAttribute(DualGraph::QOID))->GetIntval();
  Point* loc1 = new Point(*((Point*)t1->GetAttribute(DualGraph::QLOC)));
  Tuple* t2 = rel2->GetTuple(1, false);
  int oid2 = ((CcInt*)t2->GetAttribute(DualGraph::QOID))->GetIntval();
  Point* loc2 = new Point(*((Point*)t2->GetAttribute(DualGraph::QLOC)));
  cout<<"pave1 "<<oid1<<" pave2 "<<oid2<<endl;
  cout<<"loc1 "<<*loc1<<" loc2 "<<*loc2<<endl;
  int no_node_graph = const_cast<DualGraph*>(dg)->No_Of_Node();
  if(oid1 < 1 || oid1 > no_node_graph){
    cout<<"pave loc1 does not exist"<<endl;
    return;
  }
  if(oid2 < 1 || oid2 > no_node_graph){
    cout<<"pave loc2 does not exist"<<endl;
    return;
  }

  /////////////////////searching path/////////////////////////////
  walk_sp = new Line(0);
  (const_cast<DualGraph*>(dg))->WalkShortestPath(oid1, oid2,
                                *loc1, *loc2, walk_sp, walkregs);

  delete loc1;
  delete loc2;
  t1->DeleteIfAllowed();
  t2->DeleteIfAllowed();
}

/*
Randomly generates points inside pavement polygon

*/

void Walk_SP::GenerateData(int no_p)
{
  int no_node_graph = rel1->GetNoTuples();
  struct timeval tval;
  struct timezone tzone;

  gettimeofday(&tval, &tzone);
  srand48(tval.tv_sec);

  for (int i = 1; i <= no_p; i++){
      int  m = lrand48() % no_node_graph + 1;
//      cout<<"m "<<m<<endl;
      Tuple* tuple = rel1->GetTuple(m, false);
      Region* reg = (Region*)tuple->GetAttribute(DualGraph::PAVEMENT);
      BBox<2> bbox = reg->BoundingBox();
      int xx = (int)(bbox.MaxD(0) - bbox.MinD(0)) + 1;
      int yy = (int)(bbox.MaxD(1) - bbox.MinD(1)) + 1;
//      cout<<"xx "<<xx<<" yy "<<yy<<endl;
      Point p;
      bool inside = false;
      while(inside == false){
        int x = mrand48()% (xx*100);
        int y = mrand48()% (yy*100);
//        printf("x %d, y %d\n", x, y);
        double coord_x = x/100.0;
        double coord_y = y/100.0;
        Coord x_cord = coord_x + bbox.MinD(0);
        Coord y_cord = coord_y + bbox.MinD(1);
        p.Set(x_cord, y_cord);
        inside = p.Inside(*reg);
      }
      oids.push_back(m);
      q_loc.push_back(p);
      tuple->DeleteIfAllowed();
  }
}


VGraph::VGraph()
{
  dg = NULL;
  rel = NULL;
  count = 0;

  resulttype = NULL;
}
VGraph::VGraph(DualGraph* g, Relation* r):dg(g),
rel(r), count(0), resulttype(NULL)
{

}

VGraph:: ~VGraph()
{
  if(resulttype != NULL) delete resulttype;
}

/*
create the edge relation for the visibility graph

*/
void VGraph::GetVGEdge(int attr_pos1, int attr_pos2, Region* all_reg)
{


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

      cout<<"no_triangle "<<no_triangle<<endl;

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
            hs.Set(true, p1, p2);
            Line* l = new Line(0);
            l->StartBulkLoad();
            hs.attr.edgeno = 0;
            *l += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *l += hs;
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
        temp = temp->next;
      }
      assert(false);
  }
  delete head;

}