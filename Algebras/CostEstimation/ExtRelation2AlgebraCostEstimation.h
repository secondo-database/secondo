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
#include <SortMergeJoin.h>

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
    ItHashJoinCostEstimation() : buckets(999997)
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
              + ((partitions - 1) * tuplesPerIteration * xItHashJoin * p2.Size)
              +  p1.Card * uItHashJoin + p1.Time;

           // Calculate Elapsed time 
           size_t elapsedTime = p2.Time * p2.Progress 
                                + (p1.Progress * p1.Card * uItHashJoin)
                                + (p1.Progress * p1.Time);

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
              pRes->Time = p2.Time + p2.Card * vItHashJoin 
                + p1.Card * uItHashJoin + p1.Time ; 
           }
    
          // Blocking time is: adding p1.Card tuples to hashtable
          // and the blocking time of our predecessors
          pRes->BTime = p1.Card * uItHashJoin + p1.Time + p1.BTime + p2.BTime;
          pRes->BProgress = ((p1.Progress * p1.Card * uItHashJoin) 
            + (p1.Progress * p1.Time) + (p1.BProgress * p1.BTime) 
            + (p2.BProgress * p2.BTime)) / pRes->BTime;

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

          // Append time for creating new tuples. Assume that the creation 
          // of new tuples is equally distributed during the calculation. So 
          // we can add the time without affecting the progress calculation
          pRes->Time += (p1.noAttrs + p2.noAttrs)
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
                      const size_t noAttributes1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      const double memoryMB, double &costs) const{

 // Init calculation
 size_t maxmem = memoryMB * 1024 * 1024;
 // Read variables

  cerr << __PRETTY_FUNCTION__ << endl
       << "TODO: use of noAttributes and selectivity" << endl;

 
 // Time for processing one tuple in stream 2 (partitions = 1)
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

 //Calculate number of partitions
 size_t partitions = getNoOfPartitions(NoTuples1, sizeOfTuple1, maxmem);

 if(partitions > 1) { 
      // For partition 1: write 'tuplesInTupleFile' to tuplefile
      // For partition 2+n: read 'tuplesInTupleFile' from tuplefile
      costs =
           NoTuples1 * uItHashJoin // place tuples in hash table
           
           // write tuples in stream 2 to buffer
           + (NoTuples2 * wItHashJoin * sizeOfTuple2) 

           // read tuples from buffer
           + ((partitions - 1) * xItHashJoin * sizeOfTuple2); 
  } else {
      costs = NoTuples1 * uItHashJoin + NoTuples2 * vItHashJoin; 
  }

  // Write Debug output to file. Needed if called from optimizer
  if(DEBUG) {
     ofstream file;
     file.open("/tmp/secondolog",  ios::out | ios::app);
     file << "Called with NoTuples1 " << NoTuples1 
        << " sizeOfTuple1 " << sizeOfTuple1 << endl;
     file << "Called with NoTuples2 " << NoTuples2 
        << " sizeOfTuple2 " << sizeOfTuple2 << endl;
     file << "Partitions is " << partitions << endl;
     file << "Memory is " << maxmem << endl;
     file << "Costs " << costs << endl << endl;
     file.close();
  }

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

        double suffMemory = ceil(memory / (1024 * 1024));

        // At least 16 mb are required
        return max(16.0, suffMemory);
}

/*
1.6 getFunction

This function approximates the costfunction by an parametrizable
function. Allowed types are:

1: linear function
2: a / x

*/
   virtual bool getFunction(
            const size_t NoTuples1, const size_t sizeOfTuple1, 
            const size_t noAttributes1,
            const size_t NoTuples2, const size_t sizeOfTuple2,
            const size_t noAttributes2,
            const double selectivity,
            int& functionType,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB,
            double& a, double& b, double& c, double& d) const {

      // Function is a/x + b
      functionType=2;
      
      // Init variables
      a = 0; b = 0; c = 0; d = 0;

      // Calculate sufficientMemory and time at sufficientMemory and 16MB
      sufficientMemory=calculateSufficientMemory(NoTuples2, sizeOfTuple2);
      
      // Points for resolving parameter
      double point1, point2, timeAtPoint1, timeAtPoint2;
      calculateXPoints(sufficientMemory, point1, point2);

      // Calculate costs for first point
      getCosts(NoTuples1, sizeOfTuple1, noAttributes1, 
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
               point1, timeAtPoint1);

      // Calculate costs for second point
      getCosts(NoTuples1, sizeOfTuple1, noAttributes1, 
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
               point2, timeAtPoint2);

      // Calculate a and b for function f(x) = a/x+b 
      resolveInverseProportionality(point1, timeAtPoint1, point2, 
        timeAtPoint2, a, b);
      
      getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
               sufficientMemory, timeAtSuffMemory);

      // is point1 at 16mb? => We have already costs for 16mb
      if(point1 == 16) {
         timeAt16MB = timeAtPoint1;
      } else {
         getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
                  NoTuples2, sizeOfTuple2, noAttributes2,
                  selectivity,
                  16, timeAt16MB);
      }

      return true;
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
        // calculate size for one bucket
        vector<Tuple*>* bucket = new vector<Tuple*>();
        size_t sizePerBucket = sizeof(bucket);
        delete bucket;
        bucket = NULL;
        

        // Handle low memory situations, hashtable is to big for memory
        // Recalcualte size of buckets (needed if called from optimizer and 
        // not from operator)
        size_t realBuckets = buckets;

        if(buckets * sizePerBucket > maxmem / 5){
           // reduce size of table when table structure takes more than
           // 20 percent of the available memory
           realBuckets = maxmem / (5 * sizeof(void*));
        
           if(realBuckets < 3){
             realBuckets = 3;
           }
        }
        
        // calculate size of the whole datastructure
        size_t memoryOfDatastruct = sizePerBucket * realBuckets; 

        // calculate max number of tuples in hashtable
        size_t tuplesInMemory = (maxmem - memoryOfDatastruct)
           / (s1Size + sizeof(void*));
        
        // calculate number of partitions
        size_t noOfPartitions = ceil((double) s1Card / (double) tuplesInMemory);

        if(DEBUG) {
           ofstream file;
           file.open("/tmp/secondolog",  ios::out | ios::app);
           file << "s1Card " << s1Card << " s1Size " << s1Size << endl;
           file << "Memory is " << maxmem << endl;
           file << "Real buckets " << realBuckets << endl;
           file << "DEBUG: Size of datastucture is: " 
              << memoryOfDatastruct << endl;
           
           file << "DEBUG: Size per Bucket: " << sizePerBucket << endl;
           file << "DEBUG: Tuples is memory are: " << tuplesInMemory << endl;
           file << "DEBUG: total Tuples are: " << s1Card << endl;
           file << "DEBUG: No of partitons is: " << noOfPartitions << endl;
           file << endl << endl;
           file.close();
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

       // if localInfo is deleted, and we have
       // a old ProgressInfo. Use them and assume
       // that the calculation is done
       if(pi.Time > 0) {
          pRes->Copy(pi);
          pRes -> BProgress = 1;
          pRes -> Progress = 1;
          pRes -> Card = returned;
          return YIELD;
       }
       return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) {     

        li->CalcProgress(p1, p2, pRes, supplier);

        // Copy values, if the localInfo value of the 
        // operator is deleted, we can use the last progress
        // estimation
        pi.Copy(*pRes);

        return YIELD;
     } 

   // default: send cancel
   return CANCEL;
}


/*
2.2 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      const double memoryMB, double &costs) const{

     cerr << __PRETTY_FUNCTION__ << endl
          << "TODO: implement use of noAttributes and selectivity" << endl;

     double uHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "uHashJoin");
 
     double vHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "vHashJoin");

     double t_read = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "tread");
 
     double t_write = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "twrite");
 
     double t_probe = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "tprobe");
 
     double t_hash = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "thash");
     
     // Internal or external mode?
     if(calculateSufficientMemory(NoTuples1, sizeOfTuple1, 
         NoTuples2, sizeOfTuple2) > memoryMB) {

        // External mode
        costs = NoTuples1 * ( t_probe + t_hash + t_read + t_write ) +
        NoTuples2 * ( t_hash + t_read + t_write );
     } else {
        // Internal mode
         costs = NoTuples1 * vHashJoin  // reading stream B into hash table
                + NoTuples2 * uHashJoin; // probing stream A against hash table
     }

     return true;
}


/*
2.3 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1, 
  const size_t NoTuples2, const size_t sizeOfTuple2) const {

   // Space for placing all tuples in memory
   double suffMemory = ceil((NoTuples2 * sizeOfTuple2) / (1024 * 1024));

   // At least 16 mb are required
   return max(16.0, suffMemory);

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
            const size_t NoTuples1, const size_t sizeOfTuple1,
            const size_t noAttributes1,
            const size_t NoTuples2, const size_t sizeOfTuple2,
            const size_t noAttributes2,
            const double selectivity,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB )  const { 
      
      sufficientMemory=calculateSufficientMemory(NoTuples1, sizeOfTuple1,
        NoTuples2, sizeOfTuple2);
      
      getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,    
               sufficientMemory, timeAtSuffMemory);

      getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
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
   virtual bool getFunction(
            const size_t NoTuples1, const size_t sizeOfTuple1, 
            const size_t noAttributes1,
            const size_t NoTuples2, const size_t sizeOfTuple2,
            const size_t noAttributes2,
            const double selectivity,
            int& functionType,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB,
            double& a, double& b, double& c, double& d) const {

       cout << __PRETTY_FUNCTION__ << endl
            << "TODO : use of noAttributes and selectivity" << endl;

       functionType=1;
       a=0;b=0;c=0;d=0;
       return getLinearParams(NoTuples1, sizeOfTuple1, noAttributes1,
                              NoTuples2, sizeOfTuple2, noAttributes2,
                              selectivity,
                              sufficientMemory, timeAtSuffMemory, 
                              timeAt16MB);  
  }  
   
/*
2.6 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  ProgressInfo pi;          // Progress Info
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
       // if localInfo is deleted, and we have
       // a old ProgressInfo. Use them and assume
       // that the calculation is done
       if(pi.Time > 0) {
          pRes->Copy(pi);
          pRes -> BProgress = 1;
          pRes -> Progress = 1;
          pRes -> Card = returned;
          return YIELD;
       }

       return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) {
          li->CalcProgress(p1, p2, pRes, supplier);
          
          // Copy values, if the localInfo value of the 
          // operator is deleted, we can use the last progress
          // estimation
          pi.Copy(*pRes); 
          
          return YIELD; 
     } 

   // default: send cancel
   return CANCEL;
}

/*
3.2 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      const double memoryMB, double &costs) const{

     cerr << __PRETTY_FUNCTION__ << endl
          << "TODO: implement use of noAttributes and selectivity" << endl;

     double uHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "uHashJoin");
 
     double vHashJoin = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "vHashJoin");

     double t_read = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "tread");
 
     double t_write = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "twrite");
 
     double t_probe = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "tprobe");
 
     double t_hash = 
        ProgressConstants::getValue("ExtRelation2Algebra", 
        "gracehashjoin", "thash");
     
     // Internal or external mode?
     if(calculateSufficientMemory(NoTuples1, 
        sizeOfTuple1, NoTuples2, sizeOfTuple2) > memoryMB) {

        // External mode

        // No of tuples in partition 0
        double card0 = (memoryMB * 1024 * 1024) / sizeOfTuple2;


        costs = NoTuples1 * ( t_probe + t_hash + t_read + t_write ) +
        NoTuples2 * ( t_hash + t_read + t_write )

        // Tuples in Partition 0 are hold in memory
        -  card0 * ( t_read + t_write );
     } else {
        // Internal mode
         costs = 
                + NoTuples1 * vHashJoin   // reading stream B into hash table
                + NoTuples2 * uHashJoin; // probing stream A against hash table
     }

     return true;
}


/*
3.3 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1, 
  const size_t NoTuples2, const size_t sizeOfTuple2) const {

   // Space for placing all tuples in memory
   double suffMemory = ceil((NoTuples2 * sizeOfTuple2) / (1024 * 1024));

   // At least 16 mb are required
   return max(16.0, suffMemory);
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
            const size_t NoTuples1, const size_t sizeOfTuple1,
            const size_t noAttributes1,
            size_t NoTuples2, size_t sizeOfTuple2,
            const size_t noAttributes2,
            const double selectivity,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB )  const { 
      
      sufficientMemory=calculateSufficientMemory(NoTuples1, sizeOfTuple1,
        NoTuples2, sizeOfTuple2);
      
      getCosts(NoTuples1, sizeOfTuple1, noAttributes1, 
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
               sufficientMemory, timeAtSuffMemory);

      getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
               16, timeAt16MB);
      
      return true;
   }
 
/*
3.5 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
  ProgressLocalInfo *pli;   // Local Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  ProgressInfo pi;          // Progress Info
};



/*
4.0 The class ~SortMergeJoinCostEstimation~ provides cost estimation
    capabilities for the operator sortmergejoin

*/
class SortMergeJoinCostEstimation : public CostEstimation 
{

public:
    SortMergeJoinCostEstimation()
    {    
       pli = new LocalInfo<extrel2::SortMergeJoinLocalInfo>();
    }    

/*
4.1 Free local datastructures

*/
  virtual ~SortMergeJoinCostEstimation() {
     if(pli) {
        delete pli;
     }
     pli = 0;
  };

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {

     // no progress info available => cancel
     if(! argsAvialable) {
         return CANCEL;
     }

     ProgressInfo p1, p2;

     

     //millisecs per byte read in sort step
     static const double uSortBy =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "uSortBy");

     //millisecs per byte read in merge step (sortmerge)
     const double wMergeJoin = 
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "wMergeJoin");

     // millisecs per attr in result tuple (0.0001072)
     static const double yMergeJoin =
           ProgressConstants::getValue("Global",
           "ResultTuple", "attr");


     extrel2::SortProgressLocalInfo* liFirst;
     extrel2::SortProgressLocalInfo* liSecond;

     if( !localInfo )
     {
       // if localInfo is deleted and we have
       // an old ProgressInfo. Use them and assume
       // that the calculation is done
       if(pi.Time > 0) {
          pRes->Copy(pi);
          pRes -> BProgress = 1;
          pRes -> Progress = 1;
          pRes -> Card = returned;
          return YIELD;
       }
       return CANCEL;
     }
     
     typedef LocalInfo<extrel2::SortMergeJoinLocalInfo> LocalType;
     LocalType* li = static_cast<LocalType*>( localInfo );

     liFirst = static_cast<extrel2::SortProgressLocalInfo*>
                    (li->firstLocalInfo);
     liSecond = static_cast<extrel2::SortProgressLocalInfo*>
                    (li->secondLocalInfo);


     if (qp->RequestProgress(args[0].addr, &p1)
         && qp->RequestProgress(args[1].addr, &p2))
     {
          pli->SetJoinSizes(p1, p2);

          pRes->CopySizes(pli);

          long readFirst = (liFirst ? liFirst->read : 0);
          long readSecond = (liSecond ? liSecond->read : 0);

          double factor = (double) li->readFirst / p1.Card;

          // Calculate result cardinality
          if ( returned > (size_t) enoughSuccessesJoin )
          {
            double m = (double)returned;
            double k1 = (double)li->readFirst;
            double k2 = (double)li->readSecond;

            // estimated selectivity
            double sel = m / ( k1 * k2 );

            // warm state
            if ( qp->GetSelectivity(supplier) != 0.1 )
            {
              // estimated selectivity from optimizer is used
              // as more tuples are processed the weight of the
              // optimizer estimation is reduced
              pRes->Card = ( p1.Card * p2.Card ) *
                           ( factor * sel +
                             ( 1.0 - factor ) * qp->GetSelectivity(supplier) );
            }
            else
            {
              // if optimizer is not used use only estimation
              pRes->Card = sel * p1.Card * p2.Card;
            }
          }
          else
          {
            // cold state
            pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(supplier);
          }

          // total time
          pRes->Time = p1.Time + p2.Time +
                     + p1.Card * p1.Size * uSortBy +
                     + p2.Card * p2.Size * uSortBy +
                     + (p1.Card * p1.Size + p2.Card * p2.Size) * wMergeJoin
                     + pRes->Card * (pRes->noAttrs * yMergeJoin);

          pRes->Progress = (   p1.Progress * p1.Time
                             + p2.Progress * p2.Time
                             + ((double) readFirst) * p1.Size * uSortBy
                             + ((double) readSecond) * p2.Size * uSortBy
                             + (((double) li->readFirst) * p1.Size
                             + ((double) li->readSecond) * p2.Size)
                               * wMergeJoin
                             + ((double) li->returned)
                               * (pRes->noAttrs * yMergeJoin) )
                           / pRes->Time;

          // first result tuple is possible after both input streams
          // deliver the first tuples and both sort algorithm have
          // consumed their streams completely
          pRes->BTime =   p1.BTime
                        + p2.BTime
                        + p1.Card * p1.Size * uSortBy
                        + p2.Card * p2.Size * uSortBy;

          // blocking progress
          pRes->BProgress = (   p1.BProgress * p1.BTime
                              + p2.BProgress * p2.BTime
                              + ((double) readFirst) * p1.Size * uSortBy
                              + ((double) readSecond) * p2.Size * uSortBy )
                        / pRes->BTime;

          // Copy values, if the localInfo value of the 
          // operator is deleted, we can use the last progress
          // estimation
          pi.Copy(*pRes);

          return YIELD;
     } else {
         return CANCEL;
     }

   // default: send cancel
   return CANCEL;
}

/*
4.2 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t noAttributes1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const size_t noAttributes2,
                      const double selectivity,
                      const double memoryMB, double &costs) const{

      cerr << __PRETTY_FUNCTION__ << endl
           << "TODO: implement use of noAttributes and selectivity" << endl;
 
     //millisecs per byte read in sort step
     static const double uSortBy =
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "uSortBy");

     //millisecs per byte read in merge step (sortmerge)
     const double wMergeJoin = 
           ProgressConstants::getValue("ExtRelationAlgebra",
           "mergejoin", "wMergeJoin");

     // Time for sorting (uSortBy) + time for merging (wMergeJoin)
     costs = NoTuples1 * sizeOfTuple1 * uSortBy +
             NoTuples2 * sizeOfTuple2 * uSortBy +
             (NoTuples1 * sizeOfTuple1 + NoTuples2 * sizeOfTuple2) 
             * wMergeJoin;
         
     return true;
}


/*
4.3 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1, 
   size_t NoTuples2, size_t sizeOfTuple2) const {

   // Space for in memory sorting of both streams
   // + 20% memory for merge
   double suffMemory = ceil(((NoTuples1 * sizeOfTuple1 
      + NoTuples2 * sizeOfTuple2) * 1.2) / (1024 * 1024));

   // At least 16 mb are required
   return max(16.0, suffMemory);

}

/*
4.4 Get Linear Params
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
            const size_t NoTuples1, const size_t sizeOfTuple1,
            const size_t noAttributes1,
            const size_t NoTuples2, const size_t sizeOfTuple2,
            const size_t noAttributes2,
            const double selectivity,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB )  const { 
      
      sufficientMemory=calculateSufficientMemory(NoTuples1, sizeOfTuple1, 
         NoTuples2, sizeOfTuple2);
      
      getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,   
               sufficientMemory, timeAtSuffMemory);

      getCosts(NoTuples1, sizeOfTuple1, noAttributes1,
               NoTuples2, sizeOfTuple2, noAttributes2,
               selectivity,
               16, timeAt16MB);
      
      return true;
   }

/*
4.5 getFunction

This function approximates the costfunction by an parametrizable
function. Allowed types are:

1: linear function
2: a / x

*/
   virtual bool getFunction(
            const size_t NoTuples1, const size_t sizeOfTuple1,
            const size_t noAttributes1,
            size_t NoTuples2, size_t sizeOfTuple2,
            const size_t noAttributes2,
            const double selectivity,
            int& functionType,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB,
            double& a, double& b, double& c, double& d) const {
       functionType=1;
       a=0;b=0;c=0;d=0;
       return getLinearParams(NoTuples1, sizeOfTuple1, noAttributes1,
                              NoTuples2, sizeOfTuple2, noAttributes2,
                              selectivity,
                              sufficientMemory, timeAtSuffMemory, 
                              timeAt16MB);  
  }  
   
/*
4.6 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

private:
  LocalInfo<extrel2::SortMergeJoinLocalInfo> *pli;   // Local Progress info
  ProgressInfo pi;          // Progress info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
};



#endif
