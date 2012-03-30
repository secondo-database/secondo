
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

  protected:
    Supplier supplier;
    size_t returned;

  

};


/*
1.2 Creation function

*/

typedef CostEstimation*  (*CreateCostEstimation)();



#endif 

