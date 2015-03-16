
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
          simtx.lock();
          si->Terminate();
          simtx.unlock();
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
          simtx.lock();
          si->Secondo(cmd,res,err);
          simtx.unlock();
          return err.code==0;
       }

       void simpleCommand(string command, int& err, string& result){
          SecErrInfo serr;
          ListExpr resList;
          assert(si->GetNestedList() == mynl);
          simtx.lock();
          si->Secondo(command, resList, serr);
          err = serr.code;
          if(err==0){
             result = mynl->ToString(resList);
          } else {
             result = si->GetErrorMessage(err);
          }
          simtx.unlock();
       }

       void simpleCommand(string command, int& error, string& errMsg, 
                          string& resList){

          SecErrInfo serr;
          ListExpr myResList = mynl->TheEmptyList();
          simtx.lock();
          si->Secondo(command, myResList, serr);
          error = serr.code;
          if(error!=0){
            errMsg = si->GetErrorMessage(error);
          }
          simtx.unlock();
          assert(si->GetNestedList() == mynl);

          resList = mynl->ToString(myResList);
          mynl->Destroy(myResList);
       }


       void simpleCommand(string command, int& error, string& errMsg, 
                          ListExpr& resList){
          SecErrInfo serr;
          ListExpr myResList = mynl->TheEmptyList();
          simtx.lock();
          si->Secondo(command, myResList, serr);
          error = serr.code;
          if(error!=0){
            errMsg = si->GetErrorMessage(error);
          }
          simtx.unlock();

          assert(si->GetNestedList() == mynl);

          // copy resultlist from local nested list to global nested list
          static boost::mutex copylistmutex;
          copylistmutex.lock();
          assert(mynl!=nl);
          resList =  mynl->CopyList(myResList, nl);
          mynl->Destroy(myResList);
          copylistmutex.unlock();
       }


        bool transferFile( const string& local, const string& remote){
          simtx.lock();
          bool res =  si->sendFile(local,remote);
          simtx.unlock();
          return res;
        }
        

        bool requestFile( const string& remote, const string& local){
          simtx.lock();
          int res =  si->requestFile(remote, local);
          simtx.unlock();
          return res==0;
        }


  private:
    string host;
    int port;
    string config;
    SecondoInterfaceCS* si;
    NestedList* mynl;
    boost::mutex simtx; // mutex for synchronizing access to the interface
};


class ConnectionListener{
 public:

  virtual void jobDone(int id, string& command, size_t no, int error, 
                       string& errMsg, const string& resList)=0;
};


/*
2. Class ConnectionTask

This class manages the task for a single connection.
In an endless loop it will wait for a new command. If a 
new command is available, the command is executed and
the listener is informed about that. Then, then it waits for
the next command.

*/
class ConnectionTask{
  public:

     ConnectionTask( ConnectionInfo* _connection, 
                     ConnectionListener* _listener):
      connection(_connection), listener(_listener), no(0),
      done(false), pc1(this), commandList(), listmtx(), 
      condmtx(), cond(),  worker(pc1){
         
      } 
     
    ~ConnectionTask(){
          listener = 0;
          done = true;
          cond.notify_one();
          worker.join(); // wait until worker is done
      }

     void addCommand(const int id, const string& cmd){
       boost::lock_guard<boost::mutex> lock(condmtx);
       listmtx.lock(); 
       commandList.push_back(make_pair(id, cmd));
       listmtx.unlock();
       cond.notify_one();        
     }

     void removeListener(){
        listener = 0;
     }

  private:
class pc{
  public:
     pc( ConnectionTask* _ct) : ct(_ct){

     }
     void operator()(){
      while(!ct->done){
         boost::unique_lock<boost::mutex> lock(ct->condmtx);
         // wait for available data
         ct->listmtx.lock();
         while(!ct->done && ct->commandList.empty()){
            ct->listmtx.unlock();
            ct->cond.wait(lock);
            ct->listmtx.lock();
         }
         if(ct->done){
           ct->listmtx.unlock();
           return; 
         }
         pair<int, string> cmd1 = ct->commandList.front();
         ct->commandList.pop_front();
         ct->listmtx.unlock();
         ct->no++;
         string cmd = cmd1.second;
         int id = cmd1.first;

         int error;
         string errMsg; 
         string resList;
         ct->connection->simpleCommand(cmd, error,errMsg, resList);
         if(ct->listener){
            ct->listener->jobDone(id, cmd, ct->no, error, errMsg, resList);
         }
         ct->no++;
      }
     }

     ConnectionTask* ct;
};

     ConnectionInfo* connection;
     ConnectionListener* listener;
     size_t no;
     bool done;
     pc pc1;
     list<pair<int,string> > commandList;
     boost::mutex listmtx; //synchronize list access    
     boost::mutex condmtx; 
     boost::condition_variable cond;
     boost::thread worker;
     
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
    

    bool requestFile( const int con, 
                       const string& remote,
                       const string& local){
      if(con < 0 ){
       return false;
      }
      unsigned int con2 = con;
      if(con2 >= connections.size()){
        return false;
      }
      return connections[con2]->requestFile(remote,local);
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
        for(size_t i=0;i<tasks.size() ;i++){
            if(tasks[i]){
                tasks[i]->removeListener();
                delete tasks[i];
            }
        }
        delete inproc;
        tt->DeleteIfAllowed();
        listaccessmut.lock();
        while(!outTuples.empty()){
           delete outTuples.front();
           outTuples.pop_front();
        }
        listaccessmut.unlock();
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
            // delete non processed input tuples
            map<int,Tuple*>::iterator it;
            for(it = inTuples.begin(); it != inTuples.end(); it++){
                 delete it->second;
            }
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
//template<class T>
//void del(T* info){
//   delete info;
//}


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
        //   boost::thread k(del<prcmdInfo<T> >,li); 
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
1.6 Operator 'transferFile'

Copies a local file to a remove server.  This version works  
serial.

1.6.1 Type Mapping

The arguments are the server number, the name of the local file as well as 
the name of the remote file.

*/
ListExpr transferFileTM(ListExpr args){
  string err = "int x {string, text} x {string, text} expected";

  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(    !CcString::checkType(nl->Second(args)) 
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError(err);
  } 
  if(    !CcString::checkType(nl->Third(args)) 
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError(err);
  } 
  return listutils::basicSymbol<CcBool>();
}

template<class L , class R>
int transferFileVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){


   CcInt* Server = (CcInt*) args[0].addr;
   L* Local = (L*) args[1].addr;
   R* Remote = (R*) args[2].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!Server->IsDefined() || ! Local->IsDefined() || !Remote->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   int server = Server->GetValue();
   if(server < 0 || server >= algInstance->noConnections()){
      res->Set(true,false);
      return 0;
   }
   res->Set(true, algInstance->transferFile(server, 
                                 Local->GetValue(), 
                                 Remote->GetValue()));
   return 0;
}

/*
1.6.2 Specification

*/

OperatorSpec transferFileSpec(
     " int x {string, text} x {string, text} -> bool",
     " transferFile( serverNo, localFile, remoteFile) ",
     " Transfers a local file to the remote server. ",
     " query transferFile( 0, 'local.txt', 'remote.txt' ");

/*
1.6.3 Value Mapping Array and Selection

*/

ValueMapping transferFileVM[] = {
  transferFileVMT<CcString, CcString>,
  transferFileVMT<CcString, FText>,
  transferFileVMT<FText, CcString>,
  transferFileVMT<FText, FText>
};

int transferFileSelect(ListExpr args){

  int n1 = CcString::checkType(nl->Second(args))?0:2;
  int n2 = CcString::checkType(nl->Third(args))?0:1;
  return n1+n2; 
}

/*
1.6.4 Operator instance

*/

Operator transferFileOp (
    "transferFile",             //name
     transferFileSpec.getStr(),         //specification
     4,
     transferFileVM,        //value mapping
     transferFileSelect,   //trivial selection function
     transferFileTM        //type mapping
);


/*
1.7 Operator requestFile

This operator request a file from a remote server.

1.7.1 Type Mapping

The arguments for this operator are the number of the server,
the name of the file at server side and the local file name.
The result is a boolean reporting the success of the file 
transfer.

*/

ListExpr requestFileTM(ListExpr args){
  string err = "int x {string, text} x {string, text} expected";

  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  if(    !CcString::checkType(nl->Second(args)) 
      && !FText::checkType(nl->Second(args))){
    return listutils::typeError(err);
  } 
  if(    !CcString::checkType(nl->Third(args)) 
      && !FText::checkType(nl->Third(args))){
    return listutils::typeError(err);
  } 
  return listutils::basicSymbol<CcBool>();
}

/*
1.7.2 Value Mapping

*/

template<class R , class L>
int requestFileVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){


   CcInt* Server = (CcInt*) args[0].addr;
   R* Remote = (R*) args[1].addr;
   L* Local = (L*) args[2].addr;
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   if(!Server->IsDefined() || ! Local->IsDefined() || !Remote->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   int server = Server->GetValue();
   res->Set(true, algInstance->requestFile(server, 
                                 Remote->GetValue(), 
                                 Local->GetValue()));
   return 0;
}


OperatorSpec requestFileSpec(
     " int x {string, text} x {string, text} -> bool",
     " requestFile( serverNo, remoteFile, localFile) ",
     " Transfers a remopte file to the local file system. ",
     " query requestFile( 0, 'remote.txt', 'local.txt' ");

/*
1.6.3 Value Mapping Array and Selection

*/

ValueMapping requestFileVM[] = {
  requestFileVMT<CcString, CcString>,
  requestFileVMT<CcString, FText>,
  requestFileVMT<FText, CcString>,
  requestFileVMT<FText, FText>
};

int requestFileSelect(ListExpr args){

  int n1 = CcString::checkType(nl->Second(args))?0:2;
  int n2 = CcString::checkType(nl->Third(args))?0:1;
  return n1+n2; 
}

/*
1.6.4 Operator instance

*/

Operator requestFileOp (
    "requestFile",             //name
     requestFileSpec.getStr(),         //specification
     4,
     requestFileVM,        //value mapping
     requestFileSelect,   //trivial selection function
     requestFileTM        //type mapping
);



/*
1.7 Operator ~ptransferFile~

This operator transfers local files to connected SecondoServers in parallel.

1.7.1 Type Mapping

The operator receives a stream of tuples at least having a number and 
text/string attributes. Furthermore three attribute names are required. 
The first attribute name points to an integer attribute specifing the 
connection where the file should be transferred to. The second attribute 
name points to the local filename within the tuple. This may be a string 
or a text. The third attribute name points to the name of the file which 
should be created at the remote server. 
The last two attribute names may be the same (in this case the local file 
name is equal to the remote file name.

Each input tuple is extened by a boolean and an text attribute. 
The boolean value points to the success of the file transfer and 
the text value will given an error message if fauled.

*/

ListExpr ptransferFileTM(ListExpr args){

  string err = "stream(tuple) x attrname x attrname x attrname expected";

  if(!nl->HasLength(args,4)){
    return listutils::typeError(err + " (invalid number of attributes)");
  }
  ListExpr stream = nl->First(args);
  ListExpr attrs[] = { nl->Second(args), nl->Third(args), nl->Fourth(args)};

  if(!Stream<Tuple>::checkType(stream)){
    return listutils::typeError(err + " ( first arg is not a tuple stream)");
  }
  for(int i=0;i<3;i++){
    if(nl->AtomType(attrs[i])!=SymbolType){
       return listutils::typeError(err + " ( invalid attribute name found )");
    }
  }
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr type;
  string name = nl->SymbolValue(attrs[0]);
  int indexServer = listutils::findAttribute(attrList, name, type);
  if(!indexServer){
    return listutils::typeError(" attribute " + name + " not found");
  }
  if(!CcInt::checkType(type)){
    return listutils::typeError(" attribute " + name + " not an integer");
  }

  name = nl->SymbolValue(attrs[1]);
  int indexLocal = listutils::findAttribute(attrList, name, type);
  if(!indexLocal){
    return listutils::typeError(" attribute " + name + " not found");
  }
  if(!CcString::checkType(type) &&!FText::checkType(type)){
    return listutils::typeError(" attribute " + name + 
                                " is not of type string or text");
  }

  name = nl->SymbolValue(attrs[2]);
  int indexRemote = listutils::findAttribute(attrList, name, type);
  if(!indexRemote){
    return listutils::typeError(" attribute " + name + " not found");
  }
  if(!CcString::checkType(type) &&!FText::checkType(type)){
    return listutils::typeError(" attribute " + name + 
                                " is not of type string or text");
  }

  // input ok, generate output
  ListExpr extAttr = nl->TwoElemList(
                    nl->TwoElemList(
                            nl->SymbolAtom("OK"), 
                            listutils::basicSymbol<CcBool>()),
                    nl->TwoElemList(
                            nl->SymbolAtom("ErrMsg"), 
                            listutils::basicSymbol<FText>()) );

  ListExpr newAttrList = listutils::concat(attrList, extAttr);
  if(!listutils::isAttrList(newAttrList)){
    return listutils::typeError("Attribute OK or ErrMsg "
                                "already present in tuple");
  }
  ListExpr appendList = nl->ThreeElemList(
                          nl->IntAtom(indexServer-1),
                          nl->IntAtom(indexLocal-1),
                          nl->IntAtom(indexRemote-1));

  ListExpr resultList = nl->TwoElemList(
                            listutils::basicSymbol<Stream<Tuple> >(),
                            nl->TwoElemList(
                                listutils::basicSymbol<Tuple>(),
                                newAttrList));

  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            appendList,
                            resultList);  
}


/*
1.7.2 Class ~ptransferLocal~

This class implements parallel file transfer.

*/

class transferFileListener{

  public:
    virtual void  transferFinished(Tuple* intuple, 
                               bool success, 
                               const string& msg ) =0;

    virtual void setInputTupleNumber(int no)=0;
};

struct transfer{
  transfer(Tuple* _tuple, const string& _local, const string& _remote):
    inTuple(_tuple), local(_local), remote(_remote){}

  Tuple* inTuple;
  string local;
  string remote;
};

class fileTransferator{
  public:
     fileTransferator(int _server, transferFileListener* _listener) :
         server(_server), listener(_listener), stopped(false){
         //runner = new Runner(this);
         // runnerThread = new boost::thread(*runner);
         runnerThread = boost::thread(
                        boost::bind(&fileTransferator::run, this));
     }

     ~fileTransferator(){
        stop();
        runnerThread.join();
        listAccess.lock();
        while(!pendingtransfers.empty()){
           transfer t = pendingtransfers.front();
           t.inTuple->DeleteIfAllowed();
           pendingtransfers.pop_front();
        }
        listAccess.unlock();
     }

    
     void stop(){
          stopped = true;
          boost::lock_guard<boost::mutex> lock(condmtx);
          cond.notify_one();
     }

      void addTransfer(Tuple* inTuple, const string& local, 
                       const string& remote) {
          transfer newTransfer(inTuple, local, remote);
          listAccess.lock();
          pendingtransfers.push_back(newTransfer);
          listAccess.unlock();
          boost::lock_guard<boost::mutex> lock(condmtx);
          cond.notify_one();
      }

  private:

      void run(){
          boost::unique_lock<boost::mutex> lock(condmtx);
          while(!stopped){
               // wait for date
               listAccess.lock();
               while(!stopped && pendingtransfers.empty()){
                   listAccess.unlock();
                   cond.wait(lock); 
               }
               listAccess.unlock();
               if(!stopped){
                   listAccess.lock();
                   transfer t = pendingtransfers.front();
                   pendingtransfers.pop_front();   
                   listAccess.unlock();
                   bool ret = algInstance->transferFile(server, 
                                                       t.local, t.remote);
                   if(listener){
                      if(ret){
                          listener->transferFinished(t.inTuple, true, "");
                      } else {
                          listener->transferFinished(t.inTuple, false, 
                                                   "Error during transfer"); 
                      }
                   } else {
                      t.inTuple->DeleteIfAllowed();
                   }
               }
          }
      }

    int server;
    transferFileListener* listener;
    bool stopped;
    list<transfer>  pendingtransfers;
    boost::mutex listAccess;
    boost::thread runnerThread;
    boost::mutex condmtx;
    boost::condition_variable cond;
};


template<class L, class R>
class ptransferFileInputProcessor{
     public:
        ptransferFileInputProcessor(Word& _stream, 
                                   int _serverIndex, int _localIndex, 
                       int _remoteIndex, transferFileListener* _listener):
          stream(_stream), serverIndex(_serverIndex), localIndex(_localIndex),
          remoteIndex(_remoteIndex), listener(_listener), stopped(false) 
       {
          stream.open();
        }

        ptransferFileInputProcessor(const ptransferFileInputProcessor& src):
          stream(src.stream), serverIndex(src.serverIndex), 
          localIndex(src.localIndex),
          remoteIndex(src.remoteIndex), listener(src.listener), 
          stopped(src.stopped),
          activeThreads(src.activeThreads) {
        }



        ~ptransferFileInputProcessor(){
           stream.close();
           // stop active threads
           map<int, fileTransferator*>::iterator it;
           for(it = activeThreads.begin(); it!=activeThreads.end(); it++){
               it->second->stop();
           }
           // delete active threads
           for(it = activeThreads.begin(); it!=activeThreads.end(); it++){
               delete it->second;
           }
         }

         void stop(){
            stopped=true;
         }

         void run(){
            Tuple* inTuple;
            inTuple = stream.request();
            int noInTuples = 0; 
            while(inTuple && !stopped) {
               noInTuples++;
               CcInt* Server = (CcInt*) inTuple->GetAttribute(serverIndex);
               L* Local = (L*) inTuple->GetAttribute(localIndex);
               R* Remote = (R*) inTuple->GetAttribute(remoteIndex);

               if(!Server->IsDefined() || !Local->IsDefined()
                  || !Remote->IsDefined()){
                   processInvalidTuple(inTuple, "undefined value found");
               } else {
                  int server = Server->GetValue(); 
                  string localname = Local->GetValue();
                  string remotename = Remote->GetValue();
                  if(server < 0 || server >= algInstance->noConnections()){
                     processInvalidTuple(inTuple, "invalid server number");
                  } else {
                    processValidTuple(inTuple, server, localname, remotename);
                  }
               }
               inTuple = stream.request();
            }
            if(inTuple){
                inTuple->DeleteIfAllowed();
            }
            listener->setInputTupleNumber(noInTuples);
          }
    private:
       Stream<Tuple> stream;
       int serverIndex;
       int localIndex;
       int remoteIndex;
       transferFileListener* listener;
       bool stopped;
       map<int, fileTransferator*> activeThreads;

       void processInvalidTuple(Tuple* inTuple, const string& msg){
            if(listener){
               listener->transferFinished(inTuple, false, msg);
            } else {
                inTuple->DeleteIfAllowed();
            }
       }

       void processValidTuple(Tuple* inTuple, const int server, 
                              const string& local, const string& remote){

          if(activeThreads.find(server)==activeThreads.end()){
              fileTransferator* fi = new fileTransferator(server, listener);
              activeThreads[server] = fi; 
          } 
          activeThreads[server]->addTransfer(inTuple, local,remote);
       }
  };



template<class L, class R>
class ptransferLocal: public transferFileListener {

  public:
     ptransferLocal(Word& stream, int _serverindex,
                    int _localindex, int  _remoteindex,
                    ListExpr _tt) {
        inputTuples = -1; // unknown number of input tuples
        outputTuples = 0; // up to now, there is no output tuple
        tt = new TupleType(_tt);
        inputProcessor = 
              new ptransferFileInputProcessor<L,R>(stream,_serverindex, 
                                           _localindex, _remoteindex, 
                                           this);
        runner = new boost::thread( boost::bind(
                &ptransferFileInputProcessor<L,R>::run, inputProcessor));

     }


     virtual ~ptransferLocal(){
        inputProcessor->stop(); // stop processing
        runner->join(); // wait until started jobs are done
        delete runner;
        delete inputProcessor;

        list<Tuple*>::iterator it;
        // remove already created result tuples
        listAccess.lock();
        for(it = resultList.begin();it!=resultList.end();it++){
            (*it)->DeleteIfAllowed();
        }
        listAccess.unlock();
        tt->DeleteIfAllowed();
     }


     Tuple* next(){
        boost::unique_lock<boost::mutex> lock(resultMutex);
        listAccess.lock();
        while(resultList.empty()){
            listAccess.unlock();
            resultAvailable.wait(lock);
        }
        Tuple* res = resultList.front();
        resultList.pop_front();
        listAccess.unlock();
        return res;
     } 


     // callback function for a single finished transfer
     virtual void transferFinished(Tuple* inTuple, bool success, 
                                   const string& msg){


        tupleCreationMtx.lock();

        Tuple* resTuple =  new Tuple(tt);
        // copy original attributes
        for(int i=0;i<inTuple->GetNoAttributes(); i++){
           resTuple->CopyAttribute(i,inTuple,i);
        } 
        resTuple->PutAttribute(inTuple->GetNoAttributes(), 
                               new CcBool(true,success));
        resTuple->PutAttribute(inTuple->GetNoAttributes() + 1, 
                               new FText(true,msg));

        inTuple->DeleteIfAllowed();


        boost::lock_guard<boost::mutex> lock(resultMutex);

        listAccess.lock();
        resultList.push_back(resTuple);
        listAccess.unlock();




        // update counter
        outputTuples++;
        if(inputTuples==outputTuples){
          // this was the last tuple to create
          listAccess.lock();
          resultList.push_back(0);
          listAccess.unlock();
        }
        tupleCreationMtx.unlock();
        resultAvailable.notify_one();
     }

     virtual void setInputTupleNumber(int i){
        if(i==0){ // we cannot expect any results
           boost::lock_guard<boost::mutex> lock(resultMutex);
           listAccess.lock();
           resultList.push_back(0);
           listAccess.unlock();
           // inform about new tuple.
           resultAvailable.notify_one(); 
        } else {
          assert(i>0);
          countMtx.lock();
          inputTuples = i;
          if(inputTuples == outputTuples){
             // all output tuples was generated
             countMtx.unlock();
             boost::lock_guard<boost::mutex> lock(resultMutex);
             listAccess.lock();
             resultList.push_back(0);
             listAccess.unlock();
             // inform about new tuple.
             resultAvailable.notify_one(); 
          }
        }
     }
  private:
     TupleType* tt;
     bool finished;
     ptransferFileInputProcessor<L,R>* inputProcessor;
     boost::thread* runner;
     list<Tuple*> resultList;
     boost::condition_variable resultAvailable;
     boost::mutex resultMutex;
     boost::mutex listAccess;
     boost::mutex tupleCreationMtx;
     int inputTuples;
     int outputTuples;
     boost::mutex countMtx;
     
};




template<class L , class R>
int ptransferFileVMT( Word* args, Word& result, int message,
               Word& local, Supplier s ){

   ptransferLocal<L,R>* li = (ptransferLocal<L,R>*) local.addr;

   switch(message){
      case OPEN: {
               if(li) delete li;
               int serv = ((CcInt*)args[4].addr)->GetValue();
               int loc = ((CcInt*)args[5].addr)->GetValue();
               int rem = ((CcInt*)args[6].addr)->GetValue();
               ListExpr tt = nl->Second(GetTupleResultType(s));
               local.addr = new ptransferLocal<L,R>(args[0], serv, loc, rem,tt);
               return 0;
          }
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



OperatorSpec ptransferFileSpec(
     " stream(tuple) x attrName x attrName x attrName -> stream(Tuple + Ext)",
     " _ ptransferFile[ ServerAttr, LocalFileAttr, RemoteFileAttr]  ",
     " Transfers local files to remote servers in parallel. The ServerAttr "
     " must point to an integer attribute specifying the server number. "
     " The localFileAttr and remoteFileAttr arguments mark the attribute name"
     " for the local and remote file name, respectively. Both arguments can be"
     " of type text or string", 
     " query fileRel feed ptransferFile[0, LocalFile, RemoteFile] consume");

/*
1.7.3 Value Mapping Array and Selection

*/

ValueMapping ptransferFileVM[] = {
  ptransferFileVMT<CcString, CcString>,
  ptransferFileVMT<CcString, FText>,
  ptransferFileVMT<FText, CcString>,
  ptransferFileVMT<FText, FText>
};

int ptransferFileSelect(ListExpr args){

  string name1 = nl->SymbolValue(nl->Third(args));
  string name2 = nl->SymbolValue(nl->Fourth(args));
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr type1, type2;
  listutils::findAttribute(attrList, name1, type1);
  listutils::findAttribute(attrList, name2, type2);
  int n1 = CcString::checkType(type1)?0:2;
  int n2 = CcString::checkType(type2)?0:1;


  return n1+n2; 
}

/*
1.6.4 Operator instance

*/

Operator ptransferFileOp (
    "ptransferFile",             //name
     ptransferFileSpec.getStr(),         //specification
     4,
     ptransferFileVM,        //value mapping
     ptransferFileSelect,   //trivial selection function
     ptransferFileTM        //type mapping
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
   AddOperator(&transferFileOp);
   AddOperator(&requestFileOp);
   AddOperator(&ptransferFileOp);
}


extern "C"
Algebra*
   InitializeDistributed2Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

   algInstance = new Distributed2Algebra();
   return algInstance;
}


