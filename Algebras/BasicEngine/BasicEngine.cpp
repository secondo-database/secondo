/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

@author
c. Behrndt

@description
see OperatorSpec

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/


#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"
#include "BasicEngine_Control.h"
#include "ConnectionPostgres.h"
#include "ConnectionMySQL.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
0 Declaring variables

dbs\_con is a pointer to a connection, for example to postgres

*/
BasicEngine_Control* be_control = nullptr;

/*
noMaster is just a default string for an error massage.

*/
string const noMaster ="\nPlease use at first an init-Operator "
       "before using this operator!\n" ;

/*
noWorker is just a default string for an error massage.

*/
string const noWorker ="\nPlease use at first an init-Worker-Operator "
       "before using this operator!\n" ;

/*
negSlots is just a default string for an error massage.

*/
string const negSlots ="\nThe number of slots have to be greater than 0."
       "The number should be a multiple of your number of workers.\n" ;

/*
1 Operators

1.1 Operator  ~be\_init~

Establishes a connection to a running postgres System.
The result of this operator is a boolean indicating the success
of the operation.

1.1.1 Type Mapping of be\_init\_worker is used

*/

/*
1.1.2 Generic database connection factory

*/
ConnectionGeneric* getAndInitDatabaseConnection(const string &dbType, 
     const string &dbUser, const string &dbPass, 
     const int dbPort, const string &dbName) {

    ConnectionGeneric* connection = nullptr;

    if(ConnectionPG::DBTYPE == dbType) {
      connection = new ConnectionPG(dbUser, dbPass, dbPort, dbName);
    } else if(ConnectionMySQL::DBTYPE == dbType) {
      connection = new ConnectionMySQL(dbUser, dbPass, dbPort, dbName);
    } else {
      BOOST_LOG_TRIVIAL(error) << "Unsupported database type: " << dbType;
      return nullptr;
    }

    if(connection == nullptr) {
      BOOST_LOG_TRIVIAL(error) << "Unable to establish database connection";
      return nullptr;
    }

    bool connectionResult = connection->createConnection();

    if(! connectionResult) {
      BOOST_LOG_TRIVIAL(error) << "Database connection check failed";
      return nullptr;
    }

    return connection;
}

/*
1.1.1 Type Mapping

This operator has no paramter

*/
ListExpr be_shutdown_tm(ListExpr args) {
string err = "No parameter (--> bool) expected";

  if(!(nl->HasLength(args,0))){
    return listutils::typeError("No arguments expected. " + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.1.2 Value Mapping

*/
int be_shutdown_vm(Word* args, Word& result, int message,
        Word& local, Supplier s) {

  result = qp->ResultStorage(s);

  try {
    if(be_control != nullptr) {

      if(be_control->isMaster()) {
        cout << "Error: Can not shutdown worker, we are in master mode." 
            << endl;
        cout << "Please use be_shutdown_cluster() instead." 
            << endl << endl;
        ((CcBool*)result.addr)->Set(true, false);
        return 0;
      }

      cout << "Shutting down basic engine worker" << endl;
      delete be_control;
      be_control = nullptr;
      
      ((CcBool*)result.addr)->Set(true, true);
      return 0;
    } else {
      cout << "Basic engine worker is not active" << endl;
      ((CcBool*)result.addr)->Set(true, false);
      return 0;
    }
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while shutting down basic engine " << e.what();
    ((CcBool*)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.1.3 Specification

*/
OperatorSpec be_shutdown_spec (
   " --> bool",
   "be_shutdown()",
   "Shutdown the connection to the basic engine worker",
   "query be_shutdown()"
);

/*
1.1.6 Definition of operator ~be\_shutdown~

*/
Operator be_shutdown (
         "be_shutdown",             // name
         be_shutdown_spec.getStr(), // specification
         be_shutdown_vm,            // value mapping
         Operator::SimpleSelect,    // trivial selection function
         be_shutdown_tm             // type mapping
);


/*
1.1.1 Type Mapping

This operator has no paramter

*/
ListExpr be_shutdown_cluster_tm(ListExpr args) {
string err = "No parameter (--> bool) expected";

  if(!(nl->HasLength(args,0))){
    return listutils::typeError("No arguments expected. " + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.1.2 Value Mapping

*/
int be_shutdown_cluster_vm(Word* args, Word& result, int message,
        Word& local, Supplier s) {
  
  result = qp->ResultStorage(s);
  try {
    if(be_control != nullptr) {

      if(! be_control->isMaster()) {
        cout << "Error: Can not shutdown worker nodes, we are not" 
            << " in master mode" << endl;
        cout << "Please use be_shutdown to shutdown the local engine."
            << endl << endl;
        ((CcBool*)result.addr)->Set(false, true);
        return 0;
      }

      cout << "Shutting down basic engine worker" << endl;
      bool shutdownResult = be_control->shutdownWorker();

      if(! shutdownResult) {
        cout << "Error: Shutdown of the workers failed" << endl 
            << endl;
      
        ((CcBool*)result.addr)->Set(true, false);
        return 0;
      }

      cout << "Shutting down basic engine master" << endl;
      delete be_control;
      be_control = nullptr;

      ((CcBool*)result.addr)->Set(true, true);
      return 0;
    } else {
      cout << "Basic engine is not active" << endl;
      ((CcBool*)result.addr)->Set(true, false);
      return 0;
    }
   } catch (SecondoException &e) {
      BOOST_LOG_TRIVIAL(error) 
        << "Got error while shutdown basic engine " << e.what();
      ((CcBool*)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.1.3 Specification

*/
OperatorSpec be_shutdown_cluster_spec (
   " --> bool",
   "be_shutdown_cluster()",
   "Shutdown the connection to the basic engine worker",
   "query be_shutdown_cluster()"
);

/*
1.1.6 Definition of operator ~be\_shutdown~

*/
Operator be_shutdown_cluster (
         "be_shutdown_cluster",                 // name
         be_shutdown_cluster_spec.getStr(),     // specification
         be_shutdown_cluster_vm,                // value mapping
         Operator::SimpleSelect,               // trivial selection function
         be_shutdown_cluster_tm                 // type mapping
);


/*

1.2 Operator  ~be\_part\_random~

Distribute a relation by random, sends the data
to the worker and import the data

1.2.2 Type Mapping

*/
ListExpr be_partRRTM(ListExpr args){
string err = "{string, text} x int -> bool"
       "(tab-name, key, number of slots) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Three arguments expected.\n " + err);
  }
  if(!CcString::checkType(nl->First(args))
        && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text.\n" + err);
  }

  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                    "to be an integer.\n" + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.2.3 Value Mapping

*/
template<class T>
int be_partRandomSFVM(Word* args, Word& result, int message,
        Word& local, Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
CcInt* slot = (CcInt*) args[1].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

try {
  if(be_control && be_control->isMaster()){
    if (slot->GetIntval() > 0){
      val = be_control->partition_table_by_random(
        tab->toText(), slot->GetIntval(), false);
    }else{
      cout << negSlots << endl;
    }
  } else{
    cout << noWorker << endl;
  }
  res->Set(true, val);
 } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while partitioning table " << e.what();
    res->Set(true, false);
 }
return 0;
}

/*
1.2.3 Specification

*/
OperatorSpec be_partRandomSpec(
   "{string, text} x int--> bool",
   "be_part_random(_,_)",
   "This operator distribute a relation by random "
   "to the worker. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_part_random('cars','moid',60)"
);

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_partRandomVM[] = {
  be_partRandomSFVM<CcString>,
  be_partRandomSFVM<FText>
};

/*
1.2.5 Selection Function

*/
int be_partRandomSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.2.6 Operator instance

*/
Operator be_partRandomOp(
  "be_part_random",
  be_partRandomSpec.getStr(),
  sizeof(be_partRandomVM),
  be_partRandomVM,
  be_partRandomSelect,
  be_partRRTM
);


/*
1.2 Operator  ~be\_partRR~

Distribute a relation by Round-Robin, sends the data
to the worker and import the data

1.2.1 Type Mapping

This operator gets a tablename

*/

/*
1.2.2 Value Mapping

*/
template<class T>
int be_partRRSFVM(Word* args, Word& result, int message,
        Word& local, Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
CcInt* slot = (CcInt*) args[1].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

try {
  if(be_control && be_control->isMaster()){
    if (slot->GetIntval() > 0){
      val = be_control->partition_table_by_rr(
        tab->toText(), slot->GetIntval(), false);
    } else {
      cout << negSlots << endl;
    }
  } else{
    cout << noWorker << endl;
  }
  res->Set(true, val);
 } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while partitioning table " << e.what();
    res->Set(true, false);
}
return 0;
}

/*
1.2.3 Specification

*/
OperatorSpec be_partRRSpec(
   "{string, text} x int--> bool",
   "be_part_rr(_,_)",
   "This operator distribute a relation by round-robin "
   "to the worker. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_part_rr('cars','moid',60)"
);

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_partRRVM[] = {
  be_partRRSFVM<CcString>,
  be_partRRSFVM<FText>
};

/*
1.2.5 Selection Function

*/
int be_partRRSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.2.6 Operator instance

*/
Operator be_partRROp(
  "be_part_rr",
  be_partRRSpec.getStr(),
  sizeof(be_partRRVM),
  be_partRRVM,
  be_partRRSelect,
  be_partRRTM
);

/*
1.3 Operator  ~be\_partHash~

Distribute a relation by Hash, sends the data
to the worker and import the data

1.3.1 Type Mapping

This operator gets a tablename and key-list (semikolon seperated)

*/
ListExpr be_partHashTM(ListExpr args){
string err = "\n {string, text} x {string, text} x int--> bool"
       "(tab-name, key, number of slots) expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError("Three arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "to be a string or a text." + err);
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "to be an integer." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.3.2 Value Mapping

*/
template<class T, class H>
int be_partHashSFVM(Word* args,Word& result,int message,
          Word& local,Supplier s ){

  result = qp->ResultStorage(s);

  T* tab = (T*) args[0].addr;
  H* key = (H*) args[1].addr;
  CcInt* slot = (CcInt*) args[2].addr;

  bool val = false;
  CcBool* res = (CcBool*) result.addr;
  try {
    if(be_control && be_control->isMaster()){
      if (slot->GetIntval() > 0){
        val = be_control->partition_table_by_hash(
          tab->toText(), key->toText(),slot->GetIntval(),
          false);
      } else {
        cout << negSlots << endl;
      }
    } else {
      cout << noWorker << endl;
    }

    res->Set(true, val);
   } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while partitioning table " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_partHashSpec(
   "{string, text} x {string, text} x int--> bool",
   "be_part_hash(_,_,_)",
   "This operator distribute a relation by hash-value "
   "to the worker. You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_part_hash('cars','moid',60)"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_partHashVM[] = {
  be_partHashSFVM<CcString,CcString>,
  be_partHashSFVM<FText,CcString>,
  be_partHashSFVM<CcString,FText>,
  be_partHashSFVM<FText,FText>
};

/*
1.3.5 Selection Function

*/
int be_partHashSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  } else {
    return CcString::checkType(nl->Second(args))?1:3;
  }
}

/*
1.3.6 Operator instance

*/
Operator be_partHashOp(
  "be_part_hash",
  be_partHashSpec.getStr(),
  sizeof(be_partHashVM),
  be_partHashVM,
  be_partHashSelect,
  be_partHashTM
);

/*
1.4 Operator  ~be\_partFun~

Distribute a relation by a valid function, sends the data
to the worker and import the data

1.4.1 Type Mapping

This operator gets a tablename and key-list (semikolon seperated)
and a function name

*/
ListExpr be_partFunTM(ListExpr args){
string err = "\n {string, text} x {string, text} x {string, text} x int--> bool"
       "(tab-name, key, function-name, number of slots) expected";

  if(!nl->HasLength(args,4)){
    return listutils::typeError("Four arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Third(args))
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "to be a string or a text." + err);
  }
  if(!CcInt::checkType(nl->Fourth(args))){
    return listutils::typeError("Value of fourth argument have "
        "to be an integer." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.4.2 Value Mapping

*/
template<class T, class H, class N>
int be_partFunSFVM(Word* args,Word& result,int message,
  Word& local,Supplier s ) {

result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
H* fun = (H*) args[2].addr;
CcInt* slot = (CcInt*) args[3].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

try {
  if(be_control && be_control -> isMaster()){
    if (slot->GetIntval() > 0){
      val = be_control->partition_table_by_fun(
        tab->toText(), key->toText(), 
        fun->toText(),slot->GetIntval(), false);
    } else {
      cout<< negSlots << endl;
    }
  } else{
    cout << noWorker << endl;
  }

  res->Set(true, val);
} catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while partitioning table " << e.what();
  res->Set(true, false);
}

return 0;
}

/*
1.4.3 Specification

*/
OperatorSpec be_partFunSpec(
   "{string, text} x {string, text} x {string, text} x int--> bool",
   "be_part_fun(_,_,_,_)",
   "This operator distribute a relation by a special function "
   "to the worker. Special functions are RR, Hash and random. "
   "You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_part_fun('cars','moid','random',60)"
);

/*
1.4.4 ValueMapping Array

*/
ValueMapping be_partFunVM[] = {
  be_partFunSFVM<CcString,CcString,CcString>,
  be_partFunSFVM<FText,CcString,CcString>,
  be_partFunSFVM<CcString,FText,CcString>,
  be_partFunSFVM<FText,FText,CcString>,
  be_partFunSFVM<CcString,CcString,FText>,
  be_partFunSFVM<FText,CcString,FText>,
  be_partFunSFVM<CcString,FText,FText>,
  be_partFunSFVM<FText,FText,FText>
};

/*
1.4.5 Selection Function

*/
int be_partFunSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    if(CcString::checkType(nl->Second(args))){
      return CcString::checkType(nl->Third(args))?0:4;
    }else{
      return CcString::checkType(nl->Third(args))?2:6;
    }
  }else{
    if(CcString::checkType(nl->Second(args))){
      return CcString::checkType(nl->Third(args))?1:5;
    }else{
      return CcString::checkType(nl->Third(args))?3:7;
    };
  }
};

/*
1.4.6 Operator instance

*/
Operator be_partFunOp(
  "be_part_fun",
  be_partFunSpec.getStr(),
  sizeof(be_partFunVM),
  be_partFunVM,
  be_partFunSelect,
  be_partFunTM
);


/*
1.5 Operator  ~be\_query~

Execute a query on the basiceEgine and stores the result in a table

1.5.1 Type Mapping

This operator gets a query and target table name
and a config-relation

*/
ListExpr be_queryTM(ListExpr args){
string err = "\n {string, text} x {string, text} --> bool"
       "(sql-query, target tab name) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Two arguments expected." + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "          to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                  "to be a string or a text." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.5.2 Value Mapping

*/
template<class T, class H>
int be_querySFVM(Word* args,Word& result,int message,Word& local,Supplier s ) {

  string query_exec;
  bool val = false;
  result = qp->ResultStorage(s);

  T* query = (T*) args[0].addr;
  H* resultTab = (H*) args[1].addr;

  try {
    if(be_control) {
      //Delete target Table, ignore failure
      be_control->drop_table(resultTab->toText());

      //execute the query
      val=be_control->createTab(resultTab->toText(),query->toText());
    } else {
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while executing query operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.5.3 Specification

*/
OperatorSpec be_queryVMSpec(
   "{string, text} x {string, text} --> bool",
   "be_query(_ , _ )",
   "Execute a sql-statement at the locale second DBMS and stores the "
   "result in the specified table. The statement must be in a correct "
   "syntax for this DBMS. ",
   "query be_query('select * from cars where Speed = 30', 'cars_neu')"
);

/*
1.5.4 ValueMapping Array

*/
ValueMapping be_queryVM[] = {
  be_querySFVM<CcString,CcString>,
  be_querySFVM<FText,CcString>,
  be_querySFVM<CcString,FText>,
  be_querySFVM<FText,FText>
};

/*
1.5.5 Selection Function

*/
int be_querySelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
}

/*
1.5.6 Operator instance

*/
Operator be_queryOp(
  "be_query",
  be_queryVMSpec.getStr(),
  sizeof(be_queryVM),
  be_queryVM,
  be_querySelect,
  be_queryTM
);

/*
1.6 Operator  ~be\_command~

Execute a command on the BasicEngine

1.6.1 Type Mapping

This operator gets a command

*/
ListExpr be_commandTM(ListExpr args){
string err = "\n {string, text} --> bool"
       "(command sql) expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected." + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.6.2 Value Mapping

*/
template<class T>
int be_commandSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
          
  bool val = false;
  result = qp->ResultStorage(s);

  T* query = (T*) args[0].addr;

  try {
    if(be_control){
      val = be_control->sendCommand(query->GetValue(),true);
    } else {
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while executing command operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.6.3 Specification

*/
OperatorSpec be_commandVMSpec(
   "{string, text} --> bool",
   "be_command(_ )",
   "Execute a sql-statement at the locale second DBMS. "
   "The statement must be in a correct syntax for this DBMS. ",
   "query be_command('COPY cars FROM /home/filetransfers/cars_3.bin BINARY')"
);

/*
1.6.4 ValueMapping Array

*/
ValueMapping be_commandVM[] = {
  be_commandSFVM<CcString>,
  be_commandSFVM<FText>
};

/*
1.6.5 Selection Function

*/
int be_commandSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
};

/*
1.6.6 Operator instance

*/
Operator be_commandOp(
  "be_command",
  be_commandVMSpec.getStr(),
  sizeof(be_commandVM),
  be_commandVM,
  be_commandSelect,
  be_commandTM
);

/*
1.7 Operator  ~be\_copy~

Copy the data from a table to a file

1.7.1 Type Mapping

This operator gets a tablename and the target filepath.

*/
ListExpr be_copyTM(ListExpr args){
string err = "\n {string, text} x {string, text} --> bool"
       "(tab-name, target filepath) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Two arguments expected." + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                  "to be a string or a text." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.7.2 Value Mapping

*/
template<class T, class H>
int be_copySFVM(Word* args, Word& result, int message,
          Word& local, Supplier s) {

  result = qp->ResultStorage(s);

  T* source = (T*) args[0].addr;
  H* destination = (H*) args[1].addr;

  if(! source->IsDefined()) {
    cerr << "Error: Source parameter is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! destination->IsDefined()) {
    cerr << "Error: Destination  parameter is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(be_control == nullptr) {
    cout << noMaster << endl;
    ((CcBool *)result.addr)->Set(true, false);
    return 0;
  }
  try {
    string sourceParameter = source->GetValue();
    string destinationParameter = destination->GetValue();

    if(boost::algorithm::starts_with(sourceParameter, "/") && 
      ! boost::algorithm::starts_with(destinationParameter, "/") ) {

      // Import (First parameter is a file, second a table)
      bool beResult = be_control->importTable( 
        destinationParameter, sourceParameter);

      ((CcBool *)result.addr)->Set(true, beResult);
      return 0;

    } else if (! boost::algorithm::starts_with(sourceParameter, "/") && 
      boost::algorithm::starts_with(destinationParameter, "/") ) {

      // Export (First parameter is a table, second a file)
      bool beResult = be_control->exportTable(
        sourceParameter, destinationParameter);

      ((CcBool *)result.addr)->Set(true, beResult);
      return 0;
    } 

    cerr << "Error: Exactly one parameter has to be a "
        << "absolute path, starting with '/'" << endl;
    
    ((CcBool *)result.addr)->Set(true, false);
   } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while executing copy operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.7.3 Specification

*/
OperatorSpec be_copyVMSpec(
   "{string, text} x {string, text} --> bool",
   "be_copy(_,_)",
   "You can use this operator to import or export a relation "
   "to a file. Be sure to have the permissions to read an write in "
   "this folder. be_copy(From,To), From/To can be a table or a path.",
   "query be_copy('cars','/home/filetransfers/cars_3.bin')"
);

/*
1.7.4 ValueMapping Array

*/
ValueMapping be_copyVM[] = {
  be_copySFVM<CcString,CcString>,
  be_copySFVM<FText,CcString>,
  be_copySFVM<CcString,FText>,
  be_copySFVM<FText,FText>
};

/*
1.7.5 Selection Function

*/
int be_copySelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
};

/*
1.7.6 Operator instance

*/
Operator be_copyOp(
  "be_copy",
  be_copyVMSpec.getStr(),
  sizeof(be_copyVM),
  be_copyVM,
  be_copySelect,
  be_copyTM
);


/*
1.8 Operator  ~be\_mquery~

Execute a query on all worker and stores the result
in a table on each worker

1.8.1 Type Mapping

This operator gets a query and target table name

*/
ListExpr be_mqueryTM(ListExpr args){
string err = "\n {string,text} x {string,text} -> bool"
       "(sql-query, target-tab) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Two arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                  "to be a string or a text." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.8.2 Value Mapping

*/
template<class T, class H>
int be_mquerySFVM(Word* args, Word& result, int message,
          Word& local, Supplier s) {

  bool val = false;
  result = qp->ResultStorage(s);
  T* query = (T*) args[0].addr;
  H* tab = (H*) args[1].addr;

  try {
    if(be_control && be_control -> isMaster()){
      val = be_control->mquery(query->toText(), tab->toText() );
    } else {
      cout << noWorker << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while executing mquery operator " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.8.3 Specification

*/
OperatorSpec be_mqueryVMSpec(
   "{string,text} x {string,text}-> bool",
   "be_mquery(_,_)",
   "Distribute a query to the worker and writes the result in "
   "the specified table. The statement must be in a correct "
   "syntax for this DBMS.",
   "query be_mquery('select * from cars','cars_short')"
);


/*
1.8.4 ValueMapping Array

*/
ValueMapping be_mqueryVM[] = {
  be_mquerySFVM<CcString,CcString>,
  be_mquerySFVM<FText,CcString>,
  be_mquerySFVM<CcString,FText>,
  be_mquerySFVM<FText,FText>
};

/*
1.8.5 Selection Function

*/
int be_mquerySelect(ListExpr args){
    if(CcString::checkType(nl->First(args))){
      return CcString::checkType(nl->Second(args))?0:2;
    }else{
      return CcString::checkType(nl->Second(args))?1:3;
    }
};

/*
1.8.6 Operator instance

*/
Operator be_mqueryOp(
  "be_mquery",
  be_mqueryVMSpec.getStr(),
  sizeof(be_mqueryVM),
  be_mqueryVM,
  be_mquerySelect,
  be_mqueryTM
);

/*
1.9 Operator  ~be\_mcommand~

Execute a command on all worker

1.9.1 Type Mapping

This operator gets a query and a config relation

*/
ListExpr be_mcommandTM(ListExpr args){
string err = "\n {string,text} -> bool"
       "(sql-command) expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.9.2 Value Mapping

*/
template<class T>
int be_mcommandSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s) {

  bool val = false;
  result = qp->ResultStorage(s);
  T* query = (T*) args[0].addr;

  try {
    if(be_control && be_control -> isMaster()){
      val = be_control->mcommand(query->toText());
    } else{
      cout << noWorker << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);

  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while execute mcommand " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.9.3 Specification

*/
OperatorSpec be_mcommandVMSpec(
   "{string,text} -> bool",
   "_ be_mcommand(_)",
   "Distribute a sql-command to the worker. The statement "
   "must be in a correct syntax for this DBMS.",
   "query be_mcommand('Drop Table cars;')"
);


/*
1.9.4 ValueMapping Array

*/
ValueMapping be_mcommandVM[] = {
  be_mcommandSFVM<CcString>,
  be_mcommandSFVM<FText>
};

/*
1.9.5 Selection Function

*/
int be_mcommandSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
};

/*
1.9.6 Operator instance

*/
Operator be_mcommandOp(
  "be_mcommand",
  be_mcommandVMSpec.getStr(),
  sizeof(be_mcommandVM),
  be_mcommandVM,
  be_mcommandSelect,
  be_mcommandTM
);

/*
1.10 Operator  ~be\_union~

Collecting a table from all worker to the master.

1.10.1 Type Mapping

This operator gets a tablename

*/
ListExpr be_unionTM(ListExpr args){
string err = "\n {string,text} -> bool"
       "(tab-name) expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.10.2 Value Mapping

*/
template<class T>
int be_unionSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s){
            
  result = qp->ResultStorage(s);
  T* tab = (T*) args[0].addr;
  bool val;

  try {
    if(be_control && be_control -> isMaster()) {
      val = be_control->munion(tab->toText());
    } else {
      cout << noWorker << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while executing union operator" << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.10.3 Specification

*/
OperatorSpec be_unionVMSpec(
   "{string,text} -> bool",
   "be_union(_ )",
   "This operator collecting one table from all workers "
   "to the master.",
   "query be_union('cars_short')"
);

/*
1.10.4 ValueMapping Array

*/
ValueMapping be_unionVM[] = {
  be_unionSFVM<CcString>,
  be_unionSFVM<FText>,
};

/*
1.10.5 Selection Function

*/
int be_unionSelect(ListExpr args){
    return CcString::checkType(nl->First(args))?0:1;
}

/*
1.10.6 Operator instance

*/
Operator be_unionOp(
  "be_union",
  be_unionVMSpec.getStr(),
  sizeof(be_unionVM),
  be_unionVM,
  be_unionSelect,
  be_unionTM
);

/*
1.11 Operator  ~be\_struct~

Exports the structure of a table as a sql-create statement
and stores that in a file.

1.11.1 Type Mapping

This operator gets a tablename

*/
ListExpr be_structTM(ListExpr args){
string err = "\n {string,text} -> bool"
       "(tab-name) expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of second argument have "
                  "to be a string or a text." + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.11.2 Value Mapping

*/
template<class T>
int be_structSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){

  bool val = false;

  result = qp->ResultStorage(s);
  T* tab = (T*) args[0].addr;

  try {

    if(be_control){
      //export a create Statement to filetransfer
      val = be_control->createSchemaSQL(tab->GetValue());
    } else{
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
   } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while create table structure " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }
  return 0;
}

/*
1.11.3 Specification

*/
OperatorSpec be_structVMSpec(
   "{string,text} -> bool",
   "be_struct(_)",
   "This operator creates a table-create-Statement for a "
   "specified table and stores it in a file. This file is "
   "located in your local home-directory in the filetransfer folder."
   "Be sure to have this directory with the correct permissions.",
   "query be_struct('cars_short')"
);

/*
1.11.4 ValueMapping Array

*/
ValueMapping be_structVM[] = {
  be_structSFVM<CcString>,
  be_structSFVM<FText>,
};

/*
1.11.5 Selection Function

*/
int be_structSelect(ListExpr args){
    return CcString::checkType(nl->First(args))?0:1;
}

/*
1.11.6 Operator instance

*/
Operator be_structOp(
  "be_struct",
  be_structVMSpec.getStr(),
  sizeof(be_structVM),
  be_structVM,
  be_structSelect,
  be_structTM
);

/*
1.12 Operator  ~init\_pgWorker~

Establishes a connection to a running postgres System.
The result of this operator is a boolean indicating the success
of the operation.

1.12.1 Type Mapping

This operator gets a hostname,a port and a Worker relation.

*/
ListExpr be_init_cluster_tm(ListExpr args) {

  string err = "\n {string, text} x {string, text} x {string, text} "
       "x int x {string, text} x rel "
       "--> bool (db-type, db-user, db-pass, port, db-name,"
       " worker relation) expected";

  // Example
  // ((text 'pgsql') (text 'user')  (text 'pass') (int 50506) (text 'mydb') 
  //  ((rel (tuple ((Host string) (Port int) (Config string) 
  //  (PGPort int) (DBName string)))) WorkersPG))

  if(!(nl->HasLength(args, 6))){
    return listutils::typeError("Six arguments expected. " + err);
  }

  ListExpr dbType = nl->First(nl->First(args));
  ListExpr dbUser = nl->First(nl->Second(args));
  ListExpr dbPass = nl->First(nl->Third(args));
  ListExpr dbPort = nl->First(nl->Fourth(args));
  ListExpr dbName = nl->First(nl->Fifth(args));
  ListExpr relation = nl->First(nl->Sixth(args));

  string relationName = nl->ToString(nl->Second(nl->Sixth(args)));

  if(!CcString::checkType(dbType) && !FText::checkType(dbType)) {
    return listutils::typeError("Value of first argument have "
        "to be a string or a text. " + err);
  }

  if(!CcString::checkType(dbUser) && !FText::checkType(dbUser)) {
    return listutils::typeError("Value of second argument have "
        "to be a string or a text. " + err);
  }

  if(!CcString::checkType(dbPass) && !FText::checkType(dbPass)) {
    return listutils::typeError("Value of third argument have "
        "to be a string or a text. " + err);
  }

  if(!CcInt::checkType(dbPort)) {
    return listutils::typeError("Value of fourth argument have "
        "to be a int." + err);
  }

  if(!CcString::checkType(dbName) && !FText::checkType(dbName)) {
    return listutils::typeError("Value of fifth argument have "
        "to be a string or a text. " + err);
  }

  if(!Relation::checkType(relation)){
    return listutils::typeError("Value of sixth argument have "
        "to be a relation." + err);
  }

  // Append the used relation name to the result
  // The relation is distributed in the VM to the worker
  ListExpr res = nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(nl->StringAtom(relationName)),
        nl->SymbolAtom(CcBool::BasicType())
  );

  return res;
}

/*
1.12.2 Value Mapping

*/
template<class T>
int init_be_workerSFVM(Word* args, Word& result, int message, 
     Word& local, Supplier s ) {

  T* dbtype = (T*) args[0].addr;
  T* dbUser = (T*) args[1].addr;
  T* dbPass = (T*) args[2].addr;
  CcInt* port = (CcInt*) args[3].addr;
  T* dbname = (T*) args[4].addr;
  Relation* worker = (Relation*) args[5].addr;
  FText* workerRelationName = (FText*) args[6].addr;

  result = qp->ResultStorage(s);

  if(be_control != nullptr) {
    cerr << "Error: Basic engine is already initialized. "
      << "Please shutdown first, using be_shutdown_cluster()."
      << endl << endl;

    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! dbtype->IsDefined()) {
    cerr << "Error: Database type is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! dbUser->IsDefined()) {
    cerr << "Error: DBUser is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! dbPass->IsDefined()) {
    cerr << "Error: DBPass is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! port->IsDefined()) {
    cerr << "Error: Port is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }
  
  if(! dbname->IsDefined()) {
    cerr << "Error: DBName is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! workerRelationName->IsDefined()) {
    cerr << "Error: Worker relation name is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  string dbUserValue = dbUser->toText();
  string dbPassValue = dbPass->toText();
  int portValue = port->GetIntval();
  string dbNameValue = dbname->toText();
  string dbTypeValue = dbtype->toText();
  string workerRelationNameValue = workerRelationName->toText();
  

  try {
    ConnectionGeneric* dbConnection = getAndInitDatabaseConnection(
        dbTypeValue, dbUserValue, dbPassValue, portValue, dbNameValue);

    if(dbConnection != nullptr) {
      be_control = new BasicEngine_Control(dbConnection, worker, 
        workerRelationNameValue, true);
        
      bool createConnectionResult = be_control -> createAllConnections();

      if(! createConnectionResult) {
        cerr << "Error: Connection error, please check the previous messages"
            << " for error messages." << endl << endl;
        ((CcBool*) result.addr)->Set(true, false);
        return 0;
      }

      bool connectionsAvailable = be_control->checkAllConnections();

      if(! connectionsAvailable) {
        cerr << "Error: Not all connections available, please check the"
            << " previous messages for error messages." << endl << endl;
        ((CcBool*) result.addr)->Set(true, false);
        return 0;
      } 

      ((CcBool*) result.addr)->Set(true, true);
      return 0;
      } 

      cerr << endl << "Error: Unable to connect to database: " 
          << dbtype->toText() << endl;
      ((CcBool*) result.addr)->Set(true, false);
    } catch (SecondoException &e) {
      BOOST_LOG_TRIVIAL(error) 
        << "Got error while init connections " << e.what();
      ((CcBool*) result.addr)->Set(true, false);
    }

    return 0;
}

/*
1.12.3 Specification

*/
OperatorSpec be_init_cluster_spec (
   "{string, text} x {string, text}  x {string, text} "
      "x int x {string, text} x rel --> bool",
   "be_init_cluster(_,_,_,_,_,_)",
   "Set the dbtype, user, pass, port and the db-name for initialization the "
   "local BE-Worker. Additional you have to specified a Workers-Relation with "
   "all connection information from the worker, including the information "
   "about the second DBMS. The structure of this relation should be "
   "[Host: string, Port: int, Config: string, PGPort: int, DBName: string]",
   "query be_init_cluster('pgsql','user','pass',5432,'gisdb',WorkersPG)"
);

/*
1.12.4 ValueMapping Array

*/
ValueMapping be_init_cluster_vm[] = {
  init_be_workerSFVM<CcString>,
  init_be_workerSFVM<FText>,
};

/*
1.12.5 Selection Function

*/
int be_init_cluster_select(ListExpr args){
    return CcString::checkType(nl->First(args))?0:1;
}

/*
1.12.6 Operator instance

*/
Operator be_init_cluster_op (
  "be_init_cluster",
  be_init_cluster_spec.getStr(),
  sizeof(be_init_cluster_vm),
  be_init_cluster_vm,
  be_init_cluster_select,
  be_init_cluster_tm
);


/*
1.1.2 Value Mapping

*/
template<class T>
int be_init_sf_vm(Word* args, Word& result, int message,
        Word& local, Supplier s) {
  
  T* dbtype = (T*) args[0].addr;
  T* dbUser = (T*) args[1].addr;
  T* dbPass = (T*) args[2].addr;
  CcInt* port = (CcInt*) args[3].addr;
  T* dbname = (T*) args[4].addr;
  Relation* worker = (Relation*) args[5].addr;
  FText* workerRelationName = (FText*) args[6].addr;

  result = qp->ResultStorage(s);

  if(be_control != nullptr) {
    cerr << "Error: Basic engine is already initialized. "
      << "Please shutdown first, using be_shutdown_cluster()."
      << endl << endl;

    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! dbtype->IsDefined()) {
    cerr << "Error: Database type is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! dbUser->IsDefined()) {
    cerr << "Error: DBUser is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! dbPass->IsDefined()) {
    cerr << "Error: DBPass is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! port->IsDefined()) {
    cerr << "Error: Port is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }
  
  if(! dbname->IsDefined()) {
    cerr << "Error: DBName is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  if(! workerRelationName->IsDefined()) {
    cerr << "Error: Worker relation name is undefined" << endl;
    ((CcBool*) result.addr)->Set(true, false);
    return 0;
  }

  string dbUserValue = dbUser->toText();
  string dbPassValue = dbPass->toText();
  int portValue = port->GetIntval();
  string dbNameValue = dbname->toText();
  string dbTypeValue = dbtype->toText();
  string workerRelationNameValue = workerRelationName->toText();

  try {
    ConnectionGeneric* dbConnection = getAndInitDatabaseConnection(
        dbTypeValue, dbUserValue, dbPassValue, portValue, dbNameValue);

    if(dbConnection != nullptr) {
      be_control = new BasicEngine_Control(dbConnection, worker, 
        workerRelationNameValue, false);

        bool connectionState = dbConnection->checkConnection();
        ((CcBool*)result.addr)->Set(true, connectionState);
        return 0;
    } 

    cerr << endl << "Error: Unable to connect to database: " 
        << dbtype->toText() << endl;
    ((CcBool*)result.addr)->Set(true, false);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while init the basic engine " << e.what();
    ((CcBool*)result.addr)->Set(true, false);
  }

  return 0;
}


/*
1.1.3 Specification

*/
OperatorSpec init_be_spec (
   "{string, text} x {string, text} x {string, text} x "
      "int x {string, text} --> bool",
   "be_init(_,_,_,_,_)",
   "Set the db-type, user, pass, port and the db-name for initialization "
   "the local BE-Worker. Your username and password have to be stored "
   "in the .pgpass file in your home location. For creating a distributed "
   "PostgreSQL-System please use the operator be_init_cluster.",
   "query be_init('pgsql','user','pass',5432,'gisdb')"
);

/*
1.1.4 ValueMapping Array

*/
ValueMapping be_init_vm[] = {
  be_init_sf_vm<CcString>,
  be_init_sf_vm<FText>,
};

/*
1.1.5 Selection Function

*/
int be_init_select(ListExpr args){
    return CcString::checkType(nl->First(args))?0:1;
};

/*
1.1.6 Operator instance

*/
Operator be_init_op(
  "be_init",
  init_be_spec.getStr(),
  sizeof(be_init_vm),
  be_init_vm,
  be_init_select,
  be_init_cluster_tm
);


/*
1.13 Operator  ~be\_runsql~

Runs a sql-Statement from a file.

1.13.1 Type Mapping

This operator gets a filepath

*/
ListExpr be_runsqlTM(ListExpr args){
string err = "\n (string, text} --> bool"
       "(filepath) expected";

  if(!(nl->HasLength(args,1))){
    return listutils::typeError("One arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
        && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of second argument have "
        "to be a string or a text. " + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.13.2 Value Mapping

*/
template<class T>
int be_runsqlSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){

  T* path = (T*) args[0].addr;
  bool val;

  try {
    result = qp->ResultStorage(s);

    if(be_control){
      val = be_control->runsql(path->toText());
    }
    else{
      cout << noMaster << endl;
    }

    ((CcBool *)result.addr)->Set(true, val);
  }  catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while executing SQL " << e.what();
    ((CcBool *)result.addr)->Set(true, false);
  }

  return 0;
}

/*
1.13.3 Specification

*/
OperatorSpec be_runsqlSpec(
   "{string, text}  --> bool",
   "be_runsql(_)",
   "Opens a specified file and reading the SQL-Statement. After the the "
   "system execute this statement on the second DBMS. The statement "
   "must be in a correct syntax for this DBMS.",
   "query be_runsql('/home/cbe/filetransfer/createroads.sql')"
);

/*
1.13.4 ValueMapping Array

*/
ValueMapping be_runsqlVM[] = {
  be_runsqlSFVM<CcString>,
  be_runsqlSFVM<FText>,
};

/*
1.13.5 Selection Function

*/
int be_runsqlSelect(ListExpr args){
    return CcString::checkType(nl->First(args))?0:1;
};

/*
1.13.6 Operator instance

*/
Operator be_runsqlOp(
  "be_runsql",
  be_runsqlSpec.getStr(),
  sizeof(be_runsqlVM),
  be_runsqlVM,
  be_runsqlSelect,
  be_runsqlTM
);

/*
1.14 Operator  ~be\_partGrid~

Distribute a relation by a Grid, sends the data
to the worker and import the data

1.14.1 Type Mapping

This operator gets a tablename, key-column, a geo\_column, (x,y)
leftbottom coordinates number of slots per row and column
 and the slot size of each square

*/
ListExpr be_partGridTM(ListExpr args){
string err = "\n {string, text} x {string, text} x {string, text} "
             "x {string, text} x int --> bool"
             "(tab-name,geo_col,primary key, grid name, number of slots)"
             " expected";

  if(!nl->HasLength(args,5)){
    return listutils::typeError("Five arguments expected. " + err);
  }

  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }

  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "to be a string or a text." + err);
  }

  if(!CcString::checkType(nl->Third(args))
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "to be a string or a text." + err);
  }

  if(!CcString::checkType(nl->Fourth(args))
      && !FText::checkType(nl->Fourth(args))){
    return listutils::typeError("Value of fourth argument have "
        "to be a string or a text." + err);
  }

  if(!CcInt::checkType(nl->Fifth(args))){
    return listutils::typeError("Value of fifth argument have "
        "to be a int." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.14.2 Value Mapping

*/
template<class T, class H, class I, class K>
int be_partGridSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){

result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
I* geo_col = (I*) args[2].addr;
K* gridname = (K*) args[3].addr;
CcInt* slot = (CcInt*) args[4].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

try {
  if(be_control && be_control -> isMaster()){
    if (slot->GetIntval() > 0){
      val = be_control->partition_table_by_grid(
        tab->toText(), key->toText(), 
        slot->GetIntval(), geo_col->toText(),
        gridname->toText(), false);
      } else{
        cout<< negSlots << endl;
      }
  } else {
      cout << noWorker << endl;
  }
  res->Set(true, val);
} catch (SecondoException &e) {
  BOOST_LOG_TRIVIAL(error) 
    << "Got error while partitioning table " << e.what();
  res->Set(true, false);
}

  return 0;
}

/*
1.14.3 Specification

*/
OperatorSpec be_partGridSpec(
   "{string, text} x {string, text} x {string, text} "
   "x {string, text} x int --> bool",
   "be_part_grid(_,_,_,_,_)",
   "This operator distribute a relation by specified grid "
   "to the worker. You can specified the name of the grid. "
   "The number of slots and size have to be "
   "positive. The column should be a spatial attribute",
   "query be_part_grid('roads','gid','geog','mygrid',20)"
);

/*
1.14.4 ValueMapping Array

*/
ValueMapping be_partGridVM[] = {
  be_partGridSFVM<CcString,CcString,CcString,CcString>,
  be_partGridSFVM<FText,CcString,CcString,CcString>,
  be_partGridSFVM<CcString,FText,CcString,CcString>,
  be_partGridSFVM<FText,FText,CcString,CcString>,
  be_partGridSFVM<CcString,CcString,FText,CcString>,
  be_partGridSFVM<FText,CcString,FText,CcString>,
  be_partGridSFVM<CcString,FText,FText,CcString>,
  be_partGridSFVM<FText,FText,FText,CcString>,
  be_partGridSFVM<CcString,CcString,CcString,FText>,
  be_partGridSFVM<FText,CcString,CcString,FText>,
  be_partGridSFVM<CcString,FText,CcString,FText>,
  be_partGridSFVM<FText,FText,CcString,FText>,
  be_partGridSFVM<CcString,CcString,FText,FText>,
  be_partGridSFVM<FText,CcString,FText,FText>,
  be_partGridSFVM<CcString,FText,FText,FText>,
  be_partGridSFVM<FText,FText,FText,FText>
};

/*
1.14.5 Selection Function

*/
int be_partGridSelect(ListExpr args){

  if(CcString::checkType(nl->Fourth(args))) {
    if(CcString::checkType(nl->First(args))){
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?0:4;
      }else{
        return CcString::checkType(nl->Third(args))?2:6;
      }
    }else{
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?1:5;
      }else{
        return CcString::checkType(nl->Third(args))?3:7;
      }
    }
  } else {
    if(CcString::checkType(nl->First(args))){
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?8:12;
      }else{
        return CcString::checkType(nl->Third(args))?10:14;
      }
    }else{
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?9:13;
      }else{
        return CcString::checkType(nl->Third(args))?12:15;
      }
    }
  }
}


/*
1.14.6 Operator instance

*/
Operator be_partGridOp(
  "be_part_grid",
  be_partGridSpec.getStr(),
  sizeof(be_partHashVM),
  be_partGridVM,
  be_partGridSelect,
  be_partGridTM
);


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

/*
1.1.3 Specification

*/
OperatorSpec be_collect_spec (
   "(text) --> stream(tuple(...))",
   "be_collect(_)",
   "Fetches the data from the used database into SECONDO",
   "query be_collect('select * from cars') collect"
);

/*
1.1.6 Definition of operator ~be\_collect~

*/
Operator be_collect_op (
         "be_collect",              // name
         be_collect_spec.getStr(),  // specification
         be_collect_vm,             // value mapping
         Operator::SimpleSelect,    // trivial selection function
         be_collect_tm             // type mapping
);




/*
1.2 Operator  ~be\_repart\_random~

Repartition a relation by random, sends the data
to the worker and import the data

1.2.2 Value Mapping

*/
template<class T>
int be_repartRandomSFVM(Word* args, Word& result, int message,
        Word& local, Supplier s ){

  result = qp->ResultStorage(s);

  T* tab = (T*) args[0].addr;
  CcInt* slot = (CcInt*) args[1].addr;

  bool val = false;
  CcBool* res = (CcBool*) result.addr;
 
  try {
    if(be_control == nullptr){
      cerr << "Please init basic engine first" << endl;
      val = false;
    } else {
      if (slot->GetIntval() > 0){
        val = be_control->partition_table_by_random(
          tab->toText(), slot->GetIntval(), true);
      } else{
        cout << negSlots << endl;
      }
    }

    res->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while repartitioning the table " << e.what();
    res->Set(true, false);
  }

  return 0;
}


/*
1.2.3 Specification

*/
OperatorSpec be_repartRandomSpec(
   "{string, text} x int--> bool",
   "be_repart_random(_,_)",
   "This operator repartition a relation by random "
   "to the worker. The number of slots have to be positive "
   "and should be a multiple of your number of workers.",
   "query be_repart_random('cars', 60)"
);

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_repartRandomVM[] = {
  be_repartRandomSFVM<CcString>,
  be_repartRandomSFVM<FText>
};

/*
1.2.5 Selection Function

*/
int be_repartRandomSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.2.6 Operator instance

*/
Operator be_repartRandomOp(
  "be_repart_random",
  be_repartRandomSpec.getStr(),
  sizeof(be_repartRandomVM),
  be_repartRandomVM,
  be_repartRandomSelect,
  be_partRRTM
);


/*
1.2 Operator  ~be\_repart\_rr~

Repartition a relation by rr, sends the data
to the worker and import the data

1.2.2 Value Mapping

*/
template<class T>
int be_repartRRSFVM(Word* args, Word& result, int message,
        Word& local, Supplier s ){

  result = qp->ResultStorage(s);

  T* tab = (T*) args[0].addr;
  CcInt* slot = (CcInt*) args[1].addr;

  bool val = false;
  CcBool* res = (CcBool*) result.addr;

  try {
    if(be_control == nullptr){
      cerr << "Please init basic engine first" << endl;
      val = false;
    } else {
      if (slot->GetIntval() > 0){
        val = be_control->partition_table_by_rr(
          tab->toText(), slot->GetIntval(), true);
      } else{
        cout << negSlots << endl;
      }
    }

    res->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while repartitioning the table" << e.what();
    res->Set(true, false);
  }

  return 0;
}


/*
1.2.3 Specification

*/
OperatorSpec be_repartRRSpec(
   "{string, text} x int--> bool",
   "be_repart_rr(_,_)",
   "This operator repartition a relation by round-robin "
   "to the worker. The number of slots have to be positive "
   "and should be a multiple of your number of workers.",
   "query be_repart_rr('cars', 60)"
);

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_repartRRVM[] = {
  be_repartRRSFVM<CcString>,
  be_repartRRSFVM<FText>
};

/*
1.2.5 Selection Function

*/
int be_repartRRSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
};

/*
1.2.6 Operator instance

*/
Operator be_repartRROp(
  "be_repart_rr",
  be_repartRRSpec.getStr(),
  sizeof(be_repartRRVM),
  be_repartRRVM,
  be_repartRRSelect,
  be_partRRTM
);

/*
1.3 Operator  ~be\_repart\_hash~

Repartition a relation by Hash, sends the data
to the worker and import the data

1.3.2 Value Mapping

*/
template<class T, class H>
int be_repartHashSFVM(Word* args,Word& result,int message,
          Word& local,Supplier s ){

  result = qp->ResultStorage(s);

  T* tab = (T*) args[0].addr;
  H* key = (H*) args[1].addr;
  CcInt* slot = (CcInt*) args[2].addr;

  bool val = false;
  CcBool* res = (CcBool*) result.addr;

  try {
    if(be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      val = false;
    } else {
      if (slot->GetIntval() > 0){
        val = be_control->partition_table_by_hash(tab->toText(), 
          key->toText(), slot->GetIntval(), true);
      } else {
        cout << negSlots << endl;
      }
    } 

    res->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while repartitioning the table " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_repartHashSpec(
   "{string, text} x {string, text} x int--> bool",
   "be_repart_hash(_,_,_)",
   "This operator repartition a relation by hash-value "
   "to the worker. You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_repart_hash('cars','moid',60)"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_repartHashVM[] = {
  be_repartHashSFVM<CcString,CcString>,
  be_repartHashSFVM<FText,CcString>,
  be_repartHashSFVM<CcString,FText>,
  be_repartHashSFVM<FText,FText>
};

/*
1.3.5 Selection Function

*/
int be_repartHashSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  } else {
    return CcString::checkType(nl->Second(args))?1:3;
  }
}

/*
1.3.6 Operator instance

*/
Operator be_repartHashOp(
  "be_repart_hash",
  be_repartHashSpec.getStr(),
  sizeof(be_repartHashVM),
  be_repartHashVM,
  be_repartHashSelect,
  be_partHashTM
);


/*
1.14.2 Value Mapping for the operator ~be\_repart\_grid~

*/
template<class T, class H, class I, class K>
int be_repartGridSFVM(Word* args,Word& result,int message,
          Word& local,Supplier s ){

  result = qp->ResultStorage(s);

  T* tab = (T*) args[0].addr;
  H* key = (H*) args[1].addr;
  I* geo_col = (I*) args[2].addr;
  K* gridname = (K*) args[3].addr;
  CcInt* slot = (CcInt*) args[4].addr;

  bool val = false;
  CcBool* res = (CcBool*) result.addr;

  try {
    if(be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      val = false;
    } else {
      if (slot->GetIntval() > 0){
        val = be_control->partition_table_by_grid(
          tab->toText(), key->toText(), 
          slot->GetIntval(), geo_col->toText(),
          gridname->toText(), true);
      } else {
        cout<< negSlots << endl;
      }
    } 

    res->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error while repartitioning the table " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.14.3 Specification

*/
OperatorSpec be_repartGridSpec(
   "{string, text} x {string, text} x {string, text} "
   "x {string, text} x int --> bool",
   "be_repart_grid(_,_,_,_,_)",
   "This operator re-distribute a relation by specified grid "
   "to the worker. You can specified the name of the grid. "
   "The number of slots and size have to be "
   "positive. The column should be a spatial attribute",
   "query be_repart_grid('roads','gid','geog','mygrid',20)"
);

/*
1.14.4 ValueMapping Array

*/
ValueMapping be_repartGridVM[] = {
  be_repartGridSFVM<CcString,CcString,CcString,CcString>,
  be_repartGridSFVM<FText,CcString,CcString,CcString>,
  be_repartGridSFVM<CcString,FText,CcString,CcString>,
  be_repartGridSFVM<FText,FText,CcString,CcString>,
  be_repartGridSFVM<CcString,CcString,FText,CcString>,
  be_repartGridSFVM<FText,CcString,FText,CcString>,
  be_repartGridSFVM<CcString,FText,FText,CcString>,
  be_repartGridSFVM<FText,FText,FText,CcString>,
  be_repartGridSFVM<CcString,CcString,CcString,FText>,
  be_repartGridSFVM<FText,CcString,CcString,FText>,
  be_repartGridSFVM<CcString,FText,CcString,FText>,
  be_repartGridSFVM<FText,FText,CcString,FText>,
  be_repartGridSFVM<CcString,CcString,FText,FText>,
  be_repartGridSFVM<FText,CcString,FText,FText>,
  be_repartGridSFVM<CcString,FText,FText,FText>,
  be_repartGridSFVM<FText,FText,FText,FText>
};

/*
1.14.5 Selection Function

*/
int be_repartGridSelect(ListExpr args){
  if(CcString::checkType(nl->Fourth(args))) {
    if(CcString::checkType(nl->First(args))){
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?0:4;
      }else{
        return CcString::checkType(nl->Third(args))?2:6;
      }
    }else{
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?1:5;
      }else{
        return CcString::checkType(nl->Third(args))?3:7;
      }
    }
  } else {
    if(CcString::checkType(nl->First(args))){
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?8:12;
      }else{
        return CcString::checkType(nl->Third(args))?10:14;
      }
    }else{
      if(CcString::checkType(nl->Second(args))){
        return CcString::checkType(nl->Third(args))?9:13;
      }else{
        return CcString::checkType(nl->Third(args))?12:15;
      }
    }
  }
}


/*
1.14.6 Operator instance

*/
Operator be_repartGridOp(
  "be_repart_grid",
  be_repartGridSpec.getStr(),
  sizeof(be_repartHashVM),
  be_repartGridVM,
  be_repartGridSelect,
  be_partGridTM
);


/*
1.4.2 Value Mapping for the operator ~be\_repart\_fun~

*/
template<class T, class H, class N>
int be_repartFunSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s){

  result = qp->ResultStorage(s);

  T* tab = (T*) args[0].addr;
  H* key = (H*) args[1].addr;
  H* fun = (H*) args[2].addr;
  CcInt* slot = (CcInt*) args[3].addr;

  bool val = false;
  CcBool* res = (CcBool*) result.addr;

  try {
    if(be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      val = false;
    } else {
      if (slot->GetIntval() > 0){
        val = be_control->partition_table_by_fun(tab->toText(), 
          key->toText(), fun->toText(), slot->GetIntval(), true);
      } else {
        cout<< negSlots << endl;
      }
    }

    res->Set(true, val);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error during the repartitioning of the table " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.4.3 Specification

*/
OperatorSpec be_repartFunSpec(
   "{string, text} x {string, text} x {string, text} x int--> bool",
   "be_repart_fun(_,_,_,_)",
   "This operator redistribute a relation by a special function "
   "to the worker. Special functions are RR, Hash and random. "
   "You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_repart_fun('cars','moid','random',60)"
);

/*
1.4.4 ValueMapping Array

*/
ValueMapping be_repartFunVM[] = {
  be_repartFunSFVM<CcString,CcString,CcString>,
  be_repartFunSFVM<FText,CcString,CcString>,
  be_repartFunSFVM<CcString,FText,CcString>,
  be_repartFunSFVM<FText,FText,CcString>,
  be_repartFunSFVM<CcString,CcString,FText>,
  be_repartFunSFVM<FText,CcString,FText>,
  be_repartFunSFVM<CcString,FText,FText>,
  be_repartFunSFVM<FText,FText,FText>
};

/*
1.4.5 Selection Function

*/
int be_repartFunSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    if(CcString::checkType(nl->Second(args))){
      return CcString::checkType(nl->Third(args))?0:4;
    }else{
      return CcString::checkType(nl->Third(args))?2:6;
    }
  }else{
    if(CcString::checkType(nl->Second(args))){
      return CcString::checkType(nl->Third(args))?1:5;
    }else{
      return CcString::checkType(nl->Third(args))?3:7;
    };
  }
};

/*
1.4.6 Operator instance

*/
Operator be_repartFunOp(
  "be_repart_fun",
  be_repartFunSpec.getStr(),
  sizeof(be_repartFunVM),
  be_repartFunVM,
  be_repartFunSelect,
  be_partFunTM
);


/*
1.3 Operator  ~be\_share~

Share a relation with all worker

1.3.2 Type Mapping

*/

ListExpr be_shareTM(ListExpr args){
  string err = "\n {string, text} expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected. " + err);
  }

  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }
  
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
1.3.3 Value Mapping

*/
template<class T>
int be_shareSFVM(Word* args,Word& result,int message,
          Word& local,Supplier s ){

  result = qp->ResultStorage(s);
  bool shareResult = false;

  T* table = (T*) args[0].addr;
  CcBool* res = (CcBool*) result.addr;

  try {
    if(! table->IsDefined()) {
      cerr << "Table parameter has to be defined" << endl;
      shareResult = false;
    } else if (be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      shareResult = false;
    } else {
      string tableName = table -> toText();
      shareResult = be_control->shareTable(tableName);
    } 

    res->Set(true, shareResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error during the sharing of the table " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_shareSpec(
   "{string, text} --> bool",
   "be_share(_)",
   "This operator shares the given relation with all workers.",
   "query be_share('cars')"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_shareVM[] = {
  be_shareSFVM<CcString>,
  be_shareSFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_shareSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

/*
1.3.6 Operator instance

*/
Operator be_shareOp(
  "be_share",
  be_shareSpec.getStr(),
  sizeof(be_shareVM),
  be_shareVM,
  be_shareSelect,
  be_shareTM
);



/*
1.3 Operator  ~be\_validate\_query~

Validate the syntqx of the given query 

1.3.2 Type Mapping

*/

ListExpr be_validateQueryTM(ListExpr args){
  string err = "\n {string, text} expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected. " + err);
  }

  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }
  
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
1.3.3 Value Mapping

*/
template<class T>
int be_validateQuerySFVM(Word* args,Word& result,int message,
          Word& local,Supplier s ){

  result = qp->ResultStorage(s);
  bool validationResult = false;

  T* query = (T*) args[0].addr;
  CcBool* res = (CcBool*) result.addr;

  try {
    if(! query->IsDefined()) {
      cerr << "Query parameter has to be defined" << endl;
      validationResult = false;
    } else if (be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      validationResult = false;
    } else {
      string queryString = query -> toText();
      validationResult = be_control->validateQuery(queryString);
    } 

    res->Set(true, validationResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error during the query validation " << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_validateQuerySpec(
   "{string, text} --> bool",
   "be_validate_query(_)",
   "This operator validates the given SQL query.",
   "query be_validate_query('SELECT * FROM users')"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_validateQueryVM[] = {
  be_validateQuerySFVM<CcString>,
  be_validateQuerySFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_validateQuerySelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_validateQueryOp(
  "be_validate_query",
  be_validateQuerySpec.getStr(),
  sizeof(be_validateQueryVM),
  be_validateQueryVM,
  be_validateQuerySelect,
  be_validateQueryTM
);


/*
1.3 Operator  ~be\_grid\_create~

Create a new grid with the given name and 
specification

1.3.2 Type Mapping

*/

ListExpr be_gridCreateTM(ListExpr args){
string err = "\n {string, text} x real x real x real x int x int --> bool"
             "(grid name, x-value, y-value, slot size, number of slots)"
             " expected";

  if(!nl->HasLength(args,6)){
    return listutils::typeError("Six arguments expected. " + err);
  }

  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }

  if(!CcReal::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "to be an real." + err);
  }

  if(!CcReal::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "to be an real." + err);
  }

  if(!CcReal::checkType(nl->Fourth(args))){
    return listutils::typeError("Value of fourth argument have "
        "to be an real." + err);
  }

  if(!CcInt::checkType(nl->Fifth(args))){
    return listutils::typeError("Value of fifth argument have "
        "to be an integer." + err);
  }
  
  if(!CcInt::checkType(nl->Sixth(args))){
    return listutils::typeError("Value of sixth argument have "
        "to be an integer." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}


/*
1.3.3 Value Mapping

*/
template<class T>
int be_GridCreateSFVM(Word* args,Word& result,int message,
          Word& local,Supplier s) {

  result = qp->ResultStorage(s);
  bool operationResult = false;

  T* gridName = (T*) args[0].addr;
  CcReal* startX = (CcReal*) args[1].addr;
  CcReal* startY = (CcReal*) args[2].addr;
  CcReal* cellSize = (CcReal*) args[3].addr;
  CcInt* cellsX = (CcInt*) args[4].addr;
  CcInt* cellsY = (CcInt*) args[5].addr;
  
  CcBool* res = (CcBool*) result.addr;

  try {
    if(! gridName->IsDefined()) {
      cerr << "GridName parameter has to be defined" << endl;
      operationResult = false;
    } else if (be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      operationResult = false;
    } else if(! be_control -> isMaster()) {
      cout << noWorker << endl;
      operationResult = false;
    } else {
        string gridNameString = gridName -> toText();
        double startXDouble = startX -> GetValue();
        double startYDouble = startY -> GetValue();
        double cellSizeDouble = cellSize -> GetValue();
        int cellsXInt = cellsX -> GetValue();
        int cellsYInt = cellsY -> GetValue();
        
        operationResult = be_control -> createGrid(gridNameString, 
          startXDouble, startYDouble, cellSizeDouble, cellsXInt, 
          cellsYInt);
    } 

    res->Set(true, operationResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error during the creation of the grid" << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_gridCreateSpec(
   "{string, text} x real x real x real x int x int--> bool",
   "be_repart_grid(_)",
   "This operator creates a new grid with the given name and specification."
   "(1) Name of the grid, (2) start x, (3) start y, (4) cell size x/y, "
   "(5) cells x, (6) cells y",
   "query be_repart_grid('mygrid', 5.8, 50.3, 0.2, 20, 20)"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_gridCreateVM[] = {
  be_GridCreateSFVM<CcString>,
  be_GridCreateSFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_gridCreateSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_gridCreateOp(
  "be_grid_create",
  be_gridCreateSpec.getStr(),
  sizeof(be_gridCreateVM),
  be_gridCreateVM,
  be_gridCreateSelect,
  be_gridCreateTM
);




/*
1.3 Operator  ~be\_grid\_delete~

Delete the given grid

1.3.2 Type Mapping

*/

ListExpr be_gridDeleteTM(ListExpr args){
  string err = "\n {string, text} expected";

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected. " + err);
  }

  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a string or a text." + err);
  }
  
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
1.3.3 Value Mapping

*/
template<class T>
int be_GridDeleteSFVM(Word* args,Word& result,int message,
          Word& local,Supplier s ){

  result = qp->ResultStorage(s);
  bool operationResult = false;

  T* gridName = (T*) args[0].addr;
  CcBool* res = (CcBool*) result.addr;

 try {
    if(! gridName->IsDefined()) {
      cerr << "Grid name has to be defined" << endl;
      operationResult = false;
    } else if (be_control == nullptr) {
      cerr << "Please init basic engine first" << endl;
      operationResult = false;
    } else if(! be_control -> isMaster()) {
      cout << noWorker << endl;
      operationResult = false;
    } else {
      string gridNameString = gridName -> toText();
      operationResult = be_control -> deleteGrid(gridNameString);
    } 

    res->Set(true, operationResult);
  } catch (SecondoException &e) {
    BOOST_LOG_TRIVIAL(error) 
      << "Got error during the creation of the grid" << e.what();
    res->Set(true, false);
  }

  return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_gridDeleteSpec(
   "{string, text} --> bool",
   "be_grid_delete(_)",
   "This operator deletes the grid with the given name.",
   "query be_delete_grid('mygrid')"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_gridDeleteVM[] = {
  be_GridDeleteSFVM<CcString>,
  be_GridDeleteSFVM<FText>
};

/*
1.3.5 Selection Function

*/
int be_gridDeleteSelect(ListExpr args){
  return CcString::checkType(nl->First(args)) ? 0 : 1;
}

/*
1.3.6 Operator instance

*/
Operator be_gridDeleteOp(
  "be_grid_delete",
  be_gridDeleteSpec.getStr(),
  sizeof(be_gridDeleteVM),
  be_gridDeleteVM,
  be_gridDeleteSelect,
  be_gridDeleteTM
);




/*
1.15 Implementation of the Algebra

*/
class BasicEngineAlgebra : public Algebra
{
 public:
  BasicEngineAlgebra() : Algebra()
  {
    AddOperator(&be_init_op);
    be_init_op.SetUsesArgsInTypeMapping();
    AddOperator(&be_init_cluster_op);
    be_init_cluster_op.SetUsesArgsInTypeMapping();
    AddOperator(&be_shutdown);
    AddOperator(&be_shutdown_cluster);
    AddOperator(&be_queryOp);
    AddOperator(&be_commandOp);
    AddOperator(&be_copyOp);
    AddOperator(&be_mqueryOp);
    AddOperator(&be_mcommandOp);
    AddOperator(&be_unionOp);
    AddOperator(&be_structOp);
    AddOperator(&be_runsqlOp);
    AddOperator(&be_collect_op);
    be_collect_op.SetUsesArgsInTypeMapping();

    AddOperator(&be_partRandomOp);
    AddOperator(&be_partRROp);
    AddOperator(&be_partHashOp);
    AddOperator(&be_partGridOp);
    AddOperator(&be_partFunOp);
    AddOperator(&be_repartRandomOp);
    AddOperator(&be_repartRROp);
    AddOperator(&be_repartHashOp);
    AddOperator(&be_repartGridOp);
    AddOperator(&be_repartFunOp);

    AddOperator(&be_shareOp);
    AddOperator(&be_validateQueryOp);
    AddOperator(&be_gridCreateOp);
    AddOperator(&be_gridDeleteOp);

    // configure boost logger
    // TODO: Move to SECONDO core
    boost::log::core::get()->set_filter
    (
         boost::log::trivial::severity >= boost::log::trivial::debug
//         boost::log::trivial::severity >= boost::log::trivial::info
    );
  }

  ~BasicEngineAlgebra() {

    if(be_control != nullptr) {

      if(be_control->isMaster()) {
        be_control->shutdownWorker();
      }

      delete be_control;
      be_control = nullptr;
    }
  };
};

} // end of namespace BasicEngine

/*
1.16 Initialization

*/
extern "C"
Algebra*
InitializeBasicEngineAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  return (new BasicEngine::BasicEngineAlgebra);
}
