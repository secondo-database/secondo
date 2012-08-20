
/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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


1 CostEstimation

*/

#ifndef COST_ESTIMATION_H
#define COST_ESTIMATION_H

#include "AlgebraTypes.h"

// forward declarations
class ProgressInfo;

/*
1.1 Class CostEstimation

*/

class CostEstimation{
  public:
    CostEstimation() :  supplier(0), returned(0) {}

    virtual ~CostEstimation(){}

    virtual int requestProgress(Word* args,
                                ProgressInfo* result,
                                void* localInfo,
                                const bool argsAvailable) = 0;




    virtual void init(Word* args, void* localInfo) = 0;
                                
    void setSupplier(Supplier s){
        supplier = s;
    }

    inline void incReturned(){
        returned++;
    }

    inline size_t getReturned() const{
        return returned;
    }

/*
1 Static Cost Estimation


1.1. getCosts

Returns the estimated time in ms for given arguments.
For more than one tuple stream as input, use the other
version of this function.

*/

    virtual bool getCosts(const size_t NoTuples, const size_t sizeOfTuple,
                          const double memoryMB, double& costs) const{
         costs = 0;
         return false;
    }
    
    virtual bool getCosts(const size_t NoTuples1, const size_t sizeOfTuple1,
                          const size_t NoTuples2, const size_t sizeOfTuple2,
                          const double memoryMB, double &costs) const{
         costs = 0;
         return false;
    }

/*
1.2. getLinearParams

Returns value for sufficient memory, time when sufficent memeory is available and
time at 16 MB available memory.

*/
   virtual bool getLinearParams(
            const size_t NoTuples, const size_t sizeOfTuple,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB ) const { 
      sufficientMemory=0;
      timeAtSuffMemory=0;
      timeAt16MB=0;
      return false;
   }

   virtual bool getLinearParams(
            const size_t NoTuples1, const size_t sizeOfTuple1,
            const size_t NoTuples2, const size_t sizeOfTuple2,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB ) const{ 
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
   virtual bool getFunction(
            const size_t NoTuples, const size_t sizeOfTuple,
            int& functionType,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB,
            double& a, double& b, double& c, double& d) const{
       functionType=1;
       a=0;b=0;c=0;d=0;
       return getLinearParams(NoTuples, sizeOfTuple,
                              sufficientMemory, timeAtSuffMemory, 
                              timeAt16MB);  
  }  
            
   virtual bool getFunction(
            const size_t NoTuples1, const size_t sizeOfTuple1,
            const size_t NoTuples2, const size_t sizeOfTuple2,
            int& functionType,
            double& sufficientMemory, double& timeAtSuffMemory,
            double& timeAt16MB,
            double& a, double& b, double& c, double& d) const{
       functionType=1;
       a=0;b=0;c=0;d=0;
       return getLinearParams(NoTuples1, sizeOfTuple1,
                              NoTuples2, sizeOfTuple2,
                              sufficientMemory, timeAtSuffMemory, 
                              timeAt16MB);  
  }  
   
/*
1.4 ~calculateXPoints~

Determine both equation solving points (x1, x2) for a given
sufficentPoint

*/
void calculateXPoints(double sufficentPoint, double &x1, 
  double &x2) {

    x1 = 16;
    x2 = sufficentPoint * 0.8;
}

/*
1.5 ~resolveInverseProportionality~

This function calculates the parameter m and b for 
inverse proportionality function f(x) = m/x + b 
for the points (x1, y1) and (x2, y2).

This function will return true if the calculation was
successfully, false otherwise.

*/
bool resolveInverseProportionality(double x1, double y1, 
   double x2, double y2, double &m, double &b) {

   // invalid points
   if(x1 == 0 || x2 == 0) {
      return false;
   }

   // invalid input
   if(x1 == x2) {
      return false;
   }

   // resolve
   m = (x1 * x2 * (y1 - y2)) / (x2 - x1); 
   b = -1 * (m / x1) + y1;

   return true;
}


  protected:
    Supplier supplier;
    size_t returned;

  

};


/*
1.2 Creation function

*/

typedef CostEstimation*  (*CreateCostEstimation)();



#endif 

