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
