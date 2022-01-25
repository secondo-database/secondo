/*
----
This file is part of SECONDO.

Copyright (C) 2022,
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

//[$][\$]

*/

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include "Algebras/Distributed2/DArray.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"
#include "../BasicEngine.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {






/*
1.1.1 Type Mapping

This operator has no paramter

*/
bool EvaluateTypeMappingExpr(string expression, string &result) {
  
  Word res; 
  
  if(! QueryProcessor::ExecuteQuery(expression,res) ){
     result = "Could not evaluate expression";
     return false;
  }
  
  FText* fn = (FText*) res.addr;
  
  if(!fn->IsDefined()){
     fn->DeleteIfAllowed();
     result = "result of expression is undefined";
     return false;
  }
  
  result = fn->GetValue();
  fn->DeleteIfAllowed();
  fn = 0; 
  res.setAddr(0);
  
  return true;
}

ListExpr be_collect_tm(ListExpr args) {
  string err = "Expected text as parameter";

  if(!(nl->HasLength(args,1))){
    return listutils::typeError(err);
  }

  // arg evaluation is active
  // this means each argument is a two elem list (type value)
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
    if(!nl->HasLength(nl->First(tmp),2)){
       return listutils::typeError("expected (type value)");
    }    
    tmp = nl->Rest(tmp);
  }

  ListExpr query = nl->First(args);
  if(!FText::checkType(nl->First(query)) ) {
    return listutils::typeError(err);
  }

  // Evaluate query expression
  string queryValue;  
  string queryExpression = nl->ToString(nl->Second(query));
  if(! EvaluateTypeMappingExpr(queryExpression, queryValue) ) {
    return listutils::typeError(queryValue);
  }

  if(be_control == nullptr) {
    return listutils::typeError("Basic engine is not connected. "
      "Plase call be_init_cluster() first.");
  }

  ListExpr resultType = be_control -> getTypeFromSQLQuery(queryValue);
        
  if(nl->IsEmpty(resultType)) {
     return listutils::typeError("Unable to evaluate"
       " the given SQL query.");
  }

  //cout << "Result: " << nl->ToString(resultType) << endl;

  return resultType;
}

     
/*
1.1.2 Value Mapping

*/
int be_collect_vm(Word* args, Word& result, int message,
        Word& local, Supplier s) {

  ResultIteratorGeneric* cli = (ResultIteratorGeneric*) local.addr;
  string sqlQuery = ((FText*) args[0].addr)->GetValue();

  qp->GetGlobalMemory();

  try {
    switch(message) {
      case OPEN: 

        if (cli != nullptr) {
          delete cli;
          cli = nullptr;
        }

        cli = be_control -> performSQLSelectQuery(sqlQuery);
        local.setAddr( cli );
        return 0;

      case REQUEST:
        
        // Operator not ready
        if ( ! cli ) {
          return CANCEL;
        }
        
        // Fetch next tuple from database
        if(cli->hasNextTuple()) {
          result.addr = cli -> getNextTuple();
          return YIELD;
        } else {
          return CANCEL;
        }

      case CLOSE:
        if(cli) {
          delete cli;
          cli = nullptr;
          local.setAddr( cli );
        }

        return 0;
      }
   } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got exception during processing " << e.what();
    return CANCEL;
  }

  return 0;    
}

} // namespace BasicEngine