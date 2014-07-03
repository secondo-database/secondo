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
#include <time.h> 

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
#include "CassandraResult.h"
#include "CassandraConnectionPool.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

// Activate debug messages
#define __DEBUG__

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
      
#ifdef __DEBUG__
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      cout << "Relation name is " << relationName << endl;
#endif
  }
  
/*
2.3.3.1 Delete our relation

*/  
  bool deleteRelation() {
     CassandraAdapter* cassandra 
        = CassandraConnectionPool::Instance()->
          getConnection(contactPoint, keyspace, false);

      bool result = cassandra -> dropTable(relationName);
    
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
2.4 Operator ~cspread~

The operator ~cspread~ feeds a tuple stream into
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

The sixth parameter is the name of the local 
system. The name must be unique for each secondo
node

The seventh parameter is the attribute used for
clustering

2.2.1 Type mapping function of operator ~cspread~

Type mapping for ~cspread~ is

----
  stream(tuple(...)) x text x text x text x text x text x attr -> int
                
----

*/
ListExpr CSpreadTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 7){
    return listutils::typeError("seven arguments expected");
  }

  string err = " stream(tuple(...)) x text x text x text x text "
    " x text x attr expected";

  ListExpr stream = nl->First(args);
  
  ListExpr contactpoint = nl->Second(args);
  ListExpr keyspace = nl->Third(args);
  ListExpr relation = nl->Fourth(args);
  ListExpr consistence = nl->Fifth(args);
  ListExpr systemname = nl->Sixth(args);
  ListExpr attr = nl->Seventh(args);
  
  if(( !Stream<Tuple>::checkType(stream) &&
       !Stream<Attribute>::checkType(stream) ) ||
       !FText::checkType(contactpoint) ||
       !FText::checkType(keyspace) ||
       !FText::checkType(relation) ||
       !FText::checkType(consistence) ||
       !FText::checkType(systemname)) {
    return listutils::typeError(err);
  }
  
  // Check attribute name and append position
  // to nested list
  if(!listutils::isSymbol(attr)){
    return listutils::typeError(err + "(attrname is not valid)");
  }
  
  ListExpr attrType;
  string attrname = nl->SymbolValue(attr);
  ListExpr attrList = nl->Second(nl->Second(stream));
  int index = listutils::findAttribute(attrList,attrname,attrType);
  
  if(index == 0) {
    return listutils::typeError(attrname+
                     " is not an attribute of the stream");
  } 
  
  ListExpr indexList = nl->OneElemList(nl->IntAtom(index-1));
  
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->SymbolAtom(CcInt::BasicType()));
}

/*
2.4.2 Cost estimation

*/
CostEstimation* CSpreadCostEstimation() {
  return new ForwardCostEstimation();
}

/*
2.4.3 Value mapping function of operator ~cspread~

*/
class CSpreadLocalInfo {

public:
  CSpreadLocalInfo(string myContactPoint, string myKeyspace, 
                 string myRelationName, string myConsistence, 
                 string mySystemname, int myAttrIndex, string myTupleType)
  
    : contactPoint(myContactPoint), keyspace(myKeyspace), 
    relationName(myRelationName), consistence(myConsistence), 
    systemname(mySystemname), attrIndex(myAttrIndex), tupleType(myTupleType), 
    tupleNumber(0) {
      
#ifdef __DEBUG__
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      cout << "Relation name is " << relationName << endl;
      cout << "Consistence is " << consistence << endl;
      cout << "Systemname is " << systemname << endl;
      cout << "Attribute Index is " << attrIndex << endl;
      cout << "Tuple type is " << tupleType << endl;
#endif      
      
      cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);
           
      cassandra -> createTable(relationName, tupleType);
      
      if(sizeof(size_t) < 8) {
        cout << "WARNING: Your size_t datatype is smaller then 8 bytes. ";
        cout << endl;
        cout << "The size of size_t on your system is: " << sizeof(size_t);
        cout << endl;
        cout << "This leads to small hash values and performance issues ";
        cout << " with casssandra";
        cout << endl;
      }   
  }
  
  virtual ~CSpreadLocalInfo() {
    disconnect();
  }
  
  void disconnect() {
    if(cassandra) {
      // Wait for all pending futures to finish
      cassandra -> waitForPendingFutures();
    }

    // Destroy connection. 
    CassandraConnectionPool::Instance()->
       destroy();
  }
  
  bool feed(Tuple* tuple) {
    
    stringstream ss;
    ss << tuple -> HashValue(attrIndex);
    string partitionKey = ss.str();
    
    ss.clear();
    ss << tupleNumber;
    string tupleNumberStr = ss.str();
    
    cassandra->writeDataToCassandra(
                         relationName,
                         partitionKey,
                         systemname,
                         tupleNumberStr, 
                         tuple -> WriteToBinStr(), 
                         consistence, false);
    
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
  int attrIndex;               // Index of attribute to cluster
  string tupleType;            // Type of the tuples (Nested List String)
  size_t tupleNumber;          // Number of the current tuple
  CassandraAdapter *cassandra; // Cassandra connection
};

int CSpread(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CSpreadLocalInfo *cli; 
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
      } else if (! ((CcInt*) args[7].addr)->IsDefined()) {
        cout << "Attr Index is not defined" << endl;
        parameterOk = false;
      }
      
      string consistenceLevel = ((FText*) args[4].addr)->GetValue();
      
      if( ! CassandraHelper::checkConsistenceLevel(consistenceLevel) ) {
        cout << "Unknown consistence level: " << consistenceLevel << endl;
        return CANCEL;
      } 
      
      // Type of tuple stream
      ListExpr relTypeList =
         qp->GetSupplierTypeExpr(qp->GetSon(s,0));
    
      string tupleType = nl->ToString(relTypeList);
      
      
      cli = new CSpreadLocalInfo(
                      (((FText*)args[1].addr)->GetValue()),
                      (((FText*)args[2].addr)->GetValue()),
                      (((FText*)args[3].addr)->GetValue()),
                      (((FText*)args[4].addr)->GetValue()),
                      (((FText*)args[5].addr)->GetValue()),
                      (((CcInt*)args[7].addr)->GetValue()),
                      tupleType
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
2.4.4 Specification of operator ~cspread~

*/
const string CSpreadSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>stream(tuple(...)) x text x text x text "
                         "x text x text x attr -> int</text--->"
                         "<text>cspread _ op [ _ , _ , _ , _ , _ ] </text--->"
                         "<text>The operator cspread feeds a tuple stream"
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
                         ""
                         "The sixth parameter is the name of the local "
                         "system. The name must be unique for each secondo"
                         "node"
                         ""
                         "The seventh parameter is the attribute used for"
                         "clustering"
                         "</text--->"
                         "<text>query plz feed cspread['127.0.0.1', 'keyspace'"
                         ", 'plz', 'ANY'. 'node1', 'PLZ']</text--->"
                              ") )";

/*
2.4.5 Definition of operator ~cspread~

*/
Operator cassandrafeed (
         "cspread",                 // name
         CSpreadSpec,               // specification
         CSpread,                   // value mapping
         Operator::SimpleSelect,  // trivial selection function
         CSpreadTypeMap,            // type mapping
         CSpreadCostEstimation      // Cost estimation
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

2.5.0 Helper function for evaluating nested list
    expressions

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


/*
2.5.1 Fetch Mode for collect

ALL = Fetch the whole table
LOCAL = Fetch only data stored on the local node
RANGE = Fetch only data in the specified token range
QUERY = Fetch data produced by a given query id. Ensure that
        only data from failure-free nodes are fetched

*/
enum CollectFetchMode {ALL, LOCAL, RANGE, QUERY}; 

/*
2.5.1 Type mapping function of operator ~ccollect~ and ~ccollectlocal~

Type mapping for ~ccollect~ is

----
   text x text x text x text -> stream(tuple(...))
----

*/
ListExpr CCollectTypeMap( ListExpr args ) {

  if(nl->ListLength(args) != 4){
    return listutils::typeError("four arguments expected");
  }

  string err = " text x text x text x text expected";

  // arg evaluation is active
  // this means each argument is a two elem list (type value)
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
    if(!nl->HasLength(nl->First(tmp),2)){
       return listutils::typeError("expected (type value)");
    }    
    tmp = nl->Rest(tmp);
  }
  
  ListExpr contactPoint = nl->First(args);
  ListExpr keyspace = nl->Second(args);
  ListExpr relationName = nl->Third(args);
  ListExpr consistenceLevel = nl->Fourth(args);

  if(  !FText::checkType(nl->First(contactPoint)) ||
       !FText::checkType(nl->First(keyspace)) ||
       !FText::checkType(nl->First(relationName)) ||
       !FText::checkType(nl->First(consistenceLevel))) {
    return listutils::typeError(err);
  }
  
  // We need to evaluate the result list, contact cassandra
  // and read the tuple type of the table
  string contactPointVal;  
  string contactPointExpr = nl->ToString(nl->Second(contactPoint));
  if(! EvaluateTypeMappingExpr(contactPointExpr, contactPointVal) ) {
    return listutils::typeError(contactPointVal);
  }
  
  string keyspaceVal;  
  string keyspaceExpr = nl->ToString(nl->Second(keyspace));
  if(! EvaluateTypeMappingExpr(keyspaceExpr, keyspaceVal) ) {
    return listutils::typeError(keyspaceVal);
  }

  string relationVal;  
  string relationExpr = nl->ToString(nl->Second(relationName));
  if(! EvaluateTypeMappingExpr(relationExpr, relationVal) ) {
    return listutils::typeError(relationVal);
  }

  // Read tuple type from cassandra
  string tupleType;
  
  CassandraAdapter* cassandra 
     = CassandraConnectionPool::Instance()->
      getConnection(contactPointVal, keyspaceVal, false);
 
  bool result = 
     cassandra -> getTupleTypeFromTable(relationVal, tupleType);
     
  if(!result) {
     return listutils::typeError("Unable to read table type from "
       "cassandra. Does the table exist?");
  }
  
  ListExpr resList;
  nl->ReadFromString(tupleType, resList);

#ifdef __DEBUG__
  cout << "Result: " << nl->ToString(resList) << endl;
#endif
  
  return resList;
}

/*
2.5.2 Type mapping function of operator ~ccollectrange~

Type mapping for ~ccollectrange~ is

----
   text x text x text x text x text x text -> stream(tuple(...))
----

*/
ListExpr CCollectRangeTypeMap( ListExpr args ) {

  if(nl->ListLength(args) != 6){
    return listutils::typeError("six arguments expected");
  }

  string err = " text x text x text x text x text x text expected";

  // arg evaluation is active
  // this means each argument is a two elem list (type value)
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
    if(!nl->HasLength(nl->First(tmp),2)){
       return listutils::typeError("expected (type value)");
    }    
    tmp = nl->Rest(tmp);
  }
  
  ListExpr contactPoint = nl->First(args);
  ListExpr keyspace = nl->Second(args);
  ListExpr relationName = nl->Third(args);
  ListExpr consistenceLevel = nl->Fourth(args);
  ListExpr begin = nl->Fifth(args);
  ListExpr end = nl->Sixth(args);

  if(  !FText::checkType(nl->First(contactPoint)) ||
       !FText::checkType(nl->First(keyspace)) ||
       !FText::checkType(nl->First(relationName)) ||
       !FText::checkType(nl->First(consistenceLevel)) ||
       !FText::checkType(nl->First(begin)) ||
       !FText::checkType(nl->First(end))
    ) {
    return listutils::typeError(err);
  }
  
  
  // We need to evaluate the result list, contact cassandra
  // and read the tuple type of the table
  string contactPointVal;  
  string contactPointExpr = nl->ToString(nl->Second(contactPoint));
  if(! EvaluateTypeMappingExpr(contactPointExpr, contactPointVal) ) {
    return listutils::typeError(contactPointVal);
  }
  
  string keyspaceVal;  
  string keyspaceExpr = nl->ToString(nl->Second(keyspace));
  if(! EvaluateTypeMappingExpr(keyspaceExpr, keyspaceVal) ) {
    return listutils::typeError(keyspaceVal);
  }

  string relationVal;  
  string relationExpr = nl->ToString(nl->Second(relationName));
  if(! EvaluateTypeMappingExpr(relationExpr, relationVal) ) {
    return listutils::typeError(relationVal);
  }

  // Read tuple type from cassandra
  string tupleType;
  
  CassandraAdapter* cassandra 
  = CassandraConnectionPool::Instance()->
      getConnection(contactPointVal, keyspaceVal, false);

  bool result = 
     cassandra -> getTupleTypeFromTable(relationVal, tupleType);
  
  if(!result) {
     return listutils::typeError("Unable to read table type from "
       "cassandra. Does the table exist?");
  }
  
  ListExpr resList;
  nl->ReadFromString(tupleType, resList);
  
#ifdef __DEBUG__
  cout << "Result: " << nl->ToString(resList) << endl;
#endif
  
  return resList;
}



/*
2.5.2 Type mapping function of operator ~ccollectquery~

Type mapping for ~ccollecquery~ is

----
   text x text x text x text x int -> stream(tuple(...))
----

*/
ListExpr CCollectQueryTypeMap( ListExpr args ) {

  if(nl->ListLength(args) != 5){
    return listutils::typeError("five arguments expected");
  }

  string err = " text x text x text x text x int expected";

  // arg evaluation is active
  // this means each argument is a two elem list (type value)
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
    if(!nl->HasLength(nl->First(tmp),2)){
       return listutils::typeError("expected (type value)");
    }    
    tmp = nl->Rest(tmp);
  }
  
  ListExpr contactPoint = nl->First(args);
  ListExpr keyspace = nl->Second(args);
  ListExpr relationName = nl->Third(args);
  ListExpr consistenceLevel = nl->Fourth(args);
  ListExpr query = nl->Fifth(args);

  if(  !FText::checkType(nl->First(contactPoint)) ||
       !FText::checkType(nl->First(keyspace)) ||
       !FText::checkType(nl->First(relationName)) ||
       !FText::checkType(nl->First(consistenceLevel)) ||
       !CcInt::checkType(nl->First(query))
    ) {
    return listutils::typeError(err);
  }
  
  
  // We need to evaluate the result list, contact cassandra
  // and read the tuple type of the table
  string contactPointVal;  
  string contactPointExpr = nl->ToString(nl->Second(contactPoint));
  if(! EvaluateTypeMappingExpr(contactPointExpr, contactPointVal) ) {
    return listutils::typeError(contactPointVal);
  }
  
  string keyspaceVal;  
  string keyspaceExpr = nl->ToString(nl->Second(keyspace));
  if(! EvaluateTypeMappingExpr(keyspaceExpr, keyspaceVal) ) {
    return listutils::typeError(keyspaceVal);
  }

  string relationVal;  
  string relationExpr = nl->ToString(nl->Second(relationName));
  if(! EvaluateTypeMappingExpr(relationExpr, relationVal) ) {
    return listutils::typeError(relationVal);
  }

  // Read tuple type from cassandra
  string tupleType;
  
  CassandraAdapter* cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPointVal, keyspaceVal, false);
           
  bool result = 
     cassandra -> getTupleTypeFromTable(relationVal, tupleType);
     
  
  if(!result) {
     return listutils::typeError("Unable to read table type from "
       "cassandra. Does the table exist?");
  }
  
  ListExpr resList;
  nl->ReadFromString(tupleType, resList);
  
#ifdef __DEBUG__
  cout << "Result: " << nl->ToString(resList) << endl;
#endif
  
  return resList;
}



/*
2.5.3 Value mapping function of operator ~ccollect~

*/

template<CollectFetchMode fetchMode>
class CCollectLocalInfo {

public:
  CCollectLocalInfo(ListExpr myType, string myContactPoint, string myKeyspace,
                    string myRelationName, string myConsistence) 
  
    : tupleType(myType), contactPoint(myContactPoint), keyspace(myKeyspace), 
    relationName(myRelationName), consistence(myConsistence),
    cassandra(NULL), result(NULL) {

#ifdef __DEBUG__
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
      cout << "Relation name is " << relationName << endl;
      cout << "Consistence is " << consistence << endl;
#endif
  }
  
  virtual ~CCollectLocalInfo() {
    
    if(result != NULL) {
      delete result;
      result = NULL;
    }
  }
  
  void open(){
    if(cassandra == NULL) {
      // Read the whole table or only the data
      // stored on the local cassandra node.
      if(fetchMode == ALL) {
        // Connect to cassandra and use the multi node loadbalancing 
        // policy        
       cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);

        result = cassandra -> readTable(relationName, consistence);
      } else if(fetchMode == LOCAL) {
        // Connect to cassandra and use the single node loadbalancing 
        // policy
       cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, true);

        result = cassandra -> readTableLocal(relationName, consistence);
      } else if(fetchMode == RANGE) {
        // Connect to cassandra and use the single node loadbalancing 
        // policy
       cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, true);

        result = cassandra -> readTableRange(relationName, consistence, 
                                             beginToken, endToken);
        
      } else if(fetchMode == QUERY) {
        // Connect to cassandra and use the multi node loadbalancing 
        // policy
        cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);

        result = cassandra -> readTableCreatedByQuery(relationName, 
                                             consistence, queryId);
      } else {
         cerr << "Unknown mode: " << fetchMode << endl;
         return;
      }
      
    }
  }

  Tuple* fetchNextTuple() {

    // No result present
    if(result == NULL) {
      return NULL;
    }
    
    while(result->hasNext()) {
      
      string key;
      string value;
      result -> getStringValue(key, 0);
      result -> getStringValue(value, 1);
      
      // Metadata? Skip tuple
      if(key.at(0) == '_') {
#ifdef __DEBUG__
        cout << "Skipping key: " << key << " value " << value << endl;
#endif
        continue;
      }
      
      // Otherwise: Build tuple and return
      Tuple* tuple = new Tuple(tupleType);
      tuple->ReadFromBinStr(value);
      return tuple;
    }
    
    return NULL;
  }
  
  void setBeginEndToken(string myBegintoken, string myEndToken) {
    beginToken = myBegintoken;
    endToken = myEndToken;
  }
  
  void setQueryId(int myQueryId) {
    queryId = myQueryId;
  }
  
private:
  ListExpr tupleType;          // Tuple Type
  string contactPoint;         // Contactpoint for our cluster
  string keyspace;             // Keyspace
  string relationName;         // Relation name to delete
  string consistence;          // Consistence  
  CassandraAdapter* cassandra; // Our cassandra connection
  CassandraResult* result;     // Query result
  string beginToken;           // Begin Token
  string endToken;             // End Token
  int    queryId;              // Queryid
};

template<CollectFetchMode fetchMode>
int CCollect(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CCollectLocalInfo<fetchMode> *cli; 
  Word elem;
  string consistenceLevel;

  cli = (CCollectLocalInfo<fetchMode>*)local.addr;
  
  ListExpr resultType = GetTupleResultType(s);
  
  switch(message) {
    case OPEN: 

     if ( cli ) delete cli;
     
     consistenceLevel = ((FText*) args[3].addr)->GetValue();
     
     if(! ((FText*) args[0].addr)->IsDefined()) {
       cout << "Cluster contactpoint is not defined" << endl;
     } else if (! ((FText*) args[1].addr)->IsDefined()) {
       cout << "Keyspace is not defined" << endl;
     }  else if (! ((FText*) args[3].addr)->IsDefined()) {
       cout << "Consistence level is not defined" << endl;
     } else if( ! CassandraHelper::checkConsistenceLevel(consistenceLevel)) {
       cout << "Unknown consistence level: " << consistenceLevel << endl; 
     } else {
       
       
         cli = new CCollectLocalInfo<fetchMode>(nl -> Second(resultType),
                      (((FText*)args[0].addr)->GetValue()),
                      (((FText*)args[1].addr)->GetValue()),
                      (((FText*)args[2].addr)->GetValue()),
                      (((FText*)args[3].addr)->GetValue())
                 );
         
         // Parse additional parameter for RANGE MODE
         if(fetchMode == RANGE) {
            if (! ((FText*) args[4].addr)->IsDefined()) {
                cout << "Begin Token not defined" << endl;
            } else if (! ((FText*) args[5].addr)->IsDefined()) {
                cout << "End Token not defined" << endl;
            }
            
            string begin = ((FText*)args[4].addr)->GetValue();
            string end   = ((FText*)args[5].addr)->GetValue();

            cli -> setBeginEndToken(begin, end);
         }
         
         // Parse additional parameter for QUERY
         if(fetchMode == QUERY) {
           if (! ((CcInt*) args[4].addr)->IsDefined()) {
                cout << "QueryId is not defined" << endl;
           } else {
             int queryId = ((CcInt*)args[4].addr)->GetValue();
             cli -> setQueryId(queryId);
           }
         }
       
         cli -> open();
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
                         "<text>text x text x text x text "
                         "-> stream(tuple(...))</text--->"
                         "<text>ccollect [ _ , _ , _ , _ ] </text--->"
                         "<text>The operator ccollect fetches a relation from"
                         "our cassandra cluster and create a stream of"
                         "tuples. The first paramter is the contact point to "
                         "the cassandra cluster. The second parameter "
                         "specifies the keyspace to use. The third parameter "
                         "contains the name of the relation to fetch. The "
                         "fourth parameter specifies the consistence "
                         "level used for reading. "
                         "If you use the operator ccollectlocal, only"
                         "the data stored on the connected cassandra node "
                         "will be collected</text--->"
                         "<text>query ccollect['127.0.0.1', 'keyspace1', "
                         "'plz', 'ANY'] count</text--->"
                              ") )";

/*
2.5.5 Specification of operator ~ccollectrange~

*/
const string CCollectRangeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text x text x text x text x text"
                         "-> stream(tuple(...))</text--->"
                         "<text>ccollectrange [ _ , _ , _ , _ , _ , _ ]"
                         "</text--->"
                         "<text>The operator ccollectrange fetches a relation"
                         "from our cassandra cluster and create a stream of"
                         "tuples. The first paramter is the contact point to "
                         "the cassandra cluster. The second parameter "
                         "specifies the keyspace to use. The third parameter "
                         "contains the name of the relation to fetch. The "
                         "fourth parameter specifies the consistence "
                         "level used for reading. The fith and the sixth"
                         "parameter specifies the token range</text--->"
                         "<text>query ccollectrange['127.0.0.1', 'keyspace1', "
                         "'plz', 'ANY', '1234', '5678'] count</text--->"
                              ") )";

/*
2.5.5 Specification of operator ~ccollectquery~

*/
const string CCollectQuerySpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text x text x text x int"
                         "-> stream(tuple(...))</text--->"
                         "<text>ccollectquery [ _ , _ , _ , _ , _ ] </text--->"
                         "<text>The operator ccollect fetches a relation from"
                         "our cassandra cluster and create a stream of"
                         "tuples. The first paramter is the contact point to "
                         "the cassandra cluster. The second parameter "
                         "specifies the keyspace to use. The third parameter "
                         "contains the name of the relation to fetch. The "
                         "fourth parameter specifies the consistence "
                         "level used for reading. The fifth parameter"
                         "specifies the query id that produced the relation"
                         "Instad of ccollect this operator guarantees that"
                         "only data from error free nodes is fetched"
                         "</text--->"
                         "<text>query ccollectquery['127.0.0.1', 'keyspace1', "
                         "'plz', 'ANY', 3] count</text--->"
                              ") )";

                         
/*
2.4.7 Definition of operator ~ccollect~

*/
Operator cassandracollect (
         "ccollect",                // name
         CCollectSpec,              // specification
         CCollect<ALL>,             // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CCollectTypeMap            // type mapping
);                         


Operator cassandracollectlocal (
         "ccollectlocal",           // name
         CCollectSpec,              // specification
         CCollect<LOCAL>,           // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CCollectTypeMap            // type mapping
);                         

Operator cassandracollectrange (
         "ccollectrange",           // name
         CCollectRangeSpec,         // specification
         CCollect<RANGE>,           // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CCollectRangeTypeMap       // type mapping
);                        

Operator cassandracollectquery (
         "ccollectquery",           // name
         CCollectQuerySpec,         // specification
         CCollect<QUERY>,           // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CCollectQueryTypeMap       // type mapping
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

#ifdef __DEBUG__
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
#endif
      
      open();
  }
  
  void open(){
    if(cassandra == NULL) {
      cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);
           
      result = cassandra -> getAllTables(keyspace);
    }
  }

  FText* fetchNextTable() {
    
    while(result != NULL && result->hasNext()) {
      
      string myResult;
      result -> getStringValue(myResult, 0);
      
      // Skip system tables
      if(myResult.find("system_") == 0) {
        continue;
      }
      
      return new FText(true, myResult);
    }
    
    return NULL;
  }
  
  virtual ~CListLocalInfo() {
    if(result != NULL) {
      delete result;
      result = NULL;
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
2.7 Operator ~cquerylist~

The operator ~cquerylist~ list all scheduled quries in a
cassandra keyspace.

The first paramter is the contact point to the cassandra cluster
The second parameter specifies the keyspace to use

2.7.1 Type mapping function of operator ~cquerylist~

Type mapping for ~cquerylist~ is

----
  text x text -> stream(tuple(...))
----

*/
ListExpr CQueryListTypeMap( ListExpr args )
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
  ListExpr res =  nl->TwoElemList( 
                        nl->SymbolAtom(Stream<Tuple>::BasicType()),
                        nl->TwoElemList(
                            nl->SymbolAtom(Tuple::BasicType()) ,
                            nl->TwoElemList(
                                nl->TwoElemList(
                                nl->SymbolAtom("QueryID"),
                                nl->SymbolAtom(FText::BasicType())),
                                        
                                nl->TwoElemList(
                                nl->SymbolAtom("Query"),
                                nl->SymbolAtom(FText::BasicType())))            
                            ));
  
  
  return res;
}


/*
2.7.3 Value mapping function of operator ~cquerylist~

*/
class CQueryListLocalInfo {

public:
  CQueryListLocalInfo(ListExpr type, string myContactPoint, string myKeyspace)
  
    : contactPoint(myContactPoint), keyspace(myKeyspace),
    cassandra(NULL), result(NULL) {

#ifdef __DEBUG__
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
#endif
      
      // Tuple type
      ListExpr numType = nl->Second(
                         SecondoSystem::GetCatalog()->NumericType((type)));
      tupleType = new TupleType(numType);
      BasicTuple = new Tuple(tupleType);
  
      // build instances for each type
      ListExpr attrList = nl->Second(nl->Second(type));
      while(!nl->IsEmpty(attrList)){
         ListExpr attrType = nl->Second(nl->First(attrList));
         attrList = nl->Rest(attrList);
         int algId;
         int typeId;
         string tname;
         if(! ((SecondoSystem::GetCatalog())->LookUpTypeExpr(attrType,
                                               tname, algId, typeId))){
           cerr << "Unable to build types" << endl;
           return;
         }
         Word w = am->CreateObj(algId,typeId)(attrType);
         instances.push_back(static_cast<Attribute*>(w.addr));
      }   
      
      open();
  }
  
  void open(){
    if(cassandra == NULL) {
       cassandra = CassandraConnectionPool::Instance()->
          getConnection(contactPoint, keyspace, false);
          
      result = cassandra -> getQueriesToExecute();
    }
  }

  Tuple* fetchNextTuple() {
    
    if(result != NULL && result->hasNext()) {
      
      static char c = 0; 
      static string nullstr( &c,1);
      Tuple* resultTuple = BasicTuple->Clone();

      // Attribute 0
      Attribute* attr0 = instances[0]->Clone();
      size_t id = result->getIntValue(0);
      stringstream ss;
      ss << id;
      attr0->ReadFromString(ss.str());
      resultTuple->PutAttribute(0,attr0);
      
      // Attribute 1
      Attribute* attr1 = instances[1]->Clone();
      string myResult;
      result -> getStringValue(myResult, 1);
      attr1->ReadFromString(myResult);
      resultTuple->PutAttribute(1,attr1);
      
      return resultTuple;
    }
    
    return NULL;
  }
  
  virtual ~CQueryListLocalInfo() {
    if(result != NULL) {
      delete result;
      result = NULL;
    }
    
    if(BasicTuple){
       delete BasicTuple;
       BasicTuple = 0;
    }
    
    if(tupleType){
        tupleType->DeleteIfAllowed();
        tupleType = 0;
    }
    
    for(unsigned int i=0; i<instances.size();i++){
        delete instances[i];
    }
    
    instances.clear();
  }
  
  private:
  string contactPoint;         // Contactpoint for our cluster
  string keyspace;             // Keyspace
  CassandraAdapter* cassandra; // Our cassandra connection
  CassandraResult* result;     // Query result
  TupleType* tupleType;        // Tuple Type
  Tuple* BasicTuple;           // Basic tuple
  vector<Attribute*> instances; // Instaces of attributes
};


int CQueryList(Word* args, Word& result, int message, Word& local, Supplier s)
{
  CQueryListLocalInfo *cli; 
  Word elem;
  cli = (CQueryListLocalInfo*)local.addr;
  ListExpr type = qp->GetType(s);
  
  switch(message) {
    case OPEN: 
     
     if ( cli ) delete cli;
     
     if(! ((FText*) args[0].addr)->IsDefined()) {
       cout << "Cluster contactpoint is not defined" << endl;
     } else if (! ((FText*) args[1].addr)->IsDefined()) {
       cout << "Keyspace is not defined" << endl;
     } else {
       cli = new CQueryListLocalInfo(type,
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
      
      // Fetch next query from cassandra
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
2.7.4 Specification of operator ~cquerylist~

*/
const string CQueryListSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text -> stream(tuple(...))</text--->"
                         "<text>clist ( _ , _ ) </text--->"
                         "<text>The operator clist list all schduled queries"
                         "in a given cassandra keyspace</text--->"
                         "<text>query cquerylist('127.0.0.1', "
                         "'keyspace1') consume</text--->"
                              ") )";

/*
2.7.5 Definition of operator ~cquerylist~

*/
Operator cassandraquerylist (
         "cquerylist",                // name
         CQueryListSpec,              // specification
         CQueryList,                  // value mapping
         Operator::SimpleSelect,      // trivial selection function
         CQueryListTypeMap            // type mapping
);                         


/*
2.8 Operator ~cqueryexecute~

The operator ~cqueryexecute~ schedules a query for execuion

The first paramter is the contact point to the cassandra cluster
The second parameter specifies the keyspace to use
The third parameter is the id of the query
The fourth parameter is the query

2.8.1 Type mapping function of operator ~cquerinsert~

Type mapping for ~cqueryexecute~ is

----
  text x text x int x text -> bool
----

*/
ListExpr CQueryInsertTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 4){
    return listutils::typeError("four arguments expected");
  }

  string err = " text x text expected";

  ListExpr contactPoint = nl->First(args);
  ListExpr keyspace = nl->Second(args);
  ListExpr id = nl->Third(args);
  ListExpr query = nl->Fourth(args);

  if(  !FText::checkType(contactPoint) ||
       !FText::checkType(keyspace) ||
       !CcInt::checkType(id) ||
       !FText::checkType(query) ) {
    return listutils::typeError(err);
  }
  
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
2.8.2 Specification of operator ~cqueryexecute~

*/
int CQueryInsert(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  
  switch(message)
  {
    case OPEN: 
    case REQUEST:
    case CLOSE:
      
      result = qp->ResultStorage(s);
      static_cast<CcInt*>(result.addr)->Set(false, 0);
    
      bool resultValue = false;
      
      if(! ((FText*) args[0].addr)->IsDefined()) {
        cout << "Cluster contactpoint is not defined" << endl;
        return CANCEL;
      } else if (! ((FText*) args[1].addr)->IsDefined()) {
        cout << "Keyspace is not defined" << endl;
        return CANCEL;
      } else if (! ((CcInt*) args[2].addr)->IsDefined()) {
        cout << "Query id is not defined" << endl;
        return CANCEL;
      } else if (! ((FText*) args[3].addr)->IsDefined()) {
        cout << "Query is not defined" << endl;
        return CANCEL;
      } else {
      
      string contactPoint = ((FText*) args[0].addr) -> GetValue();
      string keyspace = ((FText*) args[1].addr) -> GetValue();
      int queryid = ((CcInt*) args[2].addr) -> GetValue();
      string query = ((FText*) args[3].addr) -> GetValue();
      
      CassandraAdapter* cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, true);
           
      if(cassandra != NULL && cassandra -> isConnected()) {
        bool createMetatables = cassandra -> createMetatables();
        
        cassandra -> quoteCqlStatement(query);
        
        stringstream ss;
        ss << "INSERT INTO system_queries (id, query) ";
        ss << "values(" << queryid << ", '" << query << "');";
        
        bool insertResult = cassandra 
            -> executeCQLSync(ss.str(), cql::CQL_CONSISTENCY_ALL);
  
        if(! insertResult) {
          cout << "Unable to execute query: " << ss.str() << endl;
        }  
        
        // Copy token ranges to systemtable
        if(queryid == 1) {
          cassandra -> copyTokenRangesToSystemtable(contactPoint);
        }
        
        resultValue = insertResult && createMetatables;
      }
      
      static_cast<CcBool*>(result.addr)->Set(true, resultValue);
      
      return YIELD;
      }
  }
  
  return 0;    
}

/*
2.8.3 Specification of operator ~cqueryexecute~

*/
const string CQueryInsertSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text x int x text -> bool</text--->"
                         "<text>cqueryexecute ( _ , _ , _ , _ ) </text--->"
                         "<text>The operator cqueryexecute schedules a query"
                         "for execuion. </text--->"
                         "<text>query cqueryexecute('127.0.0.1', 'keyspace1', "
                         "1, 'open database opt') "
                         "</text--->"
                              ") )";

/*
2.8.4 Definition of operator ~cqueryexecute~

*/
Operator cassandraqueryinsert (
         "cqueryexecute",            // name
         CQueryInsertSpec,          // specification
         CQueryInsert,              // value mapping
         Operator::SimpleSelect,    // trivial selection function
         CQueryInsertTypeMap        // type mapping
);                         



/*
2.9 Operator ~cquerywait~

The operator ~cquerywait~ waits until the query, given by queryid
is executed completelly

The first paramter is the contact point to the cassandra cluster
The second parameter specifies the keyspace to use
The third parameter is the query id

2.9.1 Type mapping function of operator ~cquerywait~

Type mapping for ~cquerywait~ is

----
  text x text x int -> bool
----

*/
ListExpr CQueryWaitTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 3){
    return listutils::typeError("three arguments expected");
  }

  string err = " text x text x int expected";

  ListExpr contactPoint = nl->First(args);
  ListExpr keyspace = nl->Second(args);
  ListExpr queryId = nl->Third(args);

  if(  !FText::checkType(contactPoint) ||
       !FText::checkType(keyspace) ||
       !CcInt::checkType(queryId)) {
    return listutils::typeError(err);
  }
  
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
2.9.2 Local info for operator ~cqueryreset~

*/
class CQueryWaitLocalInfo {

public:
  CQueryWaitLocalInfo(string myContactPoint, string myKeyspace, int myQueryId) 
  
    : contactPoint(myContactPoint), keyspace(myKeyspace), queryId(myQueryId),
    cassandra(NULL) {

#ifdef __DEBUG__
      cout << "Contact point is " << contactPoint << endl;
      cout << "Keyspace is " << keyspace << endl;
#endif
      
      startTime = time(0);
      
      open();
  }
  
  void open(){
    if(cassandra == NULL) {
        cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);
           
    }
  }
  
  bool getProcessedTokenRangesForQuery(vector<TokenRange> &result) {
      return cassandra -> getProcessedTokenRangesForQuery(result, queryId);
  }
  
  time_t getStartTime() {
    return startTime;
  }
  
  virtual ~CQueryWaitLocalInfo() {

  }
  
private:
  string contactPoint;         // Contactpoint for our cluster
  string keyspace;             // Keyspace
  int queryId;                 // Query id to wait for
  CassandraAdapter* cassandra; // Our cassandra connection
  time_t startTime;
};



class CQueryResetCostEstimation : public CostEstimation {

public:
  
  CQueryResetCostEstimation() {
  }
  
  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {
   
    ProgressInfo p1;    
    CQueryWaitLocalInfo* cwi = (CQueryWaitLocalInfo*) localInfo;

    // Is local info present?
    if(cwi == NULL) {
        return CANCEL;
    }
    
    // Calculate processed token ranges
    vector<TokenRange> processedIntervals;
    unsigned long long max = ULLONG_MAX;
    unsigned long long totalProcessedTokens = 0;
    
    cwi -> getProcessedTokenRangesForQuery(processedIntervals);
    
    for(vector<TokenRange>::iterator iter = processedIntervals.begin(); 
        iter != processedIntervals.end(); ++iter) {
      
      TokenRange interval = *iter;
      totalProcessedTokens = totalProcessedTokens + interval.getSize();
    }
    
    //cout << "Total: " << totalProcessedTokens << endl;
    
    pRes->Card = 1;
    pRes->Progress = (double) totalProcessedTokens / (double) max;
    pRes->BTime = 1;
    pRes->BProgress = 1;
    
    // Calculate total Procesing time
    time_t timediff = time(0) - cwi -> getStartTime();
    pRes->Time = timediff / pRes->Progress;
    
    return YIELD;
  }

  
/*
2.1.2 init our class

*/
  virtual void init(Word* args, void* localInfo) {
    
  }
};

/*
2.9.2 Get Cost Estimation Class for operator ~cqueryreset~

*/
CostEstimation* CQueryWaitCostEstimationFunc() {
  return new CQueryResetCostEstimation();
}


/*
2.9.2 Specification of operator ~cqueryreset~

*/
int CQueryWait(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  
  switch(message)
  {
    case OPEN: 
    case REQUEST:
    case CLOSE:
      
      CQueryWaitLocalInfo* cli = (CQueryWaitLocalInfo*)local.addr;
      if ( cli ) delete cli;
    
      
      result = qp->ResultStorage(s);
      static_cast<CcInt*>(result.addr)->Set(false, 0);
    
      if(! ((FText*) args[0].addr)->IsDefined()) {
        cout << "Cluster contactpoint is not defined" << endl;
        return CANCEL;
      } else if (! ((FText*) args[1].addr)->IsDefined()) {
        cout << "Keyspace is not defined" << endl;
        return CANCEL;
      } else if (! ((CcInt*) args[2].addr)->IsDefined()) {
        cout << "Query-Id is not defined" << endl;
        return CANCEL;
      } else {
          
      string contactPoint = ((FText*) args[0].addr) -> GetValue();
      string keyspace = ((FText*) args[1].addr) -> GetValue();
      int queryId = ((CcInt*) args[2].addr) -> GetValue();
      
      cli = new CQueryWaitLocalInfo(contactPoint, keyspace, queryId);
      local.addr = cli;
 
      cout << "Wait for query id to be executed: " << queryId << endl;
      
      CassandraAdapter* cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);
           
      if(cassandra != NULL && cassandra -> isConnected()) {
        
        while (true) {
          int highestValue = -1;
          
          // Get the global query state
          CassandraResult* result = cassandra -> getGlobalQueryState();
          
          // Determine the highest executed query
          if(result != NULL) {
            while(result -> hasNext()) {
              int lastExecutedQuery = result -> getIntValue(1);
              if(lastExecutedQuery > highestValue) {
                highestValue = lastExecutedQuery;
              }
            }
            
            delete result;
          }
          
          // Is query executed completelly?
          if(highestValue >= queryId) {
            break;
          }
          
          //cout << "Highest value: " << highestValue << endl;
          
          for(size_t i = 0; i < 50000; ++i) {
              // We are blocking the query processor
              // So we need to call CheckProgress 
              // manually, to receive getProgress messages
              qp->CheckProgress();
              usleep(100);
          }
          
        }
        
      }
      
      static_cast<CcBool*>(result.addr)->Set(true, true);
      
      // Delete local info
      delete cli;
      cli = NULL;
      local.addr = NULL;
      
      return YIELD;
      }
  }
  
  return 0;    
}

/*
2.9.3 Specification of operator ~cquerywait~

*/
const string CQueryWaitSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text x int -> bool</text--->"
                         "<text>cquerywait ( _ , _ , _ ) </text--->"
                         "<text>The operator cquerywait blocks until the"
                         "query specified by query id is executed"
                          "completelly</text--->"
                         "<text>query cquerywait('127.0.0.1', 'keyspace1', 3)"
                         "</text--->"
                              ") )";

/*
2.9.4 Definition of operator ~cquerywait~

*/
Operator cassandraquerywait (
         "cquerywait",            // name
         CQueryWaitSpec,          // specification
         CQueryWait,              // value mapping
         Operator::SimpleSelect,  // trivial selection function
         CQueryWaitTypeMap,       // type mapping
         CQueryWaitCostEstimationFunc // Cost Estimation
);                         


/*
2.10 Operator ~cqueryreset~

The operator ~creset~ drops all metatables and recreate 
them into the specified keyspace.

The first paramter is the contact point to the cassandra cluster
The second parameter specifies the keyspace to use

2.10.1 Type mapping function of operator ~cqueryreset~

Type mapping for ~cqueryreset~ is

----
  text x text -> bool
----

*/
ListExpr CQueryResetTypeMap( ListExpr args )
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
  
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
2.10.2 Specification of operator ~cqueryreset~

*/
int CQueryReset(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem;
  
  switch(message)
  {
    case OPEN: 
    case REQUEST:
    case CLOSE:
      
      result = qp->ResultStorage(s);
      static_cast<CcInt*>(result.addr)->Set(false, 0);
    
      bool resultValue = false;
      
      if(! ((FText*) args[0].addr)->IsDefined()) {
        cout << "Cluster contactpoint is not defined" << endl;
        return CANCEL;
      }  else if (! ((FText*) args[1].addr)->IsDefined()) {
        cout << "Keyspace is not defined" << endl;
        return CANCEL;
      } else {
      
      string contactPoint = ((FText*) args[0].addr) -> GetValue();
      string keyspace = ((FText*) args[1].addr) -> GetValue();
      
      CassandraAdapter* cassandra = CassandraConnectionPool::Instance()->
            getConnection(contactPoint, keyspace, false);
           
      if(cassandra != NULL && cassandra -> isConnected()) {
        bool dropResult = cassandra -> dropMetatables();
        
        if(! dropResult) {
          cerr << "Unable to drop metatables" << endl;
        } 
        
        bool createMetatables = cassandra -> createMetatables();
        
        if(! createMetatables ) {
          cerr << "Unable to create metatables" << endl;
        } else {
          cout << "Metatables droped and recreated" << endl;
        }
        
        resultValue = dropResult && createMetatables;
      }
      
      static_cast<CcBool*>(result.addr)->Set(true, resultValue);
      
      return YIELD;
      }
  }
  
  return 0;    
}

/*
2.10.3 Specification of operator ~cqueryreset~

*/
const string CQueryResetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x text -> bool</text--->"
                         "<text>cqueryreset ( _ , _ ) </text--->"
                         "<text>The operator cqueryreset recreate all "
                         "metatables in the specified cassandra keyspace."
                         "All planed and executed queries will be lost"
                         "</text--->"
                         "<text>query cqueryreset('127.0.0.1', 'keyspace1') "
                         "</text--->"
                              ") )";

/*
2.10.4 Definition of operator ~cqueryreset~

*/
Operator cassandraqueryreset (
         "cqueryreset",            // name
         CQueryResetSpec,          // specification
         CQueryReset,              // value mapping
         Operator::SimpleSelect,   // trivial selection function
         CQueryResetTypeMap        // type mapping
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
    cassandracollect.SetUsesArgsInTypeMapping();
    AddOperator(&cassandracollectlocal);
    cassandracollectlocal.SetUsesArgsInTypeMapping();
    AddOperator(&cassandracollectrange);
    cassandracollectrange.SetUsesArgsInTypeMapping();
    AddOperator(&cassandracollectquery);
    cassandracollectquery.SetUsesArgsInTypeMapping();
    AddOperator(&cassandralist);    
    AddOperator(&cassandraquerylist);
    AddOperator(&cassandraqueryinsert);
    AddOperator(&cassandraquerywait); 
    AddOperator(&cassandraqueryreset);
  }
  
  ~CassandraAlgebra()
  {
    // Destory open cassandra connections
    CassandraConnectionPool::Instance()->destroy();
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

