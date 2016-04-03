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


//[$][\$]

*/



#include "semaphore.h"

#include "fsrel.h"
#include "DArray.h"
#include "frel.h"
#include <iostream>
#include <vector>
#include <list>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "FileSystem.h"
#include "Algebra.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "NList.h"
#include "ArrayAlgebra.h"
#include "SocketIO.h"
#include "StopWatch.h"

#include "Bash.h"
#include "DebugWriter.h"


#include "FileRelations.h"
#include "FileAttribute.h"

#include "ConnectionInfo.h"


  // use boost for thread handling

#include <boost/thread.hpp>
#include <boost/date_time.hpp>


extern DebugWriter dwriter;

extern boost::mutex nlparsemtx;


using namespace std;
using namespace distributed2;

namespace distributed3 {


class Distributed3Algebra: public Algebra{

  public:

/*
1.1 Constructor defined at the end of this file

*/
     Distributed3Algebra();


/*
1.2 Destructor

*/
     ~Distributed3Algebra();

/*
~addConnection~

Adds a new connection to the connection pool.

*/
     int addConnection(ConnectionInfo* ci){
       boost::lock_guard<boost::mutex> guard(mtx);
       int res = connections.size();
       connections.push_back(ci);
       if(ci){
           ci->setId(connections.size());
       }
       return res; 
     }

/*
~noConnections~

Returns the number of connections

*/

     size_t noConnections(){
         boost::lock_guard<boost::mutex> guard(mtx);
         size_t res =  connections.size();
         return res;
     }
/*
~getConnections~

Returns a connection

*/

     ConnectionInfo* getConnection(const int i){
         boost::lock_guard<boost::mutex> guard(mtx);
         if(i<0 || ((size_t) i>=connections.size())){
           return 0;
         }
         ConnectionInfo* res =  connections[i];
         res->setId(i);;
         return res;
     }


/*
~noValidConnections~

Returns the number of non-null connections.

*/    

   size_t noValidConnections(){
     boost::lock_guard<boost::mutex> guard(mtx);
     size_t count = 0;
     for(size_t i=0;i<connections.size();i++){
       if(connections[i]) count++;
     }
     return count;
   }

/*
~disconnect~

Closes all connections and destroys the instances.
The return value is the number of closed connections.

*/
     int disconnect(){
        int count = 0;
        boost::lock_guard<boost::mutex> guard(mtx);
        for(size_t i=0;i<connections.size();i++){
          if(connections[i]){
             delete connections[i];
             count++;
          }
        }
        connections.clear();
        return count;
     }
     

/*
~disconnect~

Disconnects a specified connection and removes it from the
connection pool. The result is 0 or 1 depending whether the
given argument specifies an existing connection.

*/
     int disconnect( unsigned int position){
        boost::lock_guard<boost::mutex> guard(mtx);
        if( position >= connections.size()){
           return 0;
        }
        if(!connections[position]){
           return 0;
        }
        delete connections[position];
        connections[position] = 0;
        return 1;
     }

/*
~isValidServerNo~

checks whether an given integer points to a server

*/
   bool isValidServerNumber(int no){
      if(no < 0){
        return false;
      }
      boost::lock_guard<boost::mutex> guard(mtx);
      bool res = no < (int) connections.size();
      return res; 
   }

/*
~serverExists~

*/   
  bool serverExists(int s){
     return isValidServerNumber(s) && (connections[s]!=0);
  }

/*
~serverPid~

*/

   int serverPid(int s){
      boost::lock_guard<boost::mutex> guard(mtx);
 
      if(s < 0 || (size_t) s >= connections.size()){
          return 0;
      }
      ConnectionInfo* ci = connections[s];
      return ci?ci->serverPid():0;
   }



/*
~sendFile~

Transfers a local file to a remove server.

*/
    int sendFile( const int con, 
                       const string& local,
                       const string& remote,
                       const bool allowOverwrite){
      if(con < 0 ){
       return -3;
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
           return -3;
         }
         c = connections[con2];
      }
      if(!c){
         return -3;
      }
      return c->sendFile(local,remote, allowOverwrite);
    }
    

    int requestFile( const int con, 
                     const string& remote,
                     const string& local,
                     const bool allowOverwrite){
      if(con < 0 ){
       return -3;
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
            return -3;
         }
         c = connections[con2];
      }
      if(!c){
        return -3;
      }
      return c->requestFile(remote,local, allowOverwrite);
    }


    string getRequestFolder( int con){
      if(con < 0 ){
       return "";
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
           return "";
         }
         c = connections[con2];
      }
      if(!c){
         return "";
      }
      return c->getRequestFolder(); 
    }
    
    string getSendFolder( int con){
      if(con < 0 ){
       return "";
      }
      unsigned int con2 = con;
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con2 >= connections.size()){
           return "";
         }
         c = connections[con2];
      }
      if(!c){
        return "";
      }
      return c->getSendFolder(); 
    }

    string getHost(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            return "";
         }
         c = connections[con];
      }
      if(!c) return "";
      return c->getHost();
    }
    
    int getPort(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            return -3;
         }
         c = connections[con];
      }
      if(!c) return -3;
      return c->getPort();
    }
    
    string getConfig(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            return "";
         }
         c = connections[con];
      }
      if(!c) return "";
      return c->getConfig();
    }

    bool check(int con){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
             return false;
         }
         c = connections[con];
      }
      if(!c) return false;
      return c->check();
    }
    


    bool simpleCommand(int con, const string& cmd, int& error, 
                       string& errMsg, ListExpr& resList, const bool rewrite,
                       double& runtime){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            error = -3;
            errMsg = "invalid connection number";
            resList = nl->TheEmptyList();
            runtime = 0;
            return false;
         }
         c = connections[con];
      }
      if(!c) {
         error = -3;
         errMsg = "invalid connection number";
         resList = nl->TheEmptyList();
         runtime = 0;
         return false;
      }
      c->simpleCommand(cmd,error,errMsg,resList, rewrite, runtime);
      return true;
    }
    

    bool simpleCommand(int con, const string& cmd, int& error, 
                       string& errMsg, string& resList, const bool rewrite,
                       double& runtime){
      ConnectionInfo* c;
      {
         boost::lock_guard<boost::mutex> guard(mtx);
         if(con < 0 || con >= (int) connections.size()){
            error = -3;
            errMsg = "invalid connection number";
            resList = "()";
            runtime = 0;
           return false;
         }
         c = connections[con];
      }
      if(!c) {
         error = -3;
         errMsg = "invalid connection number";
         resList = "()";
         runtime = 0;
         return false;
      }
      c->simpleCommand(cmd,error,errMsg,resList, rewrite, runtime);
      return true;
    }


/*
The next functions are for interacting with workers, i.e.
connections coming from darray elements.

*/
  ConnectionInfo* getWorkerConnection( const DArrayElement& info,
                                       const string& dbname ){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string, ConnectionInfo*> >::iterator it;
     it = workerconnections.find(info);
     pair<string,ConnectionInfo*> pr("",0);
     if(it==workerconnections.end()){
        if(!createWorkerConnection(info,pr)){
             return 0;
        }
        workerconnections[info] = pr;
        it = workerconnections.find(info);
     } else {
         pr = it->second;
     }
     string wdbname = dbname;
     if(pr.first!=wdbname){
         if(!pr.second->switchDatabase(wdbname,true)){
            it->second.first="";
            return 0;
         } else {
             it->second.first=dbname;
         }
     }
     return pr.second;
  }


  string getDBName(const DArrayElement& info){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string, ConnectionInfo*> >::iterator it;
     it = workerconnections.find(info);
     string db = "";
     if(it!=workerconnections.end()){
        db=it->second.first;         
     }
     return db;
  }


/*
This operator closes all non user defined existing server connections.
It returns the numer of closed workers

*/
 int closeAllWorkers(){
    boost::lock_guard<boost::mutex> guard(workerMtx);
    map<DArrayElement, pair<string,ConnectionInfo*> > ::iterator it;
    int count = 0;
    for(it=workerconnections.begin(); it!=workerconnections.end(); it++){
       ConnectionInfo* ci = it->second.second;
       delete ci;
       count++;
    }
    workerconnections.clear();
    return count;
 } 

/*
The operator ~closeWorker~ closes the connections for a 
specified DArrayElement.

*/ 
  bool closeWorker(const DArrayElement& elem){
    boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     it = workerconnections.find(elem);
     if(it==workerconnections.end()){
        return false;
     }
     ConnectionInfo* info = it->second.second;
     if(info){
        delete info;
        it->second.second = 0;
     }
     workerconnections.erase(it);
     return true;
  }

  bool workerConnection(const DArrayElement& elem, string& dbname,
                         ConnectionInfo*& ci){
    boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     it = workerconnections.find(elem);
     if(it == workerconnections.end()){
        return false;
     } 
     dbname = it->second.first;
     ci = it->second.second;
     return true;
  }

  map<DArrayElement, pair<string,ConnectionInfo*> >::iterator
  workersIterator(){
    return workerconnections.begin();
  }

  bool isEnd( map<DArrayElement, pair<string,ConnectionInfo*> >::iterator& it){
    boost::lock_guard<boost::mutex> guard(workerMtx);
    return it==workerconnections.end();
  } 


  string getTempName(int server){
    boost::lock_guard<boost::mutex> guard(mtx);
    if(server < 0 || (size_t)server<connections.size()){
       return "";
    }
    ConnectionInfo* ci = connections[server];
    if(!ci){
       return "";
    }
    stringstream ss;
    ss << "TMP_" << WinUnix::getpid() 
       << "_" << ci->serverPid() 
       << "_" << nextNameNumber();
    return ss.str();
  } 

  string getTempName(const DArrayElement& elem){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     it = workerconnections.find(elem);
     if(it==workerconnections.end()){
        return "";
     }
     stringstream ss;
     ss << "TMP_" << WinUnix::getpid() 
        << "_" << it->second.second->serverPid() 
        << "_" << nextNameNumber();
    return ss.str();
  }

  string getTempName(){
     boost::lock_guard<boost::mutex> guard1(workerMtx);
     boost::lock_guard<boost::mutex> guard2(mtx);
     ConnectionInfo* ci=0;
     for(size_t i=0;i<connections.size() && !ci; i++){
         ci = connections[i];
     }
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     for(it = workerconnections.begin(); 
        it !=workerconnections.end() && !ci; 
        it++){
         ci = it->second.second;
     }
     int spid;
     size_t sh;
     if(ci){
        sh = stringutils::hashCode(ci->getHost());
        spid = ci->serverPid();
     } else {
        srand(time(0));
        spid = rand();
        sh = rand();
     }
     stringstream ss;
     ss << "TMP_" << WinUnix::getpid() 
        << "_" << sh 
        << "_" << spid 
        << "_" << nextNameNumber();
    return ss.str();
  }


  void cleanUp(){
    
     set<pair<string,int> > used;
     vector<ConnectionInfo*> unique;

     for(size_t i=0;i<connections.size();i++){
         ConnectionInfo* ci = connections[i];
         if(ci){
             pair<string,int> p(ci->getHost(), ci->getPort());
             if(used.find(p)==used.end()){
                  unique.push_back(ci);
                  used.insert(p);
             }
         }
     }
     map<DArrayElement, pair<string,ConnectionInfo*> >::iterator it;
     for(it = workerconnections.begin();it!=workerconnections.end();it++){
        ConnectionInfo* ci = it->second.second;
        if(ci){
            pair<string,int> p(ci->getHost(), ci->getPort());
            if(used.find(p)==used.end()){
                 unique.push_back(ci);
                 used.insert(p);
            }
        }
     }

     vector<boost::thread*> runners;
     for(size_t i=0;i<unique.size();i++){
        ConnectionInfo* ci = unique[i];
        boost::thread* r = new boost::thread(&ConnectionInfo::cleanUp, ci);
        runners.push_back(r);
     }
     for(size_t i=0;i<runners.size();i++){
        runners[i]->join();
        delete runners[i];
     }
  }


  private:
    // connections managed by the user
    vector<ConnectionInfo*> connections;
    boost::mutex mtx;

    // connections managed automatically 
    // for darray type
    // the key represents the connection information,
    // the string the used database
    // the ConnctionInfo the connection
    map<DArrayElement, pair<string,ConnectionInfo*> > workerconnections;
    boost::mutex workerMtx;

    size_t namecounter;
    boost::mutex namecounteraccess;



   size_t nextNameNumber(){
      boost::lock_guard<boost::mutex> guard(namecounteraccess);
      namecounter++;
      return namecounter;
   } 

    bool createWorkerConnection(const DArrayElement& worker, pair<string, 
                                ConnectionInfo*>& res){
      string host = worker.getHost();
      int port = worker.getPort();
      string config = worker.getConfig();
      ConnectionInfo* ci = ConnectionInfo::createConnection(host, port, config);
      res.first="";
      res.second = ci;
      return ci!=0;
    }
};

Distributed3Algebra* algInstance;


/*
2.1.1 Class ~taskElement~

This class represents the Elements of Code.

*/
class TaskElement{
 public:
    TaskElement( Word* args, Word& result, Supplier s): pos(0){
//      tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      a1Name = ((DArray*) args[0].addr)->getName();
      part1 = 0;
      worker1 = 0;
      a2Name = ((DArray*) args[1].addr)->getName();
      part2 = 0;
      worker2 = 0;
      funQuery = ((CcString*) args[4].addr)->GetValue();
      max = min(((DArray*) args[0].addr)->getSize(), 
                ((DArray*) args[1].addr)->getSize());

      resName = ((DArray*) result.addr)->getName();
      resPart = 0;
      resWorker = 0;
    }

    TaskElement( size_t _pos, 
                 string _a1Name,size_t  _part1,size_t  _worker1,
                 string _a2Name,size_t  _part2,size_t  _worker2,
                 string _funQuery,
                 string _resArrayName,size_t  _resPart,size_t  _resWorker){
      pos =_pos;
      a1Name = _a1Name;
      part1 = _part1;
      worker1 = _worker1;
      a2Name = _a2Name;
      part2 = _part2;
      worker2 = _worker2;
      funQuery = _funQuery; 
      resName = _resArrayName;
      resPart = _resPart;
      resWorker = _resWorker;
    }
    
    
    TaskElement(ListExpr list){
      if(!nl->HasLength(list,12)){
        return;
      }
      ListExpr e1 = nl->First(list);
      pos = nl->IntValue(e1);
      list = nl->Rest(list);

      ListExpr e2 = nl->First(list);
      a1Name = nl->StringValue(e2);
      list = nl->Rest(list);

      ListExpr e3 = nl->First(list);
      part1 = nl->IntValue(e3);
      list = nl->Rest(list);

      ListExpr e4 = nl->First(list);
      worker1 = nl->IntValue(e4);
      list = nl->Rest(list);

      ListExpr e5 = nl->First(list);
      a2Name = nl->StringValue(e5);
      list = nl->Rest(list);
      
      ListExpr e6 = nl->First(list);
      part2 = nl->IntValue(e6);
      list = nl->Rest(list);

      ListExpr e7 = nl->First(list);
      worker2 = nl->IntValue(e7);
      list = nl->Rest(list);

      ListExpr e8 = nl->First(list);
      funQuery = nl->StringValue(e8);
      list = nl->Rest(list);

      ListExpr e9 = nl->First(list);
      resName = nl->StringValue(e9);
      list = nl->Rest(list);

      ListExpr e10 = nl->First(list);
      resPart = nl->IntValue(e10);
      list = nl->Rest(list);

      ListExpr e11 = nl->First(list);
      resWorker = nl->IntValue(e11);
      list = nl->Rest(list);
    }

    
    ListExpr toListExpr(){
      ListExpr expr1 = nl->SixElemList(
        nl->IntAtom(part2),
        nl->IntAtom(worker2),
        nl->TextAtom(funQuery),
        nl->StringAtom(resName),
        nl->IntAtom(resPart),
        nl->IntAtom(resWorker)
      );
      
      ListExpr expr2 = nl->Cons(nl->StringAtom(a2Name),expr1);
      ListExpr expr3 = nl->Cons(nl->IntAtom(worker1),expr2);
      ListExpr expr4 = nl->Cons(nl->IntAtom(part1),expr3);
      ListExpr expr5 = nl->Cons(nl->StringAtom(a1Name),expr4);
      ListExpr expr = nl->Cons(nl->IntAtom(pos),expr5);
      return expr;
    }


    ~TaskElement(){
    }
    
    Tuple* getTuple(TupleType* tt){
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new CcInt(true,pos));
      res->PutAttribute(1, new CcString(true,a1Name));
      res->PutAttribute(2, new CcInt(true,part1));
      res->PutAttribute(3, new CcInt(true,worker1));
      res->PutAttribute(4, new CcString(true,a2Name));
      res->PutAttribute(5, new CcInt(true,part2));
      res->PutAttribute(6, new CcInt(true,worker2));
      res->PutAttribute(7, new CcString(true,funQuery));
      res->PutAttribute(8, new CcString(true,resName));
      res->PutAttribute(9, new CcInt(true,resPart));
      res->PutAttribute(10, new CcInt(true,resWorker));
      return res;
    }

    bool readFrom(SmiRecord& valueRecord, size_t& offset){
        if(!readVar(pos,valueRecord,offset)){
            return false;
        }
        if(!readVar(a1Name,valueRecord,offset)){
           return false;
        }
        if(!readVar(part1,valueRecord,offset)){
           return false;
        }
        if(!readVar(worker1,valueRecord,offset)){
           return false;
        }
        if(!readVar(a2Name,valueRecord,offset)){
           return false;
        }
        if(!readVar(part2,valueRecord,offset)){
           return false;
        }
        if(!readVar(worker2,valueRecord,offset)){
           return false;
        }
        if(!readVar(funQuery,valueRecord,offset)){
           return false;
        }
        if(!readVar(resName,valueRecord,offset)){
           return false;
        }
        if(!readVar(resPart,valueRecord,offset)){
           return false;
        }
        if(!readVar(resWorker,valueRecord,offset)){
           return false;
        }
        return true;
     }


    bool saveTo(SmiRecord& valueRecord, size_t& offset){
        if(!writeVar(pos,valueRecord,offset)){
            return false;
        }
        if(!writeVar(a1Name,valueRecord,offset)){
           return false;
        }
        if(!writeVar(part1,valueRecord,offset)){
           return false;
        }
        if(!writeVar(worker1,valueRecord,offset)){
           return false;
        }
        if(!writeVar(a2Name,valueRecord,offset)){
           return false;
        }
        if(!writeVar(part2,valueRecord,offset)){
           return false;
        }
        if(!writeVar(worker2,valueRecord,offset)){
           return false;
        }
        if(!writeVar(funQuery,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resName,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resPart,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resWorker,valueRecord,offset)){
           return false;
        }
         return true; 
     }
     
    void print(ostream& out)const{
      out << "( Task: " << pos  
        << ", Arg1 : " << a1Name << ", Part1 : " << part1 
        << ", Worker1 : " << worker1  
        << ", Arg2 : " << a2Name << ", Part2 : " << part2
        << ", Worker2 : " << worker2
        << ", Query : " << funQuery 
        << ", Res : " << resName << ", ResPart : " << resPart 
        << ", ResWorker : " << resWorker
        << ")" << endl;
    }

    size_t GetPos(){
      return pos;
    }
    
    string GetA1Name(){
      return a1Name;
    }
    
    string GetA2Name(){
      return a2Name;
    }
    
    size_t  GetPart1(){
      return part1;
    }
    
    size_t  GetPart2(){
      return part2;
    }
    
    size_t  GetWorker1(){
      return worker1;
    }
    
    size_t  GetWorker2(){
      return worker2;
    }

    string GetFunQuery(){
      return funQuery;
    }
    
    string GetResName(){
      return resName;
    }
    
    size_t GetResPart(){
      return resPart;
    }
    
    size_t GetResWorker(){
      return resWorker;
    }
    
    void SetResWorker(size_t value){
      resWorker = value;
    }
     
     
 private:
    size_t  max;
    TupleType* tt;
    
    size_t pos;
    string a1Name;
    size_t worker2;
    size_t part2;
    string a2Name;
    size_t part1;
    size_t worker1;
    string funQuery;
    string resName;
    size_t resPart;
    size_t resWorker;

};



/*
2.1.2 Class ~Code~

This class represents the Secondo type ~code~. It just stores the list of tasks.

*/

class Code{
  public:
     Code(const vector<TaskElement*>& _tasks,
          const vector<DArrayElement*>& _worker){
        tasks = _tasks;
        worker = _worker;
        defined = true;        
     }

     explicit Code() {} // only for cast function


     Code& operator=(const Code& src) {
        this->tasks = src.tasks;
        this->worker = src.worker;
        this->defined = src.defined;
        this->size = src.size;
        this->resultType = src.resultType;
        return *this;
     }     
 
     ~Code() {
         size_t s = tasks.size();
         for(size_t i=0;i<s;i++){
           delete tasks[i];
         }
         size_t w = worker.size();
         for(size_t j=0;j<w;j++){
           delete worker[j];
         }
         tasks.clear();
         worker.clear();
     }

    void set(const vector<TaskElement*>& tasks){
        if(tasks.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        defined = true;
        this->tasks = tasks;
     }

    void add(const vector<TaskElement*> tasks){
        if(tasks.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        for(size_t i=0;i<tasks.size();i++){
          this->tasks.push_back(tasks[i]);
        }
     }
     
     size_t AddIfNotContains(const DArrayElement& element){
       size_t size = worker.size();
        for(size_t i=0;i<size;i++){
          if(worker[i]->getHost() == element.getHost() &&
            worker[i]->getPort() == element.getPort() &&
            worker[i]->getConfig() == element.getConfig()){
            return i;
          }
        }
        worker.push_back(new DArrayElement(element));
        return size;
     }

     bool IsDefined(){
        return defined;
     }

     static const string BasicType() { 
       return "code";
     }

     static const bool checkType(const ListExpr list){
       if(!nl->HasLength(list,2)){
         return false;
       }  
       if(!listutils::isSymbol(nl->First(list), BasicType())){
           return false;
       }
       return true;
     }

     size_t sizeOfTasks() const{
       return tasks.size();
     }
     
     size_t getSize() const{
       return size;
     }
     
     void setSize(size_t value){
       size = value;
     }
 
     string getResultType() const{
       return resultType;
     }
 
     void setResultType(string value){
       resultType = value;
     }
 
     TaskElement* getTask(size_t i){
        if(i<0 || i>= tasks.size()){
           assert(false);
        }
        return tasks[i];
     } 

     ListExpr toListExpr(){
       if(!defined){
         return listutils::getUndefined();
       }

       ListExpr tl;
       if(tasks.empty()){
         tl =  nl->TheEmptyList();
       } else {
         int i = tasks.size() - 1;
         tl = nl->OneElemList(tasks[i]->toListExpr());
         while(i > 0){
           i--; 
           tl = nl->Cons(tasks[i]->toListExpr(), tl);
         }
       }
       
       ListExpr wl;
       if(worker.empty()){
         wl =  nl->TheEmptyList();
       } else {
         int j = worker.size() - 1;
         wl = nl->OneElemList(worker[j]->toListExpr());
         while(j > 0){
           j--; 
           wl = nl->Cons(worker[j]->toListExpr(), wl);
         }
       }

       ListExpr listExpr = nl->FiveElemList(nl->SymbolAtom( BasicType()), 
                                nl->StringAtom(resultType), 
                                nl->IntAtom(size),             
                                tl,
                                wl); 
       return listExpr; 
     }


     static Code* readFrom(ListExpr list){
        if(listutils::isSymbolUndefined(list)){
           vector<TaskElement*> tl;
           vector<DArrayElement*> wl;
           return new Code(tl,wl);
        }
        if(!nl->HasLength(list,5)){
           return 0;
        }
        ListExpr Tasks = nl->Fourth(list);
        ListExpr Worker = nl->Fifth(list);
        string rt = nl->StringValue(nl->Second(list));
        int size = nl->Third(list);
        
        vector<TaskElement*> tl;
        while(!nl->IsEmpty(Tasks)){
           TaskElement* elem = new TaskElement(nl->First(Tasks));
           tl.push_back(elem);
           Tasks = nl->Rest(Tasks);
        }
        vector<DArrayElement*> wl;
        string server;     
        int port;
        string config;
        while(!nl->IsEmpty(Worker)){
           server = nl->StringValue(nl->First(nl->First(Worker)));     
           port = nl->IntValue(nl->Second(nl->First(Worker)));
           config = nl->StringValue(nl->Third(nl->First(Worker)));
           DArrayElement* elem = new DArrayElement(server,port,-1,config);
           wl.push_back(elem);
           Worker = nl->Rest(Worker);
        }
        Code* result = new Code(tl,wl);
        result->defined = true;
        result->setResultType(rt);
        result->size = size;
        return result;
     }
     
     static bool open(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo, Word& result){
        bool defined;
        result.addr = 0;
        if(!readVar<bool>(defined,valueRecord,offset)){
           return false;
        } 
        if(!defined){
          vector<TaskElement*> tasks;
          vector<DArrayElement*> worker;
          result.addr = new Code(tasks,worker);
          return true;
        }
        // object in smirecord is defined, read resultType
        string t;
        if(!readVar<string>(t,valueRecord,offset)){
           return false;
        }

        // read  vector
        vector<TaskElement*> tasks;
        vector<DArrayElement*> worker;
        Code* res= new Code(tasks,worker);
        res->setResultType(t);

        //size
        size_t si;
        if(!readVar<size_t>(si,valueRecord,offset)){
           return false;
        }
        res->setSize(si);
        
        // append tasks
        size_t s;
        if(!readVar<size_t>(s,valueRecord,offset)){
           return false;
        }
        for(size_t i=0; i< s; i++){
           TaskElement*  task = new TaskElement(s);
           if(!task->readFrom(valueRecord, offset)){
               delete res;
               return false;
           }
           res->tasks.push_back(task);
        }
        
        // append worker
        size_t w;
        if(!readVar<size_t>(w,valueRecord,offset)){
           return false;
        }
        for(size_t i=0; i< w; i++){
           DArrayElement*  element = new DArrayElement("",0,0,"");
           if(!element->readFrom(valueRecord, offset)){
               delete res;
               return false;
           }
           res->worker.push_back(element);
        }
        
        res->defined = true;
        result.addr = res;
        return true;
     }

     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
       
         Code* c = (Code*) value.addr;
         // defined flag
         if(!writeVar(c->defined,valueRecord,offset)){
           return false;
         }
         if(!c->defined){
            return true;
         }
         // resultType
         string t = c->getResultType();
         if(!writeVar(t,valueRecord,offset)){
           return false;
         }
         // size
         size_t s = c->getSize();
         if(!writeVar(s,valueRecord,offset)){
           return false;
         }
         
         // tasks
         size_t st;
         st = (size_t)c->tasks.size();
         if(!writeVar(st,valueRecord,offset)){
           return false;
         }
         for(size_t i=0;i<st;i++){
           if(!c->tasks[i]->saveTo(valueRecord,offset)){
             return false;
           }
         }
         
         // worker
         size_t sw;
         sw = (size_t)c->worker.size();
         if(!writeVar(sw,valueRecord,offset)){
           return false;
         }
         for(size_t i=0;i<sw;i++){
           if(!c->worker[i]->saveTo(valueRecord,offset)){
             return false;
           }
         }
         return true; 
     }


     void print(ostream& out){
       if(!defined){
          out << "undefined";
          return;
       }

       out << ", size : " << size
           << ", type : " << resultType
           << " tasks : [" ;
       for(size_t i =0;i<tasks.size();i++){
          if(i>0) out << ", ";
          tasks[i]->print(out);
       }
       out << "] worker : [" ;
       for(size_t i =0;i<worker.size();i++){
          if(i>0) out << ", ";
          worker[i]->print(out);
       }
       out << "]";
     }

     void makeUndefined(){
        tasks.clear();
        defined = false;
     }
     
     size_t GetStartIndexOfLastResult(){
      size_t lastIndex = tasks.size() - 1;
      TaskElement* lastElement = tasks[lastIndex];
       for(size_t i =lastIndex - 1; i+1 != 0;i--){
          if(tasks[i]->GetResName() == lastElement->GetA1Name() ||
            tasks[i]->GetResName() == lastElement->GetA2Name()){
            return i + 1;
          }
       }
       return 0;
     }
     
     void BeginIteration(){
       iter = 0;
     }

     TaskElement* GetNext(){
       if(iter < tasks.size()){
         return tasks[iter++];
       }
       return 0;
     }

     size_t GetIndexWithArrayAndPartitionAsResult(
       string arrayName, size_t part){
       size_t st = (size_t)tasks.size();
       for(size_t i=0;i<st;i++){
         if(tasks[i]->GetResName() == arrayName && 
           tasks[i]->GetResPart() == part){
           return tasks[i]->GetPos();
         }
       }
       return 0;
     }
     
    void SetDependency(int index, int dependsOn){
      if(taskDependenceOn[index] == 0){
        taskDependenceOn[index] = new std::set<int>();
      }
      if(taskNecessaryFor[dependsOn] == 0){
        taskNecessaryFor[dependsOn] = new std::set<int>();
      }
      taskDependenceOn[index]->insert(dependsOn); 
      taskNecessaryFor[dependsOn]->insert(index); 
    }
     
    void ClearDependency(int index){
      if(taskNecessaryFor[index] == 0){
        return;
      }
      std::set<int>::iterator it;
      for (it=taskNecessaryFor[index]->begin(); 
           it!=taskNecessaryFor[index]->end(); ++it){
        taskDependenceOn[*it]->erase(index);  
      }
      if(taskDependenceOn[index] != 0){
        delete taskDependenceOn[index];
        taskDependenceOn[index] = 0;
      }
     }
    
    void BuildDependenciesVector(){

      size_t st = (size_t)tasks.size();
      taskDependenceOn.resize(st+1,0);
      taskNecessaryFor.resize(st+1,0);
      size_t dep1, dep2;
      for(size_t i=0;i<st;i++){
        dep1 = GetIndexWithArrayAndPartitionAsResult(
          tasks[i]->GetA1Name(),tasks[i]->GetPart1());
        dep2 = GetIndexWithArrayAndPartitionAsResult(
          tasks[i]->GetA2Name(),tasks[i]->GetPart2());
        if(dep1 > 0){
          SetDependency(tasks[i]->GetPos(),dep1);
        }
        if(dep2 > 0){
          SetDependency(tasks[i]->GetPos(),dep2);
        }
        if((dep1 == 0) && (dep1 == 0)){ 
          taskDependenceOn[tasks[i]->GetPos()] = new std::set<int>();
        } 
      }
    }
    
    TaskElement* GetNextTask(){
      size_t st = (size_t)tasks.size();
      for(size_t i=1;i<st+1;i++){
        if(taskDependenceOn[i] != 0 && taskDependenceOn[i]->size() == 0){
          SetDependency(tasks[i-1]->GetPos(), tasks[i-1]->GetPos());
          return tasks[i-1];
        }
      }
      return 0;
    }
    
    
    TaskElement* GetNextTask(size_t worker){
      size_t st = (size_t)tasks.size();
      for(size_t i=1;i<st+1;i++){
        if(taskDependenceOn[i] != 0 && taskDependenceOn[i]->size() == 0){
          if((tasks[i-1]->GetWorker1() == worker) && 
            (tasks[i-1]->GetWorker2() == worker &&
            tasks[i-1]->GetA2Name() == "")){
            SetDependency(tasks[i-1]->GetPos(), tasks[i-1]->GetPos());
            return tasks[i-1];
          }
        }
      }
      for(size_t i=1;i<st+1;i++){
        if(taskDependenceOn[i] != 0 && taskDependenceOn[i]->size() == 0){
          if((tasks[i-1]->GetWorker1() == worker) || 
            (tasks[i-1]->GetWorker2() == worker &&
            tasks[i-1]->GetA2Name() != "")){
            SetDependency(tasks[i-1]->GetPos(), tasks[i-1]->GetPos());
            return tasks[i-1];
          }
        }
      }
      return GetNextTask();
    }

    int mapIndex(const DArrayElement& worker){
      return AddIfNotContains(worker);
    }
     
    DArrayElement* GetWorker(size_t i){
      if(i < 0 || i > worker.size()){
        return 0;
      }
      return worker[i];
    }
    
    string GetResultName(){
      string returnString = "";
      if(tasks.size() > 0){
        return tasks[tasks.size() - 1]->GetResName();
      }
      return "";
    }

    size_t GetSizeOfWorker(){
      return worker.size();
    }    

    bool Done(){
      size_t st = (size_t)tasks.size();
      for(size_t i=1;i<st+1;i++){
        if(taskDependenceOn[i] != 0){
          return false;
        }
      }
      return true;
    }

  private:
    std::vector<TaskElement*> tasks; // the Tasks information
    std::vector<DArrayElement*> worker; // connection information
    bool defined; // defined state of this Code Element
    size_t  iter; // the actual index for an interatition
    string resultType; // the Type of the last Returnvalue in the tasks.
    size_t size; // size of the result array.
    std::vector<std::set<int> * >taskDependenceOn;
    std::vector<std::set<int> * >taskNecessaryFor;
};


/*
2.1.2.1 Property function

*/
ListExpr CodeProperty(){
   return nl->TwoElemList(
            nl->FourElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"), 
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List")),
            nl->FourElemList(
                 nl->StringAtom(" -> SIMPLE"),
                 nl->StringAtom(" (code(darray <basictype>))"),
                 nl->TextAtom("(size ( t1 t2  ...)) where "
                     "t_i =(pos dep arg1 arg2 part1 part2 query res resPart)"),
                 nl->TextAtom("( 1(1 0 'R' 'S' 0 0 " 
                     "'.feed{r} ..feed{s} hashjoin[R_a, S_b] count' 'T' 0)")));
}

/*
2.1.2.2 IN function

*/

Word InCode(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct){

   Word res((void*)0);
   res.addr = Code::readFrom(instance);
   correct = res.addr!=0;
   return res;
}

/*

2.1.2.3 Out function

*/
ListExpr OutCode(ListExpr typeInfo, Word value){
   Code* c = (Code*) value.addr;
   return c->toListExpr();
}

/*

2.1.2.4 Create function

*/
Word CreateCode(const ListExpr typeInfo){
  Word w;
  vector<TaskElement*> t;
  vector<DArrayElement*> l;
  Code* c = new Code(t,l);
  w.addr = c;
  return w;
}

/*

2.1.2.4 Delete function

*/
void DeleteCode(const ListExpr typeInfo, Word& w){
  Code* c = (Code*) w.addr;
  delete c;
  w.addr = 0;
}

/*

2.1.2.4 Close function

*/
void CloseCode(const ListExpr typeInfo, Word& w){
  Code* c = (Code*) w.addr;
  delete c;
  w.addr = 0;
}

/*

2.1.2.4 Clone function

*/
Word CloneCode(const ListExpr typeInfo, const Word& w){
    Code* c = (Code*) w.addr;
    Word res;
    res.addr = new Code(*c);
    return res;
}

void* CastCode(void* addr){
   const vector<TaskElement*> ts;
   const vector<DArrayElement*> ws;
   return (new (addr) Code(ts,ws));   
}

bool CodeTypeCheck(ListExpr type, ListExpr& errorInfo){
    return Code::checkType(type);
}


int SizeOfCode(){
  return 42; // a magic number
}


TypeConstructor CodeTC(
  Code::BasicType(),
  CodeProperty,
  OutCode, InCode,
  0,0,
  CreateCode, DeleteCode,
  Code::open, Code::save,
  CloseCode, CloneCode,
  CastCode,
  SizeOfCode,
  CodeTypeCheck );


/*
1.15 Operator ~dloopS~

This operator transforms a function that will be performed 
over all entries of a DArray into a Code object.

1.15.1 Type Mapping

The signature is
darray(X) x string x (X->Y) -> code(darray(Y))
or
Code(darray(X)) x string x (X->Y) -> code(darray(Y))

*/

ListExpr dloopSTM(ListExpr args){
   string err ="darray(X) x string x fun: X -> Y   or "
       "code(X) x string x fun: X -> Y   expected";

  if(!nl->HasLength(args,3) ){
    return listutils::typeError(err + "(wrong number of args)");
  }
  
  ListExpr temp = args;
  while(!nl->IsEmpty(temp)){
    if(!nl->HasLength(nl->First(temp),2)){
        return listutils::typeError("internal Error");
     }
     temp = nl->Rest(temp);
  }

  ListExpr darray = nl->First(args);
  ListExpr fun;
  
  if(!CcString::checkType(nl->First(nl->Second(args)))){
     return listutils::typeError("Second arg not of type string");
  }
  
  fun = nl->Third(args);
  ListExpr funType = nl->First(fun);
  ListExpr arrayType = nl->First(darray);

  if(!DArray::checkType(arrayType) && !Code::checkType(arrayType)){
    return listutils::typeError(err + ": first arg not a darray or code");
  }

  if(!listutils::isMap<1>(funType)){
    return listutils::typeError(err + ": last arg is not a function");
  }
  if(DArray::checkType(arrayType)){
    if(!nl->Equal(nl->Second(arrayType), nl->Second(funType))){
      return listutils::typeError("type mismatch between darray and "
                                  "function arg");
    }
  }

  
   // replace eventually type mapping operators by the
   // resulting types
   ListExpr funQuery = nl->Second(nl->Third(args));
   ListExpr fa1o = nl->Second(funQuery);
   ListExpr fa1 = nl->TwoElemList(
                       nl->First(fa1o),
                       nl->Second(nl->First(nl->First(args))));
   funQuery = nl->ThreeElemList(
                   nl->First(funQuery),
                   fa1, nl->Third(funQuery));
   string rfun = nl->ToString(funQuery);   
   cout << "rfun in dloop2TM: " << rfun << endl;

  ListExpr resultType = nl->Third(funType); 
  ListExpr result = nl->TwoElemList(listutils::basicSymbol<Code>(),
                                    resultType);
  
   if(DArray::checkType(nl->First(nl->First(args))) 
    && CcString::checkType(nl->First(nl->Second(args)))
    && listutils::isMap<1>(nl->First(nl->Third(args)))){
 
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(rfun)),
               result);
  }

  if(Code::checkType(nl->First(nl->First(args))) 
    && CcString::checkType(nl->First(nl->Second(args)))
    && listutils::isMap<1>(nl->First(nl->Third(args)))){
    
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(rfun)),
               result);
  }
  return listutils::typeError(err);
}

/*
1.15.2 Selection Function

*/
int loopSSelect( ListExpr args )
{
  if(DArray::checkType(nl->First(args))){
    return 0;
  }
  return 1;
}

/*
1.15.3 Value Mapping Function

*/


int dloopSVM_Array(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   DArray* array = (DArray*) args[0].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   
   if(! array->IsDefined()){
      res->makeUndefined();
      return 0;
   }  
   string a1Name = array->getName();
   size_t  worker1 = 0;
   size_t as = array->getSize();
   res->setSize(as);
   string a2Name = "";
   size_t  part2 = 0;
   size_t  worker2 = 0;

   string resName;
   CcString* name = (CcString*) args[1].addr;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }

   string fun = ((FText*) args[3].addr)->GetValue();
   vector<TaskElement*> tasks;
   int pos = 0;
   TaskElement* element;
   for(size_t i=0; i < array->getSize(); i++){
      pos++;
      worker1 = res->mapIndex(array->getWorkerForSlot(i));
      element = new TaskElement(pos,
                                a1Name,i,worker1,
                                a2Name,part2,worker2,
                                fun,
                                resName,i,0);
      tasks.push_back(element);
   }
      cout << "Wir waren da!"  << endl;
   res->set(tasks);
   return 0; 
}

int dloopSVM_Code(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   Code* code = (Code*) args[0].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;

   if(!code->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   size_t as = code->getSize();
   res->setSize(as);
   
   CcString* name = (CcString*) args[1].addr;
   string resName;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }
   if(!stringutils::isIdent(resName)){
    res->makeUndefined();
    return 0;
   }
   
   string fun = ((FText*) args[3].addr)->GetValue();
    
   vector<TaskElement*> tasks;
   TaskElement* e;
   TaskElement* element;

   size_t size = code->sizeOfTasks();
   for(size_t i = 0; i < size; i++){
     e = code->getTask(i);
     DArrayElement* worker1 = code->GetWorker(e->GetWorker1());
     res->AddIfNotContains(*worker1);
     DArrayElement* worker2 = code->GetWorker(e->GetWorker2()); 
     res->AddIfNotContains(*worker2);
     element = new TaskElement( e->GetPos(),
                   e->GetA1Name(),e->GetPart1(),e->GetWorker1(),
                   e->GetA2Name(),e->GetPart2(),e->GetWorker2(),
                   e->GetFunQuery(),
                   e->GetResName(),e->GetResPart(),e->GetResWorker());
     tasks.push_back(element);
   }

   string a2Name = "";
   size_t  part2 = 0;
   size_t  worker2 = 0;

   size_t pos = size;
   for(size_t i = code->GetStartIndexOfLastResult(); i < size;i++){
     pos++;
     e = code->getTask(i);
     element = new TaskElement( pos,
                  e->GetResName(),e->GetResPart(),e->GetResWorker(),
                  a2Name,part2,worker2,
                  e->GetFunQuery(),
                  resName,e->GetResPart(),0);
     tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

ValueMapping dloopSVM[] = {
  dloopSVM_Array,
  dloopSVM_Code
  
};

/*
1.15.4 Specification

*/
OperatorSpec dloopSSpec(
     " {darray(X), code(darray(X))} x string x  (X->Y) -> code(darray(y))",
     " _ dloopS[_,_]",
     "Generates a Code Object that performs a function "
     " on each element of a darray instance."
     "The string argument specifies the name of the result. If the name"
     " is undefined or an empty string, a name is generated automatically.",
     "query da2 dloopS[\"da3\", . toTasks"
     );

/*
1.15.5 Operator instance

*/
Operator dloopSOp(
  "dloopS",
  dloopSSpec.getStr(),
  2,
  dloopSVM,
  loopSSelect,
  dloopSTM
);

/*
1.15 Operator ~dloop2S~

This operator performs a function 
on each element of two darray instances, of one darray and one code 
instance or two code instances.

1.15.1 Type Mapping

The signature is
darray(X) x darray(Y) x string x (X,Y->Z) -> code(darray(Z))
or
darray(X) x Code(darray(Y)) x string x (X,Y->Z) -> code(darray(Z))
or
Code(darray(X)) x darray(Y) x string x (X,Y->Z) -> code(darray(Z))
or
Code(darray(X))) x Code(darray(Y)) x string x (X,Y->Z) -> code(darray(Z))

*/

ListExpr dloop2STM(ListExpr args){
   string err ="darray(X) x darray(Y) x string x fun: X,Y -> Z   or "
       "code(X) x darray(Y)  x string x fun: X -> Y or "   
       "darray(X) x code(Y)  x string x fun: X -> Y or "   
       "code(X) x code(Y)  x string x fun: X -> Y expected";   
       
 cout << "ARGS: " << nl->ToString(args) << endl;       

  if(!nl->HasLength(args,4) ){
    return listutils::typeError(err + "(wrong number of args)");
  }
  
  ListExpr temp = args;
  while(!nl->IsEmpty(temp)){
    if(!nl->HasLength(nl->First(temp),2)){
        return listutils::typeError("internal Error");
     }
     temp = nl->Rest(temp);
  }

  ListExpr first = nl->First(args);
  ListExpr second = nl->Second(args);
  if(!CcString::checkType(nl->First(nl->Third(args)))){
     return listutils::typeError("Second arg not of type string");
  }
   
  ListExpr fun;
  fun = nl->Fourth(args);
  ListExpr funType = nl->First(fun);

  ListExpr firstType = nl->First(first);
   if(!DArray::checkType(firstType) && !Code::checkType(firstType)){
    return listutils::typeError(err + ": first arg not a darray or code");
  }

  ListExpr secondType = nl->First(second);
   if(!DArray::checkType(secondType) && !Code::checkType(secondType)){
    return listutils::typeError(err + ": second arg not a darray or code");
  }

  if(!listutils::isMap<2>(funType)){
    return listutils::typeError(err + ": last arg is not a function");
  }

  if(!nl->Equal(nl->Second(firstType), nl->Second(funType))){
    return listutils::typeError("type mismatch between first argument and "
                                "function arg");
  }
  if(!nl->Equal(nl->Second(secondType), nl->Third(funType))){
    return listutils::typeError("type mismatch between second argument and "
                                "function arg");
  }
  
  ListExpr resultType = nl->Fourth(funType); 
  ListExpr result = nl->TwoElemList(listutils::basicSymbol<Code>(),
                                    resultType);
  
   // replace eventually type mapping operators by the
   // resulting types
   ListExpr funQuery = nl->Second(nl->Fourth(args));
   ListExpr fa1o = nl->Second(funQuery);
   ListExpr fa1 = nl->TwoElemList(
                       nl->First(fa1o),
                       nl->Second(nl->First(nl->First(args))));
   ListExpr fa2o = nl->Third(funQuery);
   ListExpr fa2 = nl->TwoElemList(
                       nl->First(fa2o),
                       nl->Second(nl->First(nl->Second(args))));
   funQuery = nl->FourElemList(
                   nl->First(funQuery),
                   fa1, fa2, nl->Fourth(funQuery));
   string rfun = nl->ToString(funQuery);   
  
  

  if(DArray::checkType(nl->First(nl->First(args))) 
    && DArray::checkType(nl->First(nl->Second(args)))
    && CcString::checkType(nl->First(nl->Third(args)))
    && listutils::isMap<2>(nl->First(nl->Fourth(args)))){
 
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(rfun)),
               result);
  }
  if(DArray::checkType(nl->First(nl->First(args))) 
    && Code::checkType(nl->First(nl->Second(args)))
    && CcString::checkType(nl->First(nl->Third(args)))
    && listutils::isMap<2>(nl->First(nl->Fourth(args)))){
 
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(rfun)),
               result);
  }
  if(Code::checkType(nl->First(nl->First(args))) 
    && DArray::checkType(nl->First(nl->Second(args)))
    && CcString::checkType(nl->First(nl->Third(args)))
    && listutils::isMap<2>(nl->First(nl->Fourth(args)))){
 
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(rfun)),
               result);
  }
  if(Code::checkType(nl->First(nl->First(args))) 
    && Code::checkType(nl->First(nl->Second(args)))
    && CcString::checkType(nl->First(nl->Third(args)))
    && listutils::isMap<2>(nl->First(nl->Fourth(args)))){
 
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(rfun)),
               result);
  }

  return listutils::typeError(err);
}

/*
1.16.2 Selection Function

*/
int loop2SSelect( ListExpr args )
{
  if(DArray::checkType(nl->First(args)) && DArray::checkType(nl->Second(args))){
    return 0;
  }
  if(Code::checkType(nl->First(args)) && DArray::checkType(nl->Second(args))){
    return 1;
  }
  if(DArray::checkType(nl->First(args)) && Code::checkType(nl->Second(args))){
    return 2;
  }
  return 3;
}

/*
1.16.3 Value Mapping Function

*/


int dloop2SVM_ArrayArray(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   DArray* array1 = (DArray*) args[0].addr;
   DArray* array2 = (DArray*) args[1].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   
   if(! array1->IsDefined() || ! array2->IsDefined()){
      res->makeUndefined();
      return 0;
   }  
   string a1Name = array1->getName();
   size_t a1s = array1->getSize();
   res->setSize(a1s);

   string a2Name = array2->getName();
   size_t a2s = array2->getSize();

   if(a1s != a2s){
      res->makeUndefined();
      return 0;
   }  

   string resName;
   CcString* name = (CcString*) args[2].addr;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }

   string fun = ((FText*) args[4].addr)->GetValue();
   vector<TaskElement*> tasks;
   int pos = 0;
   size_t worker1;
   size_t worker2;
   TaskElement* element;
   for(size_t i=0; i < a1s; i++){
      pos++;
      worker1 = res->mapIndex(array1->getWorkerForSlot(i));
      worker2 = res->mapIndex(array2->getWorkerForSlot(i));
      element = new TaskElement(pos,
                                a1Name,i,worker1,
                                a2Name,i,worker2,
                                fun,
                                resName,i,0);
      tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

int dloop2SVM_CodeArray(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   Code* code = (Code*) args[0].addr;
   DArray* array = (DArray*) args[1].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   string fun = ((FText*) args[4].addr)->GetValue();
   
  if(!array->IsDefined() || !code->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   CcString* name = (CcString*) args[2].addr;
   string resName;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }
   if(!stringutils::isIdent(resName)){
    res->makeUndefined();
    return 0;
   }
   
   vector<TaskElement*> tasks;
   TaskElement* e;
   TaskElement* element;
   
   string a2Name = array->getName();
   size_t as = array->getSize();
   res->setSize(as);

   size_t size = code->sizeOfTasks();
   for(size_t i = 0; i < size; i++){
     e = code->getTask(i);
     DArrayElement* worker1 = code->GetWorker(e->GetWorker1());
     res->AddIfNotContains(*worker1);
     DArrayElement* worker2 = code->GetWorker(e->GetWorker2()); 
     res->AddIfNotContains(*worker2);
     element = new TaskElement( e->GetPos(),
                   e->GetA1Name(),e->GetPart1(),e->GetWorker1(),
                   e->GetA2Name(),e->GetPart2(),e->GetWorker2(),
                   e->GetFunQuery(),
                   e->GetResName(),e->GetResPart(),e->GetResWorker());
     tasks.push_back(element);
   }
   
   size_t pos = size;
   size_t worker2;
   size_t startIndexOfLastResult = code->GetStartIndexOfLastResult();
   for(size_t i = startIndexOfLastResult; i < size;i++){
     pos++;
     e = code->getTask(i);
     worker2 = res->mapIndex(array->getWorkerForSlot(e->GetResPart()));
     element = new TaskElement( pos,
                   e->GetResName(),e->GetResPart(),e->GetResWorker(),
                   a2Name,e->GetResPart(),worker2,
                   fun,
                   resName,e->GetResPart(),0);
     tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

int dloop2SVM_ArrayCode(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   DArray* array = (DArray*) args[0].addr;
   Code* code = (Code*) args[1].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   
   if(!array->IsDefined() || !code->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   CcString* name = (CcString*) args[2].addr;
   string resName;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }
   if(!stringutils::isIdent(resName)){
    res->makeUndefined();
    return 0;
   }
   
   string fun = ((FText*) args[4].addr)->GetValue();   
   vector<TaskElement*> tasks;
   TaskElement* e;
   TaskElement* element;
   
   string a1Name = array->getName();
   size_t worker1 = 0;
   size_t as = array->getSize();
   res->setSize(as);

   size_t size = code->sizeOfTasks();
   for(size_t i = 0; i < size; i++){
     e = code->getTask(i);
     DArrayElement* worker1 = code->GetWorker(e->GetWorker1());
     res->AddIfNotContains(*worker1);
     DArrayElement* worker2 = code->GetWorker(e->GetWorker2()); 
     res->AddIfNotContains(*worker2);
     element = new TaskElement( e->GetPos(),
                   e->GetA1Name(),e->GetPart1(),e->GetWorker1(),
                   e->GetA2Name(),e->GetPart2(),e->GetWorker2(),
                   e->GetFunQuery(),
                   e->GetResName(),e->GetResPart(),e->GetResWorker());
     tasks.push_back(element);
   }
   
   size_t pos = size;
   for(size_t i = code->GetStartIndexOfLastResult(); i < size;i++){
     pos++;
     e = code->getTask(i);
     worker1 = res->mapIndex(array->getWorkerForSlot(e->GetResPart()));
     element = new TaskElement(pos,
                   a1Name,e->GetResPart(),worker1,
                   e->GetResName(),e->GetResPart(),e->GetResWorker(),
                   fun,
                   resName,e->GetResPart(),0);
     tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

int dloop2SVM_CodeCode(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   Code* code1 = (Code*) args[0].addr;
   Code* code2 = (Code*) args[1].addr;

   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;

   if(!code1->IsDefined() || !code2->IsDefined()){
      res->makeUndefined();
      return 0;
   }  
   
   CcString* name = (CcString*) args[2].addr;
   string resName;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }
   if(!stringutils::isIdent(resName)){
    res->makeUndefined();
    return 0;
   }
   
   string fun = ((FText*) args[4].addr)->GetValue();
   
   vector<TaskElement*> tasks;
   TaskElement* e;
   TaskElement* element;

   size_t size1 = code1->sizeOfTasks();
   for(size_t i = 0; i < size1; i++){
     e = code1->getTask(i);
     DArrayElement* worker1 = code1->GetWorker(e->GetWorker1());
     res->AddIfNotContains(*worker1);
     DArrayElement* worker2 = code1->GetWorker(e->GetWorker2()); 
     res->AddIfNotContains(*worker2);
     element = new TaskElement( e->GetPos(),
                   e->GetA1Name(),e->GetPart1(),e->GetWorker1(),
                   e->GetA2Name(),e->GetPart2(),e->GetWorker2(),
                   e->GetFunQuery(),
                   e->GetResName(),e->GetResPart(),e->GetResWorker());
     tasks.push_back(element);
   }

   size_t pos = code1->sizeOfTasks();
   size_t size2 = code2->sizeOfTasks();
   for(size_t i = 0; i < size2; i++){
     pos++;
     e = code2->getTask(i);
     DArrayElement* worker1 = code2->GetWorker(e->GetWorker1());
     res->AddIfNotContains(*worker1);
     DArrayElement* worker2 = code2->GetWorker(e->GetWorker2()); 
     res->AddIfNotContains(*worker2);
     element = new TaskElement( pos,
                   e->GetA1Name(),e->GetPart1(),e->GetWorker1(),
                   e->GetA2Name(),e->GetPart2(),e->GetWorker2(),
                   e->GetFunQuery(),
                   e->GetResName(),e->GetResPart(),e->GetResWorker());
     tasks.push_back(element);
   }

   TaskElement* e1;
   TaskElement* e2;
   size_t i2 = code2->GetStartIndexOfLastResult();
   for(size_t i1 = code1->GetStartIndexOfLastResult(); i1 < size1;i1++){
     pos++;
     e1 = code1->getTask(i1);
     e2 = code2->getTask(i2);
     element = new TaskElement( pos,
                   e1->GetResName(),e1->GetResPart(),e1->GetResWorker(),
                   e2->GetResName(),e2->GetResPart(),e2->GetResWorker(),
                   fun,
                   resName,e1->GetResPart(),0);
     tasks.push_back(element);
     i2++;
   }
   
   res->setSize(pos - (size1 + size2));
   res->set(tasks);
   return 0; 
}

ValueMapping dloop2SVM[] = {
  dloop2SVM_ArrayArray,
  dloop2SVM_CodeArray,
  dloop2SVM_ArrayCode,
  dloop2SVM_CodeCode
};

/*
1.16.4 Specification,

*/
OperatorSpec dloop2SSpec(
     " {darray(X), code(darray(X))} x {darray(X), code(darray(X))}" 
     " x string x  (X->Y) -> code(darray(y))",
     " _ _ dloop2S[_,_]",
     "Generates a code Object that performs a function "
     "on each element of two darray instances, of one darray and one code "
     "instance or two code instances."
     "The string argument specifies the name of the result. If the name"
     " is undefined or an empty string, a name is generated automatically.",
     "query da1 da2 dloop2S[\"da3\", . toTasks"
     );

/*
1.16.5 Operator instance

*/
Operator dloop2SOp(
  "dloop2S",
  dloop2SSpec.getStr(),
  4,
  dloop2SVM,
  loop2SSelect,
  dloop2STM
);


/*
1.17 Operator ~DARRAYELEM~ , ~DARRAYELEM2~

This operators checks whether a parameter of a function is an
element of a DARRAY


*/

ListExpr
DARRAYELEMfromCodeTM( ListExpr args )
{

  cout << "DARRAYELEMfromCodeTM args: " << nl->ToString(args) << endl;
  
  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("at least one argument required");
  }
  
  
  ListExpr first = nl->First(args);
  ListExpr type = nl->Second(first);
  if(!Code::checkType(first)){
    return listutils::typeError("code expected");
  }
  return type;
}

OperatorSpec DARRAYELEMfromCodeSpec(
     "code(X) -> X ",
     "DARRAYELEM(_)",
     "Type Mapping Operator. Extract the type of a code.",
     "query c2 dloop[\"da3\", . count]"
     );

Operator DARRAYELEMfromCodeOp (
      "DARRAYELEM",
      DARRAYELEMfromCodeSpec.getStr(),
      0,   
      Operator::SimpleSelect,
      DARRAYELEMfromCodeTM );


ListExpr
DARRAYELEM2fromCodeTM( ListExpr args )
{
  if(!nl->HasMinLength(args,2)){
    return listutils::typeError("two arguments required");
  }
  ListExpr second = nl->Second(args);
  if(!Code::checkType(second)){
    return listutils::typeError("code expected");
  }
  ListExpr res =  nl->Second(second);
  return res;
}

OperatorSpec DARRAYELEM2fromCodeSpec(
     "T x code(Y) x ... -> Y ",
     "DARRAYELEM2(_)",
     "Type Mapping Operator. Extract the type of a darray.",
     "query c2 c3 dloop[\"da3\", .. count"
     );

Operator DARRAYELEM2fromCodeOp (
      "DARRAYELEM2",
      DARRAYELEM2fromCodeSpec.getStr(),
      0,   
      Operator::SimpleSelect,
      DARRAYELEM2fromCodeTM );

/*
1.18 Operator totasks

The totasks operator exports all the tasks of a code object into a stream.

1.18.1 Type Mapping

This operator has a code object as arguments. The output is a stream of tasks.

*/
ListExpr  toTasksTM(ListExpr args){
  string err = "Code object  expected";

  if (nl->ListLength(args) != 1)
    return listutils::typeError(err);
  
  if(!Code::checkType(nl->First(args))){
    return listutils::typeError(err);    
  }

   //The mapping of the  stream(tuple)
  ListExpr attr11 = nl->TwoElemList( nl->SymbolAtom("ResWorker"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  ListExpr attrList = nl->OneElemList(attr11);

  ListExpr attr10 = nl->TwoElemList( nl->SymbolAtom("ResPart"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr10, attrList);

  ListExpr attr9 = nl->TwoElemList( nl->SymbolAtom("Res"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr9, attrList);

  ListExpr attr8 = nl->TwoElemList( nl->SymbolAtom("Query"),
                                    nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr8, attrList);
  
  ListExpr attr7 = nl->TwoElemList( nl->SymbolAtom("Worker2"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr7, attrList);

  ListExpr attr6 = nl->TwoElemList( nl->SymbolAtom("Part2"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr6, attrList);
  
  ListExpr attr5 = nl->TwoElemList( nl->SymbolAtom("Arg2"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr5, attrList);
  
  ListExpr attr4 = nl->TwoElemList( nl->SymbolAtom("Worker1"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr4, attrList);
  
  ListExpr attr3 = nl->TwoElemList( nl->SymbolAtom("Part1"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr3, attrList);
  
  ListExpr attr2 = nl->TwoElemList( nl->SymbolAtom("Arg1"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr2, attrList);
  
  ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Task"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr1, attrList);

  ListExpr returnList = nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
              nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                  attrList));  
  return returnList;
}


/*
1.18.2 Value Mapping

*/

int toTasksVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
  TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
  Code* c = (Code*)args[0].addr;
  switch (message) {
    case OPEN: {
      c->BeginIteration();
      return 0;
    }
    case REQUEST: {
      TaskElement* task = c->GetNext();
      if(task != 0){
        result.addr = task->getTuple(tt);
      }else{
        result.addr = 0;
      }
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      return 0;
    }
  }
  return 0;
}

/*
1.18.3 Specification

*/
OperatorSpec toTasksSpec(
     "Code(darray(X)) -> stream(tuble)",
     "_ toTasks",
     "The elements of a Code object are exported in a stream. ",
     "query c toTasks"
); 


/*
1.18.4 Operator instance


*/
Operator toTasksOp (
    "toTasks",                    //name
    toTasksSpec.getStr(),         //specification
    toTasksVM,                    //value mapping 
    Operator::SimpleSelect,        //trivial selection function
    toTasksTM                     //type mapping
);



/*
1.19 Operator schedule

The schedule operator execudes all the tasks of a code object.
It distributes tasks to servers. When a server informs 
the scheduler that a task is finished, the scheduler selects another task 
and assigns it to this server.The list of tasks may contain dependencies,
so that a certain task can only be started when a specified other task has been 
finished. In this way the overlapping processing of successive operations 
can be controlled.

1.19.1 Type Mapping

code(X) x rel(workers) -> darray(X)

This operator has 2 arguments. Input is a code object and a list of workers,
that are stored in an relation in th the database. 

*/

bool isWorkerRelDesc(ListExpr rel, ListExpr& positions, ListExpr& types,
                     string& errMsg){

  if(!Relation::checkType(rel)){
     errMsg = " not a relation";
     return false;
  }
  ListExpr attrList = nl->Second(nl->Second(rel));

  ListExpr htype;

  int hostPos = listutils::findAttribute(attrList,"Host",htype);
  if(!hostPos){
     errMsg = "Attribute Host not present in relation";
     return false;
  }
  if(!CcString::checkType(htype) && !FText::checkType(htype)){
     errMsg = "Attribute Host not of type text or string";
     return false;
  }
  hostPos--;

  ListExpr ptype;
  int portPos = listutils::findAttribute(attrList,"Port",ptype);
  if(!portPos){
    errMsg = "Attribute Port not present in relation";
    return false;
  }
  if(!CcInt::checkType(ptype)){
     errMsg = "Attribute Port not of type int";
     return false;
  }
  portPos--;
  ListExpr ctype;
  int configPos = listutils::findAttribute(attrList, "Config", ctype);
  if(!configPos){
    errMsg = "Attrribute Config not present in relation";
    return false;
  }
  if(!CcString::checkType(ctype) && !FText::checkType(ctype)){
     errMsg = "Attribute Config not of type text or string";
     return false;
  }
  configPos--;
  positions = nl->ThreeElemList(
               nl->IntAtom(hostPos),
               nl->IntAtom(portPos),
               nl->IntAtom(configPos));
  types = nl->ThreeElemList(htype, ptype,ctype);

  return true;
}
  
ListExpr  scheduleTM(ListExpr args){
  string lenErr = "2 parameters expected";
  string err = "code x rel expected";
  string mapErr = "It's not a map ";

  if (nl->ListLength(args) != 2)
    return listutils::typeError(lenErr);
    
  if(!Code::checkType(nl->First(args))){
    return listutils::typeError(err);    
  }
  
  ListExpr positions;
  string errMsg;
  ListExpr types;
  if(!isWorkerRelDesc(nl->Second(args),positions,types, errMsg)){
     return listutils::typeError("second argument is not a worker relation:" 
                                 + errMsg);
  }else{
    ListExpr appendList = nl->FiveElemList(
                          nl->First(positions),
                          nl->Second(positions),
                          nl->Third(positions),
                          nl->BoolAtom(
                                CcString::checkType(
                                    nl->First(types))),
                          nl->BoolAtom(
                                CcString::checkType(
                                    nl->Third(types))));
    
    ListExpr type = nl->Second(nl->First(args));
    ListExpr resType = nl->TwoElemList(
                        listutils::basicSymbol<DArray>(),
                        type);
  
    return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                           appendList,
                           resType);
  }
}


/*
1.19.2 Value Mapping

*/


void retrieveObject(const string arrayName, int i, 
                    ConnectionInfo* ci1,ConnectionInfo* ci2){

  string originalName = arrayName + "_" 
                                  + stringutils::int2str(i);
  stringstream ss;
  ss << "temp_" << arrayName << "_" << i << "_" 
      <<  ci1->serverPid() 
      << "_" << ci2->serverPid();
  string tempObject = ss.str();

  // step 1 create a file containing the object on ci2
  int err;
  string errMsg;
  string r;
  double runTime;
  string cmd;
//  if(!mi->arg2IsRel){
//      cmd =   "save " + originalName + " to " + tempObject;
//  } else {
      cmd = "query " + originalName + " feed fconsume5['" 
            + tempObject +"'] count";
//  }
  ci2->simpleCommand( cmd, err,errMsg,r,false, runTime);
  if(err){
    cerr << "cmd << " << cmd << " failed  with code " << err << endl;
    cerr << errMsg;
    return;
  }  

  // step2 transfer this file to ci1
  cmd =   "query getFileTCP('" + tempObject +"', '" 
              + ci2->getHost() 
              + "', " + stringutils::int2str(ci2->getPort()) 
              + ", TRUE, '" + tempObject+"')";
  ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
  if(err){
    cerr << "cmd << " << cmd << " failed  with code " << err << endl;
    cerr << errMsg;
    return;
  }  
  
  // step3 create the temporary object from this file
//  if(!mi->arg2IsRel){
//      cmd = "restore " + tempObject + " from " + tempObject;
//  } else {
      cmd = "let " + tempObject + " = '" + tempObject
            + "' ffeed5 consume";
//  }
  ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
  if(err){
    cerr << "cmd << " << cmd << " failed  with code " << err << endl;
    cerr << errMsg;
    return;
  }  
  // step4 delete temp files on ci1 and ci2
  cmd = "query removeFile('" + tempObject+"')";
  ci1->simpleCommand( cmd, err,errMsg,r,false, runTime);
  if(err){
    cerr << "cmd << " << cmd << " failed  with code " << err << endl;
    cerr << errMsg;
  }  
  ci2->simpleCommand( cmd, err,errMsg,r,false, runTime);
  if(err){
    cerr << "cmd << " << cmd << " failed  with code " << err << endl;
    cerr << errMsg;
  }  
}
 
class ScheduleRunner{
  public:

  ScheduleRunner(Code* _code, TaskElement* _task, DArrayElement* worker ){
    code = _code;
    task = _task;
    done = false;
  }
  
  size_t GetTaskId(){
    return task->GetPos();
  }

  void run(){
    cout << "start run task" << task->GetPos() << endl;
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    ConnectionInfo* cRes = algInstance->getWorkerConnection(
                        *code->GetWorker(task->GetResWorker()),dbname);
    if(task->GetResWorker() != task->GetWorker1()){
      cout << "retrieveObject for task->GetPos(): " << task->GetPos() << endl;
      cout << "task->GetResWorker(): " << task->GetResWorker() << endl;
      cout << "task->GetWorker1(): " << task->GetWorker1() << endl;
      ConnectionInfo* c1 = algInstance->getWorkerConnection(
                        *code->GetWorker(task->GetWorker1()),dbname);
      retrieveObject(task->GetA1Name(),task->GetPart1(),cRes,c1);
    }
    if(task->GetResWorker() != task->GetWorker1()){
      cout << "retrieveObject for task->GetPos(): " << task->GetPos() << endl;
      cout << "task->GetResWorker(): " << task->GetResWorker() << endl;
      cout << "task->GetWorker1(): " << task->GetWorker1() << endl;
      ConnectionInfo* c2 = algInstance->getWorkerConnection(
                        *code->GetWorker(task->GetWorker2()),dbname);
      retrieveObject(task->GetA2Name(),task->GetPart2(),cRes,c2);
    }

    int err = 0;
    string errMsg;
    string strres;
    string resName = task->GetResName() + "_" + 
      stringutils::int2str(task->GetResPart());
    string nameA1 = task->GetA1Name() + "_" + 
      stringutils::int2str(task->GetPart1());
    string nameA2 = task->GetA2Name() + "_" + 
      stringutils::int2str(task->GetPart2());
    // remove old stuff
    //cRes->simpleCommand("delete " + resName,err,errMsg,strres,false,runtime);
    // create new one
    string cmd ="(let " + resName + " = ( " + 
      task->GetFunQuery() + " " + nameA1 + " " + nameA2 +"))";

    //cRes->simpleCommandFromList(cmd, err,errMsg, strres, false, runtime);
    if(err!=0){
      cout << "error in creating result via " << cmd << endl;
      cout << errMsg << endl;
      return;
    }
    done = true;
    cout << "end run task" << task->GetPos() << endl;
  }

  bool Done(){
    return done;
  }
  
  private:
    Code* code;
    TaskElement* task;
    DArrayElement* worker;
    bool done;
};


int scheduleVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

  //Code:
  Code* c = (Code*) args[0].addr;
  //cout <<" code : " << nl->ToString(c->toListExpr()) << endl;
     
  
  //The result :
  Relation* rel = (Relation*) args[1].addr;
  int hostPos = ((CcInt*) args[2].addr)->GetValue();
  int portPos = ((CcInt*) args[3].addr)->GetValue();
  int configPos = ((CcInt*) args[4].addr)->GetValue();
  result = qp->ResultStorage(s);
  DArray* res = (DArray*) result.addr;
  size_t sizeOfResultArray = c->getSize();
  string nameOfResultArray = c->GetResultName();
  (*res) =  DArray::createFromRel<CcString,CcString>(rel,sizeOfResultArray,
                   nameOfResultArray,hostPos,portPos,configPos);

  if(!res->IsDefined()){
    return 0;
  }

  if(res->numOfWorkers()==0){
    res->makeUndefined();
    return 0;
  }
  
  //Add missing worker to code
  for(size_t i=0;i< res->getSize();i++){
    c->AddIfNotContains(res->getWorkerForSlot(i));
  }
  
  //Find the depencencies
  c->BuildDependenciesVector();
  
  //Define a thread for each worker and start it.
  TaskElement* task; 
  DArrayElement* worker;
  ScheduleRunner* sr;
  vector<ScheduleRunner*> srs;
  boost::thread* runner;
  vector<boost::thread*> runners;

  size_t workerSize = c->GetSizeOfWorker();
  for(size_t i = 0;i < workerSize; i++){
    worker = c->GetWorker(i);
    task = c->GetNextTask(i);

    cout << "worker [" << i << "]" << "get task" << task->GetPos() << endl;
    if(task != 0){
      task->SetResWorker(i);
      sr = new ScheduleRunner(c, task, worker);
      srs.push_back(sr);
      runner = new boost::thread(&ScheduleRunner::run, sr);
      runners.push_back(runner);
    }
  }
  
  //Execute all other tasks
  while(!c->Done()){
    for(size_t i = 0;i < workerSize; i++){
      if(srs[i] != 0 && srs[i]->Done() == true){
        runners[i]->join();
        c->ClearDependency(srs[i]->GetTaskId());  
        delete runners[i];
        runners[i] = 0;
        delete srs[i];
        srs[i] = 0;
      }
      if(runners[i] == 0){
        task = c->GetNextTask(i);  
        if(task != 0){ 
          task->SetResWorker(i);
          worker = c->GetWorker(i);
          srs[i] = new ScheduleRunner(c, task, worker);
          runners[i] = new boost::thread(&ScheduleRunner::run, srs[i]);
        }
      }
    }
  }

  //Delete all threads
  for( size_t i=0;i< workerSize;i++){
    if(runners[i] != 0){
      runners[i]->join();
      delete runners[i];
      runners[i] = 0;
      delete srs[i];
      srs[i] = 0;
    }
  }
  return 0;
}

/* 
1.19.3 Specification 

*/
OperatorSpec scheduleSpec(
     "code(X) x rel -> darray(X)",
     "_ schedule[_]",
     "The first parameter contains the code with the tasks," 
     "the second a relation that defines the worker. "
     "The name of the resulting darray is specifyed by the tasks.", 
     "query c schedule(w) "
); 


/*
1.19.4 Operator instance


*/
Operator scheduleOp (
    "schedule",                    //name
    scheduleSpec.getStr(),         //specification
    scheduleVM,                    //value mapping 
    Operator::SimpleSelect,        //trivial selection function
    scheduleTM                     //type mapping
);



/*
3 Implementation of the Algebra

*/
Distributed3Algebra::Distributed3Algebra(){

   AddTypeConstructor(&CodeTC);
   CodeTC.AssociateKind(Kind::SIMPLE());

   AddOperator(&dloopSOp);
   dloopSOp.SetUsesArgsInTypeMapping();

   AddOperator(&dloop2SOp);
   dloop2SOp.SetUsesArgsInTypeMapping();

   AddOperator(&toTasksOp);

   AddOperator(&DARRAYELEMfromCodeOp);
   AddOperator(&DARRAYELEM2fromCodeOp);
   
   AddOperator(&scheduleOp);

}


Distributed3Algebra::~Distributed3Algebra(){
}


} // end of namespace distributed3

extern "C"
Algebra*
   InitializeDistributed3Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

   distributed3::algInstance = new distributed3::Distributed3Algebra();
   //distributed3::showCommands = false;   
   return distributed3::algInstance;
}


