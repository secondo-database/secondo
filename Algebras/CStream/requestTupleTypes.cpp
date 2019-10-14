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

[1] Implementation of operation requestTupleTypes.

[toc]

1 Operation requestTupleTypes implementation

*/
 


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "ProvideTupleTypesServer.h"
#include "RequestTupleTypesClient.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace cstream 
{

/*
1.1 Operation requestTupleTypes ValueMapping 

*/

int requestTupleTypes_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) 
{

LOG << " requestTupleTypes: ValueMapping: Entering " << ENDL;
    
    Word streamWord(Address(0));
    RequestTupleTypesClient* requestTupleTypesClient = 
        (RequestTupleTypesClient*) local.addr;

    CcString* hostname = (CcString*)args[0].addr;
    CcInt* port = (CcInt*)args[1].addr;
    CcInt* numOfTypes = (CcInt*)args[2].addr;

    switch(message)
    {
        
        case OPEN:
        {
            LOG << " requestTupleTypes: ValueMapping: OPEN " << ENDL;
            
            requestTupleTypesClient = 
                new RequestTupleTypesClient(hostname->GetValue(),
                                            port->GetIntval(),
                                            numOfTypes->GetIntval());
            requestTupleTypesClient->start();
            local.addr = requestTupleTypesClient;
            return 0;
        }

        case REQUEST:
        {
            LOG << " provideTupleTypes: ValueMapping: REQUEST " << ENDL;
            TupleDescr* td;

            if ((td = requestTupleTypesClient->receiveTupleDescr())) {

                ListExpr resultTupleType = nl->TwoElemList(
                    nl->SymbolAtom(Tuple::BasicType()),
                    nl->OneElemList(
                        nl->TwoElemList(nl->SymbolAtom("Type"),
                        nl->SymbolAtom(TupleDescr::BasicType()))));
                
                SecondoCatalog* sc = SecondoSystem::GetCatalog();
                ListExpr numResultTupleType = sc->NumericType(resultTupleType);

                Tuple* t = new Tuple(new TupleType(numResultTupleType));
                t->PutAttribute(0, td);
                result = t;

                return YIELD;
            }
            else {
                result.addr = 0;
                return CANCEL;
            }
        }

        case CLOSE:
        {
            LOG << " requestTupleTypes: ValueMapping: CLOSE " << ENDL;
            
            if (local.addr != 0)  {
                local.setAddr(0);
            }
            delete requestTupleTypesClient;
            local.addr = 0;
            return 0;
        }
    }
    return 0;
}

/*
1.2 Operation requestTupleTypes TypeMapping

*/

ListExpr requestTupleTypes_TM(ListExpr args) {

    LOG << " requestTupleTypes: TypeMapping " << ENDL;
    LOG << " Argument of the Typemapping: " << nl->ToString(args) << ENDL;

    // Check for text x int x int
    if (!nl->HasLength(args,3)) 
        return listutils::typeError(" three arguments are expected");
    
    // Check first argument (hostname)
    if (!nl->HasLength(nl->First(args), 2)) 
        return listutils::typeError(" string as hostname is expected");
    if (!CcString::checkType(nl->First(nl->First(args))))
        return listutils::typeError(" string as hostname is expected");

    // Check second argument (portnumber)
    if (!nl->HasLength(nl->Second(args), 2)) 
        return listutils::typeError(" int as portnumber is expected");
    if(!CcInt::checkType(nl->First(nl->Second(args)))) 
        return listutils::typeError(" int as portnumber is expected");
    if(listutils::getNumValue(nl->Second(nl->Second(args))) < 0 || 
                listutils::getNumValue(nl->Second(nl->Second(args))) > 65535 )
                return listutils::typeError(
                    " portnumber between 0 and 65535 expected");
    
    // Check third argument (max tupledescr)
    if (!nl->HasLength(nl->Third(args), 2)) 
        return listutils::typeError(" int as maxnumber of types is expected");
    if(!CcInt::checkType(nl->First(nl->Third(args)))) 
        return listutils::typeError(" int as portnumber is expected");
    if(listutils::getNumValue(nl->Second(nl->Third(args))) < -1)
                return listutils::typeError(
                    " portnumber between 0 and 65535 expected");

    LOG << "target type: " << ENDL;
    LOG << 
        nl->ToString(nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(listutils::basicSymbol<Tuple>(), 
                nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Type"), 
                nl->SymbolAtom(TupleDescr::BasicType())))))) 
    << ENDL;

    // target type is stream(tuple((Type tupledescr)))
    return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                nl->OneElemList(nl->TwoElemList(nl->SymbolAtom("Type"), 
                nl->SymbolAtom(TupleDescr::BasicType())))));
}

/*
1.3 Operation requestTupleTypes operator specification

*/
const std::string requestTupleTypesOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>"
" -> (stream (tuple(Type tupledescr))))"
"</text--->"
"<text>requestTupleTypes (_, _,_)</text--->"
"<text>Requests  a stream of tuple of Type tupledescr to"
" a given hostname,portnumber, max number of Types</text--->"
"<text>query requestTupleTypes(""localhost"", 8080, 1) consume</text--->"
") )";


/*
1.4 Operation requestTupleTypes

*/

Operator requestTupleTypes_Op(
    "requestTupleTypes",
    requestTupleTypesOpSpec,
    requestTupleTypes_VM,
    Operator::SimpleSelect,
    requestTupleTypes_TM
);

} /* end of namespace */
