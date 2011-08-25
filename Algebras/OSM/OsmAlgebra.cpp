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
//[_] [\_]

[1] Implementation of the OSM Algebra

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This implementation file contains the implementation of the class ~OsmAlgebra~.

For more detailed information see OsmAlgebra.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
using namespace std;
#include "OsmAlgebra.h"
#include "AlgebraManager.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Symbols.h"
#include "RelationAlgebra.h"
#include "../Spatial/Geoid.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include "ShpFileReader.h"
#include "ConnCodeFinder.h"
#include "ScalingEngine.h"
#include "OsmImportOperator.h"

// --- Enabling global pointer variables
extern NestedList* nl;
extern QueryProcessor* qp;

// --- Announcing global functions from ImExAlgebra.cpp
//extern string getShpType (const string fname, bool& correct,
//                          string& errorMessage);
//template<int filePos>
//extern int shpimportVM (Word* args, Word& result, int message,
//                        Word& local, Supplier s);

// --- shpimport3-operator
// Specification of operator shpimport3
const string shpimport3Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text -> stream(T), T in {point, points, line, region}</text--->"
    "<text>shpimport3(_)</text--->"
    "<text>Produces a stream of spatial objects from a shapefile. "
    "The spatial result stream element type T is determined "
    "automatically by inspecting the import file.</text--->"
    "<text> query shpimport3('kinos.shp') count</text--->) )";

// Value-mapping-function of operator shpimport3
template<int filePos>
int shpimport3ValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
    //return shpimportVM<filePos> (args, result, message, local, s);
    ShpFileReader* reader = NULL;
    FText* fname = NULL;
    ListExpr type;
    int ret = 0;
    switch(message){
        case OPEN:
            if(local.addr){
                delete (ShpFileReader*)local.addr;
            }
            fname = static_cast<FText*>(args[filePos].addr);
            type = nl->Second(qp->GetType(s));
            local.setAddr(new ShpFileReader(type,fname));
            ret = 0;
            break;
        case REQUEST:
            if(!local.addr){
                ret = CANCEL;
            } else {
                reader = static_cast<ShpFileReader*>(local.addr);
                if(!reader){
                    ret = CANCEL;
                } else {
                    Attribute* next = reader->getNext();
                    result.addr = next;
                    ret = next?YIELD:CANCEL;
                }
            }
            break;
        case CLOSE:
            reader = static_cast<ShpFileReader*>(local.addr);
            if(reader){
                reader->close();
                delete reader;
                local.addr = 0;
            }
            ret = 0;
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}

// Type-mapping-function of operator shpimport3
ListExpr shpimport3TypeMap(ListExpr args){
   if(nl->ListLength(args)!=1){
      return listutils::typeError("one argument expected");
   }
   ListExpr arg = nl->First(args);
   if(nl->ListLength(arg) !=2){
      return listutils::typeError("Error, argument has to consists of 2 parts");
   }
   ListExpr type = nl->First(arg);
   ListExpr value = nl->Second(arg);

   if(!listutils::isSymbol(type,FText::BasicType())){
       return listutils::typeError("text expected");
   }

   // get the value if possible

   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(value),res);
   if(!success){
     return listutils::typeError("could not evaluate the value of  " +
                                  nl->ToString(value) );
   }

   FText* resText = static_cast<FText*>(res.addr);

   if(!resText->IsDefined()){
      resText->DeleteIfAllowed();
       return listutils::typeError("filename evaluated to be undefined");
   }

   string name = resText->GetValue();
   resText->DeleteIfAllowed();

   string shpType;
   bool correct;
   string errmsg;

   shpType = ShpFileReader::getShpType(name, correct, errmsg);
   if(!correct){
      return listutils::typeError(errmsg);
   }
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(shpType));
}

// Instance of operator shpimport3
Operator shpimport3( "shpimport3",
                    shpimport3Spec,
                    shpimport3ValueMap<0>,
                    Operator::SimpleSelect,
                    shpimport3TypeMap);

// --- osmimport-operator
// Specification of operator osmimport
const string osmimportSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text x ('node','way','restriction') -> stream (tuple(...)) "
    "</text--->"
    "<text> osmimport(_,_) </text--->"
    "<text> Produces a stream with the nodes, ways or restrictions from the "
    "specified OSM-file. </text--->"
    "<text> query osmimport('city.osm','node') count </text--->) )";

// Value-mapping-function of operator osmimport
int osmimportValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
    assert (args != NULL);
    OsmImportOperator* importOperator = NULL;
    FText *arg1 = NULL;
    FText *arg2 = NULL;
    result = qp->ResultStorage (s);
    int ret = 0;
    switch (message) {
        case OPEN:
            arg1 = static_cast<FText *>(args[0].addr);
            arg2 = static_cast<FText *>(args[1].addr);
            if (arg1->IsDefined () && arg2->IsDefined ()) {
                importOperator = new OsmImportOperator (arg1->GetValue (),
                    arg2->GetValue (), qp->GetType (s));
            }
            local.setAddr (importOperator);
            ret = 0; 
            break;
        case REQUEST:
            importOperator = static_cast<OsmImportOperator *>(local.addr);
            if (!importOperator) {
                ret = CANCEL;
            } else  {
                result.addr = importOperator->getNext (); 
                ret = (result.addr)? YIELD : CANCEL;
            }
            break;
        case CLOSE:
            importOperator = static_cast<OsmImportOperator *>(local.addr);
            if (importOperator)  {
                delete importOperator;
                local.setAddr (0);
            }
            break;
        default:
            break;
    }
    return ret;
}

// Type-mapping-function of operator osmimport
ListExpr osmimportTypeMap(ListExpr args)
{
    Word res;
    bool success (false);
    std::string elementType;
    ListExpr fileNameArg;
    ListExpr fileNameType;
    ListExpr fileNameValue;
    ListExpr elementTypeArg;
    ListExpr elementTypeType;
    ListExpr elementTypeValue;
    ListExpr attrList;
    ListExpr ret;
    FText* fileName (NULL);

    if(nl->ListLength(args) != 2){
        return listutils::typeError("two arguments expected");
    }

    // --- Testing if a valid file name was passed
    fileNameArg = nl->First(args);
    if(nl->ListLength(fileNameArg) != 2){
        return listutils::typeError(
            "Error, argument has to consists of 2 parts");
    }
    fileNameType = nl->First(fileNameArg);
    fileNameValue = nl->Second(fileNameArg);

    if(!listutils::isSymbol(fileNameType,FText::BasicType())){
        return listutils::typeError("text expected");
    }

    success = QueryProcessor::ExecuteQuery(nl->ToString(fileNameValue),res);
    if(!success){
        return listutils::typeError("could not evaluate the value of  " +
                nl->ToString(fileNameValue) );
    }

    fileName = static_cast<FText*>(res.addr);

    if(!fileName->IsDefined()){
        fileName->DeleteIfAllowed();
        return listutils::typeError("filename evaluated to be undefined");
    }

    fileName->DeleteIfAllowed();

    // --- Checking the element type ("node", "way" or "restriction")
    elementTypeArg = nl->Second(args);
    if(nl->ListLength(elementTypeArg) != 2){
        return listutils::typeError(
            "Error, argument has to consists of 2 parts");
    }
    elementTypeType = nl->First(elementTypeArg);
    elementTypeValue = nl->Second(elementTypeArg);

    if(!listutils::isSymbol(elementTypeType,FText::BasicType())){
        return listutils::typeError("text expected");
    }

    elementType = nl->ToString(elementTypeValue);

    // Creating a list of attributes
    if (elementType == "'node'")  {
        attrList = OsmImportOperator::getOsmNodeAttrList ();
    } else if (elementType == "'way'")  {
        attrList = OsmImportOperator::getOsmWayAttrList ();
    } else if (elementType == "'restriction'")  {
        attrList = OsmImportOperator::getOsmRestrictionAttrList (); 
    } else {
        return listutils::typeError (
            "Error: The passed element type is unknown.");
    }

    ret = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                attrList));
    return ret;
}

// Instance of operator osmimport
Operator osmimport( "osmimport",
                    osmimportSpec,
                    osmimportValueMap,
                    Operator::SimpleSelect,
                    osmimportTypeMap);

// --- getconnectivitycode-operator
// Specification of operator getconnectivitycode
const string getconnectivitycodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> d1 x d2 x d3 x d4 x o1 x o2 x o3 x o4 -> c"
    ", d1, d2, d3, d4, c int, o1, o2, o3, o4 bool</text--->"
    "<text>getconnectivitycode(_)</text--->"
    "<text>Computes and returns the connectivity code as integer value "
    "from the directions and the one-way data of four crossing "
    "sections</text--->"
    "<text> query getconnectivitycode(sec1, sec2, sec3, sec4, ow1, ow2,"
    " ow3, ow4)</text--->))";

// Value-mapping-function of operator getconnectivitycode
int getconnectivitycodeValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   assert (args != NULL);
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   int dir[4] = {0, 0, 0, 0};
   bool ow[4] = {false, false, false, false};
   CcInt *direction = NULL;
   CcBool *oneWay = NULL;
   int iDir = 0;
   int iOw = 0;
   bool foundUndefined = false;
   for (iDir = 0; iDir < 4; ++iDir)  {
      direction = static_cast<CcInt *>(args[iDir].addr);
      if (!direction->IsDefined()) {
         foundUndefined = true;
      }
      dir[iDir] = direction->GetValue ();
   }
   for (iOw = 0; iOw < 4; ++iOw)  {
      oneWay = static_cast<CcBool *>(args[4 + iOw].addr);
      if (!oneWay->IsDefined()) {
         foundUndefined = true;
      }
      ow[iOw] = oneWay->GetValue ();
   }
   if (foundUndefined)  {
      res->SetDefined(false);
   } else  {
      res->Set (true, ConnCodeFinder::getConnectivityCode (
         dir[0],dir[1],dir[2],dir[3],ow[0],ow[1],ow[2],ow[3]));
   }
   return 0;
}

// Type-mapping-function of operator getconnectivitycode
ListExpr getconnectivitycodeTypeMap(ListExpr args){
   assert (args);
   if(nl->ListLength(args) != 8){
      return listutils::typeError("eight arguments expected");
   }
   ListExpr rest = args;
   int i = 0;
   int val = 0;
   while (!nl->IsEmpty (rest)) {
      ListExpr current = nl->First (rest);
      rest = nl->Rest (rest);
      if (nl->ListLength (current) != 2){
         return listutils::typeError("argument has to consists of 2 parts");
      }
      if (i >= 0 && i < 4)  {
         if (!listutils::isSymbol (nl->First(current), CcInt::BasicType ())) {
            return listutils::typeError("int expected");
         }
         //TODO Reconsider the following line!
         val = (nl->Second(current));
         if (val >= 0 && val < 3)  {
            return listutils::typeError("value between zero and two expected");
         }
      } else if ((i >= 4 && i < 8 &&
         !listutils::isSymbol (nl->First(current), CcBool::BasicType ()))){
         return listutils::typeError("bool expected");
      }
      ++i;
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

// Instance of operator getconnectivitycode
Operator getconnectivitycode( "getconnectivitycode",
                    getconnectivitycodeSpec,
                    getconnectivitycodeValueMap,
                    Operator::SimpleSelect,
                    getconnectivitycodeTypeMap);

// --- binor-operator
// Specification of operator binor
const string binorSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> d1 x d2 -> c, d1, d2, c int</text--->"
    "<text>binor(_,_)</text--->"
    "<text>Represents the binary-or-operation of two interger values</text--->"
    "<text> query binor(0,1)</text--->))";

// Value-mapping-function of operator binor
int binorValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   assert (args != NULL);
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   CcInt *arg1 = NULL;
   CcInt *arg2 = NULL;
   int a = 0;
   int b = 0;
   bool foundUndefined = false;
   arg1 = static_cast<CcInt *>(args[0].addr);
   if (!arg1->IsDefined()) {
      foundUndefined = true;
   }
   a = arg1->GetValue ();
   arg2 = static_cast<CcInt *>(args[1].addr);
   if (!arg2->IsDefined()) {
      foundUndefined = true;
   }
   b = arg2->GetValue ();
    
   if (foundUndefined)  {
      res->SetDefined(false);
   } else  {
      res->Set (true,(a|b));
   }
   return 0;
}

// Type-mapping-function of operator binor
ListExpr binorTypeMap(ListExpr args){
   assert (args);
   if(nl->ListLength(args) != 2){
      return listutils::typeError("two arguments expected");
   }
   ListExpr a = nl->First (args);
   ListExpr b = nl->Second (args);
   if (nl->ListLength (a) != 2){
      return listutils::typeError("argument has to consists of 2 parts");
   }
   if (nl->ListLength (b) != 2){
      return listutils::typeError("argument has to consists of 2 parts");
   }
   if (!listutils::isSymbol (nl->First(a), CcInt::BasicType ())) {
      return listutils::typeError("int expected");
   }
   if (!listutils::isSymbol (nl->First(b), CcInt::BasicType ())) {
      return listutils::typeError("int expected");
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

// Instance of operator binor
Operator binor( "binor",
                binorSpec,
                binorValueMap,
                Operator::SimpleSelect,
                binorTypeMap);

// --- getscalefactorx-operator
// Specification of operator getscalefactorx
const string getscalefactorxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> -> int</text--->"
    "<text>getscalefactorx()</text--->"
    "<text>Returns the currently set scale factor for x coordinates.</text--->"
    "<text> query getscalefactorx()</text--->))";

// Value-mapping-function of operator getscalefactorx
int getscalefactorxValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   assert (args != NULL);
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   res->Set (true,ScalingEngine::getInstance ().getScaleFactorX ());
   return 0;
}

// Type-mapping-function of operator getscalefactorx
ListExpr getscalefactorxTypeMap(ListExpr args){
   if(args){
      return listutils::typeError("no arguments expected");
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

// Instance of operator getscalefactorx
Operator getscalefactorx( "getscalefactorx",
                getscalefactorxSpec,
                getscalefactorxValueMap,
                Operator::SimpleSelect,
                getscalefactorxTypeMap);

// --- getscalefactory-operator
// Specification of operator getscalefactory
const string getscalefactorySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> -> int</text--->"
    "<text>getscalefactory()</text--->"
    "<text>Returns the currently set scale factor for y coordinates.</text--->"
    "<text> query getscalefactory()</text--->))";

// Value-mapping-function of operator getscalefactory
int getscalefactoryValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   assert (args != NULL);
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   res->Set (true,ScalingEngine::getInstance ().getScaleFactorY ());
   return 0;
}

// Type-mapping-function of operator getscalefactory
ListExpr getscalefactoryTypeMap(ListExpr args){
   if(args){
      return listutils::typeError("no arguments expected");
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

// Instance of operator getscalefactory
Operator getscalefactory( "getscalefactory",
                getscalefactorySpec,
                getscalefactoryValueMap,
                Operator::SimpleSelect,
                getscalefactoryTypeMap);

// --- setscalefactorx-operator
// Specification of operator setscalefactorx
const string setscalefactorxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> int -> int</text--->"
    "<text>setscalefactorx(_)</text--->"
    "<text>Sets the scale factor for x coordinates and returns the new "
    "value.</text--->"
    "<text> query setscalefactorx(2000)</text--->))";

// Value-mapping-function of operator setscalefactorx
int setscalefactorxValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   assert (args != NULL);
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   CcInt *arg = static_cast<CcInt *>(args[0].addr);
   if (!arg->IsDefined()) {
      res->SetDefined(false);
   } else  {
      ScalingEngine::getInstance ().setScaleFactorX (arg->GetValue ());
      res->Set (true, ScalingEngine::getInstance ().getScaleFactorX ());
   }
   return 0;
}

// Type-mapping-function of operator setscalefactorx
ListExpr setscalefactorxTypeMap(ListExpr args){
   assert (args);
   if(nl->ListLength(args) != 1){
      return listutils::typeError("one argument expected");
   }
   ListExpr arg = nl->First (args);
   if (nl->ListLength (arg) != 2 ||
       !listutils::isSymbol (nl->First (arg), CcInt::BasicType ())) {
      return listutils::typeError("wrapped int expected");
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

// Instance of operator setscalefactorx
Operator setscalefactorx( "setscalefactorx",
                setscalefactorxSpec,
                setscalefactorxValueMap,
                Operator::SimpleSelect,
                setscalefactorxTypeMap);

// --- setscalefactory-operator
// Specification of operator setscalefactory
const string setscalefactorySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> int -> int</text--->"
    "<text>setscalefactory(_)</text--->"
    "<text>Sets the scale factor for y coordinates and returns the new "
    "value.</text--->"
    "<text> query setscalefactory(2000)</text--->))";

// Value-mapping-function of operator setscalefactory
int setscalefactoryValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   assert (args != NULL);
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   CcInt *arg = static_cast<CcInt *>(args[0].addr);
   if (!arg->IsDefined()) {
      res->SetDefined(false);
   } else  {
      ScalingEngine::getInstance ().setScaleFactorY (arg->GetValue ());
      res->Set (true, ScalingEngine::getInstance ().getScaleFactorY ());
   }
   return 0;
}

// Type-mapping-function of operator setscalefactory
ListExpr setscalefactoryTypeMap(ListExpr args){
   assert (args);
   if(nl->ListLength(args) != 1){
      return listutils::typeError("one argument expected");
   }
   ListExpr arg = nl->First (args);
   if (nl->ListLength (arg) != 2 ||
       !listutils::isSymbol (nl->First (arg), CcInt::BasicType ())) {
      return listutils::typeError("wrapped int expected");
   }
   return nl->SymbolAtom(CcInt::BasicType());
}

// Instance of operator setscalefactory
Operator setscalefactory( "setscalefactory",
                setscalefactorySpec,
                setscalefactoryValueMap,
                Operator::SimpleSelect,
                setscalefactoryTypeMap);

// --- Constructors
// Constructor
osm::OsmAlgebra::OsmAlgebra () : Algebra ()
{
    AddOperator(&shpimport3);
    shpimport3.SetUsesArgsInTypeMapping();;
    AddOperator(&getconnectivitycode);
    getconnectivitycode.SetUsesArgsInTypeMapping();;
    AddOperator(&binor);
    binor.SetUsesArgsInTypeMapping();;
    AddOperator(&getscalefactorx);
    getscalefactorx.SetUsesArgsInTypeMapping();;
    AddOperator(&getscalefactory);
    getscalefactory.SetUsesArgsInTypeMapping();;
    AddOperator(&setscalefactorx);
    setscalefactorx.SetUsesArgsInTypeMapping();;
    AddOperator(&setscalefactory);
    setscalefactory.SetUsesArgsInTypeMapping();;
    AddOperator(&osmimport);
    osmimport.SetUsesArgsInTypeMapping();;
}

// Destructor
osm::OsmAlgebra::~OsmAlgebra ()
{
    // empty
}

// --- Global initialization function
extern "C"
Algebra*
InitializeOsmAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new osm::OsmAlgebra());
}
