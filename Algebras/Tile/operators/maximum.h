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

#ifndef TILEALGEBRA_MAXIMUM_H
#define TILEALGEBRA_MAXIMUM_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"

namespace TileAlgebra
{

/*
declaration of maximum functions

*/

extern ValueMapping maximumFunctions[];

/*
declaration of maximum select function

*/

int maximumSelectFunction(ListExpr arguments);

/*
declaration of maximum type mapping function

*/

ListExpr maximumTypeMappingFunction(ListExpr arguments);

/*
definition of maximum Operator Info structure

*/

struct maximumInfo : OperatorInfo
{
  maximumInfo()
  {
    name      = "maximum";
    signature = "tT -> T";
    appendSignature("mtT -> T");
    syntax    = "maximum(_)";
    meaning   = "Returns the maximum value of a t type or a mt type.";
  }
};

/*
definition of template maximumFunction

*/

template <typename Type, typename Properties>
int maximumFunction(Word* pArguments,
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
        typename Properties::TypeProperties::WrapperType* pResult =
        static_cast<typename Properties::TypeProperties::WrapperType*>
        (rResult.addr);
        
        if(pResult != 0)
        {
          *pResult = Properties::TypeProperties::GetWrappedValue
                     (pType->maximum());
        }
      }
    }
  }

  return nRetVal;
}

}

#endif // TILEALGEBRA_MAXIMUM_H
