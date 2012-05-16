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
         cout << "No args available, cancel" << endl;
         return CANCEL;
     }

     if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2)) 
      {     
        pli->SetJoinSizes(p1, p2);

        // is computation dome?
        if(stream1Exhausted && stream2Exhausted) {
           pRes->Progress = 1.0;
           pRes->BProgress = 1.0;
           pRes->Card = returned;
           pRes->Time = 1.0;  // FIXME
        } else {
           if(getNoOfPartitions(p1.Card) > 1) {
              pRes->Progress = p2.Progress;
              pRes->Time = p1.Time + p1.Card * p2.Time; // FIXME
           } else {
              pRes->Progress = p2.Progress;
              pRes->Time = p1.Time + p1.Card * p2.Time; // FIXME
           }
           
           cout << "p1 is " << p1.Progress << endl;
           cout << "p2 is " << p2.Progress << endl;

           // Calculate selectivity
           if(qp->GetSelectivity(supplier) == 0.1) {
              pRes->Card = p1.Card * p2.Card;
           } else {
              pRes->Card = qp->GetSelectivity(supplier) * p1.Card * p2.Card;
           }

           pRes->BProgress = p1.Progress;
           pRes->BTime = p1.BTime * 0.1; //FIXME
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
         costs = 0;
         return false;
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
      sufficientMemory=0;
      timeAtSuffMemory=0;
      timeAt16MB=0;
      return false;
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
size_t getNoOfPartitions(size_t s1Card) {

        size_t noOfPartitions = 0;
        
        // size of hash table is currently unknown
        if(noOfTuplesInHashTable == 0) {
           noOfPartitions = s1Card / (readStream1 + 1);
        } else {
           noOfPartitions = s1Card / (noOfTuplesInHashTable + 1);
        }

        //cout << "DEBUG: No of partitons is: " << noOfPartitions << endl;

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
Trigger hashTable if full event, so we can define 
the number of tuples per partitions

*/
   void triggerHashTableFull() {
      
      //cout << "DEBUG: Trigger at " << readStream1 << endl;
      
      if(noOfTuplesInHashTable == 0) {
         noOfTuplesInHashTable = readStream1;
      }
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
1.1.0 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    cout << "----> INIT CALLED" << endl;
    returned = 0;
    stream1Exhausted = false;
    stream1Exhausted = false;
    noOfTuplesInHashTable = 0;
    readStream1 = 0;
    readStream2 = 0;
    iteration = 0;
  }

private:
  ProgressLocalInfo *pli;
  ProgressInfo p1, p2;
  bool stream1Exhausted; // is stream 1 exhaused?
  bool stream2Exhausted; // is stream 2 exhaused?
  size_t noOfTuplesInHashTable;    // number of Tuples in HashTable
  size_t readStream1;    // processed tuple in stream1
  size_t readStream2;    // processes tuple in stream2
  size_t iteration;      // iteration in operator
};


#endif
