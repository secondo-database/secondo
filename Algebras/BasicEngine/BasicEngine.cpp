/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"
#include "BasicEngine_Control.cpp"
#include "ConnectionPG.cpp"

using namespace distributed2;

namespace BasicEngine {

/*
0 Declaring variables

dbs\_con is a pointer to a connection, for example to postgres

*/
template<class L>
BasicEngine_Control<L>* dbs_conn;

/*
dbms\_name is a name of the secound dbms, for example to pg (for postgreSQL),mysql...

*/
string dbms_name;

/*
isMaster is a variable which shows, if this system is a master (true)
or a worker(false).

*/
bool isMaster = false;

/*
pg is a short name of postgres.

*/
string const pg = "pg";

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

1.1 Operator  ~init\_pg~

Establishes a connection to a running postgres System.
The result of this operator is a boolean indicating the success
of the operation.

1.1.1 Type Mapping

This operator gets a hostname and a port.

*/
ListExpr init_pgTM(ListExpr args){
string err = "int x {string, text} --> bool"
       "(port, db-name) expected";

  if(!(nl->HasLength(args,2))){
    return listutils::typeError("Two arguments expected. " + err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a int." + err);
  }
  if(!CcString::checkType(nl->Second(args))
        && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "to be a string or a text. " + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.1.2 Value Mapping

*/
template<class T, class L>
int init_pgSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
CcInt* port = (CcInt*) args[0].addr;
T* dbname = (T*) args[1].addr;
bool val;

  result = qp->ResultStorage(s);

  dbs_conn<L> = new BasicEngine_Control<L>(new ConnectionPG(port->GetIntval()
                              ,dbname->toText()));
  val = dbs_conn<L>->checkConn();
  dbms_name = (val) ? pg : "";
  isMaster = false;
  ((CcBool*)result.addr)->Set(true, val);

return 0;
}

/*
1.1.3 Specification

*/
OperatorSpec init_pgSpec(
   "int x {string, text} --> bool",
   "init_pg(_,_)",
   "Set the port and the db-name from PostgreSQL for initialization "
   "the local PG-Worker. Your username and password have to be stored "
   "in the .pgpass file in your home location. For creating a distributed "
   "PostgreSQL-System please use the operator init_pgWorker.",
   "query init_pg(5432,'gisdb')"
);

/*
1.1.4 ValueMapping Array

*/
ValueMapping init_pgVM[] = {
  init_pgSFVM<CcString,ConnectionPG>,
  init_pgSFVM<FText,ConnectionPG>,
};

/*
1.1.5 Selection Function

*/
int init_pgSelect(ListExpr args){
    return CcString::checkType(nl->First(args))?0:1;
};

/*
1.1.6 Operator instance

*/
Operator init_pgOp(
  "init_pg",
  init_pgSpec.getStr(),
  sizeof(init_pgVM),
  init_pgVM,
  init_pgSelect,
  init_pgTM
);

/*
1.2 Operator  ~be\_partRR~

Distribute a relation by Round-Robin, sends the data
to the worker and import the data

1.2.1 Type Mapping

This operator gets a tablename and key-list (semikolon seperated)

*/
ListExpr be_partRRTM(ListExpr args){
string err = "{string, text} x {string, text} x int -> bool"
       "(tab-name, key, number of slots) expected";

  if(!nl->HasLength(args,3)){
    return listutils::typeError("Three arguments expected.\n " + err);
  }
  if(!CcString::checkType(nl->First(args))
        && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text.\n" + err);
  }
  if(!CcString::checkType(nl->Second(args))
        && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                   "to be a string or a text.\n" + err);
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
                    "to be an integer.\n" + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.2.2 Value Mapping

*/
template<class T, class H, class L>
int be_partRRSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
CcInt* slot = (CcInt*) args[2].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

  if(dbs_conn<L> && isMaster){
    if (slot->GetIntval() > 0){
      val = dbs_conn<L>->partTable(tab->toText(), key->toText()
            ,"RR", slot->GetIntval());
    }else{
      cout << negSlots << endl;
    }
  }
  else{
    cout << noWorker << endl;
  }
  res->Set(true, val);

return 0;
}

/*
1.2.3 Specification

*/
OperatorSpec be_partRRSpec(
   "{string, text} x {string, text} x int--> bool",
   "be_partRR(_,_,_)",
   "This operator distribute a relation by round-robin "
   "to the worker. You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_partRR('cars','moid',60)"
);

/*
1.2.4 ValueMapping Array

*/
ValueMapping be_partRRVM[] = {
  be_partRRSFVM<CcString,CcString,ConnectionPG>,
  be_partRRSFVM<FText,CcString,ConnectionPG>,
  be_partRRSFVM<CcString,FText,ConnectionPG>,
  be_partRRSFVM<FText,FText,ConnectionPG>
};

/*
1.2.5 Selection Function

*/
int be_partRRSelect(ListExpr args){
  if (dbms_name == pg){
    if(CcString::checkType(nl->First(args))){
      return CcString::checkType(nl->Second(args))?0:2;
    }else{
      return CcString::checkType(nl->Second(args))?1:3;
    }
  }else{
    return 0;
  }
};

/*
1.2.6 Operator instance

*/
Operator be_partRROp(
  "be_partRR",
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
template<class T, class H, class L>
int be_partHashSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
CcInt* slot = (CcInt*) args[2].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

  if(dbs_conn<L> && isMaster){
    if (slot->GetIntval() > 0){
      val = dbs_conn<L>->partTable(tab->toText(), key->toText()
                ,"Hash",slot->GetIntval());
    }else{
      cout<< negSlots << endl;
    }
  }
  else{
    cout << noWorker << endl;
  }
  res->Set(true, val);

return 0;
}

/*
1.3.3 Specification

*/
OperatorSpec be_partHashSpec(
   "{string, text} x {string, text} x int--> bool",
   "be_partHash(_,_,_)",
   "This operator distribute a relation by hash-value "
   "to the worker. You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_partHash('cars','moid',60)"
);

/*
1.3.4 ValueMapping Array

*/
ValueMapping be_partHashVM[] = {
  be_partHashSFVM<CcString,CcString,ConnectionPG>,
  be_partHashSFVM<FText,CcString,ConnectionPG>,
  be_partHashSFVM<CcString,FText,ConnectionPG>,
  be_partHashSFVM<FText,FText,ConnectionPG>
};

/*
1.3.5 Selection Function

*/
int be_partHashSelect(ListExpr args){
  if (dbms_name == pg){
    if(CcString::checkType(nl->First(args))){
      return CcString::checkType(nl->Second(args))?0:2;
    }else{
      return CcString::checkType(nl->Second(args))?1:3;
    }
  }else{
    return 0;
  }
};


/*
1.3.6 Operator instance

*/
Operator be_partHashOp(
  "be_partHash",
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
template<class T, class H, class N, class L>
int be_partFunSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
H* fun = (H*) args[2].addr;
CcInt* slot = (CcInt*) args[3].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

  if(dbs_conn<L> && isMaster){
    if (slot->GetIntval() > 0){
      val = dbs_conn<L>->partTable(tab->toText(), key->toText()
                ,fun->toText(),slot->GetIntval());
    }else{
      cout<< negSlots << endl;
    }
  }
  else{
    cout << noWorker << endl;
  }

  res->Set(true, val);

return 0;
}

/*
1.4.3 Specification

*/
OperatorSpec be_partFunSpec(
   "{string, text} x {string, text} x {string, text} x int--> bool",
   "be_partFun(_,_,_,_)",
   "This operator distribute a relation by a special function "
   "to the worker. Special functions are RR, Hash and random. "
   "You can specified a multi key by separating "
   "the fields with a comma. The number of slots have to be positiv "
   "and should be a multiple of your number of workers.",
   "query be_partFun('cars','moid','random',60)"
);

/*
1.4.4 ValueMapping Array

*/
ValueMapping be_partFunVM[] = {
  be_partFunSFVM<CcString,CcString,CcString,ConnectionPG>,
  be_partFunSFVM<FText,CcString,CcString,ConnectionPG>,
  be_partFunSFVM<CcString,FText,CcString,ConnectionPG>,
  be_partFunSFVM<FText,FText,CcString,ConnectionPG>,
  be_partFunSFVM<CcString,CcString,FText,ConnectionPG>,
  be_partFunSFVM<FText,CcString,FText,ConnectionPG>,
  be_partFunSFVM<CcString,FText,FText,ConnectionPG>,
  be_partFunSFVM<FText,FText,FText,ConnectionPG>
};

/*
1.4.5 Selection Function

*/
int be_partFunSelect(ListExpr args){
if (dbms_name == pg){
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
}else{
  return 0;
}
};

/*
1.4.6 Operator instance

*/
Operator be_partFunOp(
  "be_partFun",
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
template<class T, class H, class L>
int be_querySFVM(Word* args,Word& result,int message,Word& local,Supplier s ){
string query_exec;
bool val = false;
result = qp->ResultStorage(s);

T* query = (T*) args[0].addr;
H* resultTab = (H*) args[1].addr;

  if(dbs_conn<L>){
    //Delete target Table, ignore failure
    dbs_conn<L>->drop_table(resultTab->toText());

    //execute the query
    val=dbs_conn<L>->createTab(resultTab->toText(),query->toText());
  }
  else{
    cout << noMaster << endl;
  }

  ((CcBool *)result.addr)->Set(true, val);

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
  be_querySFVM<CcString,CcString,ConnectionPG>,
  be_querySFVM<FText,CcString,ConnectionPG>,
  be_querySFVM<CcString,FText,ConnectionPG>,
  be_querySFVM<FText,FText,ConnectionPG>
};

/*
1.5.5 Selection Function

*/
int be_querySelect(ListExpr args){
if (dbms_name == pg){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
}else{
  return 0;
}
};

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
template<class T, class L>
int be_commandSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
bool val = false;
result = qp->ResultStorage(s);

T* query = (T*) args[0].addr;

  if(dbs_conn<L>){
    val = dbs_conn<L>->sendCommand(query->GetValue(),true);
  }
  else{
    cout << noMaster << endl;
  }

  ((CcBool *)result.addr)->Set(true, val);

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
  be_commandSFVM<CcString,ConnectionPG>,
  be_commandSFVM<FText,ConnectionPG>
};

/*
1.6.5 Selection Function

*/
int be_commandSelect(ListExpr args){
if (dbms_name == pg){
  return CcString::checkType(nl->First(args))?0:1;
}else{
  return 0;
}
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
template<class T, class H, class L>
int be_copySFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
bool val = false;
result = qp->ResultStorage(s);

T* from = (T*) args[0].addr;  //table
H* to = (H*) args[1].addr;    //path

  if(dbs_conn<L>){
    val = dbs_conn<L>->copy(from->GetValue(),to->GetValue(),
          (from->GetValue().length()>= to->GetValue().length()));
  }
  else{
    cout << noMaster << endl;
  }

  ((CcBool *)result.addr)->Set(true, val);

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
  be_copySFVM<CcString,CcString,ConnectionPG>,
  be_copySFVM<FText,CcString,ConnectionPG>,
  be_copySFVM<CcString,FText,ConnectionPG>,
  be_copySFVM<FText,FText,ConnectionPG>
};

/*
1.7.5 Selection Function

*/
int be_copySelect(ListExpr args){
if (dbms_name == pg){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
}else{
  return 0;
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
template<class T, class H, class L>
int be_mquerySFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
bool val = false;
result = qp->ResultStorage(s);
T* query = (T*) args[0].addr;
H* tab = (H*) args[1].addr;

  if(dbs_conn<L> && isMaster){
    val = dbs_conn<L>->mquery(query->toText(), tab->toText() );
  }
  else{
    cout << noWorker << endl;
  }

  ((CcBool *)result.addr)->Set(true, val);

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
  be_mquerySFVM<CcString,CcString,ConnectionPG>,
  be_mquerySFVM<FText,CcString,ConnectionPG>,
  be_mquerySFVM<CcString,FText,ConnectionPG>,
  be_mquerySFVM<FText,FText,ConnectionPG>
};

/*
1.8.5 Selection Function

*/
int be_mquerySelect(ListExpr args){
if (dbms_name == pg){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
}else{
  return 0;
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
template<class T, class L>
int be_mcommandSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
bool val = false;
result = qp->ResultStorage(s);
T* query = (T*) args[0].addr;

  if(dbs_conn<L> && isMaster){
    val = dbs_conn<L>->mcommand(query->toText());
  }
  else{
    cout << noWorker << endl;
  }

  ((CcBool *)result.addr)->Set(true, val);

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
  be_mcommandSFVM<CcString,ConnectionPG>,
  be_mcommandSFVM<FText,ConnectionPG>
};

/*
1.9.5 Selection Function

*/
int be_mcommandSelect(ListExpr args){
if (dbms_name == pg){
  return CcString::checkType(nl->First(args))?0:1;
}else{
  return 0;
}
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
template<class T, class L>
int be_unionSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
result = qp->ResultStorage(s);
T* tab = (T*) args[0].addr;
bool val;

  if(dbs_conn<L> && isMaster){
    val = dbs_conn<L>->munion(tab->toText());
  }
  else{
    cout << noWorker << endl;
  }

((CcBool *)result.addr)->Set(true, val);

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
  be_unionSFVM<CcString,ConnectionPG>,
  be_unionSFVM<FText,ConnectionPG>,
};

/*
1.10.5 Selection Function

*/
int be_unionSelect(ListExpr args){
if (dbms_name == pg){
  return CcString::checkType(nl->First(args))?0:1;
}else{
  return 0;
}
};

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
template<class T, class L>
int be_structSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
bool val = false;

result = qp->ResultStorage(s);
T* tab = (T*) args[0].addr;

  if(dbs_conn<L>){
    //export a create Statement to filetransfer
    val = dbs_conn<L>->createTabFile(tab->GetValue());
  }
  else{
    cout << noMaster << endl;
  }

  ((CcBool *)result.addr)->Set(true, val);

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
  be_structSFVM<CcString,ConnectionPG>,
  be_structSFVM<FText,ConnectionPG>,
};

/*
1.11.5 Selection Function

*/
int be_structSelect(ListExpr args){
if (dbms_name == pg){
  return CcString::checkType(nl->First(args))?0:1;
}else{
  return 0;
}
};

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
ListExpr init_pgWorkerTM(ListExpr args){
string err = "\n int x {string, text} x rel --> bool"
       "(port, db-name, worker relation) expected";

  if(!(nl->HasLength(args,3))){
    return listutils::typeError("Three arguments expected. " + err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "to be a int." + err);
  }
  if(!CcString::checkType(nl->Second(args))
        && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "to be a string or a text. " + err);
  }
  if(!Relation::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "to be a relation." + err);
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.12.2 Value Mapping

*/
template<class T, class L>
int init_pgWorkerSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
CcInt* port = (CcInt*) args[0].addr;
T* dbname = (T*) args[1].addr;
Relation* worker = (Relation*) args[2].addr;;
bool val;

  result = qp->ResultStorage(s);

  dbs_conn<L> = new BasicEngine_Control<L>(new ConnectionPG(port->GetIntval()
                              ,dbname->toText()),worker);
  val = dbs_conn<L>->checkConn();
  dbms_name = (val) ? pg : "";
  isMaster = val;
  ((CcBool*)result.addr)->Set(true, val);

return 0;
}

/*
1.12.3 Specification

*/
OperatorSpec init_pgWorkerSpec(
   "int x {string, text} x rel --> bool",
   "init_pgWorker(_,_,_)",
   "Set the port and the db-name from PostgreSQL for initialization the local "
   "PG-Worker. Additional you have to specified a Workers-Relation with all "
   "connection information from the worker, including the information "
   "about the second DBMS. The structure of this relation should be "
   "[Host: string, Port: int, Config: string, PGPort: int, DBName: string]",
   "query init_pgWorker(5432,'gisdb',WorkersPG)"
);

/*
1.12.4 ValueMapping Array

*/
ValueMapping init_pgWorkerVM[] = {
  init_pgWorkerSFVM<CcString,ConnectionPG>,
  init_pgWorkerSFVM<FText,ConnectionPG>,
};

/*
1.12.5 Selection Function

*/
int init_pgWorkerSelect(ListExpr args){
if (dbms_name == pg){
  return CcString::checkType(nl->Second(args))?0:1;
}else{
  return 0;
}
};

/*
1.12.6 Operator instance

*/
Operator init_pgWorkerOp(
  "init_pgWorker",
  init_pgWorkerSpec.getStr(),
  sizeof(init_pgWorkerVM),
  init_pgWorkerVM,
  init_pgWorkerSelect,
  init_pgWorkerTM
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
template<class T, class L>
int be_runsqlSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
T* path = (T*) args[0].addr;
bool val;

  result = qp->ResultStorage(s);

  if(dbs_conn<L>){
    val = dbs_conn<L>->runsql(path->toText());
  }
  else{
    cout << noMaster << endl;
  }

((CcBool *)result.addr)->Set(true, val);

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
  be_runsqlSFVM<CcString,ConnectionPG>,
  be_runsqlSFVM<FText,ConnectionPG>,
};

/*
1.13.5 Selection Function

*/
int be_runsqlSelect(ListExpr args){
if (dbms_name == pg){
  return CcString::checkType(nl->First(args))?0:1;
}else{
  return 0;
}
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
             "x real x real x real x int--> bool"
             "(tab-name,geo_col,x-value,y-value,slot size,number of slots)"
             " expected";
  if(!nl->HasLength(args,7)){
    return listutils::typeError("Seven arguments expected. " + err);
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
  if(!CcReal::checkType(nl->Fourth(args))){
    return listutils::typeError("Value of fourth argument have "
        "to be an real." + err);
  }
  if(!CcReal::checkType(nl->Fifth(args))){
    return listutils::typeError("Value of fifth argument have "
        "to be an real." + err);
  }
  if(!CcReal::checkType(nl->Sixth(args))){
    return listutils::typeError("Value of sixth argument have "
        "to be an real." + err);
  }
  if(!CcInt::checkType(nl->Seventh(args))){
    return listutils::typeError("Value of seventh argument have "
        "to be an integer." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
1.14.2 Value Mapping

*/
template<class T, class H, class I, class L>
int be_partGridSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
I* geo_col = (I*) args[2].addr;
CcReal* xstart = (CcReal*) args[3].addr;
CcReal* ystart = (CcReal*) args[4].addr;
CcReal* slotsize = (CcReal*) args[5].addr;
CcInt* slot = (CcInt*) args[6].addr;

bool val = false;
CcBool* res = (CcBool*) result.addr;

  if(dbs_conn<L> && isMaster){
    if (slot->GetIntval() > 0){
      val = dbs_conn<L>->partTable(tab->toText(),key->toText()
            ,"Grid",slot->GetIntval(),geo_col->toText()
      ,xstart->GetValue(),ystart->GetValue(),slotsize->GetValue());
    }else{
      cout<< negSlots << endl;
    }
  }
  else{
    cout << noWorker << endl;
  }
  res->Set(true, val);

return 0;
}

/*
1.14.3 Specification

*/
OperatorSpec be_partGridSpec(
   "{string, text} x {string, text} x {string, text} "
   "x real x real x real x int --> bool",
   "be_partGrid(_,_,_,_,_,_,_)",
   "This operator distribute a relation by specified grid "
   "to the worker. You can specified the leftbottom coordinates and the "
   "size and number of squares. This number of slots and size have to be "
   "positiv. The column should be a geological attribut.",
   "query be_partGrid('roads','gid','geog',5.8, 50.3,0.2,20)"
);

/*
1.14.4 ValueMapping Array

*/
ValueMapping be_partGridVM[] = {
  be_partGridSFVM<CcString,CcString,CcString,ConnectionPG>,
  be_partGridSFVM<FText,CcString,CcString,ConnectionPG>,
  be_partGridSFVM<CcString,FText,CcString,ConnectionPG>,
  be_partGridSFVM<FText,FText,CcString,ConnectionPG>,
  be_partGridSFVM<CcString,CcString,FText,ConnectionPG>,
  be_partGridSFVM<FText,CcString,FText,ConnectionPG>,
  be_partGridSFVM<CcString,FText,FText,ConnectionPG>,
  be_partGridSFVM<FText,FText,FText,ConnectionPG>
};

/*
1.14.5 Selection Function

*/
int be_partGridSelect(ListExpr args){
  if (dbms_name == pg){
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
  }else{
    return 0;
  }
};


/*
1.14.6 Operator instance

*/
Operator be_partGridOp(
  "be_partGrid",
  be_partGridSpec.getStr(),
  sizeof(be_partHashVM),
  be_partGridVM,
  be_partGridSelect,
  be_partGridTM
);

/*
1.15 Implementation of the Algebra

*/
class BasicEngineAlgebra : public Algebra
{
 public:
  BasicEngineAlgebra() : Algebra()
  {
    AddOperator(&init_pgOp);
    AddOperator(&be_partRROp);
    AddOperator(&be_partHashOp);
    AddOperator(&be_partFunOp);
    AddOperator(&be_queryOp);
    AddOperator(&be_commandOp);
    AddOperator(&be_copyOp);
    AddOperator(&be_mqueryOp);
    AddOperator(&be_mcommandOp);
    AddOperator(&be_unionOp);
    AddOperator(&be_structOp);
    AddOperator(&init_pgWorkerOp);
    AddOperator(&be_runsqlOp);
    AddOperator(&be_partGridOp);
  }
  ~BasicEngineAlgebra() {};
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
