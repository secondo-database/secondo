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

#include "atinstant.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template method atinstantFunction calls atinstant method of specific datatype
and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of atinstant operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of atinstantFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if atinstantFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int atinstantFunction(Word* pArguments,
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
    Instant* pInstant = static_cast<Instant*>(pArguments[1].addr);

    if(pType != 0 &&
       pInstant != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::itType* pResult =
        static_cast<typename Properties::itType*>(rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined() &&
             pInstant->IsDefined())
          {
            pType->atinstant(*pInstant, *pResult);
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
definition of atinstantFunctions array.

*/

ValueMapping atinstantFunctions[] =
{
  atinstantFunction<mtint, mtProperties<int> >,
  atinstantFunction<mtreal, mtProperties<double> >,
  atinstantFunction<mtbool, mtProperties<char> >,
  atinstantFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
Method atinstantSelectFunction returns the index of specific atinstant function
in atinstantFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of atinstant operator
return value: index of specific atinstant function in atinstantFunctions
exceptions: -

*/

int atinstantSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();

      if(argument2.isSymbol(Instant::BasicType()))
      {
        const int TYPE_NAMES = 4;
        const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
        {
          mtint::BasicType(),
          mtreal::BasicType(),
          mtbool::BasicType(),
          mtstring::BasicType()
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
  }

  return functionIndex;
}

/*
Method atinstantTypeMappingFunction returns the return value type
of atinstant operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of atinstant operator
return value: return value type of atinstant operator
exceptions: -

*/

ListExpr atinstantTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator atinstant expects "
                                   "a mt type and an instant.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(IsmtType(argument1) &&
       argument2 == Instant::BasicType())
    {
      type = NList(GetitType(argument1)).listExpr();
    }
  }

  return type;
}

}
