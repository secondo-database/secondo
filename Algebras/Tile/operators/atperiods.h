/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

#ifndef TILEALGEBRA_ATPERIODS_H
#define TILEALGEBRA_ATPERIODS_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "TemporalAlgebra.h"

namespace TileAlgebra
{

/*
declaration of atperiods functions

*/

extern ValueMapping atperiodsFunctions[];

/*
declaration of atperiods select function

*/

int atperiodsSelectFunction(ListExpr arguments);

/*
declaration of atperiods type mapping function

*/

ListExpr atperiodsTypeMappingFunction(ListExpr arguments);

/*
definition of atperiods Operator Info structure

*/

struct atperiodsInfo : OperatorInfo 
{
  atperiodsInfo()
  { 
    name      = "atperiods";
    signature = "mtT atperiods " + Periods::BasicType() + " -> mtT";
    syntax    = "atperiods(_)";
    meaning   = "Restricts values to periods.";
  }          
};

/*
definition of template atperiodsFunction

*/

template <typename Type>
int atperiodsFunction(Word* pArguments,
                      Word& rResult,
                      int message,
                      Word& rLocal,
                      Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    Type* pType = static_cast<Type*>(pArguments[0].addr);
    Periods* pPeriods = static_cast<Periods*>(pArguments[1].addr);

    if(pType != 0 &&
       pPeriods != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        Type* pResult = static_cast<Type*>(rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined() &&
             pPeriods->IsDefined())
          {
            pType->atperiods(*pPeriods, *pResult);
          }

          else
          {
            pResult->SetDefined(false);
          }
        }
      }
    }
  }

  return nRetVal;
}

}

#endif // TILEALGEBRA_ATPERIODS_H
