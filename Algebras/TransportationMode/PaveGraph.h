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

[1] Header File of the PaveGraph. Graph Model for Walk Planning

May, 2010 Jianqiu xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef PaveGraph_H
#define PaveGraph_H


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
#include <bitset>

///////////////////////graph model for walk planning///////////////////////

static const float EPSILON=0.0000000001f;

struct CompTriangle{
  Region* reg;
  vector<Region> triangles;
  vector<Region> sleeve;
  unsigned int count;
  Line* path;
  TupleType* resulttype;
  CompTriangle();
  CompTriangle(Region* r);
  ~CompTriangle();
  void Triangulation();
  unsigned int NoOfCycles();
  void PolygonContourPoint(unsigned int no_cyc, int no_p_contour[],
                           vector<double>&, vector<double>&);
  void NewTriangulation();
  inline bool InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py);
  bool Snip(const vector<Point>& contour,int u,int v,int w,int n,int *V);
  float Area(const vector<Point>& contour);

  bool GetTriangles(const vector<Point>& contour,vector<Point>& result);
  //detect whether a polygon is a convex or concave
  bool IsConvex(vector<Point>);
  bool PolygonConvex();
  //compute the shortest path between two points inside a polgyon
  void GeoShortestPath(Point*, Point*);
  //compute the shortest path between a point and a line inside a polgyon
  void GeoShortestPath(Point*, Line*);
  //compute the channel/sleeve between two points
  void GetChannel(Point*, Point*);
  //compute the channel/sleeve between a point and a line segment
  void GetChannel(Point*, HalfSegment*, vector<Region>&);

  void PtoSegSPath(Point*, HalfSegment*, vector<Region>&, Line*);
  //find adjacenct triangles
  void FindAdj(unsigned int, vector<bool>&, vector<int>&);

  void ConstructConvexChannel1(list<MyPoint>&, list<MyPoint>&, Point&,
                              vector<Point>&, bool);
  void ConstructConvexChannel2(list<MyPoint>, list<MyPoint>, Point&,
                              vector<Point>&, bool);
  void SelectPointOnSeg(list<MyPoint>, list<MyPoint>, HalfSegment*,
                        Point&, Point&);
};

struct ListEntry{
  ListEntry(){} //do not initialize the members
  ListEntry(int l, int h):low(l),high(h){}
  ListEntry(const ListEntry& le):low(le.low), high(le.high){}
  int low, high;
};
class BaseGraph{
public:
    BaseGraph();
    BaseGraph(ListExpr in_xValue,int in_iErrorPos,ListExpr& inout_xErrorInfo,
              bool& inout_bCorrect);
    BaseGraph(SmiRecord&, size_t&, const ListExpr);
    ~BaseGraph();
    void Destroy();
    static ListExpr BaseGraphProp();
    static int SizeOfBaseGraph();
    static void* CastBaseGraph(void* addr);
    static Word CloneBaseGraph(const ListExpr typeInfo, const Word& w);
    ///////////////////////////////////////////////////////////////////
    Relation* GetEdgeRel(){return edge_rel;}
    Relation* GetNodeRel(){return node_rel;}
    inline int No_Of_Node(){return node_rel->GetNoTuples();}
    void FindAdj(int node_id, vector<bool>& flag, vector<int>& adj_list);
    void FindAdj(int node_id, vector<int>& adj_list);

    unsigned int dg_id;
    Relation* node_rel;
    Relation* edge_rel;

    DbArray<int> adj_list;
    DbArray<ListEntry> entry_adj_list;
};

class DualGraph:public BaseGraph{
public:
    static string NodeTypeInfo;
    static string EdgeTypeInfo;
    static string QueryTypeInfo;
    /*schema for edge and node*/
    enum DGNodeTypeInfo{OID = 0, RID, PAVEMENT};
    enum DGEdgeTypeInfo{OIDFIRST = 0, OIDSECOND, COMMAREA};
    enum DGQueryTypeInfo{QOID = 0, QLOC1, QLOC2}; //relative, absolute position

    /////////////for triangle ///////////////////////
    static string TriangleTypeInfo1;
    static string TriangleTypeInfo2;
    static string TriangleTypeInfo3;
    enum Tri1TypeInfo{V1 = 0, V2,V3,CENTROID,TOID};
    enum Tri2TypeInfo{CYCLENO = 0, VERTEX};
    ////////////constructor and deconstructor///////////////////////
    ~DualGraph();
    DualGraph();
    DualGraph(ListExpr in_xValue,int in_iErrorPos,ListExpr& inout_xErrorInfo,
              bool& inout_bCorrect);
    DualGraph(SmiRecord&, size_t&, const ListExpr);
    ///////////////function for typeconstructor////////////////////////
    void Load(int, Relation*,Relation*);
    static ListExpr OutDualGraph(ListExpr typeInfo, Word value);
    ListExpr Out(ListExpr typeInfo);
    static Word InDualGraph(ListExpr in_xTypeInfo, ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect);
    static Word CreateDualGraph(const ListExpr typeInfo);
    static void CloseDualGraph(const ListExpr typeInfo, Word& w);

    static void DeleteDualGraph(const ListExpr typeInfo, Word& w);
    static bool CheckDualGraph(ListExpr type, ListExpr& errorInfo);

    static bool SaveDualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
    bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo);

    static bool OpenDualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
    static DualGraph* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);
    //////////////////////////////////////////////////////////////////
    void WalkShortestPath(int,int,Point,Point,Line*, vector<Region>&);

};

class VisualGraph: public BaseGraph{
public:
  static string NodeTypeInfo;
  static string EdgeTypeInfo;
  enum VGNodeTypeInfo{OID = 0, LOC};
  enum VGEdgeTypeInfo{OIDFIRST = 0,OIDSECOND};

};

struct Walk_SP{
  const DualGraph* dg;
  const Relation* rel1;
  const Relation* rel2;
  unsigned int count;
  TupleType* resulttype;
  Line* walk_sp;
  vector<Region> walkregs;
  vector<int> oids;
  vector<Point> q_loc1;
  vector<Point> q_loc2;
  Walk_SP();
  ~Walk_SP();
  Walk_SP(const DualGraph* g, const Relation* r1, const Relation* r2);
  void WalkShortestPath();
  void GenerateData(int no_p);
};

struct PairPoint{
  Point p1;
  Point p2;
  PairPoint(){}
  PairPoint(Point& q1, Point& q2):p1(q1),p2(q2){}
  PairPoint(const PairPoint& pp):p1(pp.p1),p2(pp.p2){}
  PairPoint& operator=(const PairPoint& pp)
  {
    p1 = pp.p1;
    p2 = pp.p2;
    return *this;
  }
};

struct Clamp{
  Point apex;
  Point foot1;
  Point foot2;
  double angle;
  Clamp(){}
  Clamp(Point& p1, Point& p2, Point& p3):apex(p1),foot1(p2),foot2(p3)
  {

      double b = apex.Distance(foot1);
      double c = apex.Distance(foot2);
      double a = foot1.Distance(foot2);
      assert(AlmostEqual(b*c,0.0) == false);
      double value = (b*b+c*c-a*a)/(2*b*c);

      if(AlmostEqual(value,-1.0)) value = -1;
      if(AlmostEqual(value,1.0)) value = 1;
      angle = acos(value);
//      cout<<"angle "<<angle<<" degree "<<angle*180.0/pi<<endl;
      assert(0.0 <= angle && angle <= 3.1416);
  }

  Clamp(const Clamp& clamp):apex(clamp.apex),
                            foot1(clamp.foot1),foot2(clamp.foot2),
                            angle(clamp.angle){}
  Clamp& operator=(const Clamp& clamp)
  {
    apex = clamp.apex;
    foot1 = clamp.foot1;
    foot2 = clamp.foot2;
    angle = clamp.angle;
    return *this;
  }
  void Print()
  {
    cout<<"apex "<<apex<<" foot1 "<<foot1<<" foot2 "<<foot2<<endl;
  }
};

struct VGraph{
  DualGraph* dg;
  Relation* rel1;
  Relation* rel2;
  Relation* rel3;
  unsigned int count;
  TupleType* resulttype;
  vector<int> oids1;
  vector<PairPoint> vpp;
  vector<int> oids2;
  vector<Point> p_list;
  vector<Line> line;
  vector<Region> regs;
  VGraph();
  ~VGraph();
  VGraph(DualGraph* g, Relation* r1, Relation* r2, Relation* r3);
  void GetVGEdge(int attr_pos1, int attr_pos2, Region*);
  void GetVNode();
  void GetAdjNode(int oid);
  void GetVisibleNode(int tri_id, Point* query_p);
  bool CheckVisibility1(Clamp& clamp, Point& checkp, int vp);
  bool CheckVisibility2(Clamp& clamp, Point& checkp1, Point& checkp2);
  void DFTraverse(int id, Clamp& clamp, int pre_id, int type,
                  vector<int> reg_id_list);
  bool PathContainHS(vector<int> tri_list, HalfSegment hs);
  bool GetIntersectionPoint(Point& p1,Point& p2,Clamp& clamp, Point& ip,bool);

};

/*
structure used for creating the triangles of a polygon in such a way that two
relations work together. we assign a unique number for each contour point
rel1. store the relation for the points of the contour
rel2. store the number of each point and the centroid

*/
struct RegVertex{
  Region* reg;
  unsigned int count;
  TupleType* resulttype;
  vector<int> cycleno;
  vector<Point> regnodes;

  vector<int> v1_list;
  vector<int> v2_list;
  vector<int> v3_list;
  Relation* rel1;
  Relation* rel2;
  vector<Line> line;

  RegVertex(){}
  RegVertex(Region* r):reg(r), count(0), resulttype(NULL){}
  RegVertex(Relation* r1, Relation* r2): count(0), resulttype(NULL),
                                         rel1(r1), rel2(r2){}

  ~RegVertex(){
    if(resulttype != NULL) delete resulttype;
  }
  void CreateVertex();
  void TriangulationNew();
  void GetDGEdge();
};

struct Triangle{
  int oid;
  int v1, v2, v3;
  int c1, c2, c3;
  int neighbor_no;//maximum 3
  Triangle(){}
  Triangle(int n1, int n2, int n3):oid(0),v1(n1),v2(n2),v3(n3),
                                   c1(0),c2(0),c3(0),neighbor_no(0){}
  Triangle(int id, int n1, int n2, int n3, int cyc1, int cyc2, int cyc3):
  oid(id),v1(n1),v2(n2),v3(n3),c1(cyc1),c2(cyc2),c3(cyc3),neighbor_no(0)
  {

  }
  Triangle(const Triangle& tri):oid(tri.oid),v1(tri.v1),v2(tri.v2),v3(tri.v3),
                                c1(tri.c1),c2(tri.c2),c3(tri.c3),
                                neighbor_no(tri.neighbor_no){}
  Triangle& operator=(const Triangle& tri)
  {
    oid = tri.oid;
    v1 = tri.v1;
    v2 = tri.v2;
    v3 = tri.v3;
    c1 = tri.c1;
    c2 = tri.c2;
    c3 = tri.c3;
    neighbor_no = tri.neighbor_no;
    return *this;
  }

  int ShareEdge(Triangle& tri)
  {
    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;
    if(v1 == tri.v1 || v1 == tri.v2 || v1 == tri.v3)
        flag1 = true;
    if(v2 == tri.v1 || v2 == tri.v2 || v2 == tri.v3)
        flag2 = true;
    if(v3 == tri.v1 || v3 == tri.v2 || v3 == tri.v3)
        flag3 = true;
    if(flag1 && flag2) return 1; //v1 v2
    if(flag1 && flag3) return 2; //v1 v3
    if(flag2 && flag3) return 3; //v2 v3
    return 0;
  }
  void Print()
  {
    cout<<"oid "<<oid<<" "<<
        v1<<" "<<v2<<" "<<v3<<" neighbor "<<neighbor_no<<endl;
    cout<<"cycleno "<<c1<<" "<<c2<<" "<<c3<<endl;
  }

};
struct TriNode{
  Triangle tri;
  TriNode* next;
  TriNode(){next = NULL;}
  TriNode(Triangle& t, TriNode* n):tri(t), next(n){}
};

/*
Calculate the Z-order value for the input point

*/
inline long ZValue(Point& p)
{
  bitset<20> b;
  double base = 2,exp = 20;
  int x = (int)p.GetX();
  int y = (int)p.GetY();
  assert (x < pow(base,exp));
  assert (y < pow(base,exp));
  bitset<10> b1(x);
  bitset<10> b2(y);
  bool val;
  b.reset();
  for(int j = 0; j < 10;j++){
      val = b1[j];
      b.set(2*j,val);
      val = b2[j];
      b.set(2*j+1,val);
  }
  return b.to_ulong();
}
#endif
