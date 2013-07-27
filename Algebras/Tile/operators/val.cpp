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

#include "val.h"
#include "../it/itint.h"
#include "../it/itreal.h"
#include "../it/itbool.h"
#include "../it/itstring.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template method valFunction calls val method of specific datatype
and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of val operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of valFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if valFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int valFunction(Word* pArguments,
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
        typename Properties::tType* pResult = static_cast
                                              <typename Properties::tType*>
                                              (rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined())
          {
            pType->val(*pResult);
          }

          else
          {
            pResult->SetDefined(false);
          }

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of valFunctions array.

*/

ValueMapping valFunctions[] =
{
  valFunction<itint, itProperties<int> >,
  valFunction<itreal, itProperties<double> >,
  valFunction<itbool, itProperties<char> >,
  valFunction<itstring, itProperties<std::string> >,
  0
};

/*
Method valSelectFunction returns the index of specific val function
in valFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of val operator
return value: index of specific val function in valFunctions
exceptions: -

*/

int valSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();
      const int TYPE_NAMES = 4;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        itint::BasicType(),
        itreal::BasicType(),
        itbool::BasicType(),
        itstring::BasicType(),
      };

      for(int i = 0; i < TYPE_NAMES; i++)
      {
        if(argument1.isSymbol(TYPE_NAMES_ARRAY[i]))
        {
          functionIndex = i;
          break;
        }
      }
    }
  }

  return functionIndex;
}

/*
Method valTypeMappingFunction returns the return value type
of val operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of val operator
return value: return value type of val operator
exceptions: -

*/

ListExpr valTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator val expects an it type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(IsitType(argument1))
    {
      type = NList(GettType(argument1)).listExpr();
    }
  }

  return type;
}

}
