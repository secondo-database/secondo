/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2013, University in Hagen, Faculty of Mathematics and
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
2.8 Operator ~Sleep~

This operator sleeps for a configurable time

2.8.1 Type mapping function of operator ~sleep~

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
2.8.2 Generic Cost Estimation Function

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
      pRes->Time = p1.Time * getDelay(localInfo);
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
  virtual int getDelay(void* localInfo) { return 1; }
  
/*
2.8.2 init our class

*/
  virtual void init(Word* args, void* localInfo) {
    
  }
};

struct SleepLocalInfo
{
  SleepLocalInfo( const int numDelay = 0 ):
    delay( numDelay )
    {}

  int delay; // in ms
};

/*
2.8.2 Specialized Cost Estimation function for operator ~sleep~

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
    
    // Convert to ms
    return li -> delay;     
  }
};

CostEstimation* SleepCostEstimationFunc() {
  return new SleepCostEstimation();
}


/*
2.8.3 Value mapping function of operator ~sleep~

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
      return 0;
  }
  return 0;
}


/*
2.8.4 Specification of operator ~sleep~

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
2.8.5 Definition of operator ~head~

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
    //addOperator

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

