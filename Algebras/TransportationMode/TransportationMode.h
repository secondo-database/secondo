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

August, 2009 Jianqiu Xu
March, 2010 Jianqiu xu
June, 2012 Jianqiu Xu

[TOC]

1 Overview

2 Defines and includes

*/

#ifndef TransportationMode_H
#define TransportationMode_H


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
#include "FTextAlgebra.h"
#include <fstream>
#include "GeneralType.h"
#include "Indoor.h"
#include "BusNetwork.h"
#include "PaveGraph.h"
#include "RoadNetwork.h"
#include "TMRTree.h"

double TM_DiffTimeb(struct timeb* t1, struct timeb* t2);

/*
check whether the building id has been used already 

*/
bool ChekBuildingId(int build_id)
{
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
       nl->SymbolValue(xObjectType) == "building")
    {
      // Get name of the pavement 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the pavement. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      Building* build = (Building*)xValue.addr;

      if(build->GetId() == build_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("building"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("building"),
                                               xValue);
    }
  }
  return true; 
}


/*
check whether the building id has been used already 

*/
bool ChekIndoorId(int build_id)
{
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
       nl->SymbolValue(xObjectType) == "indoorinfra")
    {
      // Get name of the pavement 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the pavement. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      IndoorInfra* indoor = (IndoorInfra*)xValue.addr;

      if(indoor->GetId() == build_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("indoorinfra"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("indoorinfra"),
                                               xValue);
    }
  }
  return true; 
}

/*
check whether the metro graph id has been used already 

*/

bool CheckIndoorGraphId(unsigned int ig_id)
{
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
       nl->SymbolValue(xObjectType) == "indoorgraph")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      IndoorGraph* ig = (IndoorGraph*)xValue.addr;

      if(ig->g_id == ig_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("indoorgraph"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("indoorgraph"),
                                               xValue);
    }
  }
  return true; 
}

/////////////////////////////////////////////////////////////////////
////////////// string descritpion for operators /////////////////////
////////////////////////////////////////////////////////////////////

////////////string for Operator Spec //////////////////////////////////
const string OpTMCheckSlineSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>sline-> sline</text--->"
    "<text>checksline(sline,int)</text--->"
    "<text>correct dirty route line </text--->"
    "<text>query routes(n) feed extend[newcurve: checksline(.curve,2)] count"
    "</text--->"
    ") )";
const string OpTMModifyBoundarySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rectangle x int -> region</text--->"
    "<text>modifyboundary(rectangle,2)</text--->"
    "<text>extend the boundary of road network by a small value</text--->"
    "<text>query modifyboundary(bbox(rtreeroad),2)"
    "</text--->"
    ") )";

const string OpTMSegment2RegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>relation x attr_name x int->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>segment2region(rel,attr, int)</text--->"
    "<text>extend the halfsegment to a small region </text--->"
    "<text>query segment2region(allroutes,curve,2) count"
    "</text--->"
    ") )";

const string OpTMPaveRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x attr x rel2 x attr1 x attr2 x int->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>paveregion(n,rel1,attr,rel2,attr1,attr2,int)</text--->"
    "<text>cut the intersection region between road and pavement</text--->"
    "<text>query paveregion(n,allregions_in,inborder, allregions_pave"
    ",pave1, pave2, roadwidth) count;</text--->"
    ") )";

const string OpTMJunRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x int->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>junregion(n,rel,attr1,attr2,int)</text--->"
    "<text>get the pavement region (zebra crossing) at junctions</text--->"
    "<text>query junregion(n,allregions,inborder,outborder,roadwidth) count;"
    "</text--->"
    ") )";

const string OpTMDecomposeRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>decomposeregion(region)</text--->"
    "<text>decompose a region by its faces</text--->"
    "<text>query decomposeregion(partition_regions) count; </text--->"
    ") )";

const string OpTMFillPavementSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x int->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>fillpavement(n,rel,attr1,attr2,int)</text--->"
    "<text>fill the hole between pavements at junction</text--->"
    "<text>query fillpavement(n, allregions_pave, pave1, pave2, 2)"
    "count;</text--->"
    ") )";

const string OpTMGetPaveNode1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpavenode1(network, rel, attr1, attr2, attr3)</text--->"
    "<text>decompose the pavements of one road into a set of subregions"
    "</text--->"
    "<text>query getpavenode1(n, pave_regions1, oid, pavement1,pavement2);"
    "</text--->"
    ") )";

const string OpTMGetPaveEdge1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x btree x attr1 x attr2 x attr3->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpaveedge1(network, rel, btree, attr1, attr2 , attr3)</text--->"
    "<text>get the commone area of two pavements</text--->"
    "<text>query getpaveedge1(n, subpaves, btree_pave,oid, rid ,pavement);"
    "</text--->"
    ") )";


const string OpTMGetPaveNode2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x attr1 x attr2->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpavenode2(int, rel, attr1, attr2)</text--->"
    "<text>decompose the zebra crossings into a set of subregions"
    "</text--->"
    "<text>query getpavenode2(subpaves count, pave_regions2, rid, crossreg)"
    " count; </text--->"
    ") )";


const string OpTMGetPaveEdge2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree x attr1 x attr2 x attr3->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpaveedge2(rel1, rel2, btree, attr1, attr2, attr3)</text--->"
    "<text>get the commone area between zc and pave</text--->"
    "<text>query getpaveedge2(subpaves2, subpaves,"
    "btree_pave, oid, rid , pavement) count; </text--->"
    ") )";

const string OpTMTriangulateSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region ->(stream ( (x1 t1)(x2 t2)...(xn tn)) </text--->"
    "<text>triangulate(region)</text--->"
    "<text>decompose a polygon into a set of triangles</text--->"
    "<text>query triangulation(r1) count; </text--->"
    ") )";

const string OpTMTriangulate2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region ->(stream ( (x1 t1)(x2 t2)...(xn tn)) </text--->"
    "<text>triangulate2(region)</text--->"
    "<text>decompose a polygon into a set of triangles</text--->"
    "<text>query triangulation2(r1) count; </text--->"
    ") )";
    
const string OpTMConvexSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region -> bool </text--->"
    "<text>convex(region)</text--->"
    "<text>detect whether a polygon is convex or concave</text--->"
    "<text>query convex(r1); </text--->"
    ") )";

const string OpTMGeospathSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>point x point x region -> "
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>geospath(point, point, region)</text--->"
    "<text>return the geometric shortest path for two points indie a polygon"
    "</text--->"
    "<text>query geospath(p1, p2, r1); </text--->"
    ") )";

const string OpTMCreateDGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x rel -> dualgraph</text--->"
    "<text>createdualgraph(int, rel, rel)</text--->"
    "<text>create a dual graph by the input edge and node relation</text--->"
    "<text>query createdualgraph(1, edge-rel, node-rel); </text--->"
    ") )";

const string OpTMWalkSPOldSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x visualgraph x rel1 x rel2 x rel3-> line</text--->"
    "<text>walk_sp_old(dg1, vg1, rel, rel, rel)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp_old(dg1, vg1, query_loc1, query_loc2,tri_reg_new);"
    "</text--->"
    ") )";

const string OpTMWalkSPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>pavement x rel1 x rel2 x rel3-> line</text--->"
    "<text>walk_sp(pn, rel, rel, rel)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp(pn, query_loc1, query_loc2,tri_reg_new);"
    "</text--->"
    ") )";

const string OpTMWalkSPDebugSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>pavement x rel1 x rel2 x rel3-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>walk_sp_debug(pn, rel, rel, rel)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp_debug(pn, query_loc1, query_loc2, tri_reg_new);"
    "</text--->) )";

const string OpTMTestWalkSPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x visualgraph x rel1 x rel2 x rel3 -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>test_walk_sp(dualgraph, visibilitygraph, rel, rel, rel)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query test_walk_sp(dg1, vg1, query_loc1, query_loc2,tri_reg_new);"
    "</text--->"
    ") )";

const string OpTMPaveLocToGPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree x network-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>pave_loc_togp(rel, rel, btree, network)</text--->"
    "<text>map points in pavements to gpoints</text--->"
    "<text>query pave_loc_togp(query_loc1, dg_node, btree_dg, n)"
     " count; </text--->"
    ") )";

const string OpTMSetPaveRidSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x rtree-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>setpave_rid(rel1, rel2, rtree)</text--->"
    "<text>set rid for each pavement</text--->"
    "<text>query setpave_rid(dg_node, graph_node, rtree_pave) "
     " count; </text--->"
    ") )";

const string OpTMGenerateWPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x int-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>generate_wp(rel, int)</text--->"
    "<text>generate random points inside the polygon/triangle</text--->"
    "<text>query generate_wp(graph_node,5); </text--->"
    ") )";

const string OpTMZvalSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>point -> int</text--->"
    "<text>zval(point)</text--->"
    "<text>calculate the z-order value of a point</text--->"
    "<text>query zval(p1); </text--->"
    ") )";

const string OpTMZcurveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>zcurve(rel, attr)</text--->"
    "<text>calculate the curve of the given points sortby z-order</text--->"
    "<text>query zcurve(vg_node,elem); </text--->"
    ") )";

const string OpTMRegVertexSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>reg ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>regvertex(region)</text--->"
    "<text>return the vertex of the region as well as the cycleno</text--->"
    "<text>query regvertex(node_reg); </text--->"
    ") )";

const string OpTMTriangulationNewSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>reg ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>triangulation_new(region)</text--->"
    "<text>decompose the region into a set of triangles where each is"
    "represented by the three points</text--->"
    "<text>query triangulation_new(r1) count; </text--->"
    ") )";

const string OpTMTriangulationExtSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>reg ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>triangulation_ext(region)</text--->"
    "<text>decompose the region into a set of triangles where each is"
    "represented by the three points</text--->"
    "<text>query triangulation_ext(r1) count; </text--->"
    ") )";

const string OpTMTriangulationNew2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>reg ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>triangulation_new2(region)</text--->"
    "<text>decompose the region into a set of triangles where each is"
    "represented by the three points</text--->"
    "<text>query triangulation_new2(r1) count; </text--->"
    ") )";

const string OpTMTriangulationExt2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>reg ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>triangulation_ext2(region)</text--->"
    "<text>decompose the region into a set of triangles where each is"
    "represented by the three points</text--->"
    "<text>query triangulation_ext2(r1) count; </text--->"
    ") )";

const string OpTMGetDGEdgeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>get_dg_edge(rel,rel)</text--->"
    "<text>get the edge relation for the dual graph on the triangles</text--->"
    "<text>query get_dg_edge(rel1,rel2) count; </text--->"
    ") )";
const string OpTMSMCDGTESpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rtree ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>smcdgte(rel, rtree)</text--->"
    "<text>get the edge relation for the dual graph on the triangles</text--->"
    "<text>query smcdgte(dg_node, rtree_dg) count; </text--->"
    ") )";
const string OpTMGetVNodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x rel1 x rel2 x rel3 x rel4 x btree->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getvnode(dualgraph, rel1, rel2, rel3, rel4, btree)</text--->"
    "<text>for a given point, it finds all its visible nodes</text--->"
    "<text>query getvnode(dg1, query_loc1, tri_reg_new_sort, vgnodes,"
    "vertex_tri, btr_vid) count;</text--->) )";

const string OpTMGetVGEdgeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x rel1 x rel2 x rel3 x btree->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getvgedge(dualgraph, rel1, rel2, rel3, btree)</text--->"
    "<text>get the edge relation for the visibility graph</text--->"
    "<text>query getvgedge(dg1, vgnodes, tri_reg_new_sort,"
    "vertex_tri, btr_vid) count;</text--->) )";

const string OpTMMyInsideSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>line x region -> bool</text--->"
    "<text>line myinside region</text--->"
    "<text>checks whether a line is completely inside a region</text--->"
    "<text>query l2 myinside r2; </text--->"
    ") )";

const string OpTMAtPointSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>sline x point x bool -> real</text--->"
    "<text>point inside sline</text--->"
    "<text>return the position of a point on a sline</text--->"
    "<text>query at_point(sl, p, TRUE); </text--->"
    ") )";
    
const string OpTMDecomposeTriSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>decomposetri(rel)</text--->"
    "<text>return the relation between vertices and triangle</text--->"
    "<text>query decomposetri(tri_reg_new_sort) count; </text--->"
    ") )";

const string OpTMCreateVGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x rel -> dualgraph</text--->"
    "<text>createvgraph(int, rel, rel)</text--->"
    "<text>create a visibility graph by the input edge and node"
    "relation</text--->"
    "<text>query createvgraph(1, edge-rel, node-rel); </text--->"
    ") )";

const string OpTMGetContourSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>text -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getcontour(text)</text--->"
    "<text>create regions from the data file</text--->"
    "<text>query getcontour(pppoly) count; </text--->"
    ") )";
const string OpTMGetPolygonSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr -> region</text--->"
    "<text>getpolygon(rel,attr)</text--->"
    "<text>create one region by the input relation with contours</text--->"
    "<text>query getpolygon(allcontours,hole); </text--->"
    ") )";

const string OpTMGetAllPointsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getallpoints(region)</text--->"
    "<text>get all vertices of a polygon with its two neighbors</text--->"
    "<text>query getallpoints(node_reg); </text--->"
    ") )";

const string OpTMRotationSweepSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x bbox x rel3 x attr ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>rotationsweep(rel,rel,rectangle<2>,rel,attr)</text--->"
    "<text>search visible points for the given point</text--->"
    "<text>query rotationsweep(query_loc,allpoints,bbox,holes,hole); </text--->"
    ") )";
const string OpTMGetHoleSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>gethole(r)</text--->"
    "<text>get all holes of a region</text--->"
    "<text>query gethole(node_reg) count; </text--->"
    ") )";

const string OpTMGetSectionsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x rel1 x attr1 x attr2 x attr3"
    " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getsections(n, r, r, attr,attr,attr)</text--->"
    "<text>for each route, get the possible sections where interesting"
    "points can locate</text--->"
    "<text>query getsections(n, r, paveregions, curve, rid, crossreg) count;"
    "</text--->"
    ") )";
/*const string OpTMGenInterestP1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text> rel x rel x attr1 x attr2 x attr3 x attr4"
    " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>geninterestp1(r, r, attr, attr, attr, attr)</text--->"
    "<text>generate interesting points locate in pavement</text--->"
    "<text>query geninterestp1(subsections, pave_regions1, rid, sec,"
    "pavement1, pavement2) count;</text--->"
    ") )";
const string OpTMGenInterestP2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text> rel x rel x rtree x attr1 x attr2 x int "
    " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>geninterestp2(r, r, rtree, attr, attr, int)</text--->"
    "<text>map the point inot a triangle and represent it by triangle</text--->"
    "<text>query geninterestp2(interestp, dg_node, rtree_dg, loc2, pavement, 2)"
    "count;</text--->"
    ") )";*/
const string OpTMCellBoxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>bbox x int-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>cellbox(bbox, 10)</text--->"
    "<text>partition the bbox into 10 x 10 equal size cells</text--->"
    "<text>query cellbox(bbox, 10)"
    "count;</text--->"
    ") )";

/*
create region based outdoor infrastructure 

*/
const string OpTMThePavementSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel -> pavenetwork</text--->"
    "<text>thepavement(1, rel); </text--->"
    "<text>create pavement infrastructure</text--->"
    "<text>query thepavement(1, dg_node) ;</text--->) )";


/*
create bus networks

*/
    const string OpTMCreateBusRouteSpec1  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3 x attr4 x btree x rel"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_route1(n,rel,attr1,attr2,attr3,attr4,btree,rel);"
    "</text--->"
    "<text>create bus route1</text--->"
    "<text>query create_bus_route1(n,street_sections_cell,sid_s,cellid_w_a_c,"
    "Cnt_a_c,cover_area_b_c,section_cell_index, bus_para) count;</text--->"
    ") )";
    
const string OpTMCreateBusRouteSpec2  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>space x rel1 x attr x btree x rel2 x attr1 x attr2 x attr3"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_route2(sp,rel,attr,btree,rel2,attr1,attr2,attr3);"
    "</text--->"
    "<text>create bus routes</text--->"
    "<text>query create_bus_route2(sp,street_sections_cell,cellid_w_a_c,"
    "section_cell_index,rough_pair,start_cell_id,end_cell_id,route_type) "
    "count;</text--->"
    ") )";

const string OpTMRefineBusRouteSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3 x attr4"
    " x attr5 x attr6 -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>refine_bus_route(network,rel,attr1,attr2,attr3,attr4,attr5,attr6);"
    "</text--->"
    "<text>refine bus routes,filter some bus routes which are similar</text--->"
    "<text>query refine_bus_route(n,busroutes_temp,br_id,bus_route1,"
    "bus_route2,start_loc,end_loc,route_type) count;</text--->"
    ") )";

const string OpTMCreateBusRouteSpec3  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x attr3 x real"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_route3(rel,attr1,attr2,attr3,real);"
    "</text--->"
    "<text>translate bus routes</text--->"
    "<text>query create_bus_route3(busroutes,br_id,bus_route2,route_type"
    "roadwidth/2) count;</text--->"
    ") )";
    
const string OpTMCreateBusRouteSpec4  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x attr1 x attr2 x attr3 x attr4 x rel2 x attr1"
    " x attr2 -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_route4(rel1,attr1,attr2,attr3,attr4,rel2,attr1,"
    "attr2);</text--->"
    "<text>set up and down for bus routes</text--->"
    "<text>query create_bus_route4(newbusroutes,br_id,bus_route2,"
    "route_type, br_uid, bus_stops3, br_id, startSmaller) count;</text--->"
    ") )";

/*
create bus stops for bus network

*/
const string OpTMCreateBusStopSpec1  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x attr1 x attr2 x attr3 x attr4 x rel2 x btree"
    "x rel -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops1(n,rel1,attr1,attr2, attr3, attr4, rel2,"
    " btree, rel);</text--->"
    "<text>create bus stops</text--->"
    "<text>query create_bus_stop1(n,busroutes,br_id,bus_route1,"
    "bus_route2,route_type,subpaves2, btree_pave2, rel) count;</text--->"
    ") )";
    
const string OpTMCreateBusStopSpec2  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops2(n,rel,attr1,attr2, attr3);</text--->"
    "<text>merge bus stops</text--->"
    "<text>query create_bus_stop2(n,bus_stops1,br_id,bus_stop_id,bus_stop1) "
    "count;</text--->"
    ") )";
    
const string OpTMCreateBusStopSpec3  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x attr x rel2 x attr1 x attr2 x attr3 x btree"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops3(n,rel1,attr,rel2,attr1,attr2,att3,btree);"
    "</text--->"
    "<text>merge bus stops</text--->"
    "<text>query create_bus_stop3(n,busroutes, bus_route1, bus_stops2,"
    "br_id, bus_stop_id, bus_stop1,btree_sec_id) count;</text--->"
    ") )";
    
    
const string OpTMCreateBusStopSpec4  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x attr1 x attr2 x rel2 x attr1 x attr2 x attr3 x attr4"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops4(rel1,attr_a, attr_b,rel2,attr1,attr2,attr3,attr4);"
    "</text--->"
    "<text>new position for bus stops after translate bus route</text--->"
    "<text>query create_bus_stop4(newbusroutes, bus_route1,bus_route2," 
    "bus_stops3,br_id, bus_stop_id, bus_stop2,startSmaller) count;</text--->"
    ") )";
    
const string OpTMCreateBusStopSpec5  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x attr x rel2 x attr1 x attr2 x attr3 x attr4 x attr5"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops5(rel1,attr,rel2,attr1,attr2,attr3,attr4,attr5);"
    "</text--->"
    "<text>set up and down value for each bus stop</text--->"
    "<text>query create_bus_stop5(final_busroutes, bus_direction," 
    "bus_stops4,br_id,br_uid, bus_stop_id, bus_stop2,bus_pos) count;</text--->"
    ") )";

/*
bus stops with data type busstop

*/
const string OpTMGetBusStopsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x btree x rel2"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn))))</text--->"
    "<text>getbusstops(rel1,btree,rel2); </text--->"
    "<text>create bus stops with data type busstop</text--->"
    "<text>query getbusstops(final_busstops, btree_bs, final_busroutes)"
    " count;</text--->) )";

const string OpTMGetBusRoutesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x btree x rel2"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn))))</text--->"
    "<text>getbusroutes(rel1,btree,rel2); </text--->"
    "<text>create bus routes with data type busroute</text--->"
    "<text>query getbusroutes(final_busstops, btree_bs, final_busroutes)"
    " count;</text--->) )";

const string OpTMBRGeoDataSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busroute -> sline</text--->"
    "<text>brgeodata(busroute); </text--->"
    "<text>get the geometrical data of a bus route</text--->"
    "<text>query brgeodata(br1);</text--->) )";

const string OpTMBSGeoDataSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busstop x busroute -> point</text--->"
    "<text>bsgeodata(busstop, busroute); </text--->"
    "<text>get the geometrical data of a bus stop</text--->"
    "<text>query bsgeodata(bs1, br1);</text--->) )";

const string OpTMGetStopIdSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busstop -> int</text--->"
    "<text>getstopid(busstop); </text--->"
    "<text>get bus stop id</text--->"
    "<text>query getstopid([const busstop value (1 2 TRUE)]) ;</text--->) )";
    
const string OpTMUpDownSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busstop -> bool</text--->"
    "<text>up_down(busstop); </text--->"
    "<text>get direction of the bus stop</text--->"
    "<text>query up_down([const busstop value (1 2 TRUE)]) ;</text--->) )";

/*
create bus network 

*/
const string OpTMTheBusNetworkSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x rel x rel -> busnetwork</text--->"
    "<text>thebusnetwork(1, rel, rel, rel); </text--->"
    "<text>create bus network</text--->"
    "<text>query busnetwork(1, bus_stops, bus_routes, bus) ;</text--->) )";

const string OpTMBusStopsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busnetwork -> rel</text--->"
    "<text>bn_busstops(busnetwork); </text--->"
    "<text>get bus stops relation</text--->"
    "<text>query bn_busstops(bn1) ;</text--->) )";

const string OpTMBusRoutesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busnetwork -> rel</text--->"
    "<text>bn_busroutes(busnetwork); </text--->"
    "<text>get bus routes relation</text--->"
    "<text>query bn_busroutes(bn1) ;</text--->) )";

const string OpTMMapBRSegmentsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>line x line -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>brsegments(line, line); </text--->"
    "<text>decompose a bus route</text--->"
    "<text>query brsegments(l1,l2) count ;</text--->) )";
    
const string OpTMMapBsToPaveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busnetwork x rtree x rel x int x real"
     " -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>mapbstopave(bn1, rtree_dg, dg_node, roadwidth, real);"
    " </text--->"
    "<text>map bus stops to pavement areas</text--->"
    "<text>query mapbstopave(bn1, rtree_dg, dg_node, roadwidth, real)"
    " count ;</text--->) )";

/*
build the connection between bus stops and pavements 

*/
const string OpTMBsNeighbors1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x visibility graph x rel1 x rel2 x rtree"
     " -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>bs_neighbors1(dual graph, visibility graph rel, rel, rtree);"
    " </text--->"
    "<text>for each bus stop find its neighbor bus stops</text--->"
   "<text>query bs_neighbors1(dg, vg, tri_reg_new, bs_pave_sort, rtree_bs_pave)"
    " count ;</text--->) )";

const string OpTMBsNeighbors2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busnetwork -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>bs_neighbors2(busnetwork)</text--->"
    "<text>bus stops with the same 2D point, but different bus routes</text--->"
   "<text>query bs_neighbors2(bn1) count ;</text--->) )";
   
const string OpTMBsNeighbors3Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rel x btree -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>bs_neighbors3((rel1, rel2, btree)</text--->"
    "<text>bus stops connected by moving buses</text--->"
    "<text>query bs_neighbors3(bus_time_table, all_bus_rel, btree_mo)"
    " count ;</text--->) )";   

/*
create a graph on bus network including pavements connection 

*/
const string OpTMCreateBusGraphSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel1 x rel2 x rel3 x rel4-> busgraph</text--->"
    "<text>createbgraph(int, rel, rel, rel, rel)</text--->"
    "<text>create a bus network graph by the input edges and nodes"
    "relation</text--->"
    "<text>query createbgraph(1, node-rel, edge1, edge2, edge3); </text--->"
    ") )";

const string OpTMGetAdjNodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busgraph x int -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>getadjnode(busgraph, int)</text--->"
    "<text>get the neighbor nodes of a given graph node</text--->"
    "<text>query getadjnode(bg1, 2); </text--->"
    ") )";

const string OpTMBNNavigationSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busstop x busstop x busnetwork x instant x int"
    " -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>bnnavigation(busstop,bussstop,busnetwork,instant,int)</text--->"
    "<text>navigation in bus network system</text--->"
    "<text>query bnnavigation(bs1, bs2, bn1, "
    "theInstant(2010,12,5,16,0,0,0),0) count; </text--->"
    ") )";

const string OpTMTestBNNavigationSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rel x busnetwork x instant x int-> bool</text--->"
    "<text>test_bnnavigation(rel,rel,busnetwork,instant,int)</text--->"
    "<text>test the operator bnnavigation</text--->"
    "<text>query test_bnnavigation(bus_stops, bus_stops, bn1, "
    "theInstant(2010,12,5,16,0,0,0),0) count; </text--->"
    ") )";

/*
get traffic data and set time schedule for moving buses 

*/
const string OpTMGetRouteDensity1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x attr1 x attr2 x btree x rel2 x attr1 x attr2"
    " x periods1 x periods2 -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>get_route_density1(network,rel1,attr1,attr2,btree,rel2,attr1, attr2,"
    "peridos1,periods2);</text--->"
    "<text>distinguish daytime and night bus routes</text--->"
    "<text>query get_route_density1(n,traffic_rel1,secid,flow,btree_traffic,"
     "busroutes,br_id,bus_route1,night1,night2) count;</text--->"
    ") )";

const string OpTMSETTSNightBusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x attr3 x periods1 x periods2 -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>set_ts_nightbus(rel,attr1,attr2,attr3,peridos1,periods2);</text--->"
    "<text>set time schedule for night buses</text--->"
    "<text>query set_ts_nightbus(night_bus,br_id,duration1,duration2,"
    "night1,night2) count;</text--->"
    ") )";

const string OpTMSETTSDayBusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x attr3 x periods1 x periods2 -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>set_ts_daybus(rel,attr1,attr2,attr3,peridos1,periods2);</text--->"
    "<text>set time schedule for daytime buses</text--->"
    "<text>query set_ts_daybus(day_bus,br_id,duration1,duration2,"
    "night1,night2) count;</text--->"
    ") )";
    
const string OpTMSETBRSpeedBusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x attr1 x attr2 x rel2 x attr x rel3 x attr-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>set_br_speed(network,rel1,attr1,attr2,rel2,attr,rel3,"
    "attr);</text--->"
    "<text>set speed value for each bus route</text--->"
    "<text>query set_br_speed(n,busroutes,br_i,d,bus_route1,"
    "streets,Vmax,final_busroutes,startSmaller) count;</text--->"
    ") )";
    
const string OpTMCreateBusSegmentSpeedSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x attr1 x attr2 x attr3 x attr4 x rel2 x attr1 x attr2"
    " x btree1 x rel3 x btree2-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_segment_speed(rel,attr1,attr2,attr3,attr4,rel,attr1"
    "attr2, btree, rel, btree);</text--->"
    "<text>set speed value for each bus route segment </text--->"
    "<text>query create_bus_segment_speed(final_busroutes, br_id, bus_route, "
    "bus_direction,startSmaller,final_busstops, bus_pos," 
    "stop_direction, btree_bs,br_speed, btree_br_speed) count;</text--->"
    ") )";

const string OpTMCreateNightBusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_night_bus_mo(rel,rel,btree);</text--->"
    "<text>create night moving bus </text--->"
    "<text>query create_night_bus_mo(ts_nightbus, "
    "bus_segment_speed,btree_seg_speed) count;</text--->"
    ") )";

const string OpTMCreateDayTimeBusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_daytime_bus_mo(rel,rel,btree);</text--->"
    "<text>create daytime moving bus </text--->"
    "<text>query create_daytime_bus_mo(ts_daybus, "
    "bus_segment_speed,btree_seg_speed) count;</text--->"
    ") )";

const string OpTMCreateTimeTable1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree x periods x periods"
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table1(rel,rel,btree,periods,periods);</text--->"
    "<text>create time table at each spatial location </text--->"
    "<text>query create_time_table1(final_busstops,all_bus_rel,btree_mo,"
    "night1, night2) count;</text--->"
    ") )";

const string OpTMCreateTimeTable2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table2(rel,rel,btree);</text--->"
    "<text>compact storage of time tables </text--->"
    "<text>query create_time_table2(train_stops,ubtrains,btree_train)"
    "count;</text--->"
    ") )";

const string OpTMRefMO2GenMOSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>refmo2genmo(rel1, rel2, btree );</text--->"
    "<text>convert trains to generic moving objects </text--->"
    "<text>query refmo2genmo(Trains, ubahn_line, btree_ub_line) count;"
    "</text--->) )";

////////////////////////////////////////////////////////////
//////////////////// metro network /////////////////////////
////////////////////////////////////////////////////////////

const string OpTMTheMetroNetworkSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x rel x rel-> metronetwork</text--->"
    "<text>themetronetwork(1, rel, rel, rel); </text--->"
    "<text>create metro network</text--->"
    "<text>query metronetwork(1, metro_stops, metro_routes, metros) "
    ";</text--->) )";

const string OpTMMSNeighbor1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>ms_neighbors1(rel); </text--->"
    "<text>create edges for metro graph</text--->"
    "<text>query  ms_neighbors1(metro_stops) count "
    ";</text--->) )";

const string OpTMMSNeighbor2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>metronetwork x rel x btree x rel x btree->"
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>ms_neighbors2(metronetwork,rel,btree,rel,btree); </text--->"
    "<text>create edges for metro graph</text--->"
    "<text>query ms_neighbors2(mn,timetable1,btree, genmo_rel,btree) count "
    ";</text--->) )";

const string OpTMCreateMetroGraphSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel1 x rel2 x rel3 -> metrograph</text--->"
    "<text>createmgraph(int, rel, rel, rel)</text--->"
    "<text>create a metro network graph by the input edges and nodes"
    "relation</text--->"
    "<text>query createmgraph(1, node-rel, edge1, edge2); </text--->"
    ") )";

const string OpTMCreateMetroRouteSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x rel "
     "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>createmetroroute(dualgraph, rel)</text--->"
    "<text>create metro routes</text--->"
    "<text>query createmetroroute(dualgraph, metro_para); </text--->"
    ") )";

const string OpTMCreateMetroStopSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>createmetrostop(rel)</text--->"
    "<text>create metro stops</text--->"
    "<text>query createmetrostop(rel); </text--->"
    ") )";

const string OpTMCreateMetroMOSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x duraion ->"
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>createmetromo(rel, duration)</text--->"
    "<text>create moving metro </text--->"
    "<text>query createmetromo(rel, duration); </text--->"
    ") )";

const string OpTMMapMsToPaveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rel x rtree ->"
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>mapmstopave(rel, rel, rtree)</text--->"
    "<text>map metro stops to pavement areas </text--->"
    "<text>query mapmstopave(rel, rel, rtree); </text--->"
    ") )";

const string OpTMMNNavigationSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busstop x busstop x busnetwork x instant"
    " -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>mnnavigation(busstop,bussstop,metronetwork,instant)</text--->"
    "<text>navigation in metro network system</text--->"
    "<text>query mnnavigation(ms1, ms2, mn1, "
    "theInstant(2010,12,5,16,0,0,0)) count; </text--->"
    ") )";

const string OpTMNearStopBuildingSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>space x string "
    " ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>nearstops_building(space, string)</text--->"
    "<text>find buildings near to bus stops</text--->"
    "<text>query nearstops_building(space, Bus)</text--->"
    ") )";

const string OpTMDecomposeGenmoSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x int ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>decomposegenmo(rel, int)</text--->"
    "<text>reorganize the units in genmo </text--->"
    "<text>query decomposegenmo(all_genmo, 0)</text--->"
    ") )";

const string OpTMBulkLoadTMRtreeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) x attr1 x attr2 "
    " x attr3 -> tmrtree</text--->"
    "<text>bulkloadtmrtree[_,_,_]</text--->"
    "<text>build an TM-Rtree on genmo units </text--->"
    "<text>query genmo_units feed addid bulkloadtmrtree[Time, Box2d, "
    " Mode]</text--->"
    ") )";

const string OpTMRtreeModeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>tmrtree x rel x attr-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>tmrtreemode(tmrtree, rel, attr)</text--->"
    "<text>calculate the mode value for each TM-Rtree node </text--->"
    "<text>query tmrtreemode(TM_RTree, genmo_units, Mode) </text--->"
    ") )";

const string OpTMNodesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>tmrtree ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>tm_nodes(tmrtree)</text--->"
    "<text>return nodes of tmrtree </text--->"
    "<text>query tm_nodes(TM-Rtree) count</text--->"
    ") )";

const string OpTMInstant2DaySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>instant -> int </text--->"
    "<text>instant2day(instant);</text--->"
    "<text>get the day (int value) of time</text--->"
    "<text>query instant2day(theInstant(2007,6,3,9,0,0,0));</text--->"
    ") )";

/*
build a path between the entrance of the building and the pavement area 

*/
const string OpTMPathToBuildingSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree x space->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>path_to_building(rel, rel, btree, space);</text--->"
    "<text>build the connection between building and pavement</text--->"
    "<text>query path_to_building(building_rect, new_region_elems," 
    "btree_region_elem, space_1);</text--->"
    ") )";
    
const string OpTMSetBuildingTypeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rtree x space -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>set_building_type(rel, rtree, space);</text--->"
    "<text>set the type for each building</text--->"
    "<text>query set_building_type(building_region_type, "
    "rtree_build, space_1);</text--->"
    ") )";

/*
remove dirty region data 

*/
const string OpTMRemoveDirtySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))"
     "</text--->"
    "<text>remove_dirty(rel, attr, attr);</text--->"
    "<text>clear some dirty region data</text--->"
    "<text>query remove_dirty(region_elems, id, covarea);</text--->"
    ") )";

    
const string OpTMModifyLineSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>sline -> sline</text--->"
    "<text>modifyline(sline)</text--->"
    "<text>modify the coordinates of a sline, for numeric problem</text--->"
    "<text>query modifyline([const sline value ((2.33 3.33 4.444 5.555))])"
    "</text--->"
    ") )";

const string OpTMRefineDataSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>sline -> sline</text--->"
    "<text>refinedata(sline)</text--->"
    "<text>modify the coordinates of a sline, for numeric problem</text--->"
    "<text>query refinedata([const sline value ((2.33 3.33 4.444 5.555))])"
    "</text--->"
    ") )";

const string OpTMFilterDisjointSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x btree ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>filterdisjoint(rel, btree)</text--->"
    "<text>remove disjoint pieces of roads</text--->"
    "<text>query filterdisjoint(DOPedes_L_Join, btree_l) count"
    "</text--->"
    ") )";

const string OpTMRefineBRSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr x attr -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>refinebr(rel, attr, attr)</text--->"
    "<text>discover bus routes from road segments </text--->"
    "<text>query refinebr(DOBusRoutes, RelId, BRoute) count</text--->"
    ") )";


const string OpTMBSStopSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rel x attr x attr->"
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>bs_stops(rel, rel, attr, attr)</text--->"
    "<text>set bus stops by corresponding data types</text--->"
    "<text>query bs_stops(BusSegs, BSStops_Loc, Geo, Oid_S, "
    "Loc_L) count</text--->) )";

const string OpTMSetBSSpeedSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rel x rel x attr->"
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>set_bs_speed(rel, rel, rel, attr)</text--->"
    "<text>set the speed value for bus segment</text--->"
    "<text>query st_bs_speed(BusSegs, streets_speed, BusRoadSegs, Vmax)"
    "count</text--->) )";

const string OpTMSetStopLocSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>line -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>set_stop_loc(line)</text--->"
    "<text>set certain positions</text--->"
    "<text>query st_stop_loc(M_Lines)count</text--->) )";

const string OpTMGetMetroDataSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x rel x string ->"
    " (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getmetrodata(rel, rel, string)</text--->"
    "<text>get metro stops and routes</text--->"
    "<text>query getmetrodata(MetroSegs, MetroRoadSegs, "
    "\"STOP\")count</text--->) )";

const string OpTMSLine2RegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>sline -> region </text--->"
    "<text>sl2reg(sline)</text--->"
    "<text>from sline to region</text--->"
    "<text>query sl2reg(sl1)</text--->) )";

const string OpTMSEGSSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>tm_segs(region)</text--->"
    "<text>return segments of a region</text--->"
    "<text>query tm_segs(r1)</text--->) )";


const string OpTMCheckRoadsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>relation x rtree"
    " ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>checkroads(rel, rtree)</text--->"
    "<text>check the coordinates of a line</text--->"
    "<text>query checkroads(r,rtree_road)</text--->"
    ") )";

const string OpTMTMJoin1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>relation x relation x rtree"
    " ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>tm_join1(rel, rel, rtree)</text--->"
    "<text>check the intersection of routes and cell boxes</text--->"
    "<text>query tm_join1(r, cell_box, rtree_box)</text--->"
    ") )";

/*
get the maximum rectangle from a convex polygon 

*/
const string OpTMMaxRectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region -> rect </text--->"
    "<text>maxrect(region);</text--->"
    "<text>get the maximum rectangle area for a region</text--->"
    "<text>query maxrect(r1);</text--->"
    ") )";

const string OpTMGetRect1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x rel->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getrect1(rel, attr, attr, rel);</text--->"
    "<text>get the maximum rectangle area for a region</text--->"
    "<text>query getrect1(new_region_elems2, id, covarea, para_rel);</text--->"
    ") )";

/////////////////////////////////////////////////////////////////////////////
///////////////////////  Indoor Operators    /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

const string SpatialSpecTheFloor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>float x region -> floor3d</text--->"
"<text>thefloor ( _, _ ) </text--->"
"<text>create a floor3d object.</text--->"
"<text>query thefloor (5.0, r)</text---> ) )";

const string SpatialSpecGetHeight =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>floor3d -> float</text--->"
"<text>getheight ( _ ) </text--->"
"<text>get the ground height of a floor3d object</text--->"
"<text>query getheight(floor3d_1)</text---> ) )";

const string SpatialSpecGetRegion =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>floor3d -> region</text--->"
"<text>getregion ( _ ) </text--->"
"<text>get the ground area of a floor3d object</text--->"
"<text>query getregion(floor3d_1)</text---> ) )";

const string SpatialSpecTheDoor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x line x int x line x mbool x bool -> door3d</text--->"
"<text>thedoor ( _,_,_,_, _,_ ) </text--->"
"<text>create a door3d object.</text--->"
"<text>query thedoor (1,l1,2,l3,doorstate, FALSE)</text---> ) )";


const string SpatialSpecTypeOfDoor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>door3d -> bool</text--->"
"<text>type_of_door ( _ ) </text--->"
"<text>get the type of door: lift or non-lift</text--->"
"<text>query type_of_door (door1)</text---> ) )";

const string SpatialSpecLocOfDoor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>door3d x int -> line</text--->"
"<text>loc_of_door (_, _) </text--->"
"<text>get the relative location of door</text--->"
"<text>query loc_of_door (door1,1)</text---> ) )";

const string SpatialSpecStateOfDoor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>door3d -> mbool</text--->"
"<text>state_of_door (_) </text--->"
"<text>get the time dependent state of door</text--->"
"<text>query state_of_door (door1)</text---> ) )";

const string SpatialSpecGetFloor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>groom x int -> floor3d</text--->"
"<text>get_floor (_, _) </text--->"
"<text>get one element of a groom</text--->"
"<text>query get_floor (groom1, 0)</text---> ) )";

const string SpatialSpecAddHeightGRoom =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>groom x real -> groom</text--->"
"<text>add_height_groom(_, _) </text--->"
"<text>move the groom to a new height by adding input</text--->"
"<text>query add_height_groom(groom1, 3.0)</text---> ) )";


const string SpatialSpecTranslateGRoom =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>groom x real x real -> groom</text--->"
"<text>_ translate_groom [_, _] </text--->"
"<text>translate the 2D area of a groom</text--->"
"<text>query groom1 translate_groom [20.0, 0.0]</text---> ) )";


const string SpatialSpecLengthLine3D =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>line3d -> real</text--->"
"<text> size(_) </text--->"
"<text>return the length of a 3D line</text--->"
"<text>query size(l3d1)</text---> ) )";


const string SpatialSpecBBox3D =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>line3d -> rect3</text--->"
"<text> bbox3d(_) </text--->"
"<text>return the bounding box of a 3D line</text--->"
"<text>query bbox3d(l3d1)</text---> ) )";

const string SpatialSpecTheBuilding =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x string x rel x rel  -> building </text--->"
"<text>thebuilding(int,string,rel,rel) </text--->"
"<text>create a building for its rooms</text--->"
"<text>query thebuilding(1, \"UNIVERSITY\", fernuni, fernuni_extend)"
"</text---> ) )";

const string SpatialSpecTheIndoor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x rel x rel  -> building </text--->"
"<text>theindoor(int,rel,rel) </text--->"
"<text>create the indoor infrastructure </text--->"
"<text>query theindoor(1,  paths1, buildingplustype) </text---> ) )";

const string SpatialSpecCreateDoor3D =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>createdoor3d() </text--->"
"<text>create a 3d line for each door</text--->"
"<text>query createdoor3d(university) count</text---> ) )";


const string SpatialSpecCreateDoorBox =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>createdoorbox(rel) </text--->"
"<text>create a 3d box for each door</text--->"
"<text>query createdoorbox(university) count</text---> ) )";


const string SpatialSpecCreateDoor1 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel1 x rel2 x rtree x attr1 x attr2 x attr3"
 " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn))))</text--->"
"<text>createdoor1(rel,rel,rtree,attr,attr,attr) </text--->"
"<text>create a relation storing doors of a building</text--->"
"<text>query createdoor1(university, box3d_rel, rtree_box3d, groom_oid, "
"groom_tid, Box3d) count</text---> ) )";


const string SpatialSpecCreateDoor2 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>createdoor2(rel) </text--->"
"<text>create a relation of virtual doors for staircase</text--->"
"<text>query createdoor2(university_uni) count</text---> ) )";


const string SpatialSpecCreateAdjDoor1 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel1 x rel2 x btree x attr1 x attr2 x attr3 x attr4"
 " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>createadjdoor1(rel,rel,btree,attr,attr,attr,attr) </text--->"
"<text>create the connecting edges for two doors inside one room</text--->"
"<text>query createadjdoor1(building_uni, node_rel, createbtree, "
"Door, door_loc, groom_oid1, doorheight) count</text---> ) )";


const string SpatialSpecCreateAdjDoor2 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel x rtree "
 " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>createadjdoor2(rel,rtree) </text--->"
"<text>create the connecting edge for the same door which can belong to "
"two rooms </text--->"
"<text>query createadjdoor2(node_rel, rtree_node) count</text---> ) )";


const string SpatialSpecPathInRegion =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>region x point x point -> line</text--->"
"<text>path_in_region(region,point,point) </text--->"
"<text>create the shortest path connecting two points inside a region</text--->"
"<text>query size(path_in_region(reg1, p1, p2))</text---> ) )";

const string OpTMCreateIGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x rel x string -> indoorgraph</text--->"
    "<text>createigraph(int, rel, rel, string)</text--->"
    "<text>create an indoor graph by the input edges and nodes"
    "relation</text--->"
    "<text>query createigraph(1, edge-rel, node-rel, \"cinema\"); </text--->"
    ") )";

 const string SpatialSpecGenerateIP1 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel x int x bool->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>generate_ip1(rel, int, bool) </text--->"
"<text>create indoor positions</text--->"
"<text>query generate_ip1(building_uni,20, TRUE) count</text---> ) )";

const string SpatialSpecIndoorNavigation =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel x genloc x genloc x rel x btree x int"
 " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>indoornavigation(ig,genloc,genloc,rel,btree, int) </text--->"
"<text>indoor trip planning</text--->"
"<text>query indoornavigation(ig, gloc1, gloc2, building_uni, btree_groom 0)"
" count </text---> ) )";

 const string SpatialSpecGenerateMO1 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>indoorgraph x rel x btree x rtree x int x periods ->"
" (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>generate_mo1(indoorgraph, rel,btree, rtree, int, periods) </text--->"
"<text>create indoor moving objects</text--->"
"<text>query generate_mo1(ig1, fernuni, btree_groom, rtree_groom, 20, Monday)"
" count </text---> ) )";


 const string SpatialSpecGetIndoorPath =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>string x int ->line3d</text--->"
"<text>getindorpath(string, int) </text--->"
"<text>read indoor shortest path from disk files</text--->"
"<text>query getindoorpath(UNIVERSITY, 10001001) count </text---> ))";


/*
create an empty space 

*/
const string SpatialSpecTheSpace =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int -> space </text--->"
"<text>thespace (_) </text--->"
"<text>create an empty space</text--->"
"<text>query thespace(1)</text---> ) )";

const string SpatialSpecPutInfra =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>space x network ->  (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>putinfra (space, network) </text--->"
"<text>add infrastructures to the space</text--->"
"<text>query putinfra(space_1, rn)</text---> ) )";

const string SpatialSpecPutRel =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>space x rel ->  (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>putrel (space, rel) </text--->"
"<text>add relations to the space</text--->"
"<text>query putinfra(space_1, rel)</text---> ) )";

const string SpatialSpecGetInfra =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>space x string ->  rel </text--->"
"<text>getinfra (space, \"LINE\") </text--->"
"<text>get required infrastructure from the space</text--->"
"<text>query getinfra(space_1, \"LINE\")</text---> ) )";

const string SpatialSpecGenMOTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>bool -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>genmo_tm_list (bool) </text--->"
"<text>output all possible transportation modes of moving objects</text--->"
"<text>query genmo_tm_list(TRUE)</text---> ) )";


const string SpatialSpecGenerateGMOTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x btree x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo (space, periods, real, int) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo(space_1, TwoDays, 30, 4) </text---> ) )";


const string SpatialSpecGenerateGMOBench1TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel x rel x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_bench_1 (space, periods, real, rel, rel, rel) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_bench_1(space_1, hw_time, 5.0, distri_para1,"
" H_Building, W_Building) </text---> ) )";

const string SpatialSpecGenerateGMOBench2TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel x string"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_bench_2 (space, periods, real, rel, string) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_bench_2(space_1, tuesday, 5.0, Buildingrel, "
"\"REGION\") </text---> ) )";

const string SpatialSpecGenerateGMOBench3TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_bench_3 (space, periods, real, rel, rtree) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_bench_3(space_1, monday, 3.0, NN_Building, "
"rtree_NNB) </text---> ) )";


const string SpatialSpecGenerateGMOBench4TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel x rel x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_bench_4 (space, periods, real, rel, rel, rtree) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_bench_4(space_1, monday, 3.0, dist_para, NN_Building, "
"rtree_NNB) </text---> ) )";


const string SpatialSpecGenerateGMOBench5TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel x rel x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_bench_5 (space, periods, real, rel, rel, rel) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_bench_5(space_1, hw_time, 5.0, distri_para3,"
" H_Building, W_Building) </text---> ) )";

const string SpatialSpecGenerateCarList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_car (space, periods, real, rel) </text--->"
"<text>generate moving cars in road network to get traffic </text--->"
"<text>query generate_car(space_1, TwoDays, 30.0, streets_speed) </text---> ))";


const string SpatialSpecGetRGNodesTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> network -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_rg_nodes(network) </text--->"
"<text>get road graph nodes </text--->"
"<text>query get_rg_nodes(rn) count </text---> ) )";


const string SpatialSpecGetRGEdges1TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> rel -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_rg_edges1(rel) </text--->"
"<text>get road graph edges </text--->"
"<text>query get_rg_edges1(rel) count </text---> ) )";


const string SpatialSpecGetRGEdges2TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> network x rel -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_rg_edges2(network, rel) </text--->"
"<text>get road graph edges </text--->"
"<text>query get_rg_edges2(rn, rel) count </text---> ) )";

const string SpatialSpecGetPaveEdges3TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> network x rel x btree x rel ->"
" (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_p_edges3(network, rel, btree, rel) </text--->"
"<text>get the connection between pavement regions and lines </text--->"
"<text>query get_p_edges3(P_N, P_nodes1, btree_PN1, P_Nodes2) "
"count </text---> ) )";

const string SpatialSpecGetPaveEdges4TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> rel x rel ->(stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_p_edges4(rel, rel) </text--->"
"<text>get the connection inside one region </text--->"
"<text>query get_p_edges4(P_Nodes2_tmp, DOPedesRegions) count </text---> ) )";

const string SpatialSpecTheOSMPaveTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> int x rel x rel ->(stream(((x1 t1) ... (xn tn))) </text--->"
"<text>theosmpave(int, rel, rel) </text--->"
"<text>create osm pavement environment </text--->"
"<text>query theosmpave(1, DOPedeslines, DOPedesRegions) count </text---> ) )";

const string OpTMOSMPaveGraphSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel1 x rel2 -> osmpavegraph</text--->"
    "<text>creatergraph(int, rel, rel)</text--->"
    "<text>create a osm pave graph by the input edges and nodes"
    "relation</text--->"
    "<text>query createosmgraph(1, node-rel, edge1); </text--->"
    ") )";


const string OpTMOSMLocMapSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 -> osmpavegraph</text--->"
    "<text>osmlocmap(rel, rel)</text--->"
    "<text>map osm locations to lines and regions</text--->"
    "<text>query osmlocmap(POI_L, POI_R); </text--->"
    ") )";

    
const string OpTMOSMPathSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x osmpavenetwork -> sline</text--->"
    "<text>osm_path(rel, rel, osmpavenetwork)</text--->"
    "<text>shortest path inside OSM pavement</text--->"
    "<text>query osm_path(loc1, loc2, osm_pave); </text--->"
    ") )";

const string OpTMCreateRoadGraphSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel1 x rel2 x rel3 -> roadgraph</text--->"
    "<text>creatergraph(int, rel, rel, rel)</text--->"
    "<text>create a road network graph by the input edges and nodes"
    "relation</text--->"
    "<text>query creatergraph(1, node-rel, edge1, edge2); </text--->"
    ") )";

/*
shortest path in road network

*/
const string OpTMShortestPathTMSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>gpoint x gpoint x roadgraph x network ->"
    " (stream(((x1 t1) ... (xn tn)))</text--->"
    "<text>shortestpath_tm(gpoint, gpoint, roadgraph, network)</text--->"
    "<text>return the shortest path in road network for two gpoints</text--->"
    "<text>query shortestpath_tm(gp1, gp2, rg1, rn); </text--->"
    ") )";

const string SpatialSpecNavigation1List =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x rel x rel x instant x rel x rel x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>navigation1 (space, rel,rel,instant, rel,rel, rtree) </text--->"
"<text>navigation with modes bus and walk</text--->"
"<text>query navigation1(space_1, queryloc1, queryloc2, instant1, tri_reg_new,"
"bs_pave_sort, rtree_bs_pave) </text---> ) )";


const string SpatialSpecRefId =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genloc -> int</text--->"
"<text>ref_id (genloc) </text--->"
"<text>get the reference id of a genloc object</text--->"
"<text>query ref_id (genloc1)</text---> ) )";


const string SpatialSpecSetMORefId =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> (stream uT)</text--->"
"<text>ref_id (genmo) </text--->"
"<text>get the reference id of a generic moving object</text--->"
"<text>query ref_id (genmo1)</text---> ) )";

const string SpatialSpecTMAT =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x string -> genmo</text--->"
"<text>tm_at (genmo, string) </text--->"
"<text>get the moving object with one mode</text--->"
"<text>query tm_at(genmo1, \"Indoor\")</text---> ) )";

const string SpatialSpecTMAT2 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x mreal x string -> genmo</text--->"
"<text>tm_at2(genmo, mreal, string) </text--->"
"<text>get the moving object with one mode</text--->"
"<text>query tm_at(genmo1, mreal1, \"Indoor\")</text---> ) )";


const string SpatialSpecTMAT3 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x mreal x genloc x string -> genmo</text--->"
"<text>tm_at2(genmo, mreal, genloc, string) </text--->"
"<text>get the moving object with one mode</text--->"
"<text>query tm_at(genmo1, mreal1, genloc1,\"Indoor\")</text---> ) )";


const string SpatialSpecTMVal =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>igenloc -> genloc</text--->"
"<text>val(igenloc) </text--->"
"<text>get the genloc for a igenloc</text--->"
"<text>query val(igloc)</text---> ) )";

const string SpatialSpecTMInst =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>igenloc -> instant</text--->"
"<text>tm_inst (igenloc) </text--->"
"<text>get the instant for a igenloc</text--->"
"<text>query tm_inst(igloc)</text---> ) )";

const string SpatialSpecTMContain =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x string -> bool</text--->"
"<text>contains (genmo, string) </text--->"
"<text>check whether the moving object contains one mode</text--->"
"<text>query contains(genmo1, \"Indoor\")</text---> ) )";

const string SpatialSpecTMContain2 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x mreal x int x space -> bool</text--->"
"<text>tmcontains (genmo,mreal,int,space) </text--->"
"<text>check whether the moving object contains a reference int</text--->"
"<text>query tmcontains(genmo1, uindex,123,space_1)</text---> ) )";

const string SpatialSpecTMDuration =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>periods x string -> real</text--->"
"<text>tm_duration (periods, string) </text--->"
"<text>return the period duration by specifying time unit</text--->"
"<text>query tm_duration(peri1, \"M\")</text---> ) )";


const string SpatialSpecTMInitial =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> igenloc</text--->"
"<text>tm_initia (genmo) </text--->"
"<text>return the intime genloc of a genmo</text--->"
"<text>query initial(genmo1)</text---> ) )";

const string SpatialSpecTMBuildId =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x space -> int</text--->"
"<text>tm_build_id (int, space) </text--->"
"<text>return the building id of an reference</text--->"
"<text>query tm_build_id(0, space1)</text---> ) )";

const string SpatialSpecTMBContains =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x int -> bool</text--->"
"<text>genmo bcontains int </text--->"
"<text>check whether a building id is contained</text--->"
"<text>query genmo1 bcontains 123456</text---> ) )";

const string SpatialSpecTMBContains2 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x mreal x int -> bool</text--->"
"<text>genmo bcontains int </text--->"
"<text>check whether a building id is contained, with index on units</text--->"
"<text>query bcontains(genmo1, uindex, 123456)</text---> ) )";

const string SpatialSpecTMRoomId =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x space -> int</text--->"
"<text>tm_room_id (int, space) </text--->"
"<text>return the room id of an reference</text--->"
"<text>query tm_room_id(0, space1)</text---> ) )";

/*
create a new id by combining the two input integers
e.g., 123 + 34 = 12334

*/
const string SpatialSpecTMPlusId =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x int -> int</text--->"
"<text>tm_plus_id (int, int) </text--->"
"<text>combine two integers</text--->"
"<text>query tm_plus_id(20, 13)</text---> ) )";


const string SpatialSpecTMPass =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x region x space -> bool</text--->"
"<text>passes (genmo, region, space) </text--->"
"<text>check whether a moving object passes an area</text--->"
"<text>query passes(genmo1, reg, space1)</text---> ) )";

const string SpatialSpecTMDistance =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genloc x point x space -> bool</text--->"
"<text>tm_passes (genloc, point, space) </text--->"
"<text>return the distance between a genloc and a point</text--->"
"<text>query tm_distance(genloc1, p1, space1)</text---> ) )";

const string SpatialSpecTMGenLoc =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x real x real -> genloc</text--->"
"<text>tm_genloc (int, real, real) </text--->"
"<text>create a genloc </text--->"
"<text>query tm_genloc(2, 3.0, -1.0)</text---> ) )";

const string SpatialSpecModeVal =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> int</text--->"
"<text>modeval(genmo) </text--->"
"<text>create an integer for modes  </text--->"
"<text>query modeval(genmo1)</text---> ) )";

const string SpatialSpecGenMOIndex =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> mreal</text--->"
"<text>genmoindex(genmo) </text--->"
"<text>create an index on genmo units </text--->"
"<text>query genmoindex(genmo1)</text---> ) )";

const string SpatialSpecGenMODeftime =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> periods</text--->"
"<text>deftime (genmo) </text--->"
"<text>get the deftime time of a generic moving object</text--->"
"<text>query deftime (genmo)</text---> ) )";

const string SpatialSpecGenMONoComponents =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> int</text--->"
"<text>no_components (genmo) </text--->"
"<text>get the number of units in a generic moving object</text--->"
"<text>query no_components(genmo)</text---> ) )"; 

/*
return the location representation in an approximate way

*/
const string SpatialSpecLowRes =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> genmo</text--->"
"<text>lowres (genmo) </text--->"
"<text>return the low resolution of generic moving object</text--->"
"<text>query lowres(genmo1)</text---> ) )";

const string SpatialSpecGenmoTranslate =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x duration -> genmo</text--->"
"<text>genmo tm_translate duration </text--->"
"<text>translate the time period of a genmo</text--->"
"<text>query genmo1 tm_translate [const duration value (1 0)]</text---> ) )";

const string SpatialSpecGenmoTranslate2 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>periods x duration -> periods</text--->"
"<text>periods tm_translate2 duration </text--->"
"<text>translate the time period</text--->"
"<text>query peri1 tm_translate2 [const duration value (1 0)]</text---> ) )";

const string SpatialSpecTMTrajectory =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mpoint3d -> line3d</text--->"
"<text>trajectory (mpoint3d) </text--->"
"<text>get the trajectory of a 3d moving object</text--->"
"<text>query trajectory(mp3_1)</text---> ) )"; 

const string SpatialSpecGenTrajectory =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> genrange</text--->"
"<text>trajectory (genmo) </text--->"
"<text>get the trajectory of a moving object</text--->"
"<text>query trajectory(genmo)</text---> ) )";

const string SpatialSpecGenrangeVisible =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genrange -> (stream(((x1 t1) ... (xn tn))))</text--->"
"<text>genrangevisible (genrange, space) </text--->"
"<text>get the 2d line or 3d line in space, visible in javagui</text--->"
"<text>query genrangevisible(gr1, space1)</text---> ) )";

const string SpatialSpecGetMode =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> (stream(((x1 t1) ... (xn tn)))</text--->"
"<text>getmode (_) </text--->"
"<text>return the transportation modes</text--->"
"<text>query getmode(genmo1)</text---> ) )";


const string SpatialSpecGetRef =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> (stream(((x1 t1) ... (xn tn)))</text--->"
"<text>getref (_) </text--->"
"<text>return the referenced objects in a light way</text--->"
"<text>query getref(genmo1)</text---> ) )";

const string SpatialSpecAtInstant =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x instant -> igenloc</text--->"
"<text>_ atinstant _ </text--->"
"<text>return the instant value of a generic moving object</text--->"
"<text>query genmo1 atinstant instant 1</text---> ) )";

const string SpatialSpecAtPeriods =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x periods -> genmo</text--->"
"<text>_ atperiods _ </text--->"
"<text>return the movement in a given period </text--->"
"<text>query genmo1 atperiods periods1 </text---> ) )";

const string SpatialSpecMapGenMO =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo x mpoint -> mpoint</text--->"
"<text>genmo mapgenmo mpoint </text--->"
"<text>map a genmo to a mpoint </text--->"
"<text>query mapgenmo(genmo1, mp1) </text---> ) )";

const string SpatialSpecMapTMUnits =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> (stream(((x1 t1) ... (xn tn)))) </text--->"
"<text>units(genmo) </text--->"
"<text>get units of a moving object </text--->"
"<text>query units(genmo1) </text---> ) )";


const string SpatialSpecMapGetLoc =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>ugenloc x bool -> point </text--->"
"<text>getloc(ugenloc, bool) </text--->"
"<text>get locatation of a ugenloc </text--->"
"<text>query getloc(ugenloc1,TRUE) </text---> ) )";


const string SpatialSpecMapTMTraffic =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel x periods x rel x bool -> "
" (stream(((x1 t1) ... (xn tn)))) </text--->"
"<text>tm_traffic(rel, periods, rel, bool) </text--->"
"<text>get the traffic value </text--->"
"<text>query tm_traffic(all_genmo,Qt, roadsegs,true) </text---> ) )";


/*
add graphs into infrastructures

*/
const string SpatialSpecAddInfraGraph =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>busnetwork x busgraph -> (stream(((x1 t1) ... (xn tn)))) </text--->"
"<text>addinfragraph (busnetwork,busgraph) </text--->"
"<text>add navigation graph to the corresponding infrastructure</text--->"
"<text>query addinfragraph(bn1,bg1)</text---> ) )";

/////////////////////////////////////////////////////////////////////////////
/////////////////////// Type Constructor///////////////////////////////////
//////////////////////////////////////////////////////////////////////////

////////////////////////   Indoor data Type//////////////////////////////////
///////////// point3d line3d floor3d door3d groom ///////////////////
//////////////////// functions are in Indoor.h  /////////////////////////////


TypeConstructor point3d(
    "point3d", Point3DProperty,
     OutPoint3D, InPoint3D,
     0, 0,
     CreatePoint3D, DeletePoint3D,
//     OpenPoint3D, SavePoint3D,
     OpenAttribute<Point3D>, SaveAttribute<Point3D>,
     ClosePoint3D, ClonePoint3D,
     CastPoint3D,
     SizeOfPoint3D,
     CheckPoint3D
);

TypeConstructor line3d(
        "line3d",                     //name
        Line3DProperty,               //property function describing signature
        OutLine3D,      InLine3D,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateLine3D,   DeleteLine3D, //object creation and deletion
        OpenLine3D,     SaveLine3D,   // object open and save
        CloseLine3D,    CloneLine3D,  //object close and clone
        Line3D::Cast,                   //cast function
        SizeOfLine3D,                 //sizeof function
        CheckLine3D );

TypeConstructor door3d(
        "door3d",                     //name
        Door3DProperty,               //property function describing signature
        OutDoor3D,   InDoor3D,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateDoor3D,   DeleteDoor3D, //object creation and deletion
        OpenDoor3D,     SaveDoor3D,   // object open and save
        CloseDoor3D,    CloneDoor3D,  //object close and clone
        CastDoor3D,                   //cast function
        SizeOfDoor3D,                 //sizeof function
        CheckDoor3D ); 

TypeConstructor groom(
        "groom",                     //name
        GRoomProperty,         //property function describing signature
        OutGRoom,   InGRoom,  //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateGRoom,   DeleteGRoom, //object creation and deletion
        OpenGRoom,     SaveGRoom,   // object open and save
        CloseGRoom, CloneGRoom,  //object close and clone
        CastGRoomD,              //cast function
        SizeOfGRoom,            //sizeof function
        CheckGRoom ); 

TypeConstructor floor3d(
    "floor3d", Floor3DProperty,
     OutFloor3D, InFloor3D,
     0, 0,
     CreateFloor3D, DeleteFloor3D,
     OpenFloor3D, SaveFloor3D,
     CloseFloor3D, CloneFloor3D,
     Floor3D::Cast,
     SizeOfFloor3D,
     CheckFloor3D
);

TypeConstructor upoint3d(
        "upoint3d",                     //name
        UPoint3DProperty,              //property function describing signature
        OutUPoint3D,      InUPoint3D,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateUPoint3D,   DeleteUPoint3D, //object creation and deletion
        OpenUPoint3D,    SaveUPoint3D,   // object open and save

        CloseUPoint3D,    CloneUPoint3D,  //object close and clone
        UPoint3D::Cast,
        SizeOfUPoint3D,                 //sizeof function
        CheckUPoint3D );

TypeConstructor mpoint3d(
        "mpoint3d",                     //name
        MPoint3DProperty,            //property function describing signature
        OutMapping<MPoint3D, UPoint3D,OutUPoint3D>, //Out functions 
        InMapping<MPoint3D, UPoint3D, InUPoint3D>,  //In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateMapping<MPoint3D>, //object creation 
        DeleteMapping<MPoint3D>, //object deletion
        OpenAttribute<MPoint3D>,  //object open 
        SaveAttribute<MPoint3D>,   // object save
        CloseMapping<MPoint3D>,CloneMapping<MPoint3D>,//object close and clone
        CastMapping<MPoint3D>,
        SizeOfMapping<MPoint3D>,              //sizeof function
        CheckMPoint3D); 

TypeConstructor building(
    "building",
     BuildingProperty,
     OutBuilding,      InBuilding,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateBuilding,  DeleteBuilding, //object creation and deletion
     OpenBuilding,    SaveBuilding,   // object open and save

     CloseBuilding,    CloneBuilding,  //object close and clone
     Building::Cast,
     SizeOfBuilding,                 //sizeof function
     CheckBuilding
);

TypeConstructor indoorinfra(
    "indoorinfra",
     IndoorInfraProperty,
     OutIndoorInfra,      InIndoorInfra,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateIndoorInfra,  DeleteIndoorInfra, //object creation and deletion
     OpenIndoorInfra,    SaveIndoorInfra,   // object open and save

     CloseIndoorInfra,    CloneIndoorInfra,  //object close and clone
     IndoorInfra::Cast,
     SizeOfIndoorInfra,                 //sizeof function
     CheckIndoorInfra
);


/*
public transportaton network: bus stop, bus route and bus network 

*/
TypeConstructor busstop(
    "busstop",
     BusStopProperty,
     OutBusStop,      InBusStop,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateBusStop,  DeleteBusStop, //object creation and deletion
     OpenBusStop,    SaveBusStop,   // object open and save

     CloseBusStop,    CloneBusStop,  //object close and clone
     Bus_Stop::Cast,
     SizeOfBusStop,                 //sizeof function
     CheckBusStop
);

TypeConstructor busroute(
    "busroute",
     BusRouteProperty,
     OutBusRoute,      InBusRoute,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateBusRoute,  DeleteBusRoute, //object creation and deletion
     OpenBusRoute,    SaveBusRoute,   // object open and save

     CloseBusRoute,    CloneBusRoute,  //object close and clone
     Bus_Route::Cast,
     SizeOfBusRoute,                 //sizeof function
     CheckBusRoute
);

TypeConstructor busnetwork(
    "busnetwork",
     BusNetworkProperty,
     OutBusNetwork,      InBusNetwork,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateBusNetwork,  DeleteBusNetwork, //object creation and deletion
     OpenBusNetwork,    SaveBusNetwork,   // object open and save

     CloseBusNetwork,    CloneBusNetwork,  //object close and clone
     BusNetwork::Cast,
     SizeOfBusNetwork,                 //sizeof function
     CheckBusNetwork
);


TypeConstructor metronetwork(
    "metronetwork",
     MetroNetworkProperty,
     OutMetroNetwork,      InMetroNetwork,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateMetroNetwork,  DeleteMetroNetwork, //object creation and deletion
     OpenMetroNetwork,    SaveMetroNetwork,   // object open and save

     CloseMetroNetwork,    CloneMetroNetwork,  //object close and clone
     MetroNetwork::Cast,
     SizeOfMetroNetwork,                 //sizeof function
     CheckMetroNetwork
);

/*
for the infrastructure region based outdoor 

*/

TypeConstructor pavenetwork(
    "pavenetwork",
     PavementProperty,
     OutPavement,      InPavement,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreatePavement,  DeletePavement, //object creation and deletion
     OpenPavement,    SavePavement,   // object open and save

     ClosePavement,    ClonePavement,  //object close and clone
     Pavement::Cast,
     SizeOfPavement,                 //sizeof function
     CheckPavement
);

TypeConstructor osmpavenetwork(
    "osmpavenetwork",
     OSMPavementProperty,
     OutOSMPavement,      InOSMPavement,     //Out and In functions
     0,              0,            //SaveTo and RestoreFrom List functions
     CreateOSMPavement,  DeleteOSMPavement, //object creation and deletion
     OpenOSMPavement,    SaveOSMPavement,   // object open and save

     CloseOSMPavement,    CloneOSMPavement,  //object close and clone
     OSMPavement::Cast,
     SizeOfOSMPavement,                 //sizeof function
     CheckOSMPavement
);


///////////////////////////////////////////////////////////////////////////
////////////////////  general data type  /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TypeConstructor ioref(
        "ioref",                     //name
        IORefProperty,              //property function describing signature
        OutIORef,      InIORef,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateIORef,   DeleteIORef, //object creation and deletion
        OpenGenLoc,     SaveGenLoc,   // object open and save

        CloseIORef,    CloneIORef,  //object close and clone
        IORef::Cast,
        SizeOfIORef,                 //sizeof function
        CheckIORef ); 


TypeConstructor genloc(
        "genloc",                     //name
        GenLocProperty,              //property function describing signature
        OutGenLoc,      InGenLoc,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateGenLoc,   DeleteGenLoc, //object creation and deletion
        OpenGenLoc,     SaveGenLoc,   // object open and save

        CloseGenLoc,    CloneGenLoc,  //object close and clone
        GenLoc::Cast,
        SizeOfGenLoc,                 //sizeof function
        CheckGenLoc ); 


TypeConstructor genrange(
        "genrange",                     //name
        GenRangeProperty,              //property function describing signature
        OutGenRange,      InGenRange,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateGenRange,   DeleteGenRange, //object creation and deletion
        OpenGenRange,     SaveGenRange,   // object open and save
//      OpenAttribute<GenRange>, SaveAttribute<GenRange>,//object open and save

        CloseGenRange,    CloneGenRange,  //object close and clone
        GenRange::Cast,
        SizeOfGenRange,                 //sizeof function
        CheckGenRange ); 

TypeConstructor ugenloc(
        "ugenloc",                     //name
        UGenLocProperty,              //property function describing signature
        OutUGenLoc,      InUGenLoc,     //Out and In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateUGenLoc,   DeleteUGenLoc, //object creation and deletion
        OpenUGenLoc,     SaveUGenLoc,   // object open and save

        CloseUGenLoc,    CloneUGenLoc,  //object close and clone
        UGenLoc::Cast,
        SizeOfUGenLoc,                 //sizeof function
        CheckUGenLoc ); 

TypeConstructor genmo(
        "genmo",                     //name
        GenMOProperty,            //property function describing signature
        OutMapping<GenMO, UGenLoc,OutUGenLoc>, //Out functions 
        InMapping<GenMO, UGenLoc, InUGenLoc>,  //In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateMapping<GenMO>, //object creation 
        DeleteMapping<GenMO>, //object deletion
        OpenAttribute<GenMO>,  //object open 
        SaveAttribute<GenMO>,   // object save
        CloseMapping<GenMO>,CloneMapping<GenMO>,//object close and clone
        CastMapping<GenMO>,
        SizeOfMapping<GenMO>,              //sizeof function
        CheckGenMO); 

TypeConstructor space(
        "space",                     //name
        SpaceProperty,            //property function describing signature
        OutSpace, //Out functions 
        InSpace,  //In functions
        0,              0,            //SaveTo and RestoreFrom List functions
        CreateSpace, //object creation 
        DeleteSpace, //object deletion
//        OpenAttribute<Space>,//        OpenSpace,  //object open 
//        SaveAttribute<Space>,//        SaveSpace,   // object save
        Space::OpenSpace, 
        Space::SaveSpace,
        CloseSpace, CloneSpace,//object close and clone
        Space::Cast,
        SizeOfSpace,              //sizeof function
        CheckSpace); 

TypeConstructor intimegenloc(
        IGenLoc::BasicType(),
        IntimeGenLocProperty, 
        OutIntime<GenLoc, OutGenLoc>,
        InIntime<GenLoc, InGenLoc>,
        0, 0, 
        CreateIntime<GenLoc>,
        DeleteIntime<GenLoc>,
        OpenAttribute<Intime<GenLoc> >,
        SaveAttribute<Intime<GenLoc> >,
        CloseIntime<GenLoc>,
        CloneIntime<GenLoc>,
        CastIntime<GenLoc>,
        SizeOfIntime<GenLoc>,
        CheckIntimeGenLoc
);

TypeConstructor dualgraph("dualgraph", DualGraph::BaseGraphProp,
      DualGraph::OutDualGraph, DualGraph::InDualGraph,
      0, 0,
      DualGraph::CreateDualGraph, DualGraph::DeleteDualGraph,
      DualGraph::OpenDualGraph, DualGraph::SaveDualGraph,
      DualGraph::CloseDualGraph, DualGraph::CloneBaseGraph,
      DualGraph::CastBaseGraph, DualGraph::SizeOfBaseGraph,
      DualGraph::CheckDualGraph
);

TypeConstructor visualgraph("visualgraph", VisualGraph::BaseGraphProp,
      VisualGraph::OutVisualGraph, VisualGraph::InVisualGraph,
      0, 0,
      VisualGraph::CreateVisualGraph, VisualGraph::DeleteVisualGraph,
      VisualGraph::OpenVisualGraph, VisualGraph::SaveVisualGraph,
      VisualGraph::CloseVisualGraph, VisualGraph::CloneBaseGraph,
      VisualGraph::CastBaseGraph, VisualGraph::SizeOfBaseGraph,
      VisualGraph::CheckVisualGraph
);

TypeConstructor osmpavegraph("osmpavegraph", OSMPaveGraph::BaseGraphProp,
      OSMPaveGraph::OutOSMPaveGraph, OSMPaveGraph::InOSMPaveGraph,
      0, 0,
      OSMPaveGraph::CreateOSMPaveGraph, OSMPaveGraph::DeleteOSMPaveGraph,
      OSMPaveGraph::OpenOSMPaveGraph, OSMPaveGraph::SaveOSMPaveGraph,
      OSMPaveGraph::CloseOSMPaveGraph, OSMPaveGraph::CloneBaseGraph,
      OSMPaveGraph::CastBaseGraph, OSMPaveGraph::SizeOfBaseGraph,
      OSMPaveGraph::CheckOSMPaveGraph
);

TypeConstructor indoorgraph("indoorgraph", IndoorGraph::BaseGraphProp,
      IndoorGraph::OutIndoorGraph, IndoorGraph::InIndoorGraph,
      0, 0,
      IndoorGraph::CreateIndoorGraph, IndoorGraph::DeleteIndoorGraph,
      IndoorGraph::OpenIndoorGraph, IndoorGraph::SaveIndoorGraph,
      IndoorGraph::CloseIndoorGraph, IndoorGraph::CloneBaseGraph,
      IndoorGraph::CastBaseGraph, IndoorGraph::SizeOfBaseGraph,
      IndoorGraph::CheckIndoorGraph
);

/*
bus graph built on bus network and pavements 
connects from one bus stop to another by bus and walk 

*/
TypeConstructor busgraph("busgraph", BusGraph::BusGraphProp,
      BusGraph::OutBusGraph, BusGraph::InBusGraph,
      0, 0,
      BusGraph::CreateBusGraph, BusGraph::DeleteBusGraph,
      BusGraph::OpenBusGraph, BusGraph::SaveBusGraph,
      BusGraph::CloseBusGraph, BusGraph::CloneBusGraph,
      BusGraph::CastBusGraph, BusGraph::SizeOfBusGraph,
      BusGraph::CheckBusGraph
);

TypeConstructor metrograph("metrograph", MetroGraph::MetroGraphProp,
      MetroGraph::OutMetroGraph, MetroGraph::InMetroGraph,
      0, 0,
      MetroGraph::CreateMetroGraph, MetroGraph::DeleteMetroGraph,
      MetroGraph::OpenMetroGraph, MetroGraph::SaveMetroGraph,
      MetroGraph::CloseMetroGraph, MetroGraph::CloneMetroGraph,
      MetroGraph::CastMetroGraph, MetroGraph::SizeOfMetroGraph,
      MetroGraph::CheckMetroGraph
);



TypeConstructor roadgraph("roadgraph", RoadGraph::RoadGraphProp,
      RoadGraph::OutRoadGraph, RoadGraph::InRoadGraph,
      0, 0,
      RoadGraph::CreateRoadGraph, RoadGraph::DeleteRoadGraph,
      RoadGraph::OpenRoadGraph, RoadGraph::SaveRoadGraph,
      RoadGraph::CloseRoadGraph, RoadGraph::CloneRoadGraph,
      RoadGraph::CastRoadGraph, RoadGraph::SizeOfRoadGraph,
      RoadGraph::CheckRoadGraph
);

////////////////////////////////////////////////////////////////////////////
///////////////// check id /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
check whether the road graph id has been used already 

*/
bool CheckRoadGraphId(unsigned int rg_id)
{
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
       nl->SymbolValue(xObjectType) == "roadgraph")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      RoadGraph* rg = (RoadGraph*)xValue.addr;

      if(rg->GetRG_ID() == rg_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("roadgraph"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("roadgraph"),
                                               xValue);
    }
  }
  return true; 
}


/*
check whether the metro graph id has been used already 

*/
bool CheckMetroGraphId(unsigned int mg_id)
{
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
       nl->SymbolValue(xObjectType) == "metrograph")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      MetroGraph* mg = (MetroGraph*)xValue.addr;

      if(mg->GetG_ID() == mg_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("metrograph"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("metrograph"),
                                               xValue);
    }
  }
  return true; 
}

/*
check whether the osm pavement  d has been used already 

*/
bool ChekOSMPavementId(unsigned int pn_id)
{
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
       nl->SymbolValue(xObjectType) == "osmpavenetwork")
    {
      // Get name of the pavement 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the pavement. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      OSMPavement* pn = (OSMPavement*)xValue.addr;

      if(pn->GetId() == pn_id)
      {
      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("osmpavenetwork"),
                                         xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("osmpavenetwork"),
                                               xValue);
    }
  }
  return true; 
}

/*
check whether the osm pave graph id has been used already 

*/
bool CheckOSMPaveGraphId(unsigned int osmg_id)
{
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
       nl->SymbolValue(xObjectType) == "osmpavegraph")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      OSMPaveGraph* osm_g = (OSMPaveGraph*)xValue.addr;

      if(osm_g->g_id == osmg_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("osmpavegraph"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("osmpavegraph"),
                                               xValue);
    }
  }
  return true; 
}


/*
check whether the bus network id has been used already 

*/
bool ChekPavementId(unsigned int pn_id)
{
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
       nl->SymbolValue(xObjectType) == "pavenetwork")
    {
      // Get name of the pavement 
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the pavement. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      Pavement* pn = (Pavement*)xValue.addr;

      if(pn->GetId() == pn_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("pavenetwork"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("pavenetwork"),
                                               xValue);
    }
  }
  return true; 
}

/*
check whether the bus network id has been used already 

*/
bool ChekBusNetworkId(unsigned int bn_id)
{
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
       nl->SymbolValue(xObjectType) == "busnetwork")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      BusNetwork* bn = (BusNetwork*)xValue.addr;

      if(bn->GetId() == bn_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("busnetwork"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("busnetwork"),
                                               xValue);
    }
  }
  return true; 
}

/*
check whether the bus graph id has been used already 

*/
bool ChekBusGraphId(unsigned int bg_id)
{
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
       nl->SymbolValue(xObjectType) == "busgraph")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      BusGraph* bg = (BusGraph*)xValue.addr;

      if(bg->bg_id == bg_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("busgraph"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("busgraph"),
                                               xValue);
    }
  }
  return true; 
}

/*
check whether the metro network id has been used already 

*/
bool ChekMetroNetworkId(unsigned int mn_id)
{
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
       nl->SymbolValue(xObjectType) == "metronetwork")
    {
      // Get name of the network
      ListExpr xObjectName = nl->Second(xCurrent);
      string strObjectName = nl->SymbolValue(xObjectName);

      // Load object to find out the id of the network. Normally their
      // won't be to much networks in one database giving us a good
      // chance to load only the wanted network.
      Word xValue;
      bool bDefined;
      bool bOk = SecondoSystem::GetCatalog()->GetObject(strObjectName,
                                                        xValue,
                                                        bDefined);
      if(!bDefined || !bOk)
      {
        // Undefined network
        continue;
      }
      MetroNetwork* mn = (MetroNetwork*)xValue.addr;

      if(mn->GetId() == mn_id)
      {
        SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("metronetwork"),
                                               xValue);
        return false;
      }

      SecondoSystem::GetCatalog()->CloseObject(nl->SymbolAtom("metronetwork"),
                                               xValue);
    }
  }
  return true; 
}

///////////////////////////////////////////////////////////////////////////
///////////// Type Map functions for operators ////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
TypeMap function for operator thefloor

*/
ListExpr TheFloorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "float x region expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "real") && nl->IsEqual(arg2, "region"))
      return nl->SymbolAtom("floor3d");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator getheight

*/
ListExpr GetHeightTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "floor3d expected";
      return listutils::typeError(err);
  }

  if(nl->IsEqual(nl->First(args), "floor3d") || 
     nl->IsEqual(nl->First(args), "groom"))
      return nl->SymbolAtom("real");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator getregion

*/
ListExpr GetRegionTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "floor3d expected";
      return listutils::typeError(err);
  }

  if(nl->IsEqual(nl->First(args), "floor3d") || 
     nl->IsEqual(nl->First(args), "groom"))
      return nl->SymbolAtom("region");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator thedoor

*/
ListExpr TheDoorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 6){
      string err = "int x line x int x line x mbool x bool expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  ListExpr arg6 = nl->Sixth(args);
  if(nl->IsEqual(arg1, "int") && nl->IsEqual(arg2, "line") &&
     nl->IsEqual(arg3, "int") && nl->IsEqual(arg4, "line") && 
     nl->IsEqual(arg5, "mbool") && nl->IsEqual(arg6, "bool"))
      return nl->SymbolAtom("door3d"); 

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator type of door

*/
ListExpr TypeOfDoorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "door3d";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "door3d"))
      return nl->SymbolAtom("bool");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator loc of door

*/
ListExpr LocOfDoorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "door3d x int";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "door3d") && nl->IsEqual(arg2, "int"))
      return nl->SymbolAtom("line");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator state of door

*/
ListExpr StateOfDoorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "door3d";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "door3d"))
      return nl->SymbolAtom("mbool");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator get floor 

*/
ListExpr GetFloorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "groom x int expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "groom") && nl->IsEqual(arg2, "int"))
      return nl->SymbolAtom("floor3d");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator add height groom

*/
ListExpr AddHeightGroomTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "groom x real expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "groom") && nl->IsEqual(arg2, "real"))
      return nl->SymbolAtom("groom");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator translate groom

*/
ListExpr TranslateGroomTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "groom x [] expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "groom") && 
     nl->IsEqual(nl->First(arg2), "real") && 
     nl->IsEqual(nl->Second(arg2), "real"))
      return nl->SymbolAtom("groom");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator length l3d 

*/
ListExpr LengthTMTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "line3d expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "line3d") || 
     nl->IsEqual(arg1, "genrange") || nl->IsEqual(arg1, "busroute"))
      return nl->SymbolAtom("real");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator bbox3d 

*/
ListExpr BBox3DTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "line3d expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "line3d") || nl->IsEqual(arg1, "groom"))
      return nl->SymbolAtom("rect3");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator thebuilding

*/
ListExpr TheBuildingTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "int x string x rel x rel";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "int")){
     string err = "int expected";
     return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!(listutils::isSymbol(arg2, CcString::BasicType()))){
     string err = "string expected";
     return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr xType1;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType1); 
  ListExpr xType2;
  nl->ReadFromString(Building::Indoor_GRoom_Door_Extend, xType2); 
  
  if (listutils::isRelDescription(arg3) && listutils::isRelDescription(arg4)
      && CompareSchemas(arg3, xType1) && CompareSchemas(arg4, xType2))
      return nl->SymbolAtom("building");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator theindoor 

*/
ListExpr TheIndoorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "int x rel x rel";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "int")){
     string err = "int expected";
     return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr xType1;
  nl->ReadFromString(IndoorInfra::BuildingPath_Info, xType1); 
  ListExpr xType2;
  nl->ReadFromString(IndoorInfra::BuildingType_Info, xType2); 
  
  if (listutils::isRelDescription(arg2) && listutils::isRelDescription(arg3)
      && CompareSchemas(arg2, xType1) && CompareSchemas(arg3, xType2))
      return nl->SymbolAtom("indoorinfra");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator createdoor3d

*/
ListExpr CreateDoor3DTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "rel";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  ListExpr xType;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType); 
  if (listutils::isRelDescription(arg1)){
      if(CompareSchemas(arg1, xType)){
          ListExpr result =
            nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Door"),
                                      nl->SymbolAtom("line3d"))
                  )
                )
          );
          return result; 
      }else{
      string err = 
      "rel:(oid:int,name:string,type:string,room:groom,door:line) expected";
      return listutils::typeError(err);
      } 
  }
  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator createdoorbox

*/
ListExpr CreateDoorBoxTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "rel";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  ListExpr xType;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType); 
  if (listutils::isRelDescription(arg1)){
      if(CompareSchemas(arg1, xType)){
          ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Groom_tid"),
                                    nl->SymbolAtom("int")), 
                        nl->TwoElemList(nl->SymbolAtom("Box3d"),
                                      nl->SymbolAtom("rect3"))
                  )
                )
          );
        return result; 
      }else
        return nl->SymbolAtom("schema error"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator createdoor1

*/
ListExpr CreateDoor1TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 6){
      string err = "rel x rel x rtree x attr x attr x attr";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (!listutils::isRelDescription(arg1) || 
      !listutils::isRelDescription(arg2)){
    return listutils::typeError("param1 and param2 should be a relation" );
  }

  ListExpr arg3 = nl->Third(args);
  if(!listutils::isRTreeDescription(arg3) )
    return listutils::typeError("param3 should be an rtree" );


  ListExpr attrName1 = nl->Fourth(args);
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname1, attrType1);
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }

  ListExpr attrName2 = nl->Fifth(args);
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname2, attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type int");
  }

  ListExpr attrName3 = nl->Sixth(args);
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname3, attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"rect3")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type rect3");
  }

      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
//                      nl->FiveElemList(
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Door"),
                                    nl->SymbolAtom("door3d")), 
                        nl->TwoElemList(nl->SymbolAtom("Door_loc"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Door_loc3d"),
                                      nl->SymbolAtom("line3d")), 
                        nl->TwoElemList(nl->SymbolAtom("Doorheight"),
                                      nl->SymbolAtom("real"))
                  )
                )
          );
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                   nl->ThreeElemList(nl->IntAtom(j1),
                                     nl->IntAtom(j2),
                                     nl->IntAtom(j3)),result);

}


/*
TypeMap function for operator createdoor2

*/
ListExpr CreateDoor2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "rel";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  
  if (!listutils::isRelDescription(arg1)){
    return listutils::typeError("param1 should be a relation" );
  }

      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Door"),
                                    nl->SymbolAtom("door3d")), 
                        nl->TwoElemList(nl->SymbolAtom("Door_loc"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Door_loc3d"),
                                      nl->SymbolAtom("line3d")), 
                        nl->TwoElemList(nl->SymbolAtom("Doorheight"),
                                      nl->SymbolAtom("real"))
                  )
                )
          );
    return result;

}

/*
TypeMap function for operator createadjdoor1

*/
ListExpr CreateAdjDoor1TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 7){
      string err = "rel x rel x btree x attr x attr x attr x attr";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (!listutils::isRelDescription(arg1) || 
      !listutils::isRelDescription(arg2)){
    return listutils::typeError("param1 and param2 should be a relation" );
  }

  ListExpr arg3 = nl->Third(args);
  if(!listutils::isBTreeDescription(arg3) )
    return listutils::typeError("param3 should be a btree" );


  ListExpr attrName1 = nl->Fourth(args);
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname1, attrType1);
  if(j1 == 0 || !listutils::isSymbol(attrType1,"door3d")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type door3d");
  }

  ListExpr attrName2 = nl->Fifth(args);
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname2, attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"line")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type line");
  }

  ListExpr attrName3 = nl->Sixth(args);
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname3, attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"int")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");
  }

  ListExpr attrName4 = nl->Nth(7, args);
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(arg2)),
                      aname4, attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"real")){
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type real");
  }
  
      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid"),
                                    nl->SymbolAtom("int")), 
                        nl->TwoElemList(nl->SymbolAtom("Door_tid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Door_tid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                      nl->SymbolAtom("line3d"))
                  )
                )
          );
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                   nl->FourElemList(nl->IntAtom(j1),
                                     nl->IntAtom(j2),
                                     nl->IntAtom(j3),
                                     nl->IntAtom(j4)), result);

}


/*
TypeMap function for operator createadjdoor2

*/
ListExpr CreateAdjDoor2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "rel x rtree ";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (!listutils::isRelDescription(arg1)){
    return listutils::typeError("param1 should be a relation" );
  }
  
  
  ListExpr xType;
  nl->ReadFromString(IndoorGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );

  if(!listutils::isRTreeDescription(arg2) )
    return listutils::typeError("param2 should be an rtree" );


      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
//                      nl->FiveElemList(
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid"),
                                    nl->SymbolAtom("int")), 
                        nl->TwoElemList(nl->SymbolAtom("Door_tid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Door_tid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                      nl->SymbolAtom("line3d"))
                  )
                )
          );
    return  result;

}


/*
TypeMap function for operator path in region 

*/
ListExpr PathInRegionTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "region x point x point";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  if(nl->IsEqual(arg1, "region") && nl->IsEqual(arg2, "point") && 
     nl->IsEqual(arg3, "point"))
      return nl->SymbolAtom("line");

    return listutils::typeError("region x point x point expected" );
}


/*
TypeMap fun for operator createigraph

*/

ListExpr OpTMCreateIGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);
  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc = nl->Third(args);
  ListExpr graph_type = nl->Fourth(args);
  if(!nl->IsEqual(xIdDesc, "int")) return nl->SymbolAtom ( "typeerror" );
  if(!IsRelDescription(xEdgeDesc) || !IsRelDescription(xNodeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(IndoorGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(xNodeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  nl->ReadFromString(IndoorGraph::EdgeTypeInfo, xType);
  if(!CompareSchemas(xEdgeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  if(!(nl->IsAtom(graph_type) && nl->AtomType(graph_type) == SymbolType &&
     nl->SymbolValue(graph_type) == "string"))
    return nl->SymbolAtom ("typeerror");

  return nl->SymbolAtom ( "indoorgraph" );
}


/*
TypeMap function for operator generate ip1

*/
ListExpr GenerateIP1TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "rel x int";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  ListExpr xType;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType); 
  if (listutils::isRelDescription(arg1)){
      if(CompareSchemas(arg1, xType) && 
        nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
        nl->SymbolValue(arg2) == "int" && nl->IsAtom(arg3) && 
        nl->AtomType(arg3) == SymbolType &&
        nl->SymbolValue(arg3) == "bool"){

          ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Loc1"),
                                    nl->SymbolAtom("genloc")), 
                        nl->TwoElemList(nl->SymbolAtom("Loc2"),
                                      nl->SymbolAtom("point3d"))
                  )
                )
          );
        return result; 
      }else
        return nl->SymbolAtom("schema error"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator indoornavigation

*/
ListExpr IndoorNavigationTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 6){
      string err = "indoorgraph x genloc x genloc x rel x btree x int";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  ListExpr arg6 = nl->Sixth(args);
  
  if(!(nl->IsAtom(nl->First(arg1)) && 
       nl->AtomType(nl->First(arg1)) == SymbolType &&
       nl->SymbolValue(nl->First(arg1)) == "indoorgraph")){
      string err = "param1 should be indoorgraph";
      return listutils::typeError(err);
  }
   if(!(nl->IsAtom(nl->First(arg2)) && 
        nl->AtomType(nl->First(arg2)) == SymbolType &&
        nl->SymbolValue(nl->First(arg2)) == "genloc" && 
        nl->IsAtom(nl->First(arg3)) && 
        nl->AtomType(nl->First(arg3)) == SymbolType &&
        nl->SymbolValue(nl->First(arg3)) == "genloc" )){
      string err = "param2 and param3 should be genloc";
      return listutils::typeError(err);
  } 

  if(!listutils::isRelDescription(nl->First(arg4))){
      string err = "param4 should be a relation";
      return listutils::typeError(err);
  }
  
  ListExpr xType;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType); 
  if (!CompareSchemas(nl->First(arg4), xType)){
     string err = "rel schema error";
     return listutils::typeError(err);
  }

  if(!listutils::isBTreeDescription(nl->First(arg5))){
      string err = "param5 should be a btree";
      return listutils::typeError(err);
  }
  
  if(!(nl->IsAtom(nl->First(arg6)) && 
       nl->AtomType(nl->First(arg6)) == SymbolType &&
       nl->SymbolValue(nl->First(arg6)) == "int" )){
      string err = "param6 should be int";
      return listutils::typeError(err);
  }
  
  int n = nl->IntValue(nl->Second(arg6));
  ListExpr result; 
   switch(n){
    case 0: 
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("line3d"))
                  )
                )
          );
        break;
    case 1: 
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Groom_oid"),
                                    nl->SymbolAtom("int")), 
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("groom"))
                  )
                )
          );
        break; 
    case 2: 
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("line3d")),
                        nl->TwoElemList(nl->SymbolAtom("TimeCost"),
                                    nl->SymbolAtom("real"))
                  )
                )
          );
        break;
    default:
      string err = "the value of fifth parameter([0,2]) is not correct";
      return listutils::typeError(err);
  }

  return result; 
}


/*
TypeMap function for operator generate mo1

*/
ListExpr GenerateMO1TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 6){
      string err = "indoorgraph x rel x btree x rtree x int x periods";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args); 
  ListExpr arg5 = nl->Fifth(args);
  ListExpr arg6 = nl->Sixth(args);

  ListExpr xType;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType); 
  if (nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
      nl->SymbolValue(arg1) == "indoorgraph" &&
      listutils::isRelDescription(arg2) && 
      listutils::isBTreeDescription(arg3) && 
      listutils::isRTreeDescription(arg4)){
      if(CompareSchemas(arg2, xType) && 
        nl->IsAtom(arg5) && nl->AtomType(arg5) == SymbolType &&
        nl->SymbolValue(arg6) == "periods"){
//        ListExpr result; 
        if(nl->SymbolValue(arg5) == "int"){
            ListExpr result =
              nl->TwoElemList(
                nl->SymbolAtom("stream"),
                  nl->TwoElemList(
                    nl->SymbolAtom("tuple"),
                      nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip"),
                                      nl->SymbolAtom("mpoint3d"))
                  )
                )
              );
          return result;
        }else if(nl->SymbolValue(arg5) == "real"){
            ListExpr result =
              nl->TwoElemList(
                nl->SymbolAtom("stream"),
                  nl->TwoElemList(
                    nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip"),
                                      nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("GenTrip"),
                                      nl->SymbolAtom("genmo"))
                  )
                )
              );
            return result;
        }

      }else
        return nl->SymbolAtom("schema error"); 
  }
  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator getindoorpaths

*/
ListExpr GetIndoorPathTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "string x int ";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->SymbolValue(arg1) == "string" && nl->SymbolValue(arg2) == "int")
    return nl->SymbolAtom("line3d");
  else
    return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator ref id  

*/
ListExpr RefIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one parameter expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genloc") || nl->IsEqual(arg1, "space") ||
     nl->IsEqual(arg1, "ioref") || 
     nl->IsEqual(arg1, "busstop") || 
     nl->IsEqual(arg1, "busroute") || 
     nl->IsEqual(arg1, "busnetwork") || nl->IsEqual(arg1, "metronetwork") ||
     nl->IsEqual(arg1, "pavenetwork") || 
     nl->IsEqual(arg1, "network") || 
     nl->IsEqual(arg1, "building") || 
     nl->IsEqual(arg1, "indoorinfra") || nl->IsEqual(arg1, "ugenloc"))
    return nl->SymbolAtom("int");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator setref id  

*/
ListExpr SetRefIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo  expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo") || 
    nl->IsEqual(arg1, "genrange") || nl->IsEqual(arg1, "door3d"))
    return nl->TwoElemList(nl->SymbolAtom("stream"),nl->SymbolAtom("int"));

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator at  

*/
ListExpr TMATTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "genmo") && 
    (IsRelDescription(arg2) || nl->SymbolValue(arg2) == "string" ||
     nl->IsEqual(arg2, "genloc"))){
    return nl->SymbolAtom("genmo");
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tmat2  

*/
ListExpr TMAT2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "three parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2, "mreal") &&
     (nl->SymbolValue(arg3) == "string" || nl->SymbolValue(arg3) == "genloc"))
    return nl->SymbolAtom("genmo");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tmat3  

*/
ListExpr TMAT3TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "four parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2, "mreal") &&
     nl->SymbolValue(arg3) == "genloc" && nl->IsEqual(arg4,"string"))
    return nl->SymbolAtom("genmo");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator val  

*/
ListExpr TMValTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "igenloc"))
    return nl->SymbolAtom("genloc");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator inst

*/
ListExpr TMInstTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "igenloc"))
    return nl->SymbolAtom("instant");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator contain 

*/
ListExpr TMContainTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "genmo") && nl->SymbolValue(arg2) == "string")
    return nl->SymbolAtom("bool");

  if(nl->IsEqual(arg1, "genmo") && nl->SymbolValue(arg2) == "int")
    return nl->SymbolAtom("bool");

  if(nl->IsEqual(arg1, "int") && nl->SymbolValue(arg2) == "string")
    return nl->SymbolAtom("bool");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator contain2 

*/
ListExpr TMContain2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "four parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(nl->IsEqual(arg1, "genmo") && nl->SymbolValue(arg2) == "mreal" && 
     nl->IsEqual(arg3, "int") && nl->IsEqual(arg4,"string"))
    return nl->SymbolAtom("bool");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tm minute 

*/
ListExpr TMDurationTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameter expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "periods") && nl->IsEqual(arg2, "string"))
    return nl->SymbolAtom("real");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator tm initial 

*/
ListExpr TMInitialTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one parameter expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "genmo"))
    return nl->SymbolAtom("igenloc");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tm build id 

*/
ListExpr TMBuildIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "int") && nl->IsEqual(arg2, "space"))
    return nl->SymbolAtom("int");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator bcontains

*/
ListExpr TMBContainsTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2,"int"))
    return nl->SymbolAtom("bool");

  return nl->SymbolAtom("typeerror");
}

ListExpr TMBContains2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "three parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2,"mreal") && 
     nl->IsEqual(arg3,"int"))
    return nl->SymbolAtom("bool");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tm room id 

*/
ListExpr TMRoomIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "int") && nl->IsEqual(arg2, "space"))
    return nl->SymbolAtom("int");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tm plus id 

*/
ListExpr TMPlusIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "int") && nl->IsEqual(arg2, "int"))
    return nl->SymbolAtom("int");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tm pass

*/
ListExpr TMPassTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "three parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2, "region") && 
     nl->IsEqual(arg3, "space"))
    return nl->SymbolAtom("bool");

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator tm distance

*/
ListExpr TMDistanceTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "three parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  if(nl->IsEqual(arg1, "genloc") &&  nl->IsEqual(arg3, "space")){
    if(nl->IsEqual(arg2, "point") || nl->IsEqual(arg2, "line"))
    return nl->SymbolAtom("real");
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tm genloc

*/
ListExpr TMGenLocTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "three parameters expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);

  if(nl->IsEqual(arg1, "int") &&  nl->IsEqual(arg2, "real") && 
     nl->IsEqual(arg3, "real")){
    return nl->SymbolAtom("genloc");
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator modeval

*/
ListExpr ModeValTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one parameter expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "genmo") || nl->IsEqual(arg1, "mreal")){
    return nl->SymbolAtom("int");
  }

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator genmoindex

*/
ListExpr GenMOIndexTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one parameter expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "genmo")){
    return nl->SymbolAtom("mreal");
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator deftime 

*/
ListExpr GenMODeftimeTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo") || nl->IsEqual(arg1, "mpoint3d"))
    return nl->SymbolAtom("periods");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator no components  

*/
ListExpr GenMONoComponentsTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo") || 
     nl->IsEqual(arg1, "mpoint3d") ||
     nl->IsEqual(arg1, "genrange") || 
     nl->IsEqual(arg1, "groom") || nl->IsEqual(arg1, "busroute"))
    return nl->SymbolAtom("int");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator lowres  

*/
ListExpr LowResTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo"))
    return nl->SymbolAtom("genmo");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator genmotranslate  

*/
ListExpr GenmoTranslateTypeMap(ListExpr args)
{

  if(nl->ListLength(args) != 2){
      string err = "genmo x duration expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2, "duration"))
  return nl->SymbolAtom("genmo");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator tmtranslate2  

*/
ListExpr TMTranslate2TypeMap(ListExpr args)
{

  if(nl->ListLength(args) != 2){
      string err = "periods x duration expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(nl->IsEqual(arg1, "periods") && nl->IsEqual(arg2, "duration"))
  return nl->SymbolAtom("periods");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator getmode

*/
ListExpr GetModeTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo")){
      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string"))
                  )
                )
          );
    return result;
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator getref

*/
ListExpr GetRefTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo")){
      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("RefId"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Label"),
                                    nl->SymbolAtom("string"))
                  )
                )
          );
    return result;
  }  

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator atinstant

*/
ListExpr AtInstantTypeMap(ListExpr args)
{
  if ( nl->ListLength( args ) == 2 ){
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Instant::BasicType() ) )
    {

      if( nl->IsEqual( arg1, GenMO::BasicType() ) )
        return nl->SymbolAtom( IGenLoc::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}

/*
TypeMap function for operator atperiods

*/
ListExpr AtPeriodsTypeMap(ListExpr args)
{
  if ( nl->ListLength( args ) == 2 ){
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, Periods::BasicType() ) )
    {

      if( nl->IsEqual( arg1, GenMO::BasicType() ) )
        return nl->SymbolAtom( GenMO::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}



/*
TypeMap function for operator mapgenmo

*/
ListExpr MapGenMOTypeMap(ListExpr args)
{
  if ( nl->ListLength( args ) == 2 ){
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, MPoint::BasicType() ) )
    {

      if( nl->IsEqual( arg1, GenMO::BasicType() ) )
        return nl->SymbolAtom( MPoint::BasicType() );
    }
  }
  return nl->SymbolAtom( Symbol::TYPEERROR() );
}


/*
TypeMap function for operator tmunits  

*/
ListExpr TMUnitsTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo  expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo"))
    return nl->TwoElemList(nl->SymbolAtom("stream"),nl->SymbolAtom("ugenloc"));

  return nl->SymbolAtom("typeerror");
}

ListExpr GetLocTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "genmo  x bool expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "ugenloc") && nl->IsEqual(arg2, "bool"))
    return nl->SymbolAtom( Point::BasicType() );

  return nl->SymbolAtom("typeerror");
}

/*
get the traffic value from generic moving objects

*/
ListExpr TMTrafficTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "rel x periods x rel  x bool expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  

  if (!(listutils::isRelDescription(arg1)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(GenMObject::GenMOTrip, xType1);

  if(!(CompareSchemas(arg1, xType1)))
    return nl->SymbolAtom ( "typeerror" );

  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }
  
  if (!(listutils::isRelDescription(arg3)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(GenMObject::RoadSegment, xType2);

  if(!(CompareSchemas(arg3, xType2)))
    return nl->SymbolAtom ( "typeerror" );

  if(!nl->IsEqual(arg4, "bool")){
      string err = "the fourth parameter should be bool";
      return listutils::typeError(err);
  }
  
      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("SID"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Res"),
                                    nl->SymbolAtom("int"))
                  )
                )
          );
    return result;
}

/*
TypeMap function for operator the space

*/
ListExpr TheSpaceTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "int expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "int")){
    return nl->SymbolAtom("space");
  }

  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator genmo tm list

*/
ListExpr GenMOTMListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "bool")){
    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Type"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string"))
                  )
                )
          );
    return result;
  }else
    return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator generate genmo

*/
ListExpr GenerateGMOListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "four input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!(nl->IsEqual(arg3, "real") && nl->IsEqual(arg4, "int"))){
      string err = "the 3 paramenter should be real and 4 should be int";
      return listutils::typeError(err);
  }

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate car

*/
ListExpr GenerateCarListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "four input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  

  if(!(nl->IsEqual(arg3, "real") )){
      string err = "the 3 paramenter should be real";
      return listutils::typeError(err);
  }

  ListExpr arg4 = nl->Fourth(args);
  if (!(listutils::isRelDescription(arg4)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(GenMObject::StreetSpeedInfo, xType1);

  if(!(CompareSchemas(arg4, xType1)))
    return nl->SymbolAtom ( "typeerror" );

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                    nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mgpoint"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate genmo benchmark 

*/
ListExpr GenerateGMOBench1ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 6){
      string err = "six input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);

  if(!(nl->IsEqual(arg3, "real"))){
      string err = "the 3 paramenter should be real ";
      return listutils::typeError(err);
  }
 
  ListExpr arg4 = nl->Fourth(args);

  if(!IsRelDescription(arg4))
    return listutils::typeError("para4 should be a relation");

  ListExpr xType;
  nl->ReadFromString(GenMObject::BenchModeDISTR, xType);
  if(!CompareSchemas(arg4, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg5 = nl->Fifth(args);
  if(!IsRelDescription(arg5))
    return listutils::typeError("para5 should be a relation");

  ListExpr arg6 = nl->Sixth(args);
  if(!IsRelDescription(arg6))
    return listutils::typeError("para6 should be a relation");
  
  
    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate genmo benchmark 

*/
ListExpr GenerateGMOBench2ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 5){
      string err = "five input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);

  if(!(nl->IsEqual(arg3, "real"))){
      string err = "the 3 paramenter should be real ";
      return listutils::typeError(err);
  }
 
  ListExpr arg4 = nl->Fourth(args);

  if(!IsRelDescription(arg4))
    return listutils::typeError("para4 should be a relation");

  ListExpr arg5 = nl->Fifth(args);
  if(!nl->IsEqual(arg5, "string")){
      string err = "the fifth parameter should be string";
      return listutils::typeError(err);
  }

  
    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate genmo benchmark 

*/
ListExpr GenerateGMOBench3ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 5){
      string err = "five input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);

  if(!(nl->IsEqual(arg3, "real"))){
      string err = "the 3 paramenter should be real ";
      return listutils::typeError(err);
  }
 
  ListExpr arg4 = nl->Fourth(args);

  if(!IsRelDescription(arg4))
    return listutils::typeError("para4 should be a relation");

  ListExpr xType;
  nl->ReadFromString(GenMObject::NNBuilding, xType);
  if(!CompareSchemas(arg4, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg5 = nl->Fifth(args);

  if(!listutils::isRTreeDescription(arg5))
    return listutils::typeError("para5 should be an rtree");

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate genmo benchmark 

*/
ListExpr GenerateGMOBench4ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 6){
      string err = "six input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);

  if(!(nl->IsEqual(arg3, "real"))){
      string err = "the 3 paramenter should be real ";
      return listutils::typeError(err);
  }
 
 
  ListExpr arg4 = nl->Fourth(args);

  if(!IsRelDescription(arg4))
    return listutils::typeError("para4 should be a relation");

  ListExpr xType1;
  nl->ReadFromString(GenMObject::BenchModeDISTR, xType1);
  if(!CompareSchemas(arg4, xType1))return nl->SymbolAtom ( "typeerror" );

  
  ListExpr arg5 = nl->Fifth(args);

  if(!IsRelDescription(arg5))
    return listutils::typeError("para5 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(GenMObject::NNBuilding, xType2);
  if(!CompareSchemas(arg5, xType2))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg6 = nl->Sixth(args);

  if(!listutils::isRTreeDescription(arg6))
    return listutils::typeError("para6 should be an rtree");

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );

//     ListExpr result =
//           nl->TwoElemList(
//               nl->SymbolAtom("stream"),
//                 nl->TwoElemList(
// 
//                   nl->SymbolAtom("tuple"),
//                       nl->ThreeElemList(
//                       nl->TwoElemList(nl->SymbolAtom("Trip1"),
//                                     nl->SymbolAtom("genmo")),
//                         nl->TwoElemList(nl->SymbolAtom("Trip2"),
//                                     nl->SymbolAtom("mpoint")),
//                         nl->TwoElemList(nl->SymbolAtom("IndoorTrip"),
//                                     nl->SymbolAtom("mpoint3d"))
// 
//                   )
//                 )
//           );

    return result;
}


/*
TypeMap function for operator generate genmo benchmark 

*/
ListExpr GenerateGMOBench5ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 5){
      string err = "five input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);

  if(!(nl->IsEqual(arg3, "real"))){
      string err = "the 3 paramenter should be real ";
      return listutils::typeError(err);
  }
 
  ListExpr arg4 = nl->Fourth(args);
  if(!IsRelDescription(arg4))
    return listutils::typeError("para5 should be a relation");

  ListExpr arg5 = nl->Fifth(args);
  if(!IsRelDescription(arg5))
    return listutils::typeError("para6 should be a relation");
  
  
    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );


    return result;
}

/*
TypeMap function for operator get rg ndoes

*/
ListExpr GetRGNodesTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "one input parameter expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "network")){
      string err = "the first parameter should be network";
      return listutils::typeError(err);
  }
 
    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Jun_id"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Jun_gp"),
                                      nl->SymbolAtom("gpoint")),
                        nl->TwoElemList(nl->SymbolAtom("Jun_p"),
                                      nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("Rid"),
                                      nl->SymbolAtom("int"))
                  )
                )
          );

    return result;

}


/*
TypeMap function for operator get rg edges

*/
ListExpr GetRGEdges1TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "two input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  ListExpr xType;
  nl->ReadFromString(RoadGraph::RGNodeTypeInfo, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Jun_id1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Jun_id2"),
                                      nl->SymbolAtom("int"))
                  )
                )
          );

    return result;

}


/*
TypeMap function for operator get rg edges

*/
ListExpr GetRGEdges2TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two input parameters expected";
      return listutils::typeError(err);
  }


  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "network")){
      string err = "the first parameter should be network";
      return listutils::typeError(err);
  }
  
  ListExpr arg2 = nl->Second(args);
  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  ListExpr xType;
  nl->ReadFromString(RoadGraph::RGNodeTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Jun_id1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Jun_id2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Path1"),
                                      nl->SymbolAtom("gline")),
                        nl->TwoElemList(nl->SymbolAtom("Path2"),
                                      nl->SymbolAtom("sline"))
                  )
                )
          );

    return result;

}


/*
TypeMap function for operator get pavement edges

*/
ListExpr GetPaveEdges3TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 4){
      string err = "four input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr xType;

  ListExpr arg1 = nl->First(args);
  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  nl->ReadFromString(Network::routesTypeInfo, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg2 = nl->Second(args);
  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  nl->ReadFromString(OSMPaveGraph::OSMGraphPaveNode, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg3 = nl->Third(args);
  if(!listutils::isBTreeDescription(arg3))
    return listutils::typeError("para3 should be a btree");

  
  ListExpr arg4 = nl->Fourth(args);
  if(!IsRelDescription(arg4))
    return listutils::typeError("para4 should be a relation");

  nl->ReadFromString(OSMPaveGraph::OSMGraphPaveNode, xType);
  if(!CompareSchemas(arg4, xType))return nl->SymbolAtom ( "typeerror" );

  
    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Jun_id1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Jun_id2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Path1"),
                                      nl->SymbolAtom("gline")),
                        nl->TwoElemList(nl->SymbolAtom("Path2"),
                                      nl->SymbolAtom("sline"))
                  )
                )
          );

    return result;

}

/*
TypeMap function for operator get connections inside a region

*/
ListExpr GetPaveEdges4TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr xType;

  ListExpr arg1 = nl->First(args);
  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  nl->ReadFromString(OSM_Data::OSMNodeTmp, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg2 = nl->Second(args);
  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  nl->ReadFromString(OSMPavement::OSMPaveRegion, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Jun_id1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Jun_id2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Path1"),
                                      nl->SymbolAtom("gline")),
                        nl->TwoElemList(nl->SymbolAtom("Path2"),
                                      nl->SymbolAtom("sline"))
                  )
                )
          );

    return result;

}

/*
TypeMap function for operator create osm pavement environment

*/
ListExpr TheOSMPaveTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 3){
      string err = "three input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if((nl->SymbolValue(arg1) == "int") == false){
    return nl->SymbolAtom("typeerror: param1 should be int");
  }
  
  ListExpr xType;

  ListExpr arg2 = nl->Second(args);
  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  nl->ReadFromString(OSMPavement::OSMPaveLine, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr arg3 = nl->Third(args);
  if(!IsRelDescription(arg3))
    return listutils::typeError("para3 should be a relation");

  nl->ReadFromString(OSMPavement::OSMPaveRegion, xType);
  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );


  return nl->SymbolAtom("osmpavenetwork");

}

/*
type map for operator createosmgraph 

*/
ListExpr OpTMOSMPaveGraphTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);
  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc = nl->Third(args);

  if(!nl->IsEqual(xIdDesc, "int")) return nl->SymbolAtom ( "typeerror" );
  if(!IsRelDescription(xNodeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(OSMPaveGraph::OSMGraphPaveNode, xType1);
  if(!CompareSchemas(xNodeDesc, xType1))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(xEdgeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(OSMPaveGraph::OSMGraphPaveEdge, xType2);
  if(!CompareSchemas(xEdgeDesc, xType2))return nl->SymbolAtom ( "typeerror" );

  return nl->SymbolAtom ( "osmpavegraph" );
}

/*
type map for operator osmlocmap 

*/
ListExpr OpTMOSMLocMapTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);


  if(!IsRelDescription(param1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(OSM_Data::OSMPOILine, xType1);
  if(!CompareSchemas(param1, xType1))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(param2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(OSM_Data::OSMPOIRegion, xType2);
  if(!CompareSchemas(param2, xType2))return nl->SymbolAtom ( "typeerror" );

      ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Loc1"),
                                      nl->SymbolAtom("genloc")),
                        nl->TwoElemList(nl->SymbolAtom("Loc2"),
                                      nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("Type"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("OldLoc"),
                                      nl->SymbolAtom("point"))

                  )
                )
          );

    return result;
}

/*
type map for operator osm path

*/
ListExpr OpTMOSMPathTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);
  ListExpr param3 = nl->Third(args);

  if(!IsRelDescription(param1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(OSM_Data::OSMPaveQueryLoc, xType1);
  if(!CompareSchemas(param1, xType1))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(param2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(OSM_Data::OSMPaveQueryLoc, xType2);
  if(!CompareSchemas(param2, xType2))return nl->SymbolAtom ( "typeerror" );


  if(nl->IsEqual(param3, "osmpavenetwork")){
    return nl->SymbolAtom ( "line" );

//       return  nl->TwoElemList(
//               nl->SymbolAtom("stream"),
//                 nl->TwoElemList(
//                   nl->SymbolAtom("tuple"),
//                       nl->TwoElemList(
//                         nl->TwoElemList(nl->SymbolAtom("Loc"),
//                                       nl->SymbolAtom("point")),
//                         nl->TwoElemList(nl->SymbolAtom("Oid"),
//                                       nl->SymbolAtom("int"))
//                   )
//                 )
//           );
  }else{
    return nl->SymbolAtom ( "typeerror" );
  }

}

/*
type map for operator creatergraph 

*/
ListExpr OpTMCreateRoadGraphTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);
  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc1 = nl->Third(args);
  ListExpr xEdgeDesc2 = nl->Fourth(args); 


  if(!nl->IsEqual(xIdDesc, "int")) return nl->SymbolAtom ( "typeerror" );
  if(!IsRelDescription(xNodeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(RoadGraph::RGNodeTypeInfo, xType1);
  if(!CompareSchemas(xNodeDesc, xType1))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(xEdgeDesc1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(RoadGraph::RGEdgeTypeInfo1, xType2);
  if(!CompareSchemas(xEdgeDesc1, xType2))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(xEdgeDesc2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType3;
  nl->ReadFromString(RoadGraph::RGEdgeTypeInfo2, xType3);
  if(!CompareSchemas(xEdgeDesc2, xType3))return nl->SymbolAtom ( "typeerror" );


  return nl->SymbolAtom ( "roadgraph" );
}


/*
type map for operator shortest path tm

*/
ListExpr OpTMShortestPathTMTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);
  ListExpr param3 = nl->Third(args);
  ListExpr param4 = nl->Fourth(args);


  if(!nl->IsEqual(param1, "gpoint")) return nl->SymbolAtom ( "typeerror" );
  if(!nl->IsEqual(param2, "gpoint")) return nl->SymbolAtom ( "typeerror" );
  if(!nl->IsEqual(param3, "roadgraph")) return nl->SymbolAtom ( "typeerror" );
  if(!nl->IsEqual(param4, "network")) return nl->SymbolAtom ( "typeerror" );

   return nl->SymbolAtom ( "gline" );;
}

/*
TypeMap function for operator navigation1

*/
ListExpr Navigation1ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 7){
      string err = "seven input parameter expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);

  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  ListExpr xType;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );


  ListExpr arg3 = nl->Third(args);
  if(!IsRelDescription(arg3))
    return listutils::typeError("para3 should be a relation");

  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );


  ListExpr arg4 = nl->Fourth(args);
  if(!(nl->IsAtom(arg4) && nl->AtomType(arg4) == SymbolType &&
       nl->SymbolValue(arg4) == "instant")){
      string err = "param4 should be instant";
      return listutils::typeError(err);
  }

  
  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);

  if(!(CompareSchemas(arg5, xType2)))
    return nl->SymbolAtom ( "typeerror" );


  
  ListExpr arg6 = nl->Sixth(args);
  if (!(listutils::isRelDescription(arg6)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType1);

  if(!(CompareSchemas(arg6, xType1)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg7 = nl->Nth(7, args);
  if(!listutils::isRTreeDescription(arg7))
    return listutils::typeError("para7 should be a rtree");

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Loc1"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("Loc2"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                    nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                    nl->SymbolAtom("mpoint"))
                  )
                )
          );

    return result;
}

/*
TypeMap function for operator trajectory

*/
ListExpr TMTrajectoryTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "mpoint3d expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "mpoint3d"))
    return nl->SymbolAtom("line3d");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator trajectory

*/
ListExpr GenTrajectoryTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "genmo x space expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  
  if(nl->IsEqual(arg1, "genmo") && nl->IsEqual(arg2, "space"))
    return nl->SymbolAtom("genrange");

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator genrangevisible 

*/
ListExpr GenRangeVisibleTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "genrange x space expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  
  if(nl->IsEqual(arg1, "genrange") && nl->IsEqual(arg2, "space")){

      ListExpr res = 
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("line")))
                )
          );

      return res;
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator addinfragraph

*/
ListExpr AddInfraGraphTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "busnetwork") && nl->IsEqual(arg2, "busgraph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("BusNetworkId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("BusGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  if(nl->IsEqual(arg1, "pavenetwork") && nl->IsEqual(arg2, "dualgraph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("PavementId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("DualGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  if(nl->IsEqual(arg1, "pavenetwork") && nl->IsEqual(arg2, "visualgraph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("PavementId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("VisibilityGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  
  if(nl->IsEqual(arg1, "building") && nl->IsEqual(arg2, "indoorgraph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("BuildingId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("IndoorGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }

  if(nl->IsEqual(arg1, "metronetwork") && nl->IsEqual(arg2, "metrograph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("MetroNetworkId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("MetroGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }

  if(nl->IsEqual(arg1, "osmpavenetwork") && nl->IsEqual(arg2, "osmpavegraph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("OSMPaveNetworkId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("OSMPaveGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  
  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator putinfra

*/
ListExpr PutInfraTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "space") && nl->IsEqual(arg2, "network")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("RoadNetworkId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  if(nl->IsEqual(arg1, "space") && nl->IsEqual(arg2, "pavenetwork")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("PavementId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  if(nl->IsEqual(arg1, "space") && nl->IsEqual(arg2, "busnetwork")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("BusNetworkId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  
  if(nl->IsEqual(arg1, "space") && nl->IsEqual(arg2, "metronetwork")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("MetroNetworkId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }
  
  if(nl->IsEqual(arg1, "space") && nl->IsEqual(arg2, "indoorinfra")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("IndoorInfraId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }

  if(nl->IsEqual(arg1, "space") && nl->IsEqual(arg2, "roadgraph")){

      ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("RoadGraphId"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
  }


  return nl->SymbolAtom("typeerror");
}


/*
TypeMap function for operator putrel

*/
ListExpr PutRelTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsEqual(arg1, "space")){

     ListExpr arg2 = nl->Second(args);
     if(!IsRelDescription(arg2)) return nl->SymbolAtom ( "typeerror" );

     ListExpr xType;
     //////////////////street speed ////////////////////////
     nl->ReadFromString(GenMObject::StreetSpeedInfo, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("SpeedRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );

//        return reslist;
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::SPEED_REL)),
                reslist);
     }
     
     /////////////////tri new relation  /////////////////////
     nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("TriangleRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::TRINEW_REL)),
                reslist);
     }

     /////////////////dual graph node + route id /////////////////////
     nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("DGNodeRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::DGNODE_REL)),
                reslist);
     }

    /////////////////bus stops and pavement /////////////////////
     nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("BSPaveRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::BSPAVESORT_REL)),
                reslist);
     }
      ////////////////metro stops and pavement/////////////////
     nl->ReadFromString(MetroNetwork::MetroPaveTypeInfo, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("MSPaveRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::MSPAVE_REL)),
                reslist);
     }

     ////////////////bus stops and buildings/////////////////
     nl->ReadFromString(GenMObject::BuildingInfoB, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("BSBuildRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::BSBUILD_REL)),
                reslist);
     }
     ////////////////metro stops and buildings/////////////////
     nl->ReadFromString(GenMObject::BuildingInfoM, xType);
     if(CompareSchemas(arg2, xType)){
        ListExpr reslist = nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(
              nl->SymbolAtom("tuple"),
            nl->TwoElemList(
              nl->TwoElemList(nl->SymbolAtom("SpaceId"),
                            nl->SymbolAtom("int")),
              nl->TwoElemList(nl->SymbolAtom("MSBuildRelNo"),
                            nl->SymbolAtom("int"))
          )
        )
      );
      return nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(Space::MSBUILD_REL)),
                reslist);
     }

  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap function for operator gettinfra

*/
ListExpr GetInfraTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "two parameters expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(nl->First(arg1), "space") && 
    listutils::isSymbol(nl->First(arg2), CcString::BasicType())){
    string type  = nl->StringValue(nl->Second(arg2));
    if(GetSymbol(type) == IF_LINE){//////////road network 
      ListExpr xType;
      nl->ReadFromString(Network::routesTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_FREESPACE){ ///////free space 
      ListExpr xType;
      nl->ReadFromString(Space::FreeSpaceTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_REGION){///////pavement infrastructure 
      ListExpr xType;
//      nl->ReadFromString(Pavement::PaveTypeInfo, xType);
      nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_BUSSTOP){//////////bus stops 
      ListExpr xType;
      nl->ReadFromString(BusNetwork::BusStopsInternalTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_BUSROUTE){ ////bus routes 
      ListExpr xType;
      nl->ReadFromString(BusNetwork::BusRoutesTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_BUS){ ///////////////bus trips 
      ListExpr xType;
      nl->ReadFromString(BusNetwork::BusTripsTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_METROSTOP){ //// metro stops
      ListExpr xType;
      nl->ReadFromString(MetroNetwork::MetroStopsTypeInfo, xType);
      return xType;
    }else if(GetSymbol(type) == IF_METROROUTE){ ///metro routes 
      ListExpr xType;
      nl->ReadFromString(MetroNetwork::MetroRoutesTypeInfo, xType);
      return xType;
    }else if(GetSymbol(type) == IF_METRO){ //metro trips 
      ListExpr xType;
      nl->ReadFromString(MetroNetwork::MetroTripTypeInfo, xType);
      return xType;
    }else if(GetSymbol(type) == IF_INDOOR){//indoor, outdoor area for buildings
      ListExpr xType;
      nl->ReadFromString(IndoorInfra::BuildingType_Info, xType);
      return xType;

    }else if(GetSymbol(type) == IF_INDOORPATH){//building path to pavement 

      ListExpr xType;
      nl->ReadFromString(IndoorInfra::BuildingPath_Info, xType);
      return xType;

    }else{
      string err = "infrastructure type error";
      return listutils::typeError(err);
    }
  }

  return nl->SymbolAtom("typeerror");
}

/*
TypeMap fun for operator checksline

*/

ListExpr OpTMCheckSlineTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First( args );
  ListExpr param2 = nl->Second( args );

  if(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
      nl->SymbolValue(param1) == "sline" &&
      nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&
      nl->SymbolValue(param2) == "int")

  {
    return nl->SymbolAtom ( "sline" );
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator boundaryregion

*/

ListExpr OpTMModifyBoundaryTypeMap ( ListExpr args )
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

ListExpr OpTMSegment2RegionTypeMap ( ListExpr args )
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
                      nl->TwoElemList(nl->SymbolAtom("Oid"),
                                    nl->SymbolAtom("int")),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Road1"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("Road2"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("Inborder"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("Paveroad1"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("Paveroad2"),
                                    nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("Outborder"),
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

ListExpr OpTMPaveRegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 7 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr attrName = nl->Third(args);
  ListExpr param4 = nl->Fourth(args);
  ListExpr attrName1 = nl->Fifth(args);
  ListExpr attrName2 = nl->Sixth(args);
  ListExpr param7 = nl->Nth(7, args);

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
        nl->SymbolValue(param1) == "network" &&
        nl->IsAtom(param7) && nl->AtomType(param7) == SymbolType &&
        nl->SymbolValue(param7) == "int"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Pavement1"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("Pavement2"),
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

ListExpr OpTMJunRegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 7 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr attrName1 = nl->Third(args);
  ListExpr attrName2 = nl->Fourth(args);
  ListExpr param5 = nl->Fifth(args);
  ListExpr param6 = nl->Sixth(args);
  ListExpr attrName3 = nl->Nth(7, args);

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

  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname3,attrType3);


  if(j3 == 0 || !listutils::isSymbol(attrType3,"region")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
  }

    if (listutils::isRelDescription(param2) &&
        listutils::isRelDescription(param6) &&
        nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "network" &&
        nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&
        nl->SymbolValue(param5) == "int"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),

                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Crossreg"),
                                      nl->SymbolAtom("region"))

                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),

//     nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);
     nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),result);
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator decomposeregion,
decompose the faces of the input region

*/

ListExpr OpTMDecomposeRegionTypeMap ( ListExpr args )
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
                        nl->TwoElemList(nl->SymbolAtom("Id"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Covarea"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );

    return result;
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator fillpavement

*/

ListExpr OpTMFillPavementTypeMap ( ListExpr args )
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
//                      nl->ThreeElemList(
//                      nl->FiveElemList(
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("rid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("rid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("junscurve"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("junsregion1"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("junsregion2"),
                                      nl->SymbolAtom("region")),
                        nl->TwoElemList(nl->SymbolAtom("junsregion"),
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
TypeMap fun for operator getpave1
decompose the pavement of one road into a set of subregions

*/

ListExpr OpTMGetPaveNode1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr attrName1 = nl->Third(args);
  ListExpr attrName2 = nl->Fourth(args);
  ListExpr attrName3 = nl->Fifth(args);

  if(!(listutils::isRelDescription(param2) &&
        nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror" );
  }
  
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
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

  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);


  if(j3 == 0 || !listutils::isSymbol(attrType3,"region")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
  }

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Pavement"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
//     nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);
    nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),result);

}

/*
TypeMap fun for operator getpavenode1
get the common area(line) of two pavements

*/

ListExpr OpTMGetPaveEdge1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);
  ListExpr param3 = nl->Third(args);
  ListExpr attrName1 = nl->Fourth(args);
  ListExpr attrName2 = nl->Fifth(args);
  ListExpr attrName3 = nl->Sixth(args);


  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);


  if(j2 == 0 || !listutils::isSymbol(attrType2,"int")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);


  if(j3 == 0 || !listutils::isSymbol(attrType3,"region")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
  }


    if (listutils::isRelDescription(param2) &&
        nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "network" &&
        listutils::isBTreeDescription(param3)){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Oid2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Commarea"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
//     nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);
    nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),result);

  }
  return nl->SymbolAtom ( "typeerror" );
}
/*
TypeMap fun for operator getpavenode2
decompose the zebra crossing into a set of subregions

*/

ListExpr OpTMGetPaveNode2TypeMap ( ListExpr args )
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

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
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
        nl->SymbolValue(param1) == "int"){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Pavement"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
     nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);

  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getpavenode2
get the common area(line) of two pavements

*/

ListExpr OpTMGetPaveEdge2TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
//  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->First(args);
  ListExpr param3 = nl->Second(args);
  ListExpr param4 = nl->Third(args);
  ListExpr attrName1 = nl->Fourth(args);
  ListExpr attrName2 = nl->Fifth(args);
  ListExpr attrName3 = nl->Sixth(args);


  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);


  if(j2 == 0 || !listutils::isSymbol(attrType2,"int")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);


  if(j3 == 0 || !listutils::isSymbol(attrType3,"region")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
  }



    if (listutils::isRelDescription(param2) &&
        listutils::isRelDescription(param3) &&
        listutils::isBTreeDescription(param4)){

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Oid2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Commarea"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );

    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
//     nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);
    nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),result);

  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator triangulate
decomplse a polygon into a set of triangles

*/

ListExpr OpTMTriangulateTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  if (nl->IsEqual(nl->First(args), "region")){
    return nl->TwoElemList(nl->SymbolAtom("stream"),
                           nl->SymbolAtom("region"));
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator convex
detect whether a polygon is convex or concave

*/

ListExpr OpTMConvexTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  if (nl->IsEqual(nl->First(args), "region")){
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator geospath
return the geometric shortest path for two points

*/

ListExpr OpTMGeospathTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  if (nl->IsEqual(nl->First(args), "point") &&
//      nl->IsEqual(nl->Second(args), "point") &&
      (nl->IsEqual(nl->Second(args), "point")||
       nl->IsEqual(nl->Second(args), "line") )&&
      nl->IsEqual(nl->Third(args), "region")){


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Spath"),
                                    nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("Channel"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );
    return result;
  }
  return nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator createdualgraph

*/

ListExpr OpTMCreateDGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);

  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc = nl->Third(args);

  if(!nl->IsEqual(xIdDesc, "int")) return nl->SymbolAtom ( "typeerror" );
  if(!IsRelDescription(xEdgeDesc) || !IsRelDescription(xNodeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(xNodeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  nl->ReadFromString(DualGraph::EdgeTypeInfo, xType);
  if(!CompareSchemas(xEdgeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  return nl->SymbolAtom ( "dualgraph" );
}


/*
TypeMap fun for operator walkspold with dual graph and visual graph 

*/

ListExpr OpTMWalkSPOldTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg0 = nl->First(args);
  ListExpr arg1 = nl->Second(args);
  ListExpr arg2 = nl->Third(args);
  ListExpr arg3 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  
  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");
    
  ListExpr xType;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(arg3))
  return listutils::typeError("para3 should be a relation");
  
  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(arg5))
  return listutils::typeError("para5 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg5, xType2))return nl->SymbolAtom ( "typeerror" );

  if(nl->IsAtom(arg0) && nl->AtomType(arg0) == SymbolType &&
     nl->SymbolValue(arg0) == "dualgraph"&&
     nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "visualgraph" ){


    return nl->SymbolAtom("line");
  }

  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator walksp with pavement infrastructure 

*/

ListExpr OpTMWalkSPTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  
  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");
    
  ListExpr xType;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(arg3))
  return listutils::typeError("para3 should be a relation");
  
  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(arg4))
  return listutils::typeError("para4 should be a relation");
  
  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg4, xType2))return nl->SymbolAtom ( "typeerror" );

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "pavenetwork"){
    return nl->SymbolAtom("line");
  }

  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator walksp with pavement infrastructure for debuging

*/

ListExpr OpTMWalkSPDebugTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  ListExpr xType;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(arg3))
  return listutils::typeError("para3 should be a relation");

  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(arg4))
  return listutils::typeError("para4 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg4, xType2))return nl->SymbolAtom ( "typeerror" );


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "pavenetwork"){
           ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Loc"),
                                    nl->SymbolAtom("point"))
                  )
                )
          );
    return result;
  }

  return nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator testwalksp

*/

ListExpr OpTMTestWalkSPTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg0 = nl->First(args);
  ListExpr arg1 = nl->Second(args);
  ListExpr arg2 = nl->Third(args);
  ListExpr arg3 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  
  if(!IsRelDescription(arg2))
    return listutils::typeError("para3 should be a relation");
  
  ListExpr xType;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(arg3))
    return listutils::typeError("para4 should be a relation");
  
  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(arg5))
    return listutils::typeError("para5 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg5, xType2))return nl->SymbolAtom ( "typeerror" );



  if(nl->IsAtom(arg0) && nl->AtomType(arg0) == SymbolType &&
     nl->SymbolValue(arg0) == "dualgraph"&&
     nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "visualgraph" ){

       ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("oid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("oid2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("connection"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
    return result;

  }

  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator pave loc togp

*/

ListExpr OpTMPaveLocToGPTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
 
  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  ListExpr xType1;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType1);
  if(!CompareSchemas(arg1, xType1))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType2);
  if(!CompareSchemas(arg2, xType2))return nl->SymbolAtom ( "typeerror" );

  if(!listutils::isBTreeDescription(arg3))
    return listutils::typeError("para3 should be a btree");

  if(nl->IsAtom(arg4) && nl->AtomType(arg4) == SymbolType &&
     nl->SymbolValue(arg4) == "network"){

       ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("FS_loc"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("RN_loc"),
                                    nl->SymbolAtom("gpoint"))

                  )
                )
          );
    return result;

  }

  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator setpave rid 

*/

ListExpr OpTMSetPaveRidTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
 
  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  ListExpr xType1;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType1);
  if(!CompareSchemas(arg1, xType1))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(arg2))
    return listutils::typeError("para2 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType2);
  if(!CompareSchemas(arg2, xType2))return nl->SymbolAtom ( "typeerror" );

  if(!listutils::isRTreeDescription(arg3))
    return listutils::typeError("para3 should be a rtree");

        ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Pavement"),
                                      nl->SymbolAtom("region"))
                  )
                )
          );
      return result;
}

/*
TypeMap fun for operator generatewp

*/

ListExpr OpTMGenerateWPTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);


  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  ListExpr xType;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );

  if(nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "int" ){

       ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Loc1"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("Loc2"),
                                    nl->SymbolAtom("point"))
                  )
                )
          );
    return result;
  }

  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator zval

*/

ListExpr OpTMZvalTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "point"){
      return  nl->SymbolAtom ( "int" );
  }

  return nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator zcurve

*/

ListExpr OpTMZcurveTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr attrName1 = nl->Second(args);

  if (!(listutils::isRelDescription(arg1)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(arg1)),
                      aname1, attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"point")){
      return listutils::typeError("attr name " + aname1 + " not found "
                      "or not of type point");
  }

  ListExpr result =   nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("Curve"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
             nl->OneElemList(nl->IntAtom(j1)),result);

}

/*
TypeMap fun for operator regvertex

*/

ListExpr OpTMRegVertexTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "region"){
      ListExpr result =   nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("Cycleno"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Vertex"),
                                    nl->SymbolAtom("point"))
                  )
                )
          );
      return result;
  }
  return  nl->SymbolAtom ( "typeerror" );

}

/*
TypeMap fun for operator triangulationnew

*/

ListExpr OpTMTriangulationNewTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "region"){
      ListExpr result =   nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("V1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("V2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("V3"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Centroid"),
                                    nl->SymbolAtom("point"))
                  )
                )
          );
      return result;
  }
  return  nl->SymbolAtom ( "typeerror" );

}

/*
TypeMap fun for operator triangulationext

*/

ListExpr OpTMTriangulationExtTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "region"){
      ListExpr result =   nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("V1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("V2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("V3"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Triangle"),
                                    nl->SymbolAtom("region"))
                  )
                )
          );
      return result;
  }
  return  nl->SymbolAtom ( "typeerror" );

}

/*
TypeMap fun for operator getdgedge

*/

ListExpr OpTMGetDgEdgeTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (!(listutils::isRelDescription(arg1)))
    return nl->SymbolAtom ( "typeerror" );
  if (!(listutils::isRelDescription(arg2)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(DualGraph::TriangleTypeInfo1, xType1);
  if(!CompareSchemas(arg1, xType1))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo2, xType2);
  if(!CompareSchemas(arg2, xType2))return nl->SymbolAtom ( "typeerror" );


  ListExpr result =   nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->ThreeElemList(
                       nl->TwoElemList(nl->SymbolAtom("Oid1"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Oid2"),
                                    nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Commarea"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
  return result;
}

/*
TypeMap fun for operator smcdgte

*/

ListExpr OpTMSMCDGTETypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (!(listutils::isRelDescription(arg1)))
    return nl->SymbolAtom ( "typeerror" );
  if (!(listutils::isRTreeDescription(arg2)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType1);
  if(!CompareSchemas(arg1, xType1))return nl->SymbolAtom ( "typeerror" );


  ListExpr result =   nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->ThreeElemList(
                       nl->TwoElemList(nl->SymbolAtom("Oid1"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Oid2"),
                                    nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Commarea"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
  return result;
}

/*
TypeMap fun for operator getvnode

*/

ListExpr OpTMGetVNodeTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  ListExpr arg6 = nl->Sixth(args);

  ListExpr xType1;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType1);
  if(!CompareSchemas(arg2, xType1))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg3, xType2))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType3;
  nl->ReadFromString(VisualGraph::NodeTypeInfo, xType3);
  if(!CompareSchemas(arg4, xType3))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType4;
  nl->ReadFromString(DualGraph::TriangleTypeInfo4, xType4);
  if(!CompareSchemas(arg5, xType4))return nl->SymbolAtom ( "typeerror" );

  if(!listutils::isBTreeDescription(arg6))
    return nl->SymbolAtom("typeerror");


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "dualgraph"){

    ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->ThreeElemList(
                       nl->TwoElemList(nl->SymbolAtom("oid"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("loc"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("connection"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
    return result;
  }
  return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getvgedge: get the edge relation for visibility graph

*/

ListExpr OpTMGetVGEdgeTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);

  ListExpr xType1;
  nl->ReadFromString(VisualGraph::NodeTypeInfo, xType1);
  if(!CompareSchemas(arg2, xType1))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg3, xType2))return nl->SymbolAtom ( "typeerror" );


  ListExpr xType4;
  nl->ReadFromString(DualGraph::TriangleTypeInfo4, xType4);
  if(!CompareSchemas(arg4, xType4))return nl->SymbolAtom ( "typeerror" );

  if(!listutils::isBTreeDescription(arg5))
    return nl->SymbolAtom("typeerror");


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "dualgraph"){

    ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->ThreeElemList(
                       nl->TwoElemList(nl->SymbolAtom("Oid1"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Oid2"),
                                    nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Connection"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
    return result;
  }
  return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator myinside

*/

ListExpr OpTMMyInsideTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "line" &&
     nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "region"){
    return  nl->SymbolAtom ( "bool" );
  }
  return  nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator atpoint

*/

ListExpr OpTMAtPointTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "sline" &&
     nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "point" && 
     nl->IsAtom(arg3) && nl->AtomType(arg3) == SymbolType &&
     nl->SymbolValue(arg3) == "bool"){
    return  nl->SymbolAtom ( "real" );
  }
  return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator decomposetri

*/

ListExpr OpTMDecomposeTriTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);

  ListExpr xType;
  nl->ReadFromString(DualGraph::TriangleTypeInfo1, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );

  ListExpr result = nl->TwoElemList(
           nl->SymbolAtom("stream"),
              nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                    nl->TwoElemList(
                      nl->TwoElemList(nl->SymbolAtom("Vid"),
                                  nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("Triid"),
                                  nl->SymbolAtom("int"))
                  )
                )
          );
  return result;
}


/*
TypeMap fun for operator createvgraph

*/

ListExpr OpTMCreateVGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);
  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc = nl->Third(args);
  if(!nl->IsEqual(xIdDesc, "int")) return nl->SymbolAtom ( "typeerror" );
  if(!IsRelDescription(xEdgeDesc) || !IsRelDescription(xNodeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(VisualGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(xNodeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  nl->ReadFromString(VisualGraph::EdgeTypeInfo, xType);
  if(!CompareSchemas(xEdgeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  return nl->SymbolAtom ( "visualgraph" );
}


/*
TypeMap fun for operator getcontour

*/

ListExpr OpTMGetContourTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  if(nl->IsEqual(nl->First(args),"text") ||
     nl->IsEqual(nl->First(args),"int") ||
     nl->IsEqual(nl->First(args),"real")){

      ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("Oid"),
                                   nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("Hole"),
                                    nl->SymbolAtom("region"))
                  )
                )
          );
    return result;
  }
  return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getpolygon

*/

ListExpr OpTMGetPolygonTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr attrName1 = nl->Second(args);

  if (!(listutils::isRelDescription(arg1)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(arg1)),
                      aname1, attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"region")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type point");
  }
  ListExpr result = nl->SymbolAtom ( "region" );
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
             nl->OneElemList(nl->IntAtom(j1)),result);

}

/*
TypeMap fun for operator getallpoints

*/

ListExpr OpTMGetAllPointsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);


  if(nl->IsEqual(arg1, "region")){
    ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->FourElemList(
                       nl->TwoElemList(nl->SymbolAtom("V"),
                                   nl->SymbolAtom("point")),
                      nl->TwoElemList(nl->SymbolAtom("Neighbor1"),
                                    nl->SymbolAtom("point")),
                      nl->TwoElemList(nl->SymbolAtom("Neighbor2"),
                                    nl->SymbolAtom("point")),
                      nl->TwoElemList(nl->SymbolAtom("Regid"),
                                    nl->SymbolAtom("int"))
                  )
                )
          );
    return result;
  }else
    return nl->SymbolAtom ( "typeerror" );

}


/*
TypeMap fun for operator rotationsweep

*/

ListExpr OpTMRotationSweepTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);

  ListExpr xType1;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType1);
  if(!CompareSchemas(arg1, xType1))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(CompTriangle::AllPointsInfo, xType2);
  if(!CompareSchemas(arg2, xType2))return nl->SymbolAtom ( "typeerror" );



  ListExpr attrType1;
  string aname1 = nl->SymbolValue(arg5);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(arg4)),
                      aname1, attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"region")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type point");
  }


  if(nl->IsEqual(arg3, "rect")){
    ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("loc"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("connection"),
                                    nl->SymbolAtom("line"))
//                        nl->TwoElemList(nl->SymbolAtom("angle"),
//                                    nl->SymbolAtom("real"))
                  )
                )
          );
//    return result;

  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
             nl->OneElemList(nl->IntAtom(j1)),result);

  }else
    return nl->SymbolAtom ( "typeerror" );

}

/*
TypeMap fun for operator gethole

*/

ListExpr OpTMGetHoleTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  if(nl->IsEqual(nl->First(args),"region")){

      ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("Oid"),
                                   nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("Hole"),
                                    nl->SymbolAtom("region"))
                  )
                )
          );
    return result;
  }
  return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getsections
it get a set of sections from each route where interesting points can locate

*/

ListExpr OpTMGetSectionsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
 
  ListExpr param1 = nl->Second ( args );
  ListExpr param2 = nl->Third(args);
  ListExpr attrName1 = nl->Fourth(args);
  ListExpr attrName2 = nl->Fifth(args);
  ListExpr attrName3 = nl->Sixth(args);

  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"sline")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);

  if(j2 == 0 || !listutils::isSymbol(attrType2,"int")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }


  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);

  if(j3 == 0 || !listutils::isSymbol(attrType3,"region")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
  }


    if (!(listutils::isRelDescription(param1)))
        return nl->SymbolAtom ( "typeerror" );

    if (!(listutils::isRelDescription(param2)))
        return nl->SymbolAtom ( "typeerror" );

    if(nl->IsEqual(nl->First(args),"network")){ 

      ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("rid"),
                                   nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("sec"),
                                    nl->SymbolAtom("line"))
                  )
                )
          );
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
            nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),
            nl->IntAtom(j3)),result);
  }
  return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getinterestp1
generate interesting points locate on pavement 

*/

ListExpr OpTMGetInterestP1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 7 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );
  ListExpr attrName1 = nl->Third(args);
  ListExpr attrName2 = nl->Fourth(args);
  ListExpr attrName3 = nl->Fifth(args);
  ListExpr attrName4 = nl->Sixth(args);
  ListExpr param3 = nl->Nth(7, args);


 if (!(listutils::isRelDescription(param1)))
    return nl->SymbolAtom ( "typeerror" );

 if (!(listutils::isRelDescription(param2)))
    return nl->SymbolAtom ( "typeerror" );
  
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1, "int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2,attrType2);

  if(j2 == 0 || !listutils::isSymbol(attrType2, "line")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);

  if(j3 == 0 || !listutils::isSymbol(attrType3, "region")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
  }

  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname4, attrType4);

  if(j4 == 0 || !listutils::isSymbol(attrType4, "region")){
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type region");
  }

    if(nl->IsEqual(param3,"int")){
      ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->FourElemList(
                       nl->TwoElemList(nl->SymbolAtom("Rid"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Loc1"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("Loc2"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("Ptype"),
                                    nl->SymbolAtom("bool"))
                  )
                )
          );

     return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
            nl->FourElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                              nl->IntAtom(j3), nl->IntAtom(j4)),result);
    }
    return  nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getinterestp2
map each point into a triangle 

*/

ListExpr OpTMGetInterestP2TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );
  ListExpr param3 = nl->Third( args);
  ListExpr attrName1 = nl->Fourth(args);
  ListExpr attrName2 = nl->Fifth(args);
  ListExpr param6 = nl->Sixth(args);  
  

 if (!(listutils::isRelDescription(param1)))
    return nl->SymbolAtom ( "typeerror" );

 if (!(listutils::isRelDescription(param2)))
    return nl->SymbolAtom ( "typeerror" );
  
 if (!(listutils::isRTreeDescription(param3)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1, "point")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type region");
  }

  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);

  if(j2 == 0 || !listutils::isSymbol(attrType2, "region")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

   if(nl->IsEqual(param6,"int")){
      ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->ThreeElemList(
                       nl->TwoElemList(nl->SymbolAtom("Oid"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("Loc1"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("Loc2"),
                                    nl->SymbolAtom("point"))
                  )
                )
          );

     return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
            nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),result);
    }

    return  nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator cellbox 
partition the whole box into a set of cells 

*/

ListExpr OpTMCellBoxTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "list length should be 2" );
  }
  
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second ( args );
  ListExpr MBR_ATOM;
  if(nl->IsEqual(param1,"rect") && nl->IsEqual(param2,"int")){
    MBR_ATOM = nl->SymbolAtom("rect");
     return nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                /*nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("cellid"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("cover_area"),
                        nl->SymbolAtom("region")
                    ))));*/
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Cellid"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Cover_area"),
                        nl->SymbolAtom("region")
                    ),    
                    nl->TwoElemList(
                        nl->SymbolAtom("X_id"),
                        nl->SymbolAtom("int")
                    ),    
                    nl->TwoElemList(
                        nl->SymbolAtom("Y_id"),
                        nl->SymbolAtom("int")
                    ))));

  } 

    return  nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap fun for operator createbusroute1
create rough bus routes 

*/

ListExpr OpTMCreateBusRouteTypeMap1 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 7 )
  {
    return  nl->SymbolAtom ( "list length should be 7" );
  }

  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::StreetSectionCellTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel2 scheam should be" + 
                                BusRoute::StreetSectionCellTypeInfo);
  }

  ListExpr attrName1 = nl->Third ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }


  ListExpr attrName2 = nl->Fourth ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type int");

  ListExpr attrName3 = nl->Fifth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3, attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"region"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type region");
                 
  ListExpr index = nl->Sixth(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "parameter 6 should be btree" );


  ListExpr param7 = nl->Nth (7, args );
  if(!IsRelDescription(param7))
    return nl->SymbolAtom ( "typeerror: param7 should be relation" );

  ListExpr xType2;
  nl->ReadFromString(BusRoute::BusNetworkParaInfo, xType2); 
  if(!CompareSchemas(param7, xType2)){
    return listutils::typeError("rel scheam should be" + 
                                BusRoute::BusNetworkParaInfo);
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Start_cell_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Start_cell"),
                        nl->SymbolAtom("rect")),
                    nl->TwoElemList(
                        nl->SymbolAtom("End_cell_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("End_cell"),
                        nl->SymbolAtom("rect")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Route_type"),
                        nl->SymbolAtom("int"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                         nl->IntAtom(j3)),res);
}


/*
TypeMap fun for operator createbusroute2
create bus routes 

*/

ListExpr OpTMCreateBusRouteTypeMap2 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 8 )
  {
    return  nl->SymbolAtom ( "list length should be 8" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "space")){
      return nl->SymbolAtom ( "typeerror: param1 should be space" );
  }
  
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::StreetSectionCellTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel2 scheam should be" + 
                                BusRoute::StreetSectionCellTypeInfo);
  }

  ListExpr attrName = nl->Third ( args );
  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname,attrType);

  if(j == 0 || !listutils::isSymbol(attrType,"int")){
      return listutils::typeError("attr name" + aname + "not found"
                      "or not of type int");
  }
  
  ListExpr index = nl->Fourth(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "parameter 4 should be btree" );

  ListExpr param5 = nl->Fifth ( args );
  if(!IsRelDescription(param5))
    return nl->SymbolAtom ( "typeerror: param5 should be relation" );


  ListExpr attrName1 = nl->Sixth ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param5)),
                      aname1,attrType1);
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int"))
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");


  ListExpr attrName2 = nl->Nth (7, args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param5)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type int");
  
  ListExpr attrName3 = nl->Nth (8, args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param5)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");
 

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route1"),
                        nl->SymbolAtom("gline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route2"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Start_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("End_loc"),
                        nl->SymbolAtom("point")),

                    nl->TwoElemList(
                        nl->SymbolAtom("Route_type"),
                        nl->SymbolAtom("int"))

                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j),nl->IntAtom(j1),
                         nl->IntAtom(j2),nl->IntAtom(j3)),res);
}


/*
TypeMap fun for operator refine bus route 

*/

ListExpr OpTMRefineBusRouteTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 8 )
  {
    return  nl->SymbolAtom ( "list length should be 8" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::BusRoutesTmpTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel2 scheam should be" + 
                                BusRoute::BusRoutesTmpTypeInfo);
  }

  ListExpr attrName1 = nl->Third ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name " + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Fourth ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"gline")){
      return listutils::typeError("attr name " + aname2 + "not found"
                      "or not of type gline");
  }
  
  
  ListExpr attrName3 = nl->Fifth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"line")){
      return listutils::typeError("attr name " + aname3 + "not found"
                      "or not of type line");
  }
  
  ListExpr attrName4 = nl->Sixth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"point")){
      return listutils::typeError("attr name " + aname4 + "not found"
                      "or not of type point");
  }
  
  ListExpr attrName5 = nl->Nth (7, args );
  ListExpr attrType5;
  string aname5 = nl->SymbolValue(attrName5);
  int j5 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname5,attrType5);
  if(j5 == 0 || !listutils::isSymbol(attrType5,"point")){
      return listutils::typeError("attr name " + aname5 + "not found"
                      "or not of type point");
  }
  
  ListExpr attrName6 = nl->Nth (8, args );
  ListExpr attrType6;
  string aname6 = nl->SymbolValue(attrName6);
  int j6 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname6,attrType6);
  if(j6 == 0 || !listutils::isSymbol(attrType6,"int")){
      return listutils::typeError("attr name " + aname6 + "not found"
                      "or not of type int");
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route1"),
                        nl->SymbolAtom("gline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route2"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Start_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("End_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Route_type"),
                        nl->SymbolAtom("int"))

                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->SixElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                        nl->IntAtom(j3),nl->IntAtom(j4),
                        nl->IntAtom(j5),nl->IntAtom(j6)),res);
}


/*
TypeMap fun for operator createbusroute3
translate bus routes 

*/

ListExpr OpTMCreateBusRouteTypeMap3 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::BusRoutesTmpTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::BusRoutesTmpTypeInfo);
  }

  ListExpr attrName1 = nl->Second ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  
  ListExpr attrName2 = nl->Third ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"line"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type line");

  ListExpr attrName3 = nl->Fourth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"int"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");

  ListExpr param5 = nl->Fifth ( args );
  if(!(nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&  
     nl->SymbolValue(param5) == "real")){
      return nl->SymbolAtom ( "typeerror: param5 should be real" );
  }
  

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->FiveElemList(
//                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route1"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route2"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Route_type"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_uid"),
                        nl->SymbolAtom("int"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),res);
}

/*
TypeMap fun for operator createbusroute4
set the up and down bus route

*/

ListExpr OpTMCreateBusRouteTypeMap4 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 8 )
  {
    return  nl->SymbolAtom ( "list length should be 8" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be relation" );
  
  
  ListExpr xType1;
  nl->ReadFromString(BusRoute::NewBusRoutesTmpTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::NewBusRoutesTmpTypeInfo);
  }
  
  ListExpr attrName1 = nl->Second ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name " + aname1 + "not found"
                      "or not of type int");
  }
  
  
  ListExpr attrName2 = nl->Third ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"line"))
      return listutils::typeError("attr name " + aname2 + "not found"
                      "or not of type line");

  ListExpr attrName3 = nl->Fourth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"int"))
      return listutils::typeError("attr name " + aname3 + "not found"
                      "or not of type int");

  ListExpr attrName4 = nl->Fifth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"int"))
      return listutils::typeError("attr name " + aname4 + "not found"
                      "or not of type int");
  
                      
   ListExpr param6 = nl->Sixth ( args );
   if(!IsRelDescription(param6))
    return nl->SymbolAtom ( "typeerror: param6 should be a relation" );

  ListExpr attrName_a = nl->Nth (7, args );
  ListExpr attrType_a;
  string aname_a = nl->SymbolValue(attrName_a);
  int j_a = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_a,attrType_a);
                      
  if(j_a == 0 || !listutils::isSymbol(attrType_a,"int")){
      return listutils::typeError("attr name " + aname_a + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName_b = nl->Nth (8, args );
  ListExpr attrType_b;
  string aname_b = nl->SymbolValue(attrName_b);
  int j_b = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_b,attrType_b);
  if(j_b == 0 || !listutils::isSymbol(attrType_b,"bool"))
      return listutils::typeError("attr name " + aname_b + "not found"
                      "or not of type bool");


     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Route_type"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_uid"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("StartSmaller"),
                        nl->SymbolAtom("bool"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->SixElemList(nl->IntAtom(j1), nl->IntAtom(j2),
                        nl->IntAtom(j3), nl->IntAtom(j4),
                        nl->IntAtom(j_a),nl->IntAtom(j_b)),res);
}

/*
TypeMap fun for operator createbusstops1
create bus stop

*/

ListExpr OpTMCreateBusStopTypeMap1 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 9 )
  {
    return  nl->SymbolAtom ( "list length should be 6" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }


  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );
  
  ListExpr xType1;
  nl->ReadFromString(BusRoute::BusRoutesTmpTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::BusRoutesTmpTypeInfo);
  }

  ListExpr attrName1 = nl->Third ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Fourth ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"gline"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type gline");
  
  ListExpr attrName3 = nl->Fifth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"line"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type line");

  ListExpr attrName4 = nl->Sixth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"int"))
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type int");


  ListExpr param7 = nl->Nth (7, args );
  if(!IsRelDescription(param7))
    return nl->SymbolAtom ( "typeerror: param7 should be relation" );
  ListExpr xType;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType); 
  if(!CompareSchemas(param7, xType)){
    return listutils::typeError("rel2 scheam should be" + 
                                DualGraph::NodeTypeInfo);
  }

  ListExpr param8 = nl->Nth (8, args );
  if(!listutils::isBTreeDescription(param8))
    return nl->SymbolAtom ( "typeerror: param8 should be a btree" );

  ListExpr param9 = nl->Nth (9, args );
  if(!IsRelDescription(param9))
    return nl->SymbolAtom ( "typeerror: param9 should be relation" );

  ListExpr xType2;
  nl->ReadFromString(BusRoute::BusNetworkParaInfo, xType2); 
  if(!CompareSchemas(param9, xType2)){
    return listutils::typeError("rel scheam should be" + 
                                BusRoute::BusNetworkParaInfo);
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop2"),
                        nl->SymbolAtom("point"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                          nl->IntAtom(j3),nl->IntAtom(j4)),res);
}

/*
TypeMap fun for operator createbusstops2
merge bus stop

*/

ListExpr OpTMCreateBusStopTypeMap2 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
  }

  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::BusStopTemp1TypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::BusStopTemp1TypeInfo);
  }

  ListExpr attrName1 = nl->Third ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name " + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Fourth ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name " + aname2 + "not found"
                      "or not of type int");
  
  ListExpr attrName3 = nl->Fifth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"gpoint"))
      return listutils::typeError("attr name " + aname3 + "not found"
                      "or not of type gpoint");

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop2"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Sec_id"),
                        nl->SymbolAtom("int"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)
                          ),res);
}


/*
TypeMap fun for operator createbusstops3
merge bus stop

*/

ListExpr OpTMCreateBusStopTypeMap3 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 8 )
  {
    return  nl->SymbolAtom ( "list length should be 8" );
  }
  
 
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param1 should be relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::BusRoutesTmpTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::BusRoutesTmpTypeInfo);
  }

  ListExpr attrName = nl->Third ( args );
  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname,attrType);

  if(j == 0 || !listutils::isSymbol(attrType,"gline")){
      return listutils::typeError("attr name" + aname + "not found"
                      "or not of type gline");
  }
  
  
  ListExpr param4 = nl->Fourth ( args );
  if(!IsRelDescription(param4))
    return nl->SymbolAtom ( "typeerror: param4 should be a relation" );
  
  
  ListExpr attrName1 = nl->Fifth ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname1,attrType1);

  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Sixth ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type int");
  
  ListExpr attrName3 = nl->Nth (7, args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"gpoint"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type gpoint");

  ListExpr param8 = nl->Nth (8, args );
  if(!listutils::isBTreeDescription(param8)) 
    return nl->SymbolAtom ( "typeerror: param7 should be btree" );
  
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

//                nl->FourElemList(
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop2"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("StartSmaller"),
                        nl->SymbolAtom("bool"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j),nl->IntAtom(j1),nl->IntAtom(j2),
                         nl->IntAtom(j3)),res);
}


/*
TypeMap fun for operator createbusstops4
new position for bus stops (id,pos) after translate bus route: down and up 
and get the pos value for each kind of bus stop 

*/

ListExpr OpTMCreateBusStopTypeMap4 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 8 )
  {
    return  nl->SymbolAtom ( "list length should be 8" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(BusRoute::NewBusRoutesTmpTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::NewBusRoutesTmpTypeInfo);
  }

  ListExpr attrName_a = nl->Second ( args );
  ListExpr attrType_a;
  string aname_a = nl->SymbolValue(attrName_a);
  int j_1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname_a,attrType_a);

  if(j_1 == 0 || !listutils::isSymbol(attrType_a,"line")){
      return listutils::typeError("attr name " + aname_a + "not found"
                      "or not of type line");
  }
  
  
  ListExpr attrName_b = nl->Third ( args );
  ListExpr attrType_b;
  string aname_b = nl->SymbolValue(attrName_b);
  int j_2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname_b,attrType_b);
                      
  if(j_2 == 0 || !listutils::isSymbol(attrType_b,"line")){
      return listutils::typeError("attr name " + aname_b + "not found"
                      "or not of type line");
  }
  
  
  ListExpr param4 = nl->Fourth ( args );
  if(!IsRelDescription(param4))
    return nl->SymbolAtom ( "typeerror: param4 should be a relation" );
  
  
  ListExpr attrName1 = nl->Fifth ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name " + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Sixth (args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name " + aname2 + "not found"
                      "or not of type int");
  
  ListExpr attrName3 = nl->Nth (7, args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"point"))
      return listutils::typeError("attr name " + aname3 + "not found"
                      "or not of type point");

                      
  ListExpr attrName4 = nl->Nth (8, args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname4, attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"bool"))
      return listutils::typeError("attr name " + aname4 + "not found"
                      "or not of type bool");

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                  nl->Cons(
                   nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")),
                   nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_uid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop1"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop2"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_pos"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Stop_loc_id"),
                        nl->SymbolAtom("int"))
                    ))));


      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->SixElemList(nl->IntAtom(j_1),nl->IntAtom(j_2),nl->IntAtom(j1),
                         nl->IntAtom(j2),nl->IntAtom(j3),
                         nl->IntAtom(j4)),res);

}


/*
TypeMap fun for operator createbusstops5
set up and down value for each bus stop 

*/

ListExpr OpTMCreateBusStopTypeMap5 ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 8 )
  {
    return  nl->SymbolAtom ( "list length should be 8" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr xType1;
  nl->ReadFromString(BusRoute::FinalBusRoutesTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusRoute::FinalBusRoutesTypeInfo);
  }
  
  ListExpr attrName = nl->Second ( args );
  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname,attrType);
                      
  if(j == 0 || !listutils::isSymbol(attrType,"bool")){
      return listutils::typeError("attr name" + aname + "not found"
                      "or not of type bool");
  }
  
    
  ListExpr param3 = nl->Third ( args );
  if(!IsRelDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a relation" );
  
  
  ListExpr attrName1 = nl->Fourth ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param3)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Fifth (args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param3)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"int"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type int");
  
  ListExpr attrName3 = nl->Sixth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param3)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"int"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");

                      
  ListExpr attrName4 = nl->Nth (7, args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param3)),
                      aname4, attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"point"))
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type point");
                      
  ListExpr attrName5 = nl->Nth (8, args );
  ListExpr attrType5;
  string aname5 = nl->SymbolValue(attrName5);
  int j5 = listutils::findAttribute(nl->Second(nl->Second(param3)),
                      aname5, attrType5);
  if(j5 == 0 || !listutils::isSymbol(attrType5,"real"))
      return listutils::typeError("attr name" + aname5 + "not found"
                      "or not of type real");
                      
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_uid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_pos"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Stop_direction"),
                        nl->SymbolAtom("bool"))
                    )));


      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->SixElemList(nl->IntAtom(j),nl->IntAtom(j1),nl->IntAtom(j2),
                         nl->IntAtom(j3),nl->IntAtom(j4),
                         nl->IntAtom(j5)),res);

}


/*
TypeMap fun for operator getbusstops

*/

ListExpr OpTMGetBusStopsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::bus_stop_typeinfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel11 scheam should be" + 
                                RoadDenstiy::bus_stop_typeinfo);
  }

  ListExpr param2 = nl->Second ( args );
  if(!listutils::isBTreeDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a btree" );


  ListExpr param3 = nl->Third ( args );
  if(!IsRelDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a relation" );

  ListExpr xType2;
  nl->ReadFromString(RoadDenstiy::bus_route_typeinfo, xType2); 
  if(!CompareSchemas(param3, xType2)){
    return listutils::typeError("rel3 scheam should be" + 
                                RoadDenstiy::bus_route_typeinfo);
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
//                nl->OneElemList(
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop"),
                        nl->SymbolAtom("busstop")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Stop_geodata"),
                        nl->SymbolAtom("point"))
                    )));
      return res;
}

/*
TypeMap fun for operator getbusroutes

*/

ListExpr OpTMGetBusRoutesTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::bus_stop_typeinfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel11 scheam should be" + 
                                RoadDenstiy::bus_stop_typeinfo);
  }

  ListExpr param2 = nl->Second ( args );
  if(!listutils::isBTreeDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a btree" );


  ListExpr param3 = nl->Third ( args );
  if(!IsRelDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a relation" );

  ListExpr xType2;
  nl->ReadFromString(RoadDenstiy::bus_route_typeinfo, xType2); 
  if(!CompareSchemas(param3, xType2)){
    return listutils::typeError("rel3 scheam should be" + 
                                RoadDenstiy::bus_route_typeinfo);
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route"),
                        nl->SymbolAtom("busroute"))
                    )));
      return res;
}


/*
TypeMap fun for operator brgeodata 

*/

ListExpr OpTMBRGeoDataTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  ListExpr param1 = nl->First(args); 
  
 if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->SymbolValue(param1) == "busroute")){
      return nl->SymbolAtom ( "typeerror: param should be busroute" );
  }
  return nl->SymbolAtom ( "sline" );

}


/*
TypeMap fun for operator bsgeodata 

*/

ListExpr OpTMBSGeoDataTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "list length should be 2" );
  }
  ListExpr param1 = nl->First(args); 
  ListExpr param2 = nl->Second(args); 
  
 if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->SymbolValue(param1) == "busstop" && nl->IsAtom(param2) && 
     nl->AtomType(param2) == SymbolType &&
     (nl->SymbolValue(param2) == "busroute"||
      nl->SymbolValue(param2) == "busnetwork"))){
      return nl->SymbolAtom ( "typeerror: busstop x busroute expected" );
  }
  return nl->SymbolAtom ( "point" );
}

/*
TypeMap fun for operator getstopid

*/

ListExpr OpTMGetStopIdTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "busstop")){
      return nl->SymbolAtom ( "typeerror: param should be busstop" );
  }
  return nl->SymbolAtom ( "int" );
}

/*
TypeMap fun for operator up down

*/

ListExpr OpTMUpDownTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }

  ListExpr param1 = nl->First ( args );
  if(!(nl->SymbolValue(param1) == "busstop" ||
       nl->SymbolValue(param1) == "busroute")){
      return nl->SymbolAtom ( "typeerror: param should be busstop" );
  }
  return nl->SymbolAtom ( "bool" );
}

/*
TypeMap fun for operator thepavement 

*/
ListExpr OpTMThePavementTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "list length should be 2" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "int")){
      return nl->SymbolAtom ( "typeerror: param1 should be int" );
  }
  
  ListExpr param2 = nl->Second ( args );
  
  ListExpr xType1;
//  nl->ReadFromString(Pavement::PaveTypeInfo, xType1);
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
/*    return listutils::typeError("rel1 scheam should be" + 
                                Pavement::PaveTypeInfo);*/
    return listutils::typeError("rel1 scheam should be" + 
                                DualGraph::NodeTypeInfo);

  }
  
  return nl->SymbolAtom ( "pavenetwork" );
}


/*
TypeMap fun for operator thebusnetwork

*/
ListExpr OpTMTheBusNetworkTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return  nl->SymbolAtom ( "list length should be 4" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "int")){
      return nl->SymbolAtom ( "typeerror: param1 should be int" );
  }
  
  ListExpr param2 = nl->Second ( args );
  
  ListExpr xType1;
  nl->ReadFromString(BusNetwork::BusStopsTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                BusNetwork::BusStopsTypeInfo);
  }
  
 
  ListExpr param3 = nl->Third ( args );
  ListExpr xType2;
  nl->ReadFromString(BusNetwork::BusRoutesTypeInfo, xType2); 
  if(!CompareSchemas(param3, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                                BusNetwork::BusRoutesTypeInfo);
  }
 
  ListExpr param4 = nl->Fourth ( args );
  ListExpr xType3;
  nl->ReadFromString(BusNetwork::BusTripsTypeInfo, xType3); 
  if(!CompareSchemas(param4, xType3)){
    return listutils::typeError("rel3 scheam should be" + 
                                BusNetwork::BusTripsTypeInfo);
  }
  
  return nl->SymbolAtom ( "busnetwork" );
}



/*
TypeMap fun for operator busstops

*/
ListExpr OpTMBusStopsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  
  ListExpr param1 = nl->First(args);
  if(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->SymbolValue(param1) == "busnetwork"){
  
         ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop"),
                        nl->SymbolAtom("busstop"))
                    )));
      return res;
  }
  return nl->SymbolAtom("typeerror"); 

}


/*
TypeMap fun for operator busroutes

*/
ListExpr OpTMBusRoutesTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  
  ListExpr param1 = nl->First(args);
  if(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->SymbolValue(param1) == "busnetwork"){
  
         ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_route"),
                        nl->SymbolAtom("busroute"))
                    )));
      return res;
  }
  return nl->SymbolAtom("typeerror"); 

}

/*
TypeMap fun for operator brsegments

*/
ListExpr OpTMBRSegmentTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "list length should be 2" );
  }

  ListExpr param1 = nl->First(args);
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->SymbolValue(param1) == "line"))
      return nl->SymbolAtom("typeerror");

  ListExpr param2 = nl->Second(args); 
  if(!(nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&
     nl->SymbolValue(param2) == "line"))
      return nl->SymbolAtom("typeerror");

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Segment"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("USegment"),
                        nl->SymbolAtom("line")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Type"),
                        nl->SymbolAtom("int"))
                    )));
  return res;
}

/*
TypeMap fun for operator mapbstopave

*/
ListExpr OpTMMapBsToPaveTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
  }

  ListExpr param1 = nl->First(args);
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
     nl->SymbolValue(param1) == "busnetwork"))
      return nl->SymbolAtom("typeerror");

  ListExpr param2 = nl->Second(args); 
  if(!listutils::isRTreeDescription(param2))
      return nl->SymbolAtom("typeerror");

  ListExpr param3 = nl->Third(args); 
  ListExpr xType;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType); 
  if(!listutils::isRelDescription(param3) || !CompareSchemas(param3, xType))
      return nl->SymbolAtom("typeerror");


  ListExpr param4 = nl->Fourth(args);
  if(!(nl->IsAtom(param4) && nl->AtomType(param4) == SymbolType &&
      nl->SymbolValue(param4) == "int"))
      return nl->SymbolAtom("typeerror");

 ListExpr param5 = nl->Fifth(args);
  if(!(nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&
      nl->SymbolValue(param5) == "real"))
      return nl->SymbolAtom("typeerror");
  
  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Pave_loc1"),
                        nl->SymbolAtom("genloc")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Pave_loc2"),
                        nl->SymbolAtom("point"))
                    )));
  return res;
}

/*
TypeMap fun for operator bsneighbors1 

*/
ListExpr OpTMBsNeighbors1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
  }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);


  if(!(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "dualgraph"))
    return nl->SymbolAtom ( "typeerror: param1 should be dual graph");

  if(!(nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "visualgraph" ))
    return nl->SymbolAtom ( "typeerror: param2 should be visibility graph");

  ListExpr arg3 = nl->Third ( args );
  if(!IsRelDescription(arg3))
  return listutils::typeError("param3 should be a relation");
  
  ListExpr xType1;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType1);
  if(!CompareSchemas(arg3, xType1))
    return nl->SymbolAtom ( "rel1 scheam should be" + 
                                DualGraph::TriangleTypeInfo3 );
  
  
  ListExpr arg4 = nl->Fourth ( args );
  if(!listutils::isRelDescription(arg4))
      return nl->SymbolAtom("typeerror:param4 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType2); 
  if(!CompareSchemas(arg4, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                                BN::BusStopsPaveTypeInfo);
  }

  ListExpr arg5 = nl->Fifth ( args );
  if(!listutils::isRTreeDescription(arg5))
      return nl->SymbolAtom("typeerror:param5 should be an rtree");


    ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->Cons(
                  nl->TwoElemList(
                        nl->SymbolAtom("Bus_uoid"),
                        nl->SymbolAtom("int")), 
                  nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop2"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("SubPath1"),
                        nl->SymbolAtom("sline")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("SubPath2"),
                        nl->SymbolAtom("sline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path2"),
                        nl->SymbolAtom("sline")))
                    )));
  return res;

}

/*
TypeMap fun for operator bsneighbors2 

*/
ListExpr OpTMBsNeighbors2TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }

  ListExpr arg1 = nl->First(args);

  if(!(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "busnetwork"))
    return nl->SymbolAtom ( "typeerror: param1 should be busnetwork");

    ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                nl->TwoElemList(
                        nl->SymbolAtom("Bus_uoid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Bus_stop2"),
                        nl->SymbolAtom("busstop"))
                    )));
  return res;

}

/////////////////////////////////////////////////////////////////////
////////////// index on generic moving objects/////////////////////
////////////////////////////////////////////////////////////////////
ListExpr TMRTreeProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<relation> createtmrtree [<attrname>]"
    " where <attrname> is the key of type rect3");

  return
    (nl->TwoElemList(
         nl->TwoElemList(nl->StringAtom("Creation"),
                         nl->StringAtom("Example Creation")),
         nl->TwoElemList(examplelist,
                         nl->StringAtom("(let tmrtree = genmounits"
                         " creatrtree [Box])"))));
}


bool CheckTMRTree(ListExpr type, ListExpr& errorInfo)
{
  return  nl->IsEqual( type, TM_RTree<3,TupleId>::BasicType());
}

/*
Type Constructor object for type constructor tmrtree

*/
TypeConstructor tmrtree(TM_RTree<3, TupleId>::BasicType(),
                        TMRTreeProp,
                        OutTMRTree<3>,
                        InTMRTree<3>,
                        0,
                        0,
                        CreateTMRTree<3>,
                        DeleteTMRTree<3>,
                        OpenTMRTree<3>,
                        SaveTMRTree<3>,
                        CloseTMRTree<3>,
                        CloneTMRTree<3>,
                        CastTMRTree<3>,
                        SizeOfTMRTree<3>,
                        CheckTMRTree );


#endif

