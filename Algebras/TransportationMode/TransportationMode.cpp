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

March, 2010 Jianqiu Xu Create Pavements for Pedestrian 

Oct.,2010 Jianqiu Xu Create Bus Network and Trains 

Dec. 2010 Jianqiu Xu Move Indoor Algebra to Transporation Mode Algebra 


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
#include "Indoor.h"
#include "QueryTM.h"
#include "RoadNetwork.h"
#include <sys/timeb.h>

extern NestedList* nl;
extern QueryProcessor *qp;

double TM_DiffTimeb(struct timeb* t1, struct timeb* t2)
{
  double dt1 = t1->time + (double)t1->millitm/1000.0;
  double dt2 = t2->time + (double)t2->millitm/1000.0;
  return dt1 - dt2; 
}

namespace TransportationMode{
/////////////////////////////////////////////////////////////////////////////
////////////////////////   Indoor data Type//////////////////////////////////
///////////// point3d line3d floor3d door3d groom ///////////////////
//////////////////// functions are in Indoor.h  /////////////////////////////
////////////////////////////////////////////////////////////////////////////

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

const string SpatialSpecOidOfDoor =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>door3d x int -> line</text--->"
"<text>oid_of_door (_, _) </text--->"
"<text>get the room id </text--->"
"<text>query oid_of_door (door1,1)</text---> ) )";

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
"( <text>string x int -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>getindorpath(string, int) </text--->"
"<text>read indoor shortest path from disk files</text--->"
"<text>query getindoorpath(UNIVERSITY, 10001001) count </text---> ))";


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

  if(nl->IsEqual(nl->First(args), "floor3d"))
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
TypeMap function for operator oid of door

*/
ListExpr OidOfDoorTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "door3d x int";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "door3d") && nl->IsEqual(arg2, "int"))
      return nl->SymbolAtom("int");

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
                        nl->TwoElemList(nl->SymbolAtom("groom_oid"),
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
                        nl->TwoElemList(nl->SymbolAtom("groom_oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("groom_tid"),
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
                        nl->TwoElemList(nl->SymbolAtom("door_loc"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("groom_oid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("groom_oid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("door_loc3d"),
                                      nl->SymbolAtom("line3d")), 
                        nl->TwoElemList(nl->SymbolAtom("doorheight"),
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
//                      nl->FiveElemList(
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Door"),
                                    nl->SymbolAtom("door3d")), 
                        nl->TwoElemList(nl->SymbolAtom("door_loc"),
                                      nl->SymbolAtom("line")),
                        nl->TwoElemList(nl->SymbolAtom("groom_oid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("groom_oid2"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("door_loc3d"),
                                      nl->SymbolAtom("line3d")), 
                        nl->TwoElemList(nl->SymbolAtom("doorheight"),
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
//                      nl->FiveElemList(
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("groom_oid"),
                                    nl->SymbolAtom("int")), 
                        nl->TwoElemList(nl->SymbolAtom("door_tid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("door_tid2"),
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
                        nl->TwoElemList(nl->SymbolAtom("groom_oid"),
                                    nl->SymbolAtom("int")), 
                        nl->TwoElemList(nl->SymbolAtom("door_tid1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("door_tid2"),
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
                        nl->TwoElemList(nl->SymbolAtom("loc1"),
                                    nl->SymbolAtom("genloc")), 
                        nl->TwoElemList(nl->SymbolAtom("loc2"),
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
                        nl->TwoElemList(nl->SymbolAtom("groom_oid"),
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
        ListExpr result; 
        if(nl->SymbolValue(arg5) == "int"){
            result =
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
        }else if(nl->SymbolValue(arg5) == "real"){
            result =
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
        }
        return result;
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
ValueMap function for operator thefloor

*/
int TheFloorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  float h = ((CcReal*)args[0].addr)->GetRealval();
  Region* r = (Region*)args[1].addr;
  result = qp->ResultStorage(s);
  Floor3D* fl = (Floor3D*)result.addr;
  fl->SetValue(h, r);
  return 0;
}

/*
ValueMap function for operator getheight

*/
int GetHeightValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Floor3D* fl = (Floor3D*)args[0].addr;
  result = qp->ResultStorage(s);
  CcReal* res = (CcReal*)result.addr;
  if(!fl->IsDefined()) res->Set(false,0.0);
  else
      res->Set(true,fl->GetHeight());
  return 0;
}

/*
ValueMap function for operator getregion

*/
int GetRegionFloor3DValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Floor3D* fl = (Floor3D*)args[0].addr;
  result = qp->ResultStorage(s);
  Region* res = (Region*)result.addr;
  *res = Region(*fl->GetRegion());
  return 0;
}

/*
ValueMap function for operator getregion

*/
int GetRegionGRoomValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GRoom* gr = (GRoom*)args[0].addr;
  result = qp->ResultStorage(s);
  Region* res = (Region*)result.addr;
  Region r(0);
  gr->GetRegion(r);
  *res = r;
  return 0;
}


/*
ValueMap function for operator thedoor

*/
int TheDoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  int oid1 = ((CcInt*)args[0].addr)->GetIntval();
  Line* l1 = (Line*)args[1].addr;
  int oid2 = ((CcInt*)args[2].addr)->GetIntval();
  Line* l2 = (Line*)args[3].addr;
  MBool* mb = (MBool*)args[4].addr;
  bool b = ((CcBool*)args[5].addr)->GetBoolval();
  result = qp->ResultStorage(s);
  Door3D* dr = (Door3D*)result.addr;
  dr->SetValue(oid1, l1, oid2, l2, mb, b);
  return 0;
}

/*
ValueMap function for operator type of door 

*/
int TypeOfDoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Door3D* d = (Door3D*)args[0].addr; 
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  if(d->IsDefined())
    res->Set(true, d->GetDoorType());
  else
    res->Set(false,false);
  return 0;
}

/*
ValueMap function for operator oid of door 

*/
int OidOfDoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Door3D* d = (Door3D*)args[0].addr; 
  int index = ((CcInt*)args[1].addr)->GetIntval(); 
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  if(d->IsDefined() && (index == 1 || index == 2))
    res->Set(true, d->GetOid(index));
  if(index < 1 || index > 2)
    cout<<"index should be 1 or 2"<<endl; 
  return 0;
}


/*
ValueMap function for operator loc of door 

*/
int LocOfDoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Door3D* d = (Door3D*)args[0].addr; 
  int index = ((CcInt*)args[1].addr)->GetIntval(); 
  result = qp->ResultStorage(s);
  Line* res = static_cast<Line*>(result.addr);
  if(d->IsDefined() && (index == 1 || index == 2))
    *res = *(d->GetLoc(index));
  if(index < 1 || index > 2)
    cout<<"index should be 1 or 2"<<endl; 
  return 0;
}

/*
ValueMap function for operator state of door 

*/
int StateOfDoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Door3D* d = (Door3D*)args[0].addr; 
  result = qp->ResultStorage(s);
  MBool* res = static_cast<MBool*>(result.addr);
  if(d->IsDefined()){
//    *res = *(d->GetTState());
    res->StartBulkLoad();
    for(int i = 0;i < d->GetTState()->GetNoComponents();i++){
      UBool ub;
      d->GetTState()->Get(i, ub);
      res->Add(ub);
    }
    res->EndBulkLoad();
  }
  return 0;
}

/*
ValueMap function for operator get floor from a groom 

*/
int GetFloorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GRoom* gr = (GRoom*)args[0].addr;
  int index = ((CcInt*)args[1].addr)->GetIntval(); 
  result = qp->ResultStorage(s);
  Floor3D* f = static_cast<Floor3D*>(result.addr);
  if(gr->IsDefined() && index >= 0 && index < gr->Size()){
    float h;
    Region r(0);
    gr->Get(index, h, r);
    f->SetValue(h, &r);
  }
  return 0;
}


/*
ValueMap function for operator add height for a groom 

*/
int AddHeightGroomValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GRoom* gr = (GRoom*)args[0].addr;
  float h = ((CcReal*)args[1].addr)->GetRealval(); 
  result = qp->ResultStorage(s);
  GRoom* groom = static_cast<GRoom*>(result.addr);
  *groom = *gr;
  if(groom->IsDefined() && h > 0.0){
    groom->AddHeight(h);
  }
  return 0;
}

/*
ValueMap function for operator translate groom 

*/
int TranslateGRoomValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  GRoom* cr = (GRoom *)args[0].addr; 
  GRoom* pResult = (GRoom *)result.addr;
  
  pResult->Clear();
  Supplier son = qp->GetSupplier( args[1].addr, 0 );

  Word t;
  qp->Request( son, t );
  const CcReal *tx = ((CcReal *)t.addr);

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  const CcReal *ty = ((CcReal *)t.addr);

  if(  cr->IsDefined() && tx->IsDefined() && ty->IsDefined() ) {
      const Coord txval = (Coord)(tx->GetRealval()),
                  tyval = (Coord)(ty->GetRealval());
      cr->Translate( txval, tyval, *pResult );
  }
  else
    ((GRoom*)result.addr)->SetDefined( false );
  
  return 0;
}

/*
ValueMap function for operator tm length: line3d 

*/
int LengthLine3DValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  Line3D* l = (Line3D *)args[0].addr; 
  CcReal* pResult = static_cast<CcReal*>(result.addr);
  
  if(l->IsDefined() )
    pResult->Set(true, l->Length());
  else
    pResult->Set(false, 0);
  return 0;
}

/*
ValueMap function for operator tm length: genrange

*/
int LengthGenRangeValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  GenRange* gr = (GenRange *)args[0].addr; 
  CcReal* pResult = static_cast<CcReal*>(result.addr);

  if(gr->IsDefined() )
    pResult->Set(true, gr->Length());
  else
    pResult->Set(false, 0);
  return 0;
}

/*
ValueMap function for operator tm length: busroute 

*/
int LengthBusRouteValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  Bus_Route* br = (Bus_Route *)args[0].addr; 
  CcReal* pResult = static_cast<CcReal*>(result.addr);

  if(br->IsDefined() )
    pResult->Set(true, br->Length());
  else
    pResult->Set(false, 0);
  return 0;
}

/*
ValueMap function for operator bbox l3d  

*/
int BBoxLine3DValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  Line3D* l = (Line3D *)args[0].addr; 
  Rectangle<3>* pResult = static_cast<Rectangle<3>*>(result.addr);
  
  if(l->IsDefined() )
    *pResult = l->BoundingBox();
  else
    pResult->SetDefined(false);
  return 0;
}

/*
ValueMap function for operator bbox groom   

*/
int BBoxGRoomValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  GRoom* groom = (GRoom *)args[0].addr; 
  Rectangle<3>* pResult = static_cast<Rectangle<3>*>(result.addr);
  
  if(groom->IsDefined() )
    *pResult = groom->BoundingBox3D();
  else
    pResult->SetDefined(false);
  return 0;
}

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
ValueMap function for operator thebuilding

*/
int TheBuildingValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  Building* build = (Building*)qp->ResultStorage(in_pSupplier).addr;
  int id = ((CcInt*)args[0].addr)->GetIntval();
  string type = ((CcString*)args[1].addr)->GetValue(); 
  Relation* rel1 = (Relation*)args[2].addr;
  Relation* rel2 = (Relation*)args[3].addr;
  if(ChekBuildingId(id)){
    build->Load(id, GetBuildingType(type), rel1, rel2);
    result = SetWord(build);
  }else{
    cout<<"invalid building id "<<id<<endl; 
    while(ChekBuildingId(id) == false || id <= 0){
      id++;
    }
    cout<<"new building id "<<id<<endl;
    build->Load(id, GetBuildingType(type), rel1, rel2);
    result = SetWord(build);
  }
  return 0;
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
ValueMap function for operator theindoor 

*/
int TheIndoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  IndoorInfra* indoor = (IndoorInfra*)qp->ResultStorage(in_pSupplier).addr;
  int id = ((CcInt*)args[0].addr)->GetIntval();
  Relation* rel1 = (Relation*)args[1].addr;
  Relation* rel2 = (Relation*)args[2].addr;
  if(ChekIndoorId(id)){
    indoor->Load(id, rel1, rel2);
    result = SetWord(indoor);
  }else{
    cout<<"invalid indoor infrastructure id "<<id<<endl; 
    while(ChekIndoorId(id) == false || id <= 0){
      id++;
    }
    cout<<"new indoor infrastructure id "<<id<<endl;
    indoor->Load(id,  rel1, rel2);
    result = SetWord(indoor);
  }
  return 0;
}

/*
ValueMap function for operator createdoor3d

*/
int CreateDoor3DValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel = (Relation*)args[0].addr;

        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->CreateDoor3D();
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->groom_oid_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,indoor_nav->groom_oid_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new Line3D(indoor_nav->path_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
ValueMap function for operator createdoorbox 

*/
int CreateDoorBoxValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel = (Relation*)args[0].addr;

        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->CreateDoorBox();
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->oid_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true,indoor_nav->oid_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new CcInt(true,indoor_nav->tid_list[indoor_nav->count]));
          tuple->PutAttribute(2,
               new Rectangle<3>(indoor_nav->box_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
ValueMap function for operator createdoor1

*/
int CreateDoor1ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel1 = (Relation*)args[0].addr;
        Relation* rel2 = (Relation*)args[1].addr;
        R_Tree<3, TupleId>* rtree = (R_Tree<3, TupleId>*)args[2].addr;
        int attr1 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[8].addr)->GetIntval() - 1;

        indoor_nav = new IndoorNav(rel1, rel2);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->CreateDoor1(rtree, attr1, attr2, attr3);
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->line_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new Door3D(indoor_nav->door_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new Line(indoor_nav->line_list[indoor_nav->count]));
          tuple->PutAttribute(2,
                new CcInt(true, indoor_nav->groom_id_list1[indoor_nav->count]));
          tuple->PutAttribute(3,
                new CcInt(true, indoor_nav->groom_id_list2[indoor_nav->count]));
          tuple->PutAttribute(4,
                new Line3D(indoor_nav->path_list[indoor_nav->count]));
          tuple->PutAttribute(5,
                new CcReal(true, indoor_nav->door_heights[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
ValueMap function for operator createdoor2

*/
int CreateDoor2ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel1 = (Relation*)args[0].addr;
        
        indoor_nav = new IndoorNav(rel1, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->CreateDoor2();
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->line_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new Door3D(indoor_nav->door_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new Line(indoor_nav->line_list[indoor_nav->count]));
          tuple->PutAttribute(2,
                new CcInt(true, indoor_nav->groom_id_list1[indoor_nav->count]));
          tuple->PutAttribute(3,
                new CcInt(true, indoor_nav->groom_id_list2[indoor_nav->count]));
          tuple->PutAttribute(4,
                new Line3D(indoor_nav->path_list[indoor_nav->count]));
          tuple->PutAttribute(5,
                new CcReal(true, indoor_nav->door_heights[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
ValueMap function for operator createadjdoor1 

*/
int CreateAdjDoor1ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel1 = (Relation*)args[0].addr;
        Relation* rel2 = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr;
        int attr1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        
        indoor_nav = new IndoorNav(rel1, rel2);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->CreateAdjDoor1(btree, attr1, attr2, attr3, attr4);
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->groom_oid_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true, indoor_nav->groom_oid_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new CcInt(true, indoor_nav->door_tid_list1[indoor_nav->count]));
          tuple->PutAttribute(2,
                new CcInt(true, indoor_nav->door_tid_list2[indoor_nav->count]));
          tuple->PutAttribute(3,
                new Line3D(indoor_nav->path_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
ValueMap function for operator createadjdoor2 

*/
int CreateAdjDoor2ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel = (Relation*)args[0].addr;
        R_Tree<3,TupleId>* rtree = (R_Tree<3,TupleId>*)args[1].addr;

        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->CreateAdjDoor2(rtree);
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->groom_oid_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new CcInt(true, indoor_nav->groom_oid_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new CcInt(true, indoor_nav->door_tid_list1[indoor_nav->count]));
          tuple->PutAttribute(2,
                new CcInt(true, indoor_nav->door_tid_list2[indoor_nav->count]));
          tuple->PutAttribute(3,
                new Line3D(indoor_nav->path_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
ValueMap function for operator path in region  

*/
int PathInRegionValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  result = qp->ResultStorage( in_pSupplier );
  Region* reg = (Region *)args[0].addr; 
  Point* s = (Point *)args[1].addr; 
  Point* d = (Point *)args[2].addr; 
  
  Line* pResult = (Line *)result.addr;
  
  CompTriangle* ct = new CompTriangle(reg);
  if(ct->ComplexRegion() == 0)
    ShortestPath_InRegion(reg, s, d, pResult);
  else if(ct->ComplexRegion() == 1){
      ShortestPath_InRegionNew(reg, s, d, pResult);
  }
  else
      cout<<"error "<<endl; 

  delete ct; 
  return 0;
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

/*
Value Mapping for  createigraph  operator

*/

int OpTMCreateIGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
//   IndoorGraph* ig = (IndoorGraph*)qp->ResultStorage(in_pSupplier).addr;
//   int ig_id = ((CcInt*)args[0].addr)->GetIntval();
//   Relation* node_rel = (Relation*)args[1].addr;
//   Relation* edge_rel = (Relation*)args[2].addr;
//   string type = ((CcString*)args[3].addr)->GetValue();
//   
//   ig->Load(ig_id, node_rel, edge_rel, GetBuildingType(type));
//   result = SetWord(ig);
//   return 0;


  IndoorGraph* ig = (IndoorGraph*)qp->ResultStorage(in_pSupplier).addr;
  int ig_id = ((CcInt*)args[0].addr)->GetIntval();

  if(CheckIndoorGraphId(ig_id)){
     Relation* node_rel = (Relation*)args[1].addr;
     Relation* edge_rel = (Relation*)args[2].addr;
     string type = ((CcString*)args[3].addr)->GetValue();

    ig->Load(ig_id, node_rel, edge_rel, GetBuildingType(type));
    result = SetWord(ig);
  }else{
    cout<<"invalid indoor graph id "<<ig_id<<endl; 
    while(CheckIndoorGraphId(ig_id) == false || ig_id <= 0){
      ig_id++;
    }
     cout<<"new indoor graph id "<<ig_id<<endl; 

     Relation* node_rel = (Relation*)args[1].addr;
     Relation* edge_rel = (Relation*)args[2].addr;
     string type = ((CcString*)args[3].addr)->GetValue();

     ig->Load(ig_id, node_rel, edge_rel, GetBuildingType(type));
     result = SetWord(ig);
  }
  return 0;

}

/*
for a given node, find all its adjacent nodes

*/

int OpTMGetAdjNodeIGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  IndoorNav* ig;
  switch(message){
      case OPEN:{

        IndoorGraph* g = (IndoorGraph*)args[0].addr;
        int oid = ((CcInt*)args[1].addr)->GetIntval();

        ig = new IndoorNav(g);
        ig->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        ig->GetAdjNodeIG(oid);
        local.setAddr(ig);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ig = (IndoorNav*)local.addr;
          if(ig->count == ig->door_tid_list1.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(ig->resulttype);
          tuple->PutAttribute(0,
                              new CcInt(true, ig->door_tid_list1[ig->count]));
          tuple->PutAttribute(1, 
                              new CcInt(true, ig->door_tid_list2[ig->count]));
          tuple->PutAttribute(2, new Line3D(ig->path_list[ig->count]));
          result.setAddr(tuple);
          ig->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ig = (IndoorNav*)local.addr;
            delete ig;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
ValueMap function for operator generateip1 

*/
int GenerateIP1ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        Relation* rel = (Relation*)args[0].addr;
        int num = ((CcInt*)args[1].addr)->GetIntval(); 
        bool type = ((CcBool*)args[2].addr)->GetBoolval();
        
        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        if(type)
          indoor_nav->GenerateIP1(num);
        else
          indoor_nav->GenerateIP2(num);

        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->genloc_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new GenLoc(indoor_nav->genloc_list[indoor_nav->count]));
          tuple->PutAttribute(1,
               new Point3D(indoor_nav->p3d_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
ValueMap function for operator indoornavigation  

*/
int IndoorNavigationValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        IndoorGraph* ig = (IndoorGraph*)args[0].addr;
        GenLoc* loc1 = (GenLoc*)args[1].addr;
        GenLoc* loc2 = (GenLoc*)args[2].addr;
        Relation* rel = (Relation*)args[3].addr;
        BTree* btree = (BTree*)args[4].addr; 
        int type = ((CcInt*)args[5].addr)->GetIntval();


        indoor_nav = new IndoorNav(ig);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        indoor_nav->type = type; 
        switch(type){
          case 0: 
                  indoor_nav->ShortestPath_Length(loc1, loc2, rel, btree);
                  break;
          case 1: 
                  indoor_nav->ShortestPath_Room(loc1, loc2, rel, btree);
                  break;
          case 2: 
                  indoor_nav->ShortestPath_Time(loc1, loc2, rel, btree);
                  break;
          default:
                  cout<<"invalid type "<<type<<endl;
                  break; 
        }

        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->type == 0){
             if(indoor_nav->count == indoor_nav->path_list.size())
                          return CANCEL;
              Tuple* tuple = new Tuple(indoor_nav->resulttype);
              tuple->PutAttribute(0,
              new Line3D(indoor_nav->path_list[indoor_nav->count]));

              result.setAddr(tuple);
              indoor_nav->count++;
              return YIELD;
          }else if(indoor_nav->type == 1){
             if(indoor_nav->count == indoor_nav->groom_oid_list.size())
                          return CANCEL;
            Tuple* tuple = new Tuple(indoor_nav->resulttype);
            tuple->PutAttribute(0,
               new CcInt(true, indoor_nav->groom_oid_list[indoor_nav->count]));
            tuple->PutAttribute(1,
               new GRoom(indoor_nav->room_list[indoor_nav->count]));

             result.setAddr(tuple);
             indoor_nav->count++;
             return YIELD;
          }else if(indoor_nav->type == 2){
             if(indoor_nav->count == indoor_nav->path_list.size())
                          return CANCEL;
              Tuple* tuple = new Tuple(indoor_nav->resulttype);
              tuple->PutAttribute(0,
                  new Line3D(indoor_nav->path_list[indoor_nav->count]));
              tuple->PutAttribute(1,
                  new CcReal(true, indoor_nav->cost_list[indoor_nav->count]));
              result.setAddr(tuple);
              indoor_nav->count++;
              return YIELD;
          }else assert(false);

      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
ValueMap function for operator generatemo1 

*/
int GenerateMO1_I_ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        IndoorGraph* ig = (IndoorGraph*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr;
        R_Tree<3,TupleId>* rtree = (R_Tree<3,TupleId>*)args[3].addr;
        int num = ((CcInt*)args[4].addr)->GetIntval();
        Periods* peri = (Periods*)args[5].addr; 

        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        unsigned int num_elev = indoor_nav->NumerOfElevators(); 
        if(num_elev <= 1){
           if(num > 0)
            indoor_nav->GenerateMO1(ig, btree, rtree, num, peri, false);
           else ////////special movement to building entrance 
//        indoor_nav->GenerateMO2_End(ig, btree, rtree, fabs(num), peri, false);
          ////////////////from building entrance//////////////////////////
        indoor_nav->GenerateMO2_Start(ig, btree, rtree, fabs(num), peri, false);

        }else{
            if(num > 0)
              indoor_nav->GenerateMO1_New(ig, btree, rtree, num, 
                                    peri, false, num_elev);

            else
              indoor_nav->GenerateMO2_New_Start(ig, btree, rtree, fabs(num),
                                    peri, false, num_elev);//from entrance
//              indoor_nav->GenerateMO2_New_End(ig, btree, rtree, fabs(num),
//                                    peri, false, num_elev);//to entrance
        }
        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->mo_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new MPoint3D(indoor_nav->mo_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
ValueMap function for operator generatemo1 
if the number is less then zero, it creates the path from a room to the 
  building entrance (a special case)

*/
int GenerateMO1_I_GenMO_ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  IndoorNav* indoor_nav;

  switch(message){
      case OPEN:{
        IndoorGraph* ig = (IndoorGraph*)args[0].addr;
        Relation* rel = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr;
        R_Tree<3,TupleId>* rtree = (R_Tree<3,TupleId>*)args[3].addr;
        int num = (int)((CcReal*)args[4].addr)->GetRealval();
        Periods* peri = (Periods*)args[5].addr; 

        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        unsigned int num_elev = indoor_nav->NumerOfElevators(); 
        if(num_elev <= 1){
          if(num > 0)
            indoor_nav->GenerateMO1(ig, btree, rtree, num, peri, true);
          else////////special movement to building entrance 
//         indoor_nav->GenerateMO2_End(ig, btree, rtree, fabs(num), peri, true);

          ///////////////from building entrance///////////////////////
         indoor_nav->GenerateMO2_Start(ig, btree, rtree, fabs(num), peri, true);
        }
        else{
          if(num > 0)
            indoor_nav->GenerateMO1_New(ig, btree, rtree, num, peri, 
                                      true, num_elev);
           else
            indoor_nav->GenerateMO2_New_Start(ig, btree, rtree, fabs(num), peri,
                                      true, num_elev);//from entrance
//            indoor_nav->GenerateMO2_New_End(ig, btree, rtree, fabs(num), peri,
//                                      true, num_elev);//to entrance 
        }

        local.setAddr(indoor_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          indoor_nav = (IndoorNav*)local.addr;
          if(indoor_nav->count == indoor_nav->mo_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(indoor_nav->resulttype);
          tuple->PutAttribute(0,
                new MPoint3D(indoor_nav->mo_list[indoor_nav->count]));
          tuple->PutAttribute(1,
                new GenMO(indoor_nav->genmo_list[indoor_nav->count]));

          result.setAddr(tuple);
          indoor_nav->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            indoor_nav = (IndoorNav*)local.addr;
            delete indoor_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
read the indoor shortest path from a file disk 

*/
int GetIndoorPathValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{

  string build_name = ((CcString*)args[0].addr)->GetValue();
  int path_oid = ((CcInt*)args[1].addr)->GetIntval();
  result = qp->ResultStorage(in_pSupplier);
  Line3D* res = (Line3D*)result.addr;
  ReadIndoorPath(build_name, path_oid, res);

  return 0;

}

Operator thefloor("thefloor",
    SpatialSpecTheFloor,
    TheFloorValueMap,
    Operator::SimpleSelect,
    TheFloorTypeMap
);

Operator getheight("getheight",
    SpatialSpecGetHeight,
    GetHeightValueMap,
    Operator::SimpleSelect,
    GetHeightTypeMap
);


ValueMapping GetRegionValueMapVM[]={
GetRegionFloor3DValueMap,
GetRegionGRoomValueMap,
};

int GetRegionSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "floor3d"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "groom"))
    return 1;

  return -1;
}

Operator getregion("getregion",
    SpatialSpecGetRegion,
    2,
    GetRegionValueMapVM,
    GetRegionSelect,
    GetRegionTypeMap
);

Operator thedoor("thedoor",
    SpatialSpecTheDoor,
    TheDoorValueMap,
    Operator::SimpleSelect,
    TheDoorTypeMap
);

Operator type_of_door("type_of_door",
    SpatialSpecTypeOfDoor,
    TypeOfDoorValueMap,
    Operator::SimpleSelect,
    TypeOfDoorTypeMap
);

Operator oid_of_door("oid_of_door",
    SpatialSpecOidOfDoor,
    OidOfDoorValueMap,
    Operator::SimpleSelect,
    OidOfDoorTypeMap
);

Operator loc_of_door("loc_of_door",
    SpatialSpecLocOfDoor,
    LocOfDoorValueMap,
    Operator::SimpleSelect,
    LocOfDoorTypeMap
);

Operator state_of_door("state_of_door",
    SpatialSpecStateOfDoor,
    StateOfDoorValueMap,
    Operator::SimpleSelect,
    StateOfDoorTypeMap
);

Operator get_floor("get_floor",
    SpatialSpecGetFloor,
    GetFloorValueMap,
    Operator::SimpleSelect,
    GetFloorTypeMap
);

Operator add_height_groom("add_height_groom",
    SpatialSpecAddHeightGRoom,
    AddHeightGroomValueMap,
    Operator::SimpleSelect,
    AddHeightGroomTypeMap
);


Operator translate_groom("translate_groom",
    SpatialSpecTranslateGRoom,
    TranslateGRoomValueMap,
    Operator::SimpleSelect,
    TranslateGroomTypeMap
);

ValueMapping TMLengthValueMap[]={
LengthLine3DValueMap,
LengthGenRangeValueMap,
LengthBusRouteValueMap
};


int LengthTMSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "line3d"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genrange"))
    return 1;
 if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busroute"))
    return 2;
  return -1;
}

Operator tm_length("size",
    SpatialSpecLengthLine3D,
    3,
    TMLengthValueMap,
    LengthTMSelect,
    LengthTMTypeMap
);

ValueMapping BBox3DValueMap[]={
BBoxLine3DValueMap,
BBoxGRoomValueMap
};

int BBox3DSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "line3d"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "groom"))
    return 1;

  return -1;
}

Operator bbox3d("bbox3d",
    SpatialSpecBBox3D,
    2,
    BBox3DValueMap,
    BBox3DSelect,
    BBox3DTypeMap
);

Operator thebuilding("thebuilding",
    SpatialSpecTheBuilding,
    TheBuildingValueMap,
    Operator::SimpleSelect,
    TheBuildingTypeMap
);


Operator theindoor("theindoor",
    SpatialSpecTheIndoor,
    TheIndoorValueMap,
    Operator::SimpleSelect,
    TheIndoorTypeMap
);


Operator createdoor3d("createdoor3d",
    SpatialSpecCreateDoor3D,
    CreateDoor3DValueMap,
    Operator::SimpleSelect,
    CreateDoor3DTypeMap
);


Operator createdoorbox("createdoorbox",
    SpatialSpecCreateDoorBox,
    CreateDoorBoxValueMap,
    Operator::SimpleSelect,
    CreateDoorBoxTypeMap
);

Operator createdoor1("createdoor1",
    SpatialSpecCreateDoor1,
    CreateDoor1ValueMap,
    Operator::SimpleSelect,
    CreateDoor1TypeMap
);

Operator createdoor2("createdoor2",
    SpatialSpecCreateDoor2,
    CreateDoor2ValueMap,
    Operator::SimpleSelect,
    CreateDoor2TypeMap
);


Operator createadjdoor1("createadjdoor1",
    SpatialSpecCreateAdjDoor1,
    CreateAdjDoor1ValueMap,
    Operator::SimpleSelect,
    CreateAdjDoor1TypeMap
);

Operator createadjdoor2("createadjdoor2",
    SpatialSpecCreateAdjDoor2,
    CreateAdjDoor2ValueMap,
    Operator::SimpleSelect,
    CreateAdjDoor2TypeMap
);

Operator path_in_region("path_in_region",
    SpatialSpecPathInRegion,
    PathInRegionValueMap,
    Operator::SimpleSelect,
    PathInRegionTypeMap
);


Operator createigraph(
    "createigraph",
    OpTMCreateIGSpec,
    OpTMCreateIGValueMap,
    Operator::SimpleSelect,
    OpTMCreateIGTypeMap
);


Operator generate_ip1("generate_ip1",
    SpatialSpecGenerateIP1,
    GenerateIP1ValueMap,
    Operator::SimpleSelect,
    GenerateIP1TypeMap
);

Operator indoornavigation("indoornavigation",
    SpatialSpecIndoorNavigation,
    IndoorNavigationValueMap,
    Operator::SimpleSelect,
    IndoorNavigationTypeMap
);


ValueMapping GenerateMO1ValueMap[]={
GenerateMO1_I_ValueMap,
GenerateMO1_I_GenMO_ValueMap
};

int GenerateMO1Select(ListExpr args)
{
  ListExpr arg = nl->Fifth(args);
  if(nl->IsAtom(arg) && nl->IsEqual(arg, "int"))
    return 0;
  if(nl->IsAtom(arg) && nl->IsEqual(arg, "real"))
    return 1;

  return -1;
}


Operator generate_mo1("generate_mo1",
    SpatialSpecGenerateMO1,
    2,
    GenerateMO1ValueMap,
    GenerateMO1Select,
    GenerateMO1TypeMap
);

Operator getindoorpath("getindoorpath",
    SpatialSpecGetIndoorPath,
    GetIndoorPathValueMap,
    Operator::SimpleSelect,
    GetIndoorPathTypeMap
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
        OpenAttribute<Space>,//        OpenSpace,  //object open 
        SaveAttribute<Space>,//        SaveSpace,   // object save
        CloseSpace, CloneSpace,//object close and clone
        Space::Cast,
        SizeOfSpace,              //sizeof function
        CheckSpace); 

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
"<text>tm_at (genmo,string) </text--->"
"<text>get the moving object with one mode</text--->"
"<text>query tm_at(genmo1, \"Indoor\")</text---> ) )";


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

const string SpatialSpecAddInfraGraph =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>busnetwork x busgraph -> (stream(((x1 t1) ... (xn tn)))) </text--->"
"<text>addinfragraph (busnetwork,busgraph) </text--->"
"<text>add navigation graph to the corresponding infrastructure</text--->"
"<text>query addinfragraph(bn1,bg1)</text---> ) )";

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


const string SpatialSpecGenerateGMO2TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x btree x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo2 (space, periods, real, int, rel, btree, rel) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo2(space_1, TwoDays, 30, 4,dg_node_rid,"
" btree_dg_rid, streets_speed) </text---> ) )";

const string SpatialSpecGenerateCarList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_car (space, periods, real, rel) </text--->"
"<text>generate moving cars in road network to get traffic </text--->"
"<text>query generate_car(rn, TwoDays, 30.0, streets_speed) </text---> ) )";


const string SpatialSpecGenerateCarExtList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x rel x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_car_ext (space, periods, real, rel, rel) </text--->"
"<text>generate moving cars in road network to get traffic </text--->"
"<text>query generate_car_ext(rn, TwoDays, 30.0, "
"streets_speed, mergepaths) </text---> ) )";


const string SpatialSpecGenerateGMO3TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x rel x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo3 (space, periods, int, int, rel,rel, rtree) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo3(space_1, TwoDays, 30.0, 4, tri_reg_new,"
"bs_pave_sort, rtree_bs_pave) </text---> ) )";

const string SpatialSpecGenerateGMO4TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel "
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo4 (space, periods, real, int, rel) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo4(space_1, TwoDays, 30.0, 4, tri_reg_new)"
" </text---> ) )";


const string SpatialSpecGenerateGMO5TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x btree x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo5 (space, periods, real, int, rel, "
"btree, rel) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo5(space_1, TwoDays, 30.0, 13,"
"dg_node_rid, btree_dg_rid, streets_speed) </text---> ) )";


const string SpatialSpecGenerateGMO6TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x  periods x real x int x rel x btree x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo6 (space, periods, real, int, rel, "
"btree, rtree) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo6(space_1, TwoDays, 30.0, 13,"
"tri_reg_new, bs_pave_sort, rtree_bs_pave) </text---> ) )";

const string SpatialSpecGenerateGMO6ExtTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x  periods x real x int x rel x btree x rtree x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo6 (space, periods, real, int, rel, "
"btree, rtree, rel) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo6(space_1, TwoDays, 30.0, 13,"
"tri_reg_new, bs_pave_sort, rtree_bs_pave, buildingrel) </text---> ) )";

const string SpatialSpecGenerateGMO7TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x rel x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo7 (space, periods, int, int, rel,rel, rtree) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo7(space_1, TwoDays, 30.0, 10, tri_reg_new,"
"msneighbor, rtree_ms_stop) </text---> ) )";

const string SpatialSpecGenerateGMO8TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x rel x rtree"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo8 (space, periods, int, int, rel,rel, rtree) </text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo8(space_1, TwoDays, 30.0, 15, tri_reg_new,"
"msneighbor, rtree_ms_stop) </text---> ) )";

const string SpatialSpecGenerateGMO8ExtTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> space x periods x real x int x rel x rel x rtree x rel"
" -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>generate_genmo8_ext (space, periods, int, int, rel,rel, rtree)</text--->"
"<text>generate generic moving objects </text--->"
"<text>query generate_genmo8(space_1, TwoDays, 30.0, 15, tri_reg_new,"
"msneighbor, rtree_ms_stop, mstops_build) </text---> ) )";

const string SpatialSpecGetRGNodesTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> network -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_rg_nodes(network) </text--->"
"<text>get road graph nodes </text--->"
"<text>query get_rg_nodes(rn) count </text---> ) )";

const string SpatialSpecGetRGEdges1TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> rel x rtree -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_rg_edges1(rel, rtree) </text--->"
"<text>get road graph edges </text--->"
"<text>query get_rg_edges1(rel, rtree) count </text---> ) )";

const string SpatialSpecGetRGEdges2TMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text> network x rel -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>get_rg_edges2(network, rel) </text--->"
"<text>get road graph edges </text--->"
"<text>query get_rg_edges2(rn, rel) count </text---> ) )";

const string OpTMCreateRoadGraphSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel1 x rel2 x rel3 -> roadgraph</text--->"
    "<text>creatergraph(int, rel, rel, rel)</text--->"
    "<text>create a road network graph by the input edges and nodes"
    "relation</text--->"
    "<text>query creatergraph(1, node-rel, edge1, edge2); </text--->"
    ") )";

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

/*
ValueMap function for operator get the reference id: genloc, ioref, busstop

*/
int RefIdGenLocValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenLoc* genl = (GenLoc*)args[0].addr;
  result = qp->ResultStorage(s);
  if(genl->IsDefined() && genl->GetOid() >= 0){
      ((CcInt*)result.addr)->Set(true, genl->GetOid());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

int RefIdIORefValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  IORef* ref = (IORef*)args[0].addr;
  result = qp->ResultStorage(s);
  if(ref->IsDefined() && ref->GetOid() >= 0){
      ((CcInt*)result.addr)->Set(true, ref->GetOid());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}


int RefIdBusStopValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Bus_Stop* bs = (Bus_Stop*)args[0].addr;
  result = qp->ResultStorage(s);
  if(bs->IsDefined() && bs->GetId() > 0){
      ((CcInt*)result.addr)->Set(true, bs->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

int RefIdBusRouteValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Bus_Route* br = (Bus_Route*)args[0].addr;
  result = qp->ResultStorage(s);
  if(br->IsDefined() && br->GetId() > 0){
      ((CcInt*)result.addr)->Set(true, br->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get bus network infrastructure id

*/
int RefIdBusNetworkValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  BusNetwork* bn = (BusNetwork*)args[0].addr;
  result = qp->ResultStorage(s);
  if(bn->IsDefined() && bn->GetId() > 0){
      ((CcInt*)result.addr)->Set(true, bn->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  
  if(bn->IsGraphInit())
      cout<<"bus graph id "<<bn->GraphId()<<endl;
  else
      cout<<"bus graph not initialized"<<endl; 

  return 0;
}

/*
get pavement infrastructure id

*/
int RefIdPavementValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Pavement* pn = (Pavement*)args[0].addr;
  result = qp->ResultStorage(s);
  if(pn->IsDefined() && pn->GetId() > 0){////0 is for free space 
      ((CcInt*)result.addr)->Set(true, pn->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);

  return 0;
}

/*
get road network infrastructure id 

*/
int RefIdRoadNetworkValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Network* rn = (Network*)args[0].addr;
  result = qp->ResultStorage(s);
  if(rn->IsDefined() && rn->GetId() > 0){////0 is for free space 
      ((CcInt*)result.addr)->Set(true, rn->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get building id 

*/
int RefIdBuildingValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Building* build = (Building*)args[0].addr;
  result = qp->ResultStorage(s);
  if(build->IsDefined() && build->GetId() > 0){////0 is for free space 
      ((CcInt*)result.addr)->Set(true, build->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get indoorinfra id 

*/
int RefIdIndoorInfraValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  IndoorInfra* indoor = (IndoorInfra*)args[0].addr;
  result = qp->ResultStorage(s);
  if(indoor->IsDefined() && indoor->GetId() > 0){////0 is for free space 
      ((CcInt*)result.addr)->Set(true, indoor->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}


/*
it returns a set of reference id for genmo and genrange 

*/
int GenMORefIdGenLocValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMObject* mo;

  switch(message){
      case OPEN:{
        GenMO* genmo = (GenMO*)args[0].addr;
        mo = new GenMObject();
        mo->GetIdList(genmo);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->id_list.size()) return CANCEL;
          CcInt* id = new CcInt(true, mo->id_list[mo->count]);
          mo->count++;
          result = SetWord(id);
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

int GenRangeRefIdGenLocValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMObject* mo;

  switch(message){
      case OPEN:{
        GenRange* genrange = (GenRange*)args[0].addr;
        mo = new GenMObject();
        mo->GetIdList(genrange);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->id_list.size()) return CANCEL;
          CcInt* id = new CcInt(true, mo->id_list[mo->count]);
          mo->count++;
          result = SetWord(id);
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  return 0;
}


/*
at: tm genloc genrange 

*/
int TMATValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMO* genmo = (GenMO*)args[0].addr;
  string type = ((CcString*)args[1].addr)->GetValue();
  result = qp->ResultStorage(s);
  GenMO* sub_genmo = (GenMO*)result.addr;
  if(genmo->IsDefined()){
      genmo->AtMode(type, sub_genmo);
  }
  return 0;
}


/*
get the deftime time periods for a generic moving object 

*/
int GenMODeftimeValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMO* mo = (GenMO*)args[0].addr;
  result = qp->ResultStorage(s);
  ((Periods*)result.addr)->Clear();
  if(mo->IsDefined() && mo->GetNoComponents() > 0){
      mo->DefTime(*(Periods*)result.addr); 
  }
  return 0;
}

/*
get the deftime time periods for indoor moving objects

*/

int MP3dDeftimeValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  MPoint3D* mo = (MPoint3D*)args[0].addr;
  result = qp->ResultStorage(s);
  ((Periods*)result.addr)->Clear();
  if(mo->IsDefined() && mo->GetNoComponents() > 0){
      mo->DefTime(*(Periods*)result.addr); 
  }
  return 0;
}

/*
get the number of units for a generic moving object 

*/
int GenMONoComponentsValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMO* mo = (GenMO*)args[0].addr;
  result = qp->ResultStorage(s);
  if(mo->IsDefined()){
      ((CcInt*)result.addr)->Set(true, mo->GetNoComponents());
  }else
      ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get the number of units for indoor moving objects

*/
int MP3dNoComponentsValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  MPoint3D* mo = (MPoint3D*)args[0].addr;
  result = qp->ResultStorage(s);
  if(mo->IsDefined()){
      ((CcInt*)result.addr)->Set(true, mo->GetNoComponents());
  }else
      ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get the number of units for genrange 

*/
int GenRangeNoComponentsValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenRange* genrange = (GenRange*)args[0].addr;
  result = qp->ResultStorage(s);
  if(genrange->IsDefined()){
      ((CcInt*)result.addr)->Set(true, genrange->ElemSize());
  }else
      ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get the number of units for groom 

*/
int GRoomNoComponentsValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GRoom* groom = (GRoom*)args[0].addr;
  result = qp->ResultStorage(s);
  if(groom->IsDefined()){
      ((CcInt*)result.addr)->Set(true, groom->Size());
  }else
      ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get the number of units for busroute 

*/
int BusRouteNoComponentsValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Bus_Route* br = (Bus_Route*)args[0].addr;
  result = qp->ResultStorage(s);
  if(br->IsDefined()){
      ((CcInt*)result.addr)->Set(true, br->Size());
  }else
      ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}


/*
get low resolution representation for a generic moving object 

*/
int LowResGenMOValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMO* mo = (GenMO*)args[0].addr;
  result = qp->ResultStorage(s);
  GenMO* presult = (GenMO*)result.addr; 
  if(mo->IsDefined()){
    mo->LowRes(*presult); 
  }
  return 0;
}


/*
get the trajectory for a generic moving object

*/
int GenMOTrajectoryValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  GenMO* mo = (GenMO*)args[0].addr;
  Space* sp = (Space*)args[1].addr;
  result = qp->ResultStorage(s);
  GenRange* presult = (GenRange*)result.addr; 
  if(mo->IsDefined()){
    mo->Trajectory(presult, sp);
  }
  return 0;
}

/*
get the trajectory of indoor moving objects 

*/
int MP3dTrajectoryValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{

  MPoint3D* mo3d = (MPoint3D*)args[0].addr;
  result = qp->ResultStorage(s);
  Line3D* presult = (Line3D*)result.addr; 
  if(mo3d->IsDefined()){
    mo3d->Trajectory(*presult); 
  }
  return 0;
}

/*
get the 2d line or 3d line of genrange 

*/
int GenRangeVisibleValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  QueryTM* mo;

  switch(message){
      case OPEN:{
        GenRange* gr = (GenRange*)args[0].addr;
        Space* sp = (Space*)args[1].addr;

        mo = new QueryTM();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GetLineOrLine3D(gr, sp);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (QueryTM*)local.addr;
          if(mo->count == mo->line_list1.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);
          tuple->PutAttribute(0, new Line(mo->line_list1[mo->count]));
//          tuple->PutAttribute(1, new Line3D(mo->line3d_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (QueryTM*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
get the transportation modes for a generic moving object 

*/
int GetModeValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  GenMObject* mo;

  switch(message){
      case OPEN:{
        GenMO* genmo = (GenMO*)args[0].addr;
        
        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GetMode(genmo);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->tm_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);
          tuple->PutAttribute(0,
                new CcString(true, GetTMStr(mo->tm_list[mo->count])));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
get the referenced objects for a generic moving object 

*/
int GetRefValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  GenMObject* mo;

  switch(message){
      case OPEN:{
        GenMO* genmo = (GenMO*)args[0].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GetRef(genmo);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->oid_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);
          tuple->PutAttribute(0,new CcInt(true, mo->oid_list[mo->count]));
          tuple->PutAttribute(1,
                new CcString(true, GetSymbolStr(mo->label_list[mo->count])));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
add bus graph to bus network infrastructure 

*/
int AddBusNetworkGraphValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            BusNetwork* bn = (BusNetwork*)args[0].addr;
            BusGraph* bg = (BusGraph*)args[1].addr; 
            bn->SetGraphId(bg->bg_id);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, bn->GetId()));
            t->PutAttribute(1, new CcInt(true, bn->GraphId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}


/*
add dual graph to pavement infrastructure 

*/
int AddPaveDualGraphValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Pavement* pn = (Pavement*)args[0].addr;
            DualGraph* dg = (DualGraph*)args[1].addr; 
            pn->SetDualGraphId(dg->g_id);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, pn->GetId()));
            t->PutAttribute(1, new CcInt(true, pn->GetDGId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
add visibility graph to pavement infrastructure 

*/
int AddPaveVisualGraphValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Pavement* pn = (Pavement*)args[0].addr;
            VisualGraph* vg = (VisualGraph*)args[1].addr; 
            pn->SetVisualGraphId(vg->g_id);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, pn->GetId()));
            t->PutAttribute(1, new CcInt(true, pn->GetVGId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
add indoor graph to a building

*/
int AddIndoorGraphValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Building* build = (Building*)args[0].addr;
            IndoorGraph* ig = (IndoorGraph*)args[1].addr; 
            if(ig->GetGraphType() != (int)build->GetType()){
              cout<<"error:building and graph do not match"<<endl;
              flag = 0;
              return CANCEL;
            }
            build->SetIndoorGraphId(ig->g_id);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, build->GetId()));
            t->PutAttribute(1, new CcInt(true, build->GetIGId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
add metro graph to metro network infrastructure 

*/
int AddMetroNetworkGraphValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            MetroNetwork* mn = (MetroNetwork*)args[0].addr;
            MetroGraph* mg = (MetroGraph*)args[1].addr; 
            mn->SetGraphId(mg->GetG_ID());
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, mn->GetId()));
            t->PutAttribute(1, new CcInt(true, mn->GraphId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
create an empty space with an identify

*/
int TheSpaceValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{

  int id = ((CcInt*)args[0].addr)->GetIntval();
  result = qp->ResultStorage(s);
  Space* sp = (Space*)result.addr; 
  sp->SetId(id);
  return 0;
}

/*
add road network infrastructure to the space 

*/
int PutInfraRNValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Space* space = (Space*)args[0].addr;
            Network* n = (Network*)args[1].addr; 
            space->AddRoadNetwork(n);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, space->GetSpaceId()));
            t->PutAttribute(1, new CcInt(true, n->GetId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  return 0;
}

/*
add pavement infrastructure to the space 

*/
int PutInfraPaveValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Space* space = (Space*)args[0].addr;
            Pavement* pn = (Pavement*)args[1].addr; 
            space->AddPavement(pn);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, space->GetSpaceId()));
            t->PutAttribute(1, new CcInt(true, pn->GetId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
add bus network infrastructure to the space 

*/
int PutInfraBNValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Space* space = (Space*)args[0].addr;
            BusNetwork* bn = (BusNetwork*)args[1].addr; 
            space->AddBusNetwork(bn);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, space->GetSpaceId()));
            t->PutAttribute(1, new CcInt(true, bn->GetId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
add metro network infrastructure to the space 

*/
int PutInfraMNValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Space* space = (Space*)args[0].addr;
            MetroNetwork* mn = (MetroNetwork*)args[1].addr; 
            space->AddMetroNetwork(mn);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, space->GetSpaceId()));
            t->PutAttribute(1, new CcInt(true, mn->GetId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
add indoor infrastructure to the space 

*/
int PutInfraIndoorValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Space* space = (Space*)args[0].addr;
            IndoorInfra* indoor_infra = (IndoorInfra*)args[1].addr; 
            space->AddIndoorInfra(indoor_infra);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, space->GetSpaceId()));
            t->PutAttribute(1, new CcInt(true, indoor_infra->GetId()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}


/*
add road graph to the space 

*/
int PutRoadGraphValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
          if(flag == 0){
            Space* space = (Space*)args[0].addr;
            RoadGraph* rg = (RoadGraph*)args[1].addr; 
            space->AddRoadGraph(rg);
            Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
            t->PutAttribute(0, new CcInt(true, space->GetSpaceId()));
            t->PutAttribute(1, new CcInt(true, rg->GetRG_ID()));
            result.setAddr(t);
            flag = 1;
            return YIELD;
          }else{
            flag = 0;
            return CANCEL;
          } 
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
        local.setAddr(Address(0));
        return 0;
  }
  
  return 0;
  
}

/*
get network infrastructure from the space 

*/
int GetInfraValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  Space* space = (Space*)args[0].addr;
  string type = ((CcString*)args[1].addr)->GetValue();
  result = SetWord(space->GetInfra(type));
  Relation* resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);
  return 0;
}

/*
output all possible transportation modes of generic moving objects 

*/
int GenMOTMListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        bool v = ((CcBool*)args[0].addr)->GetBoolval();

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GetTMStr(v);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->tm_str_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);
          tuple->PutAttribute(0, 
                              new CcInt(true, mo->count + 1));

          tuple->PutAttribute(1, 
                              new CcString(true, mo->tm_str_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}



/*
create generic moving objects for Walk Car
no indoor moving objects 

*/
int GenerateGMO2ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        BTree* btree = (BTree*)args[5].addr;
        Relation* rel2 = (Relation*)args[6].addr;
        
        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO2(sp, peri, mo_no, type, rel1, btree, rel2);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->trip1_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);
          tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
          tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));

//           tuple->PutAttribute(0,new Point(mo->loc_list1[mo->count]));
//           tuple->PutAttribute(1,new GPoint(mo->gp_list[mo->count]));
//           tuple->PutAttribute(2,new Point(mo->loc_list2[mo->count]));
//           tuple->PutAttribute(3,new Line(mo->line_list1[mo->count]));
//           tuple->PutAttribute(4,new GenMO(mo->trip1_list[mo->count]));
//           tuple->PutAttribute(5,new MPoint(mo->trip2_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}

/*
create moving cars (mpoint and mgpoint) to get traffic

*/
int GenerateCarListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        Relation* rel = (Relation*)args[3].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateCar(sp, peri, mo_no, rel);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
//          if(mo->count == mo->trip1_list.size()) return CANCEL;
          if(mo->count == mo->trip2_list.size()) return CANCEL;

          Tuple* tuple = new Tuple(mo->resulttype);
 
//           tuple->PutAttribute(0,new MPoint(mo->trip2_list[mo->count]));
//           tuple->PutAttribute(1,new MGPoint(mo->trip3_list[mo->count]));

/*          tuple->PutAttribute(0, new Point(mo->loc_list1[mo->count]));
          tuple->PutAttribute(1, new Point(mo->loc_list2[mo->count]));*/
          tuple->PutAttribute(0, new MPoint(mo->trip2_list[mo->count]));
          tuple->PutAttribute(1, new MGPoint(mo->trip3_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}


/*
create moving cars (mpoint and mgpoint) to get traffic 
use some stored paths to speed up data generation

*/
int GenerateCarExtValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Network* rn = (Network*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        Relation* rel1 = (Relation*)args[3].addr;
        Relation* rel2 = (Relation*)args[4].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateCarExt(rn, peri, mo_no, rel1, rel2);
        local.setAddr(mo);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
//          if(mo->count == mo->trip1_list.size()) return CANCEL;
//          if(mo->count == mo->trip2_list.size()) return CANCEL;
          if(mo->count == mo->loc_list1.size()) return CANCEL;

          Tuple* tuple = new Tuple(mo->resulttype);
 
//           tuple->PutAttribute(0,new MPoint(mo->trip2_list[mo->count]));
//           tuple->PutAttribute(1,new MGPoint(mo->trip3_list[mo->count]));

          tuple->PutAttribute(0, new Point(mo->loc_list1[mo->count]));
//          tuple->PutAttribute(1, new Point(mo->loc_list2[mo->count]));

//          tuple->PutAttribute(0, new MPoint(mo->trip2_list[mo->count]));
//          tuple->PutAttribute(1, new MGPoint(mo->trip3_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}


/*
create generic moving objects for Walk Bus

*/
int GenerateGMO3ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        Relation* rel2 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr; 

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO3(sp, peri, mo_no, type, rel1, rel2, rtree);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
//          if(mo->count == mo->line_list1.size()) return CANCEL;
          if(mo->count == mo->trip1_list.size()) return CANCEL;

          Tuple* tuple = new Tuple(mo->resulttype);

//        tuple->PutAttribute(0,new Point(mo->loc_list1[mo->count]));
//        tuple->PutAttribute(1,new Point(mo->loc_list2[mo->count]));
//          tuple->PutAttribute(0,new Line(mo->line_list1[mo->count]));
          tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
          tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}

/*
create generic moving objects for Walk Indoor

*/
int GenerateGMO4ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel = (Relation*)args[4].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO4(sp, peri, mo_no, type, rel);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->trip1_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);


/*          tuple->PutAttribute(0,new Line(mo->line_list1[mo->count]));
          tuple->PutAttribute(1,new Line(mo->path_list[mo->count]));
          tuple->PutAttribute(2,new Line(mo->line_list2[mo->count]));*/
          tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
          tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));

          tuple->PutAttribute(2,new MPoint3D(mo->indoor_mo_list1[mo->count]));
          tuple->PutAttribute(3,
               new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));

          tuple->PutAttribute(4,new MPoint3D(mo->indoor_mo_list2[mo->count]));
          tuple->PutAttribute(5,
              new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));


          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create generic moving objects for Walk Indoor + Car(Taxi)

*/
int GenerateGMO5ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        BTree* btree = (BTree*)args[5].addr;
        Relation* rel2 = (Relation*)args[6].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO5(sp, peri, mo_no, type, rel1, btree, rel2);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->trip1_list.size())
                          return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);


//          tuple->PutAttribute(0,new Line(mo->line_list1[mo->count]));
//          tuple->PutAttribute(1,new Line(mo->path_list[mo->count]));
//          tuple->PutAttribute(2,new Line(mo->line_list2[mo->count]));

//           tuple->PutAttribute(0,new Rectangle<2>(mo->rect_list1[mo->count]));
//            tuple->PutAttribute(1,new Point(mo->loc_list1[mo->count]));
//           tuple->PutAttribute(1,new Rectangle<2>(mo->rect_list2[mo->count]));
//            tuple->PutAttribute(3,new Point(mo->loc_list2[mo->count]));
//            tuple->PutAttribute(0, new Line(mo->line_list1[mo->count]));
//            tuple->PutAttribute(1, new Line(mo->line_list2[mo->count]));

            tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
            tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));
 
          tuple->PutAttribute(2,new MPoint3D(mo->indoor_mo_list1[mo->count]));
          tuple->PutAttribute(3,
               new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));
 
          tuple->PutAttribute(4,new MPoint3D(mo->indoor_mo_list2[mo->count]));
          tuple->PutAttribute(5,
               new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));


          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create generic moving objects for Walk Indoor + bus

*/
int GenerateGMO6ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        Relation* rel2 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO6(sp, peri, mo_no, type, rel1, rel2, rtree);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->trip1_list.size())return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);


//         tuple->PutAttribute(0, new Rectangle<2>(mo->rect_list1[mo->count]));
//         tuple->PutAttribute(1,new Point(mo->loc_list1[mo->count]));
//         tuple->PutAttribute(1, new Rectangle<2>(mo->rect_list2[mo->count]));

//            tuple->PutAttribute(0, new Line(mo->line_list1[mo->count]));
//            tuple->PutAttribute(1, new Line(mo->path_list[mo->count]));
// 
//            tuple->PutAttribute(2, new Line(mo->line_list2[mo->count]));

           tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
           tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));
           tuple->PutAttribute(2,new MPoint3D(mo->indoor_mo_list1[mo->count]));
           tuple->PutAttribute(3,
               new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));
 
           tuple->PutAttribute(4,new MPoint3D(mo->indoor_mo_list2[mo->count]));
           tuple->PutAttribute(5,
               new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));


          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
create generic moving objects for Walk Indoor + bus
with input building relations

*/
int GenerateGMO6ExtListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        Relation* rel2 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr;
        Relation* rel3 = (Relation*)args[7].addr;

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO6Ext(sp, peri, mo_no, type, rel1, rel2, rtree, rel3);
        local.setAddr(mo);

        return 0;
      }

      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->trip1_list.size())return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);


           tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
           tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));
           tuple->PutAttribute(2,new MPoint3D(mo->indoor_mo_list1[mo->count]));
           tuple->PutAttribute(3,
               new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));
 
           tuple->PutAttribute(4,new MPoint3D(mo->indoor_mo_list2[mo->count]));
           tuple->PutAttribute(5,
               new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));


          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
create generic moving objects for Walk Metro

*/
int GenerateGMO7ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        Relation* rel2 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr; 

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO7(sp, peri, mo_no, type, rel1, rel2, rtree);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
          if(mo->count == mo->trip1_list.size()) return CANCEL;
//          if(mo->count == mo->loc_list1.size()) return CANCEL;
          Tuple* tuple = new Tuple(mo->resulttype);

//          tuple->PutAttribute(0, new Point(mo->loc_list1[mo->count]));
//          tuple->PutAttribute(1, new Point(mo->loc_list2[mo->count]));
//          tuple->PutAttribute(2, new Point(mo->loc_list3[mo->count]));

//           tuple->PutAttribute(0,new Line(mo->line_list1[mo->count]));
           tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
           tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}


/*
create generic moving objects for Indoor Walk Metro

*/
int GenerateGMO8ListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        Relation* rel2 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr; 

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO8(sp, peri, mo_no, type, rel1, rel2, rtree);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
//          if(mo->count == mo->line_list1.size()) return CANCEL;
          if(mo->count == mo->trip1_list.size()) return CANCEL;

           Tuple* tuple = new Tuple(mo->resulttype);

//           tuple->PutAttribute(0,
//              new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));
//           tuple->PutAttribute(1,
//              new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));


          tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
          tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));
          tuple->PutAttribute(2,new MPoint3D(mo->indoor_mo_list1[mo->count]));
          tuple->PutAttribute(3,
              new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));

          tuple->PutAttribute(4,new MPoint3D(mo->indoor_mo_list2[mo->count]));
          tuple->PutAttribute(5,
             new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}


/*
create generic moving objects for Indoor Walk Metro + building relation 

*/
int GenerateGMO8ExtListValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 GenMObject* mo;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Periods* peri = (Periods*)args[1].addr;
        int mo_no = (int)((CcReal*)args[2].addr)->GetRealval();
        int type = ((CcInt*)args[3].addr)->GetIntval();
        Relation* rel1 = (Relation*)args[4].addr;
        Relation* rel2 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr; 
        Relation* rel3 = (Relation*)args[7].addr; 

        mo = new GenMObject();
        mo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mo->GenerateGenMO8Ext(sp, peri, mo_no, type, rel1, rel2, rtree, rel3);
        local.setAddr(mo);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mo = (GenMObject*)local.addr;
//          if(mo->count == mo->line_list1.size()) return CANCEL;
          if(mo->count == mo->trip1_list.size()) return CANCEL;

           Tuple* tuple = new Tuple(mo->resulttype);

//           tuple->PutAttribute(0,
//              new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));
//           tuple->PutAttribute(1,
//              new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));


          tuple->PutAttribute(0,new GenMO(mo->trip1_list[mo->count]));
          tuple->PutAttribute(1,new MPoint(mo->trip2_list[mo->count]));
          tuple->PutAttribute(2,new MPoint3D(mo->indoor_mo_list1[mo->count]));
          tuple->PutAttribute(3,
              new CcString(GetBuildingStr(mo->build_type_list1[mo->count])));

          tuple->PutAttribute(4,new MPoint3D(mo->indoor_mo_list2[mo->count]));
          tuple->PutAttribute(5,
             new CcString(GetBuildingStr(mo->build_type_list2[mo->count])));

          result.setAddr(tuple);
          mo->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            mo = (GenMObject*)local.addr;
            delete mo;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}

/*
get road graph nodes relation

*/
int GetRGNodesValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  RoadDenstiy* rd;

  switch(message){
      case OPEN:{
        Network* rn = (Network* ) args[0].addr;

        rd = new RoadDenstiy(rn, NULL, NULL, NULL);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        rd->GetRGNodes();
        local.setAddr(rd);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;

          if(rd->count == rd->unique_id_list.size()) return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, 
                                          rd->unique_id_list[rd->count]));
          tuple->PutAttribute(1, new GPoint(rd->gp_list[rd->count]));
          tuple->PutAttribute(2, new Point(rd->jun_loc_list[rd->count]));
          tuple->PutAttribute(3, new CcInt(true, rd->rid_list[rd->count]));

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
get road graph edge relation. two junction points at the same location

*/
int GetRGEdges1ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  RoadDenstiy* rd;

  switch(message){
      case OPEN:{
        Relation* rel = (Relation*) args[0].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[1].addr;

        rd = new RoadDenstiy(NULL, NULL, NULL, NULL);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        rd->GetRGEdges1(rel, rtree);
        local.setAddr(rd);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;

          if(rd->count == rd->jun_id_list1.size()) return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, 
                                          rd->jun_id_list1[rd->count]));
          tuple->PutAttribute(1, new CcInt(true,
                                           rd->jun_id_list2[rd->count]));


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
get road graph edge relation. two junction points are connected by glines 

*/
int GetRGEdges2ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  RoadDenstiy* rd;

  switch(message){
      case OPEN:{
        Network* rn = (Network*)args[0].addr;
        Relation* rel = (Relation*) args[1].addr;

        rd = new RoadDenstiy(rn, NULL, NULL, NULL);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        rd->GetRGEdges2(rel);
        local.setAddr(rd);

        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;

          if(rd->count == rd->jun_id_list1.size()) return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);
          tuple->PutAttribute(0, new CcInt(true, 
                                          rd->jun_id_list1[rd->count]));
          tuple->PutAttribute(1, new CcInt(true,
                                           rd->jun_id_list2[rd->count]));
          tuple->PutAttribute(2, new GLine(rd->gl_path_list[rd->count]));
         tuple->PutAttribute(3, new SimpleLine(rd->sline_path_list[rd->count]));
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
value map for operator creatergraph

*/

int OpTMCreateRoadGraphValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  RoadGraph* rg = (RoadGraph*)qp->ResultStorage(in_pSupplier).addr;
  int rg_id = ((CcInt*)args[0].addr)->GetIntval();
  
  if(CheckRoadGraphId(rg_id)){
    Relation* node_rel = (Relation*)args[1].addr;
    Relation* edge_rel1 = (Relation*)args[2].addr;
    Relation* edge_rel2 = (Relation*)args[3].addr; 

    rg->Load(rg_id, node_rel, edge_rel1, edge_rel2);
    result = SetWord(rg);
  }else{
    cout<<"invalid road graph id "<<rg_id<<endl; 
    while(CheckRoadGraphId(rg_id) == false || rg_id <= 0){
      rg_id++;
    }
    cout<<"new road graph id "<<rg_id<<endl; 
    Relation* node_rel = (Relation*)args[1].addr;
    Relation* edge_rel1 = (Relation*)args[2].addr;
    Relation* edge_rel2 = (Relation*)args[3].addr; 

    rg->Load(rg_id, node_rel, edge_rel1, edge_rel2);
    result = SetWord(rg);
  }
  return 0;
}


/*
value map for operator shortest path tm

*/

int OpTMShortestPathTMValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GPoint* gp1 = (GPoint*)args[0].addr;
  GPoint* gp2 = (GPoint*)args[1].addr; 
  RoadGraph* rg = (RoadGraph*)args[2].addr;
  Network* rn = (Network*)args[3].addr;
  
  GLine* pGLine = (GLine*)qp->ResultStorage(in_pSupplier).addr;
  result = SetWord(pGLine);

  RoadNav* nav = new RoadNav();

  nav->ShortestPath(gp1, gp2, rg, rn, pGLine);

  delete nav;

  return 0;

}


int OpTMShortestPath2TMValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  GPoint* gp1 = (GPoint*)args[0].addr;
  GPoint* gp2 = (GPoint*)args[1].addr; 
  RoadGraph* rg = (RoadGraph*)args[2].addr;
  Network* rn = (Network*)args[3].addr;
  
  GLine* pGLine = (GLine*)qp->ResultStorage(in_pSupplier).addr;
  result = SetWord(pGLine);

  RoadNav* nav = new RoadNav();

  nav->ShortestPath2(gp1, gp2, rg, rn, pGLine);

  delete nav;

  return 0;

}

/*
navigation with modes Walk and Bus

*/
int Navigation1ValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
 Navigation* nav;

  switch(message){
      case OPEN:{
        Space* sp = (Space*)args[0].addr; 
        Relation* rel1 = (Relation*)args[1].addr;
        Relation* rel2 = (Relation*)args[2].addr;
        Instant* start_time = (Instant*)args[3].addr;

        Relation* rel3 = (Relation*)args[4].addr;
        Relation* rel4 = (Relation*)args[5].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[6].addr; 

        nav = new Navigation();
        nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        nav->Navigation1(sp, rel1, rel2, start_time, rel3, rel4, rtree);
        local.setAddr(nav);

        return 0;
      }
      case REQUEST:{
        if(local.addr == NULL) return CANCEL;
        nav = (Navigation*)local.addr;
        if(nav->count == nav->loc_list1.size()) return CANCEL;
        Tuple* tuple = new Tuple(nav->resulttype);

        tuple->PutAttribute(0,new Point(nav->loc_list1[nav->count]));
//        tuple->PutAttribute(1,new Point(nav->neighbor1[nav->count]));
        tuple->PutAttribute(1,new Point(nav->loc_list2[nav->count]));
//        tuple->PutAttribute(3,new Point(nav->neighbor2[nav->count]));
        tuple->PutAttribute(2,new GenMO(nav->trip_list1[nav->count]));
        tuple->PutAttribute(3,new MPoint(nav->trip_list2[nav->count]));

        result.setAddr(tuple);
        nav->count++;
        return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            nav = (Navigation*)local.addr;
            delete nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}

ValueMapping RefIdValueMapVM[]={
  RefIdGenLocValueMap,
  RefIdIORefValueMap,
  RefIdBusStopValueMap,
  RefIdBusRouteValueMap,
  RefIdBusNetworkValueMap,
  RefIdPavementValueMap,
  RefIdRoadNetworkValueMap,
  RefIdBuildingValueMap,
  RefIdIndoorInfraValueMap
};

int RefIdOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genloc"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "ioref"))
    return 1;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busstop"))
    return 2;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busroute"))
    return 3;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busnetwork"))
    return 4;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "pavenetwork"))
    return 5;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "network"))
    return 6;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "building"))
    return 7;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "indoorinfra"))
    return 8;
  return -1;
}

ValueMapping TMATValueMapVM[]={
  TMATValueMap
//  GenLocATValueMap,
};

int TMATOpSelect(ListExpr args)
{
  ListExpr arg2 = nl->Second(args);
  if(nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "string")
    return 0;
  if(nl->IsAtom(arg2) && nl->IsEqual(arg2, "genloc"))
    return 1;
  if(nl->IsAtom(arg2) && nl->IsEqual(arg2, "genrange"))
    return 2;
  return -1;
}

ValueMapping SetRefIdValueMapVM[]={
  GenMORefIdGenLocValueMap,
  GenRangeRefIdGenLocValueMap
};


/*
for getting the reference id 

*/
int GenMORefIdOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genmo"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genrange"))
    return 1;
  return -1;
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
  if(nl->IsEqual(arg1, "genloc") || 
     nl->IsEqual(arg1, "ioref") || 
     nl->IsEqual(arg1, "busstop") || 
     nl->IsEqual(arg1, "busroute") || 
     nl->IsEqual(arg1, "busnetwork") || 
     nl->IsEqual(arg1, "pavenetwork") || 
     nl->IsEqual(arg1, "network") || 
     nl->IsEqual(arg1, "building") || nl->IsEqual(arg1, "indoorinfra"))
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
  if(nl->IsEqual(arg1, "genmo") ||nl->IsEqual(arg1, "genrange"))
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
    (nl->SymbolValue(arg2) == "string" || nl->IsEqual(arg2, "genloc") ||
     nl->IsEqual(arg2, "genrange")))
    return nl->SymbolAtom("genmo");

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
TypeMap function for operator generate genmo2

*/
ListExpr GenerateGMO2ListTypeMap(ListExpr args)
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

  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType);

  if(!(CompareSchemas(arg5, xType)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg6 = nl->Sixth(args);
  if(!listutils::isBTreeDescription(arg6))
    return listutils::typeError("para6 should be a btree");

  ListExpr arg7 = nl->Nth(7, args);
  if (!(listutils::isRelDescription(arg7)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(GenMObject::StreetSpeedInfo, xType1);

  if(!(CompareSchemas(arg7, xType1)))
    return nl->SymbolAtom ( "typeerror" );

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
TypeMap function for operator generate car ext

*/
ListExpr GenerateCarExtListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 5){
      string err = "five input parameter expected";
      return listutils::typeError(err);
  }
  
  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "network")){
      string err = "the first parameter should be network";
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

  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(GenMObject::CommPathInfo, xType2);

  if(!(CompareSchemas(arg5, xType2)))
    return nl->SymbolAtom ( "typeerror" );

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                    nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("loc"),
                                     nl->SymbolAtom("point"))
                  )
                )
          );
          
    return result;
}

/*
TypeMap function for operator generate genmo3

*/
ListExpr GenerateGMO3ListTypeMap(ListExpr args)
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
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!(nl->IsEqual(arg3, "real") && nl->IsEqual(arg4, "int"))){
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }

  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);

  if(!(CompareSchemas(arg5, xType)))
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
TypeMap function for operator generate genmo4

*/
ListExpr GenerateGMO4ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 5){
      string err = "five input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

//   ListExpr arg2 = nl->Second(args);
//   if(!nl->IsEqual(arg2, "indoorinfra")){
//       string err = "the second parameter should be indoor infra";
//       return listutils::typeError(err);
//   }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!(nl->IsEqual(arg3, "real") && nl->IsEqual(arg4, "int"))){
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }
 
  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);

  if(!(CompareSchemas(arg5, xType)))
    return nl->SymbolAtom ( "typeerror" );

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip1"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building1"),
                                     nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip2"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building2"),
                                     nl->SymbolAtom("string"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate genmo5

*/
ListExpr GenerateGMO5ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 7){
      string err = "seven input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

//   ListExpr arg2 = nl->Second(args);
//   if(!nl->IsEqual(arg2, "indoorinfra")){
//       string err = "the second parameter should be indoor infra";
//       return listutils::typeError(err);
//   }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!(nl->IsEqual(arg3, "real") && nl->IsEqual(arg4, "int"))){
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }
 
  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType1);

  if(!(CompareSchemas(arg5, xType1)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg6 = nl->Sixth(args);
  if(!listutils::isBTreeDescription(arg6))
    return listutils::typeError("para7 should be a btree");

  ListExpr arg7 = nl->Nth(7, args);
  if (!(listutils::isRelDescription(arg7)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(GenMObject::StreetSpeedInfo, xType2);

  if(!(CompareSchemas(arg7, xType2)))
    return nl->SymbolAtom ( "typeerror" );


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip1"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building1"),
                                     nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip2"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building2"),
                                     nl->SymbolAtom("string"))
                  )
                )
          );

    return result;
}

/*
TypeMap function for operator generate genmo6

*/
ListExpr GenerateGMO6ListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 7){
      string err = "seven input parameters expected";
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
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }
 
  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType1);

  if(!(CompareSchemas(arg5, xType1)))
    return nl->SymbolAtom ( "typeerror" );


  ListExpr arg6 = nl->Sixth(args);
  if (!(listutils::isRelDescription(arg6)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType2);

  if(!(CompareSchemas(arg6, xType2)))
    return nl->SymbolAtom ( "typeerror" );


  ListExpr arg7 = nl->Nth(7, args);
  if(!listutils::isRTreeDescription(arg7))
      return listutils::typeError("para7 should be a rtree");


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip1"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building1"),
                                     nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip2"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building2"),
                                     nl->SymbolAtom("string"))
                  )
                )
          );

    return result;
}

/*
TypeMap function for operator generate genmo6ext

*/
ListExpr GenerateGMO6ExtListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 8){
      string err = "eight input parameters expected";
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
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }
 
  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType1);

  if(!(CompareSchemas(arg5, xType1)))
    return nl->SymbolAtom ( "typeerror" );


  ListExpr arg6 = nl->Sixth(args);
  if (!(listutils::isRelDescription(arg6)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(BN::BusStopsPaveTypeInfo, xType2);

  if(!(CompareSchemas(arg6, xType2)))
    return nl->SymbolAtom ( "typeerror" );


  ListExpr arg7 = nl->Nth(7, args);
  if(!listutils::isRTreeDescription(arg7))
      return listutils::typeError("para7 should be a rtree");

  ListExpr arg8 = nl->Nth(8, args);
  if (!(listutils::isRelDescription(arg8)))
    return nl->SymbolAtom ( "typeerror" );
  
  ListExpr xType3;
  nl->ReadFromString(GenMObject::BuildingInfo, xType3);

  if(!(CompareSchemas(arg8, xType3)))
    return nl->SymbolAtom ( "typeerror" );


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip1"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building1"),
                                     nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip2"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building2"),
                                     nl->SymbolAtom("string"))
                  )
                )
          );

    return result;
}

/*
TypeMap function for operator generate genmo7

*/
ListExpr GenerateGMO7ListTypeMap(ListExpr args)
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
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!(nl->IsEqual(arg3, "real") && nl->IsEqual(arg4, "int"))){
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }

  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);

  if(!(CompareSchemas(arg5, xType)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg6 = nl->Sixth(args);
  if (!(listutils::isRelDescription(arg6)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(MetroNetwork::MetroPaveTypeInfo, xType1);

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
TypeMap function for operator generate genmo8

*/
ListExpr GenerateGMO8ListTypeMap(ListExpr args)
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
  if(!nl->IsEqual(arg2, "periods")){
      string err = "the second parameter should be periods";
      return listutils::typeError(err);
  }

  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);

  if(!(nl->IsEqual(arg3, "real") && nl->IsEqual(arg4, "int"))){
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }

  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);

  if(!(CompareSchemas(arg5, xType)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg6 = nl->Sixth(args);
  if (!(listutils::isRelDescription(arg6)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(MetroNetwork::MetroPaveTypeInfo, xType1);

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
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip1"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building1"),
                                     nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip2"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building2"),
                                     nl->SymbolAtom("string"))
                  )
                )
          );

    return result;
}


/*
TypeMap function for operator generate genmo8 ext

*/
ListExpr GenerateGMO8ExtListTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 8){
      string err = "eight input parameter expected";
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
      string err = "the 3 parameter should be real and 4 parameter be int";
      return listutils::typeError(err);
  }

  ListExpr arg5 = nl->Fifth(args);
  if (!(listutils::isRelDescription(arg5)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(DualGraph::TriangleTypeInfo3, xType);

  if(!(CompareSchemas(arg5, xType)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg6 = nl->Sixth(args);
  if (!(listutils::isRelDescription(arg6)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(MetroNetwork::MetroPaveTypeInfo, xType1);

  if(!(CompareSchemas(arg6, xType1)))
    return nl->SymbolAtom ( "typeerror" );

  ListExpr arg7 = nl->Nth(7, args);
  if(!listutils::isRTreeDescription(arg7))
    return listutils::typeError("para7 should be a rtree");

  ListExpr arg8 = nl->Nth(8, args);
  if (!(listutils::isRelDescription(arg8)))
    return nl->SymbolAtom ( "typeerror" );
  
  ListExpr xType3;
  nl->ReadFromString(GenMObject::BuildingInfo, xType3);

  if(!(CompareSchemas(arg8, xType3)))
    return nl->SymbolAtom ( "typeerror" );

    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(

                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Trip1"),
                                     nl->SymbolAtom("genmo")),
                        nl->TwoElemList(nl->SymbolAtom("Trip2"),
                                     nl->SymbolAtom("mpoint")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip1"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building1"),
                                     nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("IndoorTrip2"),
                                     nl->SymbolAtom("mpoint3d")),
                        nl->TwoElemList(nl->SymbolAtom("Building2"),
                                     nl->SymbolAtom("string"))
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
                        nl->TwoElemList(nl->SymbolAtom("jun_id"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("jun_gp"),
                                      nl->SymbolAtom("gpoint")),
                        nl->TwoElemList(nl->SymbolAtom("jun_p"),
                                      nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("rid"),
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
  if(nl->ListLength(args) != 2){
      string err = "two input parameters expected";
      return listutils::typeError(err);
  }

  ListExpr arg1 = nl->First(args);
  if(!IsRelDescription(arg1))
    return listutils::typeError("para1 should be a relation");

  ListExpr xType;
  nl->ReadFromString(RoadDenstiy::rg_nodes_typeinfo, xType);
  if(!CompareSchemas(arg1, xType))return nl->SymbolAtom ( "typeerror" );


  ListExpr arg2 = nl->Second(args);
   if(!listutils::isRTreeDescription(arg2))
    return listutils::typeError("para2 should be a rtree");


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->TwoElemList(
                        nl->TwoElemList(nl->SymbolAtom("jun_id1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("jun_id2"),
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
  nl->ReadFromString(RoadDenstiy::rg_nodes_typeinfo, xType);
  if(!CompareSchemas(arg2, xType))return nl->SymbolAtom ( "typeerror" );


    ListExpr result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("jun_id1"),
                                      nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("jun_id2"),
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
  

//     ListExpr result =
//           nl->TwoElemList(
//               nl->SymbolAtom("stream"),
//                 nl->TwoElemList(
// 
//                   nl->SymbolAtom("tuple"),
//                       nl->TwoElemList(
//                         nl->TwoElemList(nl->SymbolAtom("Path1"),
//                                     nl->SymbolAtom("gline")),
//                         nl->TwoElemList(nl->SymbolAtom("Path2"),
//                                     nl->SymbolAtom("sline"))
//                   )
//                 )
//           );


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
                        nl->TwoElemList(nl->SymbolAtom("loc1"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("loc2"),
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

Operator ref_id("ref_id",
    SpatialSpecRefId,
    9,
    RefIdValueMapVM,
    RefIdOpSelect,
    RefIdTypeMap
);


Operator setref_id("ref_id",
    SpatialSpecSetMORefId,
    2,
    SetRefIdValueMapVM,
    GenMORefIdOpSelect,
    SetRefIdTypeMap
);


Operator tm_at("tm_at",
    SpatialSpecTMAT,
    1,
    TMATValueMapVM,
    TMATOpSelect,
    TMATTypeMap
);


int TMDeftimeOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genmo"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "mpoint3d"))
    return 1;

  return -1;
}


ValueMapping GenMODeftimeValueMapVM[]={
  GenMODeftimeValueMap,
  MP3dDeftimeValueMap
};

Operator genmodeftime("deftime", //name 
    SpatialSpecGenMODeftime,  //specification
    2,
    GenMODeftimeValueMapVM,
    TMDeftimeOpSelect,
    GenMODeftimeTypeMap //type mapping 
);


ValueMapping GenMONoComponentsValueMapVM[]={
  GenMONoComponentsValueMap, 
  MP3dNoComponentsValueMap, 
  GenRangeNoComponentsValueMap,
  GRoomNoComponentsValueMap,
  BusRouteNoComponentsValueMap
};

int TMNoComponentsOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genmo"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "mpoint3d"))
    return 1;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genrange"))
    return 2;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "groom"))
    return 3;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busroute"))
    return 4;
  
  return -1;
}

Operator genmonocomponents("no_components", //name 
    SpatialSpecGenMONoComponents, //specification
    5,
    GenMONoComponentsValueMapVM,//value mapping 
    TMNoComponentsOpSelect,
    GenMONoComponentsTypeMap //type mapping 
);


Operator lowres("lowres",
    SpatialSpecLowRes, //specification
    LowResGenMOValueMap,  //value mapping 
    Operator::SimpleSelect,
    LowResTypeMap //type mapping 
);


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

Operator tmtrajectory("trajectory",
    SpatialSpecTMTrajectory,
    MP3dTrajectoryValueMap,
    Operator::SimpleSelect,
    TMTrajectoryTypeMap
);

Operator gentrajectory("trajectory",
    SpatialSpecGenTrajectory,
    GenMOTrajectoryValueMap,
    Operator::SimpleSelect,
    GenTrajectoryTypeMap
);

Operator genrangevisible("genrangevisible",
    SpatialSpecGenrangeVisible,
    GenRangeVisibleValueMap,
    Operator::SimpleSelect,
    GenRangeVisibleTypeMap
);

Operator getmode("getmode",
    SpatialSpecGetMode,
    GetModeValueMap,
    Operator::SimpleSelect,
    GetModeTypeMap
);

Operator getref("getref",
    SpatialSpecGetRef,
    GetRefValueMap,
    Operator::SimpleSelect,
    GetRefTypeMap
);

/*
add navigation graph to the infrastructure
bus network, pavement, indoor, train network

*/
ValueMapping TMAddInfraGraphValueMapVM[]={
  AddBusNetworkGraphValueMap,
  AddPaveDualGraphValueMap,
  AddPaveVisualGraphValueMap,
  AddIndoorGraphValueMap,
  AddMetroNetworkGraphValueMap
};

int TMAddInfraGraphOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busnetwork") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "busgraph"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "pavenetwork") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "dualgraph"))
    return 1;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "pavenetwork") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "visualgraph"))
    return 2;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "building") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "indoorgraph"))
    return 3;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "metronetwork") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "metrograph"))
    return 4;
  return -1;
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
            nl->TwoElemList(nl->SymbolAtom("bus network id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("bus graph id"),
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
            nl->TwoElemList(nl->SymbolAtom("pavement id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("dual graph id"),
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
            nl->TwoElemList(nl->SymbolAtom("pavement id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("visibility graph id"),
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
            nl->TwoElemList(nl->SymbolAtom("building id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("indoor graph id"),
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
            nl->TwoElemList(nl->SymbolAtom("MetroNetwork id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("Metro graph id"),
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
            nl->TwoElemList(nl->SymbolAtom("space id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("road network id"),
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
            nl->TwoElemList(nl->SymbolAtom("space id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("pavement id"),
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
            nl->TwoElemList(nl->SymbolAtom("space id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("busnetwork id"),
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
            nl->TwoElemList(nl->SymbolAtom("space id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("metronetwork id"),
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
            nl->TwoElemList(nl->SymbolAtom("space id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("indoorinfra id"),
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
            nl->TwoElemList(nl->SymbolAtom("space id"),
                            nl->SymbolAtom("int")),
            nl->TwoElemList(nl->SymbolAtom("road graph id"),
                            nl->SymbolAtom("int"))
          )
        )
      );
    return reslist;
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
      nl->ReadFromString(Pavement::PaveTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_BUSSTOP){//////////bus stops 
      ListExpr xType;
      nl->ReadFromString(BusNetwork::BusStopsInternalTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_BUSROUTE){ ////bus routes 
      ListExpr xType;
      nl->ReadFromString(BusNetwork::BusRoutesTypeInfo, xType);
      return xType; 
    }else if(GetSymbol(type) == IF_MPPTN){ ///////////////bus trips 
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
    }else{
      string err = "infrastructure type error";
      return listutils::typeError(err);
    }
  }

  return nl->SymbolAtom("typeerror");
}

Operator addinfragraph("addinfragraph",
    SpatialSpecAddInfraGraph,
    5,
    TMAddInfraGraphValueMapVM,
    TMAddInfraGraphOpSelect,
    AddInfraGraphTypeMap
);


/*
create an empty space 

*/

Operator thespace("thespace",
    SpatialSpecTheSpace,
    TheSpaceValueMap,
    Operator::SimpleSelect,
    TheSpaceTypeMap
);

/*
put infrastructures to the space 

*/

ValueMapping PutInfraValueMapVM[]={
  PutInfraRNValueMap,
  PutInfraPaveValueMap,
  PutInfraBNValueMap,
  PutInfraMNValueMap,
  PutInfraIndoorValueMap,
  PutRoadGraphValueMap
};

int PutInfraOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "space") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "network"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "space") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "pavenetwork"))
    return 1;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "space") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "busnetwork"))
    return 2;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "space") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "metronetwork"))
    return 3;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "space") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "indoorinfra"))
    return 4;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "space") &&
    nl->IsAtom(arg2) && nl->IsEqual(arg2, "roadgraph"))
    return 5;
  
  return -1;
}

Operator putinfra("putinfra",
    SpatialSpecPutInfra,
    6,
    PutInfraValueMapVM,
    PutInfraOpSelect,
    PutInfraTypeMap
);

Operator getinfra("getinfra",
    SpatialSpecGetInfra,
    GetInfraValueMap,
    Operator::SimpleSelect,
    GetInfraTypeMap
);


Operator genmo_tm_list("genmo_tm_list",
    SpatialSpecGenMOTMList,
    GenMOTMListValueMap,
    Operator::SimpleSelect,
    GenMOTMListTypeMap
);

/*
create generic moving objects 

*/

Operator generate_genmo2("generate_genmo2",
    SpatialSpecGenerateGMO2TMList,
    GenerateGMO2ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO2ListTypeMap
);

Operator generate_car("generate_car",
    SpatialSpecGenerateCarList,
    GenerateCarListValueMap,
    Operator::SimpleSelect,
    GenerateCarListTypeMap
);

/*
store some paths in road network where two locations from two areas should pass
those roads

*/
Operator generate_car_ext("generate_car_ext",
    SpatialSpecGenerateCarExtList,
    GenerateCarExtValueMap,
    Operator::SimpleSelect,
    GenerateCarExtListTypeMap
);


/*
bus + walk

*/
Operator generate_genmo3("generate_genmo3",
    SpatialSpecGenerateGMO3TMList,
    GenerateGMO3ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO3ListTypeMap
);

/*
indoor + walk

*/
Operator generate_genmo4("generate_genmo4",
    SpatialSpecGenerateGMO4TMList,
    GenerateGMO4ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO4ListTypeMap
);

/*
indoor + walk + car(taxi)

*/
Operator generate_genmo5("generate_genmo5",
    SpatialSpecGenerateGMO5TMList,
    GenerateGMO5ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO5ListTypeMap
);

/*
indoor + walk + bus

*/

Operator generate_genmo6("generate_genmo6",
    SpatialSpecGenerateGMO6TMList,
    GenerateGMO6ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO6ListTypeMap
);

/*
indoor + walk + bus with a relation specifying building relations

*/

Operator generate_genmo6_ext("generate_genmo6_ext",
    SpatialSpecGenerateGMO6ExtTMList,
    GenerateGMO6ExtListValueMap,
    Operator::SimpleSelect,
    GenerateGMO6ExtListTypeMap
);

/*
metro + walk

*/
Operator generate_genmo7("generate_genmo7",
    SpatialSpecGenerateGMO7TMList,
    GenerateGMO7ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO7ListTypeMap
);

/*
indoor + metro + walk

*/

Operator generate_genmo8("generate_genmo8",
    SpatialSpecGenerateGMO8TMList,
    GenerateGMO8ListValueMap,
    Operator::SimpleSelect,
    GenerateGMO8ListTypeMap
);


/*
indoor + metro + walk + building relation 

*/

Operator generate_genmo8_ext("generate_genmo8_ext",
    SpatialSpecGenerateGMO8ExtTMList,
    GenerateGMO8ExtListValueMap,
    Operator::SimpleSelect,
    GenerateGMO8ExtListTypeMap
);

/*
create road graph nodes and edges 

*/
Operator get_rg_nodes("get_rg_nodes",
    SpatialSpecGetRGNodesTMList,
    GetRGNodesValueMap,
    Operator::SimpleSelect,
    GetRGNodesTypeMap
);

Operator get_rg_edges1("get_rg_edges1",
    SpatialSpecGetRGEdges1TMList,
    GetRGEdges1ValueMap,
    Operator::SimpleSelect,
    GetRGEdges1TypeMap
);

Operator get_rg_edges2("get_rg_edges2",
    SpatialSpecGetRGEdges2TMList,
    GetRGEdges2ValueMap,
    Operator::SimpleSelect,
    GetRGEdges2TypeMap
);


Operator creatergraph(
  "creatergraph", 
  OpTMCreateRoadGraphSpec,
  OpTMCreateRoadGraphValueMap,
  Operator::SimpleSelect,
  OpTMCreateRoadGraphTypeMap
);

Operator shortestpath_tm(
  "shortestpath_tm", 
  OpTMShortestPathTMSpec,
  OpTMShortestPathTMValueMap,
  Operator::SimpleSelect,
  OpTMShortestPathTMTypeMap
);

Operator shortestpath_tm2(
  "shortestpath_tm2", 
  OpTMShortestPathTMSpec,
  OpTMShortestPath2TMValueMap,
  Operator::SimpleSelect,
  OpTMShortestPathTMTypeMap
);

Operator navigation1("navigation1",
    SpatialSpecNavigation1List,
    Navigation1ValueMap,
    Operator::SimpleSelect,
    Navigation1ListTypeMap
);

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

const string OpTMNodeDGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph -> rel</text--->"
    "<text>nodedualgraph(dualgraph)</text--->"
    "<text>get the node relation of the graph</text--->"
    "<text>query nodedualgraph(dg1) count; </text--->"
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

const string OpTMBusRouteRoadSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 "
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>bus_route_road(n,rel,attr1);</text--->"
    "<text>calculate the total length of bus routes in road network</text--->"
    "<text>query bus_route_road(n,busroutes,bus_route1) count;</text--->"
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
    "( <text>network x rel1 x attr1 x attr2 x attr3 x attr4 x rel2 x btree"
    "x string -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops1(n,rel1,attr1,attr2, attr3, attr4, rel2,"
    " btree, string);</text--->"
    "<text>create bus stops</text--->"
    "<text>query create_bus_stop1(n,busroutes,br_id,bus_route1,"
    "bus_route2,route_type,subpaves2, btree_pave2,Berlin) count;</text--->"
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


const string OpTMMapBsToPaveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busnetwork x rtree x rel x int x string"
     " -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>mapbstopave(bn1, rtree_dg, dg_node, roadwidth, Berlin);"
    " </text--->"
    "<text>map bus stops to pavement areas</text--->"
    "<text>query mapbstopave(bn1, rtree_dg, dg_node, roadwidth, Berlin)"
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
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table1(rel,rel,btree);</text--->"
    "<text>create time table at each spatial location </text--->"
    "<text>query create_time_table1(final_busstops,all_bus_rel,btree_mo)"
    "count;</text--->"
    ") )";

const string OpTMCreateTimeTable1NewSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree x periods x periods"
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table1_new(rel,rel,btree,periods,periods);</text--->"
    "<text>create time table at each spatial location </text--->"
    "<text>query create_time_table1_new(final_busstops,all_bus_rel,btree_mo,"
    "night1, night2) count;</text--->"
    ") )";
    
const string OpTMCreateTimeTable2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table2(rel,rel,btree);</text--->"
    "<text>create time table at each spatial location </text--->"
    "<text>query create_time_table2(train_stops,ubtrains,btree_train)"
    "count;</text--->"
    ") )";

const string OpTMCreateTimeTable2NewSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_time_table2_new(rel,rel,btree);</text--->"
    "<text>compact storage of time tables </text--->"
    "<text>query create_time_table2_new(train_stops,ubtrains,btree_train)"
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
    "( <text>dualgraph x string "
     "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>createmetroroute(dualgraph, string)</text--->"
    "<text>create metro routes</text--->"
    "<text>query createmetroroute(dualgraph, Berlin); </text--->"
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

const string OpTMInstant2DaySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>instant -> int </text--->"
    "<text>instant2day(instant);</text--->"
    "<text>get the day (int value) of time</text--->"
    "<text>query instant2day(theInstant(2007,6,3,9,0,0,0));</text--->"
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
    "( <text>rel x attr1 x attr2 x string->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getrect1(rel, attr, attr, string);</text--->"
    "<text>get the maximum rectangle area for a region</text--->"
    "<text>query getrect1(new_region_elems2, id, covarea, Berlin);</text--->"
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
    "( <text>line -> line</text--->"
    "<text>modifyline(line)</text--->"
    "<text>modify the coordinates of a line, for numeric problem</text--->"
    "<text>query modifyline([const line value ((2.33 3.33 4.444 5.555))])"
    "</text--->"
    ") )";

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

const string OpTMNearStopPaveSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>space x string "
    " ->(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>nearstops_pave(space, string)</text--->"
    "<text>find pavement areas near to bus stops</text--->"
    "<text>query nearstops_pave(space, Bus)</text--->"
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
                        nl->TwoElemList(nl->SymbolAtom("fs_loc"),
                                    nl->SymbolAtom("point")),
                        nl->TwoElemList(nl->SymbolAtom("rn_loc"),
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
                        nl->TwoElemList(nl->SymbolAtom("oid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("rid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("pavement"),
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
TypeMap fun for operator bus route road.
calculate the total length of bus routes in road network 

*/

ListExpr OpTMBusRouteRoadTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "network")){
      return nl->SymbolAtom ( "typeerror: param1 should be network" );
  }
  
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be relation" );
  

  ListExpr attrName2 = nl->Third ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param2)),
                      aname2,attrType2);
  if(j2 == 0 || !listutils::isSymbol(attrType2,"gline")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type gline");
  }

  ListExpr res = nl->SymbolAtom("real");
  
  return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->OneElemList(nl->IntAtom(j2)),res);
                        
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
  if(!(nl->IsAtom(param9) && nl->AtomType(param9) == SymbolType &&  
     nl->SymbolValue(param9) == "string")){
      return nl->SymbolAtom ( "typeerror: param9 should be string" );
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
                      
/*     ListExpr res = nl->TwoElemList(
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
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_loc_id"),
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
                        nl->SymbolAtom("bus_stop"),
                        nl->SymbolAtom("busstop")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_geodata"),
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
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_route"),
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
  nl->ReadFromString(Pavement::PaveTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                Pavement::PaveTypeInfo);
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
                        nl->SymbolAtom("bus_stop"),
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
                        nl->SymbolAtom("bus_route"),
                        nl->SymbolAtom("busroute"))
                    )));
      return res;
  }
  return nl->SymbolAtom("typeerror"); 

}


/*
TypeMap fun for operator mapbstopave

*/
ListExpr OpTMMapBsToPaveTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 4" );
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
      nl->SymbolValue(param5) == "string"))
      return nl->SymbolAtom("typeerror");
  
  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("pave_loc1"),
                        nl->SymbolAtom("genloc")),
                    nl->TwoElemList(
                        nl->SymbolAtom("pave_loc2"),
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
                        nl->SymbolAtom("bus_uoid"),
                        nl->SymbolAtom("int")), 
                  nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
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
                        nl->SymbolAtom("bus_uoid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("busstop"))
                    )));
  return res;

}

/*
TypeMap fun for operator bsneighbors3 

*/
ListExpr OpTMBsNeighbors3TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }

  ListExpr arg1 = nl->First ( args );
  if(!IsRelDescription(arg1))
  return listutils::typeError("param1 should be a relation");
  
  ListExpr xType1;
  nl->ReadFromString(BN::BusTimeTableTypeInfo, xType1);
  if(!CompareSchemas(arg1, xType1))
    return nl->SymbolAtom ( "rel1 scheam should be" + BN::BusTimeTableTypeInfo);
  
  ListExpr arg2 = nl->Second( args );
  if(!IsRelDescription(arg2))
  return listutils::typeError("param2 should be a relation");
  
  ListExpr xType2;
  nl->ReadFromString(RoadDenstiy::mo_bus_typeinfo, xType2);
  if(!CompareSchemas(arg2, xType2))
    return nl->SymbolAtom ( "rel2 scheam should be" + 
                          RoadDenstiy::mo_bus_typeinfo);
  
  ListExpr arg3 = nl->Third( args );
  if(!listutils::isBTreeDescription(arg3))
      return listutils::typeError("param3 should be a btree");

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->Cons(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_uoid"),
                        nl->SymbolAtom("int")),
                    nl->SixElemList(
                      nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                      nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("busstop")),
                      nl->TwoElemList(
                        nl->SymbolAtom("whole_time"),
                        nl->SymbolAtom("periods")),
                      nl->TwoElemList(
                        nl->SymbolAtom("schedule_interval"),
                        nl->SymbolAtom("real")),
                      nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline")),
                      nl->TwoElemList(
                        nl->SymbolAtom("TimeCost"),
                        nl->SymbolAtom("real")))
                    )));
  return res;

}

/*
TypeMap fun for operator createbgraph

*/

ListExpr OpTMCreateBusGraphTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);
  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc1 = nl->Third(args);
  ListExpr xEdgeDesc2 = nl->Fourth(args); 
  ListExpr xEdgeDesc3 = nl->Fifth(args); 
  
  if(!nl->IsEqual(xIdDesc, "int")) return nl->SymbolAtom ( "typeerror" );
  if(!IsRelDescription(xNodeDesc))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(BusGraph::NodeTypeInfo, xType1);
  if(!CompareSchemas(xNodeDesc, xType1))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(xEdgeDesc1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(BusGraph::EdgeTypeInfo1, xType2);
  if(!CompareSchemas(xEdgeDesc1, xType2))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(xEdgeDesc2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType3;
  nl->ReadFromString(BusGraph::EdgeTypeInfo2, xType3);
  if(!CompareSchemas(xEdgeDesc2, xType3))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(xEdgeDesc3))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType4;
  nl->ReadFromString(BusGraph::EdgeTypeInfo3, xType4);
  if(!CompareSchemas(xEdgeDesc3, xType4))return nl->SymbolAtom ( "typeerror" );

  return nl->SymbolAtom ( "busgraph" );
}


/*
TypeMap fun for operator getadjnode vg, dg, bg, ig

*/

ListExpr OpTMGetAdjNodeTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);



  if(nl->SymbolValue(param1) == "busgraph"){

    if(!(nl->IsEqual(param2, "int") || nl->IsEqual(param2, "busstop")))
        return nl->SymbolAtom ( "typeerror" );

      ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Type"),
                        nl->SymbolAtom("int"))
                    )));
    return res; 
  }

  if(nl->SymbolValue(param1) == "visualgraph"){
    
    if(!(nl->IsEqual(param2, "int"))) return nl->SymbolAtom ( "typeerror" );

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

  if(nl->SymbolValue(param1) == "dualgraph"){

    if(!(nl->IsEqual(param2, "int"))) return nl->SymbolAtom ( "typeerror" );

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

  if(nl->SymbolValue(param1) == "indoorgraph"){

    if(!(nl->IsEqual(param2, "int"))) return nl->SymbolAtom ( "typeerror" );

      ListExpr result = nl->TwoElemList(
             nl->SymbolAtom("stream"),
               nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                     nl->ThreeElemList(
                       nl->TwoElemList(nl->SymbolAtom("tid1"),
                                   nl->SymbolAtom("int")),
                       nl->TwoElemList(nl->SymbolAtom("tid2"),
                                    nl->SymbolAtom("int")),
                      nl->TwoElemList(nl->SymbolAtom("connection"),
                                    nl->SymbolAtom("line3d"))
                  )
                )
          );
      return result;

  }

  if(nl->SymbolValue(param1) == "metrograph"){

    if(!(nl->IsEqual(param2, "int"))) return nl->SymbolAtom ( "typeerror" );

      ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("metro_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("metro_stop2"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Type"),
                        nl->SymbolAtom("int"))
                    )));
    return res; 
  }
  
  
    if(nl->SymbolValue(param1) == "roadgraph"){

    if(!(nl->IsEqual(param2, "int"))) return nl->SymbolAtom ( "typeerror" );

      ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("jun1"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("jun2"),
                        nl->SymbolAtom("gpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("gline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Type"),
                        nl->SymbolAtom("int"))
                    )));
    return res; 
  }
  
  return nl->SymbolAtom ( "typeerror" );

}


/*
TypeMap fun for operator bnnavigation

*/

ListExpr OpTMBNNavigationTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);
  
  if(!(nl->IsAtom(nl->First(arg1)) && 
       nl->AtomType(nl->First(arg1)) == SymbolType &&
       nl->SymbolValue(nl->First(arg1)) == "busstop")){
      string err = "param1 should be busstop";
      return listutils::typeError(err);
  }
  
  if(!(nl->IsAtom(nl->First(arg2)) && 
       nl->AtomType(nl->First(arg2)) == SymbolType &&
       nl->SymbolValue(nl->First(arg2)) == "busstop")){
      string err = "param2 should be busstop";
      return listutils::typeError(err);
  }
  
  if(!(nl->IsAtom(nl->First(arg3)) && 
       nl->AtomType(nl->First(arg3)) == SymbolType &&
       nl->SymbolValue(nl->First(arg3)) == "busnetwork")){
      string err = "param3 should be bus network";
      return listutils::typeError(err);
  }
  
  if(!(nl->IsAtom(nl->First(arg4)) && 
       nl->AtomType(nl->First(arg4)) == SymbolType &&
       nl->SymbolValue(nl->First(arg4)) == "instant")){
      string err = "param4 should be instant";
      return listutils::typeError(err);
  }
  
    
  if(!(nl->IsAtom(nl->First(arg5)) && 
       nl->AtomType(nl->First(arg5)) == SymbolType &&
       nl->SymbolValue(nl->First(arg5)) == "int" )){
      string err = "param5 should be int";
      return listutils::typeError(err);
  }
  
  int n = nl->IntValue(nl->Second(arg5));
  ListExpr result; 
  switch(n){
    case 0: 
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("sline")),
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS1"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS2"),
                                    nl->SymbolAtom("string"))
                  )
                )
          );
        break;
    case 1: /////////////minimum travlling time 
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("sline")),
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS1"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS2"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("Duration"),
                                    nl->SymbolAtom("periods")),
                        nl->TwoElemList(nl->SymbolAtom("TimeCost"),
                                    nl->SymbolAtom("real"))
                  )
                )
          );
        break;
    case 2: //////////minimum number of transfers 
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("sline")),
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS1"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS2"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("Duration"),
                                    nl->SymbolAtom("periods")),
                        nl->TwoElemList(nl->SymbolAtom("TimeCost"),
                                    nl->SymbolAtom("real"))
                  )
                )
          );
        break;
    case 3: ///minimum travlling time with optimize, record searching time
          result =
          nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("sline")),
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS1"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("BS2"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("Duration"),
                                    nl->SymbolAtom("periods")),
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
TypeMap fun for operator testbnnavigation

*/

ListExpr OpTMTestBNNavigationTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  ListExpr arg5 = nl->Fifth(args);

  if(!IsRelDescription(nl->First(arg1)))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(BusGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(nl->First(arg1), xType))
    return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(nl->First(arg2)))
      return nl->SymbolAtom ( "typeerror" );

  nl->ReadFromString(BusGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(nl->First(arg2), xType))
      return nl->SymbolAtom ( "typeerror" );


  if(!(nl->IsAtom(nl->First(arg3)) && 
       nl->AtomType(nl->First(arg3)) == SymbolType &&
       nl->SymbolValue(nl->First(arg3)) == "busnetwork")){
      string err = "param3 should be bus network";
      return listutils::typeError(err);
  }
  
  if(!(nl->IsAtom(nl->First(arg4)) && 
       nl->AtomType(nl->First(arg4)) == SymbolType &&
       nl->SymbolValue(nl->First(arg4)) == "instant")){
      string err = "param4 should be instant";
      return listutils::typeError(err);
  }
  
    
  if(!(nl->IsAtom(nl->First(arg5)) && 
       nl->AtomType(nl->First(arg5)) == SymbolType &&
       nl->SymbolValue(nl->First(arg5)) == "int" )){
      string err = "param5 should be int";
      return listutils::typeError(err);
  }  
  return nl->SymbolAtom ( "bool" );

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

  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::bus_route_speed_typeinfo, xType1); 
  if(!CompareSchemas(param10, xType1)){
    return listutils::typeError("rel10 scheam should be" + 
                                RoadDenstiy::bus_route_speed_typeinfo);
  }


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

ListExpr OpTMCreateMovingBusNightTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::night_sched_typeinfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                RoadDenstiy::night_sched_typeinfo);
  }

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
TypeMap fun for operator create night and daytime moving bus 

*/

ListExpr OpTMCreateMovingBusDayTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::day_sched_typeinfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                RoadDenstiy::day_sched_typeinfo);
  }

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

ListExpr OpTMCreateTimeTable1TypeMap ( ListExpr args )
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
    return listutils::typeError("rel1 scheam should be" + 
                                RoadDenstiy::bus_stop_typeinfo);
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

  
  ListExpr xType2;
  nl->ReadFromString(RoadDenstiy::mo_bus_typeinfo, xType2); 
  if(!CompareSchemas(param2, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                                RoadDenstiy::mo_bus_typeinfo);
  }
  
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

/*
TypeMap fun for operator create time table for each spatial location 
Compact storage structure 

*/

ListExpr OpTMCreateTimeTable1NewTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::bus_stop_typeinfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                RoadDenstiy::bus_stop_typeinfo);
  }
  
  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );
  
  ListExpr xType2;
  nl->ReadFromString(RoadDenstiy::mo_bus_typeinfo, xType2); 
  if(!CompareSchemas(param2, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                                RoadDenstiy::mo_bus_typeinfo);
  }
  
  
  ListExpr index = nl->Third(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "param3  should be btree" );
  
  ListExpr param4 = nl->Fourth(args );
  if(!(nl->IsAtom(param4) && nl->AtomType(param4) == SymbolType &&  
     nl->SymbolValue(param4) == "periods")){
      return nl->SymbolAtom ( "typeerror: param4 should be periods" );
  }  
  
  
  ListExpr param5 = nl->Fifth(args );
  if(!(nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&  
     nl->SymbolValue(param5) == "periods")){
      return nl->SymbolAtom ( "typeerror: param5 should be periods" );
  }  
  
//    ListExpr res = nl->TwoElemList(
//             nl->SymbolAtom("stream"),
//             nl->TwoElemList(
//                 nl->SymbolAtom("tuple"),
//                 nl->Cons(            
//                     nl->TwoElemList(
//                         nl->SymbolAtom("stop_loc"),
//                         nl->SymbolAtom("point")),        
//                 nl->SixElemList(
//                     nl->TwoElemList(
//                         nl->SymbolAtom("br_id"),
//                         nl->SymbolAtom("int")),
//                     nl->TwoElemList(
//                         nl->SymbolAtom("stop_id"),
//                         nl->SymbolAtom("int")),
//                     nl->TwoElemList(
//                         nl->SymbolAtom("bus_direction"),
//                         nl->SymbolAtom("bool")),
//                     nl->TwoElemList(
//                         nl->SymbolAtom("whole_time"),
//                         nl->SymbolAtom("periods")),
//                     nl->TwoElemList(
//                         nl->SymbolAtom("schedule_interval"),
//                         nl->SymbolAtom("real")),
//                     nl->TwoElemList(
//                         nl->SymbolAtom("loc_id"),
//                         nl->SymbolAtom("int")))
//                     )));

   ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("whole_time"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_interval"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("loc_id"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_uoid"),
                        nl->SymbolAtom("int")))
                    ));

      return res; 
}

/*
TypeMap fun for operator create time table for each spatial location 

*/

ListExpr OpTMCreateTimeTable2TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
   ListExpr xType1;
   nl->ReadFromString(UBTrain::TrainsStopTypeInfo, xType1); 
   if(!CompareSchemas(param1, xType1)){
      return listutils::typeError("rel1 scheam should be" + 
                                UBTrain::TrainsStopTypeInfo);
   }


   ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

   ListExpr xType2;
   nl->ReadFromString(UBTrain::UBahnTrainsTypeInfo, xType2);
   if(!CompareSchemas(param2, xType2)){
      return listutils::typeError("rel2 scheam should be" + 
                                UBTrain::UBahnTrainsTypeInfo);
   }


  ListExpr index = nl->Third(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "param3  should be btree" );

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("station_loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("line_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("train_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_time"),
                        nl->SymbolAtom("instant")),
                    nl->TwoElemList(
                        nl->SymbolAtom("loc_id"),
                        nl->SymbolAtom("int"))
                    )));
      return res; 
}


/*
TypeMap fun for operator create time table for each spatial location 
Compact Storage 

*/

ListExpr OpTMCreateTimeTable2NewTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
   ListExpr xType1;
   nl->ReadFromString(UBTrain::TrainsStopTypeInfo, xType1); 
   if(!CompareSchemas(param1, xType1)){
      return listutils::typeError("rel1 scheam should be" + 
                                UBTrain::TrainsStopTypeInfo);
   }

   ListExpr param2 = nl->Second ( args );
   if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

   ListExpr xType2;
   nl->ReadFromString(UBTrain::UBahnTrainsTypeInfo, xType2);
   if(!CompareSchemas(param2, xType2)){
      return listutils::typeError("rel2 scheam should be" + 
                                UBTrain::UBahnTrainsTypeInfo);
   }
   
  
  ListExpr index = nl->Third(args);
  if(!listutils::isBTreeDescription(index))
      return  nl->SymbolAtom ( "param3  should be btree" );
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->Cons(            
                    nl->TwoElemList(
                        nl->SymbolAtom("station_loc"),
                        nl->SymbolAtom("point")),        
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("line_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("train_direction"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("whole_time"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_interval"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("loc_id"),
                        nl->SymbolAtom("int")))
                    )));
      return res; 
}

/*
conver berlintest trains to generic moving objects 

*/

ListExpr OpTMRefMO2GenMOTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }

  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(RoadDenstiy::mo_bus_typeinfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
      return listutils::typeError("rel1 scheam should be" + 
                                RoadDenstiy::mo_bus_typeinfo);
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

  ListExpr xType2;
  nl->ReadFromString(BusNetwork::BusRoutesTypeInfo, xType2); 
  if(!CompareSchemas(param2, xType2)){
     return listutils::typeError("rel2 scheam should be" + 
                               BusNetwork::BusRoutesTypeInfo);
  }

  ListExpr param3 = nl->Third ( args );
  if(!listutils::isBTreeDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a btree" );


      ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bustrip1"),
                        nl->SymbolAtom("genmo")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bustrip2"),
                        nl->SymbolAtom("mpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int"))
                    )));
    return  res;

}

/*
TypeMap fun for operator themetronetwork

*/
ListExpr OpTMTheMetroNetworkTypeMap ( ListExpr args )
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
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(MetroNetwork::MetroStopsTypeInfo, xType1); 
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                MetroNetwork::UBAHNStopsTypeInfo);
  }
  
  ListExpr param3 = nl->Third ( args );
  if(!IsRelDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a relation" );

  ListExpr xType2;
  nl->ReadFromString(MetroNetwork::MetroRoutesTypeInfo, xType2); 
  if(!CompareSchemas(param3, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                                MetroNetwork::UBAHNRoutesTypeInfo);
  }


  ListExpr param4 = nl->Fourth ( args );
  ListExpr xType3;
  nl->ReadFromString(MetroNetwork::MetroTripTypeInfo, xType3);
  if(!CompareSchemas(param4, xType3)){
    return listutils::typeError("rel3 scheam should be" + 
                                MetroNetwork::MetroTripTypeInfo);
  }

  return nl->SymbolAtom ( "metronetwork" );
}

/*
TypeMap fun for operator ms neighbor1

*/
ListExpr OpTMMSNeighbor1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }


  ListExpr param1 = nl->First( args );

  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(MetroNetwork::MetroStopsTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                MetroNetwork::MetroStopsTypeInfo);
  }

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("ms_stop1_tid"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("ms_stop2_tid"),
                        nl->SymbolAtom("int"))
                    )));
  return res; 
}

/*
TypeMap fun for operator ms neighbor2

*/
ListExpr OpTMMSNeighbor2TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
  }

  ListExpr param1 = nl->First(args);
  if(!(nl->IsAtom(param1) && nl->SymbolValue(param1) == "metronetwork")){
    return listutils::typeError("param1 should be metronetwork");
  }

  ListExpr param2 = nl->Second( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(UBTrain::UBahnTrainsTimeTable, xType1);
  if(!CompareSchemas(param2, xType1)){
    return listutils::typeError("rel1 scheam should be" +
                                UBTrain::UBahnTrainsTimeTable);
  }

  ListExpr param3 = nl->Third(args);
  if(!listutils::isBTreeDescription(param3))
    return nl->SymbolAtom("typeerror: param3 should be a btree");

  ListExpr param4 = nl->Fourth( args );
  if(!IsRelDescription(param4))
    return nl->SymbolAtom("typeerror: param4 should be a relation");

  ListExpr xType2;
  nl->ReadFromString(MetroStruct::MetroTripTypeInfo_Com, xType2); 
  if(!CompareSchemas(param4, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                                MetroStruct::MetroTripTypeInfo_Com);
  }

  ListExpr param5 = nl->Fifth(args);
  if(!listutils::isBTreeDescription(param5))
    return nl->SymbolAtom("typeerror: param5 should be a btree");

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("ms_stop1_tid"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("ms_stop2_tid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("whole_time"),
                        nl->SymbolAtom("periods")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_interval"),
                        nl->SymbolAtom("real")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline")),
                    nl->TwoElemList(
                        nl->SymbolAtom("TimeCost"),
                        nl->SymbolAtom("real"))
                    )));
  return res; 
}

/*
type map for operator createmgraph 

*/
ListExpr OpTMCreateMetroGraphTypeMap ( ListExpr args )
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
  nl->ReadFromString(MetroGraph::MGNodeTypeInfo, xType1);
  if(!CompareSchemas(xNodeDesc, xType1))return nl->SymbolAtom ( "typeerror" );

  if(!IsRelDescription(xEdgeDesc1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(MetroGraph::MGEdge1TypeInfo, xType2);
  if(!CompareSchemas(xEdgeDesc1, xType2))return nl->SymbolAtom ( "typeerror" );


  if(!IsRelDescription(xEdgeDesc2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType3;
  nl->ReadFromString(MetroGraph::MGEdge2TypeInfo, xType3);
  if(!CompareSchemas(xEdgeDesc2, xType3))return nl->SymbolAtom ( "typeerror" );


  return nl->SymbolAtom ( "metrograph" );
}


/*
type map for operator createmetroroute

*/
ListExpr OpTMCreateMetroRouteTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args); 

  if(!nl->IsEqual(param1, "dualgraph"))
    return nl->SymbolAtom ( "typeerror: param1 should be dual graph" );

  ListExpr param2 = nl->Second(args); 

  if(!nl->IsEqual(param2, "string"))
    return nl->SymbolAtom ( "typeerror: param2 should be string" );

   ListExpr res = nl->TwoElemList(
             nl->SymbolAtom("stream"),
             nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                 nl->TwoElemList(
                     nl->TwoElemList(
                         nl->SymbolAtom("mr_id"),
                         nl->SymbolAtom("int")),
                     nl->TwoElemList(
                         nl->SymbolAtom("mroute"),
                         nl->SymbolAtom("busroute"))
                     )));

  return res; 
}


/*
type map for operator createmetrostop

*/
ListExpr OpTMCreateMetroStopTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args); 

  if(!IsRelDescription(param1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(MetroStruct::MetroRouteInfo, xType);
  if(!CompareSchemas(param1, xType))
      return nl->SymbolAtom ( "typeerror" );


   ListExpr res = nl->TwoElemList(
             nl->SymbolAtom("stream"),
             nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                 nl->ThreeElemList(
                     nl->TwoElemList(
                         nl->SymbolAtom("ms_stop"),
                         nl->SymbolAtom("busstop")),
                     nl->TwoElemList(
                         nl->SymbolAtom("stop_geodata"),
                         nl->SymbolAtom("point")),
                     nl->TwoElemList(
                         nl->SymbolAtom("mr_id"),
                         nl->SymbolAtom("int"))
                     )));

  return res; 
}


/*
type map for operator createmetromo

*/
ListExpr OpTMCreateMetroMOTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args); 

  if(!IsRelDescription(param1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType;
  nl->ReadFromString(MetroStruct::MetroRouteInfo, xType);
  if(!CompareSchemas(param1, xType))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr param2 = nl->Second(args);
  if(!(nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&  
     nl->SymbolValue(param2) == "periods"))
    return nl->SymbolAtom ( "typeerror" );

   ListExpr res = nl->TwoElemList(
             nl->SymbolAtom("stream"),
             nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                 nl->FiveElemList(
                     nl->TwoElemList(
                         nl->SymbolAtom("mtrip1"),
                         nl->SymbolAtom("genmo")),
                     nl->TwoElemList(
                         nl->SymbolAtom("mtrip2"),
                         nl->SymbolAtom("mpoint")),
                     nl->TwoElemList(
                         nl->SymbolAtom("mr_id"),
                         nl->SymbolAtom("int")),
                     nl->TwoElemList(
                         nl->SymbolAtom("Up"),
                         nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                         nl->SymbolAtom("mr_oid"),
                         nl->SymbolAtom("int"))
                     )));

  return res; 
}

/*
type map for operator mapmstopave

*/
ListExpr OpTMMapMsToPaveTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args);
  if(!IsRelDescription(param1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType1;
  nl->ReadFromString(MetroNetwork::MetroStopsTypeInfo, xType1);
  if(!CompareSchemas(param1, xType1))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr param2 = nl->Second(args);
  if(!IsRelDescription(param2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr xType2;
  nl->ReadFromString(DualGraph::NodeTypeInfo, xType2);
  if(!CompareSchemas(param2, xType2))
      return nl->SymbolAtom ( "typeerror" );

  ListExpr param3 = nl->Third(args); 
  if(!listutils::isRTreeDescription(param3))
      return nl->SymbolAtom("typeerror");

   ListExpr res = nl->TwoElemList(
             nl->SymbolAtom("stream"),
             nl->TwoElemList(
                 nl->SymbolAtom("tuple"),
                 nl->FourElemList(
                     nl->TwoElemList(
                         nl->SymbolAtom("loc1"),
                         nl->SymbolAtom("genloc")),
                     nl->TwoElemList(
                         nl->SymbolAtom("loc2"),
                         nl->SymbolAtom("point")),
                     nl->TwoElemList(
                         nl->SymbolAtom("ms_stop"),
                         nl->SymbolAtom("busstop")),
                     nl->TwoElemList(
                         nl->SymbolAtom("ms_stop_loc"),
                         nl->SymbolAtom("point"))
                     )));

  return res;
}


/*
TypeMap fun for operator mnnavigation

*/

ListExpr OpTMMNNavigationTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  ListExpr arg3 = nl->Third(args);
  ListExpr arg4 = nl->Fourth(args);
  
  
  if(!(nl->IsAtom(arg1) && 
       nl->AtomType(arg1) == SymbolType && nl->SymbolValue(arg1) == "busstop")){
      string err = "param1 should be busstop";
      return listutils::typeError(err);
  }

  if(!(nl->IsAtom(arg2) && 
      nl->AtomType(arg2) == SymbolType && nl->SymbolValue(arg2) == "busstop")){
      string err = "param2 should be busstop";
      return listutils::typeError(err);
  }

  if(!(nl->IsAtom(arg3) && 
       nl->AtomType(arg3) == SymbolType &&
       nl->SymbolValue(arg3) == "metronetwork")){
      string err = "param3 should be metro network";
      return listutils::typeError(err);
  }

  if(!(nl->IsAtom(arg4) && 
      nl->AtomType(arg4) == SymbolType && nl->SymbolValue(arg4) == "instant")){
      string err = "param4 should be instant";
      return listutils::typeError(err);
  }


  ListExpr result =

        nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->SixElemList(
                        nl->TwoElemList(nl->SymbolAtom("Path"),
                                    nl->SymbolAtom("sline")),
                        nl->TwoElemList(nl->SymbolAtom("TM"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("MS1"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("MS2"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("Duration"),
                                    nl->SymbolAtom("periods")),
                        nl->TwoElemList(nl->SymbolAtom("TimeCost"),
                                    nl->SymbolAtom("real"))
                  )
                )
          );

  return result; 

}

/*
TypeMap fun for operator instant2day

*/

ListExpr OpTMInstant2DayNewTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  if(nl->IsEqual(nl->First(args),"instant")){
    return nl->SymbolAtom("int");
  }else
    return nl->SymbolAtom ( "typeerror: param1 should be instant" );
}

/*
TypeMap fun for operator maxrect 

*/

ListExpr OpTMMaxRectTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  if(nl->IsEqual(nl->First(args),"region")){
    return nl->SymbolAtom("rect");
//    return nl->SymbolAtom("region"); 
  }else
    return nl->SymbolAtom ( "typeerror: param1 should be region" );
}


/*
TypeMap fun for operator getrect. get the maximum rectangle from a convex
region 

*/

ListExpr OpTMGetRect1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return  nl->SymbolAtom ( "list length should be 4" );
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

  ListExpr attrName2 = nl->Third ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2, attrType2);

  if(j2 == 0 || !listutils::isSymbol(attrType2,"region")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

  ListExpr param4 = nl->Fourth(args);
  if(!(nl->SymbolValue(param4) == "string")){
    return nl->SymbolAtom ( "typeerror: param4 should be a string" );
    
  }

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("reg_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("geoData"),
                        nl->SymbolAtom("rect")),
                    nl->TwoElemList(
                        nl->SymbolAtom("poly_id"),
                        nl->SymbolAtom("int"))
                    )));
      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(nl->IntAtom(j1), nl->IntAtom(j2)), res);

}

/*
TypeMap fun for operator path to building.
build the connection between the entrance of the building and pavement 

*/

ListExpr OpTMPathToBuildingTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 4 )
  {
    return  nl->SymbolAtom ( "list length should be 4" );
  }
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(MaxRect::BuildingRectExtTypeInfo, xType1);
  if(!CompareSchemas(param1, xType1)){
      string err = "rel (tuple ((reg_id int) (geoData rect) (poly_id int) \
                   (reg_type int))) expected";
      return listutils::typeError(err);
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );

  ListExpr xType2;
  nl->ReadFromString(MaxRect::RegionElemTypeInfo, xType2); 
  if(!CompareSchemas(param2, xType2)){
      string err = "rel (tuple ((id int) (covarea region))) expected";
      return listutils::typeError(err);
  }


  ListExpr param3 = nl->Third(args);
  if(!listutils::isBTreeDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a btree" );

  ListExpr param4 = nl->Fourth(args);
  if(!nl->IsEqual(param4, "space"))
    return nl->SymbolAtom ( "typeerror: param4 should be space" );


  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                   nl->Cons(
                      nl->TwoElemList(
                              nl->SymbolAtom("reg_id"),
                              nl->SymbolAtom("int")),
                      nl->SixElemList(
                            nl->TwoElemList(
                              nl->SymbolAtom("sp"),
                              nl->SymbolAtom("point")),
                            nl->TwoElemList(
                              nl->SymbolAtom("sp_index"),
                              nl->SymbolAtom("int")),
                            nl->TwoElemList(
                              nl->SymbolAtom("ep"),
                              nl->SymbolAtom("point")),
                            nl->TwoElemList(
                              nl->SymbolAtom("ep2"),
                              nl->SymbolAtom("point")),
                            nl->TwoElemList(
                              nl->SymbolAtom("ep2_gloc"),
                              nl->SymbolAtom("genloc")),
                            nl->TwoElemList(
                              nl->SymbolAtom("Path"),
                              nl->SymbolAtom("line")))
                    )));

    return res;
}

/*
TypeMap fun for operator set building type.

*/

ListExpr OpTMSetBuildingTypeTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 2" );
  }
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );

  ListExpr xType1;
  nl->ReadFromString(MaxRect::BuildingRectTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
      string err = "rel (tuple ((reg_id int) (geoData rect) (poly_id int) \
                   (reg_type int))) expected";
      return listutils::typeError(err);
  }


  ListExpr param2 = nl->Second ( args );
  if(!listutils::isRTreeDescription(param2)){
      string err = "rtree expected";
      return listutils::typeError(err);
  }

  ListExpr param3 = nl->Third ( args );
  if(!nl->IsEqual(param3, "space"))
    return nl->SymbolAtom ( "typeerror: param3 should be space" );


    ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                      nl->Cons(
                            nl->TwoElemList(
                              nl->SymbolAtom("reg_id"),
                              nl->SymbolAtom("int")),
                      nl->SixElemList(
                            nl->TwoElemList(
                              nl->SymbolAtom("geoData"),
                              nl->SymbolAtom("rect")),
                            nl->TwoElemList(
                              nl->SymbolAtom("poly_id"),
                              nl->SymbolAtom("int")),
                            nl->TwoElemList(
                              nl->SymbolAtom("reg_type"),
                              nl->SymbolAtom("int")),
                            nl->TwoElemList(
                              nl->SymbolAtom("building_type"),
                              nl->SymbolAtom("int")),
                            nl->TwoElemList(
                              nl->SymbolAtom("building_type2"),
                              nl->SymbolAtom("string")),
                            nl->TwoElemList(
                              nl->SymbolAtom("building_id"),
                              nl->SymbolAtom("int")))
                    )));
    return res;
}

/*
TypeMap fun for operator remove dirty 
remove dirty region data 

*/

ListExpr OpTMRemoveDirtyTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
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
  
  ListExpr attrName2 = nl->Third ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2, attrType2);

  if(j2 == 0 || !listutils::isSymbol(attrType2,"region")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type region");
  }

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("covarea"),
//                        nl->SymbolAtom("sline"))
                        nl->SymbolAtom("region"))
                    )));
      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)), res);

}


/*
TypeMap fun for operator modifyline

*/

ListExpr OpTMModifyLineTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );


  if (nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
      nl->SymbolValue(param1) == "sline" )
  {
    return nl->SymbolAtom ( "sline" );
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator checkroads

*/
ListExpr OpTMCheckRoadsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xRoutesRelDesc = nl->First ( args );

 if ( !IsRelDescription ( xRoutesRelDesc ))
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  ListExpr xType;
  nl->ReadFromString ( Network::routesTypeInfo, xType );
  if ( !CompareSchemas ( xRoutesRelDesc, xType ) )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  
  ListExpr param2 = nl->Second( args );

 if(!listutils::isRTreeDescription(param2) )
    return listutils::typeError("param2 should be an rtree" );


  ListExpr result =

        nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FiveElemList(
                        nl->TwoElemList(nl->SymbolAtom("id"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("lengt"),
                                    nl->SymbolAtom("real")),
                        nl->TwoElemList(nl->SymbolAtom("curve"),
                                    nl->SymbolAtom("sline")),
                        nl->TwoElemList(nl->SymbolAtom("dual"),
                                    nl->SymbolAtom("bool")),
                        nl->TwoElemList(nl->SymbolAtom("startSmaller"),
                                    nl->SymbolAtom("bool"))
                  )
                )
          );

  return result; 

}


/*
TypeMap fun for operator tmjoin

*/
ListExpr OpTMTMJoin1TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xRoutesRelDesc = nl->First ( args );

  if ( !IsRelDescription ( xRoutesRelDesc ))
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  ListExpr xType;
  nl->ReadFromString ( TM_Join::RoadSectionTypeInfo, xType );
  if ( !CompareSchemas ( xRoutesRelDesc, xType ) )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  ListExpr cellRelDesc = nl->Second( args );

  if ( !IsRelDescription ( cellRelDesc ))
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  ListExpr xType2;
  nl->ReadFromString ( TM_Join::CellBoxTypeInfo, xType2 );
  if ( !CompareSchemas ( cellRelDesc, xType2 ) )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }



  ListExpr param3 = nl->Third( args );

 if(!listutils::isRTreeDescription(param3) )
    return listutils::typeError("param3 should be an rtree" );

  ListExpr result =

        nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->FourElemList(
                        nl->TwoElemList(nl->SymbolAtom("Secid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Cellid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Cnt"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Cover_area"),
                                    nl->SymbolAtom("region"))
                  )
                )
          );

  return result; 

}



/*
TypeMap fun for operator nearest stop pave

*/
ListExpr OpNearStopPaveTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  ListExpr arg1 = nl->First(args);

  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "string")){
      string err = "the second parameter should be string";
      return listutils::typeError(err);
  }

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
                                    nl->SymbolAtom("region"))                  )
                )
          );

  return result; 

}


/*
TypeMap fun for operator nearest stop pave

*/
ListExpr OpNearStopBuildingTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }

  ListExpr arg1 = nl->First(args);

  if(!nl->IsEqual(arg1, "space")){
      string err = "the first parameter should be space";
      return listutils::typeError(err);
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2, "string")){
      string err = "the second parameter should be string";
      return listutils::typeError(err);
  }

  ListExpr result =

        nl->TwoElemList(
              nl->SymbolAtom("stream"),
                nl->TwoElemList(
                  nl->SymbolAtom("tuple"),
                      nl->ThreeElemList(
                        nl->TwoElemList(nl->SymbolAtom("Tid"),
                                    nl->SymbolAtom("int")),
                        nl->TwoElemList(nl->SymbolAtom("Type"),
                                    nl->SymbolAtom("string")),
                        nl->TwoElemList(nl->SymbolAtom("Area"),
                                    nl->SymbolAtom("rect"))
                                    )
                )
          );

  return result; 

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
Correct road with dirt data, two segments are very close to each other and the
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
Value Mapping for the triangulate operator
decompose a polygon into a set of triangles.
use the code from codeproject HGRD

*/
int OpTMTriangulate2map ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  CompTriangle* ct;
  switch(message){
      case OPEN:{
        Region* reg = (Region*)args[0].addr;
        ct = new CompTriangle(reg);
        ct->NewTriangulation2();
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
the polygon should not have holes inside 

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
Value Mapping for walkspold  operator
return the shortest path for pedestrian

*/

int OpTMWalkSPOldValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
      DualGraph* dg = (DualGraph*)args[0].addr;
      VisualGraph* vg = (VisualGraph*)args[1].addr;
      Relation* r1 = (Relation*)args[2].addr;
      Relation* r2 = (Relation*)args[3].addr;
      Relation* r3 = (Relation*)args[4].addr;

      Walk_SP* wsp = new Walk_SP(dg, vg, r1, r2);
      wsp->rel3 = r3;

      result = qp->ResultStorage(in_pSupplier);
      Line* res = static_cast<Line*>(result.addr);
      wsp->WalkShortestPath(res);
      delete wsp; 
      return 0;
}

/*
Value Mapping for walksp  operator
return the shortest path for pedestrian

*/

int OpTMWalkSPValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
      Pavement* pn = (Pavement*)args[0].addr;

      Relation* r1 = (Relation*)args[1].addr;
      Relation* r2 = (Relation*)args[2].addr;
      Relation* r3 = (Relation*)args[3].addr;
      result = qp->ResultStorage(in_pSupplier);
      if(pn->IsDGInit() == false){
        cout<<"dual graph is not initialized"<<endl;
        return 0;
      }
      if(pn->IsVGInit() == false){
        cout<<"visual graph is not initialized"<<endl;
        return 0;
      }
      DualGraph* dg = pn->GetDualGraph();
      VisualGraph* vg = pn->GetVisualGraph();
      if(dg == NULL || vg == NULL){
        cout<<"graph invalid"<<endl;
        return 0; 
      }

      Walk_SP* wsp = new Walk_SP(dg, vg, r1, r2);
      wsp->rel3 = r3;

      Line* res = static_cast<Line*>(result.addr);
      wsp->WalkShortestPath(res);
      delete wsp; 
      pn->CloseDualGraph(dg);
      pn->CloseVisualGraph(vg); 
      return 0;
}

/*
Value Mapping for testwalksp old  operator
return the shortest path for pedestrian

*/

int OpTMTestWalkSPValueMap ( Word* args, Word& result, int message,
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


        wsp = new Walk_SP(dg, vg, r1, r2);
        wsp->rel3 = r3;

        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        for(int i = 1;i <= r1->GetNoTuples();i++){
            for(int j = i + 1;j <= r2->GetNoTuples();j++)
                wsp->TestWalkShortestPath(i, j);
        }
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
Value Mapping for pave loctogp  operator
map locations in pavements to gpoints 

*/

int OpTMPaveLocToGPValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Walk_SP* wsp;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr;
        Network* n = (Network*)args[3].addr;

        wsp = new Walk_SP(NULL, NULL, r1, r2);
        wsp->btree = btree;
        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        wsp->PaveLocToGP(n);
        local.setAddr(wsp);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          wsp = (Walk_SP*)local.addr;
//          if(wsp->count == wsp->gp_list.size())
          if(wsp->count == wsp->p_list.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(wsp->resulttype);
          tuple->PutAttribute(0, new Point(wsp->p_list[wsp->count]));
          tuple->PutAttribute(1, new GPoint(wsp->gp_list[wsp->count]));
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
Value Mapping for setpave rid  operator
set road id for each pavement 

*/

int OpTMSetPaveRidValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Walk_SP* wsp;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[2].addr;

        wsp = new Walk_SP(NULL, NULL, r1, r2);
        wsp->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        wsp->SetPaveRid(rtree);
        local.setAddr(wsp);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          wsp = (Walk_SP*)local.addr;
          if(wsp->count == wsp->oid_list.size())
                          return CANCEL;

          Tuple* tuple = new Tuple(wsp->resulttype);
          tuple->PutAttribute(0, new CcInt(true, wsp->oid_list[wsp->count]));
          tuple->PutAttribute(1, new CcInt(true, wsp->rid_list[wsp->count]));
          tuple->PutAttribute(2, new Region(wsp->reg_list[wsp->count]));
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
decompose a region into a set of triangles where each is represented by the
three points. We number all vertices of a polygon. It works together with
operator regvertex. the triangulation is different. it is consistent with
operator triangulation2. 

*/

int OpTMTriangulationNew2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  RegVertex* rv;
  switch(message){
      case OPEN:{

        Region* r = (Region*)args[0].addr;
        rv = new RegVertex(r);
        rv->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        rv->TriangulationNew2();
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
//        cout<<"rid "<<rid<<" m1 "<<meas1<<" m2 "<<meas2<<endl; 
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

      
      cell_array.push_back(*cell);
      cell_id++;
      x_value_min = x_value_max; 
      delete cell;
      delete box;
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
        BTree* btree = (BTree*)args[5].addr;
        Relation* bus_para = (Relation*)args[6].addr;


        int attr1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[9].addr)->GetIntval() - 1;


        br = new BusRoute(n,r,btree);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateRoute1(attr1,attr2,attr3, bus_para);
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
//        Network* n = (Network*)args[0].addr;
        Space* sp = (Space*)args[0].addr;
        Relation* r1 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[3].addr;
        Relation* r2 = (Relation*)args[4].addr; 
        
        int attr = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr1 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[11].addr)->GetIntval() - 1;

        br = new BusRoute(NULL,r1,btree,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateRoute2(sp, attr,attr1,attr2,attr3);
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
calculate the total length of bus routes in road network.
It returns the percentage value 

*/
int OpTMBusRouteRoadValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  Network* n = (Network*)args[0].addr;
  Relation* r = (Relation*)args[1].addr; 
        
  int attr1 = ((CcInt*)args[3].addr)->GetIntval() - 1;
  
  BusRoute* br = new BusRoute(n,r, NULL, NULL);
  float route_road = br->BusRouteInRoad(attr1);
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*)result.addr; 
  pResult->Set(true, route_road); 
  return 1;

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
        Relation* r2 = (Relation*)args[6].addr;
        BTree* btree = (BTree*)args[7].addr; 
        string type = ((CcString*)args[8].addr)->GetValue();

        int attr1 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[12].addr)->GetIntval() - 1;
        
        
        br = new BusRoute(n,r1,NULL,NULL);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop1(attr1, attr2, attr3, attr4, r2, btree, type);
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
          tuple->PutAttribute(6, 
                              new CcInt(true,br->stop_loc_id_list[br->count]));


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
create bus stops with data type bus stop

*/
int OpTMGetBusStopsValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr; 
        BTree* btree = (BTree*)args[1].addr; 
        Relation* r2 = (Relation*)args[2].addr; 

        br = new BusRoute(NULL,r1, btree,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        br->GetBusStops();
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->bus_stop_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);

          tuple->PutAttribute(0, new Bus_Stop(br->bus_stop_list[br->count]));
          tuple->PutAttribute(1, new Point(br->bus_stop_geodata[br->count]));

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
create bus routes with data type bus route

*/
int OpTMGetBusRoutesValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BusRoute* br;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr; 
        BTree* btree = (BTree*)args[1].addr; 
        Relation* r2 = (Relation*)args[2].addr; 

        br = new BusRoute(NULL,r1, btree,r2);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        br->GetBusRoutes();
        local.setAddr(br);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          br = (BusRoute*)local.addr;
          if(br->count == br->bus_route_list.size())return CANCEL;

          Tuple* tuple = new Tuple(br->resulttype);

          tuple->PutAttribute(0, new CcInt(br->br_id_list[br->count]));
          tuple->PutAttribute(1, new Bus_Route(br->bus_route_list[br->count]));

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
get the simpleline of a bus route 

*/
int OpTMBRGeoDataValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Bus_Route* br = (Bus_Route*)args[0].addr;
  if(br->IsDefined()){
    SimpleLine* res = static_cast<SimpleLine*>(result.addr); 
    SimpleLine sl(0); 
    br->GetGeoData(sl);
    *res = sl; 
  }
  return 0;
}

/*
get the point of a bus stop

*/
int OpTMBSGeoData1ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Bus_Stop* bs = (Bus_Stop*)args[0].addr; 
  Bus_Route* br = (Bus_Route*)args[1].addr;
  
  if(bs->IsDefined() && br->IsDefined()){
    Point* res = static_cast<Point*>(result.addr); 
    br->GetBusStopGeoData(bs, res);
  }
  return 0;
}

/*
get the point of a bus stop

*/
int OpTMBSGeoData2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Bus_Stop* bs = (Bus_Stop*)args[0].addr; 
  BusNetwork* bn = (BusNetwork*)args[1].addr;
  
  if(bs->IsDefined() && bn->IsDefined()){
    Point* res = static_cast<Point*>(result.addr); 
    bn->GetBusStopGeoData(bs, res);
  }
  return 0;
}


/*
get bus stop id

*/
int OpTMGetStopIdValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Bus_Stop* bs = (Bus_Stop*)args[0].addr;
  if(bs->IsDefined())
    ((CcInt*)result.addr)->Set(true,bs->GetStopId());
  else
    ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
get bus stop direction 

*/
int OpTMBusStopUpDownValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Bus_Stop* bs = (Bus_Stop*)args[0].addr;
  if(bs->IsDefined())
    ((CcBool*)result.addr)->Set(true,bs->GetUp());
  else
    ((CcBool*)result.addr)->Set(false, false);
  return 0;
}

/*
get bus route direction 

*/
int OpTMBusRouteUpDownValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Bus_Route* br = (Bus_Route*)args[0].addr;
  if(br->IsDefined())
    ((CcBool*)result.addr)->Set(true,br->GetUp());
  else
    ((CcBool*)result.addr)->Set(false, false);
  return 0;
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
create the pavement infrastructure 

*/
int OpTMThePavementValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  Pavement* pn = (Pavement*)qp->ResultStorage(in_pSupplier).addr;
  int id = ((CcInt*)args[0].addr)->GetIntval(); 
  if(ChekPavementId(id)){
    Relation* paves = (Relation*)args[1].addr;
    pn->Load(id, paves);
    result = SetWord(pn); 
  }else{
    cout<<"invalid pavement id "<<id<<endl; 
    while(ChekPavementId(id) == false || id <= 0){
      id++;
    }
    cout<<"new pavement id "<<id<<endl; 
    Relation* paves = (Relation*)args[1].addr;
    pn->Load(id, paves);
    result = SetWord(pn); 
  }
  return 0;
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
create the bus network infrastructure 

*/
int OpTMTheBusNetworkValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  BusNetwork* bn = (BusNetwork*)qp->ResultStorage(in_pSupplier).addr;
  int id = ((CcInt*)args[0].addr)->GetIntval(); 
  if(ChekBusNetworkId(id)){
    Relation* stops = (Relation*)args[1].addr;
    Relation* routes = (Relation*)args[2].addr;
    Relation* buses = (Relation*)args[3].addr;
    bn->Load(id, stops, routes, buses);
    result = SetWord(bn); 
  }else{
    cout<<"invalid bus network id "<<id<<endl; 
    while(ChekBusNetworkId(id) == false || id <= 0){
      id++;
    }
    cout<<"new bus network id "<<id<<endl; 
    Relation* stops = (Relation*)args[1].addr;
    Relation* routes = (Relation*)args[2].addr; 
    Relation* buses = (Relation*)args[3].addr; 
    bn->Load(id, stops, routes, buses);
    result = SetWord(bn); 
  }
  return 0;
}

/*
get bus stops data from bus network

*/
int OpTMBusStopsValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        BusNetwork* bn = (BusNetwork*)args[0].addr; 

        b_n = new BN(bn);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->GetStops();
        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

          tuple->PutAttribute(0, new Bus_Stop(b_n->bs_list[b_n->count]));

          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
get bus routes data from bus network

*/
int OpTMBusRoutesValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        BusNetwork* bn = (BusNetwork*)args[0].addr; 

        b_n = new BN(bn);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->GetRoutes();
        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->br_list.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

          tuple->PutAttribute(0, new Bus_Route(b_n->br_list[b_n->count]));

          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
map the bus stops to the pavements 

*/
int OpTMMapBsToPaveValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        BusNetwork* bn = (BusNetwork*)args[0].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[1].addr; 
        Relation* pave_rel = (Relation*)args[2].addr;
        int w = ((CcInt*)args[3].addr)->GetIntval();
        string type = ((CcString*)args[4].addr)->GetValue();

        b_n = new BN(bn);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->MapBSToPavements(rtree, pave_rel, w, type);
        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

          tuple->PutAttribute(0, new Bus_Stop(b_n->bs_list[b_n->count]));
          tuple->PutAttribute(1, new GenLoc(b_n->genloc_list[b_n->count]));
          tuple->PutAttribute(2, new Point(b_n->geo_list[b_n->count]));


          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
bs neighbors. for each bus stop, find neighbor bus stops 
(connected by pavements)

*/
int OpTMBsNeighbor1ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        DualGraph* dg = (DualGraph*)args[0].addr;
        VisualGraph* vg = (VisualGraph*)args[1].addr;
        Relation* tri_rel = (Relation*)args[2].addr;
        Relation* bs_pave_rel = (Relation*)args[3].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[4].addr; 


        b_n = new BN(NULL);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->BsNeighbors1(dg, vg, tri_rel, bs_pave_rel, rtree);
        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

         tuple->PutAttribute(0, new CcInt(true, b_n->bs_uoid_list[b_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(b_n->bs_list1[b_n->count]));
          tuple->PutAttribute(2, new Bus_Stop(b_n->bs_list2[b_n->count]));
          tuple->PutAttribute(3, new SimpleLine(b_n->path_sl_list[b_n->count]));
          tuple->PutAttribute(4, new SimpleLine(b_n->sub_path1[b_n->count]));
          tuple->PutAttribute(5, new SimpleLine(b_n->sub_path2[b_n->count]));
         tuple->PutAttribute(6, new SimpleLine(b_n->path2_sl_list[b_n->count]));
          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
bs neighbors. for each bus stop, find neighbor bus stops 
they have the same spatial location but belong to different bus routes 

*/
int OpTMBsNeighbor2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        BusNetwork* bn = (BusNetwork*)args[0].addr;

        b_n = new BN(bn);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->BsNeighbors2();
        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

         tuple->PutAttribute(0, new CcInt(true, b_n->bs_uoid_list[b_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(b_n->bs_list1[b_n->count]));
          tuple->PutAttribute(2, new Bus_Stop(b_n->bs_list2[b_n->count]));

          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
bs neighbors3. for each bus stop, find neighbor bus stops 
connected by moving buses belonging to the same route 

*/
int OpTMBsNeighbor3ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr; 

        b_n = new BN(NULL);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->BsNeighbors3(r1, r2, btree);
        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

         tuple->PutAttribute(0, new CcInt(true, b_n->bs_uoid_list[b_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(b_n->bs_list1[b_n->count]));
          tuple->PutAttribute(2, new Bus_Stop(b_n->bs_list2[b_n->count]));
          tuple->PutAttribute(3, new Periods(b_n->duration[b_n->count]));
          tuple->PutAttribute(4, 
                          new CcReal(true, b_n->schedule_interval[b_n->count]));
          tuple->PutAttribute(5, new SimpleLine(b_n->path_sl_list[b_n->count]));
          tuple->PutAttribute(6, 
                              new CcReal(true,b_n->time_cost_list[b_n->count]));

          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

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
Value Mapping for  createbusgraph  operator

*/

int OpTMCreateBusGraphValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  BusGraph* bg = (BusGraph*)qp->ResultStorage(in_pSupplier).addr;
  int bg_id = ((CcInt*)args[0].addr)->GetIntval();
  if(ChekBusGraphId(bg_id)){
    Relation* node_rel = (Relation*)args[1].addr;
    Relation* edge_rel1 = (Relation*)args[2].addr;
    Relation* edge_rel2 = (Relation*)args[3].addr; 
    Relation* edge_rel3 = (Relation*)args[4].addr; 
  
    bg->Load(bg_id, node_rel, edge_rel1, edge_rel2, edge_rel3);
    result = SetWord(bg);
  }else{
    cout<<"invalid bus graph id "<<bg_id<<endl; 
    while(ChekBusGraphId(bg_id) == false || bg_id <= 0){
      bg_id++;
    }
    cout<<"new bus graph id "<<bg_id<<endl; 
    Relation* node_rel = (Relation*)args[1].addr;
    Relation* edge_rel1 = (Relation*)args[2].addr;
    Relation* edge_rel2 = (Relation*)args[3].addr; 
    Relation* edge_rel3 = (Relation*)args[4].addr; 
    bg->Load(bg_id, node_rel, edge_rel1, edge_rel2, edge_rel3);
    result = SetWord(bg);
  }
  return 0;
}



/*
for each bus stop, find neighbor bus stops by searching on the bus graph 
  the input is bus stop node id 

*/
int OpTMGetAdjNodeBGIntValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        BusGraph* bg = (BusGraph*)args[0].addr;
        int nodeid = ((CcInt*)args[1].addr)->GetIntval();

        b_n = new BN(NULL);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->GetAdjNodeBG1(bg, nodeid);
        ////////////for testing///////////////////
//      for(int i = 1;i <= bg->node_rel->GetNoTuples(); i++){
//          nodeid = i; 
//          b_n->GetAdjNodeBG1(bg, nodeid);
//      }

        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

          tuple->PutAttribute(0, new Bus_Stop(b_n->bs_list1[b_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(b_n->bs_list2[b_n->count]));
          tuple->PutAttribute(2, new SimpleLine(b_n->path_sl_list[b_n->count]));
          tuple->PutAttribute(3, new CcInt(true,b_n->type_list[b_n->count]));

          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
for each bus stop, find neighbor bus stops by searching on the bus graph 
  the input is bus stop 

*/
int OpTMGetAdjNodeBGBSValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  BN* b_n;
  switch(message){
      case OPEN:{
        BusGraph* bg = (BusGraph*)args[0].addr;
        Bus_Stop* bs = (Bus_Stop*)args[1].addr;

        b_n = new BN(NULL);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->GetAdjNodeBG2(bg, bs);
        ////////////for testing///////////////////
//      for(int i = 1;i <= bg->node_rel->GetNoTuples(); i++){
//          Tuple* bs_tuple = bg->node_rel->GetTuple(i, false);
//          Bus_Stop* bs = (Bus_Stop*)bs_tuple->GetAttribute(BusGraph::BG_NODE);
//          b_n->GetAdjNodeBG2(bg, bs);
//          bs_tuple->DeleteIfAllowed();
//      }

        local.setAddr(b_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          b_n = (BN*)local.addr;
          if(b_n->count == b_n->bs_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(b_n->resulttype);

          tuple->PutAttribute(0, new Bus_Stop(b_n->bs_list1[b_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(b_n->bs_list2[b_n->count]));
          tuple->PutAttribute(2, new SimpleLine(b_n->path_sl_list[b_n->count]));
          tuple->PutAttribute(3, new CcInt(true, b_n->type_list[b_n->count]));

          result.setAddr(tuple);
          b_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            b_n = (BN*)local.addr;
            delete b_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
for each metro stop, find neighbor bus stops by searching on the metro graph 
  the input is metro stop node id 

*/
int OpTMGetAdjNodeMGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  MNNav* m_n;
  switch(message){
      case OPEN:{
        MetroGraph* mg = (MetroGraph*)args[0].addr;
        int nodeid = ((CcInt*)args[1].addr)->GetIntval();

        m_n = new MNNav(NULL);
        m_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        m_n->GetAdjNodeMG(mg, nodeid);

        local.setAddr(m_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          m_n = (MNNav*)local.addr;
          if(m_n->count == m_n->ms_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(m_n->resulttype);

          tuple->PutAttribute(0, new Bus_Stop(m_n->ms_list1[m_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(m_n->ms_list2[m_n->count]));
          tuple->PutAttribute(2, new SimpleLine(m_n->path_list[m_n->count]));
          tuple->PutAttribute(3, new CcInt(true,m_n->type_list[m_n->count]));

          result.setAddr(tuple);
          m_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            m_n = (MNNav*)local.addr;
            delete m_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
for each junction, find neighbor junctions by searching on the road graph 
  the input is junction id 

*/
int OpTMGetAdjNodeRGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadNav* r_n;
  switch(message){
      case OPEN:{
        RoadGraph* rg = (RoadGraph*)args[0].addr;
        int nodeid = ((CcInt*)args[1].addr)->GetIntval();

        r_n = new RoadNav();
        r_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        r_n->GetAdjNodeRG(rg, nodeid);

        local.setAddr(r_n);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          r_n = (RoadNav*)local.addr;
          if(r_n->count == r_n->jun_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(r_n->resulttype);

          tuple->PutAttribute(0, new GPoint(r_n->jun_list1[r_n->count]));
          tuple->PutAttribute(1, new GPoint(r_n->jun_list2[r_n->count]));
          tuple->PutAttribute(2, new GLine(r_n->gline_list[r_n->count]));
          tuple->PutAttribute(3, new CcInt(true, r_n->type_list[r_n->count]));

          result.setAddr(tuple);
          r_n->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            r_n = (RoadNav*)local.addr;
            delete r_n;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}



/*
navigation system in the bus network. 
it provides two kinds of shortest path from one bus stop to another:
1) length; 2) time 

*/
int OpTMBNNavigationValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  BNNav* bn_nav;

  switch(message){
      case OPEN:{
        Bus_Stop* bs1 = (Bus_Stop*)args[0].addr;
        Bus_Stop* bs2 = (Bus_Stop*)args[1].addr;
        BusNetwork* bn = (BusNetwork*)args[2].addr;
        Instant* query_time = (Instant*)args[3].addr;
        int type = ((CcInt*)args[4].addr)->GetIntval();

        bn_nav = new BNNav(bn);
        bn_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        bn_nav->type = type; 
        switch(type){
          case 0: 
                  bn_nav->ShortestPath_Length(bs1, bs2, query_time);
                  break;
          case 1: 
//                  bn_nav->ShortestPath_Time(bs1, bs2, query_time);
                  bn_nav->ShortestPath_TimeNew(bs1, bs2, query_time);
                  break;
          case 2:
//                bn_nav->ShortestPath_Transfer(bs1, bs2, query_time);
                  bn_nav->ShortestPath_TransferNew(bs1, bs2, query_time);
                  break;
          default:
                  cout<<"invalid type "<<type<<endl;
                  break;
        }

        local.setAddr(bn_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          bn_nav = (BNNav*)local.addr;
          if(bn_nav->type == 0){
             if(bn_nav->count == bn_nav->path_list.size())
                          return CANCEL;
              Tuple* tuple = new Tuple(bn_nav->resulttype);
              tuple->PutAttribute(0,
                        new SimpleLine(bn_nav->path_list[bn_nav->count]));
              tuple->PutAttribute(1,
                              new CcString(bn_nav->tm_list[bn_nav->count]));
              tuple->PutAttribute(2,
                              new CcString(bn_nav->bs1_list[bn_nav->count]));
              tuple->PutAttribute(3,
                              new CcString(bn_nav->bs2_list[bn_nav->count]));

              result.setAddr(tuple);
              bn_nav->count++;
              return YIELD;
          }else if(bn_nav->type == 1 || bn_nav->type == 3){
              if(bn_nav->count == bn_nav->path_list.size())
                          return CANCEL;
              Tuple* tuple = new Tuple(bn_nav->resulttype);
              tuple->PutAttribute(0,
                        new SimpleLine(bn_nav->path_list[bn_nav->count]));
              tuple->PutAttribute(1,
                              new CcString(bn_nav->tm_list[bn_nav->count]));
              tuple->PutAttribute(2,
                              new CcString(bn_nav->bs1_list[bn_nav->count]));
              tuple->PutAttribute(3,
                              new CcString(bn_nav->bs2_list[bn_nav->count]));
              tuple->PutAttribute(4,
                              new Periods(bn_nav->peri_list[bn_nav->count]));
              tuple->PutAttribute(5,
                       new CcReal(true,bn_nav->time_cost_list[bn_nav->count]));

              result.setAddr(tuple);
              bn_nav->count++;
              return YIELD;
          }else if(bn_nav->type == 2){
              if(bn_nav->count == bn_nav->path_list.size())
                          return CANCEL;
              Tuple* tuple = new Tuple(bn_nav->resulttype);
              tuple->PutAttribute(0,
                        new SimpleLine(bn_nav->path_list[bn_nav->count]));
              tuple->PutAttribute(1,
                              new CcString(bn_nav->tm_list[bn_nav->count]));
              tuple->PutAttribute(2,
                              new CcString(bn_nav->bs1_list[bn_nav->count]));
              tuple->PutAttribute(3,
                              new CcString(bn_nav->bs2_list[bn_nav->count]));
              tuple->PutAttribute(4,
                              new Periods(bn_nav->peri_list[bn_nav->count]));
              tuple->PutAttribute(5,
                       new CcReal(true,bn_nav->time_cost_list[bn_nav->count]));

              result.setAddr(tuple);
              bn_nav->count++;
              return YIELD;

          }else assert(false);
      }
      case CLOSE:{
          if(local.addr){
            bn_nav = (BNNav*)local.addr;
            delete bn_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}



/*
test the operator bnnavigation 

*/
int OpTMTestBNNavigationValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  Relation* rel1 = (Relation*)args[0].addr;
  Relation* rel2 = (Relation*)args[1].addr;
  BusNetwork* bn = (BusNetwork*)args[2].addr;
  Instant* query_time = (Instant*)args[3].addr;
  int type = ((CcInt*)args[4].addr)->GetIntval();

  BNNav* bn_nav = new BNNav(bn);
  bn_nav->type = type; 

  for(int i = 1;i <= rel1->GetNoTuples();i++){
    Tuple* bs1_tuple = rel1->GetTuple(i, false); 
    Bus_Stop* bs1 = (Bus_Stop*)bs1_tuple->GetAttribute(BusGraph::BG_NODE);

    for(int j = 1;j <= rel2->GetNoTuples();j++){
      if(i == j) continue; 
      clock_t start, finish;// the total CPU time 
      start = clock(); //the total CPU time

      struct timeb t1;
      struct timeb t2;
      ftime(&t1);

      Tuple* bs2_tuple = rel2->GetTuple(j, false); 
      Bus_Stop* bs2 = (Bus_Stop*)bs2_tuple->GetAttribute(BusGraph::BG_NODE);
      switch(type){
          case 0:
                  bn_nav->ShortestPath_Length(bs1, bs2, query_time);
                  if(bn_nav->path_list.size() > 0){
                    double l = 0.0;;
                    for(unsigned int k = 0;k < bn_nav->path_list.size();k++)
                      l += bn_nav->path_list[k].Length();
                    cout<<"bs1: "<<*bs1<<" bs2: "<<*bs2
                       <<" length "<<l<<endl;
                    bn_nav->path_list.clear();
                    bn_nav->tm_list.clear();
                    bn_nav->bs1_list.clear();
                    bn_nav->bs2_list.clear();
                  }
                  break;
          case 1:
                ////////////////no optimization on edges filtering///////////
//                  bn_nav->ShortestPath_Time(bs1, bs2, query_time);
                  ////////////////filtering edges////////////////////////////
                   ////////////////record searching time//////////////////////
                  bn_nav->ShortestPath_TimeNew(bs1, bs2, query_time);
                  if(bn_nav->path_list.size() > 0){
                    double l = 0.0;
                    double time_cost = 0.0;
                    for(unsigned int k = 0;k < bn_nav->path_list.size();k++){
                      l += bn_nav->path_list[k].Length();
                      time_cost += bn_nav->time_cost_list[k];
                    }
                    cout<<"bs1: "<<*bs1<<" bs2: "<<*bs2
                       <<" length: "<<l<<" time cost: "<<time_cost<<" s"<<endl;
                    bn_nav->path_list.clear();
                    bn_nav->tm_list.clear();
                    bn_nav->bs1_list.clear();
                    bn_nav->bs2_list.clear();
                    bn_nav->peri_list.clear(); 
                    bn_nav->time_cost_list.clear();
                  }

                  break;
          case 2:
                  ///////////////no optimization on edges filtering//////////
//                  bn_nav->ShortestPath_Transfer(bs1, bs2, query_time);
                  /////////////////filtering edges///////////////////////////
                  bn_nav->ShortestPath_TransferNew(bs1, bs2, query_time);
                  if(bn_nav->path_list.size() > 0){
                    double l = 0.0;
                    double time_cost = 0.0;
                    for(unsigned int k = 0;k < bn_nav->path_list.size();k++){
                      l += bn_nav->path_list[k].Length();
                      time_cost += bn_nav->time_cost_list[k];
                    }
                    cout<<"bs1: "<<*bs1<<" bs2: "<<*bs2
                        <<" length "<<l<<" time cost "<<time_cost<<" s"<<endl;

                    bn_nav->path_list.clear();
                    bn_nav->tm_list.clear();
                    bn_nav->bs1_list.clear();
                    bn_nav->bs2_list.clear();
                    bn_nav->peri_list.clear(); 
                    bn_nav->time_cost_list.clear();
                  }
                  break;
          default:
                  cout<<"invalid type "<<type<<endl;
                  break; 
      }
      bs2_tuple->DeleteIfAllowed();
      finish = clock();
      ftime(&t2);

      printf("CPU time :%.3f seconds: Real time: %.3f\n\n", 
            (double)(finish - start)/CLOCKS_PER_SEC, TM_DiffTimeb(&t2, &t1));
    }
    bs1_tuple->DeleteIfAllowed(); 
  }

  delete bn_nav;

  result = qp->ResultStorage(in_pSupplier);
  ((CcBool*)result.addr)->Set(true, true);
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
create time table for each spatial location--Bus 

*/
int OpTMCreateTimeTable1ValueMap ( Word* args, Word& result, int message,
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

/*
create time table for each spatial location--Bus 
Compact storage:
loc:point lineid:int stopid:int direction:bool deftime:periods
locid:int scheduleinterval:double 

*/
int OpTMCreateTimeTable1NewValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  RoadDenstiy* rd;
  switch(message){
      case OPEN:{

        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 
        Periods* peri1 = (Periods*)args[3].addr;
        Periods* peri2 = (Periods*)args[4].addr;


        rd = new RoadDenstiy(NULL,r1,r2, btree);
        rd->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        rd->CreateTimeTable_Compact(peri1, peri2);
        local.setAddr(rd);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          rd = (RoadDenstiy*)local.addr;
          if(rd->count == rd->bus_stop_loc.size())return CANCEL;

          Tuple* tuple = new Tuple(rd->resulttype);

          tuple->PutAttribute(0, new Point(rd->bus_stop_loc[rd->count]));
          tuple->PutAttribute(1, new Bus_Stop(rd->bs_list[rd->count]));
          tuple->PutAttribute(2, new Periods(rd->duration1[rd->count]));
          tuple->PutAttribute(3,
                         new CcReal(true,rd->schedule_interval[rd->count]));
          tuple->PutAttribute(4, 
                         new CcInt(true, rd->unique_id_list[rd->count]));
          tuple->PutAttribute(5, 
                         new CcInt(true, rd->bs_uoid_list[rd->count]));

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
create time table for each spatial location--Train 

*/
int OpTMCreateTimeTable2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  UBTrain* ubtrain;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 

        ubtrain = new UBTrain(r1,r2, btree);
        ubtrain->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ubtrain->CreateTimeTable();
        local.setAddr(ubtrain);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ubtrain = (UBTrain*)local.addr;
          if(ubtrain->count == ubtrain->line_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ubtrain->resulttype);
          tuple->PutAttribute(0, 
                             new Point(ubtrain->stop_loc_list[ubtrain->count]));
          tuple->PutAttribute(1, 
                    new CcInt(true, ubtrain->line_id_list[ubtrain->count]));
          tuple->PutAttribute(2, 
                    new CcInt(true, ubtrain->stop_id_list[ubtrain->count]));
          tuple->PutAttribute(3, 
                    new CcBool(true, ubtrain->direction_list[ubtrain->count]));
          tuple->PutAttribute(4, 
                    new Instant(ubtrain->schedule_time[ubtrain->count])); 
          tuple->PutAttribute(5, 
                    new CcInt(true, ubtrain->loc_id_list[ubtrain->count])); 

          result.setAddr(tuple);
          ubtrain->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ubtrain = (UBTrain*)local.addr;
            delete ubtrain;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}


/*
create time table for each spatial location--Train (Compact Storage)
loc:point lineid:int stopid:int direction:bool deftime:periods
locid:int scheduleinterval:double 

*/
int OpTMCreateTimeTable2NewValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  UBTrain* ubtrain;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr; 
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 

        ubtrain = new UBTrain(r1,r2, btree);
        ubtrain->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ubtrain->CreateTimeTable_Compact();
        local.setAddr(ubtrain);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ubtrain = (UBTrain*)local.addr;
          if(ubtrain->count == ubtrain->line_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ubtrain->resulttype);
          tuple->PutAttribute(0, 
                             new Point(ubtrain->stop_loc_list[ubtrain->count]));
          tuple->PutAttribute(1, 
                    new CcInt(true, ubtrain->line_id_list[ubtrain->count]));
          tuple->PutAttribute(2, 
                    new CcInt(true, ubtrain->stop_id_list[ubtrain->count]));
          tuple->PutAttribute(3, 
                    new CcBool(true, ubtrain->direction_list[ubtrain->count]));
          tuple->PutAttribute(4, 
                    new Periods(ubtrain->duration[ubtrain->count])); 
          tuple->PutAttribute(5, 
                  new CcReal(true, ubtrain->schedule_interval[ubtrain->count]));
          tuple->PutAttribute(6, 
                    new CcInt(true, ubtrain->loc_id_list[ubtrain->count])); 

          result.setAddr(tuple);
          ubtrain->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ubtrain = (UBTrain*)local.addr;
            delete ubtrain;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}




/*
convert trains, buses to generic moving objects 

*/
int OpTMRefMO2GenMOValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
      BNNav* bn_nav;
      switch(message){
        case OPEN:{

        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 

        bn_nav = new BNNav(NULL);
        bn_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        bn_nav->BusToGenMO(r1, r2, btree);
        local.setAddr(bn_nav);
        return 0;
        }
        case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          bn_nav = (BNNav*)local.addr;
          if(bn_nav->count == bn_nav->genmo_list.size())return CANCEL;

          Tuple* tuple = new Tuple(bn_nav->resulttype);
          tuple->PutAttribute(0,
                       new GenMO(bn_nav->genmo_list[bn_nav->count]));
          tuple->PutAttribute(1,
                       new MPoint(bn_nav->mp_list[bn_nav->count]));
          tuple->PutAttribute(2,
                       new CcInt(true, bn_nav->br_id_list[bn_nav->count]));

          result.setAddr(tuple);
          bn_nav->count++;
          return YIELD;
        }
        case CLOSE:{
          if(local.addr){
            bn_nav = (BNNav*)local.addr;
            delete bn_nav;
            local.setAddr(Address(0));
          }
          return 0;
        }
    }
  return 0;
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

/*
create the metro network infrastructure 

*/
int OpTMTheMetroNetworkValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroNetwork* mn = (MetroNetwork*)qp->ResultStorage(in_pSupplier).addr;
  int id = ((CcInt*)args[0].addr)->GetIntval(); 
  if(ChekMetroNetworkId(id)){
    Relation* stops = (Relation*)args[1].addr;
    Relation* routes = (Relation*)args[2].addr;
    Relation* metros = (Relation*)args[3].addr;
    mn->Load(id, stops, routes, metros);
    result = SetWord(mn); 
  }else{
    cout<<"invalid metro network id "<<id<<endl; 
    while(ChekMetroNetworkId(id) == false || id <= 0){
      id++;
    }
    cout<<"new metro network id "<<id<<endl; 
    Relation* stops = (Relation*)args[1].addr;
    Relation* routes = (Relation*)args[2].addr; 
    Relation* metros = (Relation*)args[3].addr; 
    mn->Load(id, stops, routes, metros);
    result = SetWord(mn); 
  }
  return 0;
}

/*
create the metro graph edges where two metro stops have the same spatial
location in space

*/
int OpTMMSNeighbor1ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroStruct* ub_train;
  switch(message){
      case OPEN:{
        Relation* ms_stops = (Relation*)args[0].addr;

        ub_train = new MetroStruct();
        ub_train->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ub_train->MsNeighbors1(ms_stops);
        local.setAddr(ub_train);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ub_train = (MetroStruct*)local.addr;
          if(ub_train->count == ub_train->ms_tid_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(ub_train->resulttype);

         tuple->PutAttribute(0, 
                     new CcInt(true, ub_train->ms_tid_list1[ub_train->count]));
         tuple->PutAttribute(1, 
                     new CcInt(true, ub_train->ms_tid_list2[ub_train->count]));

          result.setAddr(tuple);
          ub_train->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ub_train = (MetroStruct*)local.addr;
            delete ub_train;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
create the metro graph edges where two metro stops are connected by moving
metros

*/
int OpTMMSNeighbor2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroStruct* ub_train;
  switch(message){
      case OPEN:{
        MetroNetwork* mn = (MetroNetwork*)args[0].addr;
        Relation* timetable = (Relation*)args[1].addr;
        BTree* btree1 = (BTree*)args[2].addr;
        Relation* metrotrip = (Relation*)args[3].addr;
        BTree* btree2 = (BTree*)args[4].addr;

        ub_train = new MetroStruct();
        ub_train->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ub_train->MsNeighbors2(mn, timetable, btree1, metrotrip, btree2);
        local.setAddr(ub_train);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ub_train = (MetroStruct*)local.addr;
          if(ub_train->count == ub_train->ms_tid_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(ub_train->resulttype);

         tuple->PutAttribute(0, 
                     new CcInt(true, ub_train->ms_tid_list1[ub_train->count]));
         tuple->PutAttribute(1, 
                     new CcInt(true, ub_train->ms_tid_list2[ub_train->count]));
         tuple->PutAttribute(2,
                     new Periods(ub_train->period_list[ub_train->count]));
         tuple->PutAttribute(3,
                new CcReal(true, ub_train->schedule_interval[ub_train->count]));
         tuple->PutAttribute(4,
                     new SimpleLine(ub_train->geodata[ub_train->count]));
         tuple->PutAttribute(5,
                   new CcReal(true,ub_train->time_cost_list[ub_train->count]));

          result.setAddr(tuple);
          ub_train->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ub_train = (MetroStruct*)local.addr;
            delete ub_train;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
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
value map for operator createmgraph

*/

int OpTMCreateMetroGraphValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroGraph* mg = (MetroGraph*)qp->ResultStorage(in_pSupplier).addr;
  int mg_id = ((CcInt*)args[0].addr)->GetIntval();
  
  if(CheckMetroGraphId(mg_id)){
    Relation* node_rel = (Relation*)args[1].addr;
    Relation* edge_rel1 = (Relation*)args[2].addr;
    Relation* edge_rel2 = (Relation*)args[3].addr; 

    mg->Load(mg_id, node_rel, edge_rel1, edge_rel2);
    result = SetWord(mg);
  }else{
    cout<<"invalid metro graph id "<<mg_id<<endl; 
    while(CheckMetroGraphId(mg_id) == false || mg_id <= 0){
      mg_id++;
    }
    cout<<"new metro graph id "<<mg_id<<endl; 
    Relation* node_rel = (Relation*)args[1].addr;
    Relation* edge_rel1 = (Relation*)args[2].addr;
    Relation* edge_rel2 = (Relation*)args[3].addr; 

    mg->Load(mg_id, node_rel, edge_rel1, edge_rel2);
    result = SetWord(mg);
  }
  return 0;
}



/*
value map for operator createmetroroute

*/
int OpTMCreateMetroRouteValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroStruct* ub_train;
  switch(message){
      case OPEN:{
        DualGraph* dg = (DualGraph*)args[0].addr;
        string type = ((CcString*)args[1].addr)->GetValue();

        ub_train = new MetroStruct();
        ub_train->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ub_train->CreateMRoute(dg, type);
        local.setAddr(ub_train);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ub_train = (MetroStruct*)local.addr;
          if(ub_train->count == ub_train->id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ub_train->resulttype);

         tuple->PutAttribute(0, 
                     new CcInt(true, ub_train->id_list[ub_train->count]));
         tuple->PutAttribute(1, 
                     new Bus_Route(ub_train->mroute_list[ub_train->count]));
//         tuple->PutAttribute(2, 
//                     new Region(ub_train->cell_reg_list1[ub_train->count]));
//         tuple->PutAttribute(3, 
//                     new Region(ub_train->cell_reg_list2[ub_train->count]));

          result.setAddr(tuple);
          ub_train->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ub_train = (MetroStruct*)local.addr;
            delete ub_train;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
value map for operator createmetrostop

*/
int OpTMCreateMetroStopValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroStruct* ub_train;
  switch(message){
      case OPEN:{
        Relation* rel = (Relation*)args[0].addr;

        ub_train = new MetroStruct();
        ub_train->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ub_train->CreateMStop(rel);
        local.setAddr(ub_train);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ub_train = (MetroStruct*)local.addr;
          if(ub_train->count == ub_train->mstop_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ub_train->resulttype);

         tuple->PutAttribute(0, 
                     new Bus_Stop(ub_train->mstop_list[ub_train->count]));
         tuple->PutAttribute(1, 
                     new Point(ub_train->stop_geo_list[ub_train->count]));
         tuple->PutAttribute(2, 
                     new CcInt(true, ub_train->id_list[ub_train->count]));

          result.setAddr(tuple);
          ub_train->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ub_train = (MetroStruct*)local.addr;
            delete ub_train;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
value map for operator createmetromo

*/
int OpTMCreateMetroMOValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroStruct* ub_train;
  switch(message){
      case OPEN:{
        Relation* rel = (Relation*)args[0].addr;
        Periods* peri = (Periods*)args[1].addr; 

        ub_train = new MetroStruct();
        ub_train->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ub_train->CreateMTrips(rel, peri);
        local.setAddr(ub_train);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ub_train = (MetroStruct*)local.addr;
          if(ub_train->count == ub_train->mtrip_list1.size())return CANCEL;

          Tuple* tuple = new Tuple(ub_train->resulttype);

          tuple->PutAttribute(0, 
                     new GenMO(ub_train->mtrip_list1[ub_train->count]));
          tuple->PutAttribute(1, 
                     new MPoint(ub_train->mtrip_list2[ub_train->count]));
          tuple->PutAttribute(2, 
                     new CcInt(true, ub_train->id_list[ub_train->count]));
          tuple->PutAttribute(3, 
                     new CcBool(true, ub_train->dir_list[ub_train->count]));
          tuple->PutAttribute(4, 
                     new CcInt(true, ub_train->mr_oid_list[ub_train->count]));

          result.setAddr(tuple);
          ub_train->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ub_train = (MetroStruct*)local.addr;
            delete ub_train;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}

/*
value map for operator mapmstopave

*/
int OpTMMapMsToPaveValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MetroStruct* ub_train;
  switch(message){
      case OPEN:{
        Relation* rel1 = (Relation*)args[0].addr;
        Relation* rel2 = (Relation*)args[1].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[2].addr; 

        ub_train = new MetroStruct();
        ub_train->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ub_train->MapMSToPave(rel1, rel2, rtree);
        local.setAddr(ub_train);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ub_train = (MetroStruct*)local.addr;
         if(ub_train->count == ub_train->loc_list1.size())return CANCEL;
//         if(ub_train->count == ub_train->neighbor_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ub_train->resulttype);

         tuple->PutAttribute(0, 
                     new GenLoc(ub_train->loc_list1[ub_train->count]));
         tuple->PutAttribute(1, 
                     new Point(ub_train->loc_list2[ub_train->count]));
         tuple->PutAttribute(2, 
                     new Bus_Stop(ub_train->mstop_list[ub_train->count]));
         tuple->PutAttribute(3, 
                     new Point(ub_train->stop_geo_list[ub_train->count]));

//          tuple->PutAttribute(0, 
//                     new CcInt(true,ub_train->id_list[ub_train->count]));
//          tuple->PutAttribute(1, 
//                       new Region(ub_train->neighbor_list[ub_train->count]));
//          tuple->PutAttribute(2, 
//                       new Point(ub_train->loc_list2[ub_train->count]));


          result.setAddr(tuple);
          ub_train->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            ub_train = (MetroStruct*)local.addr;
            delete ub_train;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
navigation system in the metro network. 
it provides two kinds of shortest path from one bus stop to another by time

*/
int OpTMMNNavigationValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier in_pSupplier)
{
  MNNav* mn_nav;

  switch(message){
      case OPEN:{
        Bus_Stop* ms1 = (Bus_Stop*)args[0].addr;
        Bus_Stop* ms2 = (Bus_Stop*)args[1].addr;
        MetroNetwork* mn = (MetroNetwork*)args[2].addr;
        Instant* query_time = (Instant*)args[3].addr;

        mn_nav = new MNNav(mn);
        mn_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        mn_nav->ShortestPath_Time(ms1, ms2, query_time);

        local.setAddr(mn_nav);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          mn_nav = (MNNav*)local.addr;

          if(mn_nav->count == mn_nav->path_list.size()) return CANCEL;

          Tuple* tuple = new Tuple(mn_nav->resulttype);
          tuple->PutAttribute(0,
                        new SimpleLine(mn_nav->path_list[mn_nav->count]));
          tuple->PutAttribute(1,
                              new CcString(mn_nav->tm_list[mn_nav->count]));
          tuple->PutAttribute(2,
                              new CcString(mn_nav->ms1_list[mn_nav->count]));
          tuple->PutAttribute(3,
                              new CcString(mn_nav->ms2_list[mn_nav->count]));
          tuple->PutAttribute(4,
                              new Periods(mn_nav->peri_list[mn_nav->count]));
          tuple->PutAttribute(5,
                       new CcReal(true,mn_nav->time_cost_list[mn_nav->count]));

          result.setAddr(tuple);
          mn_nav->count++;
          return YIELD;

      }
      case CLOSE:{
          if(local.addr){
            mn_nav = (MNNav*)local.addr;
            delete mn_nav;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
}


/*
get the day of an instant 

*/
int OpTMInstant2DayValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    result = qp->ResultStorage(in_pSupplier);
    DateTime* t = (DateTime*)args[0].addr;
    ((CcInt*)result.addr)->Set(true,t->GetDay());
    return 0; 
}


/*
get the maximum rectangle from a convex region 

*/
int OpTMMaxRectValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    result = qp->ResultStorage(in_pSupplier);
    Region* reg = (Region*)args[0].addr;
    Rectangle<2>* bbox = static_cast<Rectangle<2>*>(result.addr);
//    Region* bbox = static_cast<Region*>(result.addr); 
    if(reg->IsDefined())
      *bbox = GetMaxRect(reg); 
    else
      bbox->SetDefined(false); 
    return 0;
}


/*
get the maximum rectangle from the region relations 

*/
int OpTMGetRect1ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MaxRect* max_rect;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        string type = ((CcString*)args[3].addr)->GetValue();

        int attr1 = ((CcInt*)args[4].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[5].addr)->GetIntval() - 1; 

        max_rect = new MaxRect(r);
        max_rect->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        max_rect->GetRectangle1(attr1, attr2, type);
        local.setAddr(max_rect);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          max_rect = (MaxRect*)local.addr;
          if(max_rect->count == max_rect->reg_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(max_rect->resulttype);
          tuple->PutAttribute(0, 
                       new CcInt(true,max_rect->reg_id_list[max_rect->count]));
          tuple->PutAttribute(1, 
                     new Rectangle<2>(max_rect->rect_list[max_rect->count]));
          tuple->PutAttribute(2, 
                       new CcInt(true,max_rect->poly_id_list[max_rect->count]));
          
          result.setAddr(tuple);
          max_rect->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            max_rect = (MaxRect*)local.addr;
            delete max_rect;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}


/*
build the connection between the entrance of the building and the pavement 
set the building id and map the point in the pavement area

*/
int OpTMPathToBuildingValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MaxRect* max_rect;
  switch(message){
      case OPEN:{

        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr;
        BTree* btree = (BTree*)args[2].addr; 
        Space* sp = (Space*)args[3].addr; 

        max_rect = new MaxRect(r1, r2, btree);
        max_rect->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        max_rect->PathToBuilding(sp);

        local.setAddr(max_rect);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          max_rect = (MaxRect*)local.addr;
          if(max_rect->count == max_rect->reg_id_list.size()) return CANCEL;

          Tuple* tuple = new Tuple(max_rect->resulttype);

          tuple->PutAttribute(0,
                       new CcInt(true,max_rect->reg_id_list[max_rect->count]));
          tuple->PutAttribute(1,
                       new Point(max_rect->sp_list[max_rect->count]));
          tuple->PutAttribute(2,
                     new CcInt(true, max_rect->sp_index_list[max_rect->count]));
          tuple->PutAttribute(3,
                       new Point(max_rect->ep_list[max_rect->count]));
          tuple->PutAttribute(4, 
                        new Point(max_rect->ep_list2[max_rect->count]));
          tuple->PutAttribute(5, 
                        new GenLoc(max_rect->genloc_list[max_rect->count]));
          tuple->PutAttribute(6, 
                        new Line(max_rect->path_list[max_rect->count]));

           result.setAddr(tuple);

          max_rect->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            max_rect = (MaxRect*)local.addr;
            delete max_rect;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
for each rectangle, it sets which kind of building it is

*/
int OpTMSetBuildingTypeValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MaxRect* max_rect;
  switch(message){
      case OPEN:{

        Relation* r1 = (Relation*)args[0].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[1].addr;
        Space* sp = (Space*)args[2].addr; 

        max_rect = new MaxRect(r1, NULL, NULL);
        max_rect->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        max_rect->SetBuildingType(rtree, sp);
        local.setAddr(max_rect);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          max_rect = (MaxRect*)local.addr;
          if(max_rect->count == max_rect->reg_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(max_rect->resulttype);
          tuple->PutAttribute(0, 
                       new CcInt(true,max_rect->reg_id_list[max_rect->count]));
          tuple->PutAttribute(1, 
                       new Rectangle<2>(max_rect->rect_list[max_rect->count]));
          tuple->PutAttribute(2, 
                       new CcInt(true,max_rect->poly_id_list[max_rect->count]));
          tuple->PutAttribute(3, 
                      new CcInt(true,max_rect->reg_type_list[max_rect->count]));
          tuple->PutAttribute(4,
                   new CcInt(true, max_rect->build_type_list[max_rect->count]));
          tuple->PutAttribute(5,
               new CcString(true, max_rect->build_type2_list[max_rect->count]));
          tuple->PutAttribute(6,
               new CcInt(true, max_rect->build_id_list[max_rect->count]));

          result.setAddr(tuple);
          max_rect->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            max_rect = (MaxRect*)local.addr;
            delete max_rect;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;

}

/*
clear dirty region data. remove dirty region data

*/
int OpTMRemoveDirtyValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MaxRect* max_rect;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        int attr1 = ((CcInt*)args[3].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[4].addr)->GetIntval() - 1; 

        max_rect = new MaxRect(r);
        max_rect->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        max_rect->RemoveDirty(attr1, attr2);
        local.setAddr(max_rect);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          max_rect = (MaxRect*)local.addr;
          if(max_rect->count == max_rect->reg_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(max_rect->resulttype);
          tuple->PutAttribute(0, 
                       new CcInt(true,max_rect->reg_id_list[max_rect->count]));
//          tuple->PutAttribute(1, 
//                         new SimpleLine(max_rect->sl_list[max_rect->count]));
          tuple->PutAttribute(1, 
                         new Region(max_rect->reg_list[max_rect->count]));

          result.setAddr(tuple);
          max_rect->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            max_rect = (MaxRect*)local.addr;
            delete max_rect;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  return 0;
  
}


/*
given a line value, modify its coordinates value. not so many numbers after 
dot.
for example (2.345567, 2.33444) (2.34,2.33)

*/

int OpTMModifyLinemap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  SimpleLine* l = (SimpleLine*)args[0].addr;

  result = qp->ResultStorage( in_pSupplier );
  SimpleLine *pResult = (SimpleLine *)result.addr;


  DataClean* datacl = new DataClean(); 
  datacl->ModifyLine(l, pResult);
  delete datacl;

  return 0;
}

/*
check roads lines points, wether there  are overlapping 

*/
int OpTMCheckRoadsmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{


  DataClean* datacl;
  switch(message){
      case OPEN:{

        Relation* rel = (Relation*)args[0].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[1].addr; 

        datacl = new DataClean(); 
        datacl->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        datacl->CheckRoads(rel, rtree);
        local.setAddr(datacl);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          datacl = (DataClean*)local.addr;
          if(datacl->count == datacl->sl_list.size())return CANCEL;

          Tuple* tuple = new Tuple(datacl->resulttype);
          tuple->PutAttribute(0, new CcInt(true, datacl->count + 1));
          tuple->PutAttribute(1, 
                    new CcReal(true, datacl->sl_list[datacl->count].Length()));
          tuple->PutAttribute(2,
                        new SimpleLine(datacl->sl_list[datacl->count]));
          tuple->PutAttribute(3, new CcBool(true, true));
          tuple->PutAttribute(4, new CcBool(true, true));

          result.setAddr(tuple);
          datacl->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            datacl = (DataClean*)local.addr;
            delete datacl;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  
  return 0;
}


/*
join: roads and cell box

*/
int OpTMTMJoin1map ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  TM_Join* tm_join;
  switch(message){
      case OPEN:{

        Relation* rel1 = (Relation*)args[0].addr;
        Relation* rel2 = (Relation*)args[1].addr;
        R_Tree<2,TupleId>* rtree = (R_Tree<2,TupleId>*)args[2].addr;

        tm_join = new TM_Join(); 
        tm_join->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        tm_join->Road_Cell_Join(rel1,rel2, rtree);
        local.setAddr(tm_join);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          tm_join = (TM_Join*)local.addr;
          if(tm_join->count == tm_join->id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(tm_join->resulttype);
          tuple->PutAttribute(0, 
                            new CcInt(true, tm_join->sec_list[tm_join->count]));
          tuple->PutAttribute(1, 
                             new CcInt(true, tm_join->id_list[tm_join->count]));
          tuple->PutAttribute(2, 
                      new CcInt(true, tm_join->count_list[tm_join->count]));
          tuple->PutAttribute(3, 
                      new Region(tm_join->area_list[tm_join->count]));

          result.setAddr(tuple);
          tm_join->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            tm_join = (TM_Join*)local.addr;
            delete tm_join;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }
  
  return 0;
}

/*
for each bus stop, find the pavement areas neart to the bus stop or metro stop

*/
int OpNearStopPaveMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  TM_Join* tm_join;
  switch(message){
      case OPEN:{

        Space* sp = (Space*)args[0].addr;
        string type = ((CcString*)args[1].addr)->GetValue();

        tm_join = new TM_Join(); 
        tm_join->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        tm_join->NearStopPave(sp,type);
        local.setAddr(tm_join);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          tm_join = (TM_Join*)local.addr;
          if(tm_join->count == tm_join->id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(tm_join->resulttype);
          tuple->PutAttribute(0, 
                             new CcInt(true, tm_join->id_list[tm_join->count]));
          tuple->PutAttribute(1, 
                            new CcInt(true, tm_join->rid_list[tm_join->count]));
          tuple->PutAttribute(2, 
                      new Region(tm_join->area_list[tm_join->count]));

          result.setAddr(tuple);
          tm_join->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            tm_join = (TM_Join*)local.addr;
            delete tm_join;
            local.setAddr(Address(0));
          }
          return 0;
      }
  }

  return 0;
}


/*
for each bus stop, find the buildings nearest to the bus stop or metro stop

*/
int OpNearStopBuildingMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  TM_Join* tm_join;
  switch(message){
      case OPEN:{

        Space* sp = (Space*)args[0].addr;
        string type = ((CcString*)args[1].addr)->GetValue();

        tm_join = new TM_Join(); 
        tm_join->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        tm_join->NearStopBuilding(sp,type);
        local.setAddr(tm_join);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          tm_join = (TM_Join*)local.addr;
          if(tm_join->count == tm_join->id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(tm_join->resulttype);
          tuple->PutAttribute(0, 
                             new CcInt(true, tm_join->id_list[tm_join->count]));
          tuple->PutAttribute(1, new CcString(true, 
                           GetBuildingStr(tm_join->type_list[tm_join->count])));
          tuple->PutAttribute(2, 
                      new Rectangle<2>(tm_join->rect_list[tm_join->count]));

          result.setAddr(tuple);
          tm_join->count++;
          return YIELD;
      }
      case CLOSE:{
          if(local.addr){
            tm_join = (TM_Join*)local.addr;
            delete tm_join;
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

Operator triangulation2(
    "triangulation2",               // name
    OpTMTriangulate2Spec,          // specification
    OpTMTriangulate2map,  // value mapping
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

Operator walk_sp_old(
    "walk_sp_old",
    OpTMWalkSPOldSpec,
    OpTMWalkSPOldValueMap,
    Operator::SimpleSelect,
    OpTMWalkSPOldTypeMap
);

Operator test_walk_sp(
    "test_walk_sp",
    OpTMTestWalkSPSpec,
    OpTMTestWalkSPValueMap,
    Operator::SimpleSelect,
    OpTMTestWalkSPTypeMap
);

Operator walk_sp(
    "walk_sp",
    OpTMWalkSPSpec,
    OpTMWalkSPValueMap,
    Operator::SimpleSelect,
    OpTMWalkSPTypeMap
);


Operator setpave_rid(
    "setpave_rid",
    OpTMSetPaveRidSpec,
    OpTMSetPaveRidValueMap,
    Operator::SimpleSelect,
    OpTMSetPaveRidTypeMap
);


Operator pave_loc_togp(
    "pave_loc_togp",
    OpTMPaveLocToGPSpec,
    OpTMPaveLocToGPValueMap,
    Operator::SimpleSelect,
    OpTMPaveLocToGPTypeMap
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

Operator triangulation_new2(
    "triangulation_new2",
    OpTMTriangulationNew2Spec,
    OpTMTriangulationNew2ValueMap,
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

Operator bus_route_road(
  "bus_route_road",
  OpTMBusRouteRoadSpec,
  OpTMBusRouteRoadValueMap,
  Operator::SimpleSelect,
  OpTMBusRouteRoadTypeMap
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

/*
for bus stops with data type busstop 

*/
Operator getbusstops(
  "getbusstops",
  OpTMGetBusStopsSpec,
  OpTMGetBusStopsValueMap,
  Operator::SimpleSelect,
  OpTMGetBusStopsTypeMap
);

Operator getbusroutes(
  "getbusroutes",
  OpTMGetBusRoutesSpec,
  OpTMGetBusRoutesValueMap,
  Operator::SimpleSelect,
  OpTMGetBusRoutesTypeMap
);


Operator brgeodata(
  "brgeodata",
  OpTMBRGeoDataSpec,
  OpTMBRGeoDataValueMap,
  Operator::SimpleSelect,
  OpTMBRGeoDataTypeMap
);


ValueMapping OpTMBSGeoDataVM[]=
{
  OpTMBSGeoData1ValueMap,
  OpTMBSGeoData2ValueMap,
};

int BSGeoDataSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busstop") && 
     nl->IsAtom(arg2) && nl->IsEqual(arg2, "busroute"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busstop") &&
     nl->IsAtom(arg2) && nl->IsEqual(arg2, "busnetwork"))
    return 1;

  return -1;
}

Operator bsgeodata(
  "bsgeodata",
  OpTMBSGeoDataSpec,
  2, 
  OpTMBSGeoDataVM,
  BSGeoDataSelect, 
  OpTMBSGeoDataTypeMap
);

Operator getstopid(
  "getstopid",
  OpTMGetStopIdSpec,
  OpTMGetStopIdValueMap,
  Operator::SimpleSelect,
  OpTMGetStopIdTypeMap
);

ValueMapping OpTMUpDownVM[]=
{
  OpTMBusStopUpDownValueMap,
  OpTMBusRouteUpDownValueMap
};

int UpDownSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busstop"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "busroute"))
    return 1;

  return -1;
}

Operator up_down(
  "up_down",
  OpTMUpDownSpec, 
  2,
  OpTMUpDownVM,
  UpDownSelect,
  OpTMUpDownTypeMap
);

/*
create the bus network 

*/

Operator thebusnetwork(
  "thebusnetwork",
  OpTMTheBusNetworkSpec,
  OpTMTheBusNetworkValueMap,
  Operator::SimpleSelect,
  OpTMTheBusNetworkTypeMap
);


Operator bn_busstops(
  "bn_busstops",
  OpTMBusStopsSpec,
  OpTMBusStopsValueMap,
  Operator::SimpleSelect,
  OpTMBusStopsTypeMap
);


Operator bn_busroutes(
  "bn_busroutes",
  OpTMBusRoutesSpec,
  OpTMBusRoutesValueMap,
  Operator::SimpleSelect,
  OpTMBusRoutesTypeMap
);

Operator mapbstopave(
  "mapbstopave", 
  OpTMMapBsToPaveSpec,
  OpTMMapBsToPaveValueMap,
  Operator::SimpleSelect,
  OpTMMapBsToPaveTypeMap
);

/*
create the region based outdoor infrastructure 

*/
Operator thepavement(
  "thepavement",
  OpTMThePavementSpec,
  OpTMThePavementValueMap,
  Operator::SimpleSelect,
  OpTMThePavementTypeMap
);


/*
build connections between bus stops: 1) neighbor connected by pavements; 
2) the same 2D point but belong to different bus routes 

*/

Operator bs_neighbors1(
  "bs_neighbors1", 
  OpTMBsNeighbors1Spec,
  OpTMBsNeighbor1ValueMap,
  Operator::SimpleSelect,
  OpTMBsNeighbors1TypeMap
);


Operator bs_neighbors2(
  "bs_neighbors2", 
  OpTMBsNeighbors2Spec,
  OpTMBsNeighbor2ValueMap,
  Operator::SimpleSelect,
  OpTMBsNeighbors2TypeMap
);

Operator bs_neighbors3(
  "bs_neighbors3", 
  OpTMBsNeighbors3Spec,
  OpTMBsNeighbor3ValueMap,
  Operator::SimpleSelect,
  OpTMBsNeighbors3TypeMap
);

/*
a graph on bus network including pavements connection 

*/
Operator createbgraph(
  "createbgraph", 
  OpTMCreateBusGraphSpec,
  OpTMCreateBusGraphValueMap,
  Operator::SimpleSelect,
  OpTMCreateBusGraphTypeMap
);

ValueMapping OpTMGetAdjNodeVM[]=
{
  OpTMGetAdjNodeVGValueMap,
  OpTMGetAdjNodeDGValueMap,
  OpTMGetAdjNodeBGIntValueMap,
  OpTMGetAdjNodeBGBSValueMap,
  OpTMGetAdjNodeIGValueMap,
  OpTMGetAdjNodeMGValueMap,
  OpTMGetAdjNodeRGValueMap
};

int GetAdjNodeSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->IsEqual(arg1, "visualgraph") && nl->IsEqual(arg2, "int"))
    return 0;
  if(nl->IsEqual(arg1, "dualgraph") && nl->IsEqual(arg2, "int"))
    return 1;
  if(nl->IsEqual(arg1, "busgraph") && nl->IsEqual(arg2, "int"))
    return 2;
  if(nl->IsEqual(arg1, "busgraph") && nl->IsEqual(arg2, "busstop"))
    return 3;
  if(nl->IsEqual(arg1, "indoorgraph") && nl->IsEqual(arg2, "int"))
    return 4;
  if(nl->IsEqual(arg1, "metrograph") && nl->IsEqual(arg2, "int"))
    return 5;
  if(nl->IsEqual(arg1, "roadgraph") && nl->IsEqual(arg2, "int"))
    return 6;
  return -1;
}

Operator getadjnode(
  "getadjnode", 
  OpTMGetAdjNodeSpec,
  7,
  OpTMGetAdjNodeVM,
  GetAdjNodeSelect,
  OpTMGetAdjNodeTypeMap
);

Operator bnnavigation(
  "bnnavigation", 
  OpTMBNNavigationSpec,
  OpTMBNNavigationValueMap,
  Operator::SimpleSelect,
  OpTMBNNavigationTypeMap
);

Operator test_bnnavigation(
  "test_bnnavigation", 
  OpTMTestBNNavigationSpec,
  OpTMTestBNNavigationValueMap,
  Operator::SimpleSelect,
  OpTMTestBNNavigationTypeMap
);


/*
the following are to create moving buses 

*/
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
  OpTMCreateMovingBusNightTypeMap
);

Operator create_daytime_bus_mo(
  "create_daytime_bus_mo",
  OpTMCreateDayTimeBusSpec,
  OpTMCreateDayTimeBusValueMap,
  Operator::SimpleSelect,
  OpTMCreateMovingBusDayTypeMap
);


Operator create_time_table1(
  "create_time_table1",
  OpTMCreateTimeTable1Spec,
  OpTMCreateTimeTable1ValueMap,
  Operator::SimpleSelect,
  OpTMCreateTimeTable1TypeMap
);


Operator create_time_table1_new(
  "create_time_table1_new",
  OpTMCreateTimeTable1NewSpec,
  OpTMCreateTimeTable1NewValueMap,
  Operator::SimpleSelect,
  OpTMCreateTimeTable1NewTypeMap
);


Operator create_time_table2(
  "create_time_table2",
  OpTMCreateTimeTable2Spec,
  OpTMCreateTimeTable2ValueMap,
  Operator::SimpleSelect,
  OpTMCreateTimeTable2TypeMap
);

Operator create_time_table2_new(
  "create_time_table2_new",
  OpTMCreateTimeTable2NewSpec,
  OpTMCreateTimeTable2NewValueMap,
  Operator::SimpleSelect,
  OpTMCreateTimeTable2NewTypeMap
);

Operator refmo2genmo(
  "refmo2genmo",
  OpTMRefMO2GenMOSpec,
  OpTMRefMO2GenMOValueMap,
  Operator::SimpleSelect,
  OpTMRefMO2GenMOTypeMap
); 


Operator themetronetwork(
  "themetronetwork",
  OpTMTheMetroNetworkSpec,
  OpTMTheMetroNetworkValueMap,
  Operator::SimpleSelect,
  OpTMTheMetroNetworkTypeMap
);

/*
one edges for metro graph where metro stops have the same spatial location 
and two metro stops are connected by moving metros

*/
Operator ms_neighbors1(
  "ms_neighbors1",
  OpTMMSNeighbor1Spec,
  OpTMMSNeighbor1ValueMap,
  Operator::SimpleSelect,
  OpTMMSNeighbor1TypeMap
);

Operator ms_neighbors2(
  "ms_neighbors2",
  OpTMMSNeighbor2Spec,
  OpTMMSNeighbor2ValueMap,
  Operator::SimpleSelect,
  OpTMMSNeighbor2TypeMap
);

Operator createmgraph(
  "createmgraph", 
  OpTMCreateMetroGraphSpec,
  OpTMCreateMetroGraphValueMap,
  Operator::SimpleSelect,
  OpTMCreateMetroGraphTypeMap
);

/*
create underground railways, stops and moving metros

*/
Operator createmetroroute(
  "createmetroroute", 
  OpTMCreateMetroRouteSpec,
  OpTMCreateMetroRouteValueMap,
  Operator::SimpleSelect,
  OpTMCreateMetroRouteTypeMap
);

Operator createmetrostop(
  "createmetrostop", 
  OpTMCreateMetroStopSpec,
  OpTMCreateMetroStopValueMap,
  Operator::SimpleSelect,
  OpTMCreateMetroStopTypeMap
);

Operator createmetromo(
  "createmetromo", 
  OpTMCreateMetroMOSpec,
  OpTMCreateMetroMOValueMap,
  Operator::SimpleSelect,
  OpTMCreateMetroMOTypeMap
);

Operator mapmstopave(
  "mapmstopave", 
  OpTMMapMsToPaveSpec,
  OpTMMapMsToPaveValueMap,
  Operator::SimpleSelect,
  OpTMMapMsToPaveTypeMap
);


Operator mnnavigation(
  "mnnavigation",
  OpTMMNNavigationSpec,
  OpTMMNNavigationValueMap,
  Operator::SimpleSelect,
  OpTMMNNavigationTypeMap
);

/////////////////////////////////////////////////////////////////////////////
/////////////////// auxiliary operators////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

Operator instant2day(
  "instant2day",
  OpTMInstant2DaySpec,
  OpTMInstant2DayValueMap,
  Operator::SimpleSelect,
  OpTMInstant2DayNewTypeMap
);

Operator maxrect(
  "maxrect",  //name 
  OpTMMaxRectSpec, //specification
  OpTMMaxRectValueMap, //value mapping 
  Operator::SimpleSelect, 
  OpTMMaxRectTypeMap //type mapping 
);

Operator remove_dirty(
  "remove_dirty",   // name
  OpTMRemoveDirtySpec,  // specification
  OpTMRemoveDirtyValueMap, //value mapping 
  Operator::SimpleSelect,
  OpTMRemoveDirtyTypeMap //type mapping 
);

Operator getrect1(
  "getrect1",  //name 
  OpTMGetRect1Spec, //specification
  OpTMGetRect1ValueMap, //value mapping 
  Operator::SimpleSelect,
  OpTMGetRect1TypeMap //type mapping 
);


Operator path_to_building(
  "path_to_building",  //name 
  OpTMPathToBuildingSpec, //specification
  OpTMPathToBuildingValueMap, //value mapping 
  Operator::SimpleSelect,
  OpTMPathToBuildingTypeMap //type mapping 
);

Operator set_building_type(
  "set_building_type",  //name 
  OpTMSetBuildingTypeSpec, //specification
  OpTMSetBuildingTypeValueMap, //value mapping 
  Operator::SimpleSelect,
  OpTMSetBuildingTypeTypeMap //type mapping 
);

/*
data cleaning process 

*/
Operator modifyline(
    "modifyline",               // name
    OpTMModifyLineSpec,          // specification
    OpTMModifyLinemap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMModifyLineTypeMap        // type mapping
);

Operator checkroads(
    "checkroads",
    OpTMCheckRoadsSpec,
    OpTMCheckRoadsmap,
    Operator::SimpleSelect,        // selection function
    OpTMCheckRoadsTypeMap
);

Operator tm_join1(
    "tm_join1",
    OpTMTMJoin1Spec,
    OpTMTMJoin1map,
    Operator::SimpleSelect,        // selection function
    OpTMTMJoin1TypeMap
);

Operator nearstops_pave(
  "nearstops_pave",
  OpTMNearStopPaveSpec,
  OpNearStopPaveMap,
  Operator::SimpleSelect,
  OpNearStopPaveTypeMap
);

Operator nearstops_building(
  "nearstops_building",
  OpTMNearStopBuildingSpec,
  OpNearStopBuildingMap,
  Operator::SimpleSelect,
  OpNearStopBuildingTypeMap
);

/*
Main Class for Transportation Mode
data types and operators 

*/
class TransportationModeAlgebra : public Algebra
{
 public:
  TransportationModeAlgebra() : Algebra()
  {
    //////////////////graph data type/////////////////////////////////
    AddTypeConstructor(&dualgraph);
    dualgraph.AssociateKind("DUALGRAPH");
    AddTypeConstructor(&visualgraph);
    visualgraph.AssociateKind("VISUALGRAPH");
    AddTypeConstructor(&indoorgraph);
    indoorgraph.AssociateKind("INDOORGRAPH");
    AddTypeConstructor(&busgraph); 
    AddTypeConstructor(&metrograph);
    AddTypeConstructor(&roadgraph);//road graph 
    /*   Indoor   Data Type   */
    AddTypeConstructor( &point3d);
    AddTypeConstructor( &line3d);
    AddTypeConstructor( &floor3d);
    AddTypeConstructor( &door3d);
    AddTypeConstructor( &groom);
    AddTypeConstructor( &upoint3d);
    AddTypeConstructor( &mpoint3d); 

    point3d.AssociateKind("DATA");
    line3d.AssociateKind("DATA");
    floor3d.AssociateKind("DATA");
    door3d.AssociateKind("DATA");
    groom.AssociateKind("DATA");
    upoint3d.AssociateKind("DATA"); 
    mpoint3d.AssociateKind("DATA"); 
    ///////////////////////////////////////////////////////////////////
    //////////////////////public transportation network////////////////
    ///////////////////////////////////////////////////////////////////
    AddTypeConstructor( &busstop);
    busstop.AssociateKind("DATA"); 
    AddTypeConstructor( &busroute);
    busroute.AssociateKind("DATA"); 
    AddTypeConstructor( &busnetwork); 
    AddTypeConstructor( &metronetwork);
    ///////////////////////////////////////////////////////////
    //////////////////////  Pavement //////////////////////////
    ///////////////////////////////////////////////////////////
    AddTypeConstructor( &pavenetwork); 
    ///////////////////////////////////////////////////////////
    //////////////////////  Building //////////////////////////
    ///////////////////////////////////////////////////////////
    AddTypeConstructor( &building); 
    AddTypeConstructor( &indoorinfra);

    //////////////////////////////////////////////////////////////
    ////////////////////general data type ////////////////////////
    //////////////////////////////////////////////////////////////
    AddTypeConstructor(&ioref);
    ioref.AssociateKind("DATA"); 
    AddTypeConstructor(&genloc);
    genloc.AssociateKind("DATA"); 
    AddTypeConstructor(&genrange);
    genrange.AssociateKind("DATA"); 
    AddTypeConstructor(&ugenloc);
    ugenloc.AssociateKind("DATA");
    AddTypeConstructor(&genmo); 
    genmo.AssociateKind("DATA"); 
    AddTypeConstructor(&space); 
    space.AssociateKind("DATA");
    /////////////////////////////////////////////////////////////
    ////operators for partition regions//////////////////////////
    /////////////////////////////////////////////////////////////
    AddOperator(&checksline);
    AddOperator(&modifyboundary);
    AddOperator(&segment2region);
    AddOperator(&paveregion);
    AddOperator(&junregion);
    AddOperator(&decomposeregion);
    AddOperator(&fillpavement);

    //////////operators for building the graph model on pavements//////////
    AddOperator(&getpavenode1);
    AddOperator(&getpaveedge1);
    AddOperator(&getpavenode2);
    AddOperator(&getpaveedge2);
    AddOperator(&triangulation);
    AddOperator(&triangulation2);
    AddOperator(&convex);
    AddOperator(&geospath);
    AddOperator(&createdualgraph);///////create a dual graph 
    AddOperator(&nodedualgraph);
    //////////////////////////////////////////////////////////////////
    ///////////////////visibility graph///////////////////////////////
    //////////////////////////////////////////////////////////////////
    AddOperator(&getvnode);//get visible points of a given point 
    AddOperator(&myinside);
    AddOperator(&decomposetri);
    AddOperator(&getvgedge);
    AddOperator(&createvgraph);//create a visibility graph 
    
    AddOperator(&walk_sp_old);//trip planning for pedestrian 
    AddOperator(&test_walk_sp); //test the algorithm of trip planning 
    AddOperator(&walk_sp);//trip planning for pedestrian 
    AddOperator(&setpave_rid);//set rid value for each pavement 
    AddOperator(&pave_loc_togp);//map pavements locations to gpoints 
    ///////////////////dual graph/////////////////////////////////////
    AddOperator(&zval);//z-order value of a point
    AddOperator(&zcurve);//create a curve for the points sorted by z-order
    AddOperator(&regvertex);
    AddOperator(&triangulation_new);
    AddOperator(&triangulation_new2);
    AddOperator(&get_dg_edge);//create dual graph edge relation 

    AddOperator(&smcdgte);//simple method to create dual graph, traverse RTree
    ////////////////////data process///////////////////////////////////
    AddOperator(&generate_wp1);
    AddOperator(&generate_wp2);
    AddOperator(&generate_wp3);
    AddOperator(&getcontour);
    AddOperator(&getpolygon);
    AddOperator(&getsections);  
    AddOperator(&geninterestp1);
    AddOperator(&geninterestp2);
    AddOperator(&thepavement); //create region based outdoor infrastructure 
    ///////////////////////////////////////////////////////////////////
    ///////////////rotational plane sweep algorithm////////////////////
    //////////////////////////////////////////////////////////////////
    AddOperator(&getallpoints);
    AddOperator(&rotationsweep);//rotational plan sweep to find visible points 
    AddOperator(&gethole);
    //////////////////////////////////////////////////////////////////
    /*create bus network*/
    ///////////////// bus stops and bus routes  //////////////////////
    AddOperator(&cellbox);
    AddOperator(&create_bus_route1); //rough representation
    AddOperator(&create_bus_route2); //create bus route 
    AddOperator(&refine_bus_route); //filter some bus routes which are similar 
    AddOperator(&bus_route_road); //calculate the total length of bus routes 
    AddOperator(&create_bus_route3);//copy bus route, split 
    AddOperator(&create_bus_route4);//set up and down for bus routes 
    AddOperator(&create_bus_stop1); //create bus stops on bus routes
    AddOperator(&create_bus_stop2); //merge bus stops (same road section)
    AddOperator(&create_bus_stop3); //merge bus stops (adjacent section)
    AddOperator(&create_bus_stop4); //change bus stop position 
    AddOperator(&create_bus_stop5); //set up and down for bus stops 
    AddOperator(&getbusstops);//use data type busstop representing bus stops
    AddOperator(&getstopid); 
    AddOperator(&getbusroutes);//use data type busroute representing bus routes
    AddOperator(&brgeodata);//get geometrical line of a bus route
    AddOperator(&bsgeodata);//get the point of a bus stop 
    AddOperator(&up_down);//get up or down direction for bus stops and routes
    AddOperator(&thebusnetwork);//create bus network 
    AddOperator(&bn_busstops);//get bus stops relation
    AddOperator(&bn_busroutes);//get bus routes relation
    ////////////////////////////////////////////////////////////////////////
    /*grpah for the bus network*/
    ///////////////////////////////////////////////////////////////////////
    AddOperator(&mapbstopave); //map bus stops to the pavements 
    AddOperator(&bs_neighbors1); //for each bus stop (pavement), find  neighbors
    AddOperator(&bs_neighbors2);//bus stops with same 2D point, different routes
    AddOperator(&bs_neighbors3);//connected by moving buses 
    AddOperator(&createbgraph);//create bus network graph 
    AddOperator(&getadjnode); //get neighbor nodes of a given node in vg,dg,bg..
    AddOperator(&bnnavigation);bnnavigation.SetUsesArgsInTypeMapping();
   AddOperator(&test_bnnavigation);test_bnnavigation.SetUsesArgsInTypeMapping();
    //////////////preprocess road data to get denstiy value/////////////
    ///////////// create time table and moving buses  //////////////////
    AddOperator(&get_route_density1);//get daytime and night bus routes 
    AddOperator(&set_ts_nightbus);//set time schedule for night bus 
    AddOperator(&set_ts_daybus);//set time schedule for daytime bus 
    AddOperator(&set_br_speed);// set speed value for each bus route 
    AddOperator(&create_bus_segment_speed); //set speed value for each segment 
    AddOperator(&create_night_bus_mo);//create night moving bus 
    AddOperator(&create_daytime_bus_mo);//create daytime moving bus 
    AddOperator(&create_time_table1);//create time table for each spatial stop 
    AddOperator(&create_time_table1_new);//compact storage of bus time tables 
    ///////////////////////////////////////////////////////////////////
    //////////    process UBahn Trains    /////////////////////////////
    ///////////////////////////////////////////////////////////////////
    AddOperator(&create_time_table2);//create time table for train stop 
    AddOperator(&create_time_table2_new);//compact storage of train time tables 
    ///////////////////////////////////////////////////////////////////
    ///////////convert berlin trains to genmo///////////////
    AddOperator(&refmo2genmo);
    AddOperator(&themetronetwork);//create metro network infrastructure 
    AddOperator(&ms_neighbors1);//create one kind of metro graph edges 
    AddOperator(&ms_neighbors2);//create one kind of metro graph edges(moving)
    AddOperator(&createmgraph);//create metro network graph 
    //////////////////////////////////////////////////////////////////////
    //////////////////create underground train from road data////////////
    /////////////////////////////////////////////////////////////////////
    AddOperator(&createmetroroute);//create metro routes
    AddOperator(&createmetrostop);//create metro stops from routes 
    AddOperator(&createmetromo);//create moving metros 
    AddOperator(&mapmstopave);//map metro stops to pavement areas
    AddOperator(&mnnavigation);//query processing on the metro network,graph 

    ////////////////////////////////////////////////////////////////////
    ////////////////  Indoor Operators   ///////////////////////////////
    ////////////////////////////////////////////////////////////////////
    AddOperator(&thefloor);//create a floor3d object 
    AddOperator(&getregion);//2D area for a room 
    AddOperator(&getheight);//height for a room 
    AddOperator(&thedoor);//create a doo3d object 
    AddOperator(&oid_of_door);//lift or non-lift 
    AddOperator(&type_of_door);//lift or non-lift 
    AddOperator(&loc_of_door); //relative location in one room 
    AddOperator(&state_of_door); //time dependent state:open closed 
    AddOperator(&get_floor);//get one element from a groom 
    AddOperator(&add_height_groom);//move the groom higher by the input 
    AddOperator(&translate_groom);//translate the 2D region 
    AddOperator(&tm_length);//the length of a 3d line 
    AddOperator(&bbox3d); //return the 3d box of the a line3d 
    AddOperator(&thebuilding);//create a kind of building 
    AddOperator(&theindoor);//create the indoor infrastructure 
    ///////////////////////////////////////////////////////////////////////
    //////////////// indoor  navigation //////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    AddOperator(&createdoor3d); //create line3d to denote the doors 
    AddOperator(&createdoorbox); //create a 3d box for each door 
    AddOperator(&createdoor1);//the node relation for the graph (doors)
    AddOperator(&createdoor2);//the node relation for the graph (virtual doors)
    AddOperator(&createadjdoor1); //the edge relation for indoor graph 
    AddOperator(&createadjdoor2); //the edge relation for indoor graph 

    AddOperator(&path_in_region);//shortest path between two points inside a reg
    AddOperator(&createigraph);//create indoor graph 
    AddOperator(&generate_ip1);//generate indoor positions 
    AddOperator(&generate_mo1);//generate moving objects:int indoor,real:+genmo
    AddOperator(&getindoorpath);//read indoor paths from files 
    //////////////  indoor navigation ////////////////////////////////////////
    AddOperator(&indoornavigation);indoornavigation.SetUsesArgsInTypeMapping();
    ////////////////////////////////////////////////////////////////////
    /////////////////  others  /////////////////////////////////////////
    AddOperator(&instant2day); 
    ////////////////////////////////////////////////////////////////////
    //////////////////2D areas for buildings///////////////////////////
    ///////////////////////////////////////////////////////////////////
    AddOperator(&maxrect); //get the maximum rect in a region
    AddOperator(&remove_dirty); //clear dirty data 
    AddOperator(&getrect1); //get all rectangles for buildings 
    AddOperator(&path_to_building);//connect building entrance and pavement
    AddOperator(&set_building_type);//set which type the building is 
    /////////non-temporal operators for generic data types////////////////////
    AddOperator(&ref_id); 
    AddOperator(&tm_at);//at mode, at genloc
    ///////////////////////////////////////////////////////////////////////
    /////////temporal operators for generic data types////////////////////
    //////////////////////////////////////////////////////////////////////
    AddOperator(&setref_id);//genmo, genrange, a set of ref ids 
    AddOperator(&genmodeftime); //get the define time of generic moving objects
    AddOperator(&genmonocomponents); //get number of components
    AddOperator(&lowres);  //low resolution representation 
    AddOperator(&tmtrajectory);  //trajectory 
    AddOperator(&gentrajectory);  //trajectory for generic moving objects
    AddOperator(&genrangevisible); //convert to Javagui visible data type
    AddOperator(&getmode); //get transportation modes
    AddOperator(&getref);//get referenced infrastructure objects 
    /////////////////////////////////////////////////////////////////////
    //////////////////space operators/////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    AddOperator(&addinfragraph);//add navigation graph for each infrastructure
    AddOperator(&thespace); //create an empty space
    AddOperator(&putinfra); //add network infrastructure
                            //add pavements infrastructure (graph)
                            //add bus network infrastructure (graph)
                            //add trains network infrastructure
                            //add indoor infrastructure
                            //add road graph 

    //retrieve required infrastructure relation 
    AddOperator(&getinfra); getinfra.SetUsesArgsInTypeMapping();
   //////////////////////////////////////////////////////////////////////////
   ////////////////////// generate generic moving objects///////////////////
   /////////////////////////////////////////////////////////////////////////
   AddOperator(&genmo_tm_list);//all kinds of movements

   AddOperator(&generate_genmo2);//car or taxi + walk
   AddOperator(&generate_car);//car, for creating moving gpoints to get traffic
   AddOperator(&generate_car_ext);//optimize, use some prestored paths 
   AddOperator(&generate_genmo3);//bus + walk
   AddOperator(&generate_genmo4);//indoor + walk
   AddOperator(&generate_genmo5);//indoor + walk + car(taxi)
   AddOperator(&generate_genmo6);//indoor + walk + bus
   AddOperator(&generate_genmo6_ext);//indoor + walk + bus (building relation)
   AddOperator(&generate_genmo7);//metro + walk
   AddOperator(&generate_genmo8);//indoor + metro + walk
   AddOperator(&generate_genmo8_ext);//indoor + metro + walk (building relation)
   /////////////////////////////////////////////////////////////////////////
   //////////////// improve shortest path computing in road network////////
   ///////////////////////////////////////////////////////////////////////

   AddOperator(&get_rg_nodes);//get road graph nodes 
   AddOperator(&get_rg_edges1);//get road graph edges, same location 
   AddOperator(&get_rg_edges2);//get road graph edges, glines
   AddOperator(&creatergraph);//create road network graph
   AddOperator(&shortestpath_tm);//shortest path on road graph
   AddOperator(&shortestpath_tm2);//path on road graph avoid center area
   ///////////////////////////////////////////////////////////////////////
   ///////////////overall navigation system///////////////////////////////
   /////////////////////////////////////////////////////////////////////
   AddOperator(&navigation1);//navigation with modes bus and walk 

   //////////////////////////////////////////////////////////////////
   /////////////// data clean process////////////////////////////////
   ///////////////////////////////////////////////////////////////////
   AddOperator(&modifyline);
   AddOperator(&checkroads);
   AddOperator(&tm_join1);
   ////////////////////////////////////////////////////////////////////
   /////find pavement areas and buildings clost to bus,metro stops/////
   ////////////////////////////////////////////////////////////////////
   AddOperator(&nearstops_pave);
   AddOperator(&nearstops_building);

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
