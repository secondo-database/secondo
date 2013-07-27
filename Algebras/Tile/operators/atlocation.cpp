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

#include "atlocation.h"
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
Template method atlocationFunctiont calls atlocation method
of specific t datatype and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of atlocation call
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of atlocationFunctiont
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if atlocationFunctiont successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int atlocationFunctiont(Word* pArguments,
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
    Point* pPoint = static_cast<Point*>(pArguments[1].addr);

    if(pType != 0 &&
       pPoint != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::TypeProperties::WrapperType* pResult =
        static_cast<typename Properties::TypeProperties::WrapperType*>
        (rResult.addr);

        if(pResult != 0)
        {
          if(pType->IsDefined() &&
             pPoint->IsDefined())
          {
            pType->atlocation(pPoint->GetX(), pPoint->GetY(), *pResult);
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
Template method atlocationFunctionmt calls atlocation method
of specific mt datatype and returns the result of this call.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of atlocation call
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of atlocationFunctionmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if atlocationFunctionmt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int atlocationFunctionmt(Word* pArguments,
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
    Point* pPoint = static_cast<Point*>(pArguments[1].addr);

    if(pType != 0 &&
       pPoint != 0)
    {
      if(qp->GetNoSons(supplier) == 2)
      {
        rResult = qp->ResultStorage(supplier);

        if(rResult.addr != 0)
        {
          typename Properties::TypeProperties::MType* pResult =
          static_cast<typename Properties::TypeProperties::MType*>
          (rResult.addr);

          if(pResult != 0)
          {
            if(pType->IsDefined() &&
               pPoint->IsDefined())
            {
              pType->atlocation(pPoint->GetX(), pPoint->GetY(), *pResult);
            }

            else
            {
              pResult->SetDefined(false);
            }

            nRetVal = 0;
          }
        }
      }

      else
      {
        Instant* pInstant = static_cast<Instant*>
                                       (pArguments[2].addr);

        if(pInstant != 0)
        {
          rResult = qp->ResultStorage(supplier);

          if(rResult.addr != 0)
          {
            typename Properties::TypeProperties::WrapperType* pResult =
            static_cast<typename Properties::TypeProperties::WrapperType*>
            (rResult.addr);

            if(pResult != 0)
            {
              if(pType->IsDefined() &&
                 pPoint->IsDefined() &&
                 pInstant->IsDefined())
              {
                pType->atlocation(pPoint->GetX(), pPoint->GetY(),
                                  pInstant->ToDouble(), *pResult);
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
    }
  }

  return nRetVal;
}

/*
definition of atlocationFunctions array.

*/

ValueMapping atlocationFunctions[] =
{
  atlocationFunctiont<tint, tProperties<int> >,
  atlocationFunctiont<treal, tProperties<double> >,
  atlocationFunctiont<tbool, tProperties<char> >,
  atlocationFunctiont<tstring, tProperties<std::string> >,
  atlocationFunctionmt<mtint, mtProperties<int> >,
  atlocationFunctionmt<mtreal, mtProperties<double> >,
  atlocationFunctionmt<mtbool, mtProperties<char> >,
  atlocationFunctionmt<mtstring, mtProperties<std::string> >,
  0
};

/*
Method atlocationSelectFunction returns the index of specific
atlocation function in atlocationFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of atlocation operator
return value: index of specific atlocation function in atlocationFunctions
exceptions: -

*/

int atlocationSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);
    NList argument1 = argumentsList.first();
    NList argument2 = argumentsList.second();

    if(argument2.isSymbol(Point::BasicType()))
    {
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
Method atlocationTypeMappingFunction returns the return value type
of atlocation operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of atlocation operator
return value: return value type of atlocation operator
exceptions: -

*/

ListExpr atlocationTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator atlocation expects "
                                   "a t type or a mt type "
                                   "and a point or a point and an instant.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(argument2 == Point::BasicType())
    {
      if(IstType(argument1))
      {
        type = NList(GetValueWrapperType(argument1)).listExpr();
      }

      else if(IsmtType(argument1))
      {
        type = NList(GetMType(argument1)).listExpr();
      }
    }
  }

  else if(argumentsList.hasLength(3))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();
    std::string argument3 = argumentsList.third().str();

    if(IsmtType(argument1) &&
       argument2 == Point::BasicType() &&
       argument3 == Instant::BasicType())
    {
      type = NList(GetValueWrapperType(argument1)).listExpr();
    }
  }

  return type;
}

}
