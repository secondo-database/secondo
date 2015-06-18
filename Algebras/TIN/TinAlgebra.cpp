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

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Tin.h"
#include "Stream.h"
#include "../Raster2/stype.h"
#include "../Raster2/sreal.h"

extern NestedList* nl;
extern QueryProcessor *qp;

namespace tin {

int selectTheOne(ListExpr args) {
 return 0;
}

struct atlocationInfo: OperatorInfo {
 atlocationInfo() {
  name = "atlocation";
  signature = "{" + Tin::BasicType() + "|" + TinAttribute::BasicType()
    + "} x " + CcReal::BasicType() + " x " + CcReal::BasicType() + " -> "
    + CcReal::BasicType();
  syntax = "_ atlocation [_, _]";
  meaning = "Returns the height value at location given by"
    " the coordinates x and y. If the point is "
    "not contained in the tin, the result is undefined.";
  example = "query ortetin atlocation[3.6 , 4.1]";

 }
};
ValueMapping atlocationVM[] = { tin::Tin::atlocation_vm,
  tin::TinAttribute::atlocation_vm, 0 };
struct tin2stlfileInfo: OperatorInfo {
 tin2stlfileInfo() {
  name = "tin2stlfile";
  signature = Tin::BasicType() + " x " + CcString::BasicType() + " -> "
    + CcBool::BasicType() + " x (File in .stl format)";
  syntax = "_ tin2stlfile _";
  meaning = "The operator produces a .stl file from the tin in "
    "the first parameter at the path given by the second"
    " parameter. If the file already exists, it will be "
    "overwritten. Thus the tin can be viewed in a CAD tool"
    " or in a viewer like meshlab. If the file is created"
    " successfully TRUE is returned, otherwise FALSE.";
  example = " query ortetin tin2stlfile './mytin.stl'";

 }
};
ValueMapping tin2stlfileVM[] = { tin::Tin::tin2stlfile_vm, 0 };
struct tin2tinattributeInfo: OperatorInfo {
 tin2tinattributeInfo() {
  name = "tin2tinattribute";
  signature = Tin::BasicType() + " ->  stream( tuple ([TinPart : "
    + TinAttribute::BasicType() + " ]))";
  syntax = "_ tin2tinattribute";
  meaning = "Returns a stream of tuples."
    " Each tuple contains just one attribute of "
    "type tinattribute, which represents"
    " a TinPart of the input tin. ";
  example = "query ortetin tin2tinattribute";

 }
};
ValueMapping tin2tinattributeVM[] = { tin::Tin::tin2tinattribute_vm, 0 };
struct tinattribute2tinInfo: OperatorInfo {
 tinattribute2tinInfo() {
  name = "tinattribute2tin";
  signature = "stream (tuple([T:tinattribute])) -> " + Tin::BasicType();
  syntax = "_ tinattribute2tin ";
  meaning = "Converts a stream of tuples with "
    "just a tinattribute to a tin. "
    "The tinattributes are not merged with any operation."
    " They are just collected in a tin. "
    "Thus there can be undetermined behavior "
    "if the tinattributes are overlapping. There are no checks !";
  example = "query tinrel feed project[Tin] tin2tinattribute";

 }
};
ValueMapping tinattribute2tinVM[] = {
  tin::TinAttribute::tinattribute2tin_vm, 0 };

struct unaryOpInfo: OperatorInfo {
 unaryOpInfo() {
  name = "unaryOp";
  signature = Tin::BasicType() + " x (" + CcReal::BasicType() + "-> "
    + CcReal::BasicType() + " ) ->" + Tin::BasicType();
  syntax = "_ unaryOp _";
  meaning = "Returns a new tin with the height values of the given tin "
    "manipulated by the function given with parameter two."
    " The TinParts will have the same"
    " size as configured in the input tin.";
  example = "query (orte feed project[ Ort ] extend [Y: gety( .Ort) ] "
    "sortby [Y desc] createTin 4000) unaryOp [fun(n:real) n + 10.0 ]";
 }
};
ValueMapping unaryOpVM[] = { tin::Tin::unaryOp_vm, 0 };

struct tinminInfo: OperatorInfo {
 tinminInfo() {
  name = "tinmin";
  signature = "{" + Tin::BasicType() + " | " + TinAttribute::BasicType()
    + " } ->" + CcReal::BasicType();
  syntax = " tinmin ( _ )";
  meaning = "Returns the minimum height value of a tin.";
  example = "query tinmin(ortetin) ";
 }
};
ValueMapping tinminVM[] = { tin::Tin::tinmin_vm,
  tin::TinAttribute::tinmin_vm, 0 };

struct tinmaxInfo: OperatorInfo {
 tinmaxInfo() {
  name = "tinmax";
  signature = "{" + Tin::BasicType() + " | " + TinAttribute::BasicType()
    + " } ->" + CcReal::BasicType();
  syntax = " tinmax ( _ )";
  meaning = "Returns the maximum height value of a tin.";
  example = "query tinmax( ortetin ) ";
 }
};
ValueMapping tinmaxVM[] = { tin::Tin::tinmax_vm,
  tin::TinAttribute::tinmax_vm, 0 };
struct raster2tinInfo: OperatorInfo {
 raster2tinInfo() {
  name = "raster2tin";
  signature = "{sreal|sint} x int ->" + Tin::BasicType();
  syntax = "_ raster2tin _ ";
  meaning = "Returns a new tin with all raster cells"
    " as vertices being triangulated (Delaunay triangulation). "
    "The second parameter configures the maximum"
    " size of the TinParts in bytes.";
  example = " robj raster2tin 4000  ";
  remark = "To get tins from a raster with fewer triangles,"
    " one can use contour lines."
    " See operator createTin for more information.";
 }
};
ValueMapping raster2tinVM[] = { tin::Tin::raster2tin_vm<raster2::sint>,
  tin::Tin::raster2tin_vm<raster2::sreal>, 0 };

struct createTinInfo: OperatorInfo {
 createTinInfo() {
  name = "createTin";
  signature = "stream (tuple([P:point,H:{real|int}] )) x "
    + CcInt::BasicType() + " -> " + Tin::BasicType();
  syntax = "_ createTin _";
  meaning = "Returns a tin from a stream of tuples and an integer defining"
    " the size of the TinParts in bytes. The tuples have to consist"
    " of a point attribute (coordinate) and a real|int attribute "
    "(height value). (tuple (point real)) "
    "The points will be triangulated (Delaunay triangulation)."
    " ATTENTION: The points have to be ordered by the y coordinate"
    " in DESCENDING order.";
  example = " ";
  remark =
    "To produce tins with only few triangles from a raster,"
      " one can use operator contourlines from GisAlgebra.let contourrelation "
      "= raster contourlines[40] feed extendstream["
      "V:components(vertices(.Contour)) ]"
      " extend[P:center(.V),H:.Height] extend[Y:gety(.P)]"
      " sortby[Y desc] project[P,H]"
      " consume ; let smalltinfromraster = contourrelation feed createTin 4000";
 }
};
ValueMapping createTinVM[] = { tin::Tin::createTin_vm<CcReal>,
  tin::Tin::createTin_vm<CcInt>, 0 };
struct tin2tuplestreamInfo: OperatorInfo {
 tin2tuplestreamInfo() {
  name = "tin2tuplestream";
  signature = Tin::BasicType() + " -> stream( tuple([V1:point , H1:real ,"
    " V2:point , H2:real, V3:point , H3:real, Part:integer] ))";
  syntax = "_ tin2tuplestream";
  meaning = "Returns a stream of tuples from a tin. Each tuple "
    "represents a triangle and consists of 7 attributes."
    " Two attributes (point and height) for each vertex"
    " of a triangle and an attribute part indicating"
    " the part in which the triangle is contained.";
  example = " query ortetin tin2tuplestream consume";
 }
};
ValueMapping tin2tuplestreamVM[] = { tin::Tin::tin2tuplestream_vm, 0 };

TypeConstructor TinType(tin::Tin::BasicType(), //name
  tin::Tin::Property, //property function describing signature
  tin::Tin::Out, tin::Tin::In, //Out and In functions
  0, 0, //SaveToList and RestoreFromList functions
  tin::Tin::Create, tin::Tin::Delete, //object creation and deletion
  tin::Tin::Open, tin::Tin::Save, tin::Tin::Close, tin::Tin::Clone, 0,
  tin::Tin::sizeOfObj, //sizeof function
  tin::Tin::CheckType); //kind checking function

TypeConstructor TinAttributeType(
  tin::TinAttribute::BasicType(), //name
  tin::TinAttribute::Property, //property function describing signature
  tin::TinAttribute::Out,
  tin::TinAttribute::In, //Out and In functions
  0,
  0, //SaveToList and RestoreFromList functions
  tin::TinAttribute::Create,
  tin::TinAttribute::Delete, //object creation and deletion
  tin::TinAttribute::Open, tin::TinAttribute::Save,
  tin::TinAttribute::Close, tin::TinAttribute::Clone,
  tin::TinAttribute::Cast, //cast function
  tin::TinAttribute::sizeOfObj, //sizeof function
  tin::TinAttribute::CheckType); //kind checking function

class TinAlgebra: public Algebra {
public:
 TinAlgebra() :
   Algebra() {
  AddTypeConstructor(&TinType);
  TinType.AssociateKind(Kind::SIMPLE());

  AddTypeConstructor(&TinAttributeType);
  TinAttributeType.AssociateKind(Kind::DATA());

  AddOperator(atlocationInfo(), atlocationVM, Tin::atlocation_sf,
    Tin::atlocation_tm);
  AddOperator(tin2stlfileInfo(), tin2stlfileVM, selectTheOne,
    Tin::tin2stlfile_tm);
  AddOperator(tinminInfo(), tinminVM, Tin::tinmin_sf, Tin::tinmin_tm);
  AddOperator(tinmaxInfo(), tinmaxVM, Tin::tinmax_sf, Tin::tinmax_tm);
  AddOperator(createTinInfo(), createTinVM, Tin::createTin_sf,
    Tin::createTin_tm);
  AddOperator(unaryOpInfo(), unaryOpVM, selectTheOne, Tin::unaryOp_tm);
  AddOperator(raster2tinInfo(), raster2tinVM, Tin::raster2tin_sf,
    Tin::raster2tin_tm);
  AddOperator(tin2tuplestreamInfo(), tin2tuplestreamVM, selectTheOne,
    Tin::tin2tuplestream_tm);
  AddOperator(tin2tinattributeInfo(), tin2tinattributeVM, selectTheOne,
    Tin::tin2tinattribute_tm);
  AddOperator(tinattribute2tinInfo(), tinattribute2tinVM,
    TinAttribute::tinattribute2tin_sf, TinAttribute::tinattribute2tin_tm);
 }
 ~TinAlgebra() {
 }
};

}
/*

 5 Initialization

*/

extern "C" Algebra*
InitializeTinAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
 nl = nlRef;
 qp = qpRef;
 return (new tin::TinAlgebra());
}

