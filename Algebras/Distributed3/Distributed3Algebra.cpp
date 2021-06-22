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
//#include "./Timeout.h"
#include "Algebras/Distributed2/ConnectionInfo.h"



#include "tes/TESManager.h"
#include "tes/TESClient.h"
#include "tes/typedefs.h"
//#include "tes/Operators/Tests/TESTests.h"
//#include "./DmapPdmap.h"
#include "./DmapPdmapTypeMapper.h"
#include "./PartitiondmapTypeMapper.h"
#include "tes/Operators/Interface2TES/Distribute2TES.h"
#include "tes/Operators/Interface2TES/FeedTES.h"
#include "tes/Operators/Messaging/StartTESClient.h"
#include "tes/Operators/Messaging/StartTESServer.h"
#include "tes/Operators/OnMaster/KillTES.h"
#include "tes/Operators/OnMaster/SetupTES.h"
#include "tes/Operators/OnWorker/ResetTES.h"
#include "tes/Operators/OnWorker/StartLoopbackTESClient.h"
#include "tes/Operators/OnWorker/SetTupleType.h"

//#include "Dist2Helper.h"
#include "Distributed3Algebra.h"
//#include "FileTransferServer.h"
//#include "FileTransferClient.h"
//#include "FileTransferKeywords.h"
//#include "SocketIO.h"
#include "Algebras/Distributed2/CommandLogger.h"
#include "Algebras/Relation-C++/OperatorConsume.h"
#include "Bash.h"
#include "Algebras/Distributed2/fsrel.h"
//#include "fobj.h"
//#include "FileRelations.h"
//#include "FileSystem.h"
#include "Algebras/Array/ArrayAlgebra.h"
#include "Algebras/Collection/CollectionAlgebra.h"
//#include "FileAttribute.h"
//#include "SecParser.h"
#include "Bash.h"


#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>

#include "Tools/DFS/dfs/remotedfs.h"
#include "Tools/DFS/dfs/dfs.h"
#include "Algebras/Distributed2/DFSType.h"
//#include "DFSTools.h"
//#include "BalancedCollect.h"


extern boost::mutex nlparsemtx;

using namespace std;

namespace distributed3 {

bool showCommands;

distributed2::CommandLog commandLog;

const int defaultTimeout = 0; 
Distributed3Algebra* algInstance;

DFSType* filesystem = 0;

  void Distributed3Algebra::enableDFS(const string& host, const int port){

      this->dfshost = host;
      this->dfsport = port;

      // iterate over user defined connections
      vector<distributed2::ConnectionInfo*>::iterator itc;
      for(itc = connections.begin();itc!=connections.end();itc++){
          enableDFS(*itc);
      }
      // iterate over worker connections
      map<distributed2::DArrayElement,
          pair<string,distributed2::ConnectionInfo*> >::iterator itw;
      for(itw=workerconnections.begin(); itw!=workerconnections.end(); itw++){
         enableDFS(itw->second.second);
      }
   }
    void Distributed3Algebra::enableDFS(distributed2::ConnectionInfo* ci){
     if(!ci){
       return; 
     }
     if(dfsport<0){
       return;
     }
     stringstream ss;
     ss << "query enableDFSFT('"<< dfshost<<"', " << dfsport << ")";
     int err;
     string res;
     double rt;
     ci->simpleCommand(ss.str(),err,res,false,rt,false,commandLog,true,timeout);
   }
    /*
    SM TODO ConnectionInfo* createWorkerConnection(const DArrayElement& worker) 
    sinnvoller
    
    */
    // tries to create a new connection to a worker
    bool Distributed3Algebra::createWorkerConnection(
            const distributed2::DArrayElement& worker, pair<string,
                                distributed2::ConnectionInfo*>& res){
      string host = worker.getHost();
      int port = worker.getPort();
      string config = worker.getConfig();
      distributed2::ConnectionInfo* ci = 
          distributed2::ConnectionInfo::createConnection(host, port, config, 
                                                            timeout, heartbeat);

      if(ci == nullptr){
         Bash::setFGColor(Red);
         cout << "Connection to Server " << host << "@" << port 
              << " failed" << endl;
         Bash::normalColors();
      }
      res.first="";
      res.second = ci;
      return ci!=0;
    }

   // returns a unique number
   size_t Distributed3Algebra::nextNameNumber(){
      boost::lock_guard<boost::mutex> guard(namecounteraccess);
      namecounter++;
      return namecounter;
   } 
   distributed2::ConnectionInfo* Distributed3Algebra::getWorkerConnection(
                                       const distributed2::DArrayElement& info,
                                       const string& dbname,
                                       distributed2::CommandLogger* log ){
     
     static boost::mutex gwcmtx; // allow only one thread to call this fun
     boost::lock_guard<boost::mutex> guard0(gwcmtx);
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<distributed2::DArrayElement, 
            pair<string, distributed2::ConnectionInfo*> >::iterator it;
     it = workerconnections.find(info);
     pair<string,distributed2::ConnectionInfo*> pr("",0);
     if(it==workerconnections.end()){
        if(!createWorkerConnection(info,pr)){
             return 0;
        }
        if(dfsport>0){
          enableDFS(pr.second);
        }
        workerconnections[info] = pr;
        it = workerconnections.find(info);
     } else {
        pr = it->second;
     }
     pr.second->setLogger(log);
     pr.second->setNum(info.getNum());
     string wdbname = dbname;
     if(pr.first!=wdbname){
         if(!pr.second->switchDatabase(wdbname,true, showCommands, false,
                                       getTimeout())){
            it->second.first="";
            return 0;
         } else {
             it->second.first=dbname;
         }
     } 
     return pr.second?pr.second->copy():0;
  }

  template<class A>
  distributed2::ConnectionInfo* Distributed3Algebra::getWorkerConnection(
       A* array,
       int slot,
       const string& dbname,
       distributed2::CommandLogger* log,
       bool allowArrayChange
  ) {
     distributed2::DArrayElement elem = array->getWorkerForSlot(slot);
     distributed2::ConnectionInfo* ci = getWorkerConnection(elem, dbname, log);
     if(ci || !allowArrayChange){
        return ci;
     } 
     int retry = 0;
     set<size_t> usedWorkers;
     return changeWorker1(array,slot, usedWorkers, retry,ci);
  }
  
 string Distributed3Algebra::getTempName(int server){
    boost::lock_guard<boost::mutex> guard(mtx);
    if(server < 0 || (size_t)server<connections.size()){
       return "";
    }
    distributed2::ConnectionInfo* ci = connections[server];
    if(!ci){
       return "";
    }
    stringstream ss;
    ss << "TMP_" << WinUnix::getpid() 
       << "_" << ci->serverPid() 
       << "_" << nextNameNumber();
    return ss.str();
  } 

  string Distributed3Algebra::getTempName(
         const distributed2::DArrayElement& elem){
     boost::lock_guard<boost::mutex> guard(workerMtx);
     map<distributed2::DArrayElement, 
             pair<string,distributed2::ConnectionInfo*> >::iterator it;
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
  
  // TODO for some reason the tempName is too long in Distributed3Algebra
  // so the sh is commented
  string Distributed3Algebra::getTempName(){
     boost::lock_guard<boost::mutex> guard1(workerMtx);
     boost::lock_guard<boost::mutex> guard2(mtx);
     distributed2::ConnectionInfo* ci=0;
     for(size_t i=0;i<connections.size() && !ci; i++){
         ci = connections[i];
     }
     map<distributed2::DArrayElement, 
             pair<string,distributed2::ConnectionInfo*> >::iterator it;
     for(it = workerconnections.begin(); 
        it !=workerconnections.end() && !ci; 
        it++){
         ci = it->second.second;
     }
     int spid;
     //size_t sh;
     if(ci){
        //sh = stringutils::hashCode(ci->getHost());
        spid = ci->serverPid();
     } else {
        spid = rand();
        //sh = rand();
     }
     stringstream ss;
     ss << "TMP_" << WinUnix::getpid() 
        //<< "_" << sh 
        << "_" << spid 
        << "_" << nextNameNumber();
    
    return ss.str();
  }

  
 int Distributed3Algebra::closeAllWorkers(){
    boost::lock_guard<boost::mutex> guard(workerMtx);
    int count = 0;
    for(auto it=workerconnections.begin(); it!=workerconnections.end(); it++){
       distributed2::ConnectionInfo* ci = it->second.second;
       ci->deleteIfAllowed();
       count++;
    }
    workerconnections.clear();
    
    return count;
 } 
 
ListExpr replaceSymbols(ListExpr list, 
                        const map<string,string>& replacements){

  if(nl->IsEmpty(list)){
     return nl->TheEmptyList();
  }
  switch(nl->AtomType(list)){
     case SymbolType: {
       string symb = nl->SymbolValue(list);
       map<string,string>::const_iterator it = replacements.find(symb);
       if(it==replacements.end()){
         return list;
       } else {
         boost::lock_guard<boost::mutex> lock(nlparsemtx); 
         ListExpr res;
         nl->ReadFromString(it->second, res);
         return res;
       }
     }
     case NoAtom: {
       ListExpr first = nl->OneElemList( replaceSymbols(nl->First(list),
                                               replacements));
       ListExpr last = first;
       list = nl->Rest(list);
       while(!nl->IsEmpty(list)){
          last = nl->Append(last, replaceSymbols(nl->First(list),
                            replacements));
          list = nl->Rest(list);
       }
       return first;
     }
     default: return list;    
  } 
}

template<typename T>
ListExpr Distributed3Algebra::fun2cmd(ListExpr funlist, 
                                      const vector<T>& funargs){

  if(!nl->HasLength(funlist, 2+ funargs.size())){
    return nl->TheEmptyList();
  } 
  if(!listutils::isSymbol(nl->First(funlist),"fun")){
    return nl->TheEmptyList();
  }
  funlist = nl->Rest(funlist);
  map<string,T> replacements;
  int pos = 0;
  while(!nl->HasLength(funlist,1)){
     ListExpr first = nl->First(funlist);
     funlist = nl->Rest(funlist);
     if(!nl->HasLength(first,2)){
       cerr << "invalid function argument" << endl;
       return nl->TheEmptyList();
     }
     if(nl->AtomType(nl->First(first))!=SymbolType){
       cerr << "invalid function argument name " << endl;
       return nl->TheEmptyList();
     }
     replacements[nl->SymbolValue(nl->First(first))] = funargs[pos];
     pos++;
  }
  ListExpr rep = replaceSymbols(nl->First(funlist), replacements);
  return rep;
}

template<typename T>
ListExpr Distributed3Algebra::fun2cmd(const string& fundef, 
                                      const vector<T>& funargs){
  ListExpr funlist; 
  {
     boost::lock_guard<boost::mutex> guard(nlparsemtx);
     if(!nl->ReadFromString(fundef,funlist)){
       cerr << "Function is not a nested list" << endl;
       return nl->TheEmptyList();
     }
  }
  return fun2cmd<T>(funlist, funargs);
}

ListExpr Distributed3Algebra::replaceWrite(ListExpr list, 
                                           const string& writeVer, 
                                           const string& name){
  if(nl->IsEmpty(list)){
     return nl->TheEmptyList();
  }
  switch(nl->AtomType(list)){
     case NoAtom: {
       if(nl->HasLength(list,2)){
          if(listutils::isSymbol(nl->First(list), writeVer)) {
            return nl->FourElemList(
                       nl->SymbolAtom("write"),
                       Distributed3Algebra::replaceWrite(
                                   nl->Second(list), writeVer, name),
                       nl->StringAtom(name),
                       nl->BoolAtom(false));
          }
       }


       ListExpr first = nl->OneElemList( Distributed3Algebra::replaceWrite(
                                               nl->First(list),
                                               writeVer,
                                               name));
       ListExpr last = first;
       list = nl->Rest(list);
       while(!nl->IsEmpty(list)){
          last = nl->Append(last, Distributed3Algebra::replaceWrite(
                                    nl->First(list),
                                    writeVer,
                                    name));
          list = nl->Rest(list);
       }
       return first;
     }
     default: return list;    
  } 
}


Distributed3Algebra* Distributed3Algebra::getAlgebra() {
  return algebra;
}

Distributed3Algebra* Distributed3Algebra::algebra = nullptr;

void Distributed3Algebra::setAlgebra(Distributed3Algebra *algebra) {
  assert(Distributed3Algebra::algebra == nullptr); // singleton`
  Distributed3Algebra::algebra = algebra;
}
void writeLog(distributed2::ConnectionInfo* ci, const string& msg){
  if(!algInstance){
    return;
  }
  if(ci){
    algInstance->errorWriter.writeLog(ci->getHost(), 
                                      ci->getPort(), 
                                      ci->serverPid(),
                                      msg);
  }
}


void writeLog(distributed2::ConnectionInfo* ci, const string& cmd, 
                                                const string& msg){
  if(!algInstance){
    return;
  }
  if(ci){
    string msg2 = "Error during executing command " + cmd
               + "\n " + msg;
    writeLog(ci,msg2);
  }
}

/*
17 TypeMapOperator ~DPD1~

*/
ListExpr DPD1TM(ListExpr args){
  
  if(!nl->HasLength(args,2)) {
    return listutils::typeError("2 arguments expected") ;
  }
    ListExpr arg = nl->First(args);
    if(distributed2::DArray::checkType(arg) ){
       if(!Relation::checkType(nl->Second(arg))){
          return listutils::typeError("darray's subtype musts be a relation");
       }
       return nl->Second(arg); // return the relation
    } 
    if(distributed2::DFArray::checkType(arg)){
       return nl->TwoElemList( listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second(nl->Second(arg)));
    }
    return listutils::typeError("first arg is not a d[f]array");
  
}


OperatorSpec DPD1Spec(
  "d[f]array(rela(tuple(X))) x A -> frel(tuple(X)) ",
  " DPD1(_,_)",
  "Type Map Operator",
  "query DPD1(test) getTypeNL"
);


Operator DPD1Op(
  "DPD1",
  DPD1Spec.getStr(),
  0,
  Operator::SimpleSelect,
  DPD1TM
);

/*
17 TypeMapOperator ~DPD2~

*/
ListExpr DPD2TM(ListExpr args){

  //std::cout << "args ind DPD2TM:";
  //nl->WriteListExpr(args);
  
  if(!nl->HasLength(args,3)){
    return listutils::typeError("3 arguments expected") ;
  }

  ListExpr arg = nl->Third(args);
  if(!listutils::isMap<1>(arg) && !listutils::isMap<2>(arg)){
    return listutils::typeError("third arg is not a function");
  } 
  // extract function result 
  while(!nl->HasLength(arg,1)){
    arg = nl->Rest(arg);
  }
  arg = nl->First(arg);
  if(!Stream<Tuple>::checkType(arg)){
    return listutils::typeError("function result is not a tuple stream");
  }
  ListExpr res =  nl->Second(arg);
  return res;
}


OperatorSpec DPD2Spec(
  "d[f]array(rela(tuple(X))) x A -> frel(tuple(X)) or  "
  "B x C x (frel(tuple(D) -> stream(tuple(E)) ) -> tuple(E)",
  " DPD2(_,_,_)",
  "Type Map Operator",
  "query DPD2(test) getTypeNL"
);


Operator DPD2Op(
  "DPD2",
  DPD2Spec.getStr(),
  0,
  Operator::SimpleSelect,
  DPD2TM
);
/*
18 TypeMapOperator ~PDTS~

*/

ListExpr PDTSTM(ListExpr args) {
  if (!nl->HasLength(args,2) && !nl->HasLength(args,4)) {
      return listutils::typeError("two or four arguments expected");
  }
  
  if (nl->ListLength(nl->First(args)) != 2) {
    return listutils::typeError("first argument must be a d[f]array");
  }
  ListExpr relation = nl->Second(nl->First(args));
  if(!Relation::checkType(relation)){
    return listutils::typeError("d[f]array's subtype must be a relation");
  }
  ListExpr tuple = nl->Second(relation);
  if(!Tuple::checkType(tuple)){
    return listutils::typeError("subtype of d[f]array's " 
                                "relation is not a tuple");
  }
  if (nl->HasLength(args,2)) {
    return tuple;
  }
  
  return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple>>(),
                           nl->Second(relation));
  
}
/*
17 TypeMapOperator ~DPD4~

*/
ListExpr DPD4TM(ListExpr args){
  //std::cout << "args in DPD4TM: ";
  //nl->WriteListExpr(args);
  if(   !nl->HasLength(args,2) 
     && !nl->HasLength(args,3) 
     && !nl->HasLength(args,5) ){
    return listutils::typeError("2,3 or 5 arguments expected") ;
  }

  if(nl->HasLength(args,2)){
    ListExpr arg = nl->First(args);
    if(distributed2::DArray::checkType(arg) ){
       if(!Relation::checkType(nl->Second(arg))){
          return listutils::typeError("darray's subtype muts be a relation");
       }
       return nl->Second(arg); // return the relation
    } 
    if(distributed2::DFArray::checkType(arg)){
       return nl->TwoElemList( listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second(nl->Second(arg)));
    }
    return listutils::typeError("first arg is not a d[f]array");
  }
  
  if(nl->HasLength(args,3)){
    ListExpr arg = nl->Third(args);
    if(!listutils::isMap<1>(arg) && !listutils::isMap<2>(arg)){
      return listutils::typeError("third arg is not a function");
    } 
    // extract function result 
    while(!nl->HasLength(arg,1)){
      arg = nl->Rest(arg);
    }
    arg = nl->First(arg);
    if(!Stream<Tuple>::checkType(arg)){
      return listutils::typeError("function result is not a tuple stream");
    }
    ListExpr res =  nl->Second(arg);
    return res;
  }
  // five arguments
  ListExpr arg = nl->Fourth(args);
  if(!listutils::isMap<1>(arg) && !listutils::isMap<2>(arg)){
    return listutils::typeError("fourth arg is not a function");
  }
 
  return nl->TwoElemList(
                      listutils::basicSymbol<Stream<Tuple> >(),
                      nl->Second(arg)); // (tuple (...))
  
}


OperatorSpec DPD4Spec(
  "d[f]array(rela(tuple(X))) x A -> frel(tuple(X)) or  "
  "B x C x (frel(tuple(D) -> stream(tuple(E)) ) -> tuple(E)",
  " DPD4(_,_,_)",
  "Type Map Operator",
  "query DPD4(test) getTypeNL"
);


Operator DPD4Op(
  "DPD4",
  DPD4Spec.getStr(),
  0,
  Operator::SimpleSelect,
  DPD4TM
);

OperatorSpec PDTSSpec(
  "darray(rel(Tuple)) -> Tuple or  "
  "darray(rel(Tuple)) x A -> stream(Tuple)",
  "PDTS(_,_,_)",
  "Type Operator",
  "query PDTS(test) getTypeNL"
);

Operator PDTSOp(
  "PDTS",
  PDTSSpec.getStr(),
  0,
  Operator::SimpleSelect,
  PDTSTM
);

template<class A>
class dpdInfo{

  public:

    dpdInfo(A* _array, 
                      distributed2::DArrayBase* _resultArray, 
                      size_t _workerNumber,
                      distributed2::ConnectionInfo* _ci, 
                      const string& _dmap1funString, 
                      const string& _partitionfunString,
                      ListExpr _relType, 
                      bool _isRel, 
                      bool _isStream, 
                      const string& _dmapfunString,
                      //const string& _dbname, 
                      int _eid):
        array(_array), 
        resultArray(_resultArray), // make undefined when something goes wrong
        numberOfSlots((int) resultArray->getSize()), // O(1)
        numberOfWorkers((int) resultArray->numOfWorkers()), // O(1)
        workerNumber(_workerNumber), 
        ci(_ci),
        dmap1funString(_dmap1funString), 
        partitionfunString(_partitionfunString), 
        targetName(resultArray->getName()),
        sourceName(array->getName()), 
        relType(_relType), 
        isRel(_isRel), 
        isStream(_isStream),
        dmapfunString(_dmapfunString), 
        //dbname(_dbname), 
        eid(_eid),
        runner(0){
          dbname = SecondoSystem::GetInstance()->GetDatabaseName();
          //numberOfWorkers darf nicht 0 sein.
          ci->copy();
          runner = new boost::thread(&dpdInfo::run,this);
    }

    ~dpdInfo(){
        runner->join();
        delete runner;
        ci->deleteIfAllowed();
     }

  private:
     A* array;
     distributed2::DArrayBase* resultArray;
     int numberOfSlots;
     int numberOfWorkers;
     size_t workerNumber;
     distributed2::ConnectionInfo* ci;
     string dmap1funString;
     string partitionfunString;
     string targetName;
     string sourceName;
     ListExpr relType;
     bool isRel;
     bool isStream;
     string dmapfunString;
     string dbname;
     int eid;
     boost::thread* runner;
      
     void run(){
     // dprogress-Teil 
       runpartition();
     // dprogress-Teil
       rundmap();
       //rundmapseq(); 
     // dprogress-Teil
     }
    void runpartition() {
      if(!ci){
           return;
      }

      string cmd = constructQuery();
      if(cmd==""){
        cerr << "worker " << workerNumber 
             << " does not contain any slot" << endl;
        return;
      }
      int err;
      string errMsg;
      double runtime;
      string res;
      ci->simpleCommandFromList(cmd, err,errMsg, res, false, runtime,
                                showCommands, commandLog, false,
                                algInstance->getTimeout());
      if(err!=0){
        cerr << __FILE__ << "@"  << __LINE__ << endl;
        showError(ci,cmd,err,errMsg);
        writeLog(ci,cmd,errMsg);
      } 
    }
    
    
    string constructQuery(){
        string streamstring;
        switch(array->getType()){
           case distributed2::DARRAY : 
                  streamstring = constructDstream();
                  break;
           case distributed2::DFARRAY : 
                  streamstring = constructDFstream();
                  break;
           case distributed2::DFMATRIX : assert(false);
           case distributed2::SDARRAY  : assert(false);
        }
        
        stringstream query;

        query << "(query "
              << " (count "
              << "   ( distribute2tes "
              <<       streamstring
              <<       " " << eid
              <<       " " << partitionfunString
              <<       " " << numberOfSlots
              <<       " " << numberOfWorkers
              <<       " )))";
        return query.str(); 
     }
     
     string constructDstream() {
       // create relation containing the objectnames of the source
        // for this worker
        stringstream ss;
        ss << "(" ; // open value

        for(size_t i=0;i<array->getSize();i++){
           if(array->getWorkerIndexForSlot(i)==workerNumber){
              ss << " (\"" << array->getName() << "_" << i << "\" )" << endl;
           }
        }
        ss << ")"; // end of value list

        string rel = " ( (rel(tuple((T string)))) " + ss.str() + ")";

        // the query in user syntax would be
        // query rel feed projecttransformstream[T] fdistribute7[
        // fun, numberOfSlots, dir+"/"+targetName, TRUE] count

        string stream1 = "(projecttransformstream (feed " + rel + ") T )";
        string stream2 =   "(feedS " + stream1 
                         + "("+nl->ToString(relType) 
                         + " ()))"; 
        string stream3 = stream2;
        
        if(!dmap1funString.empty()){
          stream3 = "("+ dmap1funString + "(consume "+  stream2 + "))";
        }
        
        return stream3;
     }
     
     string constructDFstream() {
       // construct query in nested list form,

        string sourceDir = ci->getSecondoHome(
                showCommands, commandLog) + "/dfarrays/"
                           + dbname + "/" + sourceName + "/";

        // create a relation containing the filenames
        
        ListExpr relTemp = nl->TwoElemList( relType, nl->TheEmptyList());

        ListExpr fnrelType = nl->TwoElemList(
                                 listutils::basicSymbol<Relation>(),
                                 nl->TwoElemList(
                                    listutils::basicSymbol<Tuple>(),
                                    nl->OneElemList(
                                       nl->TwoElemList(
                                            nl->SymbolAtom("T"),
                                          listutils::basicSymbol<FText>()))));

        // build all texts
        bool first = true;
        ListExpr value = nl->TheEmptyList();
        ListExpr last = value;
        for(size_t i=0;i<array->getSize();i++){
            if(array->getWorkerIndexForSlot(i)==workerNumber){
                 ListExpr F  = nl->OneElemList(
                                   nl->TextAtom(sourceDir+sourceName + "_"
                                   + stringutils::int2str(i) + ".bin"));
                 if(first){
                     value = nl->OneElemList(F );
                     last = value;
                     first = false;
                 } else {
                     last = nl->Append(last,F);
                 }
            }
        }
        if(nl->IsEmpty(value)){
           return "";
        }
        ListExpr fnrel = nl->TwoElemList(fnrelType, value);
        ListExpr feed = nl->TwoElemList(nl->SymbolAtom("feed"), fnrel);

        ListExpr stream = nl->ThreeElemList(
                              nl->SymbolAtom("projecttransformstream"),
                              feed,
                              nl->SymbolAtom("T"));

        string streamer = dmap1funString.empty()?"fsfeed5":"createFSrel";

        ListExpr fsfeed = nl->ThreeElemList(
                                nl->SymbolAtom(streamer),
                                stream,
                                relTemp);
      
        ListExpr streamFun = fsfeed;
        if(!dmap1funString.empty()){
           ListExpr sfunl;
           // activate in Distributed2Algebra
           //{ boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx); 
             bool ok = nl->ReadFromString(dmap1funString, sfunl);
             if(!ok){
               cerr << "error in parsing list: " << dmap1funString << endl;
               return "";
             }
           //} activate in Distribute2Algebra 
           streamFun = nl->TwoElemList( sfunl, fsfeed);
        }        
        return nl->ToString(streamFun);
     }
    
    vector<size_t> slotsOfWorker() {
      size_t numOfWorkers = array->numOfWorkers();
       vector<size_t> result;
       for (size_t i = 0; i< (size_t)numberOfSlots; ++i) {
         if (i % numOfWorkers == workerNumber) {
           result.push_back(i);
         }
       }
       return result; // should be moved
    }
    void wait_for_and_delete(vector<boost::thread*>& runners) {
      for( size_t i=0;i< runners.size();i++){
        runners[i]->join();
        delete runners[i];
      }
    }
    // start for each slot a thread that applies the
    // function at the worker that is the slot's home 
    void rundmap() {
      vector<boost::thread*> runners;
      if(isStream) {
        for( size_t slot : slotsOfWorker()) { 
          runners.push_back(
               new boost::thread(&dpdInfo::frun,this,slot));
        } 
      } else {
        for( size_t slot : slotsOfWorker()) {
          runners.push_back(
               new boost::thread(&dpdInfo::drun,this,slot));
        }
      }
      wait_for_and_delete(runners);
    }
    void rundmapseq() {
      if(isStream) {
        for (size_t slot : slotsOfWorker()) {
          frun(slot);
        }
      } else {
        for (size_t slot : slotsOfWorker()) {
          drun(slot);
        }
      }
    }
    
    
    void frun(const int slotNumber) {
         // int maxtries = array->numOfWorkers() * 2; 
           // create name for the slot file
           //string n = array->getObjectNameForSlot(slotNumber);
      string cmd;
           //string funarg;
           //bool error = false;
           //int numtries = maxtries;
           //set<size_t> usedWorkers;
           //bool reconnect = true;
      //do {
             if(!ci){
               return;
             }


             // create the target directory 
      string targetDir = ci->getSecondoHome(
            showCommands, commandLog)+"/dfarrays/"+dbname+"/"
                       + targetName + "/" ;

      string cd = "query createDirectory('"+targetDir+"', TRUE)";
      int err;
      string errMsg;
      string r;
      double runtime;
      ci->simpleCommand(cd, err, errMsg,r, false, runtime,
                        showCommands, commandLog, false,
                        algInstance->getTimeout());
      if(err){
        cerr << "creating directory failed, cmd = " << cd << endl;
        cerr << "message : " << errMsg << endl;
        cerr << "code : " << err << endl;
        cerr << "meaning : " << SecondoInterface::GetErrorMessage(err)
             << endl;
        writeLog(ci,cd,errMsg);
        //writeLog(ci,cmd,errMsg);
        //error = true;
      } else {
        string fname2 =   targetDir
                      + targetName + "_" + stringutils::int2str(slotNumber)
                      + ".bin";

               // if the result of the function is a relation, we feed it 
               // into a stream to fconsume it
               // if there is a non-temp-name and a dfs is avaiable,
               // we extend the fconsume arguments by boolean values
               // first : create dir, always true
               // second : put result to dfs 
      string aa ="";
      if(filesystem){
         aa = " TRUE TRUE ";
      }
      cmd = getcmd(slotNumber, false);
      
      // TODO isRel-Teil kann entfallen
      if(isRel) {
        assert(false);
        cmd =   "(query (count (fconsume5 (feed " + cmd +" )'" 
              + fname2+"' "+aa+")))";
      } else {
        cmd = "(query (count (fconsume5 "+ cmd +" '"+ fname2
            + "'"+aa+" )))";
      }

      ci->simpleCommandFromList(cmd,err,errMsg,r,false, runtime,
                              showCommands, commandLog, false,
                              algInstance->getTimeout());
      if((err!=0) ){ 
         showError(ci,cmd,err,errMsg);
         writeLog(ci,cmd,errMsg);
         //error = true;
      }
    }
       
    }
    
    
    string getcmd(const int slotNumber, bool replaceWrite2) {
      string funarg1 = "(feedtes "  + stringutils::int2str(eid) + " " 
                                    + stringutils::int2str(slotNumber) + ")";
      //string funarg2 = stringutils::int2str(slotNumber);

      vector<string> funargs;
      funargs.push_back(funarg1);
      //funargs.push_back(funarg2);
             // we convert the function into a usual commando
             
      ListExpr cmdList = Distributed3Algebra::fun2cmd(dmapfunString, funargs);
            // we replace write3 symbols in the commando for dbservice support
      string n = array->getObjectNameForSlot(slotNumber);
      if (replaceWrite2) {
        string name2 = resultArray->getObjectNameForSlot(slotNumber);
        cmdList = Distributed3Algebra::replaceWrite(cmdList, "write2",name2);
      }
      cmdList = Distributed3Algebra::replaceWrite(cmdList, "write3",n);
      return nl->ToString(cmdList);
    }
    
    
    void drun(const int slotNumber) {
      string funcmd = getcmd(slotNumber,true);
      string name2 = resultArray->getObjectNameForSlot(slotNumber);
      string cmd = "(let " + name2 + " = " + funcmd + ")";

      int err; string errMsg; string r;
      double runtime;
      ci->simpleCommandFromList(cmd,err,errMsg,r,false, runtime,
                                showCommands, commandLog, false,
                                algInstance->getTimeout());
      if(err){
        cerr << __FILE__ << "@"  << __LINE__ << endl;
        showError(ci,cmd,err,errMsg);
        cerr << errMsg << " result could not be written locally for slot " 
                       << slotNumber
             << "\ncmd was: " << cmd;
        writeLog(ci, cmd,errMsg);
        return;
      }  
    }     
};

/*
14a Operator partitiondmap

*/
   
ListExpr partitiondmapTM(ListExpr args){
   
  PartitiondmapTypeMapper tm{args};
  //tm.printValues();

  return tm.typeMapping();
}



template<class A>
int dpdVMT(Word* args, Word& result,
                   int message, Word& local, Supplier s) {
  

   A* array = (A*) args[0].addr;
   CcString* ccname = (CcString*) args[1].addr;
   CcInt* newNumberOfSlots;
   string dmap1funString="";
   string partitionfunString="";
   string dmap2funString="";
   bool isRel;
   bool isStream;
   ListExpr relType; // setTupleType

   if(qp->GetNoSons(s)==9){ 
      // without additional function
      // third arg is the function, we get the text
      newNumberOfSlots = (CcInt*) args[3].addr;
      partitionfunString = ((FText*) args[5].addr)->GetValue();
      dmap2funString = ((FText*) args[6].addr)->GetValue();
      isRel = ((CcBool*) args[7].addr)->GetValue();
      isStream = ((CcBool*) args[8].addr)->GetValue();
      relType = nl->Second(qp->GetType(qp->GetSon(s,0)));
   } else if(qp->GetNoSons(s)==11){
      // args[2], args[3] and args[5] are the functions
      newNumberOfSlots = (CcInt*) args[4].addr;
      dmap1funString = ((FText*) args[6].addr)->GetValue(); 
      partitionfunString = ((FText*) args[7].addr)->GetValue();
      dmap2funString = ((FText*) args[8].addr)->GetValue();
      isRel = ((CcBool*) args[9].addr)->GetValue();
      isStream = ((CcBool*) args[10].addr)->GetValue();
      relType = nl->TwoElemList(listutils::basicSymbol<Relation>(),
                                nl->Second(qp->GetType(qp->GetSon(s,2)))); 
      
   } else {
      assert(false); // invalid number of arguments
      ccname = 0;
      newNumberOfSlots = 0;
   }
   string relTypeString = nl->ToString(relType); // setTupleType
   

   /************** setting the result type **********************/
   
   result = qp->ResultStorage(s);
   distributed2::DArrayBase* resultArray; // supertype of DFArray and DArray
   if(isStream){
     resultArray = (distributed2::DFArray*) result.addr;
   } else {
     resultArray  = (distributed2::DArray*)  result.addr;
   }

   /************* Checks from partition in order to return early **********/
   
   int numberOfSlots = array->getSize();

   if(newNumberOfSlots->IsDefined() &&
      newNumberOfSlots->GetValue() > 0){
      numberOfSlots = newNumberOfSlots->GetValue();
   }
   
   if(!array->IsDefined() || !ccname->IsDefined()){
       resultArray->makeUndefined();
       return 0;
   }

   string targetName = ccname->GetValue();
   if(targetName.size()==0){
      targetName = algInstance->getTempName();
   }
   //cout << "\ntargetName: " << targetName;
   
   if(!stringutils::isIdent(targetName)){
     resultArray->makeUndefined();
     return 0;
   }

   if(targetName == array->getName()){
     resultArray->makeUndefined();
     return 0;
   }
   
   resultArray->set((size_t)numberOfSlots, targetName, array->getWorker());
  
  
    /************************ TES *********************/
  
  //TESManager tesm = TESManager::getInstance();
  //int eid = tesm.getExchangeID();
  int eid = TESManager::getInstance().getExchangeID();
  /*
    minimale Prüfung der worker sollte schon stattfinden.
  */
  if (!TESManager::equalWorkers(TESManager::getInstance().getWorkerVector(),
                                                     array->getWorker())) {
    resultArray->makeUndefined();
    // TODO
    // TESManager::printDifference(tesm.getWorkerVector(), array->getWorker());
    cerr << "Workers of TES and D[F]Array differ ";
    return 0; 
  }
  
  string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
  
  bool showCommands = false;
  distributed2::CommandLog commandLog;
  
  {
    int err;
    string errMsg;
    string res2;
    double runtime;
    
    auto workers = TESManager::getInstance().getWorkers();
    assert (workers);
    /*
    for (WorkerConfig *worker = workers();
        worker != nullptr; worker = workers()) {
      ConnectionInfo* ci = worker->connection;
    */ 
   for(size_t workerNumber=0;workerNumber<array->numOfWorkers();
                                                             workerNumber++){
     distributed2::DArrayElement de = array->getWorker(workerNumber);
     distributed2::ConnectionInfo* ci = 
         algInstance->getWorkerConnection(de,dbname);
      if(ci){
        string cmd = "(query (setTupleType " + stringutils::int2str(eid) + 
                                        " (" +     relTypeString + " ()) " +
                                stringutils::int2str(numberOfSlots) + ") )";
      
        ci->simpleCommandFromList(cmd, err, errMsg, res2, false, runtime,
                            showCommands, commandLog, false,
                            algInstance->getTimeout());
        if(err){
          cerr << "setTupleType failed on worker " 
               << workerNumber << endl;
          writeLog(ci, cmd,errMsg); 
          return 0;
        }
      }
    }
  }
  ListExpr arrayRelType = nl->Second(qp->GetType(qp->GetSon(s,0)));
  vector<dpdInfo<A>*> infos;
   
  auto workers = TESManager::getInstance().getWorkers();
  for (WorkerConfig *worker = workers();
       worker != nullptr; worker = workers()) {
    distributed2::ConnectionInfo* ci = worker->connection;
    int workerNumber = worker->workernr; 
    if(ci){
       dpdInfo<A>* info = 
         new dpdInfo<A>(array, 
                              resultArray, 
                              workerNumber, 
                              ci,
                              dmap1funString, 
                              partitionfunString,
                              arrayRelType, 
                              isRel, 
                              isStream, 
                              dmap2funString, 
                              eid);  

       infos.push_back(info);
    }
  }
  
  for(size_t i=0;i<infos.size();i++){
      delete infos[i];
  }

  return 0;
}


ValueMapping partitiondmapVM[] = {
   dpdVMT<distributed2::DArray>,
   dpdVMT<distributed2::DFArray>,
};

int partitiondmapSelect(ListExpr args){
  ListExpr arg1 = nl->First(args);
  if(distributed2::DArray::checkType(arg1)) return 0;
  if(distributed2::DFArray::checkType(arg1)) return 1;
  return -1;
}

OperatorSpec partitiondmapSpec(
 "d[f]array(rel(Tuple)) x string x (Tuple->int) x int "
 "x (stream(Tuple) -> X) -> d[f]array",
  "_ partitiondmap[_,_,_,_]",
  "Redistributes the contents of a d[f]array value "
  "according to the first function "
  "and applies the second function to the newly distributed tuples. "
  "The string argument suggests a local name on the workers, "
  "the int argument determines the number of slots "
  "of the redistribution. If this value is smaller or equal to "
  "zero, the number of slots is overtaken from the d[f]array argument.",
  "query d45strassen partitiondmap[\"zdrun451\", "
  "hashvalue(.Name,117), 77, . consume]"
);

Operator partitiondmapOp(
  "partitiondmap",
  partitiondmapSpec.getStr(),
  2,
  partitiondmapVM,
  partitiondmapSelect,
  partitiondmapTM
);

ListExpr dmapPdmapTM(ListExpr args){

  DmapPdmapTypeMapper tm{args};
  //tm.printValues();
   
  return tm.typeMapping();
}

int dmapPdmapSelect(ListExpr args){
  ListExpr arg1 = nl->First(args);
  if(distributed2::DArray::checkType(arg1)) return 0;
  if(distributed2::DFArray::checkType(arg1)) return 1;
  return -1;
}

ValueMapping dmapPdmapVM[] = {
     dpdVMT<distributed2::DArray>,
     dpdVMT<distributed2::DFArray>,
};

OperatorSpec dmapPdmapSpec(
 "d[f]array(rel(Tuple)) x string  -> d[f]array",
  "_ dmapPdmap[_]",
  "Redistributes the contents of a d[f]array value "
  "according to the first function "
  "and applies the second function to the newly distributed tuples. "
  "The string argument suggests a local name on the workers, "
  "the int argument determines the number of slots "
  "of the redistribution. If this value is smaller or equal to "
  "zero, the number of slots is overtaken from the d[f]array argument.",
  "query d45strassen dmapPdmap[\"zdrun451\"]"
);

Operator dmapPdmapOp(
  "dmapPdmap",
  dmapPdmapSpec.getStr(),
  2,
  dmapPdmapVM,
  dmapPdmapSelect,
  dmapPdmapTM
);


/*
3 Implementation of the Algebra

*/
Distributed3Algebra::Distributed3Algebra(){

   AddOperator(&partitiondmapOp);
   partitiondmapOp.SetUsesArgsInTypeMapping();
   AddOperator(&dmapPdmapOp);
   dmapPdmapOp.SetUsesArgsInTypeMapping();
   AddOperator(&PDTSOp);
   AddOperator(&DPD1Op);
   AddOperator(&DPD2Op);
   AddOperator(&DPD4Op);
   AddOperator(&Distribute2TES::distribute2TES);
   AddOperator(&FeedTES::feedTES);
   FeedTES::feedTES.SetUsesArgsInTypeMapping();
   AddOperator(&StartTESClient::startTESClient);
   AddOperator(&StartTESServer::startTESServer);
   AddOperator(&ResetTES::resetTES);
   AddOperator(&KillTES::killTES);
   AddOperator(&SetupTES::setupTES);
   AddOperator(&SetTupleType::setTupleType);
   AddOperator(&StartLoopbackTESClient::startLoopbackTESClient);
   //AddOperator(&TESTests::testests);
 
}


Distributed3Algebra::~Distributed3Algebra(){
   boost::lock_guard<boost::mutex> guard(mtx);
   for(size_t i=0;i<connections.size();i++){
      delete connections[i];
   }
   connections.clear();
   //closeAllWorkers();
   TESManager::getInstance().reset();
   connections.clear();
   TESManager::getInstance().reset();

}
extern "C"
Algebra*
   InitializeDistributed3Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {
  
  distributed3::algInstance = new distributed3::Distributed3Algebra();
  distributed3::Distributed3Algebra::setAlgebra(distributed3::algInstance);
  distributed3::showCommands = false; 
  return distributed3::algInstance;
}

}
