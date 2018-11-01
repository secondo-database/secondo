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

[1] Implementation of operation createSSPStreamSupplier.

[toc]

1 Operation createSSPStreamSupplier implementation

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"

#include "LoopStrategy/StreamSupplier.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace continuousqueries {

/*
1.1 Operation createSSPStreamSupplier ValueMappings

1.1.1 ValueMapping for Streams of Tuple

*/

int createSSPStreamSupplierTStream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    Word streamWord(Address(0));
    Tuple* tuple;
    StreamSupplier* stsupplier = 
        (StreamSupplier*) local.addr;

    CcString* cchost = (CcString*) args[1].addr;
    CcInt*    ccport = (CcInt*) args[2].addr; 

    std::string host = cchost->GetValue();   
    int port = ccport->GetValue();

    // std::thread stsuThread;
    
    switch(message)
    {
        case OPEN:
        {
            std::cout << " createSSPStreamSupplier: "
                << "ValueMapping: OPEN " << "\n";
            qp->Open(args[0].addr);

            // start the handler
            stsupplier = new StreamSupplier(host, port);

            stsupplier->Initialize();

            local.addr = stsupplier;

            return 0;
        }

        case REQUEST:
        {
            std::cout << " createSSPStreamSupplier: "
                << "ValueMapping: REQUEST " << "\n";

            qp->Request(args[0].addr, streamWord);
            if (qp->Received(args[0].addr)) {
                tuple = static_cast<Tuple*>(streamWord.addr);
                result = tuple;

                // push tuple to the clients
                tuple->IncReference();
                stsupplier->pushTuple(tuple);
                
                return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
            LOG << " createSSPStreamSupplier: ValueMapping: CLOSE " << ENDL;
            if (local.addr != 0) {
                local.setAddr(0);
            }
            
            stsupplier->Shutdown();

            qp->Close(args[0].addr);
            // delete distributeStreamServer;
            local.addr = 0;
            return 0;
        }

    }
    return 0;
}


/*
1.2 Operation createSSPStreamSupplier TypeMapping

*/

ListExpr createSSPStreamSupplier_TM(ListExpr args) {

    std::cout << " createSSPStreamSupplier: TypeMapping " << "\n";
    std::cout << " Argument of the Typemapping: " <<nl->ToString(args)<<"\n";

    std::string err = "Stream(Tuple(...)) x string x int expected";

    if(!nl->HasLength(args,3)) {
        return listutils::typeError("three arguments expected");
    }

    if(!nl->HasLength(nl->First(args), 2)) {
        return listutils::typeError(err);
    }

    ListExpr arg1Type = nl->First(nl->First(args));
    ListExpr arg2Type = nl->Second(args);
    ListExpr arg3Type = nl->Third(args);

    // Check first argument (stream of Tuple)
    if(!Stream<Tuple>::checkType(arg1Type)) {
        return listutils::typeError(err);
    }

    // Check second argument (hostname)
    if (!nl->HasLength(arg2Type, 2))
        return listutils::typeError(" internal error (hostname) ");

    if (!CcString::checkType(nl->First(arg2Type)))
        return listutils::typeError(" string as hostname is expected");

    // Check third argument (portnumber)
    if (!nl->HasLength(arg3Type, 2))
        return listutils::typeError(" internal error (portnumber) ");

    if(!CcInt::checkType(nl->First(arg3Type)))
        return listutils::typeError(" portnummer not an CcInt ");

    if(!listutils::isNumeric(nl->Second(arg3Type)))
        return listutils::typeError(
            " portnumber can't be converted to numeric "
        );

    if(listutils::getNumValue(nl->Second(arg3Type)) < 0 || 
        listutils::getNumValue(nl->Second(arg3Type)) > 65535 )
        return listutils::typeError(
            " portnumber between 0 and 65535 expected "
        );

    return arg1Type;
}

/*
1.3 Operation createSSPStreamSupplier operator selection array

*/
            
ValueMapping createSSPStreamSupplier[] = {
    createSSPStreamSupplierTStream_VM,
};

/*
1.4 Operation createSSPStreamSupplier operator selection

*/

int createSSPStreamSupplier_Select(ListExpr args) {
    return 0;
}

/*
1.5 Operation createSSPStreamSupplier operator specification

*/

const std::string createSSPStreamSupplierOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(stream (tuple([a1:d1, ... ,an:dn])) x string x int)"
" -> (stream (tuple([a1:d1, ... ,an:dn]))))</text--->"
"<text>_ createSSPStreamSupplier [ _, _ ]</text--->"
"<text>Connect a Stream Supplier to a Coordinator "
" specified by address and portnumber</text--->"
"<text>query ten feed createSSPStreamSupplier"
"[\"127.0.0.1\", 12300] consume</text--->"
") )";


/*
1.6 Operation createSSPStreamSupplier

*/

Operator createSSPStreamSupplier_Op(
    "createSSPStreamSupplier",
    createSSPStreamSupplierOpSpec,
    1,
    createSSPStreamSupplier,
    createSSPStreamSupplier_Select,
    createSSPStreamSupplier_TM
);

} /* end of namespace */
