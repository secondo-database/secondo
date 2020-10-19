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
#include "ConnectionPG.h"

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
pg is a short name of postgres.

*/
string const pg = "pg";

/*
noConnectionis is just a default string for an error massage.

*/
string const noConnection ="\nPlease use at first an init-Operator before "
    "using this Operator!\n" ;

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

  result = qp->ResultStorage(s);

  dbs_conn<L> = new BasicEngine_Control<L>(new ConnectionPG(port->GetIntval()
                              ,dbname->toText()));
  dbms_name = pg;
  ((CcBool*)result.addr)->Set(true, dbs_conn<L>->checkConn());

return 0;
}

/*
1.1.3 Specification

*/
OperatorSpec init_pgSpec(
   "int x {string, text} --> bool",
   "init_pg(_,_)",
   "Set the port and the db-name for the lokal PG-Worker",
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
  2,
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

  if(dbs_conn<L>){
  val = dbs_conn<L>->partTable(tab->toText(), key->toText()
        ,"RR", slot->GetIntval());
  }
  else{
  cout << noConnection << endl;
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
   "distribute a relation by round-robin "
   "and the workers import this data",
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
  4,
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

  if(dbs_conn<L>){
    val = dbs_conn<L>->partTable(tab->toText(), key->toText()
                ,"Hash",slot->GetIntval());
  }
  else{
    cout << noConnection << endl;
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
   "distribute a relation by hash-key "
   "and the workers import this data",
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
  4,
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

  if(dbs_conn<L>){
    val = dbs_conn<L>->partTable(tab->toText(), key->toText()
              ,fun->toText(),slot->GetIntval());
  }
  else{
    cout << noConnection << endl;
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
   "distribute a relation by e special function "
   "and the workers import this data",
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
  8,
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
    cout << noConnection << endl;
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
   "execute a sql-statement at the locale PG instance "
   "and saves the result in a table",
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
  4,
  be_queryVM,
  be_querySelect,
  be_queryTM
);

/*
1.6 Operator  ~be\_command~

Execute a command the basicengine

1.6.1 Type Mapping

This operator gets a query

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
    cout << noConnection << endl;
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
   "execute a sql-statement at the lokal PG instance",
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
  2,
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
    cout << noConnection << endl;
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
   "creates a copy-statement in postgres and do the query. "
   "be_copy(From,To), From/To can be a table or a path.",
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
  4,
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
bool val = true;
result = qp->ResultStorage(s);
T* query = (T*) args[0].addr;
H* tab = (H*) args[1].addr;

  if(dbs_conn<L>){
    val = dbs_conn<L>->mquery(query->toText(), tab->toText() );
  }
  else{
    cout << noConnection << endl;
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
   "Distribute a query to PG-Worker and "
   "writes the result in a table.",
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
  4,
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
bool val = true;
result = qp->ResultStorage(s);
T* query = (T*) args[0].addr;

  if(dbs_conn<L>){
    val = dbs_conn<L>->mcommand(query->toText());
  }
  else{
    cout << noConnection << endl;
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
   "Distribute a command to PG-Worker",
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
  return CcString::checkType(nl->Second(args))?0:1;
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
  2,
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

  if(dbs_conn<L>){
    val = dbs_conn<L>->munion(tab->toText());
  }
  else{
    cout << noConnection << endl;
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
   "Collecting one table to the PG master.",
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
  2,
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
    cout << noConnection << endl;
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
   "Creating a table-create-Statement, "
   "stores it in a local transferfile folder",
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
  2,
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

  result = qp->ResultStorage(s);

  dbs_conn<L> = new BasicEngine_Control<L>(new ConnectionPG(port->GetIntval()
                              ,dbname->toText()),worker);
  dbms_name = pg;
  ((CcBool*)result.addr)->Set(true, dbs_conn<L>->checkConn());

return 0;
}

/*
1.12.3 Specification

*/
OperatorSpec init_pgWorkerSpec(
   "int x {string, text} x rel --> bool",
   "init_pgWorker(_,_,_)",
   "Set the port and the db-name for the lokal PG-Worker",
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
  return CcString::checkType(nl->First(args))?0:1;
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
  2,
  init_pgWorkerVM,
  init_pgWorkerSelect,
  init_pgWorkerTM
);

/*
1.13 Implementation of the Algebra

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
  }
  ~BasicEngineAlgebra() {};
};

} // end of namespace BasicEngine

/*
1.14 Initialization

*/
extern "C"
Algebra*
InitializeBasicEngineAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  return (new BasicEngine::BasicEngineAlgebra);
}
