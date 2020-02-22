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

[1] Implementation of operation distributeStream.

[toc]

1 Operation distributeStream implementation

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "DistributeStreamServer.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace cstream {

/*
1.1 Operation distributeStream ValueMappings

1.1.1 ValueMapping for Streams of Tuple

*/

int distributeStreamTStream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    // T* streamType = 0;
    Word streamWord(Address(0));
    Tuple* tuple;
    DistributeStreamServer* distributeStreamServer = 
        (DistributeStreamServer*) local.addr;

    CcInt* port = (CcInt*) args[1].addr;
    CcBool* format = (CcBool*) args[2].addr;

    LOG << "Port:" << port->GetIntval() << ENDL;
    LOG << "Format:" << format->GetValue() << ENDL;

    switch(message)
    {
        case OPEN:
        {
            LOG << " distributeStream: ValueMapping: OPEN " << ENDL;
            qp->Open(args[0].addr);

            // start the server
            distributeStreamServer = 
                new DistributeStreamServer( port->GetIntval(), 
                                            format->GetValue());
            distributeStreamServer->run();
            local.addr = distributeStreamServer;

            return 0;
        }

        case REQUEST:
        {
            LOG << " distributeStream: ValueMapping: REQUEST " << ENDL;

            qp->Request(args[0].addr, streamWord);
            if (qp->Received(args[0].addr)) {
                tuple = static_cast<Tuple*>(streamWord.addr);
                result = tuple;

                // push tuple to the clients
                tuple->IncReference();
                distributeStreamServer->pushTuple(
                    new VTuple(tuple, 
                    new TupleDescr(nl->Second(nl->Second(qp->GetType(s))))));
                
                return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
            LOG << " distributeStream: ValueMapping: CLOSE " << ENDL;
            if (local.addr != 0) {
                local.setAddr(0);
            }
            qp->Close(args[0].addr);
            delete distributeStreamServer;
            local.addr = 0;
            return 0;
        }

    }
    return 0;
}

/*
1.1.2 ValueMapping for Streams of VTuple

*/

int distributeStreamVTStream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    // T* streamType = 0;
    Word streamWord(Address(0));
    VTuple* vt;
    DistributeStreamServer* distributeStreamServer = 
        (DistributeStreamServer*) local.addr;

    CcInt* port = (CcInt*) args[1].addr;
    CcBool* format = (CcBool*) args[2].addr;

    LOG << "Port:" << port->GetIntval() << ENDL;
    LOG << "Format:" << format->GetValue() << ENDL;

    switch(message)
    {
        case OPEN:
        {
            LOG << " distributeStream: ValueMapping: OPEN " << ENDL;
            qp->Open(args[0].addr);

            // start the server
            distributeStreamServer = 
                new DistributeStreamServer( port->GetIntval(), 
                                            format->GetValue());
            distributeStreamServer->run();
            local.addr = distributeStreamServer;

            return 0;
        }

        case REQUEST:
        {
            LOG << " distributeStream: ValueMapping: REQUEST " << ENDL;

            qp->Request(args[0].addr, streamWord);
            if (qp->Received(args[0].addr)) {
                vt = static_cast<VTuple*>(streamWord.addr);
                result = vt;

                // push tuple to the clients
                vt->IncReference();
                distributeStreamServer->pushTuple(vt);
                
                return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
            LOG << " distributeStream: ValueMapping: CLOSE " << ENDL;
            if (local.addr != 0) {
                local.setAddr(0);
            }
            qp->Close(args[0].addr);
            delete distributeStreamServer;
            local.addr = 0;
            return 0;
        }

    }
    return 0;
}

/*
1.2 Operation distributeStream TypeMapping

*/

ListExpr distributeStream_TM(ListExpr args) {

    LOG << " distributeStream: TypeMapping " << ENDL;
    LOG << " Argument of the Typemapping: " << nl->ToString(args) << ENDL;

    std::string err = "Stream(VTuple(...)) x int x bool ";
    err += "or Stream(Tuple(...)) x int x bool expected";

    if(!nl->HasLength(args,3)) {
        return listutils::typeError("two or three arguments expected");
    }

    if(!nl->HasLength(nl->First(args), 2)) {
        return listutils::typeError(err);
    }

    ListExpr arg1Type = nl->First(nl->First(args));
    ListExpr arg2Type = nl->Second(args);
    ListExpr arg3Type = nl->Third(args);

    // Check first argument (stream of VTuple or Tuple)
    if(!VTHelpers::IsVTupleStream(arg1Type) 
    && !Stream<Tuple>::checkType(arg1Type)) {
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
    
    // Check the third argument (bool)
    if (!nl->HasLength(arg3Type,2)) {
        return listutils::typeError(err);
    }
    if(!CcBool::checkType(nl->First(arg3Type))) {
        return listutils::typeError(err);
    }

    return arg1Type;
}

/*
1.3 Operation distributeStream operator selection array

*/
            
ValueMapping distributeStream[] = {
    distributeStreamTStream_VM,
    distributeStreamVTStream_VM
};

/*
1.4 Operation distributeStream operator selection

*/

int distributeStream_Select(ListExpr args) {
    if (listutils::isTupleStream(nl->First(args)))
        return 0;
    if (VTHelpers::IsVTupleStream(nl->First(args)))
        return 1;
    return -1;
}

/*
1.5 Operation distributeStream operator specification

*/

const std::string distributeStreamOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(stream (TTYPE([a1:d1, ... ,an:dn])) x int x bool)"
" -> (stream (TTYPE([a1:d1, ... ,an:dn])))), "
" TTYPE in {vtuple, tuple} </text--->"
"<text>_ distributeStream [ _, _ ]</text--->"
"<text>Distribute a stream of tuple oder vtuple to"
" a given portnumber. The bool parameter determines"
" the tuple format for the transfer. true means"
" binary, false means nested list.</text--->"
"<text>query ten feed distributeStream"
"[81, FALSE] consume</text--->"
") )";


/*
1.6 Operation distributeStream

*/

Operator distributeStream_Op(
    "distributeStream",
    distributeStreamOpSpec,
    2,
    distributeStream,
    distributeStream_Select,
    distributeStream_TM
);

} /* end of namespace */
