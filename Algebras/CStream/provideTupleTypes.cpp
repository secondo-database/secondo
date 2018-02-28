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

[1] Implementation of operation provideTupleTypes.

[toc]

1 Operation provideTupleTypes implementation

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "ProvideTupleTypesServer.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace cstream {

/*
1.1 Operation provideTupleTypes ValueMappings

1.1.1 ValueMapping for Streams of Tuple

*/

int provideTupleTypesTStream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    // T* streamType = 0;
    Word streamWord(Address(0));
    ProvideTupleTypesServer* provTupleTypesServer = 
        (ProvideTupleTypesServer*) local.addr;

    CcInt* port = (CcInt*) args[1].addr;

    switch(message)
    {
        case OPEN:
        {
            LOG << " provideTupleTypes: ValueMapping: OPEN " << ENDL;
            qp->Open(args[0].addr);
            provTupleTypesServer = 
                new ProvideTupleTypesServer(port->GetIntval());
            provTupleTypesServer->
                addTupleDescr(new TupleDescr(
                    nl->Second(nl->Second(qp->GetType(s)))));
            local.addr = provTupleTypesServer;
            provTupleTypesServer->run();
            return 0;
        }

        case REQUEST:
        {
            LOG << " provideTupleTypes: ValueMapping: REQUEST " << ENDL;

            qp->Request(args[0].addr, streamWord);
            if (qp->Received(args[0].addr)) {
                result = streamWord.addr;
                return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
            LOG << " provideTupleTypes: ValueMapping: CLOSE " << ENDL;
            sleep(20);
            LOG << " sleep end" << ENDL;
            if (local.addr != 0) {
                local.setAddr(0);
            }
            qp->Close(args[0].addr);
            delete provTupleTypesServer;
            local.addr = 0;
            return 0;
        }

    }
    return 0;
}

/*
1.1.2 ValueMapping for Streams of VTuple

*/

int provideTupleTypesVTStream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    Word streamWord(Address(0));
    ProvideTupleTypesServer* provTupleTypesServer = 
        (ProvideTupleTypesServer*) local.addr;

    CcInt* port = (CcInt*) args[1].addr;
    CcInt* cacheSize = (CcInt*) args[2].addr;
    VTuple* vt;

    switch(message)
    {
        case OPEN:
        {
            LOG << " provideTupleTypes: ValueMapping: OPEN " << ENDL;
            qp->Open(args[0].addr);
            provTupleTypesServer = 
                new ProvideTupleTypesServer(port->GetIntval(), 
                                            cacheSize->GetIntval());
            local.addr = provTupleTypesServer;
            provTupleTypesServer->run();
            return 0;
        }

        case REQUEST:
        {
            LOG << " provideTupleTypes: ValueMapping: REQUEST " << ENDL;

            qp->Request(args[0].addr, streamWord);
            if (qp->Received(args[0].addr)) {
                vt = static_cast<VTuple*>(streamWord.addr);
                LOG << vt->getTupleDescr()->GetString() << ENDL;
                // provTupleTypesServer->
                //     addTupleDescr(new TupleDescr(*(vt->getTupleDescr())));
                provTupleTypesServer->
                    addTupleDescr(vt->getTupleDescr());
                result = streamWord.addr;
                return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
            LOG << " provideTupleTypes: ValueMapping: CLOSE " << ENDL;
            if (local.addr != 0) {
                local.setAddr(0);
            }
            qp->Close(args[0].addr);
            delete provTupleTypesServer;
            local.addr = 0;
            return 0;
        }

    }
    return 0;
}

/*
1.2 Operation provideTupleTypes TypeMapping

*/

ListExpr provideTupleTypes_TM(ListExpr args) {

    LOG << " provideTupleTypes: TypeMapping " << ENDL;
    LOG << " Argument of the Typemapping: " << nl->ToString(args) << ENDL;

    std::string err = 
      "Stream(VTuple(...)) x int x int or Stream(Tuple(...)) x int expected";

    if(nl->HasLength(args,2)) {

        if(!nl->HasLength(nl->First(args), 2)) {
            return listutils::typeError(err);
        }

        ListExpr arg1Type = nl->First(nl->First(args));
        ListExpr arg2Type = nl->Second(args);

        // Check for Stream(Tuple(...)) x int
        if (nl->HasLength(arg1Type,2)) {
            
            if(!Stream<Tuple>::checkType(arg1Type)) {
                return listutils::typeError(err);
            }
            
            // Check the second argument, has to be a valid portnumber
            if (!nl->HasLength(arg2Type,2)) {
                return listutils::typeError(err);
            }
            if(!CcInt::checkType(nl->First(arg2Type))) {
                return listutils::typeError(err);
            }
            if(!listutils::isNumeric(nl->Second(arg2Type))) {
                return listutils::typeError(err);
            }
            if(listutils::getNumValue(nl->Second(arg2Type)) < 0 || 
                listutils::getNumValue(nl->Second(arg2Type)) > 65535 ) {
                return listutils::typeError(
                    "portnumber between 0 and 65535 expected");
            }

        }

        LOG << nl->ToString(nl->Second(nl->Second(arg1Type))) << ENDL;

        TupleDescr* td = new TupleDescr(nl->Second(nl->Second(arg1Type)));
        LOG << td->GetString() << ENDL;

        return arg1Type;

    }

    // Check for Stream(VTuple(...)) x int x int
    if (!nl->HasLength(args,3)) {
        return listutils::typeError("two or three arguments expected");
    }

    if(!nl->HasLength(nl->First(args), 2)) {
            return listutils::typeError(err);
    }

    ListExpr arg1Type = nl->First(nl->First(args));
    ListExpr arg2Type = nl->Second(args);
    ListExpr arg3Type = nl->Third(args);

    if(!VTHelpers::IsVTupleStream(arg1Type)) {
        return listutils::typeError(err);
    }

    // Check the second argument, has to be a valid portnumber
    if (!nl->HasLength(arg2Type,2)) {
        return listutils::typeError(err);
    }
    if(!CcInt::checkType(nl->First(arg2Type))) {
        return listutils::typeError(err);
    }
    if(!listutils::isNumeric(nl->Second(arg2Type))) {
        return listutils::typeError(err);
    }
    if(listutils::getNumValue(nl->Second(arg2Type)) < 0 || 
        listutils::getNumValue(nl->Second(arg2Type)) > 65535 ) {
        return listutils::typeError("portnumber between 0 and 65535 expected");
    }
    
    // Check the third argument, has to be a number > -1
    if (!nl->HasLength(arg3Type,2)) {
        return listutils::typeError(err);
    }
    if(!CcInt::checkType(nl->First(arg3Type))) {
        return listutils::typeError(err);
    }
    if(!listutils::isNumeric(nl->Second(arg3Type))) {
        return listutils::typeError(err);
    }
    if( listutils::getNumValue(nl->Second(arg3Type)) < -1 || 
        listutils::getNumValue(nl->Second(arg3Type)) == 0) {
        return listutils::typeError("Value >0 or -1 expected");
    }

    return arg1Type;
}

/*
1.3 Operation provideTupleTypes operator selection array

*/
            
ValueMapping provideTupleTypes[] = {
    provideTupleTypesTStream_VM,
    provideTupleTypesVTStream_VM
};

/*
1.4 Operation provideTupleTypes operator selection

*/

int provideTupleTypes_Select(ListExpr args) {
    if (listutils::isTupleStream(nl->First(args)))
        return 0;
    if (VTHelpers::IsVTupleStream(nl->First(args)))
        return 1;
    return -1;
}

/*
1.5 Operation provideTupleTypes operator specification

*/

const std::string provideTupleTypesOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(stream (tuple([a1:d1, ... ,an:dn])) x int)"
" -> (stream (tuple([a1:d1, ... ,an:dn]))))"
"</text--->"
"<text>_ provideTupleTypes [ _ ]</text--->"
"<text>Provides the Types of a stream of tuple oder vtuple to"
" a given portnumber</text--->"
"<text>query ten feed provideTupleTypes"
"[81] consume</text--->"
") )";


/*
1.6 Operation provideTupleTypes

*/

Operator provideTupleTypes_Op(
    "provideTupleTypes",
    provideTupleTypesOpSpec,
    2,
    provideTupleTypes,
    provideTupleTypes_Select,
    provideTupleTypes_TM
);

} /* end of namespace */
