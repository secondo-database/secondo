/*

*/

#include <StandardTypes.h>
#include "Stream.h"
#include <ListUtils.h>
#include <vector>
#include <QueryProcessor.h>
#include "DmapPdmap.h"
#include "Algebras/Distributed2/CommandLog.h"
#include "tes/TESManager.h"

using namespace std;

namespace distributed3 {

bool arraySubtypeEqualsFunctionArgument(ListExpr arrayType, 
                                        ListExpr dmap1Type) {
  ListExpr expFunArg =   distributed2::DArray::checkType(arrayType)
                        ?nl->Second(arrayType)
                        : nl->TwoElemList(
                               listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second(nl->Second(arrayType)));

  if(!nl->Equal(expFunArg, nl->Second(dmap1Type))){
     return false;
  }
  return true;
}

/** returns the result type of the functions type */

ListExpr getResultType(ListExpr function) {
  ListExpr res = fu nction;
  while(!nl->HasLength(res,1)){
     res = nl->Rest(res);
  }
  return nl->First(res);
}
/*
  returns the last argument of a function as a ListExpr
  TODO not tested for the value part of a function. 
    
*/
ListExpr getLastArgumentOf(ListExpr function) {
  ListExpr d1UsedArg = function;
  while(!nl->HasLength(d1UsedArg,2)){
    d1UsedArg = nl->Rest(d1UsedArg);
  }
  return nl->First(d1UsedArg);
}
/*
  replaces the occurrance of a type operator in the value part of a function
  (functionValue) by replacement. 
  TODO next part required?
  returns a function with one argument. If functionValue contains two arguments
  the second is used.
  
*/
ListExpr replaceTypeOperator(ListExpr functionValue, ListExpr replacement) {

  ListExpr ddarg = nl->HasLength(functionValue,3)
                    ?nl->Second(functionValue)
                    :nl->Third(functionValue);
                    
  ListExpr result = nl->HasLength(functionValue,3)
                   ?nl->Third(functionValue)
                   :nl->Fourth(functionValue);
                   
  return nl->ThreeElemList(
                       nl->First(functionValue),
                       nl->TwoElemList(
                             nl->First(ddarg), // elem_X
                             replacement),
                       result
                     );
}

ListExpr DmapPdmap::typeMapping(ListExpr args){
  // TODO delegieren an TypeMapper-Objekt
  // DmapPdmapTypeMapper : TypeMapper, der alle Hilfsmethoden enthält.
  std::cout << "\nargs in dmapPdmap :";
  nl->WriteListExpr(args);
  std::string err = "expected: d[f]array(rel(tuple(X))) x string x "
               "([fs]rel(tuple(X)) -> stream(tuple(Y))) x "
               "(tuple(Y)->int) x int x"
               "(stream(tuple(Y)) -> Z)";

 if(!nl->HasLength(args,6)){
    return listutils::typeError(err + " (wrong number of args)");
  }
   
  ListExpr array = nl->First(args);  // array
  ListExpr name = nl->Second(args); // name of the result
  ListExpr dmap1 = nl->Third(args); // function
  ListExpr partitionfunction = nl->Fourth(args);  // redistribution function
  ListExpr numberOfSlots = nl->Fifth(args);  // size of the result
  ListExpr dmap2 = nl->Sixth(args); 

  // check UsesTypes in Type Mapping
  if(   !nl->HasLength(array,2) 
     || !nl->HasLength(name,2) 
     || !nl->HasLength(dmap1,2)
     || !nl->HasLength(partitionfunction,2) 
     || !nl->HasLength(numberOfSlots,2) 
     || !nl->HasLength(dmap2,2)) {
    return listutils::typeError("internal error");
  } 

  // check Types
  ListExpr arrayType = nl->First(array);
  ListExpr nameType = nl->First(name);
  ListExpr dmap1Type = nl->First(dmap1); // (map ...)
  ListExpr partitionfunctionType = nl->First(partitionfunction);
  ListExpr numberOfSlotsType = nl->First(numberOfSlots);
  ListExpr dmap2Type = nl->First(dmap2);
  
  // first arg array
  if(   !distributed2::DArray::checkType(arrayType) 
     && !distributed2::DFArray::checkType(arrayType)){
     return listutils::typeError(err + " (first arg is not a d[f]array)");
  }
  ListExpr relation = nl->Second(arrayType);
  if(!Relation::checkType(relation)){
     return listutils::typeError(err + " (array subtype is not a relation)");
  }
  // second arg name
  if(!CcString::checkType(nameType)){
     return listutils::typeError(err + " (second arg is not a string)");
  }
  // third arg dmap1
  if(!listutils::isMap<1>(dmap1Type) 
  //&& !listutils::isMap<2>(dmap1Type)
  ){
     return listutils::typeError(err + " (third arg is not a function)");
  }
  // fourth arg partition
  if(   !listutils::isMap<1>(partitionfunctionType) 
     //&& !listutils::isMap<2>(partitionfunctionType)
     ){
     return listutils::typeError(err + " (fourth arg is not a function)");
  }
  // fifth arg number of slots
  if(!CcInt::checkType(numberOfSlotsType)){
     return listutils::typeError(err + " (fifth arg is not an int)");
  }
  // sixth arg dmap2
  if(!listutils::isMap<1>(dmap2Type) 
  //&& !listutils::isMap<2>(dmap2Type)
  ){
    return listutils::typeError(err + " (sixth arg is not a function)");
  }
  /*
   bool addedFunArg= false;
  if(listutils::isMap<1>(dmapfunctionType)){
     dmapfunctionType = nl->FourElemList( nl->First(dmapfunctionType),
                            nl->Second(dmapfunctionType),
                            listutils::basicSymbol<CcInt>(), 
                            nl->Third(dmapfunctionType));
     addedFunArg = true;
  }

  if(!listutils::isMap<2>(dmapfunctionType)){
    return listutils::typeError(
           err + " fifth arg should be a function with one or two arguments");
  } 
  */
  
  // check function arguments and results
  if (!arraySubtypeEqualsFunctionArgument(arrayType, dmap1Type))
    return listutils::typeError(" argument of function does not "
                                 "fit the array type.");
  
    
  if(!Stream<Tuple>::checkType(getResultType(dmap1Type))){
    return listutils::typeError("function result is not a tuple stream");
  }


  if(!CcInt::checkType(getResultType(partitionfunctionType))){
    return listutils::typeError("result for distribution function is "
                                "not an int");
  }

  if(!Tuple::checkType(getLastArgumentOf(partitionfunctionType))){
    return listutils::typeError("argument of the distribution function is "
                                "not a tuple");
  }
  
  if(!nl->Equal(nl->Second(getResultType(dmap1Type)),
                getLastArgumentOf(partitionfunctionType))){
    return listutils::typeError("type mismatch between result of the function "
                            "and the argument of the distribution function");
  }

  // if dmap1Type is defined to have two arguments, we ensure that the 
  // second argument is unused
  // within the whole function definition

  if(listutils::isMap<2>(dmap1Type)){
     std::string arg2Name = nl->SymbolValue(
                                  nl->First(nl->Third(nl->Second(dmap1))));
     if(listutils::containsSymbol(nl->Fourth(nl->Second(dmap1)), arg2Name)){
        return listutils::typeError("Usage of the second argument in "
                                    "function is not allowed");
     }
  }
  
  if(!nl->Equal(getResultType(dmap1Type), nl->Second(dmap2Type))){
     return listutils::typeError(" argument of third function does not "
                                 "fit the result type of the first function.");
  }
  
  // dmap1 replacement
  ListExpr expFunArg =   distributed2::DArray::checkType(arrayType)
                        ?relation
                        : nl->TwoElemList(
                               listutils::basicSymbol<distributed2::fsrel>(),
                               nl->Second(relation));
                                            
  ListExpr rDmap1FunDef = replaceTypeOperator(nl->Second(dmap1), expFunArg);
  
  // partition remplacement
  ListExpr partitionfunquery = nl->Second(partitionfunction);
   /*
   if(listutils::isMap<2>(partitionfunctionType)){
     std::string arg1Name = 
             nl->SymbolValue(nl->First(nl->Second(partitionfunquery)));
     if(listutils::containsSymbol(nl->Fourth(partitionfunquery),arg1Name)){
        return listutils::typeError("Usage of the first argument is not "
                                    "allowed within the distribution function");
     }
   }
   */

  ListExpr rPartitionFunDef = replaceTypeOperator(partitionfunquery, 
                                          nl->Second(getResultType(dmap1Type)));
  // dmap2 replacement           
  ListExpr dmap2rfun = replaceTypeOperator(nl->Second(dmap2),
                                           nl->Second(dmap2Type));
  
  /*****************  Determining the result type    *************/

  //ListExpr dmap2funRes = nl->Third(dmap2Type);
  ListExpr dmap2funRes = getResultType(dmap2Type);
  // allowed result types are streams of tuples and
  // non-stream objects
  bool isRel = Relation::checkType(dmap2funRes);
  bool isStream = Stream<Tuple>::checkType(dmap2funRes);

  if(listutils::isStream(dmap2funRes) && !isStream){
    return listutils::typeError("function produces a stream of non-tuples.");
  }
  
  // compute the subtype of the resulting array
  if(isStream){
    dmap2funRes = nl->TwoElemList(
               listutils::basicSymbol<Relation>(),
               nl->Second(dmap2funRes));
  }
  
  // determine the result array type
  // is the origin function result is a tuple stream,
  // the result will be a dfarray, otherwise a darray
  ListExpr resType = nl->TwoElemList(
               isStream?listutils::basicSymbol<distributed2::DFArray>()
                       :listutils::basicSymbol<distributed2::DArray>(),
               dmap2funRes);
      

  ListExpr res =  nl->ThreeElemList(
                   nl->SymbolAtom(Symbols::APPEND()),
                   nl->FiveElemList(
                     nl->TextAtom( nl->ToString(rDmap1FunDef)), // dmap1
                     nl->TextAtom(nl->ToString(rPartitionFunDef)),  // partition
                     nl->TextAtom(nl->ToString(dmap2rfun)),  //  dmap2
                     nl->BoolAtom(isRel),               //  dmap2
                     nl->BoolAtom(isStream)),           //  dmap2
                   resType);  
                     
  std::cout << "\nres: ";
  nl->WriteListExpr(res);
  std::cout << "\nrDmap1FunDef: ";
  nl->WriteListExpr(rDmap1FunDef);
  std::cout << "\nrPartitionFunDef: ";
  nl->WriteListExpr(rPartitionFunDef);
  std::cout << "\ndmap2rfun: ";
  nl->WriteListExpr(dmap2rfun);
  return res;

}

template<class A>
class dmapPdmapInfo{

  public:

    dmapPdmapInfo(A* _array,
                  distributed2::DArrayBase* _resultArray,
                  size_t _workerNumber,
                  distributed2::ConnectionInfo* _ci, 
                  const string& _dmap1funString, 
                  const string& _partitionfunString,
                  ListExpr _relType,
                  bool _isRel, 
                  bool _isStream, 
                  const string& _dmap2funString,
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
        dmap2funString(_dmap2funString), 
        //dbname(_dbname), 
        eid(_eid),
        runner(0){
          dbname = SecondoSystem::GetInstance()->GetDatabaseName();
          algInstance = Distributed3Algebra::getAlgebra();
          showCommands = false;
          ci->copy();
          runner = new boost::thread(&dmapPdmapInfo::run,this);
        }

    ~dmapPdmapInfo(){
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
     string dmap2funString;
     string dbname;
     int eid;
     boost::thread* runner;
     Distributed3Algebra* algInstance;
     // provided by Distributed2Algebra for free:
     bool showCommands;
     distributed2::CommandLog commandLog;
     
    void run(){
      runpartition();
      rundmap(); 
    }
     
    void runpartition() {
      if(!ci){
           return;
      }

      string cmd = constructQuery();
     // cout << "cmd:\n";
     // cout << cmd;
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
        // writeLog(ci,cmd,errMsg); // activate in Distributed2Algebra
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
           // activate as part of Distributed2Algebra:
           //{ boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx); 
             bool ok = nl->ReadFromString(dmap1funString, sfunl);
             if(!ok){
               cerr << "error in parsing list: " << dmap1funString << endl;
               return "";
             }
           //} // activate as part of Distributed2Algebra
           streamFun = nl->TwoElemList( sfunl, fsfeed);
        }
        //cout << "streamFun in constructDFStream:";
        //nl->WriteListExpr(streamFun);
        
        return nl->ToString(streamFun);
     }
     
     vector<size_t> slotsOfWorker() {
      size_t numOfWorkers = array->numOfWorkers();
       vector<size_t> result;
       for (size_t i = 0; i< (size_t)numberOfSlots; ++i) {
         if (i % numOfWorkers == workerNumber) {
           //cout << "\nworkerNumber " << workerNumber << " mit slot: " 
           //<< i; // 
           result.push_back(i); // für workerNumber 12 slots 12 und 52
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
               new boost::thread(&dmapPdmapInfo::frun,this,slot));
        } 
      } else {
        for( size_t slot : slotsOfWorker()) {
          runners.push_back(
               new boost::thread(&dmapPdmapInfo::drun,this,slot));
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
    cout << "\nin frun: ";
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
        //writeLog(ci,cd,errMsg); // TODO activate in Distributed2Algebra
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
      bool filesystem = false; // delete in Distributed2Algebra
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
  cout << "\ncmd: \n";
  cout << cmd;
      ci->simpleCommandFromList(cmd,err,errMsg,r,false, runtime,
                              showCommands, commandLog, false,
                              algInstance->getTimeout());
      if((err!=0) ){ 
         showError(ci,cmd,err,errMsg);
         //writeLog(ci,cmd,errMsg); // TODO activate in Distributed2Algebra
         //error = true;
      }
    }
      /*
          if(error){
             mapper->changeConnection(ci,nr,cmd,err,errMsg,reconnect,
                                      numtries,usedWorkers);
             if(!ci){
                cerr << "change worker has returned 0, give up" << endl;
                return;
             }
           } else { // successful finished command
           
#ifdef DPROGRESS
              ci->getInterface()
              ->removeMessageHandler(algInstance->progressObserver->
               getProgressListener(mapper->array->getObjectNameForSlot(nr), 
               mapper->array->getWorkerForSlot(nr)));
               algInstance->progressObserver->
         commitWorkingOnSlotFinished(mapper->array->getObjectNameForSlot(nr), 
         mapper->array->getWorkerForSlot(nr));
#endif
           
                 return;   
           }
           // nexttrie
         //  usleep(100);
       //  } while(numtries>0); // repeat  
       */     
    }
    //TODO anders als in Distributed3Algebra::partitiondmapInfo
    string getcmd(const int slotNumber, bool replaceWrite2) {
      string funarg1 = "(feedtes "  + stringutils::int2str(eid) + " " 
                                    + stringutils::int2str(slotNumber) + ")";
      //string funarg2 = stringutils::int2str(slotNumber);
   //cout << "\nfunarg1, funarg2: " << funarg1 << ", " << funarg2;
      vector<string> funargs;
      funargs.push_back(funarg1);
      //funargs.push_back(funarg2);
             // we convert the function into a usual commando
      cout << "\ndmap2funSTring: " << dmap2funString;
      ListExpr cmdList = Distributed3Algebra::fun2cmd(dmap2funString, funargs);
             // we replace write3 symbols in the commando for dbservice support
      cout << "\ncmdList: ";
      nl->WriteListExpr(cmdList);
      string n = array->getObjectNameForSlot(slotNumber);
      if (replaceWrite2) {
        string name2 = resultArray->getObjectNameForSlot(slotNumber);
        cmdList = Distributed3Algebra::replaceWrite(cmdList, "write2",name2);
      }
      cout << "\ncmdList: ";
      nl->WriteListExpr(cmdList);
      cmdList = Distributed3Algebra::replaceWrite(cmdList, "write3",n);
   cout << "\ncmdList";
   nl->WriteListExpr(cmdList);
      return nl->ToString(cmdList);
    }
    
    
    void drun(const int slotNumber) {
      cout << "\nin drun";
      cout << "\nstotNumner: " << slotNumber;
      string name2 = resultArray->getObjectNameForSlot(slotNumber);
      string cmd = "(let " + name2 + " = " + getcmd(slotNumber,true) + ")";
      cout << "\ncmd :" << cmd;
          
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
        // writeLog(ci, cmd,errMsg); // TODO activate in Distributed2Algebra
        return;
      }  
    }
};


template<class A>
int DmapPdmap::valueMapping(Word* args, Word& result,
                   int message, Word& local, Supplier s) {
  
    /*
#ifdef DPROGRESS
   algInstance->progressObserver->mappingCallback(qp->GetId(s), array);
#endif
*/
   A* array = (A*) args[0].addr;
   Distributed3Algebra* algInstance = Distributed3Algebra::getAlgebra();
   CcString* ccname = (CcString*) args[1].addr;
   CcInt* newNumberOfSlots;
   string dmap1funString="";
   string partitionfunString="";
   string dmap2funString="";
   bool isRel;
   bool isStream;
   ListExpr relType;

   if(qp->GetNoSons(s)==9){  // TODO could be partitiondmapVMT
      // without additional function
      // third arg is the function, we get the text
      newNumberOfSlots = (CcInt*) args[3].addr;
      partitionfunString = ((FText*) args[4].addr)->GetValue();
      relType = nl->Second(qp->GetType(qp->GetSon(s,0)));
   } else if(qp->GetNoSons(s)==11){
      // args[2], args[3] and args[5] are the functions
      newNumberOfSlots = (CcInt*) args[4].addr;
      dmap1funString = ((FText*) args[6].addr)->GetValue(); 
      partitionfunString = ((FText*) args[7].addr)->GetValue();
      dmap2funString = ((FText*) args[8].addr)->GetValue();
      isRel = ((CcBool*) args[9].addr)->GetValue();
      isStream = ((CcBool*) args[10].addr)->GetValue();
      relType = nl->TwoElemList(listutils::basicSymbol<Relation>(),//etTupleType
                                nl->Second(qp->GetType(qp->GetSon(s,2)))); 
      
   } else {
      assert(false); // invalid number of arguments
      ccname = 0;
      newNumberOfSlots = 0;
   }
   string relTypeString = nl->ToString(relType); // setTupleType
   
   cout << "\nrelType: ";
   nl->WriteListExpr(relType);

   /************** setting the result type **********************/
   
   result = qp->ResultStorage(s);
   distributed2::DArrayBase* resultArray; // supertype of DFArray and DArray
   if(isStream){
     resultArray = (distributed2::DFArray*) result.addr;
   } else {
     resultArray  = (distributed2::DArray*)  result.addr;
   }

   /************* Checks from partition in order to return early **************/
   
   // TODO hier weiter
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
   for(size_t workerNumber=0;workerNumber<array->numOfWorkers();workerNumber++){
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
          //writeLog(ci, cmd,errMsg); // TODO activate in Distributed2Algebra
          return 0;
        }
      }
    }
  }
  ListExpr arrayRelType = nl->Second(qp->GetType(qp->GetSon(s,0)));
  vector<dmapPdmapInfo<A>*> infos;
   
  auto workers = TESManager::getInstance().getWorkers();
  for (WorkerConfig *worker = workers();
       worker != nullptr; worker = workers()) {
    distributed2::ConnectionInfo* ci = worker->connection;
    int workerNumber = worker->workernr; 
    if(ci){
       dmapPdmapInfo<A>* info = 
         new dmapPdmapInfo<A>(array, 
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


int DmapPdmap::dmapPdmapSelect(ListExpr args){
  ListExpr arg1 = nl->First(args);
  if(distributed2::DArray::checkType(arg1)) return 0;
  if(distributed2::DFArray::checkType(arg1)) return 1;
  return -1;
}

ValueMapping DmapPdmap::dmapPdmapVM[] = {
     valueMapping<distributed2::DArray>,
     valueMapping<distributed2::DFArray>,
};

OperatorSpec DmapPdmap::operatorSpec(
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

Operator DmapPdmap::dmapPdmapOp(
  "dmapPdmap",
  DmapPdmap::operatorSpec.getStr(),
  2,
  DmapPdmap::dmapPdmapVM,
  DmapPdmap::dmapPdmapSelect,
  DmapPdmap::typeMapping
);


}
