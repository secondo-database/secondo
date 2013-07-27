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

#include "minimum.h"
#include "../t/tint.h"
#include "../t/treal.h"
#include "../t/tbool.h"
#include "../t/tstring.h"
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
Template method minimumFunction calls minimum method of specific datatype
and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of minimum operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of minimumFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if minimumFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int minimumFunction(Word* pArguments,
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
        typename Properties::TypeProperties::WrapperType* pResult =
        static_cast<typename Properties::TypeProperties::WrapperType*>
        (rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined())
          {
            *pResult = Properties::TypeProperties::GetWrappedValue
                       (pType->minimum());
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
definition of minimumFunctions array.

*/

ValueMapping minimumFunctions[] =
{
  minimumFunction<tint, tProperties<int> >,
  minimumFunction<treal, tProperties<double> >,
  minimumFunction<tbool, tProperties<char> >,
  minimumFunction<tstring, tProperties<std::string> >,
  minimumFunction<mtint, mtProperties<int> >,
  minimumFunction<mtreal, mtProperties<double> >,
  minimumFunction<mtbool, mtProperties<char> >,
  minimumFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
Method minimumSelectFunction returns the index of specific minimum function
in minimumFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of minimum operator
return value: index of specific minimum function in minimumFunctions
exceptions: -

*/

int minimumSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();
      const int TYPE_NAMES = 8;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        tint::BasicType(),
        treal::BasicType(),
        tbool::BasicType(),
        tstring::BasicType(),
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

  return functionIndex;
}

/*
Method minimumTypeMappingFunction returns the return value type
of minimum operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of minimum operator
return value: return value type of minimum operator
exceptions: -

*/

ListExpr minimumTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator minimum expects "
                                   "a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(IstType(argument1) ||
       IsmtType(argument1))
    {
      type = NList(GetValueWrapperType(argument1)).listExpr();
    }
  }

  return type;
}

}
