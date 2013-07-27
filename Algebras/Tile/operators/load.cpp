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

/*
TileAlgebra includes

*/

#include "load.h"
#include "../t/tintArray.h"
#include "../t/tintFlob.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template method loadFunction calls load method of specific datatype
and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of load operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of loadFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if loadFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type>
int loadFunction(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    Type* pType = static_cast<Type*>(pArguments[0].addr);

    if(pType != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        CcBool* pResult = static_cast<CcBool*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->Set(true, pType->load());
          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of loadFunctions array.

*/

ValueMapping loadFunctions[] =
{
  loadFunction<tintArray>,
  loadFunction<tintFlob>,
  0
};

/*
Method loadSelectFunction returns the index of specific load function
in loadFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of load operator
return value: index of specific load function in loadFunctions
exceptions: -

*/

int loadSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();

      if(argument1.isSymbol(tintArray::BasicType()))
      {
        functionIndex = 0;
      }

      else if(argument1.isSymbol(tintFlob::BasicType()))
      {
        functionIndex = 1;
      }
    }
  }

  return functionIndex;
}

/*
Method loadTypeMappingFunction returns the return value type
of load operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of load operator
return value: return value type of load operator
exceptions: -

*/

ListExpr loadTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator load expects "
                                   "a tintArray or a tintFlob.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(argument1 == tintArray::BasicType() ||
       argument1 == tintFlob::BasicType())
    {
      type = NList(CcBool::BasicType()).listExpr();
    }
  }

  return type;
}

}
