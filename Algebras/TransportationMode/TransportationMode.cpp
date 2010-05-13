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

const string OpTMMyRegMinusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>region x region-> region</text--->"
    "<text>region myregminus region</text--->"
    "<text>minus the first region by the second region</text--->"
    "<text>query area(partition_regions myregminus partition_regions);"
    "</text--->"
    ") )";

const string OpTMGetPave1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x attr1 x attr2 x attr3->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpave1(network, rel, attr1, attr2, attr3)</text--->"
    "<text>decompose the pavements of one road into a set of subregions"
    "</text--->"
    "<text>query getpave1(n, pave_regions1, oid, pavement1,pavement2);"
    "</text--->"
    ") )";

const string OpTMGetPaveNode1Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>network x rel x btree x attr1 x attr2 x attr3->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpavenode1(network, rel, btree, attr1, attr2 , attr3)</text--->"
    "<text>get the commone area of two pavements</text--->"
    "<text>query getpavenode1(n, subpaves, btree_pave,oid, rid ,pavement);"
    "</text--->"
    ") )";


const string OpTMGetPave2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>int x rel x attr1 x attr2->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpave2(int, rel, attr1, attr2)</text--->"
    "<text>decompose the zebra crossings into a set of subregions"
    "</text--->"
    "<text>query getpave2(subpaves count, pave_regions2, rid, crossreg) count;"
    "</text--->"
    ") )";


const string OpTMGetPaveNode2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>rel1 x rel2 x btree x attr1 x attr2 x attr3->"
    "(stream (tuple( (x1 t1)(x2 t2)...(xn tn))) </text--->"
    "<text>getpavenode2(rel1, rel2, btree, attr1, attr2, attr3)</text--->"
    "<text>get the commone area between zc and pave</text--->"
    "<text>query getpavenode2(subpaves2, subpaves,"
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
typemap fun for myregminus

*/
ListExpr OpTMMyRegMinusTypeMap ( ListExpr args )
{
  if ( nl->ListLength ( args ) != 2 )
  {
    return ( nl->SymbolAtom ( "typeerror" ) );
  }
  ListExpr param1 = nl->First ( args );
  ListExpr param2 = nl->Second(args);

    if (nl->IsAtom(param1) && nl->AtomType(param1) == SymbolType &&
        nl->SymbolValue(param1) == "region" &&
        nl->IsAtom(param2) && nl->AtomType(param2) == SymbolType &&
        nl->SymbolValue(param2) == "region"){

    return nl->SymbolAtom(symbols::REGION);
  }
  return nl->SymbolAtom ( "typeerror" );
}

/*
TypeMap fun for operator getpave1
decompose the pavement of one road into a set of subregions

*/

ListExpr OpTMGetPave1TypeMap ( ListExpr args )
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

ListExpr OpTMGetPaveNode1TypeMap ( ListExpr args )
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
                        nl->TwoElemList(nl->SymbolAtom("node"),
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
TypeMap fun for operator getpave2
decompose the zebra crossing into a set of subregions

*/

ListExpr OpTMGetPave2TypeMap ( ListExpr args )
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

ListExpr OpTMGetPaveNode2TypeMap ( ListExpr args )
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
                        nl->TwoElemList(nl->SymbolAtom("node"),
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

//////////////////////////////////////////////////////////////////////////
static int GeoSpathSelect(ListExpr args)
{
  string t = nl->SymbolValue(nl->Second(args));
  if(t == "point") return 0;
  if(t == "line") return 1;
  return -1;
}

/////////////////////////////////////////////////////////////////////////

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
Value Mapping for the myregminus operator
use the first region minusing the second

*/

int OpTMMyRegMinusmap ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{
  result = qp->ResultStorage(in_pSupplier);
  Region* reg1 = (Region*)args[0].addr;
  Region* reg2 = (Region*)args[1].addr;
  Region* res = (Region*)(result.addr);
  MyMinus(*reg1, *reg2, *res);
  return 0;
}


/*
Value Mapping for the getpave1 operator
decompose the pavement of one road into a set of subregions

*/

int OpTMGetPave1map ( Word* args, Word& result, int message,
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
Value Mapping for the getpavenode1 operator
get the commone area of two intersection pavements

*/

int OpTMGetPaveNode1map ( Word* args, Word& result, int message,
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

        l_partition->GetPavementNode1(n, rel, btree,
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
Value Mapping for the getpave2 operator
decompose the zebra crossings into a set of subregions

*/

int OpTMGetPave2map ( Word* args, Word& result, int message,
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
Value Mapping for the getpavenode2 operator
get the commone area of two intersection pavements

*/

int OpTMGetPaveNode2map ( Word* args, Word& result, int message,
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

        l_partition->GetPavementNode2(rel1, rel2, btree,
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
//        ct->Triangulation();
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
Value Mapping for geospath  operator
return the geometric shortest path for one point and a line where
both are inside the polgyon

*/

int OpTMGeospathmap_l ( Word* args, Word& result, int message,
                         Word& local, Supplier in_pSupplier )
{

  CompTriangle* ct;
  switch(message){
      case OPEN:{
        Point* p = (Point*)args[0].addr;
        Line* sl = (Line*)args[1].addr;
        Region* reg = (Region*)args[2].addr;
        ct = new CompTriangle(reg);
        ct->resulttype =
            new TupleType(nl->Second(GetTupleResultType(in_pSupplier)));
        ct->GeoShortestPath(p, sl);
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

Operator myregminus(
    "myregminus",               // name
    OpTMMyRegMinusSpec,          // specification
    OpTMMyRegMinusmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMMyRegMinusTypeMap        // type mapping
);


Operator getpave1(
    "getpave1",               // name
    OpTMGetPave1Spec,          // specification
    OpTMGetPave1map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPave1TypeMap        // type mapping
);


Operator getpavenode1(
    "getpavenode1",               // name
    OpTMGetPaveNode1Spec,          // specification
    OpTMGetPaveNode1map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPaveNode1TypeMap        // type mapping
);


Operator getpave2(
    "getpave2",               // name
    OpTMGetPave2Spec,          // specification
    OpTMGetPave2map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPave2TypeMap        // type mapping
);

Operator getpavenode2(
    "getpavenode2",               // name
    OpTMGetPaveNode2Spec,          // specification
    OpTMGetPaveNode2map,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGetPaveNode2TypeMap        // type mapping
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

/*Operator geospath(
    "geospath",               // name
    OpTMGeospathSpec,          // specification
    OpTMGeospathmap,  // value mapping
    Operator::SimpleSelect,        // selection function
    OpTMGeospathTypeMap        // type mapping
);*/

ValueMapping TMGeospathMap[]={
OpTMGeospathmap_p,
OpTMGeospathmap_l,
};

Operator geospath(
    "geospath",               // name
    OpTMGeospathSpec,          // specification
    2,
    TMGeospathMap,  // value mapping
    GeoSpathSelect,        // selection function
    OpTMGeospathTypeMap        // type mapping
);

/*
Main Class for Transportation Mode

*/
class TransportationModeAlgebra : public Algebra
{
 public:
  TransportationModeAlgebra() : Algebra()
  {
    ////operators for partition regions////
    AddOperator(&checksline);
    AddOperator(&modifyboundary);
    AddOperator(&segment2region);
    AddOperator(&paveregion);
    AddOperator(&junregion);
    AddOperator(&decomposeregion);
    AddOperator(&fillpavement);
    AddOperator(&myregminus);
    //////////operators for build the graph model on pavement////////////
    AddOperator(&getpave1);
    AddOperator(&getpavenode1);
    AddOperator(&getpave2);
    AddOperator(&getpavenode2);
    AddOperator(&triangulation);
    AddOperator(&convex);
    AddOperator(&geospath);

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
