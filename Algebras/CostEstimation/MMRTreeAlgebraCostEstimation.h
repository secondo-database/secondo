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

/*
1.0 Prototyping

Local info for operator

*/

class ItSpatialJoinInfo;

/*
1.1 The class ~ItHashJoinCostEstimation~ provides cost estimation
    capabilities for the operator itHashJoin

*/
class ItSpatialJoinCostEstimation : public CostEstimation 
{

public:
    ItSpatialJoinCostEstimation()
    {    
    }    

  virtual ~ItSpatialJoinCostEstimation() {};

  virtual int requestProgress(Word* args, ProgressInfo* pRes, void* localInfo, 
    bool argsAvialable) {

     return 0;
  }

/*
1.1.0 init our class

*/
  virtual void init(Word* args, void* localInfo)
  {
    returned = 0;
  }

/*
1.3 getCosts

Returns the estimated time in ms for given arguments.

*/
virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                      const size_t NoTuples2, const size_t sizeOfTuple2,
                      const double memoryMB, double &costs) const{
   return false;
}


/*
1.4 Calculate the sufficent memory for this operator.

*/
double calculateSufficientMemory(size_t NoTuples2, size_t sizeOfTuple2) const {
   return false;
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

   return false;

 }

private:
};


#endif
