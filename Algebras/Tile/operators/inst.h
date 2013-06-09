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

#ifndef TILEALGEBRA_INST_H
#define TILEALGEBRA_INST_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "DateTime.h"

namespace TileAlgebra
{

/*
declaration of inst functions

*/

extern ValueMapping instFunctions[];

/*
declaration of inst select function

*/

int instSelectFunction(ListExpr arguments);

/*
declaration of inst type mapping function

*/

ListExpr instTypeMapping(ListExpr arguments);

/*
definition of inst Operator Info structure

*/

struct instInfo : OperatorInfo
{
  instInfo()
  {
    name      = "inst";
    signature = "itT -> " + datetime::DateTime::BasicType();
    syntax    = "inst(_)";
    meaning   = "Returns the instant value of an it type.";
  }
};

/*
definition of template inst function

*/

template <typename Type, typename Properties>
int instFunction(Word* pArguments,
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
        datetime::DateTime* pResult = static_cast<datetime::DateTime*>
                                      (rResult.addr);
        
        if(pResult != 0)
        {
          *pResult = pType->inst();
        }
      }
    }
  }

  return nRetVal;
}

}

#endif // TILEALGEBRA_INST_H
