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

extern NestedList* nl;
extern QueryProcessor *qp;

static map<int,string> *busnetList;


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
 " -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
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
    "( <text>int x rel x rel -> indoorgraph</text--->"
    "<text>createigraph(int, rel, rel)</text--->"
    "<text>create an indoor graph by the input edges and nodes"
    "relation</text--->"
    "<text>query createigraph(1, edge-rel, node-rel); </text--->"
    ") )";

const string OpTMGetAdjNodeIGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>indoorgraph x int"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getadjnode_ig(indoorgraph,int)</text--->"
    "<text>for a given node, find its adjacent nodes</text--->"
    "<text>query getadjnode_ig(ig1,1); </text--->"
    ") )"; 

 const string SpatialSpecGenerateIP1 =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>rel x int -> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
"<text>generate_ip1(rel, int) </text--->"
"<text>create indoor positions</text--->"
"<text>query generate_ip1(building_uni,20) count</text---> ) )";

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
  nl->ReadFromString(IndoorGraph::NodeTypeInfo, xType);
  if(!CompareSchemas(xNodeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  nl->ReadFromString(IndoorGraph::EdgeTypeInfo, xType);
  if(!CompareSchemas(xEdgeDesc, xType))return nl->SymbolAtom ( "typeerror" );

  return nl->SymbolAtom ( "indoorgraph" );
}

/*
TypeMap fun for operator getadjnode

*/

ListExpr OpTMGetAdjNodeIGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return  nl->SymbolAtom ( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);


  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "indoorgraph" &&
     nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
     nl->SymbolValue(arg2) == "int"){

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
  return  nl->SymbolAtom ( "typeerror" );
}


/*
TypeMap function for operator generate ip1

*/
ListExpr GenerateIP1TypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 2){
      string err = "rel x int";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  ListExpr xType;
  nl->ReadFromString(IndoorNav::Indoor_GRoom_Door, xType); 
  if (listutils::isRelDescription(arg1)){
      if(CompareSchemas(arg1, xType) && 
        nl->IsAtom(arg2) && nl->AtomType(arg2) == SymbolType &&
        nl->SymbolValue(arg2) == "int" ){

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
  ShortestPath_InRegion(reg, s, d, pResult);
  
  return 0;
}

/*
Value Mapping for  createigraph  operator

*/

int OpTMCreateIGValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  IndoorGraph* ig = (IndoorGraph*)qp->ResultStorage(in_pSupplier).addr;
  int ig_id = ((CcInt*)args[0].addr)->GetIntval();
  Relation* node_rel = (Relation*)args[1].addr;
  Relation* edge_rel = (Relation*)args[2].addr;
  ig->Load(ig_id, node_rel, edge_rel);
  result = SetWord(ig);
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
        
        indoor_nav = new IndoorNav(rel, NULL);
        indoor_nav->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        indoor_nav->GenerateIP1(num);
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

        indoor_nav->GenerateMO1(ig, btree, rtree, num, peri, false);
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

        indoor_nav->GenerateMO1(ig, btree, rtree, num, peri, true);
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

Operator getadjnode_ig(
    "getadjnode_ig",
    OpTMGetAdjNodeIGSpec,
    OpTMGetAdjNodeIGValueMap,
    Operator::SimpleSelect,
    OpTMGetAdjNodeIGTypeMap
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
        OpenSpace,  //object open 
        SaveSpace,   // object save
        CloseSpace, CloneSpace,//object close and clone
        Space::Cast,
        SizeOfSpace,              //sizeof function
        CheckSpace); 

const string SpatialSpecRefId =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genloc -> int</text--->"
"<text>ref_id (_) </text--->"
"<text>get the reference id of a genloc object</text--->"
"<text>query ref_id (genloc1)</text---> ) )";


const string SpatialSpecGenMODeftime =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> periods</text--->"
"<text>deftime (_) </text--->"
"<text>get the deftime time of a generic moving object</text--->"
"<text>query deftime (genmo)</text---> ) )";

const string SpatialSpecGenMONoComponents =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> int</text--->"
"<text>no_components (_) </text--->"
"<text>get the number of units in a generic moving object</text--->"
"<text>query no_components(genmo)</text---> ) )"; 

const string SpatialSpecLowRes =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> genmo</text--->"
"<text>genmo (_) </text--->"
"<text>return the low resolution of generic moving object</text--->"
"<text>query lowres(genmo1)</text---> ) )";

const string SpatialSpecTMTrajectory =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> genrange</text--->"
"<text>trajectory (_) </text--->"
"<text>get the trajectory of a moving object</text--->"
"<text>query trajectory(genmo)</text---> ) )"; 

const string SpatialSpecGetMode =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>genmo -> (stream(((x1 t1) ... (xn tn)))</text--->"
"<text>getmode (_) </text--->"
"<text>return the transportation modes</text--->"
"<text>query getmode(genmo1)</text---> ) )";

/*
create an empty space 

*/
const string SpatialSpecTheSpace =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int -> space </text--->"
"<text>thespace (_) </text--->"
"<text>create an empty space</text--->"
"<text>query thespace(1)</text---> ) )";


const string SpatialSpecGenMOTMList =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>bool -> (stream(((x1 t1) ... (xn tn))) </text--->"
"<text>genmo_tm_list (bool) </text--->"
"<text>output all possible transportation modes of moving objects</text--->"
"<text>query genmo_tm_list(TRUE)</text---> ) )";


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

int RefIdBusNetworkValueMap(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  BusNetwork* bn = (BusNetwork*)args[0].addr;
  result = qp->ResultStorage(s);
  if(bn->IsDefined() && bn->GetId() > 0){
      ((CcInt*)result.addr)->Set(true, bn->GetId());
  }else
    ((CcInt*)result.addr)->Set(false, 0);
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
  if(mo->IsDefined()){
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
  if(mo->IsDefined()){
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
  result = qp->ResultStorage(s);
  GenRange* presult = (GenRange*)result.addr; 
  if(mo->IsDefined()){
    mo->Trajectory(*presult); 
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

        mo->GetTM(genmo);
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


ValueMapping RefIdValueMapVM[]={
  RefIdGenLocValueMap,
  RefIdIORefValueMap,
  RefIdBusStopValueMap,
  RefIdBusRouteValueMap,
  RefIdBusNetworkValueMap
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
  return -1;
}


/*
TypeMap function for operator ref id  

*/
ListExpr RefIdTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genloc or genrange expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genloc") || 
     nl->IsEqual(arg1, "ioref") || 
     nl->IsEqual(arg1, "busstop") || 
     nl->IsEqual(arg1, "busroute") || nl->IsEqual(arg1, "busnetwork"))
    return nl->SymbolAtom("int");

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
      string err = "no input parameter expected";
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


Operator ref_id("ref_id",
    SpatialSpecRefId,
    5,
    RefIdValueMapVM,
    RefIdOpSelect,
    RefIdTypeMap
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


ValueMapping TMTrajectoryValueMapVM[]={
  GenMOTrajectoryValueMap,
  MP3dTrajectoryValueMap, 
};

int TMTrajectoryOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genmo"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "mpoint3d"))
    return 1;
  return -1;
}

/*
TypeMap function for operator trajectory

*/
ListExpr TMTrajectoryTypeMap(ListExpr args)
{
  if(nl->ListLength(args) != 1){
      string err = "genmo expected";
      return listutils::typeError(err);
  }
  ListExpr arg1 = nl->First(args);
  if(nl->IsEqual(arg1, "genmo"))
    return nl->SymbolAtom("genrange");

  if(nl->IsEqual(arg1, "mpoint3d"))
    return nl->SymbolAtom("line3d");

  return nl->SymbolAtom("typeerror");
}

Operator tmtrajectory("trajectory",
    SpatialSpecTMTrajectory,
    2,
    TMTrajectoryValueMapVM,
    TMTrajectoryOpSelect,
    TMTrajectoryTypeMap
);



Operator getmode("getmode",
    SpatialSpecGetMode,
    GetModeValueMap,
    Operator::SimpleSelect,
    GetModeTypeMap
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


Operator genmo_tm_list("genmo_tm_list",
    SpatialSpecGenMOTMList,
    GenMOTMListValueMap,
    Operator::SimpleSelect,
    GenMOTMListTypeMap
);

/////////////////////////////////////////////////////////////////////////////
///////////////////   general data type   ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

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
    "( <text>dualgraph x visualgraph x rel1 x rel2 x rel3-> line</text--->"
    "<text>walk_sp(dg1, vg1, rel, rel, rel)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp(dg1, vg1, query_loc1, query_loc2,tri_reg_new);"
    "</text--->"
    ") )";

const string OpTMTestWalkSPSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>dualgraph x visualgraph x rel1 x rel2 x rel3 -> "
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>walk_sp(dualgraph, visibilitygraph, rel, rel, rel)</text--->"
    "<text>get the shortest path for pedestrian</text--->"
    "<text>query walk_sp(dg1, vg1, query_loc1, query_loc2,tri_reg_new);"
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
    "-> (stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_bus_stops1(n,rel1,attr1,attr2, attr3, attr4, rel2, btree);"
    "</text--->"
    "<text>create bus stops</text--->"
    "<text>query create_bus_stop1(n,busroutes,br_id,bus_route1,"
    "bus_route2,route_type,subpaves2, btree_pave2) count;</text--->"
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
    "( <text>int x rel x rel -> busnetwork</text--->"
    "<text>thebusnetwork(1, stops_rel, routes_rel); </text--->"
    "<text>create bus network</text--->"
    "<text>query busnetwork(1, bus_stops, bus_routes) ;</text--->) )";
    
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
    "( <text>busnetwork x rtree x rel x int"
     " -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>mapbstopave(bn1, rtree_dg, dg_node, roadwidth);"
    " </text--->"
    "<text>map bus stops to pavement areas</text--->"
    "<text>query mapbstopave(bn1, rtree_dg, dg_node, roadwidth)"
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

/*
create a graph on bus network including pavements connection 

*/
const string OpTMCreateBusGraphSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel1 x rel2 -> busgraph</text--->"
    "<text>createbgraph(int, rel, rel)</text--->"
    "<text>create a bus network graph by the input edges and nodes"
    "relation</text--->"
    "<text>query createbgraph(1, node-rel, edge1); </text--->"
    ") )";

const string OpTMGetAdjNodeBGSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>busgraph x int -> (stream(((x1 t1) ... (xn tn))))</text--->"
    "<text>getadjnode_bg(busgraph, int)</text--->"
    "<text>get the neighbor nodes of a given graph node</text--->"
    "<text>query getadjnodebg(bg1, 2); </text--->"
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
    
const string OpTMCreateUBTrainsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x attr3 x duration "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>createUBTrains(rel,attr1,attr2,attr3,duration);</text--->"
    "<text>create UBahn Trains </text--->"
    "<text>query createUBTrains(UBahnTrains1,Line,Up,Trip,"
    "UBTrain_time) count;</text--->"
    ") )";
    
const string OpTMCreateUBTrainStopSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x attr3 "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>create_train_stop(rel,attr,attr,attr);</text--->"
    "<text>create UBahn Train Stops </text--->"
    "<text>query create_train_stop(UBahnTrains,Line,Up,Trip) count;</text--->"
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

    
const string OpTMSplitUBahnSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>splitubahn(rel, attr, attr );</text--->"
    "<text>represent the ubahn line in a new way </text--->"
    "<text>query splitubahn(UBahn, Name, geoData) count;</text--->"
    ") )";

const string OpTMTrainsToGenMOSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree "
    "->(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>trainstogenmo(rel1, rel2, btree );</text--->"
    "<text>convert trains to generic moving objects </text--->"
    "<text>query trainstogenmo(Trains, ubahn_line, btree_ub_line) count;"
    "</text--->) )";
    
const string OpTMInstant2DaySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>instant -> int </text--->"
    "<text>instant2day(instant);</text--->"
    "<text>get the day (int value) of time</text--->"
    "<text>query instant2day(theInstant(2007,6,3,9,0,0,0));</text--->"
    ") )";

const string OpTMOutputRegionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region -> string </text--->"
    "<text>outputregion(region);</text--->"
    "<text>output the vertices in correct order"
    "(clockwise for the outer cycle and counter clockwise for holes)</text--->"
    "<text>query outputregion(r1);</text--->"
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

const string OpTMGetRect2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2 x attr3 x btree x int ->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getrect(rel, attr, attr, attr, btree, int);</text--->"
    "<text>get the maximum rectangle area for a region</text--->"
    "<text>query getrect(building_regions, id, covarea, reg_type, btree_build "
    "10);</text--->"
    ") )";
    
const string OpTMGetRect1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel x attr1 x attr2->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>getrect2(rel, attr, attr);</text--->"
    "<text>get the maximum rectangle area for a region</text--->"
    "<text>query getrect2(new_region_elems2, id, covarea);</text--->"
    ") )";

/*
build a path between the entrance of the building and the pavement area 

*/
const string OpTMPathToBuildingSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn)))</text--->"
    "<text>path_to_building(rel, rel, btree);</text--->"
    "<text>build the connection between building and pavement</text--->"
    "<text>query path_to_building(building_rect, new_region_elems," 
    "btree_region_elem);</text--->"
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
TypeMap fun for operator walksp

*/

ListExpr OpTMWalkSPTypeMap ( ListExpr args )
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
  if ( nl->ListLength ( args ) != 8 )
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
                nl->OneElemList(
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
TypeMap fun for operator thebusnetwork

*/
ListExpr OpTMTheBusNetworkTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
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
  if ( nl->ListLength ( args ) != 4 )
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
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_uoid"),
                        nl->SymbolAtom("int")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline"))
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
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_uoid"),
                        nl->SymbolAtom("int"))
                    )));
  return res;

}


/*
TypeMap fun for operator createbgraph

*/

ListExpr OpTMCreateBusGraphTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr xIdDesc = nl->First(args);
  ListExpr xNodeDesc = nl->Second(args);
  ListExpr xEdgeDesc1 = nl->Third(args);
  
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

  return nl->SymbolAtom ( "busgraph" );
}


/*
TypeMap fun for operator getadjnode bg

*/

ListExpr OpTMGetAdjNodeBGTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First(args);
  ListExpr param2 = nl->Second(args);
  
  
  if(!(nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&  
     nl->SymbolValue(param1) == "busgraph")){
      return nl->SymbolAtom ( "typeerror: param1 should be busgraph" );
  }
  if(!nl->IsEqual(param2, "int")) return nl->SymbolAtom ( "typeerror" );
  
  
  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop1"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_stop2"),
                        nl->SymbolAtom("busstop")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("sline"))
                    )));
  return res; 

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
  
   ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->Cons(            
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_loc"),
                        nl->SymbolAtom("point")),        
                nl->SixElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("br_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("bus_direction"),
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
TypeMap fun for operator create UBahn Trains

*/

ListExpr OpTMCreateUBTrainsTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 5 )
  {
    return  nl->SymbolAtom ( "list length should be 5" );
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
                      aname2,attrType2);
                      
  if(j2 == 0 || !listutils::isSymbol(attrType2,"bool")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type bool");
  }


  ListExpr attrName3 = nl->Fourth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3,attrType3);
                      
  if(j3 == 0 || !listutils::isSymbol(attrType3,"mpoint")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type mpoint");
  }


  ListExpr param5 = nl->Fifth(args );
  if(!(nl->IsAtom(param5) && nl->AtomType(param5) == SymbolType &&  
     nl->SymbolValue(param5) == "periods")){
      return nl->SymbolAtom ( "typeerror: param5 should be periods" );
  }  
  
  
     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Line"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Up"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Trip"),
                        nl->SymbolAtom("mpoint")),
                    nl->TwoElemList(
                        nl->SymbolAtom("schedule_id"),
                        nl->SymbolAtom("int"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                          nl->IntAtom(j3)), res);
}


/*
TypeMap fun for operator create UBahn Train Stops 

*/

ListExpr OpTMCreateUBTrainStopTypeMap ( ListExpr args )
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
                      aname2,attrType2);
                      
  if(j2 == 0 || !listutils::isSymbol(attrType2,"bool")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type bool");
  }


  ListExpr attrName3 = nl->Fourth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3,attrType3);

  if(j3 == 0 || !listutils::isSymbol(attrType3,"mpoint")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type mpoint");
  }

     ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),

                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("LineId"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("loc"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("stop_id"),
                        nl->SymbolAtom("int"))
                    )));

      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->ThreeElemList(nl->IntAtom(j1),nl->IntAtom(j2),
                          nl->IntAtom(j3)), res);
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
represent the UBahn line in a new way  
(lineid int)(geoData1 line)(geoData2 sline)

*/

ListExpr OpTMSplitUBahnTypeMap ( ListExpr args )
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

  if(j1 == 0 || !listutils::isSymbol(attrType1,"string")){
      return listutils::typeError("attr name" + aname1 + "not found"
                      "or not of type string");
  }

  ListExpr attrName2 = nl->Third ( args );
  ListExpr attrType2;
  string aname2 = nl->SymbolValue(attrName2);
  int j2 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname2, attrType2);

  if(j2 == 0 || !listutils::isSymbol(attrType2, "line")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type line");
  }

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->TwoElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("lineid"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("geoData"),
                        nl->SymbolAtom("sline"))
                    )));
      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)), res);
}


/*
conver berlintest trains to generic moving objects 

*/

ListExpr OpTMTrainsToGenMOTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
  }
  
  ListExpr param1 = nl->First ( args );
  if(!IsRelDescription(param1))
    return nl->SymbolAtom ( "typeerror: param1 should be a relation" );
  
  ListExpr xType1;
  nl->ReadFromString(UBTrain::TrainsTypeInfo, xType1); 
  if(!CompareSchemas(param1, xType1)){
    return listutils::typeError("rel1 scheam should be" + 
                                UBTrain::TrainsTypeInfo);
  }

  ListExpr param2 = nl->Second ( args );
  if(!IsRelDescription(param2))
    return nl->SymbolAtom ( "typeerror: param2 should be a relation" );
  
  ListExpr xType2;
  nl->ReadFromString(UBTrain::UBahnLineInfo, xType2); 
  if(!CompareSchemas(param2, xType2)){
    return listutils::typeError("rel2 scheam should be" + 
                               UBTrain::UBahnLineInfo);
  }

  ListExpr param3 = nl->Third ( args );
  if(!listutils::isBTreeDescription(param3))
    return nl->SymbolAtom ( "typeerror: param3 should be a btree" );


  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Line"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Up"),
                        nl->SymbolAtom("bool")),
                    nl->TwoElemList(
                        nl->SymbolAtom("Trip"),
                        nl->SymbolAtom("genmo"))
                    )));
  return  res;
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
TypeMap fun for operator outputregion 

*/

ListExpr OpTMOutputRegionTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 1 )
  {
    return  nl->SymbolAtom ( "list length should be 1" );
  }
  if(nl->IsEqual(nl->First(args),"region")){
    return nl->SymbolAtom("string");
  }else
    return nl->SymbolAtom ( "typeerror: param1 should be region" );
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

ListExpr OpTMGetRect2TypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 7 )
  {
    return  nl->SymbolAtom ( "list length should be 7" );
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

  if(j2 == 0 || !listutils::isSymbol(attrType2,"rect")){
      return listutils::typeError("attr name" + aname2 + "not found"
                      "or not of type rect");
  }

  ListExpr attrName3 = nl->Fourth ( args );
  ListExpr attrType3;
  string aname3 = nl->SymbolValue(attrName3);
  int j3 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname3, attrType3);

  if(j3 == 0 || !listutils::isSymbol(attrType3,"int")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");
  }

  ListExpr attrName4 = nl->Fifth ( args );
  ListExpr attrType4;
  string aname4 = nl->SymbolValue(attrName4);
  int j4 = listutils::findAttribute(nl->Second(nl->Second(param1)),
                      aname4, attrType4);

  if(j4 == 0 || !listutils::isSymbol(attrType4,"int")){
      return listutils::typeError("attr name" + aname3 + "not found"
                      "or not of type int");
  }


  ListExpr param6 = nl->Sixth ( args );
  if(!listutils::isBTreeDescription(param6))
    return nl->SymbolAtom ( "typeerror: param6 should be a btree" );

  if(!(nl->IsEqual(nl->Nth(7, args),"int")))
    return nl->SymbolAtom ( "typeerror: param7 should be an int" );

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("reg_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("geoData"),
                        nl->SymbolAtom("rect")),
                    nl->TwoElemList(
                        nl->SymbolAtom("poly_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("reg_type"),
                        nl->SymbolAtom("int"))
                    )));
      return nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->FourElemList(nl->IntAtom(j1), nl->IntAtom(j2), 
                         nl->IntAtom(j3), nl->IntAtom(j4)), res);
}


/*
TypeMap fun for operator getrect. get the maximum rectangle from a convex
region 

*/

ListExpr OpTMGetRect1TypeMap ( ListExpr args )
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
  if ( nl->ListLength ( args ) != 3 )
  {
    return  nl->SymbolAtom ( "list length should be 3" );
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

  ListExpr res = nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("reg_id"),
                        nl->SymbolAtom("int")),
                    nl->TwoElemList(
                        nl->SymbolAtom("sp"),
                        nl->SymbolAtom("point")),
                    nl->TwoElemList(
                        nl->SymbolAtom("ep"),
                        nl->SymbolAtom("point")), 
                    nl->TwoElemList(
                        nl->SymbolAtom("Path"),
                        nl->SymbolAtom("line"))
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
Value Mapping for walksp  operator
return the shortest path for pedestrian

*/

int OpTMWalkSPValueMap ( Word* args, Word& result, int message,
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
Value Mapping for testwalksp  operator
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

        int attr1 = ((CcInt*)args[8].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[10].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[11].addr)->GetIntval() - 1;
        
        br = new BusRoute(n,r1,NULL,NULL);
        br->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        br->CreateBusStop1(attr1, attr2, attr3, attr4, r2, btree);
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

          tuple->PutAttribute(0, new Bus_Route(br->bus_route_list[br->count]));

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
get bus stop id

*/
int OpTMTheBusNetworkValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  BusNetwork* bn = (BusNetwork*)qp->ResultStorage(in_pSupplier).addr;
  int id = ((CcInt*)args[0].addr)->GetIntval(); 
  map<int,string>::iterator it = busnetList->find ( id );
  if ( it != busnetList->end() ){
    cout << "Bus NetworkId used before" << id << endl;
    return 0;
  }
  busnetList->insert ( pair<int,string> ( id, "" ) );
  Relation* stops = (Relation*)args[1].addr;
  Relation* routes = (Relation*)args[2].addr; 
  bn->Load(id, stops, routes);
  result = SetWord(bn); 
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

        b_n = new BN(bn);
        b_n->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        b_n->MapBSToPavements(rtree, pave_rel, w);
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

          tuple->PutAttribute(0, new Bus_Stop(b_n->bs_list1[b_n->count]));
          tuple->PutAttribute(1, new Bus_Stop(b_n->bs_list2[b_n->count]));
         tuple->PutAttribute(2, new CcInt(true, b_n->bs_uoid_list[b_n->count]));

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
Value Mapping for  createbusgraph  operator

*/

int OpTMCreateBusGraphValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  BusGraph* bg = (BusGraph*)qp->ResultStorage(in_pSupplier).addr;
  int bg_id = ((CcInt*)args[0].addr)->GetIntval();
  Relation* node_rel = (Relation*)args[1].addr;
  Relation* edge_rel1 = (Relation*)args[2].addr;
  
  bg->Load(bg_id, node_rel, edge_rel1);
  result = SetWord(bg);
  return 0;
}



/*
for each bus stop, find neighbor bus stops by searching on the bus graph 

*/
int OpTMGetAdjNodeBGValueMap ( Word* args, Word& result, int message,
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

        b_n->GetAdjNodeBG(bg, nodeid);
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
          tuple->PutAttribute(1, new CcInt(true, rd->br_id_list[rd->count]));
          tuple->PutAttribute(2, 
                              new CcInt(true, rd->bus_stop_id_list[rd->count]));
          tuple->PutAttribute(3, new CcBool(true, rd->br_direction[rd->count]));
          tuple->PutAttribute(4, new Periods(rd->duration1[rd->count])); 
          tuple->PutAttribute(5,
                            new CcReal(true,rd->schedule_interval[rd->count]));
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
create UBahn Trains 

*/
int OpTMCreateUBahanTrainsValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  UBTrain* ubtrain;
  switch(message){
      case OPEN:{
        
        Relation* r = (Relation*)args[0].addr; 
        Periods* peri = (Periods*)args[4].addr;        
        
        int attr1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[6].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[7].addr)->GetIntval() - 1;

        ubtrain = new UBTrain(r);
        ubtrain->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        ubtrain->CreateUBTrains(attr1,attr2,attr3,peri);
        local.setAddr(ubtrain);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ubtrain = (UBTrain*)local.addr;
          if(ubtrain->count == ubtrain->id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ubtrain->resulttype);
          tuple->PutAttribute(0, 
                            new CcInt(true, ubtrain->id_list[ubtrain->count]));
          tuple->PutAttribute(1, 
                        new CcInt(true, ubtrain->line_id_list[ubtrain->count]));
          tuple->PutAttribute(2, 
                     new CcBool(true, ubtrain->direction_list[ubtrain->count]));
          tuple->PutAttribute(3, 
                     new MPoint(ubtrain->train_trip[ubtrain->count])); 
          tuple->PutAttribute(4, 
                    new CcInt(true, ubtrain->schedule_id_list[ubtrain->count]));

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
create UBahn Train Stops 

*/
int OpTMCreateUBahanTrainStopValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  UBTrain* ubtrain;
  switch(message){
      case OPEN:{
        
        Relation* r = (Relation*)args[0].addr; 
        
        int attr1 = ((CcInt*)args[4].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[5].addr)->GetIntval() - 1;
        int attr3 = ((CcInt*)args[6].addr)->GetIntval() - 1;

        ubtrain = new UBTrain(r);
        ubtrain->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        
        ubtrain->CreateUBTrainStop(attr1,attr2,attr3);
        local.setAddr(ubtrain);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ubtrain = (UBTrain*)local.addr;
          if(ubtrain->count == ubtrain->line_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ubtrain->resulttype);
          tuple->PutAttribute(0, 
                        new CcInt(true, ubtrain->line_id_list[ubtrain->count]));
          tuple->PutAttribute(1, 
                        new Point(ubtrain->stop_loc_list[ubtrain->count]));
          tuple->PutAttribute(2, 
                     new CcInt(true, ubtrain->stop_id_list[ubtrain->count]));

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
represent the ubahn in a new way (lineid:int)(geoData1:line)(geoData2:sline)

*/
int OpTMSplitUBahnValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  UBTrain* ubtrain;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr; 
        int attr1 = ((CcInt*)args[3].addr)->GetIntval() - 1; 
        int attr2 = ((CcInt*)args[4].addr)->GetIntval() - 1; 
        
        ubtrain = new UBTrain(r, NULL, NULL);
        ubtrain->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ubtrain->SplitUBahn(attr1, attr2);
        local.setAddr(ubtrain);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ubtrain = (UBTrain*)local.addr;
          if(ubtrain->count == ubtrain->line_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ubtrain->resulttype);
          tuple->PutAttribute(0, 
                       new CcInt(true,ubtrain->line_id_list[ubtrain->count]));
          tuple->PutAttribute(1, 
                             new SimpleLine(ubtrain->geodata[ubtrain->count]));

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
convert trains to generic moving objects 

*/
int OpTMTrainsToGenMOValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  UBTrain* ubtrain;
  switch(message){
      case OPEN:{
        
        Relation* r1 = (Relation*)args[0].addr;
        Relation* r2 = (Relation*)args[1].addr; 
        BTree* btree = (BTree*)args[2].addr; 

        ubtrain = new UBTrain(r1, r2, btree);
        ubtrain->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        ubtrain->TrainsToGenMO();
        local.setAddr(ubtrain);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          ubtrain = (UBTrain*)local.addr;
          if(ubtrain->count == ubtrain->id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(ubtrain->resulttype);
          tuple->PutAttribute(0, 
                       new CcInt(true,ubtrain->id_list[ubtrain->count]));
          tuple->PutAttribute(1, 
                       new CcInt(true,ubtrain->line_id_list[ubtrain->count]));
          tuple->PutAttribute(2,
                      new CcBool(true,ubtrain->direction_list[ubtrain->count]));

          tuple->PutAttribute(3,
                       new GenMO(ubtrain->genmo_list[ubtrain->count]));

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
output the vertices of a region in a correct way.
outer cycle: clockwise, holes: counter clockwise 

*/
string GetRegVertices(Region* reg)
{
  string result;
  if(reg->NoComponents() > 1){
   result = "region should have one face ";
   return result; 
  } 
  result = " ";
  CompTriangle* ct = new CompTriangle(reg);
  unsigned int no_cyc = ct->NoOfCycles();

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

  const double dist_delta = 0.001; 
  for(unsigned int i = 0;i < no_cyc;i++){
        sl_contour[i]->EndBulkLoad();
        vector<MyHalfSegment> mhs;

        sp->ReorderLine(sl_contour[i], mhs);
        vector<Point> ps;
        for(unsigned int j = 0;j < mhs.size();j++){
          if(ps.size() == 0)
            ps.push_back(mhs[j].from);
          else{
            Point p = ps[ps.size() - 1];
            if(p.Distance(mhs[j].from) < dist_delta)
              continue; 
            else
              ps.push_back(mhs[j].from);
          }
        }  


        bool clock;
        if(0.0f < ct->Area(ps)){//points counter-clockwise order
            clock = false;
        }else{// points clockwise
            clock = true;
        }

        if(i == 0){ //outer cycle 
          cout<<"outer cycle "<<endl; 
          if(clock){
            for(unsigned int j = 0;j < ps.size();j++){
              printf("(%f %f)\n",ps[j].GetX(),ps[j].GetY());
            }
          }else{
            for(int j = ps.size() - 1;j >= 0;j--){
              printf("(%f %f)\n",ps[j].GetX(),ps[j].GetY());
            }
          }
          cout<<endl; 
        }else{////////////holes 
          cout<<"holes"<<endl; 
          if(clock == false){
            for(unsigned int j = 0;j < ps.size();j++){
             printf("(%f %f)\n",ps[j].GetX(),ps[j].GetY());
            }
          }else{
            for(int j = ps.size() - 1;j >= 0;j--){
             printf("(%f %f)\n",ps[j].GetX(),ps[j].GetY());
            }
          }
          cout<<endl; 
        }

        delete sl_contour[i];
  }
  delete ct;
  delete sp;

  return result; 
}

/*
output the regino vertex 

*/
int OpTMOutputRegionValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
    result = qp->ResultStorage(in_pSupplier);
    Region* reg = (Region*)args[0].addr;
    ((CcString*)result.addr)->Set(true,GetRegVertices(reg));
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
according to the user requirement, it gets the corresponding number of 
rectangles from the input integer 

*/
int OpTMGetRect2ValueMap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  MaxRect* max_rect;
  switch(message){
      case OPEN:{

        Relation* r = (Relation*)args[0].addr;
        int attr1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
        int attr2 = ((CcInt*)args[8].addr)->GetIntval() - 1; 
        int attr3 = ((CcInt*)args[9].addr)->GetIntval() - 1;
        int attr4 = ((CcInt*)args[10].addr)->GetIntval() - 1; 
        BTree* btree = (BTree*)args[5].addr;

        int no_buildings = ((CcInt*)args[6].addr)->GetIntval(); 

        max_rect = new MaxRect(r);
        max_rect->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        max_rect->GetRectangle(attr1, attr2, attr3, attr4, 
                               btree, no_buildings);
        local.setAddr(max_rect);
        return 0;
      }
      case REQUEST:{
          if(local.addr == NULL) return CANCEL;
          max_rect = (MaxRect*)local.addr;
          if(max_rect->count == max_rect->reg_id_list.size())return CANCEL;

          Tuple* tuple = new Tuple(max_rect->resulttype);
          tuple->PutAttribute(0, 
                       new CcInt(true, max_rect->reg_id_list[max_rect->count]));
          tuple->PutAttribute(1, 
                     new Rectangle<2>(max_rect->rect_list[max_rect->count]));
          tuple->PutAttribute(2,
                      new CcInt(true, max_rect->poly_id_list[max_rect->count]));
          tuple->PutAttribute(3,
                     new CcInt(true, max_rect->reg_type_list[max_rect->count]));

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
get the maximum rectangle from the region relations 

*/
int OpTMGetRect1ValueMap ( Word* args, Word& result, int message,
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

        max_rect->GetRectangle1(attr1, attr2);
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

        max_rect = new MaxRect(r1, r2, btree);
        max_rect->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));

        max_rect->PathToBuilding();
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
                       new Point(max_rect->sp_list[max_rect->count]));
          tuple->PutAttribute(2, 
                       new Point(max_rect->ep_list[max_rect->count]));
          tuple->PutAttribute(3, 
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

Operator test_walk_sp(
    "test_walk_sp",
    OpTMTestWalkSPSpec,
    OpTMTestWalkSPValueMap,
    Operator::SimpleSelect,
    OpTMTestWalkSPTypeMap
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

Operator getadjnode_bg(
  "getadjnode_bg", 
  OpTMGetAdjNodeBGSpec,
  OpTMGetAdjNodeBGValueMap,
  Operator::SimpleSelect,
  OpTMGetAdjNodeBGTypeMap
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


Operator createUBTrains(
  "createUBTrains",
  OpTMCreateUBTrainsSpec,
  OpTMCreateUBahanTrainsValueMap,
  Operator::SimpleSelect,
  OpTMCreateUBTrainsTypeMap
);


Operator create_train_stop(
  "create_train_stop",
  OpTMCreateUBTrainStopSpec,
  OpTMCreateUBahanTrainStopValueMap,
  Operator::SimpleSelect,
  OpTMCreateUBTrainStopTypeMap
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


Operator splitubahn(
  "splitubahn",
  OpTMSplitUBahnSpec,
  OpTMSplitUBahnValueMap,
  Operator::SimpleSelect,
  OpTMSplitUBahnTypeMap
); 


Operator trainstogenmo(
  "trainstogenmo",
  OpTMTrainsToGenMOSpec,
  OpTMTrainsToGenMOValueMap,
  Operator::SimpleSelect,
  OpTMTrainsToGenMOTypeMap
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


Operator outputregion(
  "outputregion",
  OpTMOutputRegionSpec,
  OpTMOutputRegionValueMap,
  Operator::SimpleSelect,
  OpTMOutputRegionTypeMap
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


Operator getrect2(
  "getrect2",  //name 
  OpTMGetRect2Spec, //specification
  OpTMGetRect2ValueMap, //value mapping 
  Operator::SimpleSelect,
  OpTMGetRect2TypeMap //type mapping 
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
    //////////////////////public transportation network////////////////
    AddTypeConstructor( &busstop);
    busstop.AssociateKind("DATA"); 
    AddTypeConstructor( &busroute);
    busroute.AssociateKind("DATA"); 
    AddTypeConstructor( &busnetwork); 
    ////////////////////general data type ////////////////////////
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
    ////operators for partition regions//////////////////////////
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
    AddOperator(&walk_sp);//trip planning for pedestrian 
    AddOperator(&test_walk_sp); 
    AddOperator(&setpave_rid);//set rid value for each pavement 
    AddOperator(&pave_loc_togp);//map pavements locations to gpoints 
    ///////////////////dual graph/////////////////////////////////////
    AddOperator(&zval);//z-order value of a point
    AddOperator(&zcurve);//create a curve for the points sorted by z-order
    AddOperator(&regvertex);
    AddOperator(&triangulation_new);
    AddOperator(&get_dg_edge);
    AddOperator(&getadjnode_dg);
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
    ///////////////rotational plane sweep algorithm////////////////////
    AddOperator(&getallpoints);
    AddOperator(&rotationsweep);
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
    AddOperator(&createbgraph);//create bus network graph 
    AddOperator(&getadjnode_bg); //get neighbor nodes of a given node in bg 

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
    AddOperator(&createUBTrains); //create UBahn Trains 
    AddOperator(&create_train_stop);//create UBahn Train Stops 
    AddOperator(&create_time_table2);//create time table for train stop 
    AddOperator(&create_time_table2_new);//compact storage of train time tables 
    ///////////////////////////////////////////////////////////////////
    ///////////convert berlin trains to genmo///////////////
    AddOperator(&splitubahn); 
    AddOperator(&trainstogenmo); 
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
    ///////////////////////////////////////////////////////////////////////
    //////////////// indoor  navigation //////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    AddOperator(&createdoor3d); //create line3d to denote the doors 
    AddOperator(&createdoorbox); //create a 3d box for each door 
    AddOperator(&createdoor1);//the node relation for the graph (doors)
    AddOperator(&createdoor2);//the node relation for the graph (virtual doors)
    AddOperator(&createadjdoor1); //the edge relation for the graph 
    AddOperator(&createadjdoor2); //the edge relation for the graph 
    AddOperator(&path_in_region);//shortest path between two points inside a reg
    AddOperator(&createigraph);//create indoor graph 
    AddOperator(&getadjnode_ig); //get adjacent node 
    AddOperator(&generate_ip1);//generate indoor positions 
    AddOperator(&generate_mo1);//generate moving objects:int indoor,real:+genmo
    //////////////  indoor navigation ////////////////////////////////////////
    AddOperator(&indoornavigation);indoornavigation.SetUsesArgsInTypeMapping();
    ////////////////////////////////////////////////////////////////////
    /////////////////  others  /////////////////////////////////////////
    AddOperator(&instant2day); 
    AddOperator(&outputregion);//output the vertices in correct order 
    ////////////////////////////////////////////////////////////////////
    //////////////////2D areas for buildings///////////////////////////
    ///////////////////////////////////////////////////////////////////
    AddOperator(&maxrect); //get the maximum rect in a region
    AddOperator(&remove_dirty); //clear dirty data 
    AddOperator(&getrect1); //get all rectangles for buildings 
    AddOperator(&getrect2); //randomly select some of them from input int
    AddOperator(&path_to_building);//connect building entrance and pavement
    /////////non-temporal operators for generic data types////////////////////
    AddOperator(&ref_id); 
    /////////temporal operators for generic data types////////////////////
    AddOperator(&genmodeftime); //get the define time of generic moving objects
    AddOperator(&genmonocomponents); //get number of components
    AddOperator(&lowres);  //low resolution representation 
    AddOperator(&tmtrajectory);  //trajectory 
    AddOperator(&getmode); //get transportation modes
    /////////////////////////////////////////////////////////////////////
    //////////////////space operators/////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    AddOperator(&thespace); //create an empty space

                            //add network infrastructure 
                            //add pavements infrastructure
                            //add bus network infrastructure
                            //add trains network infrastructure
                            //add indoor infrastructure
                            //addgraph(pavement or busnetwork, graph)
   //////////////////////////////////////////////////////////////////////////
   ////////////////////// generate generic moving objects///////////////////
   /////////////////////////////////////////////////////////////////////////
   AddOperator(&genmo_tm_list); 

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
    busnetList = new map<int,string>();
  // The C++ scope-operator :: must be used to qualify the full name
  return new TransportationMode::TransportationModeAlgebra();
    }
