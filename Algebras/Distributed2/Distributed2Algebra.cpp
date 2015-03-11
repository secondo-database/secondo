
/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

*/


#include <iostream>
#include <vector>
#include <list>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "Algebra.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "NList.h"


  // use boost for thread handling

#include <boost/thread.hpp>
#include <boost/date_time.hpp>


/*
1 Class Connection

This class represents a connection to a remote Secondo Server.

*/

class ConnectionInfo{

   public:
     ConnectionInfo(const string& _host, const int _port,
                    const string& _config, SecondoInterfaceCS* _si,
                    NestedList* _mynl):
      host(_host), port(_port), config(_config), si(_si){
          mynl = _mynl;
      }

      ~ConnectionInfo(){
          si->Terminate();
          delete si;
          delete mynl;
       }

       string getHost() const{
          return host;
       }

       int getPort() const{
          return port;
       }
       string getConfig() const{
          return config;
       }
    
       bool check() {
          ListExpr res;
          string cmd = "list databases";
          SecErrInfo err;
          si->Secondo(cmd,res,err);
          return err.code==0;
       }

       void simpleCommand(string command, int& err, string& result){
          SecErrInfo serr;
          ListExpr resList;
          assert(si->GetNestedList() == mynl);
          si->Secondo(command, resList, serr);
          err = serr.code;
          if(err==0){
             result = mynl->ToString(resList);
          } else {
             result = si->GetErrorMessage(err);
          }
       }

       void simpleCommand(string command, int& error, string& errMsg, 
                          string& resList){

          SecErrInfo serr;
          ListExpr myResList = mynl->TheEmptyList();

          si->Secondo(command, myResList, serr);
          error = serr.code;
          if(error!=0){
            errMsg = si->GetErrorMessage(error);
          }

          assert(si->GetNestedList() == mynl);

          resList = mynl->ToString(myResList);
          mynl->Destroy(myResList);
       }


       void simpleCommand(string command, int& error, string& errMsg, 
                          ListExpr& resList){
          SecErrInfo serr;
          ListExpr myResList = mynl->TheEmptyList();
          si->Secondo(command, myResList, serr);
          error = serr.code;
          if(error!=0){
            errMsg = si->GetErrorMessage(error);
          }

          assert(si->GetNestedList() == mynl);

          // copy resultlist from local nested list to global nested list
          static boost::mutex copylistmutex;
          copylistmutex.lock();
          assert(mynl!=nl);
          resList =  mynl->CopyList(myResList, nl);
          mynl->Destroy(myResList);
          copylistmutex.unlock();
       }


        bool transferFile( const string& local, const string remote){
          return si->sendFile(local,remote);
        }


  private:
    string host;
    int port;
    string config;
    SecondoInterfaceCS* si;
    NestedList* mynl;
};


class ConnectionListener{
 public:

  virtual void jobDone(int id, string& command, size_t no, int error, 
                       string& errMsg, const string& resList)=0;
  virtual void taskDone() =0;

};


/*
2. Class ConnectionTask

This class manages several commands which should be handlet sequentially
at an existing connection. 

*/
class ConnectionTask{
  public:

     ConnectionTask( ConnectionInfo* _connection, 
                     ConnectionListener* _listener):
      connection(_connection), listener(_listener), 
      running(false),no(0),pc1(0), workerThread(0){} 

     
    ~ConnectionTask(){
        if(workerThread){
           workerThread->interrupt();
           delete workerThread;
        } 
        if(pc1){
            delete pc1; 
        }

     }

     void addCommand(const int id, const string& cmd){
       mtx.lock();
       commandList.push_back(make_pair(id, cmd));
       run();
       mtx.unlock();
     }

     void removeListener(){
        listener = 0;
     }

     


  private:


class pc{
  public:
     pc( ConnectionTask* _ct) : ct(_ct) {

     }
     void operator()(){
        while(!ct->commandList.empty()){
          ct->running = true;
          pair<int, string> cmd1 = ct->commandList.front();
          string cmd = cmd1.second;
          int id = cmd1.first;
          ct->commandList.pop_front();
          int error;
          string errMsg; 
          string resList;
          ct->connection->simpleCommand(cmd, error,errMsg, resList);
          if(ct->listener){
             ct->listener->jobDone(id, cmd, ct->no, error, errMsg, resList);
          }
          ct->no++;
          ct->running = false;
        }
        if(ct->listener){
          ct->listener->taskDone();   
        }
     }

     ConnectionTask* ct;

};

     ConnectionInfo* connection;
     ConnectionListener* listener;
     bool running;
     size_t no;
     pc* pc1;
     boost::thread* workerThread;
     list<pair<int,string> > commandList;
     boost::mutex mtx;      


     void run(){
        if(running){
           return;
        }
        running = true;
        if(workerThread) delete workerThread;
        if(pc1) delete pc1;
        pc1 = new pc(this);
        workerThread = new boost::thread(*pc1);
     } 
};



class Distributed2Algebra: public Algebra{

  public:

/*
1.1 Constructor defined at the end of this file

*/
     Distributed2Algebra();


/*
1.2 Destructor

Closes all open connections and destroys them.

*/
     ~Distributed2Algebra(){
        for(size_t i=0;i<connections.size();i++){
           delete connections[i];
        }
        connections.clear();
     }

/*
~addConnection~

Adds a new connection to the connection pool.

*/
     void addConnection(ConnectionInfo* ci){
       connections.push_back(ci);
     }

/*
~noConnections~

Returns the number of connections

*/

     size_t noConnections(){
         return connections.size();
     }

/*
~getConnection~

Returns the connection at position i

*/
     ConnectionInfo* getConnection(size_t i){
         return connections[i];
     }

/*
~disconnect~

Closes all connections and destroys the instances.
The return value is the number of closed connections.

*/
     int disconnect(){
        int res = connections.size();
        for(size_t i=0;i<connections.size();i++){
          delete connections[i];
        }
        connections.clear();
        return res;
     }
     

/*
~disconnect~

Disconnects a specified connection and removes it from the
connection pool. The result is 0 or 1 depending whether the
given argument specifies an existing connection.

*/
     int disconnect( unsigned int position){
        if( position >= connections.size()){
           return 0;
        }
        delete connections[position];
        connections.erase(connections.begin()+position);
        return 1;
     }

/*
~transferFile~

Transfers a local file to a remove server.

*/
    bool transferFile( const int con, 
                       const string& local,
                       const string& remote){
      if(con < 0 ){
       return false;
      }
      unsigned int con2 = con;
      if(con2 >= connections.size()){
        return false;
      }
      return connections[con2]->transferFile(local,remote);
    }

  private:
    vector<ConnectionInfo*> connections;

};

Distributed2Algebra* algInstance;

/*
1 Operators

1.1 Operator  ~connect~

Establishes a connection to a running Secondo Server.
The result of this operator is a boolean indicating the success 
of the operation. 

1.1.1 Type Mapping

This operator gets a hostname, a port and a file for the configuration.

*/
ListExpr connectTM(ListExpr args){
  string err = "{text,string} x int x {text,string}= "
               "(host, port, configfile) expected";
  if(!nl->HasLength(args,3)){
   return listutils::typeError(err);     
  }
  ListExpr first = nl->First(args);
  if(!FText::checkType(first)&& !CcString::checkType(first)){
    return listutils::typeError(err);     
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err);     
  }
  ListExpr third = nl->Third(args);
  if(!FText::checkType(third) && !CcString::checkType(third)){
    return listutils::typeError(err);     
  }
  return listutils::basicSymbol<CcBool>();
}

/*
1.1.2 Value Mapping

*/
template<class H, class C>
int connectVMT( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;

   H* ahost = (H*) args[0].addr;
   CcInt* aport = (CcInt*) args[1].addr;
   C* afile = (C*) args[2].addr;

   if(!ahost->IsDefined() || !aport->IsDefined() || !afile->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string host = ahost->GetValue();
   int port = aport->GetValue();
   string file = afile->GetValue();
   if(port<=0){
      res->SetDefined(false);
      return 0;
   }
   NestedList* mynl = new NestedList();
   SecondoInterfaceCS* si = new SecondoInterfaceCS(true,mynl);
   string user="";
   string passwd = "";
   string errMsg;
   MessageCenter* msgcenter = MessageCenter::GetInstance();
   si->setMaxAttempts(4);
   si->setTimeout(1);
   if(! si->Initialize(user, passwd, host, 
                       stringutils::int2str(port), file, 
                       errMsg, true)){
      msgcenter->Send(nl, nl->TwoElemList( nl->SymbolAtom("simple"),
                                       nl->TextAtom(errMsg)));
     res->Set(true,false);
   } else {
     res->Set(true,true);
     ConnectionInfo* ci= new ConnectionInfo(host, port, file, si, mynl);
     algInstance->addConnection(ci);     
   }
   return 0;
}

/*
1.1.3 Specification

*/
OperatorSpec connectSpec(
     "text x int x text -> bool",
     "connect(host, port, configfile)",
     "Connects to a Secondo Server",
     " query connect('localhost', 1234, 'SecondoConfig.ini')"); 


/*
1.1.4 ValueMapping Array

*/
ValueMapping connectVM[] = {
  connectVMT<CcString,CcString>,
  connectVMT<CcString,FText>,
  connectVMT<FText, CcString>,
  connectVMT<FText,FText>
};

/*
1.1.5 Selection Function

*/
int connectSelect(ListExpr args){
  int first = FText::checkType(nl->First(args))?2:0;
  int second = FText::checkType(nl->Third(args))?1:0;
  return first + second;
}


/*
1.1.6 Operator instance


*/
Operator connectOp (
    "connect",             //name
     connectSpec.getStr(),         //specification
     4,
     connectVM,        //value mapping
     connectSelect,   //trivial selection function
     connectTM        //type mapping
);


/*
1.2 Operator checkConnection

This operator checks available connections.
The current implementation soes it sequentially. In the future,
this will be parallelized using the boost library.

1.2.1 Type Mapping

This operator have no arguments. The result is a stream of tuples 
of the form (id, host, port, configfile, state)

*/
ListExpr  checkConnectionsTM(ListExpr args){
  
  if(!nl->IsEmpty(args)){
    return listutils::typeError("no arguments required");
  }
  ListExpr attrList = nl->FiveElemList(
    nl->TwoElemList( nl->SymbolAtom("Id"), 
                     listutils::basicSymbol<CcInt>()),
    nl->TwoElemList( nl->SymbolAtom("Host"), 
                     listutils::basicSymbol<FText>()),
    nl->TwoElemList( nl->SymbolAtom("Port"), 
                     listutils::basicSymbol<CcInt>()),
    nl->TwoElemList( nl->SymbolAtom("ConfigFile"), 
                     listutils::basicSymbol<FText>()),
    nl->TwoElemList( nl->SymbolAtom("OK"), 
                     listutils::basicSymbol<CcBool>()));
    return nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                  attrList));
}

/*
1.2.2 Value Mapping

TODO: Make it in parallel

*/

class checkConLocal{
 public:
    checkConLocal(ListExpr resType): pos(0){
      tt = new TupleType(resType);
    }
    ~checkConLocal(){
       tt->DeleteIfAllowed();
    }
    Tuple* next(){
      if(pos >= algInstance->noConnections()){
        return 0;
      }
      ConnectionInfo* con = algInstance->getConnection(pos);
      pos++;
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new CcInt(true,pos-1));
      res->PutAttribute(1, new FText(true,con->getHost()));
      res->PutAttribute(2, new CcInt(con->getPort()));
      res->PutAttribute(3, new FText(true,con->getConfig()));
      res->PutAttribute(4, new CcBool(true, con->check()));
      return res;
    }
 private:
    size_t pos;
    TupleType* tt;

};


int checkConnectionsVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

  checkConLocal* li = (checkConLocal*) local.addr;
  switch(message){
     case OPEN: 
         if(li) delete li;
         local.addr = new checkConLocal(nl->Second(GetTupleResultType(s)));
         return 0;
     case REQUEST: 
         result.addr = li?li->next():0;
         return result.addr?YIELD:CANCEL;
     case CLOSE:
           if(li){
             delete li;
             local.addr = 0;
           }              
           return 0;   
  }
  return -1;

}

/*
1.2.3 Specification

*/
OperatorSpec checkConnectionsSpec(
     "-> stream(tuple((No int)(Host text)(Port int)(Config text)(Ok bool)",
     "checkConnections()",
     "Check connections in the Distributed2Alegbra",
     " query checkConnections() consume"); 


/*
1.2.4 Operator instance


*/
Operator checkConnectionsOp (
    "checkConnections",             //name
    checkConnectionsSpec.getStr(),         //specification
    checkConnectionsVM,        //value mapping
    Operator::SimpleSelect,   //trivial selection function
    checkConnectionsTM        //type mapping
);



/*
1.3 Operator ~rcmd~

This operator performs a remote command on a server.
The result is a tuple stream containing a single tuple containing the 
result of the command.

1.3.1 Type Mapping

*/
ListExpr rcmdTM(ListExpr args){
  string err = "int x {string, text} expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  ListExpr cmdt = nl->Second(args);
  if(!FText::checkType(cmdt) && !CcString::checkType(cmdt)){
    return listutils::typeError(err);
  }
  ListExpr attrList = nl->ThreeElemList(
         nl->TwoElemList( nl->SymbolAtom("Connection"), 
                          listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("ErrorCode"), 
                          listutils::basicSymbol<CcInt>()),
         nl->TwoElemList( nl->SymbolAtom("Result"), 
                          listutils::basicSymbol<FText>()));
  return nl->TwoElemList( 
            listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(
                listutils::basicSymbol<Tuple>(),
                attrList));
}

/*
1.3.2 Value Mapping

*/
template<class C>
int rcmdVMT( Word* args, Word& result, int message,
                        Word& local, Supplier s ){
   switch(message){
      case OPEN: {
           CcInt* con = (CcInt*) args[0].addr;
           C*  cmd = (C*) args[1].addr;
           if(!con->IsDefined() || !cmd->IsDefined()){
             return 0;
           }
           int conv = con->GetValue();
           if(conv<0 || conv>=algInstance->noConnections()){
             return 0;
           }
           ConnectionInfo* ci = algInstance->getConnection(conv);
           int errorCode = 0;
           string result ="";
           ci->simpleCommand( cmd->GetValue(), errorCode, result);
           Tuple* resTuple = new Tuple(
                        new TupleType( nl->Second(GetTupleResultType(s))));
           resTuple->PutAttribute(0, new CcInt(true,conv));
           resTuple->PutAttribute(1, new CcInt(true,errorCode));
           resTuple->PutAttribute(2, new FText(true, result));
           local.addr = resTuple;
            return 0;
       }
       case REQUEST: {
           result.addr = local.addr;
           local.addr = 0;
           return result.addr?YIELD:CANCEL; 
       }
       case CLOSE: {
          if(local.addr){
             ((Tuple*)local.addr)->DeleteIfAllowed();
             local.addr = 0; 
          }
          return 0;
       }
   }
   return -1;
}

/*
1.3.4 ValueMapping Array

*/
ValueMapping rcmdVM[] = {
    rcmdVMT<CcString>,
    rcmdVMT<FText>
};

/*
1.3.5 Selection function

*/
int rcmdSelect(ListExpr args){
   ListExpr cmd = nl->Second(args);
   return CcString::checkType(cmd)?0:1;
}

/*
1.3.6 Specification

*/
OperatorSpec rcmdSpec(
     "int x {text,string} -> stream(Tuple(C,E,R))",
     "rcmd(Connection, Command)",
     "Performs a remove command at given connection",
     "query rcmd(1,'list algebras')"); 

/*
1.3.7 Operator instance

*/
Operator rcmdOp (
    "rcmd",             //name
     rcmdSpec.getStr(),         //specification
     2,
     rcmdVM,        //value mapping
     rcmdSelect,   //trivial selection function
     rcmdTM        //type mapping
);


/*
1.4 Operator disconnect

This operator closes an existing connection. The ID's of 
subsequent connects will be decreased. If no special id
id given, all existing connections will be closed.

1.4.1 Type Mapping

*/
ListExpr disconnectTM(ListExpr args){

  string err = "int or nothing expected";
  if(nl->IsEmpty(args)){
    return listutils::basicSymbol<CcInt>();
  }
  if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
     return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();
}


int disconnectVM( Word* args, Word& result, int message,
                        Word& local, Supplier s ){
  int resi=0;
  if(qp->GetNoSons(s)==0){
    // remove all commections
    resi = algInstance->disconnect();
  } else {
    // remove special connection
    CcInt* c = (CcInt*) args[0].addr;
    if(c->IsDefined() && c->GetValue()>=0){
       resi = algInstance->disconnect(c->GetValue());
    }
  }
  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  res->Set(true,resi);
  return 0;
}

/*
1.4.3 Specification

*/
OperatorSpec disconnectSpec(
     " int -> int , ->int",
     "disconnect(), disconnect(_)",
     "Closes a connection to a remove server. Without any argument"
     " all connections are closed, otherwise only the specifyied one",
     "query disconnect(0)");

/*
1.4.4 Operator instance

*/
Operator disconnectOp (
    "disconnect",             //name
     disconnectSpec.getStr(),         //specification
     disconnectVM,        //value mapping
     Operator::SimpleSelect,   //trivial selection function
     disconnectTM        //type mapping
);



/*
1..5 Operator rquery

This operator works quite similar to the rcmd command. 
The difference is that the correct type will be produced if possible.
This is done using a query on the specified server within the type mapping.
For this reason. only constants for the server number and the command
can be given. This makes only sense for queries. 

*/
template<class S>
bool getValue(ListExpr in, string&out){
   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(in),res);
   if(!success){
     out ="";
     return false;
   }
   S* s = (S*) res.addr;
   if(!s->IsDefined()){
      out = "";
      return false;
   }
   out = s->GetValue();
   s->DeleteIfAllowed();
   return true;
}


ListExpr rqueryTM(ListExpr args){
  string err = "int x {text,string} expected";
  if(!nl->HasLength(args,2)){
     return listutils::typeError(err);
  }
  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!nl->HasLength(first,2) || !nl->HasLength(second,2)){
     return listutils::typeError("internal Error: usesArgsInTypeMapping");
  }
  if(!CcInt::checkType(nl->First(first))){
     return listutils::typeError(err);
  }
  ListExpr serverNo = nl->Second(first);
  // get the server number from the expression
   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(serverNo),res);
   if(!success){
       return listutils::typeError("could not evaluate the value of  " +
                                    nl->ToString(serverNo) );
   }
   CcInt* serverNoV = (CcInt*) res.addr;
   if(!serverNoV->IsDefined()){
       return listutils::typeError("Undefined Server number");
   }
   int sn = serverNoV->GetValue();
   serverNoV->DeleteIfAllowed();
   if(sn < 0 || sn>= (int)(algInstance->noConnections())){
      return listutils::typeError("Invalid server number");
   }
   // ok server number is ok
   string query;
   bool ok;
   if(FText::checkType(nl->First(second))){
      ok = getValue<FText>(nl->Second(second),query);
   } else if(CcString::checkType(nl->First(second))){
      ok = getValue<CcString>(nl->Second(second),query);
   } else {
      return listutils::typeError(err);
   }
   if(!ok){
      return listutils::typeError("Value of second argument invalid");
   }

   query +=  " getTypeNL";
   ConnectionInfo* ci = algInstance->getConnection(sn);
   int error;
   string errMsg;
   ListExpr reslist;
   ci->simpleCommand(query, error, errMsg,  reslist);
   if(error!=0){
     return listutils::typeError("Problem in determining result type : "
                                 + errMsg);
   }
   if(!nl->HasLength(reslist,2)){
      return listutils::typeError("Invalid result get from remote server");
   }
   if(!FText::checkType(nl->First(reslist))){
      return listutils::typeError("Invalid result type for getTypeNL");
   }
   if(nl->AtomType(nl->Second(reslist))!=TextType){
     return listutils::typeError("getTypeNL returns invalid result");
   }
   string typeList = nl->Text2String(nl->Second(reslist));
   ListExpr resType;
   if(!nl->ReadFromString(typeList,resType)){
     return listutils::typeError("getTypeNL returns no valid list expression");
   }   
   if(nl->HasLength(resType,2)){
     first = nl->First(resType);
     if(listutils::isSymbol(first, Stream<Tuple>::BasicType())){
        return listutils::typeError("remote result is a stream");
     }
   }


   return resType; 
}

/*
1.5.2 Value Mapping

*/
void sendMessage(MessageCenter* msg, const string& message){
  ListExpr list = nl->TwoElemList( nl->SymbolAtom("error"),
                                   nl->TextAtom(message));
  msg->Send(nl,list );
  msg->Flush();
}


template<class T>
int rqueryVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){

  MessageCenter*  msg = MessageCenter::GetInstance();
  result = qp->ResultStorage(s);
  CcInt* server = (CcInt*) args[0].addr;
  if(!server->IsDefined()){
     sendMessage(msg, "Server ID undefined");
     return 0;
  }
  int serv = server->GetValue();
  if(serv < 0 || serv >= algInstance->noConnections()){
    sendMessage(msg, "Invalid Server ID " );
    return 0;
  }
  T* query = (T*) args[1].addr;
  if(!query->IsDefined()){
    sendMessage(msg,"Undefined query");
    return 0;
  }
  ConnectionInfo* ci = algInstance->getConnection(serv);
  int error; 
  string errMsg;
  ListExpr resList;
  ci->simpleCommand(query->GetValue(), error, errMsg, resList);
  if(error){
     sendMessage(msg, "Error during remote command" );
     return 0;
  } 
  // Create Object from resList
  if(!nl->HasLength(resList,2)){
     sendMessage(msg,"Invalid result list");
     return 0;
  }
  ListExpr typeList = nl->First(resList);
  ListExpr valueList = nl->Second(resList);
  int algId;
  int typeId;
  string typeName;
  ListExpr nTypeList = SecondoSystem::GetCatalog()->NumericType(typeList);
  bool ok = SecondoSystem::GetCatalog()->LookUpTypeExpr(typeList, 
                                               typeName, algId, typeId);
  if(!ok){ // could not determine algid and typeid for given type list
     sendMessage(msg, string("could not determine type Id and algebra "
                             "id for ") + nl->ToString(typeList) );
     return 0;
  }
  int errorPos=0;
  ListExpr errorInfo = listutils::emptyErrorInfo();
  bool correct;
  AlgebraManager* am = SecondoSystem::GetAlgebraManager();
  Word res = am->InObj(algId,typeId)(nTypeList, valueList, 
                       errorPos, errorInfo, correct);
  if(!correct){ // nested list could not read in correctly
     sendMessage(msg, "InObject failed" );
     return 0;
  }
  qp->ChangeResultStorage(s,res);
  result = qp->ResultStorage(s);
  return 0;
}

/*
1.5.3 ValueMapping Array and Selection Function

*/

ValueMapping rqueryVM[] = {
  rqueryVMT<CcString>,
  rqueryVMT<FText>
};

int rquerySelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
1.5.4  Specification

*/
OperatorSpec rquerySpec(
     " int x {string,text} -> ??",
     " rquery(server, query)",
     " Performs a remove query at a remote server."
     " The server is specified by its Id as the first argument."
     " The second argument is the query. The result type depends"
     " on the type of of the query expression",
     " query rquery(0, 'query ten count') ");

/*
1.4.4 Operator instance

*/
Operator rqueryOp (
    "rquery",             //name
     rquerySpec.getStr(),         //specification
     2,
     rqueryVM,        //value mapping
     rquerySelect,   //trivial selection function
     rqueryTM        //type mapping
);


/*
1.5 Operator prcmd

This operator takes a stream of tuples at least containing an 
integer and a text/string attribute.  These attributes are named 
as secondary arguments. It performed the commands get by the
text attribute on the servers given by the integer attribute. 
While on each connection, the processing is done in a serial way,
the processing between the different servers is processen in 
parallel. Each incoming tuple is extended by the errorCode, the 
errorMessage and the result (as text). Note that the order 
of output tuples depends on the running time of the single tasks,
not on in input order.

*/
ListExpr prcmdTM(ListExpr args){
  string err = "stream(tuple) x attrname x attrname expected";
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError(err + " (first arg is not a tuple stream)");
  }
  if(nl->AtomType(nl->Second(args))!=SymbolType) { 
    return listutils::typeError(err + " 2nd arg is not a "
                                      "valid attribute name");
  }
  if(nl->AtomType(nl->Third(args))!=SymbolType) {
    return listutils::typeError(err + " 3th arg is not a "
                                      "valid attribute name");
  }
  string intAttr = nl->SymbolValue(nl->Second(args));
  string textAttr = nl->SymbolValue(nl->Third(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type;
  int indexi = listutils::findAttribute(attrList,  intAttr, type);
  if(indexi==0){
    return listutils::typeError("Attribute name " + intAttr + " unknown ");
  }
  if(!CcInt::checkType(type)){
    return listutils::typeError("Attribute " + intAttr + " not of type int");
  }
  
  int indext = listutils::findAttribute(attrList, textAttr, type);
  if(indext==0){
    return listutils::typeError("Attribute name " + textAttr + " unknown ");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
    return listutils::typeError("Attribute " + textAttr + 
                                " not of type string or text");
  }

  ListExpr extAttr = nl->ThreeElemList(
              nl->TwoElemList( nl->SymbolAtom("ErrorCode"), 
                               listutils::basicSymbol<CcInt>()),
              nl->TwoElemList( nl->SymbolAtom("ErrorMsg") , 
                               listutils::basicSymbol<FText>()),
              nl->TwoElemList( nl->SymbolAtom("ResultList"), 
                               listutils::basicSymbol<FText>()));

  ListExpr resAttrList = listutils::concat(attrList, extAttr);
  if(!listutils::isAttrList(resAttrList)){
     return listutils::typeError("Name conflics in result Type, one of the"
                                 " names ErrorCode, ErrorMsg, or ResultList "
                                 "already present in inpout tuple");
  }
  ListExpr extList = nl->TwoElemList( nl->IntAtom(indexi-1), 
                                      nl->IntAtom(indext-1));
  ListExpr resType = nl->TwoElemList(
                       listutils::basicSymbol<Stream<Tuple> >(),
                       nl->TwoElemList(
                            listutils::basicSymbol<Tuple>(),
                             resAttrList));
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            extList,
                             resType);
}



template<class T>
class prcmdInfo{

  public:
    prcmdInfo(Word _s, int _intAttrPos, int _qAttrPos, ListExpr _tt)
    {
      tt = new TupleType(_tt);
      inproc = new InputProcessor(_s,_intAttrPos, _qAttrPos, this);
      (*inproc)();
    }

    ~prcmdInfo(){
        tt->DeleteIfAllowed();
        delete inproc;
        for(size_t i=0;i<tasks.size() ;i++){
            if(tasks[i]){
                tasks[i]->removeListener();
                delete tasks[i];
            }
        }
       
     }

    Tuple* next(){
       boost::unique_lock<boost::mutex> lock(waitmut);
       listaccessmut.lock();
       while(outTuples.empty()){
          listaccessmut.unlock();
          cond.wait(lock);
       }
       Tuple* res = outTuples.front();
       outTuples.pop_front();
       listaccessmut.unlock();
       return res;
    }


  private:

     // call back function if the next tuple is available
     void nextTupleAvailable(Tuple* tuple){
         nextmut.lock(); // allow only one process to call this function
         boost::lock_guard<boost::mutex> lock(waitmut);
         listaccessmut.lock();
         outTuples.push_back( tuple);
         listaccessmut.unlock();
         cond.notify_one();
         nextmut.unlock();
     }

 
   class InputProcessor : public ConnectionListener{
      public:
         InputProcessor(Word _s, int _intAttr, int _qAttr, prcmdInfo<T>* _info):
           stream(_s), intPos(_intAttr), qPos(_qAttr), info(_info) {
           currentId = 0;
           noInTuples = 0;
           noOutTuples = 0;
           inputDone = false;
           stopped= false;
           stream.open();
         }

         virtual ~InputProcessor(){
            mut.lock();
            stopped = true;
            stream.close();
            mut.unlock();
          }

          void operator()() {
             Tuple* inTuple;  
             mut.lock();
             inTuple = stream.request();
             bool stop = stopped;
             mut.unlock();
             while(inTuple!=0 && ! stop){
               noInTuples++;
               CcInt* Server = (CcInt*) inTuple->GetAttribute(intPos);
               T* q = (T*) inTuple->GetAttribute(qPos);
               if(!Server->IsDefined() ){
                  processInvalidTuple(inTuple, -2, "Undefined server number");
               } 
               if(!q->IsDefined()){
                  processInvalidTuple(inTuple, -3, "Undefined query");
               }
               int server = Server->GetValue();
               if(server < 0 || server>= algInstance->noConnections()){
                  processInvalidTuple(inTuple, -3, "Invalid server number");
               } else {
                  string query = q->GetValue(); 
                  processValidTuple(inTuple, server, query); 
               }
               mut.lock();
               stop = stopped;
               inTuple=stop?0:stream.request(); 
               mut.unlock();
             }
             inputDone = true;
             if(noInTuples==0 || noInTuples == noOutTuples){
                 info->nextTupleAvailable(0);
             }
          }

          virtual void jobDone(int id, string& command, size_t no, int error,
                               string& errMsg, const string& resList){
               mapmut.lock();
               if(inTuples.find(id)==inTuples.end()){
                 cerr << "internal error, could not find input tuple fot id "
                      << id;
                 mapmut.unlock();
                 return;
               } 
               Tuple* inTuple = inTuples[id];
               inTuples.erase(id);
               mapmut.unlock();
               createResultTuple(inTuple,error, errMsg, resList);
          }


      private:
         Stream<Tuple> stream;
         int intPos;
         int qPos;
         prcmdInfo<T>* info;
         bool stopped;
         boost::mutex mut;
         boost::mutex outmut;
         boost::mutex mapmut;
         map<int,Tuple*> inTuples;
         int currentId;
         int noInTuples;
         int noOutTuples;
         bool inputDone;

         void processInvalidTuple(Tuple* inTuple, int errCode, 
                                  const string& errMsg){
            createResultTuple(inTuple, errCode, errMsg, "()");
         }

         void processValidTuple(Tuple* inTuple, size_t serverno, string query){
             currentId++;
             inTuples[currentId] = inTuple;
             while(info->tasks.size() <= serverno){ // extend task vector
               info->tasks.push_back(0);
             }
             if(!info->tasks[serverno]){
                 info->tasks[serverno] = new ConnectionTask(
                                   algInstance->getConnection(serverno), 
                                   this);
             }
             info->tasks[serverno]->addCommand(currentId, query);
         }

         void createResultTuple(Tuple* inTuple, int errorCode, 
                                const string& errMsg, const string& resList){
            mut.lock();
            if(stopped){
              inTuple->DeleteIfAllowed();
              mut.unlock();
              return; 
            }
            mut.unlock();
            outmut.lock();
            Tuple* resTuple = new Tuple(info->tt);
            // copy attributes
            int no = inTuple->GetNoAttributes();
            for(int i=0;i<no ; i++){
               resTuple->CopyAttribute(i,inTuple,i);   
            }
            inTuple->DeleteIfAllowed();
            resTuple->PutAttribute(no, new CcInt(errorCode));
            resTuple->PutAttribute(no+1 , new FText(true, errMsg));
            resTuple->PutAttribute(no+2, new FText(true, resList));
            noOutTuples++;
            info->nextTupleAvailable(resTuple);
            if(inputDone && noInTuples == noOutTuples){
              info->nextTupleAvailable(0); // mark end of processing
            } 
            outmut.unlock();
         }
         void taskDone(){}
   };

   TupleType* tt;
   vector<Tuple*> tuplestore;
   InputProcessor* inproc;
   vector<ConnectionTask*> tasks;
   boost::condition_variable cond;
   boost::mutex waitmut;
   boost::mutex nextmut;
   boost::mutex listaccessmut;
   list<Tuple*> outTuples; // the next out tuple
};



/*
1.5.3 Value Mapping

*/
template<class T>
int prcmdVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){

  prcmdInfo<T>* li = (prcmdInfo<T>*) local.addr;
  switch(message){
    case OPEN: {
       if(li){
         delete li;
       }
       local.addr = new prcmdInfo<T>(args[0], 
                                  ((CcInt*)args[3].addr)->GetValue(),
                                  ((CcInt*)args[4].addr)->GetValue(),
                                  nl->Second(GetTupleResultType(s)));
       return 0;
    }
    case REQUEST:
           result.addr = li?li->next():0;
           return result.addr?YIELD:CANCEL;
    case CLOSE:
         if(li){
           delete li;
           local.addr =0;
         }
         return 0;
  } 
  return -1;
};


/*
1.6.3 ValueMapping Array and Selection Function

*/

ValueMapping prcmdVM[] = {
  prcmdVMT<CcString>,
  prcmdVMT<FText>
};

int prcmdSelect(ListExpr args){
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr textType;
  listutils::findAttribute(attrList, 
                    nl->SymbolValue(nl->Third(args)), textType);
  return CcString::checkType(textType)?0:1;
}

/*
1.5.4  Specification

*/
OperatorSpec prcmdSpec(
     " stream(tuple) x attrName x attrName -> stream(tuple + E)",
     " _ prcmd [ _,_]",
     " Performs a a set of remote queries in parallel.",
     " query intstream(0,3) namedtransformstream[S] "
     "extend[ Q : 'query plz feed count'] prcmd[S,Q] consume ");

/*
1.4.4 Operator instance

*/
Operator prcmdOp (
    "prcmd",             //name
     prcmdSpec.getStr(),         //specification
     2,
     prcmdVM,        //value mapping
     prcmdSelect,   //trivial selection function
     prcmdTM        //type mapping
);





/*
2 Implementation of the Algebra

*/
Distributed2Algebra::Distributed2Algebra(){
   AddOperator(&connectOp);
   AddOperator(&checkConnectionsOp);
   AddOperator(&rcmdOp);
   AddOperator(&disconnectOp);
   AddOperator(&rqueryOp);
   rqueryOp.SetUsesArgsInTypeMapping();
   AddOperator(&prcmdOp);
}


extern "C"
Algebra*
   InitializeDistributed2Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

   algInstance = new Distributed2Algebra();
   return algInstance;
}


