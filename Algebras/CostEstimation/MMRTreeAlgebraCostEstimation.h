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
[1] MMRTreeAlgebraCostEstimation 

Mai, 2012. Jan Kristof Nidzwetzki

[TOC]

0 Description

This file provides some CostEstimationClasses for the MMRTreeAlgebraCostEstimation. 

Mai 2012, JKN, First version of this file

*/

/*
0.1 Defines

*/

#ifndef COST_EST_MMR_ALG_H
#define COST_EST_MMR_ALG_H

#define DEBUG false

/*
1.0 Prototyping

Local info for operator

*/
template<int dim>
class ItSpatialJoinInfo;

/*
1.1 The class ~ItHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator itHashJoin

*/
class ItSpatialJoinCostEstimation : public CostEstimation 
{

public:
    ItSpatialJoinCostEstimation() {
       pli = new ProgressLocalInfo();    
    }    

  virtual ~ItSpatialJoinCostEstimation() {
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

     // Determination of constants in file bin/UpdateProgressConstants

     // Time for processing one tuple in stream 1
     static const double uItSpatialJoin =
        ProgressConstants::getValue("MMRTreeAlgebra",
        "itSpatialJoin", "uItSpatialJoin");

     // Time for processing one tuple in stream 2 (partitions = 1)
     static const double vItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "vItSpatialJoin");

     // msecs per byte written and read from/to TupleFile 
     static const double wItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "wItSpatialJoin");

     // msecs per byte read from TupleFile 
     static const double xItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "xItSpatialJoin");

     // msecs per attr in result tuple 
     static const double yItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "yItSpatialJoin");


     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) {
        
         pli->SetJoinSizes(p1, p2);

        // Read memory for operator in bytes
        size_t maxmem = qp->GetMemorySize(supplier) * 1024 * 1024;

        // Tuplesize from Progress
        size_t sizeOfTuple = p1.Size;

        // we got a correct tuple size? So we use that size!
        if(sizeOfTupleSt1 > 0) {
            sizeOfTuple = sizeOfTupleSt1;
        }
  
        // Calculate number of partitions
        size_t partitions = getNoOfPartitions(p1.Card, sizeOfTuple, maxmem);
       
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
           pRes->Time = p2.Time 
             + (tuplesPerIteration * wItSpatialJoin * p2.Size) 
             + ((partitions - 1) * tuplesPerIteration 
                 * xItSpatialJoin * p2.Size)
                 + p1.Card * uItSpatialJoin + p1.Time;
  
           // Calculate Elapsed time 
           size_t elapsedTime = p2.Time * p2.Progress
                         + (p1.Progress * p1.Card * uItSpatialJoin) 
                         + (p1.Progress * p1.Time);

           if(iteration <= 1) {
              elapsedTime += readInIteration * wItSpatialJoin * p2.Size;
           } else {
                // 1st iteration: Data are read and written from / to tuplefile
                elapsedTime += tuplesPerIteration  * wItSpatialJoin * p2.Size;

                // Time for the completed iterations
                elapsedTime += (iteration - 2) * tuplesPerIteration
                   * xItSpatialJoin * p2.Size;

                // Current iteration
                elapsedTime += readInIteration * xItSpatialJoin * p2.Size;
           }

           // Calculate progress
           pRes->Progress = (double) elapsedTime / (double) pRes->Time;

             if(DEBUG) {
               cout << "DEBUG: ellapsed time " << elapsedTime
                << " of " << pRes->Time << endl;

               cout << "DEBUG: iteration / tuplefileWritten " << iteration
                << " / " << tupleFileWritten << endl;

               cout << "DEBUG: read in iteration " << readInIteration << endl;
               cout << "DEBUG: tuples in tuplefile " << tuplesPerIteration 
                    << endl;
               cout << "DEBUG: tuplesize1 (est) " << p1.Size << " / (real) " 
                   << sizeOfTupleSt1 << endl;
             }

        } else {
           if(DEBUG) {
             cout << p2 << endl;
           }

           pRes->Progress = p2.Progress;
           pRes->Time = p2.Time + p2.Card * vItSpatialJoin 
              + p1.Card * uItSpatialJoin + p1.Time;
        }

         // Blocking time is: adding p1.Card tuples to r-tree
         // and the blocking time of our predecessors
         pRes->BTime = p1.Card * uItSpatialJoin + p1.Time + p1.BTime + p2.BTime;
         pRes->BProgress = ((p1.Progress * p1.Card * uItSpatialJoin) 
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
             * yItSpatialJoin * pRes->Card;
         
          if(DEBUG) {
             cout << "Progress is " << pRes->Progress << endl;
             cout << "Time is " << pRes->Time << endl;
             cout << "BProgress is " << pRes->BProgress << endl;
             cout << "BTime is " << pRes->BTime << endl;
             cout << "Card is: " << pRes->Card << endl;
             cout << "Paritions: " << partitions << endl;
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

     // Time for processing one tuple in stream 2 (partitions = 1)
     static const double uItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "uItSpatialJoin");

     // Time for processing one tuple in stream 2 (partitions = 1)
     static const double vItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "vItSpatialJoin");

     // msecs per byte written and read from/to TupleFile 
     static const double wItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "wItSpatialJoin");

     // msecs per byte read from TupleFile 
     static const double xItSpatialJoin = 
        ProgressConstants::getValue("MMRTreeAlgebra", 
        "itSpatialJoin", "xItSpatialJoin");


 //Calculate number of partitions
 size_t partitions = getNoOfPartitions(NoTuples1, sizeOfTuple1, maxmem);


 if(partitions > 1) { 
      // For partition 1: write 'tuplesInTupleFile' to tuplefile
      // For partition 2+n: read 'tuplesInTupleFile' from tuplefile
      costs =
           NoTuples1 * uItSpatialJoin // place tuples in mmrtree
   
           // write tuples in stream 2 to buffer
           + (NoTuples2 * wItSpatialJoin * sizeOfTuple2)
           
           // read tuples from buffer
           + ((partitions - 1) * xItSpatialJoin * sizeOfTuple2); 
  } else {
      costs = NoTuples1 * uItSpatialJoin + NoTuples2 * vItSpatialJoin; 
  }

  return true;
}


/*
1.4 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples1, size_t sizeOfTuple1) const {
   
   // size per tuple
   // calculate size for one bucket datastructure
   // code taken from MMRTreeAlgebra.cpp 
   size_t sizePerTupleReal = sizeOfTuple1 + sizeof(void*) + 100;
   
   size_t memory = NoTuples1 * sizePerTupleReal;

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
            size_t NoTuples1, size_t sizeOfTuple1,
            size_t NoTuples2, size_t sizeOfTuple2,
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
      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        point1, timeAtPoint1);

      // Calculate costs for second point
      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        point2, timeAtPoint2);

      // Calculate a and b for function f(x) = a/x+b 
      resolveInverseProportionality(point1, timeAtPoint1, point2, 
        timeAtPoint2, a, b);

      getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
        sufficientMemory, timeAtSuffMemory);

      // is point1 at 16mb? => We have already costs for 16mb
      if(point1 == 16) {
         timeAt16MB = timeAtPoint1;
      } else {
         getCosts(NoTuples1, sizeOfTuple1, NoTuples2, sizeOfTuple2, 
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
        // calculate size for one bucket datastructure
        // code taken from MMRTreeAlgebra.cpp 
        size_t sizePerTuple = s1Size + sizeof(void*) + 100;
      
        // calculate max number of tuples in hashtable
        size_t tuplesInMemory = maxmem / sizePerTuple; 
   
        // use a minimum of 10 tuples
        if(tuplesInMemory < 10) {
           tuplesInMemory = 10;
        }

        // calculate number of partitions
        size_t noOfPartitions = 
          floor(((double) s1Card / (double) tuplesInMemory) + 0.8);

        if(DEBUG) {
           cout << "DEBUG: Size per Tuple: " << sizePerTuple << endl;
           cout << "DEBUG: Tuples in memory are: " << tuplesInMemory << endl;
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
       iteration = iter;
    }

/*
1.13 Setter for readInIteration

*/
   void incReadInIteration() {
       readInIteration++;
   }

/*
1.14 Reset read in iteration

*/
   void resetReadInIteration() {
      readInIteration = 0;
   }

/*
1.15 Set number of tuples in tuplefile

*/
   void incTuplesInTupleFile() {
      tuplesInTupleFile++;
   }

/*
1.16 Set number of tuples in tuplefile

*/
   void setTuplesInTupleFile(size_t tuples) {
      tuplesInTupleFile = tuples;
   }

/*
1.17 Set tupleFileWritten state

*/
   void setTupleFileWritten(bool state) {
     tupleFileWritten = state;
   }   

/*
1.18 Setter for Min RTree

*/
   void setMinRtree(size_t value) {
      minRtree = value;
   }

/*
1.19 Setter for Max RTree

*/
  void setMaxRtree(size_t value) {
     maxRtree = value;
  }

/*
1.20 Setter for sizeOfTupleSt1

*/
  void setSizeOfTupleSt1(size_t size) {
     sizeOfTupleSt1 = size;
  }


/*
1.21 Set readPartitionDone state

*/
   void readPartitionDone() {
      if(partitionSize == 0) {
         partitionSize = readStream1;
      }
   }


/*
1.22 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
    stream1Exhausted = false;
    stream2Exhausted = false;
    tupleFileWritten = false;
    readStream1 = 0;
    readStream2 = 0;
    iteration = 1;
    readInIteration = 0;
    tuplesInTupleFile = 0;
    sizeOfTupleSt1 = 0;
    partitionSize = 0;
  }

private:
  ProgressLocalInfo *pli;   // Progress local info
  ProgressInfo p1, p2;      // Progress info for stream 1 / 2
  bool stream1Exhausted;    // is stream 1 exhaused?
  bool stream2Exhausted;    // is stream 2 exhaused?
  bool tupleFileWritten;    // is the tuplefile completely written?
  size_t readStream1;       // processed tuple in stream1
  size_t readStream2;       // processes tuple in stream2
  size_t iteration;         // number of iteration in operator
  size_t readInIteration;   // no of tuples read in this iteration
  size_t tuplesInTupleFile; // number of tuples in tuplefile
  size_t sizeOfTupleSt1;    // size of a tuple in stream 1 (RootSize);
  size_t minRtree;          // min rtree
  size_t maxRtree;          // max rtree
  size_t partitionSize;     // size of a partition
};


#endif
