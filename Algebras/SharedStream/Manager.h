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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

*/

#ifndef DS_MANAGER_H
#define DS_MANAGER_H

#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Stream.h"  // wrapper for secondo streams

//#include "GenericTC.h"          // use of generic type constructors

#include "LogMsg.h"             // send error messages

#include "Tools/Flob/DbArray.h"  // use of DbArrays

#include "Algebras/Relation-C++/RelationAlgebra.h" // use of tuples


#include <math.h>               // required for some operators
#include <stack>
#include <limits>
#include <boost/numeric/ublas/vector.hpp>
#include <vector>

/*
Global Variables

Secondo uses some variables designed as singleton pattern. For accessing these
global variables, these variables have to be declared to be extern:

*/

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace sharedstream {
    class Manager {

    public:


 /*function (operator) for adding a new StreamSource manually
 * This function will also update the data to choose from
 * the webinterface*/
        void addSource(std::string sourceName, std::string ipAdress,
                       std::string port, ListExpr tupleDescription);

//function to receive data from web interface
        void registeredQuery(std::string query);

/*function to start a StreamProcessor and transfering the needed data for
 connection to a StreamSource*/


/*Funktion von Thomas
 * TODO: Includes rausbekommen, so kompiliert es nicht,
 * Distributed2 zu includen reicht nicht, die includes werden
 * anscheinend nicht mitgenommen
ConnectionInfo *con = ConnectionInfo::createConnection
 ("localhost", 1234, "SecondoConfig.ini");

if(!con){
    cerr << "Cold not connect to Secondo" << endl; //Fehlerbehandlung
    return;
}

if(!con->switchDatabase("newdatabase", true, false)){
    cerr << "switching to database failed" << endl;
    return;
}

string cmd = "let myObject = 10";
int errorCode;
string errorMsg;
string resultList;
double runtime;
Commandlog log;

con->
simpleCommand(cmd, errorCode, errorMsg, resultList,
false, runtime, false, false, log);
if(errorCode==0){
    cout << "command " << cmd << " successful" << endl;
} else {
    cout << "command " << cmd << " failed " << endl << errorMsg << endl;
}*/

//function to add a new constant to a StreamProcessor (query)

    private:
        /*Struct for saving the SourceEntrys*/

        struct SourceEntry {
            std::string SourceName;
            std::string ipAdress;
            std::string port;
            ListExpr tupleDescription;
        };
/*Vector for storing the SourceEntries*/
        std::vector<SourceEntry> SourceEntries;


    };
}

#endif //DS_MANAGER_H


