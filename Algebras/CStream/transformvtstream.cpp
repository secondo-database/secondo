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

[1] Implementation of operation transformvtstream.

[toc]

1 Operation transformvtstream implementation

*/


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace cstream {

/*
1.1 Operation transformvtstream ValueMapping

*/

int transformvtstream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    TupleDescr* streamType = 0;
    Word vtupleword(Address(0));

    switch(message)
    {
        case OPEN:
        {
            LOG << " transformvtstream: ValueMapping: OPEN " << ENDL;
            qp->Open(args[0].addr);
            streamType = (TupleDescr*)(args[1].addr);
            /*std::string sparam = (std::string)(char*)
            ((CcString*)args[1].addr)->GetStringval();
            streamType = new TupleDescr(sparam);
                //(((CcString*)args[1].addr))->GetValue());*/
            //LOG << " TupleDescr: " << (streamType->GetString()) << ENDL;
            local.addr = streamType;
            return 0;
        }

        case REQUEST:
        {
            //LOG << " transformvtstream: ValueMapping: REQUEST " << ENDL;
            streamType = (TupleDescr*)local.addr;

            do {
                qp->Request(args[0].addr, vtupleword);
                if (qp->Received(args[0].addr)) {
                    VTuple* vt = static_cast<VTuple*>(vtupleword.addr);
                    bool projected;
                    Tuple* t = streamType->ProjectTuple(vt, projected);
                    vt->DeleteIfAllowed();
                                        
                    if (t == NULL) {
                        // Tuple doesn't match, request next tuple
                    } else {
                        result = t;
                        return YIELD;
                    }
                } else {
                    result.addr = 0;
                    return CANCEL;
                }
            } while (true);
        }
        break;

        case CLOSE:
        {
            LOG << " transformvtstream: ValueMapping: CLOSE " << ENDL;
            if (local.addr != 0) {
                // todo: find out why this causes a crash
//                streamType = (TupleDescr*)local.addr;
//                delete streamType;
                local.setAddr(0);
            }            
            qp->Close(args[0].addr);
            return 0;
        }

    }
    return 0;
}

/*
1.2 Operation transformvtstream TypeMapping

*/

ListExpr transformvtstream_TM(ListExpr args) {

    LOG << " transformvtstream: TypeMapping " << ENDL;
    LOG << " Argument of the Typemapping: " << nl->ToString(args) << ENDL;
    VTHelpers::PrintList("transformvtstream_arg", args, 2);

    if (!nl->HasLength(args,2)) {
        return listutils::typeError("two arguments expected");
    }

    // First(args): the list is codes as (<type> <query part>) 
    // (must be <stream(vtuple)> <query>)
    if(!nl->HasLength(nl->First(args),2)){
        return listutils::typeError("internal error");
    }
    ListExpr arg1Type = nl->First(nl->First(args));


    // Second(args): is the second parameter of the operator 
    // (must be a tupledescr)
    if(!nl->HasLength(nl->Second(args),2)){
        return listutils::typeError("internal error");
    }
    ListExpr arg2Type = nl->First(nl->Second(args));
    ListExpr arg2Value = nl->Second(nl->Second(args));

    if (!VTHelpers::IsVTupleStream(arg1Type)) {
        return listutils::typeError("stream(vtuple(...)) expected");
    }

    if (!TupleDescr::CheckType(arg2Type)) {
        return listutils::typeError("TupleDescr expected");
    }

    // we need to get the value of the second argument (tupledescr)
    ListExpr expression = arg2Value;
    // we need some variables for feeding the ExecuteQuery function
    Word queryResult;
    std::string typeString = "";
    std::string errorString = "";
    bool correct;
    bool evaluable;
    bool defined;
    bool isFunction;

    // use the queryprocessor for executing the expression
    qp->ExecuteQuery(expression, queryResult, 
                    typeString, errorString, correct, 
                    evaluable, defined, isFunction);
    // check correctness of the expression
    if(!correct || !evaluable || !defined || isFunction) {
        assert(queryResult.addr == 0);
        return listutils::typeError("could not extract tupledescr");
    }

    TupleDescr* td = (TupleDescr*) queryResult.addr;
    assert(td);
    if(!td->IsDefined()) {
        return listutils::typeError("tupledescr undefined");
    }

    // Create the target stream of tuples from the tupledescr
    ListExpr tdList;
    if(!nl->ReadFromString(td->GetString(), tdList)) {
        return listutils::typeError("Error in tupledescr");
    }

    return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                tdList));
}

/*
1.3 Operation transformtstream operator specification

*/

const std::string transformvtstreamOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(stream (vtuple(tupledescr tuple([a1:d1, ... ,an:dn]))) x tupledescr)"
" -> (stream (tuple([a1:d1, ... ,an:dn]))))"
"</text--->"
"<text>_ transformvtstream [ _ ]</text--->"
"<text>Transforms a vtuple stream to a tuple stream.</text--->"
"<text>query ten feed transformtstream feed transformvtstream"
"['((OP int))'] consume</text--->"
") )";


/*
1.4 Operation transformvtstream

*/

Operator transformvtstream_Op(
    "transformvtstream",
    transformvtstreamOpSpec,
    transformvtstream_VM,
    Operator::SimpleSelect,
    transformvtstream_TM
);

} /* end of namespace */
