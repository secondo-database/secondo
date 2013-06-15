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

#ifndef TILEALGEBRA_DEFTIME_H
#define TILEALGEBRA_DEFTIME_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "TemporalAlgebra.h"

namespace TileAlgebra
{

/*
declaration of deftime functions

*/

extern ValueMapping deftimeFunctions[];

/*
declaration of deftime select function

*/

int deftimeSelectFunction(ListExpr arguments);

/*
declaration of deftime type mapping function

*/

ListExpr deftimeTypeMapping(ListExpr arguments);

/*
definition of deftime Operator Info structure

*/

struct deftimeInfo : OperatorInfo
{
  deftimeInfo()
  {
    name      = "deftime";
    signature = "mtT -> " + Periods::BasicType();
    syntax    = "deftime(_)";
    meaning   = "Returns the defined periods of a mt type.";
  }
};

/*
definition of template deftime function

*/

template <typename Type>
int deftimeFunction(Word* pArguments,
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

    if(pType != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        Periods* pResult = static_cast<Periods*>(rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined())
          {
            pType->deftime(*pResult);
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

#endif // TILEALGEBRA_DEFTIME_H
