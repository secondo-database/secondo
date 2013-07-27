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

#include "atperiods.h"
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
Template method atperiodsFunction calls atperiods method of specific datatype
and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of atperiods operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of atperiodsFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if atperiodsFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type>
int atperiodsFunction(Word* pArguments,
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

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of atperiodsFunctions array.

*/

ValueMapping atperiodsFunctions[] =
{
  atperiodsFunction<mtint>,
  atperiodsFunction<mtreal>,
  atperiodsFunction<mtbool>,
  atperiodsFunction<mtstring>,
  0
};

/*
Method atperiodsSelectFunction returns the index of specific atperiods function
in atperiodsFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of atperiods operator
return value: index of specific atperiods function in atperiodsFunctions
exceptions: -

*/

int atperiodsSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();

      if(argument2.isSymbol(Periods::BasicType()))
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
Method atperiodsTypeMappingFunction returns the return value type
of atperiods operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of atperiods operator
return value: return value type of atperiods operator
exceptions: -

*/

ListExpr atperiodsTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator atperiods expects "
                                   "a mt type and a periods.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(IsmtType(argument1) &&
       argument2 == Periods::BasicType())
    {
      type = NList(argument1).listExpr();
    }
  }

  return type;
}

}
