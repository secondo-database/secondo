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
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/Stream/Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "TestTuples.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace cstream {

int TestOp_VM_0(Word* args, Word& result, int message,
              Word& local, Supplier s) {
    ListExpr* streamType = 0;

    switch(message)
    {
        case OPEN:
        {                      
            qp->Open(args[0].addr);
            // get stream type
            Supplier son = qp->GetSon(s, 0);
            streamType = new ListExpr(qp->GetType(son));
            local.setAddr(streamType);
            return 0;
        }

        case REQUEST:
        {            
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
                //tuple->DeleteIfAllowed();
                //result.addr = tuple;
                //return YIELD;
            } else {
                result.addr = 0;
                return CANCEL;
            }

        }

        case CLOSE:
        {
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

int TestOp_VM_1(Word* args, Word& result, int message,
              Word& local, Supplier s) {    

    Word currentTupleWord(Address(0));
    qp->Open(args[0].addr);
    qp->Request(args[0].addr, currentTupleWord);
    int count = 0;
    while(qp->Received(args[0].addr)) {
        VTuple* vt = static_cast<VTuple*>( currentTupleWord.addr );
        TupleDescr* td = vt->getTupleDescr();
        //Tuple* t = vt->getTuple();
        LOG << "TupleDescr: " << td->GetString() << ENDL;
        
        // Tuple-Implementierung in RelationCommon.cpp / RelationAlgebra.h

        try {

            /*TupleDescr* ptd = new TupleDescr("((PLZ int))");
            bool match;
            Tuple* newt = ptd->ProjectTuple(vt, match);
            if (newt != NULL) {
                LOG << "---Projected Tuple: ---------" << ENDL;
                newt->Print(cout);
                LOG << "-------------------" << ENDL;
                delete newt;
            }
            ptd->DeleteIfAllowed();*/

            // Check tuple type string
            /*ListExpr resulttype;
            nl->ReadFromString("((Id int)(V1(vector(int))))", resulttype);
            ListExpr resultTupleType = nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()), resulttype);
            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numResultTupleType = sc->NumericType(resultTupleType);
            LOG << nl->ToString(numResultTupleType) << ENDL;
            TupleType* tt = new TupleType(numResultTupleType);
            Tuple* t = new Tuple(tt);
            delete t;
            delete tt;*/

            
            //VTuple* vtest = TestTuples::Create4();
            //vtest->getTupleDescr()->Test();     
            //vtest->getTuple()->Print(std::cout);
            //delete vtest;       

            
            // vtdst: contains the tupledescr as parameter
            // vtsrc: is the source tuple
            VTuple* vtsrc = TestTuples::Create4();
            VTuple* vtdst = TestTuples::Create4_1();
            LOG << "---Source-Tuple: --------" << ENDL;
            vtsrc->getTuple()->Print(cout);
            LOG << "-------------------" << ENDL;
            LOG << "---Dest-Tuple: --------" << ENDL;
            vtdst->getTuple()->Print(cout);
            LOG << "-------------------" << ENDL;

            bool match = vtsrc->getTupleDescr()->
            IsProjectable(vtdst->getTupleDescr());
            LOG << "IsProjectable: " << match << ENDL;
            
            Tuple* newt = vtdst->getTupleDescr()->ProjectTuple(vtsrc, match);
            LOG << "Projected: " << match << ENDL;
            if (newt != NULL) {
                LOG << "---Projected Tuple: ---------" << ENDL;
                newt->Print(cout);
                LOG << "-------------------" << ENDL;
                delete newt;
            }

            vtdst->DeleteIfAllowed();
            vtsrc->DeleteIfAllowed();
                        
        } catch (SecondoException& e) {
            std::cout << "eerror:" << e.what() << std::endl;
        }

        //t->Print(cout);

        count++;
        qp->Request(args[0].addr, currentTupleWord);
    }
    qp->Close(args[0].addr);

    result = qp->ResultStorage(s);
    static_cast<CcInt*>( result.addr )->Set(true, count);    
    return 0;
}

ValueMapping testopVM[] = {
    TestOp_VM_0,
    TestOp_VM_1
};


int testopNSelect(ListExpr args) {
    ListExpr arg1 = nl->First(args);
    if (listutils::isTupleStream(arg1))
        return 0;    
    if (VTHelpers::IsVTupleStream(arg1))
        return 1;
    return -1;
}



ListExpr TestOp_TM(ListExpr args) {    
    if (nl->ListLength(args) != 2)
        return listutils::typeError("two argument expected");
    
    //VTHelpers::PrintList("TestOp_TM", args, 3);

    // 1. arg: stream of tuples
    ListExpr arg1 = nl->First(args);
    LOG << "TestOp_TM, arg1: " << nl->ToString(arg1) << ENDL;

    // 2. Argument : mode
    ListExpr arg2 = nl->Second(args);
    LOG << "TestOp_TM, arg2: " << nl->ToString(arg2) << ENDL;

    if (!CcInt::checkType(arg2))
        return listutils::typeError("... x int expected");

    // different modes
    // mode 0: stream(tuple(X)) x int -> stream(vtuple)
    // mode 1: stream(vtuple) x int -> int
    
    ListExpr resType;
    if (listutils::isTupleStream(arg1)) {          
        resType = nl->TwoElemList(listutils::basicSymbol<Stream<VTuple> >(),
                nl->OneElemList(listutils::basicSymbol<VTuple>()));
    } else if (VTHelpers::IsVTupleStream(arg1)) {                
        resType = nl->SymbolAtom(CcInt::BasicType());
    } else
        return listutils::typeError(
            "stream(tuple(...)) x int oder stream(vtuple x int expected"
        );
    
    return resType;
}


const std::string TestOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>((stream (tuple([a1:d1, ... ,an:dn]))))"
" -> (stream (tuple([a1:d1, ... ,an:dn])))"
"</text--->"
"<text>_ testop</text--->"
"<text>For testing purposes.</text--->"
"<text>query twenty feed ten feed concat testop consume</text--->"
     ") )";

Operator testop_Op(
    "testop",
    TestOpSpec,
    2,
    testopVM,
    testopNSelect,
    TestOp_TM
);

} /* end of namespace */
