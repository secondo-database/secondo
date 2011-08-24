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
#include "../Spatial/RegionTools.h"
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
#include "GeneralType.h"
#include <bitset>

///////////////////////graph model for walk planning///////////////////////

static const float EPSILON=0.0000000001f;

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
        <<"tri_index" <<tri_index<<" weight "<<weight<<endl;
  }
};

/*
structure for shortest path searching

*/
struct WPath_elem:public Path_elem{
  double weight;
  Point loc;
  double real_w;
  WPath_elem(){}
  WPath_elem(int p, int c, int t, double w, Point& q,double w2):
                    Path_elem(p, c, t), weight(w), loc(q), real_w(w2){}
  WPath_elem(const WPath_elem& wp):Path_elem(wp),
            weight(wp.weight),loc(wp.loc), real_w(wp.real_w){}
  WPath_elem& operator=(const WPath_elem& wp)
  {
//    cout<<"SPath_elem ="<<endl;
    Path_elem::operator=(wp);
    weight = wp.weight;
    loc = wp.loc;
    real_w = wp.real_w;
    return *this;
  }
  bool operator<(const WPath_elem& wp) const
  {
    return weight > wp.weight;
  }

  void Print()
  {
    cout<<" tri_index" <<tri_index<<" loc "<<loc
        <<" realweight "<<real_w<<" weight "<<weight<<endl;
  }
};

struct RPoint;
struct MyPoint; 
struct MySegDist; 
struct MyHalfSegment; 
struct SpacePartition; 

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
  void PolygonContourPoint2(unsigned int no_cyc, int no_p_contour[],
                           vector<double>&, vector<double>&);
  void NewTriangulation();
  void NewTriangulation2();
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
  bool PolygonConvex2(int& error);
  int ComplexRegion(); 
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
    static string NodeRTreeTypeInfo;

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
    void LoadSortNode(Relation* r1);
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
    void LineIntersectTri(Line*l, vector<Line>& line_list);
    void DFTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                          Line* line, vector<Line>& line_list);
    int GetTriId_OfPoint(Point& loc);
    void DFTraverse2(R_Tree<2,TupleId>* rtree, SmiRecordId adr,
                          Point& loc, vector<int>& tri_oid_list);
    void DFTraverse3(R_Tree<2,TupleId>* rtree, SmiRecordId adr,
                          Point& loc, vector<int>& tri_oid_list, double dist);
    //////////////////////////////////////////////////////////////////////
    //////////calculate a path, for cells in metro route//////////////////
    void Path_Weight(int start, int end, vector<int>& path);
   
    //////////////////////////////////////////////////////////////////
    int min_tri_oid_1; //smaller than the minimum tri oid (minoid - 1)

    Relation* node_rel_sort; ///////sort by bbox, better performance for search
    R_Tree<2,TupleId>* rtree_node;//build an rtree on triangles 

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
  void Load(int, Relation*, Relation*);
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
  
  vector<int> oid_list; 
  vector<int> rid_list;
  vector<Region> reg_list; 
  
  vector<GPoint> gp_list; 
  vector<Point> p_list; 
  
  Relation* rel3; //triangle relation (v1 int)(v2 int)(v3 int)(centroid point)
  Relation* rel4; //vertex relation (vid int)(triid int)
  BTree* btree;

  
  Walk_SP();
  ~Walk_SP();
  Walk_SP(DualGraph* g1, VisualGraph* g2, Relation* r1, Relation* r2);
  void WalkShortestPath(Line* res);
  void WalkShortestPath2(int oid1, int oid2, Point loc1, Point loc2,
                                Line* res);
  bool EuclideanConnect(Point loc1, Point loc2);
  void DFTraverse2(R_Tree<2,TupleId>* rtree, SmiRecordId adr, Line* line, 
                  double& l);

  bool EuclideanConnect2(int oid1, Point loc1, int oid2, Point loc2);

  void TestWalkShortestPath(int, int);
  void GenerateData1(int no_p);
  void GenerateData2(int no_p);
  void GenerateData3(int no_p);
  
  
  void DFTraverse(R_Tree<2,TupleId>* rtree, SmiRecordId adr, Region* reg, 
                  vector<int>& r_id_list);

  void SetPaveRid(R_Tree<2,TupleId>* rtree);
  void PaveLocToGP(Network* n);
  void PaveLocToGPoint(Point* loc, Network* n, vector<int> route_id_list);
  bool PaveLocToGPoint2(Point* loc, Network* n, vector<int> route_id_list);
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
  void TriangulationNew2();
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

// struct MHSNode{
//   MyHalfSegment mhs;
//   MHSNode* next;
//   MHSNode(MyHalfSegment hs, MHSNode* pointer):mhs(hs),next(pointer){}
// };
struct MHSNode; 

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

/////////////////////////////////////////////////////////////////////////////
///////////////////////////search maximum rectangle//////////////////////////
////////////////////////////////////////////////////////////////////////////
Rectangle<2> GetMaxRect(Region*);
Rectangle<2> GetMaxRect2(Region*);
Rectangle<2> RectInTriangle(vector<Point>& ps); 

struct GeomPoint{
    GeomPoint(){}
    GeomPoint(int ptx, int pty){
        x = ptx;
        y = pty;
    }
    GeomPoint(const GeomPoint& gp):x(gp.x), y (gp.y){}
    GeomPoint& operator=(const GeomPoint& gp)
    {
      x = gp.x;
      y = gp.y;
      return *this; 
    }
    int min(int a, int b){
        if(a<=b) return a; else return b;
    }
    int max(int a, int b){
        if (a>=b) return a; else return b;
    }
    void Print()
    {
      cout<<"( "<<x<<" "<<y<<" )"<<endl; 
    }
    int x;
    int y; 
};


struct GeomEdge{
    GeomEdge(){}
    GeomEdge(GeomPoint p, GeomPoint q){
        xmin = p.min(p.x, q.x);
        xmax = p.max(p.x, q.x);
        ymin = p.min(p.y, q.y);
        ymax = p.max(p.y, q.y);
        m = ((double)(q.y-p.y))/((double)(q.x-p.x));
        b = p.y - m*(p.x);
        isTop = p.x > q.x; //edge from right to left (ccw)
        isRight = p.y > q.y; //edge from bottom to top (ccw)
    }
    GeomEdge(const GeomEdge& ge):xmin(ge.xmin), xmax(ge.xmax), ymin(ge.ymin),
    ymax(ge.ymax), m (ge.m), b(ge.b), isTop(ge.isTop), isRight(ge.isRight)
    {}
    GeomEdge& operator=(const GeomEdge& ge)
    {
      xmin = ge.xmin;
      xmax = ge.xmax;
      ymin = ge.ymin;
      ymax = ge.ymax;
      isTop = ge.isTop;
      isRight = ge.isRight;
      m = ge.m;
      b = ge.b; 
      return *this; 
    }

    void Print(){
      cout<<"xmin "<<xmin<<" xmax "<<xmax<<" ymin "<<ymin<<" ymax "<<ymax<<endl;
    }

    int xmin, xmax; /* horiz, +x is right */
    int ymin, ymax; /* vertical, +y is down */
    double m,b; /* y = mx + b */
    bool isTop, isRight; /* position of edge w.r.t. hull */
}; 

////////////// minimum area for a region and a rectangle ///////////////////
//////// the building should be complex so that it coveras a large area/////
 const float mini_reg_area = 1000.0; 
// const float mini_rect_area = 800.0; 
// const float maxi_rect_area = 8000.0; 

const float mini_rect_area = 100.0; 
const float maxi_rect_area = 15000.0; 
const float mini_tri_area = 200.0; 
const float maxi_tri_area = 30000.0;

const float mini_dist_build = 15.0;

struct Build_Rect{
  int reg_id;
  Rectangle<2> rect;
  int poly_id;
  int reg_type;
  int build_type;
  bool init;
  int quadrant;
  Build_Rect(){}
  Build_Rect(int id1, Rectangle<2>& r, int id2, int t):
  reg_id(id1), rect(r), poly_id(id2), reg_type(t){
    build_type = 0, init = false; quadrant = 0;
  }
  Build_Rect(const Build_Rect& br):reg_id(br.reg_id), rect(br.rect),
  poly_id(br.poly_id), reg_type(br.reg_type), build_type(br.build_type),
  init(br.init), quadrant(br.quadrant){}
  void Print()
  {
    cout<<"reg_id "<<reg_id<<" rect "<<rect<<" poly_id "<<poly_id
        <<" reg_type "<<reg_type<<endl;
  }
};

class Building;
class IndoorGraph;

struct MaxRect{

    static string BuildingRectTypeInfo;
    static string RegionElemTypeInfo;
    static string BuildingRectExtTypeInfo; 

    bool fixed;
    int fixedX, fixedY;
    vector<Rectangle<2> > RectList; 

    int status;
    int start, stop; //tangents for iterative convex hull
    int g_xmin, g_xmax, g_ymin, g_ymax;  //position of hull
    int yxmax; //y coord of xmax
    GeomPoint rectp;
    int recth, rectw;
    bool changed;

    int result; 

    vector<GeomPoint> geo_p_list; 
    vector<GeomEdge>  geo_e_list;
    /////////////////////////////////////////////////////////////////////////
    vector<int> reg_id_list; 
    vector<Rectangle<2> > rect_list; 

    vector<Region> reg_list; 
    vector<SimpleLine> sl_list; 

    vector<int> reg_type_list; 

    vector<int> poly_id_list; 

    Relation* rel1; 
    Relation* rel2; 
    BTree* btree;
    vector<Point> sp_list;
    vector<unsigned int> sp_index_list;
    vector<Point> ep_list;

    vector<Point> ep_list2;
    vector<Line> path_list; 

    vector<int> build_id_list;
    vector<GenLoc> genloc_list;
    vector<int> sp_type_list;

    vector<int> build_type_list;
    vector<string> build_type2_list;


    vector<Building*> build_pointer;
    vector<IndoorGraph*> igraph_pointer; 

    unsigned int count;
    TupleType* resulttype;

    enum BuildingRect{REG_ID = 0, GEODATA, POLY_ID, REG_TYPE};
    enum RegionElem{REGID=0, COVAREA}; 
    enum BuildingRect_Ext{REG_ID_EXT = 0, GEODATA_EXT, POLY_ID_EXT, 
                          REG_TYPE_EXT, BUILDING_TYPE, BUILDING_TYPE_2};

    MaxRect() {
      count=0;resulttype = NULL;
    }
    MaxRect(Relation* r):rel1(r){
      count=0;
      resulttype = NULL;
    }

    MaxRect(Relation* r1, Relation* r2, BTree* bt):
    rel1(r1), rel2(r2), btree(bt){
      count=0;
      resulttype = NULL;
    }


    ~MaxRect(){
      if(resulttype != NULL) delete resulttype;
    }

    void Init(){
        fixed = false;
        fixedX = 1;
        fixedY = 1;    
    }
    void SetPoint(vector<GeomPoint>& list); 

   // position of point w.r.t. hull edgesign of twice the area of triangle abc

    bool onLeft(GeomPoint a, GeomPoint b, GeomPoint c);
    bool pointOutside(GeomPoint p);
    bool computeEdgeList(); 
    inline int yIntersect(int xi, GeomEdge e); 
    int xIntersect(int y); 
    GeomEdge findEdge(int x, bool isTop); 
    void computeLargestRectangle(); 
    
    bool IsCycle(SimpleLine* sl);
    void RemoveDirty(int attr1, int attr2);
    void RemoveDirtyRegion(int regid, Region* reg); 


    void ConvexReg(int attr1, int attr2); 

    /////////////////merge several triangles to be convex polygon/////////////
    void MergeTriangle(CompTriangle* ct, int reg_id);
    /////////////whether two triangle have a commone edge////////////////
    bool NeighborTriangle(Region* r1, Region* r2); 
    /////////get the maximum rectangle for each region/////////////////
    void GetRectangle1(int attr1, int attr2, string type);

    ////////////////check whether all coordinates are positive///////////////
    ////////////because the function to get maximum rectangle needs all ////
    /////////////positive coordinates///////////////////////////////////////
    bool ValidRegion(Region* r); 
    ////////build the path between the entrance of the building and pavement///
    bool RegionWithHole(vector<Rectangle<2> >& hole_list, Region* reg);
    void SetStartAndEndPoint(Region* r, vector<Point>& build_sp_list,
                             vector<Point>& build_ep_list);
    void MapToPavement(DualGraph* , Point loc);
    //////////set the building entrance according to the floor plan////////
    void OpenBuilding();
    void OpenIndoorGraph();
    void LoadIndoorPaths(vector< map<int, Line3D> >& paths,
                         vector< map<int, Line3D> >& rooms);

    void CloseIndoorGraph();
    void CloseBuilding();
    void PathToBuilding(Space* gl_sp);
    void CreateEntranceforBuilding(Region* r, vector<int>& tid_list, 
                                   DualGraph* dg); 
    void BuildingEntrance(int graph_type, Rectangle<2>* rect, 
                          vector<Point>& build_sp_list);
    void BuildingEntranceHouse(Rectangle<2>* rect, 
                               vector<Point>& build_sp_list,
                               vector<Point>& build_ep_list, Region* r);
    void Get2DAreadAndDoor(int build_type, Rectangle<2>& build_area, 
                           vector<Point>& door_list);
    void Path_BuildingPave(Point sp, Point ep, Rectangle<2>* rect, 
                                Region* r, DualGraph* dg);
    void PathOnBorder(Line* boundary_temp, Point sp, Point cp_border, 
                      Line* path2, int& edgeno);
    ////////////set the type for each rectange(building)///////////////////
    void SetBuildingType(R_Tree<2,TupleId>* rtree, Space* gl_sp);

    void SetAirPort(vector<Build_Rect>& list, R_Tree<2,TupleId>* rtree);
    bool NoNeighbor(Build_Rect& br, R_Tree<2,TupleId>* rtree);
    void DFTraverse1(R_Tree<2,TupleId>* rtree, SmiRecordId adr, 
                    Rectangle<2>& rect, vector<int>& tri_oid_list);

    void SetTrainStation(vector<Build_Rect>& list);

    void SetCinema(vector<Build_Rect>& build_rect_list, 
                   unsigned int no, Rectangle<2> bbox);
    bool NoNeighborCinema(vector<Build_Rect>& list1, Build_Rect br,
                          Rectangle<2> bbox);
    
    void SetHotel(vector<Build_Rect>& build_rect_list, unsigned int no,
                  Rectangle<2> bbox);
    bool NoNeighborHotel(vector<Build_Rect>& list1, Build_Rect br,
                         Rectangle<2> bbox);

    void SetShopMall(vector<Build_Rect>& build_rect_list, unsigned int no,
                  Rectangle<2> bbox);
    bool NoNeighborShopMall(vector<Build_Rect>& list1, Build_Rect br,
                         Rectangle<2> bbox);

    void SetOffice24(vector<Build_Rect>& build_rect_list, unsigned int no);
    void SetOffice38(vector<Build_Rect>& build_rect_list, unsigned int no);

    void SetHospital(vector<Build_Rect>& build_rect_list, unsigned int no);
    bool NoNeighborHospital(vector<Build_Rect>& list1, Build_Rect br);
    bool NoNearbyShopMallAndCinema(vector<Build_Rect>& list, Build_Rect br);

    void SetLibrary(vector<Build_Rect>& build_rect_list, unsigned int no);
    bool NoNeighborLibrary(vector<Build_Rect>& list, Build_Rect br);
    bool NoNearbyCommercialBuilding(vector<Build_Rect>& list, Build_Rect br);
    
    void SetSchool(vector<Build_Rect>& build_rect_list, unsigned int no);
    bool NoNeighborSchool(vector<Build_Rect>& list, Build_Rect br);
    
    void SetUniversity(vector<Build_Rect>& list, unsigned int no);
    bool NoNeighborUniversity(vector<Build_Rect>& list, Build_Rect br);
    
    
    void SetHouse(vector<Build_Rect>& build_rect_list, unsigned int no);
    bool NoNearbyNeighbors1(vector<Build_Rect>& list, Build_Rect br);
};

bool RegContainRect(Region* reg, Rectangle<2>& rect);

/*
for the infrastructure Region Based Outdoor 

*/

class Pavement{
public:
  Pavement();
  Pavement(bool d, unsigned int i);
  Pavement(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
  
  static string PaveTypeInfo; 
  
  enum PavementTypeInfo{P_OID = 0, P_RID, P_PAVEMENT};

  ~Pavement();
  
  bool IsDefined() const { return def;}
  unsigned int GetId() const {return pave_id;}
  void Load(unsigned int i, Relation* r);
  bool IsDGInit(){return dg_init;}
  bool IsVGInit(){return vg_init;}
  unsigned int GetDGId(){return dg_id;}
  unsigned int GetVGId(){return vg_id;}
  Relation* GetPaveRel();
  void SetDualGraphId(int id);
  void SetVisualGraphId(int id);
  DualGraph* GetDualGraph();
  void CloseDualGraph(DualGraph* dg);
  
  VisualGraph* GetVisualGraph();
  void CloseVisualGraph(VisualGraph* dg);
  
  bool Save(SmiRecord& valueRecord, size_t& offset, const ListExpr typeInfo);
  static Pavement* Open(SmiRecord& valueRecord, size_t& offset, 
                     const ListExpr typeInfo);

  static void* Cast(void* addr);
  
  private:
    bool def; 
    unsigned int pave_id;
    bool dg_init; 
    unsigned int dg_id; 
    bool vg_init; 
    unsigned int vg_id; 

    Relation* pave_rel; 

};
ListExpr PavementProperty();
ListExpr OutPavement( ListExpr typeInfo, Word value ); 
Word InPavement( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );
bool OpenPavement(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value); 
bool SavePavement(SmiRecord& valueRecord, size_t& offset, 
               const ListExpr typeInfo, Word& value);
Word CreatePavement(const ListExpr typeInfo);
void DeletePavement(const ListExpr typeInfo, Word& w);
void ClosePavement( const ListExpr typeInfo, Word& w );
Word ClonePavement( const ListExpr typeInfo, const Word& w ); 
int SizeOfPavement(); 
bool CheckPavement( ListExpr type, ListExpr& errorInfo ); 

#endif
