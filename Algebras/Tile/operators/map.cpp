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
SECONDO includes

*/

#include "RectangleAlgebra.h"

/*
TileAlgebra includes

*/

#include "map.h"
#include "../grid/tgrid.h"
#include "../grid/mtgrid.h"
#include "../Index/Index.h"
#include "../Types/Types.h"
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
Template method mapFunctiont implements the map operator functionality
for t datatypes.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of map operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of mapFunctiont
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if mapFunctiont successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType, typename DestinationTypeProperties>
int mapFunctiont(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType* pSourceType = static_cast<SourceType*>(pArguments[0].addr);
    Address pFunction = pArguments[1].addr;

    if(pSourceType != 0 &&
       pFunction != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType->IsDefined())
          {
            Index<2> minimumIndex;
            Index<2> maximumIndex;
            bool bOK = pSourceType->GetBoundingBoxIndexes(minimumIndex,
                                                          maximumIndex);

            if(bOK == true)
            {
              pResult->SetDefined(true);

              tgrid grid;
              pSourceType->getgrid(grid);
              pResult->SetGrid(grid);

              ArgVector& argumentsVector = *qp->Argument(pFunction);
              Word word;

              for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
              {
                for(int column = minimumIndex[0]; column < maximumIndex[0];
                    column++)
                {
                  Index<2> index((int[]){column, row});
                  typename SourceTypeProperties::TypeProperties::PropertiesType
                  value = pSourceType->GetValue(index);

                  if(SourceTypeProperties::TypeProperties::
                     IsUndefinedValue(value) == false)
                  {
                    typename SourceTypeProperties::TypeProperties::WrapperType
                    wrappedValue = SourceTypeProperties::TypeProperties::
                                   GetWrappedValue(value);

                    argumentsVector[0].setAddr(&wrappedValue);
                    qp->Request(pFunction, word);

                    if(word.addr != 0)
                    {
                      typename DestinationTypeProperties::TypeProperties::
                      WrapperType* pMappedValue = static_cast<typename
                      DestinationTypeProperties::TypeProperties::WrapperType*>
                      (word.addr);

                      if(pMappedValue != 0)
                      {
                        typename DestinationTypeProperties::TypeProperties::
                        PropertiesType unwrappedValue =
                        DestinationTypeProperties::TypeProperties::
                        GetUnwrappedValue(*pMappedValue);

                        if(DestinationTypeProperties::TypeProperties::
                           IsUndefinedValue(unwrappedValue) == false)
                        {
                          pResult->SetValue(index, unwrappedValue, true);
                        }
                      }
                    }
                  }
                }
              }
            }
          }

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
Template method mapFunctionmt implements the map operator functionality
for mt datatypes.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of map operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of mapFunctionmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if mapFunctionmt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType, typename DestinationTypeProperties>
int mapFunctionmt(Word* pArguments,
                  Word& rResult,
                  int message,
                  Word& rLocal,
                  Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType* pSourceType = static_cast<SourceType*>(pArguments[0].addr);
    Address pFunction = pArguments[1].addr;

    if(pSourceType != 0 &&
       pFunction != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType->IsDefined())
          {
            Index<3> minimumIndex;
            Index<3> maximumIndex;
            bool bOK = pSourceType->GetBoundingBoxIndexes(minimumIndex,
                                                          maximumIndex);

            if(bOK == true)
            {
              pResult->SetDefined(true);

              mtgrid grid;
              pSourceType->getgrid(grid);
              pResult->SetGrid(grid);

              ArgVector& argumentsVector = *qp->Argument(pFunction);
              Word word;

              for(int time = minimumIndex[2]; time < maximumIndex[2]; time++)
              {
                for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
                {
                  for(int column = minimumIndex[0]; column < maximumIndex[0];
                      column++)
                  {
                    Index<3> index((int[]){column, row, time});
                    typename SourceTypeProperties::TypeProperties::
                    PropertiesType value = pSourceType->GetValue(index);

                    if(SourceTypeProperties::TypeProperties::
                       IsUndefinedValue(value) == false)
                    {
                      typename SourceTypeProperties::TypeProperties::WrapperType
                      wrappedValue = SourceTypeProperties::TypeProperties::
                                     GetWrappedValue(value);

                      argumentsVector[0].setAddr(&wrappedValue);
                      qp->Request(pFunction, word);

                      if(word.addr != 0)
                      {
                        typename DestinationTypeProperties::TypeProperties::
                        WrapperType* pMappedValue =
                        static_cast<typename DestinationTypeProperties::
                        TypeProperties::WrapperType*>(word.addr);

                        if(pMappedValue != 0)
                        {
                          typename DestinationTypeProperties::TypeProperties::
                          PropertiesType unwrappedValue =
                          DestinationTypeProperties::TypeProperties::
                          GetUnwrappedValue(*pMappedValue);

                          if(DestinationTypeProperties::TypeProperties::
                             IsUndefinedValue(unwrappedValue) == false)
                          {
                            pResult->SetValue(index, unwrappedValue, true);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of mapFunctions array.

*/

ValueMapping mapFunctions[] =
{
  mapFunctiont<tint, tProperties<int>,
               tint, tProperties<int> >,
  mapFunctiont<tint, tProperties<int>,
               treal, tProperties<double> >,
  mapFunctiont<tint, tProperties<int>,
               tbool, tProperties<char> >,
  mapFunctiont<tint, tProperties<int>,
               tstring, tProperties<std::string> >,
  
  mapFunctiont<treal, tProperties<double>,
               tint, tProperties<int> >,
  mapFunctiont<treal, tProperties<double>,
               treal, tProperties<double> >,
  mapFunctiont<treal, tProperties<double>,
               tbool, tProperties<char> >,
  mapFunctiont<treal, tProperties<double>,
               tstring, tProperties<std::string> >,
          
  mapFunctiont<tbool, tProperties<char>,
               tint, tProperties<int> >,
  mapFunctiont<tbool, tProperties<char>,
               treal, tProperties<double> >,
  mapFunctiont<tbool, tProperties<char>,
               tbool, tProperties<char> >,
  mapFunctiont<tbool, tProperties<char>,
               tstring, tProperties<std::string> >,

  mapFunctiont<tstring, tProperties<std::string>,
               tint, tProperties<int> >,
  mapFunctiont<tstring, tProperties<std::string>,
               treal, tProperties<double> >,
  mapFunctiont<tstring, tProperties<std::string>,
               tbool, tProperties<char> >,
  mapFunctiont<tstring, tProperties<std::string>,
               tstring, tProperties<std::string> >,
  
  mapFunctionmt<mtint, mtProperties<int>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtint, mtProperties<int>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtint, mtProperties<int>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtint, mtProperties<int>,
                mtstring, mtProperties<std::string> >,

  mapFunctionmt<mtreal, mtProperties<double>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtreal, mtProperties<double>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtreal, mtProperties<double>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtreal, mtProperties<double>,
                mtstring, mtProperties<std::string> >,

  mapFunctionmt<mtbool, mtProperties<char>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtbool, mtProperties<char>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtbool, mtProperties<char>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtbool, mtProperties<char>,
                mtstring, mtProperties<std::string> >,
                
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtint, mtProperties<int> >,
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtreal, mtProperties<double> >,
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtbool, mtProperties<char> >,
  mapFunctionmt<mtstring, mtProperties<std::string>,
                mtstring, mtProperties<std::string> >,
  0
};

/*
Method mapSelectFunction returns the index of specific map function
in mapFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of map operator
return value: index of specific map function in mapFunctions
exceptions: -

*/

int mapSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(2))
    {
      NList argument1 = argumentsList.first();
      std::string argument2 = argumentsList.second().third().str();
      
      int argument1Index = -1;
      int argument2Index = -1;

      const int TYPE_NAMES = 12;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        tint::BasicType(),
        treal::BasicType(),
        tbool::BasicType(),
        tstring::BasicType(),
        mtint::BasicType(),
        mtreal::BasicType(),
        mtbool::BasicType(),
        mtstring::BasicType(),
        CcInt::BasicType(),
        CcReal::BasicType(),
        CcBool::BasicType(),
        CcString::BasicType()
      };

      for(int i = 0; i < TYPE_NAMES; i++)
      {
        if(argument1.isSymbol(TYPE_NAMES_ARRAY[i]))
        {
          argument1Index = i;
        }
        
        if(argument2 == TYPE_NAMES_ARRAY[i])
        {
          argument2Index = i;
        }
      }
      
      if(argument1Index >= 0 &&
         argument2Index >= 0)
      {
        functionIndex = (argument1Index * 4) +
                        (argument2Index % 4);
      }
    }
  }

  return functionIndex;
}

/*
Method mapTypeMappingFunction returns the return value type
of map operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of map operator
return value: return value type of map operator
exceptions: -

*/

ListExpr mapTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator map expects two arguments.");

  if(arguments != 0)
  {
    NList argumentsList(arguments);
    
    if(argumentsList.hasLength(2))
    {
      std::string argument1 = argumentsList.first().str();
      
      if(IstType(argument1) ||
         IsmtType(argument1))
      {
        NList argument2 = argumentsList.second();

        if(listutils::isMap<1>(argument2.listExpr()))
        {
          std::string valueWrapperType = GetValueWrapperType(argument1);
          std::string functionArgument = argument2.second().str();
  
          if(valueWrapperType == functionArgument)
          {
            std::string functionResult = argument2.third().str();
            std::string typeName;
  
            if(IstType(argument1))
            {
              typeName = GettType(functionResult);
            }
  
            else
            {
              typeName = GetmtType(functionResult);
            }
  
            if(typeName.empty() == false)
            {
              type = NList(typeName).listExpr();
            }
  
            else
            {
              type = NList::typeError("Parameter function result "
                                      "is not a valid value type");
            }
          }
  
          else
          {
            type = NList::typeError("Function argument does not fit the type");
          }
        }
  
        else
        {
          type = NList::typeError("Second argument must be a function "
                                  "with 1 argument");
        }
      }
  
      else
      {
        type = NList::typeError("First argument must be a t type or "
                                "a mt type.");
      }
    }
  }

  return type;
}

}
