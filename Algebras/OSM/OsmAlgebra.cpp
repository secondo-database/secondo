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
#include "../Spatial/Geoid.h"
#include "../Spatial/SpatialAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include "ShpFileReader.h"

// --- Enabling global pointer variables
extern NestedList* nl;
extern QueryProcessor* qp;

// --- Announcing global functions from ImExAlgebra.cpp
extern string getShpType (const string fname, bool& correct,
                          string& errorMessage);
template<int filePos>
extern int shpimportVM (Word* args, Word& result, int message,
                        Word& local, Supplier s);

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

   shpType = getShpType(name, correct, errmsg);
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

// --- Constructors
// Constructor
osm::OsmAlgebra::OsmAlgebra () : Algebra ()
{
    AddOperator(&shpimport3);
    shpimport3.SetUsesArgsInTypeMapping();;
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
