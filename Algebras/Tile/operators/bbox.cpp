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

#include "bbox.h"
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
Template method bboxFunction calls bbox method of specific datatype
and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of bbox operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of bboxFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if bboxFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int bboxFunction(Word* pArguments,
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
        typename Properties::RectangleType* pResult =
        static_cast<typename Properties::RectangleType*>(rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined())
          {
            pType->bbox(*pResult);
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
definition of bboxFunctions array.

*/

ValueMapping bboxFunctions[] =
{
  bboxFunction<tint, tProperties<int> >,
  bboxFunction<treal, tProperties<double> >,
  bboxFunction<tbool, tProperties<char> >,
  bboxFunction<tstring, tProperties<std::string> >,
  bboxFunction<mtint, mtProperties<int> >,
  bboxFunction<mtreal, mtProperties<double> >,
  bboxFunction<mtbool, mtProperties<char> >,
  bboxFunction<mtstring, mtProperties<std::string> >,
  0
};

/*
Method bboxSelectFunction returns the index of specific bbox function
in bboxFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of bbox operator
return value: index of specific bbox function in bboxFunctions
exceptions: -

*/

int bboxSelectFunction(ListExpr arguments)
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
Method bboxTypeMappingFunction returns the return value type
of bbox operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of bbox operator
return value: return value type of bbox operator
exceptions: -

*/

ListExpr bboxTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator bbox expects "
                                   "a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(1))
  {
    std::string argument1 = argumentsList.first().str();

    if(IstType(argument1))
    {
      type = NList(Rectangle<2>::BasicType()).listExpr();
    }
    
    else if(IsmtType(argument1))
    {
      type = NList(Rectangle<3>::BasicType()).listExpr();
    }
  }

  return type;
}

}
