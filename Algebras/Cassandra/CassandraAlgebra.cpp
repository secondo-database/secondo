/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2014, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 Jan Kristof Nidzwetzki

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 1 Includes and defines

*/
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <sys/time.h>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "ListUtils.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "Attribute.h"
#include "../../Tools/Flob/Flob.h"
#include "SuffixTreeAlgebra.h"
#include "FTextAlgebra.h"
#include "Stream.h"
#include "Progress.h"

#include "CassandraAdapter.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;


/*
~A Macro useful for debugging ~

*/

//#define __TRACE__ cout << __FILE__ << "@" << __LINE__ << endl;
#define __TRACE__



//namespace to avoid name conflicts
namespace cassandra
{

/*
2.1 Operator ~Sleep~

This operator sleeps for a configurable time

2.1.1 Type mapping function of operator ~sleep~

Type mapping for ~sleep~ is

----  ((stream (tuple ((x1 t1)...(xn tn))) int)   ->
              ((stream (tuple ((x1 t1)...(xn tn))))

  or ((stream T) int) -> (stream T)  for T in kind DATA
----

*/
ListExpr SleepTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 2){
    return listutils::typeError("two arguments expected");
  }

  string err = " stream(tuple(...) x int or stream(DATA) x int expected";

  ListExpr stream = nl->First(args);
  ListExpr delay = nl->Second(args);

  if(( !Stream<Tuple>::checkType(stream) &&
       !Stream<Attribute>::checkType(stream) ) ||
     !CcInt::checkType(delay) ){
    return listutils::typeError(err);
  }
  return stream;
}

/*
2.1.2 Generic Cost Estimation Function

This class will forward the progess of the predecessor

*/
class ForwardCostEstimation : public CostEstimation {

public:
  
  ForwardCostEstimation() {
  }
  
  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {
   
    ProgressInfo p1;
    Supplier sonOfFeed;
    
    sonOfFeed = qp->GetSupplierSon(supplier, 0);
    
    if ( qp->RequestProgress(sonOfFeed, &p1) ) {
      pRes->Card = p1.Card;
      pRes->CopySizes(p1);
      // Add delay per tuple (default 0)
      pRes->Time = p1.Time + (p1.Card * getDelay(localInfo));
      pRes->Progress = p1.Progress;
      pRes->BTime = p1.BTime;
      pRes->BProgress = p1.BProgress;

      return YIELD;
    } else {
      return CANCEL;
    }
    
    return CANCEL;
  }
  
  // Template Method, can be used in subclasses
  virtual int getDelay(void* localInfo) { return 0; }
  
/*
2.1.2 init our class

*/
  virtual void init(Word* args, void* localInfo) {
    
  }
};

struct SleepLocalInfo
{
  SleepLocalInfo( const int numDelay = 0 ): delay( numDelay ) {
      
  }

  int delay; // in ms
};

/*
2.1.2 Specialized Cost Estimation function for operator ~sleep~

Multiplies the delay with the estimated time of 
the predecessor

*/
class SleepCostEstimation : public ForwardCostEstimation {
  
public:
  
  virtual int getDelay(void* localInfo) { 
    SleepLocalInfo* li = (SleepLocalInfo*) localInfo;
    
    if(li == NULL) {
      return 1;
    }
    
    // delay per tuple in ms
    return li -> delay;     
  }
};

CostEstimation* SleepCostEstimationFunc() {
  return new SleepCostEstimation();
}


/*
2.1.3 Value mapping function of operator ~sleep~

*/
int Sleep(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SleepLocalInfo *sli; 
  Word tupleWord;

  sli = (SleepLocalInfo*)local.addr;

  switch(message)
  {
    case OPEN: 

      if ( sli ) delete sli;

      sli = 
        new SleepLocalInfo( ((CcInt*)args[1].addr)->GetIntval() );
      local.setAddr( sli );

      qp->Open(args[0].addr);
      return 0;

    case REQUEST:

      // Operator not ready
      if ( ! sli ) {
        return CANCEL;
      }
      
      // Delay is in ms 
      usleep(sli -> delay * 1000);
      
      
      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {     
        result = tupleWord;
        return YIELD;
      } else  {     
        return CANCEL;
      }     

    case CLOSE:
      qp->Close(args[0].addr);
      if(sli) {
        delete sli;
        sli = NULL;
        local.setAddr( sli );
      }
      return 0;
  }
  return 0;
}


/*
2.1.4 Specification of operator ~sleep~

*/
const string SleepSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>((stream (tuple([a1:d1, ... ,an:dn]"
                         "))) x int) -> (stream (tuple([a1:d1, ... ,"
                         "an:dn]))) or \n"
                         "((stream T) int) -> (stream T), "
                         "for T in kind DATA.</text--->"
                         "<text>_ sleep [ _ ]</text--->"
                         "<text> This operator forwards the stream with "
                         " a configurable delay. Delay must be specified "
                         " in ms</text--->"
                         "<text>query cities feed sleep[10] consume"
                         "</text--->"
                              ") )";

/*
2.1.5 Definition of operator ~sleep~

*/
Operator cassandrasleep (
         "sleep",                 // name
         SleepSpec,               // specification
         Sleep,                   // value mapping
         Operator::SimpleSelect,  // trivial selection function
         SleepTypeMap,            // type mapping
         SleepCostEstimationFunc  // Cost estimation
);                         
     

/*
2.2 Operator ~statistics~

The operator ~statistics~ generates tuple flow statistics. 
The first parameter is the output file for the 
operator. The second parameter is the interval for
the statistics.

2.2.1 Type mapping function of operator ~statistics~

Type mapping for ~statistics~ is

----

 stream ( tuple ( (a1 t1) ... (an tn))) x text x
                int -> stream (tuple(...))
                
----

*/
ListExpr StatisticsTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 3){
    return listutils::typeError("three arguments expected");
  }

  string err = " stream(tuple(...) x text x int expected";

  ListExpr stream = nl->First(args);
  ListExpr filename = nl->Second(args);
  ListExpr interval = nl->Third(args);

  if(( !Stream<Tuple>::checkType(stream) &&
       !Stream<Attribute>::checkType(stream) ) ||
       !FText::checkType(filename) ||
       !CcInt::checkType(interval)) {
    return listutils::typeError(err);
  }
  
  return stream;
}

/*
2.2.2 Cost estimation

*/
CostEstimation* StatisticsCostEstimationFunc() {
  return new ForwardCostEstimation();
}

/*
2.2.3 Value mapping function of operator ~statistics~

*/
class StatisticsLocalInfo {
public:
  StatisticsLocalInfo(string myFilename, int myInterval) 
    : interval(myInterval), iteration(0), filename(myFilename), 
    seenTuples(0), totalTuples(0) {
      
      //cout << "Filename is " << filename 
      //     << " interval is " << interval << endl;
      
      // Open file for output
      filehandle.open(filename.c_str());
      if(! filehandle.is_open() ) {
        cout << "Unable to open file: " << filename << endl;
      } else {
        filehandle << "Time,Current Tuples,Total Tuples" << endl;
      }
       
      // Init interval
      reset();
  }
  
  virtual ~StatisticsLocalInfo() {
    dumpTuples();
    close();
  }
  
  // We received a new tuple
  // Register and dump
  void tupleReceived() {
    timeval curtime, difference;
    gettimeofday(&curtime, NULL); 
    
    timersub(&curtime, &lastdump, &difference);
    
    
    int miliseconds = difference.tv_sec * 1000 + (difference.tv_usec / 1000);
    
    if(miliseconds >= interval) {
      dumpTuples();
      reset();
    }
      
    seenTuples++;
  }
  
  // Reset seenTuples counter
  void reset() {
    gettimeofday(&lastdump, NULL); 
    totalTuples = totalTuples + seenTuples;
    seenTuples = 0;
  }
  
  // Close file
  void close() {
    filehandle.close();
  }
  
  // Dump seen tuples to output file
  void dumpTuples() {
    //cout << "Cur tuples " << seenTuples << endl;
    filehandle << iteration * interval << "," << seenTuples 
      << "," << (totalTuples + seenTuples) << endl;
    reset();
    iteration++;
  }
  
  // Are we ready?
  bool isReady() {
    return filehandle.is_open();
  }
  
private:
  int interval;           // Intervall in sec for dumping
  int iteration;          // Current iteration
  string filename;        // Filename for statistics
  timeval lastdump;       // When did we the last dump?
  size_t seenTuples;      // Seen tuples since last dump
  size_t totalTuples;     // Total seen tuples
  ofstream filehandle;    // Filehandle
};

int Statistics(Word* args, Word& result, int message, Word& local, Supplier s)
{
  StatisticsLocalInfo *sli; 
  Word tupleWord;

  sli = (StatisticsLocalInfo*)local.addr;

  switch(message)
  {
    case OPEN: 

      if ( sli ) delete sli;
      
      if(! ((FText*)args[1].addr)->IsDefined()) {
        cout << "Filename is not defined" << endl;
      } else {
        sli = new StatisticsLocalInfo((((FText*)args[1].addr)->GetValue()),
                      (((CcInt*)args[2].addr)->GetIntval()));
                      
        // Is statistics class ready (file open...)
        if(sli -> isReady()) {
          local.setAddr( sli );
          qp->Open(args[0].addr);
        }
      }
      return 0;

    case REQUEST:
      
      // Operator not ready
      if ( ! sli ) {
        return CANCEL;
      }
      
      qp->Request(args[0].addr, tupleWord);
      if(qp->Received(args[0].addr))
      {     
        sli -> tupleReceived();
        result = tupleWord;
        return YIELD;
      } else  {     
        return CANCEL;
      }     

    case CLOSE:
      qp->Close(args[0].addr);
      if(sli) {
        delete sli;
        sli = NULL;
        local.setAddr( sli );
      }
      return 0;
  }
  return 0;
}


/*
2.2.4 Specification of operator ~statistics~

*/
const string StatisticsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>((stream (tuple([a1:d1, ... ,an:dn]"
                         "))) y string x int) -> (stream (tuple([a1:d1, ... ,"
                         "an:dn]))) or \n"
                         "((stream T) text x int) -> (stream T), "
                         "for T in kind DATA.</text--->"
                         "<text>_ statistics [ _ , _ ]</text--->"
                         "<text> The operator statistics generates tuple flow "
                         "statistics. The first parameter is the output file "
                         "for the operator. The second parameter is the "
                         "interval for the statistics (msec). </text--->"
                         "<text>query intstream(1,10) transformstream "
                         "statistics['/tmp/statistics.csv', 10] count</text--->"
                              ") )";

/*
2.2.5 Definition of operator ~statistics~

*/
Operator cassandrastatistics (
         "statistics",                 // name
         StatisticsSpec,               // specification
         Statistics,                   // value mapping
         Operator::SimpleSelect,       // trivial selection function
         StatisticsTypeMap,            // type mapping
         StatisticsCostEstimationFunc  // Cost estimation
);                         


/*
2.3 Operator ~cdelete~

The operator ~cdelete~ deletes a relation in our
cassandra cluster. The first parameter is the contact
point to the cluster, the second paramter is the 
keyspace. The third paramter is the name of the 
relation to delete.

2.3.1 Type mapping function of operator ~cdelete~

Type mapping for ~cdelete~ is

----
 (text x text x text) -> bool            
----

*/

ListExpr CDeleteTypeMap( ListExpr args )
{
  string err = "text x text x text expected";
  
  if(!nl->HasLength(args,3)){
    return listutils::typeError(err);
  }
  
  if(!listutils::isSymbol(nl->First(args),FText::BasicType())  ||
     !listutils::isSymbol(nl->Second(args),FText::BasicType()) ||
     !listutils::isSymbol(nl->Third(args),FText::BasicType())) {
    
    return listutils::typeError(err);
  }    

  return nl->SymbolAtom(CcBool::BasicType());
}


/*
2.3.3 Value mapping function of operator ~cdelete~

*/
class CDeleteLocalInfo {

public:
  CDeleteLocalInfo(string myContactPoint, string myKeyspace, 
                   string myRelationName) 
  
    : contactPoint(myContactPoint), keyspace(myKeyspace),
    relationName(myRelationName), deletePerformed(false) {
      
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      cout << "Relation name is " << relationName << endl;
  }
  
/*
2.3.3.1 Delete our relation

*/  
  size_t deleteRelation() {
     CassandraAdapter* cassandra 
        = new CassandraAdapter(contactPoint, keyspace);
        
      cassandra -> connect();
      bool result = cassandra -> dropTable(relationName);
      cassandra -> disconnect();
      delete cassandra;
      cassandra = NULL;
    
     deletePerformed = true;
     return result;
  }

/*
2.3.3.2 Did we deleted the relation?

*/  
  bool isDeletePerformed() {
    return deletePerformed;
  }
  
private:
  string contactPoint;      // Contactpoint for our cluster
  string keyspace;          // Keyspace
  string relationName;      // Relation name to delete
  bool deletePerformed;     // Did we deleted the relation
};

int CDelete(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CDeleteLocalInfo *cli; 
  Word tupleWord;
  bool deleteOk;
  
  switch(message)
  {
    case OPEN: 
    case REQUEST:
    case CLOSE:
      
      result = qp->ResultStorage(s);
      static_cast<CcInt*>(result.addr)->Set(false, 0);
      
      if(! ((FText*) args[0].addr)->IsDefined()) {
        cout << "Cluster contactpoint is not defined" << endl;
        return CANCEL;
      }  else if (! ((FText*) args[1].addr)->IsDefined()) {
        cout << "Keyspace is not defined" << endl;
        return CANCEL;
      } else if (! ((FText*) args[2].addr)->IsDefined()) {
        cout << "Relationname is not defined" << endl;
        return CANCEL;
      } else {
      cli = new CDeleteLocalInfo(
        (((FText*)args[0].addr)->GetValue()),
        (((FText*)args[1].addr)->GetValue()),
        (((FText*)args[2].addr)->GetValue())
      );
      
      deleteOk = cli -> deleteRelation();
      static_cast<CcBool*>(result.addr)->Set(true, deleteOk);
      
      delete cli;
      cli = NULL;
      
      return YIELD;
      }
  }
  return 0;
}


/*
2.3.4 Specification of operator ~cdelete~

*/
const string CDeleteSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>(text x text x text) -> bool</text--->"
                         "<text> cdelete ( _ , _ , _ )</text--->"
                         "<text> The operator cdelete deletes a relation "
                         "in our cassandra cluster. The first parameter is "
                         "the contact point to the cluster, the second "
                         "paramter is the keyspace. The third parameter is" 
                         "name of the relation to delete."
                         "</text--->"
                         "<text>query cdelete ('127.0.0.1', 'keyspace1'"
                         ", 'plz') </text--->"
                              ") )";

/*
2.3.5 Definition of operator ~cdelete~

*/
Operator cassandradelete (
         "cdelete",                 // name
         CDeleteSpec,               // specification
         CDelete,                   // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CDeleteTypeMap             // type mapping
);                         





/*
2.4 Operator ~cfeed~

The operator ~cfeed~ feeds a tuple stream into
a cassandra cluster. The first paramter is the contactpoint
to the cluster. The second paramter is the keyspace. The 
third parameter is the name of the relation.
The third parameter is the consistence level:

ANY    - Only a hinted handoff is created
ONE    - One of the cassandra nodes has written the tuple
QUORUM - More then n/2+1 cassandra nodes have written the tuple
         n is the replication factor of the cluster
ALL    - All cassandra nodes have written the tuple

The fourth parameter is the name of the local
system. The name must be unique for each secondo
node

2.2.1 Type mapping function of operator ~cfeed~

Type mapping for ~cfeed~ is

----
  stream(tuple(...)) x text x text x text x text x text -> int
                
----

*/
ListExpr CFeedTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 6){
    return listutils::typeError("six arguments expected");
  }

  string err = " stream(tuple(...)) x text x text x text x text "
    " x text expected";

  ListExpr stream = nl->First(args);
  
  ListExpr contactpoint = nl->Second(args);
  ListExpr keyspace = nl->Third(args);
  ListExpr relation = nl->Fourth(args);
  ListExpr consistence = nl->Fifth(args);
  ListExpr systemname = nl->Sixth(args);
  
  if(( !Stream<Tuple>::checkType(stream) &&
       !Stream<Attribute>::checkType(stream) ) ||
       !FText::checkType(contactpoint) ||
       !FText::checkType(keyspace) ||
       !FText::checkType(relation) ||
       !FText::checkType(consistence) ||
       !FText::checkType(systemname)) {
    return listutils::typeError(err);
  }
  
  return nl->SymbolAtom(CcInt::BasicType());
}

/*
2.4.2 Cost estimation

*/
CostEstimation* CFeedCostEstimation() {
  return new ForwardCostEstimation();
}

/*
2.4.3 Value mapping function of operator ~cfeed~

*/
class CFeedLocalInfo {

public:
  CFeedLocalInfo(string myContactPoint, string myKeyspace, 
                 string myRelationName, string myConsistence, 
                 string mySystemname) 
  
    : contactPoint(myContactPoint), keyspace(myKeyspace), 
    relationName(myRelationName), consistence(myConsistence), 
    systemname(mySystemname), tupleNumber(0) {
      
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      cout << "Relation name is " << relationName << endl;
      cout << "Consistence is " << consistence << endl;
      cout << "Systemname is " << systemname << endl;
      
      cassandra = new CassandraAdapter(contactPoint, keyspace);
      cassandra -> connect();
      cassandra -> createTable(relationName);
      
      
      /*vector < long > tokens;
      cassandra -> getLocalTokens(tokens);
      cout << "Token size " << tokens.size() << endl;
      copy(tokens.begin(), tokens.end(), 
         std::ostream_iterator<long>(cout, " "));
      cout << std::endl;
      
      tokens.clear();
      cassandra -> getPeerTokens(tokens);
      cout << "Token size " << tokens.size() << endl;
      copy(tokens.begin(), tokens.end(), 
         std::ostream_iterator<long>(cout, " "));
      cout << std::endl;*/
      
  }
  
  virtual ~CFeedLocalInfo() {
    disconnect();
  }
  
  void disconnect() {
    if(cassandra != NULL) {
      cassandra -> disconnect();
      delete cassandra;
      cassandra = NULL;
    }
  }
  
  string buildKey() {
    stringstream ss;
    ss << relationName;
    ss << "-";
    ss << systemname;
    ss << "-";
    ss << tupleNumber;
    return ss.str();
  }
  
  bool feed(Tuple* tuple) {
    cassandra->writeDataToCassandraPrepared(
                         buildKey(), 
                         tuple -> WriteToBinStr(), 
                         relationName, consistence, false);
    
    ++tupleNumber;
    return true;
  }
  
  size_t getTupleNumber() {
    return tupleNumber;
  }
  
  
private:
  string contactPoint;         // Contactpoint for our cluster
  string keyspace;             // Keyspace
  string relationName;         // Relation name to delete
  string consistence;          // Consistence
  string systemname;           // Name of our system
  size_t tupleNumber;          // Number of the current tuple
  CassandraAdapter *cassandra; // Cassandra connection
};

int CFeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CFeedLocalInfo *cli; 
  Word elem;
  bool feedOk = true;
  bool parameterOk = true;
  
  switch(message)
  {
    case OPEN: 
    case REQUEST:
    case CLOSE:
      
      result = qp->ResultStorage(s);
      static_cast<CcInt*>(result.addr)->Set(false, 0);
      
      if(! ((FText*) args[1].addr)->IsDefined()) {
        cout << "Cluster contactpoint is not defined" << endl;
        parameterOk = false;
      } else if (! ((FText*) args[2].addr)->IsDefined()) {
        cout << "Keyspace is not defined" << endl;
        parameterOk = false;        
      } else if (! ((FText*) args[3].addr)->IsDefined()) {
        cout << "Relationname is not defined" << endl;
        parameterOk = false;
      } else if (! ((FText*) args[4].addr)->IsDefined()) {
        cout << "Consistence level is not defined" << endl;
        parameterOk = false;
      } else if (! ((FText*) args[5].addr)->IsDefined()) {
        cout << "Systemname is not defined" << endl;
        parameterOk = false;
      }
      
      string consistenceLevel = ((FText*) args[4].addr)->GetValue();
      
      if( ! CassandraHelper::checkConsistenceLevel(consistenceLevel) ) {
        cout << "Unknown consistence level: " << consistenceLevel << endl;
        return CANCEL;
      } 
      
      cli = new CFeedLocalInfo(
                      (((FText*)args[1].addr)->GetValue()),
                      (((FText*)args[2].addr)->GetValue()),
                      (((FText*)args[3].addr)->GetValue()),
                      (((FText*)args[4].addr)->GetValue()),
                      (((FText*)args[5].addr)->GetValue())
                      );
      
      // Consume stream
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, elem);
      
      while ( qp->Received(args[0].addr) ) {      
        
        // Cassandra ready?
        if(parameterOk) {
          feedOk = cli -> feed((Tuple*)elem.addr);

          if(! feedOk) {
            cout << "Unable to write tuple to cassandra" << endl;
          }
        }
        
        ((Tuple*)elem.addr)->DeleteIfAllowed();         
        qp->Request(args[0].addr, elem);
      }  
      qp->Close(args[0].addr);
      static_cast<CcInt*>(result.addr)->Set(true, cli -> getTupleNumber());  
      
      cli -> disconnect();
      
      delete cli;
      cli = NULL;
      
      return YIELD;
      
  }
  return 0;
}


/*
2.4.4 Specification of operator ~cfeed~

*/
const string CFeedSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>stream(tuple(...)) x text x text x text "
                         "x text x text -> int</text--->"
                         "<text>cfeed _ op [ _ , _ , _ , _ , _ ] </text--->"
                         "<text>The operator cfeed feeds a tuple stream"
                         "into a cassandra cluster. The first paramter is "
                         "the contactpoint to the cluster. The second "
                         "paramter is the keyspace. The third parameter is "
                         "the name of the relation. The fourth "
                         "parameter is the consistence level:"
                         "ANY    - Only a hinted handoff is created"
                         "ONE    - One of the cassandra nodes has written the "
                         "         tuple"
                         "QUORUM - More then n/2+1 cassandra nodes have" 
                         "         written the tuple. Where n is the "
                         "         replication factor of the cluster"
                         "ALL    - All cassandra nodes have written the tuple"
                         "The sixth parameter is the name of the local "
                         "system. The name must be unique for each secondo"
                         "node"
                         "</text--->"
                         "<text>query plz feed cfeed['127.0.0.1', 'keyspace', "
                         "'plz', 'ANY'. 'node1']</text--->"
                              ") )";

/*
2.4.5 Definition of operator ~cfeed~

*/
Operator cassandrafeed (
         "cfeed",                 // name
         CFeedSpec,               // specification
         CFeed,                   // value mapping
         Operator::SimpleSelect,  // trivial selection function
         CFeedTypeMap,            // type mapping
         CFeedCostEstimation      // Cost estimation
);                         


/*
2.5 Operator ~ccollect~

The operator ~ccollect~ fetches a relation from our
cassandra cluster and create a stream of tuples.

The first paramter is the contact point to the cassandra cluster
The second parameter specifies the keyspace to use
The third parameter contains the name of the relation to fetch
The fourth parameter specifies the consistence level used for reading:

ANY    - Only a hinted handoff is created
ONE    - One of the cassandra nodes has written the tuple
QUORUM - More then n/2+1 cassandra nodes have written the tuple
         n is the replication factor of the cluster
ALL    - All cassandra nodes have written the tuple


2.5.1 Type mapping function of operator ~ccollect~

Type mapping for ~ccollect~ is

----
  rel(tuple(a[_]1 : t[_]1)...(a[_]n : t[_]n)) x text x
                     text x text x text -> stream(tuple(...))
----

*/
ListExpr CCollectTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 5){
    return listutils::typeError("five arguments expected");
  }

  string err = " stream(tuple(...) x text x text x text x text expected";

  ListExpr relation = nl->First(args);
  ListExpr contactPoint = nl->Second(args);
  ListExpr keyspace = nl->Third(args);
  ListExpr relationName = nl->Fourth(args);
  ListExpr consistenceLevel = nl->Fifth(args);

  if(  !Relation::checkType(relation) ||
       !FText::checkType(contactPoint) ||
       !FText::checkType(keyspace) ||
       !FText::checkType(relationName) ||
       !FText::checkType(consistenceLevel)) {
    return listutils::typeError(err);
  }
  
  // create result list
  ListExpr resList = nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                     nl->Second(relation));
  
  return resList;
}


/*
2.5.3 Value mapping function of operator ~ccollect~

*/
class CCollectLocalInfo {

public:
  CCollectLocalInfo(ListExpr myType, string myContactPoint, string myKeyspace,
                    string myRelationName, string myConsistence) 
  
    : tupleType(myType), contactPoint(myContactPoint), keyspace(myKeyspace), 
    relationName(myRelationName), consistence(myConsistence),
    cassandra(NULL), result(NULL) {
      
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      cout << "Relation name is " << relationName << endl;
      cout << "Consistence is " << consistence << endl;

      open();
  }
  
  virtual ~CCollectLocalInfo() {
    
    if(result != NULL) {
      delete result;
      result = NULL;
    }
    
    if(cassandra != NULL) {
      delete cassandra;
      cassandra = NULL;
    }
  }
  
  void open(){
    if(cassandra == NULL) {
      cassandra = new CassandraAdapter(contactPoint, keyspace);
      cassandra -> connect();
      result = cassandra -> readTable(relationName, consistence);
    }
  }

  Tuple* fetchNextTuple() {
    if(result->hasNext()) {
      
      string myResult;
      result -> getStringValue(myResult, 0);
      
      Tuple* tuple = new Tuple(tupleType);
      tuple->ReadFromBinStr(myResult);
      
      return tuple;
    }
    
    return NULL;
  }
  
private:
  ListExpr tupleType;          // Tuple Type
  string contactPoint;         // Contactpoint for our cluster
  string keyspace;             // Keyspace
  string relationName;         // Relation name to delete
  string consistence;          // Consistence  
  CassandraAdapter* cassandra; // Our cassandra connection
  CassandraResult* result;     // Query result
};

int CCollect(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CCollectLocalInfo *cli; 
  Word elem;
  string consistenceLevel;

  cli = (CCollectLocalInfo*)local.addr;
  
  ListExpr resultType = GetTupleResultType(s);
  
  switch(message) {
    case OPEN: 

     if ( cli ) delete cli;
     
     consistenceLevel = ((FText*) args[4].addr)->GetValue();
     
     if(! ((FText*) args[1].addr)->IsDefined()) {
       cout << "Cluster contactpoint is not defined" << endl;
     } else if (! ((FText*) args[2].addr)->IsDefined()) {
       cout << "Keyspace is not defined" << endl;
     } else if (! ((FText*) args[3].addr)->IsDefined()) {
       cout << "Relationname is not defined" << endl;
     } else if (! ((FText*) args[4].addr)->IsDefined()) {
       cout << "Consistence level is not defined" << endl;
     } else if( ! CassandraHelper::checkConsistenceLevel(consistenceLevel)) {
       cout << "Unknown consistence level: " << consistenceLevel << endl; 
     } else {
       cli = new CCollectLocalInfo(nl -> Second(resultType),
                      (((FText*)args[1].addr)->GetValue()),
                      (((FText*)args[2].addr)->GetValue()),
                      (((FText*)args[3].addr)->GetValue()),
                      (((FText*)args[4].addr)->GetValue())
                 );
        
         local.setAddr( cli );
     }
      
     return 0;

    case REQUEST:
      
      // Operator not ready
      if ( ! cli ) {
        return CANCEL;
      }
      
      // Fetch next tuple from cassandra
      result.addr = cli -> fetchNextTuple();
      
      if(result.addr != NULL) {     
        return YIELD;
      } else  {     
        return CANCEL;
      }     

    case CLOSE:
      if(cli) {
        delete cli;
        cli = NULL;
        local.setAddr( cli );
      }
      return 0;
  }
  
  return 0;    
}


/*
2.5.4 Specification of operator ~ccollect~

*/
const string CCollectSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>rel(tuple(a[_]1 : t[_]1)...(a[_]n : t[_]n))"
                         "x text x text x text -> stream(tuple(...))</text--->"
                         "<text>cfeed _ op [ _ , _ , _ ] </text--->"
                         "<text>The operator ccollect fetches a relation from"
                         "our cassandra cluster and create a stream of"
                         "tuples. The first paramter is the contact point to "
                         "the cassandra cluster. The second parameter "
                         "specifies the keyspace to use. The third parameter "
                         "contains the name of the relation to fetch. The "
                         "fourth parameter specifies the consistence "
                         "level used for reading</text--->"
                         "<text>query plz ccollect['127.0.0.1', 'keyspace1', "
                         "'plz', 'ANY'] count</text--->"
                              ") )";

/*
2.4.5 Definition of operator ~ccollect~

*/
Operator cassandracollect (
         "ccollect",                // name
         CCollectSpec,              // specification
         CCollect,                  // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CCollectTypeMap            // type mapping
);                         



/*
2.6 Operator ~clist~

The operator ~clist~ list all known tables in a
cassandra keyspace.

The first paramter is the contact point to the cassandra cluster
The second parameter specifies the keyspace to use

2.6.1 Type mapping function of operator ~clist~

Type mapping for ~clist~ is

----
  text x text -> stream(text)
----

*/
ListExpr CListTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 2){
    return listutils::typeError("two arguments expected");
  }

  string err = " text x text expected";

  ListExpr contactPoint = nl->First(args);
  ListExpr keyspace = nl->Second(args);

  if(  !FText::checkType(contactPoint) ||
       !FText::checkType(keyspace)) {
    return listutils::typeError(err);
  }
  
  // create result list
  ListExpr resList = nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                     nl->SymbolAtom(FText::BasicType()));
  
  return resList;
}


/*
2.6.3 Value mapping function of operator ~clist~

*/
class CListLocalInfo {

public:
  CListLocalInfo(string myContactPoint, string myKeyspace) 
  
    : contactPoint(myContactPoint), keyspace(myKeyspace),
    cassandra(NULL), result(NULL) {
      
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      
      open();
  }
  
  void open(){
    if(cassandra == NULL) {
      cassandra = new CassandraAdapter(contactPoint, keyspace);
      cassandra -> connect();
      result = cassandra -> getAllTables(keyspace);
    }
  }

  FText* fetchNextTable() {
    if(result->hasNext()) {
      
      string myResult;
      result -> getStringValue(myResult, 0);
      
      return new FText(true, myResult);
    }
    
    return NULL;
  }
  
  virtual ~CListLocalInfo() {
    if(result != NULL) {
      delete result;
      result = NULL;
    }
    
    if(cassandra != NULL) {
      delete cassandra;
      cassandra = NULL;
    }
  }
  
  private:
  string contactPoint;         // Contactpoint for our cluster
  string keyspace;             // Keyspace
  CassandraAdapter* cassandra; // Our cassandra connection
  CassandraResult* result;     // Query result
};


int CList(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CListLocalInfo *cli; 
  Word elem;
  cli = (CListLocalInfo*)local.addr;
  
  switch(message) {
    case OPEN: 

     if ( cli ) delete cli;
     
     if(! ((FText*) args[0].addr)->IsDefined()) {
       cout << "Cluster contactpoint is not defined" << endl;
     } else if (! ((FText*) args[1].addr)->IsDefined()) {
       cout << "Keyspace is not defined" << endl;
     } else {
       cli = new CListLocalInfo(
                      (((FText*)args[0].addr)->GetValue()),
                      (((FText*)args[1].addr)->GetValue())
                 );
        
       local.setAddr( cli );
     }
      
     return 0;

    case REQUEST:
      
      // Operator not ready
      if ( ! cli ) {
        return CANCEL;
      }
      
      // Fetch next table from cassandra
      result.addr = cli -> fetchNextTable();
      
      if(result.addr != NULL) {     
        return YIELD;
      } else  {     
        return CANCEL;
      }     

    case CLOSE:
      if(cli) {
        delete cli;
        cli = NULL;
        local.setAddr( cli );
      }
      return 0;
  }
  
  return 0;    
}

/*
2.6.4 Specification of operator ~clist~

*/
const string CListSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text -> stream(text)</text--->"
                         "<text>clist ( _ , _ ) </text--->"
                         "<text>The operator clist list all known tables in a"
                         "cassandra keyspace</text--->"
                         "<text>query clist('127.0.0.1', 'keyspace1') count "
                         "</text--->"
                              ") )";

/*
2.6.5 Definition of operator ~clist~

*/
Operator cassandralist (
         "clist",                  // name
         CListSpec,                // specification
         CList,                    // value mapping
         Operator::SimpleSelect,   // trivial selection function
         CListTypeMap              // type mapping
);                         



/*
 7 Creating the Algebra

*/
class CassandraAlgebra: public Algebra
{
public:
  CassandraAlgebra() :
    Algebra()
  {
    
    /*
     7.2 Registration of Operators

     */
    AddOperator(&cassandrasleep);
    AddOperator(&cassandrastatistics);
    AddOperator(&cassandradelete);
    AddOperator(&cassandrafeed);
    AddOperator(&cassandracollect);
    AddOperator(&cassandralist);
    
  }
  
  ~CassandraAlgebra()
  {
  }
  ;
};

/*
 8 Initialization

 Each algebra module needs an initialization function. The algebra
 manager has a reference to this function if this algebra is
 included in the list of required algebras, thus forcing the linker
 to include this module.

 The algebra manager invokes this function to get a reference to the
 instance of the algebra class and to provide references to the
 global nested list container (used to store constructor, type,
 operator and object information) and to the query processor.

 The function has a C interface to make it possible to load the
 algebra dynamically at runtime.

*/

} // end of namespace ~sta~

extern "C" Algebra*
InitializeCassandraAlgebra(NestedList* nlRef, QueryProcessor* qpRef,
    AlgebraManager* amRef)
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;

  return (new cassandra::CassandraAlgebra());

}

