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
  if(nl->IsEqual(arg1, "line3d") || nl->IsEqual(arg1, "genrange"))
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
ValueMap function for operator length l3d  

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
ValueMap function for operator length l3d  

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
LengthGenRangeValueMap
};


int LengthTMSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "line3d"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genrange"))
    return 1;

  return -1;
}

Operator length_l3d("size",
    SpatialSpecLengthLine3D,
    2,
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
ValueMap function for operator get the reference id 

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

ValueMapping RefIdValueMapVM[]={
  RefIdGenLocValueMap,
  RefIdIORefValueMap
};

int RefIdOpSelect(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "genloc"))
    return 0;
  if(nl->IsAtom(arg1) && nl->IsEqual(arg1, "ioref"))
    return 1;

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
  if(nl->IsEqual(arg1, "genloc") || nl->IsEqual(arg1, "ioref"))
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
     nl->IsEqual(arg1, "genrange") || nl->IsEqual(arg1, "groom"))
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


Operator ref_id("ref_id",
    SpatialSpecRefId,
    2,
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
  GRoomNoComponentsValueMap
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

  return -1;
}

Operator genmonocomponents("no_components", //name 
    SpatialSpecGenMONoComponents, //specification
    4,
    GenMONoComponentsValueMapVM,//value mapping 
    TMNoComponentsOpSelect,
    GenMONoComponentsTypeMap //type mapping 
);

