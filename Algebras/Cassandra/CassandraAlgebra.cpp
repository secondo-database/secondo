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

2.1.1 Type mapping function of operator ~statistics~

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
    : interval(myInterval), iteration(0), filename(myFilename) {
      
      //cout << "Filename is " << filename 
      //     << " interval is " << interval << endl;
      
      // Open file for output
      filehandle.open(filename.c_str());
      if(! filehandle.is_open() ) {
        cout << "Unable to open file: " << filename << endl;
      } else {
        filehandle << "Time,Tuples" << endl;
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
    timeval curtime;
    gettimeofday(&curtime, NULL); 
    
    if(lastdump.tv_sec + interval <= curtime.tv_sec) {
      dumpTuples();
      reset();
    }
      
    seenTuples++;
  }
  
  // Reset seenTuples counter
  void reset() {
    gettimeofday(&lastdump, NULL); 
    seenTuples = 0;
  }
  
  // Close file
  void close() {
    filehandle.close();
  }
  
  // Dump seen tuples to output file
  void dumpTuples() {
    //cout << "Cur tuples " << seenTuples << endl;
    filehandle << iteration * interval << "," << seenTuples << endl;
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
2.1.4 Specification of operator ~statistics~

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
                         "interval for the statistics (sec). </text--->"
                         "<text>query intstream(1,10) transformstream "
                         "statistics['/tmp/statistics.csv', 1] count</text--->"
                              ") )";

/*
2.1.5 Definition of operator ~statistics~

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

