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
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of operation createSSPHandler.

[toc]

1 Operation createSSPHandler implementation

*/


// #include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "LoopStrategy/HandlerIdle.h"
// #include "Algebras/Stream/Stream.h"
// #include "DistributeStreamServer.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace continuousqueries {

/*
1.1 Operation createSSPHandler TypeMapping

*/

ListExpr createSSPHandler_TM(ListExpr args) {
    // the list is coded as ( (<type> <query part>) (<type> <query part>) )

    std::cout << "createSSPHandler: TypeMapping" << endl;
    std::cout << "Argument: " << nl->ToString(args) << endl;

    // Check for text x int -> int
    if (!nl->HasLength(args,2)) 
        return listutils::typeError(" two arguments are expected");
    
    ListExpr arg1Type = nl->First(args);
    ListExpr arg2Type = nl->Second(args);

    // Check first argument (hostname)
    if (!nl->HasLength(arg1Type, 2))
        return listutils::typeError(" internal error (hostname) ");

    if (!CcString::checkType(nl->First(arg1Type)))
        return listutils::typeError(" string as hostname is expected");

    // Check second argument (portnumber)
    if (!nl->HasLength(arg2Type, 2))
        return listutils::typeError(" internal error (portnumber) ");

    if(!CcInt::checkType(nl->First(arg2Type)))
        return listutils::typeError(" portnummer not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg2Type)))
        return listutils::typeError(
            " portnumber can't be converted to numeric "
        );

    if(listutils::getNumValue(nl->Second(arg2Type)) < 0 || 
        listutils::getNumValue(nl->Second(arg2Type)) > 65535 )
        return listutils::typeError(
            " portnumber between 0 and 65535 expected "
        );

    return arg2Type;
}

/*
1.2 Operation createSSPHandler ValueMappings

*/

int createSSPHandler_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    CcString* cchost = (CcString*) args[0].addr;
    CcInt*    ccport = (CcInt*) args[1].addr; 

    std::string host = cchost->GetValue();   
    int port = ccport->GetValue();

    std::cout << "createSSPHandler: ValueMapping" << endl;
    std::cout << "Creating an idle handler which connects to" << endl;
    std::cout << host << ":" << port << endl << endl;
    
    HandlerIdle handler(host, port);
    handler.Initialize();

    // delete &handler;

    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;
    res->Set(true, 0);
    
    return 0;
}


/*
1.3 Operation createSSPHandler operator selection array

*/
            
ValueMapping createSSPHandler[] = {
    createSSPHandler_VM
};

/*
1.4 Operation createSSPHandler operator selection

*/

int createSSPHandler_Select(ListExpr args) {
    return 0;

    // if (listutils::isTupleStream(nl->First(args)))
    //     return 0;
    // if (VTHelpers::IsVTupleStream(nl->First(args)))
    //     return 1;
    // return -1;
}

/*
1.5 Operation createSSPHandler operator specification

*/

const std::string createSSPHandlerOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>string x int -> int</text--->"
"<text>createSSPHandler ( _, _ )</text--->"
"<text>Creates an idle handler which waits for a role"
" given by the coordinator</text--->"
"<text>query createSSPHandler(\"127.0.0.1\", 54000)"
"</text--->"
") )";


/*
1.6 Operation createSSPHandler

*/

Operator createSSPHandler_Op(
    "createSSPHandler",
    createSSPHandlerOpSpec,
    1,
    createSSPHandler,
    createSSPHandler_Select,
    createSSPHandler_TM
);

} /* end of namespace */
