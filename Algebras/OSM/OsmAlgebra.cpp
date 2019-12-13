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

[2] operator fullosmimport added

Jan-Feb 2012, Fabio Vald\'{e}s

[3] operator divide\_osm added

July 2013, Fabio Vald\'{e}s

[4] operator convertstreets added

October 2013, Fabio Vald\'{e}s

[TOC]

1 Overview

This implementation file contains the implementation of the class ~OsmAlgebra~.

For more detailed information see OsmAlgebra.h.

2 Defines and Includes

*/
#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

/*
Including header-files

*/

// using namespace std;
#include "OsmAlgebra.h"
#include "AlgebraManager.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Symbols.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "ShpFileReader.h"
#include "ConnCodeFinder.h"
#include "ScalingEngine.h"
#include "OsmImportOperator.h"
#include "SecondoCatalog.h"
#include "XmlFileReader.h"
#include "XmlParserInterface.h"
#include "Element.h"
#include "WinUnix.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <iostream>
#include <string>
#include <fstream>


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
const std::string shpimport3Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text -> stream(T), T in {point, points, line, region}</text--->"
    "<text>shpimport3(_)</text--->"
    "<text>Produces a stream of spatial objects from a shapefile. "
    "The spatial result stream element type T is determined "
    "automatically by inspecting the import file.</text--->"
    "<text> query shpimport3('kinos.shp') count</text--->) )";

/*
Value-mapping-function of operator shpimport3

*/
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

/*
Type-mapping-function of operator shpimport3

*/
ListExpr shpimport3TypeMap(ListExpr args){
   if(nl->ListLength(args)!=1){
      return listutils::typeError("one argument expected");
   }
   ListExpr arg = nl->First(args);
   if(nl->ListLength(arg) !=2){
      return listutils::typeError("Error, argument has to consist of 2 parts");
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

   std::string name = resText->GetValue();
   resText->DeleteIfAllowed();

   std::string shpType;
   bool correct;
   std::string errmsg;

   shpType = ShpFileReader::getShpType(name, correct, errmsg);
   if(!correct){
      return listutils::typeError(errmsg);
   }
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(shpType));
}

/*
Instance of operator shpimport3

*/
Operator shpimport3( "shpimport3",
                    shpimport3Spec,
                    shpimport3ValueMap<0>,
                    Operator::SimpleSelect,
                    shpimport3TypeMap);

// --- osmimport-operator
// Specification of operator osmimport
const std::string osmimportSpec  =
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
const std::string getconnectivitycodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> d1 x d2 x d3 x d4 x o1 x o2 x o3 x o4 -> c"
    ", d1, d2, d3, d4, c int, o1, o2, o3, o4 bool</text--->"
    "<text>getconnectivitycode(_)</text--->"
    "<text>Computes and returns the connectivity code as integer value "
    "from the directions and the one-way data of four crossing "
    "sections</text--->"
    "<text> query getconnectivitycode(sec1, sec2, sec3, sec4, ow1, ow2,"
    " ow3, ow4)</text--->))";

/*
Value-mapping-function of operator getconnectivitycode

*/
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
   for (iDir = 0; !foundUndefined && iDir < 4; ++iDir)  {
      direction = static_cast<CcInt *>(args[iDir].addr);
      if (!direction->IsDefined()) {
         foundUndefined = true;
      } else  {
         dir[iDir] = direction->GetValue ();
      }
   }
   for (iOw = 0; !foundUndefined && iOw < 4; ++iOw)  {
      oneWay = static_cast<CcBool *>(args[4 + iOw].addr);
      if (!oneWay->IsDefined()) {
         foundUndefined = true;
      } else  {
         ow[iOw] = oneWay->GetValue ();
      }
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
const std::string binorSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> d1 x d2 -> c, d1, d2, c int</text--->"
    "<text>binor(_,_)</text--->"
    "<text>Represents the binary-or-operation of two interger values</text--->"
    "<text> query binor(0,1)</text--->))";

/*
Value-mapping-function of operator binor

*/
int binorValueMap(Word* args, Word& result, int message,
        Word& local, Supplier s){
   result = qp->ResultStorage (s);
   CcInt *res = static_cast<CcInt*>(result.addr);;
   CcInt* arg1 = static_cast<CcInt *>(args[0].addr);
   CcInt* arg2 = static_cast<CcInt *>(args[1].addr);
   if(!arg1->IsDefined() || !arg2->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   int a = arg1->GetValue ();
   int b = arg2->GetValue ();
   res->Set(true, (a | b));
   return 0;
}

// Type-mapping-function of operator binor
ListExpr binorTypeMap(ListExpr args){
   if(nl->ListLength(args) != 2){
      return listutils::typeError("two arguments expected");
   }
   ListExpr a = nl->First (args);
   ListExpr b = nl->Second (args);
   if(!CcInt::checkType(a) || !CcInt::checkType(b)){
     return listutils::typeError("intx int expected");
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
const std::string getscalefactorxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> -> int</text--->"
    "<text>getscalefactorx()</text--->"
    "<text>Returns the currently set scale factor for x coordinates.</text--->"
    "<text> query getscalefactorx()</text--->))";

/*
Value-mapping-function of operator getscalefactorx

*/
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
const std::string getscalefactorySpec  =
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
const std::string setscalefactorxSpec  =
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
const std::string setscalefactorySpec  =
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

/*
Instance of operator setscalefactory

*/
Operator setscalefactory( "setscalefactory",
                setscalefactorySpec,
                setscalefactoryValueMap,
                Operator::SimpleSelect,
                setscalefactoryTypeMap);

/*
1 Operator fullosmimport

*/

/*
1.1 Specification of operator fullosmimport

*/
const std::string fullosmimportSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text x string (x int) -> bool</text--->"
    "<text>fullosmimport(_)</text--->"
    "<text>imports an osm file into six new relations with an optional integer"
    " appended to the name</text--->"
    "<text>query fullosmimport('dortmund.osm', \"dortmund\")</text--->))";

/*
1.2 Type mapping function for operator fullosmimport

*/
ListExpr fullosmimportTypeMap(ListExpr args) {
  if (!nl->HasLength(args, 2) && !nl->HasLength(args, 3)) {
    return listutils::typeError("two or three arguments expected");
  }
  if (!listutils::isSymbol(nl->First(args), FText::BasicType())) {
    return listutils::typeError("type text required for the first argument");
  }
  if (!listutils::isSymbol(nl->First(nl->Rest(args)), CcString::BasicType())) {
    return listutils::typeError("type string required for the 2nd argument");  
  }
  if (nl->HasLength(args, 3)) {
    if (!listutils::isSymbol(nl->Third(args), CcInt::BasicType())) {
      return listutils::typeError("type int required for the 3rd argument");
    }
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.3 Constructor and functions of class FullOsmImport

*/
FullOsmImport::FullOsmImport(const std::string& fileName, 
                          const std::string& prefix, const int suf /* = -1 */) {
  sc = SecondoSystem::GetCatalog();
  isTemp = false;
  reader = 0;
  std::string suffix = "";
  if (suf > -1) {
    std::ostringstream strstr;
    strstr << "_" << suf;
    suffix = strstr.str();
  }
  relationsInitialized = initRelations(prefix, suffix, true);
  if(!relationsInitialized) {
    cout << "relations could not be initialized" << endl;
    return;
  }
  fileOk = openFile(fileName);
  if (!fileOk) {
    cout << "file could not be initialized" << endl;
    return;
  }
  defineRelations(true);
  node = 0;
  tag = 0;
  tagged = false;
  way = 0;
  rel = 0;
  fillRelations();
  storeRelations(true);
}

FullOsmImport::~FullOsmImport() {}

bool FullOsmImport::initRelations(const std::string& prefix, 
                                  const std::string& suffix, bool all) {
  relNames[0] = prefix + "Nodes" + suffix;
  relNames[1] = prefix + "NodeTags" + suffix;
  relNames[2] = prefix + "Ways" + suffix;
  relNames[3] = prefix + "WayTags" + suffix;
  relNames[4] = prefix + "Relations" + suffix;
  relNames[5] = prefix + "RelationTags" + suffix;
  for (int i = 0; i < 6; i++) { // check the new relations' names
    if (all || (i == 0 || i == 2 || i == 3)) {
      if (sc->IsObjectName(relNames[i])) {
        cout << relNames[i] << " is already defined" << endl;
        return false;
      }
      std::string errMsg = "error";
      if (!sc->IsValidIdentifier(relNames[i], errMsg, true)) {
        cout << errMsg << endl;
        return false;
      }
      if (sc->IsSystemObject(relNames[i])) {
        cout << relNames[i] << " is a reserved name" << endl;
        return false;
      }
    }
  }
  
  return true;
} 

bool FullOsmImport::openFile(const std::string& fileName) {
  // check whether the file can be opened and is an osm file
  reader = xmlReaderForFile(fileName.c_str(), NULL, 0);
  if (reader == NULL) {
    cout << "file " << fileName << " cannot be opened" << endl;
    return false;
  }
  int read = xmlTextReaderRead(reader);
  if (read < 1) {
    cout << "file " << fileName << " is empty" << endl;
    xmlFreeTextReader(reader);
    return false;
  }
  // strcmp(x1, x2) == 0  <==> x1 == x2
  if (strcmp((char *)xmlTextReaderConstName(reader), "osm")) {
    cout << "root node of " << fileName << " has to be \"osm\"" << endl;
    xmlFreeTextReader(reader);
    return false;
  }
  cout << "file " << fileName << " opened successfully" << endl;
  return true;
}

void FullOsmImport::defineRelations(bool all) {
  nodeTypeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
      nl->ThreeElemList(nl->TwoElemList(nl->SymbolAtom("NodeId"), 
                                      nl->SymbolAtom(LongInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Lat"),
                                      nl->SymbolAtom(CcReal::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Lon"),
                                      nl->SymbolAtom(CcReal::BasicType()))));
  numNodeTypeInfo = sc->NumericType(nodeTypeInfo);
  nodeType = new TupleType(numNodeTypeInfo);
  nodeRel = new Relation(nodeType, isTemp);
  
  if (all) {
    nodeTagTypeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
        nl->ThreeElemList(nl->TwoElemList(nl->SymbolAtom("NodeIdInTag"),
                                          nl->SymbolAtom(LongInt::BasicType())),
                          nl->TwoElemList(nl->SymbolAtom("NodeTagKey"),
                                          nl->SymbolAtom(FText::BasicType())),
                          nl->TwoElemList(nl->SymbolAtom("NodeTagValue"),
                                          nl->SymbolAtom(FText::BasicType()))));
    numNodeTagTypeInfo = sc->NumericType(nodeTagTypeInfo);
    nodeTagType = new TupleType(numNodeTagTypeInfo);
    nodeTagRel = new Relation(nodeTagType, isTemp);
  }
  
  wayTypeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
      nl->ThreeElemList(nl->TwoElemList(nl->SymbolAtom("WayId"), 
                                        nl->SymbolAtom(LongInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("NodeCounter"),
                                        nl->SymbolAtom(CcInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("NodeRef"),
                                        nl->SymbolAtom(LongInt::BasicType()))));
  numWayTypeInfo = sc->NumericType(wayTypeInfo);
  wayType = new TupleType(numWayTypeInfo);
  wayRel = new Relation(wayType, isTemp);
  
  wayTagTypeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
      nl->ThreeElemList(nl->TwoElemList(nl->SymbolAtom("WayIdInTag"), 
                                        nl->SymbolAtom(LongInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("WayTagKey"),
                                        nl->SymbolAtom(FText::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("WayTagValue"),
                                        nl->SymbolAtom(FText::BasicType()))));
  numWayTagTypeInfo = sc->NumericType(wayTagTypeInfo);
  wayTagType = new TupleType(numWayTagTypeInfo);
  wayTagRel = new Relation(wayTagType, isTemp);
  
  if (all) {
    relTypeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
        nl->FiveElemList(nl->TwoElemList(nl->SymbolAtom("RelId"),
                                         nl->SymbolAtom(LongInt::BasicType())),
                         nl->TwoElemList(nl->SymbolAtom("RefCounter"),
                                         nl->SymbolAtom(CcInt::BasicType())),
                         nl->TwoElemList(nl->SymbolAtom("MemberType"),
                                         nl->SymbolAtom(FText::BasicType())),
                         nl->TwoElemList(nl->SymbolAtom("MemberRef"),
                                         nl->SymbolAtom(LongInt::BasicType())),
                         nl->TwoElemList(nl->SymbolAtom("MemberRole"),
                                         nl->SymbolAtom(FText::BasicType()))));
    numRelTypeInfo = sc->NumericType(relTypeInfo);
    relType = new TupleType(numRelTypeInfo);
    relRel = new Relation(relType, isTemp);
  
    relTagTypeInfo = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
        nl->ThreeElemList(nl->TwoElemList(nl->SymbolAtom("RelIdInTag"),
                                          nl->SymbolAtom(LongInt::BasicType())),
                          nl->TwoElemList(nl->SymbolAtom("RelTagKey"),
                                          nl->SymbolAtom(FText::BasicType())),
                          nl->TwoElemList(nl->SymbolAtom("RelTagValue"),
                                          nl->SymbolAtom(FText::BasicType()))));
    numRelTagTypeInfo = sc->NumericType(relTagTypeInfo);
    relTagType = new TupleType(numRelTagTypeInfo);
    relTagRel = new Relation(relTagType, isTemp);
  }
}

void FullOsmImport::processNode(xmlTextReaderPtr reader) {
  int attrCount = 0;
  currentId = 0;
  xmlChar *id, *lat, *lon, *subNameXml;
  id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
  if (id != NULL) {
    currentId.ReadFrom((const char*)id);
    node->PutAttribute(0, new LongInt(currentId));
    xmlFree(id);
    lat = xmlTextReaderGetAttribute(reader, (xmlChar *)"lat");
    if (lat != NULL) {
      node->PutAttribute(1, new CcReal(true, OsmImportOperator::convStrToDbl
          ((const char*)lat)));
      attrCount++;
      xmlFree(lat);
    }
    lon = xmlTextReaderGetAttribute(reader, (xmlChar *)"lon");
    if (lon != NULL) {
      node->PutAttribute(2, new CcReal(true, OsmImportOperator::convStrToDbl
          ((const char *)lon)));
      attrCount++;
      xmlFree(lon);
    }
    // ~ŧagged~ solves the node duplication problem. Otherwise, every tagged
    // node would be read twice
    if (attrCount == 2 && !tagged) {
      nodeRel->AppendTuple(node);
      tupleCount[0]++;
    }
    tagged = false;
    read = xmlTextReaderRead(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    std::string subName = (char *)subNameXml;
    xmlFree(subNameXml);
    while (subName == "tag") {
      processTag(reader, NODE);
      subNameXml = xmlTextReaderLocalName(reader);
      subName = (char *)subNameXml;
      xmlFree(subNameXml);
    }
  }
}

void FullOsmImport::processWay(xmlTextReaderPtr reader) {
  currentId = 0;
  refCount = 0;
  xmlChar *id, *subNameXml;
  id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
  if (id != NULL) {
    currentId.ReadFrom((const char*)id);
    xmlFree(id);
    read = xmlTextReaderRead(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    std::string subName = (char *)subNameXml;
    xmlFree(subNameXml);
    while (subName == "nd") {
      processWayNodeRef(reader);
      subNameXml = xmlTextReaderLocalName(reader);
      subName = (char *)subNameXml;
      xmlFree(subNameXml);
    }
    while (subName == "tag") {
      processTag(reader, WAY);
      subNameXml = xmlTextReaderLocalName(reader);
      subName = (char *)subNameXml;
      xmlFree(subNameXml);
    }
  }
}

void FullOsmImport::processWayNodeRef(xmlTextReaderPtr reader) {
  int attrCount = 0;
  xmlChar *nodeRef;
  way->PutAttribute(0, new LongInt(currentId));
  nodeRef = xmlTextReaderGetAttribute(reader, (xmlChar *)"ref");
  if (nodeRef != NULL) {
    way->PutAttribute(1, new CcInt(true, refCount));
    way->PutAttribute(2, new LongInt(std::string((const char*)nodeRef)));
    refCount++;
    attrCount++;
    xmlFree(nodeRef);
  }
  if (attrCount) {
    wayRel->AppendTuple(way);
    tupleCount[2]++;
  }
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
}

void FullOsmImport::processRel(xmlTextReaderPtr reader) {
  currentId = 0;
  refCount = 0;
  xmlChar *id, *subNameXml;
  id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
  if (id != NULL) {
    currentId.ReadFrom((const char*)id);
    xmlFree(id);
    read = xmlTextReaderRead(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    std::string subName = (char *)subNameXml;
    xmlFree(subNameXml);
    while (subName == "member") {
      processRelMemberRef(reader);
      subNameXml = xmlTextReaderLocalName(reader);
      subName = (char *)subNameXml;
      xmlFree(subNameXml);
    }
    while (subName == "tag") {
      processTag(reader, RELATION);
      subNameXml = xmlTextReaderLocalName(reader);
      subName = (char *)subNameXml;
      xmlFree(subNameXml);
    }
  }
}

void FullOsmImport::processRelMemberRef(xmlTextReaderPtr reader) {
  int attrCount = 0;
  xmlChar *memberRef, *type, *role;
  rel->PutAttribute(0, new LongInt(currentId));
  memberRef = xmlTextReaderGetAttribute(reader, (xmlChar *)"ref");
  if (memberRef != NULL) {
    rel->PutAttribute(1, new CcInt(true, refCount));
    rel->PutAttribute(3, new LongInt(std::string((const char*)memberRef)));
    refCount++;
    attrCount++;
    xmlFree(memberRef);
  }
  type = xmlTextReaderGetAttribute(reader, (xmlChar *)"type");
  if (type != NULL) {
    rel->PutAttribute(2, new FText(true, (char *)type));
    attrCount++;
    xmlFree(type);
  }
  role = xmlTextReaderGetAttribute(reader, (xmlChar *)"role");
  if (role != NULL) {
    rel->PutAttribute(4, new FText(true, (char *)role));
    attrCount++;
    xmlFree(role);
  }
  if (attrCount == 3) {
    relRel->AppendTuple(rel);
    tupleCount[4]++;
  }
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
}


void FullOsmImport::processTag(xmlTextReaderPtr reader, entityKind kind) {
  int attrCount = 0;
  xmlChar *key, *value;
  key = xmlTextReaderGetAttribute(reader, (xmlChar *)"k");
  Relation *currentRel = 0;
  int counterPos = -1;
  switch (kind) {
    case NODE:
      currentRel = nodeTagRel;
      counterPos = 1;
      break;
    case WAY:
      currentRel = wayTagRel;
      counterPos = 3;
      break;
    case RELATION:
      currentRel = relTagRel;
      counterPos = 5;
      break;
    default:
      assert(false);
  }  
  tag->PutAttribute(0, new LongInt(currentId));
  if (key != NULL) {
    tag->PutAttribute(1, new FText(true, (char *)key));
    attrCount++;
    xmlFree(key);
    value = xmlTextReaderGetAttribute(reader, (xmlChar *)"v");
    if (value != NULL) {
      tag->PutAttribute(2, new FText(true, (char *)value));
      attrCount++;
      xmlFree(value);
    }
    if (attrCount == 2) {
      currentRel->AppendTuple(tag);
      tagged = true;
      tupleCount[counterPos]++;
    }
  }
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
}

void FullOsmImport::fillRelations() {
  xmlChar *subNameXml;
  node = new Tuple(nodeType);
  tag = new Tuple(nodeTagType);
  way = new Tuple(wayType);
  rel = new Tuple(relType);
  memset(tupleCount, 0 , 6 * sizeof(int));
  std::string currentName = "undefined string";
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  
  while ((read == 1) && (next == 1)) {
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char *)subNameXml;
    xmlFree(subNameXml);
    if (currentName == "node") {
      processNode(reader);
    }
    else if (currentName == "way") {
      processWay(reader);
    }
    else if (currentName == "relation") {
      processRel(reader);
    }
    else {
      read = xmlTextReaderRead(reader);
      next = xmlTextReaderNext(reader);
    }
  }
  // clean up the memory
  xmlFreeTextReader(reader);
  xmlCleanupParser();
  node->DeleteIfAllowed();
  tag->DeleteIfAllowed();
  way->DeleteIfAllowed();
  rel->DeleteIfAllowed();
  nodeType->DeleteIfAllowed();
  nodeTagType->DeleteIfAllowed();
  wayType->DeleteIfAllowed();
  wayTagType->DeleteIfAllowed();
  relType->DeleteIfAllowed();
  relTagType->DeleteIfAllowed();
}

void FullOsmImport::storeRelations(bool all) {
  storeRel(relNames[0], nodeTypeInfo, nodeRel);
  storeRel(relNames[2], wayTypeInfo, wayRel);
  storeRel(relNames[3], wayTagTypeInfo, wayTagRel);
  if (all) {
    storeRel(relNames[1], nodeTagTypeInfo, nodeTagRel);
    storeRel(relNames[4], relTypeInfo, relRel);
    storeRel(relNames[5], relTagTypeInfo, relTagRel);
  }
  for (int i = 0; i < 6; i++) {
    if (all || (i == 0 || i == 2 || i == 3)) {
      cout << "relation " << relNames[i] << " with " << tupleCount[i]
           << " tuples stored" << endl;
    }
  }
}

void FullOsmImport::storeRel(std::string name, ListExpr typeInfo, 
                             Relation *rel) {
  ListExpr type = nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()),
                                  typeInfo);
  Word relWord;
  relWord.setAddr(rel);
  sc->InsertObject(name, "", type, relWord, true);
}

/*
1.4 Selection function for operator fullosmimport

*/
int fullosmimportSelect(ListExpr args) {
  return (nl->HasLength(args, 3) ? 1 : 0);
}

/*
1.4 Value mapping function for operator fullosmimport

*/
template<int noArgs>
int fullosmimportValueMap(Word* args, Word& result, int message, Word& local,
                          Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  FText *file = (FText *)(args[0].addr);
  if (!file->IsDefined()) {
    res->Set(true, false);
    return 0;
  }
  std::string fileName = file->GetValue();
  CcString *pr = (CcString*)args[1].addr;
  if (!pr->IsDefined()) {
    res->Set(true, false);
    return 0;
  }
  std::string prefix = pr->GetValue();
  if (noArgs == 3) {
    CcInt *suf = (CcInt*)args[2].addr;
    if (!suf->IsDefined()) {
      res->Set(true, false);
      return 0;
    }
    if (suf->GetValue() < 0) {
      res->Set(true, false);
      return 0;
    }
    int suffix = suf->GetValue();
    FullOsmImport fullOsmImport(fileName, prefix, suffix);
    res->Set(true, fullOsmImport.relationsInitialized && fullOsmImport.fileOk);
  }
  else {
    FullOsmImport fullOsmImport(fileName, prefix);
    res->Set(true, fullOsmImport.relationsInitialized && fullOsmImport.fileOk);
  }  
  return 0;
}

ValueMapping fullosmimportVMs[] = {fullosmimportValueMap<2>, 
                                   fullosmimportValueMap<3>};

/*
1.5 Operator instance

*/
Operator fullosmimport( "fullosmimport",
                fullosmimportSpec,
                2,
                fullosmimportVMs,
                fullosmimportSelect,
                fullosmimportTypeMap);

/*
\section{Operator ~convertstreets~}

\subsection{Type Mapping}

*/
ListExpr convertstreetsTM(ListExpr args) {
  std::string err = "Expected syntax: stream(tuple(..., x: line, ...)) x IDENT "
               "x string";
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  ListExpr stream = nl->First(args);
  ListExpr anlist = nl->Second(args);
  if (!CcString::checkType(nl->Third(args))) {
    return listutils::typeError(err);
  }
  if (!Stream<Tuple>::checkType(stream) || !listutils::isSymbol(anlist)){
    return listutils::typeError(err);
  }
  std::string attr = nl->SymbolValue(anlist);
  ListExpr attrlist = nl->Second(nl->Second(stream));
  ListExpr type;
  int index = listutils::findAttribute(attrlist, attr, type);
  if (!index) {
    return listutils::typeError("attribute " + attr + " not found in tuple");
  }
  if (!Line::checkType(type)) {
    return listutils::typeError("wrong type " + nl->ToString(type)
                                + " of attritube " + attr);
  }
  ListExpr attrs = nl->OneElemList(nl->StringAtom(nl->SymbolValue
                                  (nl->First(nl->First(attrlist)))));
  ListExpr lastAttr = attrs;
  ListExpr types = nl->OneElemList(nl->TextAtom( nl->SymbolValue
                                  (nl->Second(nl->First(attrlist)))));
  ListExpr lastType = types;
  attrlist = nl->Rest(attrlist);
  while (!nl->IsEmpty(attrlist)) {
    ListExpr first = nl->First(attrlist);
    ListExpr firstfirst = nl->First(first);
    ListExpr firstsecond = nl->Second(first);
    lastAttr = nl->Append(lastAttr, nl->StringAtom
                                    (nl->SymbolValue(firstfirst)));
    lastType = nl->Append(lastType, nl->TextAtom(nl->ToString(firstsecond)));
    attrlist = nl->Rest(attrlist);
  }
  ListExpr infolist1 = nl->OneElemList(nl->IntAtom(index - 1));
  ListExpr infolist2 = listutils::concat(infolist1, attrs);
  ListExpr infolist = listutils::concat(infolist2, types);
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), infolist,
                           nl->SymbolAtom(CcBool::BasicType()));
}

/*
subsection{Specification}

*/
const std::string convertstreetsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> stream(tuple(..., x:line, ...)) x IDENT x string -> bool</text--->"
    "<text>_ convertstreets[ _ , _ ]</text--->"
    "<text>Converts BerlinMOD streets into Node and Way relations.</text--->"
    "<text>query strassen feed convertstreets[GeoData, \"Berlin\"]</text--->))";

FullOsmImport::FullOsmImport(const std::string& prefix) :
         isTemp(false), reader(0), nodeRel(0), wayRel(0), wayTagRel(0), node(0),
         tag(0), way(0), rel(0), currentId(0), tagged(false), newWay(true) {
  for (int i = 0; i < 6; i++) {
    tupleCount[i] = 0;
  }
  sc = SecondoSystem::GetCatalog();
  if(!initRelations(prefix, "", false)) {
    cout << "relations could not be initialized" << endl;
    return;
  }
  defineRelations(false);
}

void FullOsmImport::insertNodes(std::list<Point> &points, LongInt &wayId,
                                Tuple *tuple, Word *args) {
  if (points.empty()) {
    return;
  }
  int refCounter = 0;
  for (std::list<Point>::iterator it= points.begin(); it != points.end(); it++){
    way = new Tuple(wayType);
    way->PutAttribute(0, new LongInt(wayId));
    if (storedPts.find(*it) != storedPts.end()) { // point found
      way->PutAttribute(2, new LongInt(pt2Id[*it]));
    }
    else { // insert new point
      storedPts.insert(*it);
      pt2Id[*it] = currentId;
      way->PutAttribute(2, new LongInt(currentId));
      node = new Tuple(nodeType);
      node->PutAttribute(0, new LongInt(currentId));
      currentId++;
      node->PutAttribute(1, new CcReal(true, it->GetY() / 100000.0 + 52.0));
      node->PutAttribute(2, new CcReal(true, it->GetX() / 100000.0 + 13.0));
      nodeRel->AppendTuple(node);
      tupleCount[0]++;
    }
    way->PutAttribute(1, new CcInt(true, refCounter));
    wayRel->AppendTuple(way);
    tupleCount[2]++;
    refCounter++;
  }
  insertWayTags(wayId, tuple, args);
  wayId++;
}

void FullOsmImport::insertWayTags(LongInt &wayId, Tuple *tuple, Word *args) {
  TupleType *tt = tuple->GetTupleType();
  int noAttrs = tt->GetNoAttributes();
  for (int i = 0; i < noAttrs; i++) {
    FText *type = static_cast<FText*>(args[i + 4 + noAttrs].addr);
    if ((type->GetValue() == "string") || (type->GetValue() == "text")) {
      CcString *attrname = static_cast<CcString*>(args[i + 4].addr);
      tag = new Tuple(wayTagType);
      tag->PutAttribute(0, new LongInt(wayId));
      tag->PutAttribute(1, new FText(true, attrname->GetValue()));
      if (type->GetValue() == "string") {
        tag->PutAttribute(2,
         new FText(true, ((CcString*)(tuple->GetAttribute(i)))->GetValue()));
      }
      else {
        tag->PutAttribute(2,
               new FText(true, ((FText*)(tuple->GetAttribute(i)))->GetValue()));
      }
      wayTagRel->AppendTuple(tag);
      tupleCount[3]++;
    }
  }
}

void FullOsmImport::processStream(Stream<Tuple> &stream, int attrNo,
                                  Word *args) {
  stream.open();
  Tuple *tuple = stream.request();
  std::list<Point> points; // store segments in correct order
  LongInt wayId = 0;
  while (tuple) {
    Line *line = (Line*)(tuple->GetAttribute(attrNo));
    LineSplitter<DbArray> *ls = new LineSplitter<DbArray>(line, true, false);
    do {
      points.clear();
      ls->NextLine(&points);
      insertNodes(points, wayId, tuple, args);
    } while (!points.empty());
    delete ls;
    deleteIfAllowed(tuple);
    tuple = stream.request();
  }
  stream.close();
}
    
/*
\subsection{Value Mapping}

*/
int convertstreetsVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  CcInt* attrNo = static_cast<CcInt*>(args[3].addr);
  CcString* prefix = static_cast<CcString*>(args[2].addr);
  if (!attrNo->IsDefined() || !prefix->IsDefined()) {
    cout << "undefined parameter" << endl;
    res->SetDefined(false);
    return 0;
  }
  FullOsmImport *osm = new FullOsmImport(prefix->GetValue());
  if (osm->nodeRel && osm->wayRel && osm->wayTagRel) {
    Stream<Tuple> str = static_cast<Stream<Tuple>* >(args[0].addr);
    osm->processStream(str, attrNo->GetValue(), args);
    osm->storeRelations(false);
    res->Set(true, true);
  }
  if (osm) {
    delete osm;
    osm = 0;
  }
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator convertstreets( "convertstreets",
                convertstreetsSpec,
                convertstreetsVM,
                Operator::SimpleSelect,
                convertstreetsTM);

/*
\section{Operator ~divide_osm~}

Randomly divides an OSM file into a number of sub-files.

\subsection{Type Mapping}

*/
ListExpr divide_osmTM(ListExpr args) {
  const std::string errMsg = "Expecting text x string x int x string";
  if (nl->HasLength(args, 4)) {
    if (FText::checkType(nl->First(args))
     && CcString::checkType(nl->Second(args))
     && CcInt::checkType(nl->Third(args))
     && CcString::checkType(nl->Fourth(args))) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError(errMsg);
}

/*
subsection{Specification}

*/
const std::string divide_osmSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text x string x int x string -> bool</text--->"
    "<text>divide_osm(_, _, _, _)</text--->"
    "<text>Randomly divides an OSM file into a number of sub-files.</text--->"
    "<text>query divide_osm('dortmund.osm', \"do\", 1909, \"DO\")</text--->))";

FullOsmImport::FullOsmImport(const std::string& fileName, 
                             const std::string& _subFileName, const int _size,
                             const std::string& prefix, const bool createrels) :
         isTemp(false), reader(0), subFileName(_subFileName), node(0), tag(0),
         way(0), rel(0), size(_size), tagged(false) {
  for (int i = 0; i < 6; i++) {
    tupleCount[i] = 0;
  }
  if (createrels) {
    sc = SecondoSystem::GetCatalog();
    relationsInitialized = initRelations(prefix, "_type", true);
    if (!relationsInitialized) {
      cout << "relations could not be initialized" << endl;
      return;
    }
    defineRelations(true);
    storeRelations(true);
  }
  divideOSMfile(fileName, createrels);
}

std::string FullOsmImport::getFileName(int64_t dest) {
  std::stringstream result;
  result << subFileName << "_" << dest;
  return result.str().c_str();
}

bool FullOsmImport::isWhitespace(const char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == 11;
}

std::string FullOsmImport::trim(const std::string &s) {
  size_t first = s.find_first_not_of(" \n\t\r");
  if (first == std::string::npos) {
    return std::string();
  }
  return s.substr(first);
}

bool FullOsmImport::isFileSwitchAllowed(const std::string& line) {
  if ((line.substr(0,5) == "<node") && (line.substr(line.length() -2) == "/>")){
    return true;
  }
  if ((line.substr(0, 7) == "</node>") || (line.substr(0, 6) == "</way>")
   || (line.substr(0, 11) == "</relation>")) {
    return true;
  }
  return false;
}

bool FullOsmImport::isValid(const std::string& line) {
  std::string text = trim(line);
  size_t endpos = text.find('>');
  if (endpos == std::string::npos) {
    return false;
  }
  return !(text.substr(1, 3) == "tag" || text.substr(1, 6) == "nd ref" ||
           text.substr(1, 6) == "member" || 
        ((text[endpos - 1] != '/') &&
          (text.substr(1, 4) == "node" || text.substr(1, 3) == "way" || 
            text.substr(1, 8) == "relation")));
}

void FullOsmImport::getOSMpart(const std::string& fileName, const int part) {
  char ch;
  std::stringstream strstr;
  std::string header;
  strstr << part;
  std::string destFileName = fileName + "_" + strstr.str();
  std::ifstream source, test;
  std::ofstream dest;
  source.open(fileName.c_str(), std::ios::in);
  if (!source.good()) {
    std::cerr << "Problem in opening file " << fileName << endl;
    return;
  }
  source.seekg(0, std::ios::end);
  int64_t fileSize = source.tellg();
  source.close();
  source.open(fileName.c_str(), std::ios::in | std::ios::binary);
//   std::streambuf *sbuf = source.rdbuf();
//   std::streambuf *tbuf = test.rdbuf();
  
  // copy header
  header += (char)source.get();
  header += (char)source.get();
  while (header.substr(header.length() - 2) != "/>") {
    header += (char)source.get();
  }
  dest.open(destFileName.c_str(), std::ios::trunc | std::ios::binary);
  dest << header;
  
    
  // find start position
  if (part > 0) {
    int64_t start = fileSize / size * (part);
    source.seekg(start);
    do {
      ch = (char)source.get();
    } while (ch != '>');
    bool valid = false;
    std::stringstream firstLines;
    do {
      std::string line;
      do {
        ch = (char)source.get();
        line += ch;
      } while (ch != '>');
      if (isValid(line)) {
        valid = true;
      }
      firstLines << line;
    } while (!valid);
    if (trim(firstLines.str()).substr(0, 5) == "<node" ||
        trim(firstLines.str()).substr(0, 4) == "<way" ||
        trim(firstLines.str()).substr(0, 9) == "<relation") {
      dest.write(firstLines.str().c_str(), firstLines.str().size());
    }
  }

  // copy main body
  int64_t end = (part + 1 == size ? fileSize : fileSize / size * (part + 1));
  int pageSize = WinUnix::getPageSize();
  char buffer[pageSize];
  while ((int64_t)source.tellg() + pageSize <= end) {
    source.read(buffer, pageSize);
    dest.write(buffer, pageSize);
  }
  int endBufferSize = end - source.tellg();
  char endBuffer[endBufferSize];
  source.read(endBuffer, endBufferSize);
  dest.write(endBuffer, endBufferSize);
  // copy end
  if (part + 1 < size) {
    do {
      ch = (char)source.get();
      dest.put(ch);
    } while (ch != '>');
    dest.close();
    
    bool valid = false;
    do {
      test.open(destFileName.c_str(), std::ios::in | std::ios::binary);
      test.seekg(0, std::ios::end);
      int64_t endpos = test.tellg();
      int64_t pos = endpos - 1;
      do {
        test.seekg(pos);
        ch = (char)test.get();
        pos--;
      } while (ch != '<');
      char lastLineBuffer[endpos - pos];
      test.read(lastLineBuffer, endpos - pos);
      std::string lastLine(lastLineBuffer, endpos - pos);
      test.close();
      lastLine = (lastLine[0] == '<' ? lastLine : "<" + lastLine);
      if (isValid(lastLine)) {
        valid = true;
      }
      else {
        dest.open(destFileName.c_str(), std::ios::app | std::ios::binary);
        do {
          ch = (char)source.get();
          dest.put(ch);
        } while (ch != '>');
        dest.close();
      }
    } while (!valid);
    dest.open(destFileName.c_str(), std::ios::app | std::ios::binary);
    std::string osmEnd = "\n</osm>\n";
    dest.write(osmEnd.c_str(), osmEnd.size());
    dest.close();
  }
  fileOk = true;
}

void FullOsmImport::divideOSMfile(const std::string& fileName, 
                                  const bool deletetts) {
  std::stringstream strstr;
  std::string destFileName = fileName + "_" + strstr.str();
  std::ifstream source;
  std::ofstream dest;
  source.open(fileName.c_str(), std::ios::in);
  if(!source.good()){
     std::cerr << "Problem in open file " << fileName << endl;
     return;
  }
  std::string line;
  size_t numOfChars(0), charCounter(0), destId(-1), nextLimit(0);
  std::streampos oldpos(0);
  source.seekg(0, std::ios::end);
  numOfChars = source.tellg();
  source.close();
  source.open(fileName.c_str(), std::ios::in);
  getline(source, line);
  charCounter += line.length();
  for (int i = 0; i < size; i++) { // clear destination files if existing
      std::string fn = getFileName(i);
      dest.open(fn.c_str(), std::ios::trunc);
      dest.close();
  }
  while (!source.eof() && source.good() &&
         (trim(line).substr(0, 5) != "<node")) { // copy head
    for (int64_t file = 0; file < size; file++) {
      dest.open(getFileName(file).c_str(), std::ios::app);
      dest << line << endl;
      dest.close();
    }
    oldpos = source.tellg();
    getline(source, line);
//     line = trim(line);
    charCounter += line.length();
    oldpos = source.tellg();
  }
  if(!source.good()){
     std::cerr << "problem in reading file(2)" << fileName << endl;
     return;
  }
  charCounter -= line.length();
  nextLimit = charCounter;
  dest.open(getFileName(0).c_str(), std::ios::app);
  dest << line << endl;
  dest.close();
  size_t partSize = (numOfChars - source.tellg() - 1) / size + 1;
  while (!source.eof() && source.good()) {
    if (charCounter >= nextLimit && isFileSwitchAllowed(line)) {
      if (dest.is_open()) {
        dest << "</osm>" << endl;
        dest.close();
      }
      nextLimit += partSize;
      destId++;
      dest.open(getFileName(destId).c_str(), std::ios::app);
    }
    getline(source, line);
    charCounter += line.length();
    line = trim(line);
    dest << line << endl;
  }
  source.close();
  dest.close();
  fileOk = true;
  if (deletetts) {
    nodeType->DeleteIfAllowed();
    nodeTagType->DeleteIfAllowed();
    wayType->DeleteIfAllowed();
    wayTagType->DeleteIfAllowed();
    relType->DeleteIfAllowed();
    relTagType->DeleteIfAllowed();
  }
}

/*
\subsection{Value Mapping}

*/
int divide_osmVM(Word* args, Word& result, int message, Word& local,Supplier s){
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  for(int i=0;i<4;i++){
     if(!((Attribute*)args[i].addr)->IsDefined()){
        res->Set(true,false);
        return 0;
     }
  }
  std::string fileName = ((FText*)args[0].addr)->GetValue();
  std::string subFileName = ((CcString*)args[1].addr)->GetValue();
  int size = ((CcInt*)args[2].addr)->GetValue();
  std::string prefix = ((CcString*)args[3].addr)->GetValue();
  FullOsmImport osm(fileName, subFileName, size, prefix, true);
  if (!(osm.relationsInitialized && osm.fileOk)) {
    res->Set(true, false);
  }
  else { // success
    res->Set(true, true);
  }
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator divide_osm( "divide_osm",
                divide_osmSpec,
                divide_osmVM,
                Operator::SimpleSelect,
                divide_osmTM);

/*
\section{Operator ~divide_osm2~}

Randomly divides an OSM file into a number of sub-files without creating
relations.

\subsection{Type Mapping}

*/
ListExpr divide_osm2TM(ListExpr args) {
  const std::string errMsg = "Expecting text x string x int";
  if (nl->HasLength(args, 3)) {
    if (FText::checkType(nl->First(args))
     && CcString::checkType(nl->Second(args))
     && CcInt::checkType(nl->Third(args))) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError(errMsg);
}

/*
subsection{Specification}

*/
const std::string divide_osm2Spec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text x string x int -> bool</text--->"
    "<text>divide_osm2(_, _, _)</text--->"
    "<text>Randomly divides an OSM file into a number of sub-files without "
    "creating relations.</text--->"
    "<text>query divide_osm2('dortmund.osm', \"do\", 1909)</text--->))";

/*
\subsection{Value Mapping}

*/
int divide_osm2VM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  for (int i = 0; i < 3; i++) {
    if (!((Attribute*)args[i].addr)->IsDefined()) {
      res->Set(true, false);
      return 0;
    }
  }
  std::string fileName = ((FText*)args[0].addr)->GetValue();
  std::string subFileName = ((CcString*)args[1].addr)->GetValue();
  int size = ((CcInt*)args[2].addr)->GetValue();
  FullOsmImport osm(fileName, subFileName, size, "", false);
  res->Set(true, osm.fileOk);
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator divide_osm2( "divide_osm2",
                divide_osm2Spec,
                divide_osm2VM,
                Operator::SimpleSelect,
                divide_osm2TM);


FullOsmImport::FullOsmImport(const std::string& fileName, const int noParts,
                             const int part) :
         isTemp(false), reader(0), subFileName(""), node(0), tag(0),
         way(0), rel(0), size(noParts), tagged(false) {
  for (int i = 0; i < 6; i++) {
    tupleCount[i] = 0;
  }
  getOSMpart(fileName, part);
}


/*
\section{Operator ~divide_osm3~}

Extracts the part i from n parts of an OSM file.

\subsection{Type Mapping}

*/
ListExpr divide_osm3TM(ListExpr args) {
  const std::string errMsg = "Expecting text x int x int";
  if (nl->HasLength(args, 3)) {
    if (FText::checkType(nl->First(args))
     && CcInt::checkType(nl->Second(args))
     && CcInt::checkType(nl->Third(args))) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return listutils::typeError(errMsg);
}

/*
subsection{Specification}

*/
const std::string divide_osm3Spec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text x int x int -> bool</text--->"
    "<text>divide_osm3(_, _, _)</text--->"
    "<text>Extracts the part i from n parts of an OSM file. The parameters "
    "refer to the file name, the total number n of subfiles, and the number i "
    "of the desired subfile, where 0 <= i < n must hold.</text--->"
    "<text>query divide_osm3('dortmund.osm', 1909, 19)</text--->))";

/*
\subsection{Value Mapping}

*/
int divide_osm3VM(Word* args, Word& result, int message, Word& local,
                  Supplier s) {
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*)result.addr;
  for (int i = 0; i < 3; i++) {
    if (!((Attribute*)args[i].addr)->IsDefined()) {
      res->Set(true, false);
      return 0;
    }
  }
  std::string fileName = ((FText*)args[0].addr)->GetValue();
  int noParts = ((CcInt*)args[1].addr)->GetValue();
  int part = ((CcInt*)args[2].addr)->GetValue();
  if (noParts < 1 || part < 0 || part >= noParts) {
    cout << "Error: invalid arguments" << endl;
    res->SetDefined(false);
    return 0;
  }
  FullOsmImport osm(fileName, noParts, part);
  res->Set(true, osm.fileOk);
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator divide_osm3( "divide_osm3",
                divide_osm3Spec,
                divide_osm3VM,
                Operator::SimpleSelect,
                divide_osm3TM);


/*
\section{Operator ~importairspaces~}

Imports an ~aip~ file containing airspace information. Every item must have the
following structure:

<ASP CATEGORY="CTR">
  <VERSION>5cc68cbb1ea6c44ca82b653c0a7efad43b32f5a1</VERSION>
  <ID>276345</ID>
  <COUNTRY>DE</COUNTRY>
  <NAME>CTR Dortmund</NAME>
  <ALTLIMIT_TOP REFERENCE="MSL">
    <ALT UNIT="F">2500</ALT>
  </ALTLIMIT_TOP>
  <ALTLIMIT_BOTTOM REFERENCE="GND">
    <ALT UNIT="F">0</ALT>
  </ALTLIMIT_BOTTOM>
  <GEOMETRY>
    <POLYGON>7.3741666666667 51.493055555556, 7.7697222222222 51.632222222222, 
    7.8463888888889 51.547777777778, 7.4519444444444 51.408333333333, 
    7.3741666666667 51.493055555556</POLYGON>
  </GEOMETRY>
</ASP>


\subsection{Implementation of base class and Local Info classes}

*/
ImportXML::ImportXML(std::string& fn) : correct(true), filename(fn) {
  sc = SecondoSystem::GetCatalog();
}

ImportXML::~ImportXML() {
  xmlFreeTextReader(reader);
  if (resultType != 0) {
    resultType->DeleteIfAllowed();
  }
}

bool ImportXML::openFile(std::string& category) {
  // check whether the file can be opened and is an aip file
  reader = xmlReaderForFile(filename.c_str(), NULL, 0);
  if (reader == NULL) {
    cout << "file " << filename << " cannot be opened" << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  if (read < 1) {
    cout << "file " << filename << " is empty" << endl;
    xmlFreeTextReader(reader);
    return false;
  }
  // strcmp(x1, x2) == 0  <==> x1 == x2
  read = xmlTextReaderRead(reader);
  if (strcmp((char *)xmlTextReaderConstName(reader), "OPENAIP")) {
    cout << "root node of " << filename << " has to be \"OPENAIP\"" << endl;
    cout << (char *)xmlTextReaderConstName(reader) << endl;
    xmlFreeTextReader(reader);
    return false;
  }
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
//   xmlChar *subNameXml;
//   subNameXml = xmlTextReaderLocalName(reader);
//   cout << "### " << (char *)subNameXml << endl;
  std::string constName = (char *)xmlTextReaderConstName(reader);
  if (constName != category) {
    cout << "2nd node of " << filename << " has to be \"" << category << "\"" 
         << endl;
    cout << (char *)xmlTextReaderConstName(reader) << endl;
    xmlFreeTextReader(reader);
    return false;
  }
  cout << "file " << filename << " opened successfully" << endl;
  return true;
}

bool ImportXML::readSimpleEntry(const std::string& name, std::string& entry,
                                CcString *attr /* = 0 */) {
  xmlChar *subNameXml, *entryXml;
  std::string currentName = "";
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != name) {
    return false;
  }
  if (attr != 0) {
    xmlChar *attrXml = (xmlChar*)((char*)attr->GetValue().c_str());
    xmlChar *attrvalueXml = xmlTextReaderGetAttribute(reader, attrXml);
    if (attrvalueXml == NULL) {
//       cout << "attr " << attr->GetValue() << " not found @ " << name << endl;
      return false;
    }
    attr->Set(true, (char*)attrvalueXml);
//     cout << "attribute FOUND and set to " << attr->GetValue() << endl;
    xmlFree(attrvalueXml);
  }
  read = xmlTextReaderRead(reader);
  entryXml = xmlTextReaderValue(reader);
  if (entryXml == NULL && name == "ILS") {
    entry = "";
  }
  else if (entryXml == NULL) {
    return false;
  }
  else {
    entry = (char*)entryXml;
    xmlFree(entryXml);
  }
  next = xmlTextReaderNext(reader);
  if (entry == "" && name == "ILS") {
    next = xmlTextReaderNext(reader);
  }
  return true;
}

bool ImportXML::readGeolocationICAO(Point* location, CcReal* elevation,
                                    CcString* icao /* = 0 */) {
  xmlChar *subNameXml, *latXml, *lonXml, *elevXml;
  std::string currentName;
  next = xmlTextReaderNext(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName == "ICAO") {
    read = xmlTextReaderRead(reader);
    xmlChar *icaoXml = xmlTextReaderValue(reader);
//     cout << "   ICAO: " << (char*)icaoXml << endl;
    icao->Set(true, (char*)icaoXml);
    xmlFree(icaoXml);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char*)subNameXml;
    xmlFree(subNameXml);
  }
  if (currentName != "GEOLOCATION") {
//     cout << "name is not GEOLOCATION" << " * " << currentName << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "LAT") {
//     cout << "name is not LAT" << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  latXml = xmlTextReaderValue(reader);
//   cout << "   LAT: " << (char*)latXml << endl;
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "LON") {
//     cout << "name is not LON" << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  lonXml = xmlTextReaderValue(reader);
//   cout << "   LON: " << (char*)lonXml << endl;
  location->Set(true, std::stod((char*)lonXml), std::stod((char*)latXml));
  xmlFree(latXml);
  xmlFree(lonXml);
  // start to read ELEVATION
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "ELEV") {
//     cout << "name is not ELEV" << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  elevXml = xmlTextReaderValue(reader);
//   cout << "   ELEVATION: " << (char*)elevXml << endl;
  elevation->Set(true, std::stod((char*)elevXml));
  xmlFree(elevXml);
  return true;
}

void ImportXML::string2ccbool(std::string& boolexpr, CcBool *result) {
  std::transform(boolexpr.begin(), boolexpr.end(), boolexpr.begin(), ::toupper);
  if (boolexpr == "TRUE") {
    result->Set(true, true);
  }
  else if (boolexpr == "FALSE") {
    result->Set(true, false);
  }
  else {
    result->SetDefined(false);
  }
}

ImportairspacesLI::ImportairspacesLI(std::string& fn) 
  : ImportXML(fn) {
  ListExpr numResultTypeList = sc->NumericType(getResultTypeList());
  resultType = new TupleType(numResultTypeList);
  std::string category = "AIRSPACES";
  correct = openFile(category);
}

Tuple* ImportairspacesLI::getNextTuple() {
  if (!correct) {
    return 0;
  }
  std::string currentName = "", contents = "";
  xmlChar *subNameXml, *catXml, *geometryXml;
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  if ((read != 1) || (next != 1)) {
    return 0;
  }
  Tuple *tuple = new Tuple(resultType);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "GENERAL TYPE is " << currentName << endl;
  if (currentName != "ASP") {
    return 0;
  }
  catXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"CATEGORY");
  if (catXml == NULL) {
    return 0;
  }
//   cout << "CATEGORY: " << (char*)catXml << endl;
  tuple->PutAttribute(0, new CcString(true, (char*)catXml));
  xmlFree(catXml);
  // start to read VERSION
  FText *version = new FText(false);
  if (readSimpleEntry("VERSION", contents)) {
    version->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(1, version);
//   cout << "   VERSION: " << version->GetValue() << endl;
//   // start to read ID
  CcInt *id = new CcInt(false);
  if (readSimpleEntry("ID", contents)) {
    id->Set(true, std::stoi(contents));
  }
  else {
    return 0;
  }
  tuple->PutAttribute(2, id);
//   cout << "   ID: " << id->GetValue() << endl;
  // start to read COUNTRY
  CcString *country = new CcString(false);
  if (readSimpleEntry("COUNTRY", contents)) {
    country->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(3, country);
//   cout << "   COUNTRY: " << country->GetValue() << endl;
  // start to read NAME
  FText *name = new FText(false);
  if (readSimpleEntry("NAME", contents)) {
    name->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(4, name);
//   cout << "   NAME: " << name->GetValue() << endl;
  // start to read ALTLIMIT_TOP
  if (!readAltlimit(true, tuple)) {
    return 0;
  }
  // start to read ALTLIMIT_BOTTOM
  if (!readAltlimit(false, tuple)) {
    return 0;
  }
  // start to read GEOMETRY
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "currentName 7 is " << currentName << endl;
  if (currentName != "GEOMETRY") {
    return 0;
  }
  read = xmlTextReaderRead(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "POLYGON") {
    return 0;
  }
  read = xmlTextReaderRead(reader);
  geometryXml = xmlTextReaderValue(reader);
//   cout << "   POLYGON: " << (char*)geometryXml << endl << endl;
  Region *reg = new Region(true);
  string2region((char*)geometryXml, reg);
  tuple->PutAttribute(11, reg);
  xmlFree(geometryXml);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
//   cout << "---------------------TUPLE COMPLETED--------------------" << endl;
  return tuple;
}

void ImportairspacesLI::string2region(std::string regstr, Region *result) {
  result->SetDefined(false);
  std::istringstream iss(regstr);
  std::string pstr;
  std::vector<Point*> points;
  double xcoord, ycoord;
  bool first = true;
  while (getline(iss, pstr, ' ')) {
    if (first) {
      xcoord = std::stod(pstr);
      first = false;
    }
    else {
      ycoord = std::stod(pstr.substr(0, pstr.find(',')));
      points.push_back(new Point(true, xcoord, ycoord));
      first = true;
    }
  }
  Line *line = new Line(true);
  line->StartBulkLoad();
  Point *lastPt = 0;
  for (auto it : points) {
    if (!first) {
      if (!AlmostEqual(*lastPt, *it)) {
        HalfSegment hs(true, *lastPt, *it);
        (*line) += hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        (*line) += hs;
      }
      lastPt->DeleteIfAllowed();
    }
    lastPt = it;
    first = false;
  }
  lastPt->DeleteIfAllowed();
  line->EndBulkLoad();
  line->Transform(*result);
  line->DeleteIfAllowed();
}

bool ImportairspacesLI::readAltlimit(const bool top, Tuple *tuple) {
  xmlChar *subNameXml, *altlimitrefXml, *altlimitunitXml, *altlimitvalueXml;
  std::string currentName = "";
  int attrNo = (top ? 5 : 8);
  if (!top) {
    next = xmlTextReaderNext(reader);
    read = xmlTextReaderRead(reader);
    next = xmlTextReaderNext(reader);
  }
  next = xmlTextReaderNext(reader);
  read = xmlTextReaderRead(reader);
  altlimitrefXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"REFERENCE");
  if (altlimitrefXml == NULL) {
    return false;
  }
//   cout << "   REFERENCE: " << (char*)altlimitrefXml << endl;
  tuple->PutAttribute(attrNo, new CcString(true, (char*)altlimitrefXml));
  xmlFree(altlimitrefXml);
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
//   cout << "currentName " << (top? "5": "6") << " is " << currentName << endl;
  xmlFree(subNameXml);
  if (currentName != "ALT") {
    return false;
  }
  altlimitunitXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"UNIT");
  if (altlimitunitXml == NULL) {
    return false;
  }
//   cout << "   UNIT: " << (char*)altlimitunitXml << endl;
  tuple->PutAttribute(attrNo + 1, new CcString(true, (char*)altlimitunitXml));
  xmlFree(altlimitunitXml);
  read = xmlTextReaderRead(reader);
  altlimitvalueXml = xmlTextReaderValue(reader);
//   cout << "   ALTLIMIT_" << (top ? "TOP" : "BOTTOM") << "_VALUE: \'" 
//        << (char*)altlimitvalueXml << "\'=" 
//        << std::stoi((char*)altlimitvalueXml) << endl;
  tuple->PutAttribute(attrNo + 2,
                      new CcInt(true, std::stoi((char*)altlimitvalueXml)));
  xmlFree(altlimitvalueXml);
  return true;
}

ListExpr ImportairspacesLI::getResultTypeList() {
  ListExpr attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Altlimit_top_ref"),
                                      nl->SymbolAtom(CcString::BasicType())),
      nl->SixElemList(nl->TwoElemList(nl->SymbolAtom("Altlimit_top_unit"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Altlimit_top"), 
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Altlimit_bottom_ref"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Altlimit_bottom_unit"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Altlimit_bottom"), 
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Geometry"),
                                      nl->SymbolAtom(Region::BasicType()))));
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Name"), 
                                nl->SymbolAtom(FText::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Country"),
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Id"),
                                nl->SymbolAtom(CcInt::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Version"),
                                nl->SymbolAtom(FText::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Category"), 
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrs);
}

/*
\subsection{Type Mapping}

*/
ListExpr importairspacesTM(ListExpr args) {
  const std::string errMsg = "Expecting a text argument (filename)";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(errMsg);
  }
  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(errMsg);
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), 
                         ImportairspacesLI::getResultTypeList());
}

/*
\subsection{Specification}

*/
const std::string importairspacesSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text> text -> stream(tuple(Category: string, Version: text, Id: int, "
   "Country: string, Name: string, Altlimit_top_ref: string, Altlimit_top_"
   "unit: string, Altlimit_top: int, Altlimit_bottom_ref: string, Altlimit_"
   "bottom_unit: string, Altlimit_bottom: int, Geometry: region))</text--->"
   "<text>importairspaces( _ )</text--->"
   "<text>Imports an XML file containing airspace data.</text--->"
   "<text>query importairspaces('openaip_airspace_germany_de.aip')</text--->))";

/*
\subsection{Value Mapping}

*/
int importairspacesVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  if (!((FText*)args[0].addr)->IsDefined()) {
    return 0;
  }
  std::string filename = ((FText*)args[0].addr)->GetValue();
  ImportairspacesLI *li = static_cast<ImportairspacesLI*>(local.addr);
  switch (message) {
    case OPEN: {
      if (li) {
        li = 0;
      }
      li = new ImportairspacesLI(filename);
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (local.addr) {
        li = (ImportairspacesLI*)local.addr;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator importairspaces("importairspaces",
                importairspacesSpec,
                importairspacesVM,
                Operator::SimpleSelect,
                importairspacesTM);

/*
Operator ~importnavaids~

*/
ImportnavaidsLI::ImportnavaidsLI(std::string& fn) 
  : ImportXML(fn) {
  ListExpr numResultTypeList = sc->NumericType(getResultTypeList());
  resultType = new TupleType(numResultTypeList);
  std::string category = "NAVAIDS";
  correct = openFile(category);
}

Tuple* ImportnavaidsLI::getNextTuple() {
  if (!correct) {
    return 0;
  }
  std::string currentName = "", contents = "";
  xmlChar *subNameXml, *typeXml,
    *freqXml, *channelXml, *rangeXml, *declXml, *alignedXml;
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  if ((read != 1) || (next != 1)) {
    return 0;
  }
  Tuple *tuple = new Tuple(resultType);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "GENERAL TYPE is " << currentName << endl;
  if (currentName != "NAVAID") {
    return 0;
  }
  typeXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"TYPE");
  if (typeXml == NULL) {
    return 0;
  }
//   cout << "TYPE: " << (char*)typeXml << endl;
  tuple->PutAttribute(0, new CcString(true, (char*)typeXml));
  xmlFree(typeXml);
  // start to read COUNTRY
  CcString *country = new CcString(false);
  if (readSimpleEntry("COUNTRY", contents)) {
    country->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(1, country);
  // start to read NAME
  CcString *name = new CcString(false);
  if (readSimpleEntry("NAME", contents)) {
    name->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(2, name);
  // start to read ID
  CcString *id = new CcString(false);
  if (readSimpleEntry("ID", contents)) {
    id->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(3, id);
  // start to read LOCATION
  Point *location = new Point(false);
  CcReal *elevation = new CcReal(false);
  if (!readGeolocationICAO(location, elevation)) {
    return 0;
  }
  tuple->PutAttribute(4, location);
  tuple->PutAttribute(5, elevation);
  // start to read RADIO FREQUENCY
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "currentName 5 is " << currentName << endl;
  if (currentName != "RADIO") {
    return 0;
  }
  read = xmlTextReaderRead(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "   subname 1 is " << currentName << endl;
  if (currentName != "FREQUENCY") {
    return 0;
  }
  read = xmlTextReaderRead(reader);
  freqXml = xmlTextReaderValue(reader);
//   cout << "   FREQUENCY: " << (char*)freqXml << endl;
  CcReal *frequency = new CcReal(false);
  if (freqXml != NULL) {
    frequency->Set(true, std::stod((char*)freqXml));
    xmlFree(freqXml);
    next = xmlTextReaderNext(reader);
  }
  tuple->PutAttribute(6, frequency);
  // start to read (optional) CHANNEL
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  CcString *channel = new CcString(false);
  if (currentName == "CHANNEL") {
//     cout << "   subname 2 is " << currentName << endl;
    read = xmlTextReaderRead(reader);
    channelXml = xmlTextReaderValue(reader);
//     cout << "   CHANNEL: " << (char*)channelXml << endl;
    channel->Set(true, (char*)channelXml);
    xmlFree(channelXml);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
  }
  tuple->PutAttribute(7, channel);
  // start to read PARAMS
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "PARAMS") {
    return 0;
  }
//   cout << "currentName 6 is " << currentName << endl;
  // start to read RANGE
  read = xmlTextReaderRead(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "RANGE") {
    return 0;
  }
//   cout << "   subname 1 is " << currentName << endl;
  read = xmlTextReaderRead(reader);
  rangeXml = xmlTextReaderValue(reader);
//   cout << "   RANGE: " << (char*)rangeXml << endl;
  tuple->PutAttribute(8, new CcInt(true, std::stoi((char*)rangeXml)));
  xmlFree(rangeXml);
  // start to read DECLINATION
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "   subname 2 is " << currentName << endl;
  if (currentName != "DECLINATION") {
    return 0;
  }
  read = xmlTextReaderRead(reader);
  declXml = xmlTextReaderValue(reader);
//   cout << "   DECLINATION: " << (char*)declXml << endl;
  CcReal *decl = new CcReal(false);
  if (declXml != NULL) {
    decl->Set(true, std::stod((char*)declXml));
    xmlFree(declXml);
    next = xmlTextReaderNext(reader);
  }
  tuple->PutAttribute(9, decl);
  // start to read ALIGNEDTOTRUENORTH
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "   subname 3 is " << currentName << endl;
  if (currentName != "ALIGNEDTOTRUENORTH") {
    return 0;
  }
  read = xmlTextReaderRead(reader);
  alignedXml = xmlTextReaderValue(reader);
//   cout << "   ALIGNEDTOTRUENORTH: " << (char*)alignedXml << endl;
  std::string alignedstr = (char*)alignedXml;
  xmlFree(alignedXml);
  CcBool *aligned = new CcBool(false);
  if (alignedstr == "TRUE") {
    aligned->Set(true, true);
  }
  else if (alignedstr == "FALSE") {
    aligned->Set(true, false);
  }
  tuple->PutAttribute(10, aligned);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
//   cout << "---------------------TUPLE COMPLETED--------------------" << endl;
  return tuple;
}

ListExpr ImportnavaidsLI::getResultTypeList() {
  ListExpr attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Location"),
                                      nl->SymbolAtom(Point::BasicType())),
      nl->SixElemList(nl->TwoElemList(nl->SymbolAtom("Elevation"),
                                      nl->SymbolAtom(CcReal::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Frequency"), 
                                      nl->SymbolAtom(CcReal::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Channel"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Range"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Declination"), 
                                      nl->SymbolAtom(CcReal::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("AlignedToTrueNorth"),
                                      nl->SymbolAtom(CcBool::BasicType()))));
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Id"), 
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Name"),
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Country"),
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Type"),
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrs);
}

/*
\subsection{Type Mapping}

*/
ListExpr importnavaidsTM(ListExpr args) {
  const std::string errMsg = "Expecting a text argument (filename)";
  if (!nl->HasLength(args, 1)) {
    return listutils::typeError(errMsg);
  }
  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(errMsg);
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), 
                         ImportnavaidsLI::getResultTypeList());
}

/*
\subsection{Specification}

*/
const std::string importnavaidsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text -> stream(tuple(Type: string, Country: string, Name: string, "
    "Id: int, Pos: point, Elevation: real, Frequency: real, Channel: string, "
    "Range: int, Declination: real, AlignedToTrueNorth: bool))</text--->"
    "<text>importnavaids( _ )</text--->"
    "<text>Imports an XML file containing navigation aid data.</text--->"
    "<text>query importnavaids('openaip_navaid_germany_de.aip')</text--->))";

/*
\subsection{Value Mapping}

*/
int importnavaidsVM(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
  if (!((FText*)args[0].addr)->IsDefined()) {
    return 0;
  }
  std::string filename = ((FText*)args[0].addr)->GetValue();
  ImportnavaidsLI *li = static_cast<ImportnavaidsLI*>(local.addr);
  switch (message) {
    case OPEN: {
      if (li) {
        li = 0;
      }
      li = new ImportnavaidsLI(filename);
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (local.addr) {
        li = (ImportnavaidsLI*)local.addr;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator importnavaids("importnavaids",
                importnavaidsSpec,
                importnavaidsVM,
                Operator::SimpleSelect,
                importnavaidsTM);


/*
Operator ~importairports~

*/
ImportairportsLI::ImportairportsLI(std::string& fn, std::string& radiorelname, 
                                   std::string& runwayrelname) 
  : ImportXML(fn), radioRel(0), runwayRel(0), radioRelName(radiorelname),
    runwayRelName(runwayrelname), counter(1) {
  ListExpr numResultTypeList = sc->NumericType(getResultTypeList());
  resultType = new TupleType(numResultTypeList);
  std::string category = "WAYPOINTS";
  correct = openFile(category);
  ListExpr radioTypeList = getRadioTypeList();
  radioRel = createRelation(radiorelname, radioTypeList);
  ListExpr runwayTypeList = getRunwayTypeList();
  runwayRel = createRelation(runwayrelname, runwayTypeList);
  correct = (radioRel != 0 && runwayRel != 0);
  if (correct) {
    radioTupleType = radioRel->GetTupleType();
    runwayTupleType = runwayRel->GetTupleType();
    read = xmlTextReaderRead(reader);
    next = xmlTextReaderNext(reader);
    correct = ((read == 1) && (next == 1));
  }
}

ImportairportsLI::~ImportairportsLI() {
  ListExpr radioRelType = nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()),
                           nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                              getRadioTypeList()));
  Word relWord;
  relWord.setAddr(radioRel);
  sc->InsertObject(radioRelName, "", radioRelType, relWord, true);
  ListExpr runwayRelType =nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()),
                           nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                              getRunwayTypeList()));
  relWord.setAddr(runwayRel);
  sc->InsertObject(runwayRelName, "", runwayRelType, relWord, true);
}

Tuple* ImportairportsLI::getNextTuple() {
  if (!correct) {
    return 0;
  }
  std::string currentName = "", contents = "";
  xmlChar *subNameXml, *typeXml;
  if ((read != 1) || (next != 1)) {
    return 0;
  }
  Tuple *tuple = new Tuple(resultType);
  tuple->PutAttribute(0, new CcInt(true, counter));
//   cout << "ID: " << counter << endl;
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
//   cout << "GENERAL TYPE is " << currentName << endl;
  if (currentName != "AIRPORT") {
    return 0;
  }
  typeXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"TYPE");
  if (typeXml == NULL) {
    return 0;
  }
//   cout << "TYPE: " << (char*)typeXml << endl;
  tuple->PutAttribute(1, new CcString(true, (char*)typeXml));
  xmlFree(typeXml);
  CcString *country = new CcString(false);
  if (readSimpleEntry("COUNTRY", contents)) {
    country->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(2, country);
  FText *name = new FText(false);
  if (readSimpleEntry("NAME", contents)) {
    name->Set(true, contents);
  }
  else {
    return 0;
  }
  tuple->PutAttribute(3, name);
  CcString *icao = new CcString(false);
  Point *location = new Point(false);
  CcReal *elevation = new CcReal(false);
  if (!readGeolocationICAO(location, elevation, icao)) {
    return 0;
  }
  tuple->PutAttribute(4, icao);
  tuple->PutAttribute(5, location);
  tuple->PutAttribute(6, elevation);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  while (readRadioInfo(currentName)) {
//     cout << "after radio, currentName = " << currentName << endl;
  }
  while (readRunwayInfo(currentName)) {
//     cout << "after runway, currentName = " << currentName << endl;
  }
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
//   cout << "---------------------TUPLE COMPLETED--------------------" << endl;
//   tuple->Print(cout);
  counter++;
  return tuple;
}

bool ImportairportsLI::readRunwayInfo(std::string& currentName) {
  xmlChar *subNameXml, *opXml;
  std::string contents = "";
  Tuple *tuple = new Tuple(runwayTupleType);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "RWY") {
//     cout << "name is not RWY but " << currentName << endl;
    return false;
  }
  opXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"OPERATIONS");
  if (opXml == NULL) {
    return false;
  }
//   cout << "   OPERATIONS: " << (char*)opXml << endl;
  tuple->PutAttribute(0, new CcString(true, (char*)opXml));
  xmlFree(opXml);
  CcString *name = new CcString(false);
  if (readSimpleEntry("NAME", contents)) {
    name->Set(true, contents);
  }
  else {
    name->DeleteIfAllowed();
    return false;
  }
  tuple->PutAttribute(1, name);
//   cout << "   NAME: " << name->GetValue() << endl;
  CcString *sfc = new CcString(false);
  if (readSimpleEntry("SFC", contents)) {
    sfc->Set(true, contents);
  }
  else {
    sfc->DeleteIfAllowed();
    return false;
  }
  tuple->PutAttribute(2, sfc);
//   cout << "   SFC: " << sfc->GetValue() << endl;
  CcReal *length = new CcReal(false);
  if (readSimpleEntry("LENGTH", contents)) {
    length->Set(true, std::stod(contents));
  }
  else {
    length->DeleteIfAllowed();
    return false;
  }
  tuple->PutAttribute(3, length);
//   cout << "   LENGTH: " << length->GetValue() << endl;
  CcReal *width = new CcReal(false);
  if (readSimpleEntry("WIDTH", contents)) {
    width->Set(true, std::stod(contents));
  }
  else {
    width->DeleteIfAllowed();
    return false;
  }
  tuple->PutAttribute(4, width);
//   cout << "   WIDTH: " << width->GetValue() << endl;
  CcString *strength = new CcString(false);
  CcString *strength_unit = new CcString(true, "UNIT");
  if (readSimpleEntry("STRENGTH", contents, strength_unit)) {
    strength->Set(true, contents);
  }
  else {
    strength_unit->Set(false, "");
  }
  tuple->PutAttribute(5, strength_unit);
  tuple->PutAttribute(6, strength);
//   cout << "   STRENGTH_UNIT: " << strength_unit->GetValue() << endl;
//   cout << "   STRENGTH: " << strength->GetValue() << endl;
  if (!readDirectionInfo(tuple, true)) {
    return false;
  }
  if (!readDirectionInfo(tuple, false)) {
    return false;
  }
  tuple->PutAttribute(17, new CcInt(true, counter));
  runwayRel->AppendTuple(tuple);
//   tuple->Print(cout);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  if (tuple->GetAttribute(12)->IsDefined()) {
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
  }
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  delete tuple;
  return true;
}

bool ImportairportsLI::readDirectionInfo(Tuple *tuple, const bool first) {
  xmlChar *subNameXml, *dirXml, *toraXml, *ldaXml, *papiXml;
  int attrno = 7 + (first ? 0 : 5);
  std::string currentName = "", contents = "";
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "DIRECTION" || !first) {
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char*)subNameXml;
    xmlFree(subNameXml);
    if (currentName != "DIRECTION") {
      read = xmlTextReaderRead(reader);
      subNameXml = xmlTextReaderLocalName(reader);
      currentName = (char*)subNameXml;
      xmlFree(subNameXml);
    }
  }
  if (currentName != "DIRECTION") {
//     cout << "name is not DIRECTION but " << currentName << endl;
    if (!first && currentName == "RWY") { // only one DIRECTION specified
      tuple->PutAttribute(attrno, new CcInt(false)); // DIRECTION
      tuple->PutAttribute(attrno + 1, new CcInt(false)); // TORA
      tuple->PutAttribute(attrno + 2, new CcInt(false)); // LDA
      tuple->PutAttribute(attrno + 3, new CcReal(false)); // ILS
      tuple->PutAttribute(attrno + 4, new CcBool(false)); // PAPI
      return true;
    }
    return false;
  }
  dirXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"TC");
  if (dirXml == NULL) {
    return false;
  }
//   cout << "DIRECTIONTC: " << (char*)dirXml << endl;
  tuple->PutAttribute(attrno, new CcInt(true, std::stoi((char*)dirXml)));
  xmlFree(dirXml);
  read = xmlTextReaderRead(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "RUNS") {
//     cout << "name is not RUNS but " << currentName << endl;
    tuple->PutAttribute(attrno + 1, new CcInt(false)); // TORA
    tuple->PutAttribute(attrno + 2, new CcInt(false)); // LDA
    if (currentName == "DIRECTION") { // no RUNS specified
      tuple->PutAttribute(attrno + 3, new CcReal(false)); // ILS
      tuple->PutAttribute(attrno + 4, new CcBool(false)); // PAPI
      return true;
    }
    else if (currentName != "LANDINGAIDS") {
      return false;
    }
  }
  else { // read RUNS
    read = xmlTextReaderRead(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char*)subNameXml;
    xmlFree(subNameXml);
    CcInt *tora = new CcInt(false);
    if (currentName == "TORA") {
      read = xmlTextReaderRead(reader);
      toraXml = xmlTextReaderValue(reader);
      tora->Set(true, std::stoi((char*)toraXml));
      xmlFree(toraXml);
      read = xmlTextReaderRead(reader);
    }
    tuple->PutAttribute(attrno + 1, tora);
//     cout << "   TORA: " << tora->GetValue() << endl;
    CcInt *lda = new CcInt(false);
    if (currentName != "LDA") {
      read = xmlTextReaderRead(reader);
      read = xmlTextReaderRead(reader);
      subNameXml = xmlTextReaderLocalName(reader);
      currentName = (char*)subNameXml;
      xmlFree(subNameXml);
    }
    if (currentName == "LDA") {
      read = xmlTextReaderRead(reader);
      ldaXml = xmlTextReaderValue(reader);
      lda->Set(true, std::stoi((char*)ldaXml));
      xmlFree(ldaXml);
      read = xmlTextReaderRead(reader);
    }
    tuple->PutAttribute(attrno + 2, lda);
//     cout << "   LDA: " << lda->GetValue() << endl;
    read = xmlTextReaderRead(reader);
    read = xmlTextReaderRead(reader);
    if (lda->IsDefined()) {
      read = xmlTextReaderRead(reader);
      read = xmlTextReaderRead(reader);
    }
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char*)subNameXml;
    xmlFree(subNameXml);
  }
  CcReal *ils = new CcReal(false);
  CcBool *papi = new CcBool(false);
  if (currentName == "DIRECTION") { // no LANDINGAIDS specified
    tuple->PutAttribute(attrno + 3, ils);
    tuple->PutAttribute(attrno + 4, papi);
    return true;
  }
  else if (currentName == "LANDINGAIDS") { // read LANDINGAIDS
    if (readSimpleEntry("ILS", contents)) {
      if (!contents.empty()) {
        ils->Set(true, std::stod(contents));
      }
    }
    tuple->PutAttribute(attrno + 3, ils);
    if (ils->IsDefined()) {
      if (readSimpleEntry("PAPI", contents)) {
        string2ccbool(contents, papi);
      }
    }
    else { // no ILS specified
      read = xmlTextReaderRead(reader);
      papiXml = xmlTextReaderValue(reader);
      contents = (char*)papiXml;
      xmlFree(papiXml);
      string2ccbool(contents, papi);
    }
    tuple->PutAttribute(attrno + 4, papi);
  }
  else {
//     cout << "currentName is neither DIRECTION nor LANDINGAIDS but"
//          << currentName << endl;
    return false;
  }
//   cout << "   ILS: " << ils->GetValue() << endl;
//   cout << "   PAPI: " << papi->GetValue() << endl;
  if (!first) {
    while (currentName != "DIRECTION") {
      read = xmlTextReaderRead(reader);
      subNameXml = xmlTextReaderLocalName(reader);
      currentName = (char*)subNameXml;
      xmlFree(subNameXml);
    }
  }
  else {
    if (!ils->IsDefined()) {
      read = xmlTextReaderRead(reader);
      read = xmlTextReaderRead(reader);
    }
    read = xmlTextReaderRead(reader);
    read = xmlTextReaderRead(reader);
    read = xmlTextReaderRead(reader);
    read = xmlTextReaderRead(reader);
  }
  return true;
}

bool ImportairportsLI::readRadioInfo(std::string& currentName) {
  xmlChar *subNameXml, *catXml, *freqXml, *typeXml, *descXml;
  Tuple *tuple = new Tuple(radioTupleType);
  next = xmlTextReaderNext(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "RADIO") {
//     cout << "name is not RADIO but " << currentName << endl;
    return false;
  }
  catXml = xmlTextReaderGetAttribute(reader, (xmlChar*)"CATEGORY");
  if (catXml == NULL) {
    return false;
  }
//   cout << "CATEGORY: " << (char*)catXml << endl;
  tuple->PutAttribute(0, new CcString(true, (char*)catXml));
  xmlFree(catXml);
  read = xmlTextReaderRead(reader);
  read = xmlTextReaderRead(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "FREQUENCY") {
//     cout << "name is not FREQUENCY but " << currentName << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  freqXml = xmlTextReaderValue(reader);
//   cout << "   FREQUENCY: " << (char*)freqXml << endl;
  tuple->PutAttribute(1, new CcReal(true, std::stod((char*)freqXml)));
  xmlFree(freqXml);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName != "TYPE") {
//     cout << "name is not TYPE but " << currentName << endl;
    return false;
  }
  read = xmlTextReaderRead(reader);
  typeXml = xmlTextReaderValue(reader);
//   cout << "   TYPE: " << (char*)typeXml << endl;
  CcString *type = new CcString(true, (char*)typeXml);
  xmlFree(typeXml);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  next = xmlTextReaderNext(reader);
  subNameXml = xmlTextReaderLocalName(reader);
  currentName = (char*)subNameXml;
  xmlFree(subNameXml);
  if (currentName == "TYPESPEC") {
    read = xmlTextReaderRead(reader);
    typeXml = xmlTextReaderValue(reader);
//     cout << "   TYPESPEC: " << (char*)typeXml << endl;
    type->Set(true, (char*)typeXml);
    xmlFree(typeXml);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char*)subNameXml;
    xmlFree(subNameXml);
  }
  tuple->PutAttribute(2, type);
  FText *desc = new FText(false);
  if (currentName != "DESCRIPTION") {
//     cout << "name is not DESCRIPTION but " << currentName << endl;
  }
  else {
    read = xmlTextReaderRead(reader);
    descXml = xmlTextReaderValue(reader);
//     cout << "   DESCRIPTION: " << (char*)descXml << endl;
    desc->Set(true, (char*)descXml);
    xmlFree(descXml);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    next = xmlTextReaderNext(reader);
    subNameXml = xmlTextReaderLocalName(reader);
    currentName = (char*)subNameXml;
    xmlFree(subNameXml);
  }
  tuple->PutAttribute(3, desc);
  tuple->PutAttribute(4, new CcInt(true, counter));
  radioRel->AppendTuple(tuple);
  delete tuple;
//   tuple->Print(cout);
  return true;
}

Relation* ImportairportsLI::createRelation(std::string& name, 
                                           ListExpr& typeList) {
  if (sc->IsObjectName(name)) {
    cout << "Object " << name << " is already defined and will be overwritten"
         << endl;
    if (!sc->DeleteObject(name)) {
      cout << "ERROR: Object " << name << " could not be deleted" << endl;
      return 0;
    }
  }
  std::string errorMsg = "error";
  if (!sc->IsValidIdentifier(name, errorMsg, true)) {
    cout << errorMsg << endl;
    return 0;
  }
  if (sc->IsSystemObject(name)) {
    cout << "ERROR: " << name << " is a reserved name" << endl;
    return 0;
  }
  typeList = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), typeList);
  ListExpr numTypeList = sc->NumericType(typeList);
  TupleType *tupleType = new TupleType(numTypeList);
  return new Relation(tupleType, false);
}

ListExpr ImportairportsLI::getResultTypeList() {
  ListExpr attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Id"),
                                      nl->SymbolAtom(CcInt::BasicType())),
      nl->SixElemList(nl->TwoElemList(nl->SymbolAtom("Type"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Country"), 
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Name"),
                                      nl->SymbolAtom(FText::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("ICAO"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Location"), 
                                      nl->SymbolAtom(Point::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Elevation"),
                                      nl->SymbolAtom(CcReal::BasicType()))));
  return nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrs);
}

ListExpr ImportairportsLI::getRadioTypeList() {
  return nl->FiveElemList(nl->TwoElemList(nl->SymbolAtom("Category"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Frequency"), 
                                      nl->SymbolAtom(CcReal::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Type"),
                                      nl->SymbolAtom(CcString::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Description"),
                                      nl->SymbolAtom(FText::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("AirportId"),
                                      nl->SymbolAtom(CcInt::BasicType())));
}

ListExpr ImportairportsLI::getRunwayTypeList() {
  ListExpr attrs = nl->SixElemList(nl->TwoElemList(nl->SymbolAtom("Direction2"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("D2_TORA"), 
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("D2_LDA"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("D2_ILS"),
                                      nl->SymbolAtom(CcReal::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("D2_PAPI"), 
                                      nl->SymbolAtom(CcBool::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("AirportId"),
                                      nl->SymbolAtom(CcInt::BasicType())));
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("D1_PAPI"),
                                nl->SymbolAtom(CcBool::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("D1_ILS"),
                                nl->SymbolAtom(CcReal::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("D1_LDA"),
                                nl->SymbolAtom(CcInt::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("D1_TORA"), 
                                nl->SymbolAtom(CcInt::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Direction1"),
                                nl->SymbolAtom(CcInt::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Strength"),
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Strength_unit"), 
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Width"),
                                nl->SymbolAtom(CcReal::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Length"),
                                nl->SymbolAtom(CcReal::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("SFC"),
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Name"), 
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  attrs = nl->Cons(nl->TwoElemList(nl->SymbolAtom("Operations"), 
                                nl->SymbolAtom(CcString::BasicType())), attrs);
  return attrs;
}


/*
\subsection{Type Mapping}

*/
ListExpr importairportsTM(ListExpr args) {
  const std::string errMsg = "Expecting a text argument (filename) and two "
                      "string arguments (names for Radio and Runway relations)";
  if (!nl->HasLength(args, 3)) {
    return listutils::typeError(errMsg);
  }
  if (!FText::checkType(nl->First(args))) {
    return listutils::typeError(errMsg);
  }
  if (!CcString::checkType(nl->Second(args))) {
    return listutils::typeError(errMsg);
  }
  if (!CcString::checkType(nl->Third(args))) {
    return listutils::typeError(errMsg);
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), 
                         ImportairportsLI::getResultTypeList());
}

/*
\subsection{Specification}

*/
const std::string importairportsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "(<text> text x string x string -> stream(tuple(Type: string, Country: "
    "string, Name: string, ICAO: string, Pos: point, Elevation: real))"
    "</text--->"
    "<text>importairports( _ , _ , _ )</text--->"
    "<text>Imports an XML file containing airport data. Two relations are "
    "created, and a tuple stream of airports is returned.</text--->"
    "<text>query importairports('openaip_airports_germany_de.aip', \"Radio\", "
                                "\"Runways\")</text--->))";

/*
\subsection{Value Mapping}

*/
int importairportsVM(Word* args, Word& result, int message, Word& local,
                     Supplier s) {
  if (!((FText*)args[0].addr)->IsDefined()) {
    return 0;
  }
  if (!((CcString*)args[1].addr)->IsDefined()) {
    return 0;
  }
  if (!((CcString*)args[2].addr)->IsDefined()) {
    return 0;
  }
  std::string filename = ((FText*)args[0].addr)->GetValue();
  std::string radiorelname = ((CcString*)args[1].addr)->GetValue();
  std::string runwayrelname = ((CcString*)args[2].addr)->GetValue();
  ImportairportsLI *li = static_cast<ImportairportsLI*>(local.addr);
  switch (message) {
    case OPEN: {
      if (li) {
        li = 0;
      }
      li = new ImportairportsLI(filename, radiorelname, runwayrelname);
      local.addr = li;
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->getNextTuple() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (local.addr) {
        li = (ImportairportsLI*)local.addr;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
\subsection{Operator instance}

*/
Operator importairports("importairports",
                importairportsSpec,
                importairportsVM,
                Operator::SimpleSelect,
                importairportsTM);


// --- Constructors
// Constructor
osm::OsmAlgebra::OsmAlgebra () : Algebra ()
{
    AddOperator(&shpimport3);
    shpimport3.SetUsesArgsInTypeMapping();;
    AddOperator(&getconnectivitycode);
    getconnectivitycode.SetUsesArgsInTypeMapping();;
    AddOperator(&binor);
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
    AddOperator(&fullosmimport);
    AddOperator(&divide_osm);
    AddOperator(&divide_osm2);
    AddOperator(&divide_osm3);
//     AddOperator(&convertstreets);
    AddOperator(&importairspaces);
    AddOperator(&importnavaids);
    AddOperator(&importairports);
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
