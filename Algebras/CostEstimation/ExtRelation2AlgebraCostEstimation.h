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
[1] ExtRelation2AlgebraCostEstimation 

Mai, 2012. Jan Kristof Nidzwetzki

[TOC]

0 Description

This file provides some CostEstimationClasses for the ExtRelation2Algebra. 

Mai 2012, JKN, First version of this file

*/

/*
0.1 Defines

*/

#ifndef COST_EST_EXT_RELATION_ALG_H
#define COST_EST_EXT_RELATION_ALG_H

#define DEBUG false 

/*
1.0 Prototyping

Local info for operator

*/

class ItHashJoinDInfo;

/*
1.1 The class ~ItHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator itHashJoin

*/
class ItHashJoinCostEstimation : public CostEstimation 
{

public:
    ItHashJoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
1.0 Free local datastructures

*/
  virtual ~ItHashJoinCostEstimation() {
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

      // Determination of constants in file bin/UpdateProgressConstants
      
      // Time for processing one tuple in stream 1
      static const double uItHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "itHashJoin", "uItHashJoin");

      // Time for processing one tuple in stream 2 (partitions = 1)
      static const double vItHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "itHashJoin", "vItHashJoin");

      // msecs per byte written and read from/to TupleFile 
      static const double wItHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "itHashJoin", "wItHashJoin");

      // msecs per byte read from TupleFile 
      static const double xItHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "itHashJoin", "xItHashJoin");


     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) 
      {     
        pli->SetJoinSizes(p1, p2);

        // Read memory for operator in bytes
        size_t maxmem = qp->GetMemorySize(supplier) * 1024 * 1024;
        
        // Calculate number of partitions
        size_t partitions = getNoOfPartitions(p1.Card, p1.Size, maxmem);

        if(partitions > 1) {
           // total tuples
           size_t totalTuples = p2.Card;
              
           // is the tuplefile written completely? Otherwise we assume
           // that all tuples of p2 are written to tuplefile
           if(tupleFileWritten == false) {
              totalTuples = totalTuples + (partitions - 1) * p2.Card;
           } else {
              totalTuples = totalTuples + (partitions - 1) * tuplesInTupleFile;
           }
 
           // Processed tuples
           size_t processedTuples = p2.Progress * p2.Card;

           // Add tuples for completed and current iteration
           processedTuples = processedTuples + (iteration - 1) 
             * tuplesInTupleFile + readInIteration;

           // Calculate progress
           pRes->Progress = (double) processedTuples / (double) totalTuples;

           if(DEBUG) {
              cout << "DEBUG: processed " << processedTuples 
                << " of " << totalTuples << endl;
              
              cout << "DEBUG: iteration / tuplefile " << iteration 
                << " / " << tupleFileWritten << endl;
              
              cout << "DEBUG: read in iteration " << readInIteration << endl;
           }

           // For partition 2: read / write 'tuplesInTupleFile' to tuplefile
           // For partition 2+n: read 'tuplesInTupleFile' from tuplefile
           pRes->Time = p1.Time + p2.Time 
              + (tuplesInTupleFile * wItHashJoin * p2.Size) 
              + ((partitions - 1) * xItHashJoin * p2.Size);
           
           } else {
              pRes->Progress = p2.Progress;
              pRes->Time = p1.Time + p2.Time + p2.Card * vItHashJoin; 
           }

          // Calculate selectivity
          if(qp->GetSelectivity(supplier) == 0.1) {
              pRes->Card = p1.Card * p2.Card;
          } else {
             pRes->Card = qp->GetSelectivity(supplier) * p1.Card * p2.Card;
          }

          pRes->BProgress = p1.Progress;
          pRes->BTime = p1.Card * uItHashJoin + p1.BTime + p2.BTime;

          // is computation done?
          if(stream1Exhausted && stream2Exhausted) {
             pRes->Progress = 1.0;
             pRes->BProgress = 1.0;
             pRes->Card = returned;
          } 

          if(DEBUG) {
             cout << "Progress is " << pRes->Progress << endl;
             cout << "Time is " << pRes->Time << endl;
             cout << "BProgress is " << pRes->BProgress << endl;
             cout << "BTime is " << pRes->BTime << endl;
          }

          pRes->CopySizes(pli);

          return YIELD;
       }

   // default: send cancel
   return CANCEL;
}


/*
1.1. getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(size_t NoTuples1, size_t sizeOfTuple1,
                  size_t NoTuples2, size_t sizeOfTuple2,
                  size_t memoryMB, size_t &costs){
   

 // Init calculation
 size_t maxmem = memoryMB * 1024 * 1024;
 setTuplesInTupleFile(NoTuples2);

 // Read variables
 
 // Time for processing one tuple in stream 2 (partitions = 1)
 static const double vItHashJoin = 
    ProgressConstants::getValue("ExtRelation2Algebra", 
    "itHashJoin", "vItHashJoin");

 // msecs per byte written and read from/to TupleFile 
 static const double wItHashJoin = 
    ProgressConstants::getValue("ExtRelation2Algebra", 
    "itHashJoin", "wItHashJoin");

 // msecs per byte read from TupleFile 
 static const double xItHashJoin = 
    ProgressConstants::getValue("ExtRelation2Algebra", 
    "itHashJoin", "xItHashJoin");

 //Calculate number of partitions
 size_t partitions = getNoOfPartitions(NoTuples1, sizeOfTuple1, maxmem);

 if(partitions > 1) { 
      // For partition 2: read / write 'tuplesInTupleFile' to tuplefile
      // For partition 2+n: read 'tuplesInTupleFile' from tuplefile
      costs = 
           + (NoTuples2 * wItHashJoin * sizeOfTuple2) 
           + ((partitions - 1) * xItHashJoin * sizeOfTuple2);
  } else {
      costs = NoTuples2 * vItHashJoin; 
  }
         
     return true;
}


/*
Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples2, size_t sizeOfTuple2) {

        // calculate size for one bucket datastructure
        vector<Tuple*>* bucket = new vector<Tuple*>();
           
        sizePerBucket = sizePerBucket + sizeof(bucket);
        sizePerBucket = sizePerBucket + sizeof(void*) * bucket->capacity();
           
        delete bucket;
        bucket = NULL;

        // calculate size of the whole datastructure
        size_t memoryOfDatastruct = sizePerBucket * buckets; 

        size_t memory = memoryOfDatastruct + (NoTuples2 * sizeOfTuple2);

        return ceil(memory / (1024 * 1024));
}

/*
Get Linear Params
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
            double& timeAt16MB ) { 
      
      sufficientMemory=calculateSufficientMemory(NoTuples2, sizeOfTuple2);
     /* 
      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        sufficientMemory, timeAtSuffMemory);

      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        16, timeAt16MB);
      */
      return true;
   }

/*
1.3 getFunction

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
            double& a, double& b, double& c, double& d){
       functionType=1;
       a=0;b=0;c=0;d=0;
       return getLinearParams(NoTuples1, sizeOfTuple1,
                              NoTuples2, sizeOfTuple2,
                              sufficientMemory, timeAtSuffMemory, 
                              timeAt16MB);  
  }  
   

/*
Calculate the numer of partitions for this operator

*/
size_t getNoOfPartitions(size_t s1Card, size_t s1Size, size_t maxmem) {

        // calculate size for one bucket datastructure
        if(sizePerBucket == 0) {
           vector<Tuple*>* bucket = new vector<Tuple*>();
           
           sizePerBucket = sizePerBucket + sizeof(bucket);
           sizePerBucket = sizePerBucket + sizeof(void*) * bucket->capacity();
           
           delete bucket;
           bucket = NULL;
        }

        // calculate size of the whole datastructure
        size_t memoryOfDatastruct = sizePerBucket * buckets; 

        // calculate max number of tuples in hashtable
        size_t tuplesInMemory = (maxmem - memoryOfDatastruct) / s1Size;
        
        // calculate number of partitions
        size_t noOfPartitions = ceil((double) s1Card / (double) tuplesInMemory);

        if(DEBUG) {
           cout << "DEBUG: Size of datastucture is: " 
              << memoryOfDatastruct << endl;
           
           cout << "DEBUG: Tuples is memory is: " << tuplesInMemory << endl;
           cout << "DEBUG: No of partitons is: " << noOfPartitions << endl;
        }

        return noOfPartitions;
}

/*
Setter for stream1Exhausted

*/
  void setStream1Exhausted(bool exhausted) {
      stream1Exhausted = exhausted;
  }

/*
Setter for stream2Exhausted

*/
  void setStream2Exhausted(bool exhausted) {
      stream2Exhausted = exhausted;
  }


/*
Update processed tuples in stream1

*/
   void processedTupleInStream1() {
      readStream1++;
   }

/*
Update processed tuples in stream2

*/
    void processedTupleInStream2() {
       readStream2++;
    }

/*
Setter for iterattion

*/
    void setIteration(size_t iter) {
       iteration = iter;
    }

/*
Setter for Buckets

*/
    void setBuckets(size_t bucketno) {
       buckets = bucketno;
    }

/*
Setter for readInIteration

*/
   void incReadInIteration() {
       readInIteration++;
   }

/*
Reset read in iteration

*/
   void resetReadInIteration() {
      readInIteration = 0;
   }

/*
Set number of tuples in tuplefile

*/
   void incTuplesInTupleFile() {
      tuplesInTupleFile++;
   }

/*
Set number of tuples in tuplefile

*/
   void setTuplesInTupleFile(size_t tuples) {
      tuplesInTupleFile = tuples;
   }

/*
Set tupleFileWritten state

*/
   void setTupleFileWritten(bool state) {
      tupleFileWritten = state;
   }

/*
1.1.0 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    stream1Exhausted = false;
    stream2Exhausted = false;
    tupleFileWritten = false;
    readStream1 = 0;
    readStream2 = 0;
    iteration = 0;
    readInIteration = 0;
    buckets = 999997; // default buckets
    tuplesInTupleFile = 0;
    sizePerBucket = 0;
  }

private:
  ProgressLocalInfo *pli;
  ProgressInfo p1, p2;
  bool stream1Exhausted;    // is stream 1 exhaused?
  bool stream2Exhausted;    // is stream 2 exhaused?
  bool tupleFileWritten;    // is the tuplefile completely written?
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
  size_t iteration;         // number of iteration in operator
  size_t readInIteration;   // no of tuples read in this iteration
  size_t buckets;           // number of buckets
  size_t tuplesInTupleFile; // number of tuples in tuplefile
  size_t sizePerBucket;     // size per bucket
};


#endif
