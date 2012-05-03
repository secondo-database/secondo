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

const string OpTMWalkSPTypeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>pavement x rel1 x rel2 x rel3 x btree x int-> line</text--->"
    "<text>walk_sp_type(pn, rel, rel, rel, btree, int)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp_type(pn, query_loc1, query_loc2, "
    "tri_reg_new, btree_qloc, 2);"
    "</text--->"
    ") )";

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


#endif

