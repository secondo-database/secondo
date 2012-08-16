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

#ifndef COST_EST_EXT2_RELATION_ALG_H
#define COST_EST_EXT2_RELATION_ALG_H

#define DEBUG false 

#include <GraceHashJoin.h>
#include <HybridHashJoin.h>

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
1.2 Free local datastructures

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

      // msecs per attr in result tuple 
       static const double yItHashJoin = 
         ProgressConstants::getValue("ExtRelation2Algebra", 
         "itHashJoin", "yItHashJoin");

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) 
      {     
        pli->SetJoinSizes(p1, p2);

        // Read memory for operator in bytes
        size_t maxmem = qp->GetMemorySize(supplier) * 1024 * 1024;
        
        // Calculate number of partitions
        size_t partitions = getNoOfPartitions(p1.Card, p1.Size, maxmem);

        // Number of tuples per iteration
        size_t tuplesPerIteration = p2.Card;

        // is the tuplefile written completely? Otherwise we assume
        // that all tuples of p2 are written to tuplefile
        if(tupleFileWritten) {
            tuplesPerIteration = tuplesInTupleFile;
        } 

        if(partitions > 1) {

           // For partition 1: write 'tuplesInTupleFile' to tuplefile
           // For partition 1+n: read 'tuplesInTupleFile' from tuplefile
           pRes->Time = p2.Time + (tuplesPerIteration * wItHashJoin * p2.Size) 
              + ((partitions - 1) * tuplesPerIteration * xItHashJoin * p2.Size);

           // Calculate Elapsed time 
           size_t elapsedTime = p2.Time * p2.Progress;

           if(iteration <= 1) {
              elapsedTime += readInIteration * wItHashJoin * p2.Size;
           } else {
                // 1st iteration: Tuples are written to tuplefile
                elapsedTime += tuplesPerIteration  * wItHashJoin * p2.Size;
                
                // Time for the completed iterations
                elapsedTime += (iteration - 2) * tuplesPerIteration 
                   * xItHashJoin * p2.Size; 

                // Current iteration
                elapsedTime += readInIteration * xItHashJoin * p2.Size;
           }
          
           // Calculate progress
           pRes->Progress = (double) elapsedTime / (double) pRes->Time;

             if(DEBUG) {
               cout << "DEBUG: ellapsed / it " << elapsedTime 
                << " of " << pRes->Time << " / " << iteration << endl;
              
               cout << "DEBUG: iteration / tuplefile " << iteration 
                << " / " << tupleFileWritten << endl;
              
               cout << "DEBUG: read in iteration " << readInIteration << endl;
             }
           
           } else {

              if(DEBUG) {
                 cout << p2 << endl;
              }

              pRes->Progress = p2.Progress;
              pRes->Time = p2.Time + p2.Card * vItHashJoin; 
           }
    
          // Blocking time is: adding p1.Card tuples to hashtable
          // and the blocking time of our predecessors
          pRes->BTime = p1.Card * uItHashJoin + p1.Time + p1.BTime + p2.BTime;
          pRes->BProgress = ((p1.Progress * p1.Card * uItHashJoin) 
            + (p1.Progress * p1.Time) + (p1.BProgress * p1.BTime) 
            + (p2.BProgress * p2.BTime)) / pRes->BTime;

          // Add Blocking time to normal time
          // and merge progress values
          pRes->Time += pRes->BTime;
          pRes->Progress = (pRes->Time * pRes->Progress 
               + pRes->BTime * pRes->BProgress) 
               / (pRes->Time + pRes->BTime);

          // Calculate cardinality
          // Warm state or cold state?
          if(qp->GetSelectivity(supplier) == 0.1 
             && returned >= (size_t) enoughSuccessesJoin) {
             pRes->Card = returned / pRes->Progress;
          } else {
             pRes->Card = qp->GetSelectivity(supplier) * p1.Card * p2.Card;
          }

          // is computation done?
          if(stream1Exhausted && stream2Exhausted) {
             pRes->Progress = 1.0;
             pRes->BProgress = 1.0;
             pRes->Card = returned;
          } 

          // Append time for creating new tuples
          pRes->Time += p1.noAttrs + p2.noAttrs 
                     * yItHashJoin * pRes->Card;

          if(DEBUG) {
             cout << "Progress is " << pRes->Progress << endl;
             cout << "Time is " << pRes->Time << endl;
             cout << "BProgress is " << pRes->BProgress << endl;
             cout << "BTime is " << pRes->BTime << endl;
             cout << "Card is: " << pRes->Card << endl;
             cout << "Partitions is: " << partitions << endl;
             cout << "Card is: " << pRes->Card << endl;
             cout << "Returned / Progress" << returned 
                 << " / " <<  pRes->Progress << endl;
          }
             
          pRes->CopySizes(pli);

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

 // Init calculation
 size_t maxmem = memoryMB * 1024 * 1024;
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

 // msecs per attr in result tuple 
 static const double yItHashJoin = 
    ProgressConstants::getValue("ExtRelation2Algebra", 
    "itHashJoin", "yItHashJoin");


 //Calculate number of partitions
 size_t partitions = getNoOfPartitions(NoTuples1, sizeOfTuple1, maxmem);

 if(partitions > 1) { 
      // For partition 1: write 'tuplesInTupleFile' to tuplefile
      // For partition 2+n: read 'tuplesInTupleFile' from tuplefile
      costs = 
           + (NoTuples2 * wItHashJoin * sizeOfTuple2) 
           + ((partitions - 1) * xItHashJoin * sizeOfTuple2);
  } else {
      costs = NoTuples2 * vItHashJoin; 
  }


  // Add costs for creating new tuples
  // with default selectivity of 0.1
  costs += NoTuples1 * NoTuples2 
          * sizeOfTuple1 * sizeOfTuple2 
          * yItHashJoin * 0.1;
         
     return true;
}


/*
1.4 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1) const {

        // calculate size for one bucket datastructure
        vector<Tuple*>* bucket = new vector<Tuple*>();
           
        size_t sizePerBucket =  sizeof(bucket);
        sizePerBucket += sizeof(void*) * bucket->capacity();
           
        delete bucket;
        bucket = NULL;

        // calculate size of the whole datastructure
        size_t memoryOfDatastruct = sizePerBucket * buckets; 

        size_t memory = memoryOfDatastruct + (NoTuples1 * sizeOfTuple1);

        return ceil(memory / (1024 * 1024));
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
1.7 Calculate the numer of partitions for this operator

*/
size_t getNoOfPartitions(size_t s1Card, size_t s1Size, size_t maxmem) const {
        
        // if the first stream is exhausted, we are in the last
        // partition / iteration
        if(stream1Exhausted) {
           return iteration;
        }

        // if we have a partition size
        // use them
        if(partitionSize > 0) {
           return ceil(s1Card / partitionSize) + 1;
        }

        // otherwise we must estimate
        // calculate size for one bucket datastructure
        vector<Tuple*>* bucket = new vector<Tuple*>();
        size_t sizePerBucket = sizeof(bucket);
        delete bucket;
        bucket = NULL;
          
        // Memory for datastruct
         if(s1Card / buckets > 10) {
            sizePerBucket += sizeof(void*) * (s1Card / buckets);      
         } else {
            sizePerBucket += sizeof(void*) * 10;
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
           
           cout << "DEBUG: Size per Bucket: " << sizePerBucket << endl;
           cout << "DEBUG: Tuples is memory are: " << tuplesInMemory << endl;
           cout << "DEBUG: total Tuples are: " << s1Card << endl;
           cout << "DEBUG: No of partitons is: " << noOfPartitions << endl;
        }

        return noOfPartitions;
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
1.11 Update processed tuples in stream2

*/
    void processedTupleInStream2() {
       readStream2++;
    }

/*
1.12 Setter for iterattion

*/
    void setIteration(size_t iter) {
       
       // reset read counter
       if(iteration != iter) {
          readInIteration = 0;
       }

       iteration = iter;
    }

/*
1.13 Setter for Buckets

*/
    void setBuckets(size_t bucketno) {
       buckets = bucketno;
    }

/*
1.14 Setter for readInIteration

*/
   void incReadInIteration() {
       readInIteration++;
   }

/*
1.15 Reset read in iteration

*/
   void resetReadInIteration() {
      readInIteration = 0;
   }

/*
1.16 Set number of tuples in tuplefile

*/
   void incTuplesInTupleFile() {
      tuplesInTupleFile++;
   }

/*
1.17 Set number of tuples in tuplefile

*/
   void setTuplesInTupleFile(size_t tuples) {
      tuplesInTupleFile = tuples;
   }

/*
1.18 Set tupleFileWritten state

*/
   void setTupleFileWritten(bool state) {
      tupleFileWritten = state;
   }

/*
1.19 Set readPartitionDone state

*/
   void readPartitionDone() {
      if(partitionSize == 0) {
         partitionSize = readStream1;
      }
   }

/*
1.20 init our class

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
    readInIteration = 1;
    buckets = 999997; // default buckets
    tuplesInTupleFile = 0;
    partitionSize = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  bool stream1Exhausted;    // is stream 1 exhaused?
  bool stream2Exhausted;    // is stream 2 exhaused?
  bool tupleFileWritten;    // is the tuplefile completely written?
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
  size_t iteration;         // number of iteration in operator
  size_t readInIteration;   // no of tuples read in this iteration
  size_t buckets;           // number of buckets
  size_t tuplesInTupleFile; // number of tuples in tuplefile
  size_t partitionSize;     // size of a partition
};


/*
2.0 The class ~GraceHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator gracehashjoin

*/
class GraceHashJoinCostEstimation : public CostEstimation 
{

public:
    GraceHashJoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
2.1 Free local datastructures

*/
  virtual ~GraceHashJoinCostEstimation() {
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

     extrel2::GraceHashJoinLocalInfo* li;
     li = static_cast<extrel2::GraceHashJoinLocalInfo*>( localInfo );
     
     if( !li ) {
          return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) {     

        return li->CalcProgress(p1, p2, pRes, supplier);
     } else {
         return CANCEL;
     }

   // default: send cancel
   return CANCEL;
}


/*
2.2 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const double memoryMB, double &costs) const{

         
     return true;
}


/*
2.3 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1) const {
   return 16.0; // FIXME
}

/*
2.4 Get Linear Params
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
2.5 getFunction

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
2.6 Setter for stream1Exhausted

*/
  void setStream1Exhausted(bool exhausted) {
      stream1Exhausted = exhausted;
  }

/*
2.7 Setter for stream2Exhausted

*/
  void setStream2Exhausted(bool exhausted) {
      stream2Exhausted = exhausted;
  }


/*
2.8 Update processed tuples in stream1

*/
   void processedTupleInStream1() {
      readStream1++;
   }

/*
2.9 Update processed tuples in stream2

*/
    void processedTupleInStream2() {
       readStream2++;
    }

/*
2.10 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    stream1Exhausted = false;
    stream2Exhausted = false;
    readStream1 = 0;
    readStream2 = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  bool stream1Exhausted;    // is stream 1 exhaused?
  bool stream2Exhausted;    // is stream 2 exhaused?
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
};



/*
3.0 The class ~HybridHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator hybridhashjoin

*/
class HybridHashJoinCostEstimation : public CostEstimation 
{

public:
    HybridHashJoinCostEstimation()
    {    
       pli = new ProgressLocalInfo();
    }    

/*
3.1 Free local datastructures

*/
  virtual ~HybridHashJoinCostEstimation() {
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

     extrel2::HybridHashJoinLocalInfo* li;
     li = static_cast<extrel2::HybridHashJoinLocalInfo*>( localInfo );

     if( !li ) {
        return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) {     
          return li->CalcProgress(p1, p2, pRes, supplier);
     } else {
          return CANCEL;
     }

   // default: send cancel
   return CANCEL;
}


/*
3.2 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const double memoryMB, double &costs) const{

         
     return true;
}


/*
3.3 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1) const {
     return 16.0; // FIXME
}

/*
3.4 Get Linear Params
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
3.5 getFunction

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
3.6 Setter for stream1Exhausted

*/
  void setStream1Exhausted(bool exhausted) {
      stream1Exhausted = exhausted;
  }

/*
3.7 Setter for stream2Exhausted

*/
  void setStream2Exhausted(bool exhausted) {
      stream2Exhausted = exhausted;
  }


/*
3.8 Update processed tuples in stream1

*/
   void processedTupleInStream1() {
      readStream1++;
   }

/*
3.9 Update processed tuples in stream2

*/
    void processedTupleInStream2() {
       readStream2++;
    }

/*
1.10 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    stream1Exhausted = false;
    stream2Exhausted = false;
    readStream1 = 0;
    readStream2 = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  bool stream1Exhausted;    // is stream 1 exhaused?
  bool stream2Exhausted;    // is stream 2 exhaused?
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
};



#endif
