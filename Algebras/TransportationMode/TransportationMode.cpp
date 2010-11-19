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
#include "PaveGraph.h"
#include "Triangulate.h"
#include "BusNetwork.h"

extern NestedList* nl;
extern QueryProcessor *qp;


namespace TransportationMode{

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

const string OpTMNodeDGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph -> rel</text--->"
    "<text>nodedualgraph(dualgraph)</text--->"
    "<text>get the node relation of the graph</text--->"
    "<text>query nodedualgraph(dg1) count; </text--->"
    ") )";

const string OpTMWalkSPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x visualgraph x rel1 x rel2 x rel3 x rel4 x btree-> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>walk_sp(dg1, vg1, rel, rel, rel, rel, btree)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp(dg1, vg1, query_loc1, query_loc2,tri_reg_new, "
    "vertex_tri, btr_vid); </text--->"
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

const string OpTMGetAdjNodeDGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x int"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getadjnode_dg(dualgraph,int)</text--->"
    "<text>for a given node, find its adjacent nodes</text--->"
    "<text>query getadjnode_dg(dg1,1); </text--->"
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

const string OpTMGetAdjNodeVGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>visualgraph x int"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getadjnode_vg(visualgraph,int)</text--->"
    "<text>for a given node, find its adjacent nodes</text--->"
    "<text>query getadjnode_vg(vg1,1); </text--->"
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

const string OpTMGenInterestP1Spec  =
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
    ") )";

const string OpTMCellBoxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>bbox x int-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>cellbox(bbox, 10)</text--->"
    "<text>partition the bbox into 10 x 10 equal size cells</text--->"
    "<text>query cellbox(bbox, 10)"
    "count;</text--->"
    ") )";
    
const string OpTMCreateBusRouteSpec1  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3 x attr4 x btree"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_route1(n,rel,attr1,attr2,attr3,attr4,btree);"
    "</text--->"
    "<text>create bus route1</text--->"
    "<text>query create_bus_route1(n,street_sections_cell,sid_s,cellid_w_a_c,"
    "Cnt_a_c,cover_area_b_c,section_cell_index) count;</text--->"
    ") )";
    
const string OpTMCreateBusRouteSpec2  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel1 x attr x btree x rel2 x attr1 x attr2 x attr3"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_route2(n,rel,attr,btree,rel2,attr1,attr2,attr3);"
    "</text--->"
    "<text>create bus routes</text--->"
    "<text>query create_bus_route2(n,street_sections_cell,cellid_w_a_c,"
    "section_cell_index,rough_pair,start_cell_id,end_cell_id,route_type) "
    "count;</text--->"
    ") )";

const string OpTMRefineBusRouteSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3 x attr4"
    " x attr5 x attr6 -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>refine_bus_route(n,rel,attr1,attr2,attr3,attr4,attr5,attr6);"
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
    
const string OpTMCreateBusStopSpec1  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3 x attr4"
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops1(n,rel,attr1,attr2, attr3, attr4);</text--->"
    "<text>create bus stops</text--->"
    "<text>query create_bus_stop1(n,busroutes,br_id,bus_route1,"
    "bus_route2,route_type) count;</text--->"
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
    "<text>query create_bus_stop5(newbusroutes, bus_route1,bus_route2," 
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
    

const string OpTMMapToIntSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>string -> int</text--->"
    "<text>maptoint(abc);"
    "</text--->"
    "<text>map a string value to an int value (for road type)</text--->"
    "<text>query maptoint(\"abc\") count;</text--->"
    ") )";
    
const string OpTMMapToRealSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>string -> real</text--->"
    "<text>maptoreal(abc);"
    "</text--->"
    "<text>map a string value to an real value (for road type)</text--->"
    "<text>query maptoint(\"abc\") count;</text--->"
    ") )";

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

const string OpTMCreateTimeTableSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table(rel,rel,btree);</text--->"
    "<text>create time table at each spatial location </text--->"
    "<text>query create_time_table(final_busstops,all_bus_rel,btree_mo)"
    "count;</text--->"
    ") )";
    
////////////////TypeMap function for operators//////////////////////////////

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
                        nl->TwoElemList(nl->SymbolAtom("rid"),
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

/*                      nl->SixElemList(
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
                                      nl->SymbolAtom("region"))*/
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("crossreg"),
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


    if (listutils::isRelDescription(param2) &&
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
                        nl->TwoElemList(nl->SymbolAtom("rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("pavement"),
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
                        nl->TwoElemList(nl->SymbolAtom("oid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("oid2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("commarea"),
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
                        nl->TwoElemList(nl->SymbolAtom("oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("pavement"),
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
                        nl->TwoElemList(nl->SymbolAtom("oid1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("oid2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("commarea"),
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
                        nl->TwoElemList(nl->SymbolAtom("spath"),
                                    nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("channel"),
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
//  ListExpr xEdgeDesc = nl->Second(args);
//  ListExpr xNodeDesc = nl->Third(args);

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
TypeMap fun for operator nodedualgraph

*/

ListExpr OpTMNodeDGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "dualgraph"){
      ListExpr xType;
      nl->ReadFromString(DualGraph::NodeTypeInfo, xType);
      return xType;
  }

  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator walksp

*/

ListExpr OpTMWalkSPTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 7 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg0 = nl->First(args);
  ListExpr arg1 = nl->Second(args);
  ListExpr arg2 = nl->Third(args);
  ListExpr arg3 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  ListExpr arg6 = nl->Sixth(args);
  ListExpr arg7 = nl->Nth(7, args);

  ListExpr xType;
  nl->ReadFromString(VisualGraph::QueryTypeInfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );

  if(!CompareSchemas(arg3, xType))return nl->SymbolAtom ( "typeerror" );


  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg5, xType2))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType4;
  nl->ReadFromString(DualGraph::TriangleTypeInfo4, xType4);
  if(!CompareSchemas(arg6, xType4))return nl->SymbolAtom ( "typeerror" );

  if(!listutils::isBTreeDescription(arg7))
    return nl->SymbolAtom("typeerror");


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
                        nl->TwoElemList(nl->SymbolAtom("oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("loc1"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("loc2"),
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
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type point");
  }

  ListExpr result =   nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("curve"),
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
                        nl->TwoElemList(nl->SymbolAtom("cycleno"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("vertex"),
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
                        nl->TwoElemList(nl->SymbolAtom("v1"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("v2"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("v3"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("centroid"),
                                    nl->SymbolAtom("point"))
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
                       nl->TwoElemList(nl->SymbolAtom("oid1"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("oid2"),
                                    nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("commarea"),
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
                       nl->TwoElemList(nl->SymbolAtom("oid1"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("oid2"),
                                    nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("commarea"),
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
TypeMap fun for operator getvgedge

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
//  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fourth(args);
  ListExpr arg6 = nl->Fifth(args);

  ListExpr xType1;
  nl->ReadFromString(VisualGraph::NodeTypeInfo, xType1);
  if(!CompareSchemas(arg2, xType1))return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType2);
  if(!CompareSchemas(arg3, xType2))return nl->SymbolAtom ( "typeerror" );


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
TypeMap fun for operator getadjnode

*/

ListExpr OpTMGetAdjNodeDGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "dualgraph" &&
     nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "int"){

      ListExpr result = nl->TwoElemList(
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
    return result;
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
                      nl->TwoElemList(nl->SymbolAtom("vid"),
                                  nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("triid"),
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
TypeMap fun for operator getadjnode

*/

ListExpr OpTMGetAdjNodeVGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "visualgraph" &&
     nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "int"){

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
                       nl->TwoElemList(nl->SymbolAtom("oid"),
                                   nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("hole"),
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
                       nl->TwoElemList(nl->SymbolAtom("v"),
                                   nl->SymbolAtom("point")),
                      nl->TwoElemList(nl->SymbolAtom("neighbor1"),
                                    nl->SymbolAtom("point")),
                      nl->TwoElemList(nl->SymbolAtom("neighbor2"),
                                    nl->SymbolAtom("point")),
                      nl->TwoElemList(nl->SymbolAtom("regid"),
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
                       nl->TwoElemList(nl->SymbolAtom("oid"),
                                   nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("hole"),
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
                       nl->TwoElemList(nl->SymbolAtom("rid"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("loc1"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("loc2"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("ptype"),
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
                       nl->TwoElemList(nl->SymbolAtom("oid"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("loc1"),
                                    nl->SymbolAtom("point")),
                       nl->TwoElemList(nl->SymbolAtom("loc2"),
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
                        nl->SymbolAtom("cellid"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("cover_area"),
                        nl->SymbolAtom("region")
                    ),    
                    nl->TwoElemList(
                        nl->SymbolAtom("x_id"),
                        nl->SymbolAtom("int")
                    ),    
                    nl->TwoElemList(
                        nl->SymbolAtom("y_id"),
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
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"int"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");
  
  
  ListExpr attrName4 = nl->Sixth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"region"))
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type region");
                 
  ListExpr index = nl->Nth(7,args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "parameter 7 should be btree" );
    
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                
/*                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route_id"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route"),
                        nl->SymbolAtom("line"))*/
                        
//                nl->FourElemList(
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("start_cell_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("start_cell"),
                        nl->SymbolAtom("rect")),
                    nl->TwoElemList(
                        nl->SymbolAtom("end_cell_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("end_cell"),
                        nl->SymbolAtom("rect")),
                    nl->TwoElemList(
                        nl->SymbolAtom("route_type"),
                        nl->SymbolAtom("int"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                         nl->IntAtom(j3),nl->IntAtom(j4)),res);
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
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }
  
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );
  
  
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
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route1"),
                        nl->SymbolAtom("gline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route2"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("start_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("end_loc"),
                        nl->SymbolAtom("point")),

                    nl->TwoElemList(
                        nl->SymbolAtom("route_type"),
                        nl->SymbolAtom("int"))
/*                    nl->TwoElemList(
                        nl->SymbolAtom("bus_section2"),
                        nl->SymbolAtom("line"))*/
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
  if(j2 == 0 || !listutils::isSymbol(attrType2,"gline")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type gline");
  }
  
  
  ListExpr attrName3 = nl->Fifth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"line")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type line");
  }
  
  ListExpr attrName4 = nl->Sixth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"point")){
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type point");
  }
  
  ListExpr attrName5 = nl->Nth (7, args );
  ListExpr attrType5;
  string aname5 = nl->SymbolValue(attrName5);
  int j5 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname5,attrType5);
  if(j5 == 0 || !listutils::isSymbol(attrType5,"point")){
      return listutils::typeError("attr name" + aname5 + "not found"
                      "or not of type point");
  }
  
  ListExpr attrName6 = nl->Nth (8, args );
  ListExpr attrType6;
  string aname6 = nl->SymbolValue(attrName6);
  int j6 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname6,attrType6);
  if(j6 == 0 || !listutils::isSymbol(attrType6,"int")){
      return listutils::typeError("attr name" + aname6 + "not found"
                      "or not of type int");
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route1"),
                        nl->SymbolAtom("gline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route2"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("start_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("end_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("route_type"),
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
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route1"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route2"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("route_type"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_uid"),
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
                      
                      
  ListExpr attrName4 = nl->Fifth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"int"))
      return listutils::typeError("attr name" + aname4 + "not found"
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
      return listutils::typeError("attr name" + aname_a + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName_b = nl->Nth (8, args );
  ListExpr attrType_b;
  string aname_b = nl->SymbolValue(attrName_b);
  int j_b = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_b,attrType_b);
  if(j_b == 0 || !listutils::isSymbol(attrType_b,"bool"))
      return listutils::typeError("attr name" + aname_b + "not found"
                      "or not of type bool");


     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("route_type"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_uid"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("startSmaller"),
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
  if ( nl->ListLength ( args ) != 6 )
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
                      
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
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
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"gpoint"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type gpoint");

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("sec_id"),
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
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("startSmaller"),
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
  
  ListExpr attrName_a = nl->Second ( args );
  ListExpr attrType_a;
  string aname_a = nl->SymbolValue(attrName_a);
  int j_1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname_a,attrType_a);
                      
  if(j_1 == 0 || !listutils::isSymbol(attrType_a,"line")){
      return listutils::typeError("attr name" + aname_a + "not found"
                      "or not of type line");
  }
  
  
  ListExpr attrName_b = nl->Third ( args );
  ListExpr attrType_b;
  string aname_b = nl->SymbolValue(attrName_b);
  int j_2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname_b,attrType_b);
                      
  if(j_2 == 0 || !listutils::isSymbol(attrType_b,"line")){
      return listutils::typeError("attr name" + aname_b + "not found"
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
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Sixth (args );
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
  if(j3 == 0 || !listutils::isSymbol(attrType3,"point"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type point");

                      
  ListExpr attrName4 = nl->Nth (8, args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param4)),
                      aname4, attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"bool"))
      return listutils::typeError("attr name" + aname4 + "not found"
                      "or not of type bool");
                      
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                                        
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_uid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_pos"),
                        nl->SymbolAtom("real"))
//                    nl->TwoElemList(
//                        nl->SymbolAtom("test_line"),
//                        nl->SymbolAtom("line"))
                    )));

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
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_uid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_pos"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_direction"),
                        nl->SymbolAtom("bool"))
                    )));


      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->SixElemList(nl->IntAtom(j),nl->IntAtom(j1),nl->IntAtom(j2),
                         nl->IntAtom(j3),nl->IntAtom(j4),
                         nl->IntAtom(j5)),res);

}

/*
TypeMap fun for operator maptoint
map a string value to an int 

*/

ListExpr OpTMMapToInt ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  
  
  if(!listutils::isSymbol(nl->First(args),CcString::BasicType()))
    return listutils::typeError("string expected");
  
  return nl->SymbolAtom("int");
  
}

/*
TypeMap fun for operator maptoreal
map a string value to an real 

*/

ListExpr OpTMMapToReal ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  
  
  if(!listutils::isSymbol(nl->First(args),CcString::BasicType()))
    return listutils::typeError("string expected");
  
  return nl->SymbolAtom("real");
  
}


/*
TypeMap fun for operator getroutedensity1
distinguish daytime and night bus routs

*/

ListExpr OpTMGetRouteDensity1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 10 )
  {
    return  nl->SymbolAtom ( "list length should be 10" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }
  
  
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );
  
  ListExpr attrName1 = nl->Third ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Fourth (args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"mint"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type mint");
                      

  ListExpr param5 = nl->Fifth ( args );
  if(!listutils::isBTreeDescription(param5))
      return  nl->SymbolAtom ( "parameter5 should be btree" );

  ListExpr param6 = nl->Sixth ( args );
  if(!IsRelDescription(param6))
    return nl->SymbolAtom ( "typeerror: param6 should be a relation" );
  
  
  ListExpr attrName_a = nl->Nth (7, args );
  ListExpr attrType_a;
  string aname_a = nl->SymbolValue(attrName_a);
  int j_a = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_a,attrType_a);
                      
  if(j_a == 0 || !listutils::isSymbol(attrType_a,"int")){
      return listutils::typeError("attr name" + aname_a + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName_b = nl->Nth (8, args );
  ListExpr attrType_b;
  string aname_b = nl->SymbolValue(attrName_b);
  int j_b = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_b,attrType_b);
                      
  if(j_b == 0 || !listutils::isSymbol(attrType_b,"gline")){
      return listutils::typeError("attr name" + aname_b + "not found"
                      "or not of type gline");
  }
  
  
  ListExpr param7 = nl->Nth (9, args );
  if(!(nl->IsAtom(param7) && nl->AtomType(param7) == SymbolType &&  
     nl->SymbolValue(param7) == "periods")){
      return nl->SymbolAtom ( "typeerror: param7 should be periods" );
  }
  
  ListExpr param8 = nl->Nth (10, args );
  if(!(nl->IsAtom(param8) && nl->AtomType(param8) == SymbolType &&  
     nl->SymbolValue(param8) == "periods")){
      return nl->SymbolAtom ( "typeerror: param8 should be periods" );
  }               
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("traffic_flow"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("duration1"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("duration2"),
                        nl->SymbolAtom("periods"))
                    )));


      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                          nl->IntAtom(j_a),nl->IntAtom(j_b)),res);

}


/*
TypeMap fun for operator set ts night bus routs

*/

ListExpr OpTMSetTSNighbBusTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return  nl->SymbolAtom ( "list length should be 6" );
  }
  
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr attrName1 = nl->Second ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Third(args);
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"periods"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type periods");
                      
  ListExpr attrName3 = nl->Fourth(args);
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"periods"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type periods");

  ListExpr param5 = nl->Fifth(args );
  if(!(nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&  
     nl->SymbolValue(param5) == "periods")){
      return nl->SymbolAtom ( "typeerror: param7 should be periods" );
  }
  
  ListExpr param6 = nl->Sixth(args );
  if(!(nl->IsAtom(param6) && nl->AtomType(param6) == SymbolType &&  
     nl->SymbolValue(param6) == "periods")){
      return nl->SymbolAtom ( "typeerror: param8 should be periods" );
  }               
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("duration1"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("duration2"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_interval"),
                        nl->SymbolAtom("real"))
                    )));


      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),res);

}

/*
TypeMap fun for operator set ts daytime bus routs

*/

ListExpr OpTMSetTSDayBusTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 6 )
  {
    return  nl->SymbolAtom ( "list length should be 6" );
  }
  
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr attrName1 = nl->Second ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Third(args);
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"periods"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type periods");
                      
  ListExpr attrName3 = nl->Fourth(args);
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3,attrType3);
  if(j3 == 0 || !listutils::isSymbol(attrType3,"periods"))
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type periods");

  ListExpr param5 = nl->Fifth(args );
  if(!(nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&  
     nl->SymbolValue(param5) == "periods")){
      return nl->SymbolAtom ( "typeerror: param7 should be periods" );
  }
  
  ListExpr param6 = nl->Sixth(args );
  if(!(nl->IsAtom(param6) && nl->AtomType(param6) == SymbolType &&  
     nl->SymbolValue(param6) == "periods")){
      return nl->SymbolAtom ( "typeerror: param8 should be periods" );
  }               
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("duration1"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("duration2"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_interval1"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_interval2"),
                        nl->SymbolAtom("real"))
                    )));


      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),nl->IntAtom(j3)),res);

}



/*
TypeMap fun for operator set br speed

*/

ListExpr OpTMSetBRSpeedTypeMap ( ListExpr args )
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
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );
  
  ListExpr attrName1 = nl->Third ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Fourth(args);
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"gline"))
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type gline");
                      
  ListExpr param5 = nl->Fifth ( args );
  if(!IsRelDescription(param5))
    return nl->SymbolAtom ( "typeerror: param5 should be a relation" );
  

  ListExpr attrName = nl->Sixth(args);
  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j = listutils::findAttribute(nl->Second(nl->Second(param5)),
                      aname, attrType);
  if(j == 0 || !listutils::isSymbol(attrType,"real"))
      return listutils::typeError("attr name" + aname + "not found"
                      "or not of type real");
  
  ListExpr param7 = nl->Nth (7, args );
  if(!IsRelDescription(param7))
    return nl->SymbolAtom ( "typeerror: param7 should be a relation" );
  

  ListExpr attrname = nl->Nth(8,args);
  ListExpr attrtype;
  string aname_k = nl->SymbolValue(attrname);
  int k = listutils::findAttribute(nl->Second(nl->Second(param7)),
                      aname_k, attrtype);
  if(k == 0 || !listutils::isSymbol(attrtype,"bool"))
      return listutils::typeError("attr name" + aname_k + "not found"
                      "or not of type bool");


     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_pos"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("speed_limit"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("route_segment"),
                        nl->SymbolAtom("line"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j1), nl->IntAtom(j2),
                          nl->IntAtom(j), nl->IntAtom(k)),res);
}


/*
TypeMap fun for operator create bus segment speed 

*/

ListExpr OpTMCreateBusSegmentSpeedTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 11 )
  {
    return  nl->SymbolAtom ( "list length should be 11" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr attrName1 = nl->Second ( args );
  ListExpr attrType1;
  string aname1 = nl->SymbolValue(attrName1);
  int j1 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname1,attrType1);
                      
  if(j1 == 0 || !listutils::isSymbol(attrType1,"int")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type int");
  }
  
  ListExpr attrName2 = nl->Third(args);
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
                      aname3, attrType3);
                      
  if(j3 == 0 || !listutils::isSymbol(attrType3,"bool")){
      return listutils::typeError("attr name " + aname3 + "not found"
                      "or not of type bool");
  }
  
  
  ListExpr attrName4 = nl->Fifth(args);
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname4,attrType4);
  if(j4 == 0 || !listutils::isSymbol(attrType4,"bool"))
      return listutils::typeError("attr name " + aname4 + "not found"
                      "or not of type bool");

                      

  ListExpr param6 = nl->Sixth ( args );
  if(!IsRelDescription(param6))
    return nl->SymbolAtom ( "typeerror: param6 should be a relation" );
  

  ListExpr attrName_a = nl->Nth(7,args);
  ListExpr attrType_a;
  string aname_a = nl->SymbolValue(attrName_a);
  int j_a = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_a, attrType_a);
  if(j_a == 0 || !listutils::isSymbol(attrType_a,"real"))
      return listutils::typeError("attr name" + aname_a + "not found"
                      "or not of type real");

  ListExpr attrName_b = nl->Nth(8, args);
  ListExpr attrType_b;
  string aname_b = nl->SymbolValue(attrName_b);
  int j_b = listutils::findAttribute(nl->Second(nl->Second(param6)),
                      aname_b, attrType_b);
  if(j_b == 0 || !listutils::isSymbol(attrType_b,"bool"))
    return listutils::typeError("attr name" + aname_a + "not found"
                      "or not of type bool");

  ListExpr index1 = nl->Nth(9, args);
  if(!listutils::isBTreeDescription(index1))
      return  nl->SymbolAtom ( "parameter 9 should be btree" );
  
  
  ListExpr param10 = nl->Nth (10, args );
  if(!IsRelDescription(param10))
    return nl->SymbolAtom ( "typeerror: param9 should be a relation" );
  
  ListExpr index2 = nl->Nth(11, args);
  if(!listutils::isBTreeDescription(index2))
      return  nl->SymbolAtom ( "parameter 11 should be btree" );
  

/*     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_sub_route"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("speed_limit"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("startSmaller"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("start_loc"),
                        nl->SymbolAtom("point"))
                    )));*/

      ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->Cons(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_sub_route"),
                        nl->SymbolAtom("line")),
                    nl->TwoElemList(
                        nl->SymbolAtom("speed_limit"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("startSmaller"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("start_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("segment_id"),
                        nl->SymbolAtom("int")))
                    )));      
                    
//      cout<<"j1 "<<j1<<" j2 "<<j2<<" j3 "<<j3<<" j4 "<<j4<<endl;
//      cout<<"j_a "<<j_a<<" j_b "<<j_b<<" j_1 "<<j_1<<" j_2 "<<j_2<<endl; 
      
      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->SixElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                          nl->IntAtom(j3),nl->IntAtom(j4),
                          nl->IntAtom(j_a),nl->IntAtom(j_b)),
                          res);

}


/*
TypeMap fun for operator create night and daytime moving bus 

*/

ListExpr OpTMCreateMovingBusTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
 
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );
  
  ListExpr index = nl->Third(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "param3  should be btree" );
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_trip"),
                        nl->SymbolAtom("mpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_type"),
                        nl->SymbolAtom("string")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_day"),
                        nl->SymbolAtom("string")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_id"),
                        nl->SymbolAtom("int"))
                    )));
      return res; 
}


/*
TypeMap fun for operator create time table for each spatial location 

*/

ListExpr OpTMCreateTimeTableTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
 
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );
  
  ListExpr index = nl->Third(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "param3  should be btree" );
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->Cons(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_loc"),
                        nl->SymbolAtom("point")),
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_day"),
                        nl->SymbolAtom("string")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_time"),
                        nl->SymbolAtom("instant")),
                    nl->TwoElemList(
                        nl->SymbolAtom("loc_id"),
                        nl->SymbolAtom("int")))
                    )));
      return res; 
}


int GetContourSelect(ListExpr args)
{
    if(nl->IsEqual(nl->First(args),"text"))return 0;
    if(nl->IsEqual(nl->First(args),"int")) return 1;
    if(nl->IsEqual(nl->First(args),"real")) return 1;
    return -1;
}
//////////////////////////////////////////////////////////////////////////

/*
Correct road with dirt data, two segment are very close to each other and the
angle is relatively small.
In Berlin road network, there are three, 454,542 and 2324.

*/
int OpTMCheckSlinemap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  static int count = 1;

  result = qp->ResultStorage(in_pSupplier);
  SimpleLine* input_line = (SimpleLine*)args[0].addr;

  int width =((CcInt*)args[1].addr)->GetIntval();

  SimpleLine* res = static_cast<SimpleLine*>(result.addr);

  const double delta_dist = 0.1;
  vector<MyHalfSegment> segs;
  SpacePartition* lp = new SpacePartition();
  lp->ReorderLine(input_line, segs);
  vector<MyHalfSegment> boundary;

  int delta = width;
  bool clock_wise = true;
  for(unsigned int i = 0;i < segs.size();i++)
    lp->TransferSegment(segs[i], boundary, delta, clock_wise);


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

int OpTMModifyBoundarymap ( Word* args, Word& result, int message,
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

  SpacePartition* lp = new SpacePartition();
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

int OpTMSegment2Regionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
//        cout<<"open "<<endl;
        Relation* l = ( Relation* ) args[0].addr;
        int width = ((CcInt*)args[2].addr)->GetIntval();
        int attr_pos = ((CcInt*)args[3].addr)->GetIntval() - 1;

        l_partition = new SpacePartition(l);
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->ExtendRoad(attr_pos, width);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
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
            l_partition = (SpacePartition*)local.addr;
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

int OpTMPaveRegionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel1 = (Relation*)args[1].addr;
        Relation* rel2 = (Relation*)args[3].addr;
        int w = ((CcInt*)args[6].addr)->GetIntval();
        int attr_pos = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr_pos1 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[9].addr)->GetIntval() - 1;

        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->Getpavement(n,rel1,attr_pos,rel2,attr_pos1,attr_pos2,w);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
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
            l_partition = (SpacePartition*)local.addr;
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

int OpTMJunRegionmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel1 = (Relation*)args[1].addr;
        int width = ((CcInt*)args[4].addr)->GetIntval();

        int attr_pos1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[8].addr)->GetIntval() - 1;

        int attr_pos3 = ((CcInt*)args[9].addr)->GetIntval() - 1;

        Relation* rel2 = (Relation*)args[5].addr;

        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->Junpavement(n, rel1, attr_pos1, attr_pos2, width,
                                rel2, attr_pos3);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
          if(l_partition->count == l_partition->junid1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,l_partition->junid1[l_partition->count]));
/*          tuple->PutAttribute(1,
                new CcInt(true,l_partition->junid2[l_partition->count]));
          tuple->PutAttribute(2,
                new Line(l_partition->pave_line1[l_partition->count]));
          tuple->PutAttribute(3,
                new Line(l_partition->pave_line2[l_partition->count]));
          tuple->PutAttribute(4,
               new Region(l_partition->outer_regions1[l_partition->count]));
          tuple->PutAttribute(5,
               new Region(l_partition->outer_regions2[l_partition->count]));*/

          tuple->PutAttribute(1,
               new Region(l_partition->outer_regions1[l_partition->count]));


          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (SpacePartition*)local.addr;
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
        result[i].SetNoComponents(1);
        result[i].EndBulkLoad(false,false,false,false);
//        result[i].EndBulkLoad();
//        cout<<"Area "<<result[i].Area()<<endl;
    }
  }

};

/*
Value Mapping for the decomposeregion operator

*/

int OpTMDecomposeRegionmap ( Word* args, Word& result, int message,
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

/*
Value Mapping for the fillpavement operator
fill the hole between two pavements at some junction positions

*/

int OpTMFillPavementmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;
        int width = ((CcInt*)args[4].addr)->GetIntval();
        int attr_pos1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[6].addr)->GetIntval() - 1;

        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->FillHoleOfPave(n, rel, attr_pos1, attr_pos2, width);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
//          cout<<" count "<<l_partition->count<<endl;
//          cout<<"pave_line1 size() "<<l_partition->pave_line1.size()<<endl;
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
                new Region(l_partition->outer_fillgap1[l_partition->count]));
          tuple->PutAttribute(4,
                new Region(l_partition->outer_fillgap2[l_partition->count]));
          tuple->PutAttribute(5,
                new Region(l_partition->outer_fillgap[l_partition->count]));
          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (SpacePartition*)local.addr;
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
Value Mapping for the getpavenode1 operator
decompose the pavement of one road into a set of subregions

*/

int OpTMGetPaveNode1map ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;


        int attr_pos1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr_pos3 = ((CcInt*)args[7].addr)->GetIntval() - 1;


        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->DecomposePavement1(n, rel, attr_pos1, attr_pos2,
                                        attr_pos3);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
          if(l_partition->count == l_partition->junid1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,l_partition->junid1[l_partition->count]));
          tuple->PutAttribute(1,
                new CcInt(true,l_partition->junid2[l_partition->count]));
          tuple->PutAttribute(2,
               new Region(l_partition->outer_regions1[l_partition->count]));

          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (SpacePartition*)local.addr;
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
Value Mapping for the getpaveedge1 operator
get the commone area of two intersection pavements

*/

int OpTMGetPaveEdge1map ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr;

        int attr_pos1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr_pos3 = ((CcInt*)args[8].addr)->GetIntval() - 1;

        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->GetPavementEdge1(n, rel, btree,
                                    attr_pos1, attr_pos2, attr_pos3);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
          if(l_partition->count == l_partition->junid1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,l_partition->junid1[l_partition->count]));
          tuple->PutAttribute(1,
                new CcInt(true,l_partition->junid2[l_partition->count]));
          tuple->PutAttribute(2,
                new Line(l_partition->pave_line1[l_partition->count]));
          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (SpacePartition*)local.addr;
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
Value Mapping for the getpavenode2 operator
decompose the zebra crossings into a set of subregions

*/

int OpTMGetPaveNode2map ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        int start_oid = ((CcInt*)args[0].addr)->GetIntval();
        Relation* rel = (Relation*)args[1].addr;

        int attr_pos1 = ((CcInt*)args[4].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[5].addr)->GetIntval() - 1;


        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->DecomposePavement2(start_oid, rel, attr_pos1, attr_pos2);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
          if(l_partition->count == l_partition->junid1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,l_partition->junid1[l_partition->count]));
          tuple->PutAttribute(1,
                new CcInt(true,l_partition->junid2[l_partition->count]));
          tuple->PutAttribute(2,
               new Region(l_partition->outer_regions1[l_partition->count]));

          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (SpacePartition*)local.addr;
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
Value Mapping for the getpaveedge2 operator
get the commone area of two intersection pavements

*/

int OpTMGetPaveEdge2map ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  SpacePartition* l_partition;

  switch(message){
      case OPEN:{
        Relation* rel1 = (Relation*)args[0].addr;
        Relation* rel2 = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr;

        int attr_pos1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr_pos2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr_pos3 = ((CcInt*)args[8].addr)->GetIntval() - 1;

        l_partition = new SpacePartition();
        l_partition->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        l_partition->GetPavementEdge2(rel1, rel2, btree,
                                    attr_pos1, attr_pos2, attr_pos3);
        local.setAddr(l_partition);
        return 0;
      }
      case REQUEST:{
//          cout<<"request"<<endl;
          if(local.addr == NULL) return CANCEL;
          l_partition = (SpacePartition*)local.addr;
          if(l_partition->count == l_partition->junid1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(l_partition->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,l_partition->junid1[l_partition->count]));
          tuple->PutAttribute(1,
                new CcInt(true,l_partition->junid2[l_partition->count]));
          tuple->PutAttribute(2,
                new Line(l_partition->pave_line1[l_partition->count]));
          result.setAddr(tuple);
          l_partition->count++;
          return YIELD;
      }
      case CLOSE:{
//          cout<<"close"<<endl;
          if(local.addr){
            l_partition = (SpacePartition*)local.addr;
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
Value Mapping for the triangulate operator
decompose a polygon into a set of triangles

*/


int OpTMTriangulatemap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  CompTriangle* ct;
  switch(message){
      case OPEN:{
        Region* reg = (Region*)args[0].addr;
        ct = new CompTriangle(reg);
        ct->NewTriangulation();
        local.setAddr(ct);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ct = (CompTriangle*)local.addr;
          if(ct->count == ct->triangles.size())
                          return CANCEL;
          Region* reg = new Region(ct->triangles[ct->count]);
          result.setAddr(reg);
          ct->count++;
          return YIELD;
      }
      case CLOSE:{

          if(local.addr){
            ct = (CompTriangle*)local.addr;
            delete ct;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
Value Mapping for the convex operator
detect whether a polygon is convex or concave

*/


int OpTMConvexmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  Region* reg = (Region*)args[0].addr;
  CompTriangle* ct = new CompTriangle(reg);
  result = qp->ResultStorage(in_pSupplier);
  CcBool* res = static_cast<CcBool*>(result.addr);
  res->Set(true, ct->PolygonConvex());
  delete ct;
  return 0;
}


/*
Value Mapping for geospath  operator
return the geometric shortest path for two points inside a polgyon

*/


int OpTMGeospathmap_p ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  CompTriangle* ct;
  switch(message){
      case OPEN:{
        Point* p1 = (Point*)args[0].addr;
        Point* p2 = (Point*)args[1].addr;
        Region* reg = (Region*)args[2].addr;
        ct = new CompTriangle(reg);
        ct->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        ct->GeoShortestPath(p1, p2);
        local.setAddr(ct);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ct = (CompTriangle*)local.addr;
          if(ct->count == ct->sleeve.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(ct->resulttype);
          tuple->PutAttribute(0,new Line(*(ct->path)));
          tuple->PutAttribute(1,new Region(ct->sleeve[ct->count]));
          result.setAddr(tuple);
          ct->count++;
          return YIELD;
      }
      case CLOSE:{

          if(local.addr){
            ct = (CompTriangle*)local.addr;
            delete ct;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
Value Mapping for  createdualgraph  operator

*/

int OpTMCreateDGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  DualGraph* dg = (DualGraph*)qp->ResultStorage(in_pSupplier).addr;
  int dg_id = ((CcInt*)args[0].addr)->GetIntval();
  Relation* node_rel = (Relation*)args[1].addr;
  Relation* edge_rel = (Relation*)args[2].addr;
  dg->Load(dg_id, node_rel, edge_rel);
  result = SetWord(dg);
  return 0;
}

/*
Value Mapping for  nodedualgraph  operator

*/

int OpTMNodeDGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  DualGraph* dg = (DualGraph*)args[0].addr;
  Relation* node_rel = dg->GetNodeRel();
  result = SetWord(node_rel->Clone());
  Relation* resultSt = (Relation*)qp->ResultStorage(in_pSupplier).addr;
  resultSt->Close();
  qp->ChangeResultStorage(in_pSupplier, result);
  return 0;
}


/*
Value Mapping for walksp  operator
return the shortest path for pedestrian

*/

int OpTMWalkSPValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Walk_SP* wsp;
  switch(message){
      case OPEN:{
        DualGraph* dg = (DualGraph*)args[0].addr;
        VisualGraph* vg = (VisualGraph*)args[1].addr;
        Relation* r1 = (Relation*)args[2].addr;
        Relation* r2 = (Relation*)args[3].addr;
        Relation* r3 = (Relation*)args[4].addr;
        Relation* r4 = (Relation*)args[5].addr;

        wsp = new Walk_SP(dg, vg, r1, r2);
        wsp->rel3 = r3;
        wsp->rel4 = r4;
        wsp->btree = (BTree*)args[6].addr;
        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        wsp->WalkShortestPath();
        local.setAddr(wsp);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          wsp = (Walk_SP*)local.addr;
          if(wsp->count == wsp->oids1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(wsp->resulttype);
          tuple->PutAttribute(0, new CcInt(true,wsp->oids1[wsp->count]));
          tuple->PutAttribute(1, new CcInt(true, wsp->oids2[wsp->count]));
          tuple->PutAttribute(2, new Line(wsp->path[wsp->count]));
          result.setAddr(tuple);
          wsp->count++;
          return YIELD;
      }
      case CLOSE:{

          if(local.addr){
            wsp = (Walk_SP*)local.addr;
            delete wsp;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
Value Mapping for generatewp1  operator
generate random points insidy pavement polgyon

*/

int OpTMGenerateWP1ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Walk_SP* wsp;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        int no_p = ((CcInt*)args[1].addr)->GetIntval();
        wsp = new Walk_SP(NULL, NULL, r, NULL);
        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        wsp->GenerateData1(no_p);
        local.setAddr(wsp);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          wsp = (Walk_SP*)local.addr;
          if(wsp->count == wsp->oids.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(wsp->resulttype);
          tuple->PutAttribute(0,new CcInt(true, wsp->oids[wsp->count]));
          tuple->PutAttribute(1, new Point(wsp->q_loc1[wsp->count]));
          tuple->PutAttribute(2, new Point(wsp->q_loc2[wsp->count]));
          result.setAddr(tuple);
          wsp->count++;
          return YIELD;
      }
      case CLOSE:{

          if(local.addr){
            wsp = (Walk_SP*)local.addr;
            delete wsp;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
Value Mapping for generatewp2  operator
generate random vertices of a polygon

*/

int OpTMGenerateWP2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Walk_SP* wsp;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        int no_p = ((CcInt*)args[1].addr)->GetIntval();
        wsp = new Walk_SP(NULL, NULL, r, NULL);
        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        wsp->GenerateData2(no_p);
        local.setAddr(wsp);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          wsp = (Walk_SP*)local.addr;
          if(wsp->count == wsp->oids.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(wsp->resulttype);
          tuple->PutAttribute(0,new CcInt(true, wsp->oids[wsp->count]));
          tuple->PutAttribute(1, new Point(wsp->q_loc1[wsp->count]));
          tuple->PutAttribute(2, new Point(wsp->q_loc2[wsp->count]));
          result.setAddr(tuple);
          wsp->count++;
          return YIELD;
      }
      case CLOSE:{

          if(local.addr){
            wsp = (Walk_SP*)local.addr;
            delete wsp;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}
/*
Value Mapping for generatewp3  operator
generate random points inside polygon (internal point. not on the
polygon edge)

*/

int OpTMGenerateWP3ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Walk_SP* wsp;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        int no_p = ((CcInt*)args[1].addr)->GetIntval();
        wsp = new Walk_SP(NULL, NULL, r, NULL);
        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        wsp->GenerateData3(no_p);
        local.setAddr(wsp);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          wsp = (Walk_SP*)local.addr;
          if(wsp->count == wsp->oids.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(wsp->resulttype);
          tuple->PutAttribute(0,new CcInt(true, wsp->oids[wsp->count]));
          tuple->PutAttribute(1, new Point(wsp->q_loc1[wsp->count]));
          tuple->PutAttribute(2, new Point(wsp->q_loc2[wsp->count]));
          result.setAddr(tuple);
          wsp->count++;
          return YIELD;
      }
      case CLOSE:{

          if(local.addr){
            wsp = (Walk_SP*)local.addr;
            delete wsp;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
Value Mapping for zval  operator
get the z-order value of a point

*/

int OpTMZvalValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  result = qp->ResultStorage(in_pSupplier);
  Point* p = (Point*)args[0].addr;
  assert(p->IsDefined());
  ((CcInt*)result.addr)->Set(true, ZValue(*p));
  return 0;

}

/*
Value Mapping for zcurve  operator
calculate the curve of points sorted by z-order

*/
struct ZCurve{
  Relation* rel;
  unsigned int count;
  TupleType* resulttype;
  vector<Line> curve;
  ZCurve(){}
  ~ZCurve()
  {
      if(resulttype != NULL) delete resulttype;
  }
  ZCurve(Relation* r):rel(r), count(0), resulttype(NULL){}
  void BuildCurve(int attr_pos)
  {
    for(int i = 1;i < rel->GetNoTuples();i++){
      Tuple* t1 = rel->GetTuple(i, false);
      Point* p1 = (Point*)t1->GetAttribute(attr_pos);
      Tuple* t2 = rel->GetTuple(i + 1, false);
      Point* p2 = (Point*)t2->GetAttribute(attr_pos);
      Line* l = new Line(0);
      l->StartBulkLoad();
      HalfSegment hs;
      hs.Set(true, *p1, *p2);
      hs.attr.edgeno = 0;
      *l += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      *l += hs;
      l->EndBulkLoad();
      curve.push_back(*l);
      delete l;
      t1->DeleteIfAllowed();
      t2->DeleteIfAllowed();
    }
  }
};

/*
create a curve for the points sorted by z-order

*/

int OpTMZcurveValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  ZCurve* zc;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        int attr_pos = ((CcInt*)args[2].addr)->GetIntval() - 1;

        zc = new ZCurve(r);
        zc->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        zc->BuildCurve(attr_pos);
        local.setAddr(zc);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          zc = (ZCurve*)local.addr;
          if(zc->count == zc->curve.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(zc->resulttype);
          tuple->PutAttribute(0, new Line(zc->curve[zc->count]));
          result.setAddr(tuple);
          zc->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            zc = (ZCurve*)local.addr;
            delete zc;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}



/*
return all the vertices of a region and the cycle no. (vertex, cycleo)

*/
int OpTMRegVertexValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  RegVertex* rv;
  switch(message){
      case OPEN:{

        Region* r = (Region*)args[0].addr;
        rv = new RegVertex(r);
        rv->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        rv->CreateVertex();
        local.setAddr(rv);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rv = (RegVertex*)local.addr;
          if(rv->count == rv->cycleno.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(rv->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rv->cycleno[rv->count]));
          tuple->PutAttribute(1, new Point(rv->regnodes[rv->count]));
          result.setAddr(tuple);
          rv->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rv = (RegVertex*)local.addr;
            delete rv;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
decompose a region into a set of triangles where each is represented by the
three points. We number all vertices of a polygon. It works together with
operator regvertex.

*/

int OpTMTriangulationNewValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  RegVertex* rv;
  switch(message){
      case OPEN:{

        Region* r = (Region*)args[0].addr;
        rv = new RegVertex(r);
        rv->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        rv->TriangulationNew();
        local.setAddr(rv);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rv = (RegVertex*)local.addr;
          if(rv->count == rv->v1_list.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(rv->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rv->v1_list[rv->count]));
          tuple->PutAttribute(1, new CcInt(true, rv->v2_list[rv->count]));
          tuple->PutAttribute(2, new CcInt(true, rv->v3_list[rv->count]));
          tuple->PutAttribute(3, new Point(rv->regnodes[rv->count]));
          result.setAddr(tuple);
          rv->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rv = (RegVertex*)local.addr;
            delete rv;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
get the edge relation for the dual graph. it is based on the triangles by
decomposing a polygon. if two triangles are adjacent (sharing a common edge),
an edge is created.

*/

int OpTMGetDGEdgeValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  RegVertex* rv;
  switch(message){
      case OPEN:{

        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        rv = new RegVertex(r1,r2);
        rv->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        rv->GetDGEdge();
        local.setAddr(rv);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rv = (RegVertex*)local.addr;
          if(rv->count == rv->v1_list.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(rv->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rv->v1_list[rv->count]));
          tuple->PutAttribute(1, new CcInt(true, rv->v2_list[rv->count]));
          tuple->PutAttribute(2, new Line(rv->line[rv->count]));
          result.setAddr(tuple);
          rv->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rv = (RegVertex*)local.addr;
            delete rv;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
get the edge relation for the dual graph. it is based on the triangles by
decomposing a polygon. if two triangles are adjacent (sharing a common edge),
an edge is created. using R-tree to find neighbors

*/

int OpTMSMCDGTEValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  RegVertex* rv;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[1].addr;
        rv = new RegVertex(r, NULL);
        rv->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        rv->GetDGEdgeRTree(rtree);
        local.setAddr(rv);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rv = (RegVertex*)local.addr;
          if(rv->count == rv->v1_list.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(rv->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rv->v1_list[rv->count]));
          tuple->PutAttribute(1, new CcInt(true, rv->v2_list[rv->count]));
          tuple->PutAttribute(2, new Line(rv->line[rv->count]));
          result.setAddr(tuple);
          rv->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rv = (RegVertex*)local.addr;
            delete rv;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}
/*
find all visible nodes for a given point

*/

int OpTMGetVNodeValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  VGraph* vg;
  switch(message){
      case OPEN:{

        DualGraph* dg = (DualGraph*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr;
        Relation* r2 = (Relation*)args[2].addr;
        Relation* r3 = (Relation*)args[3].addr;

        vg = new VGraph(dg, r1, r2, r3);
        vg->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        vg->rel4 = (Relation*)args[4].addr;
        vg->btree = (BTree*)args[5].addr;

        vg->GetVNode();
        local.setAddr(vg);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          vg = (VGraph*)local.addr;
          if(vg->count == vg->oids1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(vg->resulttype);
          tuple->PutAttribute(0, new CcInt(true, vg->oids1[vg->count]));
          tuple->PutAttribute(1, new Point(vg->p_list[vg->count]));
          tuple->PutAttribute(2, new Line(vg->line[vg->count]));
          result.setAddr(tuple);
          vg->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            vg = (VGraph*)local.addr;
            delete vg;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
get the edge relation for the visibility graph

*/

int OpTMGetVGEdgeValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  VGraph* vg;
  switch(message){
      case OPEN:{

        DualGraph* dg = (DualGraph*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr;
        Relation* r2 = (Relation*)args[2].addr;


        vg = new VGraph(dg, r1, r2, r1);
        vg->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        vg->rel4 = (Relation*)args[3].addr;
        vg->btree = (BTree*)args[4].addr;

        vg->GetVGEdge();
        local.setAddr(vg);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          vg = (VGraph*)local.addr;
          if(vg->count == vg->oids2.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(vg->resulttype);
          tuple->PutAttribute(0, new CcInt(true, vg->oids2[vg->count]));
          tuple->PutAttribute(1, new CcInt(true, vg->oids3[vg->count]));
          tuple->PutAttribute(2, new Line(vg->line[vg->count]));
          result.setAddr(tuple);
          vg->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            vg = (VGraph*)local.addr;
            delete vg;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
operator myinside, it checks whether a line is completely inside a region

*/

int OpTMMyInsideValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Line* line = (Line*)args[0].addr;
  Region* reg = (Region*)args[1].addr;
  if(line->IsDefined() && reg->IsDefined()){
    ((CcBool*)result.addr)->Set(true, MyInside(line, reg));
  }else
    ((CcBool*)result.addr)->SetDefined(false);

  return 0;
}

/*
for a given node, find all its adjacent nodes

*/

int OpTMGetAdjNodeDGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  VGraph* vg;
  switch(message){
      case OPEN:{

        DualGraph* dg = (DualGraph*)args[0].addr;
        int oid = ((CcInt*)args[1].addr)->GetIntval();

        vg = new VGraph(dg, NULL, NULL, NULL);
        vg->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        vg->GetAdjNodeDG(oid);
        local.setAddr(vg);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          vg = (VGraph*)local.addr;
          if(vg->count == vg->oids1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(vg->resulttype);
          tuple->PutAttribute(0, new CcInt(true, vg->oids1[vg->count]));
          tuple->PutAttribute(1, new Region(vg->regs[vg->count]));
          result.setAddr(tuple);
          vg->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            vg = (VGraph*)local.addr;
            delete vg;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
for a given node, find all its adjacent nodes

*/

int OpTMGetAdjNodeVGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  VGraph* vg;
  switch(message){
      case OPEN:{

        VisualGraph* dg = (VisualGraph*)args[0].addr;
        int oid = ((CcInt*)args[1].addr)->GetIntval();

        vg = new VGraph(dg);
        vg->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        vg->GetAdjNodeVG(oid);
        local.setAddr(vg);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          vg = (VGraph*)local.addr;
          if(vg->count == vg->oids1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(vg->resulttype);
          tuple->PutAttribute(0, new CcInt(true, vg->oids1[vg->count]));
          tuple->PutAttribute(1, new Point(vg->p_list[vg->count]));
          tuple->PutAttribute(2, new Line(vg->line[vg->count]));
          result.setAddr(tuple);
          vg->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            vg = (VGraph*)local.addr;
            delete vg;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}
/*
decompose the triangle relation. it outputs the pair (vid triid).
for each vertex, it reports which triangle it belgons to.

*/

int OpTMDecomposeTriValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  VGraph* vg;
  switch(message){
      case OPEN:{

        Relation* rel = (Relation*)args[0].addr;
        vg = new VGraph(NULL, rel, NULL, NULL);
        vg->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        vg->DecomposeTriangle();
        local.setAddr(vg);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          vg = (VGraph*)local.addr;
          if(vg->count == vg->oids1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(vg->resulttype);
          tuple->PutAttribute(0, new CcInt(true, vg->oids1[vg->count]));
          tuple->PutAttribute(1, new CcInt(true, vg->oids2[vg->count]));
          result.setAddr(tuple);
          vg->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            vg = (VGraph*)local.addr;
            delete vg;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
Value Mapping for  createvgraph  operator

*/

int OpTMCreateVGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  VisualGraph* vg = (VisualGraph*)qp->ResultStorage(in_pSupplier).addr;
  int vg_id = ((CcInt*)args[0].addr)->GetIntval();
  Relation* node_rel = (Relation*)args[1].addr;
  Relation* edge_rel = (Relation*)args[2].addr;
  vg->Load(vg_id, node_rel, edge_rel);
  result = SetWord(vg);
  return 0;
}

/*
for operator getcontour
from the data file, it collects a set of regions

*/
int OpTMGetContourValueMapFile ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Hole* hole;
  switch(message){
      case OPEN:{

        FText* arg = static_cast<FText*>(args[0].addr);
        hole = new Hole(arg->Get());
        hole->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        hole->GetContour();
        local.setAddr(hole);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          hole = (Hole*)local.addr;
          if(hole->count == hole->regs.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(hole->resulttype);
          tuple->PutAttribute(0, new CcInt(true, hole->count+1));
          tuple->PutAttribute(1, new Region(hole->regs[hole->count]));
          result.setAddr(tuple);
          hole->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            hole = (Hole*)local.addr;
            delete hole;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
for operator getcontour
it randomly creates a set of regions

*/
int OpTMGetContourValueMapInt ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Hole* hole;
  switch(message){
      case OPEN:{
        unsigned int no_reg = ((CcInt*)(args[0].addr))->GetIntval();
        hole = new Hole();
        hole->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        hole->GetContour(no_reg); //no_reg polygons
        local.setAddr(hole);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          hole = (Hole*)local.addr;
          if(hole->count == hole->regs.size()) return CANCEL;

          Tuple* tuple = new Tuple(hole->resulttype);
          tuple->PutAttribute(0, new CcInt(true, hole->count+1));
          tuple->PutAttribute(1, new Region(hole->regs[hole->count]));
          result.setAddr(tuple);
          hole->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            hole = (Hole*)local.addr;
            delete hole;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
generates a polygon with the given number of vertices

*/
int OpTMGetContourValueMapReal ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Hole* hole;
  switch(message){
      case OPEN:{
        int no_reg = ((CcInt*)(args[0].addr))->GetIntval();
        hole = new Hole();
        hole->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        hole->GetPolygon(no_reg); //one polygon with no_reg vertices
        local.setAddr(hole);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          hole = (Hole*)local.addr;
          if(hole->count == hole->regs.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(hole->resulttype);
          tuple->PutAttribute(0, new CcInt(true, hole->count+1));
          tuple->PutAttribute(1, new Region(hole->regs[hole->count]));
          result.setAddr(tuple);
          hole->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            hole = (Hole*)local.addr;
            delete hole;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
generates a region with a lot of holes inside
the first input contour is set as the outer contour

*/
int OpTMGetPolygonValueMap(Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier)
{
  result = qp->ResultStorage(in_pSupplier);
  Region* reg = (Region*)result.addr;
  reg->Clear();
  reg->StartBulkLoad();

  Relation* r = (Relation*)args[0].addr;
  int pos = ((CcInt*)args[2].addr)->GetIntval() - 1;
  int edgeno = 0;
  for(int i = 1;i <= r->GetNoTuples();i++){
      Tuple* reg_tuple = r->GetTuple(i, false);
      Region* contour = (Region*)reg_tuple->GetAttribute(pos);
      for(int j = 0;j < contour->Size();j++){
          HalfSegment hs;
          contour->Get(j, hs);
          if(i == 1){
            *reg += hs;
          }else{
            HalfSegment temp_hs(hs);
            temp_hs.attr.cycleno = i - 1;
            temp_hs.attr.insideAbove = !hs.attr.insideAbove;
            temp_hs.attr.edgeno = edgeno + hs.attr.edgeno;
            *reg += temp_hs;
//            cout<<"hs edgeno "<<temp_hs.attr.edgeno<<endl;
          }
      }
      edgeno += contour->Size()/2;
//      cout<<"edgeno "<<edgeno<<endl;
      reg_tuple->DeleteIfAllowed();

  }
  reg->SetNoComponents(1);
  reg->EndBulkLoad(true, true, true, false);
  return 0;
}

/*
get all vertices of a polygon together with its two neighbors

*/
int OpTMGetAllPointsValueMap( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  CompTriangle* ct;
  switch(message){
      case OPEN:{
        Region* reg = (Region*)args[0].addr;
        ct = new CompTriangle(reg);
        ct->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        ct->GetAllPoints(); //one polygon with no_reg vertices
        local.setAddr(ct);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ct = (CompTriangle*)local.addr;
          if(ct->count == ct->plist1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(ct->resulttype);
          tuple->PutAttribute(0, new Point(ct->plist1[ct->count]));
          tuple->PutAttribute(1, new Point(ct->plist2[ct->count]));
          tuple->PutAttribute(2, new Point(ct->plist3[ct->count]));
          tuple->PutAttribute(3, new CcInt(true, ct->reg_id[ct->count]));
          result.setAddr(tuple);
          ct->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ct = (CompTriangle*)local.addr;
            delete ct;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
rotational plane sweep to get all visible points

*/
int OpTMRotationSweepValueMap( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  CompTriangle* ct;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        Rectangle<2>* rect = (Rectangle<2>*)args[2].addr;
        Relation* r3 = (Relation*)args[3].addr;
        int attr_pos = ((CcInt*)args[5].addr)->GetIntval() - 1;

        ct = new CompTriangle();
        ct->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        //one polygon with no_reg vertices
        ct->GetVPoints(r1, r2, rect, r3, attr_pos);
        local.setAddr(ct);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ct = (CompTriangle*)local.addr;
          if(ct->count == ct->plist1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(ct->resulttype);
          tuple->PutAttribute(0, new Point(ct->plist1[ct->count]));
          tuple->PutAttribute(1, new Line(ct->connection[ct->count]));
//          tuple->PutAttribute(2, new CcReal(true,ct->angles[ct->count]));
          result.setAddr(tuple);
          ct->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ct = (CompTriangle*)local.addr;
            delete ct;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
collect all holes of a region and each is represented a region.
change the inside above attribute

*/
int OpTMGetHoleValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Hole* hole;
  switch(message){
      case OPEN:{
        Region* reg = (Region*)args[0].addr;
        hole = new Hole();
        hole->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        hole->GetHole(reg);
        local.setAddr(hole);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          hole = (Hole*)local.addr;
          if(hole->count == hole->regs.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(hole->resulttype);
          tuple->PutAttribute(0, new CcInt(true, hole->count+1));
          tuple->PutAttribute(1, new Region(hole->regs[hole->count]));
          result.setAddr(tuple);
          hole->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            hole = (Hole*)local.addr;
            delete hole;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
get all possible sections of a route where interesting points can locate
for each route, it takes a set of sub routes 

*/
int OpTMGetSectionsValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  StrRS* routesec;
  switch(message){
      case OPEN:{
		Network* net = (Network*)args[0].addr; 
        Relation* r1 = (Relation*)args[1].addr;
		Relation* r2 = (Relation*)args[2].addr;
        routesec = new StrRS(net, r1,r2);
        routesec->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
		int pos1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
		int pos2 = ((CcInt*)args[7].addr)->GetIntval() - 1; 
		int pos3 = ((CcInt*)args[8].addr)->GetIntval() - 1;  

        routesec->GetSections(pos1, pos2, pos3);
        local.setAddr(routesec);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          routesec = (StrRS*)local.addr;
          if(routesec->count == routesec->rids.size())
                          return CANCEL;

            Tuple* tuple = new Tuple(routesec->resulttype);
		    int rid = routesec->rids[routesec->count];
//		  cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
            tuple->PutAttribute(0, new CcInt(true, rid));
            tuple->PutAttribute(1, new Line(routesec->lines[routesec->count]));
            result.setAddr(tuple);
            routesec->count++;
            return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            routesec = (StrRS*)local.addr;
            delete routesec;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}



/*
generate interesting points on pavement 

*/
int OpTMGenInterestP1ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  StrRS* routesec;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        int no_ps = ((CcInt*)args[6].addr)->GetIntval();
        routesec = new StrRS(NULL, r1,r2);
        routesec->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        int pos1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int pos2 = ((CcInt*)args[8].addr)->GetIntval() - 1; 
        int pos3 = ((CcInt*)args[9].addr)->GetIntval() - 1;  
        int pos4 = ((CcInt*)args[10].addr)->GetIntval() - 1;  

        routesec->GenPoints1(pos1, pos2, pos3, pos4, no_ps);
        local.setAddr(routesec);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          routesec = (StrRS*)local.addr;
          if(routesec->count == routesec->rids.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(routesec->resulttype);
          int rid = routesec->rids[routesec->count];
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
          tuple->PutAttribute(0, new CcInt(true, rid));
          tuple->PutAttribute(1, 
                            new Point(routesec->ps[routesec->count]));
          tuple->PutAttribute(2, 
                            new Point(routesec->interestps[routesec->count]));
          tuple->PutAttribute(3, 
                        new CcBool(true, routesec->ps_type[routesec->count]));
          result.setAddr(tuple);
          routesec->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            routesec = (StrRS*)local.addr;
            delete routesec;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
mapping the point into a triangle 

*/
int OpTMGenInterestP2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  StrRS* routesec;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[2].addr;
        unsigned int no_ps = ((CcInt*)args[5].addr)->GetIntval();

        routesec = new StrRS(NULL, r1, r2);
        routesec->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        int pos1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int pos2 = ((CcInt*)args[7].addr)->GetIntval() - 1; 
        
        routesec->GenPoints2(rtree, pos1, pos2, no_ps);
        local.setAddr(routesec);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          routesec = (StrRS*)local.addr;
          if(routesec->count == routesec->rids.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(routesec->resulttype);
          int rid = routesec->rids[routesec->count];
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
          tuple->PutAttribute(0, new CcInt(true, rid));
          tuple->PutAttribute(1, 
                            new Point(routesec->interestps[routesec->count]));
          tuple->PutAttribute(2, 
                            new Point(routesec->ps[routesec->count]));
          result.setAddr(tuple);
          routesec->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            routesec = (StrRS*)local.addr;
            delete routesec;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


struct Cell{
  int id;
  BBox<2> box;
  Cell(int i,BBox<2> b):id(i),box(b){}
  Cell(const Cell& cell):id(cell.id),box(cell.box){}
  Cell& operator=(const Cell& cell)
  {
    id = cell.id;
    box = cell.box;
    return *this;
  }
};

struct CellList{
  CellList(Rectangle<2>* b_box, unsigned int no):
  big_box(b_box),cell_no(no),count(0),resulttype(NULL){}
  void CreateCell();
  Rectangle<2>* big_box; 
  unsigned int cell_no;
  unsigned int count;
  TupleType* resulttype;
  vector<Cell> cell_array;
};
/*
partition the global box into a set of cell box 

*/
void CellList::CreateCell()
{
  double minx = big_box->MinD(0);
  double miny = big_box->MinD(1);
  double maxx = big_box->MaxD(0);
  double maxy = big_box->MaxD(1);
  
//  cout<<"minx "<<minx<<" maxx "<<maxx<<" miny "<<miny<<" maxy "<<maxy<<endl; 
  
  int cell_id = 0; 
  double y_value_min = miny;
  double x_value_min = minx; 
  double y_value_max;
  double x_value_max; 
  
  double y_interval = ceil((maxy - miny)/cell_no);
  double x_interval = ceil((maxx - minx)/cell_no);
  
  for(unsigned int i = 0;i < cell_no;i++){
    
    y_value_max = y_value_min + y_interval;
    
    x_value_min = minx;
    for(unsigned int j = 0;j < cell_no;j++){
      
      x_value_max = x_value_min + x_interval; 
      double min[2];
      double max[2];
      min[0] = x_value_min;
      min[1] = y_value_min;
      max[0] = x_value_max;
      max[1] = y_value_max;
      
      BBox<2>* box = new BBox<2>(true, min,max); 
      Cell* cell = new Cell(cell_id,*box);
      
      delete box; 
      cell_array.push_back(*cell); 
      cell_id++;
      x_value_min = x_value_max; 
    }
  
    y_value_min = y_value_max; 
  }
}

/*
partition the box into a set of cell box

*/
int OpTMCellBoxValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  CellList* box_list;
  switch(message){
      case OPEN:{
        Rectangle<2>* b_box = (Rectangle<2>*)args[0].addr;
        unsigned int cell_no = ((CcInt*)args[1].addr)->GetIntval();
        
        box_list = new CellList(b_box, cell_no);
        box_list->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        box_list->CreateCell();
        local.setAddr(box_list);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          box_list = (CellList*)local.addr;
          if(box_list->count == box_list->cell_array.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(box_list->resulttype);
          int id = box_list->cell_array[box_list->count].id;
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
          tuple->PutAttribute(0, new CcInt(true, id + 1));
          tuple->PutAttribute(1, 
//                 new Rectangle<2>(box_list->cell_array[box_list->count].box));
               new Region(box_list->cell_array[box_list->count].box));
               
          //x - number
          tuple->PutAttribute(2, new CcInt(true, id%box_list->cell_no + 1));
          //y - number 
          tuple->PutAttribute(3, new CcInt(true, id/box_list->cell_no + 1));
               
          result.setAddr(tuple);
          box_list->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            box_list = (CellList*)local.addr;
            delete box_list;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create bus routes1
a rough description for bus routes, start -cell and end cell 

*/
int OpTMCreateBusRouteValueMap1 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[6].addr;
        

        int attr1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        
        
        br = new BusRoute(n,r,btree);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateRoute1(attr1,attr2,attr3,attr4);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
//          if(br->count == br->bus_lines.size())return CANCEL;

          if(br->count == br->start_cells.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);
//          int id = br->count;
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
/*          tuple->PutAttribute(0, new CcInt(true, id + 1));
          tuple->PutAttribute(1, new Line(br->bus_lines[br->count]));*/

          tuple->PutAttribute(0, new CcInt(true,br->start_cell_id[br->count]));
          tuple->PutAttribute(1, new Rectangle<2>(br->start_cells[br->count]));
          tuple->PutAttribute(2, new CcInt(true,br->end_cell_id[br->count]));
          tuple->PutAttribute(3, new Rectangle<2>(br->end_cells[br->count]));
          tuple->PutAttribute(4, new CcInt(true,br->bus_route_type[br->count]));

          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create bus routes2
use the given a pair of cells to create bus routes.
because each cell intersects a set of sections.
use the sections to identify a start and end location (randomly location)

*/
int OpTMCreateBusRouteValueMap2 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[3].addr;
        Relation* r2 = (Relation*)args[4].addr; 
        
        int attr = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr1 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[11].addr)->GetIntval() - 1;

        br = new BusRoute(n,r1,btree,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateRoute2(attr,attr1,attr2,attr3);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->bus_lines1.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);
          int id = br->count;
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
          tuple->PutAttribute(0, new CcInt(true, id + 1));
//          tuple->PutAttribute(1, new Line(br->bus_lines[br->count]));
          tuple->PutAttribute(1, new GLine(br->bus_lines1[br->count]));
          tuple->PutAttribute(2, new Line(br->bus_lines2[br->count]));
          tuple->PutAttribute(3, new Point(br->start_gp[br->count]));
          tuple->PutAttribute(4, new Point(br->end_gp[br->count]));
          tuple->PutAttribute(5, new CcInt(true,br->bus_route_type[br->count]));
//          tuple->PutAttribute(1, new Line(br->bus_sections1[br->count]));
//          tuple->PutAttribute(2, new Line(br->bus_sections2[br->count]));
          
          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
refine bus routes. It filters some bus routes which are very similar to each
other 

*/
int OpTMRefineBusRouteValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r = (Relation*)args[1].addr; 
        
        int attr1 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        int attr5 = ((CcInt*)args[12].addr)->GetIntval() - 1;
        int attr6 = ((CcInt*)args[13].addr)->GetIntval() - 1;

        br = new BusRoute(n,r, NULL, NULL);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->RefineBusRoute(attr1,attr2,attr3,attr4,attr5,attr6);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->bus_lines1.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);
          
          tuple->PutAttribute(0, new CcInt(true,br->br_id_list[br->count]));
          tuple->PutAttribute(1, new GLine(br->bus_lines1[br->count]));
          tuple->PutAttribute(2, new Line(br->bus_lines2[br->count]));
          tuple->PutAttribute(3, new Point(br->start_gp[br->count]));
          tuple->PutAttribute(4, new Point(br->end_gp[br->count]));
          tuple->PutAttribute(5, new CcInt(true,br->bus_route_type[br->count]));
          
          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create bus routes3
translate the bus route into two where one is for up and the other is for down

*/
int OpTMCreateBusRouteValueMap3 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Relation* r = (Relation*)args[0].addr; 
        float width = ((CcReal*)args[4].addr)->GetRealval(); 
        
        int attr1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        

        br = new BusRoute(NULL,r,NULL,NULL);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateRoute3(attr1,attr2,attr3, (int)width);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new Line(br->bus_lines2[br->count]));
          tuple->PutAttribute(2, new Line(br->bus_sections1[br->count]));
          tuple->PutAttribute(3, new CcInt(true,br->bus_route_type[br->count]));
          tuple->PutAttribute(4, new CcInt(true,br->br_uid_list[br->count]));

          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
create bus routes4
set the up and down section 

*/
int OpTMCreateBusRouteValueMap4 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[5].addr; 
        
        int attr1 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        
        int attr_a = ((CcInt*)args[12].addr)->GetIntval() - 1;
        int attr_b = ((CcInt*)args[13].addr)->GetIntval() - 1;
        

        br = new BusRoute(NULL,r1,NULL,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateRoute4(attr1,attr2,attr3,attr4,attr_a,attr_b);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new Line(br->bus_sections1[br->count]));
          tuple->PutAttribute(2, new CcInt(true,br->bus_route_type[br->count]));
          tuple->PutAttribute(3, new CcInt(true,br->br_uid_list[br->count]));
          tuple->PutAttribute(4, 
                              new CcBool(true,br->direction_flag[br->count]));
                              
          tuple->PutAttribute(5, 
                              new CcBool(true,br->startSmaller[br->count]));
          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
create bus stops
for each bus route, it creates a sequence of points on it as bus stops 

*/
int OpTMCreateBusStopValueMap1 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr; 
        
        int attr1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        
        br = new BusRoute(n,r1,NULL,NULL);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop1(attr1, attr2, attr3, attr4);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);          
          
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new CcInt(true,br->br_stop_id[br->count]));
          tuple->PutAttribute(2, new GPoint(br->bus_stop_loc_1[br->count]));
          tuple->PutAttribute(3, new Point(br->bus_stop_loc_2[br->count]));
          
          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
merge bus stops that are close to each other (in the same road section)
1) the length of the section is small
2) merge bus stops which are close to each in the same road section.

*/
int OpTMCreateBusStopValueMap2 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr; 
        
        int attr1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[7].addr)->GetIntval() - 1;

        br = new BusRoute(n,r1,NULL,NULL);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop2(attr1, attr2, attr3);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);          
          
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new CcInt(true,br->br_stop_id[br->count]));
          tuple->PutAttribute(2, new GPoint(br->bus_stop_loc_1[br->count]));
          tuple->PutAttribute(3, new Point(br->bus_stop_loc_2[br->count]));
          tuple->PutAttribute(4, new CcInt(true, br->sec_id_list[br->count]));
          
          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
merge bus stops that are close to each other (in different road section)
adjacent road section

*/
int OpTMCreateBusStopValueMap3 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr; 
        Relation* r2 = (Relation*)args[3].addr; 
        BTree* btree = (BTree*)args[7].addr;
        
        int attr = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr1 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[11].addr)->GetIntval() - 1;

        br = new BusRoute(n,r1,btree,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop3(attr,attr1, attr2, attr3);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);          
          
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new CcInt(true,br->br_stop_id[br->count]));
          tuple->PutAttribute(2, new GPoint(br->bus_stop_loc_1[br->count]));
          tuple->PutAttribute(3, new Point(br->bus_stop_loc_2[br->count]));
          tuple->PutAttribute(4, new CcBool(true,br->startSmaller[br->count]));

          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
change the position for bus stops (id,pos) after translate bus route
and translate the original bus stop to its up and down bus route.

*/
int OpTMCreateBusStopValueMap4 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[3].addr; 
        
        int attr_a = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr_b = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr1 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[12].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[13].addr)->GetIntval() - 1;
        

        br = new BusRoute(NULL,r1,NULL,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop4(attr_a, attr_b, attr1, attr2, attr3, attr4);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);          
          
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new CcInt(true, br->br_uid_list[br->count]));
          tuple->PutAttribute(2, new CcInt(true, br->br_stop_id[br->count]));
          tuple->PutAttribute(3, new Point(br->start_gp[br->count]));
          tuple->PutAttribute(4, new Point(br->end_gp[br->count]));
          tuple->PutAttribute(5, 
                              new CcReal(true,br->bus_stop_loc_3[br->count]));
                              
//          tuple->PutAttribute(5,new Line(br->bus_sections1[br->count])); 

          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
set the up and down value for each bus stop 

*/
int OpTMCreateBusStopValueMap5 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[2].addr; 
        
        int attr = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr1 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[12].addr)->GetIntval() - 1;
        int attr5 = ((CcInt*)args[13].addr)->GetIntval() - 1;
        

        br = new BusRoute(NULL,r1,NULL,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop5(attr, attr1, attr2, attr3, attr4, attr5);
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);          
          
          tuple->PutAttribute(0, new CcInt(true, br->br_id_list[br->count]));
          tuple->PutAttribute(1, new CcInt(true, br->br_uid_list[br->count]));
          tuple->PutAttribute(2, new CcInt(true, br->br_stop_id[br->count]));
          tuple->PutAttribute(3, new Point(br->start_gp[br->count]));
          tuple->PutAttribute(4, 
                              new CcReal(true,br->bus_stop_loc_3[br->count]));
                              
          tuple->PutAttribute(5,new CcBool(true,br->bus_stop_flag[br->count])); 

          result.setAddr(tuple);
          br->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            br = (BusRoute*)local.addr;
            delete br;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
map a string value to an int 

*/
int OpTMMapToIntValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  
  CcString* str = (CcString*)args[0].addr; 
  if(str->IsDefined()){
    int type = 0;
    string str1("Wichtige"); // 30 km/h ----------1 
    string str2("Haupt"); // 70 km/h ------------3
    string str3("Neben");// 50 km/h  ----------2 
    string str4("gesperrte");// -1  -----------0 
    
    string road_type = str->GetValue();
    
//    cout<<"road_type "<<road_type<<endl; 
    
    if(road_type.find(str1) != string::npos){
        type = 1;
    }    
    else if(road_type.find(str2) != string::npos){
        type = 3;
    }    
    else if(road_type.find(str3) != string::npos){
        type = 2;
    }    
    else if(road_type.find(str4) != string::npos){
        type = 0;   
    }    
    else assert(false); 

    ((CcInt*)result.addr)->Set(true, type);
  }else{
    ((CcInt*)result.addr)->Set(false, 0);
  }
    
  return 0;

}


/*
map a string value to an real 

*/
int OpTMMapToRealValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  
  CcString* str = (CcString*)args[0].addr; 
  if(str->IsDefined()){
    float speed = 0.0;
    string str1("Wichtige"); // 30 km/h ----------1 
    string str2("Haupt"); // 70 km/h ------------3
    string str3("Neben");// 50 km/h  ----------2 
    string str4("gesperrte");// -1  -----------0 
    
    string road_type = str->GetValue();
    
//    cout<<"road_type "<<road_type<<endl; 
    
    if(road_type.find(str1) != string::npos){
        speed = 30.0;
    }    
    else if(road_type.find(str2) != string::npos){
        speed = 70.0;
    }    
    else if(road_type.find(str3) != string::npos){
        speed = 50.0;
    }    
    else if(road_type.find(str4) != string::npos){
        speed = -1.0;   
    }    
    else assert(false); 

    ((CcReal*)result.addr)->Set(true, speed);
  }else{
    ((CcReal*)result.addr)->Set(false, 0.0);
  }
    
  return 0;

}


/*
distinguish daytime and night bus 

*/
int OpTMCreateRouteDensityValueMap1 ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[4].addr; 
        Relation* r2 = (Relation*)args[5].addr; 
        Periods* peri1 = (Periods*)args[8].addr;
        Periods* peri2 = (Periods*)args[9].addr;
        
        
        int attr1 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        int attr_a = ((CcInt*)args[12].addr)->GetIntval() - 1;
        int attr_b = ((CcInt*)args[13].addr)->GetIntval() - 1;
        
        rd = new RoadDenstiy(n,r1,r2,btree);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->GetNightRoutes(attr1, attr2, attr_a,attr_b, peri1, peri2);
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);          
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new CcInt(true, rd->traffic_no[rd->count]));
          tuple->PutAttribute(2, new Periods(rd->duration1[rd->count]));
          tuple->PutAttribute(3, new Periods(rd->duration2[rd->count]));
          
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
set time schedule for night bus

*/
int OpTMSetTSNightBusValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        Relation* r = (Relation*)args[0].addr; 
        Periods* peri1 = (Periods*)args[4].addr;
        Periods* peri2 = (Periods*)args[5].addr;
        
        
        int attr1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        
        
        rd = new RoadDenstiy(NULL,r,NULL,NULL);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->SetTSNightBus(attr1, attr2, attr3, peri1, peri2);
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);          
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new Periods(rd->duration1[rd->count]));
          tuple->PutAttribute(2, new Periods(rd->duration2[rd->count]));
          tuple->PutAttribute(3, new CcReal(true,rd->time_interval[rd->count]));
          
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
set time schedule for daytime bus

*/
int OpTMSetTSDayBusValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        Relation* r = (Relation*)args[0].addr; 
        Periods* peri1 = (Periods*)args[4].addr;
        Periods* peri2 = (Periods*)args[5].addr;
        
        
        int attr1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        
        
        rd = new RoadDenstiy(NULL,r,NULL,NULL);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->SetTSDayTimeBus(attr1, attr2, attr3, peri1, peri2);
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);          
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new Periods(rd->duration1[rd->count]));
          tuple->PutAttribute(2, new Periods(rd->duration2[rd->count]));
          tuple->PutAttribute(3, new CcReal(true,rd->time_interval[rd->count]));
          tuple->PutAttribute(4,new CcReal(true,rd->time_interval2[rd->count]));
          
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
set speed limit for each bus route 

*/
int OpTMSetTBRSpeedValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        Network* n = (Network*)args[0].addr; 
        Relation* r1 = (Relation*)args[1].addr; 
        Relation* r2 = (Relation*)args[4].addr; 
        Relation* r3 = (Relation*)args[6].addr; 
        
        int attr1 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr_sm = ((CcInt*)args[11].addr)->GetIntval() - 1;
        
        rd = new RoadDenstiy(n, r1,r2, r3,NULL);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->SetBRSpeed(attr1, attr2, attr, attr_sm);
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new CcReal(true, rd->br_pos[rd->count]));
          tuple->PutAttribute(2, new CcReal(true, rd->speed_limit[rd->count]));
          tuple->PutAttribute(3, new Line(rd->br_subroute[rd->count]));
          
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
set speed limit for each bus route segment  

*/
int OpTMCreateBusSegmentSpeedValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[5].addr; 
        Relation* r3 = (Relation*)args[9].addr; 
        BTree* btree1 = (BTree*)args[8].addr;
        BTree* btree2 = (BTree*)args[10].addr;
        
        
        int attr1 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[12].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[13].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[14].addr)->GetIntval() - 1;
        
        int attr_a = ((CcInt*)args[15].addr)->GetIntval() - 1;
        int attr_b = ((CcInt*)args[16].addr)->GetIntval() - 1;
        
        
        rd = new RoadDenstiy(r1,r2,r3,btree1,btree2);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->CreateSegmentSpeed(attr1, attr2, attr3, attr4, 
                               attr_a, attr_b);
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new CcBool(true, rd->br_direction[rd->count]));
          tuple->PutAttribute(2, new Line(rd->br_subroute[rd->count])); 
          tuple->PutAttribute(3, new CcReal(true, rd->speed_limit[rd->count]));
          tuple->PutAttribute(4, new CcBool(true, rd->startSmaller[rd->count]));
          tuple->PutAttribute(5, new Point(rd->start_loc_list[rd->count]));
          tuple->PutAttribute(6, 
                              new CcInt(true,rd->segment_id_list[rd->count]));

          
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create night moving bus 

*/
int OpTMCreateNightBusValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 
        
        rd = new RoadDenstiy(NULL,r1,r2, btree);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->CreateNightBus();
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new CcBool(true, rd->br_direction[rd->count]));
          tuple->PutAttribute(2, new MPoint(rd->bus_trip[rd->count])); 
          tuple->PutAttribute(3, new CcString(true, rd->trip_type[rd->count]));
          tuple->PutAttribute(4, new CcString(true, rd->trip_day[rd->count])); 
          tuple->PutAttribute(5, 
                              new CcInt(true, rd->schedule_id_list[rd->count]));
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
create daytime moving bus 

*/
int OpTMCreateDayTimeBusValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 
        
        rd = new RoadDenstiy(NULL,r1,r2, btree);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->CreateDayTimeBus();
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->br_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(1, new CcBool(true, rd->br_direction[rd->count]));
          tuple->PutAttribute(2, new MPoint(rd->bus_trip[rd->count])); 
          tuple->PutAttribute(3, new CcString(true, rd->trip_type[rd->count]));
          tuple->PutAttribute(4, new CcString(true, rd->trip_day[rd->count])); 
          tuple->PutAttribute(5, 
                              new CcInt(true, rd->schedule_id_list[rd->count]));
          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
create time table for each spatial location

*/
int OpTMCreateTimeTableValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 
        
        rd = new RoadDenstiy(NULL,r1,r2, btree);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->CreateTimeTable();
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->bus_stop_loc.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new Point(rd->bus_stop_loc[rd->count])); 
          tuple->PutAttribute(1, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(2, 
                              new CcInt(true, rd->bus_stop_id_list[rd->count]));
          tuple->PutAttribute(3, new CcBool(true, rd->br_direction[rd->count]));
          tuple->PutAttribute(4, new CcString(true, rd->trip_day[rd->count])); 
          tuple->PutAttribute(5, new Instant(rd->schedule_time[rd->count])); 
          tuple->PutAttribute(6, 
                              new CcInt(true, rd->unique_id_list[rd->count]));

          result.setAddr(tuple);
          rd->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            rd = (RoadDenstiy*)local.addr;
            delete rd;
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
    OpTMCheckSlineSpec,          // specification
    OpTMCheckSlinemap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMCheckSlineTypeMap        // type mapping
);

Operator modifyboundary (
    "modifyboundary",               // name
    OpTMModifyBoundarySpec,          // specification
    OpTMModifyBoundarymap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMModifyBoundaryTypeMap        // type mapping
);

Operator segment2region(
    "segment2region",               // name
    OpTMSegment2RegionSpec,          // specification
    OpTMSegment2Regionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMSegment2RegionTypeMap        // type mapping
);

Operator paveregion(
    "paveregion",               // name
    OpTMPaveRegionSpec,          // specification
    OpTMPaveRegionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMPaveRegionTypeMap        // type mapping
);

Operator junregion(
    "junregion",               // name
    OpTMJunRegionSpec,          // specification
    OpTMJunRegionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMJunRegionTypeMap        // type mapping
);

Operator decomposeregion(
    "decomposeregion",               // name
    OpTMDecomposeRegionSpec,          // specification
    OpTMDecomposeRegionmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMDecomposeRegionTypeMap        // type mapping
);

Operator fillpavement(
    "fillpavement",               // name
    OpTMFillPavementSpec,          // specification
    OpTMFillPavementmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMFillPavementTypeMap        // type mapping
);

Operator getpavenode1(
    "getpavenode1",               // name
    OpTMGetPaveNode1Spec,          // specification
    OpTMGetPaveNode1map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPaveNode1TypeMap        // type mapping
);


Operator getpaveedge1(
    "getpaveedge1",               // name
    OpTMGetPaveEdge1Spec,          // specification
    OpTMGetPaveEdge1map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPaveEdge1TypeMap        // type mapping
);


Operator getpavenode2(
    "getpavenode2",               // name
    OpTMGetPaveNode2Spec,          // specification
    OpTMGetPaveNode2map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPaveNode2TypeMap        // type mapping
);

Operator getpaveedge2(
    "getpaveedge2",               // name
    OpTMGetPaveEdge2Spec,          // specification
    OpTMGetPaveEdge2map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPaveEdge2TypeMap        // type mapping
);

Operator triangulation(
    "triangulation",               // name
    OpTMTriangulateSpec,          // specification
    OpTMTriangulatemap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMTriangulateTypeMap        // type mapping
);

Operator convex(
    "convex",               // name
    OpTMConvexSpec,          // specification
    OpTMConvexmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMConvexTypeMap        // type mapping
);

Operator geospath(
    "geospath",               // name
    OpTMGeospathSpec,          // specification
    OpTMGeospathmap_p,  // value mapping
    Operator::SimpleSelect,
    OpTMGeospathTypeMap        // type mapping
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

Operator createdualgraph(
    "createdualgraph",
    OpTMCreateDGSpec,
    OpTMCreateDGValueMap,
    Operator::SimpleSelect,
    OpTMCreateDGTypeMap
);


Operator nodedualgraph(
    "nodedualgraph",
    OpTMNodeDGSpec,
    OpTMNodeDGValueMap,
    Operator::SimpleSelect,
    OpTMNodeDGTypeMap
);

Operator walk_sp(
    "walk_sp",
    OpTMWalkSPSpec,
    OpTMWalkSPValueMap,
    Operator::SimpleSelect,
    OpTMWalkSPTypeMap
);

Operator generate_wp1(
    "generate_wp1",
    OpTMGenerateWPSpec,
    OpTMGenerateWP1ValueMap,
    Operator::SimpleSelect,
    OpTMGenerateWPTypeMap
);

Operator generate_wp2(
    "generate_wp2",
    OpTMGenerateWPSpec,
    OpTMGenerateWP2ValueMap,
    Operator::SimpleSelect,
    OpTMGenerateWPTypeMap
);


Operator generate_wp3(
    "generate_wp3",
    OpTMGenerateWPSpec,
    OpTMGenerateWP3ValueMap,
    Operator::SimpleSelect,
    OpTMGenerateWPTypeMap
);

Operator zval(
    "zval",
    OpTMZvalSpec,
    OpTMZvalValueMap,
    Operator::SimpleSelect,
    OpTMZvalTypeMap
);


Operator zcurve(
    "zcurve",
    OpTMZcurveSpec,
    OpTMZcurveValueMap,
    Operator::SimpleSelect,
    OpTMZcurveTypeMap
);


Operator regvertex(
    "regvertex",
    OpTMRegVertexSpec,
    OpTMRegVertexValueMap,
    Operator::SimpleSelect,
    OpTMRegVertexTypeMap
);

Operator triangulation_new(
    "triangulation_new",
    OpTMTriangulationNewSpec,
    OpTMTriangulationNewValueMap,
    Operator::SimpleSelect,
    OpTMTriangulationNewTypeMap
);

Operator get_dg_edge(
    "get_dg_edge",
    OpTMGetDGEdgeSpec,
    OpTMGetDGEdgeValueMap,
    Operator::SimpleSelect,
    OpTMGetDgEdgeTypeMap
);

Operator smcdgte(
    "smcdgte",
    OpTMSMCDGTESpec,
    OpTMSMCDGTEValueMap,
    Operator::SimpleSelect,
    OpTMSMCDGTETypeMap
);

Operator getvnode(
    "getvnode",
    OpTMGetVNodeSpec,
    OpTMGetVNodeValueMap,
    Operator::SimpleSelect,
    OpTMGetVNodeTypeMap
);

Operator getvgedge(
    "getvgedge",
    OpTMGetVGEdgeSpec,
    OpTMGetVGEdgeValueMap,
    Operator::SimpleSelect,
    OpTMGetVGEdgeTypeMap
);

Operator myinside(
    "myinside",
    OpTMMyInsideSpec,
    OpTMMyInsideValueMap,
    Operator::SimpleSelect,
    OpTMMyInsideTypeMap
);

Operator getadjnode_dg(
    "getadjnode_dg",
    OpTMGetAdjNodeDGSpec,
    OpTMGetAdjNodeDGValueMap,
    Operator::SimpleSelect,
    OpTMGetAdjNodeDGTypeMap
);

Operator decomposetri(
    "decomposetri",
    OpTMDecomposeTriSpec,
    OpTMDecomposeTriValueMap,
    Operator::SimpleSelect,
    OpTMDecomposeTriTypeMap
);


Operator createvgraph(
    "createvgraph",
    OpTMCreateVGSpec,
    OpTMCreateVGValueMap,
    Operator::SimpleSelect,
    OpTMCreateVGTypeMap
);

Operator getadjnode_vg(
    "getadjnode_vg",
    OpTMGetAdjNodeVGSpec,
    OpTMGetAdjNodeVGValueMap,
    Operator::SimpleSelect,
    OpTMGetAdjNodeVGTypeMap
);
ValueMapping getcontourVM[]=
{
  OpTMGetContourValueMapFile,
  OpTMGetContourValueMapInt,
  OpTMGetContourValueMapReal
};

Operator getcontour(
    "getcontour",
    OpTMGetContourSpec,
    3,
    getcontourVM,
    GetContourSelect,
    OpTMGetContourTypeMap
);

Operator getpolygon(
    "getpolygon",
    OpTMGetPolygonSpec,
    OpTMGetPolygonValueMap,
    Operator::SimpleSelect,
    OpTMGetPolygonTypeMap
);

Operator getallpoints(
    "getallpoints",
    OpTMGetAllPointsSpec,
    OpTMGetAllPointsValueMap,
    Operator::SimpleSelect,
    OpTMGetAllPointsTypeMap
);

Operator rotationsweep(
    "rotationsweep",
    OpTMRotationSweepSpec,
    OpTMRotationSweepValueMap,
    Operator::SimpleSelect,
    OpTMRotationSweepTypeMap
);

Operator gethole(
    "gethole",
    OpTMGetHoleSpec,
    OpTMGetHoleValueMap,
    Operator::SimpleSelect,
    OpTMGetHoleTypeMap
);

Operator getsections(
    "getsections",
    OpTMGetSectionsSpec,
    OpTMGetSectionsValueMap,
    Operator::SimpleSelect,
    OpTMGetSectionsTypeMap
);

Operator geninterestp1(
    "geninterestp1",
    OpTMGenInterestP1Spec,
    OpTMGenInterestP1ValueMap,
    Operator::SimpleSelect,
    OpTMGetInterestP1TypeMap
);

Operator geninterestp2(
    "geninterestp2",
    OpTMGenInterestP2Spec,
    OpTMGenInterestP2ValueMap,
    Operator::SimpleSelect,
    OpTMGetInterestP2TypeMap
);

Operator cellbox(
  "cellbox",
  OpTMCellBoxSpec,               
  OpTMCellBoxValueMap,
  Operator::SimpleSelect,
  OpTMCellBoxTypeMap
);

Operator create_bus_route1(
  "create_bus_route1",
  OpTMCreateBusRouteSpec1,               
  OpTMCreateBusRouteValueMap1,
  Operator::SimpleSelect,
  OpTMCreateBusRouteTypeMap1
);

Operator create_bus_route2(
  "create_bus_route2",
  OpTMCreateBusRouteSpec2,               
  OpTMCreateBusRouteValueMap2,
  Operator::SimpleSelect,
  OpTMCreateBusRouteTypeMap2
);

Operator refine_bus_route(
  "refine_bus_route",
  OpTMRefineBusRouteSpec,               
  OpTMRefineBusRouteValueMap,
  Operator::SimpleSelect,
  OpTMRefineBusRouteTypeMap
);


Operator create_bus_route3(
  "create_bus_route3",
  OpTMCreateBusRouteSpec3,               
  OpTMCreateBusRouteValueMap3,
  Operator::SimpleSelect,
  OpTMCreateBusRouteTypeMap3
);

Operator create_bus_route4(
  "create_bus_route4",
  OpTMCreateBusRouteSpec4,               
  OpTMCreateBusRouteValueMap4,
  Operator::SimpleSelect,
  OpTMCreateBusRouteTypeMap4
);


Operator create_bus_stop1(
  "create_bus_stop1",
  OpTMCreateBusStopSpec1,               
  OpTMCreateBusStopValueMap1,
  Operator::SimpleSelect,
  OpTMCreateBusStopTypeMap1
);


Operator create_bus_stop2(
  "create_bus_stop2",
  OpTMCreateBusStopSpec2,               
  OpTMCreateBusStopValueMap2,
  Operator::SimpleSelect,
  OpTMCreateBusStopTypeMap2
);

Operator create_bus_stop3(
  "create_bus_stop3",
  OpTMCreateBusStopSpec3,               
  OpTMCreateBusStopValueMap3,
  Operator::SimpleSelect,
  OpTMCreateBusStopTypeMap3
);


Operator create_bus_stop4(
  "create_bus_stop4",
  OpTMCreateBusStopSpec4,               
  OpTMCreateBusStopValueMap4,
  Operator::SimpleSelect,
  OpTMCreateBusStopTypeMap4
);

Operator create_bus_stop5(
  "create_bus_stop5",
  OpTMCreateBusStopSpec5,               
  OpTMCreateBusStopValueMap5,
  Operator::SimpleSelect,
  OpTMCreateBusStopTypeMap5
);

Operator maptoint(
  "maptoint",
  OpTMMapToIntSpec,               
  OpTMMapToIntValueMap,
  Operator::SimpleSelect,
  OpTMMapToInt
);

Operator maptoreal(
  "maptoreal",
  OpTMMapToRealSpec,               
  OpTMMapToRealValueMap,
  Operator::SimpleSelect,
  OpTMMapToReal
);

Operator get_route_density1(
  "get_route_density1",
  OpTMGetRouteDensity1Spec,               
  OpTMCreateRouteDensityValueMap1,
  Operator::SimpleSelect,
  OpTMGetRouteDensity1TypeMap
);

Operator set_ts_nightbus(
  "set_ts_nightbus",
  OpTMSETTSNightBusSpec,
  OpTMSetTSNightBusValueMap,
  Operator::SimpleSelect,
  OpTMSetTSNighbBusTypeMap
);

Operator set_ts_daybus(
  "set_ts_daybus",
  OpTMSETTSDayBusSpec,
  OpTMSetTSDayBusValueMap,
  Operator::SimpleSelect,
  OpTMSetTSDayBusTypeMap
);

Operator set_br_speed(
  "set_br_speed",
  OpTMSETBRSpeedBusSpec,
  OpTMSetTBRSpeedValueMap,
  Operator::SimpleSelect,
  OpTMSetBRSpeedTypeMap

);

Operator create_bus_segment_speed(
  "create_bus_segment_speed",
  OpTMCreateBusSegmentSpeedSpec,
  OpTMCreateBusSegmentSpeedValueMap,
  Operator::SimpleSelect,
  OpTMCreateBusSegmentSpeedTypeMap
);


Operator create_night_bus_mo(
  "create_night_bus_mo",
  OpTMCreateNightBusSpec,
  OpTMCreateNightBusValueMap,
  Operator::SimpleSelect,
  OpTMCreateMovingBusTypeMap
);

Operator create_daytime_bus_mo(
  "create_daytime_bus_mo",
  OpTMCreateDayTimeBusSpec,
  OpTMCreateDayTimeBusValueMap,
  Operator::SimpleSelect,
  OpTMCreateMovingBusTypeMap
);


Operator create_time_table(
  "create_time_table",
  OpTMCreateTimeTableSpec,
  OpTMCreateTimeTableValueMap,
  Operator::SimpleSelect,
  OpTMCreateTimeTableTypeMap
);


/*
Main Class for Transportation Mode

*/
class TransportationModeAlgebra : public Algebra
{
 public:
  TransportationModeAlgebra() : Algebra()
  {
    AddTypeConstructor(&dualgraph);
    dualgraph.AssociateKind("DUALGRAPH");
    AddTypeConstructor(&visualgraph);
    visualgraph.AssociateKind("VISUALGRAPH");
    ////operators for partition regions////
    AddOperator(&checksline);
    AddOperator(&modifyboundary);
    AddOperator(&segment2region);
    AddOperator(&paveregion);
    AddOperator(&junregion);
    AddOperator(&decomposeregion);
    AddOperator(&fillpavement);

    //////////operators for build the graph model on pavement////////////
    AddOperator(&getpavenode1);
    AddOperator(&getpaveedge1);
    AddOperator(&getpavenode2);
    AddOperator(&getpaveedge2);
    AddOperator(&triangulation);
    AddOperator(&convex);
    AddOperator(&geospath);
    AddOperator(&createdualgraph);
    AddOperator(&nodedualgraph);
    ///////////////////visibility graph///////////////////////////////
    AddOperator(&getvnode);
    AddOperator(&myinside);
    AddOperator(&decomposetri);
    AddOperator(&getvgedge);
    AddOperator(&createvgraph);
    AddOperator(&getadjnode_vg);
    AddOperator(&walk_sp);
    ///////////////////dual graph/////////////////////////////////////
    AddOperator(&zval);
    AddOperator(&zcurve);
    AddOperator(&regvertex);
    AddOperator(&triangulation_new);
    AddOperator(&get_dg_edge);
    AddOperator(&getadjnode_dg);
    AddOperator(&smcdgte);
    ////////////////////data process///////////////////////////////////
    AddOperator(&generate_wp1);
    AddOperator(&generate_wp2);
    AddOperator(&generate_wp3);
    AddOperator(&getcontour);
    AddOperator(&getpolygon);
    AddOperator(&getsections);	
    AddOperator(&geninterestp1);
    AddOperator(&geninterestp2);
    ///////////////rotational plane sweep algorithm////////////////////
    AddOperator(&getallpoints);
    AddOperator(&rotationsweep);
    AddOperator(&gethole);
    ////////////////////create bus network//////////////////////////
    AddOperator(&cellbox);
    AddOperator(&create_bus_route1); //rough representation
    AddOperator(&create_bus_route2); //create bus route 
    AddOperator(&refine_bus_route); //filter some bus routes which are similar 
    AddOperator(&create_bus_route3);//copy bus route, split 
    AddOperator(&create_bus_route4);//set up and down for bus routes 
    AddOperator(&create_bus_stop1); //create bus stops on bus routes
    AddOperator(&create_bus_stop2); //merge bus stops (same road section)
    AddOperator(&create_bus_stop3); //merge bus stops (adjacent section)
    AddOperator(&create_bus_stop4); //change bus stop position 
    AddOperator(&create_bus_stop5); //set up and down for bus stops 
    //////////////preprocess road data to get denstiy value/////////////
    AddOperator(&maptoint);
    AddOperator(&maptoreal);
    AddOperator(&get_route_density1);//get daytime and night bus routes 
    AddOperator(&set_ts_nightbus);//set time schedule for night bus 
    AddOperator(&set_ts_daybus);//set time schedule for daytime bus 
    AddOperator(&set_br_speed);// set speed value for each bus route 
    AddOperator(&create_bus_segment_speed); //set speed value for each segment 
    AddOperator(&create_night_bus_mo);//create night moving bus 
    AddOperator(&create_daytime_bus_mo);//create daytime moving bus 
    AddOperator(&create_time_table);//create time table for each spatial stop 
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
