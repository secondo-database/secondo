 
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

[1] Implementation of operation receiveStream.

[toc]

1 Operation registerForStream implementation

*/
 


#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "RegisterForStreamClient.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace cstream 
{

/*
1.1 Operation registerForStream ValueMapping 

*/
int registerForStream_VM(Word* args, Word& result, int message,
              Word& local, Supplier s) 
{

    LOG << " registerForStream: ValueMapping: Entering " << ENDL;

    Word streamWord(Address(0));
    
    RegisterForStreamClient* registerForStreamClient = 
        (RegisterForStreamClient*) local.addr;

    switch(message)
    {
        
        case OPEN:
        {
            LOG << " registerForStream: ValueMapping: OPEN " << ENDL;
   
            CcString* hostname = (CcString*) args[0].addr;
            CcInt* port = (CcInt*) args[1].addr;
            TupleDescr* tdType = (TupleDescr*) args[2].addr;
            FText* filterFunction = (FText*) args[4].addr;

            LOG << " filterFunction is = " 
                << filterFunction->GetValue() << ENDL;   
      
            registerForStreamClient = 
                new RegisterForStreamClient(hostname->GetValue(),
                                            port->GetIntval(),
                                            tdType, filterFunction->GetValue());
                
            local.addr = registerForStreamClient;
            int startRet = registerForStreamClient->start();

            if(startRet) {
                LOG << " registerForStream: got start error=" 
                    << startRet << ENDL;
                return 0;
            }
            
            return 0;
        }

        case REQUEST:
        {
            LOG << " registerForStream: ValueMapping: REQUEST " << ENDL;
          
            // Operator not ready
            if(! registerForStreamClient) {
                LOG << " registerForStream: Operator not ready" << ENDL;
                result.addr = 0;
                return CANCEL;
            }

            bool connectionOk = registerForStreamClient->connectionOK();
           
            if(! connectionOk) {
                LOG << " registerForStream: Conenction is not ok" << ENDL;
                result.addr = 0;
                return CANCEL;
            }

            // Everything is ok, try to fetch a tuple from the server        
            Tuple* tuple = registerForStreamClient->receiveTuple();

            if (tuple) {
                LOG << "Tuple received" << ENDL;
                result.addr = tuple;
                return YIELD;
            } else {
                LOG << "No more Tuples" << ENDL;
                result.addr = 0;
                return CANCEL;
            }
        }

        case CLOSE:
        {
            LOG << " registerForStream: ValueMapping: CLOSE " << ENDL;
            
            // Delete local operator state
            if (registerForStreamClient) {
                delete registerForStreamClient;
                local.setAddr(NULL);
            }

            return 0;
        }
    }
    return 0;
}

/*
1.2 convert a list to a symbol atom

*/
ListExpr convertListToSymbol(ListExpr input) {
    return nl->OneElemList(nl->TextAtom(nl->ToString(input)));
}

/*
1.2 User filter function type mapping

*/
ListExpr generateUserFilter_TM(ListExpr userFilter) {

    ListExpr funFilterFunktionList = nl->Second(userFilter);
    if(!nl->HasLength(funFilterFunktionList,3)){
        return listutils::typeError("Invalid function definition");
    }

    // No filter should be used
    std::string filterString = nl->ToString(nl->Third(funFilterFunktionList));
    if(filterString.compare("TRUE") == 0){
        LOG << "  No filter is used" << ENDL;
        return convertListToSymbol(nl->TheEmptyList());
    }

    ListExpr origArg = nl->Second(funFilterFunktionList);
    if(!nl->HasLength(origArg,2)){ // name type
        return listutils::typeError("Invalid function definition");
    }

    // replace function argument type (may be TTYPE3) by the real type
    ListExpr funArgType = nl->Second(nl->First(userFilter)); 
    ListExpr finalFunArgType = nl->TwoElemList(nl->First(origArg), funArgType);
    ListExpr fixedFuncList = nl->ThreeElemList(
                      nl->First(funFilterFunktionList),
                      finalFunArgType,
                      nl->Third(funFilterFunktionList));

    return convertListToSymbol(fixedFuncList);
}

/*
1.2 Tupledescr type calculation

*/
ListExpr generateType_TM(ListExpr expression) {

    LOG << "   -> Expression is " << nl->ToString(expression) << ENDL;

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

    return nl->TwoElemList(
          listutils::basicSymbol<Stream<Tuple>>(),
             nl->TwoElemList(listutils::basicSymbol<Tuple>(), tdList)
          );
}

/*
1.2 Operation registerForStream TypeMapping

*/
ListExpr registerForStream_TM(ListExpr args) {

    LOG << " registerForStream: TypeMapping " << ENDL;
    LOG << " Argument of the Typemapping: " << nl->ToString(args) << ENDL;

    // Check for text x int x tupledescr x tupledescr(x)-> bool
    if (!nl->HasLength(args,4)) 
        return listutils::typeError(" four arguments are expected");
    
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
    
    // Check third argument (types of tupledescr)
    // e.g. (tupledescr (tupledescr '((PLZ int) (Ort string))'))
    // e.g. (tupledescr plztype)
    if (!nl->HasLength(nl->Third(args), 2)) 
        return listutils::typeError(" Could not determine tupledescr-error 1");
    if(!TupleDescr::CheckType(nl->First(nl->Third(args)))) 
        return listutils::typeError(" Could not determine tupledescr-error 2");
                
    // Check fourth argument (filterfunction)
    if (!nl->HasLength(nl->Fourth(args), 2)) 
        return listutils::typeError(" map of tupledescr values are expected");
    if(!listutils::isMap<1>(nl->First(nl->Fourth(args)))) 
        return listutils::typeError(" map of tupledescr values are expected");
   
    // Evaluate tuple type / tupledescr 
    ListExpr targetList = generateType_TM(nl->Second(nl->Third(args)));
    
    LOG << " registerForStream: TypeMapping: target type: "
        << nl->ToString(targetList) << ENDL;

    // Create filter funtion
    ListExpr filterFuncForTM = generateUserFilter_TM(nl->Fourth(args));

    LOG << " registerForStream: TypeMapping: filterFunc: "
        << nl->ToString(filterFuncForTM) << ENDL;
    
    // Build final list
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbol::APPEND()), 
               filterFuncForTM, 
               targetList);
}

/*
1.3 Operation registerForStream operator specification

*/
const std::string registerForStreamOpSpec = 
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>"
"text x int x tupledescr x fun"
" -> (stream (tuple(TUPLEDESCR)))"
"</text--->"
"<text>_ _registerForStream (_, fun)</text--->"
"<text>Receives a stream of tuple of Type tupledescr to"
" a given hostname,portnumber, TypeDescr and filterfunction</text--->"
"<text>query""localhost"" 8080 registerForStream[[const tupledescr value" 
"'((No int))'], fun(t : tuple([No : int]) ) "
"attr(t,No) > 5 ] count;</text--->"
") )";


/*
1.4 Operation registerForStream

*/

Operator registerForStream_Op(
    "registerForStream",
    registerForStreamOpSpec,
    registerForStream_VM,
    Operator::SimpleSelect,
    registerForStream_TM
);

} /* end of namespace */
