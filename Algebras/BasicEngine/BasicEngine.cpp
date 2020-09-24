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

*/
#include "Algebras/FText/FTextAlgebra.h"
#include "StandardTypes.h"

#include "BasicEngine_Control.h"
#include "BasicEngine_Control.cpp"
#include "ConnectionPG.h"

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "include/ConnectionInfo.h"
#include <experimental/filesystem>
#include <filesystem>
#include "boost/filesystem.hpp"
#include <iostream>
#include <stdio.h>

using namespace distributed2;

namespace BasicEngine {

/*
1 Operators

1.1 Operator  ~connect~

Establishes a connection to a running Secondo Server.
The result of this operator is a boolean indicating the success
of the operation.

1.1.1 Type Mapping

This operator gets a hostname, a port and a file for the configuration.

*/
template<class L>
BasicEngine_Control<L>* dbs_conn;

CommandLog commandLog;
NestedList* mynl = new NestedList("temp_nested_list");

const int defaultTimeout = 0;

ListExpr init_pgTM(ListExpr args){
string err = "int x {string, text} --> bool"
       "(port, db-name) expected";

  if(!(nl->HasLength(args,2))){
      //|| nl->HasLength(args,3))){
    return listutils::typeError("Two or three arguments expected. " + err);
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

ListExpr be_partRRTM(ListExpr args){
string err = "{string, text} x {string, text} x rel--> bool"
       "(tab-name, key, worker-relation) expected";

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
        "          to be a string or a text." + err);
  }
  if(!Relation::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "          to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_partHashTM(ListExpr args){
string err = "{string, text} x {string, text} x rel--> bool"
       "(tab-name, key, worker-relation) expected";

  if(!nl->HasLength(args,3)){
    return listutils::typeError("Three arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "          to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "          to be a string or a text." + err);
  }
  if(!Relation::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "          to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_partFunTM(ListExpr args){
string err = "{string, text} x {string, text} x {string, text} x rel--> bool"
       "(tab-name, key, function-name, worker-relation) expected";

  if(!nl->HasLength(args,4)){
    return listutils::typeError("Four arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
        "          to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Second(args))
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
        "          to be a string or a text." + err);
  }
  if(!CcString::checkType(nl->Third(args))
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
        "          to be a string or a text." + err);
  }
  if(!Relation::checkType(nl->Fourth(args))){
    return listutils::typeError("Value of fourth argument have "
        "          to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_queryTM(ListExpr args){
string err = "{string, text} x {string, text} --> bool"
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

ListExpr be_commandTM(ListExpr args){
string err = "{string, text} --> bool"
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

ListExpr be_copyTM(ListExpr args){
string err = "{string, text} x {string, text} --> bool"
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

ListExpr be_mqueryTM(ListExpr args){
string err = "{string,text} x {string,text} x rel -> bool"
       "(sql-query, target-tab, worker-relation) expected";

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
  if(!Relation::checkType(nl->Third(args))){
    return listutils::typeError("Value of third argument have "
                  "to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_mcommandTM(ListExpr args){
string err = "{string,text} x rel -> bool"
       "(sql-command, worker-relation) expected";

  if(!nl->HasLength(args,3)){
    return listutils::typeError("Two arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }
  if(!Relation::checkType(nl->Third(args))){
    return listutils::typeError("Value of secound argument have "
                  "to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_unionTM(ListExpr args){
string err = "{string,text} x rel -> bool"
       "(tab-name, Worker-Relation) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Two arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }
  if(!Relation::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                  "to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_union2TM(ListExpr args){
string err = "{string,text} x rel -> bool"
       "(tab-name, Worker-Relation) expected";

  if(!nl->HasLength(args,2)){
    return listutils::typeError("Two arguments expected. " + err);
  }
  if(!CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError("Value of first argument have "
                  "to be a string or a text." + err);
  }
  if(!Relation::checkType(nl->Second(args))){
    return listutils::typeError("Value of second argument have "
                  "to be a relation." + err);
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

ListExpr be_structTM(ListExpr args){
string err = "{string,text} -> bool"
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

template<class T, class L>
int init_pgSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
CcInt* port = (CcInt*) args[0].addr;
T* dbname = (T*) args[1].addr;

  result = qp->ResultStorage(s);

  dbs_conn<L> = new BasicEngine_Control<L>(new ConnectionPG(port->GetIntval()
                              ,dbname->toText()));
  ((CcBool*)result.addr)->Set(true, dbs_conn<L>->get_conn()->checkConn());

return 0;
}

ValueMapping init_pgVM[] = {
  init_pgSFVM<CcString,ConnectionPG>,
  init_pgSFVM<FText,ConnectionPG>,
  //init_pg_WorkerSFVM<CcString,ConnectionPG>,
  //init_pg_WorkerSFVM<FText,ConnectionPG>
};

int init_pgSelect(ListExpr args){
  //if (nl->HasLength(args,2)){
    return CcString::checkType(nl->First(args))?0:1;
  //}else{
  //  return CcString::checkType(nl->First(args))?2:3;
  //}

};

template<class T, class H, class L>
int be_partRRSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
Relation* worker = (Relation*) args[2].addr;

CcBool* res = (CcBool*) result.addr;
res->SetDefined(false);
bool val;
int i = 1;
string errMsg;
string cmd;
string host;
string config;
string port;
string remoteName;
string localName;
string importPath;
string path = dbs_conn<L>->getFilePath();
string remoteCreateName = "create" + tab->toText() +".sql";
string localCreateName = path + remoteCreateName;

SecondoInterfaceCS* si ;
ConnectionInfo* ci;
Tuple* tuple;
GenericRelationIterator* it = worker->MakeScan();


  val = dbs_conn<L>->partTable(tab->toText(), key->toText()
                , worker->GetNoTuples(), "RR");
  if(!val) return 0;

  //val = dbs_conn<L>->importTable(tab->toText(),  *worker);

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("", "", host, port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);

      //sending data
      remoteName = dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i));
      localName = path + remoteName;
      val = (si->sendFile(localName, remoteName, true) == 0);
      // delete file
      val = val && (remove(localName.c_str()) == 0);
      if (!val){return 0;}

      //sending create Table
      val = (si->sendFile(localCreateName, remoteCreateName, true) == 0);
      if (!val){return 0;}

      importer.push_back(new BasicEngine_Thread(ci
              ,tuple->GetAttribute(4)->toText()
              ,tuple->GetAttribute(3)->toText()
              ,dbs_conn<L>->get_conn()->get_init()));
    }
    i++;
  };

  if(val){
    // delete createFile
    val = (remove(localCreateName.c_str()) == 0);

    //doing the import with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      remoteName = dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i+1));
      importer[i]->startImport( tab->toText(),remoteCreateName, remoteName);
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }


  res->Set(true, val);

return 0;
}

ValueMapping be_partRRVM[] = {
  be_partRRSFVM<CcString,CcString,ConnectionPG>,
  be_partRRSFVM<FText,CcString,ConnectionPG>,
  be_partRRSFVM<CcString,FText,ConnectionPG>,
  be_partRRSFVM<FText,FText,ConnectionPG>
};

int be_partRRSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
};

template<class T, class H, class L>
int be_partHashSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
Relation* worker = (Relation*) args[2].addr;

bool val;
int i = 1;
string errMsg;
string cmd;
string host;
string config;
string port;
string dbName;
string dbPort;
string remoteName;
string localName;
string importPath;
string path = dbs_conn<L>->getFilePath();
string remoteCreateName = "create" + tab->toText() +".sql";
string localCreateName = path + remoteCreateName;

SecondoInterfaceCS* si ;
ConnectionInfo* ci;
Tuple* tuple;
GenericRelationIterator* it = worker->MakeScan();

CcBool* res = (CcBool*) result.addr;
res->SetDefined(false);

  val = dbs_conn<L>->partTable(tab->toText(), key->toText()
                ,worker->GetNoTuples(), "Hash");
  if(!val) return 0;

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("", "", host, port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);

      //sending data
      remoteName = dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i));
      localName = path + remoteName;
      val = (si->sendFile(localName, remoteName, true) == 0);
      val = val && (remove(localName.c_str()) == 0);
      if (!val){return 0;}

      //sending create Table
      val = (si->sendFile(localCreateName, remoteCreateName, true) == 0);
      if (!val){return 0;}
      dbName = tuple->GetAttribute(4)->toText();
      dbPort = tuple->GetAttribute(3)->toText();
      importer.push_back(new BasicEngine_Thread(ci
              ,tuple->GetAttribute(4)->toText()
              , tuple->GetAttribute(3)->toText()
              ,dbs_conn<L>->get_conn()->get_init(dbName,dbPort)));
    } else val = false;
    i++;
  };

  if(val){

    val = (remove(localCreateName.c_str()) == 0);
    //doing the import with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      remoteName = dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i+1));
      importer[i]->startImport( tab->toText(),remoteCreateName, remoteName);
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }

  res->Set(true, val);

return 0;
}

ValueMapping be_partHashVM[] = {
  be_partHashSFVM<CcString,CcString,ConnectionPG>,
  be_partHashSFVM<FText,CcString,ConnectionPG>,
  be_partHashSFVM<CcString,FText,ConnectionPG>,
  be_partHashSFVM<FText,FText,ConnectionPG>
};


int be_partHashSelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
};

template<class T, class H, class N, class L>
int be_partFunSFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
result = qp->ResultStorage(s);

T* tab = (T*) args[0].addr;
H* key = (H*) args[1].addr;
H* fun = (H*) args[2].addr;
Relation* worker = (Relation*) args[3].addr;

bool val;
int i = 1;
string errMsg;
string cmd;
string host;
string config;
string port;
string dbName;
string dbPort;
string remoteName;
string localName;
string importPath;
string path = dbs_conn<L>->getFilePath();
string remoteCreateName = "create" + tab->toText() +".sql";
string localCreateName = path + remoteCreateName;

SecondoInterfaceCS* si ;
ConnectionInfo* ci;
Tuple* tuple;
GenericRelationIterator* it = worker->MakeScan();

CcBool* res = (CcBool*) result.addr;

  val = dbs_conn<L>->partTable(tab->toText(), key->toText()
                ,worker->GetNoTuples(), fun->toText());

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("", "", host, port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);

      //sending data
      remoteName = dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i));
      localName = path + remoteName;
      val = (si->sendFile(localName, remoteName, true) == 0);
      val = val && (remove(localName.c_str()) == 0);
      if (!val){return 0;}

      //sending create Table
      val = (si->sendFile(localCreateName, remoteCreateName, true) == 0);
      if (!val){return 0;}
      dbName = tuple->GetAttribute(4)->toText();
      dbPort = tuple->GetAttribute(3)->toText();
      importer.push_back(new BasicEngine_Thread(ci
              ,tuple->GetAttribute(4)->toText()
              , tuple->GetAttribute(3)->toText()
              ,dbs_conn<L>->get_conn()->get_init(dbName,dbPort)));
    } else val = false;
    i++;
  };

  if(val){

    val = (remove(localCreateName.c_str()) == 0);
    //doing the import with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      remoteName = dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i+1));
      importer[i]->startImport( tab->toText(),remoteCreateName, remoteName);
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }

  res->Set(true, val);

return 0;
}

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


template<class T, class H, class L>
int be_querySFVM(Word* args,Word& result,int message,Word& local,Supplier s ){
string query_exec;
bool val = false;
result = qp->ResultStorage(s); // @suppress("Invalid arguments")

T* query = (T*) args[0].addr;
H* resultTab = (H*) args[1].addr;

  //Delete target Table, ignore failure
  dbs_conn<L>->drop_table(resultTab->toText());

  //execute the query
  val=dbs_conn<L>->createTab(resultTab->toText(),query->toText());

  ((CcBool *)result.addr)->Set(true, val);

return 0;
}

ValueMapping be_queryVM[] = {
  be_querySFVM<CcString,CcString,ConnectionPG>,
  be_querySFVM<FText,CcString,ConnectionPG>,
  be_querySFVM<CcString,FText,ConnectionPG>,
  be_querySFVM<FText,FText,ConnectionPG>
};


int be_querySelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
};

template<class T, class L>
int be_commandSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
bool val = false;
result = qp->ResultStorage(s);

T* query = (T*) args[0].addr;

  val = dbs_conn<L>->get_conn()->sendCommand(query->GetValue());

  ((CcBool *)result.addr)->Set(true, val);

return 0;
}

ValueMapping be_commandVM[] = {
  be_commandSFVM<CcString,ConnectionPG>,
  be_commandSFVM<FText,ConnectionPG>
};


int be_commandSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
};

template<class T, class H, class L>
int be_copySFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
bool val = false;
result = qp->ResultStorage(s);

T* from = (T*) args[0].addr;  //table
H* to = (H*) args[1].addr;    //path

  val = dbs_conn<L>->copy(from->GetValue(),to->GetValue(),
            (from->GetValue().length()>= to->GetValue().length()));

  ((CcBool *)result.addr)->Set(true, val);

return 0;
}

ValueMapping be_copyVM[] = {
  be_copySFVM<CcString,CcString,ConnectionPG>,
  be_copySFVM<FText,CcString,ConnectionPG>,
  be_copySFVM<CcString,FText,ConnectionPG>,
  be_copySFVM<FText,FText,ConnectionPG>
};


int be_copySelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
};

template<class T, class H, class L>
int be_mquerySFVM(Word* args,Word& result,int message
          ,Word& local,Supplier s ){
bool val = true;
result = qp->ResultStorage(s);
T* query = (T*) args[0].addr;
H* tab = (H*) args[1].addr;
Relation* worker = (Relation*) args[2].addr;

  //val = dbs_conn<L>->mquery(tab->toText(),query->toText(),*worker);

string host;
string config;
string port;
string dbName;
string dbPort;
string cmd;
Tuple* tuple;
SecondoInterfaceCS* si ;
string errMsg;
ConnectionInfo* ci;
GenericRelationIterator* it = worker->MakeScan();

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("", "", host, port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);

      dbName = tuple->GetAttribute(4)->toText();
      dbPort = tuple->GetAttribute(3)->toText();
      importer.push_back(new BasicEngine_Thread(ci,dbName,dbPort,
                dbs_conn<L>->get_conn()->get_init(dbName,dbPort)));
    }else val = false;
  }

  if(val){
    //doing the query with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      importer[i]->startQuery(tab->toText() ,query->toText());
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }

  ((CcBool *)result.addr)->Set(true, val);

return 0;
}

/*
Hier muss mehr Kommentiert werden : todo

*/
ValueMapping be_mqueryVM[] = {
  be_mquerySFVM<CcString,CcString,ConnectionPG>,
  be_mquerySFVM<FText,CcString,ConnectionPG>,
  be_mquerySFVM<CcString,FText,ConnectionPG>,
  be_mquerySFVM<FText,FText,ConnectionPG>
};


int be_mquerySelect(ListExpr args){
  if(CcString::checkType(nl->First(args))){
    return CcString::checkType(nl->Second(args))?0:2;
  }else{
    return CcString::checkType(nl->Second(args))?1:3;
  }
};

template<class T, class L>
int be_mcommandSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
bool val = true;
result = qp->ResultStorage(s);
T* query = (T*) args[0].addr;
Relation* worker = (Relation*) args[1].addr;

string host;
string config;
string port;
string dbName;
string dbPort;
string cmd;
Tuple* tuple;
SecondoInterfaceCS* si ;
string errMsg;
ConnectionInfo* ci;
GenericRelationIterator* it = worker->MakeScan();

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("", "", host, port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);

      dbName = tuple->GetAttribute(4)->toText();
      dbPort = tuple->GetAttribute(3)->toText();
      importer.push_back(new BasicEngine_Thread(ci,dbName, dbPort
                ,dbs_conn<L>->get_conn()->get_init(dbName,dbPort)));
    }else val = false;
  }

  if(val){
    //doing the query with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      importer[i]->startCommand(query->toText());
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }

  ((CcBool *)result.addr)->Set(true, val);

return 0;
}

ValueMapping be_mcommandVM[] = {
  be_mcommandSFVM<CcString,ConnectionPG>,
  be_mcommandSFVM<FText,ConnectionPG>
};


int be_mcommandSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
};

template<class T, class L>
int be_unionSFVM(Word* args, Word& result, int message
        , Word& local, Supplier s ){
result = qp->ResultStorage(s);
T* tab = (T*) args[0].addr;
Relation* worker = (Relation*) args[1].addr;
bool val = true;
string path;
string host;
string config;
string port;
string dbName;
string dbPort;
string tabname;
Tuple* tuple;
SecondoInterfaceCS* si ;
string errMsg;
ConnectionInfo* ci;
GenericRelationIterator* it = worker->MakeScan();

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("", "", host, port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);
      dbName = tuple->GetAttribute(4)->toText();
      dbPort = tuple->GetAttribute(3)->toText();
      importer.push_back(new BasicEngine_Thread(ci,dbName, dbPort,
                  dbs_conn<L>->get_conn()->get_init(dbName,dbPort)));
    }else val = false;
  }

  if (val){
    //doing the export with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      //tabname = tab->GetValue() + "_" + to_string(i);
      path = dbs_conn<L>->getFilePath() + ""
          "" + dbs_conn<L>->get_conn()->get_partTabName(tab->toText()
                                ,to_string(i+1));
      importer[i]->startExport(tab->toText(), path,to_string(i+1));
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }

  //import in local PG-Master
  if(val) val = dbs_conn<L>->importData(tab->toText(), worker->GetNoTuples());

  ((CcBool *)result.addr)->Set(true, val);
return 0;
}

ValueMapping be_unionVM[] = {
  be_unionSFVM<CcString,ConnectionPG>,
  be_unionSFVM<FText,ConnectionPG>,
};

int be_unionSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
};

template<class T, class L>
int be_union2SFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
result = qp->ResultStorage(s);
T* tab = (T*) args[0].addr;
Relation* worker = (Relation*) args[1].addr;
bool val = true;

  val = dbs_conn<L>->munion(tab->toText(), *worker);

  ((CcBool *)result.addr)->Set(true, val);
return 0;
}

ValueMapping be_union2VM[] = {
  be_union2SFVM<CcString,ConnectionPG>,
  be_union2SFVM<FText,ConnectionPG>,
};

int be_union2Select(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
};

template<class T, class L>
int be_structSFVM(Word* args, Word& result, int message
          , Word& local, Supplier s ){
bool val = true;

result = qp->ResultStorage(s);
T* tab = (T*) args[0].addr;

  //export a create Statement to filetransfer
  val = dbs_conn<L>->createTabFile(tab->GetValue());

  ((CcBool *)result.addr)->Set(true, val);

return 0;
}

ValueMapping be_structVM[] = {
  be_structSFVM<CcString,ConnectionPG>,
  be_structSFVM<FText,ConnectionPG>,
};

int be_structSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
};

OperatorSpec init_pgSpec(
   "int x {string, text} --> bool",
   "init_pg(_ , _ )",
   "Set the port and the db-name for the lokal PG-Worker",
   "query init_pg(5432,'gisdb')"
);

Operator init_pgOp(
  "init_pg",
  init_pgSpec.getStr(),
  2,
  init_pgVM,
  init_pgSelect,
  init_pgTM
);

OperatorSpec be_partRRSpec(
   "{string, text} x {string, text} x rel--> bool",
   "be_partRR(_ , _, _ )",
   "distribute a relation by round-robin "
   "and the workers import this data",
   "query be_partRR('cars','moid', WorkersPG)"
);

Operator be_partRROp(
  "be_partRR",
  be_partRRSpec.getStr(),
  4,
  be_partRRVM,
  be_partRRSelect,
  be_partRRTM
);

OperatorSpec be_partHashSpec(
   "{string, text} x {string, text} x rel--> bool",
   "be_partHash(_ , _ , _)",
   "distribute a relation by hash-key "
   "and the workers import this data",
   "query be_partHash('cars','moid', WorkersPG)"
);

Operator be_partHashOp(
  "be_partHash",
  be_partHashSpec.getStr(),
  4,
  be_partHashVM,
  be_partHashSelect,
  be_partHashTM
);

OperatorSpec be_partFunSpec(
   "{string, text} x {string, text} x {string, text}  x rel--> bool",
   "be_partFun(_ , _ , _, _)",
   "distribute a relation by e special funktion "
   "and the workers import this data",
   "query be_partFun('cars','moid','random', WorkersPG)"
);

Operator be_partFunOp(
  "be_partFun",
  be_partFunSpec.getStr(),
  4,
  be_partFunVM,
  be_partFunSelect,
  be_partFunTM
);

OperatorSpec be_queryVMSpec(
   "{string, text} x {string, text} --> bool",
   "be_query(_ , _ )",
   "execute a sql-statement at the locale PG instance "
   "and saves the result in a table",
   "query be_query('select * from cars where Speed = 30', 'cars_neu')"
);

Operator be_queryOp(
  "be_query",
  be_queryVMSpec.getStr(),
  4,
  be_queryVM,
  be_querySelect,
  be_queryTM
);

OperatorSpec be_commandVMSpec(
   "{string, text} --> bool",
   "be_command(_ )",
   "execute a sql-statement at the lokal PG instance",
   "query be_command('COPY cars FROM /home/filetransfers/cars_3.bin BINARY')"
);

Operator be_commandOp(
  "be_command",
  be_commandVMSpec.getStr(),
  2,
  be_commandVM,
  be_commandSelect,
  be_commandTM
);

OperatorSpec be_copyVMSpec(
   "{string, text} x {string, text} --> bool",
   "be_copy(_,_)",
   "creates a copy-statement in postgres and do the query. "
   "be_copy(From,To), From/To can be a table or a path.",
   "query be_copy('cars','/home/filetransfers/cars_3.bin')"
);

Operator be_copyOp(
  "be_copy",
  be_copyVMSpec.getStr(),
  4,
  be_copyVM,
  be_copySelect,
  be_copyTM
);

OperatorSpec be_mcommandVMSpec(
   "{string,text} x rel -> bool",
   "_ be_mcommand(_,_)",
   "Distribute a cammand to PG-Worker",
   "query be_mcommand('Drop Table cars;',WorkerPG)"
);

Operator be_mcommandOp(
  "be_mcommand",
  be_mcommandVMSpec.getStr(),
  2,
  be_mcommandVM,
  be_mcommandSelect,
  be_mcommandTM
);

OperatorSpec be_mqueryVMSpec(
   "{string,text} x {string,text} x rel -> bool",
   "be_mquery(_,_,_)",
   "Distribute a query to PG-Worker and "
   "writes the result in a table.",
   "query be_mquery('select * from cars','cars_short',WorkerPG)"
);

Operator be_mqueryOp(
  "be_mquery",
  be_mqueryVMSpec.getStr(),
  4,
  be_mqueryVM,
  be_mquerySelect,
  be_mqueryTM
);

OperatorSpec be_unionVMSpec(
   "{string,text} x rel -> bool",
   "be_union(_ ,_ )",
   "Collecting one table to the PG master.",
   "query be_union('cars_short', WorkersPG)"
);

Operator be_unionOp(
  "be_union",
  be_unionVMSpec.getStr(),
  4,
  be_unionVM,
  be_unionSelect,
  be_unionTM
);

OperatorSpec be_union2VMSpec(
   "{string,text} x rel -> bool",
   "be_union(_ ,_ )",
   "Collecting one table to the PG master.",
   "query be_union2('cars_short', WorkersPG)"
);

Operator be_union2Op(
  "be_union2",
  be_union2VMSpec.getStr(),
  4,
  be_union2VM,
  be_union2Select,
  be_union2TM
);

OperatorSpec be_structVMSpec(
   "{string,text} -> bool",
   "be_struct(_)",
   "Creating a table-create-Statement, "
   "stores it in a local transferfile folder",
   "query be_struct('cars_short')"
);

Operator be_structOp(
  "be_struct",
  be_structVMSpec.getStr(),
  2,
  be_structVM,
  be_structSelect,
  be_structTM
);

/*
2.4 The algebra class

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
    AddOperator(&be_union2Op);
    AddOperator(&be_structOp);
  }
  ~BasicEngineAlgebra() {};
};

} // end of namespace BasicEngine

/*
3 Initialization

*/
extern "C"
Algebra*
InitializeBasicEngineAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  return (new BasicEngine::BasicEngineAlgebra);
}
