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

[1] Implementation of operation transformtstream.

[toc]

1 Operation transformtstream implementation

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
1.1 Operation transformtstream ValueMapping

*/

int transformtstream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) {

    ListExpr* streamType = 0;

    switch(message)
    {
        case OPEN:
        {
            LOG << " transformtstream: ValueMapping: OPEN " << ENDL;
            qp->Open(args[0].addr);
            // get stream type
            Supplier son = qp->GetSon(s, 0);
            streamType = new ListExpr(qp->GetType(son));
            local.setAddr(streamType);
            return 0;
        }

        case REQUEST:
        {
            LOG << " transformtstream: ValueMapping: REQUEST " << ENDL;
            streamType = (ListExpr*)local.addr;
            Word tupleword(Address(0));
            qp->Request(args[0].addr, tupleword);
            if (qp->Received(args[0].addr)) {
                Tuple* tuple = static_cast<Tuple*>(tupleword.addr);
                TupleDescr* td = new TupleDescr(
                    nl->Second(nl->Second(*streamType)));
                VTuple* vt = new VTuple(tuple, td);
                result = vt;
                return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
            LOG << " transformtstream: ValueMapping: CLOSE " << ENDL;
            if (local.addr != 0) {
                streamType = (ListExpr*)local.addr;
                delete streamType;
                local.setAddr(0);
            }            
            qp->Close(args[0].addr);
            return 0;
        }

    }
    return 0;
}

/*
1.2 Operation transformtstream TypeMapping

*/

ListExpr transformtstream_TM(ListExpr args) {

    LOG << " transformtstream: TypeMapping" << ENDL;
    LOG << " Argument of the Typemapping: " << nl->ToString(args) << ENDL;

    if (nl->ListLength(args) != 1) {
        return listutils::typeError("one argument expected");
    }

    // arg: stream of tuples
    ListExpr arg = nl->First(args);

    cout << nl->ToString(arg) << endl;
    if (!listutils::isTupleStream(arg)) {
        return listutils::typeError("stream(tuple(...)) expected");
    }

    return nl->TwoElemList(listutils::basicSymbol<Stream<VTuple> >(),
                nl->OneElemList(listutils::basicSymbol<VTuple>()));
}

/*
1.3 Operation transformtstream operator specification

*/

const std::string transformtstreamOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>((stream (tuple([a1:d1, ... ,an:dn]))))"
" -> (stream (vtuple(tupledescr tuple([a1:d1, ... ,an:dn])))"
"</text--->"
"<text>_ transformtstream</text--->"
"<text>Transforms a tuple stream to a vtuple stream.</text--->"
"<text>query ten feed transformtstream feed transformvtstream consume</text--->"
") )";

/*
1.4 Operation transformtstream

*/

Operator transformtstream_Op(
    "transformtstream",
    transformtstreamOpSpec,
    transformtstream_VM,
    Operator::SimpleSelect,
    transformtstream_TM
);

} /* end of namespace */
