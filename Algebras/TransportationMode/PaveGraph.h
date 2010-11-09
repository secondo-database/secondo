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

struct RPoint;
struct CompTriangle{
  Region* reg;
  vector<Region> triangles;
  vector<Region> sleeve;
  vector<Point> plist1;
  vector<Point> plist2;
  vector<Point> plist3;
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
  bool GetClockwise(const vector<Point>& contour);
  float Area(const vector<Point>& contour);

  bool GetTriangles(const vector<Point>& contour,vector<Point>& result);
  //detect whether a polygon is a convex or concave
  bool IsConvex(vector<Point>);
  bool PolygonConvex();
  //compute the shortest path between two points inside a polgyon
  void GeoShortestPath(Point*, Point*);

  //compute the channel/sleeve between two points
  void GetChannel(Point*, Point*);

  void PtoSegSPath(Point*, HalfSegment*, vector<Region>&, Line*);
  //find adjacenct triangles
  void FindAdj(unsigned int, vector<bool>&, vector<int>&);

  void ConstructConvexChannel1(list<MyPoint>&, list<MyPoint>&, Point&,
                              vector<Point>&, bool);
  void ConstructConvexChannel2(list<MyPoint>, list<MyPoint>, Point&,
                              vector<Point>&, bool);
  void SelectPointOnSeg(list<MyPoint>, list<MyPoint>, HalfSegment*,
                        Point&, Point&);
  //////////////////for rotational plane sweep ///////////////////////
  static string AllPointsInfo;
  enum AllPointsTypeInfo{V=0, NEIGHBOR1, NEIGHBOR2, REGID};
  void GetAllPoints();
  void GetVPoints(Relation* r1, Relation* r2, Rectangle<2>* bbox,
                  Relation* r3, int attr_pos);
/*  void ProcessNeighbor(multiset<MySegDist>&, RPoint&, Point&,
                      Point&, Point&, SpacePartition*, ofstream& );*/

  void ProcessNeighbor(multiset<MySegDist>&, RPoint&, Point&,
                      Point&, Point&, SpacePartition*);

  void InitializeAVL( multiset<MySegDist>& sss,
                                      Point& query_p, Point& hp,
                                      Point& p, Point& neighbor,
                                      SpacePartition* sp);
  void InitializeQueue(priority_queue<RPoint>& allps,
                                   Point& query_p, Point& hp, Point& p,
                                   SpacePartition* sp, Point* q1,
                                   Point* q2, int id);
  void PrintAVLTree(multiset<MySegDist>& sss, ofstream&);
  vector<Line> connection;
  vector<int> reg_id;
//  vector<float> angles;
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
    unsigned int g_id;
    Relation* node_rel;
    Relation* edge_rel;

    DbArray<int> adj_list;
    DbArray<ListEntry> entry_adj_list;
};

class DualGraph:public BaseGraph{
public:
    static string NodeTypeInfo;
    static string EdgeTypeInfo;

    /*schema for edge and node*/
    enum DGNodeTypeInfo{OID = 0, RID, PAVEMENT};
    enum DGEdgeTypeInfo{OIDFIRST = 0, OIDSECOND, COMMAREA};


    /////////////for triangle ///////////////////////
    static string TriangleTypeInfo1;
    static string TriangleTypeInfo2;
    static string TriangleTypeInfo3;
    static string TriangleTypeInfo4;
    enum Tri1TypeInfo{V1 = 0, V2,V3,CENTROID,TOID};
    enum Tri2TypeInfo{CYCLENO = 0, VERTEX};
    //(vid,triid)
    //for each vertex, which triangle it belongs to
    enum Tri4TypeInfo{VID = 0, TRIID};
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


};

class VisualGraph: public BaseGraph{
public:
  static string NodeTypeInfo;
  static string EdgeTypeInfo;
  static string QueryTypeInfo;
  enum VGNodeTypeInfo{OID = 0, LOC};
  enum VGEdgeTypeInfo{OIDFIRST = 0,OIDSECOND, CONNECTION};
  enum VGQueryTypeInfo{QOID = 0, QLOC1, QLOC2}; //relative, absolute position
  //////////////////////////////////////////////////////////////
  ~VisualGraph();
  VisualGraph();
  VisualGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect);
  VisualGraph(SmiRecord&, size_t&, const ListExpr);
  //////////////////////////////////////////////////////////////
  void Load(int, Relation*,Relation*);
  static ListExpr OutVisualGraph(ListExpr typeInfo, Word value);
  ListExpr Out(ListExpr typeInfo);
  static bool CheckVisualGraph(ListExpr type, ListExpr& errorInfo);
  static void CloseVisualGraph(const ListExpr typeInfo, Word& w);
  static void DeleteVisualGraph(const ListExpr typeInfo, Word& w);
  static Word CreateVisualGraph(const ListExpr typeInfo);
  static Word InVisualGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect);
  static bool OpenVisualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
  static VisualGraph* Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo);
  static bool SaveVisualGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value);
  bool Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo);
};

struct Walk_SP{
  DualGraph* dg;
  VisualGraph* vg;
  Relation* rel1; //query relation1
  Relation* rel2; //query relation2
  unsigned int count;
  TupleType* resulttype;
  vector<int> oids;

  vector<int> oids1;
  vector<int> oids2;
  vector<Line> path;

  vector<Point> q_loc1;
  vector<Point> q_loc2;
  Relation* rel3; //triangle relation (v1 int)(v2 int)(v3 int)(centroid point)
  Relation* rel4; //vertex relation (vid int)(triid int)
  BTree* btree;

  Walk_SP();
  ~Walk_SP();
  Walk_SP(DualGraph* g1, VisualGraph* g2, Relation* r1, Relation* r2);
  void WalkShortestPath();
  void GenerateData1(int no_p);
  void GenerateData2(int no_p);
  void GenerateData3(int no_p);
};

/*
clamp structure for pruning triangles searching

*/
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
    cout<<"apex "<<apex<<" foot1 "<<foot1<<" foot2 "
        <<foot2<<"angle "<<angle<<endl;
  }
};

struct Triangle;

struct VGraph{
  DualGraph* dg;
  Relation* rel1;//query relation
  Relation* rel2;//triangle relation (v1 int)(v2 int)(v3 int)(centroid point)
  Relation* rel3;//visibility graph node relation
  Relation* rel4;//vertex relation (vid int)(triid int)
  BTree* btree;
  unsigned int count;
  TupleType* resulttype;
  vector<int> oids1;

  vector<int> oids2;
  vector<int> oids3;
  vector<Point> p_list;
  vector<Line> line;
  vector<Region> regs;
  VisualGraph* vg;

  VGraph();
  ~VGraph();
  VGraph(DualGraph* g, Relation* r1, Relation* r2, Relation* r3);
  VGraph(VisualGraph* g);
  void GetVNode();
  void GetAdjNodeDG(int oid);
  void GetAdjNodeVG(int oid);
  void GetVisibleNode1(int tri_id, Point* query_p);
  void GetVisibleNode2(int tri_id, Point* query_p, int type);
  bool CheckVisibility1(Clamp& clamp, Point& checkp, int vp);
  bool CheckVisibility2(Clamp& clamp, Point& checkp1, Point& checkp2);
  void DFTraverse(int id, Clamp& clamp, int pre_id, int type);
  bool PathContainHS(vector<int> tri_list, HalfSegment hs);
  bool GetIntersectionPoint(Point& p1,Point& p2,Clamp& clamp, Point& ip,bool);
  bool GetVNode_QV(int tri_id, Point* query_p,int,int,int);
  void DecomposeTriangle();
  void FindTriContainVertex(int vid, int tri_id, Point* query_p);
  bool Collineation(Point& p1, Point& p2, Point& p3);
  void GetVNodeOnVertex(int vid, Point* query_p);
  void GetVGEdge();
  bool MyCross(const HalfSegment& hs1, const HalfSegment& hs2);
  ////////////////////for walk shortest algorithm/////////////////////////
  void GetVisibilityNode(int tri_id, Point query_p);
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
  void GetDGEdgeRTree(R_Tree<2,TupleId>*);
  void ShareEdge(Region* reg1, Region* reg2, int, int,vector<vector<int> >&);
  void DFTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr,
                  int oid, Region* reg,
                  vector<vector<int> >& adj_node);
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

inline void Modify_Point_3(Point& p)
{
    double x,y;
    x = p.GetX();
    y = p.GetY();
//    printf("%.10f %.10f\n",x, y);
    x = ((int)(x*1000.0 + 0.5))/1000.0;
    y = ((int)(y*1000.0 + 0.5))/1000.0;

//    printf("%.10f %.10f\n",x, y);

    p.Set(x,y);
}

struct MHSNode{
  MyHalfSegment mhs;
  MHSNode* next;
  MHSNode(MyHalfSegment hs, MHSNode* pointer):mhs(hs),next(pointer){}
};

struct Hole{
  Hole(char* input):in(input){count = 0;resulttype=NULL;}
  Hole(){count=0;resulttype = NULL;}
  ~Hole(){if(resulttype != NULL) delete resulttype;}
  ifstream in;
  unsigned int count;
  TupleType* resulttype;
  vector<Region> regs1;
  vector<Region> regs2;
  vector<Region> regs;
  void GetContour();
  void GetContour(unsigned int no_reg);
  void GetPolygon(int no_ps);//create a polygon
  void SpacePartitioning(Points* gen_ps, vector<HalfSegment>& hs_segs);
  void SpacePartitioning(vector<Point> ps, vector<HalfSegment>& hs_segs,
                         Point sf, Point sl);
  void DiscoverContour(MHSNode* head, Region* r);
  void DiscoverContour(Points* ps, Region* r);
  bool NoSelfIntersects(Region* r);
  void GetHole(Region* r);
};
#endif
