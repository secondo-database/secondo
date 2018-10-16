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

[1] Implementation of operation createSSPCoordinator.

[toc]

1 Operation createSSPCoordinator implementation

*/


// #include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include <boost/algorithm/string.hpp>
#include "LoopStrategy/CoordinatorLoop.h"

// #include "Algebras/Stream/Stream.h"
// #include "DistributeStreamServer.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace continuousqueries {

/*
1.1 Operation createSSPCoordinator TypeMapping

*/

ListExpr createSSPCoordinator_TM(ListExpr args) {
    // the list is coded as ( (<type> <query part>) (<type> <query part>) )

    std::cout << "createSSPCoordinator: TypeMapping" << endl;
    std::cout << "Argument: " << nl->ToString(args) << endl;

    // Check for text x int -> int
    if (!(nl->HasLength(args,1) || nl->HasLength(args,2))) 
        return listutils::typeError(" one or two arguments are expected");
    
    ListExpr arg1Type = nl->First(args);

    // Check first argument (valid portnumber)
    if (!nl->HasLength(arg1Type, 2))
        return listutils::typeError(" internal error (portnumber) ");

    if(!CcInt::checkType(nl->First(arg1Type)))
        return listutils::typeError(" portnummer not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg1Type)))
        return listutils::typeError(
            " portnumber can't be converted to numeric "
        );

    if(listutils::getNumValue(nl->Second(arg1Type)) < 0 || 
        listutils::getNumValue(nl->Second(arg1Type)) > 65535 )
        return listutils::typeError(
            " portnumber between 0 and 65535 expected "
        );

    // Check sercond argument (command)
    ListExpr arg2Type = nl->Second(args);

    if (!nl->HasLength(arg2Type, 2))
        return listutils::typeError(" internal error (command) ");

    if (!CcString::checkType(nl->First(arg2Type)))
        return listutils::typeError(" string as command is expected");

    return arg1Type;
}

/*
1.2 Operation createSSPCoordinator ValueMappings

*/

int createSSPCoordinator_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    std::cout << "createSSPCoordinator: ValueMapping" << endl;

    CcInt*   ccport = (CcInt*) args[0].addr; 
    CcString* cccmd = (CcString*) args[1].addr;
    std::string cmd = cccmd->GetValue();
    boost::to_upper(cmd);
    int port = ccport->GetValue();

    std::cout << "Creating the specified Coordinator..." << endl;

    if (cmd == "LOOP") {
        std::cout << "Creating a Loop Coordinator..." << endl;
        CoordinatorLoop coordinator(port);
        coordinator.Run();
        
    } else if (cmd == "RECOVER") {
        std::cout << "Recover from a crashed Coordinator..." << endl;
        std::cout << "TBD. Quitting..." << endl;
    } else {
        std::cout << "No known command given. Quitting..." << endl;            
    }
    
    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;
    res->Set(true, port);
    return 0;
}


/*
1.3 Operation createSSPCoordinator operator selection array

*/
            
ValueMapping createSSPCoordinator[] = {
    createSSPCoordinator_VM
};

/*
1.4 Operation createSSPCoordinator operator selection

*/

int createSSPCoordinator_Select(ListExpr args) {
    return 0;
}

/*
1.5 Operation createSSPCoordinator operator specification

*/

const std::string createSSPCoordinatorOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x string -> int</text--->"
"<text>createSSPCoordinator ( _, _ )</text--->"
"<text>Creates the coordinator for the SSP system."
" </text--->"
"<text>query createSSPCoordinator(54000, \"loop\")"
"</text--->"
") )";


/*
1.6 Operation createSSPCoordinator

*/

Operator createSSPCoordinator_Op(
    "createSSPCoordinator",
    createSSPCoordinatorOpSpec,
    1,
    createSSPCoordinator,
    createSSPCoordinator_Select,
    createSSPCoordinator_TM
);

} /* end of namespace */
