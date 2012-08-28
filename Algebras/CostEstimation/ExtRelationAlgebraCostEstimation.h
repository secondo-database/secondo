/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] ExtRelationAlgebraCostEstimation 

Jun, 2012. First Revision - Jan Kristof Nidzwetzki

[TOC]

0 Description

This file provides some CostEstimationClasses for the ExtRelationAlgebra. 

Jun 2012, JKN, First version of this file

*/

/*
0.1 Defines

*/

#ifndef COST_EST_EXT_RELATION_ALG_H
#define COST_EST_EXT_RELATION_ALG_H

#define DEBUG false 

/*

0.2 Includes

*/

#include "SortByLocalInfo.h"

/*
1.0 Prototyping

Local info for operator

*/


/*
1.1 The class ~ItHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator itHashJoin

*/

// Operation mode: join or select
// this class are used in loopjoin
// and loopsel 
template<bool join>
class LoopJoinCostEstimation : public CostEstimation 
{

public:
    LoopJoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
1.2 Free local datastructures

*/
  virtual ~LoopJoinCostEstimation() {
     if(pli) {
        delete pli;
     }
  };

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {

     // no progress info available => cancel
     if(! argsAvialable) {
         return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) 
      {     
        if(join) {
           pli->SetJoinSizes(p1, p2);
           pRes->CopySizes(pli);
        } else {
           // tuples are axtracted from fun relation
           // for each tuplex, all matching tupley will be collected,
           // therefore the time for the query for tupley is multiplied 
           pRes->CopySizes(p2);
        }

         if (returned > (size_t) enoughSuccessesJoin) {     
           pRes->Card = p1.Card  *
            ((double) (returned) /
              (double) (readStream1));
          } else {
            pRes->Card = p1.Card  * p2.Card;
          } 

          pRes->Time = p1.Time + p1.Card * p2.Time;

          if (stream1Exhausted) {
             pRes->Progress = 1.0;
          } else if ( p1.BTime < 0.1 && pipelinedProgress ) { 
             pRes->Progress = p1.Progress;
          } else {
             pRes->Progress =
              (p1.Progress * p1.Time + (double) readStream1 * p2.Time)
               / pRes->Time;

             pRes->CopyBlocking(p1);  //non-blocking operator;
                                      //second argument assumed not to block
          }

          return YIELD;
       }

   // default: send cancel
   return CANCEL;
}

/*
1.8 Setter for stream1Exhausted

*/
  void setStream1Exhausted(bool exhausted) {
      stream1Exhausted = exhausted;
  }

/*
1.9 Update processed tuples in stream1

*/
   void processedTupleInStream1() {
      readStream1++;
   }

/*
1.10 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    stream1Exhausted = false;
    readStream1 = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  bool stream1Exhausted;    // is stream 1 exhaused?
  size_t readStream1;       // processed tuple in stream1
};



/*
1.1 The class ~symmjoinCostEstimation~ provides cost estimation
    capabilities for the operator symmjoin

*/
class SymmjoinCostEstimation : public CostEstimation 
{

public:
    SymmjoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
1.2 Free local datastructures

*/
  virtual ~SymmjoinCostEstimation() {
     if(pli) {
        delete pli;
     }
  };

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {

     // no progress info available => cancel
     if(! argsAvialable) {
         return CANCEL;
     }

     // Read memory for operator in bytes
     size_t maxmem = qp->GetMemorySize(supplier) * 1024 * 1024;

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) 
      {     
        pli->SetJoinSizes(p1, p2);

        // millisecs per tuple pair 0.2
        static const double uSymmJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "symmjoin", "uSymmJoin");
        
        // millisecs per byte written
        static const double twrite =
           ProgressConstants::getValue("Global",
           "TupleFile", "twrite");

        // millisecs per byte read
        static const double tread =
           ProgressConstants::getValue("Global",
           "TupleFile", "tread");
        
        // millisecs per attr in result tuple
        static const double tattr =
           ProgressConstants::getValue("Global",
           "ResultTuple", "attr");


        if (!pli){
            return CANCEL;
        }

        pRes->CopySizes(pli);

        double predCost =
          (qp->GetPredCost(supplier) == 0.1 ? 0.004 
          : qp->GetPredCost(supplier));

        //the default value of 0.1 is only suitable for selections

        if (returned > (size_t) enoughSuccessesJoin ) {   
         // stable state assumed now
          pRes->Card = p1.Card * p2.Card *
            ((double) returned /
              (double) (readStream1 * readStream2));
        } else {
          pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(supplier);
        }

        // % of tuples in stream 1 are written to disk
        double t1d = percentTupleOnDisk(p1.Card, p1.Size, maxmem / 2);

        // % of tuples in stream 2 are written to disk
        double t2d = percentTupleOnDisk(p2.Card, p2.Size, maxmem / 2);

        // Tuples in Buffer 1 or Buffer 2 are written to disk?
        double wtd1 = 0;
        double wtd2 = 0;

        if(t1d > 0) {
          wtd1 = 1;
        }

        if(t2d > 0) {
          wtd2 = 1;
        }

        pRes->Time = p1.Time + p2.Time
          + p1.Card * p2.Card * predCost * uSymmJoin
          + pRes->Card * tattr * (p1.noAttrs + p2.noAttrs)
          + wtd1 * p1.Size * p1.Card * twrite  
          + wtd2 * p2.Size * p2.Card * twrite  
          + t1d * p1.Size * p1.Card * tread * p2.Card
          + t2d * p2.Size * p2.Card * tread * p1.Card;

        if(stream1Exhausted && stream2Exhausted) {
           pRes->Progress = 1.0;
        } else {
          pRes->Progress =
            (p1.Progress * p1.Time + p2.Progress * p2.Time 
            + readStream1 * readStream2 * predCost * uSymmJoin
            + returned * tattr * (p1.noAttrs + p2.noAttrs)
            + tupleBuffer1OnDisk * p1.Size * p1.Card * twrite 
            + tupleBuffer2OnDisk * p2.Size * p2.Card * twrite 
            + t1d * p1.Size * p1.Card * tread * p2.Card * p2.Progress
            + t2d * p2.Size * p2.Card * tread * p1.Card * p1.Progress
            ) / pRes->Time;

            /* Debug
            cout << "r1 " << readStream1 << endl;
            cout << "r2 " << readStream2 << endl;
            cout << "wtd1 " << wtd1 << endl;
            cout << "t1d " << t1d << endl;
            cout << "wtd2 " << wtd2 << endl;
            cout << "t2d " << t2d << endl;
            */
        }


        pRes->CopyBlocking(p1, p2);  //non-blocking oprator

        return YIELD;
     }

   // default: send cancel
   return CANCEL;
}


/*
1.3 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const double memoryMB, double &costs) const{

        // millisecs per tuple pair (0.2)
        static const double uSymmJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "symmjoin", "uSymmJoin");

        // millisecs per byte written
        static const double twrite =
           ProgressConstants::getValue("Global",
           "TupleFile", "twrite");

        // millisecs per byte read
        static const double tread =
           ProgressConstants::getValue("Global",
           "TupleFile", "tread");
        
        // Tuples in Buffer 1 or Buffer 2 are written to disk?
        double wtd1, wtd2;

        // % of tuples in stream 1 are written to disk
        double t1d = percentTupleOnDisk(NoTuples1, sizeOfTuple1, memoryMB / 2);

        // % of tuples in stream 2 are written to disk
        double t2d = percentTupleOnDisk(NoTuples2, sizeOfTuple2, memoryMB / 2);

        if(t1d > 0) {
          wtd1 = 1;
        }

        if(t2d > 0) {
          wtd2 = 1;
        }

        costs = NoTuples1 * NoTuples2 * uSymmJoin
                + wtd1 * NoTuples1 * sizeOfTuple1 * twrite
                + t1d * NoTuples1 * sizeOfTuple1 * tread * NoTuples2
                + wtd2 * NoTuples2 * sizeOfTuple2 * twrite
                + t2d * NoTuples2 * sizeOfTuple2 * tread * NoTuples1;
 
        return true;
}


/*
1.4 percecent of Tuple written to disk

*/
double percentTupleOnDisk(const size_t NoTuples, 
   const size_t sizeOfTuple, const double memoryMB) const {

   double tupleInMemory = 
      1 - (memoryMB / (NoTuples * sizeOfTuple / 1024 * 1024));
  
   if(tupleInMemory < 0) {
      tupleInMemory = 0;
   }
   
   return tupleInMemory;
}

/*
1.4 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, 
  size_t sizeOfTuple1, size_t NoTuples2, 
  size_t sizeOfTuple2) const {

  // Space for placing all tuples in memory
  return NoTuples1 * sizeOfTuple1 
    + NoTuples2 * sizeOfTuple2 / 1024 * 1024;

}

/*
1.5 Get Linear Params
Input: 
NoTuples1, sizeOfTuple1
NoTuples2, sizeOfTuple2,

Output:
sufficientMemory = sufficientMemory for this operator with the given 
                   input

timeAtSuffMemory = Time for the calculation with sufficientMemory

timeAt16MB - Time for the calculation with 16MB Memory

*/
   virtual bool getLinearParams(
            size_t NoTuples1, size_t sizeOfTuple1,
            size_t NoTuples2, size_t sizeOfTuple2,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB )  const { 
      
      sufficientMemory=calculateSufficientMemory(NoTuples1, sizeOfTuple1,
         NoTuples2, sizeOfTuple2);
      
      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        sufficientMemory, timeAtSuffMemory);

      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        16, timeAt16MB);
      
      return true;
   }

/*
1.8 Setter for stream1Exhausted

*/
  void setStream1Exhausted(bool exhausted) {
      stream1Exhausted = exhausted;
  }

/*
1.9 Setter for stream2Exhausted

*/
  void setStream2Exhausted(bool exhausted) {
      stream2Exhausted = exhausted;
  }


/*
1.10 Update processed tuples in stream1

*/
   void processedTupleInStream1() {
      readStream1++;
   }

/*
1.11 Tuple Buffer 1 on disk

*/
   void setTupleBuffer1OnDisk(bool onDisk) {
      tupleBuffer1OnDisk = onDisk;
   }

/*
1.12 Tuple Buffer 2 on disk

*/
   void setTupleBuffer2OnDisk(bool onDisk) {
      tupleBuffer2OnDisk = onDisk;
   }

/*
1.13 Update processed tuples in stream2

*/
    void processedTupleInStream2() {
       readStream2++;
    }

/*
1.14 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    stream1Exhausted = false;
    stream2Exhausted = false;
    tupleBuffer1OnDisk = false;
    tupleBuffer2OnDisk = false;
    readStream1 = 0;
    readStream2 = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  bool stream1Exhausted;    // is stream 1 exhaused?
  bool stream2Exhausted;    // is stream 2 exhaused?
  bool tupleBuffer1OnDisk;  // is tuple buffer 1 written to disk?
  bool tupleBuffer2OnDisk;  // is tuple buffer 2 written to disk?
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
};

/*
1.1 The class ~MergeJoinCostEstimation~ provides cost estimation
    capabilities for the operator mergejoin and sortmergejoin\_old

*/

/*

1.1.1 Prototyping

*/

template<bool expectSorted>
class MergeJoinCostEstimation : public CostEstimation 
{

public:
    MergeJoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
1.2 Free local datastructures

*/
  virtual ~MergeJoinCostEstimation() {
     if(pli) { 
        delete pli;
     }
  }

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {

     // no progress info available => cancel
     if(! argsAvialable) {
         return CANCEL;
     }
     
     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) {
      
      //millisecs per byte read in sort step (0.00043)
      static const double uSortBy =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "uSortBy");

      // millisecs per tuple read in merge step (merge) (0.0008077)
      static const double uMergeJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "uMergeJoin");
      
      //millisecs per byte read in merge step (sortmerge) 
      // (0.0001738)
      static const double wMergeJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "wMergeJoin");
      
      //millisecs per result tuple in merge step (0.0012058)
      static const double xMergeJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "xMergeJoin");

      //millisecs per result attribute in merge step
      // (0.0001072)
      static const double yMergeJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "yMergeJoin");

      pli->SetJoinSizes(p1, p2);
      pRes->CopySizes(pli);

      double factor = (double) readStream1 / p1.Card;

      if ( (qp->GetSelectivity(supplier) != 0.1) &&
           ( returned > (size_t) enoughSuccessesJoin) ) {
     
            pRes->Card = factor * ((double) returned) * p1.Card
                        / ((double) readStream1) +
                        (1.0 - factor) * p1.Card * p2.Card
                        * qp->GetSelectivity(supplier);
      } else {
          if ( returned > (size_t) enoughSuccessesJoin ) {
            pRes->Card = ((double) returned) * p1.Card
                          /  ((double) readStream1);
          } else {
            pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(supplier);
          }
      }

      if ( expectSorted ) {
            pRes->Time = p1.Time + p2.Time +
              (p1.Card + p2.Card) * uMergeJoin +
              pRes->Card * (xMergeJoin + pRes->noAttrs * yMergeJoin);

            pRes->Progress =
                (p1.Progress * p1.Time + p2.Progress * p2.Time +
                (((double) readStream1) + ((double) readStream2))
                * uMergeJoin +
                ((double) returned)
                * (xMergeJoin + pRes->noAttrs * yMergeJoin))
                / pRes->Time;
            
            //non-blocking in this case
            pRes->CopyBlocking(p1, p2);

       } else {

            pRes->Time =
               p1.Time + p2.Time +
               p1.Card * p1.Size * uSortBy +
               p2.Card * p2.Size * uSortBy +
               (p1.Card * p1.Size + p2.Card * p2.Size) * wMergeJoin +
               pRes->Card * (xMergeJoin + pRes->noAttrs * yMergeJoin);


            long readFirst = readStream1;
            long readSecond = readStream2;

            pRes->Progress =
               (p1.Progress * p1.Time +
               p2.Progress * p2.Time +
               ((double) readFirst) * p1.Size * uSortBy +
               ((double) readSecond) * p2.Size * uSortBy +
               (((double) readStream1) * p1.Size +
               ((double) readStream2) * p2.Size) * wMergeJoin +
               ((double) returned)
               * (xMergeJoin + pRes->noAttrs * yMergeJoin))
               / pRes->Time;

            pRes->BTime = p1.Time + p2.Time
               + p1.Card * p1.Size * uSortBy
               + p2.Card * p2.Size * uSortBy;
          
            pRes->BProgress =
               (p1.Progress * p1.Time + p2.Progress * p2.Time
               + ((double) readFirst) * p1.Size * uSortBy
               + ((double) readSecond) * p2.Size * uSortBy)
               / pRes->BTime;
        }
          
       return YIELD;
   }
   
   // default: send cancel
   return CANCEL;
}


/*
1.3 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const double memoryMB, double &costs) const{

     //millisecs per byte read in sort step (0.00043)
     static const double uSortBy =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "uSortBy");

     //millisecs per byte read in merge step (sortmerge) 
     // (0.0001738)
     static const double wMergeJoin =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "wMergeJoin");
      

     // Time for creating new tuples 
     costs = NoTuples1 * NoTuples2 * wMergeJoin * 0.1
     
     // Time for processing a byte in input
           + uSortBy * (NoTuples1 + NoTuples2) * (sizeOfTuple1 + sizeOfTuple2);
     
     return true;
}


/*
1.4 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1) const {
   return 16;
}


/*
1.5 Get Linear Params
Input: 
NoTuples1, sizeOfTuple1
NoTuples2, sizeOfTuple2,

Output:
sufficientMemory = sufficientMemory for this operator with the given 
                   input

timeAtSuffMemory = Time for the calculation with sufficientMemory

timeAt16MB - Time for the calculation with 16MB Memory

*/
   virtual bool getLinearParams(
            size_t NoTuples1, size_t sizeOfTuple1,
            size_t NoTuples2, size_t sizeOfTuple2,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB )  const { 
      
      sufficientMemory=calculateSufficientMemory(NoTuples2, sizeOfTuple2);
      
      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        sufficientMemory, timeAtSuffMemory);

      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        16, timeAt16MB);
      
      return true;
   }


/*
1.6 getFunction

This function approximates the costfunction by an parametrizable
function. Allowed types are:

1: linear function
2: a / x

*/
   virtual bool getLinearParams(
            size_t NoTuples1, size_t sizeOfTuple1,
            size_t NoTuples2, size_t sizeOfTuple2,
            int& functionType,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB,
            double& a, double& b, double& c, double& d) const {
       functionType=1;
       a=0;b=0;c=0;d=0;
       return getLinearParams(NoTuples1, sizeOfTuple1,
                              NoTuples2, sizeOfTuple2,
                              sufficientMemory, timeAtSuffMemory, 
                              timeAt16MB);  
  }  


/*
1.7 Update processed tuples in stream1

*/
   void processedTupleInStream1() {
      readStream1++;
   }


/*
1.8 Update processed tuples in stream2

*/
    void processedTupleInStream2() {
       readStream2++;
    }


/*
1.9 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    readStream1 = 0;
    readStream2 = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
};


#endif
