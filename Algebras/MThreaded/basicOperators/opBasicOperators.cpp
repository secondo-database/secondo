/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Implementation of the Basic Operators

These Operators are:

  * maxcore

  * setcore

  * getcore

*/
#include <thread>
#include "opBasicOperators.h"
//#include "../MThreadedAux.h"

#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Stream.h"  // wrapper for secondo streams
#include "LogMsg.h"             // send error messages


using namespace mthreaded;

extern NestedList *nl;
extern QueryProcessor *qp;

/*
1.1 The MThreaded maxcore Operator

*/
ListExpr op_maxcore::maxcoreTM(ListExpr args) {

    // maxcore has no argument
    if (!nl-> IsEmpty(args)) {
        return listutils::typeError(" no argument expected");
    }

    return listutils::basicSymbol<CcInt>();
}

int op_maxcore::maxcoreVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    result = qp->ResultStorage(s);
    CcInt* res = (CcInt*) result.addr;
    res -> Set(true, std::thread::hardware_concurrency() );
    return 0;
}

std::string op_maxcore::getOperatorSpec(){
   return OperatorSpec(
           " -> int",
           " maxcore()",
           " Get maximum number of threads",
           " query maxcore()"
   ).getStr();
}

std::shared_ptr<Operator> op_maxcore::getOperator(){
    return std::make_shared<Operator>("maxcore",
                                    getOperatorSpec(),
                                    &op_maxcore::maxcoreVM,
                                    Operator::SimpleSelect,
                                    &op_maxcore::maxcoreTM);
}

/*
1.2 The MThreaded setcore Operator

*/
ListExpr op_setcore::setcoreTM(ListExpr args) {

    const std::string err("int expected");

    if (!nl->HasLength(args,1)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }

    if (CcInt::checkType(nl->First(args))) {
       return nl->SymbolAtom(CcBool::BasicType());
    }

   return listutils::typeError(err);
}


int op_setcore::setcoreVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    CcInt* cores = static_cast<CcInt*>(args[0].addr);
    result = qp->ResultStorage(s);
    CcBool* res = static_cast<CcBool*>(result.addr);
    MThreadedSingleton::setCoresToUse(cores->GetValue());
    res -> Set(true,true);
    
    return 0;
}

std::string op_setcore::getOperatorSpec(){
    return OperatorSpec(
            " CcInt -> ",
            " setcore( _ ) ",
            " set number of threads to use ",
            " query setcore(CcInt)"
    ).getStr();
}

std::shared_ptr<Operator> op_setcore::getOperator(){
    return std::make_shared<Operator>("setcore",
                                    getOperatorSpec(),
                                    &op_setcore::setcoreVM,
                                    Operator::SimpleSelect,
                                    &op_setcore::setcoreTM);
}

/*
1.3 The MThreaded getcore Operator

*/
ListExpr op_getcore::getcoreTM(ListExpr args) {

   // getcore has no argument
   if (!nl-> IsEmpty(args)) {
      return listutils::typeError(" no argument expected");
   }

   return listutils::basicSymbol<CcInt>();
}


int op_getcore::getcoreVM(Word* args, Word& result, int message,
                          Word& local, Supplier s) {
   result = qp->ResultStorage(s);
   CcInt* res = static_cast<CcInt*>(result.addr);
   res -> Set(true, MThreadedSingleton::getCoresToUse());
   return 0;
}

std::string op_getcore::getOperatorSpec(){
   return OperatorSpec(
           " -> CcInt ",
           " getcore() ",
           " get number of threads to use ",
           " query getcore()"
   ).getStr();
}

std::shared_ptr<Operator> op_getcore::getOperator(){
   return std::make_shared<Operator>("getcore",
                                     getOperatorSpec(),
                                     &op_getcore::getcoreVM,
                                     Operator::SimpleSelect,
                                     &op_getcore::getcoreTM);
}
